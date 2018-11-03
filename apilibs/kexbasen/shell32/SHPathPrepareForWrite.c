/*
 *  KernelEx
 *  Copyright (C) 2018, Uzix
 *  Copyright (C) the Wine project
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

#if defined(_WIN32_IE) && _WIN32_IE != 0x0601
#undef _WIN32_IE
#endif
#define _WIN32_IE 0x0601
#include <shlobj.h>
#include "common.h"



/* MAKE_EXPORT SHPathPrepareForWriteA_new=SHPathPrepareForWriteA */
HRESULT WINAPI SHPathPrepareForWriteA_new(HWND hwnd, IUnknown *punkEnableModless, LPCSTR pszPathA, DWORD dwFlags)
{
    DWORD res;
    DWORD err;
    LPCSTR realpath;
    int len;
    CHAR* last_slash;
    CHAR* temppath=NULL;

    if (dwFlags & ~(SHPPFW_DIRCREATE|SHPPFW_ASKDIRCREATE|SHPPFW_IGNOREFILENAME)) {
        /* FIXME: unimplemented flags */
    }

    /* cut off filename if necessary */
    if (dwFlags & SHPPFW_IGNOREFILENAME)
    {
        last_slash = strrchr(pszPathA, '\\');
        if (last_slash == NULL)
            len = 1;
        else
            len = last_slash - pszPathA + 1;
        temppath = (LPSTR)HeapAlloc(GetProcessHeap(), 0, len * sizeof(CHAR));
        if (!temppath)
            return E_OUTOFMEMORY;
        lstrcpynA(temppath, pszPathA, len);
        realpath = temppath;
    }
    else
    {
        realpath = pszPathA;
    }

    /* try to create the directory if asked to */
    if (dwFlags & (SHPPFW_DIRCREATE|SHPPFW_ASKDIRCREATE))
    {
        if (dwFlags & SHPPFW_ASKDIRCREATE) {
            /* FIXME: treating SHPPFW_ASKDIRCREATE as SHPPFW_DIRCREATE */
        }

        SHCreateDirectoryExA(hwnd, realpath, NULL);
    }

    /* check if we can access the directory */
    res = GetFileAttributesA(realpath);

    HeapFree(GetProcessHeap(), 0, temppath);

    if (res == INVALID_FILE_ATTRIBUTES)
    {
        err = GetLastError();
        if (err == ERROR_FILE_NOT_FOUND)
            return HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND);
        return HRESULT_FROM_WIN32(err);
    }
    else if (res & FILE_ATTRIBUTE_DIRECTORY)
        return S_OK;
    else
        return HRESULT_FROM_WIN32(ERROR_DIRECTORY);
}


/* MAKE_EXPORT SHPathPrepareForWriteW_new=SHPathPrepareForWriteW */
HRESULT WINAPI SHPathPrepareForWriteW_new(HWND hwnd, IUnknown *punkEnableModless, LPCWSTR pszPathW, DWORD dwFlags)
{
	ALLOC_WtoA(pszPath);
	return SHPathPrepareForWriteA_new(hwnd, punkEnableModless, pszPathA, dwFlags);
}
