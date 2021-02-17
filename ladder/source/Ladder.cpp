
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ladder.h"

// globals

FILE *wfile, *lfile;
long wsize, lsize;
HLOCAL whandle, lhandle;
WORLD_HEADER *World;
LIGHT_HEADER *Lights;
float LightDelta[3];
float Percent = 100;

///////////////////////
// main - load files //
///////////////////////

void main(int argc, char *argv[])
{
	CUBE_HEADER *ch;
	LIGHT *light;
	PSX_LIGHT pl;
	long i, j;

// hello

	printf("\nLadder...\n\n");

// correct arg num?

	if (argc < 3 || argc > 4)
	{
		printf("Usage: Ladder cubefile lightfile percent\n");
		exit(1);
	}

// store percent?

	if (argc == 4)
	{
		Percent = (float)atol(argv[3]);
	}

// open files

	wfile = fopen(argv[1], "rb");
	if (!wfile)
	{
		printf("Can't find '%s'\n", argv[1]);
		exit(1);
	}

	lfile = fopen(argv[2], "rb");
	if (!lfile)
	{
		printf("Can't find '%s'\n", argv[2]);
		fclose(wfile);
		exit(1);
	}

// alloc mem, read files, close files

	fseek(wfile, 0, SEEK_END);
	wsize = ftell(wfile);
	rewind(wfile);

	whandle = LocalAlloc(LMEM_FIXED, wsize);
	World = (WORLD_HEADER*)LocalLock(whandle);
	fread(World, wsize, 1, wfile);
	fclose(wfile);

	fseek(lfile, 0, SEEK_END);
	lsize = ftell(lfile);
	rewind(lfile);

	lhandle = LocalAlloc(LMEM_FIXED, lsize);
	Lights = (LIGHT_HEADER*)LocalLock(lhandle);
	fread(Lights, lsize, 1, lfile);
	fclose(lfile);

// print misc info

	printf("%ld Cubes\n", World->CubeNum);
	printf("%ld Lights\n", Lights->LightNum);

// add each light to all cubes

	light = (LIGHT*)(Lights + 1);
	for (i = 0 ; i < Lights->LightNum ; i++, light++)
	{
		light->r = light->r * Percent / 100;
		light->g = light->g * Percent / 100;
		light->b = light->b * Percent / 100;

		ch = (CUBE_HEADER*)(World + 1);
		for (j = 0 ; j < World->CubeNum ; j++)
		{
			switch (light->Type)
			{
				case LIGHT_OMNI:
					ch = AddCubeLightOmni(ch, light);
					break;
				case LIGHT_OMNINORMAL:
					ch = AddCubeLightOmniNormal(ch, light);
					break;
				case LIGHT_SPOT:
					ch = AddCubeLightSpot(ch, light);
					break;
				case LIGHT_SPOTNORMAL:
					ch = AddCubeLightSpotNormal(ch, light);
					break;
			}
		}
	}

// write out world file then lights

	wfile = fopen(argv[1], "wb");
	fwrite(World, wsize, 1, wfile);

	fwrite(Lights, sizeof(long), 1, wfile);

	light = (LIGHT*)(Lights + 1);
	for (i = 0 ; i < Lights->LightNum ; i++, light++)
	{
		if (!(light->Flag & LIGHT_MOVING))
			continue;

		pl.x = (short)light->x;
		pl.y = (short)light->y;
		pl.z = (short)light->z;

		pl.Reach = (short)light->Reach;

		pl.dx = (short)(light->DirMatrix[6] * 4096);
		pl.dy = (short)(light->DirMatrix[7] * 4096);
		pl.dz = (short)(light->DirMatrix[8] * 4096);

		pl.Cone = (short)light->Cone;

		pl.r = (unsigned char)light->r;
		pl.g = (unsigned char)light->g;
		pl.b = (unsigned char)light->b;

		pl.Type = light->Type;

		fwrite(&pl, sizeof(pl), 1, wfile);
	}

	fclose(wfile);

// free, exit

	LocalFree(whandle);
	LocalFree(lhandle);

	exit(0);
}

/////////////////////////
// add light to a cube //
/////////////////////////

CUBE_HEADER *AddCubeLightOmni(CUBE_HEADER *ch, LIGHT *light)
{
	WORLD_VERTEX *wv;
	WORLD_POLY *wp;
	long i, j;
	float squaredist, squarereach, mul;
	MODEL_RGB *col;
	short r, g, b;

// loop thru polys

	squarereach = light->Reach * light->Reach;
	wp = (WORLD_POLY*)(ch + 1);
	wv = (WORLD_VERTEX*)(wp + ch->PolyNum);

	for (i = 0 ; i < ch->PolyNum ; i++, wp++)
	{

// loop thru poly verts

		col = (MODEL_RGB*)wp->rgb;
		for (j = 0 ; j < 3 + (wp->Type & POLY_QUAD) ; j++, col++)
		{
			LightDelta[0] = light->x - (wv + wp->vi[j])->x;
			LightDelta[1] = light->y - (wv + wp->vi[j])->y;
			LightDelta[2] = light->z - (wv + wp->vi[j])->z;

			squaredist = DotProduct(LightDelta, LightDelta);
			if (squaredist < squarereach)
			{
				mul = 1 - squaredist / squarereach;
		
				r = (short)(light->r * mul);
				g = (short)(light->g * mul);
				b = (short)(light->b * mul);

				if (col->r + r > 255) col->r = 255;
				else col->r += r;
				if (col->g + g > 255) col->g = 255;
				else col->g += g;
				if (col->b + b > 255) col->b = 255;
				else col->b += b;
			}
		}
	}

// return next cube

	return (CUBE_HEADER*)(wv + ch->VertNum);
}

/////////////////////////
// add light to a cube //
/////////////////////////

CUBE_HEADER *AddCubeLightOmniNormal(CUBE_HEADER *ch, LIGHT *light)
{
	WORLD_VERTEX *wv;
	WORLD_POLY *wp;
	long i, j;
	float dist, squaredist, squarereach, mul, ang, norm[3];
	MODEL_RGB *col;
	short r, g, b;

// loop thru polys

	squarereach = light->Reach * light->Reach;
	wp = (WORLD_POLY*)(ch + 1);
	wv = (WORLD_VERTEX*)(wp + ch->PolyNum);

	for (i = 0 ; i < ch->PolyNum ; i++, wp++)
	{

// loop thru poly verts

		col = (MODEL_RGB*)wp->rgb;
		for (j = 0 ; j < 3 + (wp->Type & POLY_QUAD) ; j++, col++)
		{
			LightDelta[0] = light->x - (float)(wv + wp->vi[j])->x;
			LightDelta[1] = light->y - (float)(wv + wp->vi[j])->y;
			LightDelta[2] = light->z - (float)(wv + wp->vi[j])->z;

			norm[0] = (float)(wv + wp->vi[j])->nx / 4096;
			norm[1] = (float)(wv + wp->vi[j])->ny / 4096;
			norm[2] = (float)(wv + wp->vi[j])->nz / 4096;

			ang = DotProduct(LightDelta, norm);
			if (ang > 0)
			{
				squaredist = DotProduct(LightDelta, LightDelta);
				if (squaredist < squarereach)
				{
					dist = (float)sqrt(squaredist);
					mul = (1 - squaredist / squarereach) * (ang / dist);
		
					r = (short)(light->r * mul);
					g = (short)(light->g * mul);
					b = (short)(light->b * mul);

					if (col->r + r > 255) col->r = 255;
					else col->r += r;
					if (col->g + g > 255) col->g = 255;
					else col->g += g;
					if (col->b + b > 255) col->b = 255;
					else col->b += b;
				}
			}
		}
	}

// return next cube

	return (CUBE_HEADER*)(wv + ch->VertNum);
}

/////////////////////////
// add light to a cube //
/////////////////////////

CUBE_HEADER *AddCubeLightSpot(CUBE_HEADER *ch, LIGHT *light)
{
	WORLD_VERTEX *wv;
	WORLD_POLY *wp;
	long i, j;
	float dist, squaredist, squarereach, mul, cone;
	MODEL_RGB *col;
	short r, g, b;

// loop thru polys

	squarereach = light->Reach * light->Reach;
	wp = (WORLD_POLY*)(ch + 1);
	wv = (WORLD_VERTEX*)(wp + ch->PolyNum);

	for (i = 0 ; i < ch->PolyNum ; i++, wp++)
	{

// loop thru poly verts

		col = (MODEL_RGB*)wp->rgb;
		for (j = 0 ; j < 3 + (wp->Type & POLY_QUAD) ; j++, col++)
		{
			LightDelta[0] = light->x - (wv + wp->vi[j])->x;
			LightDelta[1] = light->y - (wv + wp->vi[j])->y;
			LightDelta[2] = light->z - (wv + wp->vi[j])->z;

			squaredist = DotProduct(LightDelta, LightDelta);
			if (squaredist < squarereach)
			{
				dist = (float)sqrt(squaredist);
				cone = (-DotProduct(&light->DirMatrix[6], LightDelta) / dist - 1) * (180 / light->Cone) + 1;
				if (cone > 0)
				{
					mul = (1 - squaredist / squarereach) * cone;

					r = (short)(light->r * mul);
					g = (short)(light->g * mul);
					b = (short)(light->b * mul);

					if (col->r + r > 255) col->r = 255;
					else col->r += r;
					if (col->g + g > 255) col->g = 255;
					else col->g += g;
					if (col->b + b > 255) col->b = 255;
					else col->b += b;
				}
			}
		}
	}

// return next cube

	return (CUBE_HEADER*)(wv + ch->VertNum);
}

/////////////////////////
// add light to a cube //
/////////////////////////

CUBE_HEADER *AddCubeLightSpotNormal(CUBE_HEADER *ch, LIGHT *light)
{
	WORLD_VERTEX *wv;
	WORLD_POLY *wp;
	long i, j;
	float norm[3], dist, squaredist, squarereach, mul, ang, cone;
	MODEL_RGB *col;
	short r, g, b;

// loop thru polys

	squarereach = light->Reach * light->Reach;
	wp = (WORLD_POLY*)(ch + 1);
	wv = (WORLD_VERTEX*)(wp + ch->PolyNum);

	for (i = 0 ; i < ch->PolyNum ; i++, wp++)
	{

// loop thru poly verts

		col = (MODEL_RGB*)wp->rgb;
		for (j = 0 ; j < 3 + (wp->Type & POLY_QUAD) ; j++, col++)
		{
			LightDelta[0] = light->x - (wv + wp->vi[j])->x;
			LightDelta[1] = light->y - (wv + wp->vi[j])->y;
			LightDelta[2] = light->z - (wv + wp->vi[j])->z;

			norm[0] = (float)(wv + wp->vi[j])->nx / 4096;
			norm[1] = (float)(wv + wp->vi[j])->ny / 4096;
			norm[2] = (float)(wv + wp->vi[j])->nz / 4096;

			ang = DotProduct(LightDelta, norm);
			if (ang > 0)
			{
				squaredist = DotProduct(LightDelta, LightDelta);
				if (squaredist < squarereach)
				{
					dist = (float)sqrt(squaredist);
					cone = (-DotProduct(&light->DirMatrix[6], LightDelta) / dist - 1) * (180 / light->Cone) + 1;
					if (cone > 0)
					{
						mul = (1 - squaredist / squarereach) * (ang / dist) * cone;

						r = (short)(light->r * mul);
						g = (short)(light->g * mul);
						b = (short)(light->b * mul);

						if (col->r + r > 255) col->r = 255;
						else col->r += r;
						if (col->g + g > 255) col->g = 255;
						else col->g += g;
						if (col->b + b > 255) col->b = 255;
						else col->b += b;
					}
				}
			}
		}
	}

// return next cube

	return (CUBE_HEADER*)(wv + ch->VertNum);
}
