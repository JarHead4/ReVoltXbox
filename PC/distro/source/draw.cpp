
#include "revolt.h"
#include "draw.h"
#include "dx.h"
#include "main.h"
#include "world.h"
#include "camera.h"
#include "geom.h"
#include "model.h"
#include "text.h"
#include "texture.h"
#include "level.h"
#include "mirror.h"
#include "timing.h"

// globals

BUCKET_ENV BucketEnvStill, BucketEnvStillFog, BucketEnvStillClip, BucketEnvStillClipFog;
BUCKET_ENV BucketEnvRoll, BucketEnvRollFog, BucketEnvRollClip, BucketEnvRollClipFog;
BUCKET_TEX0 BucketRGB, BucketFogRGB, BucketClipRGB, BucketClipFogRGB;
BUCKET_TEX1 Bucket[MAX_POLY_BUCKETS], BucketFog[MAX_POLY_BUCKETS], BucketClip[MAX_POLY_BUCKETS], BucketClipFog[MAX_POLY_BUCKETS];
VERTEX_TEX0 DrawVertsTEX0[8];
VERTEX_TEX1 DrawVertsTEX1[8];
VERTEX_TEX2 DrawVertsTEX2[8];
DRAW_SEMI_POLY SemiPoly[MAX_SEMI_POLYS];
WORLD_MESH_FX WorldMeshFx[MAX_WORLD_MESH_FX];
MODEL_MESH_FX ModelMeshFx[MAX_MODEL_MESH_FX];
long SemiCount, WorldMeshFxCount, ModelMeshFxCount;

static unsigned short ClipVertNum, ClipVertFree;
static unsigned short ClipVertList[2][8];
static VERTEX_TEX1 ClipVert[32];
static long Poly3dCount;
static DRAW_3D_POLY Poly3d[MAX_3D_POLYS];
static VEC JumpSparkOffset[JUMPSPARK_OFFSET_NUM];
static long JumpSparkTime, JumpSparkOff, JumpSparkSinTime;
static float JumpSparkSinDiv;


// semi shell gaps

static long SemiShellGap[] = {13, 4, 1};

////////////////////////
// init poly buckets //
////////////////////////

void InitPolyBuckets(void)
{
	long i;

// env

	BucketEnvStill.CurrentIndex = BucketEnvStill.Index;
	BucketEnvStill.CurrentVerts = BucketEnvStill.Verts;

	BucketEnvStillFog.CurrentIndex = BucketEnvStillFog.Index;
	BucketEnvStillFog.CurrentVerts = BucketEnvStillFog.Verts;

	BucketEnvStillClip.CurrentIndex = BucketEnvStillClip.Index;
	BucketEnvStillClip.CurrentVerts = BucketEnvStillClip.Verts;

	BucketEnvStillClipFog.CurrentIndex = BucketEnvStillClipFog.Index;
	BucketEnvStillClipFog.CurrentVerts = BucketEnvStillClipFog.Verts;

	BucketEnvRoll.CurrentIndex = BucketEnvRoll.Index;
	BucketEnvRoll.CurrentVerts = BucketEnvRoll.Verts;

	BucketEnvRollFog.CurrentIndex = BucketEnvRollFog.Index;
	BucketEnvRollFog.CurrentVerts = BucketEnvRollFog.Verts;

	BucketEnvRollClip.CurrentIndex = BucketEnvRollClip.Index;
	BucketEnvRollClip.CurrentVerts = BucketEnvRollClip.Verts;

	BucketEnvRollClipFog.CurrentIndex = BucketEnvRollClipFog.Index;
	BucketEnvRollClipFog.CurrentVerts = BucketEnvRollClipFog.Verts;

// rgb

	BucketRGB.CurrentIndex = BucketRGB.Index;
	BucketRGB.CurrentVerts = BucketRGB.Verts;

	BucketFogRGB.CurrentIndex = BucketFogRGB.Index;
	BucketFogRGB.CurrentVerts = BucketFogRGB.Verts;

	BucketClipRGB.CurrentIndex = BucketClipRGB.Index;
	BucketClipRGB.CurrentVerts = BucketClipRGB.Verts;

	BucketClipFogRGB.CurrentIndex = BucketClipFogRGB.Index;
	BucketClipFogRGB.CurrentVerts = BucketClipFogRGB.Verts;

// textured

	for (i = 0 ; i < MAX_POLY_BUCKETS ; i++)
	{
		Bucket[i].CurrentIndex = Bucket[i].Index;
		Bucket[i].CurrentVerts = Bucket[i].Verts;

		BucketFog[i].CurrentIndex = BucketFog[i].Index;
		BucketFog[i].CurrentVerts = BucketFog[i].Verts;

		BucketClip[i].CurrentIndex = BucketClip[i].Index;
		BucketClip[i].CurrentVerts = BucketClip[i].Verts;

		BucketClipFog[i].CurrentIndex = BucketClipFog[i].Index;
		BucketClipFog[i].CurrentVerts = BucketClipFog[i].Verts;
	}
}

///////////////////////
// kill poly buckets //
///////////////////////

void KillPolyBuckets(void)
{
}

////////////////////////
// flush poly buckets //
////////////////////////

void FlushPolyBuckets(void)
{
	short i;

// rgb

	if (BucketRGB.CurrentVerts != BucketRGB.Verts)
	{
		FOG_OFF();
		SET_TPAGE(-1);
		FlushOneBucketTEX0(&BucketRGB, FALSE);
	}

	if (BucketFogRGB.CurrentVerts != BucketFogRGB.Verts)
	{
		FOG_ON();
		SET_TPAGE(-1);
		FlushOneBucketTEX0(&BucketFogRGB, FALSE);
	}

	if (BucketClipRGB.CurrentVerts != BucketClipRGB.Verts)
	{
		FOG_OFF();
		SET_TPAGE(-1);
		FlushOneBucketTEX0(&BucketClipRGB, TRUE);
	}

	if (BucketClipFogRGB.CurrentVerts != BucketClipFogRGB.Verts)
	{
		FOG_ON();
		SET_TPAGE(-1);
		FlushOneBucketTEX0(&BucketClipFogRGB, TRUE);
	}

// textured

	for (i = 0 ; i < MAX_POLY_BUCKETS ; i++)
	{
		if (Bucket[i].CurrentVerts != Bucket[i].Verts)
		{
			FOG_OFF();
			SET_TPAGE(i);
			FlushOneBucketTEX1(&Bucket[i], FALSE);
		}

		if (BucketFog[i].CurrentVerts != BucketFog[i].Verts)
		{
			FOG_ON();
			SET_TPAGE(i);
			FlushOneBucketTEX1(&BucketFog[i], FALSE);
		}

		if (BucketClip[i].CurrentVerts != BucketClip[i].Verts)
		{
			FOG_OFF();
			SET_TPAGE(i);
			FlushOneBucketTEX1(&BucketClip[i], TRUE);
		}

		if (BucketClipFog[i].CurrentVerts != BucketClipFog[i].Verts)
		{
			FOG_ON();
			SET_TPAGE(i);
			FlushOneBucketTEX1(&BucketClipFog[i], TRUE);
		}
	}

// reset states

	FOG_OFF();
}

///////////////////////
// flush env buckets //
///////////////////////

void FlushEnvBuckets(void)
{

// set env render states

	ZWRITE_OFF();
	ALPHA_ON();

	ALPHA_SRC(D3DBLEND_ONE);
	ALPHA_DEST(D3DBLEND_ONE);

// env still

	if (BucketEnvStill.CurrentVerts != BucketEnvStill.Verts)
	{
		FOG_OFF();
		SET_TPAGE(TPAGE_ENVSTILL);
		FlushOneBucketEnv(&BucketEnvStill, FALSE);
	}

	if (BucketEnvStillFog.CurrentVerts != BucketEnvStillFog.Verts)
	{
		FOG_ON();
		SET_TPAGE(TPAGE_ENVSTILL);
		FlushOneBucketEnv(&BucketEnvStillFog, FALSE);
	}

	if (BucketEnvStillClip.CurrentVerts != BucketEnvStillClip.Verts)
	{
		FOG_OFF();
		SET_TPAGE(TPAGE_ENVSTILL);
		FlushOneBucketEnv(&BucketEnvStillClip, TRUE);
	}

	if (BucketEnvStillClipFog.CurrentVerts != BucketEnvStillClipFog.Verts)
	{
		FOG_ON();
		SET_TPAGE(TPAGE_ENVSTILL);
		FlushOneBucketEnv(&BucketEnvStillClipFog, TRUE);
	}

// env roll

	if (BucketEnvRoll.CurrentVerts != BucketEnvRoll.Verts)
	{
		FOG_OFF();
		SET_TPAGE(TPAGE_ENVROLL);
		FlushOneBucketEnv(&BucketEnvRoll, FALSE);
	}

	if (BucketEnvRollFog.CurrentVerts != BucketEnvRollFog.Verts)
	{
		FOG_ON();
		SET_TPAGE(TPAGE_ENVROLL);
		FlushOneBucketEnv(&BucketEnvRollFog, FALSE);
	}

	if (BucketEnvRollClip.CurrentVerts != BucketEnvRollClip.Verts)
	{
		FOG_OFF();
		SET_TPAGE(TPAGE_ENVROLL);
		FlushOneBucketEnv(&BucketEnvRollClip, TRUE);
	}

	if (BucketEnvRollClipFog.CurrentVerts != BucketEnvRollClipFog.Verts)
	{
		FOG_ON();
		SET_TPAGE(TPAGE_ENVROLL);
		FlushOneBucketEnv(&BucketEnvRollClipFog, TRUE);
	}

// reset states

	FOG_OFF();
	ALPHA_OFF();
	ZWRITE_ON();
}

//////////////////////
// flush one bucket //
//////////////////////

void FlushOneBucketTEX0(BUCKET_TEX0 *bucket, long clip)
{
	DWORD flag;

	if (clip) flag = D3DDP_DONOTUPDATEEXTENTS;
	else flag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP;

	D3Ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, FVF_TEX0, bucket->Verts, (DWORD)(bucket->CurrentVerts - bucket->Verts), bucket->Index, (DWORD)(bucket->CurrentIndex - bucket->Index), flag);

	bucket->CurrentIndex = bucket->Index;
	bucket->CurrentVerts = bucket->Verts;
}

//////////////////////
// flush one bucket //
//////////////////////

void FlushOneBucketTEX1(BUCKET_TEX1 *bucket, long clip)
{
	DWORD flag;

	if (clip) flag = D3DDP_DONOTUPDATEEXTENTS;
	else flag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP;

	D3Ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, FVF_TEX1, bucket->Verts, (DWORD)(bucket->CurrentVerts - bucket->Verts), bucket->Index, (DWORD)(bucket->CurrentIndex - bucket->Index), flag);

	bucket->CurrentIndex = bucket->Index;
	bucket->CurrentVerts = bucket->Verts;
}

//////////////////////
// flush one bucket //
//////////////////////

void FlushOneBucketEnv(BUCKET_ENV *bucket, long clip)
{
	DWORD flag;

	if (clip) flag = D3DDP_DONOTUPDATEEXTENTS;
	else flag = D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP;

	D3Ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, FVF_TEX1, bucket->Verts, (DWORD)(bucket->CurrentVerts - bucket->Verts), bucket->Index, (DWORD)(bucket->CurrentIndex - bucket->Index), flag);

	bucket->CurrentIndex = bucket->Index;
	bucket->CurrentVerts = bucket->Verts;
}

//////////////////////////
// reset 3D poly list //
//////////////////////////

void Reset3dPolyList(void)
{
	Poly3dCount = 0;
}

///////////////////////
// draw 3D poly list //
///////////////////////

void Draw3dPolyList(void)
{
	long i, j;
	DRAW_3D_POLY *poly;
	VERTEX_TEX1 *verts;

// skip if none

	if (!Poly3dCount) return;

// leap thru polys

	poly = Poly3d;

	for (i = Poly3dCount ; i ; i--, poly++)
	{

// semi

		if (poly->SemiType != -1)
		{
			if (SEMI_POLY_FREE())
			{
				SEMI_POLY_SETUP(verts, poly->Fog, poly->VertNum, poly->Tpage, TRUE, poly->SemiType);
				for (j = 0 ; j < poly->VertNum ; j++)
				{
					RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &poly->Pos[j], (REAL*)&verts[j]);
					verts[j].color = poly->Verts[j].color;
					verts[j].specular = poly->Verts[j].specular;
					verts[j].tu = poly->Verts[j].tu;
					verts[j].tv = poly->Verts[j].tv;
				}
			}
		}

// opaque

		else
		{
			for (j = 0 ; j < poly->VertNum ; j++)
				RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &poly->Pos[j], (REAL*)&poly->Verts[j]);

			SET_TPAGE((short)poly->Tpage);

			if (poly->Fog)
			{
				FOG_ON();
			}
			else
			{
				FOG_OFF();
			}

			D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, poly->Verts, poly->VertNum, D3DDP_DONOTUPDATEEXTENTS);
		}
	}

// fog off

	FOG_OFF();
}

/////////////////////////////
// add a 3d poly to list //
/////////////////////////////

DRAW_3D_POLY *Get3dPoly(void)
{

// ret if full

	if (Poly3dCount == MAX_3D_POLYS)
		return NULL;

// ret ptr to poly

	return &Poly3d[Poly3dCount++];
}

////////////////////////
// draw nearclip poly //
////////////////////////

void DrawNearClipPolyTEX0(VEC *pos, long *rgb, long vertnum)
{
	long newvertnum, i, lmul, newrgb[8];
	float mul, z0, z1, z[8];
	VEC *vec0, *vec1, newpos[8];
	MODEL_RGB *rgb0, *rgb1, *rgbout;

// clip

	newvertnum = 0;

	for (i = 0 ; i < vertnum ; i++)
		z[i] = pos[i].v[X] * ViewMatrixScaled.m[RZ] + pos[i].v[Y] * ViewMatrixScaled.m[UZ] + pos[i].v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];

	for (i = 0 ; i < vertnum ; i++)
	{
		vec0 = &pos[i];
		vec1 = &pos[(i + 1) % vertnum];
		z0 = z[i];
		z1 = z[(i + 1) % vertnum];

		if (z0 >= 0)
		{
			newpos[newvertnum] = pos[i];
			newrgb[newvertnum] = rgb[i];
			newvertnum++;

			if (z1 < 0)
			{
				mul = z0 / (z0 - z1);

				newpos[newvertnum].v[X] = vec0->v[X] + (vec1->v[X] - vec0->v[X]) * mul;
				newpos[newvertnum].v[Y] = vec0->v[Y] + (vec1->v[Y] - vec0->v[Y]) * mul;
				newpos[newvertnum].v[Z] = vec0->v[Z] + (vec1->v[Z] - vec0->v[Z]) * mul;

				FTOL(mul * 256, lmul);

				rgb0 = (MODEL_RGB*)&rgb[i];
				rgb1 = (MODEL_RGB*)&rgb[(i + 1) % vertnum];
				rgbout = (MODEL_RGB*)&newrgb[newvertnum];

				rgbout->a = rgb0->a + (unsigned char)(((rgb1->a - rgb0->a) * lmul) >> 8);
				rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
				rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
				rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);

				newvertnum++;
			}
		}
		else
		{
			if (z1 >= RenderSettings.NearClip)
			{
				mul = z1 / (z1 - z0);

				newpos[newvertnum].v[X] = vec1->v[X] + (vec0->v[X] - vec1->v[X]) * mul;
				newpos[newvertnum].v[Y] = vec1->v[Y] + (vec0->v[Y] - vec1->v[Y]) * mul;
				newpos[newvertnum].v[Z] = vec1->v[Z] + (vec0->v[Z] - vec1->v[Z]) * mul;

				FTOL(mul * 256, lmul);

				rgb0 = (MODEL_RGB*)&rgb[(i + 1) % vertnum];
				rgb1 = (MODEL_RGB*)&rgb[i];
				rgbout = (MODEL_RGB*)&newrgb[newvertnum];

				rgbout->a = rgb0->a + (unsigned char)(((rgb1->a - rgb0->a) * lmul) >> 8);
				rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
				rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
				rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);

				newvertnum++;
			}
		}
	}

	if (!newvertnum)
		return;

// setup verts

	for (i = 0 ; i < newvertnum ; i++)
	{
		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &newpos[i], &DrawVertsTEX0[i].sx);
		DrawVertsTEX0[i].color = newrgb[i];
	}

// draw

	D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX0, DrawVertsTEX0, newvertnum, D3DDP_DONOTUPDATEEXTENTS);
}

//////////////////////////
// reset semi poly list //
//////////////////////////

void ResetSemiList(void)
{
	SemiCount = 0;
}

/////////////////////////
// draw semi poly list //
/////////////////////////

void DrawSemiList(void)
{
	long i, j, k, gap;
	float z;
	DRAW_SEMI_POLY *poly, swap;

// skip if none

	if (!SemiCount) return;

// setup misc render states

	ALPHA_ON();
	ZWRITE_OFF();

// set z's

	poly = SemiPoly;

	for (i = SemiCount ; i ; i--, poly++)
	{
		z = 0.0f;

		if (poly->Tpage == -1)
			for (j = 0 ; j < poly->VertNum ; j++) z += 1.0f / poly->VertsRGB[j].rhw;
		else
			for (j = 0 ; j < poly->VertNum ; j++) z += 1.0f / poly->Verts[j].rhw;

		poly->z += z / j;
	}

// sort

	for (k = 0 ; k < 3 ; k++)
	{
		gap = SemiShellGap[k];
		for (i = gap ; i < SemiCount ; i++)
		{
			swap = SemiPoly[i];
			for (j = i - gap ; j >= 0 && swap.z > SemiPoly[j].z ; j -= gap)
			{
				SemiPoly[j + gap] = SemiPoly[j];
			}
			SemiPoly[j + gap] = swap;
		}
	}

// loop thru

	poly = SemiPoly;
	for (i = SemiCount ; i ; i--, poly++)
	{

// set render states

		SET_TPAGE((short)poly->Tpage);

		if (poly->Fog)
		{
			FOG_ON();
		}
		else
		{
			FOG_OFF();
		}

		if (!poly->SemiType)
		{
			ALPHA_SRC(D3DBLEND_SRCALPHA);
			ALPHA_DEST(D3DBLEND_INVSRCALPHA);
		}
		else if (poly->SemiType == 1)
		{
			ALPHA_SRC(D3DBLEND_ONE);
			ALPHA_DEST(D3DBLEND_ONE);
		}
		else
		{
			ALPHA_SRC(D3DBLEND_ZERO);
			ALPHA_DEST(D3DBLEND_INVSRCCOLOR);
		}

// draw

		if (poly->Tpage == -1)
			D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX0, poly->VertsRGB, poly->VertNum, poly->DrawFlag);
		else
			D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, poly->Verts, poly->VertNum, poly->DrawFlag);
	}

// fog off

	FOG_OFF();
}

////////////////////////////////////////
// draw a 'face me' poly + reflection //
////////////////////////////////////////

void DrawFacingPolyMirror(VEC *pos, FACING_POLY *poly, long semi, float zbias)
{
	float screen[4];
	float xadd, yadd;
	float z, fog, mirrorfog;
	VERTEX_TEX1 *vert;
	float mirroradd, mirrory;

// draw original

	DrawFacingPoly(pos, poly, semi, zbias);

// reflect?

	if (!RenderSettings.Mirror)
		return;

	if (!GetMirrorPlane(pos))
		return;

	if (ViewCameraPos.v[Y] >= MirrorHeight)
		return;

	mirroradd = MirrorHeight - pos->v[Y];
	if (mirroradd <= -MIRROR_OVERLAP_TOL)
		return;

// yep

	mirrory = MirrorHeight + mirroradd;

// get screen coors

	z = pos->v[X] * ViewMatrixScaled.m[RZ] + mirrory * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
	if (z < RenderSettings.NearClip || z >= RenderSettings.FarClip)
		return;

	screen[0] = (pos->v[X] * ViewMatrixScaled.m[RX] + mirrory * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
	screen[1] = (pos->v[X] * ViewMatrixScaled.m[RY] + mirrory * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

	screen[3] = 1 / z;
	screen[2] = GET_ZBUFFER(z + zbias);

// get verts

	if (semi == -1)
	{
		vert = DrawVertsTEX1;
	}
	else
	{
		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
	}

// get xy adds

	xadd = (poly->Xsize * RenderSettings.MatScaleX) / z;
	yadd = (poly->Ysize * RenderSettings.MatScaleY) / z;

// build 4 from one

	vert[0].sx = vert[3].sx = screen[0] - xadd;
	vert[1].sx = vert[2].sx = screen[0] + xadd;

	vert[0].sy = vert[1].sy = screen[1] + yadd;
	vert[2].sy = vert[3].sy = screen[1] - yadd;

	vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = screen[2];
	vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = screen[3];

// set RGB's

	vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

	vert[0].tu = vert[3].tu = poly->U;
	vert[1].tu = vert[2].tu = poly->U + poly->Usize;

	vert[0].tv = vert[1].tv = poly->V;
	vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

	mirrorfog = GET_MIRROR_FOG(mirroradd);
	if (mirrorfog < 0) mirrorfog = 0;

	fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	fog -= mirrorfog;
	if (fog < 0) fog = 0;
	vert[0].specular = vert[1].specular = vert[2].specular = vert[3].specular = FTOL3(fog) << 24;

// draw

	if (semi == -1)
	{
		SET_TPAGE(poly->Tpage);
		D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
	}
}

///////////////////////////
// draw a 'face me' poly //
///////////////////////////

void DrawFacingPoly(VEC *pos, FACING_POLY *poly, long semi, float zbias)
{
	float screen[4];
	float xadd, yadd;
	float z, fog;
	VERTEX_TEX1 *vert;

// get screen coors

	z = pos->v[X] * ViewMatrixScaled.m[RZ] + pos->v[Y] * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
	if (z < RenderSettings.NearClip || z >= RenderSettings.FarClip)
		return;

	screen[0] = (pos->v[X] * ViewMatrixScaled.m[RX] + pos->v[Y] * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X]) / z + RenderSettings.GeomCentreX;
	screen[1] = (pos->v[X] * ViewMatrixScaled.m[RY] + pos->v[Y] * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y]) / z + RenderSettings.GeomCentreY;

	screen[3] = 1 / z;
	screen[2] = GET_ZBUFFER(z + zbias);

// get verts

	if (semi == -1)
	{
		vert = DrawVertsTEX1;
	}
	else
	{
		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
	}

// get xy adds

	xadd = (poly->Xsize * RenderSettings.MatScaleX) / z;
	yadd = (poly->Ysize * RenderSettings.MatScaleY) / z;

// build 4 from one

	vert[0].sx = vert[3].sx = screen[0] - xadd;
	vert[1].sx = vert[2].sx = screen[0] + xadd;

	vert[0].sy = vert[1].sy = screen[1] - yadd;
	vert[2].sy = vert[3].sy = screen[1] + yadd;

	vert[0].sz = vert[1].sz = vert[2].sz = vert[3].sz = screen[2];
	vert[0].rhw = vert[1].rhw = vert[2].rhw = vert[3].rhw = screen[3];

// set RGB's

	vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

	vert[0].tu = vert[3].tu = poly->U;
	vert[1].tu = vert[2].tu = poly->U + poly->Usize;

	vert[0].tv = vert[1].tv = poly->V;
	vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

	fog = (RenderSettings.FarClip - z) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	else if (fog < 0) fog = 0;
	vert[0].specular = vert[1].specular = vert[2].specular = vert[3].specular = FTOL3(fog) << 24;

// draw

	if (semi == -1)
	{
		SET_TPAGE(poly->Tpage);
		D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
	}
}

//////////////////////////////////////////////////////
// draw a 'face me' poly with rotation + reflection //
//////////////////////////////////////////////////////

void DrawFacingPolyRotMirror(VEC *pos, MAT *mat, FACING_POLY *poly, long semi, float zbias)
{
	float fog, mirrorfog;
	VEC trans;
	VEC v0, v1, v2, v3;
	MAT mat2, mat3, mat4;
	VERTEX_TEX1 *vert;
	float mirroradd, mirrory;

// draw original

	DrawFacingPolyRot(pos, mat, poly, semi, zbias);

// reflect?

	if (!RenderSettings.Mirror)
		return;

	if (!GetMirrorPlane(pos))
		return;

	if (ViewCameraPos.v[Y] >= MirrorHeight)
		return;

	mirroradd = MirrorHeight - pos->v[Y];
	if (mirroradd <= -MIRROR_OVERLAP_TOL)
		return;

// yep

	mirrory = MirrorHeight + mirroradd;

// get vector translation

	trans.v[Z] = pos->v[X] * ViewMatrixScaled.m[RZ] + mirrory * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
	if (trans.v[Z] < RenderSettings.NearClip - 128 || trans.v[Z] >= RenderSettings.FarClip + 128)
		return;

	trans.v[X] = pos->v[X] * ViewMatrixScaled.m[RX] + mirrory * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X];
	trans.v[Y] = pos->v[X] * ViewMatrixScaled.m[RY] + mirrory * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y];

// get verts

	if (semi == -1)
	{
		vert = DrawVertsTEX1;
	}
	else
	{
		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
	}

// set mat

	CopyMat(mat, &mat4);
	mat4.m[UX] = -mat4.m[UX];
	mat4.m[UY] = -mat4.m[UY];
	mat4.m[UZ] = -mat4.m[UZ];

	BuildLookMatrixForward(pos, &ViewCameraPos, &mat2);
	MulMatrix(&ViewMatrixScaled, &mat2, &mat3);
	MulMatrix(&mat3, &mat4, &mat2);

// setup 4 vectors

	SetVector(&v0, -poly->Xsize, -poly->Ysize, 0);
	SetVector(&v1, poly->Xsize, -poly->Ysize, 0);
	SetVector(&v2, poly->Xsize, poly->Ysize, 0);
	SetVector(&v3, -poly->Xsize, poly->Ysize, 0);

// get screen coors

	RotTransPersVectorZbias(&mat2, &trans, &v0, &vert[0].sx, zbias);
	RotTransPersVectorZbias(&mat2, &trans, &v1, &vert[1].sx, zbias);
	RotTransPersVectorZbias(&mat2, &trans, &v2, &vert[2].sx, zbias);
	RotTransPersVectorZbias(&mat2, &trans, &v3, &vert[3].sx, zbias);

// set RGB's

	vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

	vert[0].tu = vert[3].tu = poly->U;
	vert[1].tu = vert[2].tu = poly->U + poly->Usize;

	vert[0].tv = vert[1].tv = poly->V;
	vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

	mirrorfog = GET_MIRROR_FOG(mirroradd);
	if (mirrorfog < 0) mirrorfog = 0;

	fog = (RenderSettings.FarClip - 1 / vert[0].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	fog -= mirrorfog;
	if (fog < 0) fog = 0;
	vert[0].specular = FTOL3(fog) << 24;

	fog = (RenderSettings.FarClip - 1 / vert[1].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	fog -= mirrorfog;
	if (fog < 0) fog = 0;
	vert[1].specular = FTOL3(fog) << 24;

	fog = (RenderSettings.FarClip - 1 / vert[2].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	fog -= mirrorfog;
	if (fog < 0) fog = 0;
	vert[2].specular = FTOL3(fog) << 24;

	fog = (RenderSettings.FarClip - 1 / vert[3].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	fog -= mirrorfog;
	if (fog < 0) fog = 0;
	vert[3].specular = FTOL3(fog) << 24;

// draw

	if (semi == -1)
	{
		SET_TPAGE(poly->Tpage);
		D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
	}
}

/////////////////////////////////////////
// draw a 'face me' poly with rotation //
/////////////////////////////////////////

void DrawFacingPolyRot(VEC *pos, MAT *mat, FACING_POLY *poly, long semi, float zbias)
{
	float fog;
	VEC trans;
	VEC v0, v1, v2, v3;
	MAT mat2, mat3;
	VERTEX_TEX1 *vert;

// get vector translation

	trans.v[Z] = pos->v[X] * ViewMatrixScaled.m[RZ] + pos->v[Y] * ViewMatrixScaled.m[UZ] + pos->v[Z] * ViewMatrixScaled.m[LZ] + ViewTransScaled.v[Z];
	if (trans.v[Z] < RenderSettings.NearClip - 128 || trans.v[Z] >= RenderSettings.FarClip + 128)
		return;

	trans.v[X] = pos->v[X] * ViewMatrixScaled.m[RX] + pos->v[Y] * ViewMatrixScaled.m[UX] + pos->v[Z] * ViewMatrixScaled.m[LX] + ViewTransScaled.v[X];
	trans.v[Y] = pos->v[X] * ViewMatrixScaled.m[RY] + pos->v[Y] * ViewMatrixScaled.m[UY] + pos->v[Z] * ViewMatrixScaled.m[LY] + ViewTransScaled.v[Y];

// get verts

	if (semi == -1)
	{
		vert = DrawVertsTEX1;
	}
	else
	{
		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP(vert, TRUE, 4, poly->Tpage, TRUE, semi);
	}

// set mat

	BuildLookMatrixForward(pos, &ViewCameraPos, &mat2);
	MulMatrix(&ViewMatrixScaled, &mat2, &mat3);
	MulMatrix(&mat3, mat, &mat2);

// setup 4 vectors

	SetVector(&v0, -poly->Xsize, -poly->Ysize, 0);
	SetVector(&v1, poly->Xsize, -poly->Ysize, 0);
	SetVector(&v2, poly->Xsize, poly->Ysize, 0);
	SetVector(&v3, -poly->Xsize, poly->Ysize, 0);

// get screen coors

	RotTransPersVectorZbias(&mat2, &trans, &v0, &vert[0].sx, zbias);
	RotTransPersVectorZbias(&mat2, &trans, &v1, &vert[1].sx, zbias);
	RotTransPersVectorZbias(&mat2, &trans, &v2, &vert[2].sx, zbias);
	RotTransPersVectorZbias(&mat2, &trans, &v3, &vert[3].sx, zbias);

// set RGB's

	vert[0].color = vert[1].color = vert[2].color = vert[3].color = poly->RGB;

// set UV's

	vert[0].tu = vert[3].tu = poly->U;
	vert[1].tu = vert[2].tu = poly->U + poly->Usize;

	vert[0].tv = vert[1].tv = poly->V;
	vert[2].tv = vert[3].tv = poly->V + poly->Vsize;

// set fog

	fog = (RenderSettings.FarClip - 1 / vert[0].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	else if (fog < 0) fog = 0;
	vert[0].specular = FTOL3(fog) << 24;

	fog = (RenderSettings.FarClip - 1 / vert[1].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	else if (fog < 0) fog = 0;
	vert[1].specular = FTOL3(fog) << 24;

	fog = (RenderSettings.FarClip - 1 / vert[2].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	else if (fog < 0) fog = 0;
	vert[2].specular = FTOL3(fog) << 24;

	fog = (RenderSettings.FarClip - 1 / vert[3].rhw) * RenderSettings.FogMul;
	if (fog > 255) fog = 255;
	else if (fog < 0) fog = 0;
	vert[3].specular = FTOL3(fog) << 24;

// draw

	if (semi == -1)
	{
		SET_TPAGE(poly->Tpage);
		D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, vert, 4, D3DDP_DONOTUPDATEEXTENTS);
	}
}

//////////////////////////////////
// init main render state stuff //
//////////////////////////////////

void InitRenderStates(void)
{

// set misc render states

	RenderAlpha = TRUE;

	ALPHA_OFF();
	FOG_OFF();
	WIREFRAME_ON();
	ZBUFFER_ON();
	ZWRITE_ON();

// reset screen debug misc

	#if SCREEN_DEBUG
	WorldPolyCount = 0;
	WorldDrawnCount = 0;
	ModelPolyCount = 0;
	ModelDrawnCount = 0;
	RenderStateChange = 0;
	TextureStateChange = 0;
	#endif
}

//////////////////////////////
// set near / far clip vars //
//////////////////////////////

void SetNearFar(REAL n, REAL f)
{
	RenderSettings.NearClip = n;
	RenderSettings.FarClip = f;

	RenderSettings.DrawDist = RenderSettings.FarClip - RenderSettings.NearClip;
	RenderSettings.FarDivDist = RenderSettings.FarClip / RenderSettings.DrawDist;
	RenderSettings.FarMulNear = RenderSettings.FarClip * RenderSettings.NearClip;
}

//////////////////
// set fog vars //
//////////////////

void SetFogVars(REAL fogstart, REAL vertstart, REAL vertend)
{

// set fog vars

	RenderSettings.FogStart = fogstart;
	RenderSettings.FogDist = RenderSettings.FarClip - RenderSettings.FogStart;
	if (RenderSettings.FogDist <= 0) RenderSettings.FogMul = 256, RenderSettings.FogStart += 0xffff;
	else RenderSettings.FogMul = 256 / RenderSettings.FogDist;

	RenderSettings.VertFogStart = vertstart;
	RenderSettings.VertFogEnd = vertend;
	if (RenderSettings.VertFogStart == RenderSettings.VertFogEnd) RenderSettings.VertFogMul = 0;
	else RenderSettings.VertFogMul = 256 / (RenderSettings.VertFogEnd - RenderSettings.VertFogStart);
	if (RenderSettings.VertFogMul) RenderSettings.FogStart = 0;
}

///////////////////
// draw XYZ axis //
///////////////////

void DrawAxis(MAT *mat, VEC *pos)
{
	FACING_POLY fp;
	VEC in, out;

// init facing misc

	fp.Tpage = TPAGE_FONT;
	fp.Xsize = 8;
	fp.Ysize = 8;
	fp.Usize = FONT_UWIDTH / 256.0f;
	fp.Vsize = FONT_VHEIGHT / 256.0f;
	fp.RGB = 0xffffff;

// X

	SetVector(&in, 64, 0, 0);
	RotVector(mat, &in, &out);
	AddVector(&out, pos, &out);

	fp.U = 221.0f / 256.0f;
	fp.V = 34.0f / 256.0f;
	DrawFacingPoly(&out, &fp, -1, 0);

	DrawLine(pos, &out, 0xff0000, 0xff0000);

// Y

	SetVector(&in, 0, 64, 0);
	RotVector(mat, &in, &out);
	AddVector(&out, pos, &out);

	fp.U = 234.0f / 256.0f;
	fp.V = 34.0f / 256.0f;
	DrawFacingPoly(&out, &fp, -1, 0);

	DrawLine(pos, &out, 0x00ff00, 0x00ff00);

// Z

	SetVector(&in, 0, 0, 64);
	RotVector(mat, &in, &out);
	AddVector(&out, pos, &out);

	fp.U = 0.0f / 256.0f;
	fp.V = 51.0f / 256.0f;
	DrawFacingPoly(&out, &fp, -1, 0);

	DrawLine(pos, &out, 0x0000ff, 0x0000ff);
}

//////////////////////////
// dump image on screen //
//////////////////////////

void DumpImage(char handle, float x, float y, float w, float h, float u, float v, float tw, float th, unsigned long col)
{
	long i;
	float xstart, ystart, xsize, ysize;

// scale

	xstart = x * RenderSettings.GeomScaleX + ScreenLeftClip;
	ystart = y * RenderSettings.GeomScaleY + ScreenTopClip;

	xsize = w * RenderSettings.GeomScaleX;
	ysize = h * RenderSettings.GeomScaleY;

// set render states

	SET_TPAGE(handle);

// init vert misc

	for (i = 0 ; i < 4 ; i++)
	{
		DrawVertsTEX1[i].color = col;
		DrawVertsTEX1[i].rhw = 1;
	}

// set screen coors

	DrawVertsTEX1[0].sx = DrawVertsTEX1[3].sx = xstart;
	DrawVertsTEX1[1].sx = DrawVertsTEX1[2].sx = xstart + xsize;
	DrawVertsTEX1[0].sy = DrawVertsTEX1[1].sy = ystart;
	DrawVertsTEX1[2].sy = DrawVertsTEX1[3].sy = ystart + ysize;

// set uv's

	DrawVertsTEX1[0].tu = DrawVertsTEX1[3].tu = u;
	DrawVertsTEX1[1].tu = DrawVertsTEX1[2].tu = u + tw;
	DrawVertsTEX1[0].tv = DrawVertsTEX1[1].tv = v;
	DrawVertsTEX1[2].tv = DrawVertsTEX1[3].tv = v + th;

// draw

	D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
}

//////////////////////
// draw a mouse ptr //
//////////////////////

void DrawMousePointer(unsigned long color)
{
	char i;
	float x, y, xs, ys, tu, tv;

// init verts

	for (i = 0 ; i < 4 ; i++)
	{
		DrawVertsTEX1[i].color = color;
		DrawVertsTEX1[i].rhw = 1;
	}

// set screen coors

	x = MouseXpos * RenderSettings.GeomScaleX + ScreenLeftClip;
	y = MouseYpos * RenderSettings.GeomScaleY + ScreenTopClip;
	xs = 12 * RenderSettings.GeomScaleX;
	ys = 16 * RenderSettings.GeomScaleY;

	DrawVertsTEX1[0].sx = x;
	DrawVertsTEX1[0].sy = y;

	DrawVertsTEX1[1].sx = x + xs;
	DrawVertsTEX1[1].sy = y;

	DrawVertsTEX1[2].sx = x + xs;
	DrawVertsTEX1[2].sy = y + ys;

	DrawVertsTEX1[3].sx = x;
	DrawVertsTEX1[3].sy = y + ys;

// set uv's

	tu = 234.0f;
	tv = 68.0f;

	DrawVertsTEX1[0].tu = tu / 256;
	DrawVertsTEX1[0].tv = tv / 256;

	DrawVertsTEX1[1].tu = (tu + 12) / 256;
	DrawVertsTEX1[1].tv = tv / 256;

	DrawVertsTEX1[2].tu = (tu + 12) / 256;
	DrawVertsTEX1[2].tv = (tv + 16) / 256;

	DrawVertsTEX1[3].tu = tu / 256;
	DrawVertsTEX1[3].tv = (tv + 16) / 256;

// draw

	D3Ddevice->DrawPrimitive(D3DPT_TRIANGLEFAN, FVF_TEX1, DrawVertsTEX1, 4, D3DDP_DONOTUPDATEEXTENTS);
}

///////////////////
// load a bitmap //
///////////////////

BOOL LoadBitmap(char *bitmap, HBITMAP *hbm)
{
	*hbm = (HBITMAP)LoadImage(NULL, bitmap, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	return (BOOL)*hbm;
}

/////////////////
// free bitmap //
/////////////////

BOOL FreeBitmap(HBITMAP hbm)
{
	return (BOOL)DeleteObject(hbm);
}

////////////////////////////////////////////
// blit from a bitmap handle to a surface //
////////////////////////////////////////////

BOOL BlitBitmap(HBITMAP hbm, IDirectDrawSurface4 **surface)
{
	HRESULT r;
	BITMAP bm;
	HDC dcimage, dc;

// get bitmap info

	GetObject(hbm, sizeof(bm), &bm);

// get  dc's

	dcimage = CreateCompatibleDC(NULL);
	SelectObject(dcimage, hbm);
	r = (*surface)->GetDC(&dc);

// blit

	if (r == DD_OK)
		r = StretchBlt(dc, 0, 0, ScreenXsize, ScreenYsize, dcimage, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

// free dc's

	(*surface)->ReleaseDC(dc);
	DeleteDC(dcimage);

// return

	return (r == DD_OK);
}



/////////////////////////////////////////////////////////////////////
//
// DrawCollPoly:
//
/////////////////////////////////////////////////////////////////////

void DrawCollPoly(NEWCOLLPOLY *poly)
{
	long col, vertnum;
	REAL normDotUp;
	VEC pos[4];
	long rgb[4];

	// Get the polygon vertex positions and number of vertices
	vertnum = GetCollPolyVertices(poly, &pos[0], &pos[1], &pos[2], &pos[3]);
	Assert(vertnum >= 3 && vertnum <= 4);

	// calc rgb
	normDotUp = ONE - VecDotVec(&DownVec, PlaneNormal(&poly->Plane));
	col = (long)(127 * normDotUp);
	if (poly->Type & NON_PLANAR) {
	//	col |= col << 24;
	} else {
		col |= col << 8 | col << 16 | col << 24;
	}
//	rgb[0] = rgb[1] = rgb[2] = rgb[3] = col;

	rgb[0] = 0xff0000;
	rgb[1] = 0x00ff00;
	rgb[2] = 0x0000ff;
	rgb[3] = 0xffff00;

	// draw
	DrawNearClipPolyTEX0(pos, rgb, vertnum);
}

/////////////////////////
// draw a bounding box //
/////////////////////////

void DrawBoundingBox(float xmin, float xmax, float ymin, float ymax, float zmin, float zmax, long c0, long c1, long c2, long c3, long c4, long c5)
{
	VEC v0;
	VEC v1;
	VEC v2;
	VEC v3;
	VEC v4;
	VEC v5;
	VEC v6;
	VEC v7;
	VEC pos[4];
	long col[4];

// no textures

	SET_TPAGE(-1);

// set 8 world points

	SetVector(&v0, xmin, ymin, zmin);
	SetVector(&v1, xmax, ymin, zmin);
	SetVector(&v2, xmin, ymax, zmin);
	SetVector(&v3, xmax, ymax, zmin);
	SetVector(&v4, xmin, ymin, zmax);
	SetVector(&v5, xmax, ymin, zmax);
	SetVector(&v6, xmin, ymax, zmax);
	SetVector(&v7, xmax, ymax, zmax);

// draw xmin

	pos[0] = v4;
	pos[1] = v0;
	pos[2] = v2;
	pos[3] = v6;
	col[0] = col[1] = col[2] = col[3] = c0;
	DrawNearClipPolyTEX0(pos, col, 4);

// draw xmax

	pos[0] = v1;
	pos[1] = v5;
	pos[2] = v7;
	pos[3] = v3;
	col[0] = col[1] = col[2] = col[3] = c1;
	DrawNearClipPolyTEX0(pos, col, 4);

// draw ymin

	pos[0] = v4;
	pos[1] = v5;
	pos[2] = v1;
	pos[3] = v0;
	col[0] = col[1] = col[2] = col[3] = c2;
	DrawNearClipPolyTEX0(pos, col, 4);

// draw ymax

	pos[0] = v2;
	pos[1] = v3;
	pos[2] = v7;
	pos[3] = v6;
	col[0] = col[1] = col[2] = col[3] = c3;
	DrawNearClipPolyTEX0(pos, col, 4);

// draw zmin

	pos[0] = v0;
	pos[1] = v1;
	pos[2] = v3;
	pos[3] = v2;
	col[0] = col[1] = col[2] = col[3] = c4;
	DrawNearClipPolyTEX0(pos, col, 4);

// draw zmax

	pos[0] = v4;
	pos[1] = v5;
	pos[2] = v7;
	pos[3] = v6;
	col[0] = col[1] = col[2] = col[3] = c5;
	DrawNearClipPolyTEX0(pos, col, 4);
}

////////////////////////
// draw world normals //
////////////////////////

void DrawCubeNormals(WORLD_MODEL *m)
{
	long i;
	WORLD_VERTEX *v;
	VEC point1, point2;

// loop thru polys

	v = m->VertPtr;
	for (i = 0 ; i < m->VertNum + m->PolyNum ; i++, v++)
	{
		point1.v[X] = v->x;
		point1.v[Y] = v->y;
		point1.v[Z] = v->z;

		point2.v[X] = point1.v[X] + (v->nx * 64);
		point2.v[Y] = point1.v[Y] + (v->ny * 64);
		point2.v[Z] = point1.v[Z] + (v->nz * 64);

		DrawLine(&point1, &point2, 0xffffff, 0);
	}
}

//////////////////////////////////////////////////////
// Draw all the edges for a list of collision skins //
//////////////////////////////////////////////////////

bool DrawConvex(CONVEX *skin, INDEX count)
{
	long i, j, c0, c1;

// NULL skin ptr?

	if (!skin)
		return FALSE;

// loop thru all skins

	for ( i = 0 ; i < count ; i++)
	{

// good colors please!

		switch (i % 3)
		{
			case 0:
				c0 = 0xff0000;
				c1 = 0xff0000;
				break;
			case 1:
				c0 = 0x00ff00;
				c1 = 0x00ff00;
				break;
			case 2:
				c0 = 0x0000ff;
				c1 = 0x0000ff;
				break;
		}

// draw all edges

		for (j = 0 ; j < skin[i].NEdges ; j++)
		{
			DrawLine(&skin[i].Pts[skin[i].Edges[j].Vtx[0]], &skin[i].Pts[skin[i].Edges[j].Vtx[1]], c0, c1);
		}
	}

// return OK

	return TRUE;
}

//////////////////
// draw 3d line //
//////////////////

void DrawLine(VEC *v0, VEC *v1, long col0, long col1)
{
	float mul;
	long lmul;
	VEC delta;
	MODEL_RGB *rgb0, *rgb1;

// transform to camera space

	RotTransVector(&ViewMatrixScaled, &ViewTransScaled, v0, (VEC*)&DrawVertsTEX0[0].sx);
	RotTransVector(&ViewMatrixScaled, &ViewTransScaled, v1, (VEC*)&DrawVertsTEX0[1].sx);

// clip verts if < 1

	if (DrawVertsTEX0[0].sz < 1)
	{
		if (DrawVertsTEX0[1].sz < 1)
			return;

		SubVector((VEC*)&DrawVertsTEX0[1].sx, (VEC*)&DrawVertsTEX0[0].sx, &delta);
		mul = (1 - DrawVertsTEX0[0].sz) / (DrawVertsTEX0[1].sz - DrawVertsTEX0[0].sz);
		DrawVertsTEX0[0].sx += delta.v[X] * mul;
		DrawVertsTEX0[0].sy += delta.v[Y] * mul;
		DrawVertsTEX0[0].sz += delta.v[Z] * mul;

		FTOL(mul * 256, lmul);
		rgb0 = (MODEL_RGB*)&col0;
		rgb1 = (MODEL_RGB*)&col1;
		rgb0->r = (unsigned char)((rgb0->r * (256 - lmul) + rgb1->r * lmul) >> 8);
		rgb0->g = (unsigned char)((rgb0->g * (256 - lmul) + rgb1->g * lmul) >> 8);
		rgb0->b = (unsigned char)((rgb0->b * (256 - lmul) + rgb1->b * lmul) >> 8);
	}

	else if (DrawVertsTEX0[1].sz < 1)
	{
		SubVector((VEC*)&DrawVertsTEX0[0].sx, (VEC*)&DrawVertsTEX0[1].sx, &delta);
		mul = (1 - DrawVertsTEX0[1].sz) / (DrawVertsTEX0[0].sz - DrawVertsTEX0[1].sz);
		DrawVertsTEX0[1].sx += delta.v[X] * mul;
		DrawVertsTEX0[1].sy += delta.v[Y] * mul;
		DrawVertsTEX0[1].sz += delta.v[Z] * mul;

		FTOL(mul * 256, lmul);
		rgb0 = (MODEL_RGB*)&col1;
		rgb1 = (MODEL_RGB*)&col0;
		rgb0->r = (unsigned char)((rgb0->r * (256 - lmul) + rgb1->r * lmul) >> 8);
		rgb0->g = (unsigned char)((rgb0->g * (256 - lmul) + rgb1->g * lmul) >> 8);
		rgb0->b = (unsigned char)((rgb0->b * (256 - lmul) + rgb1->b * lmul) >> 8);
	}

// perspectify

	DrawVertsTEX0[0].sx = DrawVertsTEX0[0].sx / DrawVertsTEX0[0].sz + RenderSettings.GeomCentreX;
	DrawVertsTEX0[0].sy = DrawVertsTEX0[0].sy / DrawVertsTEX0[0].sz + RenderSettings.GeomCentreY;
	DrawVertsTEX0[0].rhw = 1 / DrawVertsTEX0[0].sz;
	DrawVertsTEX0[0].sz = GET_ZBUFFER(DrawVertsTEX0[0].sz);

	DrawVertsTEX0[1].sx = DrawVertsTEX0[1].sx / DrawVertsTEX0[1].sz + RenderSettings.GeomCentreX;
	DrawVertsTEX0[1].sy = DrawVertsTEX0[1].sy / DrawVertsTEX0[1].sz + RenderSettings.GeomCentreY;
	DrawVertsTEX0[1].rhw = 1 / DrawVertsTEX0[1].sz;
	DrawVertsTEX0[1].sz = GET_ZBUFFER(DrawVertsTEX0[1].sz);

// draw

	DrawVertsTEX0[0].color = col0;
	DrawVertsTEX0[1].color = col1;

	SET_TPAGE(-1);
	D3Ddevice->DrawPrimitive(D3DPT_LINELIST, FVF_TEX0, DrawVertsTEX0, 2, D3DDP_DONOTUPDATEEXTENTS);
}

///////////////////////
// clip and draw tri //
///////////////////////

void DrawTriClip(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, VERTEX_TEX1 *v2)
{

// setup misc

	ClipVertNum = 3;
	ClipVertFree = 3;

	ClipVertList[0][0] = 0;
	ClipVertList[0][1] = 1;
	ClipVertList[0][2] = 2;

	ClipVert[0] = *v0;
	ClipVert[1] = *v1;
	ClipVert[2] = *v2;

// clip

	DrawFanClip();
}

////////////////////////
// clip and draw quad //
////////////////////////

void DrawQuadClip(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, VERTEX_TEX1 *v2, VERTEX_TEX1 *v3)
{

// setup misc

	ClipVertNum = 4;
	ClipVertFree = 4;

	ClipVertList[0][0] = 0;
	ClipVertList[0][1] = 1;
	ClipVertList[0][2] = 2;
	ClipVertList[0][3] = 3;

	ClipVert[0] = *v0;
	ClipVert[1] = *v1;
	ClipVert[2] = *v2;
	ClipVert[3] = *v3;

// clip

	DrawFanClip();
}

//////////////////////////////////
// clip and draw a triangle fan //
//////////////////////////////////

void DrawFanClip(void)
{
	short i, newcount;
	VERTEX_TEX1 *vert0, *vert1;

// top

	for (i = newcount = 0 ; i < ClipVertNum ; i++)
	{
		vert0 = ClipVert + ClipVertList[0][i];
		vert1 = ClipVert + ClipVertList[0][(i + 1) % ClipVertNum];

		if (vert0->sy >= ScreenTopClip)
		{
			ClipVertList[1][newcount++] = ClipVertList[0][i];

			if (vert1->sy < ScreenTopClip)
			{
				ClipLineTEX1(vert0, vert1, (vert0->sy - ScreenTopClip) / (vert0->sy - vert1->sy), ClipVert + ClipVertFree);
				ClipVertList[1][newcount++] = ClipVertFree++;
			}
		}
		else
		{
			if (vert1->sy >= ScreenTopClip)
			{
				ClipLineTEX1(vert1, vert0, (vert1->sy - ScreenTopClip) / (vert1->sy - vert0->sy), ClipVert + ClipVertFree);
				ClipVertList[1][newcount++] = ClipVertFree++;
			}
		}
	}

	ClipVertNum = newcount;

// bottom

	for (i = newcount = 0 ; i < ClipVertNum ; i++)
	{
		vert0 = ClipVert + ClipVertList[1][i];
		vert1 = ClipVert + ClipVertList[1][(i + 1) % ClipVertNum];

		if (vert0->sy <= ScreenBottomClip)
		{
			ClipVertList[0][newcount++] = ClipVertList[1][i];

			if (vert1->sy > ScreenBottomClip)
			{
				ClipLineTEX1(vert0, vert1, (ScreenBottomClip - vert0->sy) / (vert1->sy - vert0->sy), ClipVert + ClipVertFree);
				ClipVertList[0][newcount++] = ClipVertFree++;
			}
		}
		else
		{
			if (vert1->sy <= ScreenBottomClip)
			{
				ClipLineTEX1(vert1, vert0, (ScreenBottomClip - vert1->sy) / (vert0->sy - vert1->sy), ClipVert + ClipVertFree);
				ClipVertList[0][newcount++] = ClipVertFree++;
			}
		}
	}

	ClipVertNum = newcount;

// left

	for (i = newcount = 0 ; i < ClipVertNum ; i++)
	{
		vert0 = ClipVert + ClipVertList[0][i];
		vert1 = ClipVert + ClipVertList[0][(i + 1) % ClipVertNum];

		if (vert0->sx >= ScreenLeftClip)
		{
			ClipVertList[1][newcount++] = ClipVertList[0][i];

			if (vert1->sx < ScreenLeftClip)
			{
				ClipLineTEX1(vert0, vert1, (vert0->sx - ScreenLeftClip) / (vert0->sx - vert1->sx), ClipVert + ClipVertFree);
				ClipVertList[1][newcount++] = ClipVertFree++;
			}
		}
		else
		{
			if (vert1->sx >= ScreenLeftClip)
			{
				ClipLineTEX1(vert1, vert0, (vert1->sx - ScreenLeftClip) / (vert1->sx - vert0->sx), ClipVert + ClipVertFree);
				ClipVertList[1][newcount++] = ClipVertFree++;
			}
		}
	}

	ClipVertNum = newcount;

// right

	for (i = newcount = 0 ; i < ClipVertNum ; i++)
	{
		vert0 = ClipVert + ClipVertList[1][i];
		vert1 = ClipVert + ClipVertList[1][(i + 1) % ClipVertNum];

		if (vert0->sx <= ScreenRightClip)
		{
			ClipVertList[0][newcount++] = ClipVertList[1][i];

			if (vert1->sx > ScreenRightClip)
			{
				ClipLineTEX1(vert0, vert1, (ScreenRightClip - vert0->sx) / (vert1->sx - vert0->sx), ClipVert + ClipVertFree);
				ClipVertList[0][newcount++] = ClipVertFree++;
			}
		}
		else
		{
			if (vert1->sx <= ScreenRightClip)
			{
				ClipLineTEX1(vert1, vert0, (ScreenRightClip - vert1->sx) / (vert0->sx - vert1->sx), ClipVert + ClipVertFree);
				ClipVertList[0][newcount++] = ClipVertFree++;
			}
		}
	}

	ClipVertNum = newcount;

// draw

	D3Ddevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_TLVERTEX, ClipVert, 32, ClipVertList[0], ClipVertNum, D3DDP_DONOTUPDATEEXTENTS | D3DDP_DONOTCLIP);
}

/////////////////
// clip a line //
/////////////////

void ClipLineTEX0(VERTEX_TEX0 *v0, VERTEX_TEX0 *v1, float mul, VERTEX_TEX0 *out)
{
	long lmul;
	float zout;
	MODEL_RGB *rgb0, *rgb1, *rgbout;

// clip xyz rhw

	out->sx = v0->sx + (v1->sx - v0->sx) * mul;
	out->sy = v0->sy + (v1->sy - v0->sy) * mul;
	out->rhw = v0->rhw + (v1->rhw - v0->rhw) * mul;
	zout = 1.0f / out->rhw;
	out->sz = GET_ZBUFFER(zout);

// clip fog + rgb

	FTOL(mul * 256, lmul);

	out->specular = v0->specular + ((((v1->specular >> 24) - (v0->specular >> 24)) * lmul) << 16);

	rgb0 = (MODEL_RGB*)&v0->color;
	rgb1 = (MODEL_RGB*)&v1->color;
	rgbout = (MODEL_RGB*)&out->color;

	rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
	rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
	rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);
}

/////////////////
// clip a line //
/////////////////

void ClipLineTEX1(VERTEX_TEX1 *v0, VERTEX_TEX1 *v1, float mul, VERTEX_TEX1 *out)
{
	long lmul;
	float z0, z1, zout, zmul;
	MODEL_RGB *rgb0, *rgb1, *rgbout;

// clip xyz rhw

	out->sx = v0->sx + (v1->sx - v0->sx) * mul;
	out->sy = v0->sy + (v1->sy - v0->sy) * mul;
	out->rhw = v0->rhw + (v1->rhw - v0->rhw) * mul;
	zout = 1.0f / out->rhw;
	out->sz = GET_ZBUFFER(zout);

// clip uv

	z0 = 1.0f / v0->rhw;
	z1 = 1.0f / v1->rhw;
	zmul = (zout - z0) / (z1 - z0);

	out->tu = v0->tu + (v1->tu - v0->tu) * zmul;
	out->tv = v0->tv + (v1->tv - v0->tv) * zmul;

// clip fog + rgb

	FTOL(mul * 256, lmul);

	out->specular = v0->specular + ((((v1->specular >> 24) - (v0->specular >> 24)) * lmul) << 16);

	rgb0 = (MODEL_RGB*)&v0->color;
	rgb1 = (MODEL_RGB*)&v1->color;
	rgbout = (MODEL_RGB*)&out->color;

	rgbout->r = rgb0->r + (unsigned char)(((rgb1->r - rgb0->r) * lmul) >> 8);
	rgbout->g = rgb0->g + (unsigned char)(((rgb1->g - rgb0->g) * lmul) >> 8);
	rgbout->b = rgb0->b + (unsigned char)(((rgb1->b - rgb0->b) * lmul) >> 8);
}

///////////////////////////
// save out front buffer //
///////////////////////////

void SaveFrontBuffer(char *file)
{
	DDSURFACEDESC2 ddsd;
	BITMAPFILEHEADER bf;
	BITMAPINFOHEADER bi;
	FILE *fp;
	DWORD y, x;
	short *p, r, g, b;
	short outbuf[1600];

// open file

	fp = fopen(file, "wb");
	if (!fp) return;

// lock front buffer

	ddsd.dwSize = sizeof(ddsd);
	while (FrontBuffer->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL) != DD_OK);

// write header

	bf.bfType = MAKEWORD((BYTE)'B',(BYTE)'M');
	bf.bfSize = sizeof(bf) + sizeof(bi) + ScreenXsize * ScreenYsize * sizeof(short);
	bf.bfReserved1 = 0;
	bf.bfReserved2 = 0;
	bf.bfOffBits = sizeof(bf) + sizeof(bi);

	fwrite(&bf, sizeof(bf), 1, fp);

	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = ScreenXsize;
	bi.biHeight = ScreenYsize;
	bi.biPlanes = 1;
	bi.biBitCount = 16;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	fwrite(&bi, sizeof(bi), 1, fp);

// write buffer

	p = (short*)ddsd.lpSurface;
	p += ddsd.lPitch * (ScreenYsize - 1) / 2;

	if (ddsd.ddpfPixelFormat.dwGBitMask & 1024)
	{
		for (y = 0 ; y < ScreenYsize ; y++)
		{
			for (x = 0 ; x < ScreenXsize ; x++)
			{
				b = p[x] & 31;
				g = (p[x] >> 6) & 31;
				r = (p[x] >> 11) & 31;
				outbuf[x] = b | (g << 5) | (r << 10);
			}
			fwrite(outbuf, sizeof(short), ScreenXsize, fp);
			p -= ddsd.lPitch / 2;
		}
	}
	else
	{
		for (y = 0 ; y < ScreenYsize ; y++)
		{
			fwrite(p, sizeof(short), ScreenXsize, fp);
			p -= ddsd.lPitch / 2;
		}
	}

// unlock front buffer

	FrontBuffer->Unlock(NULL);

// close file

	fclose(fp);
}

////////////////////////
// reset mesh fx list //
////////////////////////

void ResetMeshFxList(void)
{
	WorldMeshFxCount = 0;
	ModelMeshFxCount = 0;
}

/////////////////////////
// add mesh special fx //
/////////////////////////
 
void AddWorldMeshFx(void (*checker)(void *data), void *data)
{

// quit if full

	if (WorldMeshFxCount >= MAX_WORLD_MESH_FX)
		return;

// add

	WorldMeshFx[WorldMeshFxCount].Checker = checker;
	WorldMeshFx[WorldMeshFxCount].Data = data;
	WorldMeshFxCount++;
}

/////////////////////////
// add mesh special fx //
/////////////////////////
 
void AddModelMeshFx(void (*checker)(void *data), void *data)
{

// quit if full

	if (ModelMeshFxCount >= MAX_MODEL_MESH_FX)
		return;

// add

	ModelMeshFx[ModelMeshFxCount].Checker = checker;
	ModelMeshFx[ModelMeshFxCount].Data = data;
	ModelMeshFxCount++;
}

/////////////////////////////
// init jump spark offsets //
/////////////////////////////

void InitJumpSparkOffsets(void)
{
	long i;

	for (i = 0 ; i < JUMPSPARK_OFFSET_NUM ; i++)
	{
		SetVector(&JumpSparkOffset[i], frand(4.0f) - 2.0f, frand(4.0f) - 2.0f, frand(4.0f) - 2.0f);
	}
}

/////////////////////
// draw jump spark //
/////////////////////

void DrawJumpSpark(VEC *v1, VEC *v2)
{
	long i, steps, offset;
	VEC delta, pos, vec, start, end, svec;
	REAL len, fsteps, mul, dx1, dy1, dx2, dy2, ang, s, sadd, smul;
	REAL dx[JUMPSPARK_STEP_MAX], dy[JUMPSPARK_STEP_MAX];
	VERTEX_TEX0 points[JUMPSPARK_STEP_MAX + 1];
	VERTEX_TEX1 *verts;
	FACING_POLY poly;
	MAT mat;

// calc steps

	SubVector(v2, v1, &delta);
	len = Length(&delta);
	fsteps = len / JUMPSPARK_STEP_LEN;
	if (fsteps > JUMPSPARK_STEP_MAX)
		fsteps = JUMPSPARK_STEP_MAX;

	FTOL(fsteps, steps);
	fsteps = (float)steps;

// build screen xyz's

	CopyVec(v1, &pos);
	mul = 1.0f / fsteps;
	VecMulScalar(&delta, mul);

	i = TIME2MS(TimerCurrent) / 20;
	if (i != JumpSparkTime)
	{
		JumpSparkTime = i;
		JumpSparkOff = rand() % JUMPSPARK_OFFSET_NUM;
	}

	offset = JumpSparkOff;

	CrossProduct(&delta, &ViewCameraMatrix.mv[U], &vec);
	CrossProduct(&vec, &delta, &svec);
	NormalizeVector(&svec);

	s = (float)TIME2MS(TimerCurrent) / 100.0f;

	i = TIME2MS(TimerCurrent) / 500;
	if (i != JumpSparkSinTime)
	{	
		JumpSparkSinTime = i;
		JumpSparkSinDiv = frand(6.0f) + 9.0f;
	}
	sadd = len / fsteps / JumpSparkSinDiv;

	smul = (float)sin((float)TIME2MS(TimerCurrent) / 300.0f) + 3.0f;

	for (i = 0 ; i < steps + 1; i++)
	{
		CopyVec(&pos, &vec);

//		if (i && i < steps)
		{
			vec.v[X] += JumpSparkOffset[offset].v[X];
			vec.v[Y] += JumpSparkOffset[offset].v[Y];
			vec.v[Z] += JumpSparkOffset[offset].v[Z];

			mul = (float)sin(s) * smul;
			VecPlusEqScalarVec(&vec, mul, &svec);
		}

		if (!i)
			CopyVec(&vec, &start)

		if (i == steps)
			CopyVec(&vec, &end)

		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vec, (float*)&points[i]);
		AddVector(&pos, &delta, &pos);

		offset++;
		offset %= JUMPSPARK_OFFSET_NUM;

		s += sadd;
	}

// calc delta's for each step

	for (i = 0 ; i < steps ; i++)
	{
		dx1 = points[i + 1].sx - points[i].sx;
		dy1 = points[i + 1].sy - points[i].sy;
		mul = 4.0f / (float)sqrt(dx1 * dx1 + dy1 * dy1) * points[i].rhw * RenderSettings.GeomPers;
		dy[i] = dx1 * mul;
		dx[i] = -dy1 * mul;
	}

// create each poly

	for (i = 0 ; i < steps ; i++)
	{

// get semi slot

		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP_ZBIAS(verts, FALSE, 4, TPAGE_FX1, TRUE, 1, -128.0f);

// calc dx1, dy1

		if (!i)
		{
			dx1 = dx[i] * 0.75f;
			dy1 = dy[i] * 0.75f;
		}
		else
		{
			dx1 = dx2;
			dy1 = dy2;
		}

// calc dx2, dy2

		if (i == steps - 1)
		{
			dx2 = dx[i] * 0.75f;
			dy2 = dy[i] * 0.75f;
		}
		else
		{
			dx2 = dx[i] + dx[i + 1];
			dy2 = dy[i] + dy[i + 1];
			mul = 4.0f / (float)sqrt(dx2 * dx2 + dy2 * dy2) * points[i + 1].rhw * RenderSettings.GeomPers;
			dx2 *= mul;
			dy2 *= mul;
		}

// build poly

		verts[0].sx = points[i].sx - dx1;
		verts[0].sy = points[i].sy - dy1;

		verts[1].sx = points[i + 1].sx - dx2;
		verts[1].sy = points[i + 1].sy - dy2;

		verts[2].sx = points[i + 1].sx + dx2;
		verts[2].sy = points[i + 1].sy + dy2;

		verts[3].sx = points[i].sx + dx1;
		verts[3].sy = points[i].sy + dy1;

		verts[0].sz = verts[3].sz = points[i].sz;
		verts[1].sz = verts[2].sz = points[i + 1].sz;
		verts[0].rhw = verts[3].rhw = points[i].rhw;
		verts[1].rhw = verts[2].rhw = points[i + 1].rhw;

		verts[0].tu = verts[3].tu = 216.0f / 256.0f;
		verts[1].tu = verts[2].tu = 223.0f / 256.0f;
		verts[0].tv = verts[1].tv = 33.0f / 256.0f;
		verts[2].tv = verts[3].tv = 47.0f / 256.0f;

		verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xffffff;
	}

// draw 'end' flares

	poly.Xsize = poly.Ysize = (float)(TIME2MS(TimerCurrent) % 100) / 25.0f + 4.0f;
	poly.U = 192.0f / 256.0f;
	poly.V = 64.0f / 256.0f;
	poly.Usize = poly.Vsize = 64.0f / 256.0f;
	poly.Tpage = TPAGE_FX1;
	poly.RGB = 0x8080ff;

	ang = (float)TIME2MS(TimerCurrent) / 10000.0f;

	RotMatrixZ(&mat, ang);
	DrawFacingPolyRot(&start, &mat, &poly, 1, -16.0f);
	DrawFacingPolyRot(&end, &mat, &poly, 1, -16.0f);

	RotMatrixZ(&mat, ang * 2.0f);
	DrawFacingPolyRot(&start, &mat, &poly, 1, -16.0f);
	DrawFacingPolyRot(&end, &mat, &poly, 1, -16.0f);
}

void DrawJumpSpark2(VEC *v1, VEC *v2)
{
	long i, steps, offset;
	VEC delta, pos, vec, start, end, svec;
	REAL len, fsteps, mul, dx1, dy1, dx2, dy2, s, sadd, smul;
	REAL dx[JUMPSPARK_STEP_MAX], dy[JUMPSPARK_STEP_MAX];
	VERTEX_TEX0 points[JUMPSPARK_STEP_MAX + 1];
	VERTEX_TEX1 *verts;

// calc steps

	SubVector(v2, v1, &delta);
	len = Length(&delta);
	fsteps = len / JUMPSPARK_STEP_LEN;
	if (fsteps > JUMPSPARK_STEP_MAX)
		fsteps = JUMPSPARK_STEP_MAX;

	FTOL(fsteps, steps);
	fsteps = (float)steps;

// build screen xyz's

	CopyVec(v1, &pos);
	mul = 1.0f / fsteps;
	VecMulScalar(&delta, mul);

	i = TIME2MS(TimerCurrent) / 20;
	if (i != JumpSparkTime)
	{
		JumpSparkTime = i;
		JumpSparkOff = rand() % JUMPSPARK_OFFSET_NUM;
	}

	offset = JumpSparkOff;

	CrossProduct(&delta, &ViewCameraMatrix.mv[U], &vec);
	CrossProduct(&vec, &delta, &svec);
	NormalizeVector(&svec);

	s = (float)TIME2MS(TimerCurrent) / 100.0f;

	i = TIME2MS(TimerCurrent) / 500;
	if (i != JumpSparkSinTime)
	{	
		JumpSparkSinTime = i;
		JumpSparkSinDiv = frand(6.0f) + 9.0f;
	}
	sadd = len / fsteps / JumpSparkSinDiv;

	smul = (float)sin((float)TIME2MS(TimerCurrent) / 300.0f) + 3.0f;

	for (i = 0 ; i < steps + 1; i++)
	{
		CopyVec(&pos, &vec);

//		if (i && i < steps)
		{
			vec.v[X] += JumpSparkOffset[offset].v[X];
			vec.v[Y] += JumpSparkOffset[offset].v[Y];
			vec.v[Z] += JumpSparkOffset[offset].v[Z];

			mul = (float)sin(s) * smul;
			VecPlusEqScalarVec(&vec, mul, &svec);
		}

		if (!i)
			CopyVec(&vec, &start)

		if (i == steps)
			CopyVec(&vec, &end)

		RotTransPersVector(&ViewMatrixScaled, &ViewTransScaled, &vec, (float*)&points[i]);
		AddVector(&pos, &delta, &pos);

		offset++;
		offset %= JUMPSPARK_OFFSET_NUM;

		s += sadd;
	}

// calc delta's for each step

	for (i = 0 ; i < steps ; i++)
	{
		dx1 = points[i + 1].sx - points[i].sx;
		dy1 = points[i + 1].sy - points[i].sy;
		mul = 4.0f / (float)sqrt(dx1 * dx1 + dy1 * dy1) * points[i].rhw * RenderSettings.GeomPers;
		dy[i] = dx1 * mul;
		dx[i] = -dy1 * mul;
	}

// create each poly

	for (i = 0 ; i < steps ; i++)
	{

// get semi slot

		if (!SEMI_POLY_FREE()) return;
		SEMI_POLY_SETUP_ZBIAS(verts, FALSE, 4, TPAGE_FX1, TRUE, 1, -128.0f);

// calc dx1, dy1

		if (!i)
		{
			dx1 = 0;
			dy1 = 0;
		}
		else
		{
			dx1 = dx2;
			dy1 = dy2;
		}

// calc dx2, dy2

		if (i == steps - 1)
		{
			dx2 = 0;
			dy2 = 0;
		}
		else
		{
			dx2 = dx[i] + dx[i + 1];
			dy2 = dy[i] + dy[i + 1];
			mul = 4.0f / (float)sqrt(dx2 * dx2 + dy2 * dy2) * points[i + 1].rhw * RenderSettings.GeomPers;
			dx2 *= mul;
			dy2 *= mul;
		}

// build poly

		verts[0].sx = points[i].sx - dx1;
		verts[0].sy = points[i].sy - dy1;

		verts[1].sx = points[i + 1].sx - dx2;
		verts[1].sy = points[i + 1].sy - dy2;

		verts[2].sx = points[i + 1].sx + dx2;
		verts[2].sy = points[i + 1].sy + dy2;

		verts[3].sx = points[i].sx + dx1;
		verts[3].sy = points[i].sy + dy1;

		verts[0].sz = verts[3].sz = points[i].sz;
		verts[1].sz = verts[2].sz = points[i + 1].sz;
		verts[0].rhw = verts[3].rhw = points[i].rhw;
		verts[1].rhw = verts[2].rhw = points[i + 1].rhw;

		verts[0].tu = verts[3].tu = 216.0f / 256.0f;
		verts[1].tu = verts[2].tu = 223.0f / 256.0f;
		verts[0].tv = verts[1].tv = 33.0f / 256.0f;
		verts[2].tv = verts[3].tv = 47.0f / 256.0f;

		verts[0].color = verts[1].color = verts[2].color = verts[3].color = 0xffffff;
	}
}
