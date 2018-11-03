/*
 * Path Functions
 *
 * Copyright 1998, 1999, 2000 Juergen Schmied
 * Copyright 2004 Juan Lang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 *
 */

#ifdef _MSC_VER
#pragma warning(disable:4002)
#define TRACE() ((void)0)
#define TRACE_() ((void)0)
#define WARN() ((void)0)
#else
#define TRACE(...) do {} while(0)
#define TRACE_(x) TRACE
#define WARN(...) do {} while(0)
#endif

#define _WIN32_IE 0x0500

#include <windows.h>
#include <shlguid.h>
#include "auxshlguid.h"
#include "pidl.h"
#include <shlwapi.h>
#include "common.h"
#include "_shell32_apilist.h"

/**************************************************************************
 *
 *        internal functions
 *
 *    ### 1. section creating pidls ###
 *
 *************************************************************************
 */
LPITEMIDLIST _ILAlloc(PIDLTYPE type, unsigned int size)
{
    LPITEMIDLIST pidlOut = NULL;

    pidlOut = (LPITEMIDLIST) SHAlloc(size + 5);
    if(pidlOut)
    {
        LPPIDLDATA pData;
        LPITEMIDLIST pidlNext;

        ZeroMemory(pidlOut, size + 5);
        pidlOut->mkid.cb = size + 3;

        pData = _ILGetDataPointer(pidlOut);
        if (pData)
            pData->type = type;

        pidlNext = ILGetNext(pidlOut);
        if (pidlNext)
            pidlNext->mkid.cb = 0x00;
        TRACE("-- (pidl=%p, size=%u)\n", pidlOut, size);
    }

    return pidlOut;
}

LPITEMIDLIST _ILCreateDesktop()
{
    LPITEMIDLIST ret;

    TRACE("()\n");
    ret = (LPITEMIDLIST) SHAlloc(2);
    if (ret)
        ret->mkid.cb = 0;
    return ret;
}

LPITEMIDLIST _ILCreateMyComputer()
{
    TRACE("()\n");
    return _ILCreateGuid(PT_GUID, &CLSID_MyComputer);
}

LPITEMIDLIST _ILCreateMyDocuments()
{
    TRACE("()\n");
    return _ILCreateGuid(PT_GUID, &CLSID_MyDocuments);
}

LPITEMIDLIST _ILCreateIExplore()
{
    TRACE("()\n");
    return _ILCreateGuid(PT_GUID, &CLSID_Internet);
}

LPITEMIDLIST _ILCreateControlPanel()
{
    LPITEMIDLIST parent = _ILCreateGuid(PT_GUID, &CLSID_MyComputer), ret = NULL;

    TRACE("()\n");
    if (parent)
    {
        LPITEMIDLIST cpl = _ILCreateGuid(PT_SHELLEXT, &CLSID_ControlPanel);

        if (cpl)
        {
            ret = ILCombine(parent, cpl);
            SHFree(cpl);
        }
        SHFree(parent);
    }
    return ret;
}

LPITEMIDLIST _ILCreatePrinters()
{
    LPITEMIDLIST parent = _ILCreateGuid(PT_GUID, &CLSID_MyComputer), ret = NULL;

    TRACE("()\n");
    if (parent)
    {
        LPITEMIDLIST printers = _ILCreateGuid(PT_YAGUID, &CLSID_Printers);

        if (printers)
        {
            ret = ILCombine(parent, printers);
            SHFree(printers);
        }
        SHFree(parent);
    }
    return ret;
}

LPITEMIDLIST _ILCreateNetwork()
{
    TRACE("()\n");
    return _ILCreateGuid(PT_GUID, &CLSID_NetworkPlaces);
}

LPITEMIDLIST _ILCreateBitBucket()
{
    TRACE("()\n");
    return _ILCreateGuid(PT_GUID, &CLSID_RecycleBin);
}

LPITEMIDLIST _ILCreateGuid(PIDLTYPE type, const IID *guid)
{
    LPITEMIDLIST pidlOut;

    if (type == PT_SHELLEXT || type == PT_GUID || type == PT_YAGUID)
    {
        pidlOut = _ILAlloc(type, sizeof(GUIDStruct));
        if (pidlOut)
        {
            LPPIDLDATA pData = _ILGetDataPointer(pidlOut);

            memcpy(&(pData->u.guid.guid), guid, sizeof(GUID));
            TRACE("-- create GUID-pidl %s\n",
                  debugstr_guid(&(pData->u.guid.guid)));
        }
    }
    else
    {
        WARN("%d: invalid type for GUID\n", type);
        pidlOut = NULL;
    }
    return pidlOut;
}

/**************************************************************************
 *
 *    ### 4. getting pointers to parts of pidls ###
 *
 **************************************************************************
 *  _ILGetDataPointer()
 */
LPPIDLDATA _ILGetDataPointer(LPCITEMIDLIST pidl)
{
    if(pidl && pidl->mkid.cb != 0x00)
        return (LPPIDLDATA) &(pidl->mkid.abID);
    return NULL;
}

/*************************************************************************
 * SHGetFolderLocation [SHELL32.@]
 *
 * Gets the folder locations from the registry and creates a pidl.
 *
 * PARAMS
 *   hwndOwner  [I]
 *   nFolder    [I] CSIDL_xxxxx
 *   hToken     [I] token representing user, or NULL for current user, or -1 for
 *                  default user
 *   dwReserved [I] must be zero
 *   ppidl      [O] PIDL of a special folder
 *
 * RETURNS
 *  Success: S_OK
 *  Failure: Standard OLE-defined error result, S_FALSE or E_INVALIDARG
 *
 * NOTES
 *  Creates missing reg keys and directories.
 *  Mostly forwards to SHGetFolderPathW, but a few values of nFolder return
 *  virtual folders that are handled here.
 */
/* MAKE_EXPORT SHGetFolderLocation_new=SHGetFolderLocation */
HRESULT WINAPI SHGetFolderLocation_new(
	HWND hwndOwner,
	int nFolder,
	HANDLE hToken,
	DWORD dwReserved,
	LPVOID *_ppidl)
{
	LPITEMIDLIST *ppidl = (LPITEMIDLIST*) _ppidl;
    HRESULT hr = E_INVALIDARG;

    TRACE("%p 0x%08x %p 0x%08lx %p\n",
     hwndOwner, nFolder, hToken, dwReserved, ppidl);
    
    if (!ppidl)
        return E_INVALIDARG;
    if (dwReserved)
        return E_INVALIDARG;

    /* The virtual folders' locations are not user-dependent */
    *ppidl = NULL;
    switch (nFolder)
    {
        case CSIDL_DESKTOP:
            *ppidl = _ILCreateDesktop();
            break;

        case CSIDL_PERSONAL:
            *ppidl = _ILCreateMyDocuments();
            break;

        case CSIDL_INTERNET:
            *ppidl = _ILCreateIExplore();
            break;

        case CSIDL_CONTROLS:
            *ppidl = _ILCreateControlPanel();
            break;

        case CSIDL_PRINTERS:
            *ppidl = _ILCreatePrinters();
            break;

        case CSIDL_BITBUCKET:
            *ppidl = _ILCreateBitBucket();
            break;

        case CSIDL_DRIVES:
            *ppidl = _ILCreateMyComputer();
            break;

        case CSIDL_NETWORK:
            *ppidl = _ILCreateNetwork();
            break;

        default:
        {
            char szPath[MAX_PATH];

            hr = SHGetFolderPathA_new(hwndOwner, nFolder, hToken,
             SHGFP_TYPE_CURRENT, szPath);
            if (SUCCEEDED(hr))
            {
                DWORD attributes=0;

                TRACE("Value=%s\n", debugstr_w(szPath));
                hr = SHILCreateFromPath((LPWSTR)szPath, ppidl, &attributes);
            }
            else if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
            {
                /* unlike SHGetFolderPath, SHGetFolderLocation in shell32
                 * version 6.0 returns E_FAIL for nonexistent paths
                 */
                hr = E_FAIL;
            }
        }
    }
    if(*ppidl)
        hr = NOERROR;

    TRACE("-- (new pidl %p)\n",*ppidl);
    return hr;
}

