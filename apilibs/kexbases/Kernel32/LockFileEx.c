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

/* MAKE_EXPORT LockFileEx_new=LockFileEx */
BOOL WINAPI LockFileEx_new(
	HANDLE hFile,
	DWORD dwFlags,
	DWORD dwReserved,
	DWORD nNumberOfBytesToLockLow,
	DWORD nNumberOfBytesToLockHigh,
	LPOVERLAPPED lpOverlapped
)
{
	/* 
	 * FIXME: flags LOCKFILE_FAIL_IMMEDIATELY and LOCKFILE_EXCLUSIVE_LOCK not supported (always set), 
	 * Event signalling not supported
	 */
	if (dwReserved)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	return LockFile(hFile, lpOverlapped->Offset, lpOverlapped->OffsetHigh, 
			nNumberOfBytesToLockLow, nNumberOfBytesToLockHigh);
}
