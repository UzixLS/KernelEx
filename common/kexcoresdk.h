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

/** KernelEx Core SDK. */


#ifndef __KEXCORESDK_H
#define __KEXCORESDK_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef KEXCORE_EXPORTS
#define _KEXCOREIMP __declspec(dllexport)
#else
#define _KEXCOREIMP __declspec(dllimport)
#endif


/*****************************************************************************
 *
 * Every api library has to export the following functions:
 *
 *
 * const apilib_api_table* get_api_table()
 *
 *     Function should return pointer to api tables provided by the library.
 *     Function will be called when KernelEx Core builds new api
 *     configuration table, but only if the library hasn't been loaded yet.
 *     Named api and ordinal api tables have to be sorted in ascending order, 
 *     according to operator< rules below.
 *     After the last api table there has to be a closing NULL entry
 *     (all fields set to zeros).
 *     Multiple apis with same names and ordinal numbers are allowed in single
 *     api library. However only one api with given name or ordinal will be 
 *     included in single api configuration. It is possible to specify which
 *     api should be used in core.ini configuration file.
 *
 *****************************************************************************/


/** Convenience macro for defining apis. */
#define DECL_API(name_ord,func) { name_ord, (unsigned long) func }
/** Convenience macro for defining tables. */
#define DECL_TAB(lib,named,unnamed) { lib, named, sizeof(named) / sizeof(apilib_named_api), unnamed, sizeof(unnamed) / sizeof(apilib_unnamed_api) }


/** apilib_named_api - named api structure. */
typedef struct _apilib_named_api
{
	const char*    name; /**< Api name. */
	unsigned long  addr; /**< Api address. */
} apilib_named_api;


/** apilib_unnamed_api - unnamed api structure. */
typedef struct _apilib_unnamed_api
{
	unsigned short ord;  /**< Api ordinal number. */
	unsigned long  addr; /**< Api address. */
} apilib_unnamed_api;


/** apilib_api_table - api table structure. */
typedef struct _apilib_api_table
{
	const char*               target_library;     /**< DLL library for which the apis are defined. */
	const apilib_named_api*   named_apis;         /**< Pointer to table of named apis. */
	int                       named_apis_count;   /**< Number of named apis in the table. */
	const apilib_unnamed_api* ordinal_apis;       /**< Pointer to table of unnamed apis. */
	int                       ordinal_apis_count; /**< Number of unnamed apis in the table. */
} apilib_api_table;


/** Type definition for get_api_table() function. */
typedef const apilib_api_table* (* fgat_t)();


/** kexGetKEXVersion - obtain KernelEx Core version
 * 
 * Returned value is KernelEx version number encoded as follows:
 *   0xHHLLRRRR, where:
 *     HH - major version number
 *     LL - minor version number
 *     RRRR - revision number
 */ 
_KEXCOREIMP unsigned long kexGetKEXVersion();


/** kexIsDebugCore - determine release/debug KernelEx Core version
 *
 * @return Zero if release Core, one if debug Core.
 */
_KEXCOREIMP int kexIsDebugCore();


/** kexDebugPrint - output debug information
 *
 * Parameters are compatible with printf command, 
 * maximum output length is limited to 256 bytes.
 */
_KEXCOREIMP void kexDebugPrint(const char* format, ...);


/** kexGetVersion - obtain original Windows version number.
 *
 * Refer to GetVersion API documentation for parameters and output.
 */
_KEXCOREIMP DWORD kexGetVersion();


/** kexGetProcAddress - obtain original address of DLL library function.
 *
 * Refer to GetProcAddress API documentation for parameters and output.
 */
_KEXCOREIMP PROC kexGetProcAddress(HMODULE hModule, PCSTR lpProcName);


/** kexPIDtoPDB - obtain pointer to process database entry, given process identifier.
 *
 * @param pid Process identifier.
 * @return Pointer to process database entry on success, NULL on failure.
 */
_KEXCOREIMP void* kexPIDtoPDB(DWORD pid);


/** kexTIDtoTDB - obtain pointer to thread database entry, given thread identifier.
 *
 * @param tid Thread identifier.
 * @return Pointer to thread database entry on success, NULL on failure.
 */
_KEXCOREIMP void* kexTIDtoTDB(DWORD tid);


/** kexGetModuleSettings - Retrieve per module settings.
 *	
 * @param module Module path.
 * @param conf_name Receives configuration name, has to be at least 256 bytes long.
 * @param ldr_flags Receives flags.
 */
_KEXCOREIMP void kexGetModuleSettings(const char* module, 
                                      char* conf_name, BYTE* ldr_flags);


/** kexSetModuleSettings - Set per module settings.
 *
 * @param module Module path.
 * @param conf_name Configuration name to be set for the module.
 * @param ldr_flags Flags to be set for the module.
 */
_KEXCOREIMP void kexSetModuleSettings(const char* module, 
                                      const char* conf_name, BYTE ldr_flags);


/** kexFlushAppSettings - Reloads all module settings from registy. */
_KEXCOREIMP void kexFlushAppSettings(void);


#ifdef __cplusplus
} /* extern "C" */
#endif


#ifdef __cplusplus


/** Comparison operator used to order named api entries. */
inline bool operator<(const apilib_named_api& a, const apilib_named_api& b)
{
	return strcmp(a.name, b.name) < 0;
}


/** Comparison operator used to order unnamed api entries. */
inline bool operator<(const apilib_unnamed_api& a, const apilib_unnamed_api& b)
{
	return a.ord < b.ord;
}


#endif


#endif
