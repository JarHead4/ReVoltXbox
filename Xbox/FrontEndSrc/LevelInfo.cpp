//-----------------------------------------------------------------------------
// File: LevelInfo.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "LevelInfo.h"
#include "LevelLoad.h"
#include "geom.h"
#include "settings.h"
#include "main.h"
#include "timing.h"
#include "TitleScreen.h"
#include "Cheats.h"

//#define FOCUS_TEST

#ifdef _PC
static char LevelFilenameBuffer[MAX_PATH];
extern long ShadowRgb;
#endif

////////////////////////////////////////////////////////////////
//
// Global variables
//

LEVEL_SECRETS LevelSecrets;
STAR_LIST StarList;
CURRENT_LEVELINFO CurrentLevelInfo;                         // Info only needed when a level is chosen
LEVELINFO   *UserLevelInfo = NULL;                          // Level info for user-created levels

/////////////////////////
// race shipped levels //
/////////////////////////

LEVELINFO   DefaultLevelInfo[LEVEL_NSHIPPED_LEVELS] = {     // Level info for the levels shipped with the game
    {   // Neighbourhood 1
        "NHood1",
        "Toys in the Hood 1",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 54, 0),
        MAKE_TIME(0, 54, 0),
    },
    {   // Market 2
        "Market2",
        "SuperMarket 2",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 24, 0),
        MAKE_TIME(0, 24, 0),
    },
    {   // Museum 2
        "Muse2",
        "Museum 2",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 45, 0),
        MAKE_TIME(0, 45, 0),
    },
    {   // Garden 1
        "Garden1",
        "Botanical Garden",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 29, 0),
        MAKE_TIME(0, 29, 0),
    },
    {   // Toy World 1
        "ToyLite", 
        "Toy World 1", 
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_SILVER, 
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 27, 0),
        MAKE_TIME(0, 34, 0),
    },
    {   // Ghost 1
        "Wild_West1",
        "Ghost Town 1",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_SILVER,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 30, 0),
        MAKE_TIME(0, 30, 0),
    },
    {   // Toy World 2
        "Toy2",
        "Toy World 2",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_SILVER,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 37, 0),
        MAKE_TIME(0, 37, 0),
    },
    {   // Neighbourhood 2
        "NHood2",
        "Toys in the Hood 2",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_GOLD,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 45, 0),
        MAKE_TIME(0, 48, 0),
    },
    {   // Cruise 1
        "Ship1",
        "Toytanic 1",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_GOLD,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 55, 0),
        MAKE_TIME(0, 55, 0),
    },
    {   // Museum 1
        "Muse1", 
        "Museum 1", 
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_GOLD, 
        TRACK_TYPE_RACE, 
        MAKE_TIME(1, 01, 0),
        MAKE_TIME(0, 59, 0),
    },
    {   // Market 1
        "Market1",
        "Supermarket 1",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_SPECIAL,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 39, 0),
        MAKE_TIME(0, 39, 0),
    },
    {   // Ghost 2
        "Wild_West2",
        "Ghost Town 2",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_SPECIAL,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 38, 0),
        MAKE_TIME(0, 38, 0),
    },
    {   // Cruise 2
        "Ship2",
        "Toytanic 2",
        Real(0),
        LEVEL_SELECTABLE, 
        RACE_CLASS_SPECIAL,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 52, 0),
        MAKE_TIME(0, 52, 0),
    },

/////////////////////////
// misc shipped levels //
/////////////////////////

    {   // Frontend
        "frontend",
        "FrontEnd",
        Real(0),
        0, 
        RACE_CLASS_NONE,
        TRACK_TYPE_NONE, 
    },
    {   // Intro
        "Intro",
        "Intro Sequence",
        Real(0),
        0, 
        RACE_CLASS_NONE,
        TRACK_TYPE_NONE, 
    },

/////////////////
// stunt arena //
/////////////////

    {
        "stunts",
        "Stunt Arena",
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_NONE,
        TRACK_TYPE_TRAINING,
    },

///////////////////////////
// battle shipped levels //
///////////////////////////

    {   // hood battle
        "Nhood1_Battle",
        "Neighborhood Battle",
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_NONE,
        TRACK_TYPE_BATTLE,
    },
    {   // garden battle
        "Bot_Bat",
        "Garden Battle",
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_NONE,
        TRACK_TYPE_BATTLE,
    },
    {   // market battle
        "MarkAr",
        "Supermarket Battle",
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_NONE,
        TRACK_TYPE_BATTLE,
    },
    {   // museum battle
        "Muse_Bat",
        "Museum Battle",
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_NONE,
        TRACK_TYPE_BATTLE,
    },
};



////////////////////////////////////////////////////////////////
//
// GetLevelInfo: return pointer to level information for the 
// passed level number
//
////////////////////////////////////////////////////////////////

LEVELINFO *GetLevelInfo(int levelNum)
{
    // Default Level?
    if (levelNum < LEVEL_NSHIPPED_LEVELS) {
        return &DefaultLevelInfo[levelNum];
    }

    // User Level?
    if (levelNum < GameSettings.LevelNum) {
        return &UserLevelInfo[levelNum - LEVEL_NSHIPPED_LEVELS];
    }

    // Out of bounds
    return NULL;
}


////////////////////////////////////////////////////////////////
//
// InitDefaultLevels: Set the accesibility of the shipped levels
//
////////////////////////////////////////////////////////////////

void InitDefaultLevels()
{
    long i, j;

// allow the lot if dev or 'AllTracks' true

    if (Version == VERSION_DEV || AllTracks)
    {
        for (i = 0 ; i < GameSettings.LevelNum ; i++)
        {
            SetLevelSelectable(i);
            SetLevelAvailable(i);
            SetLevelReverseAvailable(i);
            SetLevelMirrorAvailable(i);
            SetLevelMirrorReverseAvailable(i);
        }
        return;
    }

// set all cup levels to selectable only

    for (i = 0 ; i < LEVEL_NCUP_LEVELS ; i++)
    {
        DefaultLevelInfo[i].ObtainFlags = 0;
        SetLevelSelectable(i);
    }

// loop thru each cup

    for (i = RACE_CLASS_BRONZE ; i < RACE_CLASS_NTYPES ; i++)
    {

// allow cup tracks?

        if (IsCupCompleted(i - 1))
        {
//          for (j = 0 ; j < CupData[i].NRaces ; j++)
//          {
//              SetLevelAvailable(CupData[i].Level[j].Num);
//          }
            for (j = 0 ; j < LEVEL_NCUP_LEVELS ; j++) if (GetLevelInfo(j)->LevelClass == i)
            {
                SetLevelAvailable(j);
            }
        }

// allow reversed?

        if (IsCupBeatTimeTrials(i))
        {
//          for (j = 0 ; j < CupData[i].NRaces ; j++)
//          {
//              SetLevelReverseAvailable(CupData[i].Level[j].Num);
//          }
            for (j = 0 ; j < LEVEL_NCUP_LEVELS ; j++) if (GetLevelInfo(j)->LevelClass == i)
            {
                SetLevelReverseAvailable(j);
            }
        }

// allow mirrored?

        if (IsCupBeatTimeTrialsReversed(i))
        {
//          for (j = 0 ; j < CupData[i].NRaces ; j++)
//          {
//              SetLevelMirrorAvailable(CupData[i].Level[j].Num);
//          }
            for (j = 0 ; j < LEVEL_NCUP_LEVELS ; j++) if (GetLevelInfo(j)->LevelClass == i)
            {
                SetLevelMirrorAvailable(j);
            }
        }

// allow reversed mirrored?

        if (IsCupBeatTimeTrialsMirrored(i))
        {
//          for (j = 0 ; j < CupData[i].NRaces ; j++)
//          {
//              SetLevelMirrorReverseAvailable(CupData[i].Level[j].Num);
//          }
            for (j = 0 ; j < LEVEL_NCUP_LEVELS ; j++) if (GetLevelInfo(j)->LevelClass == i)
            {
                SetLevelMirrorReverseAvailable(j);
            }
        }
    }
}

////////////////////////////////////////////////////////////////
//
// IsUserLevel: return true if the track is not a shipped level
//
////////////////////////////////////////////////////////////////
#ifdef _PC
bool IsUserLevel(char *dir)
{
    int iLevel;

    for (iLevel = 0; iLevel < LEVEL_NSHIPPED_LEVELS; iLevel++) {

        if (_stricmp(dir, DefaultLevelInfo[iLevel].Dir) == 0) {  //$MODIFIED: changed stricmp to _stricmp
            return FALSE;
        }
    }

    return TRUE;
}
#endif


//////////////////////////////////////////////////
//
// find all user generated levels 
//
//////////////////////////////////////////////////
#ifdef _PC
void FindUserLevels(void)
{
    long i, j;
    int ch;
    WIN32_FIND_DATA data;
    HANDLE handle;
    FILE *fp;
    LEVELINFO templevel, *levelInfo, *levelInfo2;
    char buf[256];
    char string[256];
    char names[MAX_LEVELS][MAX_LEVEL_DIR_NAME];

// add default levels to names array
    for (i = 0; i < LEVEL_NSHIPPED_LEVELS; i++) {
        strncpy(names[i], DefaultLevelInfo[i].Dir, MAX_LEVEL_DIR_NAME);
    }
    GameSettings.LevelNum = LEVEL_NSHIPPED_LEVELS;

// get first directory

//$MODIFIED
//    handle = FindFirstFile("levels\\*.*", &data);
    handle = FindFirstFile("D:\\levels\\*.*", &data);
//$END_MODIFICATIONS
    if (handle == INVALID_HANDLE_VALUE)
    {
        DumpMessage("ERROR", "Can't find any level directories");
        g_bQuitGame = TRUE;
        return;
    }

// loop thru each subsequent directory

    while (TRUE)
    {
        if (!FindNextFile(handle, &data))
            break;

        if (GameSettings.LevelNum == MAX_LEVELS) break;

// skip if not a good dir

        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (!strcmp(data.cFileName, "."))
            continue;

        if (!strcmp(data.cFileName, ".."))
            continue;
    
// skip if directory is one of the default levels

        if (!IsUserLevel(data.cFileName))
            continue;

// skip if user track?

        if (NoUser && !_strnicmp(data.cFileName, "user", 4)) //$MODIFIED: changed strnicmp to _strnicmp
            continue;

// add to dir list if valid info file in this dir

//$MODIFIED
//        wsprintf(buf, "levels\\%s\\%s.inf", data.cFileName, data.cFileName);
        wsprintf(buf, "D:\\levels\\%s\\%s.inf", data.cFileName, data.cFileName);
//$END_MODIFICATIONS
        fp = fopen(buf, "rb");
        if (fp == NULL) continue;

        for (i = 0 ; i < (long)strlen(data.cFileName) ; i++) data.cFileName[i] = toupper(data.cFileName[i]);
        memcpy(names[GameSettings.LevelNum], data.cFileName, MAX_LEVEL_DIR_NAME);
        GameSettings.LevelNum++;
        fclose(fp);
    }

// close search handle

    FindClose(handle);

// alloc LEVELINFO structure

    if (UserLevelInfo != NULL) free(UserLevelInfo);
    UserLevelInfo = (LEVELINFO*)malloc(sizeof(LEVELINFO) * (GameSettings.LevelNum - LEVEL_NSHIPPED_LEVELS));
    if (!UserLevelInfo)
    {
        DumpMessage("ERROR", "Can't alloc memory for user level info");
        GameSettings.LevelNum = LEVEL_NSHIPPED_LEVELS;
    }

// fill in LEVELINFO structure for levels

    for (i = 0 ; i < GameSettings.LevelNum ; i++)
    {
        levelInfo = GetLevelInfo(i);

// set dir name

        memcpy(levelInfo->Dir, names[i], MAX_LEVEL_DIR_NAME);

// Get Track Length

//$MODIFIED
//        sprintf(buf, "levels\\%s\\%s.pan", names[i], names[i]);
        sprintf(buf, "D:\\levels\\%s\\%s.pan", names[i], names[i]);
//$END_MODIFICATIONS
        fp = fopen(buf, "rb");
        if (fp == NULL) {
            levelInfo->Length = ZERO;
        } else {
            long dummyLong;
            fread(&dummyLong, sizeof(long), 1, fp);
            if (!dummyLong) {
                levelInfo->Length = ZERO;
            } else {
                // load start node
                fread(&dummyLong, sizeof(long), 1, fp);

                // load total dist
                fread(&levelInfo->Length, sizeof(REAL), 1, fp);
                levelInfo->Length /= 200;
            }
            fclose(fp);
        }


// set defaults
        if (IsUserLevel(levelInfo->Dir)) {
            sprintf(levelInfo->Name, "Untitled");
            levelInfo->TrackType = TRACK_TYPE_USER;
            levelInfo->LevelClass = RACE_CLASS_NONE;
            SetLevelSelectable(i);
            SetLevelAvailable(i);
            SetLevelMirrorAvailable(i);
        } else {
            continue;
        }
        
// open .INF file

//$MODIFIED
//        wsprintf(buf, "levels\\%s\\%s.inf", names[i], names[i]);
        wsprintf(buf, "D:\\levels\\%s\\%s.inf", names[i], names[i]);
//$END_MODIFICATIONS
        fp = fopen(buf, "r");
        if (fp == NULL) continue;

// scan all strings

        while (TRUE)
        {

// get a string

            if (fscanf(fp, "%s", string) == EOF)
                break;

// comment?

            if (string[0] == ';')
            {
                do
                {
                    ch = fgetc(fp);
                } while (ch != '\n' && ch != EOF);
                continue;
            }

// name?

            if (!strcmp(string, "NAME"))
            {
                do
                {
                    ch = fgetc(fp);
                } while (ch != '\'' && ch != EOF);

                j = 0;
                while (TRUE)
                {
                    ch = fgetc(fp);
                    if (ch != '\'' && ch != EOF && j < MAX_LEVEL_NAME - 1)
                    {
                        levelInfo->Name[j] = (char)ch;
                        j++;
                    }
                    else
                    {
                        levelInfo->Name[j] = 0;
                        break;
                    }
                }
                continue;
            }

        }

// close .INF file

        fclose(fp);
    }

// sort user levels alphabetically

    for (i = GameSettings.LevelNum - 1 ; i ; i--) 
    {
        for (j = LEVEL_NSHIPPED_LEVELS ; j < i ; j++)
        {
            levelInfo = GetLevelInfo(i);
            levelInfo2 = GetLevelInfo(j + 1);

            if (strcmp(levelInfo->Name, levelInfo2->Name) > 0)
            {
                templevel = *levelInfo;
                *levelInfo = *levelInfo2;
                *levelInfo2 = templevel;
            }
        }
    }
}
#endif


////////////////////////////////////////////////////////////////
//
// Verify Level Exists
//
////////////////////////////////////////////////////////////////
#ifdef _PC
bool DoesLevelExist(long levelNum)
{
    FILE *fp;

    if (fp = fopen(GetAnyLevelFilename(levelNum, FALSE, "inf", FILENAME_MAKE_BODY), "rb")) {
        fclose(fp);
        return TRUE;
    }

    return FALSE;

}
#endif

////////////////////////////////////////////////////////////////
//
// Free RAM allocated to store user level info
//
////////////////////////////////////////////////////////////////

void FreeUserLevels(void)
{
    free(UserLevelInfo);
    UserLevelInfo = NULL;
}


////////////////////////////////////////////////////////////////
//
// LoadCurrentLevelInfo:
//
////////////////////////////////////////////////////////////////
#ifdef _PC
void LoadCurrentLevelInfo(char *dirName)
{
    int ch;
    long j;
    long r, g, b;
    FILE *fp;
    char buf[256];
    char string[256];

    // Set defaults
    strncpy(CurrentLevelInfo.Dir, dirName, MAX_LEVEL_DIR_NAME);
    wsprintf(CurrentLevelInfo.EnvStill, "D:\\gfx\\envstill.bmp"); //$MODIFIED: added "D:\\" at start
    wsprintf(CurrentLevelInfo.EnvRoll, "D:\\gfx\\envroll.bmp"); //$MODIFIED: added "D:\\" at start
    wsprintf(CurrentLevelInfo.Mp3, "");
    SetVector(&CurrentLevelInfo.NormalStartPos, 0, 0, 0);
    SetVector(&CurrentLevelInfo.ReverseStartPos, 0, 0, 0);
    CurrentLevelInfo.NormalStartRot = 0;
    CurrentLevelInfo.ReverseStartRot = 0;
    CurrentLevelInfo.NormalStartGrid = 0;
    CurrentLevelInfo.ReverseStartGrid = 0;
    CurrentLevelInfo.FarClip = 6144;
    CurrentLevelInfo.FogStart = 5120;
    CurrentLevelInfo.FogColor = 0x000000;
    CurrentLevelInfo.VertFogStart = 0;
    CurrentLevelInfo.VertFogEnd = 0;
    CurrentLevelInfo.WorldRGBper = 100;
    CurrentLevelInfo.ModelRGBper = 100;
    CurrentLevelInfo.InstanceRGBper = 100;
    CurrentLevelInfo.MirrorType = 0;
    CurrentLevelInfo.MirrorMix = 0.75f;
    CurrentLevelInfo.MirrorIntensity = 1;
    CurrentLevelInfo.MirrorDist = 256;
    CurrentLevelInfo.RedbookStartTrack = -1;
    CurrentLevelInfo.RedbookEndTrack = -1;
    CurrentLevelInfo.RockX = 0.0f;
    CurrentLevelInfo.RockZ = 0.0f;
    CurrentLevelInfo.RockTimeX = 0.0f;
    CurrentLevelInfo.RockTimeZ = 0.0f;
    ShadowRgb = -96;

    // open the file
//$MODIFIED
//    wsprintf(buf, "levels\\%s\\%s.inf", dirName, dirName);
    wsprintf(buf, "D:\\levels\\%s\\%s.inf", dirName, dirName);
//$END_MODIFICATIONS
    fp = fopen(buf, "r");
    if (fp == NULL) {
        DumpMessage(NULL, "Could not find level information file");
        g_bQuitGame = TRUE;
        return;
    }

    // load the info

    while (TRUE)
    {

        // get a string
        if (fscanf(fp, "%s", string) == EOF)
            break;

        // comment?
        if (string[0] == ';')
        {
            do
            {
                ch = fgetc(fp);
            } while (ch != '\n' && ch != EOF);
            continue;
        }

        if (!strcmp(string, "NAME"))
        {
            do
            {
                ch = fgetc(fp);
            } while (ch != '\'' && ch != EOF);

            j = 0;
            while (TRUE)
            {
                ch = fgetc(fp);
                if (ch != '\'' && ch != EOF && j < MAX_LEVEL_NAME - 1)
                {
                    CurrentLevelInfo.Name[j] = (char)ch;
                    j++;
                }
                else
                {
                    CurrentLevelInfo.Name[j] = 0;
                    break;
                }
            }
            continue;
        }

        // env still?
        if (!strcmp(string, "ENVSTILL"))
        {
            do
            {
                ch = fgetc(fp);
            } while (ch != '\'' && ch != EOF);

            j = 0;
            while (TRUE)
            {
                ch = fgetc(fp);
                if (ch != '\'' && ch != EOF && j < MAX_ENV_NAME - 1)
                {
                    CurrentLevelInfo.EnvStill[j] = (char)ch;
                    j++;
                }
                else
                {
                    CurrentLevelInfo.EnvStill[j] = 0;
                    break;
                }
            }
            continue;
        }

        // env roll?
        if (!strcmp(string, "ENVROLL"))
        {
            do
            {
                ch = fgetc(fp);
            } while (ch != '\'' && ch != EOF);

            j = 0;
            while (TRUE)
            {
                ch = fgetc(fp);
                if (ch != '\'' && ch != EOF && j < MAX_ENV_NAME - 1)
                {
                    CurrentLevelInfo.EnvRoll[j] = (char)ch;
                    j++;
                }
                else
                {
                    CurrentLevelInfo.EnvRoll[j] = 0;
                    break;
                }
            }
            continue;
        }

        // mp3?
        if (!strcmp(string, "MP3"))
        {
            do
            {
                ch = fgetc(fp);
            } while (ch != '\'' && ch != EOF);

            j = 0;
            while (TRUE)
            {
                ch = fgetc(fp);
                if (ch != '\'' && ch != EOF && j < MAX_MP3_NAME - 1)
                {
                    CurrentLevelInfo.Mp3[j] = (char)ch;
                    j++;
                }
                else
                {
                    CurrentLevelInfo.Mp3[j] = 0;
                    break;
                }
            }
            continue;
        }

        // start pos
        if (!strcmp(string, "STARTPOS"))
        {
            fscanf(fp, "%f %f %f", 
                &CurrentLevelInfo.NormalStartPos.v[X], 
                &CurrentLevelInfo.NormalStartPos.v[Y], 
                &CurrentLevelInfo.NormalStartPos.v[Z]);
            continue;
        }

        // reverse start pos
        if (!strcmp(string, "STARTPOSREV"))
        {
            fscanf(fp, "%f %f %f", 
                &CurrentLevelInfo.ReverseStartPos.v[X], 
                &CurrentLevelInfo.ReverseStartPos.v[Y], 
                &CurrentLevelInfo.ReverseStartPos.v[Z]);
            continue;
        }

        // start rot
        if (!strcmp(string, "STARTROT"))
        {
            fscanf(fp, "%f", &CurrentLevelInfo.NormalStartRot);
            continue;
        }

        // reverse start rot
        if (!strcmp(string, "STARTROTREV"))
        {
            fscanf(fp, "%f", &CurrentLevelInfo.ReverseStartRot);
            continue;
        }

        // start grid
        if (!strcmp(string, "STARTGRID"))
        {
            fscanf(fp, "%d", &CurrentLevelInfo.NormalStartGrid);
            continue;
        }

        // reverse start grid
        if (!strcmp(string, "STARTGRIDREV"))
        {
            fscanf(fp, "%d", &CurrentLevelInfo.ReverseStartGrid);
            continue;
        }

        // far clip
        if (!strcmp(string, "FARCLIP"))
        {
            fscanf(fp, "%f", &CurrentLevelInfo.FarClip);
            continue;
        }

        // fog start
        if (!strcmp(string, "FOGSTART"))
        {
            fscanf(fp, "%f", &CurrentLevelInfo.FogStart);
            continue;
        }

        // fog color
        if (!strcmp(string, "FOGCOLOR"))
        {
            fscanf(fp, "%ld %ld %ld", &r, &g, &b);
            CurrentLevelInfo.FogColor = (r << 16) | (g << 8) | b;
            continue;
        }

        // vert fog start
        if (!strcmp(string, "VERTFOGSTART"))
        {
            fscanf(fp, "%f", &CurrentLevelInfo.VertFogStart);
            continue;
        }

        // vert fog end
        if (!strcmp(string, "VERTFOGEND"))
        {
            fscanf(fp, "%f", &CurrentLevelInfo.VertFogEnd);
            continue;
        }

        // world rgb percent
        if (!strcmp(string, "WORLDRGBPER"))
        {
            fscanf(fp, "%ld", &CurrentLevelInfo.WorldRGBper);
            continue;
        }

        // model rgb percent
        if (!strcmp(string, "MODELRGBPER"))
        {
            fscanf(fp, "%ld", &CurrentLevelInfo.ModelRGBper);
            continue;
        }

        // instance rgb percent
        if (!strcmp(string, "INSTANCERGBPER"))
        {
            fscanf(fp, "%ld", &CurrentLevelInfo.InstanceRGBper);
            continue;
        }

        // mirror info
        if (!strcmp(string, "MIRRORS"))
        {
            fscanf(fp, "%ld %f %f %f", 
                &CurrentLevelInfo.MirrorType, 
                &CurrentLevelInfo.MirrorMix, 
                &CurrentLevelInfo.MirrorIntensity, 
                &CurrentLevelInfo.MirrorDist);
            continue;
        }

        // redbook track
        if (!strcmp(string, "REDBOOK"))
        {
            fscanf(fp, "%ld %ld", &CurrentLevelInfo.RedbookStartTrack, &CurrentLevelInfo.RedbookEndTrack);
            continue;
        }

        // rocky ship shit
        if (!strcmp(string, "ROCK"))
        {
            fscanf(fp, "%f %f %f %f", 
                &CurrentLevelInfo.RockX, 
                &CurrentLevelInfo.RockZ, 
                &CurrentLevelInfo.RockTimeX, 
                &CurrentLevelInfo.RockTimeZ);
            continue;
        }

        // shadow box subtraction
        if (!strcmp(string, "SHADOWBOX"))
        {
            fscanf(fp, "%ld", &ShadowRgb);
            continue;
        }
    }

    fclose(fp);
}
#endif      

//////////////////////////////////////////////////////////////
// set level name - return corresponging level number or -1 //
//////////////////////////////////////////////////////////////
#ifdef _PC
long GetLevelNum(char *dir)
{
    long i;
    LEVELINFO *levelInfo;

// look for level

    for (i = 0 ; i < GameSettings.LevelNum ; i++)
    {
        levelInfo = GetLevelInfo(i);
        if (!_stricmp(levelInfo->Dir, dir))  //$MODIFIED: changed stricmp to _stricmp
        {
            return i;
        }
    }

// not found!

    return -1;
}
#endif



#ifdef _PC
////////////////////////////////////////////////////////////////
//
// Get a filename for the current level as set in GamsSettings
//
////////////////////////////////////////////////////////////////

char *GetLevelFilename(char *filename, long flag)
{
    return GetAnyLevelFilename(GameSettings.Level, (flag & FILENAME_GAME_SETTINGS) && GameSettings.Reversed, filename, flag);
}


////////////////////////////////////////////////////////////////
//
// Get a filename associated with the passed level
//
////////////////////////////////////////////////////////////////

char *GetAnyLevelFilename(long iLevel, long reversed, char *filename, long flag)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);
    
// make body

    if (flag & FILENAME_MAKE_BODY)
    {
        if (reversed)
            wsprintf(LevelFilenameBuffer, "D:\\levels\\%s\\reversed\\%s.%s", levelInfo->Dir, levelInfo->Dir, filename); //$MODIFIED: added "D:\\" at start
        else
            wsprintf(LevelFilenameBuffer, "D:\\levels\\%s\\%s.%s", levelInfo->Dir, levelInfo->Dir, filename); //$MODIFIED: added "D:\\" at start
    }

// don't make body

    else
    {
        if (reversed)
            wsprintf(LevelFilenameBuffer, "D:\\levels\\%s\\reversed\\%s", levelInfo->Dir, filename); //$MODIFIED: added "D:\\" at start
        else
            wsprintf(LevelFilenameBuffer, "D:\\levels\\%s\\%s", levelInfo->Dir, filename); //$MODIFIED: added "D:\\" at start
    }

// return filename

    return LevelFilenameBuffer;
}
#endif

////////////////////////////////////////////////////////////////
//
// Get ghost filename
//
////////////////////////////////////////////////////////////////

static char *GhostFilenameDir[] = {
    "Normal",
    "Reverse",
    "Mirror",
    "ReverseMirror",
};

char *GetGhostFilename(long level, long reversed, long mirrored, char *ext)
{
    LEVELINFO *li;

// get level info

    li = GetLevelInfo(level);
    if (!li)
    {
        return NULL;
    }

// make filename

    sprintf(LevelFilenameBuffer, "D:\\times\\%s\\%s.%s", //$MODIFIED: added "D:\\" at start
        GhostFilenameDir[(reversed ? 1 : 0) | (mirrored ? 2 : 0)],
        li->Name,
        ext);

// return buf

    return LevelFilenameBuffer;
}

////////////////////////////////////////////////////////////////
//
// Accessibility
//
////////////////////////////////////////////////////////////////

bool IsLevelSelectable(int iLevel) 
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

// selectable?

    if (levelInfo && levelInfo->ObtainFlags & LEVEL_SELECTABLE)
    {

// allow all tracks for all game types if dev version

        if (Version == VERSION_DEV)
            return TRUE;

// yep, track type OK for this game mode?

        switch (GameSettings.GameType)
        {
            case GAMETYPE_TRIAL:
            case GAMETYPE_PRACTICE:
            case GAMETYPE_MULTI:

                if (levelInfo->TrackType == TRACK_TYPE_RACE)
                    return TRUE;

                break;

            case GAMETYPE_SINGLE:
            case GAMETYPE_CLOCKWORK:

                if (levelInfo->TrackType == TRACK_TYPE_RACE || levelInfo->TrackType == TRACK_TYPE_USER)
                    return TRUE;

                break;

            case GAMETYPE_BATTLE:

                if (levelInfo->TrackType == TRACK_TYPE_BATTLE)
                    return TRUE;

                break;
        }
    }

// nope, sorry

    return FALSE;
}

bool IsLevelAvailable(int iLevel) 
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        return (levelInfo->ObtainFlags & LEVEL_AVAILABLE) != 0;
    }

    return FALSE;
}

bool IsLevelReverseAvailable(int iLevel) 
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        return ((levelInfo->ObtainFlags & LEVEL_AVAILABLE) && (levelInfo->ObtainFlags & LEVEL_REVERSE_AVAILABLE));
    }

    return FALSE;
}

bool IsLevelMirrorAvailable(int iLevel) 
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        return ((levelInfo->ObtainFlags & LEVEL_AVAILABLE) && (levelInfo->ObtainFlags & LEVEL_MIRROR_AVAILABLE));
    }

    return FALSE;
}

bool IsLevelMirrorReverseAvailable(int iLevel) 
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        return ((levelInfo->ObtainFlags & LEVEL_AVAILABLE) && 
            (levelInfo->ObtainFlags & LEVEL_MIRROR_AVAILABLE) && 
            (levelInfo->ObtainFlags & LEVEL_MIRROR_REVERSE_AVAILABLE));
    }

    return FALSE;
}

bool IsLevelTypeAvailable(int iLevel, bool mirror, bool reverse)
{
    long flag;
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    flag = LEVEL_AVAILABLE;
    if (mirror && reverse) flag |= LEVEL_MIRROR_REVERSE_AVAILABLE;
    else if (mirror) flag |= LEVEL_MIRROR_AVAILABLE;
    else if (reverse) flag |= LEVEL_REVERSE_AVAILABLE;

    if (levelInfo != NULL) {
        return ((levelInfo->ObtainFlags & flag) == flag);
    }

    return FALSE;
}

void SetLevelSelectable(int iLevel)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        levelInfo->ObtainFlags |= LEVEL_SELECTABLE;
    }
}

void SetLevelAvailable(int iLevel)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        levelInfo->ObtainFlags |= LEVEL_AVAILABLE;
    }
}

void SetLevelReverseAvailable(int iLevel)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        levelInfo->ObtainFlags |= LEVEL_REVERSE_AVAILABLE;
    }
}

void SetLevelMirrorAvailable(int iLevel)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        levelInfo->ObtainFlags |= LEVEL_MIRROR_AVAILABLE;
    }
}

void SetLevelMirrorReverseAvailable(int iLevel)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        levelInfo->ObtainFlags |= LEVEL_MIRROR_REVERSE_AVAILABLE;
    }
}

void SetLevelUnavailable(int iLevel)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        levelInfo->ObtainFlags &= ~(LEVEL_AVAILABLE | LEVEL_REVERSE_AVAILABLE | LEVEL_MIRROR_AVAILABLE | LEVEL_MIRROR_REVERSE_AVAILABLE);
    }
}
