
#include "revolt.h"
#include "piano.h"
#include "draw.h"
#include "geom.h"
#include "trigger.h"
#include "sfx.h"
#include "ctrlread.h"
#include "object.h"
#include "player.h"

// globals

static float PianoKeys[PIANO_KEY_NUM];
static float BlackPianoKeys[BLACK_PIANO_KEY_NUM];

static long KeyTypeTable[] = {
	1, 0, 0, 2,
	1, 0, 2,
	1, 0, 0, 2,
	1, 0, 2,
	1, 0, 0, 2,
	1, 0, 2,
	1, 0, 0, 2,
	1, 0, 2,
	1, 0, 0, 2,
};

static float KeyTexTable[] = {
	PIANO_TU1, PIANO_TU2, PIANO_TU3,
};

static long BlackKeyNumbers[] = {
	1, 2, 3,
	5, 6,
	8, 9, 10,
	12, 13,
	15, 16, 17,
	19, 20,
	22, 23, 24,
	26, 27,
	29, 30, 31,
};

static long RainbowCols[] = {
	0xff0c07,
	0xff4700,
	0xffe300,
	0x09c31f,
	0x0282ff,
	0x1000b9,
	0xa000e1,
};

static long RgbCols[] = {
	0xff0000,
	0x00ff00,
	0x0000ff,
};

///////////
// piano //
///////////

void TriggerPiano(PLAYER *player, long flag, long n, VEC *vec)
{
	long i, j, key, col, gotblack;
	float x, z, f;
	DRAW_3D_POLY *poly;
	CAR	*car;

	car = &player->car;
	
// zero key count if global first

	if (flag & TRIGGER_GLOBAL_FIRST)
		for (i = 0 ; i < PIANO_KEY_NUM ; i++)
			PianoKeys[i] = 0;

// loop thru each present wheel

	for (i = 0 ; i < CAR_NWHEELS ; i++) if (IsWheelPresent(&car->Wheel[i]))
	{

// in contact?

		if (IsWheelInContact(&car->Wheel[i]))
		{

// yep, check if on piano

			x = car->Wheel[i].WPos.v[X] - PIANO_XPOS;
			z = car->Wheel[i].WPos.v[Z] - PIANO_ZPOS;

			if (x >= 0 && x <= PIANO_WIDTH && z >= 0 && z <= PIANO_DEPTH)
			{

// on piano, set black or white

				gotblack = FALSE;
				if (z > (PIANO_DEPTH - BLACK_PIANO_DEPTH))
				{
					key = (long)((x + PIANO_KEY_WIDTH / 2)  / PIANO_KEY_WIDTH);
					f = x / PIANO_KEY_WIDTH - (float)key;
					if (f > -BLACK_PIANO_PER_WIDTH && f < BLACK_PIANO_PER_WIDTH)
					{
						for (j = 0 ; j < BLACK_PIANO_KEY_NUM ; j++) if (BlackKeyNumbers[j] == key)
						{
							if (!BlackPianoKeys[j]) PlayPianoNote(&car->Wheel[i].WPos, key, TRUE);
							BlackPianoKeys[j] = PIANO_KEY_SET;
							gotblack = TRUE;
							break;
						}
					}
				}

				if (!gotblack)
				{
					key = (long)(x / PIANO_KEY_WIDTH);
					if (!PianoKeys[key]) PlayPianoNote(&car->Wheel[i].WPos, key, FALSE);
					PianoKeys[key] = PIANO_KEY_SET;
				}
			}
		}
	}

// maintain keys if frame first

	if (flag & TRIGGER_FRAME_FIRST)
	{

// white keys

		for (i = 0 ; i < PIANO_KEY_NUM ; i++)
		{

// live key?

			if (PianoKeys[i])
			{

// yep, add poly to render list

				poly = Get3dPoly();
				if (poly)
				{
					x = (float)i;

					FTOL(PianoKeys[i], col);
					col *= 12;
					col <<= 24;
					col |= RainbowCols[i % 7];

					poly->VertNum = 4;
					poly->Tpage = 5;
					poly->Fog = FALSE;
					poly->SemiType = 0;

					poly->Verts[0].color = col;
					poly->Verts[1].color = col;
					poly->Verts[2].color = col;
					poly->Verts[3].color = col;

					poly->Verts[0].tu = poly->Verts[3].tu = KeyTexTable[KeyTypeTable[i]];
					poly->Verts[1].tu = poly->Verts[2].tu = KeyTexTable[KeyTypeTable[i]] + PIANO_TEX_WIDTH;
					poly->Verts[0].tv = poly->Verts[1].tv = PIANO_TV;
					poly->Verts[2].tv = poly->Verts[3].tv = PIANO_TV + PIANO_TEX_HEIGHT;

					poly->Pos[0].v[X] = poly->Pos[3].v[X] = (x * PIANO_KEY_WIDTH) + PIANO_XPOS;
					poly->Pos[1].v[X] = poly->Pos[2].v[X] = ((x + 1) * PIANO_KEY_WIDTH) + PIANO_XPOS;

					poly->Pos[0].v[Y] = poly->Pos[1].v[Y] = poly->Pos[2].v[Y] = poly->Pos[3].v[Y] = PIANO_YPOS;

					poly->Pos[0].v[Z] = poly->Pos[1].v[Z] = PIANO_ZPOS;
					poly->Pos[2].v[Z] = poly->Pos[3].v[Z] = PIANO_ZPOS + PIANO_DEPTH;

					if (!i)
					{
						poly->Pos[0].v[X] += PIANO_KEY_WIDTH / 2;
						poly->Pos[3].v[X] += PIANO_KEY_WIDTH / 2;
						poly->Verts[0].tu += PIANO_TEX_WIDTH / 2;
						poly->Verts[3].tu += PIANO_TEX_WIDTH / 2;
					}
				}

// dec count

				PianoKeys[i] -= TimeFactor;
				if (PianoKeys[i] < 0) PianoKeys[i] = 0;
			}
		}

// black keys

		for (i = 0 ; i < BLACK_PIANO_KEY_NUM ; i++)
		{

// live key?

			if (BlackPianoKeys[i])
			{

// yep, add poly to render list

				poly = Get3dPoly();
				if (poly)
				{
					x = (float)(BlackKeyNumbers[i] - 1) * PIANO_KEY_WIDTH + PIANO_XPOS + BLACK_PIANO_XOFFSET;

					FTOL(BlackPianoKeys[i], col);
					col *= 12;
					col <<= 24;
					col |= RgbCols[i % 3];

					poly->VertNum = 4;
					poly->Tpage = 5;
					poly->Fog = FALSE;
					poly->SemiType = 0;

					poly->Verts[0].color = col;
					poly->Verts[1].color = col;
					poly->Verts[2].color = col;
					poly->Verts[3].color = col;

					poly->Verts[0].tu = poly->Verts[3].tu = BLACK_PIANO_TU;
					poly->Verts[1].tu = poly->Verts[2].tu = BLACK_PIANO_TU + BLACK_PIANO_TEX_WIDTH;
					poly->Verts[0].tv = poly->Verts[1].tv = BLACK_PIANO_TV;
					poly->Verts[2].tv = poly->Verts[3].tv = BLACK_PIANO_TV + BLACK_PIANO_TEX_HEIGHT;

					poly->Pos[0].v[X] = poly->Pos[3].v[X] = x;
					poly->Pos[1].v[X] = poly->Pos[2].v[X] = x + BLACK_PIANO_WIDTH;

					poly->Pos[0].v[Y] = poly->Pos[1].v[Y] = poly->Pos[2].v[Y] = poly->Pos[3].v[Y] = PIANO_YPOS;

					poly->Pos[0].v[Z] = poly->Pos[1].v[Z] = PIANO_ZPOS + (PIANO_DEPTH - BLACK_PIANO_DEPTH);
					poly->Pos[2].v[Z] = poly->Pos[3].v[Z] = PIANO_ZPOS + PIANO_DEPTH;
				}

// dec count

				BlackPianoKeys[i] -= TimeFactor;
				if (BlackPianoKeys[i] < 0) BlackPianoKeys[i] = 0;
			}
		}
	}
}

///////////////////////
// play a piano note //
///////////////////////

void PlayPianoNote(VEC *pos, long key, long black)
{
	long c;

// calc semitone number

	c = 0;
	while (BlackKeyNumbers[c] <= key) c++;

	c += key;
	c -= black;

// play

	PlaySfx3D(SFX_TOY_PIANO, SFX_MAX_VOL, c * 800 + 12000, pos);
}
