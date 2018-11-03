/*
 *  KernelEx
 *  Copyright (C) 2009, Xeno86
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

#include <new>
#include <windows.h>
#include <stdio.h>
#include "apilog.h"
#include "debug.h"
#include "internals.h"
#include "DebugWindow.h"

#define APILOG_TLS_INDEX       78

void* get_process_env_data(const char* env, void* (*creator)())
{
	//environment variable: ENV=ProcessID:DATA
	char buf[20];
	DWORD ret;
	DWORD ProcID;
	void* data = NULL;
	
	ret = GetEnvironmentVariable(env, buf, sizeof(buf));
	if (ret == 0 || ret > sizeof(buf) 
			|| sscanf(buf, "%x:%x", &ProcID, &data) != 2
			|| ProcID != GetCurrentProcessId())
	{
		//invalid/missing value - create new data
		data = creator();
		if (data)
		{
			sprintf(buf, "%x:%x", GetCurrentProcessId(), data);
			SetEnvironmentVariable(env, buf);
		}
	}

	return data;
}

void* heap_creator()
{
	return HeapCreate(0, 0, 0);
}

HANDLE get_process_debug_heap()
{
	return get_process_env_data("KEXDBGH", heap_creator);
}

void* tls_creator()
{
	for (int i = 0 ; i < APILOG_TLS_INDEX+1 ; i++)
		TlsAlloc();
	for (int i = 0 ; i < APILOG_TLS_INDEX ; i++)
		TlsFree(i);
	return (void*) APILOG_TLS_INDEX;
}

DWORD get_process_debug_tls()
{
	return (DWORD) get_process_env_data("KEXDBGT", tls_creator);
}

extern "C"
int snprintf(char *buffer, size_t n, const char* format, ...);

DWORD __stdcall log_api(const char* source, const char* target, const char* api_name, DWORD depth, DWORD ret)
{
	DebugWindow* dw = DebugWindow::get();
	if (!dw)
		return 0;
	
	char msg[DEBUGMSG_MAXLEN];

	//fancy call stack depth indicator
	if (depth < DEBUGMSG_MAXLEN / 2)
	{
		for (int i = 0 ; i < depth ; i++)
			msg[i] = 'l';
	}
	else
	{
		msg[0] = 'E';
		msg[1] = 'E';
		depth = 2;
	}

	snprintf(msg + depth, sizeof(msg) - depth, "|%x|%s|%s|%s|%x", 
			GetCurrentThreadId(), source, target, api_name, ret);

	dw->append(msg);

	return ret;
}

ThreadAddrStack::ThreadAddrStack()
{
	pos = 0;
}

void __stdcall ThreadAddrStack::push_ret_addr(DWORD addr)
{
	//TlsGetValue clears last error value so remember & restore it
	DWORD lasterr = GetLastError();
	ThreadAddrStack* tas = (ThreadAddrStack*) TlsGetValue(APILOG_TLS_INDEX);
	SetLastError(lasterr);
	if (!tas)
	{
		void* mem = HeapAlloc(get_process_debug_heap(), 0, sizeof(ThreadAddrStack));
		tas = new (mem) ThreadAddrStack;
		TlsSetValue(APILOG_TLS_INDEX, mem);
	}
	tas->stack[tas->pos++] = addr;
	DBGASSERT(tas->pos < sizeof(tas->stack) / sizeof(tas->stack[0]));
}

DWORD __stdcall ThreadAddrStack::pop_ret_addr()
{
	//TlsGetValue clears last error value so remember & restore it
	DWORD lasterr = GetLastError();
	ThreadAddrStack* tas = (ThreadAddrStack*) TlsGetValue(APILOG_TLS_INDEX);
	SetLastError(lasterr);
	DBGASSERT(tas->pos > 0);
	return tas->stack[--tas->pos];
}

DWORD __stdcall ThreadAddrStack::get_level()
{
	//TlsGetValue clears last error value so remember & restore it
	DWORD lasterr = GetLastError();
	ThreadAddrStack* tas = (ThreadAddrStack*) TlsGetValue(APILOG_TLS_INDEX);
	SetLastError(lasterr);
	return tas->pos;
}

PROC create_log_stub(const char* caller, const char* target, const char* api, PROC orig)
{
	HANDLE heap = get_process_debug_heap();
	char* new_api = (char*) HeapAlloc(heap, 0, strlen(api) + 1);
	strcpy(new_api, api);
	get_process_debug_tls();
	void* mem = HeapAlloc(heap, 0, sizeof(log_stub));
	return (PROC) new (mem) log_stub(caller, 
			target, new_api, (unsigned long) orig, 
			(unsigned long) log_api);
}

PROC create_log_stub(const char* caller, const char* target, WORD ord, PROC orig)
{
	char ord_name[16];
	snprintf(ord_name, sizeof(ord_name), "Ordinal:%d", ord);
	return create_log_stub(caller, target, ord_name, orig);
}
