//-----------------------------------------------------------------------------
// File: ui_ContentDownload.cpp
//
// Desc: UI for content download
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "Settings.h"
#include "Text.h"
#include "ui_Menu.h"
#include "ui_menudraw.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "ui_StateEngine.h"
#include "ui_ContentDownload.h"  
#include "draw.h"
#include "ui_confirm.h"
#include "readinit.h"
#include <xgraphics.h> 

//-----------------------------------------------------------------------------
// global instance of required content download engine
//-----------------------------------------------------------------------------
CRequiredDownloadEngine g_RequiredDownloadEngine;

//-----------------------------------------------------------------------------
// global instance of optional content download engine
//-----------------------------------------------------------------------------
COptionalDownloadEngine g_OptionalDownloadEngine;



//-----------------------------------------------------------------------------
// global instance of the Enum strings
// Desc: global array of enum strings for selecting and viewing enum blobs
//-----------------------------------------------------------------------------
std::vector<std::wstring> EnumStrings;
VOID FreeEnum( )
{
    EnumStrings.clear();
}

//-----------------------------------------------------------------------------
// global isntance of the selection strings
// Desc: global array of selections strings for the select emnu
//-----------------------------------------------------------------------------
std::vector<std::wstring> SelectStrings;
VOID FreeSelect()
{
    SelectStrings.clear();  
}

//-----------------------------------------------------------------------------
// global instance of the details globals (texture) 
// Desc: some file scope global vars for storing the details texture
//-----------------------------------------------------------------------------
IDirect3DTexture8* pTexture = NULL;
BYTE* pSysMemData = NULL;
BYTE* pVidMemData = NULL;
VOID FreeDetails()
{
    // $NOTE: pTexture will be null if the details can't be read
    pTexture = NULL;

    if( pSysMemData != NULL )
    {
        delete[] pSysMemData;
        pSysMemData = NULL;
    }
    if( pVidMemData != NULL )
    {
        D3D_FreeContiguousMemory( pVidMemData );
        pVidMemData = NULL;
    }
}




//-----------------------------------------------------------------------------
// Name: ParseEnum
// Desc: Parses the enum data blob into strings of text
//-----------------------------------------------------------------------------
VOID ParseEnum( const BYTE* pEnum, DWORD dwEnumSize )
{
    FreeEnum();

    if(dwEnumSize < 2)
        return;

    //$MD: BUGBUG: should be a #define
    WCHAR strTemp[200];

    // look for uinicode marker
    if(((WCHAR*)(pEnum))[0] == 0xFEFF)
    {
        // skip unicode marker
        pEnum += 2;
        dwEnumSize -= 2;

        const WCHAR* pData = (WCHAR*)pEnum;
        DWORD dwDataSize = dwEnumSize/2;
        DWORD dwLast = 0;

        for(UINT i = 0; i < dwDataSize; i++)
        {
            // look for newlines and end of data to parse into strings
            if( (pData[i] == L'\r' && (i+1 < dwDataSize) && pData[i+1] == L'\n') ||
                ( i == dwDataSize - 1 ) )
            {
                if(i == dwDataSize - 1)
                {
                    if(pData[i] == L'\r' || pData[i] == L'\n') 
                        break;
                    i++;
                }
                memcpy(strTemp, pData + dwLast, (i - dwLast) * sizeof(WCHAR));
                strTemp[i - dwLast] = NULL;
                swprintf(strTemp, L"%s", strTemp);
                EnumStrings.push_back(std::wstring());
                EnumStrings.back() = strTemp;
                i+=2;
                dwLast = i;
            }
        }
        
    }
    // ascii
    else
    {
        const CHAR* pData = (CHAR*)pEnum;
        DWORD dwDataSize = dwEnumSize;
        DWORD dwLast = 0;

        CHAR szTemp[200];

        for(UINT i = 0; i < dwDataSize; i++)
        {
            // look for newlines and end of data to parse into strings
            if( (pData[i] == '\r' && (i+1 < dwDataSize) && pData[i+1] == '\n') ||
                ( i == dwDataSize - 1 ) )
            {
                if(i == dwDataSize - 1)
                    i++;
                memcpy(szTemp, pData + dwLast, (i - dwLast) * sizeof(CHAR));
                szTemp[i - dwLast] = NULL;
                swprintf(strTemp, L"%S", szTemp);
                EnumStrings.push_back(std::wstring());
                EnumStrings.back() = strTemp;
                i+=2;
                dwLast = i;
            }
        }
    }
}



//-----------------------------------------------------------------------------
// Name: ParseDetails
// Desc: grabs a packed resource texture from the enum blob
// NOTE: $BUGBUG: we should really be using a compressed texture
//       for the details blob but I had some trouble with rendereing
//       the compressed texture formats with the color key alpha that we're using
//-----------------------------------------------------------------------------
BOOL ParseDetails( const BYTE* pDetails, DWORD dwDetailsSize )
{
    FreeDetails();

    /*
    FILE* fp = fopen("T:\\test.bin", "wb");
    assert(fp);
    fwrite(pDetails, dwDetailsSize, 1, fp);
    fclose(fp);
    */

    /*
    static bool bOnce = false;
    static BYTE* pMem;
    static DWORD dwMemSize;
    if(!bOnce)
    {
        HANDLE hFile = CreateFile("D:\\test.bin", GENERIC_READ,
                                  FILE_SHARE_READ, NULL,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        dwMemSize = GetFileSize(hFile, NULL);
        pMem = new BYTE[dwMemSize];
        DWORD dwNumRead;
        ReadFile(hFile, pMem, dwMemSize, &dwNumRead, NULL);
        bOnce = true;
    }

     pDetails = pMem;
     dwDetailsSize = dwMemSize;
     */

    // check size
    if(dwDetailsSize < sizeof(XPR_HEADER))
    {
#ifdef _DEBUG
        DumpMessage("Warning", "Incorrect details blob texture.");
#endif
        return FALSE;
    };

    // Read in and verify the XPR magic header
    XPR_HEADER* pxprh = (XPR_HEADER*)pDetails;
    if( pxprh->dwMagic != XPR_MAGIC_VALUE )
    {
#ifdef _DEBUG
        DumpMessage("Warning", "Incorrect details blob texture.");
#endif
        return FALSE;
    }

    // check size
    if(dwDetailsSize < pxprh->dwTotalSize)
    {
#ifdef _DEBUG
        DumpMessage("Warning", "Incorrect details blob texture.");
#endif
        return FALSE;
    }

    // move past header
    pDetails += sizeof(XPR_HEADER);
    
    // Compute memory requirements
    DWORD dwSysMemDataSize = pxprh->dwHeaderSize - sizeof(XPR_HEADER);
    DWORD dwVidMemDataSize = pxprh->dwTotalSize - pxprh->dwHeaderSize;

    
    // Allocate memory
    pSysMemData = new BYTE[dwSysMemDataSize];
    pVidMemData = (BYTE*)D3D_AllocContiguousMemory( dwVidMemDataSize, D3DTEXTURE_ALIGNMENT );

    // Read in the data from the file
    memcpy( pSysMemData, pDetails, dwSysMemDataSize);
    pDetails += dwSysMemDataSize;
    memcpy( pVidMemData, pDetails, dwVidMemDataSize);
    
    // Get the resource
    D3DResource* pRes = (D3DResource*)pSysMemData;

    // Register the resource
    pRes->Register( pVidMemData );
    
    // make sure the resource is a texture
    if(pRes->GetType() != D3DRTYPE_TEXTURE)
    {
#ifdef _DEBUG
        DumpMessage("Warning", "Incorrect details blob texture.");
#endif
        return FALSE;
    }

    // store
    pTexture = (LPDIRECT3DTEXTURE8)pRes;
            
    return S_OK;
}

//-----------------------------------------------------------------------------
// global instance of the Enum strings
// Desc: global array of enum strings for selecting and viewing enum blobs
//-----------------------------------------------------------------------------
std::vector<std::wstring> OfferingDesc;
VOID FreeOfferingDesc( )
{
    OfferingDesc.clear();
}

//-----------------------------------------------------------------------------
// Name: ParseDetails
// Desc: grabs a packed resource texture from the enum blob
// NOTE: $BUGBUG: we should really be using a compressed texture
//       for the details blob but I had some trouble with rendereing
//       the compressed texture formats with the color key alpha that we're using
//-----------------------------------------------------------------------------
VOID MakeOfferingDesc()
{
    OfferingDesc.clear();

    ContentInfo* pInfo = g_ContentManager.GetWorkingInfo();
    ContentDetails* pDetails = g_ContentManager.GetWorkingDetails();

    WCHAR strPrice[64];
    if(!pDetails->GetPrice().fOfferingIsFree)
    {
        DWORD dwBufferSize = 64;
        HRESULT hr = XOnlineOfferingPriceFormat( &pDetails->GetPrice(),
                                                 strPrice, &dwBufferSize, 0 );
        assert(SUCCEEDED(hr));
    }
    else
        swprintf(strPrice, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_FREE));
    WCHAR strDispPrice[200];
    swprintf(strDispPrice, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_PRICE), strPrice);

    OfferingDesc.push_back(strDispPrice);

    // $MD: remove before shipping (the SHIPPING flag is being used now, so I couldn't wrap this) 
    // free disclamer
    if(!pDetails->GetPrice().fOfferingIsFree)
    {
        OfferingDesc.push_back(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_TEMPNOCHARGE));
    }
    
    // subcription info
    if( XONLINE_OFFERING_SUBSCRIPTION == pInfo->GetOfferingType() )
    {
        // duration
        static WCHAR strDuration[100];
        if( pDetails->GetDuration() == 0)
            wsprintfW( strDuration, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_NONTERMINATING));
        else
            wsprintfW( strDuration, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_DURATION), pDetails->GetDuration() );
        OfferingDesc.push_back(strDuration);

        // free months
        static WCHAR strFreeMonths[100];
        if( pDetails->GetFreeMonths() != 0)
        {
            wsprintfW( strFreeMonths, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_FREEMONTHS), pDetails->GetFreeMonths() );
            OfferingDesc.push_back(strFreeMonths);
        }

        // frequency
        const WCHAR* strFrequency;
        switch( pDetails->GetFrequency() )
        {
        case ONE_TIME_CHARGE:
            strFrequency = TEXT_TABLE(TEXT_CONTENTDOWNLOAD_ONE_TIME);
            break;
        case MONTHLY:
            strFrequency = TEXT_TABLE(TEXT_CONTENTDOWNLOAD_MONTHLY); 
            break;
        case QUARTERLY:
            strFrequency = TEXT_TABLE(TEXT_CONTENTDOWNLOAD_QUARTERLY);
            break;
        case BIANNUALLY:
            strFrequency = TEXT_TABLE(TEXT_CONTENTDOWNLOAD_BIANNUALLY);
            break;
        case ANNUALLY:
            strFrequency = TEXT_TABLE(TEXT_CONTENTDOWNLOAD_ANNUALLY);
            break;
        default:
            assert( FALSE );
        }

        OfferingDesc.push_back(strFrequency);

        // cancellation
        OfferingDesc.push_back(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_CANCEL_ANY_TIME));
    }
}


//-----------------------------------------------------------------------------
// forward declares of MENU structs
//-----------------------------------------------------------------------------
extern MENU ContentDownload;
extern MENU ContentSelect;

//-----------------------------------------------------------------------------
// global messages
// NOTE: cancel and continue are button pushes and StartTime is a global timer
//-----------------------------------------------------------------------------
bool    g_bCancel;
bool    g_bContinue;
DWORD   g_dwStartTime;
const WCHAR* g_strTitle;


//-----------------------------------------------------------------------------
// Name: SetMenu
// Desc: Set menu helper
//-----------------------------------------------------------------------------
VOID SetMenu( MENU* pMenu, const WCHAR* strTitle )
{
    // reset buttons
    g_bCancel = false;
    g_bContinue = false;
    g_strTitle = strTitle;

    // reset start time
    g_dwStartTime = timeGetTime();

    // set menu
    g_pMenuHeader->SetNextMenu( pMenu );
}

//-----------------------------------------------------------------------------
// Name: GetMenuTime
// Desc: Gets the time since the menu has been up
//-----------------------------------------------------------------------------
DWORD GetMenuTime()
{
    return timeGetTime() - g_dwStartTime;
}

//-----------------------------------------------------------------------------
// Name: Message menu globals
// Desc: globals used by the message menu
//-----------------------------------------------------------------------------
const WCHAR*   g_strMessage;      // menu item strings
const WCHAR*   g_strForward;
const WCHAR*   g_strBackward;
#define MAX_MESSAGE 200

//-----------------------------------------------------------------------------
// Name: Menu_Msg
// Desc: The general menu for success or error
//-----------------------------------------------------------------------------
static BOOL HandleMsg( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawMsg( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateMsg(MENU_HEADER *menuHeader, MENU *menu);

extern MENU Msg = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y ,               // Menu type
    CreateMsg,                                                   // Create menu function
    HandleMsg,                                                   // Input handler function
    DrawMsg,                                                     // Menu draw function
    0,                                                           // X coord
    0,                                                           // Y Coord
};

//-----------------------------------------------------------------------------
// Menu_Msg Create, Handle, and Draw functions
//-----------------------------------------------------------------------------
void CreateMsg(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    // nothing is selectable
    pMenu->CurrentItemIndex = long(-1);

    pMenuHeader->AddMenuItem(TEXT_NONE); 
    pMenuHeader->AddMenuItem(TEXT_NONE);
    pMenuHeader->AddMenuItem(TEXT_NONE);

    pMenuHeader->m_pMenuItem[0]->Data = (void*)g_strMessage;
    pMenuHeader->m_pMenuItem[1]->Data = (void*)g_strForward;
    pMenuHeader->m_pMenuItem[2]->Data = (void*)g_strBackward;

}
BOOL HandleMsg( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    // NOTE: only continue or back out if possible for this 
    // menu

    switch( dwInput )
    {
        case MENU_INPUT_SELECT:
            if(g_strForward)
            {
                g_bContinue = true;
                return TRUE;
            }
            break;
        case MENU_INPUT_BACK:
            if(g_strBackward)
            {
                g_bCancel = true;
                return TRUE;
            }
            break;
    }
    return FALSE;
}
VOID DrawMsg( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    WCHAR* strMessage = (WCHAR*)pMenuHeader->m_pMenuItem[0]->Data;
    WCHAR* strForward = (WCHAR*)pMenuHeader->m_pMenuItem[1]->Data;
    WCHAR* strBackward = (WCHAR*)pMenuHeader->m_pMenuItem[2]->Data;


    if(strMessage == NULL )
        return;

    pMenuHeader->m_strTitle = g_strTitle;

    FLOAT xPos  = pMenuHeader->m_XPos;
    FLOAT yPos  = pMenuHeader->m_YPos;
    FLOAT xSize;
    FLOAT ySize;

    FLOAT fWidth1 = g_pFont->GetTextWidth( strMessage );
    FLOAT fWidth2 = g_pFont->GetTextWidth( strForward );
    FLOAT fWidth3 = g_pFont->GetTextWidth( strBackward );
    FLOAT fMaxWidth = max( fWidth1, max( fWidth2, fWidth3 ) ) ;

    FLOAT fMessageWidth;
    FLOAT fMessageHeight;
    g_pFont->GetTextExtent( strMessage, &fMessageWidth, &fMessageHeight );

    xSize = fMaxWidth;
    ySize = fMessageHeight;

    if( strForward || strBackward )
        ySize += 20;
    if( strForward )
        ySize += 20;
    if( strBackward )
        ySize += 20;
    
    xPos  = xPos - xSize/2;
    yPos  = yPos - ySize/2;

    DrawNewSpruBox( xPos - 20,
                    yPos - 10,
                    xSize + 40,
                    ySize + 20 );

    BeginTextState();

    g_pFont->DrawText( xPos+xSize/2, yPos, MENU_TEXT_RGB_NORMAL, strMessage, XBFONT_CENTER_X );
    yPos += fMessageHeight;
    yPos += 20;
    
    if( strForward )
    {
        g_pFont->DrawText( xPos, yPos, MENU_TEXT_RGB_NORMAL, strForward );
        yPos += 20;
    }
    
    if( strBackward )
    {
        g_pFont->DrawText( xPos, yPos, MENU_TEXT_RGB_NORMAL, strBackward );
        yPos += 20;
    }
}


//-----------------------------------------------------------------------------
// Name: SetMsgMenu
// Desc: Creates a generic messeage menu
//-----------------------------------------------------------------------------
VOID SetMsgMenu( const WCHAR* strTitle, const WCHAR* strMessage,
                 const WCHAR* strForward, const WCHAR* strBackward)
{
    g_strMessage = strMessage;
    g_strForward = strForward;
    g_strBackward = strBackward;
    SetMenu(&Msg, strTitle);
}



//-----------------------------------------------------------------------------
// Name: Confirm
// Desc: The general menu for success or error
//-----------------------------------------------------------------------------
static void CreateConfirm( MENU_HEADER* pMenuHeader, MENU* pMenu );
static BOOL HandleConfirm( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawConfirm( MENU_HEADER* pMenuHeader, MENU* pMenu );
static std::vector<std::wstring> ConfirmDesc;

extern MENU Confirm = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateConfirm,                      // Create menu function
    HandleConfirm,                      // Input handler function
    DrawConfirm,                        // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};


//-----------------------------------------------------------------------------
// Menu_Msg Create, Handle, and Draw functions
//-----------------------------------------------------------------------------
VOID DrawConfirm( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    pMenuHeader->m_strTitle = g_strTitle;
}
void CreateConfirm( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // add menu items
    for(UINT i = 0; i < ConfirmDesc.size(); i++)
    {
        pMenuHeader->AddMenuItem( ConfirmDesc[i].c_str() );
    }
    pMenuHeader->AddMenuItem( TEXT_NONE, MENU_ITEM_INACTIVE );
    pMenuHeader->AddMenuItem( TEXT_NO );
    pMenuHeader->AddMenuItem( TEXT_YES );
    
    // Put the selection on the "No" menuitem
    pMenu->CurrentItemIndex = i + 1;
}

BOOL HandleConfirm( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_UP:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_YES )
            {
                return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            }
            break;
        
        case MENU_INPUT_DOWN:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_NO )
            {
                return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, NULL );
            }
            break;

        case MENU_INPUT_BACK:
            g_bCancel = TRUE;
            return TRUE;

        case MENU_INPUT_SELECT:
            if( pMenuHeader->m_pCurrentItem->TextIndex == TEXT_YES )
                g_bContinue = true;
            else
                g_bCancel = true;
            return TRUE;
    }

    return FALSE;
}
//-----------------------------------------------------------------------------
// Name: SetMsgMenu
// Desc: Creates a generic messeage menu
//-----------------------------------------------------------------------------
VOID FreeConfirm()  
{
    ConfirmDesc.clear();
}
//-----------------------------------------------------------------------------
// Name: SetMsgMenu
// Desc: Creates a generic messeage menu
//-----------------------------------------------------------------------------
VOID SetConfirmMenu( const WCHAR* strTitle, const WCHAR* strMessage )
                    
{
    ConfirmDesc.clear();
    ConfirmDesc.push_back(strMessage);
    SetMenu(&Confirm, strTitle);
}
VOID SetConfirmMenu( const WCHAR* strTitle, const std::vector<std::wstring> Desc )
                    
{
    ConfirmDesc.clear();
    ConfirmDesc = Desc;
    SetMenu(&Confirm, strTitle);
}






//-----------------------------------------------------------------------------
// Name: ContentDownload
// Desc: Menu for content download
//-----------------------------------------------------------------------------
static BOOL HandleContentDownload( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawContentDownload( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateContentDownload(MENU_HEADER *menuHeader, MENU *menu);

extern MENU ContentDownload = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateContentDownload,                  // Create menu function
    HandleContentDownload,                  // Input handler function
    DrawContentDownload,                    // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};

//-----------------------------------------------------------------------------
// Menu_ContentDownload Create, Handle, and Draw functions
//-----------------------------------------------------------------------------
void CreateContentDownload(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_DOWNLOADINGNEWCONTENT));

    // space for progress bar
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");

    // space for enum data
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");

    // cancel
    pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_BUTTON_B_CANCEL));

    // nothing is selectable
    pMenu->CurrentItemIndex = long(-1);
}
BOOL HandleContentDownload( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_BACK:
            // only cancelable until were done
            if(g_ContentManager.Working())
            {
                g_bCancel = true;
                return TRUE;
            }
    }
    return FALSE;
}
VOID DrawContentDownload( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // $MD: BUGBUG: should be set in menu struct
    // $LOCALIZE
    pMenuHeader->m_strTitle = g_strTitle;

    // Parse enum data
    ContentInfo* pInfo = g_ContentManager.GetWorkingInfo();
    if(pInfo)
        ParseEnum(pInfo->GetEnumBlob(), pInfo->GetEnumBlobSize());
    
    // only also 2 strings worth of enumdata
    for(UINT i = 0; i < 2 && i < EnumStrings.size(); i++)
    {
        pMenuHeader->m_pMenuItem[5 + i]->Data = (VOID*)EnumStrings[i].c_str();
    }


    // set cancel or finished
    MENU_ITEM* pMenuItem = pMenuHeader->m_pMenuItem[pMenuHeader->m_dwNumMenuItems - 1];
    if(g_ContentManager.Working())
        pMenuItem->Data = TEXT_TABLE(TEXT_BUTTON_B_CANCEL);
    else
        pMenuItem->Data = TEXT_TABLE(TEXT_CONTENTDOWNLOAD_DOWNLOAD_COMPLETE);


    // calc dimetentions of progress meter
    const FLOAT fWidthScale = 0.8f;
    const FLOAT fHeightScale = 0.4f;

    FLOAT fProgWidth   = pMenuHeader->m_XSize * fWidthScale;
    FLOAT fProgHeight  = pMenuHeader->m_YSize * fHeightScale;

    FLOAT fProgX = pMenuHeader->m_XPos + (pMenuHeader->m_XSize - fProgWidth)/2.0f;
    FLOAT fProgY = pMenuHeader->m_YPos +  1 *  MENU_TEXT_HEIGHT + MENU_BORDER_HEIGHT;
    FLOAT fPercent = g_ContentManager.GetDownloadPercent();


    // draw scale
    DrawScale( fPercent, fProgX, fProgY, fProgWidth, fProgHeight );

    // draw outlined box
    D3DDevice_SetVertexShader( FVF_TEX0 );
    
    VERTEX_TEX0 Verts[4];
    for( DWORD i=0; i<4; i++ )
    {
        Verts[i].sz    = 0.0f;
        Verts[i].rhw   = 1.0f;
        Verts[i].color =  0xffffffff;
        Verts[i].specular = 0x00000000;
    }

    FLOAT x0 = fProgX;
    FLOAT x1 = fProgX + fProgWidth;
    FLOAT y0 = fProgY;
    FLOAT y1 = fProgY + fProgHeight;
    
    Verts[0].sx = x0;
    Verts[0].sy = y0;
    Verts[1].sx = x0;
    Verts[1].sy = y1;
    Verts[2].sx = x1;
    Verts[2].sy = y1;
    Verts[3].sx = x1;
    Verts[3].sy = y0;

    D3DDevice_DrawVerticesUP( D3DPT_LINELOOP, 4, Verts, sizeof(Verts[0]) );
}


//-----------------------------------------------------------------------------
// Name: Menu_Select
// Desc: The select menu for contentdowload
// NOTE: scrollable
//-----------------------------------------------------------------------------
static BOOL HandleContentSelect( MENU_HEADER* pMenuHeader, DWORD input );
static void CreateContentSelect(MENU_HEADER *menuHeader, MENU *menu);
static VOID DrawContentSelect( MENU_HEADER* pMenuHeader, MENU* pMenu );

extern MENU ContentSelect = 
{
    TEXT_NONE, // SELECT CONTENT
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y, // Menu type
    CreateContentSelect,                          // Create menu function
    HandleContentSelect,                          // Input handler function
    DrawContentSelect,                            // Menu draw function
    0,                                            // X coord
    0,                                            // Y coord
};

//-----------------------------------------------------------------------------
// Menu_Select Create, Handle, and Draw functions
//-----------------------------------------------------------------------------
VOID CreateContentSelect(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    // free old selection strings
    FreeSelect();


    // make strings (only use first string of enum blob)
    for(UINT i = 0; i < g_ContentManager.GetNumInfos(); i++)
    {
        ContentInfo* pInfo = g_ContentManager.GetInfo(i);
        ParseEnum(pInfo->GetEnumBlob(), pInfo->GetEnumBlobSize());
        SelectStrings.push_back(EnumStrings[0]);
    }

    // init display
    // max enum blob width
    //$MD: $BUGBUG: should be a define
    pMenuHeader->m_ItemTextWidth = 250;

    // up arrow for scrolling
    pMenuHeader->AddMenuItem(L"", MENU_ITEM_ACTIVE);
    
    // items
    for(UINT i = 0; i < DISPLAY_MAX; i++)
        pMenuHeader->AddMenuItem(L"");
    
    // down arrow for scrolling
    pMenuHeader->AddMenuItem(L"", MENU_ITEM_ACTIVE);
    
    // current selection
    pMenu->CurrentItemIndex = long(-1);

    // back or select
    pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_BUTTON_A_SELECT_B_BACK), MENU_ITEM_ACTIVE);
}


BOOL HandleContentSelect( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_BACK:
            // cancel
            g_bCancel = true;
            return FALSE;

        case MENU_INPUT_UP:
        {
            // scroll up
            if(g_OptionalDownloadEngine.m_dwNumItems == 0)
                return FALSE;

            if(g_OptionalDownloadEngine.m_dwCurrentItem == 0)
                return FALSE;

            g_OptionalDownloadEngine.m_dwCurrentItem--;
            if(g_OptionalDownloadEngine.m_dwDisplayTop != 0 &&
                g_OptionalDownloadEngine.m_dwCurrentItem == g_OptionalDownloadEngine.m_dwDisplayTop - 1)
            {
                g_OptionalDownloadEngine.m_dwDisplayTop--;
                return TRUE;
            }
            else
                return SelectPreviousMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
        }   
    
        case MENU_INPUT_DOWN:
        {
            // scroll down
            if(g_OptionalDownloadEngine.m_dwNumItems == 0)
                return FALSE;

            if(g_OptionalDownloadEngine.m_dwCurrentItem == g_OptionalDownloadEngine.m_dwNumItems - 1)
                return FALSE;

            g_OptionalDownloadEngine.m_dwCurrentItem++;

            if(g_OptionalDownloadEngine.m_dwCurrentItem == g_OptionalDownloadEngine.m_dwDisplayTop + DISPLAY_MAX)
            {
                g_OptionalDownloadEngine.m_dwDisplayTop++;
                return TRUE;
            }
            else
                return SelectNextMenuItem( pMenuHeader, pMenuHeader->m_pMenu, pMenuHeader->m_pCurrentItem );
        }
    
        case MENU_INPUT_SELECT:
            // select
            g_bContinue = true;
            return TRUE;
            break;
    }

    return FALSE;
}

VOID DrawContentSelect( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // $MD: BUGBUG: should be set in menu struct
    // $LOCALIZE
    pMenuHeader->m_strTitle = g_strTitle;

    // set current index;
    pMenu->CurrentItemIndex = g_OptionalDownloadEngine.m_dwCurrentItem - g_OptionalDownloadEngine.m_dwDisplayTop + 1;

    // up arrow if more items above
    if(g_OptionalDownloadEngine.m_dwDisplayTop != 0)
        pMenuHeader->m_pMenuItem[0]->Data = L"\x2191";
    else
        pMenuHeader->m_pMenuItem[0]->Data = L"";

    // down arrow if more item below
    if(g_OptionalDownloadEngine.m_dwDisplayTop + DISPLAY_MAX < g_OptionalDownloadEngine.m_dwNumItems)
        pMenuHeader->m_pMenuItem[DISPLAY_MAX + 1]->Data = L"\x2193";
    else
        pMenuHeader->m_pMenuItem[DISPLAY_MAX + 1]->Data = L"";

    // fill in currenct DISPLAY_MAX window of itmes
    for(short i = 0; i < DISPLAY_MAX; i++)
    {
        MENU_ITEM* pMenuItem = pMenuHeader->m_pMenuItem[i + 1];
        if(g_OptionalDownloadEngine.m_dwDisplayTop + i < g_OptionalDownloadEngine.m_dwNumItems)
        {
            // only show first string of enum blob
            DWORD dwString = g_OptionalDownloadEngine.m_dwDisplayTop + i;
            if(dwString < SelectStrings.size())
                pMenuItem->Data = (void*)SelectStrings[dwString].c_str();
            else
                pMenuItem->Data = L"";
        }
        else
            pMenuItem->Data = L"";
    }
}


//-----------------------------------------------------------------------------
// Name: ContentDetails
// Desc: The general menu for success or error
//-----------------------------------------------------------------------------
static BOOL HandleContentDetail( MENU_HEADER* pMenuHeader, DWORD input );
static VOID DrawContentDetail( MENU_HEADER* pMenuHeader, MENU* pMenu );
static void CreateContentDetail(MENU_HEADER *menuHeader, MENU *menu);

extern MENU ContentDetail = 
{
    TEXT_NONE,//L"Update Content",
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y ,               // Menu type
    CreateContentDetail,                                                   // Create menu function
    HandleContentDetail,                                                   // Input handler function
    DrawContentDetail,                                                     // Menu draw function
    0,                                                           // X coord
    0,                                                           // Y Coord
};
//-----------------------------------------------------------------------------
// Menu_ContentDetails Create, Handle, and Draw functions
//-----------------------------------------------------------------------------
void CreateContentDetail(MENU_HEADER *pMenuHeader, MENU *pMenu)
{
    ContentDetails* pDetails = g_ContentManager.GetWorkingDetails();
    ContentInfo* pInfo = g_ContentManager.GetWorkingInfo();

     // init display
    //$MD: $BUGBUG: should be a define
    pMenuHeader->m_ItemTextWidth = 250;

    // parse enum info
    ParseEnum(pInfo->GetEnumBlob(), pInfo->GetEnumBlobSize());

    // parse details info
    ParseDetails(pDetails->GetDetailsBlob(), pDetails->GetDetailsBlobSize());

     // add first enum string
    pMenuHeader->AddMenuItem(EnumStrings[0].c_str());

    // room for details texture
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");
    pMenuHeader->AddMenuItem(L"");

    // add the rest of the enum strings
    for(UINT i = 1; i < EnumStrings.size(); i++)
        pMenuHeader->AddMenuItem(EnumStrings[i].c_str());

    // blanck space
    pMenuHeader->AddMenuItem(L"");

    // add offering desc
    for(UINT i = 0; i < OfferingDesc.size(); i++)
    {
        pMenuHeader->AddMenuItem(OfferingDesc[i].c_str());
    }
    
    // show if content is already owned
    if(pDetails->GetNumInstances() != 0)
    {
        if(XONLINE_OFFERING_SUBSCRIPTION == pInfo->GetOfferingType() )
            pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_ALREADY_SUB)); 
        else
            pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_ALREADY_OWN)); 

    }

    // blanck space
    pMenuHeader->AddMenuItem(L"");

    // subscribe, cancel subscritpion, purchase, or download based off of whether
    // this item is a subscription, it is owned, and the price.
    if(XONLINE_OFFERING_SUBSCRIPTION == pInfo->GetOfferingType() )
    {
        if( pDetails->GetNumInstances() == 0)
            pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_SUB_BACK)); 
        else
            pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_CANELSUB_BACK));
    }
    else
    {
        if(pDetails->GetPrice().fOfferingIsFree || pDetails->GetNumInstances() != 0)
            pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_DOWNLOAD_BACK));
        else
            pMenuHeader->AddMenuItem(TEXT_TABLE(TEXT_PURCHASE_BACK));
    }

    // nothing is selectable
    pMenu->CurrentItemIndex = long(-1);
}
BOOL HandleContentDetail( MENU_HEADER* pMenuHeader, DWORD dwInput )
{
    switch( dwInput )
    {
        case MENU_INPUT_SELECT:
            // purchase, cancel, or download depending on the contect
            g_bContinue = true;
            return TRUE;
        case MENU_INPUT_BACK:
            // back to selection menu
            g_bCancel = true;
            return TRUE;
        
    }
    return FALSE;
}
VOID DrawContentDetail( MENU_HEADER* pMenuHeader, MENU* pMenu )
{
    // $LOCALIZE
    // $BUGBUG: should be set in menu struct
    pMenuHeader->m_strTitle = g_strTitle;

    // dimetions of texture
    FLOAT fWidth   = 128.0f;
    FLOAT fHeight = 128.0f;

    FLOAT fX = pMenuHeader->m_XPos + (pMenuHeader->m_XSize - fWidth)/2.0f;
    FLOAT fY = pMenuHeader->m_YPos +  1*  MENU_TEXT_HEIGHT + MENU_BORDER_HEIGHT;
    
    // render texture
    SET_TEXTURE(0, pTexture);
    FOG_OFF();
    BLEND_OFF();
    D3DDevice_SetVertexShader( FVF_TEX1 );
    
    VERTEX_TEX1 Verts[4];
    for( DWORD i=0; i<4; i++ )
    {
        Verts[i].sz    = 0.0f;
        Verts[i].rhw   = 1.0f;
        Verts[i].color =  0xffffffff;
        Verts[i].specular = 0x00000000;
    }

    FLOAT x0 = fX;
    FLOAT x1 = fX + fWidth;
    FLOAT y0 = fY;
    FLOAT y1 = fY + fHeight;
    
    Verts[0].sx = x0;
    Verts[0].sy = y0;
    Verts[0].tu = 0.0f;
    Verts[0].tv = 0.0f;


    Verts[1].sx = x1;
    Verts[1].sy = y0;
    Verts[1].tu = 1.0f;
    Verts[1].tv = 0.0f;

    Verts[2].sx = x1;
    Verts[2].sy = y1;
    Verts[2].tu = 1.0f;
    Verts[2].tv = 1.0f;

    Verts[3].sx = x0;
    Verts[3].sy = y1;
    Verts[3].tu = 0.0f;
    Verts[3].tv = 1.0f;

    // draw outlined box
    D3DDevice_DrawVerticesUP( D3DPT_LINELOOP, 4, Verts, sizeof(Verts[0]) );

    // draw textured quad
    D3DDevice_DrawVerticesUP( D3DPT_QUADLIST, 4, Verts, sizeof(Verts[0]) );

    // clear T page
    // $MD: required?

    SET_TPAGE(-1);
}

    

//-----------------------------------------------------------------------------
// Name: Return
// Desc: Overrided virtual return that cleans up
//-----------------------------------------------------------------------------            
VOID CContentDownloadEngine::Return(DWORD dwExistStatus)
{
    // clean up the download manager
    g_ContentManager.CleanUp(); 

    // free global stings and texture used by menus
    FreeSelect();  
    FreeDetails();
    FreeEnum();
    FreeOfferingDesc();
    FreeConfirm();

    CUIStateEngine::Return(dwExistStatus);
}


//-----------------------------------------------------------------------------
// Name: BeginError
// Desc: Begins the error state, sets a message
//-----------------------------------------------------------------------------  
VOID CContentDownloadEngine::BeginError( const WCHAR* strErrorString, const WCHAR* strTitle)
{
    static WCHAR strMsgBuf[MAX_MESSAGE];
    swprintf(strMsgBuf, L"%s\n%s", TEXT_TABLE(TEXT_CONTENTDOWNLOAD_ERROR), strErrorString);

    // set message menu
    SetMsgMenu(strTitle, strMsgBuf, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE), NULL),
    
    m_State = CONTENTDOWNLOAD_STATE_ERROR;
}

//-----------------------------------------------------------------------------
// Name: BeginSuccess
// Desc: Begins the success state, sets a message
//-----------------------------------------------------------------------------  
VOID CContentDownloadEngine::BeginSuccess( const WCHAR* strSuccessString, const WCHAR* strTitle)
{
    // set message menu
    SetMsgMenu(strTitle, strSuccessString, TEXT_TABLE(TEXT_BUTTON_A_CONTINUE), NULL),

    m_State = CONTENTDOWNLOAD_STATE_SUCCESS;
}

//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT CRequiredDownloadEngine::Process()
{
    static BOOL bShowPause;
    static DWORD dwNextState;

    // title
    static const WCHAR* strTitle = TEXT_TABLE(TEXT_CONTENTDOWNLOAD_ONLINEUPDATE);

    switch( m_State )
    {
        case CONTENTDOWNLOAD_STATE_BEGIN:

            // $NOTE: Content manager should already be inited!

            // fallthrough

        case CONTENTDOWNLOAD_STATE_SHOW_ENUM:
            
            // show enum
            SetMsgMenu( strTitle, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_CHECKINGFORNEWCONTENT),
                        NULL, TEXT_TABLE(TEXT_BUTTON_B_CANCEL));
            
            m_State = CONTENTDOWNLOAD_STATE_ENUM;
            // fall through
        
        case CONTENTDOWNLOAD_STATE_ENUM:

            // cancell
            if(g_bCancel)
            {
                dwNextState = CONTENTDOWNLOAD_STATE_SHOW_ENUM;
                m_State = CONTENTDOWNLOAD_STATE_SHOWABORT;
                break;
            }   
            
            // error
            if(g_ContentManager.Error())
            {
                BeginError(g_ContentManager.GetErrorString(), strTitle);
                break;
            }

             // update enum if we are still working
            if(g_ContentManager.Working())
                g_ContentManager.UpdateEnum();

            // wait for two seconds
            else if(GetMenuTime() > 2000)
            {
                // begin download 
                if(g_ContentManager.GetNumInfos() != 0)
                {
                    g_ContentManager.BeginDownloadAll();
                    m_State = CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD;
                }
                // no new content
                else
                {
                    BeginSuccess(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_NO_NEW_CONTENT), strTitle); 
                    break;
                }
            }
            break;

        case CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD:
                SetMenu(&ContentDownload, strTitle);
                m_State = CONTENTDOWNLOAD_STATE_DOWNLOAD;
                bShowPause = TRUE;

        case CONTENTDOWNLOAD_STATE_DOWNLOAD:
            
            if(g_bCancel)
            {
                dwNextState = CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD;
                m_State = CONTENTDOWNLOAD_STATE_SHOWABORT;
                break;
            }

            // error
            if(g_ContentManager.Error())
            {
                BeginError(g_ContentManager.GetErrorString(), strTitle);
                break;
            }

            // update download if we are still working
            if(g_ContentManager.Working())
                g_ContentManager.UpdateDownload();

            // pause for two seconds
            else if(bShowPause)
            {
                bShowPause = FALSE;
                g_dwStartTime = timeGetTime();
            }
            else if(GetMenuTime() > 2000 )
            {
                
                // read all car packages
                if (!ReadAllCarPackagesMultiple())
                {
                    BeginError(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_COULDNOTREADNEWCARS), strTitle);
                    break;
                }
                CalcCarStats();

                // read car keys
                if (!ReadAllCarKeysMultiple())
                {
                    BeginError(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_COULDNOTREADNEWCARS), strTitle);
                    break;
                }
                BeginSuccess(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_UPDATECOMPLETE), strTitle);
    
            }
            break;

        case CONTENTDOWNLOAD_STATE_SHOWABORT:
            SetMsgMenu( strTitle, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_CANNOT_PLAY_WITHOUT_UPDATING),
                        TEXT_TABLE(TEXT_BUTTON_A_CONTINUE), TEXT_TABLE(TEXT_BUTTON_B_ABORT));
                           
            m_State = CONTENTDOWNLOAD_STATE_ABORT;
            break;

        case CONTENTDOWNLOAD_STATE_ABORT:
            if(g_bContinue)
                m_State = dwNextState;
            else if(g_bCancel)
            {
                g_ContentManager.CancelTask();
                Return(STATEENGINE_TERMINATED);
            }
            break;
            
        case CONTENTDOWNLOAD_STATE_SUCCESS:
            if(g_bContinue)
                // return terminated if aborted before finish
                Return( STATEENGINE_COMPLETED );
            break;

        case CONTENTDOWNLOAD_STATE_ERROR:
            if(g_bContinue)
                Return( STATEENGINE_TERMINATED );
            break;
    }
    
    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}


//-----------------------------------------------------------------------------
// Name: Process()
// Desc: Main control function for the state engine
//-----------------------------------------------------------------------------
HRESULT COptionalDownloadEngine::Process()
{
    static DWORD dwNextState;
    static BOOL bShowPause;
    static BOOL bQuit;

    // title
    static const WCHAR* strTitle = TEXT_TABLE(TEXT_ONLINESTORE);

    static WCHAR strShowBuf[MAX_MESSAGE];

    switch( m_State )
    {
        case CONTENTDOWNLOAD_STATE_BEGIN:
            // init manager
            g_ContentManager.Init();
            bQuit = false;
            m_State = CONTENTDOWNLOAD_STATE_SHOW_ENUM;

            //g_pTitleScreenCamera->SetNewPos( TITLESCREEN_CAMPOS_START );
            // fall throught
        
        case CONTENTDOWNLOAD_STATE_SHOW_ENUM:
            SetMsgMenu( strTitle, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_CHECKINGFORNEWCONTENT),
                        NULL, TEXT_TABLE(TEXT_BUTTON_B_CANCEL));
            
            
            g_ContentManager.BeginEnum( 0xFFFFFFFF, XONLINE_OFFERING_CONTENT | XONLINE_OFFERING_SUBSCRIPTION, TRUE );

            m_State = CONTENTDOWNLOAD_STATE_ENUM;
            // fall through
        
        case CONTENTDOWNLOAD_STATE_ENUM:

            if(g_bCancel)
            {
                Return(STATEENGINE_TERMINATED);
                break;
            }   
            
            // error
            if(g_ContentManager.Error())
            {
                BeginError(g_ContentManager.GetErrorString(), strTitle);
                break;
            }

             // update enum if we are still working
            if(g_ContentManager.Working())
                g_ContentManager.UpdateEnum();

            // wait for two seconds
            else if(GetMenuTime() > 2000)
            {
                if(g_ContentManager.GetNumInfos() != 0)
                {
                    m_dwDisplayTop = 0;
                    m_dwCurrentItem = 0;
                    m_dwNumItems =  g_ContentManager.GetNumInfos();
                    m_State = CONTENTDOWNLOAD_STATE_SHOW_SELECT;
                    
                }
                else
                {
                    bQuit = TRUE;
                    BeginSuccess(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_NO_NEW_CONTENT), strTitle); 
                    break;
                }
            }
            break;

        
        case CONTENTDOWNLOAD_STATE_SHOW_SELECT:
            SetMenu( &ContentSelect, strTitle);
            m_State = CONTENTDOWNLOAD_STATE_SELECT;
            // fallthrough
            
        case CONTENTDOWNLOAD_STATE_SELECT:
            if(g_bCancel)
                Return(STATEENGINE_TERMINATED);
            if(g_bContinue)
            {
                m_State = CONTENTDOWNLOAD_STATE_SHOW_GETDETAILS;
                g_ContentManager.BeginGetDetails(m_dwCurrentItem);
            }
            break;

        case CONTENTDOWNLOAD_STATE_SHOW_GETDETAILS:
            // menu
            SetMsgMenu( strTitle, TEXT_TABLE(TEXT_CONTENTDOWNLOAD_GETTING_DETAILS),
                        NULL, TEXT_TABLE(TEXT_BUTTON_B_CANCEL));
            m_State = CONTENTDOWNLOAD_STATE_GETDETAILS;
            // fall through

        case CONTENTDOWNLOAD_STATE_GETDETAILS:
            if(g_bCancel)
            {
                m_State = CONTENTDOWNLOAD_STATE_SHOW_SELECT;
                g_ContentManager.CancelTask();
                break;
            }

            // error
            if(g_ContentManager.Error())
            {
                BeginError(g_ContentManager.GetErrorString(), strTitle);
                break;
            }
        
             // update enum if we are still working
            if(g_ContentManager.Working())
                g_ContentManager.UpdateGetDetails();

            // wait for two seconds
            else if(GetMenuTime() > 2000)
            {
                // make offering Desc
                MakeOfferingDesc();
                m_State = CONTENTDOWNLOAD_STATE_SHOW_DETAILS;
            }

            break;

        case CONTENTDOWNLOAD_STATE_SHOW_DETAILS:
            SetMenu( &ContentDetail , strTitle);
            m_State = CONTENTDOWNLOAD_STATE_DETAILS;
            // fall through

        case CONTENTDOWNLOAD_STATE_DETAILS:
            if(g_bCancel)
                m_State = CONTENTDOWNLOAD_STATE_SHOW_SELECT;
            if(g_bContinue)
            {
                ContentInfo* pInfo = g_ContentManager.GetWorkingInfo();
                ContentDetails* pDetails = g_ContentManager.GetWorkingDetails();

                if(!pDetails->GetPrice().fOfferingIsFree && !g_ContentManager.CurrentUserCanPurchase())
                {
                    SetMsgMenu(strTitle, TEXT_TABLE(TEXT_BILLING_PERMISSIONS_DISABLED),
                               TEXT_TABLE(TEXT_BUTTON_A_CONTINUE), NULL);
                    m_State = CONTENTDOWNLOAD_STATE_CANT_PURCHASE;
                    break;
                }

                // free or owned content
                if(XONLINE_OFFERING_SUBSCRIPTION != pInfo->GetOfferingType() && (pDetails->GetPrice().fOfferingIsFree || pDetails->GetNumInstances() != 0))
                {
                    // download
                    g_ContentManager.BeginDownload(m_dwCurrentItem);
                    m_State = CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD;
                }
                else
                {
                    // add space
                    OfferingDesc.push_back(L"");

                  
                    if(XONLINE_OFFERING_SUBSCRIPTION == pInfo->GetOfferingType())
                    {
                        // cancel sub
                        if(pDetails->GetNumInstances() > 0 )
                        {
                            // confim cancel sub
                            OfferingDesc.push_back(TEXT_TABLE(TEXT_CONFIRM_CANCEL_SUB));
                            m_State = CONTENTDOWNLOAD_STATE_CONFIRM_CANCEL_SUBSCRIPTION;
                        }
                        // subscribe
                        else
                        {
                            OfferingDesc.push_back(TEXT_TABLE(TEXT_CONFIRM_SUB));
                            m_State = CONTENTDOWNLOAD_STATE_CONFIRM_PURCHASE_SUBSCRIPTION;
                        }
                    }
                    else
                    {
                        //purchase
                        OfferingDesc.push_back(TEXT_TABLE(TEXT_CONFIRM_PURCHASE));
                        m_State = CONTENTDOWNLOAD_STATE_CONFIRM_PURCHASE_CONTENT;
                    }

                    // Set confirm menu
                    SetConfirmMenu(strTitle, OfferingDesc);
                }

            }
            break;

        case CONTENTDOWNLOAD_STATE_CANT_PURCHASE:
            if(g_bContinue)
                m_State = CONTENTDOWNLOAD_STATE_SHOW_DETAILS;
            break;

        case CONTENTDOWNLOAD_STATE_CONFIRM_PURCHASE_CONTENT:
            if(g_bCancel)
            {
                // We tacked on some extra strings before we got
                // here, so now let's remove them
                OfferingDesc.pop_back();
                OfferingDesc.pop_back();
                m_State = CONTENTDOWNLOAD_STATE_SHOW_DETAILS;
            }
            if(g_bContinue)
            {
                g_ContentManager.BeginPurchase(m_dwCurrentItem);
                m_State = CONTENTDOWNLOAD_STATE_SHOW_PURCHASE_CONTENT;
            }
            break;

        case CONTENTDOWNLOAD_STATE_CONFIRM_PURCHASE_SUBSCRIPTION:
            if(g_bCancel)
            {
                // We tacked on some extra strings before we got
                // here, so now let's remove them
                OfferingDesc.pop_back();
                OfferingDesc.pop_back();
                m_State = CONTENTDOWNLOAD_STATE_SHOW_DETAILS;
            }
            if(g_bContinue)
            {
                g_ContentManager.BeginPurchase(m_dwCurrentItem);
                m_State = CONTENTDOWNLOAD_STATE_SHOW_PURCHASE_SUBSCRIPTION;
            }
            break;

        case CONTENTDOWNLOAD_STATE_CONFIRM_CANCEL_SUBSCRIPTION:
            if(g_bCancel)
                m_State = CONTENTDOWNLOAD_STATE_DETAILS;
            if(g_bContinue)
            {
                g_ContentManager.BeginCancelSubscription(m_dwCurrentItem);
                m_State = CONTENTDOWNLOAD_STATE_SHOW_CANCEL_SUBSCRIPTION;
            }
            break;
                        

        case CONTENTDOWNLOAD_STATE_SHOW_PURCHASE_CONTENT:
            swprintf(strShowBuf, L"%s\n%s", TEXT_TABLE(TEXT_CONTENTDOWNLOAD_PURCHASING_CONTENT), TEXT_TABLE(TEXT_DONT_TURN_OFF));
            SetMsgMenu( strTitle, strShowBuf, NULL, NULL);
            m_State = CONTENTDOWNLOAD_STATE_PURCHASE;
            break;

        case CONTENTDOWNLOAD_STATE_SHOW_PURCHASE_SUBSCRIPTION:
            swprintf(strShowBuf, L"%s\n%s", TEXT_TABLE(TEXT_CONTENTDOWNLOAD_PURCHASING_SUB), TEXT_TABLE(TEXT_DONT_TURN_OFF));
            SetMsgMenu( strTitle, strShowBuf, NULL, NULL);
            m_State = CONTENTDOWNLOAD_STATE_PURCHASE;
            break;
        
        case CONTENTDOWNLOAD_STATE_PURCHASE:
            // error
            if(g_ContentManager.Error())
            {
                BeginError(g_ContentManager.GetErrorString(), strTitle);
                break;
            }
        
             // update enum if we are still working
            if(g_ContentManager.Working())
                g_ContentManager.UpdatePurchase();

            // wait for two seconds
            else if(GetMenuTime() > 2000)
            {
                ContentInfo* pInfo = g_ContentManager.GetWorkingInfo();
                if(XONLINE_OFFERING_SUBSCRIPTION != pInfo->GetOfferingType() )
                {
                    m_State = CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD;
                    g_ContentManager.BeginDownload(m_dwCurrentItem);
                }
                else
                {
                    // $MD: NOTE: we only have one subscription, so
                    //            mark that it has been purchased here
                    g_bHasSubscription = TRUE;
                    BeginSuccess(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_HAVE_FUN), strTitle); //L"Have Fun!"
                }
                
            }
            
            break;


        case CONTENTDOWNLOAD_STATE_SHOW_CANCEL_SUBSCRIPTION:
            swprintf(strShowBuf, L"%s\n%s", TEXT_TABLE(TEXT_CONTENTDOWNLOAD_CANCELING_SUB), TEXT_TABLE(TEXT_DONT_TURN_OFF));
            SetMsgMenu( strTitle, strShowBuf, NULL, NULL);
            m_State = CONTENTDOWNLOAD_STATE_CANCEL_SUBSCRIPTION;
            break;
        
        case CONTENTDOWNLOAD_STATE_CANCEL_SUBSCRIPTION:
            // error
            if(g_ContentManager.Error())
            {
                BeginError(g_ContentManager.GetErrorString(), strTitle);
                break;
            }
        
             // update enum if we are still working
            if(g_ContentManager.Working())
                g_ContentManager.UpdateCancelSubscription();

            // wait for two seconds
            else if(GetMenuTime() > 2000)
            {
                // $MD: NOTE: we only have one subscription, so
                //            mark that it has been cancelled here
                g_bHasSubscription = FALSE;
                BeginSuccess(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_SUB_CANCELLED), strTitle);  
            }
            
            break;

        case CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD:
                SetMenu(&ContentDownload, strTitle);
                m_State = CONTENTDOWNLOAD_STATE_DOWNLOAD;
                bShowPause = TRUE;

        case CONTENTDOWNLOAD_STATE_DOWNLOAD:
            
            if(g_bCancel)
            {
                dwNextState = CONTENTDOWNLOAD_STATE_SHOWDOWNLOAD;
                m_State = CONTENTDOWNLOAD_STATE_SHOWABORT;
                break;
            }

            // error
            if(g_ContentManager.Error())
            {
                BeginError(g_ContentManager.GetErrorString(), strTitle);
                break;
            }

            // update download if we are still working
            if(g_ContentManager.Working())
                g_ContentManager.UpdateDownload();

            // pause for two seconds
            else if(bShowPause)
            {
                bShowPause = FALSE;
                g_dwStartTime = timeGetTime();
            }
            else if(GetMenuTime() > 2000 )
            {
                
                // read all car packages
                if (!ReadAllCarPackagesMultiple())
                {
                    BeginError(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_COULDNOTREADNEWCARS), strTitle);
                    break;
                }
                CalcCarStats();

                // read car keys
                if (!ReadAllCarKeysMultiple())
                {
                    BeginError(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_COULDNOTREADNEWCARS), strTitle);
                    break;
                }
                BeginSuccess(TEXT_TABLE(TEXT_CONTENTDOWNLOAD_HAVE_FUN), strTitle); 
    
            }
            break;

        case CONTENTDOWNLOAD_STATE_SHOWABORT:
            SetMsgMenu( strTitle, TEXT_TABLE(TEXT_ARE_YOU_SURE),
                        TEXT_TABLE(TEXT_BUTTON_A_CONTINUE), TEXT_TABLE(TEXT_BUTTON_B_ABORT));
            m_State = CONTENTDOWNLOAD_STATE_ABORT;
            break;

        case CONTENTDOWNLOAD_STATE_ABORT:
            if(g_bContinue)
                m_State = dwNextState;
            else if(g_bCancel)
            {
                g_ContentManager.CancelTask();
                m_State = CONTENTDOWNLOAD_STATE_SHOW_DETAILS;
            }
            break;
            
        case CONTENTDOWNLOAD_STATE_SUCCESS:
            if(g_bContinue)
            {
                if(!bQuit)
                {
                    // remove content info if not subscripton
                    ContentInfo* pInfo = g_ContentManager.GetWorkingInfo();
                    
                    if(XONLINE_OFFERING_SUBSCRIPTION != pInfo->GetOfferingType() )
                    {
                        g_ContentManager.RemoveInfo(m_dwCurrentItem);
                        // reset list
                        m_dwDisplayTop = 0;
                        m_dwCurrentItem = 0;
                        m_dwNumItems =  g_ContentManager.GetNumInfos();

                        // quit if no more content
                        if(m_dwNumItems != 0)
                        {
                            m_State = CONTENTDOWNLOAD_STATE_SHOW_SELECT;
                            break;
                        }
                    }
                    else
                    {
                        m_State = CONTENTDOWNLOAD_STATE_SHOW_SELECT;
                        break;
                    }

                }
                Return( STATEENGINE_COMPLETED );
            }
            
            break;

        case CONTENTDOWNLOAD_STATE_ERROR:
            if(g_bContinue)
                Return( STATEENGINE_TERMINATED );
            break;
    }
    
    // Handle the menus
    g_pMenuHeader->HandleMenus();

    return S_FALSE;
}



    

