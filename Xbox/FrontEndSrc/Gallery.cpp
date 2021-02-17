//-----------------------------------------------------------------------------
// File: Gallery.cpp
//
// Desc: Gallery.cpp
//
// Hist: 02.08.02 - ported/created
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "texture.h"    // LoadMipTexture
#include "panel.h"      // DrawPanelSprite
#include "menu2.h"
#include "MenuDraw.h"   // DrawSpruBox
#include "main.h"       // TimeStep
#include "Gallery.h"

// re-volt specific
#include "menutext.h"   // re-volt strings

// menus
#include "MainMenu.h"

// temporary includes?
#include "titlescreen.h"

void CreateGalleryMenu(MENU_HEADER *menuHeader, MENU *menu);
void MenuGoBackFromGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SelectPrevGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);
void SelectNextGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem);

void DrawGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex);

long GallerySlide, GallerySlideCurrent;
GALLERY_MODE GalleryMode;
REAL GalleryPos;

MENU Menu_Gallery = {
    TEXT_NONE,
    MENU_IMAGE_NONE,                        // Menu title bar image enum
    TITLESCREEN_CAMPOS_OVERVIEW,                     // Camera pos index
    MENU_DEFAULT | MENU_CENTRE_X | MENU_CENTRE_Y,   // Menu type
    SPRU_COL_RANDOM,                        // Spru colour
    CreateGalleryMenu,                      // Create menu function
    0,                                      // X coord
    0,                                      // Y Coord
    0,                                      // Current Item index
    NULL,                                   // Parent Menu (OverWritten in Create)
};


// Menu item
MENU_ITEM MenuItem_Gallery = {
    TEXT_NONE,                              // Text label index

    0,                                      // Space needed to draw item data
    &Menu_TopLevel,                         // Data
    DrawGallery,                            // Draw Function

	NULL,								    // Input handler
	0,                                      // Flags

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
    g_bMenuDrawMenuBox = FALSE;
    AddMenuItem(menuHeader, menu, &MenuItem_Gallery);

    GallerySlide = 0;
    GallerySlideCurrent = -1;
    GalleryPos = 0.0f;
    GalleryMode = GALLERY_OUT;
}

void FreeGallerySlides(void);

void MenuGoBackFromGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
//  FreeGallerySlides();
    MenuGoBack(menuHeader, menu, menuItem);
}

// utility
void SelectPrevGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (--GallerySlide < 0) GallerySlide = GALLERY_NUM - 1;
}

void SelectNextGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem)
{
    if (++GallerySlide == GALLERY_NUM) GallerySlide = 0;
}

#ifdef _PC
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
        sprintf(MenuBuffer, "D:\\gallery\\lib%d%c.bmp", num + 1, i + 'a');
        LoadMipTexture(MenuBuffer, TPAGE_MISC4 + i, 256, 256, 0, 1, FALSE);
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

void DrawGallery(MENU_HEADER *menuHeader, MENU *menu, MENU_ITEM *menuItem, int itemIndex)
{
    REAL xoff, yoff;

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

                PlaySfx(SFX_FIREWORK_BANG, SFX_MAX_VOL, SFX_CENTRE_PAN, SFX_SAMPLE_RATE, 0x7fffffff);
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
	sprintf(MenuBuffer, "%d / %d", GallerySlide + 1, GALLERY_NUM);
    DrawMenuText( 48, 35, 0xffffffff, MenuBuffer );
}

#endif