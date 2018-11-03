/*
 *  KernelEx
 *  Copyright (C) 2008-2009, Xeno86
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

/* MAKE_EXPORT CreateFileA_fix=CreateFileA */
HANDLE WINAPI CreateFileA_fix(LPCSTR lpFileName, DWORD dwDesiredAccess, 
	DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, 
	DWORD dwCreationDistribution, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	/* translate NT-specific access rights to generic ones */
	if (dwDesiredAccess & ~0xf0010100)
	{
		DWORD oldaccess = dwDesiredAccess;
		dwDesiredAccess &= 0xf0010100;
		if (oldaccess & FILE_READ_DATA)
			dwDesiredAccess |= GENERIC_READ;
		if (oldaccess & FILE_WRITE_DATA)
			dwDesiredAccess |= GENERIC_WRITE;
		if (oldaccess & FILE_EXECUTE)
			dwDesiredAccess |= GENERIC_EXECUTE;
	}
	// hTemplate has to be NULL on 9x
	hTemplateFile = NULL;
	// special case: overlapped I/O
	if (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED)
	{
		DWORD lasterr = GetLastError();
		HANDLE h;
		h = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
				dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
		if (h != INVALID_HANDLE_VALUE)
			return h;
		SetLastError(lasterr);
		dwFlagsAndAttributes &= ~FILE_FLAG_OVERLAPPED;
	}
	return CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
		dwCreationDistribution, dwFlagsAndAttributes, hTemplateFile);
}

/* MAKE_EXPORT ReadFile_fix=ReadFile */
BOOL WINAPI ReadFile_fix(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
	LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	DWORD lasterr = GetLastError();

	if (ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, 
			lpOverlapped))
		return TRUE;

	if (lpOverlapped && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		LONG high = lpOverlapped->OffsetHigh;
		DWORD nr;
		SetLastError(lasterr);
		if ((SetFilePointer(hFile, lpOverlapped->Offset, &high, FILE_BEGIN)
				== (DWORD)-1 && GetLastError() != NO_ERROR) ||
				(ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &nr, 0) && !nr))
		{
			SetLastError(ERROR_HANDLE_EOF);
			return FALSE;
		}
		*lpNumberOfBytesRead = nr;
		return TRUE;
	}
	return FALSE;
}

/* MAKE_EXPORT WriteFile_fix=WriteFile */
BOOL WINAPI WriteFile_fix(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, 
	LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped)
{
	DWORD lasterr = GetLastError();

	if (WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, 
			lpNumberOfBytesWritten, lpOverlapped))
		return TRUE;

	if (lpOverlapped && GetLastError() == ERROR_INVALID_PARAMETER)
	{
		LONG high = lpOverlapped->OffsetHigh;
		SetLastError(lasterr);
		if ((SetFilePointer(hFile, lpOverlapped->Offset, &high, FILE_BEGIN)
				== (DWORD)-1 && GetLastError() != NO_ERROR))
			return FALSE;
		return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, 0);
	}
	return FALSE;
}
