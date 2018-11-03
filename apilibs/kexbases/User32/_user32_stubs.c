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

#include "common.h"

UNIMPL_FUNC(SetLayeredWindowAttributes, 4);
//raw input APIs
UNIMPL_FUNC(DefRawInputProc, 3);
UNIMPL_FUNC(GetRawInputBuffer, 3);
UNIMPL_FUNC(GetRawInputData, 5);
UNIMPL_FUNC(GetRawInputDeviceList, 3);
UNIMPL_FUNC(GetRawInputDeviceInfoA, 4);
UNIMPL_FUNC(GetRawInputDeviceInfoW, 4);
UNIMPL_FUNC(GetRegisteredRawInputDevices, 3);
UNIMPL_FUNC(RegisterRawInputDevices, 3);
//
UNIMPL_FUNC(PrintWindow, 3);