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
#include "unifwd.h"
#include "kexcoresdk.h"
#include "common.h"

static HMODULE hUnicows;

int unifwd_init(void)
{
	hUnicows = GetModuleHandle("UNICOWS.DLL");
	if (!hUnicows)
		hUnicows = LoadLibrary("UNICOWS.DLL");
	return (hUnicows != NULL);
}

void unicows_fatal_error(const char* func)
{
	char msg[256];
	strcpy(msg, "kexbasen: Failed to get unicows proc addr: ");
	strcat(msg, func);
	fatal_error(msg);
}

unsigned long __stdcall GetUnicowsAddress(const char* name)
{
	if (!hUnicows)
		unifwd_init();
	PROC addr = GetProcAddress(hUnicows, name);
	if (!addr)
		unicows_fatal_error(name);
	return (unsigned long) addr;
}
