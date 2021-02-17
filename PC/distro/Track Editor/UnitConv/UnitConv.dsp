# Microsoft Developer Studio Project File - Name="UnitConv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=UnitConv - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "UnitConv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "UnitConv.mak" CFG="UnitConv - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "UnitConv - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "UnitConv - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Tools/Track Editor/UnitConv", KUGAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "UnitConv - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir ".\Release"
# PROP BASE Intermediate_Dir ".\Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ".\release"
# PROP Intermediate_Dir ".\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "." /I "Custom" /I "..\game" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 baclass.lib /nologo /subsystem:windows /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy release\unitconv.exe T:\Re-Volt\T_Ed_art\rtu	copy         release\jcclassd.dll t:\re-volt\t_ed_art\rtu
# End Special Build Tool

!ELSEIF  "$(CFG)" == "UnitConv - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir ".\Debug"
# PROP BASE Intermediate_Dir ".\Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir ".\debug"
# PROP Intermediate_Dir ".\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "." /I "Custom" /I "..\game" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x809 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 baclassd.lib /nologo /subsystem:windows /debug /machine:I386
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy debug\UnitConv.exe t:\re-volt\pc\editor	copy debug\jcclassd.dll t:\re-volt\pc\editor	copy C:\windows\system\mfc42d.dll t:\re-volt\pc\editor	copy C:\windows\system\msvcp60d.dll t:\re-volt\pc\editor	copy C:\windows\system\msvcrtd.dll t:\re-volt\pc\editor
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "UnitConv - Win32 Release"
# Name "UnitConv - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Group "Custom"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Custom\OutputUnitFile.cpp
# End Source File
# Begin Source File

SOURCE=.\Custom\OutputWaveFront.cpp
# End Source File
# Begin Source File

SOURCE=.\Custom\Primitives.cpp

!IF  "$(CFG)" == "UnitConv - Win32 Release"

!ELSEIF  "$(CFG)" == "UnitConv - Win32 Debug"

# ADD CPP /I "..Custom" /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\custom\RevoltModule.cpp
# End Source File
# Begin Source File

SOURCE=.\Custom\ScriptObject.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\HabExtract.cpp
# End Source File
# Begin Source File

SOURCE=.\HabExtract.rc
# End Source File
# Begin Source File

SOURCE=.\HabExtractDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\HelpDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\LogDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\PROPNAME.CPP
# End Source File
# Begin Source File

SOURCE=.\PSX_Polygon.CPP
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\user.cpp

!IF  "$(CFG)" == "UnitConv - Win32 Release"

!ELSEIF  "$(CFG)" == "UnitConv - Win32 Debug"

# ADD CPP /W3

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\JCClass.lib

!IF  "$(CFG)" == "UnitConv - Win32 Release"

!ELSEIF  "$(CFG)" == "UnitConv - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\JCClassD.lib

!IF  "$(CFG)" == "UnitConv - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "UnitConv - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Group "Custom Headers"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\custom\constants.h
# End Source File
# Begin Source File

SOURCE=.\Custom\MappedVector.h
# End Source File
# Begin Source File

SOURCE=.\Custom\NamedLists.h
# End Source File
# Begin Source File

SOURCE=.\Custom\OutputUnitFile.h
# End Source File
# Begin Source File

SOURCE=.\Custom\OutputWaveFront.h
# End Source File
# Begin Source File

SOURCE=.\Custom\Primitives.h
# End Source File
# Begin Source File

SOURCE=.\Custom\ScriptObject.h
# End Source File
# Begin Source File

SOURCE=.\custom\structs.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\AD_LightType.h
# End Source File
# Begin Source File

SOURCE=.\AD_Link.h
# End Source File
# Begin Source File

SOURCE=.\BA_3DViewFilter.h
# End Source File
# Begin Source File

SOURCE=.\BA_3DViewStyle.h
# End Source File
# Begin Source File

SOURCE=.\data.h
# End Source File
# Begin Source File

SOURCE=.\database.h
# End Source File
# Begin Source File

SOURCE=..\..\bobclass\inc\endstrm.h
# End Source File
# Begin Source File

SOURCE=.\Habextract.h
# End Source File
# Begin Source File

SOURCE=.\HabextractDlg.h
# End Source File
# Begin Source File

SOURCE=.\Helpdialog.h
# End Source File
# Begin Source File

SOURCE=.\JCClasses.h
# End Source File
# Begin Source File

SOURCE=.\JCDialog.h
# End Source File
# Begin Source File

SOURCE=.\JCGenericUndo.h
# End Source File
# Begin Source File

SOURCE=.\JCINCLUDES.H
# End Source File
# Begin Source File

SOURCE=.\JCTracker.h
# End Source File
# Begin Source File

SOURCE=.\jctypes.h
# End Source File
# Begin Source File

SOURCE=.\Logdialog.h
# End Source File
# Begin Source File

SOURCE=.\PROPNAME.H
# End Source File
# Begin Source File

SOURCE=.\PS_Line.h
# End Source File
# Begin Source File

SOURCE=.\PS_UniqueRecordList.H
# End Source File
# Begin Source File

SOURCE=.\PSX_Polygon.H
# End Source File
# Begin Source File

SOURCE=.\rv_ObjAnimation.h
# End Source File
# Begin Source File

SOURCE=.\rv_ObjTransformation.h
# End Source File
# Begin Source File

SOURCE=.\STDAFX.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\HabExtract.ico
# End Source File
# Begin Source File

SOURCE=.\res\HabExtract.rc2
# End Source File
# End Group
# End Target
# End Project
