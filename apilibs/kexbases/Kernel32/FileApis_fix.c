/*
 *  KernelEx
 *  Copyright (C) 2008-2009, Xeno86, Tihiy
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
	//FILE_SHARE_DELETE is not supported
	dwShareMode &= ~FILE_SHARE_DELETE;
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
		SetLastError(lasterr);
		if ((SetFilePointer(hFile, lpOverlapped->Offset, &high, FILE_BEGIN)
				== INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR))
				return FALSE;
		ResetEvent(lpOverlapped->hEvent);
		lpOverlapped->Internal = STATUS_PENDING;
		lpOverlapped->InternalHigh = 0;
		BOOL result = 
			ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, &lpOverlapped->InternalHigh, 0);
		lasterr = GetLastError();
		lpOverlapped->Internal = STATUS_WAIT_0;
		SetEvent(lpOverlapped->hEvent);
		SetLastError(lasterr);
		if (lpNumberOfBytesRead)
			*lpNumberOfBytesRead = lpOverlapped->InternalHigh;
		return result;
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
				== INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR))
			return FALSE;
		ResetEvent(lpOverlapped->hEvent);
		lpOverlapped->Internal = STATUS_PENDING;
		lpOverlapped->InternalHigh = 0;
		BOOL result = 
			WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &lpOverlapped->InternalHigh, 0);
		lasterr = GetLastError();
		lpOverlapped->Internal = STATUS_WAIT_0;
		SetEvent(lpOverlapped->hEvent);
		SetLastError(lasterr);
		if (lpNumberOfBytesWritten)
			*lpNumberOfBytesWritten = lpOverlapped->InternalHigh;
		return result;
	}
	return FALSE;
}

/* MAKE_EXPORT GetTempFileNameA_fix=GetTempFileNameA */
UINT WINAPI GetTempFileNameA_fix(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPTSTR lpTempFileName)
{
	static int g_tempprefix = 0;

	if (!lpPathName) 
		lpPathName = "\\";
	if (!lpPrefixString)
	{
		char temppref[2];
		g_tempprefix++;
		g_tempprefix %= 5;
		temppref[0] = '0' + g_tempprefix;
		temppref[1] = '\0';
		lpPrefixString = temppref;
	}
	return GetTempFileNameA(lpPathName,lpPrefixString,uUnique,lpTempFileName);
}

/* MAKE_EXPORT GetDiskFreeSpaceA_fix=GetDiskFreeSpaceA */
BOOL WINAPI GetDiskFreeSpaceA_fix(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, 
								  LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters,
								  LPDWORD lpTotalNumberOfClusters)
{
	char newRootPath[4] = { "X:\\" };
	if (lstrlenA(lpRootPathName) == 2 && lpRootPathName[1] == ':') //GetDiskFreeSpace fails on paths like C:
	{
		newRootPath[0] = lpRootPathName[0];
		lpRootPathName = newRootPath;
	}
	BOOL ret = GetDiskFreeSpaceA(lpRootPathName,lpSectorsPerCluster,lpBytesPerSector,lpNumberOfFreeClusters,lpTotalNumberOfClusters);
	if (!ret) //more suprisingly, it may fail on paths with two extensions (e.g. c:\game.exe.lnk)
	{
		char shortPath[MAX_PATH];
		if (GetShortPathName(lpRootPathName,shortPath,MAX_PATH))
		{
			//GetDiskFreeSpace will still fail on short path name, so we check drive root, having path validated
			newRootPath[0] = shortPath[0];
			ret = GetDiskFreeSpaceA(newRootPath,lpSectorsPerCluster,lpBytesPerSector,lpNumberOfFreeClusters,lpTotalNumberOfClusters);
		}
	}
	return ret;
}

/* MAKE_EXPORT GetDiskFreeSpaceExA_fix=GetDiskFreeSpaceExA */
BOOL WINAPI GetDiskFreeSpaceExA_fix(LPCSTR lpDirectoryName, PULARGE_INTEGER lpFreeBytesAvailable, 
									PULARGE_INTEGER lpTotalNumberOfBytes, PULARGE_INTEGER lpTotalNumberOfFreeBytes)
{
	//GetDiskFreeSpaceEx does not fail on paths like C: but still fails on on paths with two extensions
	BOOL ret = GetDiskFreeSpaceExA(lpDirectoryName,lpFreeBytesAvailable,lpTotalNumberOfBytes,lpTotalNumberOfFreeBytes);
	if (!ret)
	{
		char shortPath[MAX_PATH];
		if (GetShortPathName(lpDirectoryName,shortPath,MAX_PATH))
			ret = GetDiskFreeSpaceExA(shortPath,lpFreeBytesAvailable,lpTotalNumberOfBytes,lpTotalNumberOfFreeBytes);
	}
	return ret;
}
