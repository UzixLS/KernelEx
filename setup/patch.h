/*
 *  KernelEx
 *  Copyright (C) 2010, Xeno86
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

#ifndef __PATCH_H
#define __PATCH_H

class PEmanip;

class Patch
{
public:
	Patch(PEmanip& pem);
	void apply();

private:
	int find_pattern(DWORD offset, int size, const short* pattern, int pat_len, DWORD* found_loc);
	void set_pattern(DWORD loc, const short* new_pattern, int pat_len);
	void prepare_subsystem_check();
	void find_resource_check1();
	void find_resource_check2();
	void disable_named_and_rcdata_resources_mirroring();
	void mod_imte_alloc();
	void mod_mr_alloc();
	void mod_pdb_alloc();
	void find_ExportFromX();
	void find_IsKnownDLL();
	void find_FLoadTreeNotify1();
	void find_FLoadTreeNotify2();
	DWORD decode_call(DWORD addr, int len = 0);
	DWORD decode_jmp(DWORD addr, int len = 0);
	bool is_call_ref(DWORD loc, DWORD target);
	void set_call_ref(DWORD loc, DWORD target);
	void set_jmp_ref(DWORD loc, DWORD target);
	bool is_fixupc(DWORD addr);

	PEmanip& pefile;
	DWORD _GetOrdinal;
	DWORD _ExportFromOrdinal;
	DWORD _ExportFromName;
	DWORD _IsKnownDLL;
	DWORD _FLoadTreeNotify;
	DWORD _SubsysCheckPerform;
	DWORD GetOrdinal_call1;
	DWORD GetOrdinal_call2;
	DWORD gpa_ExportFromOrdinal_call;
	DWORD gpa_ExportFromName_call;
	DWORD EFN_EFO_call;
	DWORD IsKnownDLL_call;
	DWORD FLoadTreeNotify_call1;
	DWORD FLoadTreeNotify_call2;
	DWORD SubsysCheck_jmp;
};

#endif
