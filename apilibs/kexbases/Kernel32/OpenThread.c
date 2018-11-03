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
#include "kexcoresdk.h"

typedef HANDLE (__stdcall *AllocHandle_t)(void* current_pdb, void* tdb, DWORD access);
AllocHandle_t AllocHandle = NULL;

BOOL init_openthread()
{
	int i;
	DWORD addr = (DWORD) kexGetProcAddress(GetModuleHandle("kernel32"), "OpenProcess");
	for (i = 0 ; i < 100 ; i++, addr++)
		if (*(DWORD*) addr == 0xe832ff50)
		{
			addr += 4;
			AllocHandle = (AllocHandle_t)(addr + 4 + *(DWORD*)addr);
			return TRUE;
		}
	return FALSE;
}

/* MAKE_EXPORT OpenThread_new=OpenThread */
HANDLE WINAPI OpenThread_new(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId)
{
	HANDLE ret;
	BYTE* tdb = (BYTE*) kexTIDtoTDB(dwThreadId);
	if (!tdb || *tdb != 7)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	dwDesiredAccess &= THREAD_ALL_ACCESS;
	if (bInheritHandle)
		dwDesiredAccess |= 0x80000000;
	ret = AllocHandle(kexPIDtoPDB(GetCurrentProcessId()), tdb, dwDesiredAccess);
	if (ret == INVALID_HANDLE_VALUE)
		return NULL;
	return ret;
}
