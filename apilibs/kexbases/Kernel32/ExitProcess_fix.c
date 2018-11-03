/*
 *  KernelEx
 *  Copyright (C) 2010, Tihiy
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

/* MAKE_EXPORT ExitProcess_fix=ExitProcess */
VOID WINAPI ExitProcess_fix( UINT uExitCode )
{
	PDB98* pdb = get_pdb();
	
	//process is already terminating. we would badly crash 
	if (pdb->Flags & fTerminating)
	{
		//so instead silently fail
		SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS);
		RaiseException(EXCEPTION_ACCESS_VIOLATION, 0, 0, NULL);
	}

	//set calling thread priority to the lowest possible. this way 
	//we greatly improve chance for thread which calls ExitProcess
	//to finish latest and call dll process detach like NT
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);
	ExitProcess(uExitCode);
}
