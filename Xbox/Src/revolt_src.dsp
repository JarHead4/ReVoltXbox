# Microsoft Developer Studio Project File - Name="revolt_src" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 60000
# ** DO NOT EDIT **

# TARGTYPE "Xbox Application" 0x0b01

CFG=revolt_src - Xbox Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "revolt_src.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "revolt_src.mak" CFG="revolt_src - Xbox Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "revolt_src - Xbox Release" (based on "Xbox Application")
!MESSAGE "revolt_src - Xbox Debug" (based on "Xbox Application")
!MESSAGE "revolt_src - Xbox Profile" (based on "Xbox Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe

!IF  "$(CFG)" == "revolt_src - Xbox Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_XBOX" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_USE_XGMATH" /D "XBOX_NOT_YET_IMPLEMENTED" /D "XBOX_DISABLE_NETWORK" /D "USE_LAST_EXTERNAL_XDK" /D "SHIPPING" /FR /Fp"Release/revolt_xbox.pch" /YX /FD /G6 /Ztmp /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 d3d8.lib d3dx8.lib dsound.lib xnet.lib xgraphics.lib xapilib.lib xboxkrnl.lib /nologo /machine:I386 /subsystem:xbox /fixed:no /TMP /OPT:REF /OPT:ICF
# ADD LINK32 d3d8.lib d3dx8.lib dsound.lib xonlineS.lib xgraphics.lib xvoice.lib xbdm.lib xapilib.lib libcmt.lib libcpmt.lib xboxkrnl.lib /nologo /debug /machine:I386 /nodefaultlib /out:"Release/revolt_xbox.exe" /subsystem:xbox /fixed:no /TMP /OPT:REF /OPT:ICF
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000
# ADD XBE /nologo /testid:"0x584C0001" /testname:"Re-Volt" /stack:0x10000 /debug /out:"Release/revolt_xbox.xbe" /titleimage:"..\art\gfx\title.xbx" /testservice:0x54524150 /INSERTFILE:dspimage.bin,DSPImage,RN
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "revolt_src - Xbox Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_XBOX" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_XBOX" /D "_USE_XGMATH" /D "XBOX_NOT_YET_IMPLEMENTED" /D "XBOX_DISABLE_NETWORK" /D "USE_LAST_EXTERNAL_XDK" /D "SHIPPING" /FR /Fp"Debug/revolt_xbox.pch" /YX /FD /G6 /Ztmp /fastcap /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 d3d8d.lib d3dx8d.lib dsoundd.lib xnetd.lib xgraphicsd.lib xapilibd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /subsystem:xbox /fixed:no /TMP
# ADD LINK32 d3d8d.lib d3dx8d.lib dsoundd.lib xonlineSd.lib xgraphicsd.lib xvoiced.lib xbdm.lib xapilibd.lib libcmtd.lib libcpmtd.lib xboxkrnl.lib /nologo /incremental:no /debug /machine:I386 /nodefaultlib /out:"Debug/revolt_xboxD.exe" /subsystem:xbox /fixed:no /TMP
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000 /debug
# ADD XBE /nologo /testid:"0x584C0001" /testname:"Re-Volt" /stack:0x10000 /debug /out:"Debug/revolt_xboxd.xbe" /titleimage:"..\art\gfx\title.xbx" /testservice:0x54524150 /INSERTFILE:dspimage.bin,DSPImage,RN
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ELSEIF  "$(CFG)" == "revolt_src - Xbox Profile"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Profile"
# PROP BASE Intermediate_Dir "Profile"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Profile"
# PROP Intermediate_Dir "Profile"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_XBOX" /YX /FD /G6 /Ztmp /c
# ADD CPP /nologo /W3 /GX /Zi /O2 /D "NDEBUG" /D "WIN32" /D "_XBOX" /D "_USE_XGMATH" /D "XBOX_NOT_YET_IMPLEMENTED" /D "XBOX_DISABLE_NETWORK" /D "USE_LAST_EXTERNAL_XDK" /D "SHIPPING" /FR /Fp"Profile/revolt_xbox.pch" /YX /FD /G6 /Ztmp /fastcap /c
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 d3d8i.lib d3dx8.lib dsound.lib xnet.lib xgraphics.lib xapilib.lib xboxkrnl.lib /nologo /machine:I386 /subsystem:xbox /fixed:no /TMP /OPT:REF /OPT:ICF
# ADD LINK32 d3d8i.lib d3dx8.lib dsound.lib xonlineS.lib xgraphics.lib xvoice.lib xbdm.lib xapilib.lib libcmt.lib libcpmt.lib xboxkrnl.lib /nologo /debug /machine:I386 /nodefaultlib /out:"Profile/revolt_xboxP.exe" /subsystem:xbox /fixed:no /TMP /OPT:REF /OPT:ICF
XBE=imagebld.exe
# ADD BASE XBE /nologo /stack:0x10000
# ADD XBE /nologo /testid:"0x584C0001" /testname:"Re-Volt" /stack:0x10000 /debug /out:"Profile/revolt_xboxP.xbe" /titleimage:"..\art\gfx\title.xbx" /testservice:0x54524150 /INSERTFILE:dspimage.bin,DSPImage,RN
XBCP=xbecopy.exe
# ADD BASE XBCP /NOLOGO
# ADD XBCP /NOLOGO

!ENDIF 

# Begin Target

# Name "revolt_src - Xbox Release"
# Name "revolt_src - Xbox Debug"
# Name "revolt_src - Xbox Profile"
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

SOURCE=.\body.h
# End Source File
# Begin Source File

SOURCE=.\camera.h
# End Source File
# Begin Source File

SOURCE=.\car.h
# End Source File
# Begin Source File

SOURCE=.\cheats.h
# End Source File
# Begin Source File

SOURCE=.\competition.h
# End Source File
# Begin Source File

SOURCE=.\Content.h
# End Source File
# Begin Source File

SOURCE=.\control.h
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

SOURCE=.\FriendsManager.h
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

SOURCE=.\gaussian.h
# End Source File
# Begin Source File

SOURCE=.\geom.h
# End Source File
# Begin Source File

SOURCE=.\ghost.h
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

SOURCE=.\LevelInfo.h
# End Source File
# Begin Source File

SOURCE=.\LevelLoad.h
# End Source File
# Begin Source File

SOURCE=.\light.h
# End Source File
# Begin Source File

SOURCE=.\load.h
# End Source File
# Begin Source File

SOURCE=.\main.h
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

SOURCE=.\MusicManager.h
# End Source File
# Begin Source File

SOURCE=.\net_Statistics.h
# End Source File
# Begin Source File

SOURCE=.\net_xonline.h
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

SOURCE=.\player.h
# End Source File
# Begin Source File

SOURCE=.\podium.h
# End Source File
# Begin Source File

SOURCE=.\posnode.h
# End Source File
# Begin Source File

SOURCE=.\RatingEquation.h
# End Source File
# Begin Source File

SOURCE=.\readinit.h
# End Source File
# Begin Source File

SOURCE=.\replay.h
# End Source File
# Begin Source File

SOURCE=.\revolt.h
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

SOURCE=.\soundbank.h
# End Source File
# Begin Source File

SOURCE=.\SoundEffectEngine.h
# End Source File
# Begin Source File

SOURCE=.\sounds.h
# End Source File
# Begin Source File

SOURCE=.\spark.h
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

SOURCE=.\trigger.h
# End Source File
# Begin Source File

SOURCE=.\typedefs.h
# End Source File
# Begin Source File

SOURCE=.\ui_Animation.h
# End Source File
# Begin Source File

SOURCE=.\ui_BestTimes.h
# End Source File
# Begin Source File

SOURCE=.\ui_Confirm.h
# End Source File
# Begin Source File

SOURCE=.\ui_ConfirmGiveUp.h
# End Source File
# Begin Source File

SOURCE=.\ui_ContentDownload.h
# End Source File
# Begin Source File

SOURCE=.\ui_ContinueLobby.h
# End Source File
# Begin Source File

SOURCE=.\ui_EnterName.h
# End Source File
# Begin Source File

SOURCE=.\ui_friends.h
# End Source File
# Begin Source File

SOURCE=.\ui_Help.h
# End Source File
# Begin Source File

SOURCE=.\ui_InGameMenu.h
# End Source File
# Begin Source File

SOURCE=.\ui_LiveSignOn.h
# End Source File
# Begin Source File

SOURCE=.\ui_menu.h
# End Source File
# Begin Source File

SOURCE=.\ui_MenuDraw.h
# End Source File
# Begin Source File

SOURCE=.\ui_MenuText.h
# End Source File
# Begin Source File

SOURCE=.\ui_Options.h
# End Source File
# Begin Source File

SOURCE=.\ui_players.h
# End Source File
# Begin Source File

SOURCE=.\ui_PlayLive.h
# End Source File
# Begin Source File

SOURCE=.\ui_podium.h
# End Source File
# Begin Source File

SOURCE=.\ui_ProgressTable.h
# End Source File
# Begin Source File

SOURCE=.\ui_RaceDifficulty.h
# End Source File
# Begin Source File

SOURCE=.\ui_RaceOverview.h
# End Source File
# Begin Source File

SOURCE=.\ui_SelectCar.h
# End Source File
# Begin Source File

SOURCE=.\ui_SelectConnection.h
# End Source File
# Begin Source File

SOURCE=.\ui_SelectCup.h
# End Source File
# Begin Source File

SOURCE=.\ui_SelectLanguage.h
# End Source File
# Begin Source File

SOURCE=.\ui_SelectRaceMode.h
# End Source File
# Begin Source File

SOURCE=.\ui_SelectTrack.h
# End Source File
# Begin Source File

SOURCE=.\ui_SelectTrainingMode.h
# End Source File
# Begin Source File

SOURCE=.\ui_ShowMessage.h
# End Source File
# Begin Source File

SOURCE=.\ui_SinglePlayerGame.h
# End Source File
# Begin Source File

SOURCE=.\ui_StateEngine.h
# End Source File
# Begin Source File

SOURCE=.\ui_Statistics.h
# End Source File
# Begin Source File

SOURCE=.\ui_SystemLink.h
# End Source File
# Begin Source File

SOURCE=.\ui_TitleScreen.h
# End Source File
# Begin Source File

SOURCE=.\ui_TitleScreenCamera.h
# End Source File
# Begin Source File

SOURCE=.\ui_TopLevelMenu.h
# End Source File
# Begin Source File

SOURCE=.\ui_WaitForLobby.h
# End Source File
# Begin Source File

SOURCE=.\ui_WaitingRoom.h
# End Source File
# Begin Source File

SOURCE=.\units.h
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# Begin Source File

SOURCE=.\verify.h
# End Source File
# Begin Source File

SOURCE=.\visibox.h
# End Source File
# Begin Source File

SOURCE=.\VoiceCommunicator.h
# End Source File
# Begin Source File

SOURCE=.\VoiceManager.h
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

SOURCE=.\XBFont.h
# End Source File
# Begin Source File

SOURCE=.\xbhelp.h
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
# Begin Source File

SOURCE=.\XBUtil.h
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

SOURCE=.\body.cpp
# End Source File
# Begin Source File

SOURCE=.\camera.cpp
# End Source File
# Begin Source File

SOURCE=.\car.cpp
# End Source File
# Begin Source File

SOURCE=.\cheats.cpp
# End Source File
# Begin Source File

SOURCE=.\competition.cpp
# End Source File
# Begin Source File

SOURCE=.\Content.cpp
# End Source File
# Begin Source File

SOURCE=.\control.cpp
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

SOURCE=.\EditAi.cpp
# End Source File
# Begin Source File

SOURCE=.\EditCam.cpp
# End Source File
# Begin Source File

SOURCE=.\EditField.cpp
# End Source File
# Begin Source File

SOURCE=.\EditObject.cpp
# End Source File
# Begin Source File

SOURCE=.\EditPortal.cpp
# End Source File
# Begin Source File

SOURCE=.\EditPos.cpp
# End Source File
# Begin Source File

SOURCE=.\EditTrigger.cpp
# End Source File
# Begin Source File

SOURCE=.\EditZone.cpp
# End Source File
# Begin Source File

SOURCE=.\field.cpp
# End Source File
# Begin Source File

SOURCE=.\FriendsManager.cpp
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

SOURCE=.\gaussian.cpp
# End Source File
# Begin Source File

SOURCE=.\geom.cpp
# End Source File
# Begin Source File

SOURCE=.\ghost.cpp
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

SOURCE=.\LevelInfo.cpp
# End Source File
# Begin Source File

SOURCE=.\LevelLoad.cpp
# End Source File
# Begin Source File

SOURCE=.\light.cpp
# End Source File
# Begin Source File

SOURCE=.\load.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
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

SOURCE=.\MusicManager.cpp
# End Source File
# Begin Source File

SOURCE=.\net_Statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\net_xonline.cpp
# End Source File
# Begin Source File

SOURCE=.\network.cpp
# End Source File
# Begin Source File

SOURCE=.\newcoll.cpp
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

SOURCE=.\posnode.cpp
# End Source File
# Begin Source File

SOURCE=.\RatingEquation.cpp
# End Source File
# Begin Source File

SOURCE=.\readinit.cpp
# End Source File
# Begin Source File

SOURCE=.\replay.cpp
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

SOURCE=.\soundbank.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundEffectEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\spark.cpp
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

SOURCE=.\trigger.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_Animation.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_BestTimes.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_Confirm.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_ConfirmGiveUp.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_ContentDownload.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_ContinueLobby.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_EnterName.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_friends.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_Gallery.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_Help.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_InGameMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_LiveSignOn.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_menu.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_MenuDraw.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_MenuText.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_Options.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_players.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_PlayLive.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_podium.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_ProgressTable.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_RaceDifficulty.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_RaceOverview.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SelectCar.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SelectConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SelectCup.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SelectLanguage.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SelectRaceMode.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SelectTrack.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SelectTrainingMode.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_ShowMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SinglePlayerGame.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SplashScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_Statistics.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_SystemLink.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_TitleScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_TitleScreenCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_TopLevelMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_TrackScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_WaitForLobby.cpp
# End Source File
# Begin Source File

SOURCE=.\ui_WaitingRoom.cpp
# End Source File
# Begin Source File

SOURCE=.\util.cpp
# End Source File
# Begin Source File

SOURCE=.\verify.cpp
# End Source File
# Begin Source File

SOURCE=.\visibox.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceCommunicator.cpp
# End Source File
# Begin Source File

SOURCE=.\VoiceManager.cpp
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

SOURCE=.\XBFont.cpp
# End Source File
# Begin Source File

SOURCE=.\xbhelp.cpp
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

SOURCE=.\XBUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\xdx.cpp
# End Source File
# End Group
# End Target
# End Project
