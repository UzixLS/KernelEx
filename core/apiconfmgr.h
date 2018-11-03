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

#ifndef __APICONFMGR_H
#define __APICONFMGR_H

#include "apiconf.h"

class ApiConfigurationManager
{
public:
	ApiConfigurationManager();
	~ApiConfigurationManager();
	void reload_api_configurations();
	static ApiConfiguration* get_api_configuration(const char* conf_name);
	static ApiConfiguration* get_default_configuration();

#ifdef _DEBUG
	static void dump_configurations();
#endif

protected:
	
private:
	char core_conf_file[MAX_PATH];
	ApiConfiguration** new_apiconf_ptrs;
	int new_apiconf_cnt;
	int default_apiconf_index;
	ApiLibraryManager libmgr;

	static ApiConfiguration** curr_apiconf_ptrs;
	static int curr_apiconf_cnt;
	static ApiConfiguration** prev_apiconf_ptrs;
	static int prev_apiconf_cnt;
	static ApiConfiguration* default_apiconf;

	bool join_apilibs(ApiConfiguration* apiconf);
	bool parse_overrides(ApiConfiguration* apiconf);
	bool add_apiconf(ApiConfiguration* ac);
	ApiConfiguration* get_new_apiconf(const char* conf_name);
	bool commit_changes();
};

inline ApiConfiguration* ApiConfigurationManager::get_default_configuration()
{
	return default_apiconf;
}

#endif
