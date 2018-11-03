/*
 *  KernelEx
 *  Copyright (C) 2010, Tihiy
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

#ifndef _CRYPTUI_APILIST_H
#define _CRYPTUI_APILIST_H

#include "kexcoresdk.h"

BOOL init_cryptui();
extern const apilib_api_table apitable_cryptui;

/*** AUTOGENERATED APILIST DECLARATIONS BEGIN ***/
BOOL WINAPI CryptUIDlgViewContext_new(DWORD dwContextType, const void *pvContext, HWND hwnd, LPCWSTR pwszTitle, DWORD dwFlags, void *pvReserved);
/*** AUTOGENERATED APILIST DECLARATIONS END ***/

#endif
