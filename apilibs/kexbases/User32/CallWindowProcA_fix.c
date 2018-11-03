/*
 *  KernelEx
 *  Copyright (C) 2008, Xeno86
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

__declspec(naked)
/* MAKE_EXPORT CallWindowProcA_fix=CallWindowProcA */
LRESULT WINAPI CallWindowProcA_fix(WNDPROC lpPrevWndFunc, HWND hWnd, 
		UINT Msg, WPARAM wParam, LPARAM lParam)
{
	static const char unicows_str[] = "unicows.dll";
	static const char callwindowproca_str[] = "CallWindowProcA";
	
	/* We shouldn't write it in C because some weird programs depend 
	 * on CallWindowProc calling function directly!
	 */
__asm {
	mov eax, [esp+4] ;lpPrevWndFunc
	and eax, 0x7FFFFFF0
	cmp eax, 0x7FFFFFF0 
	je UNI
	jmp dword ptr [CallWindowProcA]
UNI:
	push offset unicows_str
	call dword ptr [GetModuleHandleA]
	push offset callwindowproca_str
	push eax
	call dword ptr [GetProcAddress]	
	jmp eax
	}
}
