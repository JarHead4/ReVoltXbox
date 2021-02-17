//-----------------------------------------------------------------------------
// File: ui_MenuText.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef MENUTEXT_H
#define MENUTEXT_H

#include "revolt.h"
#include "typedefs.h"

#define LOC(x) x





//-------------------------------------------------------------------------------
// Language enumeration
//-------------------------------------------------------------------------------
enum LANGUAGE 
{
    LANGUAGE_ENGLISH,
    LANGUAGE_FRENCH,
    LANGUAGE_GERMAN,
    LANGUAGE_ITALIAN,
    LANGUAGE_SPANISH,
    LANGUAGE_JAPANESE,
    LANGUAGE_PORTUGUESE,
    LANGUAGE_KOREAN,
    LANGUAGE_CHINESE,

    NUM_LANGUAGES,
};




//-------------------------------------------------------------------------------
// Strings
//-------------------------------------------------------------------------------
enum TITLE_TEXT_STRING
{
    // Special ID for no text
    TEXT_NONE     = -1,

    // Menu Titles
    TEXT_REVOLT = 0,
    TEXT_PRESSSTARTTOPLAY,
    TEXT_LANGUAGE,
    TEXT_OPTIONS,
    TEXT_BESTTRIALTIMES,
    TEXT_PROGRESSTABLE,
    TEXT_GAMESETTINGS,
    TEXT_VIDEOSETTINGS,
    TEXT_RENDERSETTINGS,
    TEXT_AUDIOSETTINGS,
    TEXT_CONTROLLERSETTINGS,
    TEXT_MODE,
    TEXT_ENTERNICKNAME,
    TEXT_SELECTRACE,
    TEXT_SELECTCAR,
    TEXT_SELECTTRACK,
    TEXT_SUMMARY,
    TEXT_SINGLEPLAYER,
    TEXT_MULTIPLAYER,  //$TODO(cprince): fix naming conventions (eg, change MULTIPLAYER to SPLITSCREEN, and globally, change other uses of "multi" to "network" as appropriate)
    TEXT_SYSTEMLINK,
    TEXT_GAMETYPE,
    TEXT_WAITINGROOM,
    TEXT_SIGNIN,
    TEXT_SIGNOUT,
    TEXT_PLAYLIVE,
    TEXT_QUICKMATCH,
    TEXT_OPTIMATCH,
    TEXT_SELECTMATCH,
    TEXT_CREATEMATCH,
    TEXT_STATISTICS,
    TEXT_RACERSTATISTICS,
    TEXT_PLAYERS,
    TEXT_FRIENDS,
    TEXT_QUIT,
    TEXT_QUIT_GAME,

    TEXT_COULD_NOT_OPEN_REPLAY_FILE,
    TEXT_COULD_NOT_READ_REPLAY_FILE,
    TEXT_CORRUPT_CONTENT,
    TEXT_CORRUPT_CONTENT_REDOWNLOAD,
    TEXT_CORRUPT_FILES,
    TEXT_CORRUPT_FILES_MAY_HAVE_LOST,
    
    
    // Menus
    TEXT_CHOOSECUP,
    TEXT_CONNECTION,
    TEXT_MULTISELECT,

    // Select Race
    TEXT_RACE,
    TEXT_BATTLETAG,

    // Start Race
    TEXT_CHAMPIONSHIP,
    TEXT_SINGLERACE,
    TEXT_CLOCKWORKRACE,
    TEXT_TIMETRIAL,
    TEXT_PRACTICE,
    TEXT_TRAINING,
    TEXT_EDITMODE,
    TEXT_CALCSTATS,

    // Select Track
    TEXT_NUMLAPS,
    TEXT_LENGTH,
    TEXT_METERS,
    TEXT_METERS_ABREV,
    TEXT_TRACK_LOCKED,
    TEXT_TRACK_UNKNOWN,

    // Select Cup
    TEXT_BRONZECUP,
    TEXT_SILVERCUP,
    TEXT_GOLDCUP,
    TEXT_PLATINUMCUP,

    // Options
    TEXT_CREDITS,

    // Game Settings
    TEXT_NUMBEROFCARS,
    TEXT_RANDOM_CARS,
    TEXT_RANDOM_TRACK,
    TEXT_PICKUPS,
    TEXT_NUMBEROFLAPS,
    TEXT_TIMEOFBOMBTAG,
    TEXT_DIFFICULTY,
    TEXT_GHOSTTYPE,
    TEXT_UNITS,

    // Misc.
    TEXT_ON,
    TEXT_OFF,
    TEXT_YES,
    TEXT_NO,
    TEXT_EASY,
    TEXT_MEDIUM,
    TEXT_HARD,
    TEXT_EXTREME,

    // Video Settings
    TEXT_SCREENRES,
    TEXT_SHINYNESS,
    TEXT_SHADOWS,
    TEXT_BRIGHTNESS,
    TEXT_CONTRAST,
    TEXT_DRAWDIST,
    TEXT_REFLECTIONS,
    TEXT_INSTANCES,
    TEXT_SKIDMARKS,
    TEXT_LIGHTS,
    TEXT_ANTIALIAS,
    TEXT_VSYNC,
    TEXT_SHOW_FPS,
    TEXT_TEXTUREFILTER,
    TEXT_MIPMAP,
    TEXT_WIREFRAME,
    TEXT_PARTICLES,
    TEXT_SMOKE,

    // Audio Settings
    TEXT_MUSIC_ON,
    TEXT_SFXVOLUME,
    TEXT_MUSICVOLUME,
    TEXT_SFX_CHANNELS,

    // MultiPlayer
    TEXT_STARTGAME,
    TEXT_CREATEGAME,
    TEXT_JOINGAME,
    TEXT_MP_WAITFORLOBBY,
    TEXT_HOSTMACHINE,
    TEXT_LOBBYCONTINUE,
    TEXT_LOBBYQUIT,
    TEXT_NUMPLAYERS,
    TEXT_PACKET_TYPE,
    TEXT_PROTOCOL_TYPE,
    TEXT_SEARCHINGFORGAMES,
    TEXT_JOININGGAME,
    TEXT_PRESS_A_TOREFRESHGAMELIST,
    TEXT_STARTNEXTRACE,
    TEXT_NEWRACE,

    // Replay load/save
    TEXT_LOAD_REPLAY,
    TEXT_SAVE_REPLAY,

    // In-game menu
    TEXT_RESUME_RACE,
    TEXT_CONTINUE_CHAMP,
    TEXT_END_CHAMP,
    TEXT_GIVEUP_CHAMP,
    TEXT_RESTART_RACE,
    TEXT_RESTART_REPLAY,
    TEXT_QUIT_RACE,
    TEXT_VIEW_REPLAY,
    TEXT_COULD_NOT_OPEN_FILE,
    TEXT_SAVED_IN_REPLAY_FILE,
    TEXT_COULD_NOT_SAVE_REPLAY,

    // Controller
    TEXT_CONTROLLERSELECT,
    TEXT_CONTROLLERCONFIG,
    TEXT_STEERING_DEADZONE,
    TEXT_STEERING_RANGE,
    TEXT_STEERING,
    TEXT_CONTROLLERDEFAULT,
    TEXT_CONTROLLERCONFIGNUM,

    // Misc
    TEXT_DEMO_MESSAGE,
    TEXT_PLAYERNUM,
    TEXT_MODE_SIMULATION,
    TEXT_MODE_ARCADE,
    TEXT_MODE_CONSOLE,
    TEXT_MODE_KIDS,
    TEXT_MPH,
    TEXT_SCALEDMPH,
    TEXT_FPM,
    TEXT_KPH,
    TEXT_SCALEDKPH,

    // Win/lose sequence strings
    TEXT_YOU_FINISHED,
    TEXT_TRY_TO_COME_HIGHER_TO, 
    TEXT_UNLOCK_THE_NEXT_CHALLENGE,
    TEXT_BRONZE_CUP,
    TEXT_SILVER_CUP,
    TEXT_GOLD_CUP,
    TEXT_SPECIAL_CUP,
    TEXT_IS_UNLOCKED,
    TEXT_MIRRORED_TRACKS,
    TEXT_REVERSED_TRACKS,
    TEXT_HAVE_BEEN_UNLOCKED,
    TEXT_NEW_DELIVERY_OF_CARS,
    TEXT_YOU_HAVE_UNLOCKED,
    TEXT_NEW_CARS,
    TEXT_FIRST,
    TEXT_SECOND,
    TEXT_THIRD,

    TEXT_ARE_YOU_SURE,

    // Championship summary screen
    TEXT_CHAMP_FINISH_RACE1,
    TEXT_CHAMP_FINISH_RACE2,
    TEXT_CHAMP_FINISH_CHAMP1,
    TEXT_CHAMP_FINISH_CHAMP2,
    TEXT_CHAMP_FINISH_CHAMP3,
    TEXT_CHAMP_FINISH_CHAMP4,

    // MenuDraw.cpp
    TEXT_MIRROR_ABREV,
    TEXT_REVERSE_ABREV,
    TEXT_MIRROR_ABREV_PARENTHESIS,
    TEXT_REVERSE_ABREV_PARENTHESIS,

    // Select Car
    TEXT_CAR_CLASS,
    TEXT_CAR_RATING,
    TEXT_CAR_SPEED,
    TEXT_CAR_ACCEL,
    TEXT_CAR_WEIGHT,
    TEXT_CAR_TRANS,

    TEXT_CAR_CLASS_ELECTRIC,
    TEXT_CAR_CLASS_GLOW,
    TEXT_CAR_CLASS_SPECIAL,

    TEXT_CAR_RATING_ROOKIE,
    TEXT_CAR_RATING_AMATEUR,
    TEXT_CAR_RATING_ADVANCED,
    TEXT_CAR_RATING_SEMI_PRO,
    TEXT_CAR_RATING_PRO,

    TEXT_CAR_TRANS_4WD,
    TEXT_CAR_TRANS_RWD,
    TEXT_CAR_TRANS_FWD,

    TEXT_CAR_QUESTIONMARKS,

    // Frontend Summary
    TEXT_PLAYER,

    // Select Mode
    TEXT_SIMULATION_HELPSTRING,
    TEXT_ARCADE_HELPSTRING,
    TEXT_CONSOLE_HELPSTRING,
    TEXT_KIDS_HELPSTRING,

    // Controller Config
    TEXT_CONTROL_ACC,
    TEXT_CONTROL_REV,
    TEXT_CONTROL_FIRE,
    TEXT_CONTROL_FLIP,
    TEXT_CONTROL_REPOSITION,
    TEXT_CONTROL_PAUSE,

    // Draw Progress
    TEXT_PROGRESS_TRACKS,
    TEXT_PROGRESS_WONRACE_TOP,
    TEXT_PROGRESS_WONRACE_BOTTOM,
    TEXT_PROGRESS_NORMAL,
    TEXT_PROGRESS_REVERSE,
    TEXT_PROGRESS_MIRROR,
    TEXT_PROGRESS_STARS,

    // Load Screen Info
    TEXT_LOADSCREEN_GAMEMODE,
    TEXT_LOADSCREEN_STAGE,
    TEXT_LOADSCREEN_TRACK,
    TEXT_CAR,
    TEXT_LOADSCREEN_WONRACE,
    TEXT_LOADSCREEN_FOUNDSTAR,
    TEXT_LOADSCREEN_STARSCOLLECTED,
    TEXT_OF,
    TEXT_LOADSCREEN_BEATEN,
    TEXT_LOADSCREEN_CHALLENGETIME,

    TEXT_CONFIRM_GIVEUP_YOUHAVE_1_TRYLEFT,
    TEXT_CONFIRM_GIVEUP_YOUHAVE_N_TRIESLEFT,
    
    // Position
    TEXT_1ST,
    TEXT_2ND,
    TEXT_3RD,
    TEXT_4TH,
    TEXT_5TH,
    TEXT_6TH,
    TEXT_7TH,
    TEXT_8TH,
    TEXT_9TH,
    TEXT_10TH,
    TEXT_11TH,
    TEXT_12TH,
    TEXT_13TH,
    TEXT_14TH,
    TEXT_15TH,
    TEXT_16TH,

    // Championship result
    TEXT_CHAMP_QUALIFIED1,      
    TEXT_CHAMP_QUALIFIED2,         
    TEXT_CHAMP_FINISHED1,      
    TEXT_CHAMP_FINISHED2,
    TEXT_CHAMP_FAILURE,
    TEXT_CHAMP_ROWTITLE_CAR,
    TEXT_CHAMP_ROWTITLE_POINTS,

    // Stats
    TEXT_PANEL_BESTLAP,
    TEXT_PANEL_LASTLAP,
    TEXT_PANEL_LAPTIME,
    TEXT_PANEL_RACETIME,
    TEXT_PANEL_LAPCOUNTER,
    TEXT_PANEL_GHOSTSPLIT,
    TEXT_PANEL_GHOSTTIME,
    TEXT_PANEL_CHALLENGETIME,
    TEXT_PANEL_CHALLENGE_BEATEN,
    TEXT_PANEL_TAGTIME,
    TEXT_PANEL_BATTLE_RESULTS,
    TEXT_PANEL_RACE_RESULTS,

    // Confirmation strings
    TEXT_MENU_CONFIRM_QUIT,
    TEXT_MENU_CONFIRM_RESTART,
    TEXT_MENU_CONFIRM_GENERIC,
    TEXT_MENU_CONFIRM_REALLY,
    TEXT_MENU_CONFIRM_PRETEND,

    // Compete.cpp
    TEXT_COMPETE_CUP_DEFAULT,   
    TEXT_COMPETE_CUP_BRONZE,   
    TEXT_COMPETE_CUP_SILVER,   
    TEXT_COMPETE_CUP_GOLD,     
    TEXT_COMPETE_CUP_PLATINUM, 

    TEXT_COMPETE_GAMEOVER,

    // TitleScr.cpp
    TEXT_BONUS_NEWCARS,
    TEXT_BONUS_REVERSE,
    TEXT_BONUS_MIRROR,
    TEXT_BONUS_REVMIR,
    TEXT_BONUS_COCKWORK,

    // PC missed stuff!
    TEXT_CONTROL_LEFT,
    TEXT_CONTROL_RIGHT,
    TEXT_CONTROL_HORN,
    TEXT_TRYAGAIN,
    TEXT_DEMO,
    TEXT_LOADINGFRONTEND,
    TEXT_LOADINGDEMO,
    TEXT_LOADINGCREDITS,
    TEXT_LOADINGGAMEGAUGE,
    TEXT_RACERS,
    TEXT_CPUOPPONENT,
    TEXT_RANDOM,
    TEXT_STATUS,
    TEXT_READY,
    TEXT_NOTREADY,
    TEXT_HITTABTOSTART,
    TEXT_MULTIGAMETERMINATED,
    TEXT_WAITINGFOR,
    TEXT_WAITINGFORHOST,
    TEXT_HOST,
    TEXT_TRACK,
    TEXT_GAME,
    TEXT_WRONGVERSION,
    TEXT_FULL,
    TEXT_RECORD,
    TEXT_SCALED,
    TEXT_FINISHED,
    TEXT_STARTED,
    TEXT_OPEN,
    TEXT_REPLAY,
    TEXT_GALLERY,
    TEXT_LOCAL,
    TEXT_DOWNLOAD,

    TEXT_WAITINGROOM_GAMETYPE,
    TEXT_WAITINGROOM_MAXPLAYERS,
    TEXT_WAITINGROOM_TRACK,
    TEXT_WAITINGROOM_LAPS,

    TEXT_CREATEMATCH_GAMEMODE,
    TEXT_CREATEMATCH_NUMLAPS,
    TEXT_CREATEMATCH_INVITEMODE,
    TEXT_CREATEMATCH_TOTALPLAYERS,

    TEXT_MATCHTYPE_PUBLIC,
    TEXT_MATCHTYPE_PRIVATE,

    TEXT_OPTIMATCH_GAMEMODE,
    TEXT_OPTIMATCH_TRACK,
    TEXT_OPTIMATCH_ANY,

    TEXT_PLAYLIVE_NOMATCHESFOUND,

    TEXT_MYSTATISTICS,
    TEXT_FRIENDSSTATISTICS,
    TEXT_WORLDSBEST,

    TEXT_PRESS_ANY_BUTTON,
    TEXT_PRESS_START_TO_CONTINUE,
    TEXT_RESTARTING,
    TEXT_HOST_RESTARTING,
    TEXT_STARTINGGAME,

    // Network.cpp
    TEXT_NETWORK_NOTICE,

    TEXT_NETWORK_NO_SERVER,
    TEXT_NETWORK_SERVER_LOST_OR_BLOCKED,
    TEXT_NETWORK_CANNOT_JOIN,

    // Player.cpp
    TEXT_PLAYER_CAME_IN_NTH_PLACE,
    TEXT_PLAYERS_FINISH_TIME,

    // ui_ContentDownload.cpp
    TEXT_CONTENTDOWNLOAD_ONLINEUPDATE,
    TEXT_CONTENTDOWNLOAD_CHECKINGFORNEWCONTENT,
    TEXT_CONTENTDOWNLOAD_DOWNLOADINGNEWCONTENT,
    TEXT_CONTENTDOWNLOAD_B_CANCELUPDATE,
    TEXT_CONTENTDOWNLOAD_CONFIRM, // "Confirm"
    TEXT_CONTENTDOWNLOAD_COULDNOTREADNEWCARS, // "Could not read new cars"
    TEXT_CONTENTDOWNLOAD_UPDATECOMPLETE, // "Update complete"
    TEXT_CONTENTDOWNLOAD_ERROR, // "Error:\n"
    TEXT_ONLINESTORE, // "Online Store"
    TEXT_CONTENTDOWNLOAD_DOWNLOAD_COMPLETE, // "Download Complete"
    TEXT_CONTENTDOWNLOAD_FREE, // "Free"
    TEXT_CONTENTDOWNLOAD_PRICE, // "Price: %s"
    TEXT_CONTENTDOWNLOAD_TEMPNOCHARGE, //"*You will not actually be charged.*"
    TEXT_CONTENTDOWNLOAD_NONTERMINATING, // "Non terminating."
    TEXT_CONTENTDOWNLOAD_DURATION,  //"Offer Duration: %u Months."
    TEXT_CONTENTDOWNLOAD_FREEMONTHS, //"The first %u months are Free!"
    TEXT_CONTENTDOWNLOAD_ONE_TIME, //L"This is a one time change."
    TEXT_CONTENTDOWNLOAD_MONTHLY, //L"You will be charged monthly."
    TEXT_CONTENTDOWNLOAD_QUARTERLY, //L"You will be chareged quarterly."
    TEXT_CONTENTDOWNLOAD_BIANNUALLY, //L"You will be charged biannually."
    TEXT_CONTENTDOWNLOAD_ANNUALLY, //L"You will be charged annually."
    TEXT_CONTENTDOWNLOAD_CANCEL_ANY_TIME, //L"You may cancel at any time."
    TEXT_CONTENTDOWNLOAD_ALREADY_SUB, // L"Already Subscribed."
    TEXT_CONTENTDOWNLOAD_ALREADY_OWN, // L"Already owned, download is free."
    TEXT_SUB_BACK,  //L"\200 Subscribe, \201 Back"
    TEXT_CANELSUB_BACK, //L"\200 Cancel subscripton, \201 Back"
    TEXT_DOWNLOAD_BACK, // L"\200 Download, \201 Back"
    TEXT_PURCHASE_BACK,//L"\200 Purchase, \201 Back"
    TEXT_CONTENTDOWNLOAD_CANNOT_PLAY_WITHOUT_UPDATING, 
    TEXT_CONTENTDOWNLOAD_NO_NEW_CONTENT, //L"No new content is available."
    TEXT_CONTENTDOWNLOAD_GETTING_DETAILS, //L"Getting Content Details"
    TEXT_CONTENTDOWNLOAD_HAVE_FUN, //L"Have Fun!"
    TEXT_CONTENTDOWNLOAD_SUB_CANCELLED, //L"Subscription cancelled."
    TEXT_CONTENTDOWNLOAD_PURCHASING_CONTENT, //L"Purchasing content."
    TEXT_DONT_TURN_OFF, //L"Please do not turn off your Xbox."
    TEXT_CONTENTDOWNLOAD_PURCHASING_SUB, //L"Purchasing subscription."
    TEXT_CONTENTDOWNLOAD_CANCELING_SUB, //L"Canceling subscription."
    TEXT_CONTINUE_CANCEL, //L"\200 Continue, \201 Cancel"
    TEXT_BILLING_PERMISSIONS_DISABLED,
    TEXT_CONFIRM_CANCEL_SUB,
    TEXT_CONFIRM_SUB,
    TEXT_CONFIRM_PURCHASE,

    // ui_EnterName.cpp
    TEXT_ENTERNAME_MORELETTERS, // "More letters and symbols"
    TEXT_ENTERNAME_NAMEBEINGVERIFIED, // "Name being verified..."
    TEXT_ENTERNAME_NICKNAME, // "Nickname"
    TEXT_ENTERNAME_NICKNAMEINAPPROPRIATE, // "The nickname you entered is\nnot appropriate!"

    // ui_LiveSignOn.cpp
    TEXT_LIVESIGNIN,
    TEXT_LIVESIGNIN_GUESTOFPLAYER,
    TEXT_LIVESIGNIN_PRESS_A_TOSIGNIN,
    TEXT_LIVESIGNIN_SELECTACCOUNT,
    TEXT_LIVESIGNIN_CREATEACCOUNT,
    TEXT_LIVESIGNIN_SIGNINASGUEST,
    TEXT_LIVESIGNIN_ERROR_NOACCOUNTS,
    TEXT_LIVESIGNIN_DOYOUWANTTOSTARTTHEDASH,
    TEXT_LIVESIGNIN_GOTOONLINEDASHBOARD,
    TEXT_LIVESIGNIN_ENTERPASSCODE,
    TEXT_LIVESIGNIN_WARNING_REENTERPASSCODE,
    TEXT_LIVESIGNIN_WARNING_DUPLICATEPLAYER,
    TEXT_LIVESIGNIN_WARNING_NOHOSTFORGUEST,
    TEXT_LIVESIGNIN_WAITINGFOROTHERPLAYERS,
    TEXT_LIVESIGNIN_NOMOREUSERS,
    TEXT_LIVESIGNIN_SINGLEPLAYER_SIGNIN,
    TEXT_LIVESIGNIN_MULTIPLAYER_SIGNIN,
    TEXT_LIVESIGNIN_YOUARESIGNEDIN,
    TEXT_LIVESIGNIN_SIGNINFAILED,
    TEXT_LIVESIGNIN_ERROR_HOSTHASERRORS,
    TEXT_LIVESIGNIN_ERROR_NONETWORKCONNECTION,
    TEXT_LIVESIGNIN_ERROR_CANNOTACCESSSERVICE,
    TEXT_LIVESIGNIN_ERROR_UPDATEREQUIRED,
    TEXT_LIVESIGNIN_ERROR_PROBLEMS,
    TEXT_LIVESIGNIN_ERROR_SERVERSTOOBUSY,
    TEXT_LIVESIGNIN_ERROR_ACCTREQUIRESMGMT,
    TEXT_LIVESIGNIN_ERROR_UNKNOWNPROBLEM,
    TEXT_LIVESIGNIN_YOUHAVEMESSAGES,
    TEXT_LIVESIGNIN_CHANGEACCOUNT,
    TEXT_LIVESIGNIN_BEGININSTALLATION,
    TEXT_LIVESIGNIN_BACKTOMAINMENU,
    TEXT_LIVESIGNIN_REBOOTTODASH,
    TEXT_LIVESIGNIN_NETWORKTROUBLESHOOTER,
    TEXT_LIVESIGNIN_GETMESSAGES,
    TEXT_LIVESIGNIN_LETSPLAY,

    // ui_Options.cpp
    TEXT_VOICEMASK,
    TEXT_VOICEMASK_NONE,
    TEXT_VOICEMASK_DARKMASTER,
    TEXT_VOICEMASK_CARTOON,
    TEXT_VOICEMASK_BIGGUY,
    TEXT_VOICEMASK_CHILD,
    TEXT_VOICEMASK_ROBOT,
    TEXT_VOICEMASK_WHISPER,

    // ui_PlayLive.cpp
    TEXT_PING, // "Ping"
    TEXT_REFRESHGAMELIST, // "Refresh game list"
    TEXT_SESSION_GONE,
    TEXT_COULD_NOT_JOIN,

    // ui_SelectCar.cpp
    TEXT_CHEAT_SUFFIX, // "(CHT)"

    // ui_Statistics.cpp
    TEXT_STATS_XONLINESTATRESETFORUSER, // "XOnlineStatReset for user\n"
    TEXT_STATS_LOCALRACERRANKS, // "Local Racer Ranks"
    TEXT_STATS_LOCALBATTLERRANKS, // "Local Battler Ranks"
    TEXT_STATS_RACERNAME, // "Racer Name"
    TEXT_STATS_BATTLERNAME, // "Battler Name"
    TEXT_STATS_A_RACERDETAILS, // "Racer Details \200"
    TEXT_STATS_X_BATTLEMODESTATS, // "Battle Mode Stats \202"
    TEXT_STATS_A_BATTLERDETAILS, // "Battler Details \200"
    TEXT_STATS_X_RACEMODESTATS, // "Race Mode Stats \202"
    TEXT_STATS_CHAMPRACERSTATS, // "Champ Racer Stats"
    TEXT_STATS_CHAMPBATTLERSTATS, // "Champ Battler Stats"
    TEXT_STATS_BATTLETRACKRANKS, // "Battle Track Ranks"
    TEXT_STATS_RACINGTRACKRANKS, // "Racing Track Ranks"
    TEXT_STATS_TRACK, // "Track"
    TEXT_STATS_RANK, // "Rank"
    TEXT_STATS_A_TRACKDETAILS, // "Track Details \200"
    TEXT_STATS_X_RACETRACKRANKS, // "Race Track Ranks \202"
    TEXT_STATS_X_BATTLETRACKRANKS, // "Battle Track Ranks \202"
    TEXT_STATS_RETRIEVINGDATA, // "Retrieving Data..."
    TEXT_STATS_RACINGSUMMARY, // "Racing Summary"
    TEXT_STATS_BATTLESUMMARY, // "Battle Summary"
    TEXT_STATS_NOBATTLESFOUGHT, // "No Battles Fought"
    TEXT_STATS_PERCENT_N_OF_N, // "%d%% (%d of %d)"
    TEXT_STATS_BATTLESWON, // "Battles Won"
    TEXT_STATS_BATTLESCOMPLETED, // "Battles Completed"
    TEXT_STATS_BATTLESABANDONED, // "Battles Abandoned"
    TEXT_STATS_WINLOSSRATIO, // "Win/Loss Ratio"
    TEXT_STATS_TOTALITTIME, // "Total It Time"
    TEXT_STATS_TOTALBATTLETIME, // "Total Battle Time"
    TEXT_STATS_NOTYETRACED, // "Not Yet Raced"
    TEXT_STATS_RACESWON, // "Races Won"
    TEXT_STATS_RACESCOMPLETED, // "Races Completed"
    TEXT_STATS_RACESABANDONED, // "Races Abandoned"
    TEXT_STATS_TOTALRACINGTIME, // "Total Racing Time"
    TEXT_STATS_AVERAGESPEED, // "Average Speed"
    TEXT_STATS_AVERAGE_KPH, // "%5.1f KPH";
    TEXT_STATS_BESTSPLITTIME, // "Best Split Time"
    TEXT_STATS_BESTSPLITCAR, // "Best Split Car"
    TEXT_STATS_BESTLAPTIME, // "Best Lap Time"
    TEXT_STATS_BESTLAPCAR, // "Best Lap Car"
    TEXT_STATS_A_TRACKSTATS, // "Track Stats \200"

    // ui_WaitingRoom.cpp
    TEXT_LOBBY_PRIVATESLOTSFILLED, // "Private Slots Filled:"
    TEXT_LOBBY_PUBLICSLOTSFILLED, // "Public Slots Filled:"
    TEXT_LOBBY_PRIVATESLOTSOPEN, // "Private Slots Open:"
    TEXT_LOBBY_PUBLICSLOTSOPEN, // "Public Slots Open:"
    TEXT_LOBBY_X_DISPLAYFRIENDS, // "\202 Display Friends List"
    TEXT_LOBBY_Y_DISPLAYPLAYERS, // "\203 Display Players List"

    // ui_Friends.cpp
    TEXT_FRIENDS_LISTISEMPTY, // "Your friends list is empty"
    TEXT_FRIENDS_ENUMERATING, // "Enumerating friends..."
    TEXT_FRIENDS_ACCEPTINVITATION, // "Accept Invitation"
    TEXT_FRIENDS_DECLINEINVITATION, // "Decline Invitation"
    TEXT_FRIENDS_ACCEPTREQUEST, // "Accept Friend Request"
    TEXT_FRIENDS_DECLINEREQUEST, // "Decline Friend Request"
    TEXT_FRIENDS_BLOCKREQUESTSFROMPLAYER, // "Block Friend Requests from player"
    TEXT_FRIENDS_SENDINVITATION, // "Send Invitation"
    TEXT_FRIENDS_REVOKEINVITATION, // "Revoke Invitation"
    TEXT_FRIENDS_JOINFRIEND, // "Join Friend"
    TEXT_FRIENDS_REMOVEFRIEND, // "Remove Friend"
    TEXT_FRIENDS_RECEIVEDINVITATION, // "Received invitation to %s"
    TEXT_FRIENDS_INVITATIONACCEPTED, // "Invitation accepted"
    TEXT_FRIENDS_INVITATIONDECLINED, // "Invitation declined"
    TEXT_FRIENDS_SENTINVITATION, // "Sent invitation"
    TEXT_FRIENDS_RECEIVEDFRIENDREQUEST, // "Received friend request"
    TEXT_FRIENDS_SENTFRIENDREQUEST, // "Sent friend request"
    TEXT_FRIENDS_JOINABLE, // "Joinable in %s"
    TEXT_FRIENDS_PLAYING, // "Playing %s"
    TEXT_FRIENDS_ONLINE, // "Online in %s"
    TEXT_FRIENDS_OFFLINE, // "%S is offline"
    TEXT_FRIENDS_BADSESSIONID, // "Couldn't accept\nBogus session ID",
    TEXT_FRIENDS_END_SESSION,
    TEXT_FRIENDS_REQUEST_FROM_BLOCKED,
    TEXT_FRIENDS_REMOVE_CONFIRM,
    TEXT_FRIENDS_INSERT_GAMEDISK,
    TEXT_FRIENDS_INVITE_ACCEPTED,

    // ui_Players.cpp
    TEXT_PLAYERS_ADDFRIEND,
    TEXT_PLAYERS_MUTEPLAYER,
    TEXT_PLAYERS_UNMUTEPLAYER,
    TEXT_PLAYERS_SENDFEEDBACK,
    TEXT_PLAYERS_OFFENSIVENICKNAME,
    TEXT_PLAYERS_INAPPROPRIATEGAMEPLAY,
    TEXT_PLAYERS_VERBALLYABUSIVE,
    TEXT_PLAYERS_HARASSMENT,
    TEXT_PLAYERS_LEWDNESS,
    TEXT_PLAYERS_GOODATTITUDE,
    TEXT_PLAYERS_GOODSESSION,
    TEXT_PLAYERS_NOOTHERPLAYERS,
    TEXT_PLAYERS_TALKING_SUFFIX,
    TEXT_PLAYERS_PLAYERNOTVALID,
    TEXT_PLAYERS_LEFT_GAME,
    TEXT_PLAYERS_IN_GAME,
    TEXT_PLAYERS_NEED_NICKNAME,

    // ui_Podium.cpp
    TEXT_PODIUM_FINISHED,
    TEXT_PODIUM_DOBETTER,

    TEXT_BUTTON_A_SELECT,
    TEXT_BUTTON_A_ACCEPT,
    TEXT_BUTTON_A_YES,
    TEXT_BUTTON_A_CONTINUE,
    TEXT_BUTTON_A_SELECT_B_BACK,
    TEXT_BUTTON_B_BACK,
    TEXT_BUTTON_B_NO,
    TEXT_BUTTON_B_CANCEL,
    TEXT_BUTTON_B_ABORT,

    TEXT_SELECT,
    TEXT_ACCEPT,
    TEXT_CONTINUE,
    TEXT_BACK,
    TEXT_CANCEL,

    TEXT_REVOLT_HELP,

    TEXT_DEFAULTCONTROLS,
    TEXT_HELP_RIGHTTRIGGER,
    TEXT_HELP_LEFTTRIGGER,
    TEXT_HELP_DPAD,
    TEXT_HELP_ABUTTON,
    TEXT_HELP_BBUTTON,
    TEXT_HELP_YBUTTON,

    TEXT_WEAPONS,
    TEXT_SCROLL,
    TEXT_SHOCKWAVE,
    TEXT_HELP_SHOCKWAVE,
    TEXT_FIREWORK,
    TEXT_HELP_FIREWORK,
    TEXT_FIREWORKPACK,
    TEXT_HELP_FIREWORKPACK,
    TEXT_ELECTROPULSE,
    TEXT_HELP_ELECTROPULSE,
    TEXT_BOMB,
    TEXT_HELP_BOMB,
    TEXT_OILSLICK,
    TEXT_HELP_OILSLICK,
    TEXT_WATERBALLOONS,
    TEXT_HELP_WATERBALLONS,
    TEXT_BALLBEARING,
    TEXT_HELP_BALLBEARING,
    TEXT_CLONEPICKUP,
    TEXT_HELP_CLONEPICKUP,
    TEXT_TURBOBATTERY,
    TEXT_HELP_TURBOBATTERY,
    TEXT_GLOBALPULSE,
    TEXT_HELP_GLOBALPULSE,

    TEXT_GAMERTAG,
    TEXT_FEEDBACK_FOR,
    TEXT_FEEDBACK_SENT,
    TEXT_RECONNECT_CONTROLLER_TO,
    TEXT_RECONNECT_CONTROLLER,
                 
    TEXT_NTEXTS
};




//-------------------------------------------------------------------------------
// Displayable text
//-------------------------------------------------------------------------------
extern WCHAR* gTitleScreen_Text[];

#define TEXT_TABLE(index) (gTitleScreen_Text[(index)])
#define TEXT_TABLE_NULL(index) (index >= 0?gTitleScreen_Text[(index)] : NULL)



//-------------------------------------------------------------------------------
// Languages and localized strings
//-------------------------------------------------------------------------------
extern WCHAR* g_strLanguageNames[NUM_LANGUAGES];

extern BOOL   g_bUseHardcodedStrings;

extern HRESULT LANG_LoadStrings( LANGUAGE dwLanguageID );




#endif // MENUTEXT_H

