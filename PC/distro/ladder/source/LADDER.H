
#ifndef LADDER_H
#define LADDER_H

// macros

#define POLY_QUAD 1

#define LIGHT_FIXED 1
#define LIGHT_MOVING 2
#define LIGHT_FILE 4

#define LIGHT_OMNI 0
#define LIGHT_OMNINORMAL 1
#define LIGHT_SPOT 2
#define LIGHT_SPOTNORMAL 3

#define DotProduct(a, b) \
	((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

typedef struct {
	unsigned char r, g, b, a;
} MODEL_RGB;

typedef struct {
	short x, y, z;
	short nx, ny, nz;
} WORLD_VERTEX;

typedef struct {
	short Type, Tpage;
	short vi[4];
	long rgb[4];
	unsigned char u0, v0, u1, v1, u2, v2, u3, v3;
} WORLD_POLY;

typedef struct {
	short CentreX, CentreY, CentreZ, Radius;
	short PolyNum, VertNum;
} CUBE_HEADER;

typedef struct {
	long CubeNum;
} WORLD_HEADER;

typedef struct {
	long LightNum;
} LIGHT_HEADER;

typedef struct {
	float x, y, z, Reach;
	float DirMatrix[9], Cone;
	float r, g, b;
	char Flag, Type, pad1, pad2;
} LIGHT;

typedef struct {
	short x, y, z, Reach;
	short dx, dy, dz, Cone;
	unsigned char r, g, b, Type;
} PSX_LIGHT;

// prototypes

void main(int argc, char *argv[]);
CUBE_HEADER *AddCubeLightOmni(CUBE_HEADER *ch, LIGHT *light);
CUBE_HEADER *AddCubeLightOmniNormal(CUBE_HEADER *ch, LIGHT *light);
CUBE_HEADER *AddCubeLightSpot(CUBE_HEADER *ch, LIGHT *light);
CUBE_HEADER *AddCubeLightSpotNormal(CUBE_HEADER *ch, LIGHT *light);

#endif
