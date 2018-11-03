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

/* MAKE_EXPORT GetFileSizeEx_new=GetFileSizeEx */
BOOL WINAPI GetFileSizeEx_new(HANDLE hFile, PLARGE_INTEGER lpFileSize)
{
	DWORD h, l, lasterr;
	lasterr = GetLastError();
	l = GetFileSize(hFile, &h);
	if (l == INVALID_FILE_SIZE && GetLastError() != NO_ERROR) return FALSE;
	else 
	{
		lpFileSize->HighPart = h;
		lpFileSize->LowPart = l;
	}
	SetLastError(lasterr);
	return TRUE;
}
