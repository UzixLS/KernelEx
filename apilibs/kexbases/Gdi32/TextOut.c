/*
 *  KernelEx
 *
 *  Copyright 1993 Alexandre Julliard
 *            1997 Alex Korobka
 *  Copyright 2002,2003 Shachar Shemesh 
 *  Copyright 2003 CodeWeavers Inc. (Ulrich Czekalla) 
 *
 *  Copyright (C) 2008, Tihiy
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
#include "auxdecl.h"

#ifndef ETO_PDY
#define ETO_PDY 0x2000
#endif

static void WINAPI MakeDxFromDxDy(const int* lpDx, int* newlpDx, UINT cbCount)
{
	UINT oneDx;
	for (oneDx = 0; oneDx < cbCount; oneDx++)
	{
		*newlpDx = *lpDx;
		newlpDx++;
		lpDx++;
		lpDx++;
	}
	newlpDx--;
	if (*newlpDx == 0) *newlpDx = 16; //TERRIBLE HACK but needed on some fonts/glyphs/etc!
}

/* MAKE_EXPORT ExtTextOutA_new=ExtTextOutA */
BOOL WINAPI ExtTextOutA_new(
  HDC hdc,          // handle to DC
  int X,            // x-coordinate of reference point
  int Y,            // y-coordinate of reference point
  UINT fuOptions,   // text-output options
  CONST RECT* lprc, // optional dimensions
  LPCSTR lpString, // string
  UINT cbCount,     // number of characters in string
  CONST INT* lpDx   // array of spacing values
)
{
	BOOL result;
	int* buffer = 0;
	
	if ((UINT)lpString>0xFFFFu)
	{
		if (cbCount>8192) cbCount = 8192;
		if (fuOptions & ETO_PDY)	//win9x can't understand it and messes up other flags
		{
			fuOptions = fuOptions & ~ETO_PDY; 
			if (lpDx)				//we have to make array which contains dx values only
			{	
				buffer = (int*)HeapAlloc(GetProcessHeap(),0,cbCount*sizeof(int));
				if (!buffer)		//you've managed to ruin everything
				{
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					return FALSE;
				}
				MakeDxFromDxDy(lpDx,buffer, cbCount);
				lpDx = buffer;
			}
		}
	}
	result = ExtTextOutA(hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
	if (buffer) HeapFree (GetProcessHeap(),0,buffer);
	return result;
}


/* MAKE_EXPORT ExtTextOutW_new=ExtTextOutW */
BOOL WINAPI ExtTextOutW_new(
  HDC hdc,          // handle to DC
  int X,            // x-coordinate of reference point
  int Y,            // y-coordinate of reference point
  UINT fuOptions,   // text-output options
  CONST RECT* lprc, // optional dimensions
  LPCWSTR lpString, // string
  UINT cbCount,     // number of characters in string
  CONST INT* lpDx   // array of spacing values
)
{
	BOOL result;
	BOOL optimized = FALSE;
	int* buffer = 0;
	char english[128];
	
	if ((UINT)lpString>0xFFFFu)
	{
		if (cbCount>8192) cbCount = 8192;
		if (fuOptions & ETO_PDY)
		{
			fuOptions = fuOptions & ~ETO_PDY; 
			if (lpDx)
			{	
				buffer = (int*)HeapAlloc(GetProcessHeap(),0,cbCount*sizeof(int));
				if (!buffer)
				{
					SetLastError(ERROR_NOT_ENOUGH_MEMORY);
					return FALSE;
				}
				MakeDxFromDxDy(lpDx,buffer,cbCount);
				lpDx = buffer;
			}
		}
		//now, common case optimization (English short string)
		if (!(fuOptions & ETO_GLYPH_INDEX || cbCount>128 || *lpString>0x7Fu))
		{
			UINT i;
			LPCWSTR uString = lpString;
			for (i = 0; i< cbCount; i++)
			{
				if (*uString>0x7Fu) break; else english[i]=(char)*uString;
				uString++;
			}
			optimized = (i == cbCount);
		}
		else
			optimized = FALSE;		
	}
		
	if (optimized)
		result = ExtTextOutA(hdc,X,Y,fuOptions,lprc,english,cbCount,lpDx);
	else
		result = ExtTextOutW(hdc,X,Y,fuOptions,lprc,lpString,cbCount,lpDx);
		
	if (buffer) HeapFree (GetProcessHeap(),0,buffer);
	return result;
}

/* MAKE_EXPORT PolyTextOutA_new=PolyTextOutA */
BOOL WINAPI PolyTextOutA_new( HDC hdc, const POLYTEXTA *pptxt, INT cStrings )
{
	if (!hdc || !cStrings || !pptxt)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}	
    for (; cStrings>0; cStrings--, pptxt++)
        if (!ExtTextOutA_new (hdc, pptxt->x, pptxt->y, pptxt->uiFlags, &pptxt->rcl, pptxt->lpstr, pptxt->n, pptxt->pdx ))
            return FALSE;
    return TRUE;
}

/* MAKE_EXPORT PolyTextOutW_new=PolyTextOutW */
BOOL WINAPI PolyTextOutW_new( HDC hdc, const POLYTEXTW *pptxt, INT cStrings )
{
	if (!hdc || !cStrings || !pptxt)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
    for (; cStrings>0; cStrings--, pptxt++)
        if (!ExtTextOutW_new (hdc, pptxt->x, pptxt->y, pptxt->uiFlags, &pptxt->rcl, pptxt->lpstr, pptxt->n, pptxt->pdx ))
            return FALSE;
    return TRUE;
}

/* MAKE_EXPORT GetFontUnicodeRanges_new=GetFontUnicodeRanges */
DWORD WINAPI GetFontUnicodeRanges_new(
  HDC hdc,
  LPGLYPHSET lpgs
)
{
	SetLastError(ERROR_NOT_SUPPORTED);
	return 0;
}

/* On 9x fallback to system function */
/* MAKE_EXPORT SetGraphicsMode_NT=SetGraphicsMode */
int WINAPI SetGraphicsMode_NT(
  HDC hdc,    // handle to device context
  int iMode   // graphics mode
)
{
	return GM_COMPATIBLE;
}

/* MAKE_EXPORT SetWorldTransform_9x=SetWorldTransform */
BOOL WINAPI SetWorldTransform_9x(
  HDC hdc,               // handle to device context
  CONST XFORM *lpXform   // transformation data
)
{
	return FALSE;
}

/* MAKE_EXPORT SetWorldTransform_NT=SetWorldTransform */
BOOL WINAPI SetWorldTransform_NT(
  HDC hdc,               // handle to device context
  CONST XFORM *lpXform   // transformation data
)
{
	return TRUE;
}

/* MAKE_EXPORT GetRandomRgn_NT=GetRandomRgn */
int WINAPI GetRandomRgn_NT(
  HDC  hdc,    // handle to DC
  HRGN hrgn,   // handle to region
  INT  iNum    // must be SYSRGN
)
{
	int result = GetRandomRgn(hdc,hrgn,iNum);
	if (result)
	{
		POINT pt;
		GetDCOrgEx(hdc,&pt);
		OffsetRgn(hrgn,pt.x,pt.y);
	}
	return result;
}
