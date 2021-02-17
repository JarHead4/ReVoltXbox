//-------------------------------------------------------------------------------------------------
// Re-Volt Track Unit Extraction Program.
//
//-------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "HabExtract.h"		// Structures are in here
#include <MappedVector.h>
#include <Primitives.h>
#include <Constants.h>
#include <ScriptObject.h>
#include <OutputWaveFront.h>
#include <OutputUnitFile.h>
#include <fstream>
#include <lbmsave.h>

using namespace std;

//-------------------------------------------------------------------------------------------------
// Global variables
//-------------------------------------------------------------------------------------------------
RevoltTheme* Theme;
U32          VerticesScanned;
U32          PolygonsScanned;

//-------------------------------------------------------------------------------------------------
// Prototypes for data processing routines
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
// ExtractComponent(rv_Model* model, RevoltPolySet* polyset)
//
//
//-------------------------------------------------------------------------------------------------
bool ExtractComponent(rv_Model* model, RevoltPolySet* polyset, RevoltTrackUnit* unit)
{
	ASSERT(polyset != NULL);
	
	bool success = false;
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	JC_RecordRefList& childrefs = model->ChildList();
	POSITION childpos = childrefs.GetHeadPosition();
	PS_RecordInfo recinfo;
	while(childpos && !success)
	{
		JC_RecordRef* childref = (JC_RecordRef*)childrefs.GetAt(childpos);

		U32 recordnumber = childref->RecordNum();
		if(recordnumber != PS_Database::NullRecord)
		{
			db.FirstRecord(recinfo);
			db.GetInfo(recordnumber,recinfo);
			if(recinfo.Name() == polyset->Name().c_str())
			{
				JC_Template* childmodel = GET_TEMPLATE(&db, recordnumber);
				JC_VertexList* vertices = childmodel->VertexList();
				JC_FaceList*   faces = childmodel->FaceList();

				RevoltVertex rv;
				RevoltUVCoord uv;
				RevoltUVPolygon uvpoly;
				RevoltPolygon poly;
				PS_FloatUVArray uvarray;
				CString errorinfo;
				CString modelinfo;
				U32 rotation;
				U32 polyshift;
				U32 uvshift;

				POSITION facepos = faces->GetHeadPosition();
				while(facepos)
				{
					poly.clear();
					JC_Face* face = (JC_Face*)faces->GetAt(facepos);
					const JC_PointList* points = face->PointList();
					POSITION pointpos = points->GetTailPosition();
					while(pointpos)
					{
						JC_Point* point = (JC_Point*)points->GetAt(pointpos);
						rv.X = (point->Vertex()->X() * GameScale);
						rv.Y = -(point->Vertex()->Y() * GameScale);
						rv.Z = (point->Vertex()->Z() * GameScale);
						U32 vertindex = Theme->InsertVertex(rv);
						poly.push_back(vertindex);
						points->GetPrev(pointpos);
						VerticesScanned++;
					}
					polyshift = poly.SmallestFirst();
					if(poly.size() < 3)
					{
						errorinfo.Format("*********** %s:- less than 3 verts ***********\n", childmodel->Path()); 
						api.Log(errorinfo);
					}
					U32 polyindex = Theme->InsertPolygon(poly);
					polyset->push_back(polyindex);
					PolygonsScanned ++;

					if(unit != NULL)
					{
						uvpoly.clear();
						uvarray.RemoveAll();
						uvshift = 0;

						const PS_Texture* texture = face->FrontTexture();
						U32 texturerecord = PS_Database::NullRecord;
						if(texture != NULL)
						{
							texturerecord = texture->GetBitmapID();
							texture->GetFloatUVArray(uvarray);
							U32 c = uvarray.GetSize();
							while(c--)
							{
								uv.U = uvarray[c].u;
								uv.V = uvarray[c].v;
								U32 uvindex = Theme->InsertUVCoord(uv);
								uvpoly.push_back(uvindex);
							}
							uvshift = uvpoly.SmallestFirst();
							texture->Release();
							if(poly.size() != uvpoly.size())
							{
								errorinfo.Format("*********** poly/texture mismatch ***********\n"); 
								api.Log(errorinfo);
							}
						}
						else
						{
							for(U32 c = 0; c < poly.size(); c++)
							{
								uvpoly.push_back(0);
							}
						}
						U32 uvpolyindex = Theme->InsertUVPolygon(uvpoly);

						BA_VertexOrient& orient = face->FrontOrient();
						U32 ffv	= (poly.size() -1) - orient.FirstFaceVertex;
						U32 ftv	= (poly.size() -1) - orient.FirstTextureVertex;
						if(orient.Reversed == FALSE)
						{
							rotation = poly.size() - ffv;
							rotation += ftv;
							rotation += polyshift;
							rotation += poly.size() - uvshift;
							rotation %= poly.size();
						}
						else
						{
							rotation = ffv;
							rotation += ftv;
							rotation += poly.size() - polyshift;
							rotation += poly.size() - uvshift;
							rotation %= poly.size();
						}

						UVPolyInstance* pi = new UVPolyInstance;
						pi->TPageID = Theme->InsertTPageRecnum(texturerecord);
						pi->PolyID = uvpolyindex;
						pi->Rotation = rotation;
						pi->Reversed = (orient.Reversed != FALSE);
						unit->UVPolySet.Insert(pi);
					}
					faces->GetNext(facepos);
				}
				modelinfo.Format("%s:- %d verts, %d faces\n", childmodel->Path(), vertices->GetCount(), faces->GetCount()); 
///				api.Log(modelinfo);
				success = true;
				childmodel->Release();
			}
		}
		childrefs.GetNext(childpos);
	}
	return success;
}

//-------------------------------------------------------------------------------------------------
// ExtractSurfaces(rv_Model* model, RevoltTrackUnit* unit)
//
//
//-------------------------------------------------------------------------------------------------
bool ExtractSurfaces(rv_Model* model, string* name, RevoltTrackUnit* unit)
{
	ASSERT(model != NULL);
	ASSERT(name != NULL);
	ASSERT(unit != NULL);
	
	bool success = false;
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	JC_RecordRefList& childrefs = model->ChildList();
	POSITION childpos = childrefs.GetHeadPosition();
	PS_RecordInfo recinfo;
	while(childpos && !success)
	{
		JC_RecordRef* childref = (JC_RecordRef*)childrefs.GetAt(childpos);

		U32 recordnumber = childref->RecordNum();
		if(recordnumber != PS_Database::NullRecord)
		{
			db.FirstRecord(recinfo);
			db.GetInfo(recordnumber,recinfo);
			if(recinfo.Name() == name->c_str())
			{
				U32 surfacerecnum = db.FindRecord("Properties\\Material", JC_Record::TypePropertyName);
				JC_Template* childmodel = GET_TEMPLATE(&db, recordnumber);
				JC_VertexList* vertices = childmodel->VertexList();
				JC_FaceList*   faces = childmodel->FaceList();
				POSITION facepos = faces->GetHeadPosition();
				while(facepos)
				{
					U32 surface = 0; //default to 'NORMAL' surface
					JC_Face* face = (JC_Face*)faces->GetAt(facepos);
					JC_PropertyList* propertylist = face->PropertyList();
					POSITION propertypos = propertylist->GetHeadPosition();
					BOOL found = FALSE;
					while (propertypos && (found == FALSE))
					{
						JC_Property * property = (JC_Property*)propertylist->GetNext(propertypos);
						if (property)
						{
							JC_Data * value = property->CopyValue();
							if (value)
							{
								if (value->Token() == JC_Data::TokEnumRef)
								{
									JC_EnumRef * enum_ref = (JC_EnumRef*)value;
									if(property->GetNameRecordNumber() == surfacerecnum)
									{
										surface = (U32)*enum_ref;
										found = true;
									}
								}
								delete value;
							}
						}
					}
					unit->SurfaceSet.push_back(surface);
					faces->GetNext(facepos);
				}
				success = true;
				childmodel->Release();
			}
		}
		childrefs.GetNext(childpos);
	}
	return success;
}

//-------------------------------------------------------------------------------------------------
// ExtractUnit(U32 recordnumber, RevoltMesh* mesh, RevoltTrackUnit* unit)
//
// Extracts the information from the specified model and stores it in 'mesh', filling 'unit' with
// UV polygon information (for the 'pan' component only)
//-------------------------------------------------------------------------------------------------
void ExtractUnit(U32 recordnumber, RevoltMesh* mesh, RevoltTrackUnit* unit)
{
	ASSERT(mesh != NULL);
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	static string pegname("peg");
	static string panname("pan");
	static string hullname("hul_00");

	if(recordnumber != PS_Database::NullRecord)
	{
		JC_Template* model = GET_TEMPLATE(&db, recordnumber);
		RevoltPolySet* pegset = new RevoltPolySet(pegname);
		mesh->push_back(pegset);
		ExtractComponent(model, pegset, NULL);

		RevoltPolySet* panset = new RevoltPolySet(panname);
		mesh->push_back(panset);
		ExtractComponent(model, panset, unit);

		RevoltPolySet* hullset = new RevoltPolySet(hullname);
		if(ExtractComponent(model, hullset, NULL))
		{
			mesh->push_back(hullset);
			ExtractSurfaces(model, &hullname, unit);
		}
		else
		{
			delete hullset;
		}
		model->Release();
	}
}
//-------------------------------------------------------------------------------------------------
// ExtractUnit(const CString& recordspec, RevoltMesh* mesh)
//
// Extracts the information from the specified model and stores it in 'mesh'
//-------------------------------------------------------------------------------------------------
void ExtractUnit(const CString& recordspec, RevoltMesh* mesh, RevoltTrackUnit* unit)
{
	ASSERT(mesh != NULL);
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	U32 recordnumber = db.FindRecord(recordspec, JC_Record::TypeTemplate);
	if(recordnumber != PS_Database::NullRecord)
	{
		ExtractUnit(recordnumber, mesh, unit);
	}
	else
	{
		api.Log("Missing Model:- " + recordspec + "\n"); 
	}
}

//-------------------------------------------------------------------------------------------------
// IsObjectAUnit(const JC_Object* object)
//
// Determines whether the object represents a unit or not
// This is done by checking the first character of the name to ensure that it is an
// alphabetical character - non-alphabetical characters indicate a special use object
// 
//-------------------------------------------------------------------------------------------------
bool IsObjectAUnit(const JC_Object* object)
{
	ASSERT(object != NULL);
	CString name = object->Name();
	return (isalpha(name[0]) != 0);
}

//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------
void GetObjectPosition(RevoltVertex* position, const JC_Object* object)
{
	ASSERT(object != NULL);
	const JC_Anchor& anchor = object->Anchor();
	const JC_Vector& pin = anchor.PinVector();

	position->X = pin.X() * GameScale;
	position->Y = -pin.Y() * GameScale;
	position->Z = pin.Z() * GameScale;
}

//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------
void GetZoneSizes(TRACKZONE* zone, JC_Object* object)
{
	ASSERT(object != NULL);

	float minx, miny, minz;
	float maxx, maxy, maxz;

	minx = miny = minz = FLT_MAX;
	maxx = maxy = maxz = -FLT_MAX;
	
	JC_VertexArray vertices;
	JC_Vertex*	vert;
	const JC_Vector& scale = object->Scale();
	
//	find the bounding cuboid then put the sizes into the zone
	object->GetVertices(vertices);
	U32 n = vertices.GetSize();
	while(n--)
	{
		vert = vertices[n];
		minx = min(minx, vert->X() * scale.X() * GameScale);
		miny = min(miny, -vert->Y() * scale.Y() * GameScale);
		minz = min(minz, vert->Z() * scale.Z() * GameScale);
		maxx = max(maxx, vert->X() * scale.X() * GameScale);
		maxy = max(maxy, -vert->Y() * scale.Y() * GameScale);
		maxz = max(maxz, vert->Z() * scale.Z() * GameScale);
		delete vert;
	}
	//shift the cuboid into position
	minx += zone->Centre.X;
	maxx += zone->Centre.X;
	miny += zone->Centre.Y;
	maxy += zone->Centre.Y;
	minz += zone->Centre.Z;
	maxz += zone->Centre.Z;

	//calculate the dimensions (from centre of cuboid)
	zone->XSize = ((maxx - minx) / 2);
	zone->YSize = ((maxy - miny) / 2);
	zone->ZSize = ((maxz - minz) / 2);
	//finally adjust the zones position so that it is in the centre of the cuboid
	zone->Centre.X = (minx + zone->XSize);
	zone->Centre.Y = (miny + zone->YSize);
	zone->Centre.Z = (minz + zone->ZSize);
}

//-------------------------------------------------------------------------------------------------
// 
//-------------------------------------------------------------------------------------------------
JC_Object* FindChildObject(const JC_Template* model, CString& name)
{
	ASSERT(model != NULL);
	
	JC_Object* result = NULL;
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();
	const JC_RecordRefList& childrefs = model->ChildList();
	POSITION childpos = childrefs.GetHeadPosition();
	PS_RecordInfo recinfo;
	while(childpos && (result == NULL))
	{
		JC_RecordRef* childref = (JC_RecordRef*)childrefs.GetAt(childpos);

		U32 recordnumber = childref->RecordNum();
		if(recordnumber != PS_Database::NullRecord)
		{
			db.GetInfo(recordnumber, recinfo);
			if(recinfo.Name() == name)
			{
				result = GET_OBJ(&db, recordnumber);
			}
		}
		childrefs.GetNext(childpos);
	}
	return result;
}

//-------------------------------------------------------------------------------------------------
// ExtractZoneInfo(RevoltModule* module, const JC_Template* model)
//
// Searches the model for all zone information (and associated links) and stores the results
// into module
//-------------------------------------------------------------------------------------------------
void ExtractZoneInfo(RevoltModule* module, const JC_Template* model)
{
	bool zonefound;
	U32 n = 1;
	CString name;
	TRACKZONE* zone = NULL;
	do
	{
		zonefound = false;
		name.Format("$z%d", n);
		JC_Object* zoneobject = FindChildObject(model, name);
		if(zoneobject)
		{
			name.Format("$l%da", n);
			JC_Object* linkobjecta = FindChildObject(model, name);
			if(linkobjecta)
			{
				name.Format("$l%db", n);
				JC_Object* linkobjectb = FindChildObject(model, name);
				if(linkobjectb)
				{
					zone = new TRACKZONE;
					GetObjectPosition(&zone->Centre, zoneobject);
					GetZoneSizes(zone, zoneobject);
					GetObjectPosition(&zone->Links[0].Position, linkobjecta);
					GetObjectPosition(&zone->Links[1].Position, linkobjectb);
					linkobjectb->Release();
					module->Zones.Insert(zone);
					zonefound = true;
				}
				linkobjecta->Release();
			}
			zoneobject->Release();
		}
		n++;
	}while(zonefound == true);

}

//-------------------------------------------------------------------------------------------------
// ExtractModule(const CString& recordspec, RevoltTheme* theme)
//
// Extracts the information from the specified module, stores it in a locally created module
// then attaches that modeule to 'theme'
//-------------------------------------------------------------------------------------------------
void ExtractModule(const CString& recordspec, RevoltTheme* theme)
{
	ASSERT(theme != NULL);
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	U32 recordnumber = db.FindRecord(recordspec, JC_Record::TypeTemplate);
	if(recordnumber != PS_Database::NullRecord)
	{
		JC_Template* model = GET_TEMPLATE(&db, recordnumber);
		
		string description((LPCSTR)model->Name());
		JC_PropertyList* properties = model->CreateFullPropertyList();
		JC_Property*	 desc_prop = properties->FindProperty("Description");
		if(desc_prop != NULL)
		{
			JC_Data* data =	desc_prop->CopyValue();
			description = (LPCSTR)data->String();
			delete data;
		}
		delete properties;

		RevoltModule* module = new RevoltModule(description);

 		theme->InsertModule(module);

		CString response;
		response.Format("Processing module %s\n", recordspec);
		api.Log(response);

		response.Format("Name - %s\n", description.c_str());
		api.Log(response);

		api.Indent(4);

		JC_RecordRefList& childrefs = model->ChildList();
		POSITION childpos = childrefs.GetHeadPosition();
		PS_RecordInfo recinfo;
		while(childpos)
		{
			JC_RecordRef* childref = (JC_RecordRef*)childrefs.GetAt(childpos);

			U32 recordnumber = childref->RecordNum();
			if(recordnumber != PS_Database::NullRecord)
			{
				JC_Object*	   childobject = GET_OBJ(&db, recordnumber);

				if(IsObjectAUnit(childobject))
				{
					JC_Vector pos = childobject->Anchor().AnchorPos() + childobject->Anchor().PinVector();

					RevoltTrackUnit* unit = new RevoltTrackUnit;	//create a new track unit
					RevoltMesh* mesh = new RevoltMesh((LPCSTR)childobject->Name()); //create a new mesh

					unit->MeshID = theme->InsertMesh(mesh); //add mesh to theme and store its index in the unit
					
					U32 unitID = theme->InsertUnit(unit);

					RevoltTrackUnitInstance* instance = new RevoltTrackUnitInstance;
					instance->UnitID = unitID;
					instance->Direction = NORTH;
					instance->XPos = (S16)( pos.X() / 4);
					instance->YPos = (S16)( pos.Z() / 4);
					instance->Elevation = 0;
					module->push_back(instance);
					
					const JC_RecordRef& templateref = childobject->Template();

					ExtractUnit(templateref.RecordNum(), mesh, unit);
				}

				childobject->Release();
			}
			childrefs.GetNext(childpos);
		}
		ExtractZoneInfo(module, model);
		if(module->Zones.size() == 0)
		{
			response.Format("********* No zones for this module\n");
			api.Log(response);
		}		
		api.Indent(-4);
		model->Release();
	}
	else
	{
		api.Log("Missing Model:- " + recordspec + "\n"); 
	}
}
//-------------------------------------------------------------------------------------------------
// HandleModuleScript(const CString& root)
//
// Looks in the specified location ('root') for a PropertyName called 'ModuleScript' and
// if found, uses the script as the source of the names of the track units to extract.
// The code uses the name ('UnitName') obtained from the script to build a path of the
// form ('root'/Units/'UnitName') and then passes this on to ExtractUnit() to process
// each individual unit
//-------------------------------------------------------------------------------------------------
void HandleModuleScript(const CString& root, RevoltTheme* theme)
{
	CString scriptname = root + "\\ModuleScript";
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	U32 recordnumber = db.FindRecord(scriptname, JC_Record::TypePropertyName);
	if(recordnumber != PS_Database::NullRecord)
	{
		CString response;
		response.Format("Found Script in record #%d\n", recordnumber);
		api.Log(response);

		JC_PropertyName* script = GET_PROP_NAME(&db, recordnumber);
		
		if(script->HasDefault())
		{
			api.Indent(4);
			JC_Data* data =	script->CopyDefault();
			CString text = data->String();
			delete data;
			ScriptObject scriptlines(text);

			Theme->InsertTPageRecnum(PS_Database::NullRecord);	//ensure that the TPage cache has a NULL texture in it
																//this makes the remaining TPageID values '1-based'
			CString modulepath;	
			U32 pos = 0;
			while( pos < scriptlines.size())
			{
				modulepath = root + "\\Modules\\" + scriptlines[pos] + "\\mod_" + scriptlines[pos];
				string modname = "mod_";
				modname += (LPCSTR)scriptlines[pos];

				ExtractModule(modulepath, theme);
				
				pos++;
				api.Update();
			}
			IMAGE image;
			CString filename;
			for(U32 b = 1; b < theme->TPageRecnumCount(); b++)
			{
				JC_Bitmap* bitmap = GET_BITMAP(&db, theme->TPageRecnum(b));
				if(bitmap)
				{
					BITMAPINFO* bmi = bitmap->CreateBitmapInfo();
					image.CopyFrom(bmi);
					delete bmi;
					
					filename.Format("tpage_%02d.bmp", b-1);
					response.Format("Saving %s\n", filename);
					api.Log(response);
					image.SaveBMP((char*)(LPCSTR)filename);
					bitmap->Release();
				}
			}
		}
		else
		{
			api.Log("Script has no default");
		}
		script->Release();
	}
	else
	{
		api.Log("Unable to locate Script record:- " + scriptname);
	}
}
//-------------------------------------------------------------------------------------------------
// HandleUnitScript(const CString& root)
//
// Looks in the specified location ('root') for a PropertyName called 'UnitScript' and
// if found, uses the script as the source of the names of the track units to extract.
// The code uses the name ('UnitName') obtained from the script to build a path of the
// form ('root'/Units/'UnitName') and then passes this on to ExtractUnit() to process
// each individual unit
//-------------------------------------------------------------------------------------------------
void HandleUnitScript(const CString& root, RevoltTheme* theme)
{
	CString scriptname = root + "\\UnitScript";
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	U32 recordnumber = db.FindRecord(scriptname, JC_Record::TypePropertyName);
	if(recordnumber != PS_Database::NullRecord)
	{
		CString response;
		response.Format("Found Script in record #%d\n", recordnumber);
		api.Log(response);

		JC_PropertyName* script = GET_PROP_NAME(&db, recordnumber);
		if(script->HasDefault())
		{
			api.Indent(4);
			JC_Data* data =	script->CopyDefault();
			CString text = data->String();
			ScriptObject scriptlines(text);

			CString unitpath;	
			U32 pos = 0;
			while( pos < scriptlines.size())
			{
				unitpath = root + "\\Units\\" + scriptlines[pos] + "\\" + scriptlines[pos];
				RevoltMesh* mesh = new RevoltMesh((LPCSTR)scriptlines[pos]);
				theme->InsertMesh(mesh);
				ExtractUnit(unitpath, mesh, NULL);
				
				pos++;
				api.Update();
			}
			delete data;
		}
		else
		{
			api.Log("Script has no default");
		}
		script->Release();
	}
	else
	{
		api.Log("Unable to locate Script record:- " + scriptname);
	}
}

//-------------------------------------------------------------------------------------------------
// HandleThemeScript(const CString& root)
//
// Looks in the specified location ('root') for a PropertyName called 'ThemeScript' and
// if found, uses the script as the source of the names of the track units to extract.
// The code uses the name ('ThemeName') obtained from the script to build a path of the
// form ('root'/'ThemeName') and then passes this on to HandleModuleScript() to process
// each individual module
//-------------------------------------------------------------------------------------------------
void HandleThemeScript(const CString& root)
{
	CString scriptname = root + "\\ThemeScript";
	
	PS_Database& db = ((CHabExtractApp*)AfxGetApp())->Database();

	U32 recordnumber = db.FindRecord(scriptname, JC_Record::TypePropertyName);
	if(recordnumber != PS_Database::NullRecord)
	{
		CString response;
		response.Format("Found Script in record #%d\n", recordnumber);
		api.Log(response);

		JC_PropertyName* script = GET_PROP_NAME(&db, recordnumber);
		
		if(script->HasDefault())
		{
			api.Indent(4);
			JC_Data* data =	script->CopyDefault();
			CString text = data->String();
			ScriptObject scriptlines(text);

			CString themepath;	
			U32 pos = 0;
			while( pos < scriptlines.size())
			{
				string themename = (LPCSTR)scriptlines[pos];
				themepath.Format("%s\\%s", root, (LPCSTR)scriptlines[pos]);
	
				string filename("trackunit.rtu");
				
				Theme = new RevoltTheme(themename);

				HandleModuleScript(themepath, Theme);

				CString message;

				message.Format("%d vertices were pooled from a total of %d\n", Theme->VertexCount(), VerticesScanned);
				api.Log(message);
				
				message.Format("%d polygons were pooled from a total of %d\n", Theme->PolygonCount(), PolygonsScanned);
				api.Log(message);
				
				message.Format("%d UV coords were pooled \n", Theme->UVCoordCount());
				api.Log(message);

				message.Format("%d UV polygons were pooled from a total of %d\n", Theme->UVPolyCount(), PolygonsScanned);
				api.Log(message);

				message.Format("%d TPage records were pooled \n", Theme->TPageRecnumCount());
				api.Log(message);

				UnitFileOutput converter;
				
				converter.OutputUnitFile(filename, Theme);

				delete Theme;

				pos++;
				api.Update();
			}
			delete data;
		}
		else
		{
			api.Log("Script has no default");
		}
		script->Release();
	}
	else
	{
		api.Log("Unable to locate Script record:- " + scriptname);
	}
}
//-------------------------------------------------------------------------------------------------
void theUserProgram(void)	// This is the part you are free to change as you see fit
{
	api.SetAutoRun(TRUE);

	CString filename;

	if (__argc == 1)
	{
		api.Error("No command line args");
		return;
	}

	for (int i = 1 ; i < __argc ; i++)
	{
		if (*__argv[i] != '-')
		{
			api.Error("Can't parse command line");
			return;
		}

		if (strncmp(__argv[i], "-f", 2) == 0)
		{
			filename = __argv[i];
			filename = filename.Mid(2);
		}
	}

	if (api.Begin("Track Unit extraction", filename))
	{

		VerticesScanned = 0;
		{
			HandleThemeScript("");

		}
		api.SetAutoRun(FALSE);
		api.End("End of extraction");						// Finished
	}
}
