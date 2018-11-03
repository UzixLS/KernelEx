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

#include "common.h"
#include "kexcoresdk.h"
#include "kernel32/_kernel32_apilist.h"
#include "gdi32/_gdi32_apilist.h"
#include "user32/_user32_apilist.h"
#include "advapi32/_advapi32_apilist.h"
#include "comdlg32/_comdlg32_apilist.h"
#include "shell32/_shell32_apilist.h"
#include "version/_version_apilist.h"
//#include "/__apilist.h"

static LONG inited = 0;
static apilib_api_table api_table[8];

static void fill_apitable()
{
	api_table[0] = apitable_kernel32;
	api_table[1] = apitable_gdi32;
	api_table[2] = apitable_user32;
	api_table[3] = apitable_advapi32;
	api_table[4] = apitable_comdlg32;
	api_table[5] = apitable_shell32;
	api_table[6] = apitable_version;
	//last entry is null terminator
}

extern "C"
__declspec(dllexport)
const apilib_api_table* get_api_table()
{
	fill_apitable();
	return api_table;
}

BOOL init_once()
{
	return common_init() && init_kernel32() && init_gdi32() && init_user32() && init_advapi32() && init_comdlg32() && init_shell32() && init_version();
}

BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, BOOL load_static)
{
	switch (reason) 
	{
	case DLL_PROCESS_ATTACH:
//		kexDebugPrint("KernelEx Base Shared library reporting in action!\n");
		DisableThreadLibraryCalls(instance);
		if (InterlockedExchange(&inited, 1) == 0)
		{
			if (!init_once())
				return FALSE;
		}
		break;
	case DLL_PROCESS_DETACH:
//		kexDebugPrint("KernelEx Base Shared library signing off!\n");
		break;
	}
	return TRUE;
}
