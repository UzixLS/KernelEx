/*
 *  KernelEx
 *  Copyright (C) 2008, Tihiy
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

/* COMPLETE SHIT. PORT WINE HERE */

/* MAKE_EXPORT RegisterWaitForSingleObject_new=RegisterWaitForSingleObject */
BOOL WINAPI RegisterWaitForSingleObject_new(PHANDLE phNewWaitObject, HANDLE hObject, PVOID Callback, PVOID Context, ULONG dwMilliseconds, ULONG dwFlags)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}

/* MAKE_EXPORT RegisterWaitForSingleObjectEx_new=RegisterWaitForSingleObjectEx */
HANDLE WINAPI RegisterWaitForSingleObjectEx_new(HANDLE hObject, 
		PVOID Callback, PVOID Context,
		ULONG dwMilliseconds, ULONG dwFlags)
{
	return 0;
}

/* MAKE_EXPORT UnregisterWait_new=UnregisterWait */
BOOL WINAPI UnregisterWait_new(HANDLE WaitHandle)
{
	return WaitHandle != 0;
}

/* MAKE_EXPORT UnregisterWaitEx_new=UnregisterWaitEx */
BOOL WINAPI UnregisterWaitEx_new(HANDLE WaitHandle, HANDLE CompletionEvent) 
{
	return WaitHandle != 0;
}
