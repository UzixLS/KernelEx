/*
 *  KernelEx
 *  Copyright (C) 2008-2009, Xeno86
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
#include "version.h"
#include <stdarg.h>
#include "kexcoresdk.h"
#include "resolver.h"
#include "internals.h"
#include "SettingsDB.h"
#include "debug.h"
#undef MAKE_PS_TAG
#include "ProcessStorage.h"

unsigned long kexGetKEXVersion()
{
	return VERSION_CODE;
}

int kexIsDebugCore()
{
#ifdef _DEBUG
	return 1;
#else
	return 0;
#endif
}

void kexDebugPrint(const char* format, ...)
{
#ifdef _DEBUG
	va_list argp;
	va_start(argp, format);
	dbgvprintf(format, argp);
	va_end(argp);
#endif
}

DWORD kexGetVersion()
{
	return GetVersion();
}

PROC kexGetProcAddress(HMODULE hModule, PCSTR lpProcName)
{
	return iGetProcAddress(hModule, lpProcName);
}

HANDLE kexOpenThread(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwThreadId)
{
	return _OpenThread(dwDesiredAccess, bInheritHandle, dwThreadId);
}

BOOL kexAreExtensionsEnabled()
{
	return are_extensions_enabled();
}

void kexGetModuleSettings(const char* module, 
                          char* conf_name, DWORD* mod_flags)
{
	appsetting as = SettingsDB::instance.get_appsetting(module);
	if (!as.conf)
	{
		conf_name[0] = '\0';
	}
	else
	{
		strncpy(conf_name, as.conf->get_name(), 256);
		conf_name[255] = '\0';
	}

	DWORD flags = 0;
	if (as.flags & LDR_KEX_DISABLE) flags |= KRF_KEX_DISABLE;
	if (as.flags & LDR_OVERRIDE_PROC_MOD) flags |= KRF_OVERRIDE_PROC_MOD;
	if (as.flags & LDR_LOG_APIS) flags |= KRF_LOG_APIS;
	if (as.flags & LDR_NO_INHERIT) flags |= KRF_NO_INHERIT;
	if (as.flags & LDR_VALID_FLAG) flags |= KRF_VALID_FLAG;
	*mod_flags = flags;
}

void kexSetModuleSettings(const char* module,
                          const char* conf_name, DWORD mod_flags)
{
	BYTE flags = 0;
	if (mod_flags & KRF_KEX_DISABLE) flags |= LDR_KEX_DISABLE;
	if (mod_flags & KRF_OVERRIDE_PROC_MOD) flags |= LDR_OVERRIDE_PROC_MOD;
	if (mod_flags & KRF_LOG_APIS) flags |= LDR_LOG_APIS;
	if (mod_flags & KRF_NO_INHERIT) flags |= LDR_NO_INHERIT;
	SettingsDB::instance.write_single(module, conf_name, flags);
}

void kexResetModuleSettings(const char* module)
{
	SettingsDB::instance.reset_single(module);
}

void kexFlushAppSettings(void)
{
	SettingsDB::instance.flush_all();
}

int kexPsAllocIndex(void)
{
	return ProcessStorage::allocate();
}

void* kexPsGetValue(int index)
{
	return ProcessStorage::get_instance()->get(index);
}

int kexPsSetValue(int index, void* value)
{
	return ProcessStorage::get_instance()->set(index, value);
}

void* kexGetK32Lock()
{
	return krnl32lock;
}
