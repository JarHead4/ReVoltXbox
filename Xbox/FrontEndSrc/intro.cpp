//-----------------------------------------------------------------------------
// File: intro.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "intro.h"
#include "geom.h"
#include "InitPlay.h"
#include "main.h"
#include "camera.h"
#include "input.h"
#include "timing.h"
#include "ai.h"
#include "gameloop.h"
#include "spark.h"
#include "TitleScreen.h"
#include "field.h"
#include "move.h"
#include "obj_init.h"
#include "light.h"

#include "DrawObj.h"
#include "visibox.h"
#include "settings.h"
#include "text.h"
#include "model.h"
#include "path.h"

#include "VoiceCommunicator.h"

#define NSTRIPE_SECTIONS    100

/*
////////////////////////////////////////////////////////////////
//
// Static global variables
//
////////////////////////////////////////////////////////////////

t_SmokeScreen       gSmokeScreen;
bool                gDrawSmokeScreen = TRUE;

bool                gQuitIntro = FALSE;
INTRO_STATE_VARS    gIntroStateVars;
INTRO_STATE         gCurrentIntroState;

FORCE_FIELD *gIntroField;

NEW_CAMERA *gIntroCamera = NULL;
REAL gIntroCamSpeedFactor = 1.7f;
CAMERA_POS gIntroCameraPositions[INTRO_CAMPOS_NUM] = 
{
    { {   0, -170,   200}, {0, -170, 1000}, {0, 0, 0, 1}},            // INTRO_CAM_ACLM_START
    { {   0, -170,  -100}, {0,    0,  800}, {0, 0, 0, 1}},           // INTRO_CAM_ACLM_END
    { {-350,  -70, -1000}, {0, -170, 1000}, {0, 0, 0, 1}},        // INTRO_CAM_PROBE_START
    { {   0, -700,  -300}, {0,    0,    0}, {0, 0, 0, 1}}              // INTRO_CAM_PROBE_END
};


VEC gFloorPos = {0, 0, 0};
BBOX gWorldBBox = {-LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST, -LARGEDIST, LARGEDIST};
REAL gWorldRadius = LARGEDIST;

VEC gSmokePos = {0, -150, 650};
REAL gSmokeMass = 1.0f;
REAL gSmokeWidth = 450.0f;

LIGHT *gIntroOmniLight = NULL;
LIGHT *gIntroSpotLight = NULL;

VEC gUFOAcc = {20, 100, 250};
VEC gUFOAngAcc = {250.0f, 0, 150.0f};
VEC gUFOFlipImpulse = {0, -100, 0};
VEC gUFOFlipImpulsePos = {150, 0, -150};
long gUFOHasFlipped = FALSE;

PLAYER *gIntroPlayer[INTRO_NPLAYERS];
VEC gIntroPlayerStartPos[INTRO_NPLAYERS] = {
    {0, -170, 4700},
    {155, -30, -2100},
    {1135, -30, -2000},
};
REAL gIntroPlayerStartRot1 = 0.0f;
REAL gIntroPlayerStartRot2 = 0.4f;

VEC gOffScreenPos = {-10000, 10000, -10000};

VEC gLogoPos = {0, -1000, 0};
OBJECT *gLogoObject = NULL;

ANIMATION_DATA *gUFOAnim;

STRIPE_DATA gStripeData = {0, NULL, NULL};
long gStripeStartFrame = 60;
long gStripeNSections = 100;
REAL gStripeShift = 25.0f;

long gSparkFrameStart = 58;

////////////////////////////////////////////////////////////////
//
// Prototypes
//
////////////////////////////////////////////////////////////////

static void AllocSmokeScreenData(t_SmokeScreen *pSmokeScreen, int w, int h);
static void CreateSmokeScreen(t_SmokeScreen *pSmokeScreen, VEC *pTL, REAL dim, REAL mass);
static void ReleaseSmokeScreen(t_SmokeScreen *pSmokeScreen);
static void ProcessSmokeScreen(t_SmokeScreen *pSmokeScreen, REAL dT);
static void ProcessSmokeParticle(t_SmokeParticle *pParticle, REAL dT);
static REAL ProcessSmokeNeighbour(t_SmokeScreen* pSmokeScreen, REAL dT, t_SmokeParticle *pP, t_SmokeParticle *pPN, int flag);
static void RenderSmokeScreen(t_SmokeScreen *pSmokeScreen);
static void HitSmokeScreen(t_SmokeScreen *pSmokeScreen, VEC *pPos, VEC *pVel, REAL radius);
static void ApplySmokeImpulse(t_SmokeParticle *pParticle, VEC *pImpulse, VEC *pPos);

static void InitIntroCamPosData(CAMERA_POS *camPosData);
static void UpdateIntroSpotLight();

static void BuildStripeFromAnimation(ANIMATION_DATA *animData, STRIPE_DATA *stripeData, long startFrame, long endFrame, long nSections);
static void DrawLogoStripe(STRIPE_DATA *stripeData, long lastFrame);


static void SetupIntroSequenceCars(void);
static void LoadIntroSequenceTextures(void);
static void FreeIntroSequenceTextures(void);
static void LoadIntroSequenceModels(void);
static void FreeIntroSequenceModels(void);


static void IntroSequenceInit();
static void StartUFOFlight(void);
static void IntroSequenceStartUFO(void);
static void IntroSequenceBurstCloud(void);
static void AcclaimToProbeCut(void);
static void InitCarDriveBy(void);
static void DropProbeLogo(void);

static void IntroState_InitialPause(void);
static void IntroState_UFOFly(void);
static void IntroState_ExitUFO(void);
static void IntroState_FinalPause(void);
static void IntroState_WaitingForUFOAnim(void);
static void IntroState_UFOAnim(void);
static void IntroState_DropLogo(void);
static void IntroState_CarDriveBy(void);
static void IntroState_Exit(void);

static void IntroPhysicsLoop(void);


////////////////////////////////////////////////////////////////
//
// State functions
//
////////////////////////////////////////////////////////////////
//  INTRO_ACLM_INITIAL_PAUSE,
//  INTRO_ACLM_INITIAL_UFO_FLIGHT,
//  INTRO_ACLM_EXIT_UFO,
//  INTRO_ACLM_FINAL_PAUSE,
//  INTRO_PROBE_INITIAL_WAIT
//  INTRO_PROBE_INITIAL_UFO_FLIGHT,
//  INTRO_PROBE_DROP_LOGO,
//  INTRO_PROBE_CAR_DRIVE_BY,
//  INTRO_EXIT,

INTRO_STATE_FUNCTION *IntroStateFunction[INTRO_NSTATES] = {
    IntroState_InitialPause,
    IntroState_UFOFly,
    IntroState_ExitUFO,
    IntroState_FinalPause,
    IntroState_WaitingForUFOAnim,
    IntroState_UFOAnim,
    IntroState_DropLogo,
    IntroState_CarDriveBy,
    IntroState_Exit
};


////////////////////////////////////////////////////////////////
//
// SetupIntroCars:
//
////////////////////////////////////////////////////////////////

void SetupIntroSequenceCars()
{
    // Clear start data
    InitStartData();
    StartData.LocalPlayerNum = 0;

    // Add cars
    AddPlayerToStartData(PLAYER_LOCAL, 3, CARID_UFO, FALSE, 0, CTRL_TYPE_NONE, 0, "Probe UFO");
    AddPlayerToStartData(PLAYER_LOCAL, 3, CARID_RC, FALSE, 0, CTRL_TYPE_NONE, 0, "DevilCarONE");
    AddPlayerToStartData(PLAYER_LOCAL, 3, CARID_FLAG, FALSE, 0, CTRL_TYPE_NONE, 0, "DevilCarTWO");

}


////////////////////////////////////////////////////////////////
//
// LoadIntroSequenceTextures
//
////////////////////////////////////////////////////////////////

void LoadIntroSequenceTextures()
{
//$MODIFIED
//    LoadTextureClever("levels\\intro\\acclaimlogo.bmp", TPAGE_MISC1, 256, 256, 0, 0, 1);
    LoadTextureClever("D:\\levels\\intro\\acclaimlogo.bmp", TPAGE_MISC1, 256, 256, 0, 0, 1);
//$END_MODIFICATIONS
}


void FreeIntroSequenceTextures()
{
    FreeOneTexture(TPAGE_MISC1);
}


////////////////////////////////////////////////////////////////
//
// Load the intro sequence models
//
////////////////////////////////////////////////////////////////
#ifdef _PC
extern MODEL AnimPathModel;
#endif

void LoadIntroSequenceModels()
{
    struct renderflags rflag = {0,};

    rflag.envmap = FALSE;
    rflag.envgood = TRUE;

    LoadOneLevelModel(LEVEL_MODEL_PROBELOGO, TRUE, rflag, 0);
#ifdef _PC
    //LoadModel("edit\\ainode2.m", &AnimPathModel, -1, 1, LOADMODEL_FORCE_TPAGE, 100);
#endif
}

void FreeIntroSequenceModels()
{
    FreeLevelModels();
}

/////////////////////////////////////////////////////////////////////////////////
// AllocSmokeScreenData
/////////////////////////////////////////////////////////////////////////////////
void AllocSmokeScreenData(t_SmokeScreen *pSmokeScreen, int w, int h)
{
    pSmokeScreen->pParticles = (t_SmokeParticle*)malloc(sizeof(t_SmokeParticle) * w * h);
    pSmokeScreen->width  = w;
    pSmokeScreen->height = h;
    pSmokeScreen->cParticles = w * h;
//  pSmokeScreen->damping = Real(0.5);
    pSmokeScreen->damping = Real(0.1);
}

/////////////////////////////////////////////////////////////////////////////////
// CreateSmokeSceen
/////////////////////////////////////////////////////////////////////////////////
void CreateSmokeScreen(t_SmokeScreen *pSmokeScreen, VEC *pos, REAL dim, REAL mass)
{
    t_SmokeParticle *pParticle;
    VEC             posY, posXY;
    VEC             deltaR, deltaD;
    int             cW, cH;

    pSmokeScreen->expand = ZERO;

    pSmokeScreen->lengthMin = dim / (pSmokeScreen->width - 1);
    pSmokeScreen->lengthMinD = (REAL)sqrt(MulScalar(2, MulScalar(pSmokeScreen->lengthMin, pSmokeScreen->lengthMin)));

    deltaR.v[0] = pSmokeScreen->lengthMin;
    deltaR.v[1] = Real(0);
    deltaR.v[2] = Real(0);
    deltaD.v[0] = Real(0);
    deltaD.v[1] = pSmokeScreen->lengthMin;
    deltaD.v[2] = Real(0);

    VecPlusScalarVec(pos, -(pSmokeScreen->width / 2), &deltaR, &posY);
    VecPlusEqScalarVec(&posY, -(pSmokeScreen->height / 2), &deltaD);

//  mass = DivScalar(mass, pSmokeScreen->width * pSmokeScreen->height);

    pParticle = pSmokeScreen->pParticles;
    for (cH = 0; cH < pSmokeScreen->height; cH++)
    {
        CopyVec(&posY, &posXY);

        for (cW = 0; cW < pSmokeScreen->width; cW++)
        {
            pParticle->pos.v[0] = posXY.v[0];
            pParticle->pos.v[1] = posXY.v[1];
            pParticle->pos.v[2] = posXY.v[2];

            pParticle->intensity = Real(1);

            SetVecZero(&pParticle->vel);
            SetVecZero(&pParticle->acc);
            SetVecZero(&pParticle->impulse);

            pParticle->mass = mass;
            pParticle->invMass = DivScalar(ONE, mass);
            pParticle->gravity = Real(0.0);
            pParticle->airResistance = Real(0.0);

            VecPlusEqVec(&posXY, &deltaR);
            pParticle++;
        }

        VecPlusEqVec(&posY, &deltaD);
    }
}

/////////////////////////////////////////////////////////////////////////////////
// ReleaseSmokeScreen
/////////////////////////////////////////////////////////////////////////////////
void ReleaseSmokeScreen(t_SmokeScreen *pSmokeScreen)
{
    if (pSmokeScreen->pParticles)
    {
        free(pSmokeScreen->pParticles);
        pSmokeScreen->pParticles = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////////
// ProcessSmokeScreen
/////////////////////////////////////////////////////////////////////////////////
void ProcessSmokeScreen(t_SmokeScreen *pSmokeScreen, REAL dT)
{
    t_SmokeParticle *pParticle;
    t_SmokeParticle *pP;
    int     i, cH,cW;

// No processing needed ?
    if (pSmokeScreen->expand == 0)
        return;

// Update particles
    i = pSmokeScreen->cParticles;
    pParticle = pSmokeScreen->pParticles;
    while (i--)
    {
        ProcessSmokeParticle(pParticle, dT);
        pParticle++;
    }


// Update particle intensities
    pP = pSmokeScreen->pParticles;

    for (cH = 1; cH < pSmokeScreen->height; cH++)
    {
        if (pP->intensity != 0)
        {
            ProcessSmokeNeighbour(pSmokeScreen, dT, pP, pP+1, 0);
            ProcessSmokeNeighbour(pSmokeScreen, dT, pP, pP+pSmokeScreen->width, 0);
            ProcessSmokeNeighbour(pSmokeScreen, dT, pP, pP+pSmokeScreen->width+1, 1);
        }
        pP++;

        for (cW = 2; cW < pSmokeScreen->width; cW++, pP++)
        {
            if (pP->intensity != 0)
            {
                ProcessSmokeNeighbour(pSmokeScreen, dT, pP, pP+1, 0);
                ProcessSmokeNeighbour(pSmokeScreen, dT, pP, pP+pSmokeScreen->width, 0);
                ProcessSmokeNeighbour(pSmokeScreen, dT, pP, pP+pSmokeScreen->width+1, 1);
                ProcessSmokeNeighbour(pSmokeScreen, dT, pP, pP+pSmokeScreen->width-1, 1);
            }
        }

        pP++;
    }

// Expansion
    if (pSmokeScreen->expand != 0)
    {
        pSmokeScreen->lengthMin += MulScalar(dT, pSmokeScreen->expand);
        pSmokeScreen->lengthMinD = (REAL)sqrt(MulScalar(2, MulScalar(pSmokeScreen->lengthMin, pSmokeScreen->lengthMin)));
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
REAL ProcessSmokeNeighbour(t_SmokeScreen* pSmokeScreen, REAL dT, t_SmokeParticle *pP, t_SmokeParticle *pPN, int flag)
{
    VEC     delta, impulse;
    REAL    dist, invDist;
    REAL    l, d;

    VecMinusVec(&pPN->pos, &pP->pos, &delta);

//  dist = VecLen(&delta);
    FastLength(&delta, &dist);

    if (flag == 0)  l = pSmokeScreen->lengthMin;
    else            l = pSmokeScreen->lengthMinD;

    d = MulScalar(pP->intensity, pSmokeScreen->damping);
    invDist = DivScalar(ONE, dist);
    l = MulScalar(l, invDist);
    impulse.v[0] = MulScalar(d, (delta.v[0] - MulScalar(l, delta.v[0])));
    impulse.v[1] = MulScalar(d, (delta.v[1] - MulScalar(l, delta.v[1])));
    impulse.v[2] = MulScalar(d, (delta.v[2] - MulScalar(l, delta.v[2])));
//  impulse.v[0] = d * (delta.v[0] - (l * (delta.v[0] * invDist)));
//  impulse.v[1] = d * (delta.v[1] - (l * (delta.v[1] * invDist)));
//  impulse.v[2] = d * (delta.v[2] - (l * (delta.v[2] * invDist)));

    ApplySmokeImpulse(pP, &impulse, NULL);
    NegateVec(&impulse);
    ApplySmokeImpulse(pPN, &impulse, NULL);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void ProcessSmokeParticle(t_SmokeParticle *pParticle, REAL dT)
{
    VEC         lastVel;
    REAL        s;

    // Store the old position and world matrices
    CopyVec(&pParticle->vel, &lastVel);

    // Update particle velocity from the acceleration (a = F / mass = F * 1/mass)
    VecPlusEqScalarVec(&pParticle->vel, pParticle->invMass, &pParticle->impulse);

    // Damp the velocity from the air resistance
//  s = MulScalar(pParticle->airResistance, MulScalar(FRICTION_TIME_SCALE, dT));
    s = MulScalar(pParticle->airResistance, dT);
    VecMulScalar(&pParticle->vel, ONE - s);

    // Update particle position from the velocity
    VecPlusEqScalarVec(&pParticle->pos, dT, &pParticle->vel);

    // Reset the impulse for the next timestep
    SetVecZero(&pParticle->impulse);

    // Recalculate acceleration from change in velocity
    VecMinusVec(&pParticle->vel, &lastVel, &pParticle->acc);

    // Update intensity
    pParticle->intensity -= dT * ((abs(pParticle->vel.v[0]) + abs(pParticle->vel.v[1]) + abs(pParticle->vel.v[2])) * Real(0.015));
    pParticle->intensity -= dT * frand(HALF);
    if (pParticle->intensity < 0)
        pParticle->intensity = 0;
}

/////////////////////////////////////////////////////////////////////////////////
// RenderSmokeScreen
/////////////////////////////////////////////////////////////////////////////////
void RenderSmokeScreen(t_SmokeScreen *pSmokeScreen)
{
#ifdef _PC

    t_SmokeParticle *pParticle;
    int             cW, cH;
    static QUATERNION       quat = {0,0,0,1};
    MAT             eyematrix;
    VEC             eyetrans;
    REAL            u, v, dU, dV;
    static REAL rX = 0;
    VERTEX_TEX1     vert[4];

    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
//      BLEND_OFF();
    FOG_OFF();
    ZBUFFER_ON();
    SET_TPAGE(TPAGE_MISC1);

    CopyMat(&ViewMatrixScaled, &eyematrix);
    CopyVec(&ViewTransScaled, &eyetrans);

    dU = 1.0f / (float)pSmokeScreen->width;
    dV = 1.0f / (float)pSmokeScreen->height;
    v = 0.0f;
    pParticle = pSmokeScreen->pParticles;
    for (cH = 1; cH < pSmokeScreen->height; cH++)
    {
        u = 0.0f;

        for (cW = 1; cW < pSmokeScreen->width; cW++)
        {
            if ((pParticle[0].intensity != 0) || (pParticle[1].intensity != 0) || (pParticle[pSmokeScreen->width+1].intensity != 0) || (pParticle[pSmokeScreen->width+0].intensity != 0)) {

                RotTransPersVector(&eyematrix, &eyetrans, &pParticle[0].pos, (float*)&vert[0].sx);
                RotTransPersVector(&eyematrix, &eyetrans, &pParticle[1].pos, (float*)&vert[1].sx);
                RotTransPersVector(&eyematrix, &eyetrans, &pParticle[pSmokeScreen->width+1].pos, (float*)&vert[2].sx);
                RotTransPersVector(&eyematrix, &eyetrans, &pParticle[pSmokeScreen->width].pos, (float*)&vert[3].sx);

                vert[0].tu = vert[3].tu = u;
                vert[1].tu = vert[2].tu = u + dU;
                vert[0].tv = vert[1].tv = v;
                vert[2].tv = vert[3].tv = v + dV;

                vert[0].color = 0xFFFFFF | (Int(MulScalar(pParticle[0].intensity, 255)) << 24);
                vert[1].color = 0xFFFFFF | (Int(MulScalar(pParticle[1].intensity, 255)) << 24);
                vert[2].color = 0xFFFFFF | (Int(MulScalar(pParticle[pSmokeScreen->width+1].intensity, 255)) << 24);
                vert[3].color = 0xFFFFFF | (Int(MulScalar(pParticle[pSmokeScreen->width+0].intensity, 255)) << 24);

                DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
            }

            pParticle++;
            u += dU;
        }

        pParticle++;
        v += dV;
    }

// Reset render state
    BLEND_OFF();

#endif
}

/////////////////////////////////////////////////////////////////////////////////
// HitSmokeScreen
/////////////////////////////////////////////////////////////////////////////////
void HitSmokeScreen(t_SmokeScreen *pSmokeScreen, VEC *pPos, VEC *pVel, REAL radius)
{
    t_SmokeParticle *pParticle;
    VEC     dir, pos, delta;
    int     i;
    REAL    radius2, dist;
    PLANE   plane;


    // Init. expansion
    pSmokeScreen->expand = MulScalar(pSmokeScreen->lengthMin, Real(0.1));


    // Calc. plane eq. of sphere from it's position & velocity
    CopyVec(pVel, &dir);
    dist = VecLen(&dir);
    if (dist)
        VecMulScalar(&dir, DivScalar(ONE, dist));

    BuildPlane2(&dir, pPos, &plane);


    // Radius squared
    radius2 = MulScalar(radius, radius);

    i = pSmokeScreen->width * pSmokeScreen->height;
    pParticle = pSmokeScreen->pParticles;
    while (i--)
    {
        VecMinusVec(&pParticle->pos, pPos, &delta);
        dist = VecLen2(&delta);
        if (dist < radius2)
        {
            VecMulScalar(&delta, Real(0.01));
//          VecMulScalar(&delta, Real(0.1));

            dist = (REAL)sqrt(dist);
            if (dist > (radius * HALF))
            {
                VecMulScalar(&delta, ONE - ((dist - ((radius * HALF))) / (radius * HALF)));
//              VecMulScalar(&delta, 16);
            }

            VecCrossVec(&delta, pVel, &dir);
            SetVec(&pos, Real(0), Real(0), Real(0));
            ApplySmokeImpulse(pParticle, &dir, &pos);
        }

        pParticle++;
    }
}


/////////////////////////////////////////////////////////////////////////////////
// Apply impulse to particle
/////////////////////////////////////////////////////////////////////////////////
void ApplySmokeImpulse(t_SmokeParticle *pParticle, VEC *pImpulse, VEC *pPos)
{
    VecPlusEqVec(&pParticle->impulse, pImpulse);
}


////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////
//
// State Functions
//
////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////

//  INTRO_INITIAL_PAUSE,
//  INTRO_INITIAL_UFO_FLIGHT,
//  INTRO_BURST_CLOUD,
//  INTRO_EXIT_UFO,
//  INTRO_EXIT,

#define INTRO_WAIT_TIME         TO_TIME(Real(0.0))
#define INTRO_WAIT_TIME2        TO_TIME(Real(3.0))
#define INTRO_WAIT_TIME3        TO_TIME(Real(0.8))

////////////////////////////////////////////////////////////////
//
// Initialise the intro sequence
//
////////////////////////////////////////////////////////////////

void IntroSequenceInit()
{
        gCurrentIntroState = INTRO_ACLM_INITIAL_PAUSE;

        // Reset the smoke screen
        CreateSmokeScreen(&gSmokeScreen, &gSmokePos, gSmokeWidth, gSmokeMass);
        gDrawSmokeScreen = TRUE;

        // Get rid of gravity and any fields
        if (gIntroField != NULL) {
            RemoveField(gIntroField);
            gIntroField = NULL;
        }
        if (FLD_GravityField != NULL) {
            RemoveField(FLD_GravityField);
            FLD_GravityField = NULL;
        }

        // Get rid of the logo
        if (gLogoObject != NULL) {
            OBJ_FreeObject(gLogoObject);
            gLogoObject = NULL;
        }

        // Place cars
        SetCarPos(&gIntroPlayer[INTRO_PLAYER_UFO]->car, &gIntroPlayerStartPos[INTRO_PLAYER_UFO], &Identity);
        SetCarPos(&gIntroPlayer[INTRO_PLAYER_CAR1]->car, &gIntroPlayerStartPos[INTRO_PLAYER_CAR1], &Identity);
        SetCarPos(&gIntroPlayer[INTRO_PLAYER_CAR2]->car, &gIntroPlayerStartPos[INTRO_PLAYER_CAR2], &Identity);

        CON_InitControls(&gIntroPlayer[INTRO_PLAYER_UFO]->controls);
        CON_InitControls(&gIntroPlayer[INTRO_PLAYER_CAR1]->controls);
        CON_InitControls(&gIntroPlayer[INTRO_PLAYER_CAR2]->controls);

        PLR_SetPlayerType(gIntroPlayer[0], PLAYER_LOCAL);
        gUFOHasFlipped = FALSE;

        // Reset animation state
        gIntroPlayer[INTRO_PLAYER_UFO]->ownobj->AnimState.CurrentPosKey = 0;
        gIntroPlayer[INTRO_PLAYER_UFO]->ownobj->AnimState.CurrentQuatKey = 0;

        // Init Camera
        gIntroCamera->SetCameraPos( &gIntroCameraPositions[INTRO_CAM_ACLM_START].vEye);
        gIntroCamera->SetCameraQuat( &gIntroCameraPositions[INTRO_CAM_ACLM_START].Quat);
        gIntroCamera->SetCameraDestPos( &gIntroCameraPositions[INTRO_CAM_ACLM_START].vEye);
        gIntroCamera->SetCameraDestQuat( &gIntroCameraPositions[INTRO_CAM_ACLM_START].Quat);
        gIntroCamSpeedFactor = 2.0f;

        // Switch off the spot light
        if (gIntroSpotLight != NULL) {
            FreeLight(gIntroSpotLight);
            gIntroSpotLight = NULL;
        }
}

////////////////////////////////////////////////////////////////
//
// Initial wait - for a couple of seconds
//
////////////////////////////////////////////////////////////////
void IntroState_InitialPause(void)
{
    static REAL waitTime = 0;

    if (waitTime > INTRO_WAIT_TIME) {
        waitTime = 0;

        // Set the UFO flying
        StartUFOFlight();


        gCurrentIntroState = INTRO_ACLM_INITIAL_UFO_FLIGHT;

        return;
    }

    waitTime += TimeStep;

}

////////////////////////////////////////////////////////////////
//
// Start first UFO flight
//
////////////////////////////////////////////////////////////////

void StartUFOFlight(void)
{
    SetVec(&gIntroPlayer[INTRO_PLAYER_UFO]->car.Body->Centre.Vel, 0, 0, -1500);
}

////////////////////////////////////////////////////////////////
//
// Fly UFO Through the cloud
//
////////////////////////////////////////////////////////////////

void IntroState_UFOFly(void)
{
    REAL accMag;
    VEC accDir;
    static REAL waitTime = 0;

    if (waitTime > INTRO_WAIT_TIME2) {
        waitTime = 0;
        // Set the cloud rolling
        IntroSequenceBurstCloud();
        gCurrentIntroState = INTRO_ACLM_EXIT_UFO;

        // Add a field to move the UFO
        accMag = VecLen(&gUFOAcc);
        VecEqScalarVec(&accDir, ONE / accMag, &gUFOAcc);
        gIntroField = AddLinearTwistField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &ZeroVector, &Identity, &FLD_GlobalBBox, &FLD_GlobalSize, &accDir, accMag, &gUFOAngAcc, 0.2f);

        // Set new camera destination
        gIntroCamera->SetCameraDestPos( &gIntroCameraPositions[INTRO_CAM_ACLM_END].vEye);
        gIntroCamera->SetCameraDestQuat( &gIntroCameraPositions[INTRO_CAM_ACLM_END].Quat);
        
        return;
    }

    waitTime += TimeStep;
}

////////////////////////////////////////////////////////////////
//
// Burst the cloud
//
////////////////////////////////////////////////////////////////
void IntroSequenceBurstCloud(void)
{
    VEC pos, vel;

    CopyVec(&gSmokePos, &pos);
    pos.v[Y] -= 20;
    SetVec(&vel, Real(20), Real(20), Real(-1200));
    HitSmokeScreen(&gSmokeScreen, &pos, &vel, 50);

}

////////////////////////////////////////////////////////////////
//
// UFO leaves the screen
//
////////////////////////////////////////////////////////////////

#define INTROCAM_ACC 200
#define INTROCAM_ANGACC 0.04f

void IntroState_ExitUFO(void)
{
    VEC pos, vel;
    static REAL waitTime = 0;

    // See if its time to exit
    if (waitTime > INTRO_WAIT_TIME3) {
        waitTime = 0;
        gCurrentIntroState = INTRO_ACLM_FINAL_PAUSE;

        // Get rid of the field and add gravity
        if (gIntroField != NULL) {
            RemoveField(gIntroField);
            gIntroField = NULL;
        }
        gIntroField = AddLinearField(FIELD_PARENT_NONE, FIELD_PRIORITY_MAX, &ZeroVector, &Identity, &FLD_GlobalBBox, &FLD_GlobalSize, &DownVec, FLD_Gravity, ZERO, FIELD_ACC);

        return;
    }
    waitTime += TimeStep;

    // Keep messing with the cloud
    SetVec(&pos, gSmokePos.v[X] - 150, gSmokePos.v[Y] - 20, gSmokePos.v[Z]);
    SetVec(&vel, Real(0), Real(0), Real(-10));
    HitSmokeScreen(&gSmokeScreen, &pos, &vel, 30);

    SetVec(&pos, gSmokePos.v[X] - 70, gSmokePos.v[Y] + 20, gSmokePos.v[Z]);
    SetVec(&vel, Real(0), Real(0), Real(-50));
    HitSmokeScreen(&gSmokeScreen, &pos, &vel, 20);

    SetVec(&pos, gSmokePos.v[X] + 100, gSmokePos.v[Y] -50, gSmokePos.v[Z]);
    SetVec(&vel, Real(0), Real(0), Real(-100));
    HitSmokeScreen(&gSmokeScreen, &pos, &vel, 20);

}

////////////////////////////////////////////////////////////////
//
// Final Pause to see logo disintigrate
//
////////////////////////////////////////////////////////////////
#define INTRO_FINAL_PAUSE_TIME      TO_TIME(Real(1.0))

void IntroState_FinalPause(void)
{
    static REAL waitTime = 0;

    // See if its time to exit
    if (waitTime > INTRO_FINAL_PAUSE_TIME) {
        waitTime = 0;
        gCurrentIntroState = INTRO_PROBE_INITIAL_WAIT;

        // Cut to probe intro
        AcclaimToProbeCut();

        return;
    }
    waitTime += TimeStep;
}

////////////////////////////////////////////////////////////////
//
// Set the UFO animating and cut camera
//
////////////////////////////////////////////////////////////////

void AcclaimToProbeCut(void) 
{
    // Switch off the smoke screen
    gDrawSmokeScreen = FALSE;

    // Move UFO off screen 
    SetCarPos(&gIntroPlayer[INTRO_PLAYER_UFO]->car, &gOffScreenPos, &Identity);

    // Set new camera pos
    gIntroCamera->SetCameraPos( &gIntroCameraPositions[INTRO_CAM_PROBE_START].vEye);
    gIntroCamera->SetCameraQuat( &gIntroCameraPositions[INTRO_CAM_PROBE_START].Quat);
    gIntroCamera->SetCameraDestPos( &gIntroCameraPositions[INTRO_CAM_PROBE_END].vEye);
    gIntroCamera->SetCameraDestQuat( &gIntroCameraPositions[INTRO_CAM_PROBE_END].Quat);
    gIntroCamSpeedFactor = 0.0f;

}

////////////////////////////////////////////////////////////////
//
// Wait before setting Probe UFO off in animation
//
////////////////////////////////////////////////////////////////
#define INTRO_PROBE_INITIAL_WAIT_TIME 0.0f

void IntroState_WaitingForUFOAnim(void)
{
    static REAL waitTime = 0;

    // See if its time to exit
    if (waitTime > INTRO_PROBE_INITIAL_WAIT_TIME) {
        waitTime = 0;
        gCurrentIntroState = INTRO_PROBE_INITIAL_UFO_FLIGHT;

        // Set UFO anim
        SetObjectAnimation(gIntroPlayer[INTRO_PLAYER_UFO]->ownobj, gUFOAnim);
        SetCarPos(&gIntroPlayer[INTRO_PLAYER_UFO]->car, &gUFOAnim->pPosKeyHead[0].vPos, &Identity);

        // Add a spolt light
#if 0
        gIntroSpotLight = AllocLight();
        gIntroSpotLight->x = 0;
        gIntroSpotLight->y = -200;
        gIntroSpotLight->z = 100;
        gIntroSpotLight->Reach = 2000;
        gIntroSpotLight->Flag = LIGHT_MOVING | LIGHT_FIXED;
        gIntroSpotLight->Type= LIGHT_SPOTNORMAL;
        gIntroSpotLight->r = 255;
        gIntroSpotLight->g = 155;
        gIntroSpotLight->b = 155;
        SetVec(&gIntroSpotLight->DirMatrix.mv[L], 0, 1, 0);
        gIntroSpotLight->Cone = 70;
#endif

        return;
    }
    waitTime += TimeStep;
}

////////////////////////////////////////////////////////////////
//
// UpdateIntroSpotLight:
//
////////////////////////////////////////////////////////////////

void UpdateIntroSpotLight()
{

    gIntroSpotLight->DirMatrix.m[LX] = gIntroPlayer[INTRO_PLAYER_UFO]->car.Body->Centre.Pos.v[X] - gIntroSpotLight->x;
    gIntroSpotLight->DirMatrix.m[LY] = gIntroPlayer[INTRO_PLAYER_UFO]->car.Body->Centre.Pos.v[Y] - gIntroSpotLight->y;
    gIntroSpotLight->DirMatrix.m[LZ] = gIntroPlayer[INTRO_PLAYER_UFO]->car.Body->Centre.Pos.v[Z] - gIntroSpotLight->z;

    NormalizeVec(&gIntroSpotLight->DirMatrix.mv[L]);

}



////////////////////////////////////////////////////////////////
//
// IntroState_UFOAnim:
//
////////////////////////////////////////////////////////////////
#define INTRO_WAIT_TIME_PROBE_UFO_FLIGHT        TO_TIME(Real(2.1))

void IntroState_UFOAnim(void)
{
    static REAL waitTime = 0;

    // See if its time to exit
    if (waitTime > INTRO_WAIT_TIME_PROBE_UFO_FLIGHT) {
        waitTime = 0;

        // Set the Probe logo off
        DropProbeLogo();
        gCurrentIntroState = INTRO_PROBE_DROP_LOGO;

        // Set new camera destination
        //gIntroCamera->SetCameraDestPos( &gIntroCameraPositions[INTRO_CAM_PROBE_END].eye);
        //gIntroCamera->SetCameraDestQuat( &gIntroCameraPositions[INTRO_CAM_PROBE_END].quat);

        return;
    }
    waitTime += TimeStep;

    gIntroCamSpeedFactor += 0.2f * TimeStep;
    //if (gIntroCamSpeedFactor > 2.5f) gIntroCamSpeedFactor = 2.5f;

}

////////////////////////////////////////////////////////////////
//
// Drop the logo
//
////////////////////////////////////////////////////////////////
void DropProbeLogo(void)
{
    MAT rotZ;

    RotationZ(&rotZ, 0.1f);

    gLogoObject = CreateObject(&gLogoPos, &rotZ, OBJECT_TYPE_PROBELOGO, 0);

}

////////////////////////////////////////////////////////////////
//
// Drop the logo
//
////////////////////////////////////////////////////////////////
#define DROP_LOGO_WAIT_TIME     TO_TIME(Real(1.5))

void IntroState_DropLogo(void)
{
    VEC vel;
    static REAL waitTime = ZERO;

    // See if its time to exit
    if (waitTime > DROP_LOGO_WAIT_TIME) {
        waitTime = 0;
        gCurrentIntroState = INTRO_PROBE_CAR_DRIVE_BY;

        // Make Probe Logo have infinite mass and be unmoveable
        if (gLogoObject != NULL) {
            gLogoObject->collhandler = NULL;
            gLogoObject->movehandler = NULL;
            gLogoObject->body.Centre.InvMass = ZERO;
            SetMat(&gLogoObject->body.BodyInvInertia, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO, ZERO);
        }

        // Init car drive-by
        InitCarDriveBy();

        return;
    }
    waitTime += TimeStep;

    gIntroCamSpeedFactor += 3.5f * TimeStep;

    gIntroCamera->pCamera->Shake -= TimeStep * 4.0f;
    if (gIntroCamera->pCamera->Shake < 0.0f) CAM_MainCamera->Shake = 0.0f;


    // See if the Logo has hit the floor yet (if not: move along now, nothing to see here)
    if (gLogoObject->body.BangMag == 0) return;

    // Logo hits floor, so shake camera and knock UFO over
    if (!gUFOHasFlipped) {

        // Switch off UFO Anim
        PLR_SetPlayerType(gIntroPlayer[INTRO_PLAYER_UFO], PLAYER_LOCAL);
        CON_InitControls(&gIntroPlayer[INTRO_PLAYER_UFO]->controls);

        // Make UFO jumpo off floor
        VecEqScalarVec(&gUFOFlipImpulsePos, -100, &gIntroPlayer[INTRO_PLAYER_UFO]->car.Body->Centre.WMatrix.mv[L]);
        ApplyBodyImpulse(gIntroPlayer[INTRO_PLAYER_UFO]->car.Body, &gUFOFlipImpulse, &gUFOFlipImpulsePos);

        // Shake camera
        gIntroCamera->pCamera->Shake = 0.5f;

        // Make some smoke...Honka Honka
        SetVec(&vel, 0, -5.0f, 0);
        CreateSpark(SPARK_PROBE_SMOKE, &ZeroVector, &vel, 0, 0);


        gUFOHasFlipped = TRUE;
    }

}

////////////////////////////////////////////////////////////////
//
// Start Drive by
//
////////////////////////////////////////////////////////////////

void InitCarDriveBy(void)
{

    MAT rotY;

    // Set the positions of the drive-by cars
    RotationY(&rotY, gIntroPlayerStartRot1);
    SetCarPos(&gIntroPlayer[INTRO_PLAYER_CAR1]->car, &gIntroPlayerStartPos[INTRO_PLAYER_CAR1], &rotY);
    RotationY(&rotY, gIntroPlayerStartRot2);
    SetCarPos(&gIntroPlayer[INTRO_PLAYER_CAR2]->car, &gIntroPlayerStartPos[INTRO_PLAYER_CAR2], &rotY);

    // Accelerate!
    gIntroPlayer[INTRO_PLAYER_CAR1]->controls.dy = -100;//-CTRL_RANGE_MAX;
    gIntroPlayer[INTRO_PLAYER_CAR2]->controls.dy = -100;//-CTRL_RANGE_MAX;

    // Put UFO back to proper physics car
    PLR_SetPlayerType(gIntroPlayer[INTRO_PLAYER_UFO], PLAYER_LOCAL);
    CON_InitControls(&gIntroPlayer[INTRO_PLAYER_UFO]->controls);

}

////////////////////////////////////////////////////////////////
//
// Car Drive-By
//
////////////////////////////////////////////////////////////////
#define PROBE_LOGO_WAIT_TIME    3.2f

void IntroState_CarDriveBy(void)
{
    static REAL waitTime = ZERO;

    // Accelerate!
    gIntroPlayer[INTRO_PLAYER_CAR1]->controls.dy = -100;//-CTRL_RANGE_MAX;
    gIntroPlayer[INTRO_PLAYER_CAR2]->controls.dy = -100;//-CTRL_RANGE_MAX;

    // See if its time to exit
    if (waitTime > PROBE_LOGO_WAIT_TIME) {
        waitTime = 0;
        gCurrentIntroState = INTRO_EXIT;

        return;
    }
    waitTime += TimeStep;

}

////////////////////////////////////////////////////////////////
//
// Leave the Intro
//
////////////////////////////////////////////////////////////////
void IntroState_Exit(void)
{
    if (Version == VERSION_DEV) {
        if (Keys[DIK_T]) {
            IntroSequenceInit();
        }

        if (Keys[DIK_Y]) {
            if (gUFOAnim != NULL) {
                SetObjectAnimation(gIntroPlayer[0]->ownobj, gUFOAnim);
                SetCarPos(&gIntroPlayer[0]->car, &gUFOAnim->pPosKeyHead[0].vPos, &Identity);
            }
        }
    } else {
        gQuitIntro = TRUE;
    }
}


////////////////////////////////////////////////////////////////
//
// Intro Physics Loop
//
////////////////////////////////////////////////////////////////

void IntroPhysicsLoop()
{
    unsigned long iStep;
    unsigned long totalTime;
    REAL oldTimeStep = TimeStep;

    // Calculate the time step for this set of physics loops
    totalTime = TimerDiff + OverlapTime;
    NPhysicsLoops = totalTime / MS2TIME(PHYSICSTIMESTEP);
    TimeStep = PHYSICSTIMESTEP / 1000.0f;
    OverlapTime = totalTime - (NPhysicsLoops * MS2TIME(PHYSICSTIMESTEP)); 
    if (NPhysicsLoops > 10) {
        TotalRacePhysicsTime += (NPhysicsLoops - 10) * PHYSICSTIMESTEP;
        NPhysicsLoops = 10;
    }

    // do the physics loops
    for (iStep = 0; iStep < NPhysicsLoops; iStep++) {

        // Get control inputs
        CON_DoPlayerControl();

        // Store replay data
        if (CountdownTime == 0) {
            RPL_StoreAllObjectReplayData();
        }

        //GRD_UpdateObjGrid();

        // Deal with the collisions of the objects
        COL_DoObjectCollisions();

        // Move game objects
        MOV_MoveObjects();

        // Calculate time for ghost, replay and remote interpolation
        TotalRacePhysicsTime += PHYSICSTIMESTEP;

        // Create spark trail behind UFO
        if (gUFOAnim != NULL) {
            long lastFrame = 0;
            if (gIntroPlayer[INTRO_PLAYER_UFO]->ownobj->AnimState.CurrentPosKey < gUFOAnim->cPosKeys) {
                lastFrame = gUFOAnim->pPosKeyHead[gIntroPlayer[INTRO_PLAYER_UFO]->ownobj->AnimState.CurrentPosKey].iFrame;
                lastFrame = (lastFrame * gStripeNSections) / (gUFOAnim->cFrames);
            }
            if (lastFrame > gSparkFrameStart) {
                VEC pos;
                CopyVec(&gIntroPlayer[INTRO_PLAYER_UFO]->car.Body->Centre.Pos, &pos);
                pos.v[Y] += (gStripeShift * (gUFOAnim->cFrames - lastFrame)) / (gUFOAnim->cFrames - gSparkFrameStart);
                CreateSpark(SPARK_STAR, &pos, &ZeroVector, 0, 0);
            }
        }

    }

    // clean up
    TimeStep = oldTimeStep;

}


////////////////////////////////////////////////////////////////
//
// BuildStripeFromAnimation:
//
////////////////////////////////////////////////////////////////

void BuildStripeFromAnimation(ANIMATION_DATA *animData, STRIPE_DATA *stripeData, long startFrame, long endFrame, long nSections)
{
    REAL hue, sat, val, r, g, b;
    long frame, section;
    REAL lastTime, curTime, nextTime, interpTime;
    REAL lastShift, nextShift;
    VEC lastPos, curPos, nextPos;

    long currentPosKey = 0;

    stripeData->NSections = nSections;
    stripeData->Pos = (VEC *)malloc(sizeof(VEC) * nSections);
    stripeData->RGB = (long *)malloc(sizeof(long) * nSections);

    for (section = 0; section < nSections; section++) {

        frame = startFrame + (section * (endFrame - startFrame)) / nSections;

        // Get current position key
        while ((currentPosKey < animData->cPosKeys) && (frame > animData->pPosKeyHead[currentPosKey].iFrame)) 
        {
            lastShift = (gStripeShift * (endFrame - animData->pPosKeyHead[currentPosKey].iFrame)) / (endFrame - startFrame);

            currentPosKey++;

            if (currentPosKey < animData->cPosKeys - 1) {
                nextShift = (gStripeShift * (endFrame - animData->pPosKeyHead[currentPosKey + 1].iFrame)) / (endFrame - startFrame);
            } else {
                nextShift = (gStripeShift * (endFrame - endFrame)) / (endFrame - startFrame);
            }

        }

        // Get the positions to interpolate between
        if (currentPosKey == 0) {

            CopyVec(&animData->pPosKeyHead[0].vPos, &lastPos);
            lastTime = TO_TIME(Real(0.03333)) * (animData->pPosKeyHead[0].iFrame - startFrame);

            CopyVec(&animData->pPosKeyHead[1].vPos, &curPos);
            curPos.v[Y] += (gStripeShift * (endFrame - animData->pPosKeyHead[1].iFrame)) / (endFrame - startFrame);
            curTime = TO_TIME(Real(0.03333)) * (animData->pPosKeyHead[1].iFrame - startFrame);

            CopyVec(&animData->pPosKeyHead[2].vPos, &nextPos);
            //nextPos.v[Y] += ((animData->cPosKeys - currentPosKey - 1) * gStripeShift) / animData->cPosKeys;
            nextPos.v[Y] += (gStripeShift * (endFrame - animData->pPosKeyHead[2].iFrame)) / (endFrame - startFrame);
            nextTime = TO_TIME(Real(0.03333)) * (animData->pPosKeyHead[2].iFrame - startFrame);

        } else if (currentPosKey == animData->cPosKeys) {

            CopyVec(&animData->pPosKeyHead[animData->cPosKeys - 1].vPos, &lastPos);
            lastTime = ZERO;
            
            CopyVec(&animData->pPosKeyHead[animData->cPosKeys - 1].vPos, &curPos);
            curTime = HALF;
            
            CopyVec(&animData->pPosKeyHead[animData->cPosKeys - 1].vPos, &nextPos);
            nextTime = ONE;

        } else {

            CopyVec(&animData->pPosKeyHead[currentPosKey - 1].vPos, &lastPos);
            lastPos.v[Y] += lastShift;
            //lastPos.v[Y] += ((animData->cPosKeys - currentPosKey + 1) * gStripeShift) / animData->cPosKeys;
            lastTime = TO_TIME(Real(0.03333)) * (animData->pPosKeyHead[currentPosKey - 1].iFrame - startFrame);;

            CopyVec(&animData->pPosKeyHead[currentPosKey].vPos, &curPos);
            curPos.v[Y] += (gStripeShift * (endFrame - frame)) / (endFrame - startFrame);
            //curPos.v[Y] += ((animData->cPosKeys - currentPosKey) * gStripeShift) / animData->cPosKeys;
            curTime = TO_TIME(Real(0.03333)) * (animData->pPosKeyHead[currentPosKey].iFrame - startFrame);;

            CopyVec(&animData->pPosKeyHead[currentPosKey + 1].vPos, &nextPos);
            nextPos.v[Y] += nextShift;
            //nextPos.v[Y] += ((animData->cPosKeys - currentPosKey - 1) * gStripeShift) / animData->cPosKeys;
            nextTime = TO_TIME(Real(0.03333)) * (animData->pPosKeyHead[currentPosKey + 1].iFrame - startFrame);;

        }



        interpTime = TO_TIME(Real(0.03333)) * (Real((endFrame - startFrame) * section) / nSections);

        // Calculate the new position
        QuadInterpVec(&lastPos, lastTime, &curPos, curTime, &nextPos, nextTime, interpTime, &stripeData->Pos[section]);

        // Calculate colour
        hue = 360.0f - ((410.0f * section) / (nSections));
        if (hue < 0.0f) hue = 0.0f;
        sat = 1.0f;
        val = 1.0f;

        HSVtoRGB(hue, sat, val, &r, &g, &b);
        stripeData->RGB[section] = (long)(r * 255) << 16 | (long)(g * 255) << 8 | (long)(b * 255);

    }


}


////////////////////////////////////////////////////////////////
//
// Draw the multi coloured trail
//
////////////////////////////////////////////////////////////////

void DrawLogoStripe(STRIPE_DATA *stripeData, long lastFrame)
{
    int ii;
    REAL dx, dy, dt, dLen, width, stripeWidth;
    DRAW_SEMI_POLY poly;
    VERTEX_TEX1 sPos, ePos;
    VERTEX_TEX1 *vert = poly.Verts;

    poly.Fog = FALSE;
    poly.VertNum = 4;
    poly.Tpage = TPAGE_WORLD_START + 1;
//$REMOVED    poly.DrawFlag = D3DDP_DONOTUPDATEEXTENTS;
    poly.SemiType = TRUE;

    SET_TPAGE((short)poly.Tpage);
    vert[3].tu = 2.0f / 256.0f;
    vert[3].tv = 2.0f / 256.0f;

    vert[0].tu = 32.0f / 256.0f;
    vert[0].tv = 2.0f / 256.0f;

    vert[1].tu = 32.0f / 256.0f;
    vert[1].tv = 31.0f / 256.0f;

    vert[2].tu = 2.0f / 256.0f;
    vert[2].tv = 32.0f / 256.0f;

    FOG_OFF();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    BLEND_ALPHA();


    stripeWidth = 1.0f;
    if (lastFrame > stripeData->NSections) lastFrame = stripeData->NSections;

    // Calculate first section end coordinates
    RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &stripeData->Pos[0], (REAL*)&ePos);
    RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &stripeData->Pos[1], (REAL*)&sPos);
    dx = ePos.sx - sPos.sx;
    dy = ePos.sy - sPos.sy;
    dLen = (REAL)sqrt(dx * dx + dy * dy);
    dt = dx;
    dx = dy / dLen;
    dy = -dt / dLen;

    // Set up first end vertices
    width = stripeWidth;
    vert[2].sx = ePos.sx + dx * width * ePos.rhw * RenderSettings.GeomPers;
    vert[2].sy = ePos.sy + dy * width * ePos.rhw * RenderSettings.GeomPers;
    vert[2].sz = ePos.sz;
    vert[2].rhw = ePos.rhw;
    vert[3].sx = ePos.sx - dx * width * ePos.rhw * RenderSettings.GeomPers;
    vert[3].sy = ePos.sy - dy * width * ePos.rhw * RenderSettings.GeomPers;
    vert[3].sz = ePos.sz;
    vert[3].rhw = ePos.rhw;

    for (ii = 1; ii < lastFrame; ii++) {

        // Get coordinates of next section
        RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &stripeData->Pos[ii - 1], (REAL*)&sPos);
        dx = ePos.sx - sPos.sx;
        dy = ePos.sy - sPos.sy;
        dLen = (REAL)sqrt(dx * dx + dy * dy);
        if (dLen < SMALL_REAL) {
            continue;
        }
        dt = dx;
        dx = dy / dLen;
        dy = -dt / dLen;

        // Set up next end vertices
        width = stripeWidth + (7 * stripeWidth * ii) / (stripeData->NSections);

        vert[0].sx = sPos.sx - dx * width * sPos.rhw * RenderSettings.GeomPers;
        vert[0].sy = sPos.sy - dy * width * sPos.rhw * RenderSettings.GeomPers;
        vert[0].sz = sPos.sz;
        vert[0].rhw = sPos.rhw;
        vert[1].sx = sPos.sx + dx * width * sPos.rhw * RenderSettings.GeomPers;
        vert[1].sy = sPos.sy + dy * width * sPos.rhw * RenderSettings.GeomPers;
        vert[1].sz = sPos.sz;
        vert[1].rhw = sPos.rhw;

        // Choose a colour
        vert[0].color = vert[1].color = 0xC0 << 24 | stripeData->RGB[ii];
        vert[2].color = vert[3].color = 0xC0 << 24 | stripeData->RGB[ii - 1];

        // draw the poly
        DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, poly.Verts, poly.VertNum, poly.DrawFlag);

        // copy last to next
        ePos.sx = sPos.sx;
        ePos.sy = sPos.sy;
        ePos.sz = sPos.sz;
        ePos.rhw = sPos.rhw;

        vert[2].sx = vert[1].sx;
        vert[2].sy = vert[1].sy;
        vert[2].sz = vert[1].sz;
        vert[2].rhw = vert[1].rhw;
        vert[3].sx = vert[0].sx;
        vert[3].sy = vert[0].sy;
        vert[3].sz = vert[0].sz;
        vert[3].rhw = vert[0].rhw;

    }

    BLEND_OFF();

}

/////////////////////////////////////////////////////////////////////
//
// Init the intro camera position structure
//
/////////////////////////////////////////////////////////////////////

void InitIntroCamPosData( CAMERA_POS* camPosData )
{
    for( int i = 0; i < INTRO_CAMPOS_NUM; i++ ) 
	{
	    MAT matrix;
        BuildLookMatrixForward( &camPosData[i].vEye, &camPosData[i].vFocusPt, &matrix );
        MatToQuat( &matrix, &camPosData[i].Quat );
    }
}



*/