/*
 *  KernelEx
 *
 *  Copyright (C) 2009, Tihiy
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
#include "GdiObjects.h"
#include "k32ord.h"
#include "kexcoresdk.h"

static BOOL blockkexgdiobj;
static WORD g_GDILH_addr;
static DWORD g_GdiBase;

#define REBASEGDI(x) ((PVOID)( g_GdiBase + LOWORD((DWORD)(x)) ))
#define REBASEGDIHIGH(x) ( g_GdiBase + (DWORD)(x) ) 

#define GDIHEAP32BASE 0x10000
#define GDIHEAP32TOP  0x20000

BOOL InitGDIObjects(void)
{
	g_GdiBase = MapSL( LoadLibrary16("gdi") << 16 );
	g_GDILH_addr = ((PINSTANCE16)g_GdiBase)->pLocalHeap;
	blockkexgdiobj = (BOOL)GetProcAddress(GetModuleHandle("rp10.dll"),"blockkexgdiobj");
	return (BOOL)g_GdiBase;
}

PGDIOBJ16 GetGDIObjectPtr( HGDIOBJ hgdiobj )
{
	WORD wHandle = (WORD)hgdiobj;
	if ( wHandle & 1 ) return NULL; //all gdi handles end in b10 or b00, not b01
	if ( wHandle & 3 ) //64K heap
	{		
		PLHENTRY entry = (PLHENTRY)REBASEGDI( wHandle );
		if ( wHandle <= g_GDILH_addr || entry->bFlags == LHE_FREEHANDLE || entry->wBlock <= g_GDILH_addr )
			return NULL; //deleted or invalid handle
		if ( entry->bFlags & LHE_DISCARDED ) //discarded DC
		{
			if ( entry->wBlock & 3 ) return NULL; //they have to divide by 4			
			return GetGDIObjectPtr( (HGDIOBJ)entry->wBlock ); //recurse
		}
		else
		{
			return (PGDIOBJ16)REBASEGDI(entry->wBlock);
		}
	}
	else //high heap
	__try
	{
		if ( wHandle < 0x80 ) return NULL; //high heap handles start with 0x80
		if ( wHandle > *(WORD*)REBASEGDIHIGH( GDIHEAP32BASE + 0x70 ) ) return NULL; //max hadle value there
		DWORD* high = (DWORD*)REBASEGDIHIGH( GDIHEAP32BASE + wHandle );
		if ( *high < GDIHEAP32TOP ) return NULL; //points to invalid handle memory
		PGDIOBJ16 ret = (PGDIOBJ16)REBASEGDIHIGH( *high );
		if ( ret->wType == 0 ) return NULL; //points to invalid object memory 
		return ret;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
}

static DWORD SwitchGDIObjectType( PGDIOBJ16 obj )
{
	__try
	{
		switch ( obj->wType & GDI_TYPEMASK )
		{		
			case GDI_TYPE_PEN:
				return OBJ_PEN;
			case GDI_TYPE_BRUSH:
				return OBJ_BRUSH;
			case GDI_TYPE_FONT:
				return OBJ_FONT;
			case GDI_TYPE_PAL:
				return OBJ_PAL;
			case GDI_TYPE_BITMAP:
				return OBJ_BITMAP;
			case GDI_TYPE_REGION:
				return OBJ_REGION;
			case GDI_TYPE_DC:
			{
				PDCOBJ dcobj = (PDCOBJ)obj;
				if ( dcobj->enhmetadc ) return OBJ_ENHMETADC;
				if ( dcobj->dcFlags & 1 ) return OBJ_MEMDC;
				//fall down
			}
			case GDI_TYPE_DCX:
			case GDI_TYPE_DCY:
				return OBJ_DC;
			case GDI_TYPE_METADC:
				return OBJ_METADC;
			case GDI_TYPE_ENHMETA:
				return OBJ_ENHMETAFILE;			
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		//too bad
	}
	return 0;
}


/* MAKE_EXPORT GetObjectType_fix=GetObjectType */
DWORD WINAPI GetObjectType_fix( HGDIOBJ hgdiobj )
{
	if (blockkexgdiobj) return GetObjectType(hgdiobj);
	//GetObjectType is rewritten in order to boost it's correctness and speed:
	//constantly throwing page/segfaults is very bad on virtual machines.
	PGDIOBJ16 obj = GetGDIObjectPtr( hgdiobj );	
	DWORD result = 0;
	if ( obj )
	{
		GrabWin16Lock();
		result = SwitchGDIObjectType( obj );
		ReleaseWin16Lock();
	}
	else  //still, can be metafile selector
	{
		WORD wHandle = (WORD)hgdiobj;
		if ( (wHandle & 6) == 6 ) //test for ldt selector
		{
			LDT_ENTRY selector;
			GetThreadSelectorEntry( GetCurrentThread(), wHandle, &selector );
			if ( selector.HighWord.Bits.Type == 3 )
				result = GetObjectType(hgdiobj); //resort to gdi32
		}
	}
	if ( !result ) SetLastError( ERROR_INVALID_HANDLE );
	return result;
}

/* MAKE_EXPORT DeleteObject_fix=DeleteObject */
BOOL WINAPI DeleteObject_fix( HGDIOBJ hObject )
{
	if (blockkexgdiobj) return DeleteObject(hObject);
	PGDIOBJ16 obj = GetGDIObjectPtr( hObject );
	if ( !obj || !SwitchGDIObjectType(obj) ) return FALSE;
	DWORD violated = 0;
	if ( obj->wOwner ) //not system objects
	{
		if (obj->wType == GDI_TYPE_FONT && ((PFONTOBJ16)obj)->wSelCount >= SEL_FONT_ONCE )
		{
			DBGPRINTF(("somebody is trying to delete selected font %p\n",hObject));
			violated = GDI_TYPE_FONT;
		}
		if (obj->wType == GDI_TYPE_BITMAP && ((PBITMAPOBJ16)obj)->wSelCount == SEL_BITMAP_ONCE )
		{
			DBGPRINTF(("somebody is trying to delete selected bitmap %p\n",hObject));
			violated = GDI_TYPE_BITMAP;
		}
	}
	BOOL ret = DeleteObject( hObject );
	if ( violated == GDI_TYPE_FONT )
		((PFONTOBJ16)obj)->wSelCount |= SEL_FONT_DEL;
	else if ( violated == GDI_TYPE_BITMAP )
		((PBITMAPOBJ16)obj)->wSelCount |= SEL_BITMAP_DEL;
	return ret;
}

/* MAKE_EXPORT SelectObject_fix=SelectObject */
HGDIOBJ WINAPI SelectObject_fix( HDC hdc, HGDIOBJ hgdiobj )
{
	if (blockkexgdiobj) return SelectObject(hdc,hgdiobj);
	//9x should do this
	if ( !hdc ) SetLastError(ERROR_INVALID_HANDLE);
	if ( !hdc || !hgdiobj ) return NULL;
	HGDIOBJ ret;
	ret = SelectObject( hdc, hgdiobj );
	PGDIOBJ16 obj = GetGDIObjectPtr( ret );
	if ( obj && obj->wOwner )
	{
		if ( obj->wType == GDI_TYPE_FONT && ((PFONTOBJ16)obj)->wSelCount == SEL_FONT_DEL )
		{
			DBGPRINTF(("deleting font %p on unselecting\n",ret));
			DeleteObject(ret);
		}
		if ( obj->wType == GDI_TYPE_BITMAP && ((PBITMAPOBJ16)obj)->wSelCount == SEL_BITMAP_DEL )
		{
			((PBITMAPOBJ16)obj)->wSelCount = 0;
			DBGPRINTF(("deleting bitmap %p on unselecting\n",ret));
			DeleteObject(ret);
		}		
	}
	return ret;
}

/* MAKE_EXPORT DeleteDC_fix=DeleteDC */
BOOL WINAPI DeleteDC_fix( HDC hdc )
{
	if (blockkexgdiobj) return DeleteDC(hdc);
	BOOL ret;
	PGDIOBJ16 obj = GetGDIObjectPtr( hdc );
	if ( !obj || !SwitchGDIObjectType(obj) ) return FALSE;
	HGDIOBJ fnt = GetCurrentObject(hdc,OBJ_FONT);
	HGDIOBJ bmp = GetCurrentObject(hdc,OBJ_BITMAP);
	ret = DeleteDC(hdc);
	if (ret)
	{
		PFONTOBJ16 fntobj = (PFONTOBJ16)GetGDIObjectPtr(fnt);
		PBITMAPOBJ16 bitmapobj = (PBITMAPOBJ16)GetGDIObjectPtr(bmp);
		if (fntobj && fntobj->wOwner && fntobj->wType == GDI_TYPE_FONT && fntobj->wSelCount == SEL_FONT_DEL)
		{
			DBGPRINTF(("deleting font %p on dc cleanup\n",fnt));
			DeleteObject(fnt);
		}
		if (bitmapobj && bitmapobj->header.wOwner && bitmapobj->header.wType == GDI_TYPE_BITMAP && bitmapobj->wSelCount == SEL_BITMAP_DEL)
		{
			bitmapobj->wSelCount = 0;
			DBGPRINTF(("deleting bitmap %p on dc cleanup\n",bmp));
			DeleteObject(bmp);
		}
	}
	return ret;
}

/* MAKE_EXPORT CreateDIBSection_fix=CreateDIBSection */
HBITMAP WINAPI CreateDIBSection_fix(
  HDC hdc,                 // handle to DC
  BITMAPINFO *pbmi,  // bitmap data
  UINT iUsage,             // data type indicator
  VOID **ppvBits,          // bit values
  HANDLE hSection,         // handle to file mapping object
  DWORD dwOffset           // offset to bitmap bit values
)
{
	if (pbmi && pbmi->bmiHeader.biSize == sizeof(BITMAPINFO)) //9x does not forgive
		pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER); //9x does not forget
	return CreateDIBSection(hdc,pbmi,iUsage,ppvBits,hSection,dwOffset);
}
