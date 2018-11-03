/*
 *  KernelEx
 *
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

/* MAKE_EXPORT UpdateLayeredWindow_new=UpdateLayeredWindow */
BOOL WINAPI UpdateLayeredWindow_new(
  HWND hwnd,             // handle to layered window
  HDC hdcDst,            // handle to screen DC
  POINT *pptDst,         // new screen position
  SIZE *psize,           // new size of the layered window
  HDC hdcSrc,            // handle to surface DC
  POINT *pptSrc,         // layer position
  COLORREF crKey,        // color key
  BLENDFUNCTION *pblend, // blend function
  DWORD dwFlags          // options
)
{
	BOOL ret = FALSE;
	if ( (GetWindowLong(hwnd,GWL_EXSTYLE) & WS_EX_LAYERED) && hdcSrc && pptSrc )
	{
		RECT rc;
		GetWindowRect(hwnd,&rc);
		if (pptDst)
		{
			OffsetRect(&rc,-rc.left,-rc.top);
			OffsetRect(&rc,pptDst->x,pptDst->y);
		}
		if (psize)
		{
			rc.right = rc.left + psize->cx;
			rc.bottom = rc.top + psize->cy;
		}
		MoveWindow(hwnd,rc.left,rc.top,rc.right-rc.left,rc.bottom-rc.top,FALSE);
		HDC hdc = GetDC( hwnd );
		OffsetRect(&rc,-rc.left,-rc.top);
		ret = BitBlt(hdc,0,0,rc.right-rc.left,rc.bottom-rc.top,hdcSrc,pptSrc->x,pptSrc->y,SRCCOPY);
		ReleaseDC(hwnd,hdc);
	}
	return ret;	
}
