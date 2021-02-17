/////////////////////////////////////////////////////////////////////////////////
// Menu.CPP
/////////////////////////////////////////////////////////////////////////////////
#include "revolt.h"
#include "TypeDefs.h"
#include "text.h"
#include "input.h"

#include "Menu.h"
#include "MenuData.h"
#include "MenuText.h"
#include "TitleScreen.h"

void GoFront(void);
extern	void (*Event)(void);


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//									MENU
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// MenuInit()
/////////////////////////////////////////////////////////////////////////////////
void MenuInit(t_Menu *pMenu, t_Menu *pParentMenu)
{
// Set parent
	if ((int)pParentMenu != -1)
		pMenu->pParentMenu = pParentMenu;

// Setup current selected menu item
	if (pMenu->iMenuItemStart >= 0)
	{
		pMenu->pMenuItemCur = pMenu->pMenuItems[pMenu->iMenuItemStart];
	}
	else
	{
		if (pMenu->pMenuItemCur == NULL)
			pMenu->pMenuItemCur = pMenu->pMenuItems[0];
	}

// Call startup code
	if (pMenu->StartUpFunc)
		pMenu->StartUpFunc(pMenu);
}


/////////////////////////////////////////////////////////////////////////////////
// MenuProcess()
/////////////////////////////////////////////////////////////////////////////////
t_Menu *MenuProcess(t_Menu *pMenu)
{
	int	flag;
	int	index;

// Call start gameloop code
	if (pMenu->StartLoopFunc)
	{
		flag = pMenu->StartLoopFunc(pMenu);
	}


// Back
	if (Keys[DIK_ESCAPE] && !LastKeys[DIK_ESCAPE])
	{
		if (pMenu->pParentMenu)
		{
			pMenu = pMenu->pParentMenu;
			MenuInit(pMenu, (t_Menu*)-1);
			return pMenu;
		}
		else
		{
		// Return to old menu
			return NULL;
		}
	}


// Option selection
	index = -1;
	if (Keys[DIK_UP] && !LastKeys[DIK_UP])
		index = 0;
	else if (Keys[DIK_DOWN] && !LastKeys[DIK_DOWN])
		index = 1;
	else if (Keys[DIK_LEFT] && !LastKeys[DIK_LEFT])
		index = 2;
	else if (Keys[DIK_RIGHT] && !LastKeys[DIK_RIGHT])
		index = 3;

	if (index >= 0)
	{
	// Call function
		if (pMenu->pMenuItemCur->Function[index])
			pMenu->pMenuItemCur->Function[index](pMenu);

	// Select menu item
		if (pMenu->pMenuItemCur->pSelectMenuItem[index])
			pMenu->pMenuItemCur = pMenu->pMenuItemCur->pSelectMenuItem[index];
	}


// Select
	if (Keys[DIK_RETURN] && !LastKeys[DIK_RETURN])
	{
		if (pMenu->pMenuItemCur->pSelectMenu)
		{
			MenuInit(pMenu->pMenuItemCur->pSelectMenu, pMenu);
			pMenu = pMenu->pMenuItemCur->pSelectMenu;
			return pMenu;
		}
	}


// Return menu;
	return pMenu;
}

/////////////////////////////////////////////////////////////////////////////////
// MenuRender()
/////////////////////////////////////////////////////////////////////////////////
void MenuRender(t_Menu *pMenu)
{
	t_MenuItem	**pMenuItemList;
	t_MenuItem	*pMenuItem;
	int			flags;

	pMenuItemList = pMenu->pMenuItems;
	while (*pMenuItemList)
	{
	// Get menu item
		pMenuItem = *pMenuItemList;

	// Render
		flags = 0;
		if (pMenuItem == pMenu->pMenuItemCur)
			flags |= MENUITEM_HILIGHT;

		MenuItemRender(pMenuItem, flags);

	// Next
		pMenuItemList++;
	}
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//									MENU ITEM
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
void MenuItemRender(t_MenuItem *pMenuItem, int flags)
{
	if (flags & MENUITEM_HILIGHT)
		DumpText(pMenuItem->textX,pMenuItem->textY, 12,16, 0x80C0FF, pMenuItem->pText);
	else
		DumpText(pMenuItem->textX,pMenuItem->textY, 12,16, 0x808080, pMenuItem->pText);
}



/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
void DecLightPower(t_Menu* pMenu)
{
	gLight[0].power -= 1;
}

void IncLightPower(t_Menu* pMenu)
{
	gLight[0].power += 1;
	if (gLight[0].power < 1)
		gLight[0].power = 1;
}

void DecLightStrength(t_Menu* pMenu)
{
	gLight[0].strength -= 10;
}

void IncLightStrength(t_Menu* pMenu)
{
	gLight[0].strength += 10;
}

