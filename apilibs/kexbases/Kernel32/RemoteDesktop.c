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

/********************************/
/* Remote Desktop Services APIs */
/********************************/

#include <windows.h>

/* MAKE_EXPORT ProcessIdToSessionId_new=ProcessIdToSessionId */
BOOL WINAPI ProcessIdToSessionId_new(DWORD dwProcessId, DWORD *pSessionId)
{
	if (!pSessionId)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}

	/* Process not running under RDS session */
	*pSessionId = 0;
	return TRUE;
}

/* MAKE_EXPORT WTSGetActiveConsoleSessionId_new=WTSGetActiveConsoleSessionId */
DWORD WINAPI WTSGetActiveConsoleSessionId_new(void)
{
	/* local session */
	return 0;
}
