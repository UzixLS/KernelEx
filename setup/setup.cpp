/*
 *  KernelEx
 *  Copyright (C) 2008-2010, Xeno86
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
#include <tlhelp32.h>
#include <cstdio>
#include <cstdarg>
#include "debug.h"
#include "loadstub.h"
#include "pemanip.h"
#include "setup.h"
#include "wininit.h"
#include "version.h"
#include "resource.h"
#include "patch.h"

#define DATA_SEG ".data"


void ShowError(UINT id, ...)
{
	char format[512];
	char out[512];
	va_list vargs;
	
	va_start(vargs, id);
	if (!LoadString(GetModuleHandle(NULL), id, format, sizeof(format)))
		sprintf(out, "ERROR: Missing string resource %d", id);
	else
		_vsnprintf(out, sizeof(out), format, vargs);
	va_end(vargs);
	DBGPRINTF(("%s\n", out));
	MessageBox(NULL, out, "KernelEx Setup", MB_OK | MB_ICONERROR);
	exit(id);
}

Setup::Setup(char* _backup_file) : backup_file(strupr(_backup_file))
{
	h_kernel32 = GetModuleHandle("KERNEL32");
	
	detect_old_version();
	detect_downgrade();

	version = get_signature_ver();

	OSVERSIONINFOA osv;
	osv.dwOSVersionInfoSize = sizeof(osv);

	GetVersionEx(&osv);
	if ((osv.dwPlatformId != VER_PLATFORM_WIN32_WINDOWS) 
			|| (osv.dwMajorVersion != 4)
			|| (osv.dwMinorVersion != 10 && osv.dwMinorVersion != 90))
		ShowError(IDS_WINVER);

	//on WinMe and file called "setup.exe" we get 4.10 (lol)
	if (osv.dwMinorVersion == 90)
	{
		DBGPRINTF(("Detected Windows Me\n"));
		is_winme = true;
	}
	else
	{
		DBGPRINTF(("Detected Windows 98\n"));
		is_winme = false;
	}
}

bool Setup::detect_old_version()
{
	typedef void (WINAPI *KV_t)(char *);
	KV_t h_KVersion;
	char buf[32];
	
	h_KVersion = (KV_t)GetProcAddress(h_kernel32, "KUPVersion");
	if (!h_KVersion)
		h_KVersion = (KV_t)GetProcAddress(h_kernel32, "KEXVersion");
	if (!h_KVersion)
		return false;
	h_KVersion(buf);
	if (strcmp(buf, "4") < 0)
		ShowError(IDS_OLDKEX);
	return true;
}

void Setup::detect_downgrade()
{
	typedef unsigned long (*kexGetKEXVersion_t)();
	kexGetKEXVersion_t kexGetKEXVersion_pfn;
	HMODULE h_kernelex;
	unsigned long version;
	
	h_kernelex = LoadLibrary("KERNELEX.DLL");

	//no KernelEx installed, continue
	if (!h_kernelex)
		return;

	kexGetKEXVersion_pfn = (kexGetKEXVersion_t) GetProcAddress(h_kernelex, "kexGetKEXVersion");
	version = kexGetKEXVersion_pfn();
	FreeLibrary(h_kernelex);

	//trying to downgrade - forbid
	if (version > VERSION_CODE)
		ShowError(IDS_DOWNGRADE);
}

int Setup::get_signature_ver()
{
	DWORD ptr;
	KernelEx_dataseg* dseg = NULL;
	DWORD sign_len = sizeof(KEX_SIGNATURE) - 1;

	PEmanip pemem;
	pemem.OpenMemory(h_kernel32);
	if (!pemem.HasTarget())
		ShowError(IDS_FAILOPEN, "KERNEL32 memory image");

	DWORD data = (DWORD) pemem.GetSectionByName(DATA_SEG);
	int data_size = pemem.GetSectionSize(DATA_SEG);

	ptr = data + data_size - sign_len;
	while (ptr >= data)
	{
		if (!memcmp((void*)ptr, KEX_SIGNATURE, sign_len))
		{
			dseg = (KernelEx_dataseg*) ptr;
			break;
		}
		ptr--;
	}

	if (!dseg)
	{
		DBGPRINTF(("Signature not found\n"));
		return -1;
	}
	else
		DBGPRINTF(("Signature found @ 0x%08x\n", ptr));

	return dseg->version;
}

void Setup::kill_process(const char* name)
{
	PROCESSENTRY32 pe32;
	void * h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	
	if (h == INVALID_HANDLE_VALUE) 
		return;
	
	pe32.dwSize = sizeof(PROCESSENTRY32);
	
	Process32First(h, &pe32);
	do
	{
		if (strstr(pe32.szExeFile, name))
		{
			DBGPRINTF(("Terminating process: %s... ", name));
			HANDLE hp = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
			if (hp == INVALID_HANDLE_VALUE)
			{
				DBGPRINTF(("open fail\n"));
				continue;
			}
			if (TerminateProcess(hp, 0))
				DBGPRINTF(("ok\n"));
			else
				DBGPRINTF(("fail\n"));
			CloseHandle(hp);
		}
	}
	while (Process32Next(h, &pe32));

	CloseHandle(h);
}

sstring Setup::get_temp_file_name()
{
	char tmpdir[MAX_PATH];
	char target[MAX_PATH];

	GetSystemDirectory(tmpdir, sizeof(tmpdir));
	GetTempFileName(tmpdir, "kex", 0, target);
	
	return target;
}

void Setup::install()
{
	upgrade = false;

	if (version >= 0)
	{
		if (version == KEX_STUB_VER)
		{
			DBGPRINTF(("No need to upgrade\n"));
			return;
		}
		else
		{
			upgrade = true;
		}
	}

	char kernel32path[MAX_PATH];
	GetModuleFileName(h_kernel32, kernel32path, sizeof(kernel32path));
	
	PEmanip pefile;
	pefile.OpenFile(upgrade ? backup_file : kernel32path, 0x10000);
	if (!pefile.HasTarget())
	{
		if (upgrade)
			ShowError(IDS_FAILOPENBACKUP, (const char*) backup_file, kernel32path);
		else
			ShowError(IDS_FAILOPEN, kernel32path);
	}

	Patch patch(pefile);
	patch.apply();

	// backup original file
	if (!upgrade)
	{
		if (!CopyFile(kernel32path, backup_file, FALSE))
			ShowError(IDS_FAILBAK, (const char*) backup_file);
	}

	// save patched file
	sstring tmp_file = get_temp_file_name();
	pefile.SaveFile(tmp_file);

	if (is_winme)
	{
		kill_process("\\STATEMGR.EXE");
		kill_process("\\STMGR.EXE");
	}

	Wininit.add(kernel32path, tmp_file);

	if (is_winme)
	{
		char path[MAX_PATH];
		GetWindowsDirectory(path, sizeof(path));
		if (path[strlen(path) - 1] != '\\')
			strcat(path, "\\");
		strcat(path, "WININIT.LOG");
		Wininit.add("NUL", path);
		path[3] = '\0';
		strcat(path, "_RESTORE\\TEMP");
		Wininit.add("DIRNUL", path);
	}

	Wininit.save();
}

void Setup::set_reboot_flag()
{
	HKEY hkey;
	if (RegCreateKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\"
			"CurrentVersion\\RunServicesOnce", &hkey) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkey, "KexNeedsReboot", 0, REG_SZ, (const BYTE*) "", 1);
		RegCloseKey(hkey);
	}
}

void Setup::register_verify()
{
	HKEY hkey;
	DWORD type;
	char verify_path[MAX_PATH];
	DWORD size = sizeof(verify_path);
	LONG res;

	res = RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\KernelEx", &hkey);
	if (res != ERROR_SUCCESS)
		return;
	res = RegQueryValueEx(hkey, "InstallDir", 0, &type, (BYTE*) verify_path, &size);
	RegCloseKey(hkey);
	if (res != ERROR_SUCCESS)
		return;

	strcat(verify_path, "\\verify.exe");

	if (RegCreateKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\"
			"CurrentVersion\\Run", &hkey) == ERROR_SUCCESS)
	{
		RegSetValueEx(hkey, "KexVerify", 0, REG_SZ, (BYTE*) verify_path,
				strlen(verify_path) + 1);
		RegCloseKey(hkey);
	}
}

void Setup::reboot()
{
	char out[512];
	
	if (!LoadString(GetModuleHandle(NULL), IDS_REBOOT, out, sizeof(out)))
		sprintf(out, "ERROR: Missing string resource %d", IDS_REBOOT);

	int res = MessageBox(NULL, out, "KernelEx Setup", MB_OKCANCEL | MB_ICONQUESTION);
	if (res == IDOK)
	{
		DBGPRINTF(("Rebooting...\n"));
		ExitWindowsEx(EWX_REBOOT, 0);
	}
}

int main(int argc, char** argv)
{
	bool reboot = false;
	DBGPRINTF(("KernelEx setup program running\n"));
	if (argc != 1 && argc != 2)
		return 1;
	if (argc == 2 && !strcmp(argv[1], "/R"))
		reboot = true;

	char backup_path[MAX_PATH];
	GetWindowsDirectory(backup_path, MAX_PATH);
	if (backup_path[strlen(backup_path)-1] != '\\')
		strcat(backup_path, "\\");
	strcat(backup_path, "SYSBCKUP\\KERNEL32.DLL");

	Setup setup(backup_path);
	setup.install();

	DBGPRINTF(("Setup finished\n"));
	if (reboot)
		setup.reboot();

	setup.set_reboot_flag();
	setup.register_verify();
	return 0;
}
