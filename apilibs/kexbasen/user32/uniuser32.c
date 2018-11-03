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

#include <windows.h>
#include <commctrl.h>
#include "unifwd.h"

FORWARD_TO_UNICOWS(AppendMenuW);
FORWARD_TO_UNICOWS(BroadcastSystemMessageW);
FORWARD_TO_UNICOWS(CallMsgFilterW);
FORWARD_TO_UNICOWS(CallWindowProcW);
FORWARD_TO_UNICOWS(ChangeDisplaySettingsExW);
FORWARD_TO_UNICOWS(ChangeDisplaySettingsW);
FORWARD_TO_UNICOWS(ChangeMenuW);
FORWARD_TO_UNICOWS(CharLowerBuffW);
FORWARD_TO_UNICOWS(CharLowerW);
FORWARD_TO_UNICOWS(CharNextW);
FORWARD_TO_UNICOWS(CharPrevW);
FORWARD_TO_UNICOWS(CharToOemBuffW);
FORWARD_TO_UNICOWS(CharToOemW);
FORWARD_TO_UNICOWS(CharUpperBuffW);
FORWARD_TO_UNICOWS(CharUpperW);
FORWARD_TO_UNICOWS(CopyAcceleratorTableW);
FORWARD_TO_UNICOWS(CreateAcceleratorTableW);
FORWARD_TO_UNICOWS(CreateDialogIndirectParamW);

static BOOL CALLBACK SetUnicode(HWND hwnd, LPARAM lParam)
{
    return SendMessage(hwnd, CCM_SETUNICODEFORMAT, TRUE, 0);
}

/* MAKE_EXPORT CreateDialogParamW_new=CreateDialogParamW */
HWND WINAPI CreateDialogParamW_new(HINSTANCE hInstance, LPCSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
	typedef HWND (WINAPI *CreateDialogParamW_t)(HINSTANCE, LPCSTR, HWND, DLGPROC, LPARAM);
	static CreateDialogParamW_t g_DialogParamW;
    HWND hDlg;
	if (!g_DialogParamW) g_DialogParamW = (CreateDialogParamW_t) GetUnicowsAddress("CreateDialogParamW");
	hDlg = g_DialogParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	if (hDlg) EnumChildWindows(hDlg, SetUnicode, 0);
	return hDlg;
}

FORWARD_TO_UNICOWS(CreateMDIWindowW);
FORWARD_TO_UNICOWS(CreateWindowExW);
FORWARD_TO_UNICOWS(DdeCreateStringHandleW);
FORWARD_TO_UNICOWS(DdeInitializeW);
FORWARD_TO_UNICOWS(DdeQueryStringW);
FORWARD_TO_UNICOWS(DefDlgProcW);
FORWARD_TO_UNICOWS(DefFrameProcW);
FORWARD_TO_UNICOWS(DefMDIChildProcW);

/* MAKE_EXPORT DefWindowProcW_new=DefWindowProcW */
LRESULT WINAPI DefWindowProcW_new(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	typedef LRESULT (WINAPI *DefWindowProcW_t)(HWND, UINT, WPARAM, LPARAM);
    static DefWindowProcW_t g_DefProcW;
	if (!g_DefProcW) g_DefProcW = (DefWindowProcW_t) GetUnicowsAddress("DefWindowProcW");
    if (Msg == WM_NOTIFYFORMAT && lParam == NF_QUERY && IsWindowUnicode(hWnd)) return NFR_UNICODE;
    return g_DefProcW(hWnd, Msg, wParam, lParam);
}

FORWARD_TO_UNICOWS(DialogBoxIndirectParamW);
FORWARD_TO_UNICOWS(DialogBoxParamW);
FORWARD_TO_UNICOWS(DispatchMessageW);
FORWARD_TO_UNICOWS(DlgDirListComboBoxW);
FORWARD_TO_UNICOWS(DlgDirListW);
FORWARD_TO_UNICOWS(DlgDirSelectComboBoxExW);
FORWARD_TO_UNICOWS(DlgDirSelectExW);
FORWARD_TO_UNICOWS(DrawStateW);
FORWARD_TO_UNICOWS(DrawTextExW);
FORWARD_TO_UNICOWS(DrawTextW);
FORWARD_TO_UNICOWS(EnumDisplayDevicesW);
FORWARD_TO_UNICOWS(EnumDisplaySettingsExW);
FORWARD_TO_UNICOWS(EnumDisplaySettingsW);
FORWARD_TO_UNICOWS(EnumPropsExW);
FORWARD_TO_UNICOWS(EnumPropsW);
FORWARD_TO_UNICOWS(FindWindowExW);
FORWARD_TO_UNICOWS(FindWindowW);
FORWARD_TO_UNICOWS(GetAltTabInfoW);
FORWARD_TO_UNICOWS(GetClassInfoExW);
FORWARD_TO_UNICOWS(GetClassInfoW);
FORWARD_TO_UNICOWS(GetClassLongW);
FORWARD_TO_UNICOWS(GetClassNameW);
FORWARD_TO_UNICOWS(GetClipboardFormatNameW);
FORWARD_TO_UNICOWS(GetDlgItemTextW);
FORWARD_TO_UNICOWS(GetKeyboardLayoutNameW);
FORWARD_TO_UNICOWS(GetKeyNameTextW);
FORWARD_TO_UNICOWS(GetMenuItemInfoW);
FORWARD_TO_UNICOWS(GetMenuStringW);
FORWARD_TO_UNICOWS(GetMessageW);
FORWARD_TO_UNICOWS(GetMonitorInfoW);
FORWARD_TO_UNICOWS(GetPropW);
FORWARD_TO_UNICOWS(GetTabbedTextExtentW);
FORWARD_TO_UNICOWS(GetWindowLongW);
FORWARD_TO_UNICOWS(GetWindowTextLengthW);
FORWARD_TO_UNICOWS(GetWindowTextW);
FORWARD_TO_UNICOWS(GrayStringW);
FORWARD_TO_UNICOWS(InsertMenuItemW);
FORWARD_TO_UNICOWS(InsertMenuW);
FORWARD_TO_UNICOWS(IsCharAlphaNumericW);
FORWARD_TO_UNICOWS(IsCharAlphaW);
FORWARD_TO_UNICOWS(IsCharLowerW);
FORWARD_TO_UNICOWS(IsCharUpperW);
FORWARD_TO_UNICOWS(IsDialogMessageW);
FORWARD_TO_UNICOWS(IsWindowUnicode);
FORWARD_TO_UNICOWS(LoadAcceleratorsW);
FORWARD_TO_UNICOWS(LoadBitmapW);
FORWARD_TO_UNICOWS(LoadCursorFromFileW);
FORWARD_TO_UNICOWS(LoadCursorW);
FORWARD_TO_UNICOWS(LoadIconW);
FORWARD_TO_UNICOWS(LoadImageW);
FORWARD_TO_UNICOWS(LoadKeyboardLayoutW);
FORWARD_TO_UNICOWS(LoadMenuIndirectW);
FORWARD_TO_UNICOWS(LoadMenuW);
FORWARD_TO_UNICOWS(MapVirtualKeyExW);
FORWARD_TO_UNICOWS(MapVirtualKeyW);
FORWARD_TO_UNICOWS(MessageBoxIndirectW);
FORWARD_TO_UNICOWS(ModifyMenuW);
FORWARD_TO_UNICOWS(OemToCharBuffW);
FORWARD_TO_UNICOWS(OemToCharW);
FORWARD_TO_UNICOWS(PeekMessageW);
FORWARD_TO_UNICOWS(PostMessageW);
FORWARD_TO_UNICOWS(PostThreadMessageW);
FORWARD_TO_UNICOWS(RegisterClassExW);
FORWARD_TO_UNICOWS(RegisterClassW);
FORWARD_TO_UNICOWS(RegisterClipboardFormatW);
FORWARD_TO_UNICOWS(RegisterDeviceNotificationW);
FORWARD_TO_UNICOWS(RegisterWindowMessageW);
FORWARD_TO_UNICOWS(RemovePropW);
FORWARD_TO_UNICOWS(SendDlgItemMessageW);
FORWARD_TO_UNICOWS(SendMessageCallbackW);
FORWARD_TO_UNICOWS(SendMessageTimeoutW);
FORWARD_TO_UNICOWS(SendMessageW);
FORWARD_TO_UNICOWS(SendNotifyMessageW);
FORWARD_TO_UNICOWS(SetClassLongW);
FORWARD_TO_UNICOWS(SetDlgItemTextW);
FORWARD_TO_UNICOWS(SetMenuItemInfoW);
FORWARD_TO_UNICOWS(SetPropW);
FORWARD_TO_UNICOWS(SetWindowLongW);
FORWARD_TO_UNICOWS(SetWindowsHookExW);
FORWARD_TO_UNICOWS(SetWindowsHookW);
FORWARD_TO_UNICOWS(SetWindowTextW);
FORWARD_TO_UNICOWS(SystemParametersInfoW);
FORWARD_TO_UNICOWS(TabbedTextOutW);
FORWARD_TO_UNICOWS(TranslateAcceleratorW);
FORWARD_TO_UNICOWS(UnregisterClassW);
FORWARD_TO_UNICOWS(VkKeyScanExW);
FORWARD_TO_UNICOWS(VkKeyScanW);
FORWARD_TO_UNICOWS(WinHelpW);
FORWARD_TO_UNICOWS(wsprintfW);
FORWARD_TO_UNICOWS(wvsprintfW);
