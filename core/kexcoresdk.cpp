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

unsigned long kexGetKEXVersion()
{
	return VERSION_CODE;
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

void* kexPIDtoPDB(DWORD pid)
{
	return PIDtoPDB(pid);
}

void* kexTIDtoTDB(DWORD tid)
{
	return TIDtoTDB(tid);
}

void kexGetModuleSettings(const char* module, 
                          char* conf_name, BYTE* ldr_flags)
{
	appsetting as = SettingsDB::instance.get_appsetting(module);
	if (!as.conf)
		strcpy(conf_name, "default");
	else
		strncpy(conf_name, as.conf->get_name(), 256);
	*ldr_flags = as.flags;
	conf_name[255] = '\0';
}

void kexSetModuleSettings(const char* module,
                          const char* conf_name, BYTE ldr_flags)
{
	SettingsDB::instance.write_single(module, conf_name, ldr_flags);
}

void kexFlushAppSettings(void)
{
	SettingsDB::instance.flush_all();
}
