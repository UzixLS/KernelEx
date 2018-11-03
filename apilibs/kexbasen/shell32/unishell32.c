/*
 *  KernelEx
 *  Copyright (C) 2008, Xeno86
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

#include "unifwd.h"

FORWARD_TO_UNICOWS(DragQueryFileW);
FORWARD_TO_UNICOWS(ExtractIconExW);
FORWARD_TO_UNICOWS(ExtractIconW);
FORWARD_TO_UNICOWS(FindExecutableW);
FORWARD_TO_UNICOWS(SHBrowseForFolderW);
FORWARD_TO_UNICOWS(Shell_NotifyIconW);
FORWARD_TO_UNICOWS(ShellAboutW);
FORWARD_TO_UNICOWS(ShellExecuteExW);
FORWARD_TO_UNICOWS(ShellExecuteW);
FORWARD_TO_UNICOWS(SHFileOperationW);
FORWARD_TO_UNICOWS(SHGetFileInfoW);
FORWARD_TO_UNICOWS(SHGetNewLinkInfoW);
