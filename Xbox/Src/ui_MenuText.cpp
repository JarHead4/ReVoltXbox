//-----------------------------------------------------------------------------
// File: ui_MenuText.cpp
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "ui_MenuText.h"




//-------------------------------------------------------------------------------
// Language stuff and localized strings
//-------------------------------------------------------------------------------
WCHAR *g_strLanguageNames[NUM_LANGUAGES] = 
{
    L"English",
    L"Français",
    L"Deutsch",
    L"Italiano",
    L"Español",
    L"Nihongo",
    L"Portuguese",
    L"Korean",
    L"Chinese",
};

static CHAR* g_strLocalizedStringFiles[NUM_LANGUAGES] = 
{
    "D:\\Strings\\English.bin",
    "D:\\Strings\\French.bin",
    "D:\\Strings\\German.bin",
    "D:\\Strings\\Italian.bin",
    "D:\\Strings\\Spanish.bin",
    "D:\\Strings\\Japanese.bin",
    "D:\\Strings\\English.bin",
    "D:\\Strings\\English.bin",
    "D:\\Strings\\English.bin",
};
    



//-------------------------------------------------------------------------------
// Array of all displayable text for the game
//-------------------------------------------------------------------------------

//$TODO: The following hard-coded strings are really only used for during development
static WCHAR* g_pHardCodedStrings[TEXT_NTEXTS] =
{
    // Menu Titles
    L"Re-Volt",
    L"Press START To Play",
    L"Language",
    L"Options",
    L"Best Trial Times",
    L"Progress Table",
    L"Game Settings",
    L"Video Settings",
    L"Render Settings",
    L"Audio Settings",
    L"Controller Settings",
    L"Mode",
    L"Enter Nickname",
    L"Select Race",
    L"Select Car",
    L"Select Track",
    L"Summary",
    L"Single Player",
    L"Multiplayer",
    L"System Link",
    L"Game Type",
    L"Waiting Room",
    L"Sign In",
    L"Sign Out",
    L"Play Live",
    L"Quick Match",
    L"OptiMatch",
    L"Select Match",
    L"Create Match",
    L"Scoreboards",
    L"Racer Statistics",
    L"Players",
    L"Friends",
    L"Quit",
    L"Quit Re-Volt?",

    L"Could not open replay file",
    L"Could not read replay data",
    L"Corrupt Content",
    L"Corrupt content was detected.\nThis content may be downloaded again\nfor free from the online store.",
    L"Corrupt files",
    L"Corrupt files were detected.\nThe corruption has been fixed.\nYour settings or save games may be lost.",
 

    // Menus
    L"Select Cup",
    L"Connection Type:",
    L"Select Game To Join",

    // Select Race
    L"Race",
    L"Battle Tag",

    // Start Race
    L"Championship",
    L"Single Race",
    L"Clockwork Carnage",
    L"Time Trial",
    L"Practice",
    L"Stunt Arena",
    L"Edit Mode",
    L"Calculate Car Stats",

    // Select Track
    L"Laps",
    L"Length",
    L"meters",
    L"m",
    L"Locked",
    L"Unknown",

    // Select Cup
    L"Bronze Cup",
    L"Silver Cup",
    L"Gold Cup",
    L"Platinum Cup",

    // Options
    L"View Credits",

    // Game Settings
    L"Number of Cars",
    L"Random Cars",
    L"Random Tracks",
    L"Pick ups",
    L"Number of Laps",
    L"Battle Tag Time",
    L"Difficulty",
    L"Ghost Car",
    L"Speed Units",

    // Misc.
    L"On",
    L"Off",
    L"Yes",
    L"No",
    L"Easy",
    L"Medium",
    L"Hard",
    L"Extreme",

    // Video Settings
    L"Resolution",
    L"Shininess",
    L"Shadows",
    L"Brightness",
    L"Contrast",
    L"Draw Distance",
    L"Reflections",
    L"Instance Models",
    L"Skid Marks",
    L"Lights",
    L"Antialias",
    L"Enable V-Sync",
    L"Show Frame Rate",
    L"Texture Filter",
    L"Mipmap Level",
    L"Wireframe",
    L"Particles",
    L"Smoke",

    // Audio Settings
    L"In-Game Music",
    L"SFX Volume",
    L"Music Volume",
    L"SFX Channels",

    // MultiPlayer
    L"Start Game",
    L"Create Game",
    L"Join Game",
    L"Wait for Lobby Connection",
    L"Host Console:",
    L"Continue Lobby Game",
    L"Quit Lobby Game",
    L"Number of Players",
    L"Packet Optimization",
    L"DirectPlay Protocol",
    L"Searching for Games...",
    L"Joining Game...",
    L"Press \200 To Refresh Game and Player List",
    L"Start Next Race",
    L"New Race",

    // Load Replay
    L"Load Replay",
    L"Save Replay",

    // In game menu
    L"Resume",
    L"Continue Championship",
    L"End Championship",
    L"Give Up Try",
    L"Restart Race",
    L"Restart Replay",
    L"Quit",
    L"View Replay",
    L"Could not open file",
    L"Saved in Replay.rpl",
    L"Could not save replay data",

    // Controller
    L"Select Controller:",
    L"Configure Controller",
    L"Steering Deadzone",
    L"Steering Range",
    L"Non-linear Steering",
    L"Set Controls to Default",
    L"Configuration",

    // Misc
    L"Demo Mode",
    L"Player     ",
    L"Simulation",
    L"Arcade",
    L"Console",
    L"Junior RC",
    L"MPH",
    L"Scaled MPH",
    L"FPM",
    L"KPH",
    L"Scaled KPH",

    // Win/lose sequence strings
    L"YOU FINISHED",
    L"TRY TO COME IN HIGHER TO",
    L"UNLOCK THE NEXT CHALLENGE!",
    L"BRONZE CUP",
    L"SILVER CUP",
    L"GOLD CUP",
    L"SPECIAL CUP",
    L"IS UNLOCKED",
    L"MIRRORED TRACKS",
    L"REVERSED TRACKS",
    L"HAVE BEEN UNLOCKED",
    L"NEW DELIVERY OF CARS",
    L"YOU HAVE UNLOCKED",
    L"NEW CARS",
    L"1ST",
    L"2ND",
    L"3RD",

    L"Are you sure?",

    // Championship summary screen
    L"Place 3rd or better in each",
    L"race to progress!",
    L"Finish top of the table to",
    L"access the next challenge!",
    L"Finish top of the table and",
    L"be crowned \"Re-Volt Master\"!",

    // MenuDraw.cpp
    L"M",
    L"R",
    L"(M)",
    L"(R)",

    // Select Car
    L"Class",
    L"Rating",
    L"Speed",
    L"Accel",
    L"Weight",
    L"Trans",

    L"Electric",
    L"Glow",
    L"Special",

    L"Rookie",
    L"Amateur",
    L"Advanced",
    L"Semi-Pro",
    L"Pro",

    L"4WD",
    L"RWD",
    L"FWD",

    L"???",

    // Frontend Summary
    L"Player",

    // Select Mode
    L"Full Speed; Realistic Collision",
    L"Full Speed; Simple Car Collision",
    L"Full Speed; Simple Collision",
    L"Reduced Speed; Simple Collision",

    // Controller Config
    L"Accelerate",
    L"Reverse",
    L"Fire",
    L"Flip Car",
    L"Reposition",
    L"Pause",

    // Draw Progress
    L"Tracks",
    L"Won",  
    L"Race",
    L"Normal", 
    L"Reverse",  
    L"Mirror", 
    L"Stars",

    // Load Screen Info
    L"Game Mode:",
    L"Stage:",
    L"Track:",
    L"Car",
    L"Won Race:",
    L"Found Star:",
    L"Stars Collected:",
    L"of",
    L"Beaten",
    L"Challenge Time:",

    L"You Have 1 Attempt Left",
    L"You Have %ld Attempts Left",

    // Position
    L"1st",
    L"2nd",
    L"3rd",
    L"4th",
    L"5th",
    L"6th",
    L"7th",
    L"8th",
    L"9th",
    L"10th",
    L"11th",
    L"12th",
    L"13th",
    L"14th",
    L"15th",
    L"16th",

    // Championship 
    L"CONGRATULATIONS!",
    L"YOU QUALIFIED",         
    L"CONGRATULATIONS!",      
    L"CHAMPIONSHIP FINISHED!",
    L"YOU FAILED TO QUALIFY!",
    L"car",
    L"points",

    // Stats
    L"Best Lap",
    L"Last Lap",
    L"Lap",
    L"Race",
    L"Lap",
    L"Ghost Split",
    L"Ghost",
    L"Challenge",
    L"Challenge Time Beaten!",
    L"Time",
    L"Game Results",
    L"Race Results",

    // Confirmation strings
    L"Really Quit?",
    L"Really Restart?",
    L"Really or Pretend?", 
    L"Really", 
    L"Pretend",

    // Compete.cpp
    L"Default",
    L"Bronze",
    L"Silver",
    L"Gold",
    L"Platinum",

    L"Game Over! Better Luck Next Time!",

    // TitleScr.cpp
    L"NEW DELIVERY OF CARS!\n(Check the stack of boxes on\nthe Car Selection Screen)\n",
    L"CONGRATULATIONS!\n%s TRACKS ARE NOW\nAVAILABLE IN REVERSE MODE\n(Press DOWN on Track Selection screen)\n",
    L"CONGRATULATIONS!\n%s TRACKS ARE NOW\nAVAILABLE IN MIRROR MODE\n(Press UP on Track Selection screen)\n",
    L"CONGRATULATIONS!\n%s TRACKS ARE NOW AVAILABLE\nIN REVERSE MIRROR MODE\n(Press UP and then DOWN\non Track Selection screen)\n",
    L"NEW GAME MODE!\n\"CLOCKWORK CARNAGE\"\nHAS BEEN UNLOCKED\n",

    // PC missed stuff!
    L"Left",
    L"Right",
    L"Horn",
    L"Try Again?",
    L"Demo",
    L"Loading Menu",
    L"Loading Demo",
    L"Loading Credits",
    L"Loading Game Gauge Benchmark",
    L"Racers:",
    L"CPU",
    L"Random",
    L"Status",
    L"Ready",
    L"Not Ready",
    L"Press \200 to Start",
    L"Multiplayer Game Terminated!",
    L"Waiting For:",
    L"Waiting For Host...",
    L"Host",
    L"Track",
    L"Game",
    L"Wrong Version",
    L"Full",
    L"Record",
    L"Scaled",
    L"Finished",
    L"Started",
    L"Open",
    L"Replay",
    L"View Gallery",
    L"Local",
    L"Download",

    L"Game Type:",
    L"Max Players:",
    L"Track:",
    L"Laps:",

    L"Game Mode",
    L"Num Laps",
    L"Invite Mode",
    L"Total Players",

    L"Public",
    L"Friends Only",

    L"Game Mode",
    L"Track",
    L"Any",

    L"No Games Found!",
    
    L"My Statistics",
    L"Friends Statistics",
    L"World's Best",

    L"Press any button to continue",
    L"Press START to continue",
    L"Restarting...",
    L"Host Restarting...",
    L"Starting Game...",

    // Network.cpp
    L"Notice",

    L"Connection to server\ncould not be established.\n(Maybe server was turned off, or\nit stopped hosting the session,\nor the game already started.)",
    L"Connection to server was\nlost or blocked.  (Maybe server\nexited the session, or maybe the\nsession is full.)",
    L"Server rejected your request\nto join the session.  (The game\nis already full, or you can only\njoin via a friend invitation.)",

    // Player.cpp
    L"%s place: %S",
    L"Time: %02d:%02d:%03d",

    // ui_ContentDownload.cpp
    L"Online Update",
    L"Checking for new content.",
    L"Downloading new content.",
    L"\201 Cancel Update",
    L"Confirm",
    L"Could not read new cars",
    L"Update complete",
    L"Error:\n",
    L"Online Store",
    L"Download complete",
    L"Free",
    L"Price %s",
    L"*You will not actually be charged.*",
    L"Offer Duration: Non-terminating.",
    L"Offer Duration: %u Months.",
    L"The first %u months are Free!",
    L"This is a one time change.",
    L"You will be charged monthly.",
    L"You will be chareged quarterly.",
    L"You will be charged biannually.",
    L"You will be charged annually.",
    L"You may cancel at any time.",
    L"Already Subscribed.",
    L"Already owned, download is free.",
    L"\200 Subscribe \201 Back",
    L"\200 Cancel subscripton \201 Back",
    L"\200 Download \201 Back",
    L"\200 Purchase \201 Back",
    L"You cannot play online\nwithout updating.\nDo you still want to abort?",
    L"No new content is available.",
    L"Getting Content Details",
    L"Have Fun!",
    L"Subscription cancelled.",
    L"Purchasing content.",
    L"Please do not turn off your Xbox.",
    L"Purchasing subscription.",
    L"Canceling subscription.",
    L"\200 Continue \201 Cancel",
    L"This account is not authorized\nfor billed content or subscriptions.\nThe Dashboard can be used to\nchange account permissions",
    L"Cancel Subscription?",
    L"Subscribe?",
    L"Purchase?",
    
    // ui_EnterName.cpp
    L"More letters and symbols",
    L"Name being verified...",
    L"Nickname",
    L"The nickname you entered\nis not appropriate!",

    // ui_LiveSignOn.cpp
    L"Xbox Live Sign In",
    L"Guest of %S",
    L"Press \200 to Sign In",
    L"Select Account",
    L"Create Account",
    L"Sign In As Guest",
    L"There are no accounts\non your Xbox console.",
    L"Do you want to start the\nXbox Dashboard to create\na new account?",
    L"Start the Xbox Dashboard",
    L"Enter Pass Code",
    L"Pass code incorrect.\nPlease try again.",
    L"You cannot use the same account\nas another player.",
    L"You must be a guest of\na signed in account.",
    L"Waiting for other players...",
    L"No more players",
    L"You are being signed in.\n\nPlease wait...",
    L"Players are being signed in.\n\nPlease wait...",
    L"You are signed in!",
    L"Sign in failed!",
    L"The hosting account\ncould not sign in.",
    L"No network connection was found.",
    L"Your Xbox console cannot\nconnect to Xbox Live.",
    L"A required update of Re-Volt\nis available. You cannot\nconnect to Xbox Live until\nthe update is installed.",
    L"Your Xbox console cannot\nconnect to Xbox Live.",
    L"The Xbox Live service is too\nbusy at this time. Please try\nagain later.",
    L"You have an important message from\nXbox Live that must be read before\nusing your account. The message can be\nread in the Xbox Dashboard.",
    L"Your Xbox console cannot\nconnect to Xbox Live.",
    L"You have a new Xbox Live message.\n The message can be read in the XBox Dashboard.",
    L"Change Account",
    L"Begin Installation",
    L"Back to Main Menu",
    L"Start Xbox Dashboard",
    L"Start Network Troubleshooter",
    L"Get Messages",
    L"Let's Play!",

    // ui_Options.cpp
    L"Voice Mask",
    L"None",
    L"Dark Master",
    L"Cartoon",
    L"Big Guy",
    L"Child",
    L"Robot",
    L"Whisper",

    // ui_PlayLive.cpp
    L"Ping",
    L"Refresh game list",
    L"Session Gone",
    L"Could not join session.",

    // ui_SelectCar.cpp
    L"(CHT)",

    // ui_Statistics.cpp
    L"Reset Scoreboards",
    L"Local Racer Ranks",
    L"Local Battler Ranks",
    L"Racer Name",
    L"Battler Name",
    L"Racer Details \200",
    L"Battle Mode Stats \202",
    L"Battler Details \200",
    L"Race Mode Stats \202",
    L"Champ Racer Stats",
    L"Champ Battler Stats",
    L"Battle Track Ranks",
    L"Racing Track Ranks",
    L"Track",
    L"Rank",
    L"Track Details \200",
    L"Race Track Ranks \202",
    L"Battle Track Ranks \202",
    L"Retrieving Stats...",
    L"Racing Summary",
    L"Battle Summary",
    L"No Battles Fought",
    L"%d%% (%d of %d)",
    L"Battles Won",
    L"Battles Finished",
    L"Battles Quit",
    L"Win/Loss Ratio",
    L"Total It Time",
    L"Total Battle Time",
    L"Not Yet Raced",
    L"Races Won",
    L"Races Finished",
    L"Races Quit",
    L"Total Racing Time",
    L"Average Speed",
    L"%5.1f KPH",
    L"Best Split Time",
    L"Best Split Car",
    L"Best Lap Time",
    L"Best Lap Car",
    L"Track Stats \200",

    // ui_WaitingRoom.cpp
    L"Friend Slots Filled:",
    L"Public Slots Filled:",
    L"Friend Slots Open:",
    L"Public Slots Open:",
    L"\202 Display Friends List",
    L"\203 Display Players List",

    // ui_Friends.cpp
    L"Your Friends List is empty",
    L"Updating Friends List...",
    L"Accept Invite",
    L"Decline Invite",
    L"Accept Friend Request",
    L"Decline Friend Request",
    L"Block Friend Requests from player",
    L"Send Invite",
    L"Cancel Invite",
    L"Join Friend",
    L"Remove Friend",
    L"Received invite to %s",
    L"Invite accepted",
    L"Invite declined",
    L"Sent invite",
    L"Received friend request",
    L"Sent friend request",
    L"Joinable in %s",
    L"Playing %s",
    L"Online in %s",
    L"%S is offline",
    L"This game session is no\nlonger available.",
    L"This will end the session\nAre you sure?",
    L"All future friend requests from\n%S\nhave been blocked.",
    L"This will remove your friend\nAre you sure?",
    L"Please insert the %s disc",
    L"Invite Accepted",

    // ui_Players.cpp
    L"Add Friend",
    L"Mute Player",
    L"Unmute Player",
    L"Feedback",
    L"Bad Nickname",
    L"Poor Gameplay",
    L"Spamming or Screaming",
    L"Threats or Harassment",
    L"Cursing or Lewdness",
    L"Good attitude",
    L"Great session",
    L"No other players",
    L"(talking)",
    L"Player is\nnot valid",
    L"%S has left the game",
    L"%S is in the game",
    L"Need Nickname",

    // ui_Podium.cpp
    L"Well done, you have mastered\nall Re-Volt Championships!",
    L"You'll have to do better than that\nto beat the Re-Volt Championships!",

    L"\200 Select",
    L"\200 Accept",
    L"\200 Yes",
    L"\200 Continue",
    L"\200 Select \201 Back",
    L"\201 Back",
    L"\201 No",
    L"\201 Cancel",
    L"\201 Abort",

    L"Select",
    L"Accept",
    L"Continue",
    L"Back",
    L"Cancel",

    L"Instructions",

    L"Default Controls",
    L"Right trigger:\n accelerate",
    L"Left trigger:\n reverse",
    L"steer",
    L"fire weapon",
    L"flip car",
    L"reset position",

    L"Weapons",
    L"Scroll",

    L"Shockwave", // horns // 0
        L"A blue ball of electricity shoots from the\n"
        L"front of the car, sending any cars in its path\n"
        L"flipping into the air.",

    L"Firework", // 1
        L"Fires a single rocket with limited homing\n"
        L"capabilities ahead of the car, continuing until\n"
        L"it hits a wall or an opponent.",

    L"Firework Pack", //2
        L"A pack of three rockets, individually working\n"
        L"as above.",

    L"Electro Pulse", //3
        L"An electric current hums over the car.\n"
        L"When another car is in close proximity,\n"
        L"a bolt connects the two and the victim's\n"
        L"power is temporarily cut.",
    
    L"Bomb", //4
        L"When the bomb is collected, the car's\n"
        L"antenna starts fizzing down like a fuse,\n"
        L"and the body of the vehicle turns black.\n"
        L"When the fuse reaches the bottom, the car\n"
        L"blows up! If another car is touched before\n"
        L"the fuse burns down, the bomb is transferred.",
    
    L"Oil Slick", //5
        L"A pool of oil is dropped on the floor\n"
        L"directly behind the car. Each tire that is\n"
        L"driven through this pool temporarily loses\n"
        L"traction.",

    L"Water Balloons (pack of 3)", //6
        L"A water-filled balloon is hurled from the car.\n"
        L"On impact with the floor (or opponent), it\n"
        L"bursts, affecting the grip of all vehicles in\n"
        L"the immediate vicinity.",

    L"Ball Bearing", //7
        L"An extremely heavy ball bearing is dropped\n"
        L"from the rear of the car, knocking anything in\n"
        L"its path out of the way.",
    
    L"Clone Pick Up", // lighting //8
        L"When activated, a lightning bolt identical to a\n"
        L"regular pick up is dropped on the floor. If any\n"
        L"other drivers are deceived and try to collect\n"
        L"the clone pick up it explodes on impact!",
    
    L"Turbo Battery", //9
        L"When activated, this briefly increases top\n"
        L"speed by 10%.",
    
    L"Global Pulse", // star //10
        L"Briefly robs other cars of power. When they\n"
        L"are collected, special things are unlocked in\n"
        L"the game.",

    L"Gamertag",
    L"Feedback for %S",
    L"Feedback sent",
    L"Please reconnect the controller to port %d and\npress START to continue",
    L"Reconnect controller",
};




// Array of string ptrs to be filled with localized text during LANG_LoadStrings()
WCHAR* gTitleScreen_Text[TEXT_NTEXTS];
static WCHAR* g_pStrings = NULL;

BOOL   g_bUseHardcodedStrings = TRUE;



//-------------------------------------------------------------------------------
// Name: LANG_LoadStrings()
// Desc: Loads localized strings
//-------------------------------------------------------------------------------
HRESULT LANG_LoadStrings( LANGUAGE dwLanguageID )
{
    //$TODO: This function lets you switch languages on the fly. This is only
    //       needed for development, but may not hurt just to leave it in. What
    //       could go, though, is the option for hardcoded strings.

    assert( dwLanguageID < NUM_LANGUAGES );

    if( g_bUseHardcodedStrings )
    {
        for( DWORD i=0; i<TEXT_NTEXTS; i++ )
        {
            gTitleScreen_Text[i] = g_pHardCodedStrings[i];
        }
    }
    else
    {
        // Open the localized strings file
        FILE* file = fopen( g_strLocalizedStringFiles[dwLanguageID], "rb" );
        assert( file != NULL );

        // Get the file size
        fseek( file, 0, SEEK_END );
        DWORD dwFileLength = ftell( file );
        fseek( file, 0, SEEK_SET );

        // Free the old language
        if( NULL != g_pStrings )
            delete g_pStrings;

        // Read in the strings
        g_pStrings = (WCHAR*)new BYTE[dwFileLength];
        fread( g_pStrings, 1, dwFileLength, file );
        fclose( file );

        WCHAR* pStrings = g_pStrings;

        for( DWORD i=0; i<TEXT_NTEXTS; i++ )
        {
            gTitleScreen_Text[i] = pStrings;

            // Skip to the next string
            while( *pStrings++ != L'\0' );
        }
    }

    /*
    // FOR DEBUGGING
    FILE* file = fopen( "d:\\dump.txt", "w" );
    for( DWORD i=0; i<TEXT_NTEXTS; i++ )
    {
       fprintf( file, "%d: %S\n", i, gTitleScreen_Text[i] );
    }
    fclose(file);
    */
    
    return S_OK;
}


