/*
 *  KernelEx
 *  Copyright (C) 2010, Tihiy, Xeno86
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
#include "thuni_layer.h"

/* returns TRUE if hwnd is a parent of hwndNewParent */
static BOOL WINAPI TestChild(HWND hwnd, HWND hwndNewParent)
{
	BOOL ret = FALSE;
	PWND pwnd, pwndT;
	GrabWin16Lock();
	pwnd = HWNDtoPWND(hwnd);
	pwndT = HWNDtoPWND(hwndNewParent);
	if ( pwnd && pwndT )
	{
		for ( ; pwndT != NULL; pwndT = (PWND)REBASEUSER(pwndT->spwndParent))
		{
			if (pwnd == pwndT)
			{
				ret = TRUE;
				break;
			}
		}
	}
	ReleaseWin16Lock();	
	return ret;
}

/* MAKE_EXPORT SetParent_fix=SetParent */
__declspec(naked)
HWND WINAPI SetParent_fix(HWND hWndChild, HWND hWndNewParent)
{
__asm {

	/* forbid changing parent of system windows */
	mov     ecx, [esp+4]     /* hWndChild */
	cmp     ecx, 0x88
	ja      __hwndok
	cmp     ecx, 0x80
	jb      __hwndok

	/* hwnd is system one - disallow */
	push    ERROR_ACCESS_DENIED
	jmp     __error

__hwndok:
	/* test hwnds to avoid circular references */
	mov     eax, [esp+8]     /* hWndNewParent */
	push    eax
	push    ecx
	call    TestChild
	test    eax, eax
	jnz     __childfail

	/* return control to SetParent */
	jmp     dword ptr [SetParent]

__childfail:
	/* circular reference detected - stop! */
	push    ERROR_INVALID_PARAMETER

__error:
	/* return error */
	call    dword ptr [SetLastError]
	xor     eax, eax
	retn    8

	}
}
