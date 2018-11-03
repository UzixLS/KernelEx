/*
 *  KernelEx
 *  Copyright (C) 2009, Xeno86
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

/* MAKE_EXPORT GetVolumePathNameA_new=GetVolumePathNameA */
BOOL WINAPI GetVolumePathNameA_new(LPCSTR filename, LPSTR volumepathname, DWORD buflen)
{
	//semi-stub - doesn't work with UNC
    const char *p = filename;

    if (p && tolower(p[0]) >= 'a' && tolower(p[0]) <= 'z' && p[1] ==':' && p[2] == '\\' && buflen >= 4)
    {
        volumepathname[0] = p[0];
        volumepathname[1] = ':';
        volumepathname[2] = '\\';
        volumepathname[3] = 0;
        return TRUE;
    }
    return FALSE;
}

/* MAKE_EXPORT GetVolumePathNameW_new=GetVolumePathNameW */
BOOL WINAPI GetVolumePathNameW_new(LPCWSTR filenameW, LPWSTR volumepathnameW, DWORD buflen)
{
	BOOL ret;
	file_GetCP();
	file_ALLOC_WtoA(filename);
	ALLOC_A(volumepathname, buflen);
	ret = GetVolumePathNameA_new(filenameA, volumepathnameA, buflen);
	if (ret)
		file_ABUFtoW(volumepathname, -1, buflen);
	return ret;
}
