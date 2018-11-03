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
#include "sharedmem.h"
#include "debug.h"

#define HEAP_SHARED   0x04000000

HANDLE shared_heap;

int create_shared_heap()
{
	shared_heap = HeapCreate(HEAP_SHARED, 0, 0x100000);
	return shared_heap ? 1 : 0;
}

void destroy_shared_heap()
{
	HeapDestroy(shared_heap);
}

void* malloc(size_t size)
{
	return HeapAlloc(shared_heap, 0, size);
}

void* calloc(size_t count, size_t size)
{
	return HeapAlloc(shared_heap, HEAP_ZERO_MEMORY, count * size);
}

void free(void* ptr)
{
	DBGASSERT(ptr == NULL || (DWORD)ptr >= 0x80000000);
	if (ptr)
		HeapFree(shared_heap, 0, ptr);
}

void* realloc(void* ptr, size_t size)
{
	DBGASSERT(ptr == NULL || (DWORD)ptr >= 0x80000000);
	if (ptr)
		return HeapReAlloc(shared_heap, 0, ptr, size);
	else 
		return HeapAlloc(shared_heap, 0, size);
}

void* recalloc(void* ptr, size_t size)
{
	DBGASSERT(ptr == NULL || (DWORD)ptr >= 0x80000000);
	if (ptr)
		return HeapReAlloc(shared_heap, HEAP_ZERO_MEMORY, ptr, size);
	else
		return HeapAlloc(shared_heap, HEAP_ZERO_MEMORY, size);
}

void* operator new(size_t size)
{
	void* mem = malloc(size);
	DBGASSERT(mem);
	return mem;
}

void operator delete(void* ptr)
{
	DBGASSERT((DWORD)ptr >= 0x80000000);
	free(ptr);
}
