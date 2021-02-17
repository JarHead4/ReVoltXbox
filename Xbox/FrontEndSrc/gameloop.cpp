//-----------------------------------------------------------------------------
// File: gameloop.cpp
//
// Desc: Main game loop code.
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "model.h"
#include "texture.h"
#include "particle.h"
#include "aerial.h"
#include "network.h"
#include "NewColl.h"
#include "body.h"
#include "car.h"
#include "input.h"
#include "text.h"
#include "shadow.h"
#include "camera.h"
#include "light.h"
#include "world.h"
#include "draw.h"
#include "DrawObj.h"
#include "visibox.h"
#include "LevelLoad.h"
#include "ctrlread.h"
#include "obj_init.h"
#include "object.h"
#include "control.h"
#include "move.h"
#include "gameloop.h"
#include "instance.h"
#include "player.h"
#include "timing.h"
#include "ghost.h"
#include "settings.h"
#include "trigger.h"
#include "panel.h"
#include "ai.h"
#include "weapon.h"
#include "spark.h"
#include "aizone.h"
#include "replay.h"
#include "initplay.h"
#include "TitleScreen.h"
#include "Menu2.h"
#include "MenuText.h"
#include "pickup.h"
#include "field.h"
#include "shareware.h"
#include "credits.h"
//#include "gamegauge.h"

// menus
#include "InGameMenu.h"

#include "SoundEffectEngine.h"

#include "XBInput.h"
#include "VoiceCommunicator.h"

#define DEMO_FLASH_TIME     TO_TIME(Real(1.2))
#define DEMO_TEXT_WIDTH     12
#define DEMO_TEXT_HEIGHT    16

#define GAP_CAMERA_TIME     TO_TIME(Real(1.5))
#define GAP_CAMERA_MIN_TIME TO_TIME(Real(30))

void DrawDemoLogo(REAL alphaMod, int posIndex);
void DrawDemoMessage();
void DrawReplayMessage();
void ChangeMainCamera(CAMERA *camera);
void SetMenuGameSettings();

void InitGapCameraSweep(CAMERA *camera);
void UpdateGapCameraSweep(CAMERA *camera);


// globals

GAMELOOP_QUIT GameLoopQuit;
REAL DemoTimeout = ZERO;

long CarVisi = FALSE;
bool DrawGridCollSkin = FALSE;
unsigned long NPhysicsLoops = 0;

static long PawsSelect;

extern long ShowAiNodes;
extern RESTART_DATA RestartData;
extern SLIDE *gSharewareSlides;
extern SLIDE IntroSlides[];

#if SHOW_PHYSICS_INFO
static bool PhysicsInfoTog = FALSE;
#endif

#if USE_DEBUG_ROUTINES
extern REAL DBG_dt;
#endif

bool GLP_TriggerGapCamera = FALSE;
REAL GLP_GapCameraTimer = ZERO;
VEC GLP_GapCameraPos[3];
MAT GLP_GapCameraMat;

// ghost takeover stuff

#if GHOST_TAKEOVER
static void GhostTakeover(void);
long GhostTakeoverFlag = FALSE, GhostTakeoverTime = 0;
REAL CamTime = ZERO;
REAL ChangeCamTime = Real(5);
#endif

#if CHECK_ZZZ
extern void CheckZZZ(void);
#endif




//-----------------------------------------------------------------------------
void GLP_GameLoop(void)
{
#if SCREEN_TIMES
    long j;
#endif
    long i, car;
    char buf[256];
    LEVELINFO *li;
    static bool button = FALSE;
    static bool lastButton = FALSE;

// update game gauge?

/*
    if (GAME_GAUGE)
    {
        UpdateGameGaugeVars();
    }
*/

// quit?

    if (GetFadeEffect() == FADE_DOWN_DONE && GameLoopQuit != GAMELOOP_QUIT_OFF)
    {
/*
        // straight to demo?
        if (GoStraightToDemo)
        {
            SET_EVENT(SetupSharewareIntro);
            if (IsMultiPlayer())
                KillNetwork();

            LEV_EndLevelStageTwo();
            LEV_EndLevelStageOne();
            return;
        }
*/

        // act on gameloop flag
        switch (GameLoopQuit)
        {
            // game gauge quit game
            case GAMELOOP_QUIT_GAMEGAUGE:
                g_bQuitGame = TRUE;

                LEV_EndLevelStageTwo();
                LEV_EndLevelStageOne();
                return;

            // continue championship
            case GAMELOOP_QUIT_CHAMP_CONTINUE:
                GoToNextChampionshipRace();
                return;

            // quit to frontend
            case GAMELOOP_QUIT_FRONTEND:
                SET_EVENT(GoTitleScreen);
                if (IsMultiPlayer())
                {
                    KillNetwork();
//$REMOVED
//                    LobbyFree();
//$END_REMOVAL
                }

                LEV_EndLevelStageTwo();
                LEV_EndLevelStageOne();
                return;

            // restart game
            case GAMELOOP_QUIT_RESTART:
                if (IsMultiPlayer())
                {
                    if (IsServer())
                        SendMultiplayerRestart();
                    else
                        ClientMultiplayerRestart();
                }

                if (ReplayMode)
                {
                    memcpy(&StartData, &StartDataStorage, sizeof(START_DATA));
                    GameSettings.GameType = StartData.GameType;
                    GameSettings.NumberOfLaps = StartData.Laps;
                }

                ReplayMode = FALSE;
                RPL_RecordReplay = TRUE;
                RPL_InitReplayBuffer();

                if (GameSettings.RandomCars && !IsMultiPlayer())
                {
                    car = PickRandomCar();
                    for (i = 0 ; i < StartData.PlayerNum ; i++)
                    {
                        StartData.PlayerData[i].CarType = car;

                        if (i)
                        {
                            strncpy(StartData.PlayerData[i].Name, CarInfo[car].Name, MAX_PLAYER_NAME);
                            StartData.PlayerData[i].Name[MAX_PLAYER_NAME - 1] = 0;
                        }
                    }
                }

                if (GameSettings.RandomTrack && !IsMultiPlayer())
                {
                    LEV_EndLevelStageTwo();
                    LEV_EndLevelStageOne();

                    LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
                    LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
                    LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start

                    GameSettings.Level = PickRandomTrack();
                    li = GetLevelInfo(GameSettings.Level);
                    strncpy(StartData.LevelDir, li->Dir, MAX_LEVEL_DIR_NAME);
                }

                EnableLoadThread(((GameSettings.RandomTrack || (IsMultiPlayer() && RestartData.NewTrack)) ? STAGE_ONE_LOAD_COUNT : STAGE_TWO_LOAD_COUNT) + StartData.PlayerNum);
                SetupLevelAndPlayers();
                DisableLoadThread();

                return;

            // replay
            case GAMELOOP_QUIT_REPLAY:
                RPL_RecordReplay = FALSE;
                RPL_TerminateReplay();
                memcpy(&StartDataStorage, &StartData, sizeof(START_DATA));
                GameSettings.GameType = GAMETYPE_REPLAY;

                if (IsMultiPlayer())
                {
                    KillNetwork();
                    GameSettings.MultiType = MULTITYPE_NONE;
                }

                EnableLoadThread(STAGE_TWO_LOAD_COUNT + StartData.PlayerNum);
                SetupLevelAndPlayers();
                DisableLoadThread();
                return;

            // restart replay
            case GAMELOOP_QUIT_RESTART_REPLAY:
                EnableLoadThread(STAGE_TWO_LOAD_COUNT + StartData.PlayerNum);
                SetupLevelAndPlayers();
                DisableLoadThread();
                return;

            // quit to license screens
            case GAMELOOP_QUIT_DEMO:
                gSharewareSlides = IntroSlides;
                SET_EVENT(SetupSharewareIntro);

                LEV_EndLevelStageTwo();
                LEV_EndLevelStageOne();
                return;
        }
    }

// ghost take over?

#if GHOST_TAKEOVER
    GhostTakeover();
#endif

// Update game timers

    FrameCount++;
    UpdateTimeStep();
    UpdatePacketInfo();
    if( IsServer() )  //$ADDITION -- only server checks if all clients are ready
    CheckAllPlayersReady();

// update keyboard / mouse

//$REMOVED    ReadMouse();
//$REMOVED    ReadKeyboard();
    ReadJoystick();

    // Process voice
	CVoiceCommunicator::ProcessVoice();

// cheat?

    CheckCheatStrings();

// reset 3d poly list

    Reset3dPolyList();

// Check local keys for home, camera, etc

    CRD_CheckLocalKeys();

// maintain sounds

#ifdef OLD_AUDIO
    MaintainAllSfx();
#else
    g_SoundEngine.UpdateAll();
#endif

// reset oil slick list

    ResetOilSlickList();

// remote sync?

    if (IsServer() && NextSyncReady && !CountdownTime && !ReplayMode)
    {
        SendPlayerSync();
    }

// get multiplayer messages

    if (IsMultiPlayer())
    {
        GetRemoteMessages();
    }

// anti-pause stuff

    if ((!GameSettings.Paws || IsMultiPlayer()) && !ReachedEndOfReplay)
    {

// update drum

        UpdateDrum();

// reset mesh fx list

        ResetMeshFxList();

// perform AI

        AI_ProcessAllAIs();

// move objects

        if (!ReplayMode) {
            DefaultPhysicsLoop();
//$REVISIT - might actually this call, depending on how we implement vibration
//$REMOVED            UpdatePlayerForceFeedback(PLR_LocalPlayer);     // Ingame force feedback
        } else {
            ReplayPhysicsLoop();
//$REVISIT - might actually this call, depending on how we implement vibration
//$REMOVED            UpdateReplayForceFeedback(PLR_LocalPlayer);     // Replay force feedback
        }
    }

// update race timers (moved below physics loop - bkk - 17/06/99)

    UpdateRaceTimers();

// anti-pause stuff

    if ((!GameSettings.Paws || IsMultiPlayer()) && !ReachedEndOfReplay)
    {

// update the pickups

        UpdateAllPickups();

// Update the ghost car

        if (GHO_GhostAllowed && (CountdownTime <= 0)) {
            if (PLR_LocalPlayer->type == PLAYER_LOCAL && !PLR_LocalPlayer->CarAI.PreLap) {
                StoreGhostData(&PLR_LocalPlayer->car);
            }
        }

// Process sparks

        ProcessSparks();
    
// Change camera type in replay or demo mode

        if (((GameSettings.GameType == GAMETYPE_DEMO) || ReplayMode)) {
            ChangeMainCamera(CAM_MainCamera);
        }

    }

// Build car world matrices

    BuildAllCarWorldMatrices();

// send multiplayer messages

    if (IsMultiPlayer() && !CountdownTime)
    {
        TransmitRemoteObjectData();
        UpdateMessages();
    }

// Process lights

    ProcessLights();

// texture animations

    ProcessTextureAnimations();

// check triggers

    CheckTriggers();

// Init motion blur

    MotionBlurInit();

// surface check + flip

//$REMOVED    CheckSurfaces();
    FlipBuffers();

// Clear car rendered-last-frame flags
    for (PLAYER *player = PLR_PlayerHead; player != NULL; player = player->next) {
        player->car.RenderedAll = FALSE;
    }

// Begin render

//$REMOVED    D3Ddevice->BeginScene();

// render all cameras

    for (CameraCount = 0 ; CameraCount < MAX_CAMERAS ; CameraCount++) if (Camera[CameraCount].Flag != CAMERA_FLAG_FREE)
    {

// update camera + set camera view vars

        UpdateCamera(&Camera[CameraCount]);

// set and clear viewport

        SetViewport(Camera[CameraCount].X, Camera[CameraCount].Y, Camera[CameraCount].Xsize, Camera[CameraCount].Ysize, BaseGeomPers + Camera[CameraCount].Lens);

        SetCameraView(&Camera[CameraCount].WMatrix, &Camera[CameraCount].WPos, Camera[CameraCount].Shake);
        SetCameraVisiMask(CarVisi ? &PLR_LocalPlayer->car.Body->Centre.Pos : &Camera[CameraCount].WPos);

        InitRenderStates();

        if (Camera[CameraCount].SpecialFog)
        {
            long tempcol = BackgroundColor;
            SetBackgroundColor(Camera[CameraCount].SpecialFogColor);
            FOG_COLOR(BackgroundColor);
            ClearBuffers();
            SetBackgroundColor(tempcol);
        }
        else
        {
            if (!Skybox)
                ClearBuffers();
            else
                RenderSkybox();
        }

// render opaque polys

        ResetSemiList();

        if (DrawGridCollSkin)
        {
            int iPoly;
            OBJECT *obj;
            DrawGridCollPolys(PosToCollGrid(&PLR_LocalPlayer->car.Body->Centre.Pos));
            for (obj = OBJ_ObjectHead; obj != NULL; obj = obj->next) {
                if (obj->body.CollSkin.NCollPolys > 0) {
                    for (iPoly = 0; iPoly < obj->body.CollSkin.NCollPolys; iPoly++)
                    {
                        SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
                        DrawCollPoly(&obj->body.CollSkin.WorldCollPoly[iPoly]);
                        SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
                    }
                }
            }
        }
        else
        {
            DrawWorld();
            DrawInstances();
        }

        DrawObjects();
        DrawAllCars();
        DrawAllPickups();

// draw countdown

        if (Camera[CameraCount].Flag == CAMERA_FLAG_PRIMARY)
            DrawCountdown();

// DRAW object at line-following camera pos
        //long nodeNum, linkNum;
        //FindNearestCameraPath(&PLR_LocalPlayer->car.Body->Centre.Pos, &nodeNum, &linkNum);
        //DrawModel(&PLR_LocalPlayer->car.Models->Body[0], &Identity, &CAM_NodeCamPos, MODEL_PLAIN);

// draw 3d poly list

        Draw3dPolyList();

// flush poly buckets

        FlushPolyBuckets();

// render semi polys

        DrawSemiList();
        DrawSparks();
        DrawTrails();
        DrawSkidMarks();
        DrawAllCarShadows();
        DrawAllGhostCars();

// draw drum?

        if (Camera[CameraCount].Flag == CAMERA_FLAG_PRIMARY)
            DrawDrum();

// flush env buckets

        FlushEnvBuckets();

// Render motion blur

        if (!GameSettings.Paws)
        {
            MotionBlurRender();
        }

// underwater?

        if (Camera[CameraCount].UnderWater)
        {
            RenderUnderWaterPoly();
        }
    
// draw practise star?

        if (GameSettings.GameType == GAMETYPE_PRACTICE)
            DrawPracticeStars();

// display network players

        if (IsMultiPlayer())
        {
            DisplayPlayers();
        }

// draw world wireframe?

        if (Wireframe)
        {
            DrawWorldWireframe();
        }

// begin primary camera stuff

        if (Camera[CameraCount].Flag == CAMERA_FLAG_PRIMARY)
        {

// sepia stuff?

            if (RenderSettings.Sepia)
            {
                DrawSepiaShit();
            }

// draw control panel

            if ((GameSettings.GameType != GAMETYPE_REPLAY) && (GameSettings.GameType != GAMETYPE_DEMO))
            {
                DrawControlPanel();
            }

// begin text state

            BeginTextState();

// display DEMO mode message

            if (GameSettings.GameType == GAMETYPE_DEMO) {
                gDemoTimer += TimeStep;
                gDemoFlashTimer += TimeStep;
                if (gDemoFlashTimer > DEMO_FLASH_TIME) {
                    gDemoFlashTimer = ZERO;
                    gDemoShowMessage = !gDemoShowMessage;
                }

// see if we want to quit demo
                button = FALSE;
                if (RegistrySettings.Joystick != -1) {
                    int iButton;
//$MODIFIED
//                    for (iButton = 0; iButton < Joystick[RegistrySettings.Joystick].ButtonNum; iButton++) {
                    for (iButton = 0; iButton < 8; iButton++) {  //$HACK: assume 8 joystick buttons for now.  Need to $REVISIT this.
//$END_MODIFICATIONS
                        if (JoystickState.rgbButtons[iButton] != 0) {
                            button = TRUE;
                        }
                    }
                }
                if (Keys[DIK_ESCAPE] || Keys[DIK_SPACE] || Keys[DIK_RETURN] || Keys[DIK_NUMPADENTER]) {
                    button = TRUE;
                }

                if (
                    (((DemoTimeout > ZERO) && gDemoTimer > DemoTimeout) || button) && 
                    (GetFadeEffect() != FADE_DOWN)) 
                {
                    SetFadeEffect(FADE_DOWN);
                    SetRedbookFade(REDBOOK_FADE_DOWN);

                    GameLoopQuit = button ? GAMELOOP_QUIT_FRONTEND : GAMELOOP_QUIT_DEMO;
                }
                
                if (CreditVars.State == CREDIT_STATE_INACTIVE) {
                    DrawDemoMessage();
                    DrawDemoLogo(ONE, 0);
                }
            }

// display replay message

            if (GameSettings.GameType == GAMETYPE_REPLAY) {
                DrawReplayMessage();
            }

// championship end screens

            if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
            {
                MaintainChampionshipEndScreens();
            }

// calc fps

//$MODIFIED
//            FrameTime = TimerCurrent;
//            if (FrameTime - FrameTimeLast > TimerFreq / 2)
//            {
//                FrameRate = (FrameCount - FrameCountLast) * TimerFreq / (FrameTime - FrameTimeLast);
//                FrameTimeLast = FrameTime;
//                FrameCountLast = FrameCount;
//            }
            FrameRate_NextFrame();
//$END_MODIFICATIONS

// debug text

            //wsprintf(buf, "MinDist = %d", (int)(1000.0f * LOSMinDist1));
            //DrawBigWideGameText(100, 50, 16, 24, 0xffffff, buf);

            if (Version == VERSION_DEV)
            {
                static long screendebugshow = 1;  //$MODIFIED (was originally set to 0 by default)
                if( g_Controllers[0].dwMenuInput == XBINPUT_BLACK_BUTTON ) screendebugshow = !screendebugshow;

                if (screendebugshow)
                {
                    wsprintf(buf, "State %d, %d  Lights %d  Car %d, %d, %d  Objects %d  Sfx %d, %d", RenderStateChange, TextureStateChange, TotalLightCount, (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[X], (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[Y], (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[Z], OBJ_NumObjects, ChannelsUsed, ChannelsUsed3D);
                    DrawGameText(100, 40, 0xffffff, buf); //$MODIFIED (changed x,y from 0,0 to these new coords)

                    if (ChannelsUsed == SfxSampleNum) DrawGameText(0, 128, 0xffff00, "Sfx Channels Full!");

                    wsprintf(buf, "Cube %d, %d  World %ld  Model %ld  Semi %d  Physics time %ld  Sparks %ld", WorldBigCubeCount, WorldCubeCount, WorldPolyCount, ModelPolyCount, SemiCount, TotalRacePhysicsTime, NActiveSparks);
                    DrawGameText(100, 60, 0xffffff, buf); //$MODIFIED (changed x,y from 0,464 to these new coords)
                }
            }

// record times

#if SCREEN_TIMES
            static long recordshow = 0;
            if (Keys[DIK_F10] && !LastKeys[DIK_F10]) recordshow = (recordshow + 1) % 3;

            if (recordshow == 1)
            {
                for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
                {
                    wsprintf(buf, "%02d:%02d:%03d %6.6s - %6.6s", MINUTES(TrackRecords.RecordLap[j].Time), SECONDS(TrackRecords.RecordLap[j].Time), THOUSANDTHS(TrackRecords.RecordLap[j].Time), TrackRecords.RecordLap[j].Player, TrackRecords.RecordLap[j].Car);
                    DrawGameText(16, (short)(128 + j * 16), 0x00ffff, buf);
                }
            }
            if (recordshow == 2)
            {
                for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
                {
                    wsprintf(buf, "%02d:%02d:%03d %6.6s - %6.6s", MINUTES(TrackRecords.RecordRace[j].Time), SECONDS(TrackRecords.RecordRace[j].Time), THOUSANDTHS(TrackRecords.RecordRace[j].Time), TrackRecords.RecordRace[j].Player, TrackRecords.RecordRace[j].Car);
                    DrawGameText(16, (short)(128 + j * 16), 0x00ffff, buf);
                }
            }
#endif

// paws?

            if ((gMenuHeader.pNextMenu != NULL) || (gMenuHeader.pMenu != NULL)) 
            {
                MoveResizeMenu(&gMenuHeader);
                ProcessMenuInput(&gMenuHeader);
//              if ((Version != VERSION_DEV) || !(Keys[DIK_SPACE] && Keys[DIK_LSHIFT]))
//                  DrawMenu(&gMenuHeader);

                // Set Game Settings that may have been altered in the menu
                SetMenuGameSettings();

                // Continue with game
                if ((gMenuHeader.pMenu == NULL) && GameSettings.Paws && GameLoopQuit == GAMELOOP_QUIT_OFF && ChampionshipEndMode == CHAMPIONSHIP_END_WAITING_FOR_FINISH) {
                    GameSettings.Paws = FALSE;
                    ResumeAllSfx();
                }
            }

// Show credits?

            if (CreditVars.State != CREDIT_STATE_INACTIVE) {
                ProcessCredits();
                DrawCredits();
            }

// Demo Stuff

            lastButton = button;
            button = GetDigitalState(&KeyTable[KEY_PAUSE]);
            if ((Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE]) || (button && !lastButton))
            {
                if (GameSettings.GameType != GAMETYPE_DEMO)
                {
                    if (!GameSettings.Paws)
                    {
                        GameSettings.Paws = TRUE;
                        PauseAllSfx();
//$REVISIT - might actually want this, depending on how we implement vibration
//$REMOVED
//                        SetSafeAllJoyForces();
//$END_REMOVAL

                        if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP && PLR_LocalPlayer->RaceFinishTime && !AllPlayersFinished)
                        {
                            ForceAllCarFinish();
                        }
                        else
                        {
                            ClearMenuHeader(&gMenuHeader);
                            SetNextMenu(&gMenuHeader, &Menu_InGame);
                        }
                    }
                }
            }

// physics info

#if SHOW_PHYSICS_INFO
            if (Keys[DIK_F11] && !LastKeys[DIK_F11]) PhysicsInfoTog = !PhysicsInfoTog;
            if (PhysicsInfoTog) ShowPhysicsInfo();
#endif

// read zzz

#if CHECK_ZZZ
            CheckZZZ();
#endif

        }
    }

// global pickup flash?

    if (GlobalPickupFlash)
        RenderGlobalPickupFlash();

// fade shit

    DrawFadeShit();

// render in game menu
    if ((gMenuHeader.pNextMenu != NULL) || (gMenuHeader.pMenu != NULL)) 
    {
        if ((Version != VERSION_DEV) || !(Keys[DIK_SPACE] && Keys[DIK_LSHIFT]))
        {
            SetViewport(0.0f, 0.0f, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);
            DrawMenu(&gMenuHeader);
        }
    }

// end render

//$REMOVED    D3Ddevice->EndScene();

//$REMOVED -  stupid security check
//// add NFO to 'Event'
//
//    Event += NFO;
//$END_REMOVAL
}

//////////////////////
// ghost takeover ? //
//////////////////////
#if GHOST_TAKEOVER

extern GHOST_DATA *BestGhostData;

long GhostWeapon[] = {
    OBJECT_TYPE_SHOCKWAVE,
    OBJECT_TYPE_FIREWORK,
    OBJECT_TYPE_WATERBOMB,
    OBJECT_TYPE_ELECTROPULSE,
    OBJECT_TYPE_CHROMEBALL,
};

static void GhostTakeover(void)
{
    long i, keys;
    static bool ForceGhostCam = FALSE;

// Don't fuck up edit mode

    if ((CAM_MainCamera->Type == CAM_EDIT) || (CAM_MainCamera->Type == CAM_FREEDOM))
        return;

    assert( NULL != GHO_GhostPlayer );  //$ADDITION [If we hit this assert, might need to put back CPrince's code to return here if (NULL == GHO_GhostPlayer) ]

// get key state

    keys = (PLR_LocalPlayer->controls.dx | PLR_LocalPlayer->controls.dy);

    if (!keys) for (i = 0 ; i < 256 ; i++)
    {
        if ((keys = Keys[i])) break;
    }

    if (Keys[DIK_F6] && !LastKeys[DIK_F6]) {
        ForceGhostCam = !ForceGhostCam;
        if (ForceGhostCam) {
            SetCameraRail(CAM_MainCamera, GHO_GhostPlayer->ownobj, 1);
        } else {
            GhostTakeoverFlag = FALSE;
            GhostTakeoverTime = 0;
            SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, 0);
        }
    }

    if (Keys[DIK_F7] && !LastKeys[DIK_F7]) {
        if (CAM_MainCamera->Object == GHO_GhostPlayer->ownobj) {
            CAM_MainCamera->Object = PLR_LocalPlayer->ownobj;
        } else {
            CAM_MainCamera->Object = GHO_GhostPlayer->ownobj;
        }
        InitCamPos(CAM_MainCamera);
    }

// human

    if (!GhostTakeoverFlag && !ForceGhostCam)
    {
        GhostTakeoverTime += (long)(TimeStep * 1000.0f);
        if (keys) GhostTakeoverTime = 0;
        if (GhostTakeoverTime > GHOST_TAKEOVER_TIME)
        {
            GhostTakeoverFlag = TRUE;
            SetCameraRail(CAM_MainCamera, GHO_GhostPlayer->ownobj, 1);
        }
    }

// ghost

    else
    {
        if (GHO_BestFrame > GHO_BestGhostInfo->NFrames - 3)
        {
            GHO_BestFrame = 0;
            GHO_GhostPlayer->car.CurrentLapStartTime = TimerCurrent - 
                (GHO_GhostPlayer->car.CurrentLapTime - BestGhostData[GHO_BestGhostInfo->NFrames - 1].Time);

        }

// Change camera?

        if (CamTime > ChangeCamTime) {
            REAL choice;

            CamTime = ZERO;
            ChangeCamTime = Real(2) + frand(6);

            choice = frand(100);
            if (choice < 50) {  
                // Static camera
                SetCameraRail(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_RAIL_STATIC_NEAREST);
            } else if (choice < 58) {
                // Follow camera
                SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_BEHIND);
            } else if (choice < 66) {
                // Follow camera close
                SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_CLOSE);
            } else if (choice < 74) {
                // Side Camera
                SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_LEFT);
            } else if (choice < 82) {
                // Side Camera
                SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_RIGHT);
            } else if (choice < 90) {
                // Rear View Camera
                SetCameraFollow(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_FOLLOW_FRONT);
            } else {
                // In car camera
                SetCameraAttached(CAM_MainCamera, GHO_GhostPlayer->ownobj, CAM_ATTACHED_INCAR);
            }
        } else {
            CamTime += TimeStep;
        }

        if (keys && !ForceGhostCam)
        {
            GhostTakeoverFlag = FALSE;
            GhostTakeoverTime = 0;
            SetCameraFollow(CAM_MainCamera, PLR_LocalPlayer->ownobj, 0);
        }
    }
}
#endif

//////////////
// read zzz //
//////////////

#if CHECK_ZZZ

static void CheckZZZ(void)
{
    FILE *fp;
    char name[256], action[256], mess[256], user[256];
    static char lastmp3[256], lastmess[256];
    static float time = 0.0f;
    short ch, i;
    DWORD size;

// last mess

    DrawExtraBigGameText(0, 128, 0xffff00, lastmess);

// not all the time!

    time -= TimeStep;
    if (time > 0.0f)
        return;

    time = 10.0f;

    lastmess[0] = 0;

    size = 256;
    GetUserName(user, &size);

// file exists?

    if ((fp = fopen("n:\\!\\!!!!!!!!", "r")) == NULL)
        return;

// parse

    while (TRUE)
    {

// get a name

        if (fscanf(fp, "%s%s", name, action) == EOF)
            break;

// me?

        if (!strcmp(name, "ALL") || !strcmp(name, user))
        {

// quit?

            if (!strcmp(action, "QUIT"))
            {
                g_bQuitGame = TRUE;
                return;
            }

// get message

            do {
                ch = fgetc(fp);
            } while (ch != '\'' && ch != EOF);

            i = 0;
            while (TRUE)
            {
                ch = fgetc(fp);
                if (ch != '\'' && ch != EOF && i < 255)
                {
                    mess[i] = (char)ch;
                    i++;
                }
                else
                {
                    mess[i] = 0;
                    break;
                }
            }

// text?

            else if (!strcmp(action, "TEXT"))
            {
                strcpy(lastmess, mess);
            }
        }

// next line

        do {
            ch = fgetc(fp);
        } while (ch != '\n' && ch != EOF);
    }

// close

    fclose(fp);

}
#endif

/////////////////////////////////////////////////////////////////////
//
// DefaultPhysicsLoop:
//
/////////////////////////////////////////////////////////////////////
void DefaultPhysicsLoop()
{
    unsigned long iStep;
    REAL oldTimeStep = TimeStep;

    unsigned long totalTime;


    // Calculate the time step for this set of physics loops
    //MS2TIME ((_t) * (TimerFreq / 1000))
    TimeStep = PHYSICSTIMESTEP / 1000.0f;
    totalTime = TimerDiff + OverlapTime;
    //NPhysicsLoops = totalTime / MS2TIME(PHYSICSTIMESTEP);
    NPhysicsLoops = (totalTime * 1000) / (PHYSICSTIMESTEP * TimerFreq);
    //OverlapTime = totalTime - (NPhysicsLoops * MS2TIME(PHYSICSTIMESTEP)); 
    OverlapTime = totalTime - ((NPhysicsLoops * (PHYSICSTIMESTEP * TimerFreq)) / 1000); 

#ifndef GAZZA_TEACH_CAR_HANDLING
    if (NPhysicsLoops > 10) {
        TotalRacePhysicsTime += (NPhysicsLoops - 10) * PHYSICSTIMESTEP;
        NPhysicsLoops = 10;
    }
#endif

    // do the physics loops
    for (iStep = 0; iStep < NPhysicsLoops; iStep++) {

        // Update AI time step
        CAI_UpdateTimeStep(PHYSICSTIMESTEP);

        // Get control inputs
        CON_DoPlayerControl();

        // Update Gravity Field
        REAL time = TotalRacePhysicsTime / 1000.0f;
        UpdateGravityField(time, CurrentLevelInfo.RockX, CurrentLevelInfo.RockZ, CurrentLevelInfo.RockTimeX, CurrentLevelInfo.RockTimeZ);

        // Store replay data
//      if (CountdownTime == 0) {
            RPL_StoreAllObjectReplayData();
//      }

        //GRD_UpdateObjGrid();

        // Move game objects
        MOV_MoveObjects();

        // Deal with the collisions of the objects
        COL_DoObjectCollisions();

        // Update Wind force field
        UpdateWindField();

        // Calculate time for ghost, replay and remote interpolation
        TotalRacePhysicsTime += PHYSICSTIMESTEP;

    }

    // clean up
    TimeStep = oldTimeStep;

}


/////////////////////////////////////////////////////////////////////
//
// ReplayPhysicsLoop:
//
/////////////////////////////////////////////////////////////////////
extern REPLAY_DATA *ReplayDataPtr;
void ReplayPhysicsLoop()
{
    REAL oldTimeStep = TimeStep;

    unsigned long iStep;
    unsigned long totalTime;

    // Calculate the time step for this set of physics loops
    totalTime = TimerDiff + OverlapTime;
    NPhysicsLoops = totalTime / MS2TIME(PHYSICSTIMESTEP);
    TimeStep = PHYSICSTIMESTEP / 1000.0f;
    if (NPhysicsLoops > 10) NPhysicsLoops = 10;
    OverlapTime = totalTime - (NPhysicsLoops * MS2TIME(PHYSICSTIMESTEP)); 

    // do the physics loops
    for (iStep = 0; iStep < NPhysicsLoops; iStep++) {

        // Calculate time for ghost, replay and remote interpolation
//      if (CountdownTime == 0)
            TotalRacePhysicsTime += PHYSICSTIMESTEP;

        // Store replay data
        RPL_RestoreAllObjectReplayData();

        // Get control inputs
        CON_DoPlayerControl();

        // Update Gravity Field
        REAL time = TotalRacePhysicsTime / 1000.0f;
        UpdateGravityField(time, CurrentLevelInfo.RockX, CurrentLevelInfo.RockZ, CurrentLevelInfo.RockTimeX, CurrentLevelInfo.RockTimeZ);

        //GRD_UpdateObjGrid();

        // Move game objects
        MOV_MoveObjects();

        // Deal with the collisions of the objects
        COL_DoObjectCollisions();

        // Update Wind force field
        UpdateWindField();

    }

    // clean up
    TimeStep = oldTimeStep;

    if (Keys[DIK_W] && !LastKeys[DIK_W]) GLP_TriggerGapCamera = TRUE;

    // slow down if just passed through slow-down trigger
    if (GLP_TriggerGapCamera) {
        TimerSlowDownPercentage += RealTimeStep * 200;
        if (TimerSlowDownPercentage > Real(100)) TimerSlowDownPercentage = Real(100);

        // Has time stopped?
        if (TimerSlowDownPercentage == Real(100)) {
            // Time stopped, switch camera mode if necessary
            if (GLP_GapCameraTimer <= ZERO) {
                SetCameraFreedom(CAM_MainCamera, PLR_LocalPlayer->ownobj, 0);
                InitGapCameraSweep(CAM_MainCamera);
                GLP_GapCameraTimer = ZERO;
            }

            UpdateGapCameraSweep(CAM_MainCamera);

            // Is it time to go back to normal?
            GLP_GapCameraTimer += RealTimeStep;
            if (GLP_GapCameraTimer > TO_TIME(Real(GAP_CAMERA_TIME))) {
                GLP_TriggerGapCamera = FALSE;
                GLP_GapCameraTimer = ZERO;
            SetCameraRail(CAM_MainCamera, PLR_LocalPlayer->ownobj, CAM_RAIL_DYNAMIC_MONO);
            }
        }

    } else {
        GLP_TriggerGapCamera = FALSE;
        GLP_GapCameraTimer -= RealTimeStep;
        TimerSlowDownPercentage -= RealTimeStep * 200;
        if (TimerSlowDownPercentage < ZERO) TimerSlowDownPercentage = ZERO;
    }

}


/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
void Draw2DPolyG4(REAL posX, REAL posY, REAL sizeX, REAL sizeY)
{
    if (sizeX < 0)
    {
        posX += sizeX;
        sizeX = -sizeX;
    }
    if (sizeY < 0)
    {
        posY += sizeY;
        sizeY = -sizeY;
    }

    DrawVertsTEX0[0].sx = posX;
    DrawVertsTEX0[0].sy = posY;
    DrawVertsTEX0[1].sx = posX + sizeX;
    DrawVertsTEX0[1].sy = posY;
    DrawVertsTEX0[2].sx = posX + sizeX;
    DrawVertsTEX0[2].sy = posY + sizeY;
    DrawVertsTEX0[3].sx = posX;
    DrawVertsTEX0[3].sy = posY + sizeY;

    DrawVertsTEX0[0].color = 0x00ff00;
    DrawVertsTEX0[1].color = 0x00ff00;
    DrawVertsTEX0[2].color = 0xff0000;
    DrawVertsTEX0[3].color = 0xff0000;

    DrawVertsTEX0[0].rhw = 1.0f;
    DrawVertsTEX0[1].rhw = 1.0f;
    DrawVertsTEX0[2].rhw = 1.0f;
    DrawVertsTEX0[3].rhw = 1.0f;

    SET_TPAGE(-1);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX0, DrawVertsTEX0, 4, D3DDP_DONOTUPDATEEXTENTS);
    SET_RENDER_STATE(D3DRENDERSTATE_ALPHATESTENABLE, TRUE);
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
void RenderCircle(VEC *pPos, REAL radius, long rgb)
{
    VEC     line[16];
    REAL    angle, angleDelta;
    int     i;

// Create points
    angle = 0;
    angleDelta = RAD / 16;
    for (i = 0; i < 16; i++)
    {
        line[i].v[X] = pPos->v[X] + ((REAL)sin(angle) * radius);
        line[i].v[Y] = pPos->v[Y];
        line[i].v[Z] = pPos->v[Z] + ((REAL)cos(angle) * radius);
        angle += angleDelta;
    }

// Render lines
    for (i = 0; i < 15; i++)
        DrawLine(&line[i], &line[i+1], rgb,rgb);
    DrawLine(&line[15], &line[0], rgb,rgb);
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
void RenderAINodeInfo(AINODE *pNode, long rgb)
{
    VEC     line[4];
    long    rgb2[4];
    int     i;

    CopyVec(&pNode->Node[0].Pos, &line[0]);
    CopyVec(&pNode->Node[1].Pos, &line[1]);
    for (i = 0; i < MAX_AINODE_LINKS; i++)
    {
        if (pNode->Next[i])
        {
            CopyVec(&pNode->Next[i]->Node[0].Pos, &line[2]);
            CopyVec(&pNode->Next[i]->Node[1].Pos, &line[3]);
//          if (pPlayer->CarAI.RouteChoice == i)
//              rgb = 0xff0000;
//          else
//              rgb = 0x802020;

            DrawLine(&line[0], &line[1], rgb, rgb);
            DrawLine(&line[2], &line[3], rgb, rgb);
            DrawLine(&line[0], &line[2], rgb, rgb);
            DrawLine(&line[1], &line[3], rgb, rgb);

        // Draw left wall
            SET_TPAGE(-1);
            if (pNode->link.flags & AIN_LF_WALL_LEFT)
            {
                rgb2[0] = rgb2[1] = rgb2[2] = rgb2[3] = 0x80FF80c0;
                CopyVec(&pNode->Node[0].Pos, &line[0]);
                CopyVec(&pNode->Node[0].Pos, &line[1]);
                CopyVec(&pNode->Next[i]->Node[0].Pos, &line[2]);
                CopyVec(&pNode->Next[i]->Node[0].Pos, &line[3]);
                line[1].v[Y] -= Real(100);
                line[2].v[Y] -= Real(100);

                DrawNearClipPolyTEX0(line, rgb2, 4);
            }
            if (pNode->link.flags & AIN_LF_WALL_RIGHT)
            {
                rgb2[0] = rgb2[1] = rgb2[2] = rgb2[3] = 0x8080FFc0;
                CopyVec(&pNode->Node[1].Pos, &line[0]);
                CopyVec(&pNode->Node[1].Pos, &line[1]);
                CopyVec(&pNode->Next[i]->Node[1].Pos, &line[2]);
                CopyVec(&pNode->Next[i]->Node[1].Pos, &line[3]);
                line[1].v[Y] -= Real(100);
                line[2].v[Y] -= Real(100);

                DrawNearClipPolyTEX0(line, rgb2, 4);
            }


        // Render bounds
#if 0
            VEC     line2[4];

            line2[0].v[X] = line2[3].v[X] = pNode->linkInfo[i].boundsMin[X];
            line2[1].v[X] = line2[2].v[X] = pNode->linkInfo[i].boundsMax[X];
            line2[0].v[Z] = line2[1].v[Z] = pNode->linkInfo[i].boundsMin[Y];
            line2[2].v[Z] = line2[3].v[Z] = pNode->linkInfo[i].boundsMax[Y];
            line2[0].v[Y] = line2[1].v[Y] = line2[2].v[Y] = line2[3].v[Y] = pNode->Centre.v[Y];
            DrawLine(&line2[0], &line2[1], 0x004000, 0x004000);
            DrawLine(&line2[1], &line2[2], 0x004000, 0x004000);
            DrawLine(&line2[2], &line2[3], 0x004000, 0x004000);
            DrawLine(&line2[3], &line2[0], 0x004000, 0x004000);
#endif
        }
    }
}

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
#ifdef GAZZA_TEACH_CAR_HANDLING

void DisplayNeural(PLAYER *pPlayer)
{
    int i;
    char string[256];
    int x,y;
    int rgb;
    static int iLast = -1;
    static int iLastLast = -1;

    x = 30;
    y = 30;

    for (i = 0; i < 10; i++)
    {
        if (i == pPlayer->CarAI.iLastNeuron)
        {
            if (iLast != pPlayer->CarAI.iLastNeuron)
            {
                iLastLast = iLast;
                iLast = pPlayer->CarAI.iLastNeuron;
            }

            rgb = 0xFF0000;
            sprintf(string, "%.2f (%.2f)", pPlayer->CarAI.neuralCur[i],
                                           pPlayer->CarAI.neuralCur[i] - pPlayer->CarAI.neuralLastMod[i]);
        }
        else
        if (i == iLastLast)
        {
            rgb = 0xFFFF00;
            sprintf(string, "%.2f (%.2f)", pPlayer->CarAI.neuralCur[i],
                                           pPlayer->CarAI.neuralCur[i] + pPlayer->CarAI.neuralLastMod[i]);
        }
        else
        {
            rgb = 0xFFFFFF;
            sprintf(string, "%.2f", pPlayer->CarAI.neuralCur[i]);
        }

        DrawGameText(x,y, rgb, string);

        sprintf(string, "%.2f", pPlayer->CarAI.neuralMin[i]);
        DrawGameText( (short)(x+150), (short)(y), rgb, string );
        sprintf(string, "%.2f", pPlayer->CarAI.neuralMax[i]);
        DrawGameText( (short)(x+225), (short)(y), rgb, string );

        y += 20;
    }
}
#endif


////////////////////////////////////////////////////////////////
//
// ChangeMainCamera:
//
////////////////////////////////////////////////////////////////
REAL gCameraObjectChangeTimer = ZERO;
REAL gCameraObjectChangeTime = 10.0f;

void ChangeMainCamera(CAMERA *camera)
{
    int playerNum;
    PLAYER *player;

    if (GameSettings.GameType == GAMETYPE_DEMO) {
        // time to change player which camera follows?
        gCameraObjectChangeTimer += TimeStep;
        if (gCameraObjectChangeTimer > gCameraObjectChangeTime) {
            gCameraObjectChangeTimer = ZERO;
            gCameraObjectChangeTime = 5.0f + frand(10.0f);
            playerNum = rand() % StartData.PlayerNum;
            player = PLR_PlayerHead;
            while ((--playerNum > 0) && (player->next != NULL)) {
                player = player->next;
            };
            camera->Object = player->ownobj;
        }
    }

    if (ReplayMode) {
        player = camera->Object->player;
        if (Keys[DIK_PGUP] && !LastKeys[DIK_PGUP]) {
            player = camera->Object->player->next;
            if (player == NULL) {
                player = PLR_PlayerHead;
            }
        }
        if (Keys[DIK_PGDN] && !LastKeys[DIK_PGDN]) {
            player = camera->Object->player->prev;
            if (player == NULL) {
                player = PLR_PlayerTail;
            }
        }
        camera->Object = player->ownobj;

        gCameraObjectChangeTimer += TimeStep;
        if (gCameraObjectChangeTimer > gCameraObjectChangeTime) {
            SetRandomCameraType(camera, camera->Object);
            gCameraObjectChangeTime = 2.0f + frand(5.0f);
            gCameraObjectChangeTimer = ZERO;
        }

    }
}

////////////////////////////////////////////////////////////////
//
// Set Game Settings from titlescreen vars
//
////////////////////////////////////////////////////////////////

void SetMenuGameSettings()
{
    RenderSettings.Env = RegistrySettings.EnvFlag = g_TitleScreenData.shinyness;
    RenderSettings.Light = RegistrySettings.LightFlag = g_TitleScreenData.lights;
    //RenderSettings.Instance = RegistrySettings.InstanceFlag = g_TitleScreenData.instances;
    RenderSettings.Mirror = RegistrySettings.MirrorFlag = g_TitleScreenData.reflections;
    RenderSettings.Shadow = RegistrySettings.ShadowFlag = g_TitleScreenData.shadows;
    RenderSettings.Skid = RegistrySettings.SkidFlag = g_TitleScreenData.skidmarks;

    SetNearFar(NEAR_CLIP_DIST, GET_DRAW_DIST(CurrentLevelInfo.FarClip));
    SetFogVars(GET_FOG_START(CurrentLevelInfo.FogStart), CurrentLevelInfo.VertFogStart, CurrentLevelInfo.VertFogEnd);

    SetWorldMirror();
}


////////////////////////////////////////////////////////////////
//
// InitCameraSweep:
//
////////////////////////////////////////////////////////////////

void InitGapCameraSweep(CAMERA *camera)
{
    GLP_GapCameraPos[0].v[X] = camera->Object->body.Centre.Pos.v[X] + GLP_GapCameraMat.m[RX] * 150;
    GLP_GapCameraPos[0].v[Y] = camera->Object->body.Centre.Pos.v[Y] - 50;
    GLP_GapCameraPos[0].v[Z] = camera->Object->body.Centre.Pos.v[Z] + GLP_GapCameraMat.m[RZ] * 150;

    GLP_GapCameraPos[1].v[X] = camera->Object->body.Centre.Pos.v[X] + GLP_GapCameraMat.m[LX] * 150;;
    GLP_GapCameraPos[1].v[Y] = camera->Object->body.Centre.Pos.v[Y] - 50;
    GLP_GapCameraPos[1].v[Z] = camera->Object->body.Centre.Pos.v[Z] + GLP_GapCameraMat.m[LZ] * 150;

    GLP_GapCameraPos[2].v[X] = camera->Object->body.Centre.Pos.v[X] - GLP_GapCameraMat.m[RX] * 150;
    GLP_GapCameraPos[2].v[Y] = camera->Object->body.Centre.Pos.v[Y] - 50;
    GLP_GapCameraPos[2].v[Z] = camera->Object->body.Centre.Pos.v[Z] - GLP_GapCameraMat.m[RZ] * 150;
}

void UpdateGapCameraSweep(CAMERA *camera)
{
    REAL t;
    VEC camPos;
    MAT camMat;

    t = (REAL)sin(GLP_GapCameraTimer * PI * HALF / GAP_CAMERA_TIME);
    Limit(t, ZERO, ONE);

    Interpolate3D(&GLP_GapCameraPos[0], &GLP_GapCameraPos[1], &GLP_GapCameraPos[2], t, &camPos);

    VecMinusVec(&camera->Object->body.Centre.Pos, &camPos, &camMat.mv[L]);
    NormalizeVec(&camMat.mv[L]);
    BuildMatrixFromLook(&camMat);

    CopyMat(&camMat, &camera->WMatrix);
    CopyVec(&camPos, &camera->WPos);
    MatToQuat(&camera->WMatrix,&camera->Quat);
}
