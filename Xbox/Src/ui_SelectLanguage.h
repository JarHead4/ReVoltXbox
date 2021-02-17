//-----------------------------------------------------------------------------
// File: ui_SelectLanguage.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_SELECTLANGUAGE_H
#define UI_SELECTLANGUAGE_H


extern MENU Menu_Language;




//-----------------------------------------------------------------------------
// The SelectLanguage state engine
//-----------------------------------------------------------------------------
class CSelectLanguageStateEngine : public CUIStateEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"SelectLanguage"; }
};

extern CSelectLanguageStateEngine g_SelectLanguageStateEngine;




#endif //  UI_SELECTLANGUAGE_H
