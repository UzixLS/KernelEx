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

#include "common.h"

//MAKE_EXPORT RegQueryValueExW_new=RegQueryValueExW
LONG WINAPI RegQueryValueExW_new(HKEY hKey, LPCWSTR lpValueNameW, LPDWORD lpReserved, 
	LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData)
{
	LONG ret;
	DWORD type;
	BYTE stackbuf[256];
	BYTE* ptr = stackbuf;
	BYTE* heapbuf = NULL;
	DWORD bufsize = sizeof(stackbuf);
	
	if ((lpData && !lpcbData) || lpReserved) return ERROR_INVALID_PARAMETER;
	if (!lpData && lpcbData) *lpcbData = 0;
	
	//try with stack buffer first
	if (lpValueNameW) 
	{
		ALLOC_WtoA(lpValueName);
		ret = RegQueryValueExA(hKey, lpValueNameA, lpReserved, &type, ptr, &bufsize);
		if (lpType) *lpType = type;
		if (lpcbData && type != REG_SZ && bufsize > *lpcbData) 
		{
			*lpcbData = bufsize;
			return ERROR_MORE_DATA;
		}
		//retry with dynamic buffer
		if (ret == ERROR_MORE_DATA)
		{
			ptr = heapbuf = (BYTE*) HeapAlloc(GetProcessHeap(), 0, bufsize);
			if (!heapbuf) 
			{
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			ret = RegQueryValueExA(hKey, lpValueNameA, lpReserved, &type, ptr, &bufsize);
		}
	}
	else
	{
		ret = RegQueryValueExA(hKey, 0, lpReserved, &type, ptr, &bufsize);
		if (lpType) *lpType = type;
		if (lpcbData && type != REG_SZ && bufsize > *lpcbData) 
		{
			*lpcbData = bufsize;
			return ERROR_MORE_DATA;
		}
		//retry with dynamic buffer
		if (ret == ERROR_MORE_DATA)
		{
			ptr = heapbuf = (BYTE*) HeapAlloc(GetProcessHeap(), 0, bufsize);
			if (!heapbuf) 
			{
				return ERROR_NOT_ENOUGH_MEMORY;
			}
			ret = RegQueryValueExA(hKey, 0, lpReserved, &type, ptr, &bufsize);
		}
	}
	if (ret != ERROR_SUCCESS) goto _end;
	
	if (type == REG_SZ)
	{
		if (lpcbData)
		{
			DWORD gle = GetLastError();
			int written = MultiByteToWideChar(CP_ACP, 0, (LPSTR) ptr, -1, 
				(LPWSTR)lpData, lpData ? (*lpcbData >> 1) : 0);
			if (!written)
			{
				ret = GetLastError();
				if (ret == ERROR_INSUFFICIENT_BUFFER)
				{
					*lpcbData = MultiByteToWideChar(CP_ACP, 0, (LPSTR) ptr, 
						-1, NULL, 0) << 1;
					ret = ERROR_MORE_DATA;
				}
				SetLastError(gle);
				goto _end;
			}
			*lpcbData = written << 1;
		}
	}
	else
	{
		if (lpData) memcpy(lpData, ptr, bufsize);
		if (lpcbData) *lpcbData = bufsize;
	}

_end:
	if (heapbuf) HeapFree(GetProcessHeap(), 0, heapbuf);
	return ret;
}
