//-----------------------------------------------------------------------------
// File: MenuText.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "MenuText.h"
#include "credits.h"

#ifdef _N64
#include "ffs_list.h"
#include "ffs_code.h"
#endif

//-------------------------------------------------------------------------------
// PLATFORM Defines
//-------------------------------------------------------------------------------

// \V<THISVERSION_CHAR> Toglles parsing ONLY for current version... 
// S for Sony's PSX, N for Nintendo, P for PC, any other for a comment
#define VERSION_ESCAPECHAR 'V' 
                             

    // _P_C's only texts
    #define THISVERSION_ESCAPECHAR  'P' 

    #define LOG_DEBUG
    #define WARNING

    unsigned FILE_GetLength(FILE *fp) 
        {
        unsigned uPos = ftell(fp);
        unsigned uSize;
        fseek (fp,0,SEEK_END);
        uSize = ftell(fp);
        fseek (fp,uPos,SEEK_SET);
        return uSize;
        }

//-------------------------------------------------------------------------------
// HARDWIRED VERSION OF MENU_TEXT
//-------------------------------------------------------------------------------
#if defined(OLD_MENU_TEXT)

char *gTitleScreen_Text[TEXT_NTEXTS] =
{
    "Re-Volt",

	// Top level menu
    "Play Live!",
    "Single Player",
    "Multiplayer",
    "System Link",
    "Options",
	
	// Play Live menu
    "QuickMatch",
    "Find a Match",
    "Friends",
    "Create a Match",
    "Statistics",
	
	// Menu Titles
    "Select Race",
    "Enter Name",
    "Select Car",
    "Select Track",
    "Select Cup",
    "Summary",
    "Connection Type:",
    "Render Settings",
    "Game Type",
    "Waiting Room",
    "The Game Has Already Started!",
    "Controller Settings",
    "Select Game To Join",

// Title Screen
    "Start Race",
    "Best Trial Times",
    "Progress Table",
    "Track Editor",
//    "Options",
    "Quit",

	// Select Race
    "Single Race",
    "Time Trial",
    "Battle Tag",
    "Practice",
    "Stunt Arena",


    "Championship2",
    "Single Race2",
    "Clockwork Carnage2",
    "Battle Tag2",
    "Time Trial2",
    "Practice2",
    "Stunt Arena2",
//    "Multi-Player2",
    "Edit Mode2",
    "Calculate Car Stats2",

// Select Car
    "Select Car",

// Select Track
    "Select Track",
    "Mirrored",
    "Reversed",
    "CPU Cars",
    "Laps",
    "Length",

// Select Cup
    "Bronze Cup",
    "Silver Cup",
    "Gold Cup",
    "Platinum Cup",

// Training Mode
    "Select Mode",
    "Basic Cornering",
    "Advanced Cornering",
    "Your First Stunts",

// Options
    "Game Settings",
    "Video Settings",
    "Audio Settings",
    "Controller Settings",
    "Load/Save Records",
    "View Credits",

// Game Settings
    "Number of Cars",
    "Random Cars",
    "Random Tracks",
    "Pick ups",
    "Number of Laps",
    "Battle Tag Time",
    "Difficulty",
    "Access Cars",
    "Ghost Car",
    "Collision",
    "Speed Units",

// Name Entry
    "Enter Name",

// Misc.
    "On",
    "Off",
    "Yes",
    "No",
    "Easy",
    "Medium",
    "Hard",
    "Extreme",
    "Vertical",
    "Horizontal",

// Video Settings
    "Resolution",
    "Textures",
    "Shininess",
    "Shadows",
    "Video Device",
    "Brightness",
    "Contrast",
    "Draw Distance",
    "Reflections",
    "Instance Models",
    "Skid Marks",
    "Lights",
    "Automatic",
    "Unavailable",
    "Antialias",
    "Enable V-Sync",
    "Show Frame Rate",
    "Texture Filter",
    "Mipmap Level",
    "Wireframe",
    "Car BBoxes",
    "Particles",
    "Smoke",
    "High Resolution",
    "Screen Split",

// Audio Settings
    "In-Game Music",
    "SFX Volume",
    "Music Volume",
    "SFX Test",
    "Music Test",
    "SFX Channels",
    "Audio",
    "Stereo",
    "Mono",

// TimeTrial
    "View Records",
    "Save Records",
    "Race",

    "Records",

// MultiPlayer
    "Start Game",
    "Join Game",
    "Wait for Lobby Connection",
    "Choose Game",
    "Host Computer:",
    "Continue Lobby Game",
    "Quit Lobby Game",
    "Number of Players",
    "Packet Optimization",
    "DirectPlay Protocol",
    "Searching for Games...",
    "Joining Game...",
    "Press Spacebar To Refresh Game and Player List",
    "Next Track",

// Cars Allowed
    "Default",
    "Bronze",
    "Silver",
    "Gold",
    "Special",

// Load Replay
    "Load Replay",
    "Save Replay",

// In game menu
    "Resume",
    "Continue Championship",
    "End Championship",
    "Give Up Try",
    "Restart Race",
    "Restart Replay",
    "Quit",
    "View Replay",

// Controller
    "Select Controller:",
    "Configure Controller",
    "Steering Deadzone",
    "Steering Range",
    "Non-linear Steering",
    "Set Controls to Default",
    "Configuration",

// Misc
    "Demo Mode",
    "Player     ",
    "Save Carinfo Single",
    "Save Carinfo Multiple",
    "Save Current Car Info",
    "Reset Progress Table",
    "Mode",
    "Simulation",
    "Arcade",
    "Console",
    "Junior RC",
    "MPH",
    "Scaled MPH",
    "FPM",
    "KPH",
    "Scaled KPH",

// Win/lose sequence strings
    "YOU CAME",
    "TRY TO COME HIGHER TO",
    "UNLOCK THE NEXT CHALLENGE!",
    "BRONZE CUP",
    "SILVER CUP",
    "GOLD CUP",
    "SPECIAL CUP",
    "IS UNLOCKED",
    "MIRRORED TRACKS",
    "REVERSED TRACKS",
    "HAVE BEEN UNLOCKED",
    "NEW DELIVERY OF CARS",
    "YOU HAVE UNLOCKED",
    "NEW CARS",
    "1ST",
    "2ND",
    "3RD",

//------------------
// Memory Card
//------------------
    "Load / Save",
    "Error",
    "Warning",
    "Are you sure?",
    "Load Settings",
    "Save Settings",
    "Load User Tracks",
    "Loading...",
    "Saving...",
    "                              ",
    "Load Ghost",
    "Save Ghost",
    "Continue",
    "Continue Without Saving",
    "Continue Without Loading",
    "Format Controller Pak",
    "Attempt Repair",
    "Manage Controller Pak",
    "Change Controller Pak",
    "Retry",
    "Attempting Repair...",
    "Repair Failed",
    "Repair Successful",
    "                                                    ",
    "Delete Memory Card Block",

    // Championship summary screen
    "Place 3rd or better in each",
    "race to progress!",
    "Finish top of the table to",
    "access the next challenge!",
    "Finish top of the table and",
    "be crowned \"Re-Volt Master\"!",

    // N64 Specifics MEM_CARD messages
    "",
    "CONTROLLER PAK DETECTED IN\n\rCONTROLLER \7, THIS CONTROLLER PAK\n\rWILL BE USED TO SAVE AND LOAD\n\r",
    "NEW CONTROLLER PAK INSERTED INTO\n\rCONTROLLER \7, THIS CONTROLLER PAK\n\rWILL BE USED TO SAVE AND LOAD\n\r",
    "CONTROLLER PAK INSERTED INTO\n\rCONTROLLER \7, WILL BE USED TO SAVE\n\rAND LOAD ",
    "WARNING INVALID CONTROLLER PAK!!\r\nCONTROLLER PAK \7 IS DAMAGED",
    "WARNING!!\r\nCONTROLLER PAK \7 FULL\n\rUNABLE TO SAVE ",
    "WARNING!!\r\nCONTROLLER PAK \7 FULL\n\rUNABLE TO SAVE ",
    "ERROR!!\r\nCONTROLLER DISCONNECTED",
    "WARNING!!\r\nNO CONTROLLER PAK FOUND\r\nIT WILL NOT BE POSSIBLE TO\n\rSAVE OR LOAD",
    "ERROR!!\r\nUNABLE TO LOCATE GAME\r\nON CONTROLLER PAK",
    "PLEASE REMOVE RUMBLE PAK\n\rAND INSERT A CONTROLLER PAK\n\rTHEN TRY AGAIN",
    "This game is Rumble Pak Compatible\n\rPlease Insert your Rumble Pak now\n\rIf you wish to play with it",
    "WARNING!!\r\nPreviously save data\n\rmay be lost!",
    "REPAIR?",
    "GAME PROGRESS",
    "THE GHOST CAR",
    "USER TRACKS",
    "Overwrite",

// Specifics MEM_CARD messages, shared N64 & PSX ??
    "Confirm",

//---------------
// MenuDraw.cpp
//---------------
    "M",
    "R",
    "(M)",
    "(R)",

// Select Car
    "Class",
    "Rating",
    "Speed",
    "Acc",
    "Weight",
    "Trans",

    "Electric",
    "Glow",
    "Special",

    "Rookie",
    "Amateur",
    "Advanced",
    "Semi-Pro",
    "Pro",

    "4WD",
    "RWD",
    "FWD",

// Select Track
    "Unknown",
    "Locked",

// Frontend Summary
    "Player",
    "Players",

// Select Mode
    "Full Speed; Realistic collision",
    "Full Speed; Simple car collision",
    "Full Speed; Simple collision",
    "Reduced Speed; Simple collision",

// Controller Config
    "Accelerate",
    "Reverse",
    "Fire",
    "Flip Car",
    "Reposition",
    "Change Camera",
    "Rear View",
    "Pause",

// Draw Progress
    "Tracks",
    "Won",  
    "Race",
    "Norm", 
    "Rev",  
    "Mirr", 
    "Stars",

// Load Screen Info
    "Game Mode:",
    "Stage:",
    "Track:",
    "Car",
    "Won Race:",
    "Found Star:",
    "Stars Collected:",
    "of",
    "Beaten",
    "Challenge Time:",

    "You have",
    "Try Left.",
    "Tries Left.",

//---------------
// Panel
//---------------

// Position
    "st",
    "nd",
    "rd",
    "th",

// Championship 
    "CONGRATULATIONS!",      
    "YOU QUALIFIED",         
    "CONGRATULATIONS!",      
    "CHAMPIONSHIP FINISHED!",
    "YOU FAILED TO QUALIFY!",
    "car",
    "pnts",

// Stats
    "Best Lap",
    "Last Lap",
    "Lap",
    "Race",
    "Lap",
    "ghost split",
    "Ghost",
    "Challenge",
    "Challenge Time Beaten!",
    "time",
    "Game  Results",
    "Race  Results",

//---------------
// Menudat2.cpp
//---------------

//Confirmation strings
    "Really Quit?",
    "Really Restart?",
    "Really or Pretend?", 
    "Really", 
    "Pretend",


//---------------
// Compete.cpp
//---------------
    "Default",
    "Bronze",
    "Silver",
    "Gold",
    "Platinum",

    "Game Over!  Better Luck Next Time!",

//---------------
// TitleScr.cpp
//---------------
    "NEW DELIVERY OF CARS!\n(Check the stack of boxes on\nthe Car Selection Screen)\n",
    "CONGRATULATIONS!\n%s TRACKS ARE NOW\nAVAILABLE IN REVERSE MODE\n(Press DOWN on Track Selection screen)\n",
    "CONGRATULATIONS!\n%s TRACKS ARE NOW\nAVAILABLE IN MIRROR MODE\n(Press UP on Track Selection screen)\n",
    "CONGRATULATIONS!\n%s TRACKS ARE NOW AVAILABLE\nIN REVERSE MIRROR MODE\n(Press UP and then DOWN\non Track Selection screen)\n",
    "NEW GAME MODE!\n\"CLOCKWORK CARNAGE\"\nHAS BEEN UNLOCKED\n",


//---------------
// Gameloop.cpp
//---------------
    "Press Start Button",
    "Press Start BUTTON to continue",
    "\1Press Start BUTTON to continue",
    "Pause",

//---------------
// Credits.cpp
//---------------

    "A Game by",
    "Project Manager",
    "Lead",
    "PC Coders",
    "Nintendo 64 Coders",
    "PlayStation Coders",
    "Visuals and Audio (R/C Eyes and Ears)",
    "Dynamics (R/C Hands and Feet)",
    "Intellect (R/C Brains)",
    "Track Editor",
    "Additional Programming",
    "Lead Artist",
    "Track Modelling",
    "Track Textures",
    "Car Models and Artwork",
    "Based on an original concept by",
    "Lead Designer",
    "Designers",
    "Sound Effects Design",
    "Music Written By",
    "Special N64 Arrangement",
    "QA Manager",
    "QA Team",
    "Network Manager",
    "PC Technician",
    "Director of Product Development",
    "Art Director",
    "Acting Creative Director",
    "Audio Director",
    "Business Development Manager",
    "Acting Technical Director",
    "Special Thanks",

    "",
    "Developed by Acclaim Studios London",
    "",
    "All Rights Reserved",
    "",
    "Uses Miles Sound System. @ 1991-1999 by Rad Game Tools, inc.",
    "",
    "All characters herein and distict likenesses thereof are",
    "trademark of Acclaim Entertainment, inc.",
    "",
    "TM, r & @ 1999 ACCLAIM ENTERTAINMENT, INC. ALL RIGHTS RESERVED",
    ""
    "Distributed by Acclaim Distribution, inc.",
    "",
    "",
    "THANK YOU FOR PLAYING!",

// ------------
// Track Editor
//-------------
    "modules",
    "pickups",
    "adjust",
    "save",
    "new",
    "load",
    "export",
    "quit",
    "place modules",
    "deleting",

    //mode status words for bottom left of sprue
    "menu",
    "warning",
    "question",
    "info",

    //module group names
    "start grid",
    "straights",
    "dips",
    "humps",
    "square bend",
    "round bend",
    "diagonal",
    "bank",
    "rumble",
    "narrow",
    "pipe",
    "bridge",
    "crossroad",
    "jump",
    "chicane",

    //query dialog
    "YES",
    "NO",

    //compilation status
    "stage : 1",
    "stage : 2",
    "stage : 3",
    "stage : 4",
    "stage : 5",
    "stage : 6",
    "stage : 7",
    "stage : 8",
    "stage : 9",
    "stage : 10",
    "stage : 11",
    "stage : 12",
    "Finished!!",

    "SAVE",

    //default description for all tracks ???
    "Track %d",

    "pickups placed:\n\r%d of 6",

    ": position",
    ": size",

    //warnings & errors
    "You have not saved the current track.\n\rDo you want to save it now?\n\r",
    "Failed to Save Track\n\rRetry ?"
    "Overwrite Existing Track?\n\r"
    "Do you really want to delete\n\r'%s'\n\r",
    "Do you want to keep the changes\n\ryou've just made?\n\r",
    "Track Saved!",
    "Error Saving Track!",
    "\1Track doesn`t form a loop, or has a\n\rstep in a pipe.The cursor will be\n\rplaced at the last valid position",
    "\1Track doesn`t form a loop, or has an\n\rupwards step in it.The cursor will\n\rbe placed at the last valid position",
    "\1Track doesn`t form a loop, or has an\n\rupwards step in a pipe."
    "\1Track doesn`t form a loop, or has an\n\rupwards step in it."
    "Track Too Complex!"
    "\1Track doesn`t have a Start Grid",
    "\1Track has more than one Start Grid",

//-----------------
// PC missed stuff!
//-----------------

    "CONTROL_LEFT",
    "CONTROL_RIGHT",
    "CONTROL_HORN",
    "TRYAGAIN",
    "DEMO",
    "LOADINGFRONTEND",
    "LOADINGCREDITS",
    "LOADINGGAMEGAUGE",
    "RACERS",
    "CPUOPPONENT",
    "RANDOM",
    "STATUS",
    "READY",
    "NOTREADY",
    "HITTABTOSTART",
    "MULTIGAMETERMINATED",
    "WAITINGFOR",
    "WAITINGFORHOST",
    "HOST",
    "TRACK",
    "GAME",
    "WRONGVERSION",
    "FULL",
    "RECORD",
    "SCALED",
    "FINISHED",
    "STARTED",
    "OPEN",
    "REPLAY",
    "View Gallery",
    "LOCAL",
    "DOWNLOAD",
};

#else

//-------------------------------------------------------------------------------
// EMPTY VERSION OF MENU_TEXT.... every byte of memory, is worth it
//
// => _MUST_ Make a call to LANG_LoadStrings() or _WILL_ Crash
//
//-------------------------------------------------------------------------------
char *gTitleScreen_Text[TEXT_NTEXTS];
#endif


//*******************************************************************************
//
// DYNAMIC LANGUAGE's STRING LOADING
//
//*******************************************************************************

////////////////////////////////////////////////////////////////
//
// Language stuff
//
////////////////////////////////////////////////////////////////
//  ENGLISH,
//  FRENCH,
//  GERMAN,
//  ITALIAN,
//  SPANISH,

char *LangNames[NLANGS] = {
    "English",
    "Français",
    "Deutsch",
    "Italiano",
    "Español",
};

//-------------------------------------------------------------------------------
// Locals  
//-------------------------------------------------------------------------------
static unsigned s_uLanguage = ENGLISH;
static char *   s_pStrings  = NULL;

static char * s_NameFiles[LANG_LAST] = 
{
	"D:\\Strings\\English.txt",
	"D:\\Strings\\French.txt",
	"D:\\Strings\\German.txt",
	"D:\\Strings\\Italian.txt",
	"D:\\Strings\\Spanish.txt",
};


//-------------------------------------------------------------------------------
// unsigned LANG_GetLanguage()
//-------------------------------------------------------------------------------
unsigned LANG_GetLanguage()
{
    return s_uLanguage;
};




//-------------------------------------------------------------------------------
// bool LANG_LoadStrings(unsigned uLang)
//-------------------------------------------------------------------------------
bool LANG_LoadStrings(unsigned uLang)
{
#ifdef OLD_MENU_TEXT
	return TRUE;
#endif

    FILE *      fp;
    unsigned    uFileLength;
    char *      pStrings = NULL;

    // Validating...
    //----------------
    if (uLang >= LANG_LAST)
        {
        WARNING ("WARNING: Wrong Language ID\n");
        return FALSE;
        }

    // Loading...
    //----------------
    if (  !(fp = fopen(s_NameFiles[uLang],"rb"))
       || !(uFileLength = FILE_GetLength(fp))
       || !(pStrings = (char*)malloc(uFileLength) )
       || ((fread(pStrings,1,uFileLength,fp)) != uFileLength)
       ) {
        WARNING ("WARNING, Error while opening String File or no memory\n");
        if (fp) fclose(fp);
        if (pStrings) free(pStrings);
        return FALSE;
        }    

    // Parsing...
    //----------------
    {
    unsigned uNStrings = 0;
    char *pScan = pStrings;
    char *pCopy = pStrings;

    //--------------------------
    // Lines loop
    //--------------------------
    fclose(fp);
    while ( (uNStrings < (unsigned)TEXT_NTEXTS) && ((unsigned)(pScan - pStrings) < uFileLength) )
        {
        char c;
        bool bEscapeMode = FALSE;
        unsigned uNEscapechars  = 0;
        unsigned uEscapeCode    = 0;
        char *   pStringParsed  = pCopy;
        bool     bIgnoreLine    = FALSE;

        while ( ( (c = *pScan) != '\r') && ((pScan - pStrings) < (signed)uFileLength) )
            {
            // Escape parsing
            //--------------------------
            if (bEscapeMode)
                {           
                // continuation of Code (\number) ?
                //-----------------------------------
                bool bIsDigit = (c >= '0' && c <= '9');
                if (uNEscapechars)
                    {
                    if (bIsDigit)
                        {
                        uEscapeCode *= 10;
                        uEscapeCode += c - '0';
                        uNEscapechars++;
                        pScan++;
                        }
                    if (!bIsDigit || uNEscapechars == 3)
                        {
                        *pCopy++ = uEscapeCode;
                        uNEscapechars = 0;
                        bEscapeMode = FALSE;
                        }
                    }

                // Start of Code (\number) ?
                //-----------------------------------
                else if (bIsDigit) 
                    {
                    uNEscapechars = 1;
                    uEscapeCode = c - '0';
                    pScan++;
                    }

                // Others forms 
                //-----------------------------------
                else 
                    {
                    switch (c) {
                        case '\\':  *pCopy++ = '\\'; break;
                        case 'r':   *pCopy++ = '\r'; break;
                        case 'n':   *pCopy++ = '\n'; break;
                        case '"':   *pCopy++ = '\"'; break;

                        case VERSION_ESCAPECHAR: bIgnoreLine = ( *(pScan+1) != THISVERSION_ESCAPECHAR ); if (!bIgnoreLine) pScan++; break;


                        default :   WARNING("Unknown Escape char\n"); break;
                        }
                    bEscapeMode = FALSE;
                    pScan++;
                    }
                }
            // No Escape... yet
            //--------------------------
            else
                {
                if (c == '\\')
                    bEscapeMode = TRUE;
                else if (c != '\n')
                    *pCopy++ = c;
                pScan++;
                }
            }
        pScan++;
        if (bIgnoreLine)
            pCopy = pStringParsed;
        else
            {
            /*
            unsigned uNLines    = 0;
            unsigned uMaxWidth  = 0;
            unsigned uWidth     = 0;
            char *   pScan      = pStringParsed;


            while (*pScan!= 0)
                {
                switch (*pScan)
                    {
                    case '\n': 
                        if (uMaxWidth < uWidth) uMaxWidth = uWidth;
                        uNLines++;
                        uWidth = 0;
                        break;

                    case 1:
                    case '\r': 
                        break;

                    default:
                        uWidth++;
                        break;
                    }
                ++pScan;
                }

            if (uMaxWidth < uWidth) uMaxWidth = uWidth;
            if (uMaxWidth > 35 || uNLines >= 5)
                printf ("-(%dx%d)-------------------------\n%s\n",uMaxWidth,uNLines,pStringParsed);
            */
            
            *pCopy++ = 0;
            uNStrings++;
            LOG_DEBUG("Parsed %d: %s\n",uNStrings,pStringParsed);
            }
        }

    // Succeeded... we can safely overwritte current
    //----------------------------------------------
    if (uNStrings == TEXT_NTEXTS)
        {
        char **pTextptr = gTitleScreen_Text;
        if (s_pStrings)
            free(s_pStrings);
        s_pStrings = (char*)malloc(pCopy - pStrings);
        memcpy(s_pStrings,pStrings,pCopy - pStrings);
        free(pStrings);
        pStrings = s_pStrings;
        while (uNStrings--)
            {
            *pTextptr++ = pStrings;
            while (*pStrings != 0) pStrings++;
            pStrings++;
            }
        s_uLanguage = uLang;

        return TRUE;
        }
    // Failure... Clean exit 
    //----------------------------------------------
    else
        {
        WARNING("Error, parsing Ok until line %d\n",uNStrings);
        free(pStrings);
        return FALSE;
        }

    }
}

