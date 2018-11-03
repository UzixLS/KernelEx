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
#include <stdio.h>
#include <malloc.h>
#include "debug.h"
#include "internals.h"
#include "resolver.h"
#include "apiconfmgr.h"

#define ALLOC_CAPACITY 10

ApiConfiguration** ApiConfigurationManager::curr_apiconf_ptrs = NULL;
int ApiConfigurationManager::curr_apiconf_cnt = 0;
ApiConfiguration** ApiConfigurationManager::prev_apiconf_ptrs = NULL;
int ApiConfigurationManager::prev_apiconf_cnt = 0;
ApiConfiguration* ApiConfigurationManager::default_apiconf = NULL;

//TODO: I should check if new ApiConfiguration is identical to existing one
//and deallocate it if this is true

ApiConfigurationManager::ApiConfigurationManager()
{
	new_apiconf_ptrs = NULL;
	new_apiconf_cnt = 0;
	strcpy(core_conf_file, kernelex_dir.get());
	strcat(core_conf_file, "core.ini");
}

ApiConfigurationManager::~ApiConfigurationManager()
{
	for (int i = 0 ; i < new_apiconf_cnt ; i++)
		delete new_apiconf_ptrs[i];
	if (new_apiconf_ptrs)
		free(new_apiconf_ptrs);
}

bool ApiConfigurationManager::add_apiconf(ApiConfiguration* ac)
{
	//allocate space for new ApiConfigurations
	if (new_apiconf_cnt % ALLOC_CAPACITY == 0)
	{
		void* new_block = realloc(new_apiconf_ptrs, 
				(new_apiconf_cnt + ALLOC_CAPACITY) * sizeof(ApiConfiguration*));
		
		if (!new_block)
		{
			DBGPRINTF(("Failed to add api configuration to list of new ACs\n"));
			return false;
		}

		new_apiconf_ptrs = (ApiConfiguration**) new_block;
	}

	//add to table of new ApiConfigurations
	new_apiconf_ptrs[new_apiconf_cnt++] = ac;
	return true;
}

ApiConfiguration* ApiConfigurationManager::get_new_apiconf(const char* conf_name)
{
	for (int i = 0 ; i < new_apiconf_cnt ; i++)
		if (!strcmp(new_apiconf_ptrs[i]->conf_name, conf_name))
			return new_apiconf_ptrs[i];
	return NULL;
}

ApiConfiguration* ApiConfigurationManager::get_api_configuration(const char* conf_name)
{
	if (!strcmp("default", conf_name))
		return get_default_configuration();
	
	for (int i = 0 ; i < curr_apiconf_cnt ; i++)
		if (!strcmp(curr_apiconf_ptrs[i]->conf_name, conf_name))
			return curr_apiconf_ptrs[i];
	return NULL;
}

void ApiConfigurationManager::reload_api_configurations()
{
	DBGPRINTF(("Reloading api configurations\n"));
	
	default_apiconf_index = GetPrivateProfileInt(
			"ApiConfigurations", "default", 0, core_conf_file);

	for (int i = 0 ; i < 65536 ; i++)
	{
		char num[6];
		char apiconf_name[256];
		ApiConfiguration* apiconf = NULL;

		sprintf(num, "%d", i);
		if (!GetPrivateProfileString("ApiConfigurations", num, "", 
				apiconf_name, sizeof(apiconf_name), core_conf_file))
		{
			//no more entries
			if (i <= default_apiconf_index)
			{
				DBGPRINTF(("Failed to load default api configuration - aborting\n"));
				return;
			}
			break;
		}
		DBGPRINTF(("Processing api configuration #%s: %s\n", num, apiconf_name));

		char buf[256];

		if (!GetPrivateProfileString(apiconf_name, "inherit", "", buf, 
				sizeof(buf), core_conf_file) || !strcmpi(buf, "none"))
		{
			//no inherit
			apiconf = new ApiConfiguration(apiconf_name);
		}
		else
		{
			ApiConfiguration* src_conf = get_new_apiconf(buf);
			if (!src_conf)
			{
				DBGPRINTF(("Cannot inherit: %s. Configuration not found\n", buf));
				goto __error;
			}

			apiconf = new ApiConfiguration(apiconf_name, *src_conf);
		}

		if (!apiconf || !apiconf->is_initialized())
		{
			DBGPRINTF(("Failed to create new api configuration object\n"));
			goto __error;
		}

		if (!join_apilibs(apiconf) || !parse_overrides(apiconf) || !add_apiconf(apiconf))
			goto __error;

		continue;

__error:
		if (apiconf)
			delete apiconf;
		if (i == default_apiconf_index)
		{
			DBGPRINTF(("Failed to load default api configuration - aborting\n"));
			return;
		}
	} //for loop

	DBGPRINTF(("No more api configurations\n"));

	FullCritLock();
	
	if (commit_changes())
	{
		DBGPRINTF(("Re-add api libraries for previous api configurations\n"));
		for (int i = 0 ; i < prev_apiconf_cnt ; i++)
			for (int j = 0 ; j < prev_apiconf_ptrs[i]->used_apilibs_count ; j++)
				if (!libmgr.load_apilib(prev_apiconf_ptrs[i]->used_apilibs[j]->apilib_name))
					DBGPRINTF(("Error\n"));

		libmgr.commit_changes();
	}
	else
		DBGPRINTF(("Bailing out\n"));

	FullCritUnlock();
}

bool ApiConfigurationManager::join_apilibs(ApiConfiguration* apiconf)
{
	char buf[256];

	if (!GetPrivateProfileString(apiconf->get_name(), "contents", "", 
			buf, sizeof(buf), core_conf_file) || !strcmpi(buf, "none"))
	{
		DBGPRINTF(("No contents found\n"));
	}
	else
	{
		char buf2[256];

		strupr(buf);
		strcpy(buf2, buf);
		char* lib = strtok(buf, ",");
		while (lib)
		{
			if (!libmgr.load_apilib(lib))
			{
				DBGPRINTF(("Failed to load api library: %s\n", lib));
				return false;
			}
			lib = strtok(NULL, ",");
		}

		//merge has to be done in a separate loop because STD
		//api library is based on other loaded api libraries
		lib = strtok(buf2, ",");
		while (lib)
		{
			if (!apiconf->merge(libmgr.get_new_apilib(lib)))
			{
				DBGPRINTF(("Failed to merge api library: %s\n", lib));
				return false;
			}
			lib = strtok(NULL, ",");
		}
	}
	return true;
}

bool ApiConfigurationManager::parse_overrides(ApiConfiguration* apiconf)
{
	//parse names and ordinal overrides
	char buf[MAX_PATH];
	char section_data[32767];
	DWORD res;

	//first try [conf.names.xx]
	strcpy(buf, apiconf->get_name());
	strcat(buf, ".names");
	if (isWinMe())
		strcat(buf, ".me");
	else
		strcat(buf, ".98");

	res = GetPrivateProfileSection(buf, section_data, sizeof(section_data), 
			core_conf_file);

	//else try [conf.names]
	if (!res)
	{
		strcpy(buf, apiconf->get_name());
		strcat(buf, ".names");
		res = GetPrivateProfileSection(buf, section_data, sizeof(section_data), 
				core_conf_file);
	}

	if (res)
	{
		DBGPRINTF(("Parsing named api overrides\n"));

		char* line = section_data;
		while (*line)
		{
			char* module;
			char* api_name;
			char* apilib_name;
			int id;

			char* p = line;
			char* oldline = line;
			line += strlen(line) + 1;

			module = p;
			p = strchr(p, '.');
			if (!p)
			{
				DBGPRINTF(("Error parsing line: %s\n", oldline));
				continue;
			}
			*p++ = '\0';
			strcpy(buf, module);
			strcat(buf, ".DLL");
			module = buf;
			api_name = p;
			p = strchr(p, '=');
			if (!p)
			{
				DBGPRINTF(("Error parsing line: %s\n", oldline));
				continue;
			}
			*p++ = '\0';
			apilib_name = p;
			p = strchr(p, '.');
			if (!p)
				id = 0;
			else
			{
				*p++ = '\0';
				id = atoi(p);
				if (id < 0)
				{
					DBGPRINTF(("Error parsing line: %s\n", oldline));
					continue;
				}
			}
			strupr(apilib_name);
			if (!strcmp(apilib_name, "NONE"))
			{
				if (!apiconf->erase(module, api_name))
				{
					DBGPRINTF(("Failed to merge named api overrides\n"));
					return false;
				}
			}
			else
			{
				if (!apiconf->merge(module, api_name, 
						libmgr.get_new_apilib(apilib_name), id))
				{
					DBGPRINTF(("Failed to merge named api overrides\n"));
					return false;
				}
			}
		}
	}

	//first try [conf.ordinals.xx]
	strcpy(buf, apiconf->get_name());
	strcat(buf, ".ordinals");

	if (isWinMe())
		strcat(buf, ".me");
	else
		strcat(buf, ".98");

	//else try [conf.ordinals]
	res = GetPrivateProfileSection(buf, section_data, sizeof(section_data), 
			core_conf_file);
	
	if (!res)
	{
		strcpy(buf, apiconf->get_name());
		strcat(buf, ".ordinals");
		res = GetPrivateProfileSection(buf, section_data, sizeof(section_data), 
				core_conf_file);		
	}

	if (res)
	{
		DBGPRINTF(("Parsing ordinal api overrides\n"));

		char* line = section_data;
		while (*line)
		{
			char* module;
			char* ordinal_s;
			int ordinal;
			char* apilib_name;
			int id;

			char* p = line;
			char* oldline = line;
			line += strlen(line) + 1;

			module = p;
			p = strchr(p, '.');
			if (!p)
			{
				DBGPRINTF(("Error parsing line: %s\n", oldline));
				continue;
			}
			*p++ = '\0';
			strcpy(buf, module);
			strcat(buf, ".DLL");
			module = buf;
			ordinal_s = p;
			p = strchr(p, '=');
			if (!p)
			{
				DBGPRINTF(("Error parsing line: %s\n", oldline));
				continue;
			}
			*p++ = '\0';
			ordinal = atoi(ordinal_s);
			if (ordinal < 1 || ordinal > 65535)
			{
				DBGPRINTF(("Error parsing line: %s\n", oldline));
				continue;
			}
			apilib_name = p;
			p = strchr(p, '.');
			if (!p)
				id = 0;
			else
			{
				*p++ = '\0';
				id = atoi(p);
				if (id < 0)
				{
					DBGPRINTF(("Error parsing line: %s\n", oldline));
					continue;
				}
			}
			strupr(apilib_name);
			if (!strcmp(apilib_name, "NONE"))
			{
				if (!apiconf->erase(module, (WORD) ordinal))
				{
					DBGPRINTF(("Failed to merge ordinal api overrides\n"));
					return false;
				}
			}
			else 
			{
				if (!apiconf->merge(module, (WORD) ordinal, 
					libmgr.get_new_apilib(apilib_name), id))
				{
					DBGPRINTF(("Failed to merge ordinal api overrides\n"));
					return false;
				}
			}
		}
	}
	return true;
}

bool ApiConfigurationManager::commit_changes()
{
	DBGPRINTF(("Updating api configuration list\n"));

	//calculate number of apiconf in use
	WORD imteMax = *pimteMax;
	IMTE** pmteModTable = *ppmteModTable;
	int used = 0;
	for (int j = 0 ; j < curr_apiconf_cnt ; j++)
	{
		for (WORD i = 0 ; i < imteMax ; i++)
		{
			IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[i];
			if (imte && imte->config == curr_apiconf_ptrs[j])
			{
				used++;
				break;
			}
		}
	}
	for (int j = 0 ; j < prev_apiconf_cnt ; j++)
	{
		for (WORD i = 0 ; i < imteMax ; i++)
		{
			IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[i];
			if (imte && imte->config == prev_apiconf_ptrs[j])
			{
				used++;
				break;
			}
		}
	}

	//alloc space for new_prev
	ApiConfiguration** new_prev;
	new_prev = (ApiConfiguration**) malloc(used * sizeof(ApiConfiguration*));
	if (!new_prev)
		return false;

	int cnt = 0;

	//move used entries from curr and prev to new_prev, free unused
	for (int j = 0 ; j < curr_apiconf_cnt ; j++)
	{
		WORD i;
		for (i = 0 ; i < imteMax ; i++)
		{
			IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[i];
			if (imte && imte->config == curr_apiconf_ptrs[j])
			{
				new_prev[cnt++] = curr_apiconf_ptrs[j];
				break;
			}
		}
		if (i == imteMax)
			delete curr_apiconf_ptrs[j];
	}
	for (int j = 0 ; j < prev_apiconf_cnt ; j++)
	{
		WORD i;
		for (i = 0 ; i < imteMax ; i++)
		{
			IMTE_KEX* imte = (IMTE_KEX*) pmteModTable[i];
			if (imte && imte->config == prev_apiconf_ptrs[j])
			{
				new_prev[cnt++] = prev_apiconf_ptrs[j];
				break;
			}
		}
		if (i == imteMax)
			delete curr_apiconf_ptrs[j];
	}

	//replace prev with new_prev
	if (prev_apiconf_ptrs)
		free(prev_apiconf_ptrs);
	prev_apiconf_ptrs = new_prev;
	prev_apiconf_cnt = cnt;

	//replace curr with new
	if (curr_apiconf_ptrs)
		free(curr_apiconf_ptrs);
	curr_apiconf_ptrs = new_apiconf_ptrs;
	curr_apiconf_cnt = new_apiconf_cnt;
	new_apiconf_ptrs = NULL;
	new_apiconf_cnt = 0;

	//set default apiconf
	if (default_apiconf_index >= 0)
		default_apiconf = curr_apiconf_ptrs[default_apiconf_index];
	else
		default_apiconf = NULL;

	DBGPRINTF(("Default api configuration is: %s\n", 
			default_apiconf ? default_apiconf->get_name() : "system"));

	return true;
}

#ifdef _DEBUG

void ApiConfigurationManager::dump_configurations()
{
	dbgprintf("Dumping all api configurations (count = %d) ...\n\n", curr_apiconf_cnt);
	for (int i = 0 ; i < curr_apiconf_cnt ; i++)
	{
		curr_apiconf_ptrs[i]->dump();
	}
	dbgprintf("End dump\n\n");
}

#endif
