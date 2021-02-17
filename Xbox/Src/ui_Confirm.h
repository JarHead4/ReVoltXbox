//-----------------------------------------------------------------------------
// File: ui_Confirm.h
//
// Desc: UI implementation
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_CONFIRM_H
#define UI_CONFIRM_H


extern MENU Menu_ConfirmYesNo;


extern void SetConfirmMenuStrings(WCHAR *title, WCHAR *yes, WCHAR *no, long def);

enum {
    CONFIRM_TYPE_YESNO,

    CONFIRM_NTYPES
};

enum {
    CONFIRM_NONE,
    CONFIRM_YES,
    CONFIRM_NO,
    CONFIRM_GOBACK,

    CONFIRM_NRETURNS
};

extern WCHAR gConfirmMenuText[64];
extern WCHAR gConfirmMenuTextYes[32];
extern WCHAR gConfirmMenuTextNo[32];
extern long  gConfirmMenuType;
extern long  gConfirmMenuReturnVal;
extern bool  gConfirmGiveup;
extern long  gConfirmType;



#endif //UI_CONFIRM_H
