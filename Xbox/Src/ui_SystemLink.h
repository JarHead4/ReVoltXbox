//-----------------------------------------------------------------------------
// File: ui_SystemLink.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SYSTEMLINK_H
#define SYSTEMLINK_H




//-----------------------------------------------------------------------------
// The System Link state engine
//-----------------------------------------------------------------------------
class CSystemLinkStateEngine : public CUIStateEngine
{
protected:
    virtual VOID HandleEnterFromParent();
    virtual VOID HandleExitToParent();
    virtual VOID HandleEnterFromChild();
    virtual VOID HandleExitToChild();
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SystemLink"; }
};

extern CSystemLinkStateEngine g_SystemLinkStateEngine;




#endif // SYSTEMLINK_H