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

#ifndef __LOADSTUB_H
#define __LOADSTUB_H

#define KEX_SIGNATURE "KrnlEx"
#define KEX_STUB_VER 0

#define JTAB_SIZE 4
#define JTAB_EFO_DYN 0
#define JTAB_EFO_STA 1
#define JTAB_EFN_DYN 2
#define JTAB_EFN_STA 3

#include <pshpack1.h>
typedef struct
{
	char signature[6];   /* "KrnlEx" */
	unsigned short version;
	struct
	{
		WORD opcode; /* 0xFF 0x25 */
		DWORD addr;   /* address to jtab */
		WORD nop;    /* dword align */
	} jmp_stub[JTAB_SIZE];
} KernelEx_codeseg;

typedef struct
{
	char signature[6];   /* "KrnlEx" */
	unsigned short version;
	DWORD jtab[JTAB_SIZE];
} KernelEx_dataseg;
#include <poppack.h>

#endif
