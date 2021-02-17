
// macros
#define _RCONV

#define MAX_PLANAR_TOLERANCE 0.99984769515f

#define MAX_TEXANIMS		16

#define N64_CRAP			FALSE
#define N64_LOG				FALSE

#define TYPE_QUAD			1
#define TYPE_DSIDED			2
#define TYPE_SEMITRANS		4
#define TYPE_TEXTURED		8
#define TYPE_GOURAUD		16
#define TYPE_PSXMODEL1		32
#define TYPE_PSXMODEL2		64
#define TYPE_MIRROR			128
#define TYPE_SEMITRANS_ONE	256
#define TYPE_TEXANIM		512
#define TYPE_NOENV			1024
#define TYPE_ENV			2048

#define COLLTYPE_QUAD		1
#define COLLTYPE_DSIDED		2
#define COLLTYPE_OBJECTONLY	4
#define	COLLTYPE_CAMERAONLY	8

#define PSX_GROUP_SHIFT		5

#define MAX_EDIT_PORTALS	1024

enum {
	PORTAL_AXIS_XY,
	PORTAL_AXIS_XZ,
	PORTAL_AXIS_ZY,
	PORTAL_AXIS_X,
	PORTAL_AXIS_Y,
	PORTAL_AXIS_Z,
};

enum VectorElements {X, Y, Z};
enum MatrixElements {
	XX, XY, XZ, 
	YX, YY, YZ,
	ZX, ZY, ZZ,
	RX=0, RY, RZ,
	UX, UY, UZ,
	LX, LY, LZ
};
enum MatrixVectors {R, U, L};
enum PlaneElements {A, B, C, D};

//#define GRID_EXPAND	150.0f
										   
typedef float REAL;
typedef short INDEX;

enum {
	LIGHT_OMNI,
	LIGHT_OMNINORMAL,
	LIGHT_SPOT,
	LIGHT_SPOTNORMAL,
};

#define AddVector(a, b, c) \
	(c)->x = (a)->x + (b)->x; \
	(c)->y = (a)->y + (b)->y; \
	(c)->z = (a)->z + (b)->z;

#define SetVector(_v, _x, _y, _z) \
{ \
	(_v)->x = _x; \
	(_v)->y = _y; \
	(_v)->z = _z; \
}

#define VecDotPlane(vec, plane) \
	( (vec)->x * (plane)->a + (vec)->y * (plane)->b + (vec)->z * (plane)->c + (plane)->d )

#define VecMinusVec(a, b, c) \
	(c)->x = (a)->x - (b)->x; \
	(c)->y = (a)->y - (b)->y; \
	(c)->z = (a)->z - (b)->z; \

#define VecLen(a) \
	((float)sqrt((a)->x * (a)->x + (a)->y * (a)->y + (a)->z * (a)->z))

#define VecPlusScalarVec(a, t, b, c) \
	(c)->x = (a)->x + (t) * (b)->x; \
	(c)->y = (a)->y + (t) * (b)->y; \
	(c)->z = (a)->z + (t) * (b)->z; 

#define SubVector(a, b, c) \
	(c)[0] = (a)[0] - (b)[0]; \
	(c)[1] = (a)[1] - (b)[1]; \
	(c)[2] = (a)[2] - (b)[2];

#define CrossProduct(a, b, c) \
	(c)[0] = (a)[1] * (b)[2] - (a)[2] * (b)[1]; \
	(c)[1] = (a)[2] * (b)[0] - (a)[0] * (b)[2]; \
	(c)[2] = (a)[0] * (b)[1] - (a)[1] * (b)[0];

#define CrossProduct3(a, b, c) \
	((c)[0] * ((a)[1] * (b)[2] - (a)[2] * (b)[1]) + \
	(c)[1] * ((a)[2] * (b)[0] - (a)[0] * (b)[2]) + \
	(c)[2] * ((a)[0] * (b)[1] - (a)[1] * (b)[0]))

#define DotProduct(a, b) \
	((a)[0] * (b)[0] + (a)[1] * (b)[1] + (a)[2] * (b)[2])

#define SwapRGB(rgb) (rgb = (rgb & 0x00ff00) | ((rgb & 0xff0000) >> 16) | ((rgb & 0x0000ff) << 16))

#define HalfGouraud(rgb) \
	{ \
	(rgb)->r = (char)(((rgb)->r + 1) >> 1); \
	(rgb)->g = (char)(((rgb)->g + 1) >> 1); \
	(rgb)->b = (char)(((rgb)->b + 1) >> 1); \
	}

#define NormalizeVector(_v) \
{ \
	REAL _mul = 1 / (REAL)sqrt((_v)[0] * (_v)[0] + (_v)[1] * (_v)[1] + (_v)[2] * (_v)[2]); \
	(_v)[0] *= _mul; \
	(_v)[1] *= _mul; \
	(_v)[2] *= _mul; \
}

#define ModelAddGouraud(_a, _b, _c) \
{ \
	long _i; \
	_i = (_b)[0] + (long)(_a)->r; \
	if (_i > 255) (_c)->r = 255; \
	else if (_i < 0) (_c)->r = 0; \
	else (_c)->r = (unsigned char)_i; \
	_i = (_b)[1] + (long)(_a)->g; \
	if (_i > 255) (_c)->g = 255; \
	else if (_i < 0) (_c)->g = 0; \
	else (_c)->g = (unsigned char)_i; \
	_i = (_b)[2] + (long)(_a)->b; \
	if (_i > 255) (_c)->b = 255; \
	else if (_i < 0) (_c)->b = 0; \
	else (_c)->b = (unsigned char)_i; \
}

#define ModelChangeGouraud(c, p) \
{ \
	if (p != 100) \
	{ \
		(c)->r = (unsigned char)(((c)->r + 1) * p / 100); \
		(c)->g = (unsigned char)(((c)->g + 1) * p / 100); \
		(c)->b = (unsigned char)(((c)->b + 1) * p / 100); \
	} \
}

#define SWAP_ENDIAN(n) \
	(((n) >> 24) | \
	(((n) >> 8) & 0x0000ff00) | \
	(((n) << 8) & 0x00ff0000) | \
	((n) << 24))

typedef struct {
	unsigned char r, g, b;
} RGB;

typedef struct {
	float u, v;
} UV;

typedef struct {
	char u, v;
} UV_PSX;

typedef struct {									// !MT! Added for N64
	short u, v;
} UV_N64;

typedef struct {
	float x, y, z;
	long Smooth;
	float nx, ny, nz;
	unsigned char r, g, b, a;
} VERTEX;

typedef struct {
	short x, y, z;
} VERTEX_PSX;

typedef struct {
	short Type, Tpage;
	short vi[4];
	U32 rgb[4];
	UV uv[4];
	VERTEX v[4];
	long CubeX, CubeY, CubeZ, CubeNum;
	long Material, EnvRGB;
	float nx, ny, nz;
	long Group;
	long Done;										// !MT! Used for marking polys during processing
	long Tmem;										// !MT! Index into TMEM list
	long CacheIdx[4];								// !MT! Reverse reference for poly ref from vertex cache
	UV_N64 uv64[4];									// !MT! Final UV coords needed for vertex modification		
	long Region;
} POLY;

typedef struct {
	float x, y, z;
} VECTOR;

typedef struct {
	float a, b, c, d;
} PLANE;

typedef struct {
	float x, y, z, Radius;
	long BigCube, Cube;
} CUBE_INFO;

typedef struct {
	long Count, Temp;
	POLY *PolyPtr;
} CUBELIST;

typedef struct {
	long Count;
} BIGCUBELIST;

typedef struct {
	REAL	XMin, XMax;
	REAL	YMin, YMax;
	REAL	ZMin, ZMax;
} BBOX;

typedef struct {
	long	Type;
	long	Material;
	
	PLANE	Plane;
	PLANE	EdgePlane[4];
	BBOX	BBox;
} NEWCOLLPOLY;

typedef struct {
	long a, b, c, d;
} PLANE_PSX;

typedef struct {
	long	XMin, XMax;
	long	YMin, YMax;
	long	ZMin, ZMax;
} BBOX_PSX;

typedef struct {
	long	Type;
	long	Material;
	
	PLANE_PSX	Plane;
	PLANE_PSX	EdgePlane[4];
	BBOX_PSX	BBox;
} NEWCOLLPOLY_PSX;

typedef struct {
	INDEX	Vtx[2];
} EDGE;

typedef struct {
	INDEX	NSkins;
} COLLSKIN_FILEHDR;

typedef struct {
	INDEX	NPts;
	INDEX	NEdges;
	INDEX	NFaces;

	BBOX	BBox;
	VECTOR	Offset;
	
	VECTOR	*Pts;
	EDGE	*Edges;
	PLANE	*Faces;
} COLLSKIN;

typedef union {
	REAL m[9];
	VECTOR mv[3];
	struct {
		REAL r[3];
		REAL u[3];
		REAL l[3];
	} row;
} MATRIX;

typedef struct {
	unsigned char b, g, r, a;
} MODEL_RGB;

typedef struct {
	float x, y, z, Reach;
	MATRIX DirMatrix;
	float Cone;
	float r, g, b;
	unsigned char Flag, Type, Speed, pad2;
} FILELIGHT;

typedef struct {
	short x, y, z, Reach;
	short dx, dy, dz, Cone;
	short r, g, b;
	unsigned char Flag, Type;
} FILELIGHT_PSX;

typedef struct {
	char Flag, ID, pad[2];
	float xmin, xmax;
	float ymin, ymax;
	float zmin, zmax;
} VISIBOX;

typedef struct {
	char Flag, ID;
	short xmin, xmax;
	short ymin, ymax;
	short zmin, zmax;
} VISIBOX_PSX;

typedef struct {
	float Xstart, Zstart;
	float Xnum, Znum;
	float GridSize;
} GRID_HEADER;

typedef struct {
	long Xstart, Zstart;
	long Xnum, Znum;
	long GridSize;
} GRID_HEADER_PSX;

typedef struct {
	long Tpage;
	float Time;
	UV uv[4];
} TEXANIM_FRAME;

typedef struct {
	HLOCAL Handle;
	TEXANIM_FRAME *Frame;
	long ID, FrameNum;
} TEXANIM_HEADER;

typedef struct {
	long data[4];
} MEM16;

typedef struct {
	long data[6];
} MEM24;

typedef struct {
	long data[8];
} MEM32;

typedef struct {
	long Region;
	long ID1, ID2;
	VECTOR Pos;
	MATRIX Matrix;
	float Size[3];
	long Flag[4];
} EDIT_PORTAL;

// prototypes

void CountObjectFaces(QOBJECT *obj);
void ProcessObject(QOBJECT *obj);
void ProcessFace(QFACE *qface);
void OutputModel(void);
void OutputModelPsx();
void OutputLog(void);
void WeldModel(char display);
void CalcFaceCubes(void);
void OutputCollision(void);
void OutputNewCollision(void);
void OutputNewCollisionTris(void);
void OutputHulls(void);
void OutputOneHull(long hullnum);
void OutputOneBall(long hullnum);
void OutputHullGrid(void);
void FindFaceNormals(void);
void FindNormals(void);
void CalcWorldGouraud(void);
void CalcWorldFlat(void);
void SortModel(void);
void SortModelPsx(void);
void BuildPlane(VECTOR *a, VECTOR *b, VECTOR *c, PLANE *p);
void AddLitFileLights(void);
void AddLightOmni(FILELIGHT *light);
void AddLightOmniNormal(FILELIGHT *light);
void AddLightSpot(FILELIGHT *light);
void AddLightSpotNormal(FILELIGHT *light);
void OutputVisFile(void);
void PromoteTriangles(void);
long AddAnimatingTexture(QTEXTURE *qtex);
void OutputAnimatingTextures();

//-----------------------------------------------------------------------------------------------

#define MAX_GFX					16384

#define G_MWO_POINT_RGBA		0x10
#define G_MWO_POINT_ST			0x14
#define G_MWO_POINT_XYSCREEN	0x18
#define G_MWO_POINT_ZSCREEN		0x1c

#define SIZE_GFX_VERTEX		(1 * 8)
#define SIZE_GFX_TRI		(1 * 8)
#define SIZE_GFX_QUAD		(1 * 8)
#define SIZE_GFX_TEX		(1 * 8)
#define SIZE_GFX_MODVTX		(1 * 8)
#define SIZE_GFX_CCFLAT		(2 * 8)
#define SIZE_GFX_CCTEXT		(2 * 8)
#define SIZE_GFX_RMSEMI		(1 * 8)
#define SIZE_GFX_RMNORM		(1 * 8)
#define SIZE_GFX_SSIDED		(1 * 8)
#define SIZE_GFX_DSIDED		(1 * 8)

#define BMP_TYPE_4BIT		4
#define BMP_TYPE_8BIT		8

typedef struct
{
	unsigned char	b;
	unsigned char	g;
	unsigned char	r;
	unsigned char	a;
} RGB_QUAD;


// N64 texture page dimensions
typedef struct
{
	short	w;
	short	h;
	short	type;
	char	name[64];
	BITMAPINFO	*addr;
	long	palsize;
	long	paloffset;
} TPDIM;

// N64 specific defines
typedef enum
{
	GFX_NULL = 0,
	GFX_VTX,
	GFX_TRI,
    GFX_QUAD,
	GFX_TEX,
	GFX_MODVTX,
	GFX_CCFLAT,
	GFX_CCTEXT,
	GFX_SSIDED,
	GFX_DSIDED,
	GFX_RMSEMI,
	GFX_RMNORM,
} GFX_CMD;

// N64 specific typedef, used for marking UV rectangles on texture maps
typedef struct 
{
	short	sl;
	short	tl;
	short	sh;
	short	th;
	long	tpage;
	long	texoffset;				// Offset into output texture file
} TMEM;

// N64 specific cache vertex definition
typedef struct
{
	short	vtx;
	short	u;
	short	v;
	long	rgba;
	long	tmem;
} CACHE;


// N64 typedefs (copied over from GBI.H)

//
// Vertex (set up for use with colors)
//

typedef struct
{
	short			ob[3]; 	// x, y, z
	unsigned short	flag;
	short			tc[2]; 	// texture coord
	unsigned char	cn[4];	// color & alpha
} Vtx_t;

//
// Vertex (set up for use with normals)
//

typedef struct 
{
	short			ob[3];	// x, y, z
	unsigned short	flag;
	short			tc[2]; 	// texture coord
	signed char		n[3];  	// normal
	unsigned char   a;      // alpha
} Vtx_tn;

typedef union {
    Vtx_t	v; 				 // Use this one for colors
    Vtx_tn	n;  			 // Use this one for normals
    LARGE_INTEGER	force_structure_alignment;
} Vtx;

typedef struct
{
	long	msw;
	long	lsw;
} GFX;


// N64 specific prototpyes
void GenerateTmemList(void);
void OutputModelN64(void);
void ProcessModelN64(void);
void OutputCollisionN64(void);
void SortModelN64(void);
void fputlong(long value, FILE *file);
void fputshort(short value, FILE *file);
void fputfloat(float value, FILE *file);
