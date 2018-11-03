/*
 *  KernelEx
 *  Copyright (C) 2010, Tihiy
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
#include "kexcoresdk.h"
#include "kstructs.h"
#include "k32ord.h"

#define RELTOABS(x) ( (DWORD)(x) + *(DWORD*)(x) + 4 )
#define MAPHANDLE_MAPPED 0x1000

/*
 *	A bit of mapping file 9x rules:
 *	copy-on-write access is possible to file mappings created with 
 *	PAGE_WRITECOPY. Note that it makes sense only to real files.
 *	If you CreateFileMapping on INVALID_HANDLE_VALUE with PAGE_WRITECOPY
 *	you will get mapping handle which can be only mapped with FILE_MAP_READ
 *	PAGE_READONLY can be mapped only with FILE_MAP_READ, end of story
 *	PAGE_READWRITE can be mapped with any FILE_MAP_READ or FILE_MAP_WRITE
 *	but not FILE_MAP_COPY; however FILE_MAP_ALL_ACCESS is fine
 *	kernel checks exactly for it, other masks with FILE_MAP_COPY will fail.
 *
 *	Office 2003 workaround is hack for specific access mask which breaks
 *	that FILE_MAP_COPY rule
 */

#define FILE_MAP_OFFICE2003 (SECTION_QUERY | SECTION_MAP_WRITE | \
		SECTION_MAP_READ |  SECTION_MAP_EXECUTE | \
		SECTION_EXTEND_SIZE | READ_CONTROL)

//MapHandle: maps kernel handle to object
static PVOID MapHandle(HANDLE hObject, DWORD dwFlags, DWORD dwReserved)
{
	typedef PDWORD (WINAPI *MapHandle_t)(HANDLE,DWORD,DWORD);
	static MapHandle_t KernelMapHandle;
	if (!KernelMapHandle)
		KernelMapHandle = (MapHandle_t)RELTOABS((DWORD)kexGetProcAddress(
					GetModuleHandle("KERNEL32.DLL"),"OpenVxDHandle")+0xC);
	return KernelMapHandle(hObject,dwFlags,dwReserved);
}

static LPVOID GetMapViewAddress(HANDLE hFileMapping)
{
	PVOID ret = (PVOID)-1;
	PFILEMAPPING mapobj;
	static CRITICAL_SECTION* k32lock;
	if (!k32lock)
		k32lock = (CRITICAL_SECTION*) kexGetK32Lock();
	_EnterSysLevel(k32lock);
	mapobj = (PFILEMAPPING)MapHandle(hFileMapping,MAPHANDLE_MAPPED,0);
	if (mapobj && mapobj->type == WIN98_K32OBJ_MEM_MAPPED_FILE)
		ret = mapobj->mapaddr;
	_LeaveSysLevel(k32lock);
	return ret;
}

static DWORD FindSharedMem64K(DWORD StartAddress)
{
	MEMORY_BASIC_INFORMATION mbi;
	DWORD TryAddress = StartAddress;
	while (true)
	{
		if (TryAddress & 0x0FFFF) //64K align
			TryAddress = (TryAddress & 0xFFFF0000) + 0x10000;
		if ( !VirtualQuery((PVOID)TryAddress,&mbi,sizeof(mbi)) ) return NULL;
		if (mbi.State == MEM_FREE)
			return TryAddress;
		TryAddress = (DWORD)mbi.BaseAddress + (DWORD)mbi.RegionSize;
	}	
}


/* MAKE_EXPORT MapViewOfFileEx_new=MapViewOfFileEx */
LPVOID WINAPI MapViewOfFileEx_new(
	HANDLE hFileMappingObject,
	DWORD dwDesiredAccess,
	DWORD dwFileOffsetHigh,
	DWORD dwFileOffsetLow,
	SIZE_T dwNumberOfBytesToMap,
	LPVOID lpBaseAddress
)
{
	/* 
	 * With this modification MapViewOfFileEx won't fail if lpBaseAddress 
	 * doesn't point to Shared Arena - it will ignore suggested starting address
	 */
	if ((DWORD)lpBaseAddress < 0x80000000 || (DWORD)lpBaseAddress >= 0xc0000000)
	{
		lpBaseAddress = 0;
	}

	LPVOID ret = NULL;
	if ( dwDesiredAccess == FILE_MAP_OFFICE2003 )
	{
		DBGPRINTF(("KEX: Using 64K align workaround on memmap handle %p\n",
				hFileMappingObject));
		//was it already mapped?
		LPVOID oldmapaddr = GetMapViewAddress(hFileMappingObject);
		if (oldmapaddr == (LPVOID)-1)
		{
			SetLastError(ERROR_INVALID_HANDLE);
			return NULL;
		}
		if (oldmapaddr) //if it was, try that address first
			ret = MapViewOfFileEx(hFileMappingObject,FILE_MAP_ALL_ACCESS,
				dwFileOffsetHigh,dwFileOffsetLow,dwNumberOfBytesToMap,oldmapaddr);
		//search shared memory piece, skipping 64K to not deadloop
		DWORD mem64K = 0x81000000;
		while (!ret)
		{			
			mem64K = FindSharedMem64K(mem64K + 0x10000);
			if (!mem64K)
			{
				DBGPRINTF(("KEX: Failed to find memory for mapping handle %p!",
						hFileMappingObject));
				SetLastError(ERROR_NOT_ENOUGH_MEMORY);
				return NULL;
			}
			DWORD gle = GetLastError();
			SetLastError(0);
			ret = MapViewOfFileEx(hFileMappingObject,FILE_MAP_ALL_ACCESS,dwFileOffsetHigh,
				dwFileOffsetLow,dwNumberOfBytesToMap,(LPVOID)mem64K);
			if ( GetLastError() == ERROR_INVALID_PARAMETER ) //not according to plan
				return NULL;
			else
				SetLastError(gle);
		}
	}
	else
	{
		ret = MapViewOfFileEx(hFileMappingObject, dwDesiredAccess, dwFileOffsetHigh, 
				dwFileOffsetLow, dwNumberOfBytesToMap, lpBaseAddress);
	}
	
	return ret;
}

/* MAKE_EXPORT MapViewOfFile_new=MapViewOfFile */
LPVOID WINAPI MapViewOfFile_new( HANDLE hFileMappingObject, DWORD dwDesiredAccess,
  DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap )
{
	return MapViewOfFileEx_new(hFileMappingObject, dwDesiredAccess, 
			dwFileOffsetHigh, dwFileOffsetLow, dwNumberOfBytesToMap, NULL);
}
