//-----------------------------------------------------------------------------
// File: ui_InGameMenu.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_INGAMEMENU_H
#define UI_INGAMEMENU_H

//-----------------------------------------------------------------------------
// The Play Live state engine
//-----------------------------------------------------------------------------
class CInGameMenuStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"InGameMenu"; }


    enum
    {
        INGAMEMENU_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
        INGAMEMENU_STATE_MAINLOOP,
		INGAMEMENU_STATE_POSTCONFIRMQUIT,
		INGAMEMENU_STATE_QUITRACE,
		INGAMEMENU_STATE_POSTCONFIRMRESTART,
		INGAMEMENU_STATE_RESTARTRACE,
        INGAMEMENU_STATE_EXIT = STATEENGINE_STATE_EXIT,
    };


};

extern CInGameMenuStateEngine g_InGameMenuStateEngine;




extern MENU Menu_InGame;
//extern MENU Menu_InGameGraphicsOptions;
//extern MENU Menu_InGameOptions;



#endif //  UI_INGAMEMENU_H
