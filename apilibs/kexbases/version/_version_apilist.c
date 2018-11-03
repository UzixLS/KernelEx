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

#include "common.h"
#include "kexcoresdk.h"
#include "_version_apilist.h"

BOOL init_version()
{
	return TRUE;
}

static const apilib_named_api version_named_apis[] = 
{
/*** AUTOGENERATED APILIST NAMED EXPORTS BEGIN ***/
	DECL_API("GetFileVersionInfoSizeW", GetFileVersionInfoSizeW_new),
	DECL_API("GetFileVersionInfoW", GetFileVersionInfoW_new),
	DECL_API("VerFindFileW", VerFindFileW_new),
	DECL_API("VerInstallFileW", VerInstallFileW_new),
	DECL_API("VerLanguageNameW", VerLanguageNameW_new),
	DECL_API("VerQueryValueW", VerQueryValueW_new),
/*** AUTOGENERATED APILIST NAMED EXPORTS END ***/
};

#if 0
static const apilib_unnamed_api version_ordinal_apis[] =
{
/*** AUTOGENERATED APILIST ORDINAL EXPORTS BEGIN ***/
/*** AUTOGENERATED APILIST ORDINAL EXPORTS END ***/
};
#endif

const apilib_api_table apitable_version = DECL_TAB("VERSION.DLL", version_named_apis, 0 /*version_ordinal_apis*/);
