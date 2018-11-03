/*
 *  KernelEx
 *  Copyright (C) 2010, Tihiy
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

#include "common.h"
#include "_kernel32_apilist.h"

/* MAKE_EXPORT FindFirstFileExA_new=FindFirstFileExA */
HANDLE WINAPI FindFirstFileExA_new( LPCSTR lpFileNameA, FINDEX_INFO_LEVELS fInfoLevelId, 
					   LPWIN32_FIND_DATAA lpFindFileDataA, FINDEX_SEARCH_OPS fSearchOp,
					   LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
	return FindFirstFileA(lpFileNameA,lpFindFileDataA);
}

/* MAKE_EXPORT FindFirstFileExW_new=FindFirstFileExW */
HANDLE WINAPI FindFirstFileExW_new( LPWSTR lpFileNameW, FINDEX_INFO_LEVELS fInfoLevelId, 
					   LPWIN32_FIND_DATAW lpFindFileDataW, FINDEX_SEARCH_OPS fSearchOp,
					   LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{	
	return FindFirstFileW_new(lpFileNameW,lpFindFileDataW);
}
