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
#include <algorithm>
#include "debug.h"
#include "resolver.h"
#include "apiconf.h"
#include "apiconfmgr.h"
#include "internals.h"
#include "../setup/loadstub.h"
#include "thunks.h"
#include "SettingsDB.h"

using namespace std;

char system_path[MAX_PATH];
int system_path_len;

static PLONG jtab;
static LONG old_jtab[4];


/** Get API configuration for selected module.
 * @param module Target module.
 * @return Pointer to API configuration or NULL to not use extended API.
 */
static ApiConfiguration* get_config(MODREF* moduleMR)
{
	IMTE** pmteModTable = *ppmteModTable;
	PDB98* ppdbCur = *pppdbCur;
	IMTE_KEX* module = (IMTE_KEX*) pmteModTable[moduleMR->mteIndex];
	IMTE_KEX* process = (IMTE_KEX*) pmteModTable[ppdbCur->pExeMODREF->mteIndex];
	ApiConfiguration* conf;
	BYTE flags;

	//unless override flag is set try to get module configuration first
	if (!(process->flags & LDR_OVERRIDE_PROC_MOD))
	{
		if (!(module->flags & LDR_VALID_FLAG))
		{
			appsetting as = SettingsDB::instance.get_appsetting(module->pszFileName);
			module->config = as.conf;
			module->flags = as.flags | LDR_VALID_FLAG;
		}
		conf = module->config;
		flags = module->flags;

		if (flags & LDR_KEX_DISABLE)
			return NULL;
	}
	else
		conf = NULL;

	//if above failed or override flag was set try to get process configuration
	if (!conf)
	{
		if (!(process->flags & LDR_VALID_FLAG))
		{
			appsetting as = SettingsDB::instance.get_appsetting(process->pszFileName);
			process->config = as.conf;
			process->flags = as.flags | LDR_VALID_FLAG;
		}
		conf = process->config;
		flags = process->flags;

		if (flags & LDR_KEX_DISABLE)
			return NULL;
	}

	//if no process configuration then get parent configuration
	if (!conf)
	{
		PDB98* ppdbParent = ppdbCur->ParentPDB;

		if (ppdbParent && !(ppdbParent->Flags & (fTerminated | fTerminating | 
                  fNearlyTerminating | fDosProcess | fWin16Process)))
		{
			IMTE_KEX* parent = (IMTE_KEX*) pmteModTable[ppdbParent->pExeMODREF->mteIndex];
			conf = parent->config;
			flags = parent->flags;

			if (flags & LDR_KEX_DISABLE)
				return NULL;
		}
	}

	if (flags & LDR_LOG_APIS)
	{
		//TODO: not implemented yet
		DBGPRINTF(("Resolver flag LDR_LOG_APIS not implemented\n"));
	}

	if (flags & LDR_FILTER_APIS)
	{
		//TODO: not implemented yet
		DBGPRINTF(("Resolver flag LDR_FILTER_APIS not implemented\n"));
	}

	//finally if everything else fails take default configuration
	if (!conf)
		conf = ApiConfigurationManager::get_default_configuration();

	return conf;
}

/** Finds overridden module index for target module.
 * @param target Module from which functions are imported.
 * @return Index or 0xffff if module doesn't have entry in API configuration tables.
 */
static WORD resolve_mod_index(IMTE_KEX* target)
{
	const char* file_name;

	/* Skip non-system modules */
	if (target->cbFileName - target->cbModName - 1 != system_path_len
			|| strncmp(system_path, target->pszFileName, system_path_len))
		return target->mod_index = 0xffff;

	file_name = target->pszModName;

	for (int i = 0 ; i < overridden_module_count ; i++)
		if (strcmp(file_name, overridden_module_names[i]) == 0)
			return target->mod_index = i + 1;

	/* No override API list for target module available. */
	return target->mod_index = 0xffff;
}

static PROC resolve_nonshared_addr(DWORD addr, MODREF* caller, PMODREF** refmod)
{
	MODREF* mr;
	WORD idx;
	DWORD img_base;
	int api_lib_num;
	char dllpath[MAX_PATH];
	ApiLibrary* apilib;
	IMTE** pmteModTable = *ppmteModTable;
	static MODREF* buffer[1024];

	DBGASSERT(addr >= 0xc0000000);
	api_lib_num = (addr >> 24) - 0xc0;
	DBGASSERT(api_lib_num > 0); //ensure apilib ID isn't STD's
	apilib = ApiLibraryManager::get_apilib(api_lib_num);
	DBGASSERT(apilib != NULL);
	DBGASSERT((DWORD) apilib->mod_handle < 0x80000000);
	idx = 0xff00 + api_lib_num;

	//first check if api library has already been loaded
	mr = (*pppdbCur)->MODREFList;
	do 
	{
		IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[mr->mteIndex];
		if (imte->mod_index == idx)
		{
			IMAGE_NT_HEADERS* nthdr = imte->pNTHdr;
			img_base = nthdr->OptionalHeader.ImageBase;
			return (PROC)(img_base + (addr & 0x00ffffff));
		}
		mr = mr->pNextModRef;
	} 
	while (mr);

	//if not - load it

	strcpy(dllpath, kernelex_dir.get());
	strcat(dllpath, apilib->apilib_name);

	_EnterSysLevel(krnl32lock);
	mr = MRLoadTree(dllpath);
	if (!mr)
	{
		FreeLibRemove();
		_LeaveSysLevel(krnl32lock);
		return 0;
	}
	_LeaveSysLevel(krnl32lock);

	if (refmod) //static resolve (implicit)
	{
		for (int i = 0 ; i < caller->cImportedModules ; i++)
			buffer[i] = caller->ImplicitImports[i].pMR;

		//FIXME: this will cause problems if apilib references another non-shared apilib!!
		//it is okay to use global buffer because static resolve code is protected by k32 lock
		**refmod = mr;
		*refmod += buffer - &caller->ImplicitImports[0].pMR;
	}
	else //dynamic resolve (GetProcAddress)
	{
		if (FLoadTreeNotify(mr, 0))
		{
			FreeLibTree(mr);
			return 0;
		}
	}

	IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[mr->mteIndex];

	img_base = imte->pNTHdr->OptionalHeader.ImageBase;

	if (img_base == (DWORD) apilib->mod_handle)
		DBGASSERT(imte->mod_index == apilib->index + 0xff00);

	imte->mod_index = idx;

	return (PROC)(img_base + (addr & 0x00ffffff));
}

static PROC WINAPI OriExportFromOrdinal(IMAGE_NT_HEADERS* PEh, WORD ordinal)
{
	DWORD img_base;
	IMAGE_EXPORT_DIRECTORY* Exports;
	DWORD export_table_size;
	DWORD export_no;
	DWORD export_cnt;
	DWORD* function_table;
	DWORD addr;

	export_table_size = PEh->OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	if (!export_table_size)
		return NULL;
	
	img_base = PEh->OptionalHeader.ImageBase;
	Exports = (IMAGE_EXPORT_DIRECTORY *)(img_base + PEh->OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	export_no = ordinal - Exports->Base;
	export_cnt = Exports->NumberOfFunctions;

	if (!export_cnt || export_no > export_cnt)
		return NULL;

	function_table = (DWORD*)(Exports->AddressOfFunctions + img_base);
	addr = function_table[export_no];

	if (!addr)
		return NULL;

	addr += img_base;

	//handle export forwarding case
	if (addr >= (DWORD)Exports && addr < (DWORD)Exports + export_table_size)
	{
		char module_name[MAX_PATH];
		char* p;
		char c;
		HMODULE hmod;

		p = module_name;
		do
		{
			c = *(char*) addr;
			addr++;
			if (c == '.') break;
			*p = c;
			p++;
		} 
		while (c);
		*p = '\0';

		hmod = GetModuleHandle(module_name); //should be IGetModuleHandle
		if (!hmod) 
			return NULL;

		return GetProcAddress(hmod, (char*) addr); //should be IGetProcAddress
	}

	return (PROC) addr;
}

static PROC WINAPI OriExportFromName(IMAGE_NT_HEADERS* PEh, WORD hint, LPCSTR name)
{
	DWORD img_base;
	IMAGE_EXPORT_DIRECTORY* Exports;
	DWORD* names;
	WORD* ordinals;
	int lo;
	int hi;
	int curr;
	int res;

	if (!PEh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size)
		return NULL;

	img_base = PEh->OptionalHeader.ImageBase;
	Exports = (IMAGE_EXPORT_DIRECTORY *)(img_base + PEh->OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

	hi = Exports->NumberOfNames;

	if (!hi) 
		return NULL;

	ordinals = (WORD*)(Exports->AddressOfNameOrdinals + img_base);
	names = (DWORD*)(img_base + Exports->AddressOfNames);
	
	if (hint < hi)
	{
		res = strcmp(name, (char*)(names[hint] + img_base));
		if (!res)
		{
			return OriExportFromOrdinal(PEh, (WORD)(Exports->Base + ordinals[hint]));
		}
		else if (res > 0) 
		{
			lo = hint + 1;
			hi--;
		}
		else
		{
			lo = 0;
			hi = hint - 1;
		}
		curr = (hi + lo) >> 1;
	}
	else 
	{
		lo = 0;
		hi--;
		curr = hi >> 1;
	}	

	while (lo <= hi)
	{
		res = strcmp(name, (char*)(names[curr] + img_base));
		if (!res)
		{
			return OriExportFromOrdinal(PEh, (WORD)(Exports->Base + ordinals[curr]));
		}
		else if (res > 0)
		{
			lo = curr + 1;
		}
		else
		{
			hi = curr - 1;
		}
		curr = (hi + lo) >> 1;
	}
	return 0;
}

DWORD encode_address(DWORD addr, const ApiLibrary* apilib)
{
	//note: rules have to be the same as in decode_address
	int index = apilib->index;

	//ensure that we haven't run out of indexes
	DBGASSERT(index + 0xc0 < 0xff);

	//STD apilib
	if (index == 0)
		return addr;

	//non-shared apilib
	if ((DWORD) apilib->mod_handle < 0x80000000)
	{
		//max non-shared apilib size 16MB
		DBGASSERT(addr - (DWORD) apilib->mod_handle < 0x01000000);

		return ((0xc0 + index) << 24) + addr - (DWORD) apilib->mod_handle;
	}
	//shared apilib
	return addr;
}

inline PROC decode_address(DWORD p, IMAGE_NT_HEADERS* target_NThdr, MODREF* caller, PMODREF** refmod)
{
	//note: rules have to be the same as in encode_address
	//zero address
	if (!p)
		return (PROC) p;
	//export forwarding - ordinal number
	if ((p & 0xffff0000) == 0xffff0000)
		return OriExportFromOrdinal(target_NThdr, LOWORD(p));
	//non-shared system library
	if (p < 0x80000000)
		return (PROC)(p + target_NThdr->OptionalHeader.ImageBase);
	//non-shared api library
	if (p >= 0xc0000000)
		return resolve_nonshared_addr(p, caller, refmod);
	//shared system or api library
	return (PROC) p;
}

PROC WINAPI ExportFromOrdinal(IMTE_KEX* target, MODREF* caller, PMODREF** refmod, WORD ordinal)
{
	PROC ret;
	
	//if caller is unknown - assume it is process's exe
	if (!caller)
		caller = (*pppdbCur)->pExeMODREF;
	
	ApiConfiguration* apiconf = get_config(caller);
	if (apiconf)
	{
		WORD mod_index = target->mod_index;

		if (!mod_index)
			mod_index = resolve_mod_index(target);

		DBGASSERT(mod_index);
		mod_index--;

		if (!apiconf->is_table_empty(mod_index))
			ret = decode_address(apiconf->get(mod_index, ordinal), 
					target->pNTHdr, caller, refmod);
		else 
			ret = OriExportFromOrdinal(target->pNTHdr, ordinal);
	}
	else 
		ret = OriExportFromOrdinal(target->pNTHdr, ordinal);

	if (!ret && refmod)
		DBGPRINTF(("%s: unresolved export %s:%d\n", 
				((*ppmteModTable)[caller->mteIndex])->pszModName,
				target->pszModName, ordinal));

	return ret;
}

PROC WINAPI ExportFromName(IMTE_KEX* target, MODREF* caller, PMODREF** refmod, WORD hint, LPCSTR name)
{
	PROC ret;

	//if caller is unknown - assume it is process's exe
	if (!caller)
		caller = (*pppdbCur)->pExeMODREF;

	ApiConfiguration* apiconf = get_config(caller);
	if (apiconf)
	{
		WORD mod_index = target->mod_index;

		if (!mod_index)
			mod_index = resolve_mod_index(target);

		DBGASSERT(mod_index);
		mod_index--;

		if (!apiconf->is_table_empty(mod_index))
			ret = decode_address(apiconf->get(mod_index, hint, name), 
					target->pNTHdr, caller, refmod);
		else 
			ret = OriExportFromName(target->pNTHdr, hint, name);
	}
	else 
		ret = OriExportFromName(target->pNTHdr, hint, name);

	if (!ret && refmod)
		DBGPRINTF(("%s: unresolved export %s:%s\n", 
				((*ppmteModTable)[caller->mteIndex])->pszModName,
				target->pszModName, name));

	return ret;
}

#if 0

//todo: vGetProcAddress_new
PROC WINAPI GetProcAddress_new(HMODULE hModule, LPCSTR lpProcName)
{
	DWORD caller = *((DWORD*)&hModule - 1);
	PDB98* ppdbCur = *pppdbCur;
	MODREF* mod_mr;
	IMAGE_NT_HEADERS* peh;
	PROC ret;

	_EnterSysLevel(&ppdbCur->CriticalSection);
	mod_mr = MRFromHLib(hModule);
	if (!mod_mr)
	{
		_SetError(ERROR_INVALID_HANDLE);
		ret = NULL;
	}
	else
	{
		IMTE_KEX* target = (IMTE_KEX*) pmteModTable[mod_mr->mteIndex];

		if (HIWORD(lpProcName))
		{
			ret = ExportFromName(target, (IMTE_KEX*) IMTEfromCallerAddr(caller), 0, lpProcName);
		}
		else
		{
			ret = ExportFromOrdinal(target, (IMTE_KEX*) IMTEfromCallerAddr(caller), (WORD) lpProcName);
		}
		if (!ppdbCur->DebuggeeCB || _CheckMustComplete())
		{
			if (!ret)
				_SetError(ERROR_PROC_NOT_FOUND);
		}
		else
		{
			ret = _DEBCreateDIT(*((DWORD*)ppdbCur->DebuggeeCB + 5), ret);
		}

	}
	_LeaveSysLevel(&ppdbCur->CriticalSection);
	return ret;
}
#endif

PROC WINAPI iGetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
	IMAGE_DOS_HEADER* dos_hdr;
	IMAGE_NT_HEADERS* nt_hdr;

	dos_hdr = (IMAGE_DOS_HEADER*) hModule;
	nt_hdr = (IMAGE_NT_HEADERS*)((int)dos_hdr + dos_hdr->e_lfanew);

	if ((DWORD)lpProcName < 0x10000) 
		return OriExportFromOrdinal(nt_hdr, (WORD)lpProcName);
	return OriExportFromName(nt_hdr, 0, lpProcName);
}

static IMAGE_SECTION_HEADER* get_data_section()
{
	int i;
	IMAGE_DOS_HEADER* MZh = (IMAGE_DOS_HEADER*) GetModuleHandle("kernel32");
	IMAGE_NT_HEADERS* PEh = (IMAGE_NT_HEADERS*) ((int)MZh + MZh->e_lfanew);
	IMAGE_SECTION_HEADER* section = (IMAGE_SECTION_HEADER*) 
		((int)PEh 
		+ PEh->FileHeader.SizeOfOptionalHeader 
		+ sizeof(IMAGE_FILE_HEADER) 
		+ sizeof(DWORD));
	for (i = 0 ; i < PEh->FileHeader.NumberOfSections ; i++)
	{
		if (!strcmpi((char*) section->Name, ".data"))
			return section;
		section++;
	}
	return NULL;
}

static void reset_imtes()
{
	DBGPRINTF(("Reseting IMTEs\n"));
	_EnterSysLevel(krnl32lock);

	WORD imteMax = *pimteMax;
	IMTE** pmteModTable = *ppmteModTable;
	for (WORD i = 0 ; i < imteMax ; i++)
	{
		IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[i];
		if (imte)
		{
			imte->config = NULL;
			imte->flags = 0;
			imte->mod_index = 0;
		}
	}

	_LeaveSysLevel(krnl32lock);
}

#ifdef _DEBUG

void dump_imtes(void)
{
	WORD imteMax = *pimteMax;
	IMTE** pmteModTable = *ppmteModTable;
	int total = 0;
	
	dbgprintf("Dumping IMTEs...\n");
	dbgprintf("%-6s %-12s %s %s %s\n", "No.", "Process", "Module", "Config", "Flags");
	for (WORD i = 0 ; i < imteMax ; i++)
	{
		IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[i];
		if (imte)
		{
			dbgprintf("#%-5d %-12s %s %s %02x\n", i,
					pmteModTable[imte->pMR->ppdb->pExeMODREF->mteIndex]->pszSModName, 
					imte->pszFileName, 
					imte->config ? imte->config->get_name() : "unknown", 
					imte->flags);
			total++;
		}
	}
	dbgprintf("\nEnd dump total %d IMTEs\n\n", total);
}

#endif

int resolver_init()
{
	DBGPRINTF(("resolver_init()\n"));
	DWORD ptr;
	KernelEx_dataseg* dseg = NULL;
	IMAGE_SECTION_HEADER* section = get_data_section();
	DWORD sign_len = sizeof(KEX_SIGNATURE) - 1;

	if (!section)
		return 0;
	
	ptr = (DWORD) h_kernel32 + section->VirtualAddress + section->Misc.VirtualSize - sign_len;
	while (ptr >= (DWORD) h_kernel32 + section->VirtualAddress)
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
		ShowError(IDS_NOTREADY);
		return 0;
	}
	else
		DBGPRINTF(("Signature found @ 0x%08x\n", ptr));

	if (dseg->version != KEX_STUB_VER)
	{
		DBGPRINTF(("Wrong stub version, expected: %d, got: %d\n", 
				KEX_STUB_VER, dseg->version));
		ShowError(IDS_STUBMISMATCH, KEX_STUB_VER, dseg->version);
		return 0;
	}

	jtab = (PLONG) dseg->jtab;

	system_path_len = GetSystemDirectory(system_path, sizeof(system_path));

	SettingsDB::instance.flush_all();

	return 1;
}

void resolver_uninit()
{
	SettingsDB::instance.clear();
	reset_imtes();
}

void resolver_hook()
{
	DBGPRINTF(("resolver_hook()\n"));
	old_jtab[0] = InterlockedExchange(jtab + JTAB_EFO_DYN, (LONG) ExportFromOrdinalDynamic_thunk);
	old_jtab[1] = InterlockedExchange(jtab + JTAB_EFO_STA, (LONG) ExportFromOrdinalStatic_thunk);
	old_jtab[2] = InterlockedExchange(jtab + JTAB_EFN_DYN, (LONG) ExportFromNameDynamic_thunk);
	old_jtab[3] = InterlockedExchange(jtab + JTAB_EFN_STA, (LONG) ExportFromNameStatic_thunk);
}

void resolver_unhook()
{
	DBGPRINTF(("resolver_unhook()\n"));
	InterlockedExchange(jtab + JTAB_EFO_DYN, old_jtab[0]);
	InterlockedExchange(jtab + JTAB_EFO_STA, old_jtab[1]);
	InterlockedExchange(jtab + JTAB_EFN_DYN, old_jtab[2]);
	InterlockedExchange(jtab + JTAB_EFN_STA, old_jtab[3]);
}
