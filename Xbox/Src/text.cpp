//-----------------------------------------------------------------------------
// File: text.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "draw.h"
#include "main.h"
#include "text.h"
#include "input.h"
#include "network.h"
#include "model.h"
#include "world.h"
#include "texture.h"
#include "geom.h"
#include "camera.h"
#include "particle.h"
#include "NewColl.h"
#include "body.h"
#include "aerial.h"
#include "wheel.h"
#include "LevelLoad.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"
#include "gameloop.h"
#include "settings.h"
#include "ghost.h"
#include "ai.h"
#include "spark.h"
#include "timing.h"
#include "initplay.h"
#include "replay.h"

#include "ui_menu.h"
#include "ui_TitleScreen.h"
#include "XBFont.h"

// globals
static VERTEX_TEX1 TextVert[4];
short MenuCount;
extern unsigned long ReplayDataBufSize;

// ascii conv shit
static unsigned char AsciiOld[] = "àÀèÈìÌòÒùÙáÁéÉíÍóÓúÚâÂêÊîÎôÔûÛäÄëËïÏöÖüÜçÇñÑº¡¿ß°";
static unsigned char AsciiNew[] = "aAeEiIoOuUaAeEiIoOuUaAeEiIoOuUaAeEiIoOuU\x87\x86nN\x88\x89\x8a\x8d\x88";




CXBFont*    g_pFont;

//-----------------------------------------------------------------------------
// Name: LoadFonts()
// Desc: 
//-----------------------------------------------------------------------------
HRESULT LoadFonts()
{
    // Construct the font classes
    g_pFont          = new CXBFont();

    // Create the fonts from their resource files
    if( FAILED( g_pFont->Create( "d:\\gfx\\MenuFont.xpr" ) ) )
        return E_FAIL;

    return S_OK;
}




//////////////////////
// begin text state //
//////////////////////

void BeginTextState(void)
{
    ZBUFFER_OFF();
    BLEND_OFF();
    FOG_OFF();
    SET_TPAGE(TPAGE_FONT);
}




//-----------------------------------------------------------------------------
// Name: DrawMenuText()
// Desc: Same as DrawMenuText(), but optionally can bracket the text with
//       left/right arrows.
//-----------------------------------------------------------------------------
void DrawMenuTextWithArrows( BOOL bDrawArrows, FLOAT x, FLOAT y, DWORD color, 
                             WCHAR* strText, FLOAT fMaxWidth )
{
    g_pFont->DrawText( x, y, color, strText, XBFONT_TRUNCATED, fMaxWidth  );

    if( bDrawArrows )
    {
        static WCHAR strLeftArrowString[3]  = { 0x2190, 0x0020, 0x0000 };
        static WCHAR strRightArrowString[3] = { 0x0020, 0x2192, 0x0000 };

        g_pFont->DrawText( color, strRightArrowString );
        g_pFont->DrawText( x, y, color, strLeftArrowString, XBFONT_RIGHT );
    }
}





//-----------------------------------------------------------------------------
// Name: DrawMenuText()
// Desc: Draw menu text, but add three dots to end of string if it will be
//       bigger than the passed "maxLen".
//-----------------------------------------------------------------------------
void DrawMenuText( FLOAT x, FLOAT y, DWORD color, WCHAR* strText, FLOAT fMaxWidth )
{
#ifdef _DEBUG
    static BOOL bCheckForNTSCSafeColors = FALSE;
    if( bCheckForNTSCSafeColors )
    {
        DWORD r = (color&0x00ff0000)>>16L;
        DWORD g = (color&0x0000ff00)>>8L;
        DWORD b = (color&0x000000ff)>>0L;
        if( r > 0xeb || g > 0xeb || b > 0xeb )
        {
            OutputDebugString( "WARNING: Non-NTSC safe colors are used for drawing text!" );
        }
    }
#endif

    g_pFont->DrawText( x, y, color, strText, XBFONT_TRUNCATED, fMaxWidth );
}




//////////////////////////////
// draw text to back buffer //
//////////////////////////////

void DumpText(int x, int y, int xs, int ys, long color, WCHAR *text)
{
    REAL xR = (REAL)x;
    REAL yR = (REAL)y;
    REAL xsR = (REAL)xs;
    REAL ysR = (REAL)ys;
    DumpTextReal(xR, yR, xsR, ysR, color, text);
}

//$NOTE: don't change args to float until we change function name to match (and there's no need for that right now).
void DumpTextReal(REAL x, REAL y, REAL xs, REAL ys, long color, WCHAR *text)
{
#ifdef _DEBUG
    static BOOL bCheckForNTSCSafeColors = FALSE;
    if( bCheckForNTSCSafeColors )
    {
        DWORD r = (color&0x00ff0000)>>16L;
        DWORD g = (color&0x0000ff00)>>8L;
        DWORD b = (color&0x000000ff)>>0L;
        if( r > 0xeb || g > 0xeb || b > 0xeb )
        {
            OutputDebugString( "WARNING: Non-NTSC safe colors are used for drawing text!" );
        }
    }
#endif

    char i;
    float tu, tv;
    float xstart, ystart, xsize, ysize, ystart2, ysize2;
    long lu, lv, ch, ch2;

    // calc size / pos
    xstart = x * RenderSettings.GeomScaleX + ScreenLeftClip;
    ystart = y * RenderSettings.GeomScaleY + ScreenTopClip;

    xsize = xs * RenderSettings.GeomScaleX;
    ysize = ys * RenderSettings.GeomScaleY;

    ystart2 = ystart;
    ysize2 = ysize;
    
    // init vert misc
    for (i = 0 ; i < 4 ; i++)
    {
        TextVert[i].color = color;
        TextVert[i].sz = 0; //$ADDITION: explicitly set z (even though z-buffer off) to avoid near/far clipping
        TextVert[i].rhw = 1;
    }

    // draw chars
    while (*text)
    {
        // get char
        ch2 = 0;
        ch = *(WCHAR*)text;
        if (ch > 127)
        {
            for (i = 0 ; i < sizeof(AsciiOld) ; i++)
            {
                if (AsciiOld[i] == ch)
                {
                    ch = AsciiNew[i];
                    if (i < 40)
                    {
                        ch2 = 129 + (i / 10);
                    }
                    else if (i >= 42 && i <= 43)
                    {
                        ch2 = 133;
                    }
                    break;
                }
            }
        }

        // draw
        if (ch2)
        {
            ystart += ysize * 0.2f;
            ysize *= 0.8f; 
        }

        if (ch >= '!')
        {
            ch -= 33;

            TextVert[0].sx = xstart - 0.5f;
            TextVert[0].sy = ystart - 0.5f;

            TextVert[1].sx = xstart - 0.5f + xsize;
            TextVert[1].sy = ystart - 0.5f;

            TextVert[2].sx = xstart - 0.5f + xsize;
            TextVert[2].sy = ystart - 0.5f + ysize;

            TextVert[3].sx = xstart - 0.5f;
            TextVert[3].sy = ystart - 0.5f + ysize;

            lu = ch % FONT_PER_ROW;
            lv = ch / FONT_PER_ROW;

            tu = (float)lu * FONT_WIDTH + 0.5f;
            tv = (float)lv * FONT_HEIGHT + 1.0f;

            TextVert[0].tu = tu / 256.0f;
            TextVert[0].tv = tv / 256.0f;

            TextVert[1].tu = (tu + FONT_UWIDTH - 0.5f) / 256.0f;
            TextVert[1].tv = tv / 256.0f;

            TextVert[2].tu = (tu + FONT_UWIDTH - 0.5f) / 256.0f;
            TextVert[2].tv = (tv + FONT_VHEIGHT) / 256.0f;

            TextVert[3].tu = tu / 256.0f;
            TextVert[3].tv = (tv + FONT_VHEIGHT) / 256.0f;

            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, TextVert, 4, D3DDP_DONOTUPDATEEXTENTS);

            ystart = ystart2;
            ysize = ysize2;

            // draw extra bit?
            if (ch2)
            {
                ch2 -= 33;

                TextVert[0].sy = ystart;
                TextVert[1].sy = ystart;

                lu = ch2 % FONT_PER_ROW;
                lv = ch2 / FONT_PER_ROW;

                tu = (float)lu * FONT_WIDTH + 1.0f;
                tv = (float)lv * FONT_HEIGHT + 1.0f;

                TextVert[0].tu = tu / 256.0f;
                TextVert[0].tv = tv / 256.0f;

                TextVert[1].tu = (tu + FONT_UWIDTH - 0.5f) / 256.0f;
                TextVert[1].tv = tv / 256.0f;

                TextVert[2].tu = (tu + FONT_UWIDTH - 0.5f) / 256.0f;
                TextVert[2].tv = (tv + FONT_VHEIGHT) / 256.0f;

                TextVert[3].tu = tu / 256.0f;
                TextVert[3].tv = (tv + FONT_VHEIGHT) / 256.0f;

                DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, TextVert, 4, D3DDP_DONOTUPDATEEXTENTS);
            }
        }

        // next
        xstart += xsize;
        text++;
    }
}




//////////////////////////////
// draw text to back buffer //
//////////////////////////////
void DumpText3D(VEC *pos, float xs, float ys, long color, WCHAR *text)
{
    char i;
    float tu, tv, sz, rhw;
    float x, y;
    long lu, lv, ch;

    // set tpage
    SET_TPAGE(TPAGE_FONT);

    // calc size / pos
    xs = xs * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleX;
    ys = ys * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleY;
    x = pos->v[X] * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleX + RenderSettings.GeomCentreX;
    y = pos->v[Y] * RenderSettings.GeomPers / pos->v[Z] * RenderSettings.GeomScaleY + RenderSettings.GeomCentreY;

    // init vert misc
    sz = GET_ZBUFFER(pos->v[Z]);
    rhw = 1 / pos->v[Z];

    for (i = 0 ; i < 4 ; i++)
    {
        TextVert[i].color = color;
        TextVert[i].sz = sz;
        TextVert[i].rhw = rhw;
        
    }

    // draw chars
    while (*text)
    {
        // get char
        ch = *text - 33;
        if (ch < -1) ch += 256;
        if (ch != -1)
        {
            // set screen coors
            TextVert[0].sx = x;
            TextVert[0].sy = y;

            TextVert[1].sx = (x + xs);
            TextVert[1].sy = y;

            TextVert[2].sx = (x + xs);
            TextVert[2].sy = (y + ys);

            TextVert[3].sx = x;
            TextVert[3].sy = (y + ys);

            // set uv's
            lu = ch % FONT_PER_ROW;
            lv = ch / FONT_PER_ROW;

            tu = (float)lu * FONT_WIDTH + 1.0f;
            tv = (float)lv * FONT_HEIGHT + 1.0f;

            TextVert[0].tu = tu / 256;
            TextVert[0].tv = tv / 256;

            TextVert[1].tu = (tu + FONT_UWIDTH - 1.0f) / 256;
            TextVert[1].tv = tv / 256;

            TextVert[2].tu = (tu + FONT_UWIDTH - 1.0f) / 256;
            TextVert[2].tv = (tv + FONT_VHEIGHT) / 256;

            TextVert[3].tu = tu / 256;
            TextVert[3].tv = (tv + FONT_VHEIGHT) / 256;

            // draw
            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, TextVert, 4, D3DDP_DONOTUPDATEEXTENTS);
        }

        // next
        x += xs;
        text++;
    }
}




/////////////////////////////////////////////////////////////////////
//
// ShowPhysicsInfo: Display info about the passed car on the screen
//
/////////////////////////////////////////////////////////////////////

#if SHOW_PHYSICS_INFO

#if USE_DEBUG_ROUTINES
extern FLOAT DEBUG_MaxImpulseMag;
extern FLOAT DEBUG_MaxAngImpulseMag;
#endif

extern GHOST_INFO *GhostInfo;
extern GHOST_INFO *GHO_BestGhostInfo;
extern int COL_NCollsTested;
extern int DEBUG_NIts;
extern bool DEBUG_Converged;
extern FLOAT DEBUG_Res;

void ShowPhysicsInfo() 
{
    char buf[256];

    if (PLR_LocalPlayer == NULL) return;
        
    // Defines
#if REMOVE_JITTER
    sprintf(buf, "Jitter: On   %1d", PLR_LocalPlayer->car.Body->IsJittering);
#else 
    sprintf(buf, "Jitter: Off");
#endif
    DumpText(0, 40, 6, 12, 0xffffff, buf);

    // Engine voltage and steering angle + autobraking
    sprintf(buf, "Engine: %3d   Steer: %3d   Revs: %5d   AutoBrake: %s", 
        (int) (100.0f * PLR_LocalPlayer->car.EngineVolt),
        (int) (100.0f * PLR_LocalPlayer->car.SteerAngle),
        (int) (100.0f * PLR_LocalPlayer->car.Revs),
        (GameSettings.AutoBrake)? "On": "Off");
    DumpText(100, 40, 8, 16, 0xffffff, buf);

    // TimeStep
    sprintf(buf, "TimeStep: %4d (%2d)", (int)(10000.0f * TimeStep), NPhysicsLoops);
    DumpText(0, 60, 8, 16, 0xffffff, buf);

    // Ghost Info
    sprintf(buf, "Ghost frame: %8d / %8d", GHO_BestFrame, GHO_BestGhostInfo->NFrames);
    DumpText(200, 60, 8, 16, 0xffffff, buf);

    // Number of collisions flagged
    sprintf(buf, "NBodyCols: %3d (%3d)  NWheelColls: %3d (%3d)", COL_NBodyColls - COL_NBodyDone, COL_NBodyColls, COL_NWheelColls - COL_NWheelDone, COL_NWheelColls);
    DumpText(0, 80, 8, 16, 0xffffff, buf);

    // Pos and orientation of car
    sprintf(buf, "Pos: %5d %5d %5d  Vel: %5d %5d %5d  Ang: %5d %5d %5d", 
        (int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Pos.v[X]),
        (int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Pos.v[Y]),
        (int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Pos.v[Z]),
        (int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Vel.v[X]),
        (int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Vel.v[Y]),
        (int) (1.0f * PLR_LocalPlayer->car.Body->Centre.Vel.v[Z]),
        (int) (100.0f * PLR_LocalPlayer->car.Body->AngVel.v[X]),
        (int) (100.0f * PLR_LocalPlayer->car.Body->AngVel.v[Y]),
        (int) (100.0f * PLR_LocalPlayer->car.Body->AngVel.v[Z]));
    DumpText(0, 100, 8, 16, 0xffffff, buf);

    // Grid and # polys
    COLLGRID *grid = PosToCollGrid(&PLR_LocalPlayer->car.Body->Centre.Pos);
    if (grid != NULL) {
        sprintf(buf, "Grid: %5ld (%5ld Polys)", grid - COL_CollGrid, grid->NCollPolys);
        DumpText(320, 120, 8, 16, 0xffffff, buf);
    }
#if USE_DEBUG_ROUTINES
    sprintf(buf, "Its %d", DEBUG_NIts);
    DumpText(320, 140, 8, 16, 0xffffff, buf);
#endif

    sprintf(buf, "%6d %6d %6d %6d", 
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[XX]),
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[XY]),
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[XZ]),
        (int) (1000.0f * Length(&PLR_LocalPlayer->car.Body->Centre.WMatrix.mv[R])));
    DumpText(0, 120, 8, 16, 0xffffff, buf);
    sprintf(buf, "%6d %6d %6d %6d", 
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[YX]),
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[YY]),
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[YZ]),
        (int) (1000.0f * Length(&PLR_LocalPlayer->car.Body->Centre.WMatrix.mv[U])));
    DumpText(0, 140, 8, 16, 0xffffff, buf);
    sprintf(buf, "%6d %6d %6d %6d", 
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[ZX]),
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[ZY]),
        (int) (1000.0f * PLR_LocalPlayer->car.Body->Centre.WMatrix.m[ZZ]),
        (int) (1000.0f * Length(&PLR_LocalPlayer->car.Body->Centre.WMatrix.mv[L])));
    DumpText(0, 160, 8, 16, 0xffffff, buf);

    // Show impulses
#if USE_DEBUG_ROUTINES
    sprintf(buf, "imp: %6d %6d %6d  ang: %6d %6d %6d",
        (int) (100.0f * DEBUG_Impulse.v[X]),
        (int) (100.0f * DEBUG_Impulse.v[Y]),
        (int) (100.0f * DEBUG_Impulse.v[Z]),
        (int) (100.0f * DEBUG_AngImpulse.v[X]),
        (int) (100.0f * DEBUG_AngImpulse.v[Y]),
        (int) (100.0f * DEBUG_AngImpulse.v[Z]));
    DumpText(0, 180, 8, 16, 0xffffff, buf);
#endif

    // Show friction mode and coefficients
    sprintf(buf, "SFric: %3d   KFric: %3d",
        (int) (100.0f * PLR_LocalPlayer->car.Body->Centre.StaticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Body->Centre.KineticFriction));
    DumpText(0, 200, 8, 16, 0xffffff, buf);

    // Wheel Info
    sprintf(buf, "Wheels: Pos %4d %4d %4d %4d", 
        (int) (1000.0f * PLR_LocalPlayer->car.Wheel[0].Pos),
        (int) (1000.0f * PLR_LocalPlayer->car.Wheel[1].Pos),
        (int) (1000.0f * PLR_LocalPlayer->car.Wheel[2].Pos),
        (int) (1000.0f * PLR_LocalPlayer->car.Wheel[3].Pos));
    DumpText(0, 220, 8, 16, 0xffffff, buf);
    sprintf(buf, "        Vel %4d %4d %4d %4d, %4d %4d %4d %4d", 
        (int) (PLR_LocalPlayer->car.Wheel[0].Vel),
        (int) (PLR_LocalPlayer->car.Wheel[1].Vel),
        (int) (PLR_LocalPlayer->car.Wheel[2].Vel),
        (int) (PLR_LocalPlayer->car.Wheel[3].Vel),
        (int) (PLR_LocalPlayer->car.Wheel[0].AngVel),
        (int) (PLR_LocalPlayer->car.Wheel[1].AngVel),
        (int) (PLR_LocalPlayer->car.Wheel[2].AngVel),
        (int) (PLR_LocalPlayer->car.Wheel[3].AngVel));
    DumpText(0, 240, 8, 16, 0xffffff, buf);
    sprintf(buf, "        Frc %3d/%3d(%3d)  %3d/%3d(%3d)  %3d/%3d(%3d)  %3d/%3d(%3d)", 
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[0].StaticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[0].KineticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Spring[0].Restitution),
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[1].StaticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[1].KineticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Spring[1].Restitution),
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[2].StaticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[2].KineticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Spring[2].Restitution),
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[3].StaticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Wheel[3].KineticFriction),
        (int) (100.0f * PLR_LocalPlayer->car.Spring[3].Restitution));
    DumpText(0, 260, 8, 16, 0xffffff, buf);
    sprintf(buf, "        C/S %1d/%1d  %1d/%1d  %1d/%1d  %1d/%1d",
        (IsWheelInContact(&PLR_LocalPlayer->car.Wheel[0]))? 1: 0,
        (IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[0]))? 1: 0,
        (IsWheelInContact(&PLR_LocalPlayer->car.Wheel[1]))? 1: 0,
        (IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[1]))? 1: 0,
        (IsWheelInContact(&PLR_LocalPlayer->car.Wheel[2]))? 1: 0,
        (IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[2]))? 1: 0,
        (IsWheelInContact(&PLR_LocalPlayer->car.Wheel[3]))? 1: 0,
        (IsWheelSkidding(&PLR_LocalPlayer->car.Wheel[3]))? 1: 0);
    DumpText(0, 280, 8, 16, 0xffffff, buf);

    // Number of sparks active
    sprintf(buf, "Sparks: %5d  Trails: %5d", NActiveSparks, NActiveTrails);
    DumpText(0, 300, 8, 16, 0xffffff, buf);

    // Size of ghost currently stored and being recorded
    sprintf(buf, "Ghost: %8d (%8d)", GhostInfo->NFrames * sizeof(GHOST_INFO), GHO_BestGhostInfo->NFrames * sizeof(GHOST_INFO));
    DumpText(320, 300, 8, 16, 0xffffff, buf);

    sprintf(buf, "RPL Buffer %8d (%8d)", ReplayBufferBytesStored, ReplayDataBufSize);
    DumpText(320, 320, 8, 12, 0xffff0000, buf);

    sprintf(buf, "ToeIn: (f)%8d   (r)%8d", (int)(1000.0f * PLR_LocalPlayer->car.Wheel[0].ToeIn), (int)(1000.0f * PLR_LocalPlayer->car.Wheel[2].ToeIn));
    DumpText(320, 340, 8, 12, 0xffff0000, buf);

    // Equation solver tests
#if USE_DEBUG_ROUTINES
    sprintf(buf, "Converged: %s   Res = %d", (DEBUG_Converged)? "Yes": "No ", (int)(1000000 * DEBUG_Res));
    DumpText(0, 320, 8, 16, 0xffffff, buf);
#endif

#if USE_DEBUG_ROUTINES
    sprintf(buf, "Max Imp: %9d    Max Ang Imp: %9d", (int)DEBUG_MaxImpulseMag, (int)DEBUG_MaxAngImpulseMag);
    DumpText(0,340, 8, 16, 0xffffff, buf);
#endif

}
#endif
