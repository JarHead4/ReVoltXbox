
#include "fileio.h"
#include "editor.h"

#include <endstrm.h>
#include <assert.h>

using namespace std;

extern char**		FileList;
extern U16			FileCount;
extern DIRECTION	ModuleDirection;
extern S16			ModuleElevation;

void ReadModules(EndianInputStream& is, TRACKTHEME* theme);
void ReadUnits(EndianInputStream& is, TRACKTHEME* theme);
void ReadMeshes(EndianInputStream& is, TRACKTHEME* theme);
void ReadPolygons(EndianInputStream& is, TRACKTHEME* theme);
void ReadVertices(EndianInputStream& is, TRACKTHEME* theme);
void ReadUVCoords(EndianInputStream& is, TRACKTHEME* theme);
void ReadUVPolygons(EndianInputStream& is, TRACKTHEME* theme);


//-----------------------------------------------------------------------------
//  Name:       ReadRTUFile
//  Purpose:	Loads data from RTU (Revolt Track Unit) file into memory
//-----------------------------------------------------------------------------


bool ReadRTUFile(const string& pathname, TRACKTHEME* theme)
{
	bool fileloaded = false;
	
	char filename[MAX_PATH];

	sprintf(filename, "%s\\trackunit.rtu", pathname.c_str());
	EndianInputStream is(filename);
	if(is.is_open())
	{
		U32 fileid = is.GetU32();
		if(fileid == MAKE_ID('R','T','U',' '))
		{
			U16 version = is.GetU16();
			if(version == RTU_READ_VERSION)
			{
				VALID_TARGETS targets = is.GetU16();
				
				ReadVertices(is, theme);
				ReadPolygons(is, theme);
				ReadMeshes(is, theme);
				ReadUVCoords(is, theme);
				ReadUVPolygons(is, theme);
				ReadUnits(is, theme);
				ReadModules(is, theme);
				theme->TPageCount = is.GetU32();

				fileloaded = true;
			}
		}
	}
	return fileloaded;
}

void ReadUVCoords(EndianInputStream& is, TRACKTHEME* theme)
{
	theme->UVCoordCount = is.GetU32();
	theme->UVCoords = new RevoltUVCoord[theme->UVCoordCount];
	for(U32 v = 0; v < theme->UVCoordCount; v++)
	{
		theme->UVCoords[v].U = is.GetFloat();
		theme->UVCoords[v].V = is.GetFloat();
	}
}

void ReadUVPolygons(EndianInputStream& is, TRACKTHEME* theme)
{
	theme->UVPolyCount = is.GetU32();
	theme->UVPolys = new BASICPOLY[theme->UVPolyCount];
	BASICPOLY* polyptr = theme->UVPolys;
	for(U32 p = 0; p < theme->UVPolyCount; p++)
	{
		U16 vertexcount = is.GetU16();
		polyptr->Vertices[0] = is.GetU32();
		polyptr->Vertices[1] = is.GetU32();
		polyptr->Vertices[2] = is.GetU32();
		if(vertexcount > 3)
		{
			polyptr->IsTriangle = FALSE;
			do{
				polyptr->Vertices[3] = is.GetU32();
			}while(--vertexcount > 3);
		}
		else
		{
			polyptr->IsTriangle = TRUE;
		}
		polyptr++;			
	}
}

void ReadVertices(EndianInputStream& is, TRACKTHEME* theme)
{
	theme->VertCount = is.GetU32();
	theme->Verts = new D3DVECTOR[theme->VertCount];
	for(U32 v = 0; v < theme->VertCount; v++)
	{
		theme->Verts[v].x = is.GetFloat();
		theme->Verts[v].y = is.GetFloat();
		theme->Verts[v].z = is.GetFloat();
	}
}

void ReadPolygons(EndianInputStream& is, TRACKTHEME* theme)
{
	theme->PolyCount = is.GetU32();
	theme->Polys = new BASICPOLY[theme->PolyCount];
	BASICPOLY* polyptr = theme->Polys;
	for(U32 p = 0; p < theme->PolyCount; p++)
	{
		U16 vertexcount = is.GetU16();
		polyptr->Vertices[0] = is.GetU32();
		polyptr->Vertices[1] = is.GetU32();
		polyptr->Vertices[2] = is.GetU32();
		if(vertexcount > 3)
		{
			polyptr->IsTriangle = FALSE;
			do{
				polyptr->Vertices[3] = is.GetU32();
			}while(--vertexcount > 3);
		}
		else
		{
			polyptr->IsTriangle = TRUE;
		}
		polyptr++;			
	}
}

void ReadMeshes(EndianInputStream& is, TRACKTHEME* theme)
{
	theme->MeshCount = is.GetU32();
	theme->Meshes = new MESH[theme->MeshCount];
	for(U32 m = 0; m < theme->MeshCount; m++)
	{
		U16 polysetcount = theme->Meshes[m].PolySetCount = is.GetU16();
		theme->Meshes[m].PolySets = new POLYSET*[polysetcount];
		POLYSET** psptr = theme->Meshes[m].PolySets;
		for(U16 ps = 0; ps < polysetcount; ps++)
		{
			POLYSET* polyset = new POLYSET;
			*psptr = polyset;
			psptr++;
			polyset->PolygonCount = is.GetU32();
			polyset->VertexCount = 0;
			polyset->Indices = new U32[polyset->PolygonCount];
			for(U32 p = 0; p < polyset->PolygonCount; p++)
			{
				polyset->Indices[p] = is.GetU32();
				polyset->VertexCount += 3;
				if(theme->Polys[polyset->Indices[p]].IsTriangle == FALSE)
				{
					polyset->VertexCount += 3;
				}
			}
		}
	}
}

void ReadUnits(EndianInputStream& is, TRACKTHEME* theme)
{
	theme->UnitCount = is.GetU32();
	theme->Units = new TRACKUNIT*[theme->UnitCount];
	for(U32 i = 0; i < theme->UnitCount; i++)
	{
		TRACKUNIT* unit = new TRACKUNIT;
		theme->Units[i] = unit;
		unit->MeshID = is.GetU32();
		unit->UVPolyCount = is.GetU32();
		unit->UVPolys = new UVPOLYINSTANCE[unit->UVPolyCount];
		for(U32 p = 0; p < unit->UVPolyCount; p++)
		{
			unit->UVPolys[p].TPageID = is.GetU32();
			unit->UVPolys[p].PolyID = is.GetU32();
			unit->UVPolys[p].Rotation = is.GetU8();
			unit->UVPolys[p].Reversed = (is.GetU8() != 0);
		}
		unit->SurfaceCount = is.GetU32();
		unit->Surfaces = new U32[unit->SurfaceCount];
		for(U32 s = 0; s < unit->SurfaceCount; s++)
		{
			unit->Surfaces[s] = is.GetU32();
		}
	}
}

void ReadModules(EndianInputStream& is, TRACKTHEME* theme)
{
	theme->ModuleCount = is.GetU32();

	theme->Modules = new TRACKMODULE*[theme->ModuleCount];
	for(U32 m = 0; m < theme->ModuleCount; m++)
	{
		TRACKMODULE* module = new TRACKMODULE;
		theme->Modules[m] = module;
		
		is.GetPaddedText(module->Name, sizeof(module->Name) - 1);

		module->InstanceCount = is.GetU16();
		module->Instances = new RevoltTrackUnitInstance*[module->InstanceCount];
		for(U32 i = 0; i < module->InstanceCount; i++)
		{
			RevoltTrackUnitInstance* instance = new RevoltTrackUnitInstance;
			module->Instances[i] = instance;
			instance->UnitID = is.GetU32();
			instance->Direction = (DIRECTION)is.GetU16();
			instance->XPos = is.GetS16();
			instance->YPos = is.GetS16();
			instance->Elevation = is.GetS16();
		}
		module->ZoneCount = is.GetU16();
		module->Zones = new TRACKZONE*[module->ZoneCount];
		for(U32 n = 0; n < module->ZoneCount; n++)
		{
			TRACKZONE* zone = new TRACKZONE;
			module->Zones[n] = zone;
			zone->Centre.X = is.GetFloat();
			zone->Centre.Y = is.GetFloat();
			zone->Centre.Z = is.GetFloat();
			zone->XSize = is.GetFloat();
			zone->YSize = is.GetFloat();
			zone->ZSize = is.GetFloat();
			zone->Links[0].Position.X = is.GetFloat();
			zone->Links[0].Position.Y = is.GetFloat();
			zone->Links[0].Position.Z = is.GetFloat();
			zone->Links[1].Position.X = is.GetFloat();
			zone->Links[1].Position.Y = is.GetFloat();
			zone->Links[1].Position.Z = is.GetFloat();
		}
	}
}

bool NamedFileExists(const char* filename)
{
	WIN32_FIND_DATA find_data;
	HANDLE h = FindFirstFile((LPCTSTR)filename, &find_data);
	if(h == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	FindClose(h);
	return true;
}

bool GetFileDescription(const char* filename, char* description)
{
	assert(description != NULL);

	bool success = false;
	EndianInputStream is(filename);
	if(is.is_open())
	{
		U32 fileid = is.GetU32();
		if(fileid == MAKE_ID('T','D','F',' '))
		{
			is.GetU16(); //skip version info
			is.GetPaddedText(description, MAX_DESCRIPTION_LENGTH - 1);
			success = true;
		}
	}
	return success;
}

bool GetDescribedFilename(const char* description, char* filename)
{
	assert(filename != NULL);

	bool descriptionfound = false;
	bool filefound = false;
	WIN32_FIND_DATA find_data;
	HANDLE h = FindFirstFile((LPCTSTR)"TDF\\FILE????.TDF", &find_data);
	if(h != INVALID_HANDLE_VALUE)
	{
		filefound = true;
		while((filefound == true) && (descriptionfound == false))
		{
			char filedescription[MAX_DESCRIPTION_LENGTH];
			sprintf(filename, "TDF\\%s", find_data.cFileName); 
			GetFileDescription(filename, filedescription);
			if(strcmp(filedescription, (const char*)description) == 0)
			{
				descriptionfound = true;
			}
			else
			{
				strcpy(filename, ""); //blank out filename if descriptions don't match in case user decides
									  //to ignore return value otherwise it will be the name of the last file encountered
				filefound = (FindNextFile(h, &find_data) == TRUE);
			}
		}
		FindClose(h);
	}
	return descriptionfound;
}

bool DescribedFileExists(const char* description)
{
	char filename[MAX_PATH];
	return GetDescribedFilename(description, filename);
}

bool NextFreeFile(char* filename)
{
	assert(filename != NULL);

	U16		n = 0;
	bool	foundfreefile = false;
	while((foundfreefile == false) && (n < 10000))
	{
		sprintf(filename, "TDF\\FILE%04d.TDF", n);
		foundfreefile = (NamedFileExists(filename) == false);
		n++;
	}
	if(foundfreefile == false)
	{
		strcpy(filename, "");
	}
	return foundfreefile;
}

bool NextFreeDescription(char* description)
{
	assert(description != NULL);

	U16 n = 0;
	bool foundfreedescription = false;
	while((foundfreedescription == false) && (n < 10000))
	{
		sprintf(description, "Track %d", n);
		foundfreedescription = (DescribedFileExists(description) == false);
		n++;
	}
	if(foundfreedescription == false)
	{
		strcpy(description, "");
	}
	return foundfreedescription;
}

bool WriteTDF(const char* filename, const TRACKDESC* track)
{
	bool success = false;
	CreateDirectory("TDF", NULL);
	EndianOutputStream os(filename);
	if(os.is_open())
	{
		os.PutU32(MAKE_ID('T','D','F',' '));
		os.PutU16(TDF_READ_VERSION);

		os.PutPaddedText(track->Name);
		os.PutU16(track->ThemeType);
		
		os.PutU32(SECTION_MODULE_GRID);
		os.PutU16(track->Width);
		os.PutU16(track->Height);

		U32 m = track->Width * track->Height;
		for(U32 i = 0; i < m; i++)
		{
			RevoltTrackModuleInstance* module;
			module = &track->Modules[i];
			os.PutS16(module->ModuleID);
			os.PutU16((U16)module->Direction);
			os.PutU16(module->Elevation);
		}

		success = true;
	}
	return success;
}

bool ReadTDF(const char* filename, TRACKDESC* track)
{
	bool success = false;
	EndianInputStream is(filename);
	if(is.is_open())
	{
		U32 fileid = is.GetU32();
		if(fileid == MAKE_ID('T','D','F',' '))
		{
			U16 version = is.GetU16();
			if(version == TDF_READ_VERSION)
			{
				char name[MAX_DESCRIPTION_LENGTH];
				is.GetPaddedText(name, MAX_DESCRIPTION_LENGTH);
				is.GetU16();	//skip Theme type for now
				is.GetU32();	//skip Section Flags
				U16 width = is.GetU16();
				U16 height = is.GetU16();
				CreateTrackAndCursor(track, width, height, name);
				U32 m = track->Width * track->Height;
				for(U32 i = 0; i < m; i++)
				{
					RevoltTrackModuleInstance* module;
					module = &track->Modules[i];
					INDEX mod = is.GetU16();	//get module ID from file
					ModuleDirection = (DIRECTION)is.GetU16();
					ModuleElevation = is.GetU16();
					if(mod != MAX_INDEX)
					{
						U16 xpos = i % track->Width;
						U16 ypos = i / track->Width;
						PlaceModuleNumber(track, mod, xpos, ypos);
					}
					module->ModuleID = mod;
					module->Direction = ModuleDirection;
					module->Elevation = ModuleElevation;
				}
			}
		}
	}
	return success;
}

U16 CountTDFFiles(void)
{
	U16 filecount = 0;
	bool filefound = false;
	WIN32_FIND_DATA find_data;
	HANDLE h = FindFirstFile((LPCTSTR)"TDF\\FILE????.TDF", &find_data);
	if(h != INVALID_HANDLE_VALUE)
	{
		filefound = true;
		while(filefound == true)
		{
			filefound = (FindNextFile(h, &find_data) == TRUE);
			filecount++;
		}
		FindClose(h);
	}
	return filecount;
}

void EraseFileList()
{
	if(FileList != NULL)
	{
		while(FileCount--)
		{
			delete[] FileList[FileCount];
		}
		delete[] FileList;
		FileCount = 0;
		FileList = NULL;
	}
}

bool MakeFileList(void)
{
	EraseFileList();

	U16 numberoffiles = CountTDFFiles();
	FileList = new char*[numberoffiles];
	if(FileList != NULL)
	{
		char filename[MAX_PATH];
		WIN32_FIND_DATA find_data;
		HANDLE h = FindFirstFile((LPCTSTR)"TDF\\FILE????.TDF", &find_data);
		if(h != INVALID_HANDLE_VALUE)
		{
			bool filefound = true;
			while((FileCount < numberoffiles) && (filefound == true))
			{
				FileList[FileCount] = new char[MAX_DESCRIPTION_LENGTH];
				if(FileList[FileCount] != NULL)
				{
					sprintf(filename, "TDF\\%s", find_data.cFileName); 
					GetFileDescription(filename, FileList[FileCount]);
					filefound = (FindNextFile(h, &find_data) == TRUE);
					FileCount++;
				}
				else
				{
					EraseFileList();
					break;
				}
			}
		}
	}
	return (FileList != NULL);
}

//*****************************************************************
bool NamedLevelExists(const char* levelname)
{
	assert(levelname != NULL);
	
	WIN32_FIND_DATA find_data;
	char levelpath[MAX_PATH];
	sprintf(levelpath, "..\\game\\levels\\%s", levelname);

	HANDLE h = FindFirstFile((LPCTSTR)levelpath, &find_data);
	if(h == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	FindClose(h);
	return true;
}

bool GetLevelDescription(const char* levelname, char* description)
{
	assert(description != NULL);

	char linebuffer[MAX_DESCRIPTION_LENGTH + 40];
	static char nametoken[] = "NAME "; //note - the trailing space is important
	
	char infospec[MAX_PATH];
	sprintf(infospec, "..\\game\\levels\\%s\\%s.inf", levelname, levelname);

	bool success = false;
	FILE* fi;
	fi = fopen(infospec, "rt");
	if(fi != NULL)
	{
		bool done = false;
		while((done == false) && (success == false))
		{
			char* lineptr = fgets(linebuffer, sizeof(linebuffer), fi);
			if(lineptr == NULL)
			{	
				done = true;
			}
			else
			{
				if(strnicmp(nametoken, linebuffer, strlen(nametoken)) == 0)
				{
					char* openquote = strchr(linebuffer, '\'');
					if(openquote != NULL)
					{
						char* dash = strchr(openquote+1, '-');
						if(dash != NULL)
						{
							char* closequote = strchr(dash+1, '\'');
							if(closequote != NULL)
							{
								*closequote = '\0';
								strncpy(description, dash+1, MAX_DESCRIPTION_LENGTH - 1);
								success = true;
							}
						}
					}
				}
			}
		}
		fclose(fi);
	}
	return success;
}

bool GetDescribedLevelName(const char* description, char* levelname)
{
	assert(levelname != NULL);

	bool descriptionfound = false;
	bool filefound = false;
	WIN32_FIND_DATA find_data;
	
	HANDLE h = FindFirstFile((LPCTSTR)"..\\game\\levels\\USER???", &find_data);
	if(h != INVALID_HANDLE_VALUE)
	{
		bool folderfound = true;
		while((folderfound == true) && (descriptionfound == false))
		{
			char leveldescription[MAX_DESCRIPTION_LENGTH];
			strcpy(levelname, find_data.cFileName); 
			GetLevelDescription(levelname, leveldescription);
			if(strcmp(leveldescription, (const char*)description) == 0)
			{
				descriptionfound = true;
			}
			else
			{
				strcpy(levelname, ""); //blank out levelname if descriptions don't match in case user decides
									   //to ignore return value otherwise it will be the name of the last file encountered
				folderfound = (FindNextFile(h, &find_data) == TRUE);
			}
		}
		FindClose(h);
	}
	return descriptionfound;
}

bool DescribedLevelExists(const char* description)
{
	char levelname[MAX_PATH];
	return GetDescribedLevelName(description, levelname);
}

bool NextFreeLevel(char* levelname)
{
	assert(levelname != NULL);

	U16	 n = 0;
	bool foundfreelevel = false;

	while((foundfreelevel == false) && (n < 1000))
	{
		sprintf(levelname, "USER%03d", n);
		foundfreelevel = (NamedLevelExists(levelname) == false);
		n++;
	}
	if(foundfreelevel == false)
	{
		strcpy(levelname, "");
	}
	return foundfreelevel;
}

//*****************************************************************
