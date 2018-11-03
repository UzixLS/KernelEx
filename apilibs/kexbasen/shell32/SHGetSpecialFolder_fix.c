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

#include <shlobj.h>
#include "folderfix.h"

/* MAKE_EXPORT SHGetSpecialFolderLocation_fix=SHGetSpecialFolderLocation */
HRESULT WINAPI SHGetSpecialFolderLocation_fix(HWND hwndOwner, int nFolder, LPVOID *_ppidl)
{
	LPITEMIDLIST *ppidl = (LPITEMIDLIST*) _ppidl;
	// explorer shouldn't receive this because of start menu bug which displays entries twice
	if (GetModuleHandle("EXPLORER.EXE") == NULL)
		nFolder = folder_fix(nFolder);
	return SHGetSpecialFolderLocation(hwndOwner, nFolder, ppidl);
}

/* MAKE_EXPORT SHGetSpecialFolderPathA_fix=SHGetSpecialFolderPathA */
BOOL WINAPI SHGetSpecialFolderPathA_fix(HWND hwndOwner, LPSTR lpszPath, int nFolder, BOOL fCreate)
{
	nFolder = folder_fix(nFolder);
	return SHGetSpecialFolderPathA(hwndOwner, lpszPath, nFolder, fCreate);
}

/* MAKE_EXPORT SHGetSpecialFolderPathW_fix=SHGetSpecialFolderPathW */
BOOL WINAPI SHGetSpecialFolderPathW_fix(HWND hwndOwner, LPWSTR lpszPath, int nFolder, BOOL fCreate)
{
	nFolder = folder_fix(nFolder);
	return SHGetSpecialFolderPathW(hwndOwner, lpszPath, nFolder, fCreate);
}
