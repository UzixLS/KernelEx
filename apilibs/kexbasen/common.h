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

#ifndef __COMMON_H
#define __COMMON_H

#include <windows.h>
#include <malloc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int __stdcall STUB(void);
typedef int __stdcall FWDPROC(void);

extern int acp_mcs;
BOOL common_init(void);
int WINAPI CommonUnimpStub(void);
char* file_fixWprefix(char* in);
void fatal_error(const char* msg);

#ifdef __cplusplus
};
#endif

#ifdef __GNUC__
#define UNIMPL_FUNC(name,params) \
	__asm__( ".text\n" \
			 ".globl _" #name "_stub@0\n" \
			 "_" #name "_stub@0:\n\t" \
			 "xor %eax, %eax\n\t" \
			 "movb $" #params ", %cl\n\t" \
			 "jmp _CommonUnimpStub@0\n\t" \
	)
#else
#define UNIMPL_FUNC(name,params) \
	int __declspec(naked) __stdcall name##_stub() \
	{ \
		__asm xor eax,eax \
		__asm mov cl, params \
		__asm jmp CommonUnimpStub \
	}
#endif

#define AtoW(str, bsize) MultiByteToWideChar(CP_ACP, 0, str##A, -1, str##W, bsize)
#define WtoA(str, bsize) WideCharToMultiByte(CP_ACP, 0, str##W, -1, str##A, bsize, NULL, NULL)

#define _ALLOC_WtoA(str) \
	const WCHAR* p; \
	int c; \
	for (p = str##W ; *p ; p++); \
	c = p - str##W + 1; \
	c *= acp_mcs; \
	str##A = (char*) alloca(c); \
	WtoA(str, c)
	

#define ALLOC_WtoA(str) \
	char* str##A; \
	if (str##W) \
	{ \
		_ALLOC_WtoA(str); \
	} \
	else str##A = NULL

#define ALLOC_A(str, cnt) \
	char* str##A = str##W ? ((char*) alloca(cnt)) : NULL

#define ABUFtoW(str, inbsize, outbsize) \
	MultiByteToWideChar(CP_ACP, 0, str##A, inbsize, str##W, outbsize)

#define file_GetCP() \
	UINT file_CP = AreFileApisANSI() ? CP_ACP : CP_OEMCP
#define file_AtoW(str, bsize) MultiByteToWideChar(file_CP, 0, str##A, -1, str##W, bsize)
#define file_WtoA(str, bsize) WideCharToMultiByte(file_CP, 0, str##W, -1, str##A, bsize, "_", NULL); \
	str##A = file_fixWprefix(str##A)

#define file_ALLOC_WtoA(str) \
	char* str##A; \
	if (str##W) \
	{ \
		const WCHAR* p; \
		int c; \
		for (p = str##W ; *p ; p++); \
		c = p - str##W + 1; \
		c *= acp_mcs; \
		str##A = (char*) alloca(c); \
		file_WtoA(str, c); \
	} \
	else str##A = NULL

#define file_ABUFtoW(str, cntsrc, bsize) \
	MultiByteToWideChar(file_CP, 0, str##A, cntsrc, str##W, bsize)

#endif
