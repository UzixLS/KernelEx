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

#ifndef __RESOLVER_H
#define __RESOLVER_H

#include "structs.h"
#include "apilib.h"

/***** loader flags *****/
#define LDR_KEX_DISABLE          1  /* disable KernelEx API extensions for this module */
#define LDR_OVERRIDE_PROC_MOD    2  /* use same configuration and flags for all modules in a process */
#define LDR_LOG_APIS             4  /* enable API tracing */
#define LDR_FILTER_APIS          8  /* allow to control single APIs - enable, disable, switch */
#define LDR_VALID_FLAG         128  /* denotes that flags field is valid */

#pragma pack(push,1)

class ApiConfiguration;

struct IMTE_KEX : public IMTE
{
	ApiConfiguration* config;   /* pointer to API configuration required by the module
	                             * 0 - not checked */
	BYTE            flags;      /* loader flags */
	BYTE            unused;     /* unused */
	WORD            mod_index;  /* this value minus 1 is index into MODAPI table in API configurations
								 * 0xff00-0xfffe - api libraries
	                             * 0 - not checked, 0xffff - not an overridden module */
};

#pragma pack(pop)

DWORD encode_address(DWORD addr, const ApiLibrary* apilib);
PROC WINAPI iGetProcAddress(HMODULE hModule, LPCSTR lpProcName);
PROC WINAPI ExportFromOrdinal(IMTE_KEX* target, MODREF* caller, PMODREF** refmod, WORD ordinal);
PROC WINAPI ExportFromName(IMTE_KEX* target, MODREF* caller, PMODREF** refmod, WORD hint, LPCSTR name);

#ifdef _DEBUG
void dump_imtes(void);
#endif

#endif
