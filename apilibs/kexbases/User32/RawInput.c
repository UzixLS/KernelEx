/*
 *  KernelEx
 *  Copyright (C) 2008-2010, Xeno86, Tihiy
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

/*	Raw input functions should reset output size buffers to zero.
 *	While (UINT)-1 is "total failure" return, programs (e.g. mpc-hc)
 *	don't check for it, and fail hard thinking it's returned buffer size.
 */
#define RAWNOTIMPLEMETED(intptr) \
	if (intptr) *intptr = 0; \
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED); \
	return 0;
	

/* MAKE_EXPORT DefRawInputProc_new=DefRawInputProc */
LRESULT WINAPI DefRawInputProc_new(PVOID paRawInput, INT nInput, UINT cbSizeHeader)
{
	return E_NOTIMPL;
}

/* MAKE_EXPORT GetRawInputBuffer_new=GetRawInputBuffer */
UINT WINAPI GetRawInputBuffer_new(PVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
	RAWNOTIMPLEMETED(pcbSize);
}

/* MAKE_EXPORT GetRawInputData_new=GetRawInputData */
UINT WINAPI GetRawInputData_new(PVOID hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
	RAWNOTIMPLEMETED(pcbSize);
}

/* MAKE_EXPORT GetRawInputDeviceList_new=GetRawInputDeviceList */
UINT WINAPI GetRawInputDeviceList_new(PVOID pRawInputDeviceList, PUINT puiNumDevices, UINT cbSize)
{
	RAWNOTIMPLEMETED(puiNumDevices);
}

/* MAKE_EXPORT GetRawInputDeviceInfo_new=GetRawInputDeviceInfoA */
/* MAKE_EXPORT GetRawInputDeviceInfo_new=GetRawInputDeviceInfoW */
UINT WINAPI GetRawInputDeviceInfo_new(HANDLE hDevice, UINT uiCommand,  LPVOID pData, PUINT pcbSize)
{
	RAWNOTIMPLEMETED(pcbSize);
}

/* MAKE_EXPORT GetRegisteredRawInputDevices_new=GetRegisteredRawInputDevices */
UINT WINAPI GetRegisteredRawInputDevices_new(PVOID pRawInputDevices, PUINT puiNumDevices, UINT cbSize)
{
	RAWNOTIMPLEMETED(puiNumDevices);
}

/* MAKE_EXPORT RegisterRawInputDevices_new=RegisterRawInputDevices */
BOOL WINAPI RegisterRawInputDevices_new(PVOID pRawInputDevices, UINT uiNumDevices, UINT cbSize)
{
	SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
	return FALSE;
}