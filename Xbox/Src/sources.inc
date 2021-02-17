TARGETPATH = obj
TARGETTYPE = PROGRAM

UMTYPE     = xboxapp
USE_LIBCMT = 1
USE_NATIVE_EH = 1
USE_STL = 1

# $BUGBUG: do we really want to specify "NOT_UNICODE" ?
NOT_UNICODE = 1

XE_FLAGS = $(XE_FLAGS) /INSERTFILE:"..\dspimage.bin",DSPImage,RN /TESTID:"0x584C0001" /TESTNAME:"Re-Volt" /TITLEIMAGE:"..\..\art\gfx\title.xbx"

C_DEFINES = \
  -DXBOX_DISABLE_NETWORK \
  -DXBOX_NOT_YET_IMPLEMENTED \
  -D_USE_XGMATH \
  $(C_DEFINES)

!IF defined(SHIPPING)
C_DEFINES = $(C_DEFINES) \
            -DSHIPPING
!ENDIF

!IF defined(STRICTBUILD)
MSC_WARNING_LEVEL = /W4
!ENDIF

SOURCES = \
  ..\aerial.cpp \
  ..\ai.cpp \
  ..\ai_car.cpp \
  ..\ai_init.cpp \
  ..\ainode.cpp \
  ..\aizone.cpp \
  ..\body.cpp \
  ..\camera.cpp \
  ..\car.cpp \
  ..\cheats.cpp \
  ..\competition.cpp \
  ..\content.cpp \
  ..\control.cpp \
  ..\credits.cpp \
  ..\ctrlread.cpp \
  ..\debug.cpp \
  ..\draw.cpp \
  ..\drawobj.cpp \
  ..\dx.cpp \
  ..\EditAi.cpp \
  ..\EditCam.cpp \
  ..\EditField.cpp \
  ..\EditObject.cpp \
  ..\EditPortal.cpp \
  ..\EditPos.cpp \
  ..\EditTrigger.cpp \
  ..\EditZone.cpp \
  ..\field.cpp \
  ..\friendsManager.cpp \
  ..\gamegauge.cpp \
  ..\gameloop.cpp \
  ..\gamepad.cpp \
  ..\gaussian.cpp \
  ..\geom.cpp \
  ..\ghost.cpp \
  ..\InitPlay.cpp \
  ..\input.cpp \
  ..\instance.cpp \
  ..\intro.cpp \
  ..\LevelInfo.cpp \
  ..\LevelLoad.cpp \
  ..\light.cpp \
  ..\load.cpp \
  ..\main.cpp \
  ..\mirror.cpp \
  ..\model.cpp \
  ..\move.cpp \
  ..\musicmanager.cpp \
  ..\network.cpp \
  ..\net_xonline.cpp \
  ..\net_statistics.cpp \
  ..\newcoll.cpp \
  ..\obj_init.cpp \
  ..\object.cpp \
  ..\objinfo.cpp \
  ..\panel.cpp \
  ..\particle.cpp \
  ..\path.cpp \
  ..\piano.cpp \
  ..\pickup.cpp \
  ..\player.cpp \
  ..\posnode.cpp \
  ..\RatingEquation.cpp \
  ..\readinit.cpp \
  ..\replay.cpp \
  ..\settings.cpp \
  ..\sfx.cpp \
  ..\shadow.cpp \
  ..\shareware.cpp \
  ..\SoundBank.cpp \
  ..\SoundEffectEngine.cpp \
  ..\spark.cpp \
  ..\text.cpp \
  ..\texture.cpp \
  ..\timing.cpp \
  ..\trigger.cpp \
  ..\ui_Animation.cpp \
  ..\ui_BestTimes.cpp \
  ..\ui_Confirm.cpp \
  ..\ui_ConfirmGiveUp.cpp \
  ..\ui_ContentDownload.cpp \
  ..\ui_ContinueLobby.cpp \
  ..\ui_EnterName.cpp \
  ..\ui_Friends.cpp \
  ..\ui_Gallery.cpp \
  ..\ui_Help.cpp \
  ..\ui_InGameMenu.cpp \
  ..\ui_LiveSignOn.cpp \
  ..\ui_Menu.cpp \
  ..\ui_MenuDraw.cpp \
  ..\ui_MenuText.cpp \
  ..\ui_Options.cpp \
  ..\ui_Players.cpp \
  ..\ui_PlayLive.cpp \
  ..\ui_Podium.cpp \
  ..\ui_ProgressTable.cpp \
  ..\ui_RaceDifficulty.cpp \
  ..\ui_RaceOverview.cpp \
  ..\ui_SelectCar.cpp \
  ..\ui_SelectConnection.cpp \
  ..\ui_SelectCup.cpp \
  ..\ui_SelectLanguage.cpp \
  ..\ui_SelectRaceMode.cpp \
  ..\ui_SelectTrack.cpp \
  ..\ui_SelectTrainingMode.cpp \
  ..\ui_SinglePlayerGame.cpp \
  ..\ui_SplashScreen.cpp \
  ..\ui_Statistics.cpp \
  ..\ui_SystemLink.cpp \
  ..\ui_ShowMessage.cpp \
  ..\ui_TitleScreen.cpp \
  ..\ui_TitleScreenCamera.cpp \
  ..\ui_TopLevelMenu.cpp \
  ..\ui_TrackScreen.cpp \
  ..\ui_WaitForLobby.cpp \
  ..\ui_WaitingRoom.cpp \
  ..\util.cpp \
  ..\verify.cpp \
  ..\visibox.cpp \
  ..\VoiceCommunicator.cpp \
  ..\VoiceManager.cpp \
  ..\weapon.cpp \
  ..\wheel.cpp \
  ..\world.cpp \
  ..\XBFont.cpp \
  ..\XBHelp.cpp \
  ..\XBInput.cpp \
  ..\XBMediaDebug.cpp \
  ..\XBOnline.cpp \
  ..\XBResource.cpp \
  ..\XBUtil.cpp \
  ..\xdx.cpp 

