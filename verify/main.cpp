#include <windows.h>
#include "resource.h"

/* Program used to verify KernelEx installation.
 * If it annoys you add DWORD value "NoVerify" 
 * under HKLM\Software\KernelEx and set it to 1.
 */

HINSTANCE hInstance;

void install_success()
{
	char msg[256];
	char title[100];

	LoadString(hInstance, IDS_TITLE, title, sizeof(title));
	
	LoadString(hInstance, IDS_SUCCESS, msg, sizeof(msg));
	MessageBox(NULL, msg, title, MB_OK | MB_ICONINFORMATION);
}

void install_failure()
{
	char msg[256];
	char title[100];

	LoadString(hInstance, IDS_TITLE, title, sizeof(title));
	
	LoadString(hInstance, IDS_FAILURE, msg, sizeof(msg));
	MessageBox(NULL, msg, title, MB_OK | MB_ICONERROR);
}

void erase_from_startup()
{
	HKEY key;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",
			0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
		return;

	RegDeleteValue(key, "KexVerify");
	RegCloseKey(key);
}

bool is_disable_verify()
{
	HKEY key;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\KernelEx",
			0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS)
		return false;

	DWORD type;
	DWORD data;
	DWORD size = sizeof(data);
	LONG ret = RegQueryValueEx(key, "NoVerify", NULL, &type, (BYTE*) &data, &size);

	RegCloseKey(key);

	if (ret != ERROR_SUCCESS || type != REG_DWORD || size != sizeof(DWORD) || data == 0)
		return false;
	return true;
}

bool test_core()
{
	HMODULE h = LoadLibrary("KERNELEX.DLL");
	PROC proc = GetProcAddress(h, "kexGetKEXVersion");
	FreeLibrary(h);
	
	if (!proc)
		return false;

	return true;
}

bool test_apis()
{
	HMODULE h = GetModuleHandle("KERNEL32.DLL");
	PROC proc = GetProcAddress(h, "GetSystemWindowsDirectoryA");
	if (!proc)
		return false;

	h = LoadLibrary("GDI32.DLL");
	proc = GetProcAddress(h, "GetGlyphIndicesW");
	FreeLibrary(h);

	if (!proc)
		return false;

	return true;
}

__declspec(noreturn)
void entrypoint()
{
	hInstance = GetModuleHandle(NULL);

	erase_from_startup();

	if (!is_disable_verify())
	{
		if (test_core() && test_apis())
			install_success();
		else
			install_failure();
	}

	ExitProcess(0);
}
