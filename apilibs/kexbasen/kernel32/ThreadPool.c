/*
 *  KernelEx
 *  Copyright (C) 2009, Tihiy
 *  Copyright (c) 2006 Robert Shearman (WINE Project)
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
#include "shlord.h"

/*
 *  The following functions are implemented:
 *  QueueUserWorkItem (with SHLWAPI.SHQueueUserWorkItem)
 *  RegisterWaitForSingleObject
 *  RegisterWaitForSingleObjectEx
 *  UnregisterWait
 *  UnregisterWaitEx
 *  all functions could be implemented with shlwapi,
 *  but they don't support async de-registration and most flags
 *  Also, Wine functions can cause problems in cases like
 *  when second unregister is called, or handle is reused...
 *  But Windows XP also fails hard in such cases.
 */

#define TPS_EXECUTEIO 0x00000001
#define TPS_LONGEXECTIME 0x00000008

typedef BOOL (WINAPI* SHQueueUserWorkItem_API) (LPTHREAD_START_ROUTINE pfnCallback,
                     LPVOID pContext,
                     LONG lPriority,
                     PDWORD dwTag,
                     PDWORD * pdwId,
                     LPCSTR pszModule,
                     DWORD dwFlags);

static SHQueueUserWorkItem_API SHQueueUserWorkItem;


/* MAKE_EXPORT QueueUserWorkItem_new=QueueUserWorkItem */
BOOL WINAPI QueueUserWorkItem_new( LPTHREAD_START_ROUTINE Function, PVOID Context, ULONG Flags)
{
	if (!SHQueueUserWorkItem)
		SHQueueUserWorkItem = (SHQueueUserWorkItem_API)GetShlwapiProc(260);

	DWORD dwFlags = 0;
	if (Flags & WT_EXECUTEINIOTHREAD) dwFlags |= TPS_EXECUTEIO;
	if (Flags & WT_EXECUTELONGFUNCTION) dwFlags |= TPS_LONGEXECTIME;

	return SHQueueUserWorkItem( Function, Context, 0, NULL, NULL, NULL, dwFlags );
}

//registerwait routines
typedef struct
{
    HANDLE Object;
    HANDLE CancelEvent;
    WAITORTIMERCALLBACK Callback;
    PVOID Context;
    ULONG Milliseconds;
    ULONG Flags;
    HANDLE CompletionEvent;
    LONG DeleteCount;
    BOOLEAN CallbackInProgress;
} wait_work_item_struct, *wait_work_item_ptr;

static void delete_wait_work_item(wait_work_item_ptr wait_work_item)
{
    CloseHandle( wait_work_item->CancelEvent );
	wait_work_item->CancelEvent = 0; //in case someone tries to work on deleted handle
    HeapFree( GetProcessHeap(), 0, wait_work_item );
}

static DWORD CALLBACK wait_thread_proc(LPVOID Arg)
{
    wait_work_item_ptr wait_work_item = (wait_work_item_ptr)Arg;
    BOOL alertable = (wait_work_item->Flags & WT_EXECUTEINIOTHREAD) ? TRUE : FALSE;
    HANDLE handles[2] = { wait_work_item->Object, wait_work_item->CancelEvent };
    HANDLE completion_event;
	DWORD status;


    while (TRUE)
    {
        status = WaitForMultipleObjectsEx( 2, handles, FALSE, wait_work_item->Milliseconds, alertable );
        if (status == STATUS_WAIT_0 || status == STATUS_TIMEOUT)
        {
            BOOL TimerOrWaitFired = (status == STATUS_WAIT_0) ? FALSE : TRUE;

            wait_work_item->CallbackInProgress = TRUE;
            wait_work_item->Callback( wait_work_item->Context, TimerOrWaitFired );
            wait_work_item->CallbackInProgress = FALSE;

            if (wait_work_item->Flags & WT_EXECUTEONLYONCE)
                break;
        }
        else
            break; //CancelEvent signalled
    }

    completion_event = wait_work_item->CompletionEvent;
    if (completion_event)
		SetEvent( completion_event );

    if ( InterlockedIncrement( &wait_work_item->DeleteCount ) == 2 )
        delete_wait_work_item( wait_work_item );

    return 0;
}

/* MAKE_EXPORT RegisterWaitForSingleObject_new=RegisterWaitForSingleObject */
BOOL WINAPI RegisterWaitForSingleObject_new(PHANDLE phNewWaitObject, HANDLE hObject, WAITORTIMERCALLBACK Callback, 
											PVOID Context, ULONG dwMilliseconds, ULONG dwFlags)
{
	//validate stuff first. we aren't Wine.
	if (!phNewWaitObject || IsBadCodePtr((FARPROC)Callback) || WaitForSingleObject(hObject,0) == WAIT_FAILED)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	wait_work_item_ptr wait_work_item;
	wait_work_item = (wait_work_item_ptr)HeapAlloc(GetProcessHeap(), 0, sizeof(*wait_work_item));

    if (!wait_work_item)
	{
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
	}
	
	if ( dwMilliseconds == 0 ) dwFlags |= WT_EXECUTEONLYONCE;
    wait_work_item->Object = hObject;
    wait_work_item->Callback = Callback;
    wait_work_item->Context = Context;
    wait_work_item->Milliseconds = dwMilliseconds;
    wait_work_item->Flags = dwFlags;
    wait_work_item->CallbackInProgress = FALSE;
    wait_work_item->DeleteCount = 0;
    wait_work_item->CompletionEvent = NULL;
	wait_work_item->CancelEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if (!wait_work_item->CancelEvent)
    {
        HeapFree( GetProcessHeap(), 0, wait_work_item );
        return FALSE;
    }

    if ( !QueueUserWorkItem_new(wait_thread_proc, wait_work_item, dwFlags) )
    {
        delete_wait_work_item( wait_work_item );
        return FALSE;
    }

    *phNewWaitObject = (HANDLE)wait_work_item;
    return TRUE;		
}

/* MAKE_EXPORT RegisterWaitForSingleObjectEx_new=RegisterWaitForSingleObjectEx */
HANDLE WINAPI RegisterWaitForSingleObjectEx_new(HANDLE hObject, 
		WAITORTIMERCALLBACK Callback, PVOID Context,
		ULONG dwMilliseconds, ULONG dwFlags)
{
	HANDLE retHandle;
	if ( RegisterWaitForSingleObject_new(&retHandle,hObject,Callback,Context,dwMilliseconds,dwFlags) )
		return retHandle;
	else
		return NULL;
}

/* MAKE_EXPORT UnregisterWaitEx_new=UnregisterWaitEx */
BOOL WINAPI UnregisterWaitEx_new(HANDLE WaitHandle, HANDLE CompletionEvent) 
{
	if (!WaitHandle)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return FALSE;
	}
	BOOL pending = FALSE;
	wait_work_item_ptr wait_work_item = (wait_work_item_ptr)WaitHandle;
    if ( !SetEvent( wait_work_item->CancelEvent ) ) //signal cancel
		return FALSE;
    if (wait_work_item->CallbackInProgress)
    {
        if (CompletionEvent != NULL)
        {
            if (CompletionEvent == INVALID_HANDLE_VALUE)
            {
                CompletionEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
                if (!CompletionEvent)
					return FALSE;
                InterlockedExchangePointer( &wait_work_item->CompletionEvent, CompletionEvent );
                if (wait_work_item->CallbackInProgress)
                    WaitForSingleObject( CompletionEvent, INFINITE );
                CloseHandle( CompletionEvent );
            }
            else
            {
                InterlockedExchangePointer( &wait_work_item->CompletionEvent, CompletionEvent );
				if (wait_work_item->CallbackInProgress)
					pending = TRUE;
           }
        }
        else
            pending = TRUE;
    }

    if ( InterlockedIncrement(&wait_work_item->DeleteCount) == 2 )
    {
		pending = FALSE; //somehow callback is complete already
        delete_wait_work_item( wait_work_item );
    }
	if ( pending )
	{
		SetLastError(ERROR_IO_PENDING);
		return FALSE;
	}

    return TRUE;
	
}

/* MAKE_EXPORT UnregisterWait_new=UnregisterWait */
BOOL WINAPI UnregisterWait_new(HANDLE WaitHandle)
{
	return UnregisterWaitEx_new(WaitHandle,NULL);
}
