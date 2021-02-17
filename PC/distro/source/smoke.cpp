
#include "revolt.h"
#include "main.h"
#include "geom.h"
#include "dx.h"
#include "texture.h"
#include "model.h"
#include "smoke.h"

// globals

static long UsedSmokeCount;
static SMOKE *SmokeList;
static SMOKE *UsedSmokeHead;
static SMOKE *FreeSmokeHead;

///////////////////////
// init smoke engine //
///////////////////////

long InitSmoke(void)
{
	long i;

// alloc list memory

	SmokeList = (SMOKE*)malloc(sizeof(SMOKE) * MAX_SMOKES);
	if (!SmokeList)
	{
		Box("ERROR", "Can't alloc memory for smoke engine", MB_OK);
		QuitGame = TRUE;
		return FALSE;
	}

// NULL used head

	UsedSmokeHead = NULL;

// link free list

	FreeSmokeHead = SmokeList;

	for (i = 0 ; i < MAX_SMOKES ; i++)
	{
		SmokeList[i].Prev = &SmokeList[i - 1];
		SmokeList[i].Next = &SmokeList[i + 1];
	}

// NULL first->Prev & last->Next

	SmokeList[0].Prev = NULL;
	SmokeList[MAX_SMOKES - 1].Next = NULL;

// zero count

	UsedSmokeCount = 0;

// return OK

	return TRUE;
}

///////////////////////
// kill smoke engine //
///////////////////////

void KillSmoke(void)
{
	free(SmokeList);
}

///////////////////
// alloc a smoke //
///////////////////

SMOKE *AllocSmoke(void)
{
	SMOKE *smoke;

// return NULL if none free

	if (!FreeSmokeHead)
	{
		return NULL;
	}

// remove our smoke from free list

	smoke = FreeSmokeHead;
	FreeSmokeHead = smoke->Next;

	if (FreeSmokeHead)
	{
		FreeSmokeHead->Prev = NULL;
	}
	
// add to used list

	smoke->Prev = NULL;
	smoke->Next = UsedSmokeHead;
	UsedSmokeHead = smoke;

	if (smoke->Next)
	{
		smoke->Next->Prev = smoke;
	}

// inc used count

	UsedSmokeCount++;

// return our smoke

	return smoke;
}

//////////////////
// free a smoke //
//////////////////

void FreeSmoke(SMOKE *smoke)
{

// return if NULL ptr

	if (!smoke)
	{
		return;
	}

// remove our smoke from used list

	if (smoke->Prev)
	{
		smoke->Prev->Next = smoke->Next;
	}
	else
	{
		UsedSmokeHead = smoke->Next;
	}

	if (smoke->Next)
	{
		smoke->Next->Prev = smoke->Prev;
	}

// add to free list

	smoke->Prev = NULL;
	smoke->Next = FreeSmokeHead;
	FreeSmokeHead = smoke;

	if (smoke->Next)
	{
		smoke->Next->Prev = smoke;
	}

// dec used count

	UsedSmokeCount--;
}

////////////////////
// create a smoke //
////////////////////

long CreateSmoke(VEC *pos, VEC *dir, REAL size, REAL growrate, REAL age, REAL spinstart, REAL spinrate, long rgb)
{
	SMOKE *smoke;

// alloc a smoke

	smoke = AllocSmoke();
	if (!smoke)
	{
		return FALSE;
	}

// setup smoke

	smoke->Pos = *pos;
	smoke->Dir = *dir;
	smoke->GrowRate = growrate;
	smoke->AgeStart = smoke->Age = age;
	smoke->Spin = spinstart;
	smoke->SpinRate = spinrate;

// setup poly

	smoke->Poly.Xsize = size;
	smoke->Poly.Ysize = size;
	smoke->Poly.U = 0.0f / 256.0f;
	smoke->Poly.V = 0.0f / 256.0f;
	smoke->Poly.Usize = 64.0f / 256.0f;
	smoke->Poly.Vsize = 64.0f / 256.0f;
	smoke->Poly.Tpage = TPAGE_FX1;
	smoke->rgb = rgb;

// return OK

	return TRUE;
}

////////////////////
// process smokes //
////////////////////

void ProcessSmoke(void)
{
	long per;
	SMOKE *smoke;
	SMOKE smoketemp;

// quit if none

	if (!UsedSmokeCount)
		return;

// loop thru all smokes

	for (smoke = UsedSmokeHead ; smoke ; smoke = smoke->Next)
	{

// dec age

		smoke->Age -= TimeFactor;
		if (smoke->Age < 0)
		{
			smoketemp.Next = smoke->Next;
			FreeSmoke(smoke);
			smoke = &smoketemp;
			continue;
		}

// rotate

		smoke->Spin += smoke->SpinRate * TimeFactor;
		RotMatrixZ(&smoke->Matrix, smoke->Spin);

// move

		smoke->Pos.v[X] += smoke->Dir.v[X] * TimeFactor;
		smoke->Pos.v[Y] += smoke->Dir.v[Y] * TimeFactor;
		smoke->Pos.v[Z] += smoke->Dir.v[Z] * TimeFactor;

// stretch

		smoke->Poly.Xsize += smoke->GrowRate * TimeFactor;
		smoke->Poly.Ysize += smoke->GrowRate * TimeFactor;

// rgb

		FTOL(smoke->Age * 100 / smoke->AgeStart, per);
		smoke->Poly.RGB = smoke->rgb;
		ModelChangeGouraud((MODEL_RGB*)&smoke->Poly.RGB, per);
	}
}

/////////////////
// draw smokes //
/////////////////

void DrawSmoke(void)
{
	SMOKE *smoke;

// quit if none

	if (!UsedSmokeCount)
		return;

// alpha on, z write off

	ALPHA_ON();
	ALPHA_SRC(D3DBLEND_ONE);
	ALPHA_DEST(D3DBLEND_ONE);

	ZWRITE_OFF();

// loop thru all smokes

	for (smoke = UsedSmokeHead ; smoke ; smoke = smoke->Next)
	{
		DrawFacingPolyRot(&smoke->Pos, &smoke->Matrix, &smoke->Poly, -1, 0.0f);
	}
}
