/*
 * Shlwapi string functions
 *
 * Copyright 1998 Juergen Schmied
 * Copyright 2002 Jon Griffiths
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <windows.h>
#pragma warning(disable:4002)
#define TRACE()

EXTERN_C DECLSPEC_IMPORT BOOL WINAPI StrToIntExW(LPCWSTR lpszStr, DWORD dwFlags, LPINT lpiRet);
EXTERN_C DECLSPEC_IMPORT BOOL WINAPI ChrCmpIW(WCHAR ch1, WCHAR ch2);

static int isdigitW(WCHAR wc)
{
	if (wc >= '0' && wc <= '9')
		return 1;
	return 0;
}

/*************************************************************************
 * StrCmpLogicalW	[SHLWAPI.@]
 *
 * Compare two strings, ignoring case and comparing digits as numbers.
 *
 * PARAMS
 *  lpszStr  [I] First string to compare
 *  lpszComp [I] Second string to compare
 *  iLen     [I] Length to compare
 *
 * RETURNS
 *  TRUE  If the strings are equal.
 *  FALSE Otherwise.
 */
/* MAKE_EXPORT StrCmpLogicalW_new=StrCmpLogicalW */
INT WINAPI StrCmpLogicalW_new(LPCWSTR lpszStr, LPCWSTR lpszComp)
{
  INT iDiff;

  TRACE("(%s,%s)\n", debugstr_w(lpszStr), debugstr_w(lpszComp));

  if (lpszStr && lpszComp)
  {
    while (*lpszStr)
    {
      if (!*lpszComp)
        return 1;
      else if (isdigitW(*lpszStr))
      {
        int iStr, iComp;

        if (!isdigitW(*lpszComp))
          return -1;

        /* Compare the numbers */
        StrToIntExW(lpszStr, 0, &iStr);
        StrToIntExW(lpszComp, 0, &iComp);

        if (iStr < iComp)
          return -1;
        else if (iStr > iComp)
          return 1;

        /* Skip */
        while (isdigitW(*lpszStr))
          lpszStr++;
        while (isdigitW(*lpszComp))
          lpszComp++;
      }
      else if (isdigitW(*lpszComp))
        return 1;
      else
      {
        iDiff = ChrCmpIW(*lpszStr,*lpszComp);
        if (iDiff > 0)
          return 1;
        else if (iDiff < 0)
          return -1;

        lpszStr++;
        lpszComp++;
      }
    }
    if (*lpszComp)
      return -1;
  }
  return 0;
}

