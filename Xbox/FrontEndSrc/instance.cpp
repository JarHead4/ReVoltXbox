//-----------------------------------------------------------------------------
// File: instance.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "instance.h"
#include "LevelLoad.h"
#include "camera.h"
#include "geom.h"
#include "main.h"
#include "light.h"
#include "visibox.h"
#include "drawobj.h"
#include "newcoll.h"

#ifdef _PC
#include "text.h"
#include "draw.h"
#include "input.h"
#include "mirror.h"
#include "settings.h"

#elif defined _N64
#include "ffs_list.h"
#include "TitleScreen.h"
#endif

//----------------------
// N64 Statics globals
//----------------------
#ifdef _N64

#if DEBUG                                                               
static unsigned s_uNErrors;
static char     s_aErrors[MAX_INSTANCES][64];
#define INSTANCEERROR(pcError)                                          \
    {                                                                   \
    unsigned i;                                                         \
    for (i =0; i<s_uNErrors && strcmp(s_aErrors[i],pcError); i++);      \
    if (i==s_uNErrors)                                                  \
        {                                                               \
        strcpy(s_aErrors[s_uNErrors++],pcError);                        \
        printf("WARNING: instance %s not created.\n",pcError); \
        }                                                               \
    }                                                                   
#else
#define INSTANCEERROR(pcError)                                          
#endif

static struct 
    {
    char * pcName;
    long   lId;
    } s_N64InstanceTabNames[] = {

        // TOYS1
        //-------------------------------------
        { "TOYROCKE", INSTANCE_MODEL_TOYROCKE },
        { "ROCKETPA", INSTANCE_MODEL_ROCKETPA },
        { "PALMTREE", INSTANCE_MODEL_PALMTREE },
        { "CARDBOX" , INSTANCE_MODEL_CARDBOX  },
        { "GENBOX"  , INSTANCE_MODEL_GENBOX   },
        { "CARBOX"  , INSTANCE_MODEL_CARBOX   },
        { "TOYDRAWE", INSTANCE_MODEL_TOYDRAWE },
        { "ABCBLOCK", INSTANCE_MODEL_ABCBLOCK },

        // NHOOD1
        //-------------------------------------
        { "TREE"    , INSTANCE_MODEL_TREE1    },
        { "TREE1"   , INSTANCE_MODEL_TREE1    },
        { "HOUSE1"  , INSTANCE_MODEL_HOUSE1   },
        { "HOUSE2"  , INSTANCE_MODEL_HOUSE2   },
        { "LAMPOST" , INSTANCE_MODEL_LAMPPOST },
        { "WHITEPOS", INSTANCE_MODEL_WHITEPOS },
        { "HOOP"    , INSTANCE_MODEL_HOOP     },
        { "BARRIER" , INSTANCE_MODEL_BARRIER  },
        { "BARRIERP", INSTANCE_MODEL_BARRIERP },
        { "START"   , INSTANCE_MODEL_START    },
        { "BMW"     , INSTANCE_MODEL_BMW      },
        { "CARRAMP" , INSTANCE_MODEL_CARRAMP  },
        { "BIN"     , INSTANCE_MODEL_BIN      },
        { "RAMP"    , INSTANCE_MODEL_RAMP     },

        // MUSE1
        //-------------------------------------
        { "SIGNGO"  , INSTANCE_MODEL_SIGNGO   },
        { "FIREXT"  , INSTANCE_MODEL_FIREXT   },
        { "CABINE"  , INSTANCE_MODEL_CABINE   },
        { "SIGN2"   , INSTANCE_MODEL_SIGN2    },
        { "MUMMY"   , INSTANCE_MODEL_MUMMY    },
        { "BENCH"   , INSTANCE_MODEL_BENCH    },
        { "AMON"    , INSTANCE_MODEL_AMON     },
        { "MUSEBIN" , INSTANCE_MODEL_MUSEBIN  },
        { "TRICER"  , INSTANCE_MODEL_TRICER   },
        { "STAND"   , INSTANCE_MODEL_STAND    },
        { "ROPE"    , INSTANCE_MODEL_ROPE     },
        { "SIGN4"   , INSTANCE_MODEL_SIGN4    },
        { "MUSERAMP", INSTANCE_MODEL_MUSERAMP },
        { "CABGLASS", INSTANCE_MODEL_CABGLASS },
        { "GLASSCAS", INSTANCE_MODEL_GLASSCAS },
        { "LILGLASS", INSTANCE_MODEL_LILGLASS },
        { "SWORD"   , INSTANCE_MODEL_SWORD    },
        { "DGLASSB" , INSTANCE_MODEL_DGLASSB  },
        { "DGLASSS" , INSTANCE_MODEL_DGLASSS  },

        // GARDEN1
        //-------------------------------------
        { "BUSH1"   , INSTANCE_MODEL_BUSH1    },
        { "PLANT2"  , INSTANCE_MODEL_PLANT2   },
        { "PLANT3"  , INSTANCE_MODEL_PLANT3   },
        { "PLANT4"  , INSTANCE_MODEL_PLANT4   },
        { "LEAF1"   , INSTANCE_MODEL_LEAF1    },
        { "GTREE1"  , INSTANCE_MODEL_GTREE1   },
        { "FOUNTAIN", INSTANCE_MODEL_FOUNTAIN },
        { "GFOUNTAI", INSTANCE_MODEL_FOUNTAIN },
        { "IVY1"    , INSTANCE_MODEL_IVY1     },
        { "GPALM"   , INSTANCE_MODEL_GPALM    },

        // MUSE2
        //-------------------------------------
        { "ROCKETB" , INSTANCE_MODEL_ROCKETB  },
        { "SATURNB" , INSTANCE_MODEL_SATURNB  },

        // FRONTEND
        //-------------------------------------
        { "BOX01"   , INSTANCE_MODEL_BOX1     },
        { "BOX02"   , INSTANCE_MODEL_BOX2     },
        { "BOX03"   , INSTANCE_MODEL_BOX3     },
        { "BOX04"   , INSTANCE_MODEL_BOX4     },
        { "BOX5"    , INSTANCE_MODEL_BOX5     },
        { "PODIUM"  , INSTANCE_MODEL_PODIUM   },
        { "BBALLBOX", INSTANCE_MODEL_BBALLBOX },
        { "POSTERST", INSTANCE_MODEL_POSTERST },

        // WILD WEST
        //-------------------------------------
        { "TABLE"   , INSTANCE_MODEL_TABLE    },
        { "SSHAHID" , INSTANCE_MODEL_SSHAHID  },
        { "ROCKERC" , INSTANCE_MODEL_ROCKERC  },    
        { "SGSTORE" , INSTANCE_MODEL_SGSTORE  },    
        { "SVSTONE" , INSTANCE_MODEL_SVSTONE  },    
        { "SSALOON" , INSTANCE_MODEL_SSALOON  },    
        { "SUTAKERS", INSTANCE_MODEL_SUTAKERS },
        { "SBANK"   , INSTANCE_MODEL_SBANK    }, 
        { "WCHAIR"  , INSTANCE_MODEL_WCHAIR   }, 
        { "COFFIN"  , INSTANCE_MODEL_COFFIN   },
        { "SBAYNES" , INSTANCE_MODEL_SBAYNES  }, 
        { "SPHIPENS", INSTANCE_MODEL_SPHIPENS }, 
        { "BARRELL" , INSTANCE_MODEL_BARRELL  },    
        { "PENANT"  , INSTANCE_MODEL_PENANT   }, 
        { "BELLPOST", INSTANCE_MODEL_BELLPOST },
        { "WWBOX"   , INSTANCE_MODEL_WWBOX    },

        // MARKET1
        //-------------------------------------
        { "STACK"   , INSTANCE_MODEL_STACK    },
        { "PACKET"  , INSTANCE_MODEL_PACKET   },
        { "FORKLIFT", INSTANCE_MODEL_FORKLIFT },

        // NHOOD2
        //-------------------------------------
        { "SFLOWER" , INSTANCE_MODEL_SFLOWERS },
        { "DUSTBIN" , INSTANCE_MODEL_DUSTBIN  },
        { "STOOL"   , INSTANCE_MODEL_STOOL    },
        { "PANTS"   , INSTANCE_MODEL_PANTS    },
        { "DOGB"    , INSTANCE_MODEL_DOGB     },

        // WWEST2
        //-------------------------------------
        { "WWBIGBOX", INSTANCE_MODEL_WWBIGBOX },
        { "BOTTLE"  , INSTANCE_MODEL_BOTTLE   },
        { "SLEEPER" , INSTANCE_MODEL_SLEEPER  },
        { "WWDANGER", INSTANCE_MODEL_WWDANGER },
        { "WWRAMP"  , INSTANCE_MODEL_WWRAMP   },
        { "BEAM"    , INSTANCE_MODEL_BEAM     },

        // SHIP
        //-------------------------------------
        { "CSRING"  , INSTANCE_MODEL_CSRING   },
        { "DECKRAMP", INSTANCE_MODEL_DECKRAMP },
        { "DECKSEAT", INSTANCE_MODEL_DECKSEAT },
        { "CSSTART" , INSTANCE_MODEL_CSSTART  },
        { "RINGROPE", INSTANCE_MODEL_RINGROPE },

        };                                    

static long s_N64NInstances = sizeof(s_N64InstanceTabNames) / sizeof(s_N64InstanceTabNames[0]);
#endif


// globals

long InstanceNum;
INSTANCE *CurrentInstance;
INSTANCE Instances[MAX_INSTANCES];
long InstanceModelNum;
INSTANCE_MODELS *InstanceModels;

static char InstanceAxis = 0, InstanceAxisType = 0, InstanceRgbType = TRUE;
static unsigned char LastModel;

// misc edit text

static char *InstanceAxisNames[] = {
    "X Y",
    "X Z",
    "Z Y",
    "X",
    "Y",
    "Z",
};

static char *InstanceAxisTypeNames[] = {
    "Camera",
    "World",
};

////////////////////////////
// load / setup instances //
////////////////////////////
#ifdef _PC
void LoadInstances(char *file)
{
    long i, j;
    FILE *fp;
    FILE_INSTANCE fin;

// open instance file

    fp = fopen(file, "rb");

// if not there create empty one

    if (!fp)
    {
        fp = fopen(file, "wb");
        if (!fp) return;
        i = 0;
        fwrite(&i, sizeof(i), 1, fp);
        fclose(fp);
        fp = fopen(file, "rb");
        if (!fp) return;
    }

// loop thru all instances

    fread(&InstanceNum, sizeof(InstanceNum), 1, fp);

    for (i = 0 ; i < InstanceNum ; i++)
    {

// load one file instance

        fread(&fin, sizeof(fin), 1, fp);

// find it's model set

        fin.Name[MAX_INSTANCE_FILENAME - 1] = 0;
        for (j = 0 ; j < InstanceModelNum ; j++)
        {
            if (!strcmp(fin.Name, InstanceModels[j].Name))
            {
                Instances[i].Model = (char)j;
                break;
            }
        }

// ignore if can't find model set

        if (j == InstanceModelNum)
        {
            i--;
            InstanceNum--;
            continue;
        }

// setup misc from file

        if (!_stricmp(fin.Name, "flouro") || !_stricmp(fin.Name, "lflouro")) //$MODIFIED: changed stricmp to _stricmp
            Instances[i].FrigMirrors = TRUE;
        else
            Instances[i].FrigMirrors = FALSE;

        Instances[i].r = fin.r;
        Instances[i].g = fin.g;
        Instances[i].b = fin.b;
        Instances[i].Priority = fin.Priority;
        Instances[i].Flag = fin.Flag;
        Instances[i].EnvRGB = fin.EnvRGB;
        Instances[i].LodBias = fin.LodBias;
        Instances[i].WorldPos = fin.WorldPos;
        Instances[i].WorldMatrix = fin.WorldMatrix;

// zero model rgb?

        if (!(Instances[i].Flag & INSTANCE_SET_MODEL_RGB))
        {
            Instances[i].r = 0;
            Instances[i].g = 0;
            Instances[i].b = 0;
            Instances[i].Flag |= INSTANCE_SET_MODEL_RGB;
        }

// set bounding boxes + rgb's

        SetInstanceBoundingBoxes(&Instances[i]);
        AllocOneInstanceRGB(&Instances[i]);

// set mirror flags

        if ((Instances[i].MirrorFlag = GetMirrorPlane(&Instances[i].WorldPos)))
            Instances[i].MirrorHeight = MirrorHeight;
    }

// close instance file

    fclose(fp);
}

////////////////////////
// save instance file //
////////////////////////

void SaveInstances(char *file)
{
    long i;
    FILE *fp;
    FILE_INSTANCE finst;
    INSTANCE *inst;
    char bak[256];

// backup old file

    memcpy(bak, file, strlen(file) - 3);
    wsprintf(bak + strlen(file) - 3, "fi-");
    remove(bak);
    rename(file, bak);

// open object file

    fp = fopen(file, "wb");
    if (!fp) return;

// write num

    fwrite(&InstanceNum, sizeof(InstanceNum), 1, fp);

// write out each instance

    inst = Instances;
    for (i = 0 ; i < InstanceNum ; i++, inst++)
    {

// set file instance

        memcpy(finst.Name, InstanceModels[inst->Model].Name, MAX_INSTANCE_FILENAME);

        finst.r = inst->r;
        finst.g = inst->g;
        finst.b = inst->b;
        finst.Priority = inst->Priority;
        finst.Flag = inst->Flag;
        finst.EnvRGB = inst->EnvRGB;
        finst.LodBias = inst->LodBias;
        finst.WorldPos = inst->WorldPos;
        finst.WorldMatrix = inst->WorldMatrix;

// write it

        fwrite(&finst, sizeof(finst), 1, fp);
    }

// close file

    DumpMessage("Saved Instance File:", file);
    fclose(fp);
}

//////////////////////
// edit an instance //
//////////////////////

void EditInstances(void)
{
    long i, flag;
    INSTANCE *inst, *ninst;
    VEC vec, vec2;
    MAT mat, mat2;
    VEC r, u, l, r2, u2, l2;
    MODEL_RGB *rgb;
    float xrad, yrad, sx, sy, ndist, dist;

// quit if not in edit mode

    if (CAM_MainCamera->Type != CAM_EDIT)
    {
        CurrentInstance = NULL;
        return;
    }

// rotate camera?

    if (MouseRight)
    {
//$REMOVED (tentative!!)
//        RotMatrixZYX(&mat, (float)-Mouse.lY / 3072, -(float)Mouse.lX / 3072, 0);
//        MulMatrix(&CAM_MainCamera->WMatrix, &mat, &mat2);
//        CopyMatrix(&mat2, &CAM_MainCamera->WMatrix);
//
//        CAM_MainCamera->WMatrix.m[RY] = 0;
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[X]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[Z], &CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y]);
//        NormalizeVector(&CAM_MainCamera->WMatrix.mv[Y]);
//        CrossProduct(&CAM_MainCamera->WMatrix.mv[X], &CAM_MainCamera->WMatrix.mv[Y], &CAM_MainCamera->WMatrix.mv[Z]);
//$END_REMOVAL
    }

// save instances?

    if (Keys[DIK_LCONTROL] && Keys[DIK_F4] && !LastKeys[DIK_F4])
    {
        SaveInstances(GetLevelFilename("fin", FILENAME_MAKE_BODY | FILENAME_GAME_SETTINGS));
    }

// get a current instance?

    if (!CurrentInstance && Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        ninst = NULL;
        ndist = 99999;

        inst = Instances;
        for (i = 0 ; i < InstanceNum ; i++, inst++)
        {
            RotTransVector(&ViewMatrix, &ViewTrans, &inst->WorldPos, &vec);

            if (vec.v[Z] < RenderSettings.NearClip || vec.v[Z] >= RenderSettings.FarClip) continue;

            sx = vec.v[X] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_XHALF;
            sy = vec.v[Y] * RenderSettings.GeomPers / vec.v[Z] + REAL_SCREEN_YHALF;

            xrad = (InstanceModels[inst->Model].Models[0].Radius * RenderSettings.GeomPers) / vec.v[Z];
            yrad = (InstanceModels[inst->Model].Models[0].Radius * RenderSettings.GeomPers) / vec.v[Z];

            if (MouseXpos > sx - xrad && MouseXpos < sx + xrad && MouseYpos > sy - yrad && MouseYpos < sy + yrad)
            {
                dist = (float)sqrt((sx - MouseXpos) * (sx - MouseXpos) + (sy - MouseYpos) * (sy - MouseYpos));
                if (dist < ndist)
                {
                    ninst = inst;
                    ndist = dist;
                }
            }
        }
        if (ninst)
        {
            CurrentInstance = ninst;
            return;
        }
    }

// new instance?

    if (Keys[DIK_INSERT] && !LastKeys[DIK_INSERT])
    {
        if ((inst = AllocInstance()))
        {
            vec.v[X] = 0;
            vec.v[Y] = 0;
            vec.v[Z] = 256;
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
            AddVector(&CAM_MainCamera->WPos, &vec2, &inst->WorldPos);

            RotMatrixZYX(&inst->WorldMatrix, 0, 0, 0);

            inst->r = 0;
            inst->g = 0;
            inst->b = 0;
            inst->EnvRGB = 0x808080;
            inst->Model = LastModel;
            inst->Priority = FALSE;
            inst->LodBias = 1024;
            inst->Flag = INSTANCE_SET_MODEL_RGB;

            SetInstanceBoundingBoxes(inst);
            AllocOneInstanceRGB(inst);

            CurrentInstance = inst;
        }
    }

// quit now if no current instance

    if (!CurrentInstance) return;

// set last mode

    LastModel = CurrentInstance->Model;

// set mirror flags

    if ((CurrentInstance->MirrorFlag = GetMirrorPlane(&CurrentInstance->WorldPos)))
        CurrentInstance->MirrorHeight = MirrorHeight;

// exit current instance edit?

    if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
    {
        CurrentInstance = NULL;
        return;
    }

// update bounding box and VisiMask

    SetInstanceBoundingBoxes(CurrentInstance);

// delete current instance?

    if (Keys[DIK_DELETE] && !LastKeys[DIK_DELETE])
    {
        FreeOneInstanceRGB(CurrentInstance);
        FreeInstance(CurrentInstance);
        CurrentInstance = NULL;
        return;
    }

// change axis?

    if (Keys[DIK_TAB] && !LastKeys[DIK_TAB])
    {
        if (Keys[DIK_LSHIFT]) InstanceAxis--;
        else InstanceAxis++;
        if (InstanceAxis == -1) InstanceAxis = 5;
        if (InstanceAxis == 6) InstanceAxis = 0;
    }

// change axis type?

    if (Keys[DIK_LALT] && !LastKeys[DIK_LALT])
        InstanceAxisType ^= 1;

// change model?

    if (Keys[DIK_LSHIFT]) LastKeys[DIK_NUMPADMINUS] = LastKeys[DIK_NUMPADPLUS] = 0;

    if (Keys[DIK_NUMPADMINUS] && !LastKeys[DIK_NUMPADMINUS] && CurrentInstance->Model)
    {
        FreeOneInstanceRGB(CurrentInstance);
        CurrentInstance->Model--;
        AllocOneInstanceRGB(CurrentInstance);
    }

    if (Keys[DIK_NUMPADPLUS] && !LastKeys[DIK_NUMPADPLUS] && CurrentInstance->Model < InstanceModelNum - 1)
    {
        FreeOneInstanceRGB(CurrentInstance);
        CurrentInstance->Model++;
        AllocOneInstanceRGB(CurrentInstance);
    }

// change priority?

    if (Keys[DIK_NUMPADSLASH] && !LastKeys[DIK_NUMPADSLASH]) CurrentInstance->Priority = FALSE;
    if (Keys[DIK_NUMPADSTAR] && !LastKeys[DIK_NUMPADSTAR]) CurrentInstance->Priority = TRUE;

// toggle env?

    if (Keys[DIK_NUMPADENTER] && !LastKeys[DIK_NUMPADENTER]) CurrentInstance->Flag ^= INSTANCE_ENV;

// toggle collision flags?

    if (Keys[DIK_C] && !LastKeys[DIK_C])
    {
        switch (CurrentInstance->Flag & (INSTANCE_NO_OBJECT_COLLISION | INSTANCE_NO_CAMERA_COLLISION))
        {
            case 0:
                CurrentInstance->Flag |= INSTANCE_NO_OBJECT_COLLISION;
                break;

            case INSTANCE_NO_OBJECT_COLLISION:
                CurrentInstance->Flag &= ~INSTANCE_NO_OBJECT_COLLISION;
                CurrentInstance->Flag |= INSTANCE_NO_CAMERA_COLLISION;
                break;

            case INSTANCE_NO_CAMERA_COLLISION:
                CurrentInstance->Flag &= ~INSTANCE_NO_CAMERA_COLLISION;
                break;
        }
    }

// toggle hide?

    if (Keys[DIK_H] && !LastKeys[DIK_H])
    {
        FreeOneInstanceRGB(CurrentInstance);
        CurrentInstance->Flag ^= INSTANCE_HIDE;
        AllocOneInstanceRGB(CurrentInstance);
    }

// toggle mirror?

    if (Keys[DIK_M] && !LastKeys[DIK_M]) CurrentInstance->Flag ^= INSTANCE_NO_MIRROR;

// toggle rgb type?

    if (Keys[DIK_SPACE] && !LastKeys[DIK_SPACE]) InstanceRgbType ^= TRUE;

// toggle lit

    if (Keys[DIK_L] && !LastKeys[DIK_L]) CurrentInstance->Flag ^= INSTANCE_NO_FILE_LIGHTS;

// change env rgb?

    if (!InstanceRgbType)
    {
        rgb = (MODEL_RGB*)&CurrentInstance->EnvRGB;

        if (Keys[DIK_LSHIFT]) LastKeys[DIK_P] = LastKeys[DIK_SEMICOLON] = LastKeys[DIK_LBRACKET] = LastKeys[DIK_APOSTROPHE] = LastKeys[DIK_RBRACKET] = LastKeys[DIK_BACKSLASH] = 0;

        if (Keys[DIK_P] && !LastKeys[DIK_P] && rgb->r < 255) rgb->r++;
        if (Keys[DIK_SEMICOLON] && !LastKeys[DIK_SEMICOLON] && rgb->r) rgb->r--;

        if (Keys[DIK_LBRACKET] && !LastKeys[DIK_LBRACKET] && rgb->g < 255) rgb->g++;
        if (Keys[DIK_APOSTROPHE] && !LastKeys[DIK_APOSTROPHE] && rgb->g) rgb->g--;

        if (Keys[DIK_RBRACKET] && !LastKeys[DIK_RBRACKET] && rgb->b < 255) rgb->b++;
        if (Keys[DIK_BACKSLASH] && !LastKeys[DIK_BACKSLASH] && rgb->b) rgb->b--;
    }

// change model rgb?

    if (InstanceRgbType)
    {
        flag = FALSE;

        if (Keys[DIK_LSHIFT]) LastKeys[DIK_P] = LastKeys[DIK_SEMICOLON] = LastKeys[DIK_LBRACKET] = LastKeys[DIK_APOSTROPHE] = LastKeys[DIK_RBRACKET] = LastKeys[DIK_BACKSLASH] = 0;

        if (Keys[DIK_P] && !LastKeys[DIK_P] && CurrentInstance->r < 127) CurrentInstance->r++, flag = TRUE;
        if (Keys[DIK_SEMICOLON] && !LastKeys[DIK_SEMICOLON] && CurrentInstance->r > -128) CurrentInstance->r--, flag = TRUE;

        if (Keys[DIK_LBRACKET] && !LastKeys[DIK_LBRACKET] && CurrentInstance->g < 127) CurrentInstance->g++, flag = TRUE;
        if (Keys[DIK_APOSTROPHE] && !LastKeys[DIK_APOSTROPHE] && CurrentInstance->g > -128) CurrentInstance->g--, flag = TRUE;

        if (Keys[DIK_RBRACKET] && !LastKeys[DIK_RBRACKET] && CurrentInstance->b < 127) CurrentInstance->b++, flag = TRUE;
        if (Keys[DIK_BACKSLASH] && !LastKeys[DIK_BACKSLASH] && CurrentInstance->b > -128) CurrentInstance->b--, flag = TRUE;

        if (flag)
        {
            FreeOneInstanceRGB(CurrentInstance);
            AllocOneInstanceRGB(CurrentInstance);
        }
    }

// change LOD bias?

    if (Keys[DIK_MINUS]) CurrentInstance->LodBias -= TimeStep * 288;
    if (Keys[DIK_EQUALS]) CurrentInstance->LodBias += TimeStep * 288;

    if (CurrentInstance->LodBias < 64) CurrentInstance->LodBias = 64;
    if (CurrentInstance->LodBias > 8192) CurrentInstance->LodBias = 8192;

// copy instance?

    if (MouseLeft && !MouseLastLeft && Keys[DIK_LSHIFT])
    {
        if ((inst = AllocInstance()))
        {
            memcpy(inst, CurrentInstance, sizeof(INSTANCE));
            CurrentInstance = inst;
            return;
        }
    }

// move instance?

    if (MouseLeft)
    {
        RotTransVector(&ViewMatrix, &ViewTrans, &CurrentInstance->WorldPos, &vec);

        switch (InstanceAxis)
        {
            case INSTANCE_AXIS_XY:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case INSTANCE_AXIS_XZ:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case INSTANCE_AXIS_ZY:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
            case INSTANCE_AXIS_X:
                vec.v[X] = MouseXrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case INSTANCE_AXIS_Y:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditYrel;
                vec.v[Z] = CameraEditZrel;
                break;
            case INSTANCE_AXIS_Z:
                vec.v[X] = CameraEditXrel;
                vec.v[Y] = CameraEditYrel;
                vec.v[Z] = -MouseYrel * vec.v[Z] / RenderSettings.GeomPers + CameraEditZrel;
                break;
        }

        if (InstanceAxisType == 1) 
        {
            SetVector(&vec2, vec.v[X], vec.v[Y], vec.v[Z]);
        }
        else
        {
            RotVector(&CAM_MainCamera->WMatrix, &vec, &vec2);
        }

        CurrentInstance->WorldPos.v[X] += vec2.v[X];
        CurrentInstance->WorldPos.v[Y] += vec2.v[Y];
        CurrentInstance->WorldPos.v[Z] += vec2.v[Z];
    }

// rotate instance?

    vec.v[X] = vec.v[Y] = vec.v[Z] = 0;

    if (Keys[DIK_NUMPAD7]) vec.v[X] -= 0.005f;
    if (Keys[DIK_NUMPAD4]) vec.v[X] += 0.005f;
    if (Keys[DIK_NUMPAD8]) vec.v[Y] -= 0.005f;
    if (Keys[DIK_NUMPAD5]) vec.v[Y] += 0.005f;
    if (Keys[DIK_NUMPAD9]) vec.v[Z] -= 0.005f;
    if (Keys[DIK_NUMPAD6]) vec.v[Z] += 0.005f;

    if (Keys[DIK_NUMPAD1] && !LastKeys[DIK_NUMPAD1]) vec.v[X] += 0.25f;
    if (Keys[DIK_NUMPAD2] && !LastKeys[DIK_NUMPAD2]) vec.v[Y] += 0.25f;
    if (Keys[DIK_NUMPAD3] && !LastKeys[DIK_NUMPAD3]) vec.v[Z] += 0.25f;

    if (Keys[DIK_NUMPAD0]) CopyMatrix(&IdentityMatrix, &CurrentInstance->WorldMatrix);

    RotMatrixZYX(&mat, vec.v[X], vec.v[Y], vec.v[Z]);

    if (InstanceAxisType)
    {
        MulMatrix(&mat, &CurrentInstance->WorldMatrix, &mat2);
        CopyMatrix(&mat2, &CurrentInstance->WorldMatrix);
        NormalizeMatrix(&CurrentInstance->WorldMatrix);
    }
    else if (vec.v[X] || vec.v[Y] || vec.v[Z])
    {
        RotVector(&ViewMatrix, &CurrentInstance->WorldMatrix.mv[X], &r);
        RotVector(&ViewMatrix, &CurrentInstance->WorldMatrix.mv[Y], &u);
        RotVector(&ViewMatrix, &CurrentInstance->WorldMatrix.mv[Z], &l);

        RotVector(&mat, &r, &r2);
        RotVector(&mat, &u, &u2);
        RotVector(&mat, &l, &l2);

        RotVector(&CAM_MainCamera->WMatrix, &r2, &CurrentInstance->WorldMatrix.mv[X]);
        RotVector(&CAM_MainCamera->WMatrix, &u2, &CurrentInstance->WorldMatrix.mv[Y]);
        RotVector(&CAM_MainCamera->WMatrix, &l2, &CurrentInstance->WorldMatrix.mv[Z]);

        NormalizeMatrix(&CurrentInstance->WorldMatrix);
    }
}

#endif 

//----------------------------------------------------------------------
// load / setup instances (N64) 
//----------------------------------------------------------------------
#ifdef _N64
void LoadInstances(char *file)
{

    FIL *fp = NULL;
    long Mem1, Mem2;
    long i, j;
    FILE_INSTANCE fin;

    static struct renderflags renderflag;
    renderflag.envmap = FALSE;
    renderflag.envonly = FALSE;
    renderflag.light = TRUE;
    renderflag.litsimple = FALSE;
    renderflag.reflect = TRUE;
    renderflag.fog = TRUE;
    renderflag.glare = FALSE;
    renderflag.meshfx = TRUE;

    // open instance file
    //-------------------------
    printf("Loading level instances...\n");
    Mem1 = MEM_GetMemUsed();
    if (GameSettings.Reversed)
        fp = FFS_Open(FFS_TYPE_TRACK | TRK_REV_INSTANCES );
    if (!fp) 
        fp = FFS_Open(FFS_TYPE_TRACK | TRK_INSTANCES );

    // if not there set number to 0
    //--------------------------------
    if (!fp)
        {
        InstanceNum = 0;
        printf("WARNING: could not open instance file.\n");
        return;
        }

    // loop thru all instances
    //-------------------------
    FFS_Read(&InstanceNum, sizeof(InstanceNum), fp);
    InstanceNum = EndConvLong(InstanceNum);
    
    #if DEBUG                                                               
    s_uNErrors = 0;
    #endif
    
    for (i = 0 ; i < InstanceNum ; i++)
    {

        // load one file instance
        //-------------------------
        long ii;
        float a;
        FFS_Read(&fin, sizeof(fin), fp);

        // find it's model set
        //-------------------------
        fin.Name[MAX_INSTANCE_FILENAME - 1] = 0;
        for (j = 0 ; j < s_N64NInstances; j++)
        {
            if (!strcmp(fin.Name, s_N64InstanceTabNames[j].pcName))
            {
                renderflag.envmap  = (Instances[i].Flag & INSTANCE_ENV);
                Instances[i].Model = LoadOneInstanceModel(s_N64InstanceTabNames[j].lId, TRUE, renderflag, 0) ;
                break;
            }
        }

        // ignore if can't find model set
        //-------------------------------
        if ((j == s_N64NInstances) || ( Instances[i].Model == 255 ) )
        {
            INSTANCEERROR(fin.Name)
            i--;
            InstanceNum--;
            continue;
        }

        // Endian converting
        //-------------------------------       
        fin.EnvRGB        = EndConvLong(fin.EnvRGB );

        MyEndConvReal(&fin.LodBias);
        MyEndConvReal(&fin.WorldPos.v[0]);
        MyEndConvReal(&fin.WorldPos.v[1]);
        MyEndConvReal(&fin.WorldPos.v[2]);
        for (ii=0; ii<9; ii++) 
            {
            MyEndConvReal(&fin.WorldMatrix.m[ii]);
            }

        // setup misc from file
        //-------------------------------       
        Instances[i].r = fin.r;
        Instances[i].g = fin.g;
        Instances[i].b = fin.b;
        Instances[i].Priority = fin.Priority;
        Instances[i].Flag = (fin.Flag & (~INSTANCE_HIDE) ) | ( (((fin.Flag & INSTANCE_NO_MIRROR) && g_TitleScreenData.iLevelNum == 4)  || InstanceModel[Instances[i].Model].Model.hdr->semiptr) ? INSTANCE_HIDE : 0) ;
        Instances[i].EnvRGB = fin.EnvRGB;
        Instances[i].LodBias = fin.LodBias;
        Instances[i].WorldPos = fin.WorldPos;
        Instances[i].WorldMatrix = fin.WorldMatrix;


        // zero model rgb?
        //-------------------------------       
        if (!(Instances[i].Flag & INSTANCE_SET_MODEL_RGB))
        {
            Instances[i].r = 0;
            Instances[i].g = 0;
            Instances[i].b = 0;
            Instances[i].Flag |= INSTANCE_SET_MODEL_RGB;
        }

        // set bounding boxes + rgb's
        //-------------------------------       
        SetInstanceBoundingBoxes(&Instances[i]);

        // get model RGB
        //----------------------------------------------
        if (!(Instances[i].Flag & INSTANCE_HIDE))
            AllocOneInstanceRGB(&Instances[i]);
        else
            {
            Instances[i].vtx = NULL;
            Instances[i].gfx = NULL;
            Instances[i].rgb = NULL;
            InstanceModel[Instances[i].Model].KeepData = TRUE;
            }

        // set mirror flags (stay here, for 
        //-------------------------------       
        #if 0
        if ((Instances[i].MirrorFlag = GetMirrorPlane(&Instances[i].WorldPos)))
            Instances[i].MirrorHeight = MirrorHeight;
        #endif

    }

    // close instance file
    //-------------------------------       
    FFS_Close(fp);

    // Clean Memory
    //-------------------------------       
    for (i=0; i<InstanceNum; i++)
        {
        MODEL *mod = &InstanceModel[Instances[i].Model].Model;
        if (mod && mod->hdr->gfxptr)
            {
            if (!(Instances[i].Flag & INSTANCE_HIDE) && !(InstanceModel[Instances[i].Model].KeepData))
                {
                free(mod->hdr->gfxptr);
                free(mod->hdr->vtxptr);
                free(mod->hdr->rgbptr);
                mod->hdr->gfxptr = NULL;
                mod->hdr->vtxptr = NULL;
                mod->hdr->rgbptr = NULL;
                }
            free(mod->hdr->pVtxListPtr);               
            mod->hdr->pVtxListPtr = NULL;
            }
        }


    Mem2 = MEM_GetMemUsed();
    log_printf("...instance objects use %d bytes.\n", Mem2 - Mem1);
}

//----------------------------------------------------------------------
// void AllocOneInstanceRGB(INSTANCE *inst)
//----------------------------------------------------------------------
void AllocOneInstanceRGB(INSTANCE *inst)
{
    
    // alloc Vtx space
    //----------------------------------------------
    long i, j, k, col;
    MODEL *     model       = &InstanceModel[inst->Model].Model;
    unsigned    uNVtx       = model->hdr->vtxnum;   
    int         iVtxColor   = 0;        
    unsigned    uSizeVtxBuf = sizeof(Vtx) * uNVtx;
    unsigned    uSizeRgbBuf = sizeof(RGBA) * uNVtx;

    inst->vtx = (Vtx *)malloc(uSizeVtxBuf);
    inst->rgb = (RGBA*)malloc(uSizeRgbBuf);

    // Copy the whole thing in one pass
    //----------------------------------------------
    memcpy(inst->vtx,model->hdr->vtxptr,uSizeVtxBuf);
    memcpy(inst->rgb,model->hdr->rgbptr,uSizeRgbBuf);

    // Now do copy gfx list and swap vertices 
    //----------------------------------------------
        {   
        // Search for the end;
        //-------------------
        Gfx         EndList;
        Gfx         GspVertex;
        unsigned    uSizeGfx;
        Vtx *       pOldList    = model->hdr->vtxptr;
        Gfx *       pSrcList    = model->hdr->gfxptr;
        VTXList *   pVtxList    = model->hdr->pVtxListPtr;
        Gfx **      pVtxPtr     = pVtxList->pVtxList; 
        unsigned    uLastElem   = pVtxList->uNElems - 1;
        Gfx *       pLastRecord = pVtxPtr[uLastElem];
        
        gSPEndDisplayList(&EndList);
        while (pLastRecord->words.w1 != EndList.words.w1 || pLastRecord->words.w0 != EndList.words.w0) pLastRecord++;

        // alloc and copy
        //-------------------
        ++pLastRecord;
        uSizeGfx  = ((char*)pLastRecord - (char*)pSrcList);
        inst->gfx = (Gfx*) malloc( uSizeGfx );
        memcpy(inst->gfx,pSrcList,uSizeGfx);

        // personalize list with vertex set
        //---------------------------------     
        for (i=pVtxList->uNElems;i--;)
            {
            Gfx *pRealVtxPtr = *pVtxPtr++ - pSrcList + inst->gfx;
            pRealVtxPtr->words.w1 = (unsigned int) ( ((Vtx*)(pRealVtxPtr->words.w1) - pOldList) + inst->vtx);
            }
        }
    
    for (j = 0 ; j < uNVtx; j++)
    {
        col = model->hdr->vtxptr[j].v.cn[0] * (inst->r + 128) / 128;
        if (col > 255) col = 255;
        inst->vtx[j].v.cn[0] = inst->rgb[j].r = inst->vtx[j].v.cn[0] = (unsigned char)col;

        col = model->hdr->vtxptr[j].v.cn[1] * (inst->g + 128) / 128;
        if (col > 255) col = 255;
        inst->vtx[j].v.cn[1] = inst->rgb[j].g = inst->vtx[j].v.cn[1] = (unsigned char)col;

        col = model->hdr->vtxptr[j].v.cn[2] * (inst->b + 128) / 128;
        if (col > 255) col = 255;
        inst->vtx[j].v.cn[2] = inst->rgb[j].b = inst->vtx[j].v.cn[2] = (unsigned char)col;
    }
}


//----------------------------------------------------------------------
// void DrawInstances(void)
//----------------------------------------------------------------------
void DrawInstances(void)
{
    static iDraw = 0;
    short flag;
    long i, visflag, lit;
    float z, flod;
    MODEL *model;
    INSTANCE *inst;
    Vtx * savevtx;
    RGBA *savergb;
    Gfx * savegfx;

    // loop thru all instances
    //-------------------------------       
    inst = Instances;
    for (i = 0 ; i < InstanceNum ; i++, inst++)
    {

#if 0
        // skip if mirror hide and mirrors off  
        //------------------------------------
        if ((inst->Flag & INSTANCE_HIDE) && !RenderSettings.Mirror)
            continue;

        // skip if turned off
        //------------------------------------
        if (!RenderSettings.Instance && !inst->Priority && !(inst->Flag & INSTANCE_HIDE))
            continue;

#endif


        // visicube reject?
        //------------------------------------
        if (inst->VisiMask & CamVisiMask)
            continue;

        // reset draw flag
        //------------------------------------
        flag = MODEL_PLAIN;

        // skip if offscreen
        //------------------------------------
        model = &InstanceModel[inst->Model].Model;

        visflag = TestSphereToFrustum(&inst->WorldPos, model->Radius, &z);
        if (visflag == SPHERE_OUT) continue;


        // in light?
        //------------------------------------          
        if (!(inst->Flag & INSTANCE_HIDE))
        {
            // re-direct model rgb
            //------------------------------------  
            savevtx = model->hdr->vtxptr;
            savergb = model->hdr->rgbptr;
            savegfx = model->hdr->gfxptr;
            model->hdr->vtxptr = inst->vtx;
            model->hdr->rgbptr = inst->rgb;
            model->hdr->gfxptr = inst->gfx;

            lit = CheckInstanceLight(inst, model->Radius);
            if (lit)
                {
                flag |= MODEL_LIT;              
                AddModelLight(model, &inst->WorldPos, &inst->WorldMatrix);
                }
        }

        // env map?
        //------------------------------------          
        if (inst->Flag & INSTANCE_ENV)
        {
            //flag |= MODEL_ENV;
            //SetEnvStatic(&inst->WorldPos, &inst->WorldMatrix, inst->EnvRGB, 0.0f, 0.0f, 1.0f);
        }

#if 0
        // reflect?
        //------------------------------------          
        if (!(inst->Flag & INSTANCE_NO_MIRROR) && inst->MirrorFlag && RenderSettings.Mirror)
        {
            if (ViewCameraPos.v[Y] < inst->MirrorHeight)
            {
                MirrorHeight = inst->MirrorHeight;
                flag |= MODEL_MIRROR;
            }
        }
#endif

#if 0
        // mesh fx?
        //------------------------------------          
        CheckModelMeshFx(model, &inst->WorldMatrix, &inst->WorldPos, &flag);
#endif

        // draw
        //------------------------------------          
        MOD_DrawModel(model,&inst->WorldMatrix, &inst->WorldPos, flag, inst->EnvRGB);

        // restore model rgb
        //------------------------------------                  
        // in light?
        //------------------------------------          
        if (!(inst->Flag & INSTANCE_HIDE))
            {
            model->hdr->vtxptr = savevtx;
            model->hdr->rgbptr = savergb;
            model->hdr->gfxptr = savegfx;
            }
    }
}

//----------------------------------------------------------------------
// void LoadOneInstanceModel(void)
//----------------------------------------------------------------------
// Loads the specified instance from the instance model list
//----------------------------------------------------------------------
long LoadOneInstanceModel(long id, long UseMRGBper, struct renderflags renderflag, long tpage)
{
    long    ii;
    FIL     *fp;
    long    rgbper;
    long    Flag = 0;

    // look for existing model
    //------------------------------------          
    for (ii = 0; ii < MAX_INSTANCE_MODELS ; ii++)
    {
        if (InstanceModel[ii].ID == id)
        {
            InstanceModel[ii].RefCount++;
            return ii;
        }
    }

    // find new slot
    //------------------------------------          
    for (ii = 0; ii < MAX_INSTANCE_MODELS; ii++)
    {
        if (InstanceModel[ii].ID == -1)
        {
            // load model
            //------------------------------------          
            if (UseMRGBper)
                rgbper = CurrentLevelInfo.ModelRGBper;
            else
                rgbper = 100;

            if (renderflag.envmap)              Flag |= MODEL_ENV;
            if (renderflag.envonly)             Flag |= MODEL_ENVONLY;
            if (renderflag.light)               Flag |= MODEL_LIT;

            Flag |= MODEL_SETVTXPTR;
            
            if (!InstanceModelList[id].Model)
                return -1;

            MOD_LoadModel(InstanceModelList[id].Model, InstanceModelList[id].Tex, &InstanceModel[ii].Model, 0x808080, rgbper, Flag | MODEL_SETVTXPTR);
    
            // load coll skin
            //------------------------------------          
            if (InstanceModelList[id].Coll)
            {
                if ((fp = FFS_Open(InstanceModelList[id].Coll)) != NULL) 
                {
                    InstanceModel[ii].CollPoly = LoadNewCollPolys(fp, &InstanceModel[ii].NCollPolys);
                    FFS_Close(fp);
                }
            }

            // set ID / ref count
            //------------------------------------          
            InstanceModel[ii].ID = id;
            InstanceModel[ii].RefCount = 1;
            InstanceModel[ii].KeepData = FALSE;
            return ii;
        }
    }

    // slots full
    //------------------------------------              
    return -1;
}


//----------------------------------------------------------------------
// void FreeInstanceModels(void)
//----------------------------------------------------------------------
// free all Instance models //
//----------------------------------------------------------------------
void FreeInstanceModels(void)
{
    long i;

    for (i = 0 ; i < MAX_INSTANCE_MODELS ; i++)
    {
        if (InstanceModel[i].ID != -1)
        {
            InstanceModel[i].RefCount = 1;
            FreeOneInstanceModel(i);
        }
    }

    for (i = 0 ; i < InstanceNum; i++)
    {
        if (Instances[i].rgb)
            free(Instances[i].rgb);
        if (Instances[i].vtx)
            free(Instances[i].vtx);
        if (Instances[i].gfx)
            free(Instances[i].gfx);
    }

    InstanceNum = 0;
}

//----------------------------------------------------------------------
// void FreeOneInstanceModel(long slot)
//----------------------------------------------------------------------
// free one Instance model
//----------------------------------------------------------------------
void FreeOneInstanceModel(long slot)
{

    // skip if empty
    //------------------------------------              
    if (InstanceModel[slot].ID == -1)
        return;

    // dec ref count
    //------------------------------------              
    InstanceModel[slot].RefCount--;

    // free model + coll if no owners
    //------------------------------------              
    if (InstanceModel[slot].RefCount < 1)
    {
        MOD_FreeModel(&InstanceModel[slot].Model);
        DestroyCollPolys(InstanceModel[slot].CollPoly);
        InstanceModel[slot].CollPoly = NULL;
        InstanceModel[slot].NCollPolys = 0;
    }
}

#endif


///////////////////////
// alloc an instance //
///////////////////////

INSTANCE *AllocInstance(void)
{

// full?

    if (InstanceNum >= MAX_INSTANCES)
        return NULL;

// inc counter, return slot

    return &Instances[InstanceNum++];
}

//////////////////////
// free an instance //
//////////////////////

void FreeInstance(INSTANCE *inst)
{
    long idx, i;

// find index into list

    idx = (long)(inst - Instances);

// copy all higher instances down one

    for (i = idx ; i < InstanceNum - 1; i++)
    {
        Instances[i] = Instances[i + 1];
    }

// dec num

    InstanceNum--;
}

////////////////////////
// draw all instances //
////////////////////////
#ifdef _PC
void DrawInstances(void)
{
    short flag;
    long i, lod, visflag, lit;
    float z;
    MODEL *model;
    INSTANCE *inst;
    POLY_RGB *savergb;

// loop thru all instances

    inst = Instances;
    for (i = 0 ; i < InstanceNum ; i++, inst++)
    {

// skip if mirror hide and mirrors off

        if ((inst->Flag & INSTANCE_HIDE) && !RenderSettings.Mirror)
            continue;

// skip if turned off

        if (!RenderSettings.Instance && !inst->Priority && !(inst->Flag & INSTANCE_HIDE))
            continue;

// skip if 'no object collision' flag set

        if (inst->Flag & INSTANCE_NO_OBJECT_COLLISION )
            continue;

// visicube reject?

        if (inst->VisiMask & CamVisiMask)
            continue;

// reset draw flag

        flag = MODEL_PLAIN;

// skip if offscreen

        model = InstanceModels[inst->Model].Models;

        visflag = TestSphereToFrustum(&inst->WorldPos, model->Radius, &z);
        if (visflag == SPHERE_OUT) continue;
        if (visflag == SPHERE_IN) flag |= MODEL_DONOTCLIP;
        if (z - model->Radius < RenderSettings.NearClip) flag |= MODEL_NEARCLIP;

// calc lod

//      flod = z / inst->LodBias - 1;
//      if (flod < 0) flod = 0;
//      FTOL(flod, lod);
//      if (lod > InstanceModels[inst->Model].Count - 1) lod = InstanceModels[inst->Model].Count - 1;
        lod = 0;
        
// calc model

        model += lod;

//$ADDITION(jedl) - short circuit all the local lighting and transform and fog tests
        if (RegistrySettings.bUseGPU
            && model->m_pEffect != NULL)
        {
            // TODO: lights, glare, fx, mirror
            DrawModelGPU(model, &inst->WorldMatrix, &inst->WorldPos, flag);
            continue;
        }
//$END_ADDITION

// re-point model rgb

        savergb = model->PolyRGB;
        model->PolyRGB = inst->rgb[lod];

// in fog?

        if (z + model->Radius > RenderSettings.FogStart && DxState.Fog)
        {
            ModelVertFog = (inst->WorldPos.v[1] - RenderSettings.VertFogStart) * RenderSettings.VertFogMul;
            if (ModelVertFog < 0) ModelVertFog = 0;
            if (ModelVertFog > 255) ModelVertFog = 255;

            flag |= MODEL_FOG;
            FOG_ON();
        }

// in light?

        if (!(inst->Flag & INSTANCE_HIDE))
        {
            lit = CheckInstanceLight(inst, model->Radius);

            if (lit)
            {
                flag |= MODEL_LIT;
                AddModelLight(model, &inst->WorldPos, &inst->WorldMatrix);
            }
        }

// env map?

        if (inst->Flag & INSTANCE_ENV)
        {
            flag |= MODEL_ENV;
            SetEnvStatic(&inst->WorldPos, &inst->WorldMatrix, inst->EnvRGB, 0.0f, 0.0f, 1.0f);
        }

// reflect?

        if (!(inst->Flag & INSTANCE_NO_MIRROR) && inst->MirrorFlag && RenderSettings.Mirror)
        {
            if (ViewCameraPos.v[Y] < inst->MirrorHeight)
            {
                MirrorHeight = inst->MirrorHeight;
                flag |= MODEL_MIRROR;
            }
        }

// mesh fx?

        CheckModelMeshFx(model, &inst->WorldMatrix, &inst->WorldPos, &flag);

// draw

        if (inst->FrigMirrors)
        {
            MirrorDist = 1100.0f;
            MirrorMul = (256 - MirrorAdd) / MirrorDist;
        }

        DrawModel(model, &inst->WorldMatrix, &inst->WorldPos, flag);

        if (inst->FrigMirrors)
        {
            MirrorDist = CurrentLevelInfo.MirrorDist;
            MirrorMul = (256 - MirrorAdd) / MirrorDist;
        }

        if (inst->FrigMirrors)
        {
            MirrorDist = CurrentLevelInfo.MirrorDist;
            MirrorMul = (256 - MirrorAdd) / MirrorDist;
        }

// reset render states?

        if (flag & MODEL_FOG)
            FOG_OFF();

// restore model rgb

        model->PolyRGB = savergb;
    }
}

/////////////////////////////////
// display info on an instance //
/////////////////////////////////

void DisplayInstanceInfo(INSTANCE *inst)
{
    char buf[128];

// model

    DrawGameText(450, 0, 0xffff00, InstanceModels[inst->Model].Name);

// priority

    wsprintf(buf, "High priority: %s", inst->Priority ? "Yes" : "No");
    DrawGameText(450, 24, 0x00ffff, buf);

// env

    wsprintf(buf, "Env %s", inst->Flag & INSTANCE_ENV ? "On" : "Off");
    DrawGameText(450, 48, 0xff00ff, buf);

// LOD bias

    wsprintf(buf, "LOD Bias %d", (long)inst->LodBias);
    DrawGameText(450, 72, 0xffff00, buf);

// env rgb

    wsprintf(buf, "Env RGB %d %d %d", ((MODEL_RGB*)&inst->EnvRGB)->r, ((MODEL_RGB*)&inst->EnvRGB)->g, ((MODEL_RGB*)&inst->EnvRGB)->b);
    DrawGameText(450, 96, 0x00ff00, buf);

// model rgb

    wsprintf(buf, "Model RGB %d %d %d", inst->r, inst->g, inst->b);
    DrawGameText(450, 120, 0x00ffff, buf);

// axis

    wsprintf(buf, "Axis %s - %s", InstanceAxisNames[InstanceAxis], InstanceAxisTypeNames[InstanceAxisType]);
    DrawGameText(450, 144, 0xff0000, buf);

// rgb type

    wsprintf(buf, "RGB Type: %s", InstanceRgbType? "Model" : "Env");
    DrawGameText(450, 168, 0xffff00, buf);

// hide

    wsprintf(buf, "Mirror Hide: %s", inst->Flag & INSTANCE_HIDE? "Yes" : "No");
    DrawGameText(450, 192, 0x00ff00, buf);

// mirror

    wsprintf(buf, "Mirror: %s", inst->Flag & INSTANCE_NO_MIRROR? "No" : "Yes");
    DrawGameText(450, 216, 0x00ffff, buf);

// lit

    wsprintf(buf, "Fixed lights: %s", inst->Flag & INSTANCE_NO_FILE_LIGHTS? "No" : "Yes");
    DrawGameText(450, 240, 0xff0000, buf);

// collision flags

    switch (inst->Flag & (INSTANCE_NO_OBJECT_COLLISION | INSTANCE_NO_CAMERA_COLLISION))
    {
        case 0:
            DrawGameText(450, 264, 0xff00ff, "All Collision");
            break;

        case INSTANCE_NO_OBJECT_COLLISION:
            DrawGameText(450, 264, 0xff00ff, "No Object Collision");
            break;

        case INSTANCE_NO_CAMERA_COLLISION:
            DrawGameText(450, 264, 0xff00ff, "No Camera Collision");
            break;
    }

// draw axis

    TEXTUREFILTER_ON();
    ZBUFFER_ON();
    ZWRITE_ON();

    if (InstanceAxisType)
        DrawAxis(&IdentityMatrix, &CurrentInstance->WorldPos);
    else
        DrawAxis(&CAM_MainCamera->WMatrix, &CurrentInstance->WorldPos);

    BeginTextState();
}

//////////////////////////
// load instance models //
//////////////////////////

void LoadInstanceModels(void)
{
    long i;
    WIN32_FIND_DATA data;
    HANDLE handle;
    FILE *fp;
    char buf[256];
    char names[MAX_INSTANCE_MODELS][16];

// zero num / ptr

    InstanceModelNum = 0;
    InstanceModels = NULL;

// get first prm

//$MODIFIED
//    wsprintf(buf, "levels\\%s\\*.prm", CurrentLevelInfo.Dir);
    wsprintf(buf, "D:\\levels\\%s\\*.prm", CurrentLevelInfo.Dir);
//$END_MODIFICATIONS
    handle = FindFirstFile(buf, &data);
    if (handle == INVALID_HANDLE_VALUE)
        return;

// loop thru each prm

    while (TRUE)  //$NOTE: umm, how about a do-while here, guys?
    {

// add to list?

        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && InstanceModelNum < MAX_INSTANCE_MODELS)
        {
            for (i = 0 ; i < (long)strlen(data.cFileName) - 4 ; i++)
            {
//$MODIFIED
//                data.cFileName[i] = toupper(data.cFileName[i]);
                data.cFileName[i] = (char) toupper(data.cFileName[i]);
//$END_MODIFICATIONS
            }
            data.cFileName[i] = 0;
            memcpy(names[InstanceModelNum], data.cFileName, 16);
            InstanceModelNum++;
        }

// get next

        if (!FindNextFile(handle, &data))
            break;
    }

// close search handle

    FindClose(handle);

// alloc ram for instance models structure

    InstanceModels = (INSTANCE_MODELS*)malloc(sizeof(INSTANCE_MODELS) * InstanceModelNum);
    if (!InstanceModels)
    {
        DumpMessage("ERROR", "Can't alloc memory for instance models!");
        g_bQuitGame = TRUE;
        return;
    }

// fill instance model structure

    for (i = 0 ; i < InstanceModelNum ; i++)
    {
        memcpy(InstanceModels[i].Name, names[i], MAX_INSTANCE_FILENAME);
        InstanceModels[i].Name[MAX_INSTANCE_FILENAME - 1] = 0;
//$MODIFIED
//        wsprintf(buf, "levels\\%s\\%s.prm", CurrentLevelInfo.Dir, names[i]);
        wsprintf(buf, "D:\\levels\\%s\\%s.prm", CurrentLevelInfo.Dir, names[i]);
//$END_MODIFICATIONS
        InstanceModels[i].Count = LoadModel(buf, InstanceModels[i].Models, 0, MAX_INSTANCE_LOD, LOADMODEL_OFFSET_TPAGE, CurrentLevelInfo.InstanceRGBper);
        
//$ADDITION(jedl) - first pass at resource loading
        // get resource pointers from world resource bundle
        if (World.m_pXBR != NULL)
            LoadModelGPU(buf, InstanceModels[i].Models, MAX_INSTANCE_LOD, World.m_pXBR);
        else
        {
            extern HRESULT ExportInstanceModel(INT InstanceModelIndex, CHAR *file, MODEL *m, INT ModelCount);
            ExportInstanceModel(i, buf, InstanceModels[i].Models, InstanceModels[i].Count);
        }
//$END_ADDITION

        // Load collision skin if it exists
//$MODIFIED
//        wsprintf(buf, "levels\\%s\\%s.ncp", CurrentLevelInfo.Dir, names[i]);
        wsprintf(buf, "D:\\levels\\%s\\%s.ncp", CurrentLevelInfo.Dir, names[i]);
//$END_MODIFICATIONS
        if ((fp = fopen(buf, "rb")) != NULL) {
            InstanceModels[i].CollPoly = LoadNewCollPolys(fp, &InstanceModels[i].NCollPolys);
            fclose(fp);
        } else {
            InstanceModels[i].CollPoly = NULL;
            InstanceModels[i].NCollPolys = 0;
        }

    }
}

//////////////////////////
// free instance models //
//////////////////////////

void FreeInstanceModels(void)
{
    long i;

    for (i = 0 ; i < InstanceModelNum ; i++)
    {
        DestroyCollPolys(InstanceModels[i].CollPoly);
        InstanceModels[i].CollPoly = NULL;
        InstanceModels[i].NCollPolys = 0;
        FreeModel(InstanceModels[i].Models, InstanceModels[i].Count);
    }

    free(InstanceModels);
    InstanceModelNum = 0;
}

////////////////////////
// free instance rgbs //
////////////////////////

void FreeInstanceRGBs(void)
{
    long i;

    for (i = 0 ; i < InstanceNum ; i++)
    {
        FreeOneInstanceRGB(&Instances[i]);
    }
}

//////////////////////////////
// free one instances RGB's //
//////////////////////////////

void FreeOneInstanceRGB(INSTANCE *inst)
{
    long i;

    for (i = 0 ; i < InstanceModels[inst->Model].Count ; i++)
    {
        free(inst->rgb[i]);
    }
}

/////////////////////////////
// alloc one instances rgb //
/////////////////////////////

void AllocOneInstanceRGB(INSTANCE *inst)
{
    long i, j, k, col;
    MODEL *model;

// step through each LOD

    for (i = 0 ; i < InstanceModels[inst->Model].Count ; i++)
    {

// alloc RGB space

        model = &InstanceModels[inst->Model].Models[i];
        inst->rgb[i] = (POLY_RGB*)malloc(sizeof(POLY_RGB) * model->PolyNum);
        if (!inst->rgb[i])
        {
            DumpMessage("ERROR", "Can't alloc memory for Instance RGB");
            g_bQuitGame = TRUE;
            return;
        }

// get model RGB

        for (j = 0 ; j < model->PolyNum ; j++)
        {
            for (k = 0 ; k < (model->PolyPtr[j].Type & POLY_QUAD ? 4 : 3) ; k++)
            {
                if (inst->Flag & INSTANCE_HIDE)
                {
                    *(long*)&inst->rgb[i][j].rgb[k] = CurrentLevelInfo.FogColor;
                }
                else
                {
                    inst->rgb[i][j].rgb[k].a = model->PolyRGB[j].rgb[k].a;

                    col = model->PolyRGB[j].rgb[k].r * (inst->r + 128) / 128;
                    if (col > 255) col = 255;
                    inst->rgb[i][j].rgb[k].r = (unsigned char)col;

                    col = model->PolyRGB[j].rgb[k].g * (inst->g + 128) / 128;
                    if (col > 255) col = 255;
                    inst->rgb[i][j].rgb[k].g = (unsigned char)col;

                    col = model->PolyRGB[j].rgb[k].b * (inst->b + 128) / 128;
                    if (col > 255) col = 255;
                    inst->rgb[i][j].rgb[k].b = (unsigned char)col;
                }
            }
        }
    }
}
#endif

////////////////////////////////////////////
// set instance bounding boxes + VisiMask //
////////////////////////////////////////////

void SetInstanceBoundingBoxes(INSTANCE *inst)
{
    long j;
    MODEL *model;
    VEC vec;

// get model
#ifdef _PC
    model = InstanceModels[inst->Model].Models;
#elif defined _N64
    model = &InstanceModel[inst->Model].Model;
#endif

// transform all model verts to find bounding box

    inst->Box.Xmin = inst->Box.Ymin = inst->Box.Zmin = 999999;
    inst->Box.Xmax = inst->Box.Ymax = inst->Box.Zmax = -999999;

#ifdef _PC
    for (j = 0 ; j < model->VertNum ; j++)
    {
        RotTransVector(&inst->WorldMatrix, &inst->WorldPos, (VEC*)&model->VertPtr[j].x, &vec);
#elif defined _N64
    for (j = 0 ; j < model->hdr->vtxnum ; j++)
    {
        VEC tmpv;
        tmpv.v[0] = model->hdr->vtxptr[j].v.ob[0];
        tmpv.v[1] = model->hdr->vtxptr[j].v.ob[1];
        tmpv.v[2] = model->hdr->vtxptr[j].v.ob[2];
        RotTransVector(&inst->WorldMatrix, &inst->WorldPos, &tmpv, &vec);
#endif

        if (vec.v[X] < inst->Box.Xmin) inst->Box.Xmin = vec.v[X];
        if (vec.v[X] > inst->Box.Xmax) inst->Box.Xmax = vec.v[X];
        if (vec.v[Y] < inst->Box.Ymin) inst->Box.Ymin = vec.v[Y];
        if (vec.v[Y] > inst->Box.Ymax) inst->Box.Ymax = vec.v[Y];
        if (vec.v[Z] < inst->Box.Zmin) inst->Box.Zmin = vec.v[Z];
        if (vec.v[Z] > inst->Box.Zmax) inst->Box.Zmax = vec.v[Z];
    }

// set visi mask

    inst->VisiMask = SetObjectVisiMask(&inst->Box);
}




/////////////////////////////////////////////////////////////////////
//
// BuildInstanceCollPolys:
//
/////////////////////////////////////////////////////////////////////

void BuildInstanceCollPolys()
{
    int         iInst, iPoly, nInstCollPolys;
    INSTANCE    *instance;
#ifdef _PC
    INSTANCE_MODELS *instModel;
#else
    INSTANCE_MODEL  *instModel;
#endif
    NEWCOLLPOLY *worldPoly, *instModelPoly;

    COL_NInstanceCollPolys = 0;
    // Count number of collision polys needed for instances
    for (iInst = 0; iInst < InstanceNum; iInst++) {
        instance = &Instances[iInst];

        if (!instance->Priority && !RenderSettings.Instance)
            continue;   // skip if turned off

#ifdef _PC
        COL_NInstanceCollPolys += InstanceModels[instance->Model].NCollPolys;
#else
        COL_NInstanceCollPolys += InstanceModel[instance->Model].NCollPolys;
#endif

    }

    // Allocate space for the instance polys
    if (COL_NInstanceCollPolys > 0) {
        COL_InstanceCollPoly = CreateCollPolys(COL_NInstanceCollPolys);
    } else {
        COL_InstanceCollPoly = NULL;
    }

    // Transform models collision polys into world coords and store in array
    nInstCollPolys = 0;
    for (iInst = 0; iInst < InstanceNum; iInst++) {
        instance = &Instances[iInst];

        if (!instance->Priority && !RenderSettings.Instance)
            continue;   // skip if turned off

#ifdef _PC
        instModel = &InstanceModels[instance->Model];
#else
        instModel = &InstanceModel[instance->Model];
#endif

        // Store pointer for the world collision polys for this instance
        instance->NCollPolys = instModel->NCollPolys;
        if (instModel->NCollPolys > 0) {
            instance->CollPoly = &COL_InstanceCollPoly[nInstCollPolys];
        } else {
            instance->CollPoly = NULL;
        }


        // Do the actual copying and transformation
        for (iPoly = 0; iPoly < instModel->NCollPolys; iPoly++) {
            worldPoly = &COL_InstanceCollPoly[nInstCollPolys++];
            instModelPoly = &instModel->CollPoly[iPoly];

            worldPoly->Type = instModelPoly->Type;
            worldPoly->Material = instModelPoly->Material;
            RotTransPlane(&instModelPoly->Plane, &instance->WorldMatrix, &instance->WorldPos, &worldPoly->Plane);
            RotTransPlane(&instModelPoly->EdgePlane[0], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[0]);
            RotTransPlane(&instModelPoly->EdgePlane[1], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[1]);
            RotTransPlane(&instModelPoly->EdgePlane[2], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[2]);
            if (IsPolyQuad(instModelPoly)) {
                RotTransPlane(&instModelPoly->EdgePlane[3], &instance->WorldMatrix, &instance->WorldPos, &worldPoly->EdgePlane[3]);
            }
////$CPRINCE_REMOVAL -- I don't think we actually need this.  (It was added as a precaution, before we got the updated code from Acclaim.)
//
// //$ADDITION(FrankSav)
//            else
//            {
//                worldPoly->EdgePlane[3].v[0] =
//                worldPoly->EdgePlane[3].v[1] =
//                worldPoly->EdgePlane[3].v[2] =
//                worldPoly->EdgePlane[3].v[3] = 0.0f;
//            }
// //$END_ADDITION

            RotTransBBox(&instModelPoly->BBox, &instance->WorldMatrix, &instance->WorldPos, &worldPoly->BBox);

            worldPoly->Type |= (instance->Flag & INSTANCE_NO_OBJECT_COLLISION ? CAMERA_ONLY : 0);
            worldPoly->Type |= (instance->Flag & INSTANCE_NO_CAMERA_COLLISION ? OBJECT_ONLY : 0);
        }

    }

#ifdef _N64
    for (iInst = 0; iInst < InstanceNum; iInst++) {
        instance = &Instances[iInst];
        if (instModel->NCollPolys)
            {
            DestroyCollPolys(instModel->CollPoly);
            instModel->NCollPolys = 0;
            instModel->CollPoly = NULL;
            }           
        }
#endif

}
