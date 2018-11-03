#ifndef __STRUCTS_H
#define __STRUCTS_H

#define WIN98_K32OBJ_SEMAPHORE            0x1
#define WIN98_K32OBJ_EVENT                0x2
#define WIN98_K32OBJ_MUTEX                0x3
#define WIN98_K32OBJ_CRITICAL_SECTION     0x4
#define WIN98_K32OBJ_CHANGE               0x5
#define WIN98_K32OBJ_PROCESS              0x6
#define WIN98_K32OBJ_THREAD               0x7
#define WIN98_K32OBJ_FILE                 0x8
#define WIN98_K32OBJ_CONSOLE              0x9
#define WIN98_K32OBJ_SCREEN_BUFFER        0xA
#define WIN98_K32OBJ_MAILSLOT             0xB
#define WIN98_K32OBJ_SERIAL               0xC
#define WIN98_K32OBJ_MEM_MAPPED_FILE      0xD
#define WIN98_K32OBJ_PIPE                 0xE
#define WIN98_K32OBJ_DEVICE_IOCTL         0xF
#define WIN98_K32OBJ_TOOLHELP_SNAPSHOT    0x10
#define WIN98_K32OBJ_SOCKET               0x11

// Process Database flags (WIN95)
#define fDebugSingle                  0x00000001  // Set if process is being debugged
#define fCreateProcessEvent           0x00000002  // Set in debugged process after starting
#define fExitProcessEvent             0x00000004  // Might be set in debugged process at exit time
#define fWin16Process                 0x00000008  // 16-bit process
#define fDosProcess                   0x00000010  // DOS process
#define fConsoleProcess               0x00000020  // 32-bit console process
#define fFileApisAreOem               0x00000040  // SetFileAPIsToOEM
#define fNukeProcess                  0x00000080
#define fServiceProcess               0x00000100  // RegisterServiceProcess
#define fLoginScriptHack              0x00000800  // Might be a Novell network login process
#define fSendDllNotifications         0x00200000
#define fDebugEventPending            0x00400000  // e.g. stopped in debugger
#define fNearlyTerminating            0x00800000
#define fFaulted                      0x08000000
#define fTerminating                  0x10000000
#define fTerminated                   0x20000000
#define fInitError                    0x40000000
#define fSignaled                     0x80000000

#pragma pack(push,1)

// Structured Exception Handler
typedef struct _SEH {
    struct _SEH *pNext;
    FARPROC     pfnHandler;
} SEH, *PSEH;

typedef struct _HANDLE_TABLE_ENTRY {
    DWORD  flags;                   // Valid flags depend on what type of object this is
    PVOID  pObject;                 // Pointer to the object that the handle refers to
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

// Handle Table
typedef struct _HANDLE_TABLE {
    DWORD cEntries;                 // Max number of handles in table
    HANDLE_TABLE_ENTRY array[1];    // An array (number is given by cEntries)
} HANDLE_TABLE, *PHANDLE_TABLE;

struct _PDB98;

// MODREF
typedef struct _MODREF {          // Size = 0x1C + 4*cImportedModules
    struct _MODREF*  pNextModRef; // 00 Pointer to next process's MODREF in list
    struct _MODREF*  pPrevModRef; // 04 Pointer to previous process's MODREF in list
    struct _MODREF*  pNextMteMR;  // 08 Next MODREF in IMTE list ??
    struct _MODREF*  pPrevMteMR;  // 0C Prev MODREF in IMTE list ??
    WORD             mteIndex;    // 10 Index to global array of pointers to IMTEs
    WORD             cUsage;      // 12 ref count
    WORD             flags;       // 14 flags
    WORD             cImportedModules; // 16 Number of modules imported implicitly
    struct _PDB98*   ppdb;        // 18 Pointer to process database
	union {                       // 1C
		PSTR         pszModuleName;
		struct _MODREF*  pMR;
	} ImplicitImports[1];         // * cImportedModules
} MODREF, *PMODREF;

// IMTE
typedef struct _IMTE {            // Size = 0x3C
    DWORD           unknown1;     // 00
    IMAGE_NT_HEADERS*   pNTHdr;   // 04 pointer to shared PE header for module
    DWORD           unknown2;     // 08
    PSTR            pszFileName;  // 0C long path name
    PSTR            pszModName;   // 10 long module name
    WORD            cbFileName;   // 14 long path name length
    WORD            cbModName;    // 16 long module name length
    DWORD           unknown3;     // 18
    DWORD           cSections;    // 1C number of sections in the module
    DWORD           unknown4;     // 20
    DWORD           baseAddress;  // 24 module base address before relocation
    WORD            hModule16;    // 28 16-bit selector to NE header
    WORD            cUsage;       // 2A usage count
    PMODREF         pMR;          // 2C pointer to MODREF structure
    PSTR            pszSFileName; // 30 short path name
    WORD            cbSFileName;  // 34 short path name length
    PSTR            pszSModName;  // 36 short module name
    WORD            cbSModName;   // 3A short module name length
} IMTE, *PIMTE;

// Environment Database
typedef struct _ENVIRONMENT_DATABASE {
    PSTR    pszEnvironment;             //00 Pointer to Process Environment
    DWORD   un1;                        //04 (always 0)
    PSTR    pszCmdLine;                 //08 Pointer to command line
    PSTR    pszCurrDirectory;           //0C Pointer to current directory
    LPSTARTUPINFOA pStartupInfo;        //10 Pointer to STARTUPINFOA struct
    HANDLE  hStdIn;                     //14 Standard Input handle
    HANDLE  hStdOut;                    //18 Standard Output handle
    HANDLE  hStdErr;                    //1C Standard Error handle
    DWORD   un2;                        //20 (always 1)
    DWORD   InheritConsole;             //24 Inherit console from parent
    DWORD   BreakType;                  //28 Handle console events (like CTRL+C)
    DWORD   BreakSem;                   //2C Pointer to K32OBJ_SEMAPHORE
    DWORD   BreakEvent;                 //30 Pointer to K32OBJ_EVENT
    DWORD   BreakThreadID;              //34 Pointer to K32OBJ_THREAD
    DWORD   BreakHandlers;              //38 Pointer to list of installed console control handlers
} EDB, *PEDB;

// Process Database
typedef struct _PDB98 {                 // Size = 0xC4 (from Kernel32)
    BYTE    Type;                       // 00 Kernel object type = K32OBJ_PROCESS (6)
    BYTE    Unknown_A;                  // 01 (align ?)
    WORD    cReference;                 // 02 Number of references to process
    DWORD   Unknown_B;                  // 04 Pointer to ???
    DWORD   Unknown1;                   // 08 (zero)
    DWORD   pEvent;                     // 0C Event for process waiting
    DWORD   TerminationStatus;          // 10 GetExitCodeProcess
    DWORD   Unknown2;                   // 14 May be used for private purposes
    HANDLE  DefaultHeap;                // 18 GetProcessHeap
    PCONTEXT MemoryContext;             // 1C Pointer to process context
    DWORD   Flags;                      // 20 Flags
    DWORD   pPSP;                       // 24 Linear address of DOS PSP
    WORD    PSPSelector;                // 28 Selector to DOS PSP
    WORD    MTEIndex;                   // 2A Index into global module table
    WORD    cThreads;                   // 2C Threads.ItemCount
    WORD    cNotTermThreads;            // 2E Threads.ItemCount
    WORD    Unknown3;                   // 30 (zero)
    WORD    cRing0Threads;              // 32 Normally Threads.ItemCount (except kernel32)
    HANDLE  HeapHandle;                 // 34 Kernel32 shared heap
    DWORD   w16TDB;                     // 38 Win16 task database selector
    DWORD   MemMappedFiles;             // 3C List of memory mapped files
    PEDB    pEDB;                       // 40 Pointer to Environment Database
    PHANDLE_TABLE pHandleTable;         // 44 Pointer to Handle Table
    struct _PDB98* ParentPDB;           // 48 Pointer to parent process (PDB)
    PMODREF MODREFList;                 // 4C Pointer to list of modules
    DWORD   ThreadList;                 // 50 Pointer to list of threads
    DWORD   DebuggeeCB;                 // 54 Debuggee context block
    DWORD   LocalHeapFreeHead;          // 58 Free list for process default heap
    DWORD   InitialRing0ID;             // 5C Meaning unknown
    CRITICAL_SECTION CriticalSection;   // 60 For synchronizing threads
    DWORD   Unknown4[3];                // 78
    DWORD   pConsole;                   // 84 Output console
    DWORD   tlsInUseBits1;              // 88 Status of TLS indexes  0 - 31
    DWORD   tlsInUseBits2;              // 8C Status of TLS indexes 32 - 63
    DWORD   ProcessDWORD;               // 90 Undocumented API GetProcessDword, meaning unknown
    struct _PDB98* ProcessGroup;        // 94 Master process PDB (in debugging)
    PMODREF pExeMODREF;                 // 98 Points to exe's module structure
    DWORD   TopExcFilter;               // 9C SetUnhandledExceptionFilter
    DWORD   PriorityClass;              // A0 PriorityClass (8 = NORMAL)
    DWORD   HeapList;                   // A4 List of heaps
    DWORD   HeapHandleList;             // A8 List of moveable memory blocks
    DWORD   HeapPointer;                // AC Pointer to one moveable memory block, meaning unknown
    DWORD   pConsoleProvider;           // B0 Console for DOS apps
    WORD    EnvironSelector;            // B4 Environment database selector
    WORD    ErrorMode;                  // B6 SetErrorMode
    DWORD   pEventLoadFinished;         // B8 Signaled when the process has finished loading
    WORD    UTState;                    // BC Universal thunking, meaning unknown
    WORD    Unknown5;                   // BE (zero)
    DWORD   Unknown6;                   // C0
} PDB98, *PPDB98;

// Process Database
typedef struct _PDBME {                 // Size = 0xC4 (from Kernel32)
    BYTE    Type;                       // 00 Kernel object type = K32OBJ_PROCESS (6)
    BYTE    Unknown_A;                  // 01 (align ?)
    WORD    cReference;                 // 02 Number of references to process
    DWORD   Unknown_B;                  // 04 Pointer to ???
    DWORD   Unknown1;                   // 08 (zero)
    DWORD   pEvent;                     // 0C Event for process waiting
    DWORD   TerminationStatus;          // 10 GetExitCodeProcess
    DWORD   Unknown2;                   // 14 May be used for private purposes
    DWORD   DefaultHeap;                // 18 GetProcessHeap
    DWORD   MemoryContext;              // 1C Pointer to process context
    DWORD   Flags;                      // 20 Flags
    DWORD   pPSP;                       // 24 Linear address of DOS PSP
    WORD    PSPSelector;                // 28 Selector to DOS PSP
    WORD    MTEIndex;                   // 2A Index into global module table
    WORD    cThreads;                   // 2C Threads.ItemCount
    WORD    cNotTermThreads;            // 2E Threads.ItemCount
    WORD    Unknown3;                   // 30 (zero)
    WORD    cRing0Threads;              // 32 Normally Threads.ItemCount (except kernel32)
    HANDLE  HeapHandle;                 // 34 Kernel32 shared heap
    DWORD   w16TDB;                     // 38 Win16 task database selector
    DWORD   MemMappedFiles;             // 3C List of memory mapped files
    PEDB    pEDB;                       // 40 Pointer to Environment Database
    PHANDLE_TABLE pHandleTable;         // 44 Pointer to Handle Table
    struct _PDBME* ParentPDB;            // 48 Pointer to parent process (PDB)
    PMODREF MODREFList;                 // 4C Pointer to list of modules
    DWORD   ThreadList;                 // 50 Pointer to list of threads
    DWORD   DebuggeeCB;                 // 54 Debuggee context block
    DWORD   LocalHeapFreeHead;          // 58 Free list for process default heap
    DWORD   InitialRing0ID;             // 5C Meaning unknown
    CRITICAL_SECTION CriticalSection;   // 60 For synchronizing threads
    DWORD   Unknown4[2];                // 78
    DWORD   pConsole;                   // 80 Output console
    DWORD   tlsInUseBits1;              // 84 Status of TLS indexes  0 - 31
    DWORD   tlsInUseBits2;              // 88 Status of TLS indexes 32 - 63
    DWORD   ProcessDWORD;               // 8C Undocumented API GetProcessDword, meaning unknown
    DWORD   Unknown_C;                  // 90 Unknown
    struct _PDBME* ProcessGroup;         // 94 Master process PDB (in debugging)
    PMODREF pExeMODREF;                 // 98 Points to exe's module structure
    DWORD   TopExcFilter;               // 9C SetUnhandledExceptionFilter
    DWORD   PriorityClass;              // A0 PriorityClass (8 = NORMAL)
    DWORD   HeapList;                   // A4 List of heaps
    DWORD   HeapHandleList;             // A8 List of moveable memory blocks
    DWORD   HeapPointer;                // AC Pointer to one moveable memory block, meaning unknown
    DWORD   pConsoleProvider;           // B0 Console for DOS apps
    WORD    EnvironSelector;            // B4 Environment database selector
    WORD    ErrorMode;                  // B6 SetErrorMode
    DWORD   pEventLoadFinished;         // B8 Signaled when the process has finished loading
    WORD    UTState;                    // BC Universal thunking, meaning unknown
    WORD    Unknown5;                   // BE (zero)
    DWORD   Unknown6;                   // C0
} PDBME, *PPDBME;

// Thread Information Block (FS:[0x18])
typedef struct _TIB98 {        // Size = 0x38
    PSEH    pvExcept;          // 00 Head of exception record list
    PVOID   pvStackUserTop;    // 04 Top of user stack
    PVOID   pvStackUserBase;   // 08 Base of user stack
    WORD    pvTDB;             // 0C Ptr to win-16 task database
    WORD    pvThunksSS;        // 0E SS selector used for thunking to 16 bits
    DWORD   SelmanList;        // 10 Pointer to selector manager list
    PVOID   pvArbitrary;       // 14 Available for application use
    struct _TIB98 *pTIBSelf;   // 18 Linear address of TIB structure
    WORD    TIBFlags;          // 1C TIBF_WIN32 = 1, TIBF_TRAP = 2
    WORD    Win16MutexCount;   // 1E Win16Lock
    DWORD   DebugContext;      // 20 Pointer to debug context structure
    DWORD   pCurrentPriority;  // 24 Pointer to DWORD containing current priority level
    DWORD   pvQueue;           // 28 Message Queue selector
    PVOID   *pvTLSArray;       // 2C Thread Local Storage (TLS) array
    PVOID   *pProcess;         // 30 Pointer to owning process database (PDB)
    DWORD   Unknown;           // 34 Pointer to ???
} TIB98, *PTIB98;

typedef struct _TDBX98 TDBX98;

// Thread database (FS:[0x18] - 0x8)
typedef struct _TDB98 {        // Size = 0x228 (from Kernel32)
    WORD    Type;              // 00 K32 object type
    WORD    cReference;        // 02 Reference count
    DWORD   pSomeEvent;        // 04 K32 event object used when someone waits on the thread object
    TIB98   tib;               // 08 Thread information block (TIB)
    DWORD   Unknown;           // 40
    DWORD   Flags;             // 44 Flags
    DWORD   TerminationStatus; // 48 Exit code
    WORD    TIBSelector;       // 4C Selector used in FS to point to TIB
    WORD    EmulatorSelector;  // 4E Memory block for saving x87 state
    DWORD   cHandles;          // 50 Handle count
    DWORD   Ring0Thread;       // 54 R0 thread control block (TCB)
    TDBX98  *pTDBX;            // 58 R0 thread database extension (TDBX)
    DWORD   un1[109];          // 5C
    DWORD   APISuspendCount;   // 210 Count of SuspendThread's minus ResumeThread's
} TDB98, *PTDB98;

typedef struct _TDBME {        // Size = 0x228 (from Kernel32)
    WORD    Type;              // 00 K32 object type
    WORD    cReference;        // 02 Reference count
    DWORD   pSomeEvent;        // 04 K32 event object used when someone waits on the thread object
    TIB98   tib;               // 08 Thread information block (TIB)
    DWORD   Unknown;           // 40
    DWORD   Unknown2;          // 44
    WORD    TIBSelector;       // 46 Selector used in FS to point to TIB
    DWORD   TerminationStatus; // 48 Exit code
    DWORD   Flags;             // 4C Flags
    DWORD   cHandles;          // 50 Handle count
    DWORD   Ring0Thread;       // 54 R0 thread control block (TCB)
    DWORD   Unknown3;          // 58 Selector for ???
    DWORD   un1[11];           // 5C
    TDBX98  *pTDBX;            // 88
    DWORD   un2[97];           // 8C
    DWORD   APISuspendCount;   // 210 Count of SuspendThread's minus ResumeThread's
} TDBME, *PTDBME;

// Thread database extension
typedef struct _TDBX98 {
    DWORD  un0;                // 00
    TDB98  *ptdb;              // 04 R3 thread database
    PDB98  *ppdb;              // 08 R3 process database
    DWORD  ContextHandle;      // 0C R0 memory context
    DWORD  Ring0Thread;        // 10 R0 thread control block [TCB *]
    DWORD  WaitNodeList;       // 14 Anchor of things we're waiting on  [WAITNODE *]
    DWORD  WaitFlags;          // 18 Blocking flags
    DWORD  un1;                // 1C
    DWORD  TimeOutHandle;      // 20
    DWORD  WakeParam;          // 24
    DWORD  BlockHandle;        // 28 R0 semaphore on which thread will wait inside VWIN32
    DWORD  BlockState;         // 2C
    DWORD  SuspendCount;       // 30
    DWORD  SuspendHandle;      // 34
    DWORD  MustCompleteCount;  // 38 Count of EnterMustComplete's minus LeaveMustComplete's
    DWORD  WaitExFlags;        // 3C Flags
    DWORD  SyncWaitCount;      // 40
    DWORD  QueuedSyncFuncs;    // 44
    DWORD  UserAPCList;        // 48
    DWORD  KernAPCList;        // 4C
    DWORD  pPMPSPSelector;     // 50
    DWORD  BlockedOnID;        // 54
} TDBX98, *PTDBX98;

#pragma pack(pop)

#endif
