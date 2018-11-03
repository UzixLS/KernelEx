/*
 *  KernelEx
 *
 *  Copyright (C) 2010, Xeno86
 *  Copyright (C) 2008, Tihiy
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
#include "kstructs.h"

/* MAKE_EXPORT DeleteCriticalSection_fix=DeleteCriticalSection */
void WINAPI DeleteCriticalSection_fix(PCRITICAL_SECTION lpCriticalSection)
{
	//make DeleteCriticalSection not fail on deleted section
	byte* deleted = (byte*) lpCriticalSection;
	if (*deleted != 0) DeleteCriticalSection(lpCriticalSection);
}

/*
 * Terminate process if EnterCriticalSection is called on orphaned CS
 * in DLL_PROCESS_DETACH - Vista behaviour.
 * refer: http://blogs.msdn.com/b/oldnewthing/archive/2010/01/22/9951750.aspx
 */

static void orphaned_cs_handler();

__declspec(naked)
/* MAKE_EXPORT EnterCriticalSection_fix=EnterCriticalSection */
void WINAPI EnterCriticalSection_fix(LPCRITICAL_SECTION lpCriticalSection)
{
__asm {
	mov eax, [esp+4]
	cmp byte ptr [eax], WIN98_K32OBJ_CRITICAL_SECTION
	jnz __error
	jmp dword ptr [EnterCriticalSection]
__error:
	jmp orphaned_cs_handler
	}
}

static
void orphaned_cs_handler()
{
	PDB98* pdb = get_pdb();
	DWORD want_flags = fSendDllNotifications | fTerminating;
	//if process is in dll detach - terminate silently
	if ((pdb->Flags & want_flags) == want_flags)
	{
		SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
	}
	//throw exception
	RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
}
