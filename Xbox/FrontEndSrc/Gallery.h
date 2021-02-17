////////////////////////////////////////////////////////////////
//
// View concept gallery
//
////////////////////////////////////////////////////////////////

#pragma once


extern MENU Menu_Gallery;


#define GALLERY_NUM 12

enum GALLERY_MODE
{
    GALLERY_OUT,
    GALLERY_MOVING_OUT,
    GALLERY_IN,
    GALLERY_MOVING_IN,
};

extern long GallerySlide, GallerySlideCurrent;
extern GALLERY_MODE GalleryMode;
extern REAL GalleryPos;

