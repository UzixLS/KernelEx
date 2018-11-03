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
//#include <usp10.h>

typedef void *SCRIPT_CACHE;
typedef struct {
  int   cBytes; 
  WORD  wgBlank; 
  WORD  wgDefault; 
  WORD  wgInvalid; 
  WORD  wgKashida; 
  int   iKashidaWidth; 
} SCRIPT_FONTPROPERTIES;


typedef HRESULT (WINAPI *PFNSCRIPTGETCMAP) (
	  HDC hdc, 
	  SCRIPT_CACHE *psc, 
	  const WCHAR *pwcInChars, 
	  int cChars, 
	  DWORD dwFlags, 
	  WORD *pwOutGlyphs);
typedef HRESULT (WINAPI *PFNSFC) (SCRIPT_CACHE *psc);
typedef HRESULT (WINAPI *PFNCGH) ( HDC hdc, SCRIPT_CACHE *psc, long *tmHeight );
typedef HRESULT (WINAPI *PFNGFP) ( HDC hdc, SCRIPT_CACHE *psc, SCRIPT_FONTPROPERTIES *sfp );
typedef HRESULT (WINAPI *PFNGETGABCWIDTH) ( HDC hdc, SCRIPT_CACHE *psc, WORD wGlyph, ABC *pABC);

static HRESULT WINAPI ScriptGetCMap_dld(HDC hdc, SCRIPT_CACHE *psc, const WCHAR *pwcInChars, int cChars, DWORD dwFlags, WORD *pwOutGlyphs);
static HRESULT WINAPI ScriptFreeCache_dld(SCRIPT_CACHE *psc);
static HRESULT WINAPI ScriptCacheGetHeight_dld(HDC hdc, SCRIPT_CACHE *psc, long *tmHeight);
static HRESULT WINAPI ScriptGetFontProperties_dld(HDC hdc, SCRIPT_CACHE *psc, SCRIPT_FONTPROPERTIES *sfp);

static PFNSCRIPTGETCMAP ScriptGetCMap_pfn = ScriptGetCMap_dld;
static PFNSFC ScriptFreeCache_pfn = ScriptFreeCache_dld;
static PFNCGH ScriptCacheGetHeight_pfn = ScriptCacheGetHeight_dld;
static PFNGFP ScriptGetFontProperties_pfn = ScriptGetFontProperties_dld;
static PFNGETGABCWIDTH ScriptGetGlyphABCWidth_pfn = NULL;

/* delay-loaded Uniscribe start */
static const char c_szUsp10[]="usp10.dll";

static FARPROC WINAPI LoadUniscribeProc(LPCSTR proc)
{
	static HMODULE usp10;
	FARPROC ret;
	DWORD lasterr = GetLastError();
	
	if (!usp10)
	{
		usp10 = GetModuleHandleA(c_szUsp10);
		if (!usp10) usp10 = LoadLibraryA(c_szUsp10);
		if (!usp10)
			fatal_error("kexbasen: Failed to load Uniscribe!");
	}
	ret = GetProcAddress(usp10,proc);
	SetLastError(lasterr);
	return ret;
}

static HRESULT WINAPI ScriptGetCMap_dld(
  HDC hdc, 
  SCRIPT_CACHE *psc, 
  const WCHAR *pwcInChars, 
  int cChars, 
  DWORD dwFlags, 
  WORD *pwOutGlyphs
)
{
	PFNSCRIPTGETCMAP pfn;
	pfn = (PFNSCRIPTGETCMAP)LoadUniscribeProc("ScriptGetCMap");
	if ( !pfn ) return E_FAIL;
	ScriptGetCMap_pfn = pfn;
	return pfn (hdc,psc,pwcInChars,cChars,dwFlags,pwOutGlyphs);
}

static HRESULT WINAPI ScriptFreeCache_dld(
  SCRIPT_CACHE *psc 
)
{
	PFNSFC pfn;
	pfn = (PFNSFC)LoadUniscribeProc("ScriptFreeCache");
	if ( !pfn ) return E_FAIL;
	ScriptFreeCache_pfn = pfn;
	return pfn (psc);
}

static HRESULT WINAPI ScriptCacheGetHeight_dld(
  HDC hdc, 
  SCRIPT_CACHE *psc, 
  long *tmHeight 
)
{
	PFNCGH pfn;
	pfn = (PFNCGH)LoadUniscribeProc("ScriptCacheGetHeight");
	if ( !pfn ) return E_FAIL;
	ScriptCacheGetHeight_pfn = pfn;
	return pfn (hdc,psc,tmHeight);
}

static HRESULT WINAPI ScriptGetFontProperties_dld(
  HDC hdc, 
  SCRIPT_CACHE *psc, 
  SCRIPT_FONTPROPERTIES *sfp 
)
{
	PFNGFP pfn;
	pfn = (PFNGFP)LoadUniscribeProc("ScriptGetFontProperties");
	if ( !pfn ) return E_FAIL;
	ScriptGetFontProperties_pfn = pfn;
	return pfn (hdc,psc,sfp);
}

/* delay-loaded Uniscribe END */

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
	result = ScriptGetCMap_pfn(hdc,&cache,lpstr,c,0,pgi);
	if ( !( result == S_OK || result == S_FALSE ) ) return GDI_ERROR;
	if ( fl && result == S_FALSE)
	{		
		WORD* checkglyph = pgi;
		int i;
		SCRIPT_FONTPROPERTIES fpr;
		fpr.cBytes = sizeof(SCRIPT_FONTPROPERTIES);
		if (FAILED(ScriptGetFontProperties_pfn(hdc,&cache,&fpr))) return GDI_ERROR;
		for (i = 0; i < c; i++)
		{
			if (*checkglyph == fpr.wgDefault) *checkglyph = 0xFFFF;
			checkglyph++;
		}
	}
	ScriptFreeCache_pfn(&cache);
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
	
	if (!ScriptGetGlyphABCWidth_pfn) 
		ScriptGetGlyphABCWidth_pfn = (PFNGETGABCWIDTH)LoadUniscribeProc("ScriptGetGlyphABCWidth");
	if ( !hdc || !pgiIn || cgi<=0 || !lpSize || !ScriptGetGlyphABCWidth_pfn)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	//in UberKern, ScriptPlace was used. However, it's too costly...
	//so let's compute the info ourselves
	for (i = 0; i < cgi; i++)
	{
		if ( ScriptGetGlyphABCWidth_pfn(hdc,&cache,*glyph,&abc) != S_OK ) break;
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
	ScriptCacheGetHeight_pfn(hdc,&cache,&lpSize->cy);
	ScriptFreeCache_pfn(&cache);
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
	
	if (!ScriptGetGlyphABCWidth_pfn) 
		ScriptGetGlyphABCWidth_pfn = (PFNGETGABCWIDTH)LoadUniscribeProc("ScriptGetGlyphABCWidth");
	if ( !hdc || !lpBuffer || cgi<=0 || !ScriptGetGlyphABCWidth_pfn)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	if ( !pgi ) //cgi glyphs starting giFirst
	{
		for ( glyph = giFirst; glyph < giFirst+cgi; glyph++)
		{
			ScriptGetGlyphABCWidth_pfn(hdc,&cache,glyph,&abc);
			*lpBuffer = abc.abcA + abc.abcB + abc.abcC;
			lpBuffer++;
		}
	}
	else
	{
		for ( glyph = 0; glyph < cgi; glyph++)
		{
			ScriptGetGlyphABCWidth_pfn(hdc,&cache,*pgi,&abc);
			*lpBuffer = abc.abcA + abc.abcB + abc.abcC;
			pgi++;
			lpBuffer++;
		}		
	}
	ScriptFreeCache_pfn(&cache);
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
	if (uFormat & GGO_GLYPH_INDEX)
		return GetGlyphOutlineA( hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2 );
	GetGlyphIndicesW_new( hdc, (LPWSTR)&uChar, 1, (LPWORD)&glyph, 0 );
	return GetGlyphOutlineA( hdc, glyph, uFormat | GGO_GLYPH_INDEX, lpgm, cbBuffer, lpvBuffer, lpmat2 );
}
