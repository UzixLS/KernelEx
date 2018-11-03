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
#include "kexcoresdk.h"

typedef HRESULT (WINAPI *SHGetFolderPathA_t)(HWND, int, HANDLE, DWORD, LPSTR);
typedef HRESULT (WINAPI *SHGetFolderPathW_t)(HWND, int, HANDLE, DWORD, LPWSTR);

static SHGetFolderPathA_t SHGetFolderPathA_pfn = (SHGetFolderPathA_t)-1;
static SHGetFolderPathW_t SHGetFolderPathW_pfn = (SHGetFolderPathW_t)-1;

static PROC LoadShfolderProc(const char* proc)
{
	static const char ShfolderFn[] = "SHFOLDER.DLL";
	static const char Shell32Fn[] = "SHELL32.DLL";
	static HMODULE hShfolder;
	static HMODULE hShell32;
	PROC ret = NULL;
	DWORD lasterr = GetLastError();
	
	//first try with shell32
	if (!hShell32)
	{
		hShell32 = GetModuleHandle(Shell32Fn);
		if (!hShell32) hShell32 = LoadLibrary(Shell32Fn);
	}
	if (hShell32) ret = kexGetProcAddress(hShell32, proc);
	
	//fallback to shfolder
	if (!ret)
	{
		if (!hShfolder)
		{
			hShfolder = GetModuleHandle(ShfolderFn);
			if (!hShfolder) hShfolder = LoadLibrary(ShfolderFn);
		}
		if (hShfolder) ret = kexGetProcAddress(hShfolder, proc);
	}
	SetLastError(lasterr);
	return ret;
}

/* MAKE_EXPORT SHGetFolderPathA_new=SHGetFolderPathA */
HRESULT WINAPI SHGetFolderPathA_new(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPSTR pszPath)
{
	if (SHGetFolderPathA_pfn == (void*)-1)
		SHGetFolderPathA_pfn = (SHGetFolderPathA_t) LoadShfolderProc("SHGetFolderPathA");
	if (SHGetFolderPathA_pfn == NULL)
		return E_NOTIMPL;
	return SHGetFolderPathA_pfn(hwndOwner, nFolder, hToken, dwFlags, pszPath);
}

/* MAKE_EXPORT SHGetFolderPathW_new=SHGetFolderPathW */
HRESULT WINAPI SHGetFolderPathW_new(HWND hwndOwner, int nFolder, HANDLE hToken, DWORD dwFlags, LPWSTR pszPath)
{
	if (SHGetFolderPathW_pfn == (void*)-1)
		SHGetFolderPathW_pfn = (SHGetFolderPathW_t) LoadShfolderProc("SHGetFolderPathW");
	if (SHGetFolderPathW_pfn == NULL)
		return E_NOTIMPL;
	return SHGetFolderPathW_pfn(hwndOwner, nFolder, hToken, dwFlags, pszPath);
}
