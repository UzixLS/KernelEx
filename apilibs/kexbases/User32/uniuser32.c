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
#include "_user32_apilist.h"
#include <commctrl.h>

/* MAKE_EXPORT DrawCaptionTempW_new=DrawCaptionTempW */
BOOL WINAPI DrawCaptionTempW_new(HWND hwnd, HDC hdc, const RECT *rect, HFONT hFont, HICON hIcon, LPCWSTR strW, UINT uFlags)
{
	if (!(uFlags & DC_TEXT) || !strW)
		return DrawCaptionTempA_pfn(hwnd, hdc, rect, hFont, hIcon, (LPCSTR)strW, uFlags);
	else
	{
		ALLOC_WtoA(str);
		return DrawCaptionTempA_pfn(hwnd, hdc, rect, hFont, hIcon, strA, uFlags);
	}
}

/* MAKE_EXPORT LoadStringW_new=LoadStringW */
int WINAPI LoadStringW_new(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBufferMax)
{
	int len;
	LPWSTR wstr;

	len = 0;
	if (lpBuffer)
	{
		// find stringtable resource
		HRSRC res = FindResourceA(hInstance, MAKEINTRESOURCE((uID >> 4) + 1), RT_STRING);
		if (res)
		{
			// load stringtable resource
			HGLOBAL data = LoadResource(hInstance, res);
			if (data)
			{
				// find string in stringtable
				uID &= 0xf;
				do
				{
					len = *(WORD*)data;
					wstr = (LPWSTR)data + 1;
					data = (HGLOBAL)(wstr + len);
				}
				while (uID--);
				// if nBufferMax is zero, then lpBuffer receives pointer to the resource itself
				if (!nBufferMax)
				{
					*(LPWSTR*)lpBuffer = wstr;
					return len;
				}
				if (len > nBufferMax - 1)
				{
					len = nBufferMax - 1;
				}
				memcpy(lpBuffer, wstr, len * sizeof(WCHAR));
			}
		}
		lpBuffer[len] = 0;
	}
	return len;
}

/* MAKE_EXPORT RealGetWindowClassW_new=RealGetWindowClassW */
UINT WINAPI RealGetWindowClassW_new(HWND  hwnd, LPWSTR pszTypeW, UINT cchType)
{
	UINT ret;
	ALLOC_A(pszType, cchType * acp_mcs);
	ret = RealGetWindowClass(hwnd, pszTypeA, cchType * acp_mcs);
	if (ret)
		ret = ABUFtoW(pszType, ret, cchType);
	return ret;
}
