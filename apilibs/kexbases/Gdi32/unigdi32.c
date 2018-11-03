/*
 *  KernelEx
 *  Copyright (C) 2009-2010, Xeno86
 *  Copyright (C) 2009, Tihiy
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
#include "_gdi32_apilist.h"
#include "../kernel32/_kernel32_apilist.h"

typedef struct
{
	FONTENUMPROCW EnumProcW;
	LPARAM lParam;
} EnumFamilies_t;


//MAKE_EXPORT AddFontResourceExW_new=AddFontResourceExW
INT WINAPI AddFontResourceExW_new(LPCWSTR strW, DWORD fl, PVOID pdv)
{
	file_GetCP();
	file_ALLOC_WtoA(str);
	return AddFontResourceExA_new(strA, fl, pdv);
}

//MAKE_EXPORT AddFontResourceW_new=AddFontResourceW
INT WINAPI AddFontResourceW_new(LPCWSTR strW)
{
	file_GetCP();
	file_ALLOC_WtoA(str);
	return AddFontResourceA(strA);
}

static int CALLBACK EnumFontFamExConv(const LOGFONTA *plfA, 
		const TEXTMETRICA* ptmA, DWORD FontType, LPARAM lParam)
{
	const ENUMLOGFONTEXA* fontA = (const ENUMLOGFONTEXA*) plfA;
	const NEWTEXTMETRICEXA* tmA = (const NEWTEXTMETRICEXA*) ptmA;
	ENUMLOGFONTEXW elfeW;
	NEWTEXTMETRICEXW ntmeW;
	ENUMLOGFONTEXW* fontW = &elfeW;
	NEWTEXTMETRICEXW* tmW = &ntmeW;

	EnumFamilies_t* pef = (EnumFamilies_t*) lParam;

	memcpy(&fontW->elfLogFont, &fontA->elfLogFont, sizeof(LOGFONTA) - LF_FACESIZE);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) fontA->elfLogFont.lfFaceName, -1, 
			fontW->elfLogFont.lfFaceName, LF_FACESIZE);
	fontW->elfLogFont.lfFaceName[LF_FACESIZE - 1] = 0;
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) fontA->elfFullName, -1,
			fontW->elfFullName, LF_FULLFACESIZE);
	fontW->elfFullName[LF_FULLFACESIZE - 1] = 0;
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) fontA->elfStyle, -1,
			fontW->elfStyle, LF_FACESIZE);
	fontW->elfStyle[LF_FACESIZE - 1] = 0;
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) fontA->elfScript, -1,
			fontW->elfScript, LF_FACESIZE);
	fontW->elfScript[LF_FACESIZE - 1] = 0;

	tmW->ntmTm.tmHeight = tmA->ntmTm.tmHeight;
	tmW->ntmTm.tmAscent = tmA->ntmTm.tmAscent;
	tmW->ntmTm.tmDescent = tmA->ntmTm.tmDescent;
	tmW->ntmTm.tmInternalLeading = tmA->ntmTm.tmInternalLeading;
	tmW->ntmTm.tmExternalLeading = tmA->ntmTm.tmExternalLeading;
	tmW->ntmTm.tmAveCharWidth = tmA->ntmTm.tmAveCharWidth;
	tmW->ntmTm.tmMaxCharWidth = tmA->ntmTm.tmMaxCharWidth;
	tmW->ntmTm.tmWeight = tmA->ntmTm.tmWeight;
	tmW->ntmTm.tmOverhang = tmA->ntmTm.tmOverhang;
	tmW->ntmTm.tmDigitizedAspectX = tmA->ntmTm.tmDigitizedAspectX;
	tmW->ntmTm.tmDigitizedAspectY = tmA->ntmTm.tmDigitizedAspectY;
	tmW->ntmTm.tmItalic = tmA->ntmTm.tmItalic;
	tmW->ntmTm.tmUnderlined = tmA->ntmTm.tmUnderlined;
	tmW->ntmTm.tmStruckOut = tmA->ntmTm.tmStruckOut;
	tmW->ntmTm.tmPitchAndFamily = tmA->ntmTm.tmPitchAndFamily;
	tmW->ntmTm.tmCharSet = tmA->ntmTm.tmCharSet;

	//needs review - are this conversions necessary or is it enough to just copy?
#if 1
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) &tmA->ntmTm.tmFirstChar, 1, 
			&tmW->ntmTm.tmFirstChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) &tmA->ntmTm.tmLastChar, 1, 
			&tmW->ntmTm.tmLastChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) &tmA->ntmTm.tmDefaultChar, 1, 
			&tmW->ntmTm.tmDefaultChar, 1);
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR) &tmA->ntmTm.tmBreakChar, 1, 
			&tmW->ntmTm.tmBreakChar, 1);
#else
	tmW->ntmTm.tmFirstChar = tmA->ntmTm.tmFirstChar;
	tmW->ntmTm.tmLastChar = tmA->ntmTm.tmLastChar;
	tmW->ntmTm.tmDefaultChar = tmA->ntmTm.tmDefaultChar;
	tmW->ntmTm.tmBreakChar = tmA->ntmTm.tmBreakChar;
#endif

	if (FontType == TRUETYPE_FONTTYPE)
	{
		tmW->ntmTm.ntmFlags = tmA->ntmTm.ntmFlags;
		tmW->ntmTm.ntmSizeEM = tmA->ntmTm.ntmSizeEM;
		tmW->ntmTm.ntmCellHeight = tmA->ntmTm.ntmCellHeight;
		tmW->ntmTm.ntmAvgWidth = tmA->ntmTm.ntmAvgWidth;
		memset(&tmW->ntmFontSig, 0, sizeof(FONTSIGNATURE));
	}
	else
	{
		memset(&tmW->ntmTm.ntmFlags, 0, sizeof(NEWTEXTMETRICEXW) 
				- FIELD_OFFSET(NEWTEXTMETRICEXW, ntmTm.ntmFlags));
	}

	return pef->EnumProcW((LOGFONTW*) &elfeW, (TEXTMETRICW*) &ntmeW, FontType, pef->lParam);
}

/*  Surprise surprise!
 *  logfont* is optional in EnumFontFamiliesEx on NT
 *  and means - all fonts, all charsets
 */

/* MAKE_EXPORT EnumFontFamiliesExA_new=EnumFontFamiliesExA */
int WINAPI EnumFontFamiliesExA_new(HDC hdc, LPLOGFONTA pLogfontA, 
		FONTENUMPROCA pEnumFontFamExProc, LPARAM lParam, DWORD dwFlags)
{
	LOGFONTA logfont;
	if (!pLogfontA) 
	{
		memset(&logfont, 0, sizeof(logfont));
		logfont.lfCharSet = DEFAULT_CHARSET;
		pLogfontA = &logfont;
	}
	return EnumFontFamiliesExA(hdc, pLogfontA, pEnumFontFamExProc, lParam, dwFlags);
}

/* MAKE_EXPORT EnumFontFamiliesExW_new=EnumFontFamiliesExW */
int WINAPI EnumFontFamiliesExW_new(HDC hdc, LPLOGFONTW pLogfontW, 
		FONTENUMPROCW pEnumFontFamExProc, LPARAM lParam, DWORD dwFlags)
{
	EnumFamilies_t ef;
	LOGFONTA logfont;
	if (pLogfontW)
	{		
		memcpy(&logfont, pLogfontW, sizeof(LOGFONTA) - LF_FACESIZE);
		WideCharToMultiByte(CP_ACP, 0, pLogfontW->lfFaceName, -1, logfont.lfFaceName, 
				LF_FACESIZE, NULL, NULL);
		logfont.lfFaceName[LF_FACESIZE - 1] = '\0';
	}
	else
	{
		memset(&logfont, 0, sizeof(logfont));
		logfont.lfCharSet = DEFAULT_CHARSET;
	}
	ef.EnumProcW = pEnumFontFamExProc;
	ef.lParam = lParam;
	return EnumFontFamiliesExA(hdc, &logfont, EnumFontFamExConv, (LPARAM) &ef, dwFlags);
}

/* MAKE_EXPORT EnumFontFamiliesW_new=EnumFontFamiliesW */
int WINAPI EnumFontFamiliesW_new(HDC hdc, LPCWSTR lpszFamily, 
		FONTENUMPROCW lpEnumFontFamProc, LPARAM lParam)
{
	LOGFONTW lf;
	LOGFONTW* plf = NULL;

	if (lpszFamily)
	{
		if (!*lpszFamily) 
			return 1;
		lstrcpynW_new(lf.lfFaceName, lpszFamily, LF_FACESIZE);
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfPitchAndFamily = 0;
		plf = &lf;
	}

	return EnumFontFamiliesExW_new(hdc, plf, lpEnumFontFamProc, lParam, 0);
}

//MAKE_EXPORT RemoveFontResourceExW_new=RemoveFontResourceExW
BOOL WINAPI RemoveFontResourceExW_new(LPCWSTR strW, DWORD fl, PVOID pdv)
{
	file_GetCP();
	file_ALLOC_WtoA(str);
	return RemoveFontResourceExA_new(strA, fl, pdv);
}

//MAKE_EXPORT RemoveFontResourceW_new=RemoveFontResourceW
BOOL WINAPI RemoveFontResourceW_new(LPCWSTR strW)
{
	file_GetCP();
	file_ALLOC_WtoA(str);
	return RemoveFontResourceA(strA);
}

/* MAKE_EXPORT ExtCreatePen_fix=ExtCreatePen */
HPEN WINAPI ExtCreatePen_fix(
  DWORD dwPenStyle,      // pen style
  DWORD dwWidth,         // pen width
  CONST LOGBRUSH *lplb,  // brush attributes
  DWORD dwStyleCount,    // length of custom style array
  CONST DWORD *lpStyle   // custom style array
)
{
	dwPenStyle &= ~PS_USERSTYLE;
	return ExtCreatePen(dwPenStyle,dwWidth,lplb,dwStyleCount,lpStyle);
}

/* MAKE_EXPORT GetObjectW_new=GetObjectW */
int WINAPI GetObjectW_new(
  HGDIOBJ hgdiobj,  // handle to graphics object
  int cbBuffer,     // size of buffer for object information
  LPVOID lpvObject  // buffer for object information
)
{
	int type = GetObjectType_fix(hgdiobj);
	if (type != OBJ_FONT) return GetObjectA(hgdiobj,cbBuffer,lpvObject);
	if (!lpvObject) return sizeof(LOGFONTW);
	LOGFONTA fontA = {0};
	LOGFONTW fontW = {0};
	if (!GetObjectA(hgdiobj,sizeof(LOGFONTA),&fontA)) return 0; //err not font
	memcpy(&fontW,&fontA,FIELD_OFFSET(LOGFONTA,lfFaceName));
	MultiByteToWideChar(CP_ACP,0,fontA.lfFaceName,-1,fontW.lfFaceName,LF_FACESIZE);
	memcpy(lpvObject,&fontW,cbBuffer);
	return cbBuffer;
}
