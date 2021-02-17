//-----------------------------------------------------------------------------
// File: credits.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "credits.h"
#include "timing.h"
#include "InitPlay.h"
#include "geom.h"
#include "camera.h"
#include "player.h"
#include "ui_Menu.h"
#include "ui_MenuDraw.h"
#include "ui_MenuText.h"
#include "ui_TitleScreen.h"
#include "gameloop.h"
#include "text.h"
#include "draw.h"


// Text size and positioning defines
#define CREDIT_TEXT_HEIGHT  20.0f

#define CREDIT_TEXT_INDENT  16.0f

#define CREDIT_X_BOUNDARY   64
#define CREDIT_X_INDENT     32
#define CREDIT_Y_BOUNDARY   64
#define CREDIT_Y_INDENT     32

#define CREDIT_CENTER_X     0x01
#define CREDIT_CENTER_Y     0x02
#define CREDIT_SCROLL       0x04
#define CREDIT_CENTER_TEXT  0x08
#define CREDIT_RANDOM_POS   0x10
 





#define CREDIT_MOVE_TIME             0.25f
#define CREDIT_FADE_TIME             0.25f
#define CREDIT_SHOW_TIME             1.50f
#define CREDIT_INIT_TIME             1.00f
#define CREDIT_EXIT_TIME            10.00f

#define CREDIT_MAX_TEXT_LINES       12
#define CREDIT_MIN_TEXT_LINES        4

BOOL SetNextCreditEntry();
VOID DrawCreditFadePoly();
VOID ChangeCreditCamera();




WCHAR*           g_pCreditStrings   = NULL;
NEW_CREDIT_PAGE* g_pCreditPages     = NULL;
DWORD            g_dwNumCreditPages = 0;
CREDIT_VARS      g_CreditVars;





//-----------------------------------------------------------------------------
// Name: InitCreditEntries()
// Desc: Initialise the credit entries
//-----------------------------------------------------------------------------
void InitCreditEntries()
{
    // Determine which localized version of the credits file to use
    CHAR* strCreditsFile = "d:\\Strings\\Credits_English.inf";
    switch( XGetLanguage() )
    {
        case XC_LANGUAGE_GERMAN:
            strCreditsFile = "d:\\Strings\\Credits_German.inf";
            break;
        case XC_LANGUAGE_FRENCH:
            strCreditsFile = "d:\\Strings\\Credits_French.inf";
            break;
        case XC_LANGUAGE_ITALIAN:
            strCreditsFile = "d:\\Strings\\Credits_Italian.inf";
            break;
        case XC_LANGUAGE_JAPANESE:
            strCreditsFile = "d:\\Strings\\Credits_Japanese.inf";
            break;
        case XC_LANGUAGE_SPANISH:
            strCreditsFile = "d:\\Strings\\Credits_Spanish.inf";
            break;

        // Not yet supported:
//      case XC_LANGUAGE_PORTUGUESE:
//      case XC_LANGUAGE_KOREAN:
//      case XC_LANGUAGE_CHINESE:
    }

    // Open the credits file
    FILE* file = fopen( strCreditsFile, "r" );
    assert( file != NULL );
    
    // Get the file size
    fseek( file, 0, SEEK_END );
    long dwFileLength = ftell( file );
    fseek( file, 0, SEEK_SET );

    // Read in the strings
    g_pCreditStrings = (WCHAR*)new BYTE[dwFileLength];
    fread( g_pCreditStrings, 1, dwFileLength, file );
    fclose( file );

    // Skip the unicode marker
    WCHAR* pStrings = g_pCreditStrings+1;

    // Count the number of credit pages 
    g_dwNumCreditPages = 0;
    while( pStrings-g_pCreditStrings < dwFileLength/2 )
    {
        // Check for a new credit page
        if( !_wcsnicmp( pStrings, L"<TITLE>", 7 ) )
            g_dwNumCreditPages++;

        // Advance and null-terminate the string
        while( *pStrings != 13 && ( pStrings-g_pCreditStrings < dwFileLength/2 ) )
            pStrings++;
        *pStrings++ = 0; // Skip the 13
        *pStrings++ = 0; // Skip the 10
    }

    // Reset the string
    pStrings = g_pCreditStrings+1;

    // Allocate the credit pages
    g_pCreditPages = new NEW_CREDIT_PAGE[g_dwNumCreditPages];

    // Loop through all credit pages
    for( DWORD i=0; i<g_dwNumCreditPages; i++ )
    {
        NEW_CREDIT_PAGE* page = &g_pCreditPages[i];

        // Get the credit page title
        g_pCreditPages[i].strTitle = pStrings+8;
        pStrings += wcslen(pStrings)+2;

        // Compute the initial page width, based on the title
        g_pFont->SetScaleFactors( 1.5f, 1.5f );
        g_pCreditPages[i].fWidth = g_pFont->GetTextWidth( g_pCreditPages[i].strTitle );
        g_pFont->SetScaleFactors( 1.0f, 1.0f );

        // Fix Scroll speed for now
        g_pCreditPages[i].fScrollSpeed = 40.0f;

        // Get the credit page flags
        g_pCreditPages[i].dwFlags = 0;
        if( wcsstr( pStrings, L"CENTER_X" ) )    g_pCreditPages[i].dwFlags |= CREDIT_CENTER_X;
        if( wcsstr( pStrings, L"CENTER_Y" ) )    g_pCreditPages[i].dwFlags |= CREDIT_CENTER_Y;
        if( wcsstr( pStrings, L"CENTER_TEXT" ) ) g_pCreditPages[i].dwFlags |= CREDIT_CENTER_TEXT;
        if( wcsstr( pStrings, L"RANDOM_POS" ) )  g_pCreditPages[i].dwFlags |= CREDIT_RANDOM_POS;
        if( wcsstr( pStrings, L"SCROLL" ) )      g_pCreditPages[i].dwFlags |= CREDIT_SCROLL;
        pStrings += wcslen(pStrings)+2;

        // Save the string ptr position
        WCHAR* pStartingPoint = pStrings;

        // Count the subtitle headings
        g_pCreditPages[i].dwNumStrings = 0;
        while( pStrings-g_pCreditStrings < dwFileLength/2 )
        {
            // Check for a new credit page
            if( !_wcsnicmp( pStrings, L"<TITLE>", 7 ) )
                break;

            // Count strings
            g_pCreditPages[i].dwNumStrings++;

            // Advance to the next string
            pStrings += wcslen(pStrings)+2;
        }

        // Allocate ptrs for the strings
        g_pCreditPages[i].pStrings     = (WCHAR**)new DWORD[g_pCreditPages[i].dwNumStrings];
        g_pCreditPages[i].pStringTypes = new DWORD[g_pCreditPages[i].dwNumStrings];

        g_pCreditPages[i].dwNumTextLines = 0;

        // Rewind the string ptr and fill in string pointsrs
        pStrings = pStartingPoint;

        for( DWORD j=0; j<g_pCreditPages[i].dwNumStrings; j++ )
        {
            if( !_wcsnicmp( pStrings, L"<HEADING>", 9 ) )
            {
                g_pCreditPages[i].pStrings[j] = pStrings+10;
                g_pCreditPages[i].pStringTypes[j] = 1;
                g_pCreditPages[i].dwNumTextLines += 1;
            }
            else
            {
                g_pCreditPages[i].pStrings[j] = pStrings;
                g_pCreditPages[i].pStringTypes[j] = 0;
                g_pCreditPages[i].dwNumTextLines += 1;
            }

            // Adjust page width based on strings
            FLOAT fWidth = g_pFont->GetTextWidth( g_pCreditPages[i].pStrings[j] ) + CREDIT_TEXT_INDENT;
            if( fWidth > g_pCreditPages[i].fWidth )
                g_pCreditPages[i].fWidth = fWidth;
            
            // Advance to the next string
            pStrings += wcslen(pStrings)+2;
        }

        // Get Height
        if ((page->dwFlags & CREDIT_SCROLL) == 0) 
        {
            page->fHeight = (page->dwNumTextLines + 1) * CREDIT_TEXT_HEIGHT;
        } 
        else if (page->dwNumTextLines > CREDIT_MAX_TEXT_LINES) 
        {
            page->fHeight = (CREDIT_MAX_TEXT_LINES + 1) * CREDIT_TEXT_HEIGHT;
        } 
        else if (page->dwNumTextLines < CREDIT_MIN_TEXT_LINES) 
        {
            page->fHeight = (CREDIT_MIN_TEXT_LINES) * CREDIT_TEXT_HEIGHT;
        } 
        else 
        {
            page->fHeight = (page->dwNumTextLines + 1) * CREDIT_TEXT_HEIGHT;
        }

        // Choose a random position that fits on the screen if told to do so
        if (page->dwFlags & CREDIT_RANDOM_POS) 
        {
            FLOAT spare;
            // X position
            spare = (FLOAT)(MENU_SCREEN_WIDTH) - page->fWidth - (FLOAT)(MENU_FRAME_WIDTH) * 2 - (FLOAT)(CREDIT_X_BOUNDARY);
            if (spare < 0.0f) 
            {
                page->dwFlags |= CREDIT_CENTER_X;
            } 
            else 
            {
                page->fXPos = frand(spare) + (FLOAT)(MENU_FRAME_WIDTH) + (FLOAT)(CREDIT_X_INDENT);
            }
            // Y Position
            spare = (FLOAT)(MENU_SCREEN_HEIGHT) - page->fHeight - (FLOAT)(MENU_FRAME_HEIGHT) * 2 - (FLOAT)(CREDIT_Y_BOUNDARY);
            if (spare < 0.0f) 
            {
                page->dwFlags |= CREDIT_CENTER_Y;
            } 
            else 
            {
                page->fYPos = frand(spare) + (FLOAT)(MENU_FRAME_HEIGHT) + (FLOAT)(CREDIT_Y_INDENT);
            }
        }

        // Centre if necessary
        if (page->dwFlags & CREDIT_CENTER_X) 
        {
            page->fXPos = (640.0f - page->fWidth) / 2;
        }
        if (page->dwFlags & CREDIT_CENTER_Y) 
        {
            page->fYPos = (480.0f - page->fHeight) / 2;
        }
    }
}




////////////////////////////////////////////////////////////////
//
// Init Credit Control vars to inactive state
//
////////////////////////////////////////////////////////////////
void InitCreditStateInactive()
{

    g_CreditVars.State = CREDIT_STATE_INACTIVE;
    g_CreditVars.Timer = 0.0f;
    g_CreditVars.MaxTime = 0.0f;
    g_CreditVars.CurrentPage = 0;

}




////////////////////////////////////////////////////////////////
//
// Init Credit Control vars to inactive state
//
////////////////////////////////////////////////////////////////
void InitCreditStateActive()
{
    // Initialize the credits if we haven't done so yet
    if( 0 == g_dwNumCreditPages )
        InitCreditEntries();

    g_CreditVars.State = CREDIT_STATE_INIT;
    g_CreditVars.Timer = 0.0f;
    g_CreditVars.ScrollTimer = 0.0f;
    g_CreditVars.MaxTime = CREDIT_INIT_TIME;
    g_CreditVars.CurrentPage = -1;

    g_CreditVars.TimerCurrent = 0;
    g_CreditVars.TimerLast = 0;

    g_CreditVars.XPos = 320.0f;
    g_CreditVars.YPos = 240.0f;
    g_CreditVars.XSize = 0.0f;
    g_CreditVars.YSize = 0.0f;
    
    SetNextCreditEntry();

    g_CreditVars.ColIndex = SPRU_COL_YELLOW;

}




////////////////////////////////////////////////////////////////
//
// Process Credits In Game
//
////////////////////////////////////////////////////////////////

void ProcessCredits()
{
    NEW_CREDIT_PAGE *page;
    
    g_CreditVars.Timer += TimeStep;
    g_CreditVars.ScrollTimer += TimeStep;

    switch (g_CreditVars.State) 
    {
        case CREDIT_STATE_INACTIVE:
            // Do nothing
            break;

        case CREDIT_STATE_INIT:                         // Wait for Countdown
            if (CountdownTime == 0) 
            {
                ChangeCreditCamera();
                g_CreditVars.State = CREDIT_STATE_MOVING;
                g_CreditVars.MaxTime = CREDIT_MOVE_TIME;
                g_CreditVars.Timer = 0.0f;
                return;
            }

            g_CreditVars.Alpha = 0.0f;
            break;

        case CREDIT_STATE_MOVING:                       // Move Box
            // Change State?
            if (g_CreditVars.Timer > g_CreditVars.MaxTime) 
            {
                page = &g_pCreditPages[g_CreditVars.CurrentPage];

                g_CreditVars.State = CREDIT_STATE_FADE_IN;
                g_CreditVars.Timer = 0.0f;
                g_CreditVars.ScrollTimer = 0.0f;
            
                g_CreditVars.ScrollMaxTime = DivScalar(page->dwNumTextLines * CREDIT_TEXT_HEIGHT - CREDIT_TEXT_HEIGHT + page->fHeight, page->fScrollSpeed);
            
                if ((page->dwFlags & CREDIT_SCROLL) != 0) {
                    g_CreditVars.MaxTime = 0.0f;
                } else {
                    g_CreditVars.MaxTime = CREDIT_FADE_TIME;
                }

                g_CreditVars.XPos = g_CreditVars.DestXPos;
                g_CreditVars.YPos = g_CreditVars.DestYPos;
                g_CreditVars.XSize = g_CreditVars.DestXSize;
                g_CreditVars.YSize = g_CreditVars.DestYSize;

                return;
            }

            g_CreditVars.XPos += MulScalar(g_CreditVars.XVel, TimeStep);
            g_CreditVars.YPos += MulScalar(g_CreditVars.YVel, TimeStep);
            g_CreditVars.XSize += MulScalar(g_CreditVars.XGrow, TimeStep);
            g_CreditVars.YSize += MulScalar(g_CreditVars.YGrow, TimeStep);

            g_CreditVars.Alpha = 0.0f;
            break;

        case CREDIT_STATE_FADE_IN:                      // Fade Text In
            // Change State?
            if (g_CreditVars.Timer > g_CreditVars.MaxTime) 
            {
                page = &g_pCreditPages[g_CreditVars.CurrentPage];
            
                g_CreditVars.State = CREDIT_STATE_SHOWING;

                if ((page->dwFlags & CREDIT_SCROLL) != 0) 
                {
                    g_CreditVars.MaxTime = g_CreditVars.ScrollMaxTime;
                } 
                else 
                {
                    if (g_CreditVars.CurrentPage == g_dwNumCreditPages - 1) 
                    {
                        g_CreditVars.MaxTime = CREDIT_EXIT_TIME;
                    } 
                    else 
                    {
                        g_CreditVars.MaxTime = CREDIT_SHOW_TIME;
                    }
                }

                g_CreditVars.Timer = 0.0f;

                g_CreditVars.Alpha = 1.0f;
                return;
            }

            g_CreditVars.Alpha = DivScalar(g_CreditVars.Timer, g_CreditVars.MaxTime);
            break;

        case CREDIT_STATE_SHOWING:                      // Continue Showing Text
            // Scroll the credits

            // Change State?
            if (g_CreditVars.Timer > g_CreditVars.MaxTime) 
            {
                page = &g_pCreditPages[g_CreditVars.CurrentPage];

                g_CreditVars.State = CREDIT_STATE_FADE_OUT;

                if ((page->dwFlags & CREDIT_SCROLL) != 0) 
                {
                    g_CreditVars.MaxTime = 0.0f;
                } 
                else 
                {
                    g_CreditVars.MaxTime = CREDIT_FADE_TIME;
                }

                g_CreditVars.Timer = 0.0f;
                return;
            }
            break;

        case CREDIT_STATE_FADE_OUT:                     // Fade Text Out
            // Change State?
            if (g_CreditVars.Timer > g_CreditVars.MaxTime) 
            {
                g_CreditVars.Timer = 0.0f;
                g_CreditVars.Alpha = 0.0f;

                if (SetNextCreditEntry()) 
                {
                    g_CreditVars.State = CREDIT_STATE_MOVING;
                    g_CreditVars.MaxTime = CREDIT_MOVE_TIME;
                }
                else 
                {
                    g_CreditVars.State = CREDIT_STATE_DONE;
                    SetFadeEffect(FADE_DOWN);
                    GameLoopQuit = GAMELOOP_QUIT_FRONTEND;
                }

                ChangeCreditCamera();
                return;
            }

            g_CreditVars.Alpha = 1.0f - DivScalar(g_CreditVars.Timer, g_CreditVars.MaxTime);       
            break;

        case CREDIT_STATE_DONE:
            break;
    }
}




////////////////////////////////////////////////////////////////
//
// SetNextCreditEntry
//
////////////////////////////////////////////////////////////////
BOOL SetNextCreditEntry()
{
    if( g_CreditVars.CurrentPage + 1 >= (long)g_dwNumCreditPages ) 
        return FALSE;

    // Go to the next credit page
    g_CreditVars.CurrentPage++;

    NEW_CREDIT_PAGE* page = &g_pCreditPages[g_CreditVars.CurrentPage];

    g_CreditVars.DestXPos  = page->fXPos;
    g_CreditVars.DestYPos  = page->fYPos;
    g_CreditVars.DestXSize = page->fWidth;
    g_CreditVars.DestYSize = page->fHeight;

    g_CreditVars.XVel  = DivScalar( (g_CreditVars.DestXPos - g_CreditVars.XPos), CREDIT_MOVE_TIME );
    g_CreditVars.YVel  = DivScalar( (g_CreditVars.DestYPos - g_CreditVars.YPos), CREDIT_MOVE_TIME );
    g_CreditVars.XGrow = DivScalar( (g_CreditVars.DestXSize - g_CreditVars.XSize), CREDIT_MOVE_TIME );
    g_CreditVars.YGrow = DivScalar( (g_CreditVars.DestYSize - g_CreditVars.YSize), CREDIT_MOVE_TIME );

    return TRUE;
}




////////////////////////////////////////////////////////////////
//
// Change the camera mode and refereced object
//
////////////////////////////////////////////////////////////////
void ChangeCreditCamera()
{
    PLAYER *player;
    bool done;
    int cam;
    int iCount = 0;

    do 
    {
        done = TRUE;
        player = &Players[rand()%StartData.PlayerNum];

        if ((player->CarAI.AIState == CAI_S_REVERSE_CORRECT) ||
            (player->CarAI.AIState == CAI_S_FORWARD_CORRECT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_FORWARDLEFT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_FORWARDRIGHT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_REVERSELEFT) ||
            (player->CarAI.AIState == CAI_S_CORRECT_REVERSERIGHT))
        {
            done = FALSE;
        }

        if (++iCount >= StartData.PlayerNum) 
        {
            done = TRUE;
        }
    } while (!done);

    cam = rand() % 10;

    if (cam > 5) 
    {
        SetCameraRail(CAM_MainCamera, player->ownobj, CAM_RAIL_DYNAMIC_MONO);
    } 
    else 
    {
        SetCameraFollow(CAM_MainCamera, player->ownobj, cam);
    }
}




////////////////////////////////////////////////////////////////
//
// Draw Credits
//
////////////////////////////////////////////////////////////////
void DrawCredits()
{
    FLOAT xPos, yPos, xSize, fade, xOff;
    long col, alpha;
    NEW_CREDIT_PAGE *page;

    page = &g_pCreditPages[g_CreditVars.CurrentPage];

    // Draw Fade poly
    DrawCreditFadePoly();

    // Draw the title
    g_pFont->SetScaleFactors( 1.5f, 1.5f );
    xSize = g_pFont->GetTextWidth( page->strTitle );
    g_pFont->SetScaleFactors( 1.0f, 1.0f );
    xPos = 50.0f;
    yPos = 50.0f;
/*
    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH), 
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (xSize + 2*MENU_FRAME_WIDTH),
        gMenuHeightScale * (CREDIT_TEXT_HEIGHT*2 + 2*MENU_FRAME_HEIGHT),
        g_CreditVars.ColIndex,
        1);

    g_pFont->DrawText( (floorf(xPos), floorf(yPos), MENU_COLOR_OPAQUE|MENU_COLOR_BLUE, page->Title);
*/

    // Don't draw credits whilst initialising
    if (g_CreditVars.State == CREDIT_STATE_INIT) return;

    // Draw the Credits
    xPos = g_CreditVars.XPos;
    yPos = g_CreditVars.YPos;

    DrawSpruBox(
        gMenuWidthScale * (xPos - MENU_FRAME_WIDTH), 
        gMenuHeightScale * (yPos - MENU_FRAME_HEIGHT),
        gMenuWidthScale * (g_CreditVars.XSize + 2*MENU_FRAME_WIDTH),
        gMenuHeightScale * (g_CreditVars.YSize + 2*MENU_FRAME_HEIGHT),
        g_CreditVars.ColIndex,
        0);
    
    // Draw Title
    FTOL(g_CreditVars.Alpha * 255, alpha);
    if (alpha > 255) alpha = 255;
    col = alpha << 24 | MENU_COLOR_MED_BLUE;

    g_pFont->SetScaleFactors( 1.5f, 1.5f );
    g_pFont->DrawText( floorf(xPos+(page->fWidth)/2), floorf(yPos), col, page->strTitle, XBFONT_CENTER_X|XBFONT_CENTER_Y );
    g_pFont->SetScaleFactors( 1.0f, 1.0f );

    if ((page->dwFlags & CREDIT_SCROLL) != 0) 
    {
        //int yOff = Int(MulScalar(g_CreditVars.ScrollTimer, page->fScrollSpeed));
        yPos += page->fHeight - MulScalar(g_CreditVars.ScrollTimer, page->fScrollSpeed);//yOff;
    }   

//    yPos -= CREDIT_TEXT_HEIGHT;

    for( DWORD i=0; i<page->dwNumStrings; i++ )
    {
        BOOL bIsHeading = page->pStringTypes[i] ? TRUE : FALSE;

        if( bIsHeading )
        {
//            yPos += CREDIT_TEXT_HEIGHT;

            fade = (yPos - g_CreditVars.YPos + CREDIT_TEXT_HEIGHT);
            if (fade < CREDIT_TEXT_HEIGHT) 
                fade = 0.0f;
            else if (fade > g_CreditVars.YSize) 
                fade = 0.0f;
            else if (fade < CREDIT_TEXT_HEIGHT * 3) 
                fade = (fade - CREDIT_TEXT_HEIGHT) / (CREDIT_TEXT_HEIGHT * 2) ;
            else if (fade > g_CreditVars.YSize - CREDIT_TEXT_HEIGHT * 2) 
                fade = (g_CreditVars.YSize - fade) / (CREDIT_TEXT_HEIGHT * 2);
            else 
                fade = 1.0f;

            if (fade > 1.0f) fade = 1.0f;
            if (fade < 0.0f) fade = 0.0f;

            if (!(page->dwFlags & CREDIT_SCROLL)) 
                fade = 1.0f;

            yPos += CREDIT_TEXT_HEIGHT;

            if (fade > 0.0f) 
            {
                FTOL(g_CreditVars.Alpha * fade * 255, alpha);
                if (alpha > 255) alpha = 255;
        
                // Draw SubSection Header
                col = alpha << 24 | MENU_COLOR_YELLOW;
                if( (page->dwFlags & CREDIT_CENTER_TEXT) != 0 ) 
                {
                    xOff = ( page->fWidth - g_pFont->GetTextWidth( page->pStrings[i] ) ) / 2;
                } 
                else 
                {
                    xOff = 0.0f;
                }

                g_pFont->DrawText( floorf(xPos+xOff), floorf(yPos), col, page->pStrings[i] );
            }
        }
        else
        {
            fade = (yPos - g_CreditVars.YPos + CREDIT_TEXT_HEIGHT);
            if (fade < CREDIT_TEXT_HEIGHT) 
                fade = 0.0f;
            else if (fade > g_CreditVars.YSize) 
                fade = 0.0f;
            else if (fade < CREDIT_TEXT_HEIGHT * 3) 
                fade = (fade - CREDIT_TEXT_HEIGHT) / (CREDIT_TEXT_HEIGHT * 2) ;
            else if (fade > g_CreditVars.YSize - CREDIT_TEXT_HEIGHT * 2) 
                fade = (g_CreditVars.YSize - fade) / (CREDIT_TEXT_HEIGHT * 2);
            else 
                fade = 1.0f;

            if (fade > 1.0f) fade = 1.0f;
            if (fade < 0.0f) fade = 0.0f;

            if (!(page->dwFlags & CREDIT_SCROLL)) 
                fade = 1.0f;

            yPos += CREDIT_TEXT_HEIGHT;
            
            if( fade > 0.0f ) 
            {
                FTOL(g_CreditVars.Alpha * fade * 255, alpha);
                if (alpha > 255) alpha = 255;

                // Draw Names
                col = alpha << 24 | MENU_COLOR_WHITE;
                if( (page->dwFlags & CREDIT_CENTER_TEXT) != 0 ) 
                {
                    xOff = ( page->fWidth - g_pFont->GetTextWidth( page->pStrings[i] ) ) / 2;
                } 
                else 
                {
                    xOff = 0.0f;
                }

                g_pFont->DrawText( floorf(xPos+xOff), floorf(yPos), col, page->pStrings[i] );
            }
        }
    }
}




////////////////////////////////////////////////////////////////
//
// Draw Poly in background to darken image
//
////////////////////////////////////////////////////////////////
void DrawCreditFadePoly()
{
    VERTEX_TEX1 vert[4];
    long col;

    if (g_CreditVars.State != CREDIT_STATE_MOVING) return;

    vert[0].sx = 0.0f;
    vert[0].sy = 0.0f;
    vert[1].sx = (FLOAT)ScreenXsize;
    vert[1].sy = 0.0f;
    vert[2].sx = (FLOAT)ScreenXsize;
    vert[2].sy = (FLOAT)ScreenYsize;
    vert[3].sx = 0.0f;
    vert[3].sy = (FLOAT)ScreenYsize;

//$MODIFIED(cprince) - near/far clipping occurs even if ZENABLE is false, so set reasonable z value
//    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 300.0f;
    vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = 0.0f;
//$END_MODIFICATIONS
    vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = 1.0f;

    FTOL(g_CreditVars.Timer * 960, col);
    col = 255 - col;
    if (col < 0) return;
    col <<= 24;
    col |= 0xFFFFFF;

    vert[0].color = col; 
    vert[1].color = col;
    vert[2].color = col;
    vert[3].color = col;

    ZBUFFER_OFF();
    FOG_OFF();
    BLEND_ALPHA();
    BLEND_SRC(D3DBLEND_SRCALPHA);
    BLEND_DEST(D3DBLEND_INVSRCALPHA);
    SET_TPAGE(-1);

    DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
}

