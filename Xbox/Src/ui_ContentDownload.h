//-----------------------------------------------------------------------------
// File: ui_ContentDownload.h
//
// Desc: Public UI Data Declarations
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_CONTENTDOWNLOAD_H
#define UI_CONTENTDOWNLOAD_H

#include "Content.h"

//-----------------------------------------------------------------------------
// content download states
//-----------------------------------------------------------------------------
enum
{
    CONTENTDOWNLOAD_STATE_BEGIN = STATEENGINE_STATE_BEGIN,
    
    CONTENTDOWNLOAD_STATE_SHOW_ENUM,
    CONTENTDOWNLOAD_STATE_ENUM,
    
    CONTENTDOWNLOAD_STATE_SHOW_SELECT,
    CONTENTDOWNLOAD_STATE_SELECT,

    CONTENTDOWNLOAD_STATE_SHOW_GETDETAILS,
    CONTENTDOWNLOAD_STATE_GETDETAILS,
    
    CONTENTDOWNLOAD_STATE_SHOW_DETAILS,
    CONTENTDOWNLOAD_STATE_DETAILS,

    CONTENTDOWNLOAD_STATE_CONFIRM_PURCHASE_CONTENT,
    CONTENTDOWNLOAD_STATE_SHOW_PURCHASE_CONTENT,
    
    CONTENTDOWNLOAD_STATE_CONFIRM_PURCHASE_SUBSCRIPTION,
    CONTENTDOWNLOAD_STATE_SHOW_PURCHASE_SUBSCRIPTION,
    
    CONTENTDOWNLOAD_STATE_PURCHASE,
    
    CONTENTDOWNLOAD_STATE_CONFIRM_CANCEL_SUBSCRIPTION,
    CONTENTDOWNLOAD_STATE_SHOW_CANCEL_SUBSCRIPTION,
    CONTENTDOWNLOAD_STATE_CANCEL_SUBSCRIPTION,

    CONTENTDOWNLOAD_STATE_SHOWPURCHASEING,
    CONTENTDOWNLOAD_STATE_PURCHASEING,

    CONTENTDOWNLOAD_STATE_CANT_PURCHASE,
    
    CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD,
    CONTENTDOWNLOAD_STATE_DOWNLOAD,
    
    CONTENTDOWNLOAD_STATE_SHOWABORT,
    CONTENTDOWNLOAD_STATE_ABORT,

    CONTENTDOWNLOAD_STATE_ERROR,
    CONTENTDOWNLOAD_STATE_SUCCESS
};

//-----------------------------------------------------------------------------
// Name: CContentDownloadEngine
// Desc: The content download engine
//-----------------------------------------------------------------------------
class CContentDownloadEngine : public CUIStateEngine
{
public:

    virtual WCHAR*  DebugGetName()   { return L"ContentDownload"; }
    virtual VOID Return(DWORD dwExitStatus = STATEENGINE_COMPLETED); // NOTE that owned cars is not cleared
    
protected:
    
    
    // begins error and succes states
    VOID BeginError(const WCHAR* strErrorMsg, const WCHAR* strTitle);
    VOID BeginSuccess(const WCHAR* strSuccessMsg, const WCHAR* strTitle);
};


//-----------------------------------------------------------------------------
// Name: CRequiredDownloadEngine
// Desc: The content download engine
// Note: only handles phase 1 for now
//-----------------------------------------------------------------------------
class CRequiredDownloadEngine : public CContentDownloadEngine
{
public:
    virtual HRESULT Process();
    virtual WCHAR*  DebugGetName()   { return L"RequiredDownload"; }
};


//-----------------------------------------------------------------------------
// Name: g_RequiredDownloadEngine
// Desc: global instance of content download engine
//-----------------------------------------------------------------------------
extern CRequiredDownloadEngine g_RequiredDownloadEngine;

//-----------------------------------------------------------------------------
// Name: COptionalDownloadEngine
// Desc: The content download engine
//-----------------------------------------------------------------------------

#define DISPLAY_MAX 5
#define DISPLAY_WIDTH 41

class COptionalDownloadEngine : public CContentDownloadEngine
{
public:

    virtual     HRESULT Process();
    virtual     WCHAR*  DebugGetName()   { return L"OptionalDownload"; }

    
    DWORD       m_dwCurrentItem;
    DWORD       m_dwDisplayTop;
    DWORD       m_dwNumItems;
    WCHAR       m_strHeader[DISPLAY_WIDTH];
};


//-----------------------------------------------------------------------------
// Name: g_OptionalDownloadEngine
// Desc: global instance of content download engine
// Note: be sure to call the class Init before and CleanUp after.  CleanUp 
//-----------------------------------------------------------------------------
extern COptionalDownloadEngine g_OptionalDownloadEngine;





#endif //  UI_CONTENTDOWNLOAD_H
