/*
 *  KernelEx
 *  Copyright (C) Matt Pietrek 1995
 *  Copyright (C) 2009 Tihiy
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

#ifndef _HWND32_H
#define _HWND32_H

#define IS_SYSTEM_HWND(hwnd) ((hwnd)>=(HWND)0x80 && (hwnd)<=(HWND)0x88)

#pragma pack (1)
typedef struct _RECTS
{
	SHORT    left;
	SHORT    top;
	SHORT    right;
	SHORT    bottom;
} RECTS, *PRECTS, *LPRECTS;

typedef struct _WND
{
	struct _WND *spwndNext;    // 00h (GW_HWNDNEXT) HWND of next sibling window
	struct _WND *spwndChild;   // 04h (GW_CHILD) First child window
	struct _WND *spwndParent;  // 08h Parent window handle
	struct _WND *spwndOwner;   // 0Ch Owning window handle
	RECTS   rcWindow;          // 10h Rectangle describing entire window
	RECTS   rcClient;          // 18h Rectangle for client area of window
	WORD    hQueue;            // 20h Application message queue handle
	WORD    hrgnUpdate;        // 22h window region needing an update
	WORD    wndClass;          // 24h handle to an INTWNDCLASS
	WORD    hInstance;         // 26h hInstance of creating application
	WNDPROC lpfnWndProc;       // 28h Window procedure address
	DWORD   dwFlags;           // 2Ch internal state flags
	DWORD   style;             // 30h WS_XXX style flags
	DWORD   ExStyle;           // 34h WS_EX_XXX extended style flags
	DWORD   moreFlags;         // 38h flags
	HANDLE  spmenu;            // 3Ch GetDlgCtrlId or hMenu
	WORD    windowTextOffset;  // 40h Offset of the window's text in atom heap
	WORD    scrollBar;         // 42h DWORD associated with the scroll bars
	WORD    properties;        // 44h Handle for first window property
	WORD    hWnd16;            // 46h Actual HWND value for this window
	struct _WND *lastActive;   // 48h Last active owned popup window
	HANDLE  hMenuSystem;       // 4Ch handle to the system menu
	DWORD   un1;               // 50h
	WORD    un2;               // 54h
	WORD    classAtom;         // 56h See also offs. 2 in the field 24 struct ptr
} WND, *PWND;

typedef struct _MSGQUEUE
{
	WORD    nextQueue;  // 00h  next queue in the list
	WORD    hTask;      // 02h  Task that this queue is associated with
	WORD    headMsg;    // 04h  Near ptr to head of linked list of QUEUEMSGs
	WORD    tailMsg;    // 06h  Near ptr to end of list of QUEUEMSGs
	WORD    cMsgs;      // 08h  Number of posted messages (98,Me)
	WORD    nilword;	// 0Ah  ???
	BYTE    un1;        // 0Ch  ???
	BYTE    sig[3];     // 0Dh  "MJT" (Michael Jackson ..??)
	WORD    npPerQueue; // 10h  16 bit offset in USER DGROUP to PERQUEUEDATA
	                    //      type == LT_USER_VWININFO???
	WORD    un2;        // 12h  ???
	WORD    un2_5;      // 14h  ??
	WORD    npProcess;  // 16h  near pointer in USER DGROUP to a QUEUEPROCESSDATA
	DWORD   un3[3];     // 18h  ???
	DWORD   messageTime;// 24h  retrieved by GetMessageTime()
	DWORD   messagePos; // 28h  retrived by GetMessagePos()
	WORD    un4;        // 2Ch  ??? (seems to always be 0)
	WORD    lastMsg2;   // 2Eh  Near ptr to last retrieved QUEUEMSG
	DWORD   extraInfo;  // 30h  returned by GetMessageExtraInfo()
	DWORD   un5;		// 34h  ???
	WORD	cQuit;		// 38h	PostQuitMessage
	WORD	exitCode;	// 3Ah	same	
	DWORD   threadId;   // 3Ch  See GetWindowProcessThreadId
	WORD    un6;        // 40h  ??
	WORD    expWinVer;  // 42h  Version of Windows this app expects
	DWORD   un7;        // 44h  ???
	WORD    ChangeBits; // 48h  high order word returned by GetQueueStatus
	WORD    WakeBits;   // 4Ah  low order word returned by GetQueueStatus
	WORD    WakeMask;   // 4Ch  The QS_xxx bits that GetMessage/PeekMessage are
	                    //      waiting for
	WORD    un8;        // 4Eh  ???
	WORD    hQueueSend; // 50h  App that's in SendMessage to this queue
	WORD	hSMS;       // 52h  SMS (inter-thread sendmessage) struct ptr
	WORD	dunno;		// 54h  ???
	WORD    sig2;       // 56h  "HQ"
	HKL     hkl;        // 58h  current thread HKL (keyboard layout)
	DWORD   un10[3];    // 5Ch  ??? filler
	WORD    block1;     // 5Eh+0Ah      block for SendMessageA (hook?)
	WORD    un11;       // 5Eh+0Ah+2    ??? filler
	DWORD   un12[3];    // 5Eh+0Ah+4    ??? filler
	WORD    block2;     // 5Eh+1Ah      block for SendMessageA (event?)
} MSGQUEUE, *PMSGQUEUE;

typedef struct _PERQUEUEDATA
{
	WORD    npNext;         // 00h a USER heap handle (type == LT_USER_VWININFO)
	WORD    un2;            // 02h
	WORD    wSumFlags;      // 04h
	WORD    npQMsg;         // 06h type == LT_USER_QMSG
	WORD    un5;            // 08h
	WORD    un6;            // 0Ah
	WORD    un7;            // 0Ch
	WORD    un8;            // 0Eh
	WORD    un9;            // 10h
	WORD    un10;           // 12h
	WORD    somehQueue1;    // 14h a msg queue handle
	WORD    somehQueue2;    // 16h a msg queue handle
	DWORD   hWndCapture;    // 18h
	DWORD   hWndFocus;      // 1Ch
	DWORD   hWndActive;     // 20h
	BYTE	filler[0x34];	// 24h
	DWORD	keysbuffer;		// 58h 16:16 keyboard state ptr
} PERQUEUEDATA, *PPERQUEUEDATA;

typedef struct _QUEUEKEYBUFFER
{
	BYTE filler[0x20];		// 00h sth
	BYTE keystate[0xFF];	// 20h keys state (0x80-pressed, 0x1-toggled)
} QUEUEKEYBUFFER, *PQUEUEKEYBUFFER;


#pragma pack()

/* IsWindow returns PWND */
#define HWNDtoPWND(hwnd) (PWND)IsWindow(hwnd) 

#endif
