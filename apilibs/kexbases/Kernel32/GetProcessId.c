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

#include <windows.h>
#include "common.h"

/* MAKE_EXPORT GetProcessId_new=GetProcessId */
DWORD WINAPI GetProcessId_new(
  HANDLE hProcess
)
{
	typedef DWORD (WINAPI *MPH) (HANDLE hProcess);
	static MPH MapProcessHandle = 0;
	
	if (!MapProcessHandle)
	{
		DWORD *faddr;
		DWORD addr;
		
		faddr = (DWORD *) ( (DWORD)SetFilePointer + 0x1D ); //there is jmp _SetFilePointer	
		addr = (DWORD) faddr + *faddr + 4 - 0x16; //0x16 bytes before _SetFilePointer there is MapProcessHandle, just what we need	
		faddr = (DWORD *) addr;
		if (*faddr != 0x206A006A)
			fatal_error("GetProcessId: ASSERTION FAILED"); //push 0; push 0x20
		MapProcessHandle = (MPH) addr;
	}
	return MapProcessHandle(hProcess);
}
