//-----------------------------------------------------------------------------
// File: LevelLoad.cpp
//
// Desc: Code and data used when loading a level.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#ifndef _PSX
#include "main.h"
#include "dx.h"
#include "geom.h"
#include "particle.h"
#include "texture.h"
#include "model.h"
#include "aerial.h"
#include "NewColl.h"
#include "body.h"
#include "car.h"
#include "camera.h"
#include "light.h"
#include "world.h"
#include "draw.h"
#include "visibox.h"
#include "texture.h"
#include "ctrlread.h"
#include "object.h"
#include "control.h"
#include "LevelLoad.h"
#include "player.h"
#include "EditObject.h"
#include "instance.h"
#include "editai.h"
#include "editzone.h"
#include "aizone.h"
#include "settings.h"
#include "field.h"
#include "ghost.h"
#include "mirror.h"
#include "timing.h"
#include "trigger.h"
#include "EditTrigger.h"
#include "editcam.h"
#include "EditField.h"
#include "aizone.h"
#include "ainode.h"
#include "input.h"
#include "obj_init.h"
#include "ai.h"
#include "EditPortal.h"
#include "text.h"
#include "initplay.h"
#include "editpos.h"
#endif
#ifdef _PC
#include "spark.h"
#include "panel.h"
#endif
#include "LevelInfo.h"
#include "pickup.h"

#include "SoundEffectEngine.h"
#include "MusicManager.h"

//
// Static function prototypes
//

static bool IsUserLevel(const char *szDir);
//$REMOVED_NOTEXIST    static void s_LoadTrackData(void);
static bool LoadLevelFields();

static bool LoadTrackDataStageOne();
static void UnloadTrackDataStageOne();
static bool LoadTrackDataStageTwo();
static void UnloadTrackDataStageTwo();
static void LoadStaticLevelModels(void);

//
// Global function prototypes
//

bool LEV_InitLevelStageOne();
void LEV_EndLevelStageOne();
bool LEV_InitLevelStageTwo();
void LEV_EndLevelStageTwo();

LEVELINFO *GetLevelInfo(int levelNum);


//
// Global variables
//

VEC     LEV_StartPos;
REAL    LEV_StartRot; 
long    LEV_StartGrid;
VEC     *LEV_LevelFieldPos = NULL;
MAT     *LEV_LevelFieldMat = NULL;

static short LoadingPos;


//////////////////////////////
// load static level models //
//////////////////////////////

void LoadStaticLevelModels(void)
{
    struct renderflags rflag;

    rflag.envmap = FALSE;

    LoadOneLevelModel(LEVEL_MODEL_PICKUP, FALSE, rflag, 0);
    LoadOneLevelModel(LEVEL_MODEL_FIREWORK, FALSE, rflag, TPAGE_FX1);
    LoadOneLevelModel(LEVEL_MODEL_WATERBOMB, FALSE, rflag, 0);
    LoadOneLevelModel(LEVEL_MODEL_CHROMEBALL, FALSE, rflag, 0);
    LoadOneLevelModel(LEVEL_MODEL_BOMBBALL, FALSE, rflag, 0);
    StarModelNum = LoadOneLevelModel(LEVEL_MODEL_STAR, FALSE, rflag, 0);
}


////////////////////////////////////////////////////////////////
//
// Stage I level loading and initialisation:
// for things that need only be done once, no matter how many 
// times the level is played
//
////////////////////////////////////////////////////////////////

bool LEV_InitLevelStageOne()
{
    LEVELINFO *levelInfo; 

    Assert(GameSettings.LoadStage == LOAD_STAGE_ZERO);

    // Find info about the level
    levelInfo = GetLevelInfo(GameSettings.Level);
    if (levelInfo == NULL) {
        DumpMessage(NULL, "Level Number out of bounds");
        QuitGame();
        return FALSE;
    } 

    // Load current level info
    LoadCurrentLevelInfo(levelInfo->szName, levelInfo->szDir); 

    // Initialise grids
    //if (!GRD_AllocGrids())
    //{
    //  QuitGame();
    //  return FALSE;
    //}

    // Initialise object system
    if (!OBJ_InitObjSys())
    {
        DumpMessage(NULL, "Can't initialise object system!");
        QuitGame();
        return FALSE;
    }

    // Set up current level fogging and colour information
    SetNearFar(NEAR_CLIP_DIST, GET_DRAW_DIST(CurrentLevelInfo.FarClip));
    SetFogVars(GET_FOG_START(CurrentLevelInfo.FogStart), CurrentLevelInfo.VertFogStart, CurrentLevelInfo.VertFogEnd);
    SetBackgroundColor(CurrentLevelInfo.FogColor);
    FOG_COLOR(BackgroundColor);

    // set start pos / rot /grid
    if (GameSettings.Reversed)
    {
        LEV_StartPos = CurrentLevelInfo.ReverseStartPos;
        LEV_StartRot = CurrentLevelInfo.ReverseStartRot;
        LEV_StartGrid = CurrentLevelInfo.ReverseStartGrid;
    }
    else
    {
        LEV_StartPos = CurrentLevelInfo.NormalStartPos;
        LEV_StartRot = CurrentLevelInfo.NormalStartRot;
        LEV_StartGrid = CurrentLevelInfo.NormalStartGrid;
    }

    // init jump spark offsets
    InitJumpSparkOffsets();

    // Initialise the field array (must be done before any objects with fields are loaded or initialised)
    InitFields();

    // Add gravity
    FLD_GravityField = AddLinearField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &ZeroVector, &Identity, &FLD_GlobalBBox, &FLD_GlobalSize, &DownVec, FLD_Gravity, ZERO, FIELD_ACC);

    // Load any level data that does not need to be reloaded on a restart
    if (!LoadTrackDataStageOne()) {
        QuitGame();
        return FALSE;
    }

    GameSettings.LoadStage = LOAD_STAGE_ONE;

    return TRUE;
}

////////////////////////////////////////////////////////////////
//
// LEV_EndLevelStageOne: undo everything that was done in
// stage one initialiser (in reverse order)
//
////////////////////////////////////////////////////////////////

void LEV_EndLevelStageOne()
{
    Assert(GameSettings.LoadStage == LOAD_STAGE_ONE);

    // Free stuff loaded in LoadTrackDataStageOne
    UnloadTrackDataStageOne();

    // kill object system
    OBJ_KillObjSys();

    // Kill grid buffers
    //GRD_FreeGrids();

    GameSettings.LoadStage = LOAD_STAGE_ZERO;
}

////////////////////////////////////////////////////////////////
//
// Init Level Stage II:
// for things that must be redone when the level is restarted
// (Stage one must have been called once before any calls to this)
//
////////////////////////////////////////////////////////////////

bool LEV_InitLevelStageTwo()
{
    int i;

    Assert(GameSettings.LoadStage == LOAD_STAGE_ONE);

    // Make sure objects always have same replay ID when restarting
    ReplayID = 0;
    MinReplayID = 0;

    // Initialise player structures
    PLR_InitPlayers();

    // init level model system
    InitLevelModels();

    // init poly buckets
    InitPolyBuckets();

    // init skidmarks
    ClearSkids();

    // Init spark engine
    InitSparks();

    // init cameras
    InitCameras();
    CAM_MainCamera = AddCamera(0, 0, 0, 0, CAMERA_FLAG_PRIMARY);
    GameSettings.DrawFollowView = FALSE;
    if (GameSettings.DrawRearView)
    {
        CAM_RearCamera = AddCamera(96, 32, 144, 108, CAMERA_FLAG_SECONDARY);
    }
    

    // init console
    InitConsole();

    // zero finish table
    for (i = 0 ; i < MAX_NUM_PLAYERS ; i++)
    {
        FinishTable[i].Time = 0;
    }

    // Reset pickups
    InitPickupArray();

    // Reset the wind force field if there is one
    InitWindField();

    // Load any level data that does need to be reloaded on a restart
    if (!LoadTrackDataStageTwo()) {
        QuitGame();
        return FALSE;
    }

    GameSettings.LoadStage = LOAD_STAGE_TWO;

    return TRUE;
}


////////////////////////////////////////////////////////////////
//
// LEV_EndLevelStageTwo: free everything allocated in 
// stage two initialisation.
//
////////////////////////////////////////////////////////////////

void LEV_EndLevelStageTwo()
{
    Assert(GameSettings.LoadStage == LOAD_STAGE_TWO);

    // Free everything loaded in LoadTrackDataStageTwo
    UnloadTrackDataStageTwo();
    
    // Free all players
    PLR_KillAllPlayers();
    PLR_LocalPlayer = NULL;
    GHO_GhostPlayer = NULL;

    // Free all objects
    OBJ_KillAllObjects();

    // Free ghost Light if there was one
    ReleaseGhostLight();

    // Remove all pickups
    FreeAllPickups();

    // misc
    ReplayID = 0;
    MinReplayID = 0;

    GameSettings.LoadStage = LOAD_STAGE_ONE;
}

////////////////////////////////////////////////////////////////
//
// LoadTrackDataStageOne:
//
////////////////////////////////////////////////////////////////

bool LoadTrackDataStageOne()
{
    int ii;
    char buf[256];
    FILE *fp;

#ifdef OLD_AUDIO
    //$NOTE: corresponding code is LoadSfx() in LoadTrackDataStageTwo()
#else // !OLD_AUDIO
    // Load sfx
    if( FAILED( g_SoundEngine.LoadSounds( "d:\\levels\\common\\sounds_common.sfx" ) ) )
    {
        DumpMessage( "Error", "Couldn't load common sound effects." );
    }

    char str[256];
    sprintf( str, "%s\\sounds_%s.sfx", CurrentLevelInfo.szDir, CurrentLevelInfo.szName);
    if( FAILED( g_SoundEngine.LoadSounds( str, &g_dwLevelSoundsOffset ) ) )
    {
        DumpMessage( "Warning", "Couldn't load level-specific sound effects." );
    }
#endif // !OLD_AUDIO

//$ADDITION(jedl) - world resource loading
    if( LoadWorldGPU(GetLevelFilename("w", FILENAME_MAKE_BODY)) != S_OK )
    {
        CHAR *file = GetLevelFilename("w", FILENAME_MAKE_BODY);
        CHAR strMessage[1000];
        sprintf(strMessage, "World \"%s\" resources not loaded. Will export old resources.", file);
        DumpMessage(NULL, strMessage);
    }
//$END_ADDITION

    // Load textures
//$ADDITION(jedl) - instead of loading textures from bmp files,
// look up texture resources in world resource bundle.
  if (World.m_pXBR != NULL)
  {
    // $NOTE When we get rid of the old-style rendering, tpages won't be needed.

    LoadTextureGPU("fxpage1", "D:\\gfx\\fxpage1.bmp", TPAGE_FX1, World.m_pXBR);
    LoadTextureGPU("fxpage2", "D:\\gfx\\fxpage2.bmp", TPAGE_FX2, World.m_pXBR);
    LoadTextureGPU("fxpage3", "D:\\gfx\\fxpage3.bmp", TPAGE_FX3, World.m_pXBR);

    LoadTextureGPU("envstill", CurrentLevelInfo.EnvStill, TPAGE_ENVSTILL, World.m_pXBR);
    LoadTextureGPU("envroll",  CurrentLevelInfo.EnvRoll, TPAGE_ENVROLL, World.m_pXBR);
    LoadTextureGPU("shadow" ,  "D:\\gfx\\shadow.bmp", TPAGE_SHADOW, World.m_pXBR);

    IncLoadThreadUnitCount();

    for (ii = 0 ; ii < TPAGE_WORLD_NUM ; ii++)
    {
        // skip texture that aren't specified
        sprintf(buf, "%s\\%s%c.bmp", CurrentLevelInfo.szDir, CurrentLevelInfo.szName, ii + 'a');
        if( -1 == GetFileAttributes(buf) )
            continue;
            
        char strIdentifier[128];
        sprintf(strIdentifier, "%s%c", CurrentLevelInfo.szName, ii + 'a');
        LoadTextureGPU(strIdentifier, buf, ii, World.m_pXBR);
        
    }
  }
  else
  {
//$END_ADDITION
//$MODIFIED
//    LoadMipTexture("gfx\\fxpage1.bmp", TPAGE_FX1, 256, 256, 0, 1, FALSE);
//    LoadMipTexture("gfx\\fxpage2.bmp", TPAGE_FX2, 256, 256, 0, 1, FALSE);
//    LoadMipTexture("gfx\\fxpage3.bmp", TPAGE_FX3, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\fxpage1.bmp", TPAGE_FX1, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\fxpage2.bmp", TPAGE_FX2, 256, 256, 0, 1, FALSE);
    LoadMipTexture("D:\\gfx\\fxpage3.bmp", TPAGE_FX3, 256, 256, 0, 1, FALSE);
//$END_MODIFICATIONS

    LoadTextureClever(CurrentLevelInfo.EnvStill, TPAGE_ENVSTILL, 256, 256, 0, FxTextureSet, TRUE);
    LoadTextureClever(CurrentLevelInfo.EnvRoll, TPAGE_ENVROLL, 256, 256, 0, FxTextureSet, TRUE);
//$MODIFIED
//    LoadTextureClever("gfx\\shadow.bmp", TPAGE_SHADOW, 256, 256, 0, FxTextureSet, TRUE);
    LoadTextureClever("D:\\gfx\\shadow.bmp", TPAGE_SHADOW, 256, 256, 0, FxTextureSet, TRUE);
//$END_MODIFICATIONS

    IncLoadThreadUnitCount();

    for (ii = 0 ; ii < TPAGE_WORLD_NUM ; ii++)
    {
//$MODIFIED
//        sprintf(buf, "levels\\%s\\%s%c.bmp", CurrentLevelInfo.Dir, CurrentLevelInfo.Dir, ii + 'a');
        sprintf(buf, "%s\\%s%c.bmp", CurrentLevelInfo.szDir, CurrentLevelInfo.szName, ii + 'a');
//$END_MODIFICATIONS
        LoadTextureClever(buf, ii, 256, 256, 0, WorldTextureSet, TRUE);
    }
  } //<-- $ADDITION(jedl)

    IncLoadThreadUnitCount();

    // Load world model
    SetMirrorParams();
    if (GameSettings.LoadWorld) {
        LoadWorld(GetLevelFilename("w", FILENAME_MAKE_BODY));
    } else {
        World.Cube = NULL;
        World.BigCube = NULL;
        World.CubeList = NULL;
        World.BigCubeNum = 0;
        World.CubeNum = 0;
    }

    IncLoadThreadUnitCount();

    // Load visiboxes (must be before instances!)
    InitVisiBoxes();
    LoadVisiBoxes(GetLevelFilename("vis", FILENAME_MAKE_BODY));
    SetPermVisiBoxes();

    IncLoadThreadUnitCount();

    // load mirror planes (must be before instances)
    LoadMirrorPlanes(GetLevelFilename("rim", FILENAME_MAKE_BODY));
    MirrorWorldPolys();
    SetWorldMirror();

    IncLoadThreadUnitCount();

    // load instances (must be before lights!)
    LoadInstanceModels();
    LoadInstances(GetLevelFilename("fin", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
    BuildInstanceCollPolys();

    IncLoadThreadUnitCount();

    // Load collision polygon data (must be after instances!)
    if ((fp = fopen(GetLevelFilename("ncp", FILENAME_MAKE_BODY), "rb")) == NULL) {
        COL_WorldCollPoly = NULL;
        COL_NWorldCollPolys = 0;
        DumpMessage("Error", "Level has no Collision Polygon data");
        return FALSE;
    }
    if ((COL_WorldCollPoly = LoadNewCollPolys(fp, &COL_NWorldCollPolys)) == NULL) {
        DumpMessage("Error", "Collision polygon data corrupt");
        fclose(fp);
        return FALSE;
    }
    // Grid up the collision polygons (and include the instance collisions)
    if (!LoadGridInfo(fp)) {
        DumpMessage("Warning", "No collision grid information\nCollision data may be corrupt");
        fclose(fp);
    }
    fclose(fp);

    IncLoadThreadUnitCount();

    // Load lights
    InitLights();
    LoadLights(GetLevelFilename("lit", FILENAME_MAKE_BODY));

    IncLoadThreadUnitCount();

    // load ai zones
    LoadAiZones(GetLevelFilename("taz", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

    IncLoadThreadUnitCount();

    // load ai nodes (must be after ai zones)
    LoadAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
    ZoneAiNodes();

    IncLoadThreadUnitCount();

    // load force fields
    LoadLevelFields();

    IncLoadThreadUnitCount();

    // load pos nodes
    LoadPosNodes(GetLevelFilename("pan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

    IncLoadThreadUnitCount();

    // Load the camera nodes if there are any
    if ((fp = fopen(GetLevelFilename("cam", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS), "rb")) != NULL) {
        CAM_NCameraNodes = LoadCameraNodes(fp);
        fclose(fp);
    } else {
        CAM_NCameraNodes = 0;
    }

    IncLoadThreadUnitCount();

    // load countdown models
    LoadCountdownModels();

    // load drum
    LoadDrum();

    IncLoadThreadUnitCount();

    // load ai test models
    if (AI_Testing)
    {
        LoadEditAiNodeModels();
    }

    // Load edit lights
    if (EditMode == EDIT_LIGHTS)
    {
        LoadEditLightModels();
    }

    // Load edit objects
    if (EditMode == EDIT_OBJECTS)
    {
        InitFileObjects();
        LoadFileObjects(GetLevelFilename("fob", FILENAME_MAKE_BODY));
        LoadFileObjectModels();
    }

    // load edit visicock
    if (EditMode == EDIT_VISIBOXES)
    {
        if (!AI_Testing) LoadEditAiNodeModels();
    }

    // Load edit ai nodes
    if (EditMode == EDIT_AINODES || EditMode == EDIT_OBJECTS)
    {
        InitEditAiNodes();
        LoadEditAiNodes(GetLevelFilename("fan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
        if (!AI_Testing) LoadEditAiNodeModels();
    }

    // Load edit track zones
    if (EditMode == EDIT_ZONES)
    {
        InitFileZones();
        LoadFileZones(GetLevelFilename("taz", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
        if (!AI_Testing) LoadEditAiNodeModels();
    }

    // Load edit triggers
    if (EditMode == EDIT_TRIGGERS)
    {
        InitFileTriggers();
        LoadFileTriggers(GetLevelFilename("tri", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
    }

    // Load edit cam nodes
    if (EditMode == EDIT_CAM || EditMode == EDIT_TRIGGERS)
    {
        InitEditCamNodes();
        LoadEditCamNodes(GetLevelFilename("cam", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
        LoadEditCamNodeModels();
    }

    // Load edit fields
    if (EditMode == EDIT_FIELDS)
    {
        InitFileFields();
        LoadFileFields(GetLevelFilename("fld", FILENAME_MAKE_BODY));
        LoadFileFieldModels();
    }

    // Load edit portals
    if (EditMode == EDIT_PORTALS)
    {
        LoadEditPortals(GetLevelFilename("por", FILENAME_MAKE_BODY));
    }

    // Load edit pos nodes
    if (EditMode == EDIT_POSNODE)
    {
        InitEditPosNodes();
        LoadEditPosNodeModels();
        LoadEditPosNodes(GetLevelFilename("pan", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
    }

    // Show a message moaning about too many collision polys
    if (Version == VERSION_DEV) {
        if (COL_NWorldCollPolys + COL_NInstanceCollPolys >= GRIDINDEX_MASK) {
//$REMOVED            char buf[128];
//$REMOVED            DWORD size = 128;
            unsigned long endTime;
//$REMOVED            GetUserName(buf, &size);
//$REMOVED            if ((stricmp(buf, "SHarrison") == 0) || (stricmp(buf, "PPhippen") == 0))
            {
                DumpMessage("Oy", "This world has got far too many collision polys in it\n"
                    "There should be less than 8000 world+instance collision polys\n"
                    "I'm now going to sulk for 10 seconds");
                endTime = CurrentTimer() + MS2TIME(10000);
                while (CurrentTimer() < endTime);
            }
        }
    }

    return TRUE;

}

////////////////////////////////////////////////////////////////
//
// UnloadTrackDataStageOne:
//
////////////////////////////////////////////////////////////////

void UnloadTrackDataStageOne()
{

    // kill world collision
    DestroyCollGrids();
    DestroyCollPolys(COL_WorldCollPoly);
    COL_WorldCollPoly = NULL;
    COL_NWorldCollPolys = 0;
    DestroyCollPolys(COL_InstanceCollPoly);
    COL_InstanceCollPoly = NULL;
    COL_NInstanceCollPolys = 0;

    // Free any force field stuff
    FreeForceFields();

    // kill textures
    FreeTextures();

    // kill world mesh
    FreeWorld();

    // kill poly buckets
    KillPolyBuckets();

    // free mirror planes
    FreeMirrorPlanes();

    // free instance stuff
    FreeInstanceRGBs();
    FreeInstanceModels();

    // free ai nodes
    FreeAiNodes();

    // free ai zones
    FreeAiZones();

    // free pos nodes
    FreePosNodes();

    // free edit stuff
    if (AI_Testing)
    {
        FreeEditAiNodeModels();                         // !MT! Temp moved out during AI testing
    }

    if (EditMode == EDIT_VISIBOXES)
    {
        if (!AI_Testing) FreeEditAiNodeModels();
    }

    if (EditMode == EDIT_LIGHTS)
    {
        FreeEditLightModels();
    }

    if (EditMode == EDIT_OBJECTS)
    {
        KillFileObjects();
        FreeFileObjectModels();
    }

    if (EditMode == EDIT_AINODES || EditMode == EDIT_OBJECTS)
    {
        KillEditAiNodes();
        if (!AI_Testing) FreeEditAiNodeModels();
    }

    if (EditMode == EDIT_ZONES)
    {
        KillFileZones();
        if (!AI_Testing) FreeEditAiNodeModels();
    }

    if (EditMode == EDIT_TRIGGERS)
    {
        KillFileTriggers();
    }

    if (EditMode == EDIT_CAM)
    {
        KillEditCamNodes();
        FreeEditCamNodeModels();
    }

    if (EditMode == EDIT_FIELDS)
    {
        KillFileFields();
        FreeFileFieldModels();
    }

    if (EditMode == EDIT_PORTALS)
    {
        KillEditPortals();
    }

    if (EditMode == EDIT_POSNODE)
    {
        FreeEditPosNodes();
        FreeEditPosNodeModels();
    }

    // free countdown models
    FreeCountdownModels();

    // free drum
    FreeDrum();

#ifdef OLD_AUDIO
    //$NOTE: corresponding code is FreeSfx() in UnloadTrackDataStageTwo()
#else // !OLD_AUDIO
    // kill sfx
    g_SoundEngine.Unload();
#endif // !OLD_AUDIO

}


////////////////////////////////////////////////////////////////
//
// LoadTrackDataStageTwo:
//
////////////////////////////////////////////////////////////////

bool LoadTrackDataStageTwo()
{

#ifdef OLD_AUDIO
    // load sfx
    LoadSfx(CurrentLevelInfo.Dir);
#else
    //$NOTE: corresponding code is in LoadTrackDataStageOne()
#endif

#ifdef OLD_AUDIO
    //$NOTE: corresponding code is in SetupLevelAndPlayers()
#else
    // start music
    g_MusicManager.RandomSong();
    if( RegistrySettings.MusicOn )
        g_MusicManager.Play();
#endif

    IncLoadThreadUnitCount();

    // load triggers
    LoadTriggers(GetLevelFilename("tri", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));

    IncLoadThreadUnitCount();

    // load best track times
    if (GameSettings.GameType == GAMETYPE_TRIAL)
        LoadTrackTimes(GameSettings.Level, GameSettings.Mirrored, GameSettings.Reversed);

    IncLoadThreadUnitCount();

    // load static models
    LoadStaticLevelModels();

    IncLoadThreadUnitCount();

    // load objects (should be last because they get freed and reloaded when you start a replay)
    LoadObjects(GetLevelFilename("fob", FILENAME_MAKE_BODY));

    IncLoadThreadUnitCount();

//$ADDITION(jedl) - export world if not yet bundled
#ifdef SHIPPING
// Shipping version does not need exporter code.
#else
    //$REVISIT: Jed, is this the right place to export the world?  Could do in StageOne, but static level models won't be loaded yet...
    //NOTE: This code will go away when we stop using old Acclaim asset file formats.
    if (World.m_pXBR == NULL)
    {
        // Export world in XDX resource format
        CHAR *filename = GetLevelFilename("w", FILENAME_MAKE_BODY);
        CHAR strMessage[1000];
        sprintf(strMessage, "Exporting world \"%s\".", filename);
        DumpMessage(NULL, strMessage);
        extern HRESULT ExportWorld(char *filename);
        ExportWorld(filename);
    }
#endif	
//$END_ADDITION

    return TRUE;
}


////////////////////////////////////////////////////////////////
//
// UnloadTrackDataStageTwo:
//
////////////////////////////////////////////////////////////////

void UnloadTrackDataStageTwo()
{

#ifdef OLD_AUDIO
    // stop mp3?
    if (strlen(CurrentLevelInfo.Mp3))
    {
        StopMP3();
    }

    // stop redbook?
    StopRedbook();
#else
    // stop music
    g_MusicManager.Stop();
#endif // OLD_AUDIO

#ifdef OLD_AUDIO
    // free sfx
    FreeSfx();
#else
    //$NOTE: corresponding code is in UnloadTrackDataStageOne()
#endif

    // free triggers
    FreeTriggers();

    // Save best track times
    if (GameSettings.GameType == GAMETYPE_TRIAL && GameSettings.Level != LEVEL_FRONTEND)
        SaveTrackTimes();

    // free static level models
    FreeLevelModels();

    g_SoundEngine.StopAll();
}


//////////////////////////
// init loading display //
//////////////////////////

void InitLoadingDisplay(void)
{

// init text states

    D3Ddevice->BeginScene();
    SetBackgroundColor(0x000000);
    D3Ddevice->EndScene();

// clear both buffers

    FlipBuffers();

    D3Ddevice->BeginScene();
    ClearBuffers();
    D3Ddevice->EndScene();

    FlipBuffers();

    D3Ddevice->BeginScene();
    ClearBuffers();
    D3Ddevice->EndScene();

    FlipBuffers();

    D3Ddevice->BeginScene();
    ClearBuffers();
    D3Ddevice->EndScene();

// set loading pos

    LoadingPos = 48;
}

/////////////////////
// loading display //
/////////////////////

void LoadingDisplay(WCHAR *text)
{
    short pos;

// dump text on both buffers

    pos = (640 - (wcslen(text) * 12)) / 2;

    FlipBuffers();

    D3Ddevice->BeginScene();
    InitRenderStates();
    BeginTextState();
    DumpText(pos, LoadingPos, 12, 16, 0xffffff, text);
    D3Ddevice->EndScene();

    FlipBuffers();

    D3Ddevice->BeginScene();
    InitRenderStates();
    BeginTextState();
    DumpText(pos, LoadingPos, 12, 16, 0xffffff, text);
    D3Ddevice->EndScene();

    FlipBuffers();

    D3Ddevice->BeginScene();
    InitRenderStates();
    BeginTextState();
    DumpText(pos, LoadingPos, 12, 16, 0xffffff, text);
    D3Ddevice->EndScene();

// update loading pos

    LoadingPos += 24;
}

/////////////////////////////////////////////////////////////////////
//
// LoadLevelFields: load in the static fields associtaed with the 
// passed level.
//
/////////////////////////////////////////////////////////////////////
#ifdef _PC
bool LoadLevelFields()
{
    int         iField;
    long        nFields;
    REAL        rad;
    BBOX        bBox;
    FILE        *fp;
    FILE_FIELD  fileField;
    LEVELINFO *levelInfo = GetLevelInfo(GameSettings.Level);

    // open the file
    fp = fopen(GetLevelFilename("fld", FILENAME_MAKE_BODY), "rb");
    if (fp == NULL) {
        return FALSE;
    }

    // get the number of fields
    if (fread(&nFields, sizeof(long), 1, fp) < 1) {
        fclose(fp);
        return FALSE;
    }

    if (nFields < 1) {
        fclose(fp);
        return TRUE;
    }

    // Allocate space for the positions and matrices
    LEV_LevelFieldPos = (VEC *)malloc(sizeof(VEC) * nFields);
    LEV_LevelFieldMat = (MAT *)malloc(sizeof(MAT) * nFields);

    // Read in all the fields
    for (iField = 0; iField < nFields; iField++) {

        if (fread(&fileField, sizeof(FILE_FIELD), 1, fp) < 1) {
            fclose(fp);
            return FALSE;
        }

        // Set the position and matrix in the array
        CopyVec(&fileField.Pos, &LEV_LevelFieldPos[iField]); 
        CopyMat(&fileField.Matrix, &LEV_LevelFieldMat[iField]);

        // Create the field
        switch(fileField.Type) {

        case FILE_FIELD_TYPE_LINEAR:
            rad = (REAL)sqrt(fileField.Size[0] * fileField.Size[0] + fileField.Size[1] * fileField.Size[1] + fileField.Size[2] * fileField.Size[2]);
            SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
            AddLinearField(
                FIELD_PARENT_NONE, 
                FIELD_PRIORITY_MAX, 
                &LEV_LevelFieldPos[iField], 
                &LEV_LevelFieldMat[iField], 
                &bBox, 
                (VEC *)&fileField.Size, 
                &fileField.Dir, 
                fileField.Mag, 
                fileField.Damping,
                FIELD_FORCE);
            break;

        case FILE_FIELD_TYPE_ORIENTATION:
            rad = (REAL)sqrt(fileField.Size[0] * fileField.Size[0] + fileField.Size[1] * fileField.Size[1] + fileField.Size[2] * fileField.Size[2]);
            SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
            AddOrientationField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &LEV_LevelFieldPos[iField], &LEV_LevelFieldMat[iField], &bBox, (VEC *)&fileField.Size, &fileField.Dir, fileField.Mag, fileField.Damping);
            break;

        case FILE_FIELD_TYPE_VELOCITY:
            rad = (REAL)sqrt(fileField.Size[0] * fileField.Size[0] + fileField.Size[1] * fileField.Size[1] + fileField.Size[2] * fileField.Size[2]);
            SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
            AddVelocityField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &LEV_LevelFieldPos[iField], &LEV_LevelFieldMat[iField], &bBox, (VEC *)&fileField.Size, &fileField.Dir, fileField.Mag);
            break;

        case FILE_FIELD_TYPE_SPHERICAL:
            rad = fileField.RadEnd;
            SetBBox(&bBox, -rad, rad, -rad, rad, -rad, rad);
            AddSphericalField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &LEV_LevelFieldPos[iField], fileField.RadStart, fileField.RadEnd, fileField.GradStart, fileField.GradEnd);
            break;
        
        case FILE_FIELD_TYPE_WIND:
            SetBBox(&bBox, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST);
            FLD_WindField = AddLinearField(
                FIELD_PARENT_NONE, 
                FIELD_PRIORITY_MIN, 
                &ZeroVector, 
                &Identity, 
                &bBox, 
                &FLD_WindSize, 
                &fileField.Dir,
                fileField.Mag, 
                fileField.Damping, 
                FIELD_FORCE);
            
            GenerateWindFieldData(FLD_WindField);

        default:
            break;
        }
    }

    fclose(fp);
    return TRUE;
}
#endif

