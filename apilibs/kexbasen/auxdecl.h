/* This file contains required declarations missing from MINGW and MSVC6 without PSDK */

#ifndef __AUXDECL_H
#define __AUXDECL_H


#include <windows.h>

/* declarations for MSVC6 without PSDK */
#if defined (_MSC_VER) && (WINVER < 0x0500)

#define VER_MINORVERSION 0x0000001
#define VER_MAJORVERSION 0x0000002
#define VER_BUILDNUMBER 0x0000004
#define VER_PLATFORMID 0x0000008
#define VER_SERVICEPACKMINOR 0x0000010
#define VER_SERVICEPACKMAJOR 0x0000020
#define VER_SUITENAME 0x0000040
#define VER_PRODUCT_TYPE 0x0000080
#define VER_EQUAL 1
#define VER_GREATER 2
#define VER_GREATER_EQUAL 3
#define VER_LESS 4
#define VER_LESS_EQUAL 5
#define VER_AND 6
#define VER_OR 7

#define VER_NT_WORKSTATION 1
#define VER_NT_DOMAIN_CONTROLLER 2
#define VER_NT_SERVER 3
#define VER_SUITE_SMALLBUSINESS 1
#define VER_SUITE_ENTERPRISE 2
#define VER_SUITE_BACKOFFICE 4
#define VER_SUITE_TERMINAL 16
#define VER_SUITE_SMALLBUSINESS_RESTRICTED 32
#define VER_SUITE_EMBEDDEDNT 64
#define VER_SUITE_DATACENTER 128
#define VER_SUITE_SINGLEUSERTS 256
#define VER_SUITE_PERSONAL 512
#define VER_SUITE_BLADE 1024
#define VER_SUITE_STORAGE_SERVER 8192
#define VER_SUITE_COMPUTE_SERVER 16384

#define OSVERSIONINFOEXA __OSVERSIONINFOEXA
#define POSVERSIONINFOEXA __POSVERSIONINFOEXA
#define LPOSVERSIONINFOEXA __LPOSVERSIONINFOEXA
#define OSVERSIONINFOEXW __OSVERSIONINFOEXW
#define POSVERSIONINFOEXW __POSVERSIONINFOEXW
#define LPOSVERSIONINFOEXW __LPOSVERSIONINFOEXW

typedef struct {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	CHAR szCSDVersion[128];
	WORD wServicePackMajor;
	WORD wServicePackMinor;
	WORD wSuiteMask;
	BYTE wProductType;
	BYTE wReserved;
} __OSVERSIONINFOEXA, *__POSVERSIONINFOEXA, *__LPOSVERSIONINFOEXA;
typedef struct {
	DWORD dwOSVersionInfoSize;
	DWORD dwMajorVersion;
	DWORD dwMinorVersion;
	DWORD dwBuildNumber;
	DWORD dwPlatformId;
	WCHAR szCSDVersion[128];
	WORD wServicePackMajor;
	WORD wServicePackMinor;
	WORD wSuiteMask;
	BYTE wProductType;
	BYTE wReserved;
} __OSVERSIONINFOEXW, *__POSVERSIONINFOEXW, *__LPOSVERSIONINFOEXW;

typedef long LONG_PTR;
typedef BOOL (CALLBACK *UILANGUAGE_ENUMPROCA)(LPSTR, LONG_PTR);
typedef BOOL (CALLBACK *UILANGUAGE_ENUMPROCW)(LPWSTR, LONG_PTR);

#define MEMORYSTATUSEX __MEMORYSTATUSEX
#define PMEMORYSTATUSEX __PMEMORYSTATUSEX
#define LPMEMORYSTATUSEX __LPMEMORYSTATUSEX

typedef struct
{
  DWORD dwLength;
  DWORD dwMemoryLoad;
  DWORDLONG ullTotalPhys;
  DWORDLONG ullAvailPhys;
  DWORDLONG ullTotalPageFile;
  DWORDLONG ullAvailPageFile;
  DWORDLONG ullTotalVirtual;
  DWORDLONG ullAvailVirtual;
  DWORDLONG ullAvailExtendedVirtual;
} __MEMORYSTATUSEX, *__PMEMORYSTATUSEX, *__LPMEMORYSTATUSEX;

typedef enum {
    SHGFP_TYPE_CURRENT  = 0,
    SHGFP_TYPE_DEFAULT  = 1,
} SHGFP_TYPE;

#define     GA_ROOT         2

typedef void *LPPRINTDLGEX;

EXTERN_C int WINAPI GetRandomRgn(HDC hdc, HRGN hrgn, INT iNum);
EXTERN_C HRESULT WINAPI SHGetFolderPathA(HWND hwnd, int csidl, HANDLE hToken, DWORD dwFlags, LPSTR pszPath);
EXTERN_C HWND WINAPI GetAncestor(HWND hwnd, UINT gaFlags);
EXTERN_C UINT WINAPI RealGetWindowClass(HWND  hwnd, LPTSTR pszType, UINT  cchType);

#endif /* defined (_MSC_VER) && (WINVER < 0x0500) */


/* declarations for MINGW32 */
#ifdef __MINGW32_VERSION

/* The security descriptor structure */
typedef struct {
    BYTE Revision;
    BYTE Sbz1;
    SECURITY_DESCRIPTOR_CONTROL Control;
    DWORD Owner;
    DWORD Group;
    DWORD Sacl;
    DWORD Dacl;
} SECURITY_DESCRIPTOR_RELATIVE, *PISECURITY_DESCRIPTOR_RELATIVE;

#endif /* __MINGW32_VERSION */

#endif /* __AUXDECL_H */
