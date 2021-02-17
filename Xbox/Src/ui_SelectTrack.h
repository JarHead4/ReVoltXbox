//-----------------------------------------------------------------------------
// File: ui_SelectTrack.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_SELECTTRACK_H
#define UI_SELECTTRACK_H


extern MENU Menu_TrackSelect;


extern void TrackSelectMirrorTrack();
extern void TrackSelectReverseTrack();
extern void TrackSelectPrevTrack();
extern void TrackSelectNextTrack();



//-----------------------------------------------------------------------------
// The SelectTrack state engine
//-----------------------------------------------------------------------------
class CSelectTrackStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectTrack"; }
};

extern CSelectTrackStateEngine g_SelectTrackStateEngine;




#endif //UI_SELECTTRACK_H
