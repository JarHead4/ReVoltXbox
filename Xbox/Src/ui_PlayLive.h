//-----------------------------------------------------------------------------
// File: PlayLive.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef PLAYLIVE_H
#define PLAYLIVE_H




//-----------------------------------------------------------------------------
// The Play Live state engine
//-----------------------------------------------------------------------------
class CPlayLiveStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"PlayLive"; }
};

extern CPlayLiveStateEngine g_PlayLiveStateEngine;


// NOTE (JHarding): moved here to be accessible for accepting invitations
class CQuickMatchStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"QuickMatch"; }
};

extern CQuickMatchStateEngine g_QuickMatchStateEngine;

class CJoinMatchStateEngine : public CUIStateEngine
{
protected:
    virtual VOID HandleEnterFromParent();
    virtual VOID HandleExitToParent();
    virtual VOID HandleEnterFromChild();
    virtual VOID HandleExitToChild();
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"JoinMatch"; }
};

extern CJoinMatchStateEngine g_JoinMatchStateEngine;

#endif // PLAYLIVE_H