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

#ifndef __SETUP_H
#define __SETUP_H

#include "sstring.hpp"

class Setup
{
public:
	Setup(char* _backup_file);
	int get_signature_ver();
	void install();

protected:
private:
	sstring backup_file;
	DWORD _ExportFromOrdinal;
	DWORD _ExportFromName;
	DWORD _IsKnownDLL;
	DWORD _FLoadTreeNotify;
	HMODULE h_kernel32;
	PEmanip pemem;
	PEmanip pefile;
	DWORD gpa_ExportFromOrdinal_call;
	DWORD gpa_ExportFromName_call;
	DWORD EFN_EFO_call;
	DWORD IsKnownDLL_call;
	DWORD FLoadTreeNotify_call1;
	DWORD FLoadTreeNotify_call2;
	int version;
	bool is_winme;
	bool upgrade;

	bool detect_old_version();
	void detect_downgrade();
	int find_pattern(DWORD offset, int size, const short* pattern, int pat_len, DWORD* found_loc);
	void set_pattern(DWORD loc, const short* new_pattern, int pat_len);
	void disable_platform_check();
	void disable_resource_check();
	void disable_named_and_rcdata_resources_mirroring();
	void positive_pids_patch();
	void mod_imte_alloc();
	void mod_mr_alloc();
	void mod_pdb_alloc();
	void find_ExportFromX();
	void find_IsKnownDLL();
	void find_FLoadTreeNotify1();
	void find_FLoadTreeNotify2();
	void kill_process(const char* name);
	DWORD decode_call(DWORD addr, int len);
	bool is_call_ref(DWORD loc, DWORD target);
	void set_call_ref(DWORD loc, DWORD target);
	bool is_fixupc(DWORD addr);
	sstring get_temp_file_name();
	void ShowError(UINT id, ...);
};

#endif
