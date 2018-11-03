# Microsoft Developer Studio Project File - Name="KernelEx Base NonShared" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=KernelEx Base NonShared - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "kexbasen.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "kexbasen.mak" CFG="KernelEx Base NonShared - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "KernelEx Base NonShared - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "KernelEx Base NonShared - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "KernelEx Base NonShared - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEXBASEN_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "." /I "../../common" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D _WIN32_WINNT=0x0500 /YX /FD /TP /GF /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /i "../../common" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shlwapi.lib shell32.lib rpcrt4.lib usp10.lib ../../common/KernelEx.lib ../../kexcrt/kexcrt.lib libc.lib delayimp.lib /nologo /dll /map /machine:I386 /nodefaultlib /OPT:NOWIN98 /DELAYLOAD:shell32.dll /DELAYLOAD:rpcrt4.dll /DELAYLOAD:usp10.dll /DELAYLOAD:comdlg32.dll /DELAYLOAD:winspool.drv /DELAYLOAD:shlwapi.dll
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "KernelEx Base NonShared - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "KEXBASEN_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /Zi /Od /I "." /I "../../common" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D _WIN32_WINNT=0x0500 /YX /FD /TP /GZ /GF /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /i "../../common" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shlwapi.lib shell32.lib rpcrt4.lib usp10.lib ../../common/KernelEx.lib ../../kexcrt/kexcrt.lib libc.lib delayimp.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib /pdbtype:sept /OPT:NOWIN98 /DELAYLOAD:shell32.dll /DELAYLOAD:rpcrt4.dll /DELAYLOAD:usp10.dll /DELAYLOAD:comdlg32.dll /DELAYLOAD:winspool.drv /DELAYLOAD:shlwapi.dll
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "KernelEx Base NonShared - Win32 Release"
# Name "KernelEx Base NonShared - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "kernel32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\kernel32\_kernel32_apilist.c
# End Source File
# Begin Source File

SOURCE=.\kernel32\_kernel32_apilist.h
# End Source File
# Begin Source File

SOURCE=.\kernel32\unikernel32.c
# End Source File
# End Group
# Begin Group "user32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\user32\_user32_apilist.c
# End Source File
# Begin Source File

SOURCE=.\user32\_user32_apilist.h
# End Source File
# Begin Source File

SOURCE=.\user32\uniuser32.c
# End Source File
# End Group
# Begin Group "gdi32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\gdi32\_gdi32_apilist.c
# End Source File
# Begin Source File

SOURCE=.\gdi32\_gdi32_apilist.h
# End Source File
# Begin Source File

SOURCE=.\gdi32\UberGDI.c
# End Source File
# Begin Source File

SOURCE=.\gdi32\unigdi32.c
# End Source File
# End Group
# Begin Group "advapi32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\advapi32\_advapi32_apilist.c
# End Source File
# Begin Source File

SOURCE=.\advapi32\_advapi32_apilist.h
# End Source File
# Begin Source File

SOURCE=.\advapi32\uniadvapi32.c
# End Source File
# End Group
# Begin Group "comdlg32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\comdlg32\_comdlg32_apilist.c
# End Source File
# Begin Source File

SOURCE=.\comdlg32\_comdlg32_apilist.h
# End Source File
# Begin Source File

SOURCE=.\comdlg32\openfilename_fix.c
# End Source File
# Begin Source File

SOURCE=.\comdlg32\unicomdlg32.c
# End Source File
# End Group
# Begin Group "shell32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shell32\_shell32_apilist.c
# End Source File
# Begin Source File

SOURCE=.\shell32\_shell32_apilist.h
# End Source File
# Begin Source File

SOURCE=.\shell32\auxshlguid.h
# End Source File
# Begin Source File

SOURCE=.\shell32\pidl.h
# End Source File
# Begin Source File

SOURCE=.\shell32\SHGetFolderLocation.c
# End Source File
# Begin Source File

SOURCE=.\shell32\SHGetFolderPath.c
# End Source File
# Begin Source File

SOURCE=.\shell32\unishell32.c
# End Source File
# End Group
# Begin Group "rpcrt4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\rpcrt4\_rpcrt4_apilist.c
# End Source File
# Begin Source File

SOURCE=.\rpcrt4\_rpcrt4_apilist.h
# End Source File
# Begin Source File

SOURCE=.\rpcrt4\unirpcrt4.c
# End Source File
# End Group
# Begin Group "winspool"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\winspool\_winspool_apilist.c
# End Source File
# Begin Source File

SOURCE=.\winspool\_winspool_apilist.h
# End Source File
# Begin Source File

SOURCE=.\winspool\DefaultPrinter.c
# End Source File
# End Group
# Begin Group "shlwapi"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\shlwapi\_shlwapi_apilist.c
# End Source File
# Begin Source File

SOURCE=.\shlwapi\_shlwapi_apilist.h
# End Source File
# Begin Source File

SOURCE=.\shlwapi\string.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\common.c
# End Source File
# Begin Source File

SOURCE=.\kexbasen.def
# End Source File
# Begin Source File

SOURCE=.\main.c
# End Source File
# Begin Source File

SOURCE=.\unifwd.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\unifwd.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\kexbasen.rc
# End Source File
# End Group
# End Target
# End Project
