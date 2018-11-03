/*
 *  KernelEx
 *  Copyright (C) 2008-2010, Xeno86
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
#include <tlhelp32.h>
#include <cassert>
#include <cstdio>
#include <cstdarg>
#include "debug.h"
#include "pemanip.h"
#include "loadstub.h"
#include "setup.h"
#include "wininit.h"
#include "version.h"
#include "resource.h"

#define CODE_SEG ".text"
#define DATA_SEG ".data"
#define INIT_SEG "_INIT"


Setup::Setup(char* _backup_file) : backup_file(strupr(_backup_file))
{
	h_kernel32 = GetModuleHandle("KERNEL32");
	
	detect_old_version();
	detect_downgrade();

	pemem.OpenMemory(h_kernel32);
	if (!pemem.HasTarget())
		ShowError(IDS_FAILOPEN, "KERNEL32 memory image");

	version = get_signature_ver();

	OSVERSIONINFOA osv;
	osv.dwOSVersionInfoSize = sizeof(osv);

	GetVersionEx(&osv);
	if ((osv.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) 
			|| (osv.dwMajorVersion != 4)
			|| (osv.dwMinorVersion != 10 && osv.dwMinorVersion != 90))
		ShowError(IDS_WINVER);

	//on WinMe and file called "setup.exe" we get 4.10 (lol)
	if (osv.dwMinorVersion == 90)
	{
		DBGPRINTF(("Detected Windows Me\n"));
		is_winme = true;
	}
	else
	{
		DBGPRINTF(("Detected Windows 98\n"));
		is_winme = false;
	}
}

bool Setup::detect_old_version()
{
	typedef void (WINAPI *KV_t)(char *);
	KV_t h_KVersion;
	char buf[32];
	
	h_KVersion = (KV_t)GetProcAddress(h_kernel32, "KUPVersion");
	if (!h_KVersion)
		h_KVersion = (KV_t)GetProcAddress(h_kernel32, "KEXVersion");
	if (!h_KVersion)
		return false;
	h_KVersion(buf);
	if (strcmp(buf, "4") < 0)
		ShowError(IDS_OLDKEX);
	return true;
}

void Setup::detect_downgrade()
{
	typedef unsigned long (*kexGetKEXVersion_t)();
	kexGetKEXVersion_t kexGetKEXVersion_pfn;
	HMODULE h_kernelex;
	unsigned long version;
	
	h_kernelex = LoadLibrary("KERNELEX.DLL");

	//no KernelEx installed, continue
	if (!h_kernelex)
		return;

	kexGetKEXVersion_pfn = (kexGetKEXVersion_t) GetProcAddress(h_kernelex, "kexGetKEXVersion");
	version = kexGetKEXVersion_pfn();
	FreeLibrary(h_kernelex);

	//trying to downgrade - forbid
	if (version > VERSION_CODE)
		ShowError(IDS_DOWNGRADE);
}

int Setup::find_pattern(DWORD offset, int size, const short* pattern, 
						int pat_len, DWORD* found_loc)
{
	DWORD end_offset = offset + size - pat_len;
	int found = 0;

	for (DWORD i = offset ; i <= end_offset ; i++)
	{
		if (pattern[0] < 0 || *(unsigned char*)i == pattern[0])
		{
			int j;
			for (j = 1 ; j < pat_len ; j++)
			{
				if (pattern[j] >= 0 && *(unsigned char*)(i + j) != pattern[j]) 
					break;
			}
			if (j == pat_len) //pattern found
			{
				*found_loc = i;
				found++;
			}
		}
	}
	return found;
}

void Setup::set_pattern(DWORD loc, const short* new_pattern, int pat_len)
{
	unsigned char* offset = (unsigned char*) loc;
	const short* pat_ptr = new_pattern;

	while (pat_len--)
	{
		if (*pat_ptr != -1)
			*offset = *pat_ptr & 0xff;
		offset++;
		pat_ptr++;
	}
}

int Setup::get_signature_ver()
{
	DWORD ptr;
	KernelEx_dataseg* dseg = NULL;
	DWORD sign_len = sizeof(KEX_SIGNATURE) - 1;

	DWORD data = (DWORD) pemem.GetSectionByName(DATA_SEG);
	int data_size = pemem.GetSectionSize(DATA_SEG);

	ptr = data + data_size - sign_len;
	while (ptr >= data)
	{
		if (!memcmp((void*)ptr, KEX_SIGNATURE, sign_len))
		{
			dseg = (KernelEx_dataseg*) ptr;
			break;
		}
		ptr--;
	}

	if (!dseg)
	{
		DBGPRINTF(("Signature not found\n"));
		return -1;
	}
	else
		DBGPRINTF(("Signature found @ 0x%08x\n", ptr));

	return dseg->version;
}

void Setup::prepare_subsystem_check()
{
	static const short pattern[] = {
		0x66,0x8b,0x46,0x48,0x66,0x3d,0x04,0x00,0x0f,0x87,0x9c,0x01,0x00,0x00,
		0x75,0x0b,0x66,0x83,0x7e,0x4a,0x0a,0x0f,0x87,0x8f,0x01,0x00,0x00,0x66,
		0x81,0x7e,0x04,0x4c,0x01,
	};
	static const short after[] = {
		0x66,0x83,0x7E,0x48,0x04,0x75,0x05,0x66,0x83,0x7E,0x4A,0x0A,0xE9,0x00,
		0x00,0x00,0x00,0x0F,0x87,0x93,0x01,0x00,0x00,0x90,0x90,0x90,0x90,0x66,
		0x81,0x7e,0x04,0x4c,0x01,
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size, pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "subsystem_check");
		else ShowError(IDS_MULPAT, "subsystem_check");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "subsystem_check", 
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	set_pattern(found_loc, after, length);
	
	SubsysCheck_jmp = found_loc + 12;
	_SubsysCheckPerform = decode_jmp(SubsysCheck_jmp, 5);
}

void Setup::find_resource_check1()
{
	static const short pattern[] = {
		0x6A,0x00,0x8D,0x45,0xE0,0x50,0xFF,0x75,0xA4,0xFF,0x75,0xD0,0x8D,0x45,
		0xCC,0x50,0xFF,0x75,0x08,0xE8,-2,-2,-2,-2,0x85,0xC0
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size,pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "resource_check1");
		else ShowError(IDS_MULPAT, "resource_check1");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "resource_check1", 
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	GetOrdinal_call1 = found_loc + 19;
	_GetOrdinal = decode_call(GetOrdinal_call1, 5);
}

void Setup::find_resource_check2()
{
	static const short pattern[] = {
		0x66,0x8B,0x45,0xE0,0x66,0x2D,0x06,0x80,0x66,0x3D,0x01,0x00,0x1B,0xC0,
		0xF7,0xD8,0x50,0x8D,0x45,0xDE,0x50,0xFF,0x75,  -1,0xFF,0x75,0xD0,0x8D,
		0x45,0xCC,0x50,0xFF,0x75,0x08,0xE8,  -2,  -2,  -2,  -2,0x85,0xC0
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size,pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "resource_check2");
		else ShowError(IDS_MULPAT, "resource_check2");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "resource_check2", 
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	GetOrdinal_call2 = found_loc + 34;
}

//no named/rcdata resource types mirroring
void Setup::disable_named_and_rcdata_resources_mirroring()
{
	static const short pattern[] = {
		0x66,0x8B,0x40,0x0E,0x66,0x89,0x45,0xDA,0x8B,0x45,  -1,0x66,0x8B,0x48,
		0x0C,0x66,0x89,0x4D,0xD8,0x66,0x8B,0x45,0xE0,0x8B,0x4D,0xD4,0x66,0x89,
		0x01,0x83,0x45,0xD4,0x02,0x66,0x8B,0x45,0xDA,0x66,0x03,0x45,0xD8,0x66,
		0x89,0x45,0x8C,0x8B,0x4D,0xD4,0x66,0x89,0x01,0x83,0x45,0xD4,0x02
	};
	static const short after[] = {
		0x66,0x8B,0x48,0x0E,0x66,0x03,0x48,0x0C,0x66,0x89,0x4D,0x8C,0x8B,0x45,
		0xA4,0x83,0x38,0x0A,0x74,0x40,0x83,0x38,0x00,0x79,0x04,0x3B,0xC0,0xEB,
		0x37,0x66,0x8B,0x45,0xE0,0x8B,0x4D,0xD4,0x66,0x89,0x01,0x83,0xC1,0x02,
		0x66,0x8B,0x45,0x8C,0x66,0x89,0x01,0x83,0xC1,0x02,0x89,0x4D,0xD4
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size, pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "disable_named_and_rcdata_resources_mirroring");
		else ShowError(IDS_MULPAT, "disable_named_and_rcdata_resources_mirroring");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "disable_named_and_rcdata_resources_mirroring",
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	set_pattern(found_loc, after, length);
}

void Setup::mod_imte_alloc()
{
	//VA BFF8745C, RVA 1745C, file 15A5C, sec E45C
	static const short pattern[] = {
		0x66,0xff,0x05,-1,-1,-1,0xbf,0x6a,0x3c,0xe8,
	};
	static const short after[] = {
		0x66,0xff,0x05,-1,-1,-1,0xbf,0x6a,0x40,0xe8,
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size, pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "mod_imte_alloc");
		else ShowError(IDS_MULPAT, "mod_imte_alloc");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "mod_imte_alloc",
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	set_pattern(found_loc, after, length);
}

void Setup::mod_mr_alloc()
{
	static const short pattern[] = {
		0x75,0xF6,0x8D,0x04,-1,0x1C,0x00,0x00,0x00,0x50
	};
	static const short after[] = {
		0x75,0xF6,0x8D,0x04,-1,0x24,0x00,0x00,0x00,0x50
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size, pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "mod_mr_alloc");
		else ShowError(IDS_MULPAT, "mod_mr_alloc");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "mod_mr_alloc",
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	set_pattern(found_loc, after, length);
}

void Setup::mod_pdb_alloc()
{
	static const short pattern[] = {
		0x53,0x56,0x57,0x6A,0x06,0x68,0xC4,0x00,0x00,0x00,0xE8
	};
	static const short after[] = {
		0x53,0x56,0x57,0x6A,0x06,0x68,0xC8,0x00,0x00,0x00,0xE8
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size, pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "mod_pdb_alloc");
		else ShowError(IDS_MULPAT, "mod_pdb_alloc");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "mod_pdb_alloc",
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	set_pattern(found_loc, after, length);
}

void Setup::find_ExportFromX()
{
	static const short pattern[] = {
		0x8B,0x0D,-1,-1,-1,-1,0x0F,0xBF,0x40,0x10,0x8B,0x14,0x81,0x8B,0x44,
		0x24,0x14,0x3D,0x00,0x00,0x01,0x00,0x8B,0x4A,0x04,0x73,0x15,-1,0x1D,
		-1,-1,-1,-1,0x75,0x04,0x6A,0x32,0xEB,0x3E,0x50,0x51,0xE8,-2,-2,-2,-2,
		0xEB,0x0C,0xFF,0x74,0x24,0x14,0x6A,0x00,0x51,0xE8,-2,-2,-2,-2,0x83,
		0x7F,0x54,0x00,0x8B,0xF0
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size,pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "exportfromX");
		else ShowError(IDS_MULPAT, "exportfromX");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "exportfromX", 
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	gpa_ExportFromOrdinal_call = found_loc + 0x29;
	gpa_ExportFromName_call = found_loc + 0x37;
	_ExportFromOrdinal = decode_call(gpa_ExportFromOrdinal_call, 5);
	_ExportFromName = decode_call(gpa_ExportFromName_call, 5);
	DWORD start = (DWORD) pefile.RvaToPointer(_ExportFromName);
	for (DWORD a = start ; a < start + 0x100 ; a++)
		if (is_call_ref(a, _ExportFromOrdinal))
			EFN_EFO_call = a;
}

void Setup::find_IsKnownDLL()
{
	static const short pattern[] = {
		0xFF,0x75,0xFC,0x8D,0x8D,0xF0,0xFE,0xFF,0xFF,0x51,0xE8,-2,-2,-2,-2,
		0x85,0xC0,0x75,0x1E,0x8D,0x85,0xE8,0xFD,0xFF,0xFF,
		0x8D,0x8D,0xF0,0xFE,0xFF,0xFF,0x50,0x51
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size,pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "IsKnownDLL");
		else ShowError(IDS_MULPAT, "IsKnownDLL");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "IsKnownDLL", 
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	IsKnownDLL_call = found_loc + 10;
	_IsKnownDLL = decode_call(IsKnownDLL_call, 5);
}

void Setup::find_FLoadTreeNotify1()
{
	static const short pattern[] = {
		0x56,0xA1,-1,-1,-1,-1,0x6A,0x01,0x8B,0x08,0xFF,0xB1,0x98,0x00,0x00,
		0x00,0xE8,-2,-2,-2,-2,0x83,0xF8,0x01,0x1B,0xF6,0xF7,0xDE
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size,pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "FLoadTreeNotify1");
		else ShowError(IDS_MULPAT, "FLoadTreeNotify1");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "FLoadTreeNotify1", 
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	FLoadTreeNotify_call1 = found_loc + 16;
	_FLoadTreeNotify = decode_call(FLoadTreeNotify_call1, 5);
}

void Setup::find_FLoadTreeNotify2()
{
	static const short pattern[] = {
		0x6A,0x00,0x57,0xE8,-1,-1,-1,-1,0x6A,0x00,0x56,0xE8,-2,-2,-2,-2,
		0x85,0xC0,0x74,0x12,0x56,0xE8,-1,-1,-1,-1,0x68,0x5A,0x04,0x00,0x00,
		0x33,0xF6,0xE8,-1,-1,-1,-1
	};

	DWORD offset = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int size = pefile.GetSectionSize(CODE_SEG);
	int length = sizeof(pattern) / sizeof(short);
	DWORD found_loc;
	int found = find_pattern(offset, size,pattern, length, &found_loc);
	if (found != 1)
	{
		if (!found) ShowError(IDS_NOPAT, "FLoadTreeNotify2");
		else ShowError(IDS_MULPAT, "FLoadTreeNotify2");
	}
	DBGPRINTF(("%s: pattern found @ 0x%08x\n", "FLoadTreeNotify2", 
			pefile.PointerToRva((void*) found_loc) + pefile.GetImageBase()));
	FLoadTreeNotify_call2 = found_loc + 11;
}

void Setup::kill_process(const char* name)
{
	PROCESSENTRY32 pe32;
	void * h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if (h == INVALID_HANDLE_VALUE) 
		return;
	
	pe32.dwSize = sizeof(PROCESSENTRY32);
	
	Process32First(h, &pe32);
	do
	{
		if (strstr(pe32.szExeFile, name))
		{
			DBGPRINTF(("Terminating process: %s... ", name));
			HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
			if (hp == INVALID_HANDLE_VALUE)
			{
				DBGPRINTF(("open fail\n"));
				continue;
			}
			if (TerminateProcess(hp, 0))
				DBGPRINTF(("ok\n"));
			else
				DBGPRINTF(("fail\n"));
			CloseHandle(hp);
		}
	}
	while (Process32Next(h, &pe32));

	CloseHandle(h);
}

DWORD Setup::decode_call(DWORD addr, int len)
{
	unsigned char* code = (unsigned char*)addr;

	/* CALL rel32 */
	if (code[0] == 0xe8)
	{
		if (len && len == 5 || !len)
		return (DWORD)(pefile.PointerToRva(code) + 5 + *(INT32*)(code + 1));
	}
	/* CALL imm32 */
	else if (code[0] == 0xff && code[1] == 0x15)
	{
		if (len && len == 6 || !len)
		return *(DWORD*)(code + 2);
	}
	/* unmatched */
	return 0;
}

DWORD Setup::decode_jmp(DWORD addr, int len)
{
	unsigned char* code = (unsigned char*)addr;

	/* JMP rel8 */
	if (code[0] == 0xeb)
	{
		if (len && len == 2 || !len)
		return (DWORD)(pefile.PointerToRva(code) + 2 + *(INT8*)(code + 1));
	}
	/* JMP rel32 */
	else if (code[0] == 0xe9)
	{
		if (len && len == 5 || !len)
		return (DWORD)(pefile.PointerToRva(code) + 5 + *(INT32*)(code + 1));
	}
	/* JMP imm32 */
	else if (code[0] == 0xff && code[1] == 0x25)
	{
		if (len && len == 6 || !len)
		return *(DWORD*)(code + 2);
	}
	/* Jxx rel8 */
	else if (code[0] >= 0x70 && code[0] < 0x7f || code[0] == 0xe3)
	{
		if (len && len == 2 || !len)
		return (DWORD)(pefile.PointerToRva(code) + 2 + *(INT8*)(code + 1));
	}
	/* Jxx rel32 */
	else if (code[0] == 0x0f && code[1] >= 0x80 && code[1] <= 0x8f)
	{
		if (len && len == 6 || !len)
		return (DWORD)(pefile.PointerToRva(code) + 6 + *(INT32*)(code + 2));
	}
	/* unmatched */
	return 0;
}

bool Setup::is_call_ref(DWORD loc, DWORD target)
{
	DWORD addr = decode_call(loc, 5); // size of call imm32 opcode
	if (addr == target)
		return true;
	return false;
}

// Both addresses have to be from the same section!
void Setup::set_call_ref(DWORD loc, DWORD target)
{
	DWORD rel;

	assert(*(BYTE*) loc == 0xe8);

	rel = target - (loc + 5);
	*(DWORD*)(loc + 1) = rel;
}

// Both addresses have to be from the same section!
void Setup::set_jmp_ref(DWORD loc, DWORD target)
{
	DWORD rel;
	unsigned char* code = (unsigned char*)loc;

	if (code[0] == 0xe9)
	{
		rel = target - (loc + 5);
		*(DWORD*)(loc + 1) = rel;
	}
	else if (code[0] == 0x0f && code[1] >= 0x80 && code[1] <= 0x8f)
	{
		rel = target - (loc + 6);
		*(DWORD*)(loc + 2) = rel;
	}
	else assert(false);
}

bool Setup::is_fixupc(DWORD addr)
{
	if (*(DWORD*)(addr - 5) == 0xDAC)
		return true;
	return false;
}

sstring Setup::get_temp_file_name()
{
	char tmpdir[MAX_PATH];
	char target[MAX_PATH];

	GetSystemDirectory(tmpdir, sizeof(tmpdir));
	GetTempFileName(tmpdir, "kex", 0, target);
	
	return target;
}

void Setup::ShowError(UINT id, ...)
{
	char format[512];
	char out[512];
	va_list vargs;
	
	va_start(vargs, id);
	if (!LoadString(GetModuleHandle(NULL), id, format, sizeof(format)))
		sprintf(out, "ERROR: Missing string resource %d", id);
	else
		_vsnprintf(out, sizeof(out), format, vargs);
	va_end(vargs);
	DBGPRINTF(("%s\n", out));
	MessageBox(NULL, out, "KernelEx Setup", MB_OK | MB_ICONERROR);
	exit(id);
}

void Setup::install()
{
	upgrade = false;

	if (version >= 0)
	{
		if (version == KEX_STUB_VER)
		{
			DBGPRINTF(("No need to upgrade\n"));
			return;
		}
		else
		{
			upgrade = true;
		}
	}

	char kernel32path[MAX_PATH];
	GetModuleFileName(h_kernel32, kernel32path, sizeof(kernel32path));
	
	pefile.OpenFile(upgrade ? backup_file : kernel32path, 0x10000);
	if (!pefile.HasTarget())
	{
		if (upgrade)
			ShowError(IDS_FAILOPENBACKUP, (const char*) backup_file, kernel32path);
		else
			ShowError(IDS_FAILOPEN, kernel32path);
	}

	find_ExportFromX();
	find_IsKnownDLL();
	find_FLoadTreeNotify1();
	find_FLoadTreeNotify2();
	prepare_subsystem_check();
	find_resource_check1();
	find_resource_check2();
	disable_named_and_rcdata_resources_mirroring();
	mod_imte_alloc();
	mod_mr_alloc();
	mod_pdb_alloc();

	KernelEx_codeseg* cseg;
	KernelEx_dataseg* dseg;
	if (!pefile.AllocSectionSpace(CODE_SEG, 
			sizeof(KernelEx_codeseg), (void**) &cseg, sizeof(DWORD)))
		ShowError(IDS_FAILSEC, CODE_SEG);
	if (!pefile.AllocSectionSpace(DATA_SEG,
			sizeof(KernelEx_dataseg), (void**) &dseg, sizeof(DWORD)))
		ShowError(IDS_FAILSEC, DATA_SEG);

	memcpy(cseg->signature, "KrnlEx", 6);
	cseg->version = KEX_STUB_VER;
	for (int i = 0 ; i < JTAB_SIZE ; i++)
	{
		cseg->jmp_stub[i].opcode = 0x25ff;
		cseg->jmp_stub[i].addr = (DWORD) 
				pefile.PointerToRva(&dseg->jtab[i]) + pefile.GetImageBase();
		cseg->jmp_stub[i].nop = 0x9090;
	}

	memcpy(dseg->signature, "KrnlEx", 6);
	dseg->version = KEX_STUB_VER;
	dseg->jtab[JTAB_EFO_DYN] = _ExportFromOrdinal + pefile.GetImageBase();
	dseg->jtab[JTAB_EFO_STA] = _ExportFromOrdinal + pefile.GetImageBase();
	dseg->jtab[JTAB_EFN_DYN] = _ExportFromName + pefile.GetImageBase();
	dseg->jtab[JTAB_EFN_STA] = _ExportFromName + pefile.GetImageBase();
	dseg->jtab[JTAB_KNO_DLL] = _IsKnownDLL + pefile.GetImageBase();
	dseg->jtab[JTAB_FLD_TRN] = _FLoadTreeNotify + pefile.GetImageBase();
	dseg->jtab[JTAB_SYS_CHK] = _SubsysCheckPerform + pefile.GetImageBase();
	dseg->jtab[JTAB_RES_CHK] = _GetOrdinal + pefile.GetImageBase();

	//exportfromx patch
	DWORD code = (DWORD) pefile.GetSectionByName(CODE_SEG);
	int code_size = pefile.GetSectionSize(CODE_SEG);

	int efo_cnt = 0;
	int efn_cnt = 0;

	for (DWORD a = code ; a < code + code_size ; a++)
	{
		if (is_call_ref(a, _ExportFromOrdinal))
		{
			DWORD file_loc = a;
			if (a == EFN_EFO_call) 
				continue;
			if (a == gpa_ExportFromOrdinal_call)
				set_call_ref(file_loc, (DWORD) &cseg->jmp_stub[JTAB_EFO_DYN]);
			else
				set_call_ref(file_loc, (DWORD) &cseg->jmp_stub[JTAB_EFO_STA]);
			DBGPRINTF(("%s: address 0x%08x\n", "EFO", pefile.PointerToRva((void*) a) 
					+ pefile.GetImageBase()));
			efo_cnt++;
		}
		else if (is_call_ref(a, _ExportFromName))
		{
			DWORD file_loc = a;
			if (is_fixupc(a))
				continue;
			if (a == gpa_ExportFromName_call)
				set_call_ref(file_loc, (DWORD) &cseg->jmp_stub[JTAB_EFN_DYN]);
			else
				set_call_ref(file_loc, (DWORD) &cseg->jmp_stub[JTAB_EFN_STA]);
			DBGPRINTF(("%s: address 0x%08x\n", "EFN", pefile.PointerToRva((void*) a) 
					+ pefile.GetImageBase()));
			efn_cnt++;
		}
	}

	if (efo_cnt != 2 || efn_cnt != 2)
		ShowError(IDS_ERRCHECK);

	//isknowndll patch
	set_call_ref(IsKnownDLL_call, (DWORD) &cseg->jmp_stub[JTAB_KNO_DLL]);
	DBGPRINTF(("%s: address 0x%08x\n", "KNO_DLL", pefile.PointerToRva((void*) IsKnownDLL_call) 
			+ pefile.GetImageBase()));

	//FLoadTreeNotify patch
	set_call_ref(FLoadTreeNotify_call1, (DWORD) &cseg->jmp_stub[JTAB_FLD_TRN]);
	DBGPRINTF(("%s: address 0x%08x\n", "FLD_TRN1", pefile.PointerToRva((void*) FLoadTreeNotify_call1)
			+ pefile.GetImageBase()));
	set_call_ref(FLoadTreeNotify_call2, (DWORD) &cseg->jmp_stub[JTAB_FLD_TRN]);
	DBGPRINTF(("%s: address 0x%08x\n", "FLD_TRN2", pefile.PointerToRva((void*) FLoadTreeNotify_call2)
			+ pefile.GetImageBase()));

	//subsys check patch
	set_jmp_ref(SubsysCheck_jmp, (DWORD) &cseg->jmp_stub[JTAB_SYS_CHK]);
	DBGPRINTF(("%s: address 0x%08x\n", "SYS_CHK", pefile.PointerToRva((void*) SubsysCheck_jmp)
			+ pefile.GetImageBase()));

	//resource check patch
	set_call_ref(GetOrdinal_call1, (DWORD) &cseg->jmp_stub[JTAB_RES_CHK]);
	DBGPRINTF(("%s: address 0x%08x\n", "RES_CHK1", pefile.PointerToRva((void*) GetOrdinal_call1)
			+ pefile.GetImageBase()));
	set_call_ref(GetOrdinal_call2, (DWORD) &cseg->jmp_stub[JTAB_RES_CHK]);
	DBGPRINTF(("%s: address 0x%08x\n", "RES_CHK2", pefile.PointerToRva((void*) GetOrdinal_call2)
			+ pefile.GetImageBase()));

	// backup original file
	if (!upgrade)
	{
		if (!CopyFile(kernel32path, backup_file, FALSE))
			ShowError(IDS_FAILBAK, (const char*) backup_file);
	}

	// save patched file
	sstring tmp_file = get_temp_file_name();
	pefile.SaveFile(tmp_file);

	if (is_winme)
	{
		kill_process("\\STATEMGR.EXE");
		kill_process("\\STMGR.EXE");
	}

	Wininit.add(kernel32path, tmp_file);

	if (is_winme)
	{
		char path[MAX_PATH];
		GetWindowsDirectory(path, sizeof(path));
		if (path[strlen(path) - 1] != '\\')
			strcat(path, "\\");
		strcat(path, "WININIT.LOG");
		Wininit.add("NUL", path);
		path[3] = '\0';
		strcat(path, "_RESTORE\\TEMP");
		Wininit.add("DIRNUL", path);
	}

	Wininit.save();
}

int main(int argc, char** argv)
{
	DBGPRINTF(("KernelEx setup program running\n"));
	if (argc != 2)
		return 1;

	Setup setup(argv[1]);
	setup.install();
	DBGPRINTF(("Setup finished\n"));
	return 0;
}
