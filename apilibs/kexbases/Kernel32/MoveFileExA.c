/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MoveFileExA.cpp,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005/09/08 16:21:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

//#include "macros.h"
#include <windows.h>

#define	WININIT_FILENAME	"wininit.ini"
#define RENAME_SECTION		"rename"
#define BUFSIZE				32767

/* MAKE_EXPORT MoveFileExA_new=MoveFileExA */
BOOL WINAPI MoveFileExA_new( LPCSTR lpExistingFileNameA, LPCSTR lpNewFileNameA, DWORD dwFlags )
{
	BOOL	fSuccess = FALSE;	// assume failure

	// Windows 9x has a special mechanism to move files after reboot

	if ( dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT )
	{
		CHAR	szExistingFileNameA[MAX_PATH];
		CHAR	szNewFileNameA[MAX_PATH];
		lstrcpyA( szNewFileNameA, "NUL" );

		// Path names in WININIT.INI must be in short path name form

		if ( 
			GetShortPathNameA( lpExistingFileNameA, szExistingFileNameA, MAX_PATH ) &&
			(!lpNewFileNameA || GetShortPathNameA( lpNewFileNameA, szNewFileNameA, MAX_PATH ))
			)
		{
			CHAR*	szBuffer = (CHAR*) HeapAlloc( GetProcessHeap(), 0, BUFSIZE );	// The buffer size must not exceed 32K
			DWORD	dwBufLen = GetPrivateProfileSectionA( RENAME_SECTION, szBuffer, BUFSIZE, WININIT_FILENAME );

			CHAR	szRename[MAX_PATH];	// This is enough for at most to times 67 chracters
			size_t	lnRename;
			lstrcpyA( szRename, szNewFileNameA );
			lstrcatA( szRename, "=" );
			lstrcatA( szRename, szExistingFileNameA );
			lnRename = lstrlenA(szRename);

			if ( dwBufLen + lnRename + 2 <= BUFSIZE )
			{
				lstrcpyA( &szBuffer[dwBufLen], szRename );
				szBuffer[ dwBufLen + lnRename + 1 ] = 0;

				fSuccess = WritePrivateProfileSectionA( RENAME_SECTION, szBuffer, WININIT_FILENAME );
			}
			else
				SetLastError( ERROR_BUFFER_OVERFLOW );
				
			HeapFree( GetProcessHeap(), 0, szBuffer );
		}
	}
	else
	{

		fSuccess = MoveFileA( lpExistingFileNameA, lpNewFileNameA );

		if ( !fSuccess && 0 != (dwFlags & (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) )
		{
			BOOL	bFailIfExist = 0 == (dwFlags & MOVEFILE_REPLACE_EXISTING);

			fSuccess = CopyFileA( lpExistingFileNameA, lpNewFileNameA, bFailIfExist );

			// In case of successfull copy do not return FALSE if delete fails.
			// Error detection is done by GetLastError()

			if ( fSuccess )
			{
				SetLastError( NO_ERROR );
				DeleteFileA( lpExistingFileNameA );
			}
		}

	}

	return fSuccess;
}

