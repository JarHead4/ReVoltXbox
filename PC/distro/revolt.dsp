# Microsoft Developer Studio Project File - Name="revolt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=revolt - Win32 Preprocess Only
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "revolt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "revolt.mak" CFG="revolt - Win32 Preprocess Only"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "revolt - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "revolt - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "revolt - Win32 Preprocess Only" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Game/BalorTest", DAAAAAAA"
# PROP Scc_LocalPath "."
CPP=snCl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "revolt - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /O2 /I "source\inc" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /I "c:\Dev\Re-Volt\rvPC\source\inc" /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 mss32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib d3drm.lib dxguid.lib winmm.lib dinput8.lib dsound.lib wsock32.lib /nologo /subsystem:windows /profile /machine:I386 /out:"C:\Dev\Re-Volt\RVPC\Release\revolt.exe" /libpath:"source\lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy executable to T
PostBuild_Cmds=copy Release\ReVolt.exe T:\Re-Volt\PC\Game\Chris\ReVolt.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "revolt - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /ZI /Od /I "source\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FAs /Fr /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /I "c:\Dev\Re-Volt\rvPC\source\inc" /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mss32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib d3drm.lib dxguid.lib winmm.lib dinput8.lib dsound.lib wsock32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"C:\Dev\Re-Volt\RVPC\Debug\Revoltd.exe" /libpath:"source\lib"
# SUBTRACT LINK32 /verbose
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copy to Network
PostBuild_Cmds=Copy debug\ReVoltD.exe T:\Re-Volt\PC\Game\Chris\ReVoltD.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "revolt - Win32 Preprocess Only"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "revolt__"
# PROP BASE Intermediate_Dir "revolt__"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "revolt__"
# PROP Intermediate_Dir "revolt__"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /G5 /W3 /GX /Zi /Od /I "c:\dev\re-volt\rvPC\source\inc" /I "c:\dev\re-volt\rvPC" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FAs /Fr /YX /FD /c
# ADD CPP /nologo /G5 /W3 /GX /ZI /Od /I "source\inc" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /P /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /I "c:\Dev\Re-Volt\rvPC\source\inc" /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /I "c:\Dev\Re-Volt\rvPC\source\inc" /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=snLink.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib d3drm.lib dxguid.lib winmm.lib dinput.lib dsound.lib wsock32.lib /nologo /subsystem:windows /profile /map /debug /machine:I386 /out:"C:\Dev\Re-Volt\RVPC\Debug\Revoltd.exe"
# SUBTRACT BASE LINK32 /verbose
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ddraw.lib d3drm.lib dxguid.lib winmm.lib dinput8.lib dsound.lib wsock32.lib mss32.lib /nologo /subsystem:windows /incremental:no /machine:I386 /out:"C:\Dev\Re-Volt\RVPC\Debug\Revoltd.exe" /libpath:"source\lib"
# SUBTRACT LINK32 /verbose /profile /map /debug

!ENDIF 

# Begin Target

# Name "revolt - Win32 Release"
# Name "revolt - Win32 Debug"
# Name "revolt - Win32 Preprocess Only"
# Begin Group "Include Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\source\inc\Aerial.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\ai.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\ai_car.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\ai_init.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\ainode.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\aizone.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Body.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\camera.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\car.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Control.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\ctrlread.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Debug.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\draw.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\DrawObj.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\dx.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\dxerrors.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\edfield.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\editai.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\editcam.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\editobj.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\edittrig.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\editzone.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Field.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\gameloop.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Gaussian.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Geom.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Ghost.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\grid.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\input.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\instance.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\level.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\light.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\main.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\mirror.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\model.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\move.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\mss.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\NewColl.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\obj_init.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\object.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\panel.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Particle.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\piano.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\play.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\player.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\ReadInit.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\registry.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\revolt.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\sfx.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\shadow.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\smoke.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Spark.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\text.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\texture.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\timing.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\trigger.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\TypeDefs.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Util.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\visibox.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\Wheel.h
# End Source File
# Begin Source File

SOURCE=.\source\inc\world.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\source\ACM_Simplex..cpp
# End Source File
# Begin Source File

SOURCE=.\source\Aerial.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ai_car.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ai_init.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ainode.cpp
# End Source File
# Begin Source File

SOURCE=.\source\aizone.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Body.cpp
# End Source File
# Begin Source File

SOURCE=.\source\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\source\car.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Control.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ctrlread.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Debug.cpp
# End Source File
# Begin Source File

SOURCE=.\source\draw.cpp
# End Source File
# Begin Source File

SOURCE=.\source\DrawObj.cpp
# End Source File
# Begin Source File

SOURCE=.\source\dx.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Dxerrors.cpp
# End Source File
# Begin Source File

SOURCE=.\source\edfield.cpp
# End Source File
# Begin Source File

SOURCE=.\source\editai.cpp
# End Source File
# Begin Source File

SOURCE=.\source\editcam.cpp
# End Source File
# Begin Source File

SOURCE=.\source\editobj.cpp
# End Source File
# Begin Source File

SOURCE=.\source\edittrig.cpp
# End Source File
# Begin Source File

SOURCE=.\source\editzone.cpp
# End Source File
# Begin Source File

SOURCE=.\source\edportal.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Field.cpp
# End Source File
# Begin Source File

SOURCE=.\source\gameloop.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Gaussian.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Geom.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Ghost.cpp
# End Source File
# Begin Source File

SOURCE=.\source\grid.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Input.cpp
# End Source File
# Begin Source File

SOURCE=.\source\instance.cpp
# End Source File
# Begin Source File

SOURCE=.\source\level.cpp
# End Source File
# Begin Source File

SOURCE=.\source\light.cpp
# End Source File
# Begin Source File

SOURCE=.\source\main.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Menu.cpp
# End Source File
# Begin Source File

SOURCE=.\source\MenuData.cpp
# End Source File
# Begin Source File

SOURCE=.\source\MenuText.cpp
# End Source File
# Begin Source File

SOURCE=.\source\mirror.cpp
# End Source File
# Begin Source File

SOURCE=.\source\model.cpp
# End Source File
# Begin Source File

SOURCE=.\source\move.cpp
# End Source File
# Begin Source File

SOURCE=.\source\NewColl.cpp
# End Source File
# Begin Source File

SOURCE=.\source\obj_init.cpp
# End Source File
# Begin Source File

SOURCE=.\source\object.cpp
# End Source File
# Begin Source File

SOURCE=.\source\objinfo.cpp
# End Source File
# Begin Source File

SOURCE=.\source\panel.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Particle.cpp
# End Source File
# Begin Source File

SOURCE=.\source\piano.cpp
# End Source File
# Begin Source File

SOURCE=.\source\play.cpp
# End Source File
# Begin Source File

SOURCE=.\source\player.cpp
# End Source File
# Begin Source File

SOURCE=.\source\ReadInit.cpp
# End Source File
# Begin Source File

SOURCE=.\source\registry.cpp
# End Source File
# Begin Source File

SOURCE=.\source\sfx.cpp
# End Source File
# Begin Source File

SOURCE=.\source\shadow.cpp
# End Source File
# Begin Source File

SOURCE=.\source\smoke.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Spark.cpp
# End Source File
# Begin Source File

SOURCE=.\source\text.cpp
# End Source File
# Begin Source File

SOURCE=.\source\texture.cpp
# End Source File
# Begin Source File

SOURCE=.\source\timing.cpp
# End Source File
# Begin Source File

SOURCE=.\source\TitleScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\source\trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Util.cpp
# End Source File
# Begin Source File

SOURCE=.\source\visibox.cpp
# End Source File
# Begin Source File

SOURCE=.\source\weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\source\Wheel.cpp
# End Source File
# Begin Source File

SOURCE=.\source\world.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\CarInfo.txt
# End Source File
# Begin Source File

SOURCE=.\revolt.ico
# End Source File
# Begin Source File

SOURCE=.\Revolt.rc
# End Source File
# End Target
# End Project