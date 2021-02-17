//-----------------------------------------------------------------------------
// File: ui_Options.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_OPTIONS_H
#define UI_OPTIONS_H

#include <xvoice.h>


extern MENU Menu_Options;
extern MENU Menu_GameSettings;
extern MENU Menu_VideoSettings;
extern MENU Menu_RenderSettings;
extern MENU Menu_AudioSettings;
extern MENU Menu_ControllerSettings;

extern void SetRaceCredits();


// g_VoiceMaskPresets is an array of structs, each containing a preset
// XVOICE_MASK and a WCHAR string labelling it.  For localization,
// strLabel will need to point to a string table, rather than a hardcoded
// string
struct VOICE_MASK_PRESET
{
    INT         LabelTextID;
    XVOICE_MASK mask;
};

extern VOICE_MASK_PRESET g_VoiceMaskPresets[];
extern const DWORD g_dwNumVoiceMaskPresets;


//-----------------------------------------------------------------------------
// The Options state engine
//-----------------------------------------------------------------------------
class COptionsStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"Options"; }
};

extern COptionsStateEngine g_OptionsStateEngine;


#endif //  UI_OPTIONS_H
