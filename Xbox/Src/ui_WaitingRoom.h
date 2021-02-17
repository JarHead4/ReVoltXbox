//-----------------------------------------------------------------------------
// File: ui_WaitingRoom.h
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_WAITINGROOM_H
#define UI_WAITINGROOM_H


extern MENU Menu_WaitingRoom;

extern long  HostnameEntry;


//-----------------------------------------------------------------------------
// The WaitingRoom state engine
//-----------------------------------------------------------------------------
class CWaitingRoomStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"WaitingRoom"; }
};

extern CWaitingRoomStateEngine g_WaitingRoomStateEngine;




#endif //  UI_WAITINGROOM_H
