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

#ifndef VA_SHARED
#define VA_SHARED	0x8000000
#endif

/* MAKE_EXPORT VirtualAllocEx_new=VirtualAllocEx */
LPVOID WINAPI VirtualAllocEx_new(HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD flAllocationType, DWORD flProtect)
{
	if (hProcess != GetCurrentProcess())
	//if (GetProcessID(hProcess) != GetCurrentProcessId())
		flAllocationType |= VA_SHARED;
	return VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
}

/* MAKE_EXPORT VirtualFreeEx_new=VirtualFreeEx */
BOOL WINAPI VirtualFreeEx_new(HANDLE hProcess, LPVOID lpAddress, DWORD dwSize, DWORD dwFreeType)
{
	return VirtualFree(lpAddress, dwSize, dwFreeType);
}
