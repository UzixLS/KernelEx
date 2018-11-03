/*
 *  KernelEx
 *
 *  Copyright (C) 2008, Tihiy
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

static BOOL IsValidHeap(HANDLE hHeap)
{
	WORD *sig;
	if ( IsBadReadPtr(hHeap,0x74) ) return FALSE;
	sig = (WORD*)((DWORD)hHeap+0x72);
	if ( *sig != 0x4948 ) return FALSE;
	return TRUE;
}

/* MAKE_EXPORT HeapLock_new=HeapLock */
BOOL WINAPI HeapLock_new(
	HANDLE hHeap
)
{
	if ( !IsValidHeap(hHeap) ) return FALSE;
	EnterCriticalSection((CRITICAL_SECTION*)((DWORD)hHeap+0x50));
	return TRUE;
}

/* MAKE_EXPORT HeapUnlock_new=HeapUnlock */
BOOL WINAPI HeapUnlock_new(
	HANDLE hHeap
)
{
	if ( !IsValidHeap(hHeap) ) return FALSE;
	LeaveCriticalSection((CRITICAL_SECTION*)((DWORD)hHeap+0x50));
	return TRUE;
}

#define ISPOINTER(h)         (((ULONG_PTR)(h)&2)==0)

/* MAKE_EXPORT GlobalLock_fix=GlobalLock */
LPVOID WINAPI GlobalLock_fix(
	HGLOBAL hMem    // address of the global memory object 
)
{
	if (ISPOINTER(hMem))
		return IsBadReadPtr(hMem, 1) ? NULL : hMem;
	return GlobalLock(hMem);
}

/* MAKE_EXPORT GlobalUnlock_fix=GlobalUnlock */
BOOL WINAPI GlobalUnlock_fix(
	HGLOBAL hMem    // handle to the global memory object 
)
{
	if (ISPOINTER(hMem)) 
		return TRUE;
	return GlobalUnlock(hMem);
}
