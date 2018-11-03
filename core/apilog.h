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

#ifndef __APILOG_H
#define __APILOG_H

#include "resolver.h"


class ThreadAddrStack
{
public:
	ThreadAddrStack();
	static void __stdcall push_ret_addr(DWORD addr);
	static DWORD __stdcall pop_ret_addr();
	static DWORD __stdcall get_level();

private:
	int pos;
	DWORD stack[1023];
};

#pragma pack(push,1)

/* Creates a stub for api logging. */
class log_stub
{
public:
	log_stub(const char* source, const char* target, const char* name, 
			unsigned long proc, unsigned long log_fun) 
			: call_orig(proc, true), jmp_logfun(log_fun), 
			tas_store((unsigned long) ThreadAddrStack::push_ret_addr, true),
			tas_restore((unsigned long) ThreadAddrStack::pop_ret_addr, true),
			tas_depth((unsigned long) ThreadAddrStack::get_level, true)
	{
		c_push2 = c_push3 = c_push4 = 0x68;
		v_source = source;
		v_target = target;
		v_name = name;
		c_pusheax1 = c_pusheax2 = c_pusheax3 = c_pusheax4 = 0x50;
		c_popeax4 = 0x58;
		c_pushecx = 0x51;
		c_popecx = 0x59;
	}

private:
	unsigned char c_popeax4;  //caller ret
	unsigned char c_pushecx;
	unsigned char c_pusheax4; //caller ret
	redir_stub tas_store;
	unsigned char c_popecx;
	redir_stub call_orig;
	unsigned char c_pusheax1; //orig ret
	redir_stub tas_depth;
	unsigned char c_pusheax3;  //call stack depth
	unsigned char c_push2;    //api name
	const char* v_name;
	unsigned char c_push3;    //target module
	const char* v_target;
	unsigned char c_push4;    //calling module
	const char* v_source;
	redir_stub tas_restore;
	unsigned char c_pusheax2; //caller return address
	redir_stub jmp_logfun;    //jump to log_fun
};

#pragma pack(pop)

PROC create_log_stub(const char* caller, const char* target, const char* api, PROC orig);
PROC create_log_stub(const char* caller, const char* target, WORD ord, PROC orig);

#endif
