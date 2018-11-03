/*
 *  KernelEx
 *
 *  Copyright 1993 Alexandre Julliard
 *            1997 Alex Korobka
 *  Copyright 2002,2003 Shachar Shemesh 
 *  Copyright 2003 CodeWeavers Inc. (Ulrich Czekalla) 
 *
 *  Copyright (C) 2008, Tihiy
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

#include <malloc.h>
#include <windows.h>
#include "common.h"
#include <usp10.h>

/* MAKE_EXPORT GetGlyphIndicesW_new=GetGlyphIndicesW */
int WINAPI GetGlyphIndicesW_new(
  HDC hdc,       // handle to DC
  LPWSTR lpstr, // string to convert
  int c,         // number of characters in string
  LPWORD pgi,    // array of glyph indices
  DWORD fl       // glyph options
)
{
	HRESULT result;
	SCRIPT_CACHE cache = 0;
	if (!hdc || !pgi || (UINT)lpstr<0xFFFFu || !c) return GDI_ERROR;
	result = ScriptGetCMap(hdc,&cache,lpstr,c,0,pgi);
	if ( !( result == S_OK || result == S_FALSE ) ) return GDI_ERROR;
	if ( fl && result == S_FALSE)
	{		
		WORD* checkglyph = pgi;
		int i;
		SCRIPT_FONTPROPERTIES fpr;
		fpr.cBytes = sizeof(SCRIPT_FONTPROPERTIES);
		if (FAILED(ScriptGetFontProperties(hdc,&cache,&fpr))) return GDI_ERROR;
		for (i = 0; i < c; i++)
		{
			if (*checkglyph == fpr.wgDefault) *checkglyph = 0xFFFF;
			checkglyph++;
		}
	}
	ScriptFreeCache(&cache);
	return c;
}

static int WINAPI GdiGetCodePage( HDC hdc )
{
    UINT cp = CP_ACP;
    CHARSETINFO csi;
    int charset = GetTextCharset(hdc);

    /* Hmm, nicely designed api this one! */
    if(TranslateCharsetInfo((DWORD*)charset, &csi, TCI_SRCCHARSET))
        cp = csi.ciACP;
    else
    {
        switch(charset)
        {
        case OEM_CHARSET:
            cp = GetOEMCP();
            break;
        case DEFAULT_CHARSET:
            cp = GetACP();          
            break;
        }
    }
   return cp;
}

/* MAKE_EXPORT GetGlyphIndicesA_new=GetGlyphIndicesA */
int WINAPI GetGlyphIndicesA_new(
  HDC hdc,       // handle to DC
  LPCSTR lpstr, // string to convert
  int c,         // number of characters in string
  LPWORD pgi,    // array of glyph indices
  DWORD fl       // glyph options
)
{
	int result;
	LPWSTR lpstrwide;
	if (!hdc || !pgi || (UINT)lpstr<0xFFFF || c<=0) return GDI_ERROR;	
	lpstrwide = (LPWSTR)alloca(c*sizeof(WCHAR));
	if (MultiByteToWideChar(GdiGetCodePage(hdc),0,lpstr,c,lpstrwide,c))
		result = GetGlyphIndicesW_new(hdc,lpstrwide,c,pgi,fl);
	else
		result = GDI_ERROR;
	return result;
}

/* MAKE_EXPORT GetTextExtentExPointI_new=GetTextExtentExPointI */
BOOL WINAPI GetTextExtentExPointI_new(
  HDC hdc,         // handle to DC
  LPWORD pgiIn,    // array of glyph indices
  int cgi,         // number of glyphs in array
  int nMaxExtent,  // maximum width of formatted string
  LPINT lpnFit,    // maximum number of characters
  LPINT alpDx,     // array of partial string widths
  LPSIZE lpSize    // string dimensions
)
{
	SCRIPT_CACHE cache = 0;
	ABC abc;
	WORD* glyph = pgiIn;
	int* dxs = alpDx;
	int i;
	int sum = 0;	
	int glyphwidth;
	BOOL unfit = FALSE;
	
	if ( !hdc || !pgiIn || cgi<=0 || !lpSize)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	//in UberKern, ScriptPlace was used. However, it's too costly...
	//so let's compute the info ourselves
	for (i = 0; i < cgi; i++)
	{
		if ( ScriptGetGlyphABCWidth(hdc,&cache,*glyph,&abc) != S_OK ) break;
		glyphwidth = abc.abcA + abc.abcB + abc.abcC;
		sum += glyphwidth;		
		if ( !unfit )
		{
			unfit = ( sum > nMaxExtent );
			if (alpDx) {*dxs = sum; dxs++;}
			if (unfit && lpnFit) *lpnFit = i+1; //test test!
		}
		glyph++;
	}
	lpSize->cx = sum;	
	ScriptCacheGetHeight(hdc,&cache,&lpSize->cy);
	ScriptFreeCache(&cache);
	return TRUE;
}

/* MAKE_EXPORT GetTextExtentPointI_new=GetTextExtentPointI */
BOOL WINAPI GetTextExtentPointI_new(
  HDC hdc,           // handle to DC
  LPWORD pgiIn,      // glyph indices
  int cgi,           // number of indices in array
  LPSIZE lpSize      // string size  
)
{
	return GetTextExtentExPointI_new(hdc,pgiIn,cgi,32768,0,0,lpSize);
}

/* MAKE_EXPORT GetCharWidthI_new=GetCharWidthI */
BOOL WINAPI GetCharWidthI_new(
  HDC hdc,         // handle to DC
  UINT giFirst,    // first glyph index in range
  UINT cgi,        // number of glyph indices in range
  WORD* pgi,      // array of glyph indices
  INT* lpBuffer   // buffer for widths
)
{
	SCRIPT_CACHE cache = 0;
	ABC abc;
	WORD glyph;
	
	if ( !hdc || !lpBuffer || cgi<=0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	if ( !pgi ) //cgi glyphs starting giFirst
	{
		for ( glyph = giFirst; glyph < giFirst+cgi; glyph++)
		{
			ScriptGetGlyphABCWidth(hdc,&cache,glyph,&abc);
			*lpBuffer = abc.abcA + abc.abcB + abc.abcC;
			lpBuffer++;
		}
	}
	else
	{
		for ( glyph = 0; glyph < cgi; glyph++)
		{
			ScriptGetGlyphABCWidth(hdc,&cache,*pgi,&abc);
			*lpBuffer = abc.abcA + abc.abcB + abc.abcC;
			pgi++;
			lpBuffer++;
		}		
	}
	ScriptFreeCache(&cache);
	return TRUE;
}

/* MAKE_EXPORT GetGlyphOutlineW_new=GetGlyphOutlineW */
DWORD WINAPI GetGlyphOutlineW_new(
  HDC hdc,             // handle to DC
  UINT uChar,          // character to query
  UINT uFormat,        // data format
  LPGLYPHMETRICS lpgm, // glyph metrics
  DWORD cbBuffer,      // size of data buffer
  LPVOID lpvBuffer,    // data buffer
  CONST MAT2 *lpmat2   // transformation matrix
)
{
	UINT glyph = 0;
	//GetGlyphOutlineW must receive same fix GetGlyphOutlineA received since apilibs interlinking changed
	MAT2 matr;
	if ( lpmat2 ) 
	{
		memcpy( &matr, lpmat2, sizeof(MAT2) );
		lpmat2 = &matr;
	}
	if (uFormat & GGO_GLYPH_INDEX)
		return GetGlyphOutlineA( hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2 );
	GetGlyphIndicesW_new( hdc, (LPWSTR)&uChar, 1, (LPWORD)&glyph, 0 );
	return GetGlyphOutlineA( hdc, glyph, uFormat | GGO_GLYPH_INDEX, lpgm, cbBuffer, lpvBuffer, lpmat2 );
}
