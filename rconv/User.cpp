
#include "stdafx.h"
#include "HabExtract.h"
#include "PSX_Polygon.h"
#include "user.h"
#include "units.h"

// globals

FILE *ofile, *lfile, *pfile;
HLOCAL PolyHandle, VertHandle;
POLY *PolyPtr;
HLOCAL HullPolyHandle;
POLY *HullPolyPtr;
HLOCAL HullBBoxHandle;
BBOX *HullBBoxPtr;
VERTEX *VertPtr;
short PolyCount, PolyNum, VertNum, TotalCubeVertNum, HullPolyNum;
char Coll = 0;
char NewColl = 0;
char Auto = 0;
char psx = 0;
char Hull = 0;
char Balls = 0;
char AutoTex = 0;
char Gouraud = 0;
char Flat = 0;
char UsePortals = 0;
char NoTriPromote = FALSE;
char AppendFile = FALSE;
char *file = NULL;
char *outfile = NULL;
char *logfile = NULL;
char *litfile = NULL;
char *litoutfile = NULL;
char *visfile = NULL;
char *visoutfile = NULL;
char *dbase = NULL;
char *filter = NULL;
char *SingleGroup = NULL;
char CurrentGroupName[128];
unsigned char WorldPer = 100;
float LitPer = 1;
float Scale = 1;
float Weld = 0;
float HullGridSize = 0;
float NewCollGridSize = 0;
float GridExpand = 70;
bool CollTris = FALSE;
float Cube = 0;
float BigCube = 0;
float MinX = 99999, MaxX = -99999;
float MinY = 99999, MaxY = -99999;
float MinZ = 99999, MaxZ = -99999;
float HullMinSep = 5000.0f;
float CubeStartX, CubeStartY, CubeStartZ;
float Lx, Ly, Lz, Lr, Lg, Lb;
float LightDelta[3];
long CubeNumX, CubeNumY, CubeNumZ, CubeNumTotal, CubeNumUsed;
long BigCubeNumX, BigCubeNumY, BigCubeNumZ, BigCubeNumTotal, BigCubeNumUsed;
long GroupNum;
long SingleGroupNum;
long TexAnimNum;
TEXANIM_HEADER TexAnim[MAX_TEXANIMS];

// N64 specific definitions
char		n64 = 0;
HLOCAL		TmemHandle;
TMEM       *TmemPtr;
HLOCAL		VtxHandle;
Vtx  	   *VtxPtr;
HLOCAL		GfxHandle;
GFX	   	   *GfxPtr;
HLOCAL		TPHandle[26];
long		MaxTpage = -1;
TPDIM		TPdim[26];
short		VtxIdx[65536];
long		RegCnt[10240];
EDIT_PORTAL	Portal[MAX_EDIT_PORTALS];
EDIT_PORTAL	Region[MAX_EDIT_PORTALS];
long		NumPortal = 0;
long		NumRegion = 0;
char	   *TexFname = NULL;
long		Tmem;


// auto tex uv table

float AutoTexTable[] = {0, 0, 0.25f, 0, 0.25f, 0.25f, 0, 0.25f};

void RotTransVector(MATRIX *mat, VECTOR *trans, VECTOR *in, VECTOR *out);
void BuildPlane(VECTOR *a, VECTOR *b, VECTOR *c, PLANE *p);
void FixTexture(POLY *SrcPoly, long TPw, long TPh);
void LoadPortals(void);
void WritePortals(void);
void WriteRegionModels(void);


///////////////
// user prog //
///////////////

void theUserProgram(void)
{
	short i;
	QOBJECT *obj;
	char buf[256];

// parse command line args

	if (__argc == 1)
	{
		api.Error("No command line args");
		return;
	}

	for (i = 1 ; i < __argc ; i++)
	{
		if (*__argv[i] != '-')
		{
			api.Error("Can't parse command line");
			return;
		}

// psx mode

		if (!strcmp(__argv[i], "-psx"))
		{
			api.Log("PSX output...\n");
			psx = TRUE;
			continue;
		}

// n64 mode

		if (!strcmp(__argv[i], "-n64"))
		{
			api.Log("N64 output...\n");
			n64 = TRUE;
			continue;
		}

// use .LIT file

		if (!strcmp(__argv[i], "-lit"))
		{
			i++;
			litfile = __argv[i];
			i++;
			litoutfile = __argv[i];
			i++;
			WorldPer = (unsigned char)atol(__argv[i]);
			i++;
			LitPer = (float)atof(__argv[i]) / 100;
			continue;
		}

// use .VIS file

		if (!strcmp(__argv[i], "-vis"))
		{
			i++;
			visfile = __argv[i];
			i++;
			visoutfile = __argv[i];
			continue;
		}

// bucket filter

		if (!strcmp(__argv[i], "-filter"))
		{
			i++;
			filter = __argv[i];
			continue;
		}

// output single group

		if (!strcmp(__argv[i], "-group"))
		{
			i++;
			SingleGroup = __argv[i];
			continue;
		}

// n64 texture filename

		if (!strcmp(__argv[i], "-tex"))
		{
			i++;
			TexFname = __argv[i];
			continue;
		}
			
// no tri promote

		if (!strcmp(__argv[i], "-nop"))
		{
			NoTriPromote = TRUE;
			continue;
		}

// single letters

		switch (*(__argv[i] + 1))
		{

// p3d file

			case 'f':
			i++;
			file = __argv[i];
			break;

// database model

			case 'd':
			i++;
			dbase = __argv[i];
			break;

// output file

			case 'o':
			i++;
			outfile = __argv[i];
			break;

// log file

			case 'l':
			i++;
			logfile = __argv[i];
			break;

// weld tolerance

			case 'w':
			Weld = (float)atof(__argv[i] + 2);
			Weld *= Weld;
			break;

// cube size

			case 'c':
			Cube = (float)atof(__argv[i] + 2);
			BigCube = Cube * 4;
			break;

// scale

			case 's':
			Scale = (float)atof(__argv[i] + 2);
			break;

// output collision file

			case 'x':
			Coll = TRUE;
			break;

// output new collision file

			case 'X':
			NewColl = TRUE;
			if (*(__argv[i] + 2)) NewCollGridSize = (float)atof(__argv[i] + 2);
			break;

// convert new collision to Tris
			case 'T':
			CollTris = TRUE;
			break;

// set the collision grid expansion
			case 'E':
			if (*(__argv[i] + 2)) GridExpand = (float)atof(__argv[i] + 2);
			break;

// autorun

			case 'a':
			Auto = TRUE;
			break;

// auto texture

			case 't':
			AutoTex = TRUE;
			break;

// flat shade

			case 'z':
			case 'Z':
			Flat = (char)(1 + (*(__argv[i] + 1) == 'Z'));
			i++;
			Lx = (float)atof(__argv[i]);
			i++;
			Ly = (float)atof(__argv[i]);
			i++;
			Lz = (float)atof(__argv[i]);
			i++;
			Lr = (float)atof(__argv[i]);
			i++;
			Lg = (float)atof(__argv[i]);
			i++;
			Lb = (float)atof(__argv[i]);
			break;

// gouraud shade

			case 'g':
			Gouraud = TRUE;
			i++;
			Lx = (float)atof(__argv[i]);
			i++;
			Ly = (float)atof(__argv[i]);
			i++;
			Lz = (float)atof(__argv[i]);
			i++;
			Lr = (float)atof(__argv[i]);
			i++;
			Lg = (float)atof(__argv[i]);
			i++;
			Lb = (float)atof(__argv[i]);
			break;

// output convex hull

			case 'h':
			Hull = TRUE;
			if (*(__argv[i] + 2)) HullMinSep = (float)atof(__argv[i] + 2);
			break;

// output spheres		

			case 'b':
			Hull = TRUE;
			Balls = TRUE;
			AppendFile = TRUE;
			break;

			case 'p':
			case 'P':
			UsePortals = TRUE;
			
		}
	}

// got all we need?

	if (!file)
	{
		api.Error("No file specified");
		return;
	}

	if (!dbase)
	{
		api.Error("No dbase object specified");
		return;
	}

	if (!outfile)
	{
		api.Error("No output file specified");
		return;
	}

// ok, go

	api.SetAutoRun(Auto);

	if (api.Begin("RCONV", file))
	{
		QBUCKET *bkt = api.GetBucket(dbase, TRUE);
		if (bkt == NULL)
		{
			api.Error("Can't find root object");
			return;
		}

// filter bucket

		if (filter)
		{
			wsprintf(buf, "Name = ~%s", filter);
			bkt->RemoveObjects(buf);
			wsprintf(buf, "Filter = %s\n", filter);
			api.Log(buf);
		}

// Tell % display how many objects to expect

		api.SetProgress(bkt->GetNumObjectDescendants());

// open output files

		if (AppendFile) {
			ofile = fopen(outfile, "ab");
		} else {
			ofile = fopen(outfile, "wb");
		}
		if (ofile == NULL)
		{
			api.Error("Can't open output file");
			return;
		}

		if (logfile)
		{
			lfile = fopen(logfile, "w");
			if (lfile == NULL)
			{
				api.Error("Can't open log file");
				return;
			}
		}

// count total polys

		PolyNum = 0;
		obj = bkt->GetFirstModel();

		if (!obj)
		{
			api.Error("Empty object!");
			return;
		}

		while (obj != NULL)
		{
			CountObjectFaces(obj);
			obj = bkt->GetNextModel();
		}

		if (!PolyNum)
		{
			api.Error("No polys found!");
			return;
		}

// alloc memory for poly space

		PolyHandle = LocalAlloc(LMEM_FIXED, sizeof(POLY) * PolyNum);
		PolyPtr = (POLY*)LocalLock(PolyHandle);

// alloc memory for vert space

		VertHandle = LocalAlloc(LMEM_FIXED, sizeof(VERTEX) * PolyNum * 4);
		VertPtr = (VERTEX*)LocalLock(VertHandle);

		if (n64)
		{
			// alloc memory for tmem buffers
			TmemHandle = LocalAlloc(LMEM_FIXED, sizeof(TMEM) * PolyNum);
			TmemPtr = (TMEM *)LocalLock(TmemHandle);

			// alloc memory for Vtx buffer
			VtxHandle = LocalAlloc(LMEM_FIXED, sizeof(Vtx) * PolyNum * 4);
			VtxPtr = (Vtx *)LocalLock(VtxHandle);

			// alloc memory for toeknised Gfx list buffer
			GfxHandle = LocalAlloc(LMEM_FIXED, sizeof(GFX) * MAX_GFX);
			GfxPtr = (GFX *)LocalLock(GfxHandle);

			for (i = 0; i < 16; i++)						// Clear texture page dimension list
			{
				TPdim[i].w = TPdim[i].h = 0;
			}
		}

// process objects

		GroupNum = 0;
		PolyCount = 0;
		obj = bkt->GetFirstModel();

		while (obj != NULL)
		{
			ProcessObject(obj);
			obj = bkt->GetNextModel();
		}

// get face normals

		FindFaceNormals();

// promote tri's to quads

		if (!Coll && !NewColl && !Hull && !NoTriPromote)
		{
			PromoteTriangles();
		}

// display misc info

		wsprintf(buf, "Total Faces = %d\n", PolyNum);
		api.Log(buf);

		if (!psx) {
			wsprintf(buf, "Bounding Box:  X %ld to %ld, Y %ld to %ld, Z %ld to %ld\n\n", (long)MinX, (long)MaxX, (long)MinY, (long)MaxY, (long)MinZ, (long)MaxZ);
			api.Log(buf);
		} else {
			wsprintf(buf, "Bounding Box:  X %ld to %ld, Y %ld to %ld, Z %ld to %ld\n\n", TO_LENGTH((long)MinX), TO_LENGTH((long)MaxX), TO_LENGTH((long)MinY), TO_LENGTH((long)MaxY), TO_LENGTH((long)MinZ), TO_LENGTH((long)MaxZ));
			api.Log(buf);
		}

		api.Update();

// weld model / find vert normals if not collision file

		if (!Coll && !NewColl && !Hull)
		{
			WeldModel(TRUE);
			api.Log("Finding normals...\n");
			FindNormals();
		}

// a bit of gouraud?

		if (Gouraud)
		{
			wsprintf(buf, "Adding gouraud %ld, %ld, %ld\n", (long)Lr, (long)Lg, (long)Lb);
			api.Log(buf);
			CalcWorldGouraud();
		}

// a bit of flat?

		if (Flat)
		{
			wsprintf(buf, "Adding flat %ld, %ld, %ld\n", (long)Lr, (long)Lg, (long)Lb);
			api.Log(buf);
			CalcWorldFlat();
		}

// lit file?

		if (litfile)
		{								 
			wsprintf(buf, "Adding lights from '%s'\n", litfile);
			api.Log(buf);
			AddLitFileLights();
		}

// log file?

		if (logfile)
		{
			wsprintf(buf, "Writing log file %s\n", logfile);
			api.Log(buf);
			OutputLog();
		}

// collision file?

		if (Coll)
		{
			wsprintf(buf, "Writing collision file %s\n", outfile);
			api.Log(buf);
			if (n64)
			{
				OutputCollisionN64();
			}
			else
			{
				OutputCollision();
			}
		}

// new collision file

		else if (NewColl)
		{
			wsprintf(buf, "Writing new collision file %s\n", outfile);
			api.Log(buf);
			if (CollTris) {
				OutputNewCollisionTris();
			} else {
				OutputNewCollision();
			}
		}

// convex hull?

		else if (Hull)
		{
			wsprintf(buf, "Writing convex hull file %s\n", outfile);
			api.Log(buf);
			OutputHulls();
		}

// cubes?

		else if ((UsePortals) & (n64))
		{
			GenerateTmemList();
			LoadPortals();											// Load portals & region data
			WritePortals();											// Write separated portal data 
			WriteRegionModels();									// Now write region headers, each header followed by the region model
		}
		else if (Cube)
		{
			if (n64)
			{
				// Generate TMEM areas for entire model
				GenerateTmemList();
			}
			wsprintf(buf, "Writing cube file %s\n", outfile);
			api.Log(buf);
			CalcFaceCubes();
		}

// simple model

		else
		{
			if (n64)
			{
				// Generate TMEM areas for entire model
				GenerateTmemList();
			}

			wsprintf(buf, "Writing model file %s\n", outfile);
			api.Log(buf);

			if (psx)
			{
				SortModelPsx();
				OutputModelPsx();
			}
			else if (n64)
			{
				SortModelN64();
				OutputModelN64();
			}
			else
			{
				SortModel();
				OutputModel();
			}
		}

// free mem

		LocalFree(PolyHandle);
		LocalFree(VertHandle);
		if (n64)
		{
			LocalFree(TmemHandle);
			LocalFree(VtxHandle);
			LocalFree(GfxHandle);
		}

// conv .VIS file?

		if (visfile)
		{
			OutputVisFile();
		}

// done

		fclose(ofile);
		if (logfile) fclose(lfile);
		api.End("Done");
	}
}

////////////////////////////
// count one object faces //
////////////////////////////

void CountObjectFaces(QOBJECT *obj)
{

// load faces, get num

	obj->LoadFaces();
	PolyNum = (short)(PolyNum + obj->GetFaceCount());
	obj->UnloadFaces();

// go thru children

//	QOBJECT *child = obj->GetFirstChild();
//	while (child != NULL)
//	{
//		CountObjectFaces(child);
//		child = obj->GetNextChild();
//		if (api.Update()) break;
//	}
}

////////////////////////
// process one object //
////////////////////////

void ProcessObject(QOBJECT *obj)
{
	QFACE *qface;
	short count, i;
	char buf[128];
	char groupname[128];
	QVALUE *qval;

// single group?

	if (SingleGroup)
	{
		wsprintf(groupname, "%s", obj->GetName());
		for (i = 0 ; i < (short)strlen(groupname) ; i++) groupname[i] = (char)tolower(groupname[i]);

		if (!strcmp(groupname, SingleGroup))
		{
			wsprintf(buf, "Removing all non '%s' groups...\n", SingleGroup);
			api.Log(buf);
			SingleGroupNum = GroupNum;
		}
	}

// load faces, get count

	obj->LoadFaces();
	count = (short)obj->GetFaceCount();

// process if any faces

	if (count)
	{

// display object info

		wsprintf(CurrentGroupName, obj->GetName());
		wsprintf(buf, "Group '%s' = %d faces\n", CurrentGroupName, count);
		api.Log(buf);

// grab all faces

		qface = obj->GetFirstFace();

		for (i = 0 ; i < count ; i++, PolyCount++)
		{
			qval = (QVALUE*)qface->Property("Ignore");
			if (qval)
			{
				PolyNum--;
				PolyCount--;
			}
			else
			{
				ProcessFace(qface);
			}
			qface = obj->GetNextFace();
		}

// inc group

		GroupNum++;
	}

// free faces, advance progress counter

	obj->UnloadFaces();
	api.AdvanceProgress();

// go thru children

//	QOBJECT *child = obj->GetFirstChild();
//	while (child != NULL)
//	{
//		ProcessObject(child);
//		child = obj->GetNextChild();
//		if (api.Update()) break;
//	}
}

///////////////////////
// process one qface //
///////////////////////

void ProcessFace(QFACE *qface)
{
	QPOINT qp;
	QTEXTURE *qf, *qb;
	QUV *quv;
	short vcount, i;
	QVALUE *qval, *st;
	long smoothing, psxgroup, col;
	const char *name;
	char buf[128];
	char tpage;
	BITMAPINFO *Bmpinfo;

// quad?

	(PolyPtr + PolyCount)->Type = 0;

	vcount = (short)qface->GetPointCount();
	if (vcount < 3 || vcount > 4)
	{
		wsprintf(buf, "Poly %d has %d vertices", PolyCount, vcount);
		api.Error(buf);
		PolyCount--;
		return;
	}

	if (vcount == 4) (PolyPtr + PolyCount)->Type |= TYPE_QUAD;

// double sided?

	qf = qface->GetFrontTexture();
	qb = qface->GetBackTexture();

	qval = (QVALUE*)qface->Property("Double");
	if (qb || qval) (PolyPtr + PolyCount)->Type |= TYPE_DSIDED;

// set tpage or anim number

	if (!qf)
	{
		if (!AutoTex) (PolyPtr + PolyCount)->Tpage = -1;
		else (PolyPtr + PolyCount)->Tpage = 0;
	}
	else
	if (qf->IsAnimation())
	{
		PolyPtr[PolyCount].Type |= TYPE_TEXANIM;
		PolyPtr[PolyCount].Tpage = (short)AddAnimatingTexture(qf);
		qf = NULL;
	}
	else
	{
		name = qf->GetBitmapName();
		i = (short)strlen(name);
		tpage = name[i - 1];
		if (tpage >= 'A' && tpage <= 'Z') tpage += ('a' - 'A');
		if (tpage < 'a' || tpage > 'z') tpage = 'a';
		(PolyPtr + PolyCount)->Tpage = (short)(tpage - 'a');

		if (n64)																	
		{
			if ((PolyPtr + PolyCount)->Tpage >= 0)
			{
				if ((PolyPtr + PolyCount)->Tpage > MaxTpage)
				{
					MaxTpage = (PolyPtr + PolyCount)->Tpage;
				}	
				if (TPdim[(short)(tpage - 'a')].w == 0)										// !MT! Check to see if we have an entry for this tpage
				{
					Bmpinfo = qf->GetBitmapData();											// !MT! Get BMP info header
					strcpy(TPdim[(short)(tpage - 'a')].name, name);							// !MT! Store filename of bitmap (sans .BMP)
					TPdim[(short)(tpage - 'a')].w = (short)Bmpinfo->bmiHeader.biWidth;		// !MT! Store TP width
					TPdim[(short)(tpage - 'a')].h = (short)Bmpinfo->bmiHeader.biHeight;		// !MT! Store TP height (can be negative in some BMPs)
					TPdim[(short)(tpage - 'a')].type = (short)Bmpinfo->bmiHeader.biBitCount;
					if ((TPdim[(short)(tpage - 'a')].type != BMP_TYPE_4BIT) && (TPdim[(short)(tpage - 'a')].type != BMP_TYPE_8BIT))
					{
						wsprintf(buf, "Texture file '%s' is not 16 or 256 colours! (type is %d\n)", name, TPdim[(short)(tpage - 'a')].type);
						api.Error(buf);
						return;
					}
					TPdim[(short)(tpage - 'a')].addr = Bmpinfo;
					wsprintf(buf, "Found texture map: %s (%d-bit, w: %d, h: %d)\n", TPdim[(short)(tpage - 'a')].name, TPdim[(short)(tpage - 'a')].type, TPdim[(short)(tpage - 'a')].w, TPdim[(short)(tpage - 'a')].h);
					api.Log(buf);
				}
			}
		}
	}

// semi trans?

	st = (QVALUE*)qface->Property("Semi");
	if (st) (PolyPtr + PolyCount)->Type |= TYPE_SEMITRANS;

	qval = (QVALUE*)qface->Property("Semi2");
	if (qval) (PolyPtr + PolyCount)->Type |= (TYPE_SEMITRANS | TYPE_SEMITRANS_ONE);

// smoothing?

	qval = (QVALUE*)qface->Property("Smooth");
	if (!qval) smoothing = 0;
	else smoothing = ((long)*qval) / 25;

	for (i = 0 ; i < vcount ; i++)
		PolyPtr[PolyCount].v[i].Smooth = smoothing;

// material

	qval = (QVALUE*)qface->Property("material");
	if (!qval) (PolyPtr + PolyCount)->Material = 0;
	else (PolyPtr + PolyCount)->Material = (long)*qval;

// mirror?

	qval = (QVALUE*)qface->Property("Mirror");
	if (qval) (PolyPtr + PolyCount)->Type |= TYPE_MIRROR;

// coll object only

	qval = (QVALUE*)qface->Property("Object only");
	if (qval) (PolyPtr + PolyCount)->Type |= COLLTYPE_OBJECTONLY;

// coll camera only

	qval = (QVALUE*)qface->Property("Camera only");
	if (qval) (PolyPtr + PolyCount)->Type |= COLLTYPE_CAMERAONLY;

// no env

	qval = (QVALUE*)qface->Property("NoEnv");
	if (qval) (PolyPtr + PolyCount)->Type |= TYPE_NOENV;

// env

	qval = (QVALUE*)qface->Property("Env");
	if (qval)
	{
		(PolyPtr + PolyCount)->Type |= TYPE_ENV;
		col = ((long)*qval) * 255 / 100;
		(PolyPtr + PolyCount)->EnvRGB = col | col << 8 | col << 16;
	}

// group

	(PolyPtr + PolyCount)->Group = GroupNum;

// psx group bits

	if (psx && Cube)
	{
		psxgroup = (CurrentGroupName[0] - '1');
		if (psxgroup < 0 || psxgroup > 2)
		{
			wsprintf(buf, "You bloody numbnuts!  '%s' isn't a good group name", CurrentGroupName);
			api.Error(buf);
			return;
		}
		(PolyPtr + PolyCount)->Type |= (psxgroup << PSX_GROUP_SHIFT);
	}

// !MT! N64: Clear 'processed' flag

	(PolyPtr + PolyCount)->Done = 0;

// get verts, UV's, rgb's

	qp = qface->GetFirstPoint();
	if (qf) quv = qf->GetFirstUV();

	for (i = 0 ; i < vcount ; i++)
	{
		(PolyPtr + PolyCount)->v[i].x = (float)qp.Vertex.X * Scale;
		(PolyPtr + PolyCount)->v[i].y = -(float)qp.Vertex.Y * Scale;
		(PolyPtr + PolyCount)->v[i].z = (float)qp.Vertex.Z * Scale;

		if ((PolyPtr + PolyCount)->v[i].x < MinX) MinX = (PolyPtr + PolyCount)->v[i].x;
		if ((PolyPtr + PolyCount)->v[i].x > MaxX) MaxX = (PolyPtr + PolyCount)->v[i].x;
		if ((PolyPtr + PolyCount)->v[i].y < MinY) MinY = (PolyPtr + PolyCount)->v[i].y;
		if ((PolyPtr + PolyCount)->v[i].y > MaxY) MaxY = (PolyPtr + PolyCount)->v[i].y;
		if ((PolyPtr + PolyCount)->v[i].z < MinZ) MinZ = (PolyPtr + PolyCount)->v[i].z;
		if ((PolyPtr + PolyCount)->v[i].z > MaxZ) MaxZ = (PolyPtr + PolyCount)->v[i].z;

		if (qf)
		{
			(PolyPtr + PolyCount)->uv[i].u = (float)quv->U;
			(PolyPtr + PolyCount)->uv[i].v = (float)quv->V;
		}
		else if (AutoTex)
		{
			(PolyPtr + PolyCount)->uv[i].u = AutoTexTable[i * 2];
			(PolyPtr + PolyCount)->uv[i].v = AutoTexTable[i * 2 + 1];
		}
		else
		{
			(PolyPtr + PolyCount)->uv[i].u = 0;
			(PolyPtr + PolyCount)->uv[i].v = 0;
		}

		(PolyPtr + PolyCount)->rgb[i] = (qp.Colour.A << 24) | (qp.Colour.R << 16) | (qp.Colour.G << 8) | qp.Colour.B;
		if (st) (PolyPtr + PolyCount)->rgb[i] |= (((long)*st) * 255 / 100) << 24;
		qp = qface->GetNextPoint();
		if (qf)
		{
			quv = qf->GetNextUV();
			if (!quv)
				quv = qf->GetFirstUV();
		}
	}
}

///////////////////////////
// add texture animation //
///////////////////////////

long AddAnimatingTexture(QTEXTURE *qtex)
{
	long i, j, id;
	QTEXTURE *qcel;
	QUV *quv;
	const char *name;
	char tpage;
	TEXANIM_FRAME frame;

// get 3 UV's from 1st frame

	qcel = qtex->GetCel(0);
	quv = qcel->GetFirstUV();
	j = 0;
	while (quv && (j < 4))
	{
		frame.uv[j].u = (float)quv->U;
		frame.uv[j].v = (float)quv->V;

		quv = qcel->GetNextUV();
		j++;
	}

// check for existing animating texture

	id = qtex->GetTextureID();
	for (i = 0 ; i < TexAnimNum ; i++)
	{
		if (id == TexAnim[i].ID && !memcmp((char*)frame.uv, (char*)TexAnim[i].Frame[0].uv, 24))
			return i;
	}

// ok, create new anim

	TexAnim[TexAnimNum].ID = id;
	TexAnim[TexAnimNum].FrameNum = qtex->GetNumberOfCels();
	TexAnim[TexAnimNum].Handle = LocalAlloc(LMEM_FIXED, sizeof(TEXANIM_FRAME) * TexAnim[TexAnimNum].FrameNum);
	TexAnim[TexAnimNum].Frame = (TEXANIM_FRAME*)LocalLock(TexAnim[TexAnimNum].Handle);

// get each frame

	for (i = 0 ; i < TexAnim[TexAnimNum].FrameNum; i++)
	{
		qcel = qtex->GetCel(i);
		if (qcel->IsAnimation())
		{
			api.Error("Don't bloody nest animations!!!");
			continue;
		}

		name = qcel->GetBitmapName();
		j = strlen(name);
		tpage = name[j - 1];
		if (tpage >= 'A' && tpage <= 'Z') tpage += ('a' - 'A');
		if (tpage < 'a' || tpage > 'z') tpage = 'a';
		TexAnim[TexAnimNum].Frame[i].Tpage = (long)(tpage - 'a');

		TexAnim[TexAnimNum].Frame[i].Time = (float)qtex->GetCelDuration(i) / 50.0f;

		quv = qcel->GetFirstUV();
		j = 0;
		while (quv && (j < 4))
		{
			TexAnim[TexAnimNum].Frame[i].uv[j].u = (float)quv->U;
			TexAnim[TexAnimNum].Frame[i].uv[j].v = (float)quv->V;

			quv = qcel->GetNextUV();
			j++;
		}
	}

// return anim num

	return TexAnimNum++;
}

//////////////////
// output model //
//////////////////

void OutputModel()
{
	POLY *p;
	VERTEX *v;
	short i;

// write PolyNum, VertNum

	fwrite(&PolyNum, sizeof(PolyNum), 1, ofile);
	fwrite(&VertNum, sizeof(VertNum), 1, ofile);

// write out polys

	p = PolyPtr;
	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// write properties

		fwrite(&p->Type, sizeof(p->Type), 1, ofile);
		fwrite(&p->Tpage, sizeof(p->Tpage), 1, ofile);

// write vert indexes

		fwrite(&p->vi[0], sizeof(short), 1, ofile);
		fwrite(&p->vi[1], sizeof(short), 1, ofile);
		fwrite(&p->vi[2], sizeof(short), 1, ofile);
		fwrite(&p->vi[3], sizeof(short), 1, ofile);

// write RGB's

		fwrite(&p->rgb[0], sizeof(U32), 1, ofile);
		fwrite(&p->rgb[1], sizeof(U32), 1, ofile);
		fwrite(&p->rgb[2], sizeof(U32), 1, ofile);
		fwrite(&p->rgb[3], sizeof(U32), 1, ofile);

// write UV's

		fwrite(&p->uv[0], sizeof(UV), 1, ofile);
		fwrite(&p->uv[1], sizeof(UV), 1, ofile);
		fwrite(&p->uv[2], sizeof(UV), 1, ofile);
		fwrite(&p->uv[3], sizeof(UV), 1, ofile);
	}

// write out verts

	v = VertPtr;
	for (i = 0 ; i < VertNum ; i++, v++)
	{
		fwrite(&v->x, sizeof(float), 1, ofile);
		fwrite(&v->y, sizeof(float), 1, ofile);
		fwrite(&v->z, sizeof(float), 1, ofile);
		fwrite(&v->nx, sizeof(float), 1, ofile);
		fwrite(&v->ny, sizeof(float), 1, ofile);
		fwrite(&v->nz, sizeof(float), 1, ofile);
	}
}

///////////////////////////////
// output animating textures //
///////////////////////////////

void OutputAnimatingTextures()
{
	long i;
	char buf[128];

// write number

	fwrite(&TexAnimNum, sizeof(TexAnimNum), 1, ofile);

// loop thru all

	for (i = 0 ; i < TexAnimNum ; i++)
	{

// output info

		wsprintf(buf, "Anim %d = %d frames\n", i, TexAnim[i].FrameNum);
		api.Log(buf);

// output frame count

		fwrite(&TexAnim[i].FrameNum, sizeof(TexAnim[i].FrameNum), 1, ofile);

// output frames

		fwrite(TexAnim[i].Frame, sizeof(TEXANIM_FRAME), TexAnim[i].FrameNum, ofile);

// free mem

		LocalFree(TexAnim[i].Handle);
	}
}

///////////////////////
// output model - PSX//
///////////////////////

void OutputModelPsx()
{
	POLY *p;
	UV_PSX uv;
	VERTEX *v;
	VERTEX_PSX pv;
	short i, j, k, n[3], dflag, pnum;
	long temp;
	UV tempuv;
	char buf[128];

// count double sided to add to PolyNum

	pnum = PolyNum;
	for (i = 0 ; i < PolyNum ; i++)
		if (PolyPtr[i].Type & TYPE_DSIDED)
			pnum++;

// write PolyNum, VertNum

	fwrite(&pnum, sizeof(pnum), 1, ofile);
	fwrite(&VertNum, sizeof(VertNum), 1, ofile);

// write out polys

	p = PolyPtr;
	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// double sided?

		dflag = FALSE;
		if (p->Type & TYPE_DSIDED)
		{
			p->Type &= ~TYPE_DSIDED;
			dflag = TRUE;
		}

// write properties

		if (p->Tpage != -1) p->Type |= TYPE_TEXTURED;
		p->Type |= TYPE_GOURAUD;

		fwrite(&p->Type, sizeof(p->Type), 1, ofile);
		fwrite(&p->Tpage, sizeof(p->Tpage), 1, ofile);

// write vert indexes

		if (p->Type & TYPE_QUAD)
		{
			fwrite(&p->vi[1], sizeof(short), 1, ofile);
			fwrite(&p->vi[2], sizeof(short), 1, ofile);
			fwrite(&p->vi[3], sizeof(short), 1, ofile);
			fwrite(&p->vi[0], sizeof(short), 1, ofile);
		}
		else
		{
			fwrite(&p->vi[0], sizeof(short), 1, ofile);
			fwrite(&p->vi[1], sizeof(short), 1, ofile);
			fwrite(&p->vi[2], sizeof(short), 1, ofile);
			fwrite(&p->vi[3], sizeof(short), 1, ofile);
		}

// write RGB's

		SwapRGB(p->rgb[0]);
		SwapRGB(p->rgb[1]);
		SwapRGB(p->rgb[2]);
		SwapRGB(p->rgb[3]);

		if (p->Type & TYPE_QUAD)
		{
			fwrite(&p->rgb[1], sizeof(U32), 1, ofile);
			fwrite(&p->rgb[2], sizeof(U32), 1, ofile);
			fwrite(&p->rgb[3], sizeof(U32), 1, ofile);
			fwrite(&p->rgb[0], sizeof(U32), 1, ofile);
		}
		else
		{
			fwrite(&p->rgb[0], sizeof(U32), 1, ofile);
			fwrite(&p->rgb[1], sizeof(U32), 1, ofile);
			fwrite(&p->rgb[2], sizeof(U32), 1, ofile);
			fwrite(&p->rgb[3], sizeof(U32), 1, ofile);
		}

// write UV's

		if (p->Type & TYPE_QUAD)
		{
			for (j = 1 ; j < 5 ; j++)
			{
				k = (short)(p->uv[j % 4].u * 256);
				if (k > 255) k = 255;
				uv.u = (char)k;

				k = (short)(p->uv[j % 4].v * 256);
				if (k > 255) k = 255;
				uv.v = (char)k;

				fwrite(&uv, sizeof(uv), 1, ofile);
			}
		}
		else
		{
			for (j = 0 ; j < 4 ; j++)
			{
				k = (short)(p->uv[j % 4].u * 256);
				if (k > 255) k = 255;
				uv.u = (char)k;

				k = (short)(p->uv[j % 4].v * 256);
				if (k > 255) k = 255;
				uv.v = (char)k;

				fwrite(&uv, sizeof(uv), 1, ofile);
			}
		}

// if double sided, do it again in reverse order

		if (dflag)
		{
			wsprintf(buf, "Adding double sided poly %d\n", i);
			api.Log(buf);

			temp = p->vi[0];
			p->vi[0] = p->vi[2];
			p->vi[2] = (short)temp;

			SwapRGB(p->rgb[0]);
			SwapRGB(p->rgb[1]);
			SwapRGB(p->rgb[2]);
			SwapRGB(p->rgb[3]);

			temp = p->rgb[0];
			p->rgb[0] = p->rgb[2];
			p->rgb[2] = temp;

			tempuv = p->uv[0];
			p->uv[0] = p->uv[2];
			p->uv[2] = tempuv;

			i--;
			p--;
		}
	}

// write out verts

	v = VertPtr;
	for (i = 0 ; i < VertNum ; i++, v++)
	{
		pv.x = (short)(v->x);
		pv.y = (short)(v->y);
		pv.z = (short)(v->z);
		fwrite(&pv, sizeof(pv), 1, ofile);
		n[0] = (short)(v->nx * 4096);
		n[1] = (short)(v->ny * 4096);
		n[2] = (short)(v->nz * 4096);
		fwrite(n, sizeof(n), 1, ofile);
	}
}

/////////////////////
// output log file //
/////////////////////

void OutputLog(void)
{
	short vcount;
	POLY *p;
	VERTEX *v;
	short i, j, k;

// default group

	fprintf(lfile, "g default\n");

// write vertices

	v = VertPtr;
	for (i = 0 ; i < VertNum ; i++, v++)
	{
		fprintf(lfile, "v %f %f %f\n", v->x, v->y, v->z);
	}

// write out polys

	p = PolyPtr;
	for (i = j = 0 ; i < PolyNum ; i++, p++)
	{
		vcount = (short)(3 + (p->Type & TYPE_QUAD));
		fprintf(lfile, "f");
		for (k = 0 ; k < vcount ; k++) fprintf(lfile, " %d", p->vi[k] + 1);
		fprintf(lfile, "\n");
	}
}

///////////////////////////
// split + weld vertices //
///////////////////////////

void WeldModel(char display)
{
	short i, j, k, vcount, nextmess, per;
	float dx, dy, dz, dist;
	BOOL flag;
	POLY *p;
	char buf[128];

// display weld info?

	if (display)
		api.Log("Welding...\n");

// loop thru faces

	VertNum = 0;
	p = PolyPtr;
	nextmess = 0;
	per = 0;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// display percentage

	if (display)
	{
		if (i > nextmess)
		{
			per++;
			wsprintf(buf, " %d%%\n", per);
			api.Log(buf);
			api.Update();
			nextmess = PolyNum * per / 100;
		}
	}

// get vert num

		vcount = (short)(3 + (p->Type & TYPE_QUAD));

// loop thru verts for this face

		for (j = 0 ; j < vcount ; j++)
		{
			flag = FALSE;

// loop thru done vertices testing for weld

			if (Weld) for (k = 0 ; k < VertNum ; k++)
			{
				dx = VertPtr[k].x - p->v[j].x;
				dy = VertPtr[k].y - p->v[j].y;
				dz = VertPtr[k].z - p->v[j].z;
				dist = dx * dx + dy * dy + dz * dz;

// weld

				if (dist <= Weld && p->v[j].Smooth == VertPtr[k].Smooth)
				{
					p->vi[j] = k;
					flag = TRUE;
					break;
				}
			}

// copy new?

			if (!flag)
			{
				p->vi[j] = VertNum;
				VertPtr[VertNum] = p->v[j];
				VertNum++;
			}
		}
	}
}

/////////////////////
// calc face cubes //
/////////////////////

void CalcFaceCubes(void)
{
	long i, j, k, vcount, cubecount, bx, by, bz;
	POLY *p;
	float x, y, z, rx, ry, rz, cx, cy, cz, dist, rad;
	short x16, y16, z16, temp16;
	float Xmin, Xmax, Ymin, Ymax, Zmin, Zmax;
	char buf[256];
	HLOCAL ListHandle, CubeHandle, InfoHandle, BigHandle, BigInfoHandle, EnvHandle;
	long EnvNum, *EnvPtr;
	CUBELIST *ListPtr;
	POLY *CubePtr;
	CUBE_INFO *InfoPtr, *BigInfoPtr;
	BIGCUBELIST *BigPtr;
	VERTEX *v;

// calc cube dimensions

	CubeNumX = (long)((MaxX - MinX) / Cube) + 1;
	CubeNumY = (long)((MaxY - MinY) / Cube) + 1;
	CubeNumZ = (long)((MaxZ - MinZ) / Cube) + 1;
	CubeNumTotal = CubeNumX * CubeNumY * CubeNumZ;
	wsprintf(buf, "World cubes - %d, %d, %d = %d\n", CubeNumX, CubeNumY, CubeNumZ, CubeNumTotal);
	api.Log(buf);

// alloc list of polys per cube

	ListHandle = LocalAlloc(LPTR, sizeof(CUBELIST) * CubeNumTotal);
	ListPtr = (CUBELIST*)LocalLock(ListHandle);

// calc each poly's cube

	TotalCubeVertNum = 0;
	p = PolyPtr;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// get centre of poly

		x = y = z = 0;
		vcount = 3 + (p->Type & TYPE_QUAD);

		for (j = 0 ; j < vcount ; j++)
		{
			x += p->v[j].x;
			y += p->v[j].y;
			z += p->v[j].z;
		}

		x /= vcount;
		y /= vcount;
		z /= vcount;

// calc XYZ cube num

		p->CubeX = (long)((x - MinX) / Cube);
		p->CubeY = (long)((y - MinY) / Cube);
		p->CubeZ = (long)((z - MinZ) / Cube);

		p->CubeNum = p->CubeZ * (CubeNumX * CubeNumY);
		p->CubeNum += p->CubeY * CubeNumX;
		p->CubeNum += p->CubeX;

// inc count for cube concerned

		ListPtr[p->CubeNum].Count++;
	}

// alloc mem for putting each cube of poly's in

	CubeHandle = LocalAlloc(LMEM_FIXED, sizeof(POLY) * PolyNum);
	CubePtr = (POLY*)LocalLock(CubeHandle);

// setup each PolyPtr

	j = 0;
	for (i = 0 ; i < CubeNumTotal ; i++)
	{
		ListPtr[i].PolyPtr = CubePtr + j;
		j += ListPtr[i].Count;
	}

// loop thru each poly, copying it to correct slot

	p = PolyPtr;
	for (i = 0 ; i < PolyNum ; i++, p++)
	{
		memcpy(ListPtr[p->CubeNum].PolyPtr + ListPtr[p->CubeNum].Temp, p, sizeof(POLY));
		ListPtr[p->CubeNum].Temp++;
	}

// calc used cube num

	CubeNumUsed = 0;
	for (i = 0 ; i < CubeNumTotal ; i++) if (ListPtr[i].Count) CubeNumUsed++;

	wsprintf(buf, "Cubes used = %d\n", CubeNumUsed);
	api.Log(buf);

	wsprintf(buf, "Average poly count per cube = %d\n", PolyNum / CubeNumUsed);
	api.Log(buf);

// alloc mem for saving cube info

	InfoHandle = LocalAlloc(LMEM_FIXED, sizeof(CUBE_INFO) * CubeNumUsed);
	InfoPtr = (CUBE_INFO*)LocalLock(InfoHandle);

// alloc mem for env rgb's

	EnvHandle = LocalAlloc(LMEM_FIXED, sizeof(long) * PolyNum);
	EnvPtr = (long*)LocalLock(EnvHandle);
	EnvNum = 0;

// write used cube num

	if (n64)
	{
		wsprintf(buf, "CUB");
		buf[3] = 0;
		fwrite(buf, 1, sizeof(long), ofile);												// Write cubed world header
		fputlong(CubeNumUsed, ofile);
	}
	else
	{
		fwrite(&CubeNumUsed, sizeof(CubeNumUsed), 1, ofile);
	}

// loop thru each cube

	cubecount = 0;
	for (x = 0 ; x < CubeNumX ; x++) for (y = 0 ; y < CubeNumY ; y++) for (z = 0 ; z < CubeNumZ ; z++)
	{

// get cube index

		i = (long)z * (CubeNumX * CubeNumY);
		i += (long)y * CubeNumX;
		i += (long)x;

// if any poly's, write out cube info

		if (ListPtr[i].Count)
		{

// get poly num, poly ptr

			PolyPtr = ListPtr[i].PolyPtr;
			PolyNum = (short)ListPtr[i].Count;

// weld / sort model

			WeldModel(FALSE);
			TotalCubeVertNum = (short)(TotalCubeVertNum + VertNum);

			if (n64)
			{
				SortModelN64();
			}
			else if (psx)
			{
				SortModelPsx();
			}
			else
			{
				SortModel();
			}

// calc bounding box

			Xmin = Xmax = VertPtr->x;
			Ymin = Ymax = VertPtr->y;
			Zmin = Zmax = VertPtr->z;

			v = VertPtr;
			for (j = 0 ; j < VertNum ; j++, v++)
			{
				if (v->x < Xmin) Xmin = v->x;
				if (v->x > Xmax) Xmax = v->x;
	
				if (v->y < Ymin) Ymin = v->y;
				if (v->y > Ymax) Ymax = v->y;
	
				if (v->z < Zmin) Zmin = v->z;
				if (v->z > Zmax) Zmax = v->z;
			}

// calc cube centre

			cx = (Xmin + Xmax) / 2;
			cy = (Ymin + Ymax) / 2;
			cz = (Zmin + Zmax) / 2;

// calc radius

			rad = 0;
			v = VertPtr;
			for (j = 0 ; j < VertNum ; j++, v++)
			{
				rx = v->x - cx;
				ry = v->y - cy;
				rz = v->z - cz;
				dist = (float)sqrt((double)(rx * rx + ry * ry + rz * rz));

				if (dist > rad) rad = dist;
			}

// save info for big cubes

			InfoPtr[cubecount].x = cx;
			InfoPtr[cubecount].y = cy;
			InfoPtr[cubecount].z = cz;
			InfoPtr[cubecount].Radius = rad;
			InfoPtr[cubecount].Cube = cubecount;

// write out cube centre

			if (psx)
			{
				x16 = (short)cx;
				y16 = (short)cy;
				z16 = (short)cz;
				fwrite(&x16, sizeof(x16), 1, ofile);
				fwrite(&y16, sizeof(y16), 1, ofile);
				fwrite(&z16, sizeof(z16), 1, ofile);
			}
			else
			if (n64)
			{
				fputfloat(cx, ofile);
				fputfloat(cy, ofile);
				fputfloat(cz, ofile);
			}
			else
			{
				fwrite(&cx, sizeof(cx), 1, ofile);
				fwrite(&cy, sizeof(cy), 1, ofile);
				fwrite(&cz, sizeof(cz), 1, ofile);
			}

// write radius


			if (psx)
			{
				temp16 = (short)rad;
				fwrite(&temp16, sizeof(temp16), 1, ofile);
			}
			else
			if (n64)
			{
				fputfloat(rad, ofile);
			}
			else
			{
				fwrite(&rad, sizeof(rad), 1, ofile);
			}

// write bounding box

			if (n64)
			{
				fputfloat(Xmin, ofile);
				fputfloat(Xmax, ofile);
				fputfloat(Ymin, ofile);
				fputfloat(Ymax, ofile);
				fputfloat(Zmin, ofile);
				fputfloat(Zmax, ofile);
			}
			else if (!psx)
			{
				fwrite(&Xmin, sizeof(float), 1, ofile);
				fwrite(&Xmax, sizeof(float), 1, ofile);
				fwrite(&Ymin, sizeof(float), 1, ofile);
				fwrite(&Ymax, sizeof(float), 1, ofile);
				fwrite(&Zmin, sizeof(float), 1, ofile);
				fwrite(&Zmax, sizeof(float), 1, ofile);
			}

// remove unwanted groups?

			if (SingleGroup)
			{
				for (j = 0 ; j < PolyNum ; j++)
				{
					if (PolyPtr[j].Group != SingleGroupNum)
					{
						for (k = j ; k < PolyNum - 1 ; k++) PolyPtr[k] = PolyPtr[k + 1];
						PolyNum--;
						j--;
					}
				}

				WeldModel(FALSE);

				if (n64)
				{
					SortModelN64();
				}
				else if (psx)
				{
					SortModelPsx();
				}
				else
				{
					SortModel();
				}
			}

// write out model

			if (psx)
			{
				OutputModelPsx();
			}
			else
			if (n64)
			{
				OutputModelN64();
			}
			else
			{
				OutputModel();
			}

// save env rbg's

			for (j = 0 ; j < PolyNum ; j++) if (PolyPtr[j].Type & TYPE_ENV)
			{
				EnvPtr[EnvNum++] = PolyPtr[j].EnvRGB;
			}

// inc cube count

			cubecount++;
		}
	}

// free cube mem

	LocalFree(CubeHandle);
	LocalFree(ListHandle);

// display average cube vert num

	wsprintf(buf, "Average vert count per cube = %d\n", TotalCubeVertNum / CubeNumUsed);
	api.Log(buf);

// calc big cube dimensions

	BigCubeNumX = (long)((MaxX - MinX) / BigCube) + 1;
	BigCubeNumY = (long)((MaxY - MinY) / BigCube) + 1;
	BigCubeNumZ = (long)((MaxZ - MinZ) / BigCube) + 1;
	BigCubeNumTotal = BigCubeNumX * BigCubeNumY * BigCubeNumZ;
	wsprintf(buf, "Big cubes - %d, %d, %d = %d\n", BigCubeNumX, BigCubeNumY, BigCubeNumZ, BigCubeNumTotal);
	api.Log(buf);

// alloc list of big cubes

	BigHandle = LocalAlloc(LPTR, sizeof(BIGCUBELIST) * BigCubeNumTotal);
	BigPtr = (BIGCUBELIST*)LocalLock(BigHandle);

// calc each cubes big cube num

	for (i = 0 ; i < CubeNumUsed ; i++)
	{

// calc XYZ big cube num

		bx = (long)((InfoPtr[i].x - MinX) / BigCube);
		by = (long)((InfoPtr[i].y - MinY) / BigCube);
		bz = (long)((InfoPtr[i].z - MinZ) / BigCube);

		InfoPtr[i].BigCube = bz * (BigCubeNumX * BigCubeNumY);
		InfoPtr[i].BigCube += by * BigCubeNumX;
		InfoPtr[i].BigCube += bx;

// inc count for big cube concerned

		BigPtr[InfoPtr[i].BigCube].Count++;
	}

// calc used big cube num

	BigCubeNumUsed = 0;
	for (i = 0 ; i < BigCubeNumTotal ; i++) if (BigPtr[i].Count) BigCubeNumUsed++;
	wsprintf(buf, "Big cubes used = %d\n", BigCubeNumUsed);
	api.Log(buf);
	wsprintf(buf, "Average count per big cube = %d\n", CubeNumUsed / BigCubeNumUsed);
	api.Log(buf);

// write big cube header

	if (n64)
	{
		fputlong(BigCubeNumUsed, ofile);
	}
	else if (psx)
	{
		temp16 = (short)BigCubeNumUsed;
		fwrite(&temp16, sizeof(temp16), 1, ofile);
	}
	else
	{
		fwrite(&BigCubeNumUsed, sizeof(BigCubeNumUsed), 1, ofile);
	}

// loop thru each used big cube

	for (i = 0 ; i < BigCubeNumTotal ; i++) if (BigPtr[i].Count)
	{

// alloc mem for cubes within this big cube

		BigInfoHandle = LocalAlloc(LMEM_FIXED, sizeof(CUBE_INFO) * BigPtr[i].Count);
		BigInfoPtr = (CUBE_INFO*)LocalLock(BigInfoHandle);

// now get them with a bounding box

		Xmin = Ymin = Zmin = 999999;
		Xmax = Ymax = Zmax = -999999;
		cubecount = 0;

		for (j = 0 ; j < CubeNumUsed ; j++)
		{
			if (InfoPtr[j].BigCube == i)
			{
				if (InfoPtr[j].x - InfoPtr[j].Radius < Xmin) Xmin = InfoPtr[j].x - InfoPtr[j].Radius;
				if (InfoPtr[j].x + InfoPtr[j].Radius > Xmax) Xmax = InfoPtr[j].x + InfoPtr[j].Radius;

				if (InfoPtr[j].y - InfoPtr[j].Radius < Ymin) Ymin = InfoPtr[j].y - InfoPtr[j].Radius;
				if (InfoPtr[j].y + InfoPtr[j].Radius > Ymax) Ymax = InfoPtr[j].y + InfoPtr[j].Radius;

				if (InfoPtr[j].z - InfoPtr[j].Radius < Zmin) Zmin = InfoPtr[j].z - InfoPtr[j].Radius;
				if (InfoPtr[j].z + InfoPtr[j].Radius > Zmax) Zmax = InfoPtr[j].z + InfoPtr[j].Radius;

				BigInfoPtr[cubecount] = InfoPtr[j];
				cubecount++;
			}
		}

// calc big cube centre

		cx = (Xmin + Xmax) / 2;
		cy = (Ymin + Ymax) / 2;
		cz = (Zmin + Zmax) / 2;

// calc big cube radius

		rad = 0;
		for (j = 0 ; j < BigPtr[i].Count ; j++)
		{
			rx = BigInfoPtr[j].x - cx;
			ry = BigInfoPtr[j].y - cy;
			rz = BigInfoPtr[j].z - cz;
			dist = (float)sqrt(rx * rx + ry * ry + rz * rz) + BigInfoPtr[j].Radius;
			if (dist > rad) rad = dist;
		}

// write big cube centre + radius

		if (n64)
		{
			fputfloat(cx, ofile);
			fputfloat(cy, ofile);
			fputfloat(cz, ofile);
			fputfloat(rad, ofile);
		}
		else if (psx)
		{
			x16 = (short)cx;
			y16 = (short)cy;
			z16 = (short)cz;
			temp16 = (short)rad;
			fwrite(&x16, sizeof(x16), 1, ofile);
			fwrite(&y16, sizeof(y16), 1, ofile);
			fwrite(&z16, sizeof(z16), 1, ofile);
			fwrite(&temp16, sizeof(temp16), 1, ofile);
		}
		else
		{
			fwrite(&cx, sizeof(cx), 1, ofile);
			fwrite(&cy, sizeof(cy), 1, ofile);
			fwrite(&cz, sizeof(cz), 1, ofile);
			fwrite(&rad, sizeof(rad), 1, ofile);
		}

// write big cube count

		if (n64)
		{
			fputlong(BigPtr[i].Count, ofile);
		}
		else if (psx)
		{
			temp16 = (short)BigPtr[i].Count;
			fwrite(&temp16, sizeof(temp16), 1, ofile);
		}
		else
		{
			fwrite(&BigPtr[i].Count, sizeof(BigPtr[i].Count), 1, ofile);
		}

// write out small cube indexes

		for (j = 0 ; j < BigPtr[i].Count ; j++)
		{
			if (n64)
			{	
				fputlong(BigInfoPtr[j].Cube, ofile);
			}
			else if (psx)
			{
				temp16 = (short)BigInfoPtr[j].Cube;
				fwrite(&temp16, sizeof(temp16), 1, ofile);
			}
			else
			{
				fwrite(&BigInfoPtr[j].Cube, sizeof(BigInfoPtr[j].Cube), 1, ofile);
			}
		}

// free cube handle

		LocalFree(BigInfoHandle);
	}

// free big cube mem

	LocalFree(InfoHandle);
	LocalFree(BigHandle);

// write out texture anims

	OutputAnimatingTextures();

// write env rgb's + free mem

	wsprintf(buf, "Env RGB's = %d\n", EnvNum);
	api.Log(buf);

	fwrite(EnvPtr, sizeof(long), EnvNum, ofile);
	LocalFree(EnvHandle);
}

/////////////////////////////
// output a collision file //
/////////////////////////////

void OutputCollision(void)
	{
	short i, j, vcount;
	POLY *p;
	VERTEX refpoint;
	VERTEX normal;
	VERTEX *u, *v;
	float plane[4], len;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
	long flag;

// write poly num

	fwrite(&PolyNum, sizeof(PolyNum), 1, ofile);

// loop thru

	p = PolyPtr;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// write out flag

		flag = p->Type & TYPE_QUAD;
		fwrite(&flag, sizeof(long), 1, ofile);

// write out plane coefficients

		vcount = (short)(3 + (p->Type & TYPE_QUAD));
		normal.x = normal.y = normal.z = 0;
		refpoint.x = refpoint.y = refpoint.z = 0;

		for (j = 0 ; j < vcount ; j++)
		{
			u = &p->v[j];
			v = &p->v[(j + 1) % vcount];

			normal.x += (v->y - u->y) * (v->z + u->z);
			normal.y += (v->z - u->z) * (v->x + u->x);
			normal.z += (v->x - u->x) * (v->y + u->y);

			refpoint.x += u->x;
			refpoint.y += u->y;
			refpoint.z += u->z;
		}

		len = (float)sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
		plane[0] = normal.x / len;
		plane[1] = normal.y / len;
		plane[2] = normal.z / len;

		len *= vcount;
		plane[3] = -(refpoint.x * normal.x + refpoint.y * normal.y + refpoint.z * normal.z) / len;

		fwrite(plane, sizeof(float), 4, ofile);

// write out poly centre

/*		refpoint.x /= vcount;
		refpoint.y /= vcount;
		refpoint.z /= vcount;

		fwrite((float*)&refpoint, sizeof(float), 3, ofile);

// write out poly squared radius

		rad = 0;

		for (j = 0 ; j < vcount ; j++)
		{
			dx = p->v[j].x - refpoint.x;
			dy = p->v[j].y - refpoint.y;
			dz = p->v[j].z - refpoint.z;
			len = (float)sqrt((double)(dx * dx + dy * dy + dz * dz));
			if (len > rad) rad = len;
		}

		rad *= rad;
		fwrite(&rad, sizeof(float), 1, ofile);*/

// write out bounding box

		MinX = MaxX = p->v[0].x;
		MinY = MaxY = p->v[0].y;
		MinZ = MaxZ = p->v[0].z;

		for (j = 1 ; j < vcount ; j++)
		{
			if (p->v[j].x < MinX) MinX = p->v[j].x;
			if (p->v[j].x > MaxX) MaxX = p->v[j].x;
			if (p->v[j].y < MinY) MinY = p->v[j].y;
			if (p->v[j].y > MaxY) MaxY = p->v[j].y;
			if (p->v[j].z < MinZ) MinZ = p->v[j].z;
			if (p->v[j].z > MaxZ) MaxZ = p->v[j].z;
		}	

		fwrite(&MinX, sizeof(float), 1, ofile);
		fwrite(&MaxX, sizeof(float), 1, ofile);
		fwrite(&MinY, sizeof(float), 1, ofile);
		fwrite(&MaxY, sizeof(float), 1, ofile);
		fwrite(&MinZ, sizeof(float), 1, ofile);
		fwrite(&MaxZ, sizeof(float), 1, ofile);

// write out verts

		fwrite((float*)&p->v[0], sizeof(float), 3, ofile);
		fwrite((float*)&p->v[1], sizeof(float), 3, ofile);
		fwrite((float*)&p->v[2], sizeof(float), 3, ofile);
		fwrite((float*)&p->v[3], sizeof(float), 3, ofile);
	}
}

/////////////////////////////////
// output a new collision file //
/////////////////////////////////

void OutputNewCollision(void)
	{
	short sh;
	long i, j, k, vcount, count;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ, f, z, x;
	char buf[128];
	POLY *p;
	VERTEX *a, *b;
	VECTOR v0, v1, v2;
	NEWCOLLPOLY collpoly, *ncoll;
	NEWCOLLPOLY_PSX collpolypsx;
	HLOCAL handle;
	GRID_HEADER header;
	GRID_HEADER_PSX headerpsx;

// alloc ram for list

	handle = LocalAlloc(LMEM_FIXED, sizeof(NEWCOLLPOLY) * PolyNum);
	ncoll = (NEWCOLLPOLY*)LocalLock(handle);

// write poly num

	if (n64)
	{
		fputshort(PolyNum, ofile);
	}
	/*else if (psx)
	{
		i = (long)PolyNum;
		fwrite(&i, sizeof(i), 1, ofile);
	}*/
	else
	{
		fwrite(&PolyNum, sizeof(PolyNum), 1, ofile);
	}

// loop thru

	p = PolyPtr;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// get vert count

		vcount = (short)(3 + (p->Type & TYPE_QUAD));

// set flag

		collpoly.Type = p->Type;

// set material

		collpoly.Material = p->Material;

// create main plane

		v0.x = p->v[0].x;
		v0.y = p->v[0].y;
		v0.z = p->v[0].z;

		v1.x = p->v[1].x;
		v1.y = p->v[1].y;
		v1.z = p->v[1].z;

		v2.x = p->v[2].x;
		v2.y = p->v[2].y;
		v2.z = p->v[2].z;

		BuildPlane(&v0, &v1, &v2, &collpoly.Plane);

// create edge planes

		for (j = 0 ; j < vcount ; j++)
		{
			a = &p->v[j];
			b = &p->v[(j + 1) % vcount];

			v0.x = a->x;
			v0.y = a->y;
			v0.z = a->z;

			v1.x = b->x;
			v1.y = b->y;
			v1.z = b->z;

			v2.x = (v0.x + v1.x) / 2 - collpoly.Plane.a * 64;
			v2.y = (v0.y + v1.y) / 2 - collpoly.Plane.b * 64;
			v2.z = (v0.z + v1.z) / 2 - collpoly.Plane.c * 64;

			BuildPlane(&v2, &v1, &v0, &collpoly.EdgePlane[j]);
		}

// calc bounding box

		MinX = MaxX = p->v[0].x;
		MinY = MaxY = p->v[0].y;
		MinZ = MaxZ = p->v[0].z;

		for (j = 1 ; j < vcount ; j++)
		{
			if (p->v[j].x < MinX) MinX = p->v[j].x;
			if (p->v[j].x > MaxX) MaxX = p->v[j].x;
			if (p->v[j].y < MinY) MinY = p->v[j].y;
			if (p->v[j].y > MaxY) MaxY = p->v[j].y;
			if (p->v[j].z < MinZ) MinZ = p->v[j].z;
			if (p->v[j].z > MaxZ) MaxZ = p->v[j].z;
		}	

		collpoly.BBox.XMin = MinX;
		collpoly.BBox.XMax = MaxX;
		collpoly.BBox.YMin = MinY;
		collpoly.BBox.YMax = MaxY;
		collpoly.BBox.ZMin = MinZ;
		collpoly.BBox.ZMax = MaxZ;

// write

		if (n64)
		{
			fputlong(collpoly.Type, ofile);
			fputlong(collpoly.Material, ofile);
			fputfloat(collpoly.Plane.a, ofile);
			fputfloat(collpoly.Plane.b, ofile);
			fputfloat(collpoly.Plane.c, ofile);
			fputfloat(collpoly.Plane.d, ofile);
			for (j = 0; j < 4; j++)
			{
				fputfloat(collpoly.EdgePlane[j].a, ofile);
				fputfloat(collpoly.EdgePlane[j].b, ofile);
				fputfloat(collpoly.EdgePlane[j].c, ofile);
				fputfloat(collpoly.EdgePlane[j].d, ofile);
			}
			fputfloat(collpoly.BBox.XMin, ofile);
			fputfloat(collpoly.BBox.XMax, ofile);
			fputfloat(collpoly.BBox.YMin, ofile);
			fputfloat(collpoly.BBox.YMax, ofile);
			fputfloat(collpoly.BBox.ZMin, ofile);
			fputfloat(collpoly.BBox.ZMax, ofile);
		}
		else if (psx)
		{
			collpolypsx.Type = collpoly.Type;
			collpolypsx.Material = (long)collpoly.Material;

			collpolypsx.Plane.a = (long)(collpoly.Plane.a * 65536);
			collpolypsx.Plane.b = (long)(collpoly.Plane.b * 65536);
			collpolypsx.Plane.c = (long)(collpoly.Plane.c * 65536);
			collpolypsx.Plane.d = TO_LENGTH((long)(collpoly.Plane.d * 65536));

			for (j = 0 ; j < 4 ; j++)
			{
				collpolypsx.EdgePlane[j].a = (long)(collpoly.EdgePlane[j].a * 65536);
				collpolypsx.EdgePlane[j].b = (long)(collpoly.EdgePlane[j].b * 65536);
				collpolypsx.EdgePlane[j].c = (long)(collpoly.EdgePlane[j].c * 65536);
				collpolypsx.EdgePlane[j].d = TO_LENGTH((long)(collpoly.EdgePlane[j].d * 65536));
			}

			collpolypsx.BBox.XMin = TO_LENGTH((long)(collpoly.BBox.XMin * 65536));
			collpolypsx.BBox.XMax = TO_LENGTH((long)(collpoly.BBox.XMax * 65536));
			collpolypsx.BBox.YMin = TO_LENGTH((long)(collpoly.BBox.YMin * 65536));
			collpolypsx.BBox.YMax = TO_LENGTH((long)(collpoly.BBox.YMax * 65536));
			collpolypsx.BBox.ZMin = TO_LENGTH((long)(collpoly.BBox.ZMin * 65536));
			collpolypsx.BBox.ZMax = TO_LENGTH((long)(collpoly.BBox.ZMax * 65536));

			fwrite(&collpolypsx, sizeof(collpolypsx), 1, ofile);
		}
		else
		{
			fwrite(&collpoly, sizeof(collpoly), 1, ofile);
		}

// save for grid use

		ncoll[i] = collpoly;
	}

// quit now if no grid

	if (!NewCollGridSize)
	{
		LocalFree(handle);
		return;
	}

// ok, calc XZ bounding box of all collpolys

	MinX = MinZ = 999999;
	MaxX = MaxZ = -999999;

	for (i = 0 ; i < PolyNum ; i++)
	{
		if (ncoll[i].BBox.XMin < MinX) MinX = ncoll[i].BBox.XMin;
		if (ncoll[i].BBox.XMax > MaxX) MaxX = ncoll[i].BBox.XMax;

		if (ncoll[i].BBox.ZMin < MinZ) MinZ = ncoll[i].BBox.ZMin;
		if (ncoll[i].BBox.ZMax > MaxZ) MaxZ = ncoll[i].BBox.ZMax;
	}

// calc XZ starting point + dimensions

	i = (long)(MinX / NewCollGridSize);
	header.Xstart = (float)(i * NewCollGridSize) - NewCollGridSize;
	i = (long)(MinZ / NewCollGridSize);
	header.Zstart = (float)(i * NewCollGridSize) - NewCollGridSize;

// calc XZ dimensions

	i = (long)(MaxX / NewCollGridSize);
	f = (float)(i * NewCollGridSize) + NewCollGridSize;
	header.Xnum = (f - header.Xstart) / NewCollGridSize;
	i = (long)(MaxZ / NewCollGridSize);
	f = (float)(i * NewCollGridSize) + NewCollGridSize;
	header.Znum = (f - header.Zstart) / NewCollGridSize;

// display info

	if (!psx) {
		wsprintf(buf, "\nOutputting new coll grid:  X %d, Z %d, Width %d, Depth %d\n", (long)header.Xstart, (long)header.Zstart, (long)header.Xnum, (long)header.Znum);
		api.Log(buf);
	} else {
		wsprintf(buf, "\nOutputting new coll grid:  X %d, Z %d, Width %d, Depth %d\n", TO_LENGTH((long)header.Xstart), TO_LENGTH((long)header.Zstart), (long)header.Xnum, (long)header.Znum);
		api.Log(buf);
	}

// write header

	header.GridSize = NewCollGridSize;
	if (n64)
	{
		fputfloat(header.Xstart, ofile);
		fputfloat(header.Zstart, ofile);
		fputfloat(header.Xnum, ofile);
		fputfloat(header.Znum, ofile);
		fputfloat(header.GridSize, ofile);
	}
	else if (psx)
	{
		headerpsx.Xstart = TO_LENGTH((long)(header.Xstart * 65536));
		headerpsx.Zstart = TO_LENGTH((long)(header.Zstart * 65536));
		headerpsx.Znum = (long)(header.Znum * 65536);
		headerpsx.Xnum = (long)(header.Xnum * 65536);
		headerpsx.GridSize = TO_LENGTH((long)(header.GridSize * 65536));
		fwrite(&headerpsx, sizeof(headerpsx), 1, ofile);
	}
	else
	{
		fwrite(&header, sizeof(header), 1, ofile);
	}

// loop thru all grid entries

	for (z = header.Zstart ; z < (header.Zstart + header.Znum * NewCollGridSize) ; z += NewCollGridSize)
	{
		for (x = header.Xstart ; x < (header.Xstart + header.Xnum * NewCollGridSize) ; x += NewCollGridSize)
		{

// set bounding box for grid

			MinX = x - GridExpand;
			MaxX = x + NewCollGridSize - 1 + GridExpand;

			MinZ = z - GridExpand;
			MaxZ = z + NewCollGridSize - 1 + GridExpand;

// 2 pass test

			count = 0;
			for (j = 0 ; j < 2 ; j++)
			{
				for (k = 0 ; k < PolyNum ; k++)
				{

// check one poly against grid

					if (MinX > ncoll[k].BBox.XMax || MaxX < ncoll[k].BBox.XMin ||
						MinZ > ncoll[k].BBox.ZMax || MaxZ < ncoll[k].BBox.ZMin)
							continue;

// passed, inc count first pass, output second pass

					if (!j)
					{
						count++;
					}
					else
					{
						if (n64)
						{
							fputlong(k, ofile);
						}
						else if (psx)
						{
							sh = (short)k;
							fwrite(&sh, sizeof(sh), 1, ofile);
						}
						else
						{
							fwrite(&k, sizeof(k), 1, ofile);
						}
					}
				}

// output count if end of first pass

				if (!j)
				{
					if (n64)
					{
						fputlong(count, ofile);
					}
					else if (psx)
					{
						sh = (short)count;
						fwrite(&sh, sizeof(sh), 1, ofile);
					}
					else
					{
						fwrite(&count, sizeof(count), 1, ofile);
					}
				}
			}
		}
	}

// free ram

	LocalFree(handle);
}

void OutputNewCollisionTris(void)
	{
	long i, j, k, vcount, count, iPoly, quadCount;
	short totPolys;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ, f, z, x;
	char buf[128];
	POLY *p;
	VERTEX *a, *b;
	VECTOR v0, v1, v2;
	NEWCOLLPOLY collpoly, *ncoll;
	NEWCOLLPOLY_PSX collpolypsx;
	HLOCAL handle;
	GRID_HEADER header;
	GRID_HEADER_PSX headerpsx;

	p = PolyPtr;

// count the number of quads in the poly list
	quadCount = 0;
	for (i = 0; i < PolyNum; i++) {
		if (p->Type & TYPE_QUAD) {
			quadCount++;
		}
		p++;
	}

	totPolys = PolyNum + quadCount;

// alloc ram for list

	handle = LocalAlloc(LMEM_FIXED, sizeof(NEWCOLLPOLY) * totPolys);
	ncoll = (NEWCOLLPOLY*)LocalLock(handle);

// write poly num

	if (n64)
	{
		fputshort(totPolys, ofile);
	}
	else if (psx)
	{
		i = (long)totPolys;
		fwrite(&i, sizeof(i), 1, ofile);
	}
	else
	{
		fwrite(&totPolys, sizeof(totPolys), 1, ofile);
	}

// loop thru

	p = PolyPtr;
	iPoly = 0;
	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// get vert count

		vcount = (short)(3 + (p->Type & TYPE_QUAD));

// set flag

		//collpoly.Type = p->Type;
		collpoly.Type = 0;

// set material

		collpoly.Material = p->Material;

// create main plane

		v0.x = p->v[0].x;
		v0.y = p->v[0].y;
		v0.z = p->v[0].z;

		v1.x = p->v[1].x;
		v1.y = p->v[1].y;
		v1.z = p->v[1].z;

		v2.x = p->v[2].x;
		v2.y = p->v[2].y;
		v2.z = p->v[2].z;

		BuildPlane(&v0, &v1, &v2, &collpoly.Plane);

// create edge planes

		for (j = 0 ; j < 3 ; j++)
		{
			a = &p->v[j];
			b = &p->v[(j + 1) % 3];

			v0.x = a->x;
			v0.y = a->y;
			v0.z = a->z;

			v1.x = b->x;
			v1.y = b->y;
			v1.z = b->z;

			v2.x = (v0.x + v1.x) / 2 - collpoly.Plane.a * 64;
			v2.y = (v0.y + v1.y) / 2 - collpoly.Plane.b * 64;
			v2.z = (v0.z + v1.z) / 2 - collpoly.Plane.c * 64;

			BuildPlane(&v2, &v1, &v0, &collpoly.EdgePlane[j]);
		}

// calc bounding box

		MinX = MaxX = p->v[0].x;
		MinY = MaxY = p->v[0].y;
		MinZ = MaxZ = p->v[0].z;

		for (j = 1 ; j < vcount ; j++)
		{
			if (p->v[j].x < MinX) MinX = p->v[j].x;
			if (p->v[j].x > MaxX) MaxX = p->v[j].x;
			if (p->v[j].y < MinY) MinY = p->v[j].y;
			if (p->v[j].y > MaxY) MaxY = p->v[j].y;
			if (p->v[j].z < MinZ) MinZ = p->v[j].z;
			if (p->v[j].z > MaxZ) MaxZ = p->v[j].z;
		}	

		collpoly.BBox.XMin = MinX;
		collpoly.BBox.XMax = MaxX;
		collpoly.BBox.YMin = MinY;
		collpoly.BBox.YMax = MaxY;
		collpoly.BBox.ZMin = MinZ;
		collpoly.BBox.ZMax = MaxZ;

// write

		if (n64)
		{
			fputlong(collpoly.Type, ofile);
			fputlong(collpoly.Material, ofile);
			fputfloat(collpoly.Plane.a, ofile);
			fputfloat(collpoly.Plane.b, ofile);
			fputfloat(collpoly.Plane.c, ofile);
			fputfloat(collpoly.Plane.d, ofile);
			for (j = 0; j < 4; j++)
			{
				fputfloat(collpoly.EdgePlane[j].a, ofile);
				fputfloat(collpoly.EdgePlane[j].b, ofile);
				fputfloat(collpoly.EdgePlane[j].c, ofile);
				fputfloat(collpoly.EdgePlane[j].d, ofile);
			}
			fputfloat(collpoly.BBox.XMin, ofile);
			fputfloat(collpoly.BBox.XMax, ofile);
			fputfloat(collpoly.BBox.YMin, ofile);
			fputfloat(collpoly.BBox.YMax, ofile);
			fputfloat(collpoly.BBox.ZMin, ofile);
			fputfloat(collpoly.BBox.ZMax, ofile);
		}
		else if (psx)
		{
			collpolypsx.Type = collpoly.Type;
			collpolypsx.Material = (long)collpoly.Material;

			collpolypsx.Plane.a = (long)(collpoly.Plane.a * 65536);
			collpolypsx.Plane.b = (long)(collpoly.Plane.b * 65536);
			collpolypsx.Plane.c = (long)(collpoly.Plane.c * 65536);
			collpolypsx.Plane.d = (long)(collpoly.Plane.d * 65536);

			for (j = 0 ; j < 4 ; j++)
			{
				collpolypsx.EdgePlane[j].a = (long)(collpoly.EdgePlane[j].a * 65536);
				collpolypsx.EdgePlane[j].b = (long)(collpoly.EdgePlane[j].b * 65536);
				collpolypsx.EdgePlane[j].c = (long)(collpoly.EdgePlane[j].c * 65536);
				collpolypsx.EdgePlane[j].d = (long)(collpoly.EdgePlane[j].d * 65536);
			}

			collpolypsx.BBox.XMin = (long)(collpoly.BBox.XMin * 65536);
			collpolypsx.BBox.XMax = (long)(collpoly.BBox.XMax * 65536);
			collpolypsx.BBox.YMin = (long)(collpoly.BBox.YMin * 65536);
			collpolypsx.BBox.YMax = (long)(collpoly.BBox.YMax * 65536);
			collpolypsx.BBox.ZMin = (long)(collpoly.BBox.ZMin * 65536);
			collpolypsx.BBox.ZMax = (long)(collpoly.BBox.ZMax * 65536);

			fwrite(&collpolypsx, sizeof(collpolypsx), 1, ofile);
		}
		else
		{
			fwrite(&collpoly, sizeof(collpoly), 1, ofile);
		}

// save for grid use

		ncoll[iPoly++] = collpoly;

// Second poly when splitting quads

		if (p->Type & TYPE_QUAD) {

	// set flag

			collpoly.Type = 0; //p->Type;

	// set material

			collpoly.Material = p->Material;

	// create main plane

			v0.x = p->v[0].x;
			v0.y = p->v[0].y;
			v0.z = p->v[0].z;

			v1.x = p->v[2].x;
			v1.y = p->v[2].y;
			v1.z = p->v[2].z;

			v2.x = p->v[3].x;
			v2.y = p->v[3].y;
			v2.z = p->v[3].z;

			BuildPlane(&v0, &v1, &v2, &collpoly.Plane);

	// create edge planes

			for (j = 0 ; j < 3 ; j++)
			{
				if (j == 0) {
					a = &p->v[0];
					b = &p->v[2];
				} else if (j == 1) {
					a = &p->v[2];
					b = &p->v[3];
				} else if (j == 2) {
					a = &p->v[3];
					b = &p->v[0];
				}

				v0.x = a->x;
				v0.y = a->y;
				v0.z = a->z;

				v1.x = b->x;
				v1.y = b->y;
				v1.z = b->z;

				v2.x = (v0.x + v1.x) / 2 - collpoly.Plane.a * 64;
				v2.y = (v0.y + v1.y) / 2 - collpoly.Plane.b * 64;
				v2.z = (v0.z + v1.z) / 2 - collpoly.Plane.c * 64;

				BuildPlane(&v2, &v1, &v0, &collpoly.EdgePlane[j]);
			}

	// calc bounding box

			MinX = MaxX = p->v[0].x;
			MinY = MaxY = p->v[0].y;
			MinZ = MaxZ = p->v[0].z;

			for (j = 1 ; j < vcount ; j++)
			{
				if (p->v[j].x < MinX) MinX = p->v[j].x;
				if (p->v[j].x > MaxX) MaxX = p->v[j].x;
				if (p->v[j].y < MinY) MinY = p->v[j].y;
				if (p->v[j].y > MaxY) MaxY = p->v[j].y;
				if (p->v[j].z < MinZ) MinZ = p->v[j].z;
				if (p->v[j].z > MaxZ) MaxZ = p->v[j].z;
			}	

			collpoly.BBox.XMin = MinX;
			collpoly.BBox.XMax = MaxX;
			collpoly.BBox.YMin = MinY;
			collpoly.BBox.YMax = MaxY;
			collpoly.BBox.ZMin = MinZ;
			collpoly.BBox.ZMax = MaxZ;

	// write

			if (n64)
			{
				fputlong(collpoly.Type, ofile);
				fputlong(collpoly.Material, ofile);
				fputfloat(collpoly.Plane.a, ofile);
				fputfloat(collpoly.Plane.b, ofile);
				fputfloat(collpoly.Plane.c, ofile);
				fputfloat(collpoly.Plane.d, ofile);
				for (j = 0; j < 4; j++)
				{
					fputfloat(collpoly.EdgePlane[j].a, ofile);
					fputfloat(collpoly.EdgePlane[j].b, ofile);
					fputfloat(collpoly.EdgePlane[j].c, ofile);
					fputfloat(collpoly.EdgePlane[j].d, ofile);
				}
				fputfloat(collpoly.BBox.XMin, ofile);
				fputfloat(collpoly.BBox.XMax, ofile);
				fputfloat(collpoly.BBox.YMin, ofile);
				fputfloat(collpoly.BBox.YMax, ofile);
				fputfloat(collpoly.BBox.ZMin, ofile);
				fputfloat(collpoly.BBox.ZMax, ofile);
			}
			else if (psx)
			{
				collpolypsx.Type = collpoly.Type;
				collpolypsx.Material = (long)collpoly.Material;

				collpolypsx.Plane.a = (long)(collpoly.Plane.a * 65536);
				collpolypsx.Plane.b = (long)(collpoly.Plane.b * 65536);
				collpolypsx.Plane.c = (long)(collpoly.Plane.c * 65536);
				collpolypsx.Plane.d = (long)(collpoly.Plane.d * 65536);

				for (j = 0 ; j < 4 ; j++)
				{
					collpolypsx.EdgePlane[j].a = (long)(collpoly.EdgePlane[j].a * 65536);
					collpolypsx.EdgePlane[j].b = (long)(collpoly.EdgePlane[j].b * 65536);
					collpolypsx.EdgePlane[j].c = (long)(collpoly.EdgePlane[j].c * 65536);
					collpolypsx.EdgePlane[j].d = (long)(collpoly.EdgePlane[j].d * 65536);
				}

				collpolypsx.BBox.XMin = (long)(collpoly.BBox.XMin * 65536);
				collpolypsx.BBox.XMax = (long)(collpoly.BBox.XMax * 65536);
				collpolypsx.BBox.YMin = (long)(collpoly.BBox.YMin * 65536);
				collpolypsx.BBox.YMax = (long)(collpoly.BBox.YMax * 65536);
				collpolypsx.BBox.ZMin = (long)(collpoly.BBox.ZMin * 65536);
				collpolypsx.BBox.ZMax = (long)(collpoly.BBox.ZMax * 65536);

				fwrite(&collpolypsx, sizeof(collpolypsx), 1, ofile);
			}
			else
			{
				fwrite(&collpoly, sizeof(collpoly), 1, ofile);
			}

	// save for grid use

			ncoll[iPoly++] = collpoly;
		}
	}

// quit now if no grid

	if (!NewCollGridSize)
	{
		LocalFree(handle);
		return;
	}

// ok, calc XZ bounding box of all collpolys

	MinX = MinZ = 999999;
	MaxX = MaxZ = -999999;

	for (i = 0 ; i < totPolys ; i++)
	{
		if (ncoll[i].BBox.XMin < MinX) MinX = ncoll[i].BBox.XMin;
		if (ncoll[i].BBox.XMax > MaxX) MaxX = ncoll[i].BBox.XMax;

		if (ncoll[i].BBox.ZMin < MinZ) MinZ = ncoll[i].BBox.ZMin;
		if (ncoll[i].BBox.ZMax > MaxZ) MaxZ = ncoll[i].BBox.ZMax;
	}

// calc XZ starting point + dimensions

	i = (long)(MinX / NewCollGridSize);
	header.Xstart = (float)(i * NewCollGridSize) - NewCollGridSize;
	i = (long)(MinZ / NewCollGridSize);
	header.Zstart = (float)(i * NewCollGridSize) - NewCollGridSize;

// calc XZ dimensions

	i = (long)(MaxX / NewCollGridSize);
	f = (float)(i * NewCollGridSize) + NewCollGridSize;
	header.Xnum = (f - header.Xstart) / NewCollGridSize;
	i = (long)(MaxZ / NewCollGridSize);
	f = (float)(i * NewCollGridSize) + NewCollGridSize;
	header.Znum = (f - header.Zstart) / NewCollGridSize;

// display info

	wsprintf(buf, "\nOutputting new coll grid:  X %d, Z %d, Width %d, Depth %d\n", (long)header.Xstart, (long)header.Zstart, (long)header.Xnum, (long)header.Znum);
	api.Log(buf);

// write header

	header.GridSize = NewCollGridSize;
	if (n64)
	{
		fputfloat(header.Xstart, ofile);
		fputfloat(header.Zstart, ofile);
		fputfloat(header.Xnum, ofile);
		fputfloat(header.Znum, ofile);
		fputfloat(header.GridSize, ofile);
	}
	else if (psx)
	{
		headerpsx.Xstart = (long)(header.Xstart * 65536);
		headerpsx.Zstart = (long)(header.Zstart * 65536);
		headerpsx.Znum = (long)header.Znum;
		headerpsx.Xnum = (long)header.Xnum;
		headerpsx.GridSize = (long)(header.GridSize * 65536);
		fwrite(&headerpsx, sizeof(headerpsx), 1, ofile);
	}
	else
	{
		fwrite(&header, sizeof(header), 1, ofile);
	}

// loop thru all grid entries

	for (z = header.Zstart ; z < (header.Zstart + header.Znum * NewCollGridSize) ; z += NewCollGridSize)
	{
		for (x = header.Xstart ; x < (header.Xstart + header.Xnum * NewCollGridSize) ; x += NewCollGridSize)
		{

// set bounding box for grid

			MinX = x - GridExpand;
			MaxX = x + NewCollGridSize - 1 + GridExpand;

			MinZ = z - GridExpand;
			MaxZ = z + NewCollGridSize - 1 + GridExpand;

// 2 pass test

			count = 0;
			for (j = 0 ; j < 2 ; j++)
			{
				for (k = 0 ; k < totPolys ; k++)
				{

// check one poly against grid

					if (MinX > ncoll[k].BBox.XMax || MaxX < ncoll[k].BBox.XMin ||
						MinZ > ncoll[k].BBox.ZMax || MaxZ < ncoll[k].BBox.ZMin)
							continue;

// passed, inc count first pass, output second pass

					if (!j)
					{
						count++;
					}
					else
					{
						if (n64)
						{
							fputlong(k, ofile);
						}
						else
						{
							fwrite(&k, sizeof(k), 1, ofile);
						}
					}
				}

// output count if end of first pass

				if (!j)
				{
					if (n64)
					{
						fputlong(count, ofile);
					}
					else
					{
						fwrite(&count, sizeof(count), 1, ofile);
					}
				}
			}
		}
	}

// free ram

	LocalFree(handle);
}

///////////////////////
// find face normals //
///////////////////////

void FindFaceNormals(void)
{
	long i;
	float v0[3], v1[3], len;
	POLY *p;

// get face normals

	p = PolyPtr;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{
		v0[0] = p->v[1].x - p->v[0].x;
		v0[1] = p->v[1].y - p->v[0].y;
		v0[2] = p->v[1].z - p->v[0].z;

		v1[0] = p->v[2].x - p->v[0].x;
		v1[1] = p->v[2].y - p->v[0].y;
		v1[2] = p->v[2].z - p->v[0].z;

		CrossProduct(v1, v0, &p->nx);

		len = (float)sqrt(p->nx * p->nx + p->ny * p->ny + p->nz * p->nz);
		p->nx /= len;
		p->ny /= len;
		p->nz /= len;
	}
}

//////////////////
// find normals //
//////////////////

void FindNormals(void)
{
	float len;
	short i, j, vcount;
	POLY *p;
	VERTEX *vert;

// zero vert normals

	vert = VertPtr;
	for (i = 0 ; i < VertNum ; i++, vert++)
	{
		vert->nx = 0;
		vert->ny = 0;
		vert->nz = 0;
	}

// build normals

	p = PolyPtr;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// add face normal to it's vert normals

		vcount = (short)(3 + (p->Type & TYPE_QUAD));

		for (j = 0 ; j < vcount ; j++)
		{
			(VertPtr + p->vi[j])->nx += p->nx;
			(VertPtr + p->vi[j])->ny += p->ny;
			(VertPtr + p->vi[j])->nz += p->nz;
		}
	}

// normalize vert normals

	vert = VertPtr;
	for (i = 0 ; i < VertNum ; i++, vert++)
	{
		len = (float)sqrt(vert->nx * vert->nx + vert->ny * vert->ny + vert->nz * vert->nz);

		vert->nx /= len;
		vert->ny /= len;
		vert->nz /= len;
	}

// copy vert normals to face vert normals

	p = PolyPtr;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{
		vcount = (short)(3 + (p->Type & TYPE_QUAD));

		for (j = 0 ; j < vcount ; j++)
		{
			p->v[j].nx = (VertPtr + p->vi[j])->nx;
			p->v[j].ny = (VertPtr + p->vi[j])->ny;
			p->v[j].nz = (VertPtr + p->vi[j])->nz;
		}
	}
}

////////////////////////
// calc world gouraud //
////////////////////////

void CalcWorldGouraud(void)
{
	float len, intensity, vec[3];
	short i, j, vcount;
	POLY *p;
	VERTEX *vert;

// add gouraud

	vert = VertPtr;
	for (i = 0 ; i < VertNum ; i++, vert++)
	{

// get light normal

		vec[0] = vert->x - Lx;
		vec[1] = vert->y - Ly;
		vec[2] = vert->z - Lz;

		len = (float)sqrt((double)vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);

		vec[0] /= len;
		vec[1] /= len;
		vec[2] /= len;

// calc intensity / RGB

		intensity = vert->nx * vec[0] + vert->ny * vec[1] + vert->nz * vec[2];
 		intensity = (float)(acos(intensity) / 1.57079632679) - 1;
		if (intensity < 0) intensity = 0;

		vert->r = (unsigned char)(intensity * Lr);
		vert->g = (unsigned char)(intensity * Lg);
		vert->b = (unsigned char)(intensity * Lb);
	}

// add vertex rgb to face rgb

	p = PolyPtr;
	for (i = 0 ; i < PolyNum ; i++, p++)
	{
		vcount = (short)(3 + (p->Type & TYPE_QUAD));

		for (j = 0 ; j < vcount ; j++)
		{
			vert = (VertPtr + p->vi[j]);
			p->rgb[j] = vert->b | (vert->g << 8) | (vert->r << 16) | (p->rgb[j] & 0xFF000000);
		}
	}
}

/////////////////////
// calc world flat //
/////////////////////

void CalcWorldFlat(void)
	{
	float len, intensity, vec[3];
	short i, j, vcount;
	POLY *p;
	unsigned char r, g, b;
	unsigned long col;
	float centre[3];

// calc shade

	p = PolyPtr;
	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// skip if textured?

		if (Flat == 1 && p->Tpage != -1) continue;

// get light normal

		vcount = (short)(3 + (p->Type & TYPE_QUAD));
		centre[0] = centre[1] = centre[2] = 0;

		for (j = 0 ; j < vcount ; j++)
		{
			centre[0] += p->v[j].x;
			centre[1] += p->v[j].y;
			centre[2] += p->v[j].z;
		}

		centre[0] /= vcount;
		centre[1] /= vcount;
		centre[2] /= vcount;

		vec[0] = centre[0] - Lx;
		vec[1] = centre[1] - Ly;
		vec[2] = centre[2] - Lz;
		len = (float)sqrt((double)vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);

		vec[0] /= len;
		vec[1] /= len;
		vec[2] /= len;

// get rgb's

		intensity = p->nx * vec[0] + p->ny * vec[1] + p->nz * vec[2];
 		intensity = (float)(acos(intensity) / 1.57079632679) - 1;
		if (intensity < 0) intensity = 0;
		r = (unsigned char)(intensity * Lr * 3 / 4 + 64);
		g = (unsigned char)(intensity * Lg * 3 / 4 + 64);
		b = (unsigned char)(intensity * Lb * 3 / 4 + 64);

		col = b | (g << 8) | (r << 16);
		p->rgb[0] = p->rgb[1] = p->rgb[2] = p->rgb[3] = col;
	}
}

/////////////////////////////////////
// sort a model according to tpage //
/////////////////////////////////////

void SortModel(void)
{
	short i, j;
	POLY poly;

// loop thru faces

	if (PolyNum > 1) for (i = (short)(PolyNum - 1) ; i ; i--) for (j = 0 ; j < i ; j++)
	{
		if (PolyPtr[j].Tpage > PolyPtr[j + 1].Tpage)
		{
			poly = PolyPtr[j];
			PolyPtr[j] = PolyPtr[j + 1];
			PolyPtr[j + 1] = poly;
		}
	}
}

//////////////////////////////////////////////////////
// sort a model according to quad / tri + tex / rgb //
//////////////////////////////////////////////////////

void SortModelPsx(void)
{
	short i, j;
	POLY poly;
	long a, b;

// loop thru faces

	if (PolyNum > 1) for (i = (short)(PolyNum - 1) ; i ; i--) for (j = 0 ; j < i ; j++)
	{
		a = PolyPtr[j].Type & TYPE_QUAD;
		if (PolyPtr[j].Tpage != -1) a += 256;
		if (PolyPtr[j].Type & TYPE_PSXMODEL1) a += 512;
		if (PolyPtr[j].Type & TYPE_PSXMODEL2) a += 1024;

		b = PolyPtr[j + 1].Type & TYPE_QUAD;
		if (PolyPtr[j + 1].Tpage != -1) b += 256;
		if (PolyPtr[j + 1].Type & TYPE_PSXMODEL1) b += 512;
		if (PolyPtr[j + 1].Type & TYPE_PSXMODEL2) b += 1024;

		if (b > a)
		{
			poly = PolyPtr[j];
			PolyPtr[j] = PolyPtr[j + 1];
			PolyPtr[j + 1] = poly;
		}
	}
}


/////////////////////////
// output convex hulls //
/////////////////////////

void OutputHulls(void)
{
	long hullgroup, i;
	COLLSKIN_FILEHDR header;
	char buf[256];

// display total hull count

	wsprintf(buf, "Total Hulls = %d\n\n", GroupNum);
	api.Log(buf);

// alloc ram to store bounding box for each hull

	HullBBoxHandle = LocalAlloc(LMEM_FIXED, sizeof(BBOX) * GroupNum);
	HullBBoxPtr = (BBOX*)LocalLock(HullBBoxHandle);

// write out header

	header.NSkins = (INDEX)GroupNum;
	if (n64)
	{
		fputshort(header.NSkins, ofile);
	}
	else
	{
		fwrite(&header, sizeof(header), 1, ofile);
	}

// copy poly list

	HullPolyNum = PolyNum;
	HullPolyHandle = LocalAlloc(LMEM_FIXED, sizeof(POLY) * PolyNum);
	HullPolyPtr = (POLY*)LocalLock(HullPolyHandle);
	memcpy(HullPolyPtr, PolyPtr, sizeof(POLY) * PolyNum);

// loop thru copied poly list

	hullgroup = 0;
	PolyNum = 0;

	for (i = 0 ; i < HullPolyNum ; i++)
	{

// copy poly back to PolyPtr

		memcpy(&PolyPtr[PolyNum], &HullPolyPtr[i], sizeof(POLY));
		PolyNum++;

// last of this hull?

		if (i == HullPolyNum - 1 || HullPolyPtr[i + 1].Group != hullgroup)
		{
			if (!Balls) {
				OutputOneHull(hullgroup);
			} else {
				OutputOneBall(hullgroup);
			}
			hullgroup++;
			PolyNum = 0;
		}
	}

// output grid?

	if (HullGridSize)
	{
		OutputHullGrid();
	}

// free copied poly list

	LocalFree(HullPolyHandle);

// free hull bbox ram

	LocalFree(HullBBoxHandle);
}

/////////////////////
// create one hull //
/////////////////////

void OutputOneHull(long hullnum)
{
	long i, j, k, r, vcount;
	short a, b, c;
	COLLSKIN hull;
	char buf[256];
	HLOCAL phandle, ehandle, fhandle, xhandle;

// weld this hull

	if (!Weld) Weld = 1 * 1;
	WeldModel(FALSE);

// alloc mem for points, edges, planes

	hull.NPts = VertNum;
	phandle = LocalAlloc(LMEM_FIXED, sizeof(VECTOR) * hull.NPts);
	hull.Pts = (VECTOR*)LocalLock(phandle);

	hull.NEdges = 0;
	ehandle = LocalAlloc(LMEM_FIXED, sizeof(EDGE) * PolyNum * 4);
	hull.Edges = (EDGE*)LocalLock(ehandle);

	hull.NFaces = PolyNum;
	fhandle = LocalAlloc(LMEM_FIXED, sizeof(PLANE) * hull.NFaces);
	hull.Faces = (PLANE*)LocalLock(fhandle);

// read points

	for (i = 0 ; i < hull.NPts ; i++)
	{
		hull.Pts[i].x = VertPtr[i].x;
		hull.Pts[i].y = VertPtr[i].y;
		hull.Pts[i].z = VertPtr[i].z;
	}

// create edges from points

	for (i = 0 ; i < PolyNum ; i++)
	{
		vcount = 3 + (PolyPtr[i].Type & TYPE_QUAD);

		for (j = 0 ; j < vcount ; j++)
		{
			a = PolyPtr[i].vi[j];
			b = PolyPtr[i].vi[(j + 1) % vcount];
			if (b < a)
			{
				c = a;
				a = b;
				b = c;
			}
			for (k = 0 ; k < hull.NEdges ; k++)
			{
				if (a == hull.Edges[k].Vtx[0] && b == hull.Edges[k].Vtx[1])
					break;
			}
			if (!hull.NEdges || k == hull.NEdges)
			{
				hull.Edges[hull.NEdges].Vtx[0] = a;
				hull.Edges[hull.NEdges].Vtx[1] = b;
				hull.NEdges++;
			}
		}
	}

// create extra points along the edges
	short totExtra, iExtra, nExtra, totPts;
	REAL edgeLen, dist, scale;
	VECTOR dR, *extraPts;
	totExtra = 0;
	for (i = 0; i < hull.NEdges; i++) {
		VecMinusVec(&hull.Pts[hull.Edges[i].Vtx[1]], &hull.Pts[hull.Edges[i].Vtx[0]], &dR);
		edgeLen = VecLen(&dR);
		totExtra = (short)(totExtra + (short)(edgeLen / HullMinSep));
	}
	xhandle = LocalAlloc(LMEM_FIXED, sizeof(VECTOR) * totExtra);
	extraPts = (VECTOR *)LocalLock(fhandle);

	iExtra = 0;
	for (i = 0; i < hull.NEdges; i++) {
		VecMinusVec(&hull.Pts[hull.Edges[i].Vtx[1]], &hull.Pts[hull.Edges[i].Vtx[0]], &dR);
		edgeLen = VecLen(&dR);
		nExtra = (short) (edgeLen / HullMinSep);
		dist = edgeLen / (nExtra + 1);

		for (j = 0; j < nExtra; j++) {
			scale = (float)(j + 1) / (float)(nExtra + 1);
			VecPlusScalarVec(&hull.Pts[hull.Edges[i].Vtx[0]], scale, &dR, &extraPts[iExtra]);
			iExtra++;
		}
	}
	totPts = hull.NPts + totExtra;

// create planes

	for (i = 0 ; i < hull.NFaces ; i++)
	{
		BuildPlane((VECTOR*)&PolyPtr[i].v[0].x, (VECTOR*)&PolyPtr[i].v[1].x, (VECTOR*)&PolyPtr[i].v[2].x, &hull.Faces[i]);
	}

// create bounding box

	hull.BBox.XMin = hull.BBox.YMin = hull.BBox.ZMin = 999999;
	hull.BBox.XMax = hull.BBox.YMax = hull.BBox.ZMax = -999999;

	for (i = 0 ; i < hull.NPts ; i++)
	{
		if (hull.Pts[i].x < hull.BBox.XMin) hull.BBox.XMin = hull.Pts[i].x;
		if (hull.Pts[i].x > hull.BBox.XMax) hull.BBox.XMax = hull.Pts[i].x;

		if (hull.Pts[i].y < hull.BBox.YMin) hull.BBox.YMin = hull.Pts[i].y;
		if (hull.Pts[i].y > hull.BBox.YMax) hull.BBox.YMax = hull.Pts[i].y;

		if (hull.Pts[i].z < hull.BBox.ZMin) hull.BBox.ZMin = hull.Pts[i].z;
		if (hull.Pts[i].z > hull.BBox.ZMax) hull.BBox.ZMax = hull.Pts[i].z;
	}

// save for later grid use

	HullBBoxPtr[hullnum] = hull.BBox;

// create offset

	hull.Offset.x = (hull.BBox.XMin + hull.BBox.XMax) / 2;
	hull.Offset.y = (hull.BBox.YMin + hull.BBox.YMax) / 2;
	hull.Offset.z = (hull.BBox.ZMin + hull.BBox.ZMax) / 2;

// shift bounding box by negative offset

	hull.BBox.XMin -= hull.Offset.x;
	hull.BBox.XMax -= hull.Offset.x;

	hull.BBox.YMin -= hull.Offset.y;
	hull.BBox.YMax -= hull.Offset.y;

	hull.BBox.ZMin -= hull.Offset.z;
	hull.BBox.ZMax -= hull.Offset.z;

// print stats

	wsprintf(buf, "Hull %d = Points %d + %d = %d, Edges %d, Faces %d\n", hullnum, hull.NPts, totExtra, hull.NPts + totExtra, hull.NEdges, hull.NFaces);
	api.Log(buf);

// write out hull

	if (n64)
	{
		fputshort(totPts, ofile);
		fputshort(hull.NEdges, ofile);
		fputshort(hull.NFaces, ofile);

		fputfloat(hull.BBox.XMin, ofile);
		fputfloat(hull.BBox.XMax, ofile);
		fputfloat(hull.BBox.YMin, ofile);
		fputfloat(hull.BBox.YMax, ofile);
		fputfloat(hull.BBox.ZMin, ofile);
		fputfloat(hull.BBox.ZMax, ofile);

		fputfloat(hull.Offset.x, ofile);
		fputfloat(hull.Offset.y, ofile);
		fputfloat(hull.Offset.z, ofile);
		
		for (i = 0; i < hull.NPts; i++)
		{
			fputfloat(hull.Pts[i].x, ofile);
			fputfloat(hull.Pts[i].y, ofile);
			fputfloat(hull.Pts[i].z, ofile);
		}

		for (i = 0; i < totExtra; i++)
		{
			fputfloat(extraPts[i].x, ofile);
			fputfloat(extraPts[i].y, ofile);
			fputfloat(extraPts[i].z, ofile);
		}

		for (i = 0; i < hull.NEdges; i++)
		{
			fputshort(hull.Edges->Vtx[0], ofile);
			fputshort(hull.Edges->Vtx[1], ofile);
		}
	}
	else if (psx)
	{
		fwrite(&totPts, sizeof(short), 1, ofile);
		fwrite(&hull.NEdges, sizeof(short), 1, ofile);
		fwrite(&hull.NFaces, sizeof(short), 1, ofile);

		r = (long)(hull.BBox.XMin * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.BBox.XMax * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.BBox.YMin * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.BBox.YMax * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.BBox.ZMin * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.BBox.ZMax * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);

		r = (long)(hull.Offset.x * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.Offset.y * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.Offset.z * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		
		for (i = 0; i < hull.NPts; i++)
		{
			r = (long)(hull.Pts[i].x * 65536.0f);
			r = TO_LENGTH(r);
			fwrite(&r, sizeof(r), 1, ofile);
			r = (long)(hull.Pts[i].y * 65536.0f);
			r = TO_LENGTH(r);
			fwrite(&r, sizeof(r), 1, ofile);
			r = (long)(hull.Pts[i].z * 65536.0f);
			r = TO_LENGTH(r);
			fwrite(&r, sizeof(r), 1, ofile);
		}

		for (i = 0; i < hull.NEdges; i++)
		{
			fwrite(&hull.Edges->Vtx[0], sizeof(short), 1, ofile);
			fwrite(&hull.Edges->Vtx[1], sizeof(short), 1, ofile);
		}
	}
	else
	{
		fwrite(&totPts, sizeof(hull.NPts), 1, ofile);
		fwrite(&hull.NEdges, sizeof(hull.NEdges), 1, ofile);
		fwrite(&hull.NFaces, sizeof(hull.NFaces), 1, ofile);

		fwrite(&hull.BBox, sizeof(hull.BBox), 1, ofile);
		fwrite(&hull.Offset, sizeof(hull.Offset), 1, ofile);

		fwrite(hull.Pts, sizeof(VECTOR), hull.NPts, ofile);
		fwrite(extraPts, sizeof(VECTOR), totExtra, ofile);
		fwrite(hull.Edges, sizeof(EDGE), hull.NEdges, ofile);
	}

	for (i = 0 ; i < hull.NFaces ; i++)
	{
		wsprintf(buf, "     Face %d:  %ld, %ld, %ld, %ld\n", i, (long)(hull.Faces[i].a * 1000), (long)(hull.Faces[i].b * 1000), (long)(hull.Faces[i].c * 1000), (long)hull.Faces[i].d);
		api.Log(buf);
		if (n64)
		{
			fputfloat(hull.Faces[i].a, ofile);
			fputfloat(hull.Faces[i].b, ofile);
			fputfloat(hull.Faces[i].c, ofile);
			fputfloat(hull.Faces[i].d, ofile);
		} 
		else if (psx) 
		{
			r = (long)(hull.Faces[i].a * 65536.0f);
			fwrite(&r, sizeof(long), 1, ofile);
			r = (long)(hull.Faces[i].b * 65536.0f);
			fwrite(&r, sizeof(long), 1, ofile);
			r = (long)(hull.Faces[i].c * 65536.0f);
			fwrite(&r, sizeof(long), 1, ofile);
			r = (long)(hull.Faces[i].d * 65536.0f);
			r = TO_LENGTH(r);
			fwrite(&r, sizeof(long), 1, ofile);
		}
		else
		{
			fwrite(&hull.Faces[i], sizeof(PLANE), 1, ofile);
		}
	}

// free mem

	LocalFree(phandle);
	LocalFree(ehandle);
	LocalFree(fhandle);
	LocalFree(xhandle);
}


/////////////////////////////////////////////////////////////////////
// OutputOneBall:
/////////////////////////////////////////////////////////////////////

void OutputOneBall(long hullnum)
{
	long i, r, totPts;
	COLLSKIN hull;
	HLOCAL phandle, ehandle, fhandle;
	char buf[256];
	float radius;

// weld this hull

	if (!Weld) Weld = 1 * 1;
	WeldModel(FALSE);

// alloc mem for points, edges, planes

	hull.NPts = VertNum;
	phandle = LocalAlloc(LMEM_FIXED, sizeof(VECTOR) * hull.NPts);
	hull.Pts = (VECTOR*)LocalLock(phandle);

	hull.NEdges = 0;
	ehandle = LocalAlloc(LMEM_FIXED, sizeof(EDGE) * PolyNum * 4);
	hull.Edges = (EDGE*)LocalLock(ehandle);

	hull.NFaces = PolyNum;
	fhandle = LocalAlloc(LMEM_FIXED, sizeof(PLANE) * hull.NFaces);
	hull.Faces = (PLANE*)LocalLock(fhandle);

// read points

	for (i = 0 ; i < hull.NPts ; i++)
	{
		hull.Pts[i].x = VertPtr[i].x;
		hull.Pts[i].y = VertPtr[i].y;
		hull.Pts[i].z = VertPtr[i].z;
	}

// create bounding box

	hull.BBox.XMin = hull.BBox.YMin = hull.BBox.ZMin = 999999;
	hull.BBox.XMax = hull.BBox.YMax = hull.BBox.ZMax = -999999;

	for (i = 0 ; i < hull.NPts ; i++)
	{
		if (hull.Pts[i].x < hull.BBox.XMin) hull.BBox.XMin = hull.Pts[i].x;
		if (hull.Pts[i].x > hull.BBox.XMax) hull.BBox.XMax = hull.Pts[i].x;

		if (hull.Pts[i].y < hull.BBox.YMin) hull.BBox.YMin = hull.Pts[i].y;
		if (hull.Pts[i].y > hull.BBox.YMax) hull.BBox.YMax = hull.Pts[i].y;

		if (hull.Pts[i].z < hull.BBox.ZMin) hull.BBox.ZMin = hull.Pts[i].z;
		if (hull.Pts[i].z > hull.BBox.ZMax) hull.BBox.ZMax = hull.Pts[i].z;
	}

// calculate radius
	radius = (hull.BBox.XMax - hull.BBox.XMin) / 2.0f;

// save for later grid use

	HullBBoxPtr[hullnum] = hull.BBox;

// create offset

	hull.Offset.x = (hull.BBox.XMin + hull.BBox.XMax) / 2;
	hull.Offset.y = (hull.BBox.YMin + hull.BBox.YMax) / 2;
	hull.Offset.z = (hull.BBox.ZMin + hull.BBox.ZMax) / 2;

// write out ball position and radius

	wsprintf(buf, "Rad:  %d;  Pos: %d %d %d\n", (int)radius, (int)hull.Offset.x, (int)hull.Offset.y, (int)hull.Offset.z);
	api.Log(buf);

	if (n64)
	{
		//fputshort(1, ofile);
		//fputshort(0, ofile);
		//fputshort(0, ofile);

		//fputfloat(hull.BBox.XMin, ofile);
		//fputfloat(hull.BBox.XMax, ofile);
		//fputfloat(hull.BBox.YMin, ofile);
		//fputfloat(hull.BBox.YMax, ofile);
		//fputfloat(hull.BBox.ZMin, ofile);
		//fputfloat(hull.BBox.ZMax, ofile);

		fputfloat(hull.Offset.x, ofile);
		fputfloat(hull.Offset.y, ofile);
		fputfloat(hull.Offset.z, ofile);
		
		fputfloat(radius, ofile);

		//fputfloat(hull.Offset.x, ofile);
		//fputfloat(hull.Offset.y, ofile);
		//fputfloat(hull.Offset.z, ofile);
		
	}
	else if (psx) 
	{
		r = (long)(hull.Offset.x * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.Offset.y * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		r = (long)(hull.Offset.z * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
		
		r = (long)(radius * 65536.0f);
		r = TO_LENGTH(r);
		fwrite(&r, sizeof(r), 1, ofile);
	}
	else
	{
		totPts = 1;
		hull.NEdges = 0;
		hull.NFaces = 0;
		//fwrite(&totPts, sizeof(hull.NPts), 1, ofile);
		//fwrite(&hull.NEdges, sizeof(hull.NEdges), 1, ofile);
		//fwrite(&hull.NFaces, sizeof(hull.NFaces), 1, ofile);

		//fwrite(&hull.BBox, sizeof(hull.BBox), 1, ofile);
		fwrite(&hull.Offset, sizeof(hull.Offset), 1, ofile);
		fwrite(&radius, sizeof(radius), 1, ofile);

		//fwrite(&hull.Offset, sizeof(hull.Offset), 1, ofile);
	}

// free mem

	LocalFree(phandle);
	LocalFree(ehandle);
	LocalFree(fhandle);
}

//////////////////////
// output hull grid //
//////////////////////

void OutputHullGrid(void)
{
	long i, j, k, count;
	float xmin, xmax, zmin, zmax, f, x, z;
	GRID_HEADER header;
	char buf[256];

// calc XZ bounding box of all hulls

	xmin = zmin = 999999;
	xmax = zmax = -999999;

	for (i = 0 ; i < GroupNum ; i++)
	{
		if (HullBBoxPtr[i].XMin < xmin) xmin = HullBBoxPtr[i].XMin;
		if (HullBBoxPtr[i].XMax > xmax) xmax = HullBBoxPtr[i].XMax;

		if (HullBBoxPtr[i].ZMin < zmin) zmin = HullBBoxPtr[i].ZMin;
		if (HullBBoxPtr[i].ZMax > zmax) zmax = HullBBoxPtr[i].ZMax;
	}

// calc XZ starting point + dimensions

	i = (long)(xmin / HullGridSize);
	header.Xstart = (float)(i * HullGridSize) - HullGridSize;
	i = (long)(zmin / HullGridSize);
	header.Zstart = (float)(i * HullGridSize) - HullGridSize;

// calc XZ dimensions

	i = (long)(xmax / HullGridSize);
	f = (float)(i * HullGridSize) + HullGridSize;
	header.Xnum = (f - header.Xstart) / HullGridSize;
	i = (long)(zmax / HullGridSize);
	f = (float)(i * HullGridSize) + HullGridSize;
	header.Znum = (f - header.Zstart) / HullGridSize;

// display info

	wsprintf(buf, "\nOutputting hull grid:  X %d, Z %d, Width %d, Depth %d\n", (long)header.Xstart, (long)header.Zstart, (long)header.Xnum, (long)header.Znum);
	api.Log(buf);

// write header

	header.GridSize = HullGridSize;
	if (n64)
	{
		fputfloat(header.Xstart, ofile);
		fputfloat(header.Zstart, ofile);
		fputfloat(header.Xnum, ofile);
		fputfloat(header.Znum, ofile);
		fputfloat(header.GridSize, ofile);
	}
	else
	{
		fwrite(&header, sizeof(header), 1, ofile);
	}

// loop thru all grid entries

	for (z = header.Zstart ; z < (header.Zstart + header.Znum * HullGridSize) ; z += HullGridSize)
	{
		for (x = header.Xstart ; x < (header.Xstart + header.Xnum * HullGridSize) ; x += HullGridSize)
		{

// set bounding box for grid

			xmin = x - GridExpand;
			xmax = x + HullGridSize - 1 + GridExpand;

			zmin = z - GridExpand;
			zmax = z + HullGridSize - 1 + GridExpand;

// 2 pass test

			count = 0;
			for (j = 0 ; j < 2 ; j++)
			{
				for (k = 0 ; k < GroupNum ; k++)
				{

// check one hull against grid

					if (xmin > HullBBoxPtr[k].XMax || xmax < HullBBoxPtr[k].XMin ||
						zmin > HullBBoxPtr[k].ZMax || zmax < HullBBoxPtr[k].ZMin)
							continue;

// passed, inc count first pass, output second pass

					if (!j)
					{
						count++;
					}
					else
					{
						if (n64)
						{
							fputlong(k, ofile);
						}
						else
						{
							fwrite(&k, sizeof(k), 1, ofile);
						}

					}
				}

// output count if end of first pass

				if (!j)
				{
					if (n64)
					{
						fputlong(count, ofile);
					}
					else
					{
						fwrite(&count, sizeof(count), 1, ofile);
					}
				}
			}
		}
	}
}

/////////////////////////////////
// build a plane from 3 points //
/////////////////////////////////

void BuildPlane(VECTOR *a, VECTOR *b, VECTOR *c, PLANE *p)
{
	VECTOR vec1, vec2;

// build plane a, b, c

	vec1.x = b->x - a->x;
	vec1.y = b->y - a->y;
	vec1.z = b->z - a->z;

	vec2.x = c->x - a->x;
	vec2.y = c->y - a->y;
	vec2.z = c->z - a->z;

	CrossProduct((float*)&vec2, (float*)&vec1, (float*)p);
	NormalizeVector((float*)p);

// build plane d

	vec1.x = a->x + b->x + c->x;
	vec1.y = a->y + b->y + c->y;
	vec1.z = a->z + b->z + c->z;

	p->d = -DotProduct((float*)p, (float*)&vec1) / 3.0f;
}

//////////////////////////
// add .LIT file lights //
//////////////////////////

void AddLitFileLights(void)
{
	long lightnum, i, j, vcount;
	char buf[128];
	FILE *fp;
	HANDLE handle;
	FILELIGHT *light;
	FILELIGHT_PSX lightpsx;

// open .LIT file

	fp = fopen(litfile, "rb");
	if (!fp)
	{
		wsprintf(buf, "Can't find '%s'", litfile);
		api.Error(buf);
		return;
	}

// alloc ram for lights

	fread(&lightnum, sizeof(lightnum), 1, fp);
	handle = LocalAlloc(LMEM_FIXED, sizeof(FILELIGHT) * lightnum);
	light = (FILELIGHT*)LocalLock(handle);

// load lights

	fread(light, sizeof(FILELIGHT), lightnum, fp);

/*	if (psx) for (i = 0 ; i < lightnum ; i++)
	{
		light[i].x *= 2;
		light[i].y *= 2;
		light[i].z *= 2;
		light[i].Reach *= 2;
	}*/

// close .LIT file

	fclose(fp);

// apply world per

	if (WorldPer != 100) for (i = 0 ; i < PolyNum ; i++)
	{
		vcount = 3 + (PolyPtr[i].Type & 1);
		for (j = 0 ; j < vcount ; j++)
		{
			ModelChangeGouraud((MODEL_RGB*)&PolyPtr[i].rgb[j], WorldPer);
		}
	}

// loop through all lights

	for (i = 0 ; i < lightnum ; i++) if (light[i].Flag & 1)
	{
		switch (light[i].Type)
		{
			case LIGHT_OMNI:
				AddLightOmni(&light[i]);
				break;

			case LIGHT_OMNINORMAL:
				AddLightOmniNormal(&light[i]);
				break;

			case LIGHT_SPOT:
				AddLightSpot(&light[i]);
				break;

			case LIGHT_SPOTNORMAL:
				AddLightSpotNormal(&light[i]);
				break;
		}
	}

// output console .LIT file

	if (strcmp(litoutfile, "null"))
	{
		fp = fopen(litoutfile, "wb");
		if (!fp)
		{
			wsprintf(buf, "Can't create '%s'", litoutfile);
			api.Error(buf);
			return;
		}

		if (n64)
		{
			fputlong(lightnum, fp);
		}
		else
		{
			fwrite(&lightnum, sizeof(lightnum), 1, fp);
		}

		if (n64)
		{
			for (i = 0 ; i < lightnum ; i++)
			{
				fputfloat(light[i].x, fp);
				fputfloat(light[i].y, fp);
				fputfloat(light[i].z, fp);
				fputfloat(light[i].Reach, fp);
				for (j = 0; j < 9; j++)
				{
					fputfloat(light[i].DirMatrix.m[j], fp);
				}
				fputfloat(light[i].Cone, fp);
				fputfloat(light[i].r, fp);
				fputfloat(light[i].g, fp);
				fputfloat(light[i].b, fp);
				fwrite(&light[i].Flag, 1, 1, fp);
				fwrite(&light[i].Type, 1, 1, fp);
				fwrite(&light[i].Speed, 1, 1, fp);
				fwrite(&light[i].pad2, 1, 1, fp);
			}
		}
		else
		{
			for (i = 0 ; i < lightnum ; i++)
			{
				lightpsx.x = (short)light[i].x;
				lightpsx.y = (short)light[i].y;
				lightpsx.z = (short)light[i].z;
				lightpsx.Reach = (short)light[i].Reach;
	
				lightpsx.dx = (short)(light[i].DirMatrix.m[6] * 4096);
				lightpsx.dy = (short)(light[i].DirMatrix.m[7] * 4096);
				lightpsx.dz = (short)(light[i].DirMatrix.m[8] * 4096);
				lightpsx.Cone = (short)light[i].Cone;

				lightpsx.r = (short)light[i].r;
				lightpsx.g = (short)light[i].g;
				lightpsx.b = (short)light[i].b;

				lightpsx.Flag = (unsigned char)light[i].Flag;
				lightpsx.Type = (unsigned char)light[i].Type;

				fwrite(&lightpsx, sizeof(lightpsx), 1, fp);
			}
		}
	}

// close console .LIT file

	fclose(fp);

// free ram

	LocalFree(handle);
}

///////////////
// add light //
///////////////

void AddLightOmni(FILELIGHT *light)
{
	long i, j, vcount, mul;
	long rgb[3];
	float squaredist;

// loop thru all polys

	for (i = 0 ; i < PolyNum ; i++)
	{

// loop thru all verts

		vcount = 3 + (PolyPtr[i].Type & 1);
		for (j = 0 ; j < vcount ; j++)
		{

// add lights

			LightDelta[0] = light->x - PolyPtr[i].v[j].x;
			LightDelta[1] = light->y - PolyPtr[i].v[j].y;
			LightDelta[2] = light->z - PolyPtr[i].v[j].z;

			squaredist = DotProduct(LightDelta, LightDelta);
			if (squaredist < (light->Reach * light->Reach))
			{
				mul = (long)((1 - squaredist / (light->Reach * light->Reach)) * LitPer * 256);

				rgb[0] = ((long)light->r * mul) >> 8;
				rgb[1] = ((long)light->g * mul) >> 8;
				rgb[2] = ((long)light->b * mul) >> 8;

				ModelAddGouraud((MODEL_RGB*)&PolyPtr[i].rgb[j], rgb, (MODEL_RGB*)&PolyPtr[i].rgb[j]);
			}
		}
	}
}

///////////////
// add light //
///////////////

void AddLightOmniNormal(FILELIGHT *light)
{
	long i, j, vcount, mul;
	long rgb[3];
	float squaredist, dist, ang;

// loop thru all polys

	for (i = 0 ; i < PolyNum ; i++)
	{

// loop thru all verts

		vcount = 3 + (PolyPtr[i].Type & 1);
		for (j = 0 ; j < vcount ; j++)
		{

// add lights

			LightDelta[0] = light->x - PolyPtr[i].v[j].x;
			LightDelta[1] = light->y - PolyPtr[i].v[j].y;
			LightDelta[2] = light->z - PolyPtr[i].v[j].z;

			ang = DotProduct(LightDelta, &VertPtr[PolyPtr[i].vi[j]].nx);
			if (ang > 0)
			{
				squaredist = DotProduct(LightDelta, LightDelta);
				if (squaredist < (light->Reach * light->Reach))
				{
					dist = (float)sqrt(squaredist);
					mul = (long)(((1 - squaredist / (light->Reach * light->Reach)) * (ang / dist)) * LitPer * 256);

					rgb[0] = ((long)light->r * mul) >> 8;
					rgb[1] = ((long)light->g * mul) >> 8;
					rgb[2] = ((long)light->b * mul) >> 8;

					ModelAddGouraud((MODEL_RGB*)&PolyPtr[i].rgb[j], rgb, (MODEL_RGB*)&PolyPtr[i].rgb[j]);
				}
			}
		}
	}
}

///////////////
// add light //
///////////////

void AddLightSpot(FILELIGHT *light)
{
	long i, j, vcount, mul;
	long rgb[3];
	float squaredist, cone, dist;

// loop thru all polys

	for (i = 0 ; i < PolyNum ; i++)
	{

// loop thru all verts

		vcount = 3 + (PolyPtr[i].Type & 1);
		for (j = 0 ; j < vcount ; j++)
		{

// add lights

			LightDelta[0] = light->x - PolyPtr[i].v[j].x;
			LightDelta[1] = light->y - PolyPtr[i].v[j].y;
			LightDelta[2] = light->z - PolyPtr[i].v[j].z;

			squaredist = DotProduct(LightDelta, LightDelta);
			if (squaredist < (light->Reach * light->Reach))
			{
				dist = (float)sqrt(squaredist);
				cone = (-DotProduct(&light->DirMatrix.m[6], LightDelta) / dist - 1) * (180 / light->Cone) + 1;
				if (cone > 0)
				{
					mul = (long)(((1 - squaredist / (light->Reach * light->Reach)) * cone) * LitPer * 256);

					rgb[0] = ((long)light->r * mul) >> 8;
					rgb[1] = ((long)light->g * mul) >> 8;
					rgb[2] = ((long)light->b * mul) >> 8;

					ModelAddGouraud((MODEL_RGB*)&PolyPtr[i].rgb[j], rgb, (MODEL_RGB*)&PolyPtr[i].rgb[j]);
				}
			}
		}
	}
}

///////////////
// add light //
///////////////

void AddLightSpotNormal(FILELIGHT *light)
{
	long i, j, vcount, mul;
	long rgb[3];
	float squaredist, cone, dist, ang;

// loop thru all polys

	for (i = 0 ; i < PolyNum ; i++)
	{

// loop thru all verts

		vcount = 3 + (PolyPtr[i].Type & 1);
		for (j = 0 ; j < vcount ; j++)
		{

// add lights

			LightDelta[0] = light->x - PolyPtr[i].v[j].x;
			LightDelta[1] = light->y - PolyPtr[i].v[j].y;
			LightDelta[2] = light->z - PolyPtr[i].v[j].z;

			ang = DotProduct(LightDelta, &VertPtr[PolyPtr[i].vi[j]].nx);
			if (ang > 0)
			{
				squaredist = DotProduct(LightDelta, LightDelta);
				if (squaredist < (light->Reach * light->Reach))
				{
					dist = (float)sqrt(squaredist);
					cone = (-DotProduct(&light->DirMatrix.m[6], LightDelta) / dist - 1) * (180 / light->Cone) + 1;
					if (cone > 0)
					{
						mul = (long)(((1 - squaredist / (light->Reach * light->Reach)) * (ang / dist) * cone) * LitPer * 256);

						rgb[0] = ((long)light->r * mul) >> 8;
						rgb[1] = ((long)light->g * mul) >> 8;
						rgb[2] = ((long)light->b * mul) >> 8;

						ModelAddGouraud((MODEL_RGB*)&PolyPtr[i].rgb[j], rgb, (MODEL_RGB*)&PolyPtr[i].rgb[j]);
					}
				}
			}
		}
	}
}

////////////////////
// conv .VIS file //
////////////////////

void OutputVisFile(void)
{
	long visnum, i;
	char buf[128];
	FILE *fp;
	HANDLE handle;
	VISIBOX *vb;
	VISIBOX_PSX vbpsx;

// if not psx or n64 quit

	if (!(psx | n64))
		return;

// open .VIS file

	fp = fopen(visfile, "rb");
	if (!fp)
	{
		wsprintf(buf, "Can't find '%s'", visfile);
		api.Error(buf);
		return;
	}

// alloc ram

	fread(&visnum, sizeof(visnum), 1, fp);
	handle = LocalAlloc(LMEM_FIXED, sizeof(VISIBOX) * visnum);
	vb = (VISIBOX*)LocalLock(handle);

// load visiboxes

	fread(vb, sizeof(VISIBOX), visnum, fp);

// close .LIT file

	fclose(fp);

// open out file

	fp = fopen(visoutfile, "wb");
	if (!fp)
	{
		wsprintf(buf, "Can't create '%s'", visfile);
		api.Error(buf);
		return;
	}

// write out psx

	if (psx)
	{
		// write header

		fwrite(&visnum, sizeof(visnum), 1, fp);

		for (i = 0 ; i < visnum ; i++)
		{
			vbpsx.Flag = vb[i].Flag;
			vbpsx.ID = vb[i].ID;

			vbpsx.xmin = (short)vb[i].xmin;
			vbpsx.xmax = (short)vb[i].xmax;

			vbpsx.ymin = (short)vb[i].ymin;
			vbpsx.ymax = (short)vb[i].ymax;

			vbpsx.zmin = (short)vb[i].zmin;
			vbpsx.zmax = (short)vb[i].zmax;

			fwrite(&vbpsx, sizeof(vbpsx), 1, fp);
		}
	}

// write out n64

	else
	{
		// write header
		fputlong(visnum, fp);

		for (i = 0 ; i < visnum ; i++)
		{
			fwrite(&vb[i].Flag, 1, 1, fp);
			fwrite(&vb[i].ID, 1, 1, fp);
			fwrite(&vb[i].pad, 1, 2, fp);
			fputfloat(vb[i].xmin, fp);
			fputfloat(vb[i].xmax, fp);
			fputfloat(vb[i].ymin, fp);
			fputfloat(vb[i].ymax, fp);
			fputfloat(vb[i].zmin, fp);
			fputfloat(vb[i].zmax, fp);
		}
	}

// close out file

	fclose(fp);

// free visibox ram

	LocalFree(handle);
}

////////////////////////////
// promote tri's to quads //
////////////////////////////

void PromoteTriangles(void)
{
	long i, j, k, l, m;
	float vec[3], vec1[3], vec2[3];
	char buf[128];
	POLY poly;

// output info

	api.Log("Promoting Tri's...\n\n");

// loop thru all tri's

	for (i = 0 ; i < PolyNum ; i++)
	{
		if (PolyPtr[i].Type & TYPE_QUAD) continue;
		if (PolyPtr[i].Type & TYPE_TEXANIM) continue;

// compare with all lower tri's

		for (j = 0 ; j < i ; j++)
		{
			if (PolyPtr[j].Type & TYPE_QUAD) continue;
			if (PolyPtr[j].Type & TYPE_TEXANIM) continue;

// skip if different tpages

			if (PolyPtr[i].Tpage != PolyPtr[j].Tpage) continue;

// skip if not planar

			if (DotProduct(&PolyPtr[i].nx, &PolyPtr[j].nx) < MAX_PLANAR_TOLERANCE) continue;

// loop thru all possible 'edge share' combinations

			for (k = 0 ; k < 3 ; k++) for (l = 0 ; l < 3 ; l++)
			{

// check for shared coors

				SubVector(&PolyPtr[i].v[k].x, &PolyPtr[j].v[(l + 1) % 3].x, vec);
				if ((vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]) > 1) continue;
				SubVector(&PolyPtr[i].v[(k + 1) % 3].x, &PolyPtr[j].v[l].x, vec);
				if ((vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]) > 1) continue;

// check for shared rgbs

				if (PolyPtr[i].rgb[k] != PolyPtr[j].rgb[(l + 1) % 3]) continue;
				if (PolyPtr[i].rgb[(k + 1) % 3] != PolyPtr[j].rgb[l]) continue;

// check for shared uv's

				if (PolyPtr[i].uv[k].u != PolyPtr[j].uv[(l + 1) % 3].u) continue;
				if (PolyPtr[i].uv[k].v != PolyPtr[j].uv[(l + 1) % 3].v) continue;
				if (PolyPtr[i].uv[(k + 1) % 3].u != PolyPtr[j].uv[l].u) continue;
				if (PolyPtr[i].uv[(k + 1) % 3].v != PolyPtr[j].uv[l].v) continue;

// found a promotion, insert 4th point into 'j' tri

				poly = PolyPtr[j];

				PolyPtr[j].Type |= TYPE_QUAD;

				for (m = 2 ; m > l ; m--)
				{
					PolyPtr[j].v[m + 1] = PolyPtr[j].v[m];
					PolyPtr[j].rgb[m + 1] = PolyPtr[j].rgb[m];
					PolyPtr[j].uv[m + 1] = PolyPtr[j].uv[m];
				}

				PolyPtr[j].v[l + 1] = PolyPtr[i].v[(k + 2) % 3];
				PolyPtr[j].rgb[l + 1] = PolyPtr[i].rgb[(k + 2) % 3];
				PolyPtr[j].uv[l + 1] = PolyPtr[i].uv[(k + 2) % 3];

// check valid cull corner before allowing

				SubVector(&PolyPtr[j].v[2].x, &PolyPtr[j].v[1].x, vec1);
				SubVector(&PolyPtr[j].v[0].x, &PolyPtr[j].v[1].x, vec2);
				NormalizeVector(vec1);
				NormalizeVector(vec2);

				if (CrossProduct3(vec2, vec1, &PolyPtr[j].nx) < MAX_PLANAR_TOLERANCE)
				{
					PolyPtr[j] = poly;
					continue;
				}

// kill 'i' poly

				for (m = i ; m < PolyNum - 1 ; m++) PolyPtr[m] = PolyPtr[m + 1];
				PolyNum--;

// dump info

				wsprintf(buf, "Promoted Tri %d with %d\n", j, i);
				api.Log(buf);

// jump out of all loops but 'i'

				k = 3;
				l = 3;
				j = i;
			}
		}
	}
}

//-----------------------------------------------------------------------------------------------------------------------
//                                                  N64 Specific Functions
//-----------------------------------------------------------------------------------------------------------------------

//
// GenerateTmemList
//
// Generates a list of maximal texture area definitions for the entire model
//

void GenerateTmemList(void)
{
	long			ii, jj, kk;
	long			Duff;
	long			Untextured;
	short			MinS1, MaxS1, MinT1, MaxT1;
	short			MinS2, MaxS2, MinT2, MaxT2;
	short			MinS3, MaxS3, MinT3, MaxT3;
	short			sl, tl, sh, th;
	char			buf[256];
	FILE			*TexFile;
	long			AddPoly;
	long			offset;
	long			tpage;
	RGB_QUAD		temprgb;
	unsigned short	temp;
	long			width;
	long			zero = 0;
	long			TexFound = 0;
	long			dummy[2] = { 0, 0 };

	Untextured = 0;
	Duff = 0;

	wsprintf(buf, "Generating TMEM areas for entire model...\n");
	api.Log(buf);

	// Calculate N64 UVs for all polygons
	for (ii = 0; ii < PolyNum; ii++)
	{
		if (PolyPtr[ii].Tpage >= 0)
		{
			FixTexture(&PolyPtr[ii], TPdim[PolyPtr[ii].Tpage].w, abs(TPdim[PolyPtr[ii].Tpage].h));
		
			for (jj = 0; jj < (3 + (PolyPtr[ii].Type & TYPE_QUAD)); jj++)
			{
				if ((PolyPtr[ii].uv64[jj].u < 0) || (PolyPtr[ii].uv64[jj].u > 255) || (PolyPtr[ii].uv64[jj].v < 0) || (PolyPtr[ii].uv64[jj].v > 255))
				{
					wsprintf(buf, "Duff poly with out of bound UV\n");
					api.Log(buf);
					PolyPtr[ii].Tpage = -1;
					PolyPtr[ii].rgb[0] = 0xFF00FF;
					PolyPtr[ii].rgb[1] = 0xFF00FF;
					PolyPtr[ii].rgb[2] = 0xFF00FF;
					PolyPtr[ii].rgb[3] = 0xFF00FF;
					Duff++;
				}
			}
		}
		else
		if (PolyPtr[ii].Type & TYPE_TEXANIM)
		{
			PolyPtr[ii].Tpage = -1;
			PolyPtr[ii].rgb[0] = 0xFF00FF;
			PolyPtr[ii].rgb[1] = 0xFF00FF;
			PolyPtr[ii].rgb[2] = 0xFF00FF;
			PolyPtr[ii].rgb[3] = 0xFF00FF;
		}
		else
		{
			for (jj = 0; jj < (3 + (PolyPtr[ii].Type & TYPE_QUAD)); jj++)
			{
				PolyPtr[ii].uv64[jj].u = 0;						// UV set to zero for untextured polys
				PolyPtr[ii].uv64[jj].v = 0;
			}
			PolyPtr[ii].Tmem = 0;
		}
	}

	// Scan the list for textured polygons whose texel area is greater than 2048 (256 colour) or 4096 (16 colour)
	for (ii = 0; ii < PolyNum; ii++)
	{
		if (PolyPtr[ii].Tpage == -1) 
		{
			Untextured++;
			continue;
		}
		MinS1 = MinT1 = 256;
		MaxS1 = MaxT1 = 0;
		for (jj = 0; jj < (3 + (PolyPtr[ii].Type & TYPE_QUAD)); jj++)
		{
			if (PolyPtr[ii].uv64[jj].u > MaxS1) { MaxS1 = PolyPtr[ii].uv64[jj].u; }
			if (PolyPtr[ii].uv64[jj].v > MaxT1) { MaxT1 = PolyPtr[ii].uv64[jj].v; }
			if (PolyPtr[ii].uv64[jj].u < MinS1) { MinS1 = PolyPtr[ii].uv64[jj].u; }
			if (PolyPtr[ii].uv64[jj].v < MinT1) { MinT1 = PolyPtr[ii].uv64[jj].v; }
		}

		if (TPdim[PolyPtr[ii].Tpage].type == BMP_TYPE_8BIT)
		{
			if (((((MaxS1 - MinS1) + 7) & ~7) * (MaxT1 - MinT1)) > 2048)
			{
				// Poly has 8-bit textured area > 2048, so mark as 'bad'
				PolyPtr[ii].Tpage = -1;
				PolyPtr[ii].rgb[0] = 0xFF00FF;
				PolyPtr[ii].rgb[1] = 0xFF00FF;
				PolyPtr[ii].rgb[2] = 0xFF00FF;
				PolyPtr[ii].rgb[3] = 0xFF00FF;
				Duff++;
				wsprintf(buf, "Duff poly [tp: %s] (%d, %d -> %d, %d)\n", TPdim[PolyPtr[ii].Tpage].name, MinS1, MinT1, MaxS1, MaxT1);
				api.Log(buf);
			}
		}
		else
		{
			if (((((MaxS1 - MinS1) + 7) & ~7) * (MaxT1 - MinT1)) > 4096)
			{
				// Poly has 4-bit textured area > 4096, so mark as 'bad'
				PolyPtr[ii].Tpage = -1;
				PolyPtr[ii].rgb[0] = 0xFF00FF;
				PolyPtr[ii].rgb[1] = 0xFF00FF;
				PolyPtr[ii].rgb[2] = 0xFF00FF;
				PolyPtr[ii].rgb[3] = 0xFF00FF;
				Duff++;
				wsprintf(buf, "Duff poly [tp: %s] (%d, %d -> %d, %d)\n", TPdim[PolyPtr[ii].Tpage].name, MinS1, MinT1, MaxS1, MaxT1);
				api.Log(buf);
			}
		}
	}

	// Now go through valid polys and generate TMEM areas for them
	Tmem = 0;
	for (ii = 0; ii < PolyNum; ii++)
	{
		if (PolyPtr[ii].Tpage == -1) continue;					// If non-textured poly, then skip
		if (PolyPtr[ii].Done == 1) continue;					// If poly is already included in a texture ref, then skip

		TexFound = 1;
		// New poly to process, first get Min and Max S,T
		MinS1 = MinT1 = 256;
		MaxS1 = MaxT1 = 0;
		for (jj = 0; jj < (3 + (PolyPtr[ii].Type & TYPE_QUAD)); jj++)
		{
			if (PolyPtr[ii].uv64[jj].u > MaxS1) { MaxS1 = PolyPtr[ii].uv64[jj].u; }
			if (PolyPtr[ii].uv64[jj].v > MaxT1) { MaxT1 = PolyPtr[ii].uv64[jj].v; }
			if (PolyPtr[ii].uv64[jj].u < MinS1) { MinS1 = PolyPtr[ii].uv64[jj].u; }
			if (PolyPtr[ii].uv64[jj].v < MinT1) { MinT1 = PolyPtr[ii].uv64[jj].v; }
		}
		TmemPtr[Tmem].sl = MinS1;									// Store the initial texture block descriptor for this poly
		TmemPtr[Tmem].tl = MinT1;
		TmemPtr[Tmem].sh = MaxS1;
		TmemPtr[Tmem].th = MaxT1;
		TmemPtr[Tmem].tpage = PolyPtr[ii].Tpage;
		PolyPtr[ii].Tmem = Tmem;
		PolyPtr[ii].Done = 1;										// Mark comparative poly as 'done'
																	// Now we try to add polys, and see if size of box is still acceptable
		for (kk = 0; kk < PolyNum; kk++)
		{
			if (kk == ii) continue;									// Don't want to check poly against itself
			if (PolyPtr[kk].Tpage != PolyPtr[ii].Tpage) continue;	// Can't check against polys on different tpages
			if (PolyPtr[kk].Tpage == -1) continue;					// If non-textured poly, then skip
			if (PolyPtr[kk].Done) continue;							// If poly is already included in a texture ref, then skip

			MinS3 = MinS1;											// Make a temp copy of our min and max values
			MaxS3 = MaxS1;									
			MinT3 = MinT1;
			MaxT3 = MaxT1;
			MinS2 = MinT2 = 256;
			MaxS2 = MaxT2 = 0;										// Then get our min/max for current poly
			for (jj = 0; jj < (3 + (PolyPtr[kk].Type & TYPE_QUAD)); jj++)
			{
				if (PolyPtr[kk].uv64[jj].u > MaxS2) { MaxS2 = PolyPtr[kk].uv64[jj].u; }
				if (PolyPtr[kk].uv64[jj].v > MaxT2) { MaxT2 = PolyPtr[kk].uv64[jj].v; }
				if (PolyPtr[kk].uv64[jj].u < MinS2) { MinS2 = PolyPtr[kk].uv64[jj].u; }
				if (PolyPtr[kk].uv64[jj].v < MinT2) { MinT2 = PolyPtr[kk].uv64[jj].v; }
			}

			if (MinS2 < MinS1) { MinS3 = MinS2; }				// Update our temp copy if necessary
			if (MinT2 < MinT1) { MinT3 = MinT2; }
			if (MaxS2 > MaxS1) { MaxS3 = MaxS2; }
			if (MaxT2 > MaxT1) { MaxT3 = MaxT2; }
																// Is our new area still aceptable?
			AddPoly = 0;
			if (TPdim[PolyPtr[ii].Tpage].type == BMP_TYPE_8BIT)
			{
				if (((((MaxS3 - MinS3) + 7) & ~7) * (MaxT3 - MinT3)) <= 2048)
				{
					AddPoly = 1;
				}
			}
			else
			{
				if (((((MaxS3 - MinS3) + 7) & ~7) * (MaxT3 - MinT3)) <= 4096)
				{
					AddPoly = 1;
				}
			}

			if (AddPoly)
			{
				TmemPtr[Tmem].sl = MinS3; 						// Update the texture block descriptor for these polys
				TmemPtr[Tmem].tl = MinT3;
				TmemPtr[Tmem].sh = MaxS3;
				TmemPtr[Tmem].th = MaxT3;
				PolyPtr[kk].Tmem = Tmem;
				PolyPtr[kk].Done = 1;							// Mark poly as 'done'
				MinS1 = MinS3;									// Store the new comparison area
				MaxS1 = MaxS3;
				MinT1 = MinT3;
				MaxT1 = MaxT3;
			}								  
		}
		Tmem++;
	}
	wsprintf(buf, "%d N64 unfriendly textured polys found.\n", Duff);
	api.Log(buf);
	wsprintf(buf, "%d untextured polys.\n", Untextured);
	api.Log(buf);
	wsprintf(buf, "%d TMEM areas created for %d textured polys.\n\n", Tmem, (PolyNum - Duff - Untextured));
	api.Log(buf);

	if ((TexFname == NULL) || (TexFound == FALSE)) return;

	// Build palsize and paloffset list in TPDIM list
	offset = 4 * sizeof(long) * Tmem;
	for (ii = 0; ii < (MaxTpage + 1); ii++)
	{
		if (TPdim[ii].type == BMP_TYPE_4BIT)
		{
			TPdim[ii].palsize = 16 * 2;
		}
		else
		{
			TPdim[ii].palsize = 256 * 2;
		}
		TPdim[ii].paloffset = offset;
		offset += TPdim[ii].palsize;
	}

	for (ii = 0; ii < Tmem; ii++)
	{
		TmemPtr[ii].texoffset = offset;
		tpage = TmemPtr[ii].tpage;
		if (TPdim[tpage].type == BMP_TYPE_4BIT)
		{
			offset += (((((((TmemPtr[ii].sh - TmemPtr[ii].sl) + 7) & ~7) * (TmemPtr[ii].th - TmemPtr[ii].tl)) / 2) + 7) & ~7);
		}
		else
		{
			offset += ((((((TmemPtr[ii].sh - TmemPtr[ii].sl) + 7) & ~7) * (TmemPtr[ii].th - TmemPtr[ii].tl)) + 7) & ~7);
		}
	}

	// We can now generate the texture file. The header of this file consists of the number of TMEM areas, followed
	// by a list of TMEM descriptors (palette, bitmap, w, h, fmt), followed by the palettes, and finally the texture data

	wsprintf(buf, "Writing texture file: %s\n", TexFname);
	api.Log(buf);

	TexFile = fopen(TexFname, "wb");
	if (TexFile == NULL)
	{
		wsprintf(buf, "Can't Open % for writing\n", TexFname);
		api.Error(buf);
	}
	for (ii = 0; ii < Tmem; ii++)														// Write header
	{
		wsprintf(buf, "Tmem %d - TP: %d, PO: %d, TO: %d, W: %d, H: %d, T: %d\n", ii, TmemPtr[ii].tpage, TPdim[TmemPtr[ii].tpage].paloffset, TmemPtr[ii].texoffset, (long)(((TmemPtr[ii].sh - TmemPtr[ii].sl) + 7) & ~7), (long)(TmemPtr[ii].th - TmemPtr[ii].tl), TPdim[TmemPtr[ii].tpage].type);
		api.Log(buf);
		fputlong(TPdim[TmemPtr[ii].tpage].paloffset, TexFile);
		fputlong(TmemPtr[ii].texoffset, TexFile);
		fputshort((short)(((TmemPtr[ii].sh - TmemPtr[ii].sl) + 7) & ~7), TexFile);
		fputshort((short)(TmemPtr[ii].th - TmemPtr[ii].tl), TexFile);
		fputlong(TPdim[TmemPtr[ii].tpage].type, TexFile);
	}

	temprgb.a = 0;					   													// Write palettes
	for (ii = 0; ii < (MaxTpage + 1); ii++)
	{
		if (TPdim[ii].type == BMP_TYPE_4BIT)
		{
			for (jj = 0; jj < 16; jj++)
			{
				temprgb.b = *((char *)TPdim[ii].addr + sizeof(BITMAPINFOHEADER) + (jj << 2));
				temprgb.g = *((char *)TPdim[ii].addr + sizeof(BITMAPINFOHEADER) + (jj << 2) + 1);
				temprgb.r = *((char *)TPdim[ii].addr + sizeof(BITMAPINFOHEADER) + (jj << 2) + 2);
				temp = (unsigned short)(((unsigned short)(temprgb.r >> 3) << 11) | ((unsigned short)(temprgb.g >> 3) << 6) | ((unsigned short)(temprgb.b >> 3) << 1));
				if (temp)
				{
					temp |= 1;
				}
				fputshort(temp, TexFile);
			}
		}
		else
		{
			for (jj = 0; jj < 256; jj++)
			{
				temprgb.b = *((char *)TPdim[ii].addr + sizeof(BITMAPINFOHEADER) + (jj << 2));
				temprgb.g = *((char *)TPdim[ii].addr + sizeof(BITMAPINFOHEADER) + (jj << 2) + 1);
				temprgb.r = *((char *)TPdim[ii].addr + sizeof(BITMAPINFOHEADER) + (jj << 2) + 2);
				temp = (unsigned short)(((unsigned short)(temprgb.r >> 3) << 11) | ((unsigned short)(temprgb.g >> 3) << 6) | ((unsigned short)(temprgb.b >> 3) << 1));
				if (temp)
				{
					temp |= 1;
				}
				fputshort(temp, TexFile);
			}
		}
	}

	for (ii = 0; ii < Tmem; ii++)														// Write TMEM textures
	{
		tpage = TmemPtr[ii].tpage;
		if (TPdim[tpage].type == BMP_TYPE_4BIT)
		{
			width = ((TPdim[tpage].w + 7) & ~7) / 2; 									// Calculate 4-pixel aligned width
			sl = (short)(TmemPtr[ii].sl);
			sh = (short)(TmemPtr[ii].sh);
			tl = (short)(TmemPtr[ii].tl);
			th = (short)(TmemPtr[ii].th);
			wsprintf(buf, "Writing 4-bit (tp:%d): (%d, %d, %d, %d)\n", tpage, sl, tl, sh, th);
			api.Log(buf);
			if (TPdim[tpage].h < 0)
			{
				for (jj = 0; jj < (th - tl); jj++)
				{
					fwrite((char *)TPdim[tpage].addr + sizeof(BITMAPINFOHEADER) + (16 * 4) + ((tl + jj) * width) + (sl / 2), 1, (((sh - sl) + 7) & ~7) / 2, TexFile);
				}
			}
			else
			{
				for (jj = 0; jj < (th - tl); jj++)
				{
					fwrite((char *)TPdim[tpage].addr + sizeof(BITMAPINFOHEADER) + (16 * 4) + ((abs(TPdim[tpage].h) - tl - jj - 1) * width) + (sl / 2), 1, (((sh - sl)  + 7) & ~7) / 2, TexFile);
				}
			}
			if (((((sh - sl) + 7) & ~7) * (th - tl) / 2) & 7)
			{
				fwrite((char *)&dummy, 1, 8 - (((((sh - sl) + 7) & ~7) * (th - tl) / 2) & 7), TexFile);
			}
		}
		else
		{
			width = (TPdim[tpage].w + 7) & ~7; 											// Calculate 4-pixel aligned width
			sl = (short)(TmemPtr[ii].sl);
			sh = (short)(TmemPtr[ii].sh);
			tl = (short)(TmemPtr[ii].tl);
			th = (short)(TmemPtr[ii].th);
			wsprintf(buf, "Writing 8-bit (tp:%d): (%d, %d, %d, %d)\n", tpage, sl, tl, sh, th);
			api.Log(buf);
			if (TPdim[tpage].h < 0)
			{
				for (jj = 0; jj < (th - tl); jj++)
				{
					fwrite((char *)TPdim[tpage].addr + sizeof(BITMAPINFOHEADER) + (256 * 4) + ((tl + jj) * width) + sl, 1, (((sh - sl) + 7) & ~7), TexFile);
				}
			}
			else
			{
				for (jj = 0; jj < (th - tl); jj++)
				{
					fwrite((char *)TPdim[tpage].addr + sizeof(BITMAPINFOHEADER) + (256 * 4) + ((abs(TPdim[tpage].h) - tl - jj - 1) * width) + sl, 1, (((sh - sl) + 7) & ~7), TexFile);
				}
			}
			if (((((sh - sl) + 7) & ~7) * (th - tl)) & 7)
			{
				fwrite((char *)&dummy, 1, 8 - (((((sh - sl) + 7) & ~7) * (th - tl)) & 7), TexFile);
			}
		}
	}
	fclose(TexFile);
}



//
// OutputModelN64
//
// Outputs a standard model in N64 format
//

void OutputModelN64()
{
	long		ii, jj;
	long		temp1 = 0;
	short		temp2 = 0;
	char		cacheidx;
	long		GfxIdx;
	long		GFX_LSW;
	long		GFX_MSW;
	long		vtxtot;
	long		polytot;
	long		curpoly;
	long		curtpage;
	long		curmode;
	long		cursided;
	long		cursemi;
	long		curtmem;
	CACHE 		cache[32];
	char		cacheused[32];
	long		cachefull;
//#if (N64_CRAP|N64_LOG)
	char		buf[256];
//#endif
	long		modelsize;
	long		semisize;
	long		envsize;
	long		semiflg;
	long		VtxOrig;
	short		tempu, tempv;

	modelsize = 0;
	semisize = 0;
	semiflg = 0;
	envsize = 0;
	polytot = 0;
	vtxtot = 0;
	GfxIdx = 0;
	VtxOrig = 0;
	cursemi = -1;

#if N64_CRAP
		wsprintf(buf, "*** NEW model ***\n");
		api.Log(buf);
#endif
	
	while (polytot < PolyNum)
	{
		for (ii = 0; ii < 32; ii++)										// Clear cache ref array
		{
			cache[ii].vtx = -1;
		}

		curpoly = polytot;
		cacheidx = 0;
		cachefull = 0;

		if (cursemi != (PolyPtr[curpoly].Type & TYPE_SEMITRANS))
		{
			cursemi = PolyPtr[curpoly].Type & TYPE_SEMITRANS;
			if (PolyPtr[curpoly].Type & TYPE_SEMITRANS)
			{
				GFX_LSW = GFX_RMSEMI;
				GFX_MSW = 0;
				GfxPtr[GfxIdx].msw = GFX_MSW;
				GfxPtr[GfxIdx++].lsw = GFX_LSW;
				modelsize += SIZE_GFX_RMSEMI;
#if N64_CRAP
				wsprintf(buf, "GFX_RMSEMI\n");
				api.Log(buf);
#endif
				semiflg = 1;
			}
			else
			{
				GFX_LSW = GFX_RMNORM;
				GFX_MSW = 0;
				GfxPtr[GfxIdx].msw = GFX_MSW;
				GfxPtr[GfxIdx++].lsw = GFX_LSW;
				modelsize += SIZE_GFX_RMNORM;
#if N64_CRAP
				wsprintf(buf, "GFX_RMNORM\n");
				api.Log(buf);
#endif
			}
		}

		while((cachefull != 1) && (curpoly < PolyNum))					// Fill cache with vertices
		{
			for (ii = 0; ii < (3 + (PolyPtr[curpoly].Type & TYPE_QUAD)); ii++)
			{
				for (jj = 0; jj < cacheidx; jj++) 						// See if we have current vertex in the cache already
				{
					if (cache[jj].vtx == PolyPtr[curpoly].vi[ii])
					{
						break;											// Vertex reference is already in the cache
					}
				}
				if (cacheidx == 0) { jj = 0; }
				if (jj == cacheidx)										// If vertex was not found in cache, then add it
				{
					cache[cacheidx].vtx  = PolyPtr[curpoly].vi[ii];
					cache[cacheidx].u    = (short)((PolyPtr[curpoly].uv64[ii].u - TmemPtr[PolyPtr[curpoly].Tmem].sl) << 6);
					cache[cacheidx].v    = (short)((PolyPtr[curpoly].uv64[ii].v - TmemPtr[PolyPtr[curpoly].Tmem].tl) << 6);
					cache[cacheidx].rgba = PolyPtr[curpoly].rgb[ii];
					cache[cacheidx].tmem = PolyPtr[curpoly].Tmem;
					cacheidx++;
				}
				PolyPtr[curpoly].CacheIdx[ii] = jj;
			}
			curpoly++;
			if ((PolyPtr[curpoly].Type & TYPE_SEMITRANS) && (!semiflg))
			{																// Flush cache if semitrans poly is next
				cachefull = 1;
				semiflg = 1;												// Signal first semitrans poly has been found (assumes polys are sorted)
			}

			if (curpoly == PolyNum) { cachefull = 1; }						// If this is last poly, indicate cache is full
			else
			if ((cacheidx + 3 + (PolyPtr[curpoly].Type & TYPE_QUAD)) >= 31) { cachefull = 1; }
		}

		// Store the vertices from the "full" cache
		for (ii = 0; ii < cacheidx; ii++)
		{
			VtxPtr[vtxtot + ii].v.ob[0] = (short)VertPtr[cache[ii].vtx].x;
			VtxPtr[vtxtot + ii].v.ob[1] = (short)VertPtr[cache[ii].vtx].y;
			VtxPtr[vtxtot + ii].v.ob[2] = (short)VertPtr[cache[ii].vtx].z;
			VtxPtr[vtxtot + ii].v.flag  = 0;
			VtxPtr[vtxtot + ii].v.tc[0] = cache[ii].u;
			VtxPtr[vtxtot + ii].v.tc[1] = cache[ii].v;
			VtxPtr[vtxtot + ii].v.cn[0] = (char)((long)(cache[ii].rgba >> 16));
			VtxPtr[vtxtot + ii].v.cn[1] = (char)((long)(cache[ii].rgba >> 8));
			VtxPtr[vtxtot + ii].v.cn[2] = (char)((long)(cache[ii].rgba >> 0));
			VtxPtr[vtxtot + ii].v.cn[3] = (char)((long)(cache[ii].rgba >> 24));
			VtxIdx[VtxOrig++] = cache[ii].vtx;
		}

		// Create a Gfx command to upload the vertices
		GFX_LSW  = GFX_VTX;
		GFX_LSW |= (cacheidx << 8);
		GFX_MSW  = vtxtot;
		GfxPtr[GfxIdx].msw = GFX_MSW;
		GfxPtr[GfxIdx++].lsw = GFX_LSW;
		modelsize += SIZE_GFX_VERTEX;
		envsize += SIZE_GFX_VERTEX;
		vtxtot += cacheidx;
#if N64_CRAP
		wsprintf(buf, "GFX_VTX\n");
		api.Log(buf);
#endif
	
		// Now create the gfx list for the polys we have derived vertices from
		for (jj = 0; jj < 32; jj++)										// Clear vertex cache used indicator array
		{
			cacheused[jj] = 0;
		}

		curtpage = -1;
		curmode = -1;
		cursided = -1;
		curtmem = -1;

		for (ii = polytot; ii < curpoly; ii++)
		{
			if ((PolyPtr[ii].Tpage != curtpage) || 						// Do we need to change Tpage or Tmem load)?
			  	(PolyPtr[ii].Tmem != curtmem))
			{
				if ((PolyPtr[ii].Tpage == -1) && (curmode != GFX_CCFLAT))
				{														// This poly is untextured
					GFX_LSW = GFX_CCFLAT;
					GFX_MSW = 0;
					GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
					GfxPtr[GfxIdx++].lsw = GFX_LSW;
					curmode = GFX_CCFLAT;
#if N64_CRAP
					wsprintf(buf, "GFX_CCFLAT\n");
					api.Log(buf);
#endif
						if (cursemi == TYPE_SEMITRANS)
					{
						semisize += SIZE_GFX_CCFLAT;
					}
					else
					{
						modelsize += SIZE_GFX_CCFLAT;
					}

				}
				else
				if ((PolyPtr[ii].Tpage >= 0) && (curmode != GFX_CCTEXT))
				{														// Change back to texture mode again
					GFX_LSW = GFX_CCTEXT;
					GFX_MSW = 0;
					GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
					GfxPtr[GfxIdx++].lsw = GFX_LSW;
					if (cursemi)
					{
						semisize += SIZE_GFX_CCTEXT;
					}
					else
					{
						modelsize += SIZE_GFX_CCTEXT;
					}
#if N64_CRAP
					wsprintf(buf, "GFX_CCTEXT\n");
					api.Log(buf);
#endif
					}

				if ((PolyPtr[ii].Tpage >= 0) && (PolyPtr[ii].Tmem != curtmem))
				{
					GFX_LSW  = GFX_TEX;									// Set GFX_TEX command with parameters for texture load
					GFX_LSW |= ((PolyPtr[ii].Tmem & 0xFF) << 8);
					GFX_MSW  = 0;
					GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
					GfxPtr[GfxIdx++].lsw = GFX_LSW;
					curtmem = PolyPtr[ii].Tmem;
					curtpage = PolyPtr[ii].Tpage;
					if (cursemi == TYPE_SEMITRANS)
					{
						semisize += SIZE_GFX_TEX;
					}
					else
					{
						modelsize += SIZE_GFX_TEX;
					}
#if N64_CRAP
					wsprintf(buf, "GFX_TEX: TMEM = TP:%d\n", PolyPtr[ii].Tmem);
					api.Log(buf);
#endif
				}
			}
		
			// Check if any of the required vertices need modifying
			for (jj = 0; jj < (3 + (PolyPtr[ii].Type & TYPE_QUAD)); jj++)
			{
				if (cacheused[PolyPtr[ii].CacheIdx[jj]] == 1)
				{
					// Do we need to modify ST?
					tempu = (short)((PolyPtr[ii].uv64[jj].u - TmemPtr[PolyPtr[ii].Tmem].sl) << 6);
					tempv = (short)((PolyPtr[ii].uv64[jj].v - TmemPtr[PolyPtr[ii].Tmem].tl) << 6);
					if ((cache[PolyPtr[ii].CacheIdx[jj]].u != tempu) ||	(cache[PolyPtr[ii].CacheIdx[jj]].v != tempv))
					{
						GFX_LSW  = GFX_MODVTX;
						GFX_LSW |= (PolyPtr[ii].CacheIdx[jj] << 8);
						GFX_LSW |= (G_MWO_POINT_ST << 16);
						GFX_MSW  = ((tempu << 15) & 0xFFFF0000) | (tempv >> 1);
						GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
						GfxPtr[GfxIdx++].lsw = GFX_LSW;
						if (cursemi == TYPE_SEMITRANS)
						{
							semisize += SIZE_GFX_MODVTX;
						}
						else
						{
							modelsize += SIZE_GFX_MODVTX;
						}
					}
				}
			}

			if ((PolyPtr[ii].Type & TYPE_DSIDED) && (cursided == GFX_SSIDED))
			{
				GFX_LSW  = GFX_DSIDED;
				GFX_MSW = 0;
				GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
				GfxPtr[GfxIdx++].lsw = GFX_LSW;
				curmode = GFX_DSIDED;
				if (cursemi == TYPE_SEMITRANS)
				{
					semisize += SIZE_GFX_DSIDED;
				}
				else
				{
					modelsize += SIZE_GFX_DSIDED;
				}
#if N64_CRAP
					wsprintf(buf, "GFX_DSIDED\n");
					api.Log(buf);
#endif
				}
			else
			if (!(PolyPtr[ii].Type & TYPE_DSIDED) && (cursided == GFX_DSIDED))
			{
				GFX_LSW  = GFX_SSIDED;
				GFX_MSW = 0;
				GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
				GfxPtr[GfxIdx++].lsw = GFX_LSW;
				curmode = GFX_SSIDED;
				if (cursemi == TYPE_SEMITRANS)
				{
					semisize += SIZE_GFX_SSIDED;
				}
				else
				{
					modelsize += SIZE_GFX_SSIDED;
				}
#if N64_CRAP
					wsprintf(buf, "GFX_SSIDED\n");
					api.Log(buf);
#endif
				}

			if (PolyPtr[ii].Type & TYPE_QUAD)
			{
				GFX_LSW  = GFX_QUAD;
				GFX_LSW |= (PolyPtr[ii].CacheIdx[0] << 8);
				GFX_LSW |= (PolyPtr[ii].CacheIdx[1] << 16);
				GFX_LSW |= (PolyPtr[ii].CacheIdx[2] << 24);
				GFX_MSW  = PolyPtr[ii].CacheIdx[3];
				cacheused[PolyPtr[ii].CacheIdx[0]] = 1;
				cacheused[PolyPtr[ii].CacheIdx[1]] = 1;
				cacheused[PolyPtr[ii].CacheIdx[2]] = 1;
				cacheused[PolyPtr[ii].CacheIdx[3]] = 1;
				GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
				GfxPtr[GfxIdx++].lsw = GFX_LSW;
				if (cursemi == TYPE_SEMITRANS)
				{
					semisize += SIZE_GFX_QUAD;
				}
				else
				{
					modelsize += SIZE_GFX_QUAD;
				}
				envsize += SIZE_GFX_QUAD;
#if N64_CRAP
				wsprintf(buf, "GFX_QUAD\n");
				api.Log(buf);
#endif
			}
			else
			{
				GFX_LSW  = GFX_TRI;
				GFX_LSW |= (PolyPtr[ii].CacheIdx[0] << 8);
				GFX_LSW |= (PolyPtr[ii].CacheIdx[1] << 16);
				GFX_LSW |= (PolyPtr[ii].CacheIdx[2] << 24);
				GFX_MSW  = 0;
				cacheused[PolyPtr[ii].CacheIdx[0]] = 1;
				cacheused[PolyPtr[ii].CacheIdx[1]] = 1;
				cacheused[PolyPtr[ii].CacheIdx[2]] = 1;
				GfxPtr[GfxIdx].msw = GFX_MSW;						// Store tokenised Gfx command
				GfxPtr[GfxIdx++].lsw = GFX_LSW;
				if (cursemi == TYPE_SEMITRANS)
				{
					semisize += SIZE_GFX_TRI;
				}
				else
				{
					modelsize += SIZE_GFX_TRI;
				}
				envsize += SIZE_GFX_TRI;
#if N64_CRAP
				wsprintf(buf, "GFX_TRI\n");
				api.Log(buf);
#endif
			}
			polytot++;	
		}
	}

	temp1 = (long)vtxtot;
	fputlong(temp1, ofile);									// Write number of Vtxs we created
#if N64_LOG
	wsprintf(buf, "Wrote %d vertices.\n", vtxtot);
	api.Log(buf);
#endif
	temp1 = (long)GfxIdx;
	fputlong(temp1, ofile);									// Write number of Gfx commands we created
#if N64_LOG
	wsprintf(buf, "Wrote %d GFX commands.\n", GfxIdx);
	api.Log(buf);
#endif
	temp1 = (long)modelsize;
	fputlong(temp1, ofile);									// Write the size of the expanded Gfx buffer
#if N64_LOG
	wsprintf(buf, "Size of expanded Gfx buffer required is %d.\n", modelsize);
	api.Log(buf);
#endif
	temp1 = (long)semisize;
	fputlong(temp1, ofile);									// Write the size of the expanded semitrans Gfx buffer
#if N64_LOG
	wsprintf(buf, "Size of expanded ""semi"" Gfx buffer required is %d.\n", semisize);
	api.Log(buf);
#endif
	temp1 = (long)envsize;
	fputlong(temp1, ofile);									// Write the size of the expanded envmap Gfx buffer
#if N64_LOG
	wsprintf(buf, "Size of expanded envmap Gfx buffer required is %d.\n", modelsize);
	api.Log(buf);
#endif

	for (ii = 0 ; ii < vtxtot ; ii++)
	{
		temp2 = (short)VtxPtr[ii].v.ob[0];
		fputshort(temp2, ofile);
		temp2 = (short)VtxPtr[ii].v.ob[1];
		fputshort(temp2, ofile);
		temp2 = (short)VtxPtr[ii].v.ob[2];
		fputshort(temp2, ofile);
		temp2 = (short)VtxPtr[ii].v.flag;		  
		fputshort(temp2, ofile);
		temp2 = (short)VtxPtr[ii].v.tc[0];
		fputshort(temp2, ofile);
		temp2 = (short)VtxPtr[ii].v.tc[1];
		fputshort(temp2, ofile);
		fwrite(&VtxPtr[ii].v.cn[0], 1, 1, ofile);
		fwrite(&VtxPtr[ii].v.cn[1], 1, 1, ofile);
		fwrite(&VtxPtr[ii].v.cn[2], 1, 1, ofile);
		fwrite(&VtxPtr[ii].v.cn[3], 1, 1, ofile);
	}

	for (ii = 0 ; ii < GfxIdx ; ii++)
	{
		temp1 = GfxPtr[ii].msw;
		fputlong(temp1, ofile);
		temp1 = GfxPtr[ii].lsw;
		fputlong(temp1, ofile);
	}

	if (!Cube)
	{
		wsprintf(buf, "Writing %d model normals.\n", vtxtot);
		api.Log(buf);
		for (ii = 0; ii < vtxtot; ii++)
		{	
			VtxPtr[ii].n.n[0] = (char)((float)(VertPtr[VtxIdx[ii]].nx * 127));
			VtxPtr[ii].n.n[1] = (char)((float)(VertPtr[VtxIdx[ii]].ny * 127));
			VtxPtr[ii].n.n[2] = (char)((float)(VertPtr[VtxIdx[ii]].nz * 127));
			fwrite(&VtxPtr[ii].n.n[0], 1, 1, ofile);
			fwrite(&VtxPtr[ii].n.n[1], 1, 1, ofile);
			fwrite(&VtxPtr[ii].n.n[2], 1, 1, ofile);
		}
	}
}


void OutputCollisionN64(void)
{
	short i, j, vcount;
	POLY *p;
	VERTEX refpoint;
	VERTEX normal;
	VERTEX *u, *v;
	float plane[4], len;
	float MinX, MaxX, MinY, MaxY, MinZ, MaxZ;
	long flag;


// write poly num
	fputlong((long)PolyNum, ofile);

// loop thru

	p = PolyPtr;

	for (i = 0 ; i < PolyNum ; i++, p++)
	{

// write out flag

		flag = p->Type & TYPE_QUAD;
		fputlong(flag, ofile);
		
// write out plane coefficients

		vcount = (short)(3 + (p->Type & TYPE_QUAD));
		normal.x = normal.y = normal.z = 0;
		refpoint.x = refpoint.y = refpoint.z = 0;

		for (j = 0 ; j < vcount ; j++)
		{
			u = &p->v[j];
			v = &p->v[(j + 1) % vcount];

			normal.x += (v->y - u->y) * (v->z + u->z);
			normal.y += (v->z - u->z) * (v->x + u->x);
			normal.z += (v->x - u->x) * (v->y + u->y);

			refpoint.x += u->x;
			refpoint.y += u->y;
			refpoint.z += u->z;
		}

		len = (float)sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
		plane[0] = normal.x / len;
		plane[1] = normal.y / len;
		plane[2] = normal.z / len;

		len *= vcount;
		plane[3] = -(refpoint.x * normal.x + refpoint.y * normal.y + refpoint.z * normal.z) / len;

		fputfloat(plane[0], ofile);
		fputfloat(plane[1], ofile);
		fputfloat(plane[2], ofile);
		fputfloat(plane[3], ofile);

// write out poly centre

/*		refpoint.x /= vcount;
		refpoint.y /= vcount;
		refpoint.z /= vcount;

		fwrite((float*)&refpoint, sizeof(float), 3, ofile);

// write out poly squared radius

		rad = 0;

		for (j = 0 ; j < vcount ; j++)
		{
			dx = p->v[j].x - refpoint.x;
			dy = p->v[j].y - refpoint.y;
			dz = p->v[j].z - refpoint.z;
			len = (float)sqrt((double)(dx * dx + dy * dy + dz * dz));
			if (len > rad) rad = len;
		}

		rad *= rad;
		fwrite(&rad, sizeof(float), 1, ofile);*/

// write out bounding box

		MinX = MaxX = p->v[0].x;
		MinY = MaxY = p->v[0].y;
		MinZ = MaxZ = p->v[0].z;

		for (j = 1 ; j < vcount ; j++)
		{
			if (p->v[j].x < MinX) MinX = p->v[j].x;
			if (p->v[j].x > MaxX) MaxX = p->v[j].x;
			if (p->v[j].y < MinY) MinY = p->v[j].y;
			if (p->v[j].y > MaxY) MaxY = p->v[j].y;
			if (p->v[j].z < MinZ) MinZ = p->v[j].z;
			if (p->v[j].z > MaxZ) MaxZ = p->v[j].z;
		}	

		fputfloat(MinX, ofile);
		fputfloat(MaxX, ofile);
		fputfloat(MinY, ofile);
		fputfloat(MaxY, ofile);
		fputfloat(MinZ, ofile);
		fputfloat(MaxZ, ofile);

// write out verts

		fputfloat(p->v[0].x,  ofile);
		fputfloat(p->v[0].y,  ofile);
		fputfloat(p->v[0].z,  ofile);
		fputfloat(p->v[1].x,  ofile);
		fputfloat(p->v[1].y,  ofile);
		fputfloat(p->v[1].z,  ofile);
		fputfloat(p->v[2].x,  ofile);
		fputfloat(p->v[2].y,  ofile);
		fputfloat(p->v[2].z,  ofile);
		fputfloat(p->v[3].x,  ofile);
		fputfloat(p->v[3].y,  ofile);
		fputfloat(p->v[3].z,  ofile);

// write out cross vectors (A to B, B to C, C to D, D to A)

//		for (j = 0 ; j < 4 ; j++)
//		{
//			vec[0] = p->v[(j + 1) % vcount].x - p->v[j % vcount].x;
//			vec[1] = p->v[(j + 1) % vcount].y - p->v[j % vcount].y;
//			vec[2] = p->v[(j + 1) % vcount].z - p->v[j % vcount].z;
//
//			fwrite(vec, sizeof(float), 3, ofile);
//		}
	}
}

//
// SortModelN64
//
// Sort polys into tpage and texture size order, so process model can work with the largest polys first
//

void SortModelN64(void)
{
	short ii, jj, kk;
	long a, b;
	POLY poly;
	float	MinS1, MaxS1, MinT1, MaxT1;
	float	MinS2, MaxS2, MinT2, MaxT2;

	for (ii = (short)(PolyNum - 1) ; ii ; ii--) for (jj = 0 ; jj < ii ; jj++)				// Loop through faces
	{
		MinS1 = MinT1 = MinS2 = MinT2 =2;
		MaxS1 = MaxT1 = MaxS2 = MaxT2 =0;
		for (kk = 0; kk < (3 + (PolyPtr[jj].Type & TYPE_QUAD)); kk++)						// Determine texture sizes
		{
			if (PolyPtr[jj].uv[kk].u > MaxS1) { MaxS1 = PolyPtr[jj].uv[kk].u; }
			if (PolyPtr[jj].uv[kk].v > MaxT1) { MaxT1 = PolyPtr[jj].uv[kk].v; }
			if (PolyPtr[jj].uv[kk].u < MinS1) { MinS1 = PolyPtr[jj].uv[kk].u; }
			if (PolyPtr[jj].uv[kk].v < MinT1) { MinT1 = PolyPtr[jj].uv[kk].v; }
			if (PolyPtr[jj + 1].uv[kk].u > MaxS2) { MaxS2 = PolyPtr[jj + 1].uv[kk].u; }
			if (PolyPtr[jj + 1].uv[kk].v > MaxT2) { MaxT2 = PolyPtr[jj + 1].uv[kk].v; }
			if (PolyPtr[jj + 1].uv[kk].u < MinS2) { MinS2 = PolyPtr[jj + 1].uv[kk].u; }
			if (PolyPtr[jj + 1].uv[kk].v < MinT2) { MinT2 = PolyPtr[jj + 1].uv[kk].v; }
		}

		// Get 'scores' for each
		a = PolyPtr[jj].Tmem;
		if (PolyPtr[jj].Type & TYPE_SEMITRANS) { a += 65536; }
		if (PolyPtr[jj].Tpage < 0) { a += 1024; }
		b = PolyPtr[jj + 1].Tmem;
		if (PolyPtr[jj + 1].Type & TYPE_SEMITRANS) { b += 65536; }
		if (PolyPtr[jj].Tpage < 0) { b += 1024; }

		// Swap?
		if (a > b)
		{
			memcpy(&poly, &PolyPtr[jj], sizeof(POLY));
			memcpy(&PolyPtr[jj], &PolyPtr[jj + 1], sizeof(POLY));
			memcpy(&PolyPtr[jj + 1], &poly, sizeof(POLY));
		}
	}
}



//
// FixTexture
//
// Provides GOOD integer UVs for N64, instead of chav rounded ones.
//

void FixTexture(POLY *SrcPoly, long TPw, long TPh)
{
	long	ii;
	PSX_Polygon poly;
	unsigned int Iu[4];
	unsigned int Iv[4];
	long	flag = 0;

	flag = 0;
	for (ii = 0; ii < (3 + (SrcPoly->Type & TYPE_QUAD)); ii++)
	{
		poly.AddPoint(PS_Point(SrcPoly->uv[ii].u * TPw, SrcPoly->uv[ii].v * TPh));
	}

	for (ii = 0; ii < (3 + (SrcPoly->Type & TYPE_QUAD)); ii++)
	{
		poly.GetPSXPoint(ii, Iu[ii], Iv[ii]);
		SrcPoly->uv64[ii].u = (short)Iu[ii];
		SrcPoly->uv64[ii].v = (short)Iv[ii];
	}
}



//
// LoadPortals
//
// Loads portal/region data file (always track.por)
//

void LoadPortals(void)
{
	long		ii;
	long		Num;
	EDIT_PORTAL	TempPort;

	pfile = fopen("track.por", "rb");
	if (pfile == NULL)
	{
		api.Error("Could not open portal file!");
	}	
	
	fread((char *)&Num, 1, sizeof(long), pfile);
	if (Num <= 0)
	{
		api.Error("Invalid number of portals/regions!");
	}
	if (Num > MAX_EDIT_PORTALS)
	{
		api.Error("Too many regions/portals!");
	}

	for (ii = 0; ii < Num; ii++)
	{
		fread((char *)&TempPort, 1, sizeof(EDIT_PORTAL), pfile);
		if (TempPort.Region)
		{
			memcpy((char *)&Region[NumRegion++], (char *)&TempPort, sizeof(EDIT_PORTAL));
		}
		else
		{
			memcpy((char *)&Portal[NumPortal++], (char *)&TempPort, sizeof(EDIT_PORTAL));
		}
	}
	fclose(pfile);
}


//
// WritePortals
//
// Writes out file header and portal data
//

void WritePortals(void)
{
	char	buf[256];
	long	ii, jj;
	VECTOR	v[4];
	VECTOR	pos[4];

	api.Log("Writing portal/regioned world data...\n\n");
	wsprintf(buf, "World has %d portals defined.\n", NumPortal);
	api.Log(buf);

	wsprintf(buf, "POR");
	buf[3] = 0;
	fwrite(buf, 1, sizeof(long), ofile);												// Write portal/region world header
	fputlong(NumPortal, ofile);															// Write number of portals

	for (ii = 0; ii < NumPortal; ii++)													// Write portal structures
	{
		fputlong(Portal[ii].ID1, ofile);												// Region ID of side 1
		fputlong(Portal[ii].ID2, ofile);												// Region ID of side 2

		SetVector(&v[0], -Portal[ii].Size[X], -Portal[ii].Size[Y], 0);					// Set vectors of corners of this portal
		SetVector(&v[1], Portal[ii].Size[X], -Portal[ii].Size[Y], 0);
		SetVector(&v[2], Portal[ii].Size[X], Portal[ii].Size[Y], 0);
		SetVector(&v[3], -Portal[ii].Size[X], Portal[ii].Size[Y], 0);
		for (jj = 0; jj < 4; jj++)														// Generate and write positions of corners in world space
		{
			RotTransVector(&Portal[ii].Matrix, &Portal[ii].Pos, &v[jj], &pos[jj]);		
			fputfloat(pos[jj].x, ofile);
			fputfloat(pos[jj].y, ofile);
			fputfloat(pos[jj].z, ofile);
		}
	}
}


//
// WriteRegionModels
//
// Generates and writes out models for each marked region. Each polygon in the world mesh is only
// included in ONE region only.
//


void WriteRegionModels(void)
{
	char	buf[256];
	long	ii, jj, kk;
	PLANE	Plane[3];
	VECTOR	Centre;
	long	Pass;
	long	MaxReg = -1;
	float	dist;
	HLOCAL	ListHandle;
	POLY	*ListPtr;
	POLY	*TempPtr;
	short	ListCnt;
	short	TempCnt;

	ListHandle = LocalAlloc(LPTR, sizeof(POLY) * PolyNum);
	ListPtr = (POLY *)LocalLock(ListHandle);

	wsprintf(buf, "World has %d regions defined.\nGetting region poly counts...\n", NumPortal);
	api.Log(buf);

	fputlong(NumRegion, ofile);															// Write number of regions
	
	for (ii = 0; ii < PolyNum; ii++)
	{
		PolyPtr[ii].Region = -1;
	}
	for (ii = 0; ii < NumRegion; ii++)													// Write region data
	{
		fputlong(Region[ii].ID1, ofile);
		fputfloat(Region[ii].Pos.x, ofile);
		fputfloat(Region[ii].Pos.y, ofile);
		fputfloat(Region[ii].Pos.z, ofile);
		for (jj = 0; jj < 9; jj++)
		{
			fputfloat(Region[ii].Matrix.m[jj], ofile);
		}
		fputfloat(Region[ii].Size[0], ofile);
		fputfloat(Region[ii].Size[1], ofile);
		fputfloat(Region[ii].Size[2], ofile);
		RegCnt[ii] = 0;
	}

	for (ii = 0; ii < NumRegion; ii++)													// Check all polys for region occupation, and set their region index
	{
		Plane[0].a = Region[ii].Matrix.m[RX];
		Plane[0].b = Region[ii].Matrix.m[RY];
		Plane[0].c = Region[ii].Matrix.m[RZ];
		Plane[0].d = -DotProduct((float *)&Region[ii].Matrix.mv[R], (float *)&Region[ii].Pos);
		Plane[1].a = Region[ii].Matrix.m[UX];
		Plane[1].b = Region[ii].Matrix.m[UY];
		Plane[1].c = Region[ii].Matrix.m[UZ];
		Plane[1].d = -DotProduct((float *)&Region[ii].Matrix.mv[U], (float *)&Region[ii].Pos);
		Plane[2].a = Region[ii].Matrix.m[LX];
		Plane[2].b = Region[ii].Matrix.m[LY];
		Plane[2].c = Region[ii].Matrix.m[LZ];
		Plane[2].d = -DotProduct((float *)&Region[ii].Matrix.mv[L], (float *)&Region[ii].Pos);


		for (jj = 0; jj < PolyNum; jj++)
		{
			if (PolyPtr[jj].Type & TYPE_QUAD)
			{
				Centre.x = (PolyPtr[jj].v[0].x + PolyPtr[jj].v[1].x + PolyPtr[jj].v[2].x + PolyPtr[jj].v[3].x) / 4;
				Centre.y = (PolyPtr[jj].v[0].y + PolyPtr[jj].v[1].y + PolyPtr[jj].v[2].y + PolyPtr[jj].v[3].y) / 4;
				Centre.z = (PolyPtr[jj].v[0].z + PolyPtr[jj].v[1].z + PolyPtr[jj].v[2].z + PolyPtr[jj].v[3].z) / 4;
			}
			else
			{
				Centre.x = (PolyPtr[jj].v[0].x + PolyPtr[jj].v[1].x + PolyPtr[jj].v[2].x) / 3;
				Centre.y = (PolyPtr[jj].v[0].y + PolyPtr[jj].v[1].y + PolyPtr[jj].v[2].y) / 3;
				Centre.z = (PolyPtr[jj].v[0].z + PolyPtr[jj].v[1].z + PolyPtr[jj].v[2].z) / 3;
			}
			Pass = 1;
			for (kk = 0; kk < 3; kk++)
			{
				dist = VecDotPlane(&Centre, &Plane[kk]);
				if (dist < -Region[ii].Size[kk] || dist > Region[ii].Size[kk])
				{
					Pass = 0;
					break;
				}
			}
			if ((Pass) & (PolyPtr[jj].Region == -1))
			{
				PolyPtr[jj].Region = Region[ii].ID1;
				RegCnt[Region[ii].ID1]++;
				if (Region[ii].ID1 > MaxReg) { MaxReg = Region[ii].ID1; }
			}
		}
	}

	for (ii = 0; ii < MaxReg; ii++)
	{
		wsprintf(buf, "Region ID %d: %d polys.\n", ii, RegCnt[ii]);
		api.Log(buf);
	}

	api.Log("Writing region model data...\n");
	fputlong(MaxReg, ofile);															// Write number of region models
	for (ii = 0; ii < MaxReg; ii++)														// Output region models
	{
		ListCnt = 0;
		for (jj = 0; jj < PolyNum; jj++)
		{
			if (PolyPtr[jj].Region == ii)
			{
				memcpy((char *)&ListPtr[ListCnt++], (char *)&PolyPtr[jj], sizeof(POLY));
			}
		}
		fputlong(ii, ofile);															// Write region ID number for this model
		TempPtr = PolyPtr;
		TempCnt = PolyNum;
		PolyPtr = ListPtr;
		PolyNum = ListCnt;
		SortModelN64();																	// Sort region model
		OutputModelN64();																// Write region model
		PolyPtr = TempPtr;
		PolyNum = TempCnt;
	}
}


//////////////////////////
// rot / trans a vector //
//////////////////////////

void RotTransVector(MATRIX *mat, VECTOR *trans, VECTOR *in, VECTOR *out)
{
	out->x = in->x * mat->m[RX] + in->y * mat->m[UX] + in->z * mat->m[LX] + trans->x;
	out->y = in->x * mat->m[RY] + in->y * mat->m[UY] + in->z * mat->m[LY] + trans->y;
	out->z = in->x * mat->m[RZ] + in->y * mat->m[UZ] + in->z * mat->m[LZ] + trans->z;
}


//
// fputlong
//
// Write big endian long value to a file
//

void fputlong(long value, FILE *file)
{
	long	temp;

	temp = value >> 24;
	fwrite(&temp, 1, 1, file);
	temp = value >> 16;
	fwrite(&temp, 1, 1, file);
	temp = value >> 8;
	fwrite(&temp, 1, 1, file);
	fwrite(&value, 1, 1, file);
}


//
// fputshort
//
// Write big endian short value to a file
//

void fputshort(short value, FILE *file)
{
	long	temp;

	temp = value >> 8;
	fwrite(&temp, 1, 1, file);
	fwrite(&value, 1, 1, file);
}


//
// Write big endian float value to a file
//

void fputfloat(float value, FILE *file)
{
	char	*temp;

	temp = (char *)&value;	
	fwrite(temp + 3, 1, 1, file);
	fwrite(temp + 2, 1, 1, file);
	fwrite(temp + 1, 1, 1, file);
	fwrite(temp + 0, 1, 1, file);
}
