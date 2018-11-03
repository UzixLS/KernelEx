/*
 *  KernelEx
 *
 *  Copyright (C) 2009, Tihiy
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

#ifndef __GDI9X_H
#define __GDI9X_H

#define LHE_DISCARDED	0x40
#define LHE_FREEHANDLE	0xFF

#define GDI_OBJTYPE_MASK	0x5FFF
#define GDI_OBJTYPE_DC		0x4F4D
#define GDI_OBJTYPE_DC_NO	0x4F4E

#define ResetMapMode( hdc ) SetMapMode( hdc, GetMapMode(hdc) )

#pragma pack(push,1)

typedef struct
{
	WORD wBlock;
	BYTE bFlags;
	BYTE bLocks;
} LHENTRY, *PLHENTRY;

typedef struct
{
	WORD wZero;				//+0
	WORD wType;				//+2
	BYTE filler[0x80];		//+4
	WORD WindowExtX;		//+84
	WORD WindowExtY;		//+86
	BYTE filler2[4];		//+88
	WORD ViewportExtX;		//+8C
	WORD ViewportExtY;		//+8E
	BYTE filler3[4];		//+90
	WORD mapmode;			//+94
} DCOBJ, *PDCOBJ;

#pragma pack(pop)

#endif
