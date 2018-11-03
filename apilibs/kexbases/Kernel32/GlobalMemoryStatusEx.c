/*
 *  KernelEx
 *  Copyright (C) 2008, Xeno86
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

/* MAKE_EXPORT GlobalMemoryStatusEx_new=GlobalMemoryStatusEx */
BOOL WINAPI GlobalMemoryStatusEx_new(LPMEMORYSTATUSEX lpmemex)
{
	MEMORYSTATUS mem;

	if (lpmemex->dwLength != sizeof(*lpmemex))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

	mem.dwLength = sizeof(mem);
	GlobalMemoryStatus(&mem);
	lpmemex->dwMemoryLoad = mem.dwMemoryLoad;
	lpmemex->ullTotalPhys = mem.dwTotalPhys;
	lpmemex->ullAvailPhys = mem.dwAvailPhys;
	lpmemex->ullTotalPageFile = mem.dwTotalPageFile;
	lpmemex->ullAvailPageFile = mem.dwAvailPageFile;
	lpmemex->ullTotalVirtual = mem.dwTotalVirtual;
	lpmemex->ullAvailVirtual = mem.dwAvailVirtual;
	lpmemex->ullAvailExtendedVirtual = 0;

	return TRUE;
}
