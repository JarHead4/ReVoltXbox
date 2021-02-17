//-------------------------------------------------------------------------------------------------
// Re-Volt Track Unit Extraction Program.
//
//-------------------------------------------------------------------------------------------------
#include "stdafx.h"

#include "OutputUnitFile.h"

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputPolySet(const RevoltPolySet* polyset)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);
	ASSERT(polyset != NULL);

	U32 polycount = polyset->size();
	Output->PutU32(polycount);

	for(U32 i = 0; i < polycount; i++)
	{
		Output->PutU32(polyset->at(i));
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputMesh(const RevoltMesh* mesh)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);
	ASSERT(mesh != NULL);

	U16 polysetcount = mesh->size();
	Output->PutU16(polysetcount);

	for(U16 i = 0; i < polysetcount; i++)
	{
		const RevoltPolySet* polyset = mesh->at(i);
		OutputPolySet(polyset);
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputModule(const RevoltModule* module)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);
	ASSERT(module != NULL);

	Output->PutPaddedText(module->Name().c_str());
	U16 instancecount = module->size();
	Output->PutU16(instancecount);

	for(U16 i = 0; i < instancecount; i++)
	{
		const RevoltTrackUnitInstance* instance = module->at(i);

		Output->PutU32(instance->UnitID);
		Output->PutU16(instance->Direction);
		Output->PutS16(instance->XPos);
		Output->PutS16(instance->YPos);
		Output->PutS16(instance->Elevation);
	}

	U16 zonecount = module->Zones.size();
	Output->PutU16(zonecount);
	for(U16 n = 0; n < zonecount; n++)
	{
		const TRACKZONE* zone = module->Zones.at(n);

		Output->PutFloat(zone->Centre.X);
		Output->PutFloat(zone->Centre.Y);
		Output->PutFloat(zone->Centre.Z);
		Output->PutFloat(zone->XSize);
		Output->PutFloat(zone->YSize);
		Output->PutFloat(zone->ZSize);
		Output->PutFloat(zone->Links[0].Position.X);
		Output->PutFloat(zone->Links[0].Position.Y);
		Output->PutFloat(zone->Links[0].Position.Z);
		Output->PutFloat(zone->Links[1].Position.X);
		Output->PutFloat(zone->Links[1].Position.Y);
		Output->PutFloat(zone->Links[1].Position.Z);
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputMeshes(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);
	
	U32 meshcount = Theme->MeshCount();
	Output->PutU32(meshcount);

	for(U32 i = 0; i < meshcount; i++)
	{
		const RevoltMesh* mesh = Theme->Mesh(i);
		OutputMesh(mesh);
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputVertices(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);

	U32 vertcount = Theme->VertexCount();
	Output->PutU32(vertcount);
	for(U32 i = 0; i < vertcount; i++)
	{
		const RevoltVertex* vert = Theme->Vertex(i);
		Output->PutFloat(vert->X);
		Output->PutFloat(vert->Y);
		Output->PutFloat(vert->Z);
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputPolygons(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);

	U32 polycount = Theme->PolygonCount();
	Output->PutU32(polycount);
	for(U32 i = 0; i < polycount; i++)
	{
		const RevoltPolygon* poly = Theme->Polygon(i);
		U16 vertcount = poly->size();
		Output->PutU16(max(vertcount, 3));
		for(U16 v = 0; v < vertcount; v++)
		{
			Output->PutU32(poly->at(v));
		}
		while(vertcount < 3)
		{
			Output->PutU32(0);
			vertcount++;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputUVCoords(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);

	U32 uvcount = Theme->UVCoordCount();
	Output->PutU32(uvcount);
	for(U32 i = 0; i < uvcount; i++)
	{
		const RevoltUVCoord* uv = Theme->UVCoord(i);
		Output->PutFloat(uv->U);
		Output->PutFloat(uv->V);
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputUVPolys(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);

	U32 polycount = Theme->UVPolyCount();
	Output->PutU32(polycount);
	for(U32 i = 0; i < polycount; i++)
	{
		const RevoltUVPolygon* poly = Theme->UVPolygon(i);
	
		U16 vertcount = poly->size();
		Output->PutU16(max(vertcount, 3));
		for(U16 v = 0; v < vertcount; v++)
		{
			Output->PutU32(poly->at(v));
		}
		while(vertcount < 3)
		{
			Output->PutU32(0);
			vertcount++;
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputUnits(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);

	U32 unitcount = Theme->UnitCount();
	Output->PutU32(unitcount);
	for(U32 i = 0; i < unitcount; i++)
	{
		const RevoltTrackUnit* unit = Theme->Unit(i);
		Output->PutU32(unit->MeshID);
		
		U32 uvpolycount = unit->UVPolySet.size();

		Output->PutU32(uvpolycount);
		for(U32 n = 0; n < uvpolycount; n++)
		{
			const UVPolyInstance* poly = unit->UVPolySet.at(n);
			Output->PutU32(poly->TPageID);
			Output->PutU32(poly->PolyID);
			Output->PutU8(poly->Rotation);
			Output->PutU8(poly->Reversed);
		}

		U32 surfacecount = unit->SurfaceSet.size();
		Output->PutU32(surfacecount);
		for(n = 0; n < surfacecount; n++)
		{
			Output->PutU32(unit->SurfaceSet.at(n));
		}
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputModules(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);

	U32 modulecount = Theme->ModuleCount();
	Output->PutU32(modulecount);

	for(U32 i = 0; i < modulecount; i++)
	{
		const RevoltModule* module = Theme->Module(i);

		OutputModule(module);		
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputTPages(void)
{
	ASSERT(Output != NULL);
	ASSERT(Theme != NULL);

	U32 tpagecount = Theme->TPageRecnumCount();
	Output->PutU32(tpagecount);
	for(U32 t = 0; t < tpagecount; t++)
	{
		Output->PutU32(Theme->TPageRecnum(t));
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void UnitFileOutput::OutputUnitFile(const string& filename, RevoltTheme* theme)
{
	ASSERT(theme != NULL);
	
	Theme = theme;
	
	Output = new EndianOutputStream(filename.c_str());

	if(Output != NULL)
	{
		if(Output->is_open())
		{
			Output->PutU32(MAKE_ID('R','T','U',' '));
			Output->PutU16(RTU_WRITE_VERSION);
			Output->PutU16(FOR_PC);

			OutputVertices();
			OutputPolygons();
			OutputMeshes();
			OutputUVCoords();
			OutputUVPolys();
			OutputUnits();
			OutputModules();
			OutputTPages();			
	
		}
		delete Output;
		Output = NULL;
	}
	Theme = NULL;
}


UnitFileOutput::UnitFileOutput()
{
	Theme = NULL;
	Output = NULL;
}
