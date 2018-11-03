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

/* MapVirtualKey translation types */
#define MAPVK_VK_TO_VSC     0
#define MAPVK_VSC_TO_VK     1
#define MAPVK_VK_TO_CHAR    2
#define MAPVK_VSC_TO_VK_EX  3
#define MAPVK_VK_TO_VSC_EX  4

/* MAKE_EXPORT MapVirtualKeyA_new=MapVirtualKeyA */
UINT WINAPI MapVirtualKeyA_new(UINT uCode, UINT uMapType)
{
	if (uMapType == MAPVK_VK_TO_VSC_EX)
		uMapType = MAPVK_VK_TO_VSC;
	else if (uMapType == MAPVK_VSC_TO_VK_EX)
		uMapType = MAPVK_VSC_TO_VK;
	return MapVirtualKeyA(uCode, uMapType);
}

/* MAKE_EXPORT MapVirtualKeyExA_new=MapVirtualKeyExA */
UINT WINAPI MapVirtualKeyExA_new(UINT uCode, UINT uMapType, HKL dwhkl)
{
	if (uMapType == MAPVK_VK_TO_VSC_EX)
		uMapType = MAPVK_VK_TO_VSC;
	else if (uMapType == MAPVK_VSC_TO_VK_EX)
		uMapType = MAPVK_VSC_TO_VK;	
	return MapVirtualKeyExA(uCode, uMapType, dwhkl);
}
