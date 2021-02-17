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
#include "geom.h"
#include "camera.h"
#include "player.h"
#include "cheats.h"
#include "ui_TitleScreen.h"
#include "content.h"

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

// $MD: added szName, changed selectability of some levels to conform with spec
// We can only ship 8 default levels.  4 are selectable only online.

LEVELINFO   DefaultLevelInfo[LEVEL_NSHIPPED_LEVELS] = {     // Level info for the levels shipped with the game
    {   // Neighbourhood 1
        "NHood1",
        "D:\\levels\\NHood1",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 54, 0),
        MAKE_TIME(0, 54, 0),
    },
    {   // Market 2
        "Market2",
        "D:\\levels\\Market2",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 24, 0),
        MAKE_TIME(0, 24, 0),
    },
    {   // Museum 2
        "Muse2",
        "D:\\levels\\Muse2",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 45, 0),
        MAKE_TIME(0, 45, 0),
    },
    {   // Garden 1
        "Garden1",
        "D:\\levels\\Garden1",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_BRONZE,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 29, 0),
        MAKE_TIME(0, 29, 0),
    },
    {   // Toy World 1
        "ToyLite",
        "D:\\levels\\ToyLite",    
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_SELECTABLE | LEVEL_AVAILABLE, 
        RACE_CLASS_SILVER, 
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 27, 0),
        MAKE_TIME(0, 34, 0),
    },
    {   // Ghost 1
        "Wild_West1",
        "D:\\levels\\Wild_West1",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_SILVER,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 30, 0),
        MAKE_TIME(0, 30, 0),
    },
    {   // Toy World 2
        "Toy2",
        "D:\\levels\\Toy2",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_SILVER,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 37, 0),
        MAKE_TIME(0, 37, 0),
    },
    {   // Neighbourhood 2
        "NHood2",
        "D:\\levels\\NHood2",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_ONLINEONLY | LEVEL_AVAILABLE, 
        RACE_CLASS_GOLD,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 45, 0),
        MAKE_TIME(0, 48, 0),
    },
    {   // Cruise 1
        "Ship1",
        "D:\\levels\\Ship1",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_ONLINEONLY | LEVEL_AVAILABLE, 
        RACE_CLASS_GOLD,
        TRACK_TYPE_RACE, 
        MAKE_TIME(0, 55, 0),
        MAKE_TIME(0, 55, 0),
    },
    {   // Museum 1
        "Muse1",
        "D:\\levels\\Muse1", 
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_GOLD, 
        TRACK_TYPE_RACE, 
        MAKE_TIME(1, 01, 0),
        MAKE_TIME(0, 59, 0),
    },
    {   // Market 1
        "Market1",
        "D:\\levels\\Market1",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_ONLINEONLY | LEVEL_AVAILABLE, 
        RACE_CLASS_SPECIAL,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 39, 0),
        MAKE_TIME(0, 39, 0),
    },
    {   // Ghost 2
        "Wild_West2",
        "D:\\levels\\Wild_West2",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_SPECIAL,
        TRACK_TYPE_RACE,
        MAKE_TIME(0, 38, 0),
        MAKE_TIME(0, 38, 0),
    },
    {   // Cruise 2
        "Ship2",
        "D:\\levels\\Ship2",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        LEVEL_ONLINEONLY | LEVEL_AVAILABLE, 
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
        "D:\\levels\\frontend",
        L"FrontEnd",
        Real(0),
        0, 
        RACE_CLASS_NONE,
        TRACK_TYPE_NONE, 
    },
    {   // Intro
        "Intro",
        "D:\\levels\\Intro",
        L"Intro Sequence",
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
        "D:\\levels\\stunts",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_NONE,
        TRACK_TYPE_TRAINING,
    },

///////////////////////////
// battle shipped levels //
///////////////////////////

    {   // hood battle
        "Nhood1_Battle",
        "D:\\levels\\Nhood1_Battle",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_NONE,
        TRACK_TYPE_BATTLE,
    },
    {   // garden battle
        "Bot_Bat",
        "D:\\levels\\Bot_Bat",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_NONE,
        TRACK_TYPE_BATTLE,
    },
    {   // market battle
        "MarkAr",
        "D:\\levels\\MarkAr",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
        RACE_CLASS_NONE,
        TRACK_TYPE_BATTLE,
    },
    {   // museum battle
        "Muse_Bat",
        "D:\\levels\\Muse_Bat",
        L"", // Note: Track name is filled-in from the track's .inf file
        Real(0),
        0, 
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
// $MD: We set the default levels in the level info structs
/*
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

// allow hood 1 if shareware

    if (Version == VERSION_SHAREWARE)
    {
        SetLevelAvailable(LEVEL_NEIGHBOURHOOD1);
        return;
    }

// allow bronze tracks if E3 demo

    if (Version == VERSION_E3)
    {
        for (i = 0 ; i < CupData[RACE_CLASS_BRONZE].NRaces ; i++)
        {
            SetLevelAvailable(CupData[RACE_CLASS_BRONZE].Level[i].Num);
        }
        return;
    }

// allow hood1 + muse2 if creative

    if (Version == VERSION_CREATIVE)
    {
        SetLevelAvailable(LEVEL_NEIGHBOURHOOD1);
        SetLevelAvailable(LEVEL_MUSEUM2);
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
*/

////////////////////////////////////////////////////////////////
//
// IsUserLevel: return true if the track is not a shipped level
//
////////////////////////////////////////////////////////////////
#ifdef _PC
bool IsUserLevel(const char *szDir)
{
    int iLevel;

    for (iLevel = 0; iLevel < LEVEL_NSHIPPED_LEVELS; iLevel++) {

        if (_stricmp(szDir, DefaultLevelInfo[iLevel].szDir) == 0) {  //$MODIFIED: changed stricmp to _stricmp
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
    //$MD: split into load defualt and load user!!
    //$MD: ditch the getuser level function and just store the dires here..
    long i, j;
    wint_t wch;
    WIN32_FIND_DATA data;
    HANDLE handle;
    FILE *fp;
    LEVELINFO *levelInfo;
    WCHAR buf[256];
    WCHAR string[256];
    std::string astrNames[MAX_LEVELS];
    std::string astrDirs[MAX_LEVELS];

    // add default levels to names array
    for (i = 0; i < LEVEL_NSHIPPED_LEVELS; i++) 
    {
        astrNames[i] =  DefaultLevelInfo[i].szName;
        astrDirs[i] = DefaultLevelInfo[i].szDir;
    }

    GameSettings.LevelNum = LEVEL_NSHIPPED_LEVELS;

    // get first directory
    handle = FindFirstFile("D:\\levels\\*.*", &data);
    if (handle == INVALID_HANDLE_VALUE)
    {
        DumpMessage("ERROR", "Can't find any level directories");
        QuitGame();
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

        //$MD these won't show up on XBOX
        /*
        if (!strcmp(data.cFileName, "."))
            continue;

        if (!strcmp(data.cFileName, ".."))
            continue;
        */
    
        // skip if directory is one of the default levels
        std::string strDir("D:\\levels\\");
        strDir += data.cFileName;
        if (!IsUserLevel(strDir.c_str()))
            continue;

        /*
        // $MD: NoUser == 0
        // skip if user track?
        if (NoUser && !_strnicmp(data.cFileName, "user", 4)) //$MODIFIED: changed strnicmp to _strnicmp
            continue;
        */

        // add to dir list if valid info file in this dir
        swprintf( buf, L"D:\\levels\\%S\\%S.inf", data.cFileName, data.cFileName );
        fp = _wfopen( buf, L"rb" );
        if (fp == NULL)
        {
#ifdef _DEBUG
            std::string Error("Could not find level inf: ");
            Error += data.cFileName;
            DumpMessage("Warning", Error.c_str());
#endif
            continue;
        }
        fclose(fp);

        // make name upper case
        for (i = 0 ; i < (long)strlen(data.cFileName) ; i++)
            data.cFileName[i] = (char)toupper(data.cFileName[i]);
        
        astrNames[GameSettings.LevelNum] = data.cFileName;
        astrDirs[GameSettings.LevelNum] = "D:\\levels\\";
        astrDirs[GameSettings.LevelNum] += data.cFileName;

        GameSettings.LevelNum++;
        
    }

    // close search handle
    FindClose(handle);
    handle = INVALID_HANDLE_VALUE;

    XCONTENT_FIND_DATA Finddata;

    ////////////////////////////////////////////////////////////
    // Load in the downloaded levels
    //
      
    // only look for levels
    handle = XFindFirstContent("T:\\", CONTENT_LEVEL_FLAG, &Finddata);
    if(handle != INVALID_HANDLE_VALUE)
    {
        do
        {
            g_ContentManager.AddOwnedContent(Finddata.qwOfferingId, Finddata.szContentDirectory, true);
            
            // don't add corrupt content
            if(g_ContentManager.IsOwnedContentCorrupted(Finddata.qwOfferingId))
                continue;

            // load content signnature
            HANDLE hSigs = XLoadContentSignatures( Finddata.szContentDirectory );
            assert(hSigs != INVALID_HANDLE_VALUE);

            // $MD and ignore them for now
            XCloseContentSignatures(hSigs);

            if (GameSettings.LevelNum == MAX_LEVELS)
            {
#ifdef _DEBUG
                std::string Error("More levels that Max levels found: ");
                Error += Finddata.szContentDirectory;
                DumpMessage("Warning", Error.c_str());
#endif
                break;
            }

            // find INF
            HANDLE hInf;
            std::string strBuf(Finddata.szContentDirectory);
            strBuf += "\\*.inf";
            hInf = FindFirstFile(strBuf.c_str(), &data);
            if(hInf == INVALID_HANDLE_VALUE)
            {
#ifdef _DEBUG
                std::string Error("Could not find level inf: ");
                Error += Finddata.szContentDirectory;
                DumpMessage("Warning", Error.c_str());
#endif
                continue;
            }
            FindClose(hInf);

            
            // make name upper case
            for (i = 0 ; i < (long)strlen(data.cFileName) ; i++)
                data.cFileName[i] = (char)toupper(data.cFileName[i]);

            // remove .inf
            data.cFileName[strlen(data.cFileName) - 4] = NULL;
        
            astrNames[GameSettings.LevelNum] = data.cFileName;
            astrDirs[GameSettings.LevelNum] = Finddata.szContentDirectory;
            GameSettings.LevelNum++;
        }
        while( XFindNextContent(handle, &Finddata) );

        XFindClose(handle);
    }

    
    // alloc user LEVELINFO structure
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

        // set name
        strncpy(levelInfo->szName, astrNames[i].c_str(), MAX_LEVEL_INF_NAME);
        levelInfo->szName[MAX_LEVEL_INF_NAME - 1] = NULL;
        
        // set dir
        strncpy(levelInfo->szDir, astrDirs[i].c_str(), MAX_PATH);
        levelInfo->szDir[MAX_PATH - 1] = NULL;


        // Get Track Length from the track's .pan file
        swprintf( buf, L"%S\\%S.pan", astrDirs[i].c_str(), astrNames[i].c_str());
        fp = _wfopen(buf, L"rb");
        if (fp == NULL) 
        {
            levelInfo->Length = 0.0f;
        } 
        else 
        {
            long dummyLong;
            fread(&dummyLong, sizeof(long), 1, fp);
            if (!dummyLong) 
            {
                levelInfo->Length = 0.0f;
            } 
            else 
            {
                // load start node
                fread(&dummyLong, sizeof(long), 1, fp);

                // load total dist
                fread(&levelInfo->Length, sizeof(REAL), 1, fp);
                levelInfo->Length /= 200;
            }
            fclose(fp);
        }

        // set defaults
        if( IsUserLevel(levelInfo->szDir) ) 
        {
            // we don't want user tracks in shareware version
            if (Version == VERSION_SHAREWARE)
                continue;
            
            swprintf( levelInfo->strName, L"Untitled" );
            levelInfo->TrackType = TRACK_TYPE_USER;
            levelInfo->LevelClass = RACE_CLASS_NONE;
            SetLevelSelectable(i);
            SetLevelAvailable(i);
            SetLevelMirrorAvailable(i);
        }
        
        // Get the localized track name from the track's .inf file
        swprintf(buf, L"%S\\%S.inf", astrDirs[i].c_str(), astrNames[i].c_str());
        fp = _wfopen(buf, L"rb");
        if (fp == NULL) continue;

        // Check for the unicode marker
        WORD wUnicodeMarker;
        fread( &wUnicodeMarker, sizeof(WORD), 1, fp );
        if( wUnicodeMarker != 0xfeff )
            continue;

        WCHAR* strNameToken = L"NAME_ENGLISH";
        switch( XGetLanguage() )
        {
            case XC_LANGUAGE_FRENCH:     strNameToken = L"NAME_FRENCH";     break;
            case XC_LANGUAGE_GERMAN:     strNameToken = L"NAME_GERMAN";     break;
            case XC_LANGUAGE_ITALIAN:    strNameToken = L"NAME_ITALIAN";    break;
            case XC_LANGUAGE_JAPANESE:   strNameToken = L"NAME_JAPANESE";   break;
            case XC_LANGUAGE_SPANISH:    strNameToken = L"NAME_SPANISH";    break;
            case XC_LANGUAGE_PORTUGUESE: strNameToken = L"NAME_PORTUGUESE"; break;
            case XC_LANGUAGE_KOREAN:     strNameToken = L"NAME_KOREAN";     break;
            case XC_LANGUAGE_TCHINESE:   strNameToken = L"NAME_CHINESE";    break;
        }
        
        // scan all strings
        while (TRUE)
        {
            // get a string
            if( fwscanf(fp, L"%s", string) <= 0 )
            {
                if( feof(fp) )
                    break;
                continue;
            }

            // comment?
            if (string[0] == L';')
            {
                do
                {
                    wch = fgetwc(fp);
                } while (wch != L'\n' && wch != WEOF);
                continue;
            }

            // name?
            if (!wcsncmp(string, L"NAME_", 5 ))
            {
                if (!wcscmp(string, strNameToken))
                {
                    do
                    {
                        wch = fgetwc(fp);
                    } while (wch != L'\'' && wch != WEOF);

                    j = 0;
                    while (TRUE)
                    {
                        wch = fgetwc(fp);
                        if (wch != L'\'' && wch != WEOF && j < MAX_LEVEL_NAME - 1)
                        {
                            levelInfo->strName[j] = (WCHAR)wch;
                            j++;
                        }
                        else
                        {
                            levelInfo->strName[j] = L'\0';
                            break;
                        }
                    }
                }
                else
                {
                    do
                    {
                        wch = fgetwc(fp);
                    } while (wch != L'\n' && wch != WEOF);
                }
                continue;
            }
        }

        // close .INF file
        fclose(fp);
    }



    //$MD: we wan't levels to be in the correct order
    /*
    LEVELINFO templevel, *levelInfo2;

    // sort user levels alphabetically
    for (i = GameSettings.LevelNum - 1 ; i ; i--) 
    {
        for (j = LEVEL_NSHIPPED_LEVELS ; j < i ; j++)
        {
            levelInfo = GetLevelInfo(i);
            levelInfo2 = GetLevelInfo(j + 1);

            if (wcscmp(levelInfo->strName, levelInfo2->strName) > 0)
            {
                templevel = *levelInfo;
                *levelInfo = *levelInfo2;
                *levelInfo2 = templevel;
            }
        }
    }
    */
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
void LoadCurrentLevelInfo(char *szName, char* szDir)
{
    wint_t wch;
    long j;
    long r, g, b;
    FILE *fp;
    WCHAR buf[256];
    WCHAR string[256];

    // Set defaults
    strncpy(CurrentLevelInfo.szName, szName, MAX_LEVEL_INF_NAME);
    strncpy(CurrentLevelInfo.szDir, szDir, MAX_PATH);
    
    sprintf(CurrentLevelInfo.EnvStill, "D:\\gfx\\envstill.bmp"); //$MODIFIED: added "D:\\" at start
    sprintf(CurrentLevelInfo.EnvRoll, "D:\\gfx\\envroll.bmp"); //$MODIFIED: added "D:\\" at start
    sprintf(CurrentLevelInfo.Mp3, "");
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
//    sprintf(buf, "levels\\%s\\%s.inf", dirName, dirName);
    swprintf(buf, L"%S\\%S.inf", szDir, szName);
//$END_MODIFICATIONS
    fp = _wfopen(buf, L"rb");
    if (fp == NULL) {
        DumpMessage(NULL, "Could not find level information file");
        QuitGame();
        return;
    }

    // Check for the unicode marker
    WORD wUnicodeMarker;
    fread( &wUnicodeMarker, sizeof(WORD), 1, fp );
    if( wUnicodeMarker != 0xfeff )
    {
        fclose(fp);
        DumpMessage(NULL, "Invalid level information file");
        QuitGame();
        return;
    }

    // In the .inf files, names are localized
    WCHAR* strNameToken = L"NAME_ENGLISH";
    switch( XGetLanguage() )
    {
        case XC_LANGUAGE_FRENCH:     strNameToken = L"NAME_FRENCH";     break;
        case XC_LANGUAGE_GERMAN:     strNameToken = L"NAME_GERMAN";     break;
        case XC_LANGUAGE_ITALIAN:    strNameToken = L"NAME_ITALIAN";    break;
        case XC_LANGUAGE_JAPANESE:   strNameToken = L"NAME_JAPANESE";   break;
        case XC_LANGUAGE_SPANISH:    strNameToken = L"NAME_SPANISH";    break;
        case XC_LANGUAGE_PORTUGUESE: strNameToken = L"NAME_PORTUGUESE"; break;
        case XC_LANGUAGE_KOREAN:     strNameToken = L"NAME_KOREAN";     break;
        case XC_LANGUAGE_TCHINESE:   strNameToken = L"NAME_CHINESE";    break;
    }

    // load the info
    while (TRUE)
    {
        // get a string
        if( fwscanf(fp, L"%s", string) <= 0 )
        {
            if( feof(fp) )
                break;
            continue;
        }

        // comment?
        if (string[0] == L';')
        {
            do
            {
                wch = fgetwc(fp);
            } while (wch != L'\n' && wch != WEOF);
            continue;
        }

        if (!wcsncmp(string, L"NAME_", 5 ))
        {
            if (!wcscmp(string,strNameToken))
            {
                do
                {
                    wch = fgetwc(fp);
                } while (wch != L'\'' && wch != WEOF);

                j = 0;
                while (TRUE)
                {
                    wch = fgetwc(fp);
                    if (wch != L'\'' && wch != WEOF && j < MAX_LEVEL_NAME - 1)
                    {
                        CurrentLevelInfo.strName[j] = (WCHAR)wch;
                        j++;
                    }
                    else
                    {
                        CurrentLevelInfo.strName[j] = L'\0';
                        break;
                    }
                }
            }
            else
            {
                do
                {
                    wch = fgetwc(fp);
                } while (wch != L'\n' && wch != WEOF);
            }
            continue;
        }

        // env still?
        if (!wcscmp(string, L"ENVSTILL"))
        {
            do
            {
                wch = fgetwc(fp);
            } while (wch != '\'' && wch != WEOF);

            j = 0;
            while (TRUE)
            {
                wch = fgetwc(fp);
                if (wch != '\'' && wch != WEOF && j < MAX_ENV_NAME - 1)
                {
                    CurrentLevelInfo.EnvStill[j] = (char)wch;
                    j++;
                }
                else
                {
                    CurrentLevelInfo.EnvStill[j] = '\0';
                    break;
                }
            }
            continue;
        }

        // env roll?
        if (!wcscmp(string, L"ENVROLL"))
        {
            do
            {
                wch = fgetwc(fp);
            } while (wch != '\'' && wch != WEOF);

            j = 0;
            while (TRUE)
            {
                wch = fgetwc(fp);
                if (wch != '\'' && wch != WEOF && j < MAX_ENV_NAME - 1)
                {
                    CurrentLevelInfo.EnvRoll[j] = (char)wch;
                    j++;
                }
                else
                {
                    CurrentLevelInfo.EnvRoll[j] = '\0';
                    break;
                }
            }
            continue;
        }

        // mp3?
        if (!wcscmp(string, L"MP3"))
        {
            do
            {
                wch = fgetwc(fp);
            } while (wch != '\'' && wch != WEOF);

            j = 0;
            while (TRUE)
            {
                wch = fgetwc(fp);
                if (wch != '\'' && wch != WEOF && j < MAX_MP3_NAME - 1)
                {
                    CurrentLevelInfo.Mp3[j] = (char)wch;
                    j++;
                }
                else
                {
                    CurrentLevelInfo.Mp3[j] = '\0';
                    break;
                }
            }
            continue;
        }

        // start pos
        if (!wcscmp(string, L"STARTPOS"))
        {
            fwscanf(fp, L"%f %f %f", 
                &CurrentLevelInfo.NormalStartPos.v[X], 
                &CurrentLevelInfo.NormalStartPos.v[Y], 
                &CurrentLevelInfo.NormalStartPos.v[Z]);
            continue;
        }

        // reverse start pos
        if (!wcscmp(string, L"STARTPOSREV"))
        {
            fwscanf(fp, L"%f %f %f", 
                &CurrentLevelInfo.ReverseStartPos.v[X], 
                &CurrentLevelInfo.ReverseStartPos.v[Y], 
                &CurrentLevelInfo.ReverseStartPos.v[Z]);
            continue;
        }

        // start rot
        if (!wcscmp(string, L"STARTROT"))
        {
            fwscanf(fp, L"%f", &CurrentLevelInfo.NormalStartRot);
            continue;
        }

        // reverse start rot
        if (!wcscmp(string, L"STARTROTREV"))
        {
            fwscanf(fp, L"%f", &CurrentLevelInfo.ReverseStartRot);
            continue;
        }

        // start grid
        if (!wcscmp(string, L"STARTGRID"))
        {
            fwscanf(fp, L"%d", &CurrentLevelInfo.NormalStartGrid);
            continue;
        }

        // reverse start grid
        if (!wcscmp(string, L"STARTGRIDREV"))
        {
            fwscanf(fp, L"%d", &CurrentLevelInfo.ReverseStartGrid);
            continue;
        }

        // far clip
        if (!wcscmp(string, L"FARCLIP"))
        {
            fwscanf(fp, L"%f", &CurrentLevelInfo.FarClip);
            continue;
        }

        // fog start
        if (!wcscmp(string, L"FOGSTART"))
        {
            fwscanf(fp, L"%f", &CurrentLevelInfo.FogStart);
            continue;
        }

        // fog color
        if (!wcscmp(string, L"FOGCOLOR"))
        {
            fwscanf(fp, L"%ld %ld %ld", &r, &g, &b);
            CurrentLevelInfo.FogColor = (r << 16) | (g << 8) | b;
            continue;
        }

        // vert fog start
        if (!wcscmp(string, L"VERTFOGSTART"))
        {
            fwscanf(fp, L"%f", &CurrentLevelInfo.VertFogStart);
            continue;
        }

        // vert fog end
        if (!wcscmp(string, L"VERTFOGEND"))
        {
            fwscanf(fp, L"%f", &CurrentLevelInfo.VertFogEnd);
            continue;
        }

        // world rgb percent
        if (!wcscmp(string, L"WORLDRGBPER"))
        {
            fwscanf(fp, L"%ld", &CurrentLevelInfo.WorldRGBper);
            continue;
        }

        // model rgb percent
        if (!wcscmp(string, L"MODELRGBPER"))
        {
            fwscanf(fp, L"%ld", &CurrentLevelInfo.ModelRGBper);
            continue;
        }

        // instance rgb percent
        if (!wcscmp(string, L"INSTANCERGBPER"))
        {
            fwscanf(fp, L"%ld", &CurrentLevelInfo.InstanceRGBper);
            continue;
        }

        // mirror info
        if (!wcscmp(string, L"MIRRORS"))
        {
            fwscanf(fp, L"%ld %f %f %f", 
                &CurrentLevelInfo.MirrorType, 
                &CurrentLevelInfo.MirrorMix, 
                &CurrentLevelInfo.MirrorIntensity, 
                &CurrentLevelInfo.MirrorDist);
            continue;
        }

        // redbook track
        if (!wcscmp(string, L"REDBOOK"))
        {
            fwscanf(fp, L"%ld %ld", &CurrentLevelInfo.RedbookStartTrack, &CurrentLevelInfo.RedbookEndTrack);
            continue;
        }

        // rocky ship shit
        if (!wcscmp(string, L"ROCK"))
        {
            fwscanf(fp, L"%f %f %f %f", 
                &CurrentLevelInfo.RockX, 
                &CurrentLevelInfo.RockZ, 
                &CurrentLevelInfo.RockTimeX, 
                &CurrentLevelInfo.RockTimeZ);
            continue;
        }

        // shadow box subtraction
        if (!wcscmp(string, L"SHADOWBOX"))
        {
            fwscanf(fp, L"%ld", &ShadowRgb);
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
        if (!_stricmp(levelInfo->szDir, dir))  //$MODIFIED: changed stricmp to _stricmp
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

char* GetAnyLevelFilename(long iLevel, long reversed, char *filename, long flag)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);
    
// make body

    if (flag & FILENAME_MAKE_BODY)
    {
        if (reversed)
            sprintf(LevelFilenameBuffer, "%s\\reversed\\%s.%s", levelInfo->szDir, levelInfo->szName, filename); //$MODIFIED: added "D:\\" at start
        else
            sprintf(LevelFilenameBuffer, "%s\\%s.%s", levelInfo->szDir, levelInfo->szName, filename); //$MODIFIED: added "D:\\" at start
    }

// don't make body

    else
    {
        if (reversed)
            sprintf(LevelFilenameBuffer, "%s\\reversed\\%s", levelInfo->szDir, filename); //$MODIFIED: added "D:\\" at start
        else
            sprintf(LevelFilenameBuffer, "%s\\%s", levelInfo->szDir, filename); //$MODIFIED: added "D:\\" at start
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

    sprintf(LevelFilenameBuffer, "D:\\times\\%s\\%S.%s", //$MODIFIED: added "D:\\" at start
        GhostFilenameDir[(reversed ? 1 : 0) | (mirrored ? 2 : 0)],
        li->strName,
        ext);

// return buf

    return LevelFilenameBuffer;
}

////////////////////////////////////////////////////////////////
//
// Accessibility
//
////////////////////////////////////////////////////////////////

bool IsLevelSelectable( int iLevel ) 
{
    LEVELINFO* pLevelInfo = GetLevelInfo(iLevel);
    if( NULL == pLevelInfo )
        return FALSE;

    // Is the track selectable?
    if( pLevelInfo->ObtainFlags & LEVEL_SELECTABLE )
    {
#ifdef SHIPPING
        // Don't allow users to select arbitrary tracks in shipping versions.
#else
        // Allow all tracks for all game types if dev version
        if( Version == VERSION_DEV )
            return TRUE;
#endif

        // Is the track type OK for this game mode?
        switch( GameSettings.GameType )
        {
            case GAMETYPE_TRIAL:
            case GAMETYPE_PRACTICE:
            case GAMETYPE_NETWORK_RACE:
                if( pLevelInfo->TrackType == TRACK_TYPE_RACE )
                    return TRUE;
                break;

            case GAMETYPE_SINGLE:
            case GAMETYPE_CLOCKWORK:
                if( pLevelInfo->TrackType == TRACK_TYPE_RACE || pLevelInfo->TrackType == TRACK_TYPE_USER )
                    return TRUE;
                break;

            case GAMETYPE_NETWORK_BATTLETAG:
                if( pLevelInfo->TrackType == TRACK_TYPE_BATTLE )
                    return TRUE;
                break;
        }
    }

    // Track is not selectable
    return FALSE;
}

bool IsLevelAvailable( int iLevel ) 
{
    LEVELINFO* pLevelInfo = GetLevelInfo( iLevel );
    if( NULL == pLevelInfo ) 
        return FALSE;

    return ( pLevelInfo->ObtainFlags & LEVEL_AVAILABLE ) ? TRUE : FALSE;
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

void SetLevelUnselectable(int iLevel)
{
    LEVELINFO *levelInfo = GetLevelInfo(iLevel);

    if (levelInfo != NULL) {
        levelInfo->ObtainFlags &= ~LEVEL_SELECTABLE;
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
