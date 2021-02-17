//-----------------------------------------------------------------------------
// File: ui_Menu.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_MENU_H
#define UI_MENU_H

// Max number of items per menu
#define MENU_MAX_ITEMS      30 // $MD: changed

#define MENU_SCREEN_WIDTH   640
#define MENU_SCREEN_HEIGHT  480
#define MENU_START_OFFSET_X 64
#define MENU_START_OFFSET_Y 64

enum 
{
    SPRU_COL_BLUE,
    SPRU_COL_RED,
    SPRU_COL_YELLOW,
    SPRU_COL_RANDOM,
    SPRU_COL_KEEP,

    SPRU_COL_NTYPES
};

// menu move vel

#define MENU_MOVE_VEL 1500.0f

// Color scheme
// Note: We are using NTSC-safe colors
#define MENU_COLOR_WHITE            0x00ebebeb
#define MENU_COLOR_LIGHTGRAY        0x00b4b4b4
#define MENU_COLOR_GRAY             0x007d7d7d
#define MENU_COLOR_DARKGRAY         0x00464646
#define MENU_COLOR_BLACK            0x00101010
#define MENU_COLOR_RED              0x00eb1010
#define MENU_COLOR_DIMRED           0x007d1010
#define MENU_COLOR_GREEN            0x0010eb10
#define MENU_COLOR_DIMGREEN         0x00107d10
#define MENU_COLOR_BLUE             0x001010eb
#define MENU_COLOR_MED_BLUE         0x005050eb
#define MENU_COLOR_DIMBLUE          0x0010107d
#define MENU_COLOR_YELLOW           0x00ebeb10
#define MENU_COLOR_DIMYELLOW        0x007d7d10
#define MENU_COLOR_ORANGE           0x00eb7d20
#define MENU_COLOR_DIMORANGE        0x007d3010
#define MENU_COLOR_CYAN             0x0010ebeb

#define MENU_COLOR_OPAQUE           0xff000000
#define MENU_COLOR_SEETHROUGH       0x44000000

#define MENU_TEXT_RGB_NORMAL        (MENU_COLOR_OPAQUE|MENU_COLOR_WHITE)
#define MENU_TEXT_RGB_HILITE        (MENU_COLOR_OPAQUE|MENU_COLOR_ORANGE)
#define MENU_TEXT_RGB_MIDLITE       (MENU_COLOR_OPAQUE|MENU_COLOR_DIMORANGE)
#define MENU_TEXT_RGB_LOLITE        (MENU_COLOR_OPAQUE|MENU_COLOR_DARKGRAY)
#define MENU_TEXT_RGB_CHOICE        (MENU_COLOR_OPAQUE|MENU_COLOR_YELLOW)
#define MENU_TEXT_RGB_NOTCHOICE     (MENU_COLOR_OPAQUE|MENU_COLOR_DIMYELLOW)

#define MENU_TEXT_RGB_TITLE         (MENU_COLOR_OPAQUE|MENU_COLOR_WHITE)

// Menu test dimensions and margins
#define MENU_TEXT_WIDTH     8
#define MENU_TEXT_HEIGHT    18
#define MENU_TEXT_VSKIP     2
#define MENU_TEXT_HSKIP     2
#define MENU_TEXT_GAP       32

#define MENU_BORDER_WIDTH   16
#define MENU_BORDER_HEIGHT  16

// Menu text movement types
#define MENU_ENTER_LEFT     0x0001
#define MENU_EXIT_LEFT      0x0002
#define MENU_ENTER_RIGHT    0x0004
#define MENU_EXIT_RIGHT     0x0008
#define MENU_ENTER_TOP      0x0010
#define MENU_EXIT_TOP       0x0020
#define MENU_ENTER_BOTTOM   0x0040
#define MENU_EXIT_BOTTOM    0x0080
#define MENU_CENTRE_X       0x0100
#define MENU_CENTRE_Y       0x0200
#define MENU_NOBOX          0x0400
#define MENU_PAD_FOR_ARROWS 0x0800

#define MENU_DEFAULT        (MENU_ENTER_LEFT | MENU_EXIT_BOTTOM)

#define MENU_DATA_WIDTH_TEXT    220.0f
#define MENU_DATA_WIDTH_INT      64.0f
#define MENU_DATA_WIDTH_BOOL     64.0f
#define MENU_DATA_WIDTH_SCREEN  600.0f
#define MENU_DATA_WIDTH_NAME    200.0f
#define MENU_DATA_WIDTH_SLIDER   50.0f

// Menu State enum
enum MENU_STATE
{
    MENU_STATE_PROCESSING,
    MENU_STATE_LEAVING,
    MENU_STATE_ENTERING,
    MENU_STATE_OFFSCREEN,

    MENU_NSTATES
};

// Menu input enum
enum MenuInputTypeEnum
{
    MENU_INPUT_NONE = -1,
    MENU_INPUT_UP = 0,
    MENU_INPUT_DOWN,
    MENU_INPUT_LEFT,
    MENU_INPUT_RIGHT,
    MENU_INPUT_BACK,
    MENU_INPUT_SELECT,
    MENU_INPUT_X,
    MENU_INPUT_Y,

#ifdef _DEBUG
    MENU_INPUT_DEBUG_BLACK,
    MENU_INPUT_DEBUG_WHITE,
#endif

    MENU_INPUT_NTYPES
};

// Title bar image enum
enum MENU_IMAGE
{
    MENU_IMAGE_NONE      = -1,
    MENU_IMAGE_BESTTIMES = 0,
    MENU_IMAGE_CARSELECT,
    MENU_IMAGE_TRACKSELECT,
    MENU_IMAGE_CUPSELECT,
    MENU_IMAGE_STARTRACE,
    MENU_IMAGE_TRAINING,
    MENU_IMAGE_MULTI,
    MENU_IMAGE_OPTIONS,
    MENU_IMAGE_NAMEENTER,

    MENU_NIMAGES
};

// Forward type definitions
class MENU_HEADER;
struct MENU;
struct MENU_ITEM;

// Control function definitions
typedef void (*MENU_CREATE_FUNC)( MENU_HEADER* pMenuHeader, MENU* pMenu );
typedef BOOL (*MENU_INPUT_FUNC)( MENU_HEADER* pMenuHeader, DWORD dwInput );
typedef void (*MENU_DRAW_FUNC)( MENU_HEADER* pMenuHeader, MENU* pMenu );

typedef BOOL (*MENUITEM_INPUT_FUNC)( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
typedef void (*MENUITEM_DRAW_FUNC)( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex );


////////////////////////////////////////////////////////////////
//
// Slider data for "long" data type
//
////////////////////////////////////////////////////////////////

struct SLIDER_DATA_LONG
{
    long *Data;
    long Min;
    long Max;
    long Step;
    bool DrawSlider, DrawText;
};

struct SLIDER_DATA_ULONG
{
    unsigned long *Data;
    unsigned long Min;
    unsigned long Max;
    unsigned long Step;
    bool DrawSlider, DrawText;
};

/////////////////////////////////////////////////////////////////////
//
// Menu Item Data
//
/////////////////////////////////////////////////////////////////////

// Menu Item selectability
#define MENU_ITEM_INACTIVE          0x0
#define MENU_ITEM_SELECTABLE        0x1
#define MENU_ITEM_ACTIVE            0x2
#define MENU_ITEM_HARDCODEDTEXT     0x4                 // For hardcoded text

struct MENU_ITEM
{
    long                TextIndex;                      // Index into text array for item name
    FLOAT               DataWidth;                      // Width in pixels required to draw data
    VOID*               Data;                           // Data associated with item
    MENUITEM_DRAW_FUNC  DrawFunc;                       // Function to draw item if necessary
    MENUITEM_INPUT_FUNC InputAction[MENU_INPUT_NTYPES]; // Acion funtions when item is selected and inputs pressed
    long                ActiveFlags;                    // Whether Item is currently active
};


/////////////////////////////////////////////////////////////////////
//
// Menu Data: info for each menu page
//
/////////////////////////////////////////////////////////////////////

struct MENU
{
    long                TextIndex;
    DWORD               dwFlags;
    MENU_CREATE_FUNC    CreateFunc;
    MENU_INPUT_FUNC     InputFunc;
    MENU_DRAW_FUNC      DrawFunc;
    FLOAT               XPos, YPos;
    long                CurrentItemIndex;
    MENU*               ParentMenu;
    FLOAT               fWidth, fHeight;
};


/////////////////////////////////////////////////////////////////////
//
// Menu Header Data: info for current menu page. Set up from the
// current menu's CreateFunc.
//
/////////////////////////////////////////////////////////////////////

class MENU_HEADER
{
public:
    MENU*               m_pMenu;                          // Menu Currently displayed
    MENU*               m_pNextMenu;                      // Next menu (when != Menu, the m,enu changes)
    MENU_ITEM*          m_pMenuItem[MENU_MAX_ITEMS];      // Pointers to menu item data
    MENU_ITEM*          m_pCurrentItem;
    short               m_dwNumMenuItems;                 // Number of Menu Items
    FLOAT               m_XPos,      m_YPos;              // Current Screen Position and dimensions
    FLOAT               m_XSize,     m_YSize;             // Current Screen Position and dimensions
    FLOAT               m_DestXPos,  m_DestYPos;          // Desired position and size
    FLOAT               m_DestXSize, m_DestYSize;         // Desired position and size
    FLOAT               m_ItemTextWidth;                  // max width of item text
    FLOAT               m_ItemDataWidth;                  // max width of item data
    MENU_STATE          m_State;
    const WCHAR*        m_strTitle;                       // Menu title
    int                 m_nLastControllerInput;           // what controller generated last menu input
public:
    void DrawMenuBox();
    void DrawMenuLogo();
    void DrawMenuTitle();
    void InitMenuHeader();
    void CalcMenuDimensions( MENU* pMenu );

    void DrawMenuItemText( MENU* pMenu, MENU_ITEM* pMenuItem, int itemIndex);

public:
    void MoveResizeMenu();
    void ProcessMenuInput();
    void DrawMenu();

    void ClearMenuHeader();
    void AddMenuItem( MENU_ITEM* pMenuItem, DWORD dwFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE );
    void AddMenuItem( DWORD TextIndex, DWORD dwFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE );
    void AddMenuItem( const WCHAR* strText, DWORD dwFlags = MENU_ITEM_ACTIVE | MENU_ITEM_SELECTABLE );

    void SetNextMenu( MENU* pMenu );

    void HandleMenus();
    
    MENU_HEADER();
};


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
// External prototypes
//
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


extern BOOL  SelectPreviousMenuItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  SelectNextMenuItem( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  ToggleMenuData( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  ToggleMenuDataOn( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  ToggleMenuDataOff( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  MenuGoBack( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  MenuGoForward( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

extern FLOAT CalcMaxStringWidth( DWORD dwNumStrings, WCHAR** pstrStrings );

extern BOOL  IncreaseSliderDataLong( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  DecreaseSliderDataLong( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  IncreaseSliderDataULong( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );
extern BOOL  DecreaseSliderDataULong( MENU_HEADER* pMenuHeader, MENU* pMenu, MENU_ITEM* pMenuItem );

extern MenuInputTypeEnum GetFullScreenMenuInput( BOOL bMergeButtons = TRUE, int *pnController = NULL );


/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////
//
// External Variables
//
/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

extern MENU_HEADER* g_pMenuHeader;

extern FLOAT        gMenuWidthScale;
extern FLOAT        gMenuHeightScale;

extern BOOL         g_bShowMenuLogo; // Whether or not to show the Revolt logo


extern long gMenuInputSFXIndex[MENU_INPUT_NTYPES];



#endif // UI_MENU_H

