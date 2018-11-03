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

#include <windows.h>
#include <commctrl.h>
#include <malloc.h>
#include "DebugWindow.h"
#include "resource.h"
#include "internals.h"
#include "debug.h"

const unsigned short WM_KEXSTOPDEBUG = 0x6eee;
const unsigned short WM_KEXAPPENDLOG = 0x6eef;

DebugWindow* DebugWindow::instance = NULL;

extern "C"
char* strtok_r(char* s, const char* delim, char** holder);


DebugWindow::DebugWindow()
{
	DWORD tid;
	hwnd = (HWND) -1;
	includes.push_back("*");
	InitializeCriticalSection(&cs);
	InitCommonControls();
	hThread = CreateThread(NULL, 0, thread, (void*) this, 0, &tid);
}

DebugWindow::~DebugWindow()
{
	DeleteCriticalSection(&cs);
	SendMessage(hwnd, WM_KEXSTOPDEBUG, 0, 0);
}

BOOL CALLBACK DebugWindow::DebugDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	NMHDR* nmhdr;
	DebugWindow* _this = (DebugWindow*) GetWindowLong(hwnd, GWL_USERDATA);

	switch (msg)
	{
	case WM_INITDIALOG:
		_this = (DebugWindow*) lParam;
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		_this->InitDialog(hwnd);
		break;
	case WM_SIZE:
		MoveWindow(GetDlgItem(hwnd, IDC_LOG), 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		SendDlgItemMessage(hwnd, IDC_LOG, WM_VSCROLL, SB_BOTTOM, 0);
		break;
	case WM_KEXSTOPDEBUG:
		DestroyWindow(hwnd);
		break;
	case WM_KEXAPPENDLOG:
		_this->AppendLog((char*) lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_NOTIFY:
		nmhdr = (NMHDR*) lParam;
		if (nmhdr->idFrom == IDC_LOG && nmhdr->code == NM_RCLICK)
		{
			_this->HandleMenu();
			break;
		}
	default:
		return FALSE;
	}
	return TRUE;
}

void DebugWindow::InitDialog(HWND hwnd)
{
	hList = GetDlgItem(hwnd, IDC_LOG);
	SetClassLong(hwnd, GCL_STYLE, GetClassLong(hwnd, GCL_STYLE) | CS_NOCLOSE);
	MoveWindow(hwnd, 0, 0, 320, 200, TRUE);
	SendMessage(hList, LVM_SETEXTENDEDLISTVIEWSTYLE,
           0, LVS_EX_FULLROWSELECT);

	LV_COLUMN col;
	memset(&col, 0, sizeof(col));
	col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;

	col.cx = 80;
	col.pszText = "Process";
	ListView_InsertColumn(hList, 0, &col);
	col.cx = 60;
	col.pszText = "Thread";
	ListView_InsertColumn(hList, 1, &col);
	col.cx = 80;
	col.pszText = "Source";
	ListView_InsertColumn(hList, 2, &col);
	col.cx = 80;
	col.pszText = "Dest";
	ListView_InsertColumn(hList, 3, &col);
	col.cx = 120;
	col.pszText = "Function";
	ListView_InsertColumn(hList, 4, &col);
	col.cx = 40;
	col.mask |= LVCF_FMT;
	col.fmt = LVCFMT_RIGHT;
	col.pszText = "Return";
	ListView_InsertColumn(hList, 5, &col);

	menu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_LOGMENU));
	menu = GetSubMenu(menu, 0);
}

void DebugWindow::HandleMenu()
{
	POINT p;
	GetCursorPos(&p);

	int res = TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, 
			p.x, p.y, 0, hwnd, NULL);
	switch (res) 
	{
	case IDM_TOFILE:
		WriteToFile();
		break;
	case IDM_CLEAR:
		//FIXME: when there is a lot of entries it takes very long to clear the list
		//how to improve this?
		ListView_DeleteAllItems(hList);
		break;
	case IDM_FILTER:
		DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DEBUGFILTER), 
				hwnd, FilterDlgProc, (LPARAM) this);
		break;
	}
}

void DebugWindow::AppendLog(char* msg)
{
	LV_ITEM item;
	int idx;
	HWND hList = GetDlgItem(hwnd, IDC_LOG);
	char* p;
	char* pch;
	
	memset(&item, 0, sizeof(item));

	pch = strtok_r(msg, "|", &p);
	if (!pch)
		return;

	item.mask = LVIF_TEXT;
	item.iItem = ListView_GetItemCount(hList);
	item.iSubItem = 0;
	item.pszText = pch;
	idx = ListView_InsertItem(hList, &item);

	while ((pch = strtok_r(NULL, "|", &p)) != NULL)
	{
		item.iSubItem++;
		item.pszText = pch;
		ListView_SetItem(hList, &item);
	}
}

BOOL CALLBACK DebugWindow::FilterDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DebugWindow* _this = (DebugWindow*) GetWindowLong(hwnd, GWL_USERDATA);
	int len1, len2;
	char* buf;
	char* pch;
	char* p;
	list<sstring>::const_iterator it;

	switch (msg)
	{
	case WM_INITDIALOG:
		_this = (DebugWindow*) lParam;
		SetWindowLong(hwnd, GWL_USERDATA, lParam);
		
		len1 = 0; 
		len2 = 0;
		for (it = _this->includes.begin() ; it != _this->includes.end() ; it++)
			len1 += it->length() + 1;
		for (it = _this->excludes.begin() ; it != _this->excludes.end() ; it++)
			len2 += it->length() + 1;
		buf = (char*) alloca(max(len1, len2));
		
		buf[0] = '\0';
		for (it = _this->includes.begin() ; it != _this->includes.end() ; it++)
		{
			if (it != _this->includes.begin())
				strcat(buf, ";");
			strcat(buf, *it);
		}
		SetDlgItemText(hwnd, IDC_DFINCLUDE, buf);
		
		buf[0] = '\0';
		for (it = _this->excludes.begin() ; it != _this->excludes.end() ; it++)
		{
			if (it != _this->excludes.begin())
				strcat(buf, ";");
			strcat(buf, *it);
		}
		SetDlgItemText(hwnd, IDC_DFEXCLUDE, buf);
		
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) 
		{
		case IDCANCEL:
			EndDialog(hwnd, 0);
			break;
		case IDOK:
			len1 = GetWindowTextLength(GetDlgItem(hwnd, IDC_DFINCLUDE)) + 1;
			len2 = GetWindowTextLength(GetDlgItem(hwnd, IDC_DFEXCLUDE)) + 1;
			buf = (char*) alloca(max(len1, len2));
			EnterCriticalSection(&_this->cs);

			GetDlgItemText(hwnd, IDC_DFINCLUDE, buf, len1);
			_this->includes.clear();
			pch = strtok_r(buf, ";", &p);
			if (pch)
			{
				_this->includes.push_back(pch);
				while ((pch = strtok_r(NULL, ";", &p)) != NULL)
					_this->includes.push_back(pch);
			}

			GetDlgItemText(hwnd, IDC_DFEXCLUDE, buf, len2);
			_this->excludes.clear();
			pch = strtok_r(buf, ";", &p);
			if (pch)
			{
				_this->excludes.push_back(pch);
				while ((pch = strtok_r(NULL, ";", &p)) != NULL)
					_this->excludes.push_back(pch);
			}

			LeaveCriticalSection(&_this->cs);
			EndDialog(hwnd, 0);
			break;
		}
		break;

	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}

void DebugWindow::WriteToFile()
{
	char buf[DEBUGMSG_MAXLEN];
	OPENFILENAME ofn;
	int rows;
	LV_COLUMN col;
	char filename[MAX_PATH];
	HANDLE hFile;
	HMODULE hComDlg32;
	BOOL (WINAPI* pGetSaveFileName)(OPENFILENAME*);

	hComDlg32 = LoadLibrary("COMDLG32.DLL");
	if (!hComDlg32)
		return;
	pGetSaveFileName = (BOOL (WINAPI*)(OPENFILENAME*)) 
			GetProcAddress(hComDlg32, "GetSaveFileNameA");
	if (!pGetSaveFileName)
		goto __fini;

	filename[0] = '\0';
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.hInstance = hInstance;
	ofn.lpstrFilter = "Text file\0*.txt\0";
	ofn.lpstrDefExt = ".txt";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = sizeof(filename);
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	if (!pGetSaveFileName(&ofn))
		goto __fini;

	hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		goto __fini;

	//write column headers
	col.mask = LVCF_TEXT;
	col.pszText = buf;
	col.cchTextMax = sizeof(buf);
	DWORD wlen;
	for (int j = 0 ; j < 6 ; j++)
	{
		DWORD len;
		ListView_GetColumn(hList, j, &col);
		len = strlen(buf);
		if (j) WriteFile(hFile, "|", 1, &wlen, NULL);
		WriteFile(hFile, buf, len, &wlen, NULL);
	}
	WriteFile(hFile, "\r\n", 2, &wlen, NULL);

	//write rows
	rows = ListView_GetItemCount(hList);
	for (int i = 0 ; i < rows ; i++)
	{
		for (int j = 0 ; j < 6 ; j++)
		{
			DWORD len; DWORD wlen;
			ListView_GetItemText(hList, i, j, buf, sizeof(buf));
			len = strlen(buf);
			if (j) WriteFile(hFile, "|", 1, &wlen, NULL);
			WriteFile(hFile, buf, len, &wlen, NULL);
		}
		WriteFile(hFile, "\r\n", 2, &wlen, NULL);
	}

	CloseHandle(hFile);

	MessageBox(hwnd, "File written successfully", "Information", MB_ICONINFORMATION | MB_OK);

__fini:
	FreeLibrary(hComDlg32);
}

DWORD WINAPI DebugWindow::thread(void* param)
{
	MSG msg;
	DebugWindow* _this = (DebugWindow*) param;
	_this->hwnd = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DEBUG), 
			NULL, DebugDlgProc, (LPARAM) _this);
	ShowWindow(_this->hwnd, SW_MINIMIZE);
	while (GetMessage(&msg, NULL, 0, 0))
		DispatchMessage(&msg);
	return 0;
}

void DebugWindow::append(const char* str)
{
	static char msg[DEBUGMSG_MAXLEN];
	bool filter_out = true;
	list<sstring>::const_iterator it;

	EnterCriticalSection(&cs);

	//filter out based on includes and excludes
	if (includes.size() != 0)
	{
		if (includes.size() == 1 && strcmp(includes.front(), "*") == 0)
			filter_out = false;
		else for (it = includes.begin() ; it != includes.end() ; it++)
			if (strstr(str, *it))
			{
				filter_out = false;
				break;
			}
	}

	if (!filter_out)
		for (it = excludes.begin() ; it != excludes.end() ; it++)
			if (strstr(str, *it))
			{
				filter_out = true;
				break;
			}

	if (filter_out)
	{
		LeaveCriticalSection(&cs);
		return;
	}

	strncpy(msg, str, sizeof(msg));
	msg[sizeof(msg) - 1] = '\0';

	SendMessage(hwnd, WM_KEXAPPENDLOG, 0, (LPARAM) msg);

	LeaveCriticalSection(&cs);
	SendMessage(hList, WM_VSCROLL, SB_BOTTOM, 0);
}

DebugWindow* DebugWindow::get()
{
	if (instance->hwnd == (HWND) -1 || instance->hwnd == NULL)
	{
		delete instance;
		instance = NULL;
	}
	return instance;
}

bool DebugWindow::create()
{
	instance = new DebugWindow;
	if (instance->hThread)
		return true;
	else
	{
		delete instance;
		instance = NULL;
		return false;
	}
}

void DebugWindow::destroy()
{
	if (instance)
		delete instance;
}
