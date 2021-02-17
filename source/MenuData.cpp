/////////////////////////////////////////////////////////////////////////////////
// Menu.CPP
/////////////////////////////////////////////////////////////////////////////////
#include "Menu.h"
#include "MenuText.h"
#include "MenuData.h"


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////

#if 0
// Menu model
typedef struct t_MenuModel
{
	VEC			pos;				// Position
//	MODEL		*pModel;			// Model

} s_MenuModel;
#endif


/////////////////////////////////////////
// Main Options Menu
/////////////////////////////////////////

// Menu
t_Menu		gMainOptions_Menu =
{
	gMainOptions_MenuItemList,
	NULL,-1,

	MainOptionsInitFunc, MainOptionStartLoopFunc,		// StartUp code, StartLoop code
};

// Menu Item List
t_MenuItem	*gMainOptions_MenuItemList[] =
{
	&gMainOptions_MenuItem_1,
	&gMainOptions_MenuItem_2,
	&gMainOptions_MenuItem_3,
	NULL,
};

// Menu Item 1
t_MenuItem	gMainOptions_MenuItem_1 =
{
	64,64,gMainOptions_MenuItem_1_Text,
	NULL,

	NULL,
	&gMainOptions_MenuItem_2,
	NULL,
	NULL,

	&gSubMenu_Menu,

	NULL,NULL,NULL,NULL,
};

// Menu Item 2
t_MenuItem	gMainOptions_MenuItem_2 =
{
	64,80,gMainOptions_MenuItem_2_Text,
	NULL,

	&gMainOptions_MenuItem_1,
	&gMainOptions_MenuItem_3,
	NULL,
	NULL,

	NULL,

	NULL,NULL,NULL,NULL,
};

// Menu Item 3
t_MenuItem	gMainOptions_MenuItem_3 =
{
	64,96,gMainOptions_MenuItem_3_Text,
	NULL,

	&gMainOptions_MenuItem_2,
	NULL,
	NULL,
	NULL,

	NULL,

	NULL,NULL,NULL,NULL,
};



/////////////////////////////////////////
// Sub Menu 1
/////////////////////////////////////////

// Menu
t_Menu		gSubMenu_Menu =
{
	gSubMenu_MenuItemList,		// Menu item list
	NULL,0,						// Current item & start item index

	NULL,NULL,					// StartUp code, StartLoop code
};

// Menu Item List
t_MenuItem	*gSubMenu_MenuItemList[] =
{
	&gSubMenu_MenuItem_1,
	&gSubMenu_MenuItem_2,
	&gSubMenu_MenuItem_3,
	NULL,
};

// Menu Item 1
t_MenuItem	gSubMenu_MenuItem_1 =
{
	64,64,gSubMenu_MenuItem_1_Text,
	NULL,

	NULL,
	&gSubMenu_MenuItem_2,
	NULL,
	NULL,

//	&gMainOptions_Menu,
	NULL,

	NULL,NULL,
	DecLightPower,IncLightPower,
};

// Menu Item 2
t_MenuItem	gSubMenu_MenuItem_2 =
{
	64,80,gSubMenu_MenuItem_2_Text,
	NULL,

	&gSubMenu_MenuItem_1,
	&gSubMenu_MenuItem_3,
	NULL,
	NULL,

//	&gMainOptions_Menu,
	NULL,

	NULL,NULL,
	DecLightStrength,IncLightStrength,
};

// Menu Item 3
t_MenuItem	gSubMenu_MenuItem_3 =
{
	64,96,gSubMenu_MenuItem_3_Text,
	NULL,

	&gSubMenu_MenuItem_2,
	NULL,
	NULL,
	NULL,

//	&gMainOptions_Menu,
	NULL,

	NULL,NULL,NULL,NULL,
};




/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void MainOptionsInitFunc(t_Menu* pMenu)
{
}

int MainOptionStartLoopFunc(t_Menu* pMenu)
{
	return 0;
}
