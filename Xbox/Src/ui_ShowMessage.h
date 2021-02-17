//-----------------------------------------------------------------------------
// File: ui_ShowMessage.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_SHOWMESSAGE_H
#define UI_SHOWMESSAGE_H

#include "ui_StateEngine.h"
#include "ui_menu.h"


class CShowSimpleMessage : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"ShowSimpleMessage"; }

public:
    const WCHAR* m_strTitle;
    const WCHAR* m_strMessage;
    const WCHAR* m_strForward;
    const WCHAR* m_strBackward;

    //$HACK(Apr02_GameBash)
    virtual VOID MakeActive( CStateEngine* pParent = g_pActiveStateEngine );

    VOID Begin( const WCHAR* strTitle,
                const WCHAR* strMessage,
                const WCHAR* strForward,
                const WCHAR* strBackward);
};

extern CShowSimpleMessage g_ShowSimpleMessage;
extern WCHAR g_SimpleMessageBuffer[];

extern MENU Menu_InitialMessage;
extern MENU Menu_Message;

class CControllerRemoved : public CShowSimpleMessage
{
public:
    virtual WCHAR*  DebugGetName()  { return L"ControllerRemoved"; }

    VOID Begin( DWORD dwPort );

    WCHAR m_strMessage[100];
};

extern CControllerRemoved g_ControllerRemoved;


#endif //  UI_SHOWMESSAGE_H
