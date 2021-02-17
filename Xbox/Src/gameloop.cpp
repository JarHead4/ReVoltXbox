//-----------------------------------------------------------------------------
// File: gameloop.cpp
//
// Desc: Main game loop code.
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#define CARAI_DEBUG     // For drawing AI graphical info.

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
#include "EditAi.h"
#include "EditObject.h"
#include "EditZone.h"
#include "instance.h"
#include "player.h"
#include "timing.h"
#include "ghost.h"
#include "settings.h"
#include "EditTrigger.h"
#include "trigger.h"
#include "EditCam.h"
#include "EditField.h"
#include "panel.h"
#include "ai.h"
#include "EditPortal.h"
#include "weapon.h"
#ifdef _PC
#include "spark.h"
#include "EditAi.h"
#include "aizone.h"
#endif
#include "replay.h"
#include "initplay.h"
#include "EditPos.h"
#include "pickup.h"
#include "field.h"
#include "shareware.h"
#include "credits.h"
#include "gamegauge.h"
#include "cheats.h"

#include "MusicManager.h"
#include "SoundEffectEngine.h"
#include "VoiceManager.h"

#include "ui_Menu.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_menu.h"
#include "ui_MenuText.h"
#include "ui_InGameMenu.h"
#include "net_xonline.h"
#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
#include "net_Statistics.h"
#endif // ENABLE_STATISTICS
#include "ui_menudraw.h"
#include "ui_LiveSignOn.h"
#include "FriendsManager.h"

#define DEMO_FLASH_TIME     TO_TIME(Real(1.2))
#define DEMO_TEXT_WIDTH     12
#define DEMO_TEXT_HEIGHT    16

#define GAP_CAMERA_TIME     TO_TIME(Real(1.5))
#define GAP_CAMERA_MIN_TIME TO_TIME(Real(30))

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

void DisplayCarAIInfo(PLAYER *pPlayer);


unsigned __int64 g_DrawMask = ~0; // $ADDED jedl - for debug and perf, clear bits to skip drawing parts of the scene

//-----------------------------------------------------------------------------
void GLP_GameLoop(void)
{
#if SCREEN_TIMES
    long j;
#endif
    long i, car;
    LEVELINFO *li;

// update game gauge?

    if (GAME_GAUGE)
    {
        UpdateGameGaugeVars();
    }

// quit?

    if (GetFadeEffect() == FADE_DOWN_DONE && GameLoopQuit != GAMELOOP_QUIT_OFF)
    {
        // straight to demo?
        if (GoStraightToDemo)
        {
            SET_EVENT(SetupSharewareIntro);
            if (IsMultiPlayer())
            {
//$REMOVED                KillNetwork();
            }

            LEV_EndLevelStageTwo();
            LEV_EndLevelStageOne();
            return;
        }

        // act on gameloop flag
        switch (GameLoopQuit)
        {
            // game gauge quit game
            case GAMELOOP_QUIT_GAMEGAUGE:
                QuitGame();

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
//$REMOVED          KillNetwork();
//$REMOVED          LobbyFree();
                }

                //$HACK: We're always telling the online APIs the user signed in on controller 0.
                RemoveOnlinePresenceFlag( 0, XONLINE_FRIENDSTATE_FLAG_PLAYING );
                LEV_EndLevelStageTwo();
                LEV_EndLevelStageOne();
                return;

            // restart game
            case GAMELOOP_QUIT_RESTART:
                RegistrySettings.PositionSave = FALSE;  // $TEMPORARY jedl - Turn off car position saving on restart

                // Set new race data, in case the player changed he between-game options
                //$MODIFIED (JHarding): Used to set everything to be whatever the title
                // screen var said, but this is wrong.  When restarting, we should use
                // whatever the host told us in StartData.
                // StartData.Laps = GameSettings.NumberOfLaps = gTitleScreenVars.numberOfLaps;
                GameSettings.NumberOfLaps = StartData.Laps;
                StartData.AllowPickups = GameSettings.AllowPickups = gTitleScreenVars.pickUps;

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

                // For single player games, advance the track (if necessary )
                if( !IsMultiPlayer() )
                {
                    if( GameSettings.RandomTrack )
                    {
                        LEV_EndLevelStageTwo();
                        LEV_EndLevelStageOne();

                        LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
                        LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
                        LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start

                        GameSettings.Level = PickRandomTrack();
                        li = GetLevelInfo(GameSettings.Level);
                        strncpy(StartData.LevelDir, li->szDir, MAX_PATH);
                    }
                    else 
                    {
                        long track = GetLevelNum(StartData.LevelDir);
                        if (track != -1 && track != GameSettings.Level)
                        {
                            GameSettings.Level = track;

                            LEV_EndLevelStageTwo();
                            LEV_EndLevelStageOne();

                            LoadMipTexture("D:\\gfx\\font.bmp", TPAGE_FONT, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
                            LoadMipTexture("D:\\gfx\\loading.bmp", TPAGE_LOADING, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start
                            LoadMipTexture("D:\\gfx\\spru.bmp", TPAGE_SPRU, 256, 256, 0, 1, FALSE); //$MODIFIED: added "D:\\" at start

                            li = GetLevelInfo(GameSettings.Level);
                            strncpy(StartData.LevelDir, li->szDir, MAX_PATH);
                        }
                    }
                }

                EnableLoadThread(((GameSettings.RandomTrack || (IsMultiPlayer() && RestartData.NewTrack)) ? STAGE_ONE_LOAD_COUNT : STAGE_TWO_LOAD_COUNT) + StartData.PlayerNum);
                SetupLevelAndPlayers();
                DisableLoadThread();

#ifdef ENABLE_STATISTICS
//$REVISIT: Statistics disabled for July Consumer Beta
                // Do work specific to XOnline networking
                if( gTitleScreenVars.bUseXOnline )
                {
                    // Updated Number of Started Races/Battles Statistics
                    StatsLocalPlayersStartingMatch();
                }
#endif // ENABLE_STATISTICS
                return;

            // replay
            case GAMELOOP_QUIT_REPLAY:
                RPL_RecordReplay = FALSE;
                RPL_TerminateReplay();
                memcpy(&StartDataStorage, &StartData, sizeof(START_DATA));
                GameSettings.GameType = GAMETYPE_REPLAY;

                if (IsMultiPlayer())
                {
//$REMOVED          KillNetwork();
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

    // Read input from gamepads
    ReadJoystick();

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
    if( IsMultiPlayer() )
    {
        g_VoiceManager.ProcessVoice();
    }
    g_FriendsManager.Process();

// reset oil slick list

    ResetOilSlickList();

// remote sync?

    if (IsServer() && NextSyncReady && !CountdownTime && !ReplayMode)
    {
        SendPlayerSync();
    }

// send/receive multiplayer messages

    if (IsMultiPlayer())
    {
        TransmitRemoteObjectData();
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

        if (((GameSettings.GameType == GAMETYPE_DEMO) || ReplayMode) && !GAME_GAUGE) {
            ChangeMainCamera(CAM_MainCamera);
        }

    }

// Build car world matrices

    BuildAllCarWorldMatrices();

// Process lights

    ProcessLights();

//$ADDITION
    // Pump any pending XOnline tasks
    OnlineTasks_Continue();
    if( IsLoggedIn(0) )
    {
        // BUGBUG: Should make this flash for a couple seconds, then disappear
        //$HACK: We're always telling the online APIs the user signed in on controller 0.
        if( XOnlineGetNotification( 0, XONLINE_NOTIFICATION_GAME_INVITE ) )
        {
            DrawScreenSpaceQuad( 560, 420, g_pGameInviteReceivedTexture );
        }
        if( XOnlineGetNotification( 0, XONLINE_NOTIFICATION_FRIEND_REQUEST ) )
        {
            DrawScreenSpaceQuad( 520, 420, g_pFriendReqReceivedTexture );
        }
    }
//$END_ADDITION

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

        // $ADDED(jedl) clear block toggle
        if (g_DrawMask & (1 << 0))  // $ADDED jedl - debug and perf
        {
        // $END_ADDITION
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
        // $ADDED(jedl) clear block toggle
        }
        // $END_ADDITION

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
            if (g_DrawMask & (1 << 1))  // $ADDED jedl - debug and perf
                DrawWorld();
            if (g_DrawMask & (1 << 2))  // $ADDED jedl - debug and perf
                DrawInstances();
        }
        if (g_DrawMask & (1 << 3))  // $ADDED jedl - debug and perf
        DrawObjects();
        if (g_DrawMask & (1 << 4))  // $ADDED jedl - debug and perf
        DrawAllCars();
        if (g_DrawMask & (1 << 5))  // $ADDED jedl - debug and perf
        DrawAllPickups();

// draw countdown

        if (Camera[CameraCount].Flag == CAMERA_FLAG_PRIMARY)
            if (g_DrawMask & (1 << 6))  // $ADDED jedl - debug and perf
            DrawCountdown();

// DRAW object at line-following camera pos
        //long nodeNum, linkNum;
        //FindNearestCameraPath(&PLR_LocalPlayer->car.Body->Centre.Pos, &nodeNum, &linkNum);
        //DrawModel(&PLR_LocalPlayer->car.Models->Body[0], &Identity, &CAM_NodeCamPos, MODEL_PLAIN);

// render edit models?

        if (EditMode != EDIT_NONE)
        {
            FlushPolyBuckets();

            if (EditMode == EDIT_LIGHTS) DrawFileLights();
            if (EditMode == EDIT_VISIBOXES) DrawVisiBoxes();
            if (EditMode == EDIT_OBJECTS) DrawFileObjects();
            if (EditMode == EDIT_AINODES || (EditMode == EDIT_OBJECTS && ShowAiNodes)) DrawAiNodes();
            if (EditMode == EDIT_ZONES) DrawFileZones();
            if (EditMode == EDIT_TRIGGERS) DrawTriggers();
            if (EditMode == EDIT_CAM || EditMode == EDIT_TRIGGERS) DrawEditCamNodes();
            if (EditMode == EDIT_FIELDS) DrawFields();
            if (EditMode == EDIT_PORTALS) DrawPortals();
            if (EditMode == EDIT_POSNODE) DrawPosNodes();
        }

// draw ai 'current node'

#ifdef CARAI_DEBUG
        if ((Version == VERSION_DEV) && gGazzasAICar)
        {
            if (PLR_LocalPlayer->CarAI.pCurNode)
            {
                DrawModel(&EditAiNodeModel[1], &IdentityMatrix, &PLR_LocalPlayer->CarAI.pCurNode->Node[0].Pos, MODEL_PLAIN);
                DrawModel(&EditAiNodeModel[0], &IdentityMatrix, &PLR_LocalPlayer->CarAI.pCurNode->Node[1].Pos, MODEL_PLAIN);
            }
        }
#endif

// draw 3d poly list

        if (g_DrawMask & (1 << 7))  // $ADDED jedl - debug and perf
        Draw3dPolyList();

// flush poly buckets

        FlushPolyBuckets();

// render semi polys

        if (g_DrawMask & (1 << 8))  // $ADDED jedl - debug and perf
        DrawSemiList();
        if (g_DrawMask & (1 << 9))  // $ADDED jedl - debug and perf
        DrawSparks();
        if (g_DrawMask & (1 << 10)) // $ADDED jedl - debug and perf
        DrawTrails();
        if (g_DrawMask & (1 << 11)) // $ADDED jedl - debug and perf
        DrawSkidMarks();
        if (g_DrawMask & (1 << 12)) // $ADDED jedl - debug and perf
        DrawAllCarShadows();
        if (g_DrawMask & (1 << 13)) // $ADDED jedl - debug and perf
        DrawAllGhostCars();

// draw drum?

        if (Camera[CameraCount].Flag == CAMERA_FLAG_PRIMARY)
            if (g_DrawMask & (1 << 14)) // $ADDED jedl - debug and perf
            DrawDrum();

// flush env buckets

        FlushEnvBuckets();

// Render motion blur

        if (!GameSettings.Paws)
        {
            if (g_DrawMask & (1 << 15)) // $ADDED jedl - debug and perf
            MotionBlurRender();
        }

// underwater?

        if (Camera[CameraCount].UnderWater)
        {
            if (g_DrawMask & (1 << 16)) // $ADDED jedl - debug and perf
            RenderUnderWaterPoly();
        }
    
// draw practise star?

        if (GameSettings.GameType == GAMETYPE_PRACTICE)
            DrawPracticeStars();

// display network players

        if (IsMultiPlayer())
        {
            if (g_DrawMask & (1 << 17)) // $ADDED jedl - debug and perf
            DisplayPlayers();
        }

// draw world wireframe?

        if (Wireframe)
        {
            if (g_DrawMask & (1 << 18)) // $ADDED jedl - debug and perf
            DrawWorldWireframe();
        }

// begin primary camera stuff

        if (Camera[CameraCount].Flag == CAMERA_FLAG_PRIMARY)
        {

// sepia stuff?

            if (RenderSettings.Sepia)
            {
                if (g_DrawMask & (1 << 19)) // $ADDED jedl - debug and perf
                DrawSepiaShit();
            }

// edit mode?

            if (EditMode != EDIT_NONE)
            {
                if (EditMode == EDIT_LIGHTS) EditFileLights();
                if (EditMode == EDIT_VISIBOXES) EditVisiBoxes();
                if (EditMode == EDIT_OBJECTS) EditFileObjects();
                if (EditMode == EDIT_INSTANCES) EditInstances();
                if (EditMode == EDIT_AINODES) EditAiNodes();
                if (EditMode == EDIT_ZONES) EditFileZones();
                if (EditMode == EDIT_TRIGGERS) EditTriggers();
                if (EditMode == EDIT_CAM) EditCamNodes();
                if (EditMode == EDIT_FIELDS) EditFields();
                if (EditMode == EDIT_PORTALS) EditPortals();
                if (EditMode == EDIT_POSNODE) EditPosNodes();
            }

// draw control panel

            if ((GameSettings.GameType != GAMETYPE_REPLAY) && (GameSettings.GameType != GAMETYPE_DEMO) && !GAME_GAUGE)
            {
                if (g_DrawMask & (1 << 20)) // $ADDED jedl - debug and perf
                DrawControlPanel();
            }

// game gauge info

            if (GAME_GAUGE)
            {
                if (g_DrawMask & (1 << 21)) // $ADDED jedl - debug and perf
                DrawGameGaugeInfo();
            }

            // $ADDED(jedl) whole text block toggle
            if (g_DrawMask & (1 << 22)) // $ADDED jedl - debug and perf
            {
            // $END_ADDITION

// begin text state

            BeginTextState();

            // display DEMO mode message
            if( GameSettings.GameType == GAMETYPE_DEMO ) 
            {
                gDemoTimer += TimeStep;
                gDemoFlashTimer += TimeStep;
                if( gDemoFlashTimer > DEMO_FLASH_TIME ) 
                {
                    gDemoFlashTimer = ZERO;
                    gDemoShowMessage = !gDemoShowMessage;
                }

                // See if we want to quit demo because the demo timed out, or
                // the user hit a button on a gamepad
                if( ( ( (DemoTimeout > ZERO) && gDemoTimer > DemoTimeout ) || g_bAnyButtonPressed ) && 
                    ( GetFadeEffect() != FADE_DOWN ) ) 
                {
                    SetFadeEffect( FADE_DOWN );
#ifdef OLD_AUDIO
                    SetRedbookFade( REDBOOK_FADE_DOWN );
#endif // OLD_AUDIO

                    GameLoopQuit = g_bAnyButtonPressed ? GAMELOOP_QUIT_FRONTEND : GAMELOOP_QUIT_DEMO;
                }
                
                if( g_CreditVars.State == CREDIT_STATE_INACTIVE ) 
                {
                    DrawDemoMessage();
                    DrawDemoLogo(ONE, 0);
                }
            }

            // display replay message
            if( GameSettings.GameType == GAMETYPE_REPLAY ) 
            {
                DrawReplayMessage();
            }

// championship end screens

            if (GameSettings.GameType == GAMETYPE_CHAMPIONSHIP)
            {
                MaintainChampionshipEndScreens();
            }

// display edit info?

            if (CAM_MainCamera->Type == CAM_EDIT)
            {
                if (EditMode == EDIT_LIGHTS && CurrentEditLight) DisplayLightInfo(CurrentEditLight);
                if (EditMode == EDIT_VISIBOXES && CurrentVisiBox) DisplayVisiBoxInfo(CurrentVisiBox);
                if (EditMode == EDIT_OBJECTS && CurrentFileObject) DisplayFileObjectInfo(CurrentFileObject);
                if (EditMode == EDIT_INSTANCES && CurrentInstance) DisplayInstanceInfo(CurrentInstance);
                if (EditMode == EDIT_AINODES && CurrentEditAiNode) DisplayAiNodeInfo(CurrentEditAiNode);
                if (EditMode == EDIT_ZONES && CurrentFileZone) DisplayZoneInfo(CurrentFileZone);
                if (EditMode == EDIT_TRIGGERS && CurrentTrigger) DisplayTriggerInfo(CurrentTrigger);
                if (EditMode == EDIT_CAM && CurrentEditCamNode) DisplayCamNodeInfo(CurrentEditCamNode);
                if (EditMode == EDIT_FIELDS && CurrentField) DisplayFieldInfo(CurrentField);
                if (EditMode == EDIT_PORTALS && CurrentPortal) DisplayPortalInfo(CurrentPortal);

                DrawMousePointer(0xffffff);
            }

            if (EditMode == EDIT_VISIBOXES) DisplayCamVisiMask();
            if (EditMode == EDIT_ZONES) DisplayCurrentTrackZone();
            if (EditMode == EDIT_POSNODE) DisplayCurrentFinishDist();

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

            //swprintf(buf, L"MinDist = %d", (int)(1000.0f * LOSMinDist1));
            //DumpText(100, 50, 16, 24, 0xffffff, buf);

#ifdef SHIPPING
            // Code below is disabled in shipping versions.
#else // !SHIPPING
            if (Version == VERSION_DEV)
            {
                if (g_DrawMask & (1 << 23)) // $ADDED jedl - debug and perf
                if (RegistrySettings.bGraphicsDebug) // $NOTE(jedl) - this is turned off for gamebash
                {
                    WCHAR buf[256];
                    swprintf(buf, L"State %d, %d  Lights %d  Car %d, %d, %d  Objects %d  Sfx %d, %d", RenderStateChange, TextureStateChange, TotalLightCount, (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[X], (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[Y], (long)PLR_LocalPlayer->car.Body->Centre.Pos.v[Z], OBJ_NumObjects, ChannelsUsed, ChannelsUsed3D);
                    DumpText(100, 40, 8, 16, 0xffffff, buf); //$MODIFIED (changed x,y from 0,0 to these new coords)

//$REMOVED                    if (ChannelsUsed == SfxSampleNum) DumpText(0, 128, 8, 16, 0xffff00, "Sfx Channels Full!");

                    swprintf(buf, L"Cube %d, %d  World %ld  Model %ld  Semi %d  Physics time %ld  Sparks %ld", WorldBigCubeCount, WorldCubeCount, WorldPolyCount, ModelPolyCount, SemiCount, TotalRacePhysicsTime, NActiveSparks);
                    DumpText(100, 60, 8, 16, 0xffffff, buf); //$MODIFIED (changed x,y from 0,464 to these new coords)
                    // $BEGIN_TEMPORARY jedl --- Show state of position saving and draw mode
                    if (RegistrySettings.PositionSave)
                        DumpText(100, 80, 8, 16, 0xffffff, L"PositionSave");
                    if (RegistrySettings.bUseGPU)
                        DumpText(200, 80, 8, 16, 0xffffff, L"bUseGPU");
                    swprintf(buf, L"Frame time %.3f ms", FrameRate_GetMsecPerFrame());
                    DumpText(100, 100, 8, 16, 0xffffff, buf);
                    // $END_TEMPORARY
                }
            }
#endif // !SHIPPING

            // Car AI Vars
#ifdef CARAI_DEBUG
            if ((Version == VERSION_DEV) && gGazzasAICarDraw)
                DisplayCarAIInfo(&Players[0]);
#endif

// record times

#if SCREEN_TIMES
            // $ADDED(jedl) screen text block toggle
            if (g_DrawMask & (1 << 24))
            {
            // $END_ADDITION
                
            static long recordshow = 0;
            if (Keys[DIK_F10] && !LastKeys[DIK_F10]) recordshow = (recordshow + 1) % 3;

            if (recordshow == 1)
            {
                for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
                {
                    char buf[256];
                    sprintf(buf, "%02d:%02d:%03d %6.6s - %6.6s", MINUTES(TrackRecords.RecordLap[j].Time), SECONDS(TrackRecords.RecordLap[j].Time), THOUSANDTHS(TrackRecords.RecordLap[j].Time), TrackRecords.RecordLap[j].Player, TrackRecords.RecordLap[j].Car);
                    DumpText(16, 128 + j * 16, 8, 16, 0x00ffff, buf);
                }
            }
            if (recordshow == 2)
            {
                for (j = 0 ; j < MAX_RECORD_TIMES ; j++)
                {
                    char buf[256];
                    sprintf(buf, "%02d:%02d:%03d %6.6s - %6.6s", MINUTES(TrackRecords.RecordRace[j].Time), SECONDS(TrackRecords.RecordRace[j].Time), THOUSANDTHS(TrackRecords.RecordRace[j].Time), TrackRecords.RecordRace[j].Player, TrackRecords.RecordRace[j].Car);
                    DumpText(16, 128 + j * 16, 8, 16, 0x00ffff, buf);
                }
            }
            // $ADDED(jedl) screen text block toggle
            } 
            // $END_ADDITION
#endif
            
            // $ADDED(jedl) whole text block toggle
            } 
            // $END_ADDITION

// paws?


            // Show credits?
            if (g_CreditVars.State != CREDIT_STATE_INACTIVE) {
                ProcessCredits();
                DrawCredits();
            }


            // Check to see if we should enter InGameMenu
            if( GetFullScreenMenuInput(FALSE) == MENU_INPUT_SELECT )
            {
                if (GameSettings.GameType != GAMETYPE_DEMO)  //$REVISIT: should we be checking for other cases here (eg, replay mode, gamegauge)
                {
                    if (!GameSettings.Paws)
                    {
                        // Initiate  in game menu, ensure no parent menu present
                        g_pMenuHeader->ClearMenuHeader();

                        // $MD ADDED
                        // remove contosl mesages
                        SetConsoleMessage(NULL, NULL, 0, 0, 10, CONSOLE_MESSAGE_FOREVER);
                        g_InGameMenuStateEngine.MakeActive( NULL);
                    }
                }
            }

            if( g_pActiveStateEngine )
            {
                if( GameSettings.GameType != GAMETYPE_DEMO )  //$ADDITION(cprince): want to turn off FrontEnd menu drawing during demo mode
                                                              //$REVISIT: should we be checking for other cases here (eg, replay mode, gamegauge)
                {
                    // Pass control to the active state engine
                    // state engine if exists will process menu
                    g_pActiveStateEngine->Process();
                }
            }
            else if ((g_pMenuHeader->m_pNextMenu != NULL) || (g_pMenuHeader->m_pMenu != NULL)) 
            {
                // legacy menu must still get processed if it exists
                g_pMenuHeader->MoveResizeMenu();
                g_pMenuHeader->ProcessMenuInput();

                // Set Game Settings that may have been altered in the menu
                SetMenuGameSettings();

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
        if (g_DrawMask & (1 << 25)) // $ADDED jedl - debug and perf
        RenderGlobalPickupFlash();

// fade shit

    if (g_DrawMask & (1 << 26)) // $ADDED jedl - debug and perf
    DrawFadeShit();

// render in game menu

    if (g_DrawMask & (1 << 27)) // $ADDED jedl - debug and perf
    if (!GAME_GAUGE)
    {
        if ((g_pMenuHeader->m_pNextMenu != NULL) || (g_pMenuHeader->m_pMenu != NULL)) 
        {
            if ((Version != VERSION_DEV) || !(Keys[DIK_SPACE] && Keys[DIK_LSHIFT]))
            {
                SetViewport(0.0f, 0.0f, (float)ScreenXsize, (float)ScreenYsize, RenderSettings.GeomPers);
                g_pMenuHeader->DrawMenu();
            }
        }
    }

// end render

    // $BEGIN_TEMPORARY jedl - save car position every second to be loaded in
    //                         for next editing session, if PositionSave flag is set.
    if (RegistrySettings.PositionSave)
    {
        static DWORD TimerSaved = 0;
        if (TimerCurrent - TimerSaved > MS2TIME(1000))
        {
            TimerSaved = TimerCurrent;
            CHAR buf[_MAX_PATH];
            sprintf(buf, "%s\\SavedPosition.txt", CurrentLevelInfo.szDir);
            FILE *fp = fopen(buf, "wb");
            if (fp != NULL)
            {
                // Write position and orientation to file
                for (int j = 0; j < 3; j++)
                    fprintf(fp, "%g ", PLR_LocalPlayer->car.Body->Centre.Pos.v[j]);
                fprintf(fp, "\n");
                for (int i = 0; i < 3; i++)
                {
                    for (int j = 0; j < 3; j++)
                        fprintf(fp, "%g ", PLR_LocalPlayer->car.Body->Centre.WMatrix.m[i * 3 + j]);
                    fprintf(fp, "\n");
                }
                fclose(fp);
            }
        }
    }
    // $END_TEMPORARY
    
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

    DumpText(0, 128, 16, 32, 0xffff00, lastmess);

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
                QuitGame();
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

#ifdef OLD_AUDIO
            // NOTE (JHarding): Might want to add debug control over
            // music at some point
            // play MP3?

            if (!strcmp(action, "MP3"))
            {
                if (Version == VERSION_SHAREWARE)
                {
                    if (strcmp(lastmp3, mess))
                    {
                        strcpy(lastmp3, mess);
                        StopMP3();
                        PlayMP3(mess);
                    }
                }
            }
#endif

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
// DisplayCarAIInfo()
/////////////////////////////////////////////////////////////////////

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

        DumpText(x,y, 8,16, rgb, string);

        sprintf(string, "%.2f", pPlayer->CarAI.neuralMin[i]);
        DumpText( x+150, y, 8, 16, rgb, string );
        sprintf(string, "%.2f", pPlayer->CarAI.neuralMax[i]);
        DumpText( x+225, y, 8, 16, rgb, string );

        y += 20;
    }
}
#endif

/////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////
extern char *PriorityEnum[];

void DisplayCarAIInfo(PLAYER *pPlayer)
{
    WCHAR       string[256];
    AIZONE      *pZone;
    AINODE      *pNode, *pNodeN, *pNodeT;
    AINODE_LINKINFO *pLinkInfo, *pLinkInfoN;
    VEC         line[4];
    bool        a,b;
    int         side;
    int         i,j;
    REAL        t;

#ifdef GAZZA_TEACH_CAR_HANDLING
        DisplayNeural(pPlayer);
//      return;
#endif

    if (pPlayer->CarAI.CurZone < 0)
        return;

    BeginTextState();

    if ((Version == VERSION_DEV) && gGazzasRouteChoice)
    {
        if (giGazzaForceRoute != LONG_MAX)
        {
            swprintf(string, L"Route: %S", PriorityEnum[giGazzaForceRoute]);
            DumpText(300,10, 8,16, 0xff0000, string);
        }

        if (gGazzasOvertake)
            DumpText(300,30, 8,16, 0xff0000, L"Overtake");
    }

    swprintf(string, L"AngVel: %.2f,%.2f", pPlayer->car.Body->FrontPointVelDotRight, pPlayer->car.Body->RearPointVelDotRight);
    DumpText(100,10, 8,16, 0xffffff, string);

    swprintf(string, L"AI State: %S (Route: %d, Over: %d)", gCAI_StateDebugStrings[pPlayer->CarAI.AIState],
                                             pPlayer->CarAI.iRouteCurNode, pPlayer->CarAI.bOvertake);
    DumpText(100,30, 8,16, 0xffffff, string);

    swprintf(string, L"CurZone/ID: %d,%d", pPlayer->CarAI.CurZone, pPlayer->CarAI.CurZoneID);
    DumpText(100,50, 8,16, 0xffffff, string);

    a = (bool)pPlayer->CarAI.pCurNode;
    b = (bool)pPlayer->CarAI.pLastValidNode;
    side = Sign(pPlayer->CarAI.exitSideDist);
    if (a)
        swprintf(string, L"Node Cur/Last/ExitSide: %d,%d,%d (%S)", a,b,side,PriorityEnum[pPlayer->CarAI.pCurNode->Priority]);
    else
        swprintf(string, L"Node Cur/Last/ExitSide: %d,%d,%d (%S)", a,b,side,"OUTSIDE NODE");

    DumpText(100,70, 8,16, 0xffffff, string);

    // Draw car's front over/understeer bar
    Draw2DPolyG4(320,32,    pPlayer->car.Body->FrontPointVelDotRight * 0.5f, 10);
    Draw2DPolyG4(320,32+12, pPlayer->car.Body->RearPointVelDotRight  * 0.5f, 10);


    // Draw car's destination forward vector
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[0]);
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[1]);
    line[1].v[0] += pPlayer->CarAI.forwardVecDest.v[0] * 1000;
    line[1].v[1] += pPlayer->CarAI.forwardVecDest.v[1] * 1000;
    line[1].v[2] += pPlayer->CarAI.forwardVecDest.v[2] * 1000;
    DrawLine(&line[0], &line[1], 0xffff00, 0xffff00);

    // Draw car's destination right vector
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[0]);
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[1]);
    line[1].v[0] += pPlayer->CarAI.rightVec.v[0] * 250;
    line[1].v[1] += pPlayer->CarAI.rightVec.v[1] * 250;
    line[1].v[2] += pPlayer->CarAI.rightVec.v[2] * 250;
    DrawLine(&line[0], &line[1], 0x808000, 0x808000);

    // Draw car's destination position
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[0]);
    CopyVec(&pPlayer->CarAI.destPos, &line[1]);
    DrawLine(&line[0], &line[1], 0xffff80, 0xffff80);
    RenderCircle(&pPlayer->CarAI.destPos, pPlayer->CarAI.biasSize, 0xFFFF00);

    // Draw car's forward vector
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[0]);
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[1]);
    line[1].v[0] += pPlayer->car.Body->Centre.WMatrix.mv[L].v[0] * pPlayer->CarAI.lookAheadDist;
    line[1].v[1] += pPlayer->car.Body->Centre.WMatrix.mv[L].v[1] * pPlayer->CarAI.lookAheadDist;
    line[1].v[2] += pPlayer->car.Body->Centre.WMatrix.mv[L].v[2] * pPlayer->CarAI.lookAheadDist;
    DrawLine(&line[0], &line[1], 0xff0000, 0xff0000);

    // Draw future position
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[0]);
    CopyVec(&pPlayer->CarAI.futurePos, &line[1]);
    DrawLine(&line[0], &line[1], 0xff00FF, 0xff00FF);

    // Draw nodes in current zone
    if (pZone = &AiZones[pPlayer->CarAI.CurZone])
    {
        pNodeT = pZone->FirstNode;
        for (i = 0; i < pZone->Count; i++, pNodeT = pNodeT->ZoneNext)
        {
            RenderAINodeInfo(pNodeT, 0x008000);

            for (j = 0; j < MAX_AINODE_LINKS; j++)
            {
                if (pNodeT->CheckNext[j])
                    RenderAINodeInfo(pNodeT->Next[j], 0x80ff80);
            }
        }
    }

    // Draw future node
    if (pPlayer->CarAI.pFutureNode)
        RenderAINodeInfo(pPlayer->CarAI.pFutureNode, 0xFF00FF);

    // Draw last valid node bounds
    if (pPlayer->CarAI.pLastValidNode && (pPlayer->CarAI.pLastValidNode != pPlayer->CarAI.pCurNode))
        RenderAINodeInfo(pPlayer->CarAI.pLastValidNode, 0xFF0000);

    if (!(pNode = pPlayer->CarAI.pCurNode))
        return;
    if (!(pNodeN = pNode->Next[pPlayer->CarAI.iRouteCurNode]))
        return;
    pLinkInfo = &pNode->link;
    pLinkInfoN = &pNodeN->link;

#ifndef GAZZA_TEACH_CAR_HANDLING
    BeginTextState();

    VEC *tvec[3];
    tvec[0] = &pLinkInfo->forwardVec;
    tvec[1] = &pLinkInfo->rightVec;
    tvec[2] = &pPlayer->car.Body->Centre.WMatrix.mv[L];
    swprintf(string, L"Racing F,R Vec: %d (%.2f,%.2f,%.2f) (%.2f,%.2f,%.2f)",
                     pLinkInfo->dir,
                     tvec[0]->v[X], tvec[0]->v[Y], tvec[0]->v[Z],
                     tvec[1]->v[X], tvec[1]->v[Y], tvec[1]->v[Z]);
    DumpText(100,90, 8,16, 0xffffff, string);

    swprintf(string, L"Angle to RL: %.2f (%.2f,%d) (%.2f,%.2f,%.2f)", pPlayer->CarAI.cosAngleToRacingLine,
                                                                     pPlayer->CarAI.raceLineSide,
                                                                     pPlayer->CarAI.raceLineFaceDir,
                                                                     tvec[2]->v[X], tvec[2]->v[Y], tvec[2]->v[Z]);
    DumpText(100,110, 8,16, 0xffffff, string);

    // Controls DX,DY
    swprintf(string, L"DX, DY: %d,%d (%d,%d)", pPlayer->CarAI.dx, pPlayer->CarAI.dy, pPlayer->controls.dx, pPlayer->controls.dy);
    DumpText(100,130, 8,16, 0xffffff, string);

    // Distance along node
    swprintf(string, L"Node dist: %d", (int)pPlayer->CarAI.distAlongNode);
    DumpText(100,150, 8,16, 0xffffff, string);

    // Racing line speed
    swprintf(string, L"Speed Rating: %d%%", (int)(pLinkInfo->speed * 100));
    DumpText(100,170, 8,16, 0xffffff, string);
#endif


    // Draw Racing line
    t = pNode->RacingLine;
    line[0].v[X] = pNode->Node[0].Pos.v[X] + MulScalar((pNode->Node[1].Pos.v[X] - pNode->Node[0].Pos.v[X]), t);
    line[0].v[Y] = pNode->Node[0].Pos.v[Y] + MulScalar((pNode->Node[1].Pos.v[Y] - pNode->Node[0].Pos.v[Y]), t);
    line[0].v[Z] = pNode->Node[0].Pos.v[Z] + MulScalar((pNode->Node[1].Pos.v[Z] - pNode->Node[0].Pos.v[Z]), t);
    t = pNodeN->RacingLine;
    line[1].v[X] = pNodeN->Node[0].Pos.v[X] + MulScalar((pNodeN->Node[1].Pos.v[X] - pNodeN->Node[0].Pos.v[X]), t);
    line[1].v[Y] = pNodeN->Node[0].Pos.v[Y] + MulScalar((pNodeN->Node[1].Pos.v[Y] - pNodeN->Node[0].Pos.v[Y]), t);
    line[1].v[Z] = pNodeN->Node[0].Pos.v[Z] + MulScalar((pNodeN->Node[1].Pos.v[Z] - pNodeN->Node[0].Pos.v[Z]), t);
    DrawLine(&line[0], &line[1], 0x00FF00, 0x00FF00);

    // Draw Overtaking line
    t = pNode->OvertakingLine;
    line[0].v[X] = pNode->Node[0].Pos.v[X] + MulScalar((pNode->Node[1].Pos.v[X] - pNode->Node[0].Pos.v[X]), t);
    line[0].v[Y] = pNode->Node[0].Pos.v[Y] + MulScalar((pNode->Node[1].Pos.v[Y] - pNode->Node[0].Pos.v[Y]), t);
    line[0].v[Z] = pNode->Node[0].Pos.v[Z] + MulScalar((pNode->Node[1].Pos.v[Z] - pNode->Node[0].Pos.v[Z]), t);
    t = pNodeN->OvertakingLine;
    line[1].v[X] = pNodeN->Node[0].Pos.v[X] + MulScalar((pNodeN->Node[1].Pos.v[X] - pNodeN->Node[0].Pos.v[X]), t);
    line[1].v[Y] = pNodeN->Node[0].Pos.v[Y] + MulScalar((pNodeN->Node[1].Pos.v[Y] - pNodeN->Node[0].Pos.v[Y]), t);
    line[1].v[Z] = pNodeN->Node[0].Pos.v[Z] + MulScalar((pNodeN->Node[1].Pos.v[Z] - pNodeN->Node[0].Pos.v[Z]), t);
    DrawLine(&line[0], &line[1], 0xFF0000, 0xFF0000);



    // Draw forward vector
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[0]);
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[1]);
    line[1].v[0] += pLinkInfo->forwardVec.v[0] * 1000;
    line[1].v[1] += pLinkInfo->forwardVec.v[1] * 1000;
    line[1].v[2] += pLinkInfo->forwardVec.v[2] * 1000;
    DrawLine(&line[0], &line[1], 0xffffff, 0xffffff);

    // Draw right vector
    CopyVec(&pNode->Centre, &line[0]);
    CopyVec(&pNode->Centre, &line[1]);
    line[1].v[0] += pLinkInfo->rightVec.v[0] * 250;
    line[1].v[1] += pLinkInfo->rightVec.v[1] * 250;
    line[1].v[2] += pLinkInfo->rightVec.v[2] * 250;
    DrawLine(&line[0], &line[1], 0x808080, 0x808080);

    // Draw node bounds
    RenderAINodeInfo(pNode, 0x0000FF);

    // Draw lines to nearest cars
    CopyVec(&pPlayer->car.Body->Centre.Pos, &line[0]);
    if (pPlayer->CarAI.pNearestCarInFront)
    {
        CopyVec(&pPlayer->CarAI.pNearestCarInFront->car.Body->Centre.Pos, &line[1]);
        DrawLine(&line[0], &line[1], 0xff0000, 0x800000);
    }
    if (pPlayer->CarAI.pNearestCarBehind)
    {
        CopyVec(&pPlayer->CarAI.pNearestCarBehind->car.Body->Centre.Pos, &line[1]);
        DrawLine(&line[0], &line[1], 0x00ff00, 0x008000);
    }


    // Restore text draw state
    BeginTextState();
}



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
    RenderSettings.Env = RegistrySettings.EnvFlag = gTitleScreenVars.shinyness;
    RenderSettings.Light = RegistrySettings.LightFlag = gTitleScreenVars.lights;
    //RenderSettings.Instance = RegistrySettings.InstanceFlag = gTitleScreenVars.instances;
    RenderSettings.Mirror = RegistrySettings.MirrorFlag = gTitleScreenVars.reflections;
    RenderSettings.Shadow = RegistrySettings.ShadowFlag = gTitleScreenVars.shadows;
    RenderSettings.Skid = RegistrySettings.SkidFlag = gTitleScreenVars.skidmarks;

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
