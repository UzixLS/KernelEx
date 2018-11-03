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
#include <stdio.h>
#include <shlwapi.h>
#include "sheet.h"
#include "server.h"
#include "resource.h"
#include "KexLinkage.h"


KexShlExt::KexShlExt()
{
	g_LockCount++;
	m_RefCount = 0;
}

KexShlExt::~KexShlExt()
{
	g_LockCount--;
}


STDMETHODIMP KexShlExt::QueryInterface(REFIID riid,LPVOID *ppv)
{
	if (riid == IID_IUnknown)
		*ppv = static_cast<IUnknown*>(static_cast<IShellExtInit*>(this));
	else if	(riid == IID_IShellExtInit)
		*ppv = static_cast<IShellExtInit*>(this);
	else if (riid == IID_IShellPropSheetExt)
		*ppv = static_cast<IShellPropSheetExt*>(this);
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


STDMETHODIMP_(ULONG) KexShlExt::AddRef() 
{	
	return ++m_RefCount;
}


STDMETHODIMP_(ULONG) KexShlExt::Release() 
{
	if (!--m_RefCount)
	{
		delete this;
		return 0;
	}
	return m_RefCount;
}


bool KexShlExt::IsPEModule(const char* path)
{
	IMAGE_DOS_HEADER MZh;
	IMAGE_NT_HEADERS PEh;
	FILE* f;
	bool result = false;

	f = fopen(path, "rb");
	if (!f)
		return false;

	if (fread(&MZh, sizeof(MZh), 1, f) != 1)
		goto __end;
	
	if (MZh.e_magic != IMAGE_DOS_SIGNATURE)
		goto __end;

	if (fseek(f, MZh.e_lfanew, SEEK_SET))
		goto __end;

	if (fread(&PEh, sizeof(PEh), 1, f) != 1)
		goto __end;

	if ((PEh.Signature != IMAGE_NT_SIGNATURE) 
			|| (PEh.FileHeader.Machine != IMAGE_FILE_MACHINE_I386) 
			|| (PEh.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC))
		goto __end;

	result = true;

__end:
	fclose(f);
	
	return result;
}

bool KexShlExt::ResolveShortcut(const char* shortcutPath, char* filePath)
{
    HRESULT result;
    IShellLink* shellLink;
	IPersistFile* persistFile; 
    char path[MAX_PATH]; 
    WCHAR tmp[MAX_PATH];

    result = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (void**) &shellLink); 

	if (FAILED(result))
		return false;

	result = shellLink->QueryInterface(IID_IPersistFile, (void**) &persistFile);
	if (SUCCEEDED(result))
	{
		MultiByteToWideChar(CP_ACP, 0, shortcutPath, -1, tmp, MAX_PATH);

		result = persistFile->Load(tmp, STGM_READ); 
		if (SUCCEEDED(result)) 
		{
			result = shellLink->Resolve(NULL, SLR_UPDATE); 
			if (SUCCEEDED(result)) 
			{
				result = shellLink->GetPath(path, 
									 MAX_PATH, NULL, SLGP_RAWPATH); 
				if (SUCCEEDED(result))
					lstrcpyn(filePath, path, MAX_PATH); 
			} 
		} 

		persistFile->Release();
	}

	shellLink->Release();

    return SUCCEEDED(result);
}

STDMETHODIMP KexShlExt::Initialize(LPCITEMIDLIST pidlFolder, 
									LPDATAOBJECT pDataObj, HKEY hProgID)
{
	FORMATETC etc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
	STGMEDIUM stg;
	HDROP hdrop;
	HRESULT result = S_OK;

	if (KexLinkage::instance.confs.empty())
		return E_FAIL;
	
	InitCommonControls();

    if (FAILED(pDataObj->GetData(&etc, &stg)))
        return E_INVALIDARG;

    // Get an HDROP handle.
    hdrop = (HDROP) GlobalLock (stg.hGlobal);

    if (!hdrop)
	{
		ReleaseStgMedium(&stg);
		return E_INVALIDARG;
	}

	ms = new ModuleSetting;
	if (!ms)
		return E_OUTOFMEMORY;

	// Determine how many files are involved in this operation.
    UINT numFiles = DragQueryFile (hdrop, 0xFFFFFFFF, NULL, 0);

	if (numFiles != 1)
		result = E_FAIL;
	else
	{
		// Get the filename.
		if (!DragQueryFile(hdrop, 0, ms->file, MAX_PATH) || PathIsDirectory(ms->file))
			result = E_FAIL;
		else
		{
			char* ext = PathFindExtension(ms->file);
			if (!lstrcmpi(ext, ".lnk"))
			{
				if (!ResolveShortcut(ms->file, ms->file))
					result = E_FAIL;
			}
			
			if (!IsPEModule(ms->file))
				result = E_FAIL;
		}
	}

	// Release resources.
	GlobalUnlock(stg.hGlobal);
	ReleaseStgMedium(&stg);

	strupr(ms->file);

	return result;
}


STDMETHODIMP KexShlExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPageProc, LPARAM lParam)
{
	PROPSHEETPAGE psp;
	HPROPSHEETPAGE hPage;

	memset(&psp, 0, sizeof(psp));
	psp.dwSize = sizeof(psp);
	psp.dwFlags = PSP_USEREFPARENT | PSP_USECALLBACK;
	psp.hInstance = g_hModule;
	psp.pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE);
	psp.pfnDlgProc = DlgProc;
	psp.pfnCallback = CallbackProc;
	psp.lParam = (LPARAM) ms;
	psp.pcRefParent = &g_LockCount;

	hPage = CreatePropertySheetPage(&psp);
	if (hPage)
	{
		if (!lpfnAddPageProc(hPage, lParam))
			DestroyPropertySheetPage(hPage);
	}

	return S_OK;
}


STDMETHODIMP KexShlExt::ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM)
{
	return E_NOTIMPL;
}


BOOL CALLBACK KexShlExt::DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{	
	switch (uMsg) 
	{
	case WM_INITDIALOG:
		{
			PROPSHEETPAGE* ppsp = (PROPSHEETPAGE*) lParam;
			OnInitDialog(hwnd, (ModuleSetting*) ppsp->lParam);
		}
		break;

	case WM_SIZE:
		{
			WORD h = HIWORD(lParam);
			WORD w = LOWORD(lParam);

			RECT r;
			POINT p;
			GetWindowRect(GetDlgItem(hwnd, IDC_GCOMPAT), &r);
			p.x = r.left;
			p.y = r.top;
			ScreenToClient(hwnd, &p);

			//reposition horizontal spacer and version text
			MoveWindow(GetDlgItem(hwnd, IDC_GCOMPAT), p.x, p.y, w - 2 * p.x, r.bottom - r.top, TRUE);
			MoveWindow(GetDlgItem(hwnd, IDC_HORIZ1), p.x, h - 14 - p.x, w -  2 * p.x, 1, TRUE);
			MoveWindow(GetDlgItem(hwnd, IDC_KEXVER), p.x, h - 12 - p.x, w - 2 * p.x, 12, TRUE);
			GetWindowRect(GetDlgItem(hwnd, IDC_TCOMPAT), &r);
			MoveWindow(GetDlgItem(hwnd, IDC_TCOMPAT), p.x, p.x, w - 2 * p.x, r.bottom - r.top, TRUE);
		}
		break;

	case WM_NOTIFY:
		{
			NMHDR* phdr = (NMHDR*) lParam;
			if (phdr->code == PSN_APPLY)
				OnApply(hwnd);
		}
		break;
	
	case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_DISABLE:
				if (IsDlgButtonChecked(hwnd, IDC_DISABLE))
				{
					EnableWindow(GetDlgItem(hwnd, IDC_COMPAT), FALSE);
					EnableWindow(GetDlgItem(hwnd, IDC_SYSTEM), FALSE);
				}
				else
				{
					EnableWindow(GetDlgItem(hwnd, IDC_COMPAT), TRUE);
					EnableWindow(GetDlgItem(hwnd, IDC_SYSTEM), 
							IsDlgButtonChecked(hwnd, IDC_COMPAT));
				}
				PropSheet_Changed(GetParent(hwnd), hwnd);
				break;
			case IDC_COMPAT:
				EnableWindow(GetDlgItem(hwnd, IDC_SYSTEM), 
						IsDlgButtonChecked(hwnd, IDC_COMPAT));
				PropSheet_Changed(GetParent(hwnd), hwnd);
				break;
			case IDC_SYSTEM:
				PropSheet_Changed(GetParent(hwnd), hwnd);
				break;
			}
		}
		break;
	}

	return FALSE;
}


void KexShlExt::OnInitDialog(HWND hwnd, ModuleSetting* ms)
{
	SetWindowLong(hwnd, GWL_USERDATA, (LONG) ms);
	vector<KexLinkage::conf>::const_iterator it;
	for (it = KexLinkage::instance.confs.begin() ; 
			it != KexLinkage::instance.confs.end() ; it++)	
		SendMessage(GetDlgItem(hwnd, IDC_SYSTEM), CB_ADDSTRING, 
				0, (LPARAM) it->desc.get());

	if (KexLinkage::instance.default_index >= 0 
			&& KexLinkage::instance.default_index < KexLinkage::instance.confs.size())
		SendMessage(GetDlgItem(hwnd, IDC_SYSTEM), CB_SETCURSEL,
				KexLinkage::instance.default_index, 0);
	else
		SendMessage(GetDlgItem(hwnd, IDC_SYSTEM), CB_SETCURSEL, 0, 0);

	KexLinkage::instance.m_kexGetModuleSettings(ms->file, ms->conf, &ms->flags);
	
	for (int i = 0 ; i < KexLinkage::instance.confs.size() ; i++)
		if (!strcmp(ms->conf, KexLinkage::instance.confs[i].name.get()))
		{
			CheckDlgButton(hwnd, IDC_COMPAT, BST_CHECKED);
			EnableWindow(GetDlgItem(hwnd, IDC_SYSTEM), TRUE);
			SendMessage(GetDlgItem(hwnd, IDC_SYSTEM), CB_SETCURSEL, i, 0);
			break;
		}
	if (ms->flags & 1)
	{
		CheckDlgButton(hwnd, IDC_DISABLE, BST_CHECKED);
		EnableWindow(GetDlgItem(hwnd, IDC_COMPAT), FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_SYSTEM), FALSE);
	}

	//set KernelEx version
	unsigned long ver = KexLinkage::instance.m_kexGetKEXVersion();
	char ver_s[32];
	sprintf(ver_s, "KernelEx Core v%d.%d.%d", ver>>24, (ver>>16) & 0xff, ver & 0xffff);
	SendMessage(GetDlgItem(hwnd, IDC_KEXVER), WM_SETTEXT, 0, (LPARAM) ver_s);
}


void KexShlExt::OnApply(HWND hwnd)
{
	ModuleSetting* ms = (ModuleSetting*) GetWindowLong(hwnd, GWL_USERDATA);
	BYTE flags = 0;
	const char* conf = "default";
	if (IsDlgButtonChecked(hwnd, IDC_DISABLE))
		flags |= 1;
	if (IsDlgButtonChecked(hwnd, IDC_COMPAT))
		conf = KexLinkage::instance.confs[SendMessage(
				GetDlgItem(hwnd, IDC_SYSTEM), CB_GETCURSEL, 0, 0)].name.get();

	if (flags != ms->flags || strcmp(conf, ms->conf) != 0)
		KexLinkage::instance.m_kexSetModuleSettings(ms->file, conf, flags);
}


UINT CALLBACK KexShlExt::CallbackProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp)
{
    if (uMsg == PSPCB_RELEASE)
		delete (ModuleSetting*) ppsp->lParam;

    return 1;   // used for PSPCB_CREATE - let the page be created
}
