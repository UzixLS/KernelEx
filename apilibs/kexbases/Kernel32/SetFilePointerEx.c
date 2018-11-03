/*
 *  KernelEx
 *  Copyright (C) 2006, Xeno86
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

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER	((DWORD)-1)
#endif

/* MAKE_EXPORT SetFilePointerEx_new=SetFilePointerEx */
BOOL WINAPI SetFilePointerEx_new(HANDLE hFile, LARGE_INTEGER liDistanceToMove, PLARGE_INTEGER lpNewFilePointer, DWORD dwMoveMethod)
{
	DWORD lasterr = GetLastError();
	if ((liDistanceToMove.LowPart = SetFilePointer(hFile, liDistanceToMove.LowPart, &liDistanceToMove.HighPart, dwMoveMethod)) == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	if (lpNewFilePointer) *lpNewFilePointer = liDistanceToMove;
	SetLastError(lasterr);
	return TRUE;
}
