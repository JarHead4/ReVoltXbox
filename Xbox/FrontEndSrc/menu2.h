//-----------------------------------------------------------------------------
// File: Menu2.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef MENU2_H
#define MENU2_H

// Max number of items per menu
#define MENU_MAX_ITEMS      16

#define MENU_SCREEN_WIDTH   640
#define MENU_SCREEN_HEIGHT  480
#define MENU_START_OFFSET_X 64
#define MENU_START_OFFSET_Y 64

#define MENU_DEFAULT            MENU_ENTER_LEFT | MENU_EXIT_BOTTOM

enum {
    SPRU_COL_BLUE,
    SPRU_COL_RED,
    SPRU_COL_YELLOW,
    SPRU_COL_RANDOM,
    SPRU_COL_KEEP,

    SPRU_COL_NTYPES
};

// menu move vel

#define MENU_MOVE_VEL 1500.0f

// Colour scheme
#define MENU_TEXT_RGB_NORMAL        0xffffff
#define MENU_TEXT_RGB_HILITE        0xff7722
#define MENU_TEXT_RGB_MIDLITE       0x803010
#define MENU_TEXT_RGB_LOLITE        0x444444
#define MENU_TEXT_RGB_CHOICE        0xffff00
#define MENU_TEXT_RGB_NOTCHOICE     0x888800

#define MENU_TEXT_RGB_TITLE         0xffffffff

#define MENU_SPRU_RGB               0xffffffff
#define MENU_SPRU_RGB_BACK          0xb0181818
#define MENU_SPRU_RGB_BACK_SOLID    0xFFB1373D


// Menu test dimensions and margins
#define MENU_TEXT_WIDTH     8
#define MENU_TEXT_HEIGHT    12
#define MENU_TEXT_VSKIP     4
#define MENU_TEXT_HSKIP     4
#define MENU_TEXT_GAP       32

// Menu text movement types
#define MENU_ENTER_LEFT     0x1
#define MENU_EXIT_LEFT      0x2
#define MENU_ENTER_RIGHT    0x4
#define MENU_EXIT_RIGHT     0x8
#define MENU_ENTER_TOP      0x10
#define MENU_EXIT_TOP       0x20
#define MENU_ENTER_BOTTOM   0x40
#define MENU_EXIT_BOTTOM    0x80
#define MENU_CENTRE_X       0x100
#define MENU_CENTRE_Y       0x200

#define MENU_DATA_WIDTH_TEXT    Real(220)
#define MENU_DATA_WIDTH_INT     Real(64)
#define MENU_DATA_WIDTH_BOOL    Real(64)
#define MENU_DATA_WIDTH_SCREEN  Real(600)
#define MENU_DATA_WIDTH_NAME    Real(200)
#define MENU_DATA_WIDTH_SLIDER  Real(50)

// Menu navigation flags
#define MENU_FLAG_ADVANCE       0x1
#define MENU_FLAG_GOBACK        0x2


// Menu State enum
enum MENU_STATE
{
    MENU_STATE_PROCESSING,
    MENU_STATE_LEAVING,
    MENU_STATE_ENTERING,
    MENU_STATE_OFFSCREEN,

    NUM_MENU_STATES
};

// Menu input enum
enum MenuInputTypeEnum 
{
    MENU_INPUT_NONE = -1,
    MENU_INPUT_UP   = 0,
    MENU_INPUT_DOWN,
    MENU_INPUT_LEFT,
    MENU_INPUT_RIGHT,
    MENU_INPUT_BACK,
    MENU_INPUT_SELECT,
    NUM_MENU_INPUT_TYPES
};

// Title bar image enum
 enum MENU_IMAGE
 {
    MENU_IMAGE_NONE      = -1,
    MENU_IMAGE_BESTTIMES = 0,
    MENU_IMAGE_CARSELECT,
    MENU_IMAGE_SELECTTRACK,
    MENU_IMAGE_CUPSELECT,
    MENU_IMAGE_SELECTRACE,
    MENU_IMAGE_TRAINING,
    MENU_IMAGE_MULTI,
    MENU_IMAGE_OPTIONS,
    MENU_IMAGE_NAMEENTER,

    MENU_NIMAGES
};

// Forward type definitions
struct MENU_HEADER;
struct MENU;
struct MENU_ITEM;

// Control function definitions
typedef BOOL (*MENU_INPUT_HANDLER)( DWORD input, MENU_HEADER *menuHeader, MENU* menu, MENU_ITEM* menuItem );
typedef void (*MENU_CREATE_FUNC)( MENU_HEADER* menuHeader, MENU* menu );
typedef void (*MENU_INPUT_ACTION)( MENU_HEADER* menuHeader, MENU* menu, MENU_ITEM* menuItem );
typedef void (*MENU_ITEM_DRAW)( MENU_HEADER* menuHeader, MENU *menu, MENU_ITEM* menuItem, int itemIndex );
typedef void MENU_DATA;




//-----------------------------------------------------------------------------
// Slider data structures
//-----------------------------------------------------------------------------
struct SLIDER_DATA_LONG
{
    long* pData;
    long  Min;
    long  Max;
    long  Step;
    BOOL  bDrawSlider;
	BOOL  bDrawText;
};

struct SLIDER_DATA_ULONG
{
    DWORD* pData;
    DWORD  Min;
    DWORD  Max;
    DWORD  Step;
    BOOL   bDrawSlider;
	BOOL   bDrawText;
};

struct SLIDER_DATA_STRING_TABLE
{
    CHAR**  pstrStringTable;
    DWORD   dwCurrentString;
    DWORD   dwNumStrings;
};




//-----------------------------------------------------------------------------
// Menu Item Data
//-----------------------------------------------------------------------------

// Menu Item selectability
#define MENU_ITEM_SELECTABLE        0x1
#define MENU_ITEM_ACTIVE            0x2

struct MENU_ITEM
{
    SHORT               TextIndex;                      // Index into text array for item name
    FLOAT               DataWidth;                      // Width in pixels required to draw data
    MENU_DATA*          pData;                          // Data associated with item
    MENU_ITEM_DRAW      DrawFunc;                       // Function to draw item if necessary
    MENU_INPUT_HANDLER  pfnInputHandler;                // Input handler
    long                ActiveFlags;                    // Whether Item is currently active
    MENU_INPUT_ACTION   InputAction[NUM_MENU_INPUT_TYPES]; // Action funtions when item is selected and inputs pressed
};




//-----------------------------------------------------------------------------
// Menu Data: info for each menu page
//-----------------------------------------------------------------------------
struct MENU
{
    long                TextIndex;
    MENU_IMAGE          ImageIndex;
    short               CamPosIndex;                    // Index into camera pos/orientation array
    unsigned long       Type;
    long                SpruColIndex;
    MENU_CREATE_FUNC    CreateFunc;
    FLOAT               XPos, YPos;
    long                CurrentItemIndex;
    MENU*			    pParentMenu;
};




//-----------------------------------------------------------------------------
// Menu Header Data: info for current menu page. Set up from the
// current menu's CreateFunc.
//-----------------------------------------------------------------------------
struct MENU_HEADER
{
    MENU*               pMenu;                          // Menu Currently displayed
    MENU*               pNextMenu;                      // Next menu (when != Menu, the m,enu changes)
    MENU_ITEM*          pMenuItem[MENU_MAX_ITEMS];      // Pointers to menu item data
    short               NMenuItems;                     // Number of Menu Items
    REAL                XPos, YPos, XSize, YSize;       //  Current Screen Position and dimensions
    REAL                DestXPos, DestYPos, DestXSize, DestYSize;   // Desired position and size
    REAL                ItemTextWidth;                  // max width of item text
    REAL                ItemDataWidth;                  // max width of item data
    long                SpruColIndex;                   // Colour index of the spru
    MENU_STATE          State;
    char*               strTitle;                         // Menu title

    long                NavFlags;                       // Flags to say which navigation icons should be shown
    REAL                NavXPos;                        // Position of the navigation icons
    REAL                NavYPos;
};




//-----------------------------------------------------------------------------
// External prototypes
//-----------------------------------------------------------------------------
extern void AddMenuItem(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void ClearMenuHeader(MENU_HEADER *menuHeader);
extern void InitMenuHeader(MENU_HEADER *menuHeader);
extern void SetNextMenu(MENU_HEADER *menuHeader, MENU *menu);
extern void SelectPreviousMenuItem(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void SelectNextMenuItem(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void ToggleMenuData(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void ToggleMenuDataOn(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void ToggleMenuDataOff(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void MenuGoBack(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void MenuGoForward(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void MoveResizeMenu(MENU_HEADER *menuHeader);
extern void ProcessMenuInput(MENU_HEADER *menuHeader);
extern void DrawMenu(MENU_HEADER *menuHeader);
extern BOOL HandleMenus();
extern void IncreaseSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void DecreaseSliderDataLong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void IncreaseSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void DecreaseSliderDataULong(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void IncreaseSliderDataStringTable(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
extern void DecreaseSliderDataStringTable(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

void InitMenuMessage( FLOAT fTimeOut );
void SetMenuMessage( char* strMessage );
void SetBonusMenuMessage();
void SetTitleScreenInitialMenu();

extern BOOL MenuDefHandler( DWORD input, MENU_HEADER* pMenuHeader, MENU* pMenu, 
					 MENU_ITEM* pMenuItem );


//-----------------------------------------------------------------------------
// External Variables
//-----------------------------------------------------------------------------
extern MENU_HEADER  gMenuHeader;
extern bool         gChangeParentMenu;
extern bool         g_bMenuDrawMenuBox;

extern long  InitialMenuMessage;
extern int   InitialMenuMessageCount;
extern int   InitialMenuMessageWidth;
extern char  InitialMenuMessageString[256];
extern char* InitialMenuMessageLines[10];
extern FLOAT InitialMenuMessageTimer;
extern FLOAT InitialMenuMessageMaxTime;

#endif // MENU2_H

