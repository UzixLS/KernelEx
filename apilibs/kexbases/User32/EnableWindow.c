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

/* 
 * Win9x seems to look at only the low 16 bits of bEnable.
 * This causes a problem, for example, if the caller passes
 * a count of items with the intent to enable a window when
 * the count is >0.  That will fail when the count is
 * a multiple of 64K 
 */

/* MAKE_EXPORT EnableWindow_new=EnableWindow */
BOOL WINAPI EnableWindow_new(HWND hWnd, BOOL bEnable)
{
	return EnableWindow(hWnd, bEnable ? TRUE : FALSE);
}
