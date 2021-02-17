//-----------------------------------------------------------------------------
// File: ui_BestTimes.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_BESTTIMES_H
#define UI_BESTTIMES_H


//-----------------------------------------------------------------------------
// The BestTimes state engine
//-----------------------------------------------------------------------------
class CBestTimesStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"BestTimes"; }
};

extern CBestTimesStateEngine g_BestTimesStateEngine;


#endif //  UI_BESTTIMES_H
