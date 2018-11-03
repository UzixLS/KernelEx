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

#include <windows.h>

static const char c_szUnicows[]="unicows.dll";
static const char c_szUnicowsProp[]="GodotMemoryBlock";

static int WINAPI GetCPFromLocale(LCID Locale)
{
	int cp;	
	Locale = Locale & 0xFFFF;			//we can assume that HKL == LCID in 9x... i hope :)
	if (GetLocaleInfoA(Locale,LOCALE_IDEFAULTANSICODEPAGE | LOCALE_RETURN_NUMBER,(LPSTR)&cp,sizeof(int)))
		return cp;
	else
		return CP_ACP;
}

/* MAKE_EXPORT ToUnicodeEx_new=ToUnicodeEx */
int WINAPI ToUnicodeEx_new(      
    UINT wVirtKey,
    UINT wScanCode,
    const PBYTE lpKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags,
    HKL dwhkl
)
{
	int result;
	WORD asciibuf[4];
	char ansibuf[4];
	if (!lpKeyState || !pwszBuff)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	result = ToAsciiEx(wVirtKey,wScanCode,lpKeyState,asciibuf,wFlags,dwhkl);
	if (result && cchBuff)	
	{
		int i;
		for (i = 0; i < result; i++) ansibuf[i]=(char)asciibuf[i];
		MultiByteToWideChar(GetCPFromLocale((LCID)dwhkl),0,ansibuf,result,pwszBuff,cchBuff);
	}
	return result;
}

/* MAKE_EXPORT ToUnicode_new=ToUnicode */
int WINAPI ToUnicode_new(      
    UINT wVirtKey,
    UINT wScanCode,
    const PBYTE lpKeyState,
    LPWSTR pwszBuff,
    int cchBuff,
    UINT wFlags
)
{	
	return ToUnicodeEx_new(wVirtKey,wScanCode,lpKeyState,pwszBuff,cchBuff,wFlags,GetKeyboardLayout(GetCurrentThreadId()));
}

/* MAKE_EXPORT GetLastInputInfo_new=GetLastInputInfo */
BOOL WINAPI GetLastInputInfo_new(
  PLASTINPUTINFO plii   // last input event
)
{
	if ( !plii ) return FALSE;
	plii->dwTime = GetTickCount() - 123;
	return TRUE;
}

/* MAKE_EXPORT GetMessageW_new=GetMessageW */
BOOL WINAPI GetMessageW_new(
  LPMSG lpMsg,         // message information
  HWND hWnd,           // handle to window
  UINT wMsgFilterMin,  // first message
  UINT wMsgFilterMax   // last message
)
{
	BOOL result;
	/* HACKHACK ALERT: don't bother calling MSLU. We know how it works already. Win speed instead! */	
	result = GetMessageA(lpMsg,hWnd,wMsgFilterMin,wMsgFilterMax);
	if (lpMsg && lpMsg->message == WM_CHAR && lpMsg->wParam > 0x7Fu && lpMsg->wParam <= 0xFFu && IsWindow(lpMsg->hwnd) && GetPropA(lpMsg->hwnd,c_szUnicowsProp))
	{
		WCHAR unichar;
		char ansichar = lpMsg->wParam;
		MultiByteToWideChar(GetCPFromLocale((LCID)GetKeyboardLayout(GetCurrentThreadId())),0,&ansichar,1,&unichar,1);
		lpMsg->wParam = (WORD) unichar;
	}
	return result;
}

/* This fix is mainly for Winamp. Proper fix would be to hook SetWindowLongA too, but it's too hard and still, 
   MSLU would fail to deliver right ANSI functions. Forget it!
*/

/* MAKE_EXPORT GetWindowLongA_new=GetWindowLongA */
LONG WINAPI GetWindowLongA_new(HWND hWnd, int nIndex)
{
	LONG ret = GetWindowLongA( hWnd, nIndex );
	if ( nIndex == GWL_WNDPROC && ret & 0x80000000 && GetPropA(hWnd, c_szUnicowsProp) )
	{
		typedef HRESULT (WINAPI *GWLU) (HWND hWnd, int nIndex);
		GWLU GetWindowLongAU = (GWLU)GetProcAddress( GetModuleHandleA(c_szUnicows), "GetWindowLongA" );
		if ( GetWindowLongAU )
		{
			LONG retU = GetWindowLongAU( hWnd, nIndex );
			if ( retU ) return retU;
		}
	}
	return ret;
}
