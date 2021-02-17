//-----------------------------------------------------------------------------
// File: ui_Gallery.cpp
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#include "ctrlread.h"
#include "object.h"
#include "light.h"
#include "obj_init.h"
#include "player.h"
#include "ai.h"
#include "ai_init.h"
#include "EditObject.h"
#include "drawobj.h"
#include "move.h"
#include "timing.h"
#include "visibox.h"
#include "spark.h"
#include "field.h"
#include "weapon.h"
#include "input.h"
#include "initplay.h"
#include "pickup.h"
#include "SoundEffectEngine.h"
#include "Panel.h"
#include "Text.h"

#include "ui_TitleScreen.h"
#include "ui_Menu.h"
#include "ui_menudraw.h"
#include "ui_MenuText.h"
#include "ui_TopLevelMenu.h"


// PC gallery mode shit

#define GALLERY_NUM 12

enum GALLERY_MODE
{
    GALLERY_OUT,
    GALLERY_MOVING_OUT,
    GALLERY_IN,
    GALLERY_MOVING_IN,
};



void CreateGalleryMenu(MENU_HEADER *menuHeader, MENU *menu);
BOOL MenuGoBackFromGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
BOOL SelectPrevGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
BOOL SelectNextGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void DrawGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

long GallerySlide, GallerySlideCurrent;
GALLERY_MODE GalleryMode;
FLOAT GalleryPos;




////////////////////////////////////////////////////////////////
//
// View concept gallery
//
////////////////////////////////////////////////////////////////
extern MENU Menu_Gallery = 
{
    TEXT_NONE,
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    CreateGalleryMenu,                      // Create menu function
    NULL,                                   // Input handler function
    NULL,                                   // Menu draw function
    0,                                      // X coord
    0,                                      // Y Coord
};


// Menu item
MENU_ITEM MenuItem_Gallery = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    &Menu_TopLevel,                         // Data
    DrawGallery,                            // Draw Function

    SelectPrevGallery,                      // Up Action
    SelectNextGallery,                      // Down Action
    SelectPrevGallery,                      // Left Action
    SelectNextGallery,                      // Right Action
    MenuGoBackFromGallery,                  // Back Action
    NULL,                                   // Forward Action
};

// Create
void CreateGalleryMenu(MENU_HEADER *menuHeader, MENU *menu)
{
    menuHeader->m_pMenu->dwFlags |= MENU_NOBOX;
    menuHeader->AddMenuItem( &MenuItem_Gallery);

    GallerySlide = 0;
    GallerySlideCurrent = -1;
    GalleryPos = 0.0f;
    GalleryMode = GALLERY_OUT;
}

void FreeGallerySlides(void);

BOOL MenuGoBackFromGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
//  FreeGallerySlides();
    MenuGoBack(menuHeader, menu, menuItem);
    return TRUE;
}

// utility
BOOL SelectPrevGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if( --GallerySlide < 0 )
    {
        GallerySlide = GALLERY_NUM - 1;
        return TRUE;
    }

    return FALSE;
}

BOOL SelectNextGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if( ++GallerySlide == GALLERY_NUM ) 
    {
        GallerySlide = 0;
        return TRUE;
    }

    return FALSE;
}




////////////////////////////////////////////////////////////////
//
// Load Gallery slides
//
////////////////////////////////////////////////////////////////

static void LoadGallerySlides(long num)
{
    char i;

    for (i = 0 ; i < 4 ; i++)
    {
        CHAR strFilename[80];
//$MODIFIED
//        sprintf(strFilename, "gallery\\lib%d%c.bmp", num + 1, i + 'a');
        sprintf(strFilename, "D:\\gallery\\lib%d%c.bmp", num + 1, i + 'a');
//$END_MODIFICATIONS
        LoadMipTexture(strFilename, TPAGE_MISC4 + i, 256, 256, 0, 1, FALSE);
    }
}

////////////////////////////////////////////////////////////////
//
// Free Gallery slides
//
////////////////////////////////////////////////////////////////

void FreeGallerySlides(void)
{
    char i;

    for (i = 0 ; i < 4 ; i++)
    {
        FreeOneTexture(TPAGE_MISC4 + i);
    }
}

////////////////////////////////////////////////////////////////
//
// Draw Gallery slides
//
////////////////////////////////////////////////////////////////

extern long GallerySlide, GallerySlideCurrent;
extern GALLERY_MODE GalleryMode;
extern FLOAT GalleryPos;

void DrawGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    FLOAT xoff, yoff;

// act on mode

    switch (GalleryMode)
    {

// out

        case GALLERY_OUT:
            GallerySlideCurrent = GallerySlide;
            LoadGallerySlides(GallerySlide);
            GalleryMode = GALLERY_MOVING_IN;

            break;

// moving out

        case GALLERY_MOVING_OUT:
            GalleryPos -= TimeStep * 4.0f;
            if (GalleryPos <= 0.0f)
            {
                GalleryPos = 0.0f;
                GalleryMode = GALLERY_OUT;
            }
            break;

// in

        case GALLERY_IN:
            if (GallerySlideCurrent != GallerySlide)
            {
                GalleryMode = GALLERY_MOVING_OUT;
            }
            break;

// moving in

        case GALLERY_MOVING_IN:
            GalleryPos += TimeStep * 4.0f;
            if (GalleryPos >= 1.0f)
            {
                GalleryPos = 1.0f;
                GalleryMode = GALLERY_IN;

#ifdef OLD_AUDIO
                PlaySfx(SFX_FIREWORK_BANG, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
#else
                g_SoundEngine.Play2DSound( EFFECT_FireworkBang, FALSE );
#endif // OLD_AUDIO
            }
            break;
    }

// draw slides

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, 0);

    xoff = (1.0f - GalleryPos) * 320.0f;
    yoff = (1.0f - GalleryPos) * 240.0f;

    SET_TPAGE(TPAGE_MISC4);
    DrawPanelSprite(64.0f - xoff, 40.0f - yoff, 256.0f, 200.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff);
    SET_TPAGE(TPAGE_MISC5);
    DrawPanelSprite(320.0f + xoff, 40.0f - yoff, 256.0f, 200.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff);
    SET_TPAGE(TPAGE_MISC6);
    DrawPanelSprite(64.0f - xoff, 240.0f + yoff, 256.0f, 200.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff);
    SET_TPAGE(TPAGE_MISC7);
    DrawPanelSprite(320.0f + xoff, 240.0f + yoff, 256.0f, 200.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0xffffffff);

    SET_RENDER_STATE(D3DRENDERSTATE_ALPHAREF, AlphaRef);

// show counts

    SET_TPAGE(TPAGE_FONT);
    swprintf( MenuBuffer, L"%d / %d", GallerySlide + 1, GALLERY_NUM );
    g_pFont->DrawText( 48, 36, MENU_TEXT_RGB_NORMAL, MenuBuffer );
}
