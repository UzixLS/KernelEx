/*
 *  KernelEx
 *  Copyright (C) 2009, Xeno86
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
#include "storage.h"
#include "internals.h"
#include "debug.h"

struct PDB_KEX : PDB98
{
	void* kex_data_storage;
};

const int storage::storage_size = 0x1000;

storage* storage::get_storage(bool alloc)
{
	PDB_KEX* pdb = (PDB_KEX*) *pppdbCur;
	storage* s = (storage*) pdb->kex_data_storage;

	if (!s && alloc)
	{
		pdb->kex_data_storage = VirtualAlloc(NULL, storage_size, 
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		DBGASSERT(pdb->kex_data_storage != NULL);
		memset(pdb->kex_data_storage, 0, storage_size);
		s = (storage*) pdb->kex_data_storage;
		s->size = 0;
	}

	return s;
}

void storage::return_storage()
{
	PDB_KEX* pdb = (PDB_KEX*) *pppdbCur;
	if (pdb->kex_data_storage)
	{
		BOOL ret = VirtualFree(pdb->kex_data_storage, 0, MEM_RELEASE);
		DBGASSERT(ret != FALSE);
		pdb->kex_data_storage = NULL;
	}
}
