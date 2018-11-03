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

#ifndef __APILIB_H
#define __APILIB_H

#include "kexcoresdk.h"

struct ApiLibrary
{
	apilib_api_table* api_tables;
	HMODULE mod_handle;
	int index;
	char apilib_name[1]; // variable size array
};

class ApiLibraryManager
{
public:
	ApiLibraryManager();
	~ApiLibraryManager();
	bool load_apilib(const char* apilib_name);
	ApiLibrary* get_new_apilib(const char* apilib_name);
	static ApiLibrary* get_apilib(int index);
	void commit_changes();
	void rollback_changes();

protected:
	
private:
	bool initialized;
	ApiLibrary** new_apilib_ptrs;
	int new_apilib_cnt;

	static ApiLibrary** apilib_ptrs;
	static int apilib_cnt;

	bool initialize();
	bool are_api_tables_sorted(const apilib_api_table* tab);
	apilib_api_table* make_shared_api_tables(const apilib_api_table* in);
	int required_api_table_space(const apilib_api_table* tab, bool copy_strings);
	bool add_overridden_module(const char* mod);
	bool parse_system_dll(const char* dll_name, apilib_api_table* api_tab);
};

extern int          overridden_module_count;
extern const char** overridden_module_names;
extern const char** new_overridden_mod_nms;
extern int          new_overridden_mod_cnt;

#endif
