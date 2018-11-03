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
#include "apilib.h"
#include <algorithm>
#include "resolver.h"
#include "internals.h"
#include "debug.h"
#include "is_sorted.hpp"
#include "pemanip.h"

using namespace std;

#define ALLOC_CAPACITY 10

ApiLibrary** ApiLibraryManager::apilib_ptrs = NULL;
int ApiLibraryManager::apilib_cnt = 0;

int          overridden_module_count;
const char** overridden_module_names;
const char** new_overridden_mod_nms;
int          new_overridden_mod_cnt;

ApiLibraryManager::ApiLibraryManager()
{
	new_apilib_ptrs = NULL;
	new_apilib_cnt = 0;
	new_overridden_mod_nms = NULL;
	new_overridden_mod_cnt = 0;
	initialized = false;
}

ApiLibraryManager::~ApiLibraryManager()
{
	rollback_changes();
}

bool ApiLibraryManager::initialize()
{
	ApiLibrary* std_apilib;

	new_apilib_cnt = 0;
	new_apilib_ptrs = (ApiLibrary**) malloc(ALLOC_CAPACITY * sizeof(ApiLibrary*));

	if (!new_apilib_ptrs)
		goto __error;

	std_apilib = (ApiLibrary*) malloc(sizeof(ApiLibrary) + strlen("STD"));
		
	if (!std_apilib)
		goto __error;

	strcpy(std_apilib->apilib_name, "STD");
	std_apilib->mod_handle = NULL;
	std_apilib->api_tables = NULL;
	std_apilib->index = 0;

	//copy previous STD api library tables and overridden module names
	if (overridden_module_count)
	{
		int size = (overridden_module_count / ALLOC_CAPACITY + 1) * ALLOC_CAPACITY;
		
		std_apilib->api_tables = (apilib_api_table*) 
				malloc((size + 1) * sizeof(apilib_api_table));

		if (!std_apilib->api_tables)
			goto __error;

		apilib_api_table* prev_std_tab = apilib_ptrs[0]->api_tables;
		copy(prev_std_tab, prev_std_tab + size + 1, std_apilib->api_tables);

		new_overridden_mod_nms = (const char**) malloc(size * sizeof(char*));

		if (!new_overridden_mod_nms)
			goto __error;

		copy(overridden_module_names, overridden_module_names + size,
				new_overridden_mod_nms);

		new_overridden_mod_cnt = overridden_module_count;
	}

	new_apilib_ptrs[new_apilib_cnt++] = std_apilib;

	initialized = true;
	return true;

__error:
	free(new_overridden_mod_nms);
	if (std_apilib)
		free(std_apilib->api_tables);
	free(std_apilib);
	free(new_apilib_ptrs);
	return false;
}

bool ApiLibraryManager::load_apilib(const char* apilib_name)
{
	if (!initialized && !initialize())
	{
		DBGPRINTF(("Failed to initialize api library manager\n"));
		return false;
	}

	if (new_apilib_cnt >= 0xff)
	{
		DBGPRINTF(("Too many api libraries loaded\n"));
		return false;
	}

	//STD is a special api library name
	if (!strcmp(apilib_name, "STD"))
		return true;
	
	//check if library wasn't loaded in this instance
	for (int i = 0 ; i < new_apilib_cnt ; i++)
		if (!strcmp(new_apilib_ptrs[i]->apilib_name, apilib_name))
			return true;

	ApiLibrary* apilib;
	bool already_loaded = false;

	DBGPRINTF(("Loading api library: %s... ", apilib_name));

	//check if library wasn't previously loaded
	for (int i = 0 ; i < apilib_cnt ; i++)
	{
		if (!strcmp(apilib_ptrs[i]->apilib_name, apilib_name))
		{
			DBGPRINTF(("already loaded... "));
			apilib = apilib_ptrs[i];
			already_loaded = true;
		}
	}

	//if it wasn't loaded
	if (!already_loaded)
	{
		char dllpath[MAX_PATH];
		int size = sizeof(ApiLibrary) + strlen(apilib_name);

		apilib = (ApiLibrary*) malloc(size);

		if (!apilib)
		{
			DBGPRINTF(("Failed to allocate memory\n"));
			goto __error;
		}

		strcpy(apilib->apilib_name, apilib_name);

		strcpy(dllpath, kernelex_dir.get());
		strcat(dllpath, apilib_name);
		apilib->mod_handle = LoadLibrary(dllpath);

		if (!apilib->mod_handle)
		{
			DBGPRINTF(("Failed to load api library\n"));
			goto __error;
		}

		fgat_t get_api_table;
		get_api_table = (fgat_t) GetProcAddress(
				apilib->mod_handle, "get_api_table");

		if (!get_api_table)
		{
			DBGPRINTF(("Failed to get api library entry point\n"));
			goto __error;
		}

		const apilib_api_table* file_api_tables;
		file_api_tables = get_api_table();

		if (!file_api_tables)
		{
			DBGPRINTF(("Failed to get api tables\n"));
			goto __error;
		}

		apilib->api_tables = make_shared_api_tables(file_api_tables);

		if (!apilib->api_tables)
		{
			DBGPRINTF(("Failed to create shared api tables\n"));
			goto __error;
		}

		DBGPRINTF(("loaded @ 0x%08x... ", (DWORD) apilib->mod_handle));
	}

	//allocate space for new ApiLibraries
	if (new_apilib_cnt % ALLOC_CAPACITY == 0)
	{
		void* new_block = realloc(new_apilib_ptrs,
				(new_apilib_cnt + ALLOC_CAPACITY) * sizeof(ApiLibrary*));

		if (!new_block)
		{
			DBGPRINTF(("Failed to allocate memory\n"));
			goto __error;
		}

		new_apilib_ptrs = (ApiLibrary**) new_block;
	}

//	DBGPRINTF(("Listing modules overridden by api library:\n"));
	for (apilib_api_table* p = apilib->api_tables ; p->target_library ; p++)
	{
//		DBGPRINTF(("  * %s\n", p->target_library));
		add_overridden_module(p->target_library);
	}

	//set or update index value which is used by encode_address() 
	//and to update mod_index in commit_changes()
	apilib->index = new_apilib_cnt;

	//add to table of new ApiLibraries
	new_apilib_ptrs[new_apilib_cnt++] = apilib;

	DBGPRINTF(("ok\n"));
	return true;

__error:
	if (!already_loaded && apilib)
	{
		if (apilib->mod_handle)
			FreeLibrary(apilib->mod_handle);
		if (apilib->api_tables)
			free(apilib->api_tables);
		free(apilib);
	}
	return false;
}

ApiLibrary* ApiLibraryManager::get_new_apilib(const char* apilib_name)
{
	for (int i = 0 ; i < new_apilib_cnt ; i++)
		if (!strcmp(new_apilib_ptrs[i]->apilib_name, apilib_name))
			return new_apilib_ptrs[i];
	DBGPRINTF(("Api library %s not found\n", apilib_name));
	return NULL;
}

ApiLibrary* ApiLibraryManager::get_apilib(int index)
{
	if (index < 0 || index >= apilib_cnt)
		return NULL;

	DBGASSERT(apilib_ptrs[index]);
	DBGASSERT(apilib_ptrs[index]->index == index);
	return apilib_ptrs[index];
}

bool ApiLibraryManager::are_api_tables_sorted(const apilib_api_table* tab)
{
	for (const apilib_api_table* p = tab ; p->target_library ; p++)
	{
		if (!::is_sorted(p->named_apis, p->named_apis + p->named_apis_count)) 
			return false;

		if (!::is_sorted(p->ordinal_apis, p->ordinal_apis + p->ordinal_apis_count)) 
			return false;
	}
	return true;
}

apilib_api_table* ApiLibraryManager::make_shared_api_tables(const apilib_api_table* in)
{
	bool copy_strings = (size_t) in < 0x80000000 ? true : false;
	bool sorted = are_api_tables_sorted(in);
	
	if (!sorted)
	{
		DBGPRINTF(("Error: api tables are not sorted\n"));
		return NULL;
	}
	
	if (!copy_strings && sorted)
	{
		//nothing bad will happen if I call HeapFree 
		//on non-HeapAlloc'ed mem later right?
		return const_cast<apilib_api_table*>(in);
	}

	int size = required_api_table_space(in, copy_strings);
	apilib_api_table* out = (apilib_api_table*) malloc(size);

	if (!out) 
		return NULL;

	int i;
	char* pstr = (char*) out + size;

	//copy apilib_api_tables
	const apilib_api_table* pin;
	apilib_api_table* pout;

	for (pin = in, pout = out ; pin->target_library ; pin++, pout++)
	{
		if (copy_strings)
		{
			pstr -= strlen(pin->target_library) + 1;
			strcpy(pstr, pin->target_library);
			pout->target_library = pstr;
		}
		else
		{
			pout->target_library = pin->target_library;
		}
	}
	*pout++ = *pin++; //terminating NULL entry

	//copy apilib_named_apis
	const apilib_named_api* pina;
	apilib_named_api* pona;
	pona = (apilib_named_api*) pout;

	for (pin = in, pout = out ; pin->target_library ; pin++, pout++)
	{
		pout->named_apis = pona;
		pout->named_apis_count = pin->named_apis_count;

		for (i = 0, pina = pin->named_apis ; i < pin->named_apis_count ; i++, pina++, pona++)
		{
			if (copy_strings)
			{
				pstr -= strlen(pina->name) + 1;
				strcpy(pstr, pina->name);
				pona->name = pstr;
			}
			else
			{
				pona->name = pina->name;
			}
			pona->addr = pina->addr;
		}
	}

	//copy apilib_unnamed_apis
	const apilib_unnamed_api* piua;
	apilib_unnamed_api* poua;
	poua = (apilib_unnamed_api*) pona;

	for (pin = in, pout = out ; pin->target_library ; pin++, pout++)
	{
		pout->ordinal_apis = poua;
		pout->ordinal_apis_count = pin->ordinal_apis_count;

		for (i = 0, piua = pin->ordinal_apis ; i < pin->ordinal_apis_count ; i++, piua++, poua++)
		{
			*poua = *piua;
		}
	}

//	//sort the tables
//	if (!sorted)
//	{
//		while (pout-- != out)
//		{
//			stable_sort(pout->named_apis, pout->named_apis + pout->named_apis_count);
//			stable_sort(pout->ordinal_apis, pout->ordinal_apis + pout->ordinal_apis_count);
//		}
//	}

	//finish line
	return out;
}

int ApiLibraryManager::required_api_table_space(const apilib_api_table* tab, bool copy_strings)
{
	int space = 0;

	for (const apilib_api_table* p = tab ; p->target_library ; p++)
	{
		space += sizeof(apilib_api_table);
		space += p->named_apis_count * sizeof(apilib_named_api);
		space += p->ordinal_apis_count * sizeof(apilib_unnamed_api);

		if (copy_strings)
		{
			space += strlen(p->target_library) + 1;
			for (int i = 0 ; i < p->named_apis_count ; i++)
				space += strlen(p->named_apis[i].name) + 1;
		}
	}
	space += sizeof(apilib_api_table);
	
	return space;
}

bool ApiLibraryManager::add_overridden_module(const char* mod)
{
	apilib_api_table*& std_api_table =  new_apilib_ptrs[0]->api_tables;

	//ensure that module isn't already on list
	for (int i = 0 ; i < new_overridden_mod_cnt ; i++)
	{
		if (!strcmpi(new_overridden_mod_nms[i], mod))
			return true;
	}

	//allocate space for new overridden modules
	if (new_overridden_mod_cnt % ALLOC_CAPACITY == 0)
	{
		void* new_block = realloc(new_overridden_mod_nms,
				(new_overridden_mod_cnt + ALLOC_CAPACITY) * sizeof(char*));
		
		if (!new_block)
		{
			DBGPRINTF(("Failed to allocate memory\n"));
			return false;
		}

		new_overridden_mod_nms = (const char**) new_block;

		new_block = realloc(std_api_table,
				(new_overridden_mod_cnt + 1 + ALLOC_CAPACITY) * sizeof(apilib_api_table));
			//+ 1 because api_tables are NULL terminated

		if (!new_block)
		{
			DBGPRINTF(("Failed to allocate memory\n"));
			return false;
		}

		std_api_table = (apilib_api_table*) new_block;
	}

	if (!parse_system_dll(mod, &std_api_table[new_overridden_mod_cnt]))
		return false;

	//add to table of overridden modules
	new_overridden_mod_nms[new_overridden_mod_cnt] 
			= std_api_table[new_overridden_mod_cnt].target_library;
	new_overridden_mod_cnt++;

	memset(&std_api_table[new_overridden_mod_cnt], 0, sizeof(apilib_api_table));

	return true;
}

bool ApiLibraryManager::parse_system_dll(const char* dll_name, apilib_api_table* api_tab)
{
	PEmanip pemod;
	HMODULE mem_dll = GetModuleHandle(dll_name);
	
	if (mem_dll)
	{
		if (!pemod.OpenMemory(mem_dll))
			return false;
	}
	else
	{
		char path[MAX_PATH];
		GetSystemDirectory(path, sizeof(path));
		strcat(path, "\\");
		strcat(path, dll_name);

		if (!pemod.OpenFile(path))
			return false;
	}

	IMAGE_NT_HEADERS* PEh = pemod.GetPEHeader();
	DWORD ExportsRva = PEh->OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress; 

	IMAGE_EXPORT_DIRECTORY* Exports = (IMAGE_EXPORT_DIRECTORY*) 
			pemod.RvaToPointer(ExportsRva);

	DWORD ExportsSize = PEh->OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;

	if (!Exports || !ExportsSize)
		return false;

	DWORD* Names = (DWORD*) pemod.RvaToPointer(Exports->AddressOfNames);
	WORD* OrdinalTable = (WORD*) pemod.RvaToPointer(Exports->AddressOfNameOrdinals);
	DWORD* FunctionTable = (DWORD*) pemod.RvaToPointer(Exports->AddressOfFunctions);

	unsigned long offset = PEh->OptionalHeader.ImageBase >= 0x80000000
			? PEh->OptionalHeader.ImageBase : 0;

	//calculate required space
	int space = 0;

	space += sizeof(apilib_unnamed_api) * (Exports->NumberOfFunctions + 1);
	space += sizeof(apilib_named_api) * (Exports->NumberOfNames + 1);

	DWORD* pname = Names;
	for (DWORD i = 0 ; i < Exports->NumberOfNames ; i++)
		space += strlen((char*) pemod.RvaToPointer(*pname++)) + 1;

	void* mem = malloc(space);
	
	if (!mem)
		return false;

	char* pstr = (char*) mem + space;

	// read exports by name
	apilib_named_api* pna = (apilib_named_api*) mem;
	api_tab->named_apis = pna;
	api_tab->named_apis_count = Exports->NumberOfNames;
	pname = Names;
	for (DWORD i = 0 ; i < Exports->NumberOfNames ; i++, pna++)
	{
		WORD ord = OrdinalTable[i];
		DWORD addr = FunctionTable[ord];
		char* api_name = (char*) pemod.RvaToPointer(*pname++);

		pstr -= strlen(api_name) + 1;
		strcpy(pstr, api_name);
		pna->name = pstr;

		//export forwarding case
		if (addr >= ExportsRva && addr < ExportsRva + ExportsSize)
			addr = 0xffff0000 | ord + Exports->Base;
		else
			addr += offset;

		pna->addr = addr;
	}
	
	// read exports by ordinals
	apilib_unnamed_api* pua = (apilib_unnamed_api*) pna;
	api_tab->ordinal_apis = pua;
	api_tab->ordinal_apis_count = Exports->NumberOfFunctions;
	DBGASSERT(Exports->NumberOfFunctions < 0x10000);

	for (DWORD i = 0 ; i < Exports->NumberOfFunctions ; i++, pua++)
	{
		DWORD addr = FunctionTable[i];

		pua->ord = (WORD)(Exports->Base + i);

		//export forwarding case
		if (addr >= ExportsRva && addr < ExportsRva + ExportsSize)
			addr = 0xffff0000 | i + Exports->Base;
		else
			addr += offset;

		pua->addr = addr;
	}

	char* new_mod = (char*) malloc(strlen(dll_name) + 1);
	if (!new_mod)
	{
		DBGPRINTF(("Failed to allocate memory\n"));
		return false;
	}
	strcpy(new_mod, dll_name);
	strupr(new_mod);
	api_tab->target_library = new_mod;

	return true;
}

void ApiLibraryManager::rollback_changes()
{
	if (!initialized)
		return;

	//STD api library case
	//unload new system module override tables <overridden_mod_cnt;new_overridden_mod_cnt)
	for (apilib_api_table* p = &new_apilib_ptrs[0]->api_tables
			[overridden_module_count] ; p->target_library ; p++)
		free((void*) p->named_apis); //consistent with parse_system_dll
	free(new_apilib_ptrs[0]->api_tables);
	free(new_apilib_ptrs[0]);

	//other api libraries
	for (int i = 1 ; i < new_apilib_cnt ; i++)
	{
		if (apilib_ptrs)
		{
			int j;

			for (j = 1 ; j < apilib_cnt ; j++)
				if (new_apilib_ptrs[i] == apilib_ptrs[j])
					break;

			if (j != apilib_cnt)
				continue;
		}
		FreeLibrary(new_apilib_ptrs[i]->mod_handle);
		free(new_apilib_ptrs[i]->api_tables);
		free(new_apilib_ptrs[i]);
	}

	free(new_apilib_ptrs);
	new_apilib_ptrs = NULL;
	new_apilib_cnt = 0;

	for (int i = overridden_module_count ; i < new_overridden_mod_cnt ; i++)
		free((void*) new_overridden_mod_nms[i]);
	
	free(new_overridden_mod_nms);
	new_overridden_mod_nms = NULL;
	new_overridden_mod_cnt = 0;

	initialized = false;
}

void ApiLibraryManager::commit_changes()
{
	if (!initialized)
		return;

	ApiLibrary** old_apilib_ptrs = apilib_ptrs;
	int old_apilib_cnt = apilib_cnt;

	//LOCK ALL CRITICAL SECTIONS!
	apilib_ptrs = new_apilib_ptrs;
	apilib_cnt = new_apilib_cnt;
	free(overridden_module_names);
	overridden_module_names = new_overridden_mod_nms;
	overridden_module_count = new_overridden_mod_cnt;

	//update old api library mod_index values to new
	//set all non-overridden modules to 'not checked'
	WORD imteMax = *pimteMax;
	IMTE** pmteModTable = *ppmteModTable;
	for (WORD i = 0 ; i < imteMax ; i++)
	{
		IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[i];
		if (imte)
		{
			WORD index = imte->mod_index;
			if (index >= 0xff00 && index < 0xffff)
			{
				int api_lib_num = index & 0xff;
				DBGASSERT(api_lib_num != 0); //reserved for STD apilib
				DBGASSERT(api_lib_num >= old_apilib_cnt);
				imte->mod_index = old_apilib_ptrs[api_lib_num]->index + 0xff00;
			}
			else if (index == 0xffff)
				imte->mod_index = 0;
		}
	}

	//set mod_index for newly loaded api libraries
	for (int i = 1 ; i < apilib_cnt ; i++)
	{
		ApiLibrary* apilib = apilib_ptrs[i];
		MODREF* mr = MRFromHLib(apilib->mod_handle);
		DBGASSERT(mr);
		((IMTE_KEX*) pmteModTable[mr->mteIndex])->mod_index = 0xff00 + i;
	}
	//UNLOCK ALL CRITICAL SECTIONS!

	new_overridden_mod_nms = NULL;
	new_overridden_mod_cnt = 0;
	new_apilib_ptrs = NULL;
	new_apilib_cnt = 0;

	//STD api library case
	if (old_apilib_cnt > 0)
	{
		free(old_apilib_ptrs[0]->api_tables);
		free(old_apilib_ptrs[0]);
	}

	//other api libraries
	for (int i = 1 ; i < old_apilib_cnt ; i++)
	{
		int j;

		for (j = 1 ; j < apilib_cnt ; j++)
			if (old_apilib_ptrs[i] == apilib_ptrs[j])
				break;

		if (j != apilib_cnt)
			continue;

		FreeLibrary(old_apilib_ptrs[i]->mod_handle);
		free(old_apilib_ptrs[i]->api_tables);
		free(old_apilib_ptrs[i]);
	}

	free(old_apilib_ptrs);

	initialized = false;
}

