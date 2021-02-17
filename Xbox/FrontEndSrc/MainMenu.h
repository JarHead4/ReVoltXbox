// todo header

extern MENU Menu_TopLevel;


enum MAIN_MENU_OPTIONS 
{
    MAIN_MENU_SINGLE,
    MAIN_MENU_AI_TEST,
    MAIN_MENU_MULTI,
    MAIN_MENU_JOIN,
    MAIN_MENU_TRACK,
    MAIN_MENU_RES,
    MAIN_MENU_TEXBPP,
    MAIN_MENU_DEVICE,
    MAIN_MENU_JOYSTICK,
    MAIN_MENU_CAR,
    MAIN_MENU_NAME,
    MAIN_MENU_EDIT,
    MAIN_MENU_BRIGHTNESS,
    MAIN_MENU_CONTRAST,
    MAIN_MENU_REVERSED,
    MAIN_MENU_MIRRORED,

    MAIN_MENU_NEW_TITLE_SCREEN,

    MAIN_MENU_NUM,
};


extern short MenuCount;

extern void MainMenu(void);