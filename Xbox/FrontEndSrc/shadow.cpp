//-----------------------------------------------------------------------------
// File: shadow.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "geom.h"
#include "newcoll.h"
#include "shadow.h"
#include "text.h"
#include "dx.h"
#include "geom.h"
#include "texture.h"
#include "camera.h"
#include "draw.h"
#include "main.h"

static unsigned short ShadowVertNum, ShadowVertFree;
static unsigned short ShadowVertList[2][8];
static SHADOW_VERT ShadowVert[32];
static VERTEX_TEX1 ShadowDrawVert[8];

///////////////////
// draw a shadow //
///////////////////

void DrawShadow(VEC *p0, VEC *p1, VEC *p2, VEC *p3, REAL tu, REAL tv, REAL twidth, REAL theight, long rgb, REAL yoff, REAL maxy, long semi, long tpage, BOUNDING_BOX *box)
{
    char los[4];
    float qdist, lqdist;
    NEWCOLLPOLY *q;
    short i, j, k, newcount, vcount, togg;
    float yfade;
    float dist0, dist1, dist2, dist3;
    float ldist0, ldist1, ldist2, ldist3;
    float xmin, xmax, ymin, ymax, zmin, zmax;
    NEWCOLLPOLY *p;
    VEC lp0, lp1, lp2, lp3, pos;
    SHADOW_VERT *vert0, *vert1;
    PLANE *plane;
    COLLGRID *header;
    VERTEX_TEX1 *vert;
    SHADOW_VERT *svert;
    
// set fade y pos

    yfade = (p0->v[Y] + p1->v[Y] + p2->v[Y] + p3->v[Y]) * 0.25f;

// create bounding box

    xmin = xmax = p0->v[X];
    ymin = p0->v[Y];
    zmin = zmax = p0->v[Z];

    if (p1->v[X] < xmin) xmin = p1->v[X];
    if (p1->v[X] > xmax) xmax = p1->v[X];
    if (p1->v[Y] < ymin) ymin = p1->v[Y];
    if (p1->v[Z] < zmin) zmin = p1->v[Z];
    if (p1->v[Z] > zmax) zmax = p1->v[Z];

    if (p2->v[X] < xmin) xmin = p2->v[X];
    if (p2->v[X] > xmax) xmax = p2->v[X];
    if (p2->v[Y] < ymin) ymin = p2->v[Y];
    if (p2->v[Z] < zmin) zmin = p2->v[Z];
    if (p2->v[Z] > zmax) zmax = p2->v[Z];

    if (p3->v[X] < xmin) xmin = p3->v[X];
    if (p3->v[X] > xmax) xmax = p3->v[X];
    if (p3->v[Y] < ymin) ymin = p3->v[Y];
    if (p3->v[Z] < zmin) zmin = p3->v[Z];
    if (p3->v[Z] > zmax) zmax = p3->v[Z];

    if (maxy)
        ymax = ymin + maxy;
    else
        ymax = 1000000.0f;

// set 'above' vectors

    SetVector(&lp0, p0->v[X], p0->v[Y] - 256, p0->v[Z]);
    SetVector(&lp1, p1->v[X], p1->v[Y] - 256, p1->v[Z]);
    SetVector(&lp2, p2->v[X], p2->v[Y] - 256, p2->v[Z]);
    SetVector(&lp3, p3->v[X], p3->v[Y] - 256, p3->v[Z]);

// set default shadow bounding box

    if (box)
    {
        box->Xmin = box->Ymin = box->Zmin = 1000000.0f;
        box->Xmax = box->Ymax = box->Zmax = -1000000.0f;
    }

// loop thru coll polys

    SetVector(&pos, (p0->v[X] + p2->v[X]) / 2, (p0->v[Y] + p2->v[Y]) / 2, (p0->v[Z] + p2->v[Z]) / 2);
    header = PosToCollGrid(&pos);
    if (header == NULL) return;

    for (i = 0 ; i < header->NCollPolys ; i++)
    {

// get this poly

        p = GetCollPoly(header->CollPolyIndices[i]);

// skip?

        if (p->Plane.v[B] > -0.1f) continue;
        if (xmin > p->BBox.XMax || xmax < p->BBox.XMin || zmin > p->BBox.ZMax || zmax < p->BBox.ZMin || ymin > p->BBox.YMax || ymax < p->BBox.YMin) continue;

// get plane distances for each corner

        dist0 = -PlaneDist(&p->Plane, p0);
        dist1 = -PlaneDist(&p->Plane, p1);
        dist2 = -PlaneDist(&p->Plane, p2);
        dist3 = -PlaneDist(&p->Plane, p3);

// skip if all points below plane

        if (dist0 >= 0 && dist1 >= 0 && dist2 >= 0 && dist3 >= 0) continue;

// get intersection points

        ldist0 = -PlaneDist(&p->Plane, &lp0);
        ldist1 = -PlaneDist(&p->Plane, &lp1);
        ldist2 = -PlaneDist(&p->Plane, &lp2);
        ldist3 = -PlaneDist(&p->Plane, &lp3);

        FindIntersection(&lp0, ldist0, p0, dist0, &ShadowVert[0].Pos);
        FindIntersection(&lp1, ldist1, p1, dist1, &ShadowVert[1].Pos);
        FindIntersection(&lp2, ldist2, p2, dist2, &ShadowVert[2].Pos);
        FindIntersection(&lp3, ldist3, p3, dist3, &ShadowVert[3].Pos);

// skip if line of sight fails to intersection points

        if (dist0 < -64.0f && dist1 < -64.0f && dist2 < -64.0f && dist3 < -64.0f)
        {
            *(long*)los = FALSE;

            for (j = 0 ; j < header->NCollPolys ; j++)
            {
                q = GetCollPoly(header->CollPolyIndices[j]);

                if (q == p) continue;
                if (q->Plane.v[B] > -0.1f) continue;
                if (xmin > q->BBox.XMax || xmax < q->BBox.XMin || zmin > q->BBox.ZMax || zmax < q->BBox.ZMin) continue;
                if (ymin > q->BBox.YMax || p->BBox.YMax < q->BBox.YMin) continue;

                qdist = -PlaneDist(&q->Plane, p0);
                if (qdist > dist0)
                {
                    lqdist = -PlaneDist(&q->Plane, &lp0);
                    FindIntersection(&lp0, lqdist, p0, qdist, &pos);
                    if (PointInCollPolyBounds(&pos, q))
                        los[0] = TRUE;
                }

                qdist = -PlaneDist(&q->Plane, p1);
                if (qdist > dist1)
                {
                    lqdist = -PlaneDist(&q->Plane, &lp1);
                    FindIntersection(&lp1, lqdist, p1, qdist, &pos);
                    if (PointInCollPolyBounds(&pos, q))
                        los[1] = TRUE;
                }

                qdist = -PlaneDist(&q->Plane, p2);
                if (qdist > dist2)
                {
                    lqdist = -PlaneDist(&q->Plane, &lp2);
                    FindIntersection(&lp2, lqdist, p2, qdist, &pos);
                    if (PointInCollPolyBounds(&pos, q))
                        los[2] = TRUE;
                }

                qdist = -PlaneDist(&q->Plane, p3);
                if (qdist > dist3)
                {
                    lqdist = -PlaneDist(&q->Plane, &lp3);
                    FindIntersection(&lp3, lqdist, p3, qdist, &pos);
                    if (PointInCollPolyBounds(&pos, q))
                        los[3] = TRUE;
                }

                if (*(long*)los == 0x01010101) break;
            }

            if (j < header->NCollPolys) continue;
        }

// set uv's

        ShadowVert[0].tu = tu;
        ShadowVert[0].tv = tv;
        ShadowVert[1].tu = tu + twidth;
        ShadowVert[1].tv = tv;
        ShadowVert[2].tu = tu + twidth;
        ShadowVert[2].tv = tv + theight;
        ShadowVert[3].tu = tu;
        ShadowVert[3].tv = tv + theight;

// init clip vars

        ShadowVertNum = 4;
        ShadowVertFree = 4;

        ShadowVertList[0][0] = 0;
        ShadowVertList[0][1] = 1;
        ShadowVertList[0][2] = 2;
        ShadowVertList[0][3] = 3;

// loop thru all edges of coll poly

        vcount = (short)(3 + (p->Type & QUAD));  //$MODIFIED(JHarding): added (short) typecast
        togg = TRUE;

        for (j = 0 ; j < vcount ; j++)
        {
            plane = &p->EdgePlane[j];

// clip shadow poly against one edge

            for (k = newcount = 0 ; k < ShadowVertNum ; k++)
            {
                vert0 = ShadowVert + ShadowVertList[!togg][k];
                vert1 = ShadowVert + ShadowVertList[!togg][(k + 1) % ShadowVertNum];

                dist0 = PlaneDist(plane, &vert0->Pos);
                if (dist0 <= 0)
                {
                    ShadowVertList[togg][newcount++] = ShadowVertList[!togg][k];

                    dist1 = PlaneDist(plane, &vert1->Pos);
                    if (dist1 > 0)
                    {
                        ClipShadowEdge(vert1, vert0, dist1 / (dist1 - dist0), ShadowVert + ShadowVertFree);
                        ShadowVertList[togg][newcount++] = ShadowVertFree++;
                    }
                }
                else
                {
                    dist1 = PlaneDist(plane, &vert1->Pos);
                    if (dist1 <= 0)
                    {
                        ClipShadowEdge(vert0, vert1, dist0 / (dist0 - dist1), ShadowVert + ShadowVertFree);
                        ShadowVertList[togg][newcount++] = ShadowVertFree++;
                    }
                }
            }
            ShadowVertNum = newcount;
            togg = !togg;
        }

// skip if no poly here

        if (ShadowVertNum < 3)
            continue;

// skip if any shadow vert points above car

        for (j = 0 ; j < ShadowVertNum ; j++)
        {
            if (ShadowVert[j].Pos.v[Y] < ymin - 64.0f)
                break;
        }

        if (j < ShadowVertNum)
            continue;

// process verts

        for (j = 0 ; j < ShadowVertNum ; j++)
        {
            svert = &ShadowVert[ShadowVertList[!togg][j]];

// update bounding box

            if (box)
            {
                if (svert->Pos.v[X] < box->Xmin) box->Xmin = svert->Pos.v[X];
                if (svert->Pos.v[X] > box->Xmax) box->Xmax = svert->Pos.v[X];
                if (svert->Pos.v[Y] < box->Ymin) box->Ymin = svert->Pos.v[Y];
                if (svert->Pos.v[Y] > box->Ymax) box->Ymax = svert->Pos.v[Y];
                if (svert->Pos.v[Z] < box->Zmin) box->Zmin = svert->Pos.v[Z];
                if (svert->Pos.v[Z] > box->Zmax) box->Zmax = svert->Pos.v[Z];
            }

// transform

            svert->Pos.v[Y] += yoff;

            RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &svert->Pos, &ShadowDrawVert[j].sx);
            ShadowDrawVert[j].tu = svert->tu;
            ShadowDrawVert[j].tv = svert->tv;
            ShadowDrawVert[j].color = rgb;

            FTOL((yfade - svert->Pos.v[Y]) * 0.2f + 100.0f, k);
            if (k > 100) k = 100;
            if (k < 0) k = 0;

            ModelChangeGouraudAlpha((MODEL_RGB*)&ShadowDrawVert[j].color, k);
        }

// draw fan

        if (semi == -1)
        {
            Assert(ShadowVertNum > 0); //$ADDITION [If we hit this assert, need to put back Jed's code to only call this if (ShadowVertNum > 0) ]
            DRAW_PRIM(D3DPT_TRIANGLEFAN, FVF_TEX1, ShadowDrawVert, ShadowVertNum, D3DDP_DONOTUPDATEEXTENTS);
        }
        else
        {
            for (j = 0 ; j < ShadowVertNum - 2 ; j++)
            {
                if (!SEMI_POLY_FREE()) continue;
                SEMI_POLY_SETUP_ZBIAS(vert, FALSE, 3, tpage, TRUE, semi, -256.0f);

                *(MEM32*)&vert[0] = *(MEM32*)&ShadowDrawVert[0];
                *(MEM32*)&vert[1] = *(MEM32*)&ShadowDrawVert[j + 1];
                *(MEM32*)&vert[2] = *(MEM32*)&ShadowDrawVert[j + 2];
            }
        }
    }
}

//////////////////////////////////////////
// find shadow / poly edge intersection //
//////////////////////////////////////////

void ClipShadowEdge(SHADOW_VERT *sv0, SHADOW_VERT *sv1, float mul, SHADOW_VERT *svout)
{
    svout->Pos.v[X] = sv0->Pos.v[X] + (sv1->Pos.v[X] - sv0->Pos.v[X]) * mul;
    svout->Pos.v[Y] = sv0->Pos.v[Y] + (sv1->Pos.v[Y] - sv0->Pos.v[Y]) * mul;
    svout->Pos.v[Z] = sv0->Pos.v[Z] + (sv1->Pos.v[Z] - sv0->Pos.v[Z]) * mul;

    svout->tu = sv0->tu + (sv1->tu - sv0->tu) * mul;
    svout->tv = sv0->tv + (sv1->tv - sv0->tv) * mul;
}
