//-----------------------------------------------------------------------------
// File: Confirm.h
//
// Desc: 
//
// Hist: 02.08.02 - revolt port/reorg
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------



enum 
{
    CONFIRM_NONE,
    CONFIRM_YES,
    CONFIRM_NO,
    CONFIRM_GOBACK,

    CONFIRM_NRETURNS
};


extern char gConfirmMenuText[64];
extern char gConfirmMenuTextYes[32];
extern char gConfirmMenuTextNo[32];
extern long gConfirmMenuType;
extern long gConfirmMenuReturnVal;
extern bool gConfirmGiveup;
extern long gConfirmType;

extern MENU Menu_ConfirmYesNo;


extern void SetConfirmMenuStrings(char *title, char *yes, char *no, long def);
