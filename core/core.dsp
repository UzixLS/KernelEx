# Microsoft Developer Studio Project File - Name="Core" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=Core - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "core.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core.mak" CFG="Core - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Core - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Core - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Core - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KERNELEX_EXPORTS" /YX /FD /c
# ADD CPP /nologo /W3 /O2 /I "." /I "../common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEXCORE_EXPORTS" /YX /FD /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /i "../common" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib comctl32.lib ..\kexcrt\kexcrt.lib libc.lib /nologo /entry:"PreDllMain@12" /dll /map /machine:I386 /nodefaultlib /out:"Release/KernelEx.dll" /implib:"../common/KernelEx.lib" /ignore:4092 /OPT:NOWIN98
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "Core - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KERNELEX_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /ML /W3 /Gm /Zi /Od /I "." /I "../common" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEXCORE_EXPORTS" /YX /FD /GZ /GF /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /i "../common" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib advapi32.lib comctl32.lib ..\kexcrt\kexcrt.lib libc.lib /nologo /entry:"PreDllMain@12" /dll /incremental:no /map /debug /machine:I386 /nodefaultlib /out:"Debug/KernelEx.dll" /implib:"../common/KernelEx.lib" /ignore:4092 /OPT:NOWIN98
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "Core - Win32 Release"
# Name "Core - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\apiconf.cpp
# End Source File
# Begin Source File

SOURCE=.\apiconfmgr.cpp
# End Source File
# Begin Source File

SOURCE=.\apilib.cpp
# End Source File
# Begin Source File

SOURCE=.\apilog.cpp

!IF  "$(CFG)" == "Core - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Core - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core.def

!IF  "$(CFG)" == "Core - Win32 Release"

# Begin Custom Build
OutDir=.\Release
WkspDir=.
InputPath=.\core.def

"$(OutDir)\k32ord.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /c /TC /DK32ORD_IMPLIB /Fo$(OutDir)\k32ord.obj "$(WkspDir)\common\k32ord.h" 
	link /DLL /NOENTRY /NOLOGO /IGNORE:4070 /MACHINE:IX86 /DEF:"$(WkspDir)\common\k32ord.def" /OUT:$(OutDir)\k32ord.dll /IMPLIB:$(OutDir)\k32ord.lib $(OutDir)\k32ord.obj 
	del $(OutDir)\k32ord.exp 
	del $(OutDir)\k32ord.obj 
	del $(OutDir)\k32ord.dll 
	
# End Custom Build

!ELSEIF  "$(CFG)" == "Core - Win32 Debug"

# Begin Custom Build
OutDir=.\Debug
WkspDir=.
InputPath=.\core.def

"$(OutDir)\k32ord.lib" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /c /TC /DK32ORD_IMPLIB /Fo$(OutDir)\k32ord.obj "$(WkspDir)\common\k32ord.h" 
	link /DLL /NOENTRY /NOLOGO /IGNORE:4070 /MACHINE:IX86 /DEF:"$(WkspDir)\common\k32ord.def" /OUT:$(OutDir)\k32ord.dll /IMPLIB:$(OutDir)\k32ord.lib $(OutDir)\k32ord.obj 
	del $(OutDir)\k32ord.exp 
	del $(OutDir)\k32ord.obj 
	del $(OutDir)\k32ord.dll 
	
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debug.cpp

!IF  "$(CFG)" == "Core - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Core - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\debugproto.cpp

!IF  "$(CFG)" == "Core - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Core - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\DebugWindow.cpp

!IF  "$(CFG)" == "Core - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "Core - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\internals.cpp
# End Source File
# Begin Source File

SOURCE=.\kexcoresdk.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=..\common\pemanip.cpp
# End Source File
# Begin Source File

SOURCE=.\resolver.cpp
# End Source File
# Begin Source File

SOURCE=.\SettingsDB.cpp
# End Source File
# Begin Source File

SOURCE=.\sharedmem.cpp
# End Source File
# Begin Source File

SOURCE=.\storage.cpp
# End Source File
# Begin Source File

SOURCE=.\thunks.cpp
# End Source File
# Begin Source File

SOURCE=.\wildcmp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\apiconf.h
# End Source File
# Begin Source File

SOURCE=.\apiconfmgr.h
# End Source File
# Begin Source File

SOURCE=.\apilib.h
# End Source File
# Begin Source File

SOURCE=.\apilog.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\debugproto.h
# End Source File
# Begin Source File

SOURCE=.\DebugWindow.h
# End Source File
# Begin Source File

SOURCE=.\internals.h
# End Source File
# Begin Source File

SOURCE=..\common\is_sorted.hpp
# End Source File
# Begin Source File

SOURCE=..\common\k32ord.def
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\common\k32ord.h
# End Source File
# Begin Source File

SOURCE=..\common\kexcoresdk.h
# End Source File
# Begin Source File

SOURCE=..\common\pemanip.h
# End Source File
# Begin Source File

SOURCE=.\resolver.h
# End Source File
# Begin Source File

SOURCE=.\SettingsDB.h
# End Source File
# Begin Source File

SOURCE=.\sharedmem.h
# End Source File
# Begin Source File

SOURCE=..\common\sstring.hpp
# End Source File
# Begin Source File

SOURCE=.\storage.h
# End Source File
# Begin Source File

SOURCE=.\structs.h
# End Source File
# Begin Source File

SOURCE=.\thunks.h
# End Source File
# Begin Source File

SOURCE=..\common\version.h
# End Source File
# Begin Source File

SOURCE=.\wildcmp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\core.rc
# End Source File
# End Group
# End Target
# End Project
