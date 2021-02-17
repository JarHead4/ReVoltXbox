# Microsoft Developer Studio Project File - Name="revolt_xbox" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Application" 0x0b01

CFG=revolt_xbox - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "revolt_xbox.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "revolt_xbox.mak" CFG="revolt_xbox - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "revolt_xbox - Xbox Release" (based on "Xbox Application")
!MESSAGE "revolt_xbox - Xbox Debug" (based on "Xbox Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "revolt_xbox - Xbox Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "_XBOX" /D "NDEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_USE_XGMATH" /D "XBOX_NOT_YET_IMPLEMENTED" /D "XBOX_DISABLE_NETWORK" /FR /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xnet.lib xboxkrnl.lib /nologo /machine:I386 /subsystem:xbox /fixed:no /debugtype:vc6 /OPT:REF
# ADD LINK32 xapilib.lib d3d8.lib d3dx8.lib xgraphics.lib dsound.lib dmusic.lib xboxkrnl.lib xbdm.lib xvoice.lib xonline.lib /nologo /debug /machine:I386 /subsystem:xbox /fixed:no /debugtype:vc6 /OPT:REF
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000
# ADD XBE /nologo /testid:"0xFFEDB18A" /testname:"Revolt (Microsoft ATG)" /stack:0x10000 /titleimage:"..\art\gfx\title.xbx" /INSERTFILE:dspimage.bin,DSPImage,RN
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying art to the Xbox
PostBuild_Cmds=cd ..\Art	call copyart.bat
# End Special Build Tool

!ELSEIF  "$(CFG)" == "revolt_xbox - Xbox Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_XBOX" /D "_DEBUG" /YX /FD /G6 /Zvc6 /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_XBOX" /D "_USE_XGMATH" /D "XBOX_NOT_YET_IMPLEMENTED" /D "XBOX_DISABLE_NETWORK" /FR /YX /FD /G6 /Zvc6 /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib dsoundd.lib dmusicd.lib xnetd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /debugtype:vc6
# ADD LINK32 xapilibd.lib d3d8d.lib d3dx8d.lib xgraphicsd.lib xbdm.lib dsoundd.lib dmusicd.lib xboxkrnl.lib xvoiced.lib xonlined.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /debugtype:vc6
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /testid:"0xFFEDB18A" /testname:"Revolt (Microsoft ATG)" /stack:0x10000 /debug /titleimage:"..\art\gfx\title.xbx" /INSERTFILE:dspimage.bin,DSPImage,RN
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying art to the Xbox
PostBuild_Cmds=cd ..\Art	call copyart.bat
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "revolt_xbox - Xbox Release"
# Name "revolt_xbox - Xbox Debug"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\aerial.h
# End Source File
# Begin Source File

SOURCE=.\ai.h
# End Source File
# Begin Source File

SOURCE=.\ai_car.h
# End Source File
# Begin Source File

SOURCE=.\ai_init.h
# End Source File
# Begin Source File

SOURCE=.\ainode.h
# End Source File
# Begin Source File

SOURCE=.\aizone.h
# End Source File
# Begin Source File

SOURCE=.\AudioSettings.h
# End Source File
# Begin Source File

SOURCE=.\BestTimes.h
# End Source File
# Begin Source File

SOURCE=.\body.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\car.h
# End Source File
# Begin Source File

SOURCE=.\Cheats.h
# End Source File
# Begin Source File

SOURCE=.\competition.h
# End Source File
# Begin Source File

SOURCE=.\Confirm.h
# End Source File
# Begin Source File

SOURCE=.\ConfirmGiveUp.h
# End Source File
# Begin Source File

SOURCE=.\ConnectionType.h
# End Source File
# Begin Source File

SOURCE=.\control.h
# End Source File
# Begin Source File

SOURCE=.\ControllerOptions.h
# End Source File
# Begin Source File

SOURCE=.\credits.h
# End Source File
# Begin Source File

SOURCE=.\ctrlread.h
# End Source File
# Begin Source File

SOURCE=.\debug.h
# End Source File
# Begin Source File

SOURCE=.\dinput.h
# End Source File
# Begin Source File

SOURCE=.\draw.h
# End Source File
# Begin Source File

SOURCE=.\drawobj.h
# End Source File
# Begin Source File

SOURCE=.\dx.h
# End Source File
# Begin Source File

SOURCE=.\editai.h
# End Source File
# Begin Source File

SOURCE=.\editcam.h
# End Source File
# Begin Source File

SOURCE=.\EditField.h
# End Source File
# Begin Source File

SOURCE=.\EditObject.h
# End Source File
# Begin Source File

SOURCE=.\EditPortal.h
# End Source File
# Begin Source File

SOURCE=.\EditPos.h
# End Source File
# Begin Source File

SOURCE=.\EditTrigger.h
# End Source File
# Begin Source File

SOURCE=.\editzone.h
# End Source File
# Begin Source File

SOURCE=.\field.h
# End Source File
# Begin Source File

SOURCE=.\FrontEndCamera.h
# End Source File
# Begin Source File

SOURCE=.\Gallery.h
# End Source File
# Begin Source File

SOURCE=.\gamegauge.h
# End Source File
# Begin Source File

SOURCE=.\gameloop.h
# End Source File
# Begin Source File

SOURCE=.\gamepad.h
# End Source File
# Begin Source File

SOURCE=.\GameSettings.h
# End Source File
# Begin Source File

SOURCE=.\gaussian.h
# End Source File
# Begin Source File

SOURCE=.\geom.h
# End Source File
# Begin Source File

SOURCE=.\GetPlayerName.h
# End Source File
# Begin Source File

SOURCE=.\ghost.h
# End Source File
# Begin Source File

SOURCE=.\InGameMenu.h
# End Source File
# Begin Source File

SOURCE=.\InitPlay.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\instance.h
# End Source File
# Begin Source File

SOURCE=.\intro.h
# End Source File
# Begin Source File

SOURCE=.\Language.h
# End Source File
# Begin Source File

SOURCE=.\LevelInfo.h
# End Source File
# Begin Source File

SOURCE=.\LevelLoad.h
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\LiveSignOn.h
# End Source File
# Begin Source File

SOURCE=.\load.h
# End Source File
# Begin Source File

SOURCE=.\Lobby.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\MainMenu.h
# End Source File
# Begin Source File

SOURCE=.\Menu2.h
# End Source File
# Begin Source File

SOURCE=.\menudat2.h
# End Source File
# Begin Source File

SOURCE=.\MenuDraw.h
# End Source File
# Begin Source File

SOURCE=.\menutext.h
# End Source File
# Begin Source File

SOURCE=.\mirror.h
# End Source File
# Begin Source File

SOURCE=.\model.h
# End Source File
# Begin Source File

SOURCE=.\move.h
# End Source File
# Begin Source File

SOURCE=.\mss.h
# End Source File
# Begin Source File

SOURCE=.\MultiPlayer.h
# End Source File
# Begin Source File

SOURCE=.\MusicManager.h
# End Source File
# Begin Source File

SOURCE=.\network.h
# End Source File
# Begin Source File

SOURCE=.\newcoll.h
# End Source File
# Begin Source File

SOURCE=.\obj_init.h
# End Source File
# Begin Source File

SOURCE=.\object.h
# End Source File
# Begin Source File

SOURCE=.\Options.h
# End Source File
# Begin Source File

SOURCE=.\panel.h
# End Source File
# Begin Source File

SOURCE=.\particle.h
# End Source File
# Begin Source File

SOURCE=.\path.h
# End Source File
# Begin Source File

SOURCE=.\piano.h
# End Source File
# Begin Source File

SOURCE=.\pickup.h
# End Source File
# Begin Source File

SOURCE=.\play_TEMP.h
# End Source File
# Begin Source File

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\podium.h
# End Source File
# Begin Source File

SOURCE=.\posnode.h
# End Source File
# Begin Source File

SOURCE=.\RaceSummary.h
# End Source File
# Begin Source File

SOURCE=.\readinit.h
# End Source File
# Begin Source File

SOURCE=.\RenderSettings.h
# End Source File
# Begin Source File

SOURCE=.\replay.h
# End Source File
# Begin Source File

SOURCE=.\revolt.h
# End Source File
# Begin Source File

SOURCE=.\Secrets.h
# End Source File
# Begin Source File

SOURCE=.\SelectCar.h
# End Source File
# Begin Source File

SOURCE=.\SelectCup.h
# End Source File
# Begin Source File

SOURCE=.\SelectRace.h
# End Source File
# Begin Source File

SOURCE=.\SelectTrack.h
# End Source File
# Begin Source File

SOURCE=.\settings.h
# End Source File
# Begin Source File

SOURCE=.\sfx.h
# End Source File
# Begin Source File

SOURCE=.\shadow.h
# End Source File
# Begin Source File

SOURCE=.\shareware.h
# End Source File
# Begin Source File

SOURCE=.\SimpleMessage.h
# End Source File
# Begin Source File

SOURCE=.\soundbank.h
# End Source File
# Begin Source File

SOURCE=.\SoundEffectEngine.h
# End Source File
# Begin Source File

SOURCE=.\SoundEffects.h
# End Source File
# Begin Source File

SOURCE=.\sounds.h
# End Source File
# Begin Source File

SOURCE=.\spark.h
# End Source File
# Begin Source File

SOURCE=.\StateEngine.h
# End Source File
# Begin Source File

SOURCE=.\SyslinkGameSelect.h
# End Source File
# Begin Source File

SOURCE=.\text.h
# End Source File
# Begin Source File

SOURCE=.\texture.h
# End Source File
# Begin Source File

SOURCE=.\timing.h
# End Source File
# Begin Source File

SOURCE=.\titlescreen.h
# End Source File
# Begin Source File

SOURCE=.\trigger.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\units.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\VideoSettings.h
# End Source File
# Begin Source File

SOURCE=.\visibox.h
# End Source File
# Begin Source File

SOURCE=.\VoiceCommunicator.h
# End Source File
# Begin Source File

SOURCE=.\WaitingRoom.h
# End Source File
# Begin Source File

SOURCE=.\weapon.h
# End Source File
# Begin Source File

SOURCE=.\wheel.h
# End Source File
# Begin Source File

SOURCE=.\windows.h
# End Source File
# Begin Source File

SOURCE=.\world.h
# End Source File
# Begin Source File

SOURCE=.\xbfont.h
# End Source File
# Begin Source File

SOURCE=.\XBInput.h
# End Source File
# Begin Source File

SOURCE=.\XBMediaDebug.h
# End Source File
# Begin Source File

SOURCE=.\XBOnline.h
# End Source File
# Begin Source File

SOURCE=.\XBResource.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\aerial.cpp
# End Source File
# Begin Source File

SOURCE=.\ai.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_car.cpp
# End Source File
# Begin Source File

SOURCE=.\ai_init.cpp
# End Source File
# Begin Source File

SOURCE=.\ainode.cpp
# End Source File
# Begin Source File

SOURCE=.\aizone.cpp
# End Source File
# Begin Source File

SOURCE=.\AudioSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\BestTimes.cpp
# End Source File
# Begin Source File

SOURCE=.\body.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\car.cpp
# End Source File
# Begin Source File

SOURCE=.\Cheats.cpp
# End Source File
# Begin Source File

SOURCE=.\competition.cpp
# End Source File
# Begin Source File

SOURCE=.\Confirm.cpp
# End Source File
# Begin Source File

SOURCE=.\ConfirmGiveUp.cpp
# End Source File
# Begin Source File

SOURCE=.\ConnectionType.cpp
# End Source File
# Begin Source File

SOURCE=.\control.cpp
# End Source File
# Begin Source File

SOURCE=.\ControllerOptions.cpp
# End Source File
# Begin Source File

SOURCE=.\credits.cpp
# End Source File
# Begin Source File

SOURCE=.\ctrlread.cpp
# End Source File
# Begin Source File

SOURCE=.\debug.cpp
# End Source File
# Begin Source File

SOURCE=.\draw.cpp
# End Source File
# Begin Source File

SOURCE=.\DrawObj.cpp
# End Source File
# Begin Source File

SOURCE=.\dx.cpp
# End Source File
# Begin Source File

SOURCE=.\field.cpp
# End Source File
# Begin Source File

SOURCE=.\FrontEndCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\Gallery.cpp
# End Source File
# Begin Source File

SOURCE=.\gamegauge.cpp
# End Source File
# Begin Source File

SOURCE=.\gameloop.cpp
# End Source File
# Begin Source File

SOURCE=.\gamepad.cpp
# End Source File
# Begin Source File

SOURCE=.\GameSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\gaussian.cpp
# End Source File
# Begin Source File

SOURCE=.\geom.cpp
# End Source File
# Begin Source File

SOURCE=.\GetPlayerName.cpp
# End Source File
# Begin Source File

SOURCE=.\ghost.cpp
# End Source File
# Begin Source File

SOURCE=.\InGameMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\InitPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\input.cpp
# End Source File
# Begin Source File

SOURCE=.\instance.cpp
# End Source File
# Begin Source File

SOURCE=.\intro.cpp
# End Source File
# Begin Source File

SOURCE=.\Language.cpp
# End Source File
# Begin Source File

SOURCE=.\LevelInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\LevelLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\light.cpp
# End Source File
# Begin Source File

SOURCE=.\LiveSignOn.cpp
# End Source File
# Begin Source File

SOURCE=.\load.cpp
# End Source File
# Begin Source File

SOURCE=.\Lobby.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MainMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\Menu2.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuText.cpp
# End Source File
# Begin Source File

SOURCE=.\mirror.cpp
# End Source File
# Begin Source File

SOURCE=.\model.cpp
# End Source File
# Begin Source File

SOURCE=.\move.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\MusicManager.cpp
# End Source File
# Begin Source File

SOURCE=.\network.cpp
# End Source File
# Begin Source File

SOURCE=.\newcoll.cpp
# End Source File
# Begin Source File

SOURCE=.\NumberOfPlayers.cpp
# End Source File
# Begin Source File

SOURCE=.\obj_init.cpp
# End Source File
# Begin Source File

SOURCE=.\object.cpp
# End Source File
# Begin Source File

SOURCE=.\ObjInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\Options.cpp
# End Source File
# Begin Source File

SOURCE=.\panel.cpp
# End Source File
# Begin Source File

SOURCE=.\particle.cpp
# End Source File
# Begin Source File

SOURCE=.\path.cpp
# End Source File
# Begin Source File

SOURCE=.\piano.cpp
# End Source File
# Begin Source File

SOURCE=.\pickup.cpp
# End Source File
# Begin Source File

SOURCE=.\player.cpp
# End Source File
# Begin Source File

SOURCE=.\podium.cpp
# End Source File
# Begin Source File

SOURCE=.\posnode.cpp
# End Source File
# Begin Source File

SOURCE=.\RaceSummary.cpp
# End Source File
# Begin Source File

SOURCE=.\readinit.cpp
# End Source File
# Begin Source File

SOURCE=.\RenderSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\replay.cpp
# End Source File
# Begin Source File

SOURCE=.\Secrets.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectCar.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectCup.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectRace.cpp
# End Source File
# Begin Source File

SOURCE=.\SelectTrack.cpp
# End Source File
# Begin Source File

SOURCE=.\settings.cpp
# End Source File
# Begin Source File

SOURCE=.\sfx.cpp
# End Source File
# Begin Source File

SOURCE=.\shadow.cpp
# End Source File
# Begin Source File

SOURCE=.\shareware.cpp
# End Source File
# Begin Source File

SOURCE=.\SimpleMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\soundbank.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundEffectEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\spark.cpp
# End Source File
# Begin Source File

SOURCE=.\SyslinkGameSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\text.cpp
# End Source File
# Begin Source File

SOURCE=.\texture.cpp
# End Source File
# Begin Source File

SOURCE=.\timing.cpp
# End Source File
# Begin Source File

SOURCE=.\TitleScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\TopLevelMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainingMode.cpp
# End Source File
# Begin Source File

SOURCE=.\trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=.\VideoSettings.cpp
# End Source File
# Begin Source File

SOURCE=.\visibox.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceCommunicator.cpp
# End Source File
# Begin Source File

SOURCE=.\WaitingRoom.cpp
# End Source File
# Begin Source File

SOURCE=.\weapon.cpp
# End Source File
# Begin Source File

SOURCE=.\wheel.cpp
# End Source File
# Begin Source File

SOURCE=.\world.cpp
# End Source File
# Begin Source File

SOURCE=.\xbfont.cpp
# End Source File
# Begin Source File

SOURCE=.\XBInput.cpp
# End Source File
# Begin Source File

SOURCE=.\XBMediaDebug.cpp
# End Source File
# Begin Source File

SOURCE=.\XBOnline.cpp
# End Source File
# Begin Source File

SOURCE=.\XBResource.cpp
# End Source File
# Begin Source File

SOURCE=.\xdx.cpp
# End Source File
# End Group
# End Target
# End Project
