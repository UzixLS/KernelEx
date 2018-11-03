#include "hwnd9x.h"

static const char c_szDBCSProp[]="kexDBCS";


static DWORD GetWindowProcessId( HWND hwnd )
{
	DWORD procID = 0;
	GetWindowThreadProcessId( hwnd, &procID );
	return procID;
}

#define ISOURPROCESSHWND(hwnd) ( GetWindowProcessId(hwnd) == GetCurrentProcessId() )

#define WS_EX_UNICODE 0x80000000
#define WF_EX_WIN32 0x02000000
#define WF_M_DIALOG	0x2
 
#define IS32BITWIN(pwnd) (pwnd->dwFlags & WF_EX_WIN32)
#define ISDIALOG(pwnd) (pwnd->moreFlags & WF_M_DIALOG)

typedef PMSGQUEUE (WINAPI *GCQ_API) ();

#define RELTOABS(x) ( (DWORD)(x) + *(DWORD*)(x) + 4 )

int GetCPFromLocale(LCID Locale);

LRESULT WINAPI CallWindowProc_stdcall( WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

WPARAM wparam_AtoW( HWND hwnd, UINT message, WPARAM wParam, BOOL messDBCS );
WPARAM wparam_WtoA( UINT message, WPARAM wParam );

LRESULT WINAPI CallProcUnicodeWithAnsi( WNDPROC callback, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI CallProcAnsiWithUnicode( WNDPROC callback, HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI DefWindowProcW_NEW( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI DefDlgProcW_NEW( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI DefMDIChildProcW_NEW( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL IsWindowReallyUnicode(HWND hwnd);

LRESULT WINAPI SendMessageA_fix(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

BOOL InitUniThunkLayer();
