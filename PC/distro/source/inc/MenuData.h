/////////////////////////////////////////////////////////////////////////////////
// MenuData.h
/////////////////////////////////////////////////////////////////////////////////
#ifndef __MENUDATA_H
#define __MENUDATA_H

#include "revolt.h"
#include "TypeDefs.h"
#include "Menu.h"


/////////////////////////////////////////////////////////////////////////////////
// Externs
/////////////////////////////////////////////////////////////////////////////////
extern t_Menu		gMainOptions_Menu;
extern t_MenuItem	*gMainOptions_MenuItemList[];
extern t_MenuItem	gMainOptions_MenuItem_1;
extern t_MenuItem	gMainOptions_MenuItem_2;
extern t_MenuItem	gMainOptions_MenuItem_3;

extern t_Menu		gSubMenu_Menu;
extern t_MenuItem	*gSubMenu_MenuItemList[];
extern t_MenuItem	gSubMenu_MenuItem_1;
extern t_MenuItem	gSubMenu_MenuItem_2;
extern t_MenuItem	gSubMenu_MenuItem_3;


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void MainOptionsInitFunc(t_Menu* pMenu);
int MainOptionStartLoopFunc(t_Menu* pMenu);


/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
#endif //__MENUDATA_H