/*
 *  KernelEx
 *  Copyright (C) 2008-2009, Xeno86
 *
 *  This file is part of KernelEx source code.
 *
 *  KernelEx is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; version 2 of the License.
 *
 *  KernelEx is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <windows.h>
#include <cstdio>
#include "internals.h"
#include "resolver.h"
#include "debug.h"
#include "pemanip.h"

static bool is_winme;
HINSTANCE hInstance;

IMTE*** ppmteModTable = NULL;
HMODULE h_kernel32;
CRITICAL_SECTION* krnl32lock = NULL;
PDB98** pppdbCur = NULL;
WORD* pimteMax = NULL;

MRFromHLib_t MRFromHLib = NULL;
TIDtoTDB_t TIDtoTDB = NULL;
PIDtoPDB_t PIDtoPDB = NULL;
MRLoadTree_t MRLoadTree = NULL;
FreeLibTree_t FreeLibTree = NULL;
FLoadTreeNotify_t FLoadTreeNotify = NULL;
FreeLibRemove_t FreeLibRemove = NULL;

sstring kernelex_dir("");
sstring own_path("");

bool isWinMe()
{
	return is_winme;
}

void ShowError(UINT id, ...)
{
	char format[512];
	char out[512];
	va_list vargs;
	
	va_start(vargs, id);
	if (!LoadString(hInstance, id, format, sizeof(format)))
		sprintf(out, "ERROR: Missing string resource %d", id);
	else
		_vsnprintf(out, sizeof(out), format, vargs);
	va_end(vargs);
	MessageBox(NULL, out, "KernelEx Core", MB_OK | MB_ICONERROR);
}

DWORD* find_unique_pattern(void* start, int size, const short* pattern, int pat_len, const char* pat_name)
{
	unsigned char* pos = (unsigned char*) start;
	unsigned char* end_pos = pos + size - pat_len;
	unsigned char* found_loc = NULL;
	int found = 0;

	while (pos <= end_pos)
	{
		if (pattern[0] < 0 || *pos == pattern[0])
		{
			int j;
			for (j = 1 ; j < pat_len ; j++)
			{
				if (pattern[j] >= 0 && pos[j] != pattern[j])
					break;
			}
			if (j == pat_len) //pattern found
			{
				found++;
				if (!found_loc)
					found_loc = pos;
			}
		}
		pos++;
	}

	if (found != 1)
	{
		if (!found)
			DBGPRINTF(("%s: pattern not found\n", pat_name));
		else
			DBGPRINTF(("%s: pattern not unique, found occurrences: %d\n", pat_name, found));
		return NULL;
	}

	for (int i = 0 ; i < pat_len ; i++)
		if (pattern[i] == -2)
			return (DWORD*) &found_loc[i];

	DBGPRINTF(("%s: invalid pattern\n", pat_name));
	return NULL;
}

static DWORD decode_calljmp(DWORD* addr)
{
	unsigned char* code = (unsigned char*)addr;

	if (code[-1] == 0xe8 || code[-1] == 0xe9) /* call/jmp rel32 */
	{
		return (DWORD)(code + 4 + *(DWORD*)code);
	}
	else if (code[-2] == 0xff 
			&& (code[-1] == 0x15 || code[-1] == 0x25)) /* call/jmp m32 */
	{
		return *(DWORD*)code;
	}
	else if (code[-1] == 0xeb) /* jmp rel8 */
	{
		return (DWORD)(code + 1 + *(char*)code);
	}
	else return 0;
}

MODREF* MRfromCallerAddr(DWORD addr)
{
	MODREF* mr;
	PDB98* ppdbCur = *pppdbCur;
	IMTE** pmteModTable = *ppmteModTable;

	mr = ppdbCur->pExeMODREF;
	if (mr)
	{
		IMTE* imte = pmteModTable[mr->mteIndex];
		IMAGE_NT_HEADERS* nthdr = imte->pNTHdr;
		DWORD img_base = nthdr->OptionalHeader.ImageBase;
		if (addr >= img_base && addr < img_base + nthdr->OptionalHeader.SizeOfImage)
			return mr;
	}
	mr = ppdbCur->MODREFList;
	if (mr)
	{
		do 
		{
			IMTE* imte = pmteModTable[mr->mteIndex];
			IMAGE_NT_HEADERS* nthdr = imte->pNTHdr;
			DWORD img_base = nthdr->OptionalHeader.ImageBase;
			if (addr >= img_base && addr < img_base + nthdr->OptionalHeader.SizeOfImage)
				return mr;
			mr = mr->pNextModRef;
		} 
		while (mr);
	}
	return NULL;
}

/* find win32 mutex */
static CRITICAL_SECTION* find_krnl32lock()
{
	CRITICAL_SECTION* ret;

	const char* pat_name = "Win32 lock";
	short pat[] = {0x55,0xA1,-2,-2,-2,-2,0x8B,0xEC,0x56,0x57,0x33,0xF6,0x50,0xE8};
	int pat_len = sizeof(pat) / sizeof(short);

	DWORD* res = find_unique_pattern((void*) iGetProcAddress(h_kernel32, "VirtualQueryEx"), pat_len, pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = *(CRITICAL_SECTION**)*res;
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

/* find current process PDB */
static PDB98** find_curPDB()
{
	PDB98** ret;

	const char* pat_name = "pppdbCur";
	short pat[] = {0xA1,-2,-2,-2,-2,0xFF,0x30,0xE8,-1,-1,-1,-1,0xC3};
	int pat_len = sizeof(pat) / sizeof(short);

	DWORD* res = find_unique_pattern((void*) iGetProcAddress(h_kernel32, "GetCurrentProcessId"), pat_len, pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = *(PDB98***)*res;
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

/* find global module tables */
static IMTE*** find_mod_table()
{
	IMTE*** ret;
	
	const char* pat_name = "Module table";
	short pat[] = {0x8B,0x0D,-2,-2,-2,-2};
	int pat_len = sizeof(pat) / sizeof(short);

	DWORD* res = find_unique_pattern((void*) iGetProcAddress(h_kernel32, (LPSTR)23), 0x20, pat, pat_len, pat_name);
	
	ret = (IMTE***)*res;
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static MRFromHLib_t find_MRFromHLib()
{
	MRFromHLib_t ret;

	const char* pat_name = "MRFromHLib";
	short pat[] = {0xE8,-2,-2,-2,-2};
	int pat_len = sizeof(pat) / sizeof(short);

	DWORD* res = find_unique_pattern((void*) iGetProcAddress(h_kernel32, (LPSTR)23), 0x20, pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (MRFromHLib_t)decode_calljmp(res);
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static WORD* find_pimteMax()
{
	WORD* ret;
	PEmanip pe(h_kernel32);
	if (!pe.HasTarget())
		return NULL;
	
	const char* pat_name = "pimteMax";
	short pat[] = {0x66,0x8B,0x44,0x24,0x04,0x66,-1,0x05,-2,-2,-2,-2,-1,0x17,0x8B,0x0D,-1,-1,-1,-1,0x0F,0xBF,0xC0,0x8B,0x04,0x81,0x85,0xC0,0x74,0x07,0x8B,0x40,0x04,0x85,0xC0,0x75,0x09};
	int pat_len = sizeof(pat) / sizeof(short);
	const char* sec_name = ".text";

	DWORD* res = find_unique_pattern(pe.GetSectionByName(sec_name), pe.GetSectionSize(sec_name), pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (WORD*) *res;
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static TIDtoTDB_t find_TIDtoTDB()
{
	TIDtoTDB_t ret;
	PEmanip pe(h_kernel32);
	if (!pe.HasTarget())
		return NULL;

	const char* pat_name = "TIDtoTDB";
	short pat[] = {0x89,-1,0xFF,0x75,0xFC,0xFF,0x77,0x14,0xE8,-2,-2,-2,-2,0x50};
	int pat_len = sizeof(pat) / sizeof(short);
	const char* sec_name = ".text";
	
	DWORD* res = find_unique_pattern(pe.GetSectionByName(sec_name), pe.GetSectionSize(sec_name), pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (TIDtoTDB_t)decode_calljmp(res);
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static PIDtoPDB_t find_PIDtoPDB()
{
	PIDtoPDB_t ret;

	const char* pat_name = "PIDtoPDB";
	short pat[] = {0xFF,0x74,0x24,0x0C,0xE8,-2,-2,-2,-2};
	int pat_len = sizeof(pat) / sizeof(short);
	
	DWORD* res = find_unique_pattern((void*) iGetProcAddress(h_kernel32, "OpenProcess"), pat_len, pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (PIDtoPDB_t)decode_calljmp(res);
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static MRLoadTree_t find_MRLoadTree()
{
	MRLoadTree_t ret;
	PEmanip pe(h_kernel32);
	if (!pe.HasTarget())
		return NULL;

	const char* pat_name = "MRLoadTree";
	short pat[] = {0x33,0xF6,0xE8,-1,-1,-1,-1,0x39,0x35,-1,-1,-1,-1,0x74,0x11,0xA1,-1,-1,-1,-1,0x50,0xE8,-1,-1,-1,-1,0x89,0x35,-1,-1,-1,-1,0xFF,0x74,0x24,0x14,0xE8,-2,-2,-2,-2,0x8B,0xF0,0x85,0xF6};
	int pat_len = sizeof(pat) / sizeof(short);
	const char* sec_name = ".text";

	DWORD* res = find_unique_pattern(pe.GetSectionByName(sec_name), pe.GetSectionSize(sec_name), pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (MRLoadTree_t)decode_calljmp(res);
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static FreeLibTree_t find_FreeLibTree()
{
	FreeLibTree_t ret;
	
	const char* pat_name = "FreeLibTree";
	short pat[] = {0x75,0x09,0x6A,0x06,0xE8,-1,-1,-1,-1,0xEB,0x08,0x50,0xE8,-2,-2,-2,-2,0x8B,0xF0};
	int pat_len = sizeof(pat) / sizeof(short);
	
	DWORD* res = find_unique_pattern((void*) iGetProcAddress(h_kernel32, "FreeLibrary"), 0x80, pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (FreeLibTree_t)decode_calljmp(res);
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static FLoadTreeNotify_t find_FLoadTreeNotify()
{
	FLoadTreeNotify_t ret;
	PEmanip pe(h_kernel32);
	if (!pe.HasTarget())
		return NULL;

	const char* pat_name = "FLoadTreeNotify";
	short pat[] = {0x56,0xA1,-1,-1,-1,-1,0x6A,0x01,0x8B,0x08,0xFF,0xB1,0x98,0x00,0x00,0x00,0xE8,-2,-2,-2,-2,0x83,0xF8,0x01,0x1B,0xF6,0xF7,0xDE,0x85,0xF6};
	int pat_len = sizeof(pat) / sizeof(short);
	const char* sec_name = ".text";

	DWORD* res = find_unique_pattern(pe.GetSectionByName(sec_name), pe.GetSectionSize(sec_name), pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (FLoadTreeNotify_t)decode_calljmp(res);
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static FreeLibRemove_t find_FreeLibRemove()
{
	FreeLibRemove_t ret;
	PEmanip pe(h_kernel32);
	if (!pe.HasTarget())
		return NULL;

	const char* pat_name = "FreeLibRemove";
	short pat[] = {0x8B,0xF0,0x85,0xF6,0x75,0x05,0xE8,-2,-2,-2,-2,0xA1,-1,-1,-1,-1,0x50};
	int pat_len = sizeof(pat) / sizeof(short);
	const char* sec_name = ".text";

	DWORD* res = find_unique_pattern(pe.GetSectionByName(sec_name), pe.GetSectionSize(sec_name), pat, pat_len, pat_name);
	if (!res)
		return NULL;

	ret = (FreeLibRemove_t)decode_calljmp(res);
	DBGPRINTF(("%s @ 0x%08x\n", pat_name, ret));
	return ret;
}

static bool find_kernelex_install_dir()
{
	//registry value InstallDir is written by the installer
	HKEY key;
	DWORD type;
	char path[MAX_PATH];
	DWORD len = sizeof(path);
	DWORD attr;
	long result;

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\KernelEx", 
			0, KEY_QUERY_VALUE, &key);
	if (result != ERROR_SUCCESS)
	{
		DBGPRINTF(("Failed to open KernelEx registry key\n"));
		return false;
	}

	result = RegQueryValueEx(key, "InstallDir", NULL, &type, 
			(BYTE*)path, &len);
	RegCloseKey(key);
	if (result != ERROR_SUCCESS || type != REG_SZ || len == 0)
	{
		DBGPRINTF(("Failed to read InstallDir registry value\n"));
		return false;
	}

	if (path[strlen(path) - 1] == '\\')
		path[strlen(path) - 1] = '\0';

	attr = GetFileAttributes(path);
	if (attr == 0xffffffff || !(attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		DBGPRINTF(("KernelEx directory [%s] doesn't exist\n", path));
		return false;
	}

	DBGPRINTF(("KernelEx directory: %s\n", path));
	strcat(path, "\\");
	kernelex_dir = strupr(path);
	return true;
}

int internals_init()
{
	DBGPRINTF(("KernelEx loaded @ %08x\n", hInstance));
	DBGPRINTF(("internals_init()\n"));
	h_kernel32 = GetModuleHandle("kernel32");
	ppmteModTable = find_mod_table();
	krnl32lock = find_krnl32lock();
	pppdbCur = find_curPDB();
	MRFromHLib = find_MRFromHLib();
	pimteMax = find_pimteMax();
	TIDtoTDB = find_TIDtoTDB();
	PIDtoPDB = find_PIDtoPDB();
	MRLoadTree = find_MRLoadTree();
	FreeLibTree = find_FreeLibTree();
	FLoadTreeNotify = find_FLoadTreeNotify();
	FreeLibRemove = find_FreeLibRemove();
	bool instdir_rslt = find_kernelex_install_dir();
	is_winme = (GetVersion() == 0xc0005a04);

	if (!h_kernel32 || !ppmteModTable || !krnl32lock || !pppdbCur || !MRFromHLib
			|| !pimteMax || !TIDtoTDB || !PIDtoPDB || !MRLoadTree || !FreeLibTree 
			|| !FLoadTreeNotify || !FreeLibRemove || !instdir_rslt)
		return 0;
	return 1;
}

void internals_uninit()
{
	DBGPRINTF(("internals_uninit()\n"));
}
