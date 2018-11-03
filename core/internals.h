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

#ifndef __INTERNALS_H
#define __INTERNALS_H

#include "structs.h"
#include "sharedmem.h"
#include "sstring.hpp"
#include "resource.h"

/** MSVC for-loop workaround. */
#ifdef _MSC_VER
#if _MSC_VER < 1201
#define for if (0); else for
#endif
#endif

extern IMTE*** ppmteModTable;
extern HMODULE h_kernel32;
extern CRITICAL_SECTION* krnl32lock;
extern PDB98** pppdbCur;
extern WORD* pimteMax;

extern sstring kernelex_dir;
extern sstring own_path;

void FullCritLock();
void FullCritUnlock();
void ShowError(UINT id, ...);
bool isWinMe();

typedef MODREF* (__stdcall *MRFromHLib_t)(HMODULE);
typedef TDB98* (__stdcall *TIDtoTDB_t)(DWORD);
typedef PDB98* (__stdcall *PIDtoPDB_t)(DWORD);
typedef MODREF* (__stdcall * MRLoadTree_t)(LPCSTR);
typedef BOOL (__stdcall * FreeLibTree_t)(MODREF*);
typedef BOOL (__stdcall * FLoadTreeNotify_t)(MODREF*, BOOL);
typedef VOID (__stdcall * FreeLibRemove_t)(VOID);

extern MRFromHLib_t MRFromHLib;

/** Convert Thread ID into pointer to Thread Database. 
 * @param tid Thread ID.
 * @return Pointer to Thread Database.
 */
extern TIDtoTDB_t TIDtoTDB;

/** Convert Process ID into pointer to Process Database. 
 * @param pid Process ID.
 * @return Pointer to Process Database.
 */
extern PIDtoPDB_t PIDtoPDB;

extern MRLoadTree_t MRLoadTree;
extern FreeLibTree_t FreeLibTree;
extern FLoadTreeNotify_t FLoadTreeNotify;
extern FreeLibRemove_t FreeLibRemove;

MODREF* MRfromCallerAddr(DWORD addr);

#ifdef __cplusplus
extern "C" {
#endif

void __stdcall _EnterSysLevel(CRITICAL_SECTION*);
void __stdcall _LeaveSysLevel(CRITICAL_SECTION*);

ULONG __stdcall VxDCall1(ULONG);
ULONG __stdcall VxDCall2(ULONG, ULONG);
ULONG __stdcall VxDCall3(ULONG, ULONG, ULONG);
ULONG __stdcall VxDCall4(ULONG, ULONG, ULONG, ULONG);
ULONG __stdcall VxDCall5(ULONG, ULONG, ULONG, ULONG, ULONG);
ULONG __stdcall VxDCall6(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
ULONG __stdcall VxDCall7(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
ULONG __stdcall VxDCall8(ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);

#ifdef __cplusplus
};
#endif

#endif
