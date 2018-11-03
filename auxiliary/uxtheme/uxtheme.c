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

#include <windows.h>

int WINAPI CommonUnimpStub(void);

#ifdef __GNUC__
#define UNIMPL_FUNC(name,params) \
	__asm__( ".text\n" \
			 ".globl _" #name "@0\n" \
			 "_" #name "_new@0:\n\t" \
			 "xor %eax, %eax\n\t" \
			 "movb $" #params ", %cl\n\t" \
			 "jmp _CommonUnimpStub@0\n\t" \
	)
#else
#define UNIMPL_FUNC(name,params) \
	int __declspec(naked) __stdcall name() \
	{ \
		__asm xor eax,eax \
		__asm mov cl, params \
		__asm jmp CommonUnimpStub \
	}
#endif

BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
		OSVERSIONINFO osv;
		osv.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&osv);
		if (osv.dwMajorVersion < 5 || (osv.dwMajorVersion == 5 && osv.dwMinorVersion == 0))
			return FALSE;
		DisableThreadLibraryCalls(hinstDLL);
    }
    return TRUE;
}

UNIMPL_FUNC(CloseThemeData, 1);
UNIMPL_FUNC(DrawThemeBackground, 6);
UNIMPL_FUNC(DrawThemeBackgroundEx, 6);
UNIMPL_FUNC(DrawThemeEdge, 8);
UNIMPL_FUNC(DrawThemeIcon, 7);
UNIMPL_FUNC(DrawThemeParentBackground, 3);
UNIMPL_FUNC(DrawThemeText, 9);
UNIMPL_FUNC(EnableThemeDialogTexture, 2);
UNIMPL_FUNC(EnableTheming, 1);
UNIMPL_FUNC(GetCurrentThemeName, 6);
UNIMPL_FUNC(GetThemeAppProperties, 0);
UNIMPL_FUNC(GetThemeBackgroundContentRect, 6);
UNIMPL_FUNC(GetThemeBackgroundExtent, 6);
UNIMPL_FUNC(GetThemeBackgroundRegion, 6);
UNIMPL_FUNC(GetThemeBool, 5);
UNIMPL_FUNC(GetThemeColor, 5);
UNIMPL_FUNC(GetThemeDocumentationProperty, 4);
UNIMPL_FUNC(GetThemeEnumValue, 5);
UNIMPL_FUNC(GetThemeFilename, 6);
UNIMPL_FUNC(GetThemeFont, 6);
UNIMPL_FUNC(GetThemeInt, 5);
UNIMPL_FUNC(GetThemeIntList, 5);
UNIMPL_FUNC(GetThemeMargins, 7);
UNIMPL_FUNC(GetThemeMetric, 6);
UNIMPL_FUNC(GetThemePartSize, 7);
UNIMPL_FUNC(GetThemePosition, 5);
UNIMPL_FUNC(GetThemePropertyOrigin, 5);
UNIMPL_FUNC(GetThemeRect, 5);
UNIMPL_FUNC(GetThemeString, 6);
UNIMPL_FUNC(GetThemeSysBool, 2);
UNIMPL_FUNC(GetThemeSysColor, 2);
UNIMPL_FUNC(GetThemeSysColorBrush, 2);
UNIMPL_FUNC(GetThemeSysFont, 3);
UNIMPL_FUNC(GetThemeSysInt, 3);
UNIMPL_FUNC(GetThemeSysSize, 2);
UNIMPL_FUNC(GetThemeSysString, 4);
UNIMPL_FUNC(GetThemeTextExtent, 9);
UNIMPL_FUNC(GetThemeTextMetrics, 5);
UNIMPL_FUNC(GetWindowTheme, 1);
UNIMPL_FUNC(HitTestThemeBackground, 10);
UNIMPL_FUNC(IsAppThemed, 0);
UNIMPL_FUNC(IsThemeActive, 0);
UNIMPL_FUNC(IsThemeBackgroundPartiallyTransparent, 3);
UNIMPL_FUNC(IsThemeDialogTextureEnabled, 1);
UNIMPL_FUNC(IsThemePartDefined, 3);
UNIMPL_FUNC(OpenThemeData, 2);
UNIMPL_FUNC(SetThemeAppProperties, 1);
UNIMPL_FUNC(SetWindowTheme, 3);
