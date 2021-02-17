//-------------------------------------------------------------------------------------------------
// Re-Volt Track Unit Extraction Program.
//
//-------------------------------------------------------------------------------------------------
#include "stdafx.h"

#include "OutputWaveFront.h"

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void WaveFrontOutput::OutputPolySet(ofstream& os, RevoltPolySet* polyset)
{
	os << "[" << polyset->Name().c_str() << "]" << endl;
	for(U32 i = 0; i < polyset->size(); i++)
	{
		U32 polyindex = (*polyset)[i];
		const RevoltPolygon* poly = Theme->Polygon(polyindex);
		os << "f";
		for(U32 v = 0; v < poly->size(); v++)
		{
			os << " " << ((*poly)[v]) + 1;
		}
		os << endl;
	}
}//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void WaveFrontOutput::OutputMesh(ofstream& os, const RevoltMesh* mesh)
{
	for(U32 i = 0; i < mesh->size(); i++)
	{
		RevoltPolySet* polyset = (*mesh)[i];
		os << "g " << mesh->Name().c_str();
		OutputPolySet(os, polyset);
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void WaveFrontOutput::OutputTheme(ofstream& os)
{
	for(U32 i = 0; i < Theme->MeshCount(); i++)
	{
		const RevoltMesh* mesh = Theme->Mesh(i);
		OutputMesh(os, mesh);
	}
}

//-------------------------------------------------------------------------------------------------
//
//
//-------------------------------------------------------------------------------------------------
void WaveFrontOutput::OutputWaveFrontFile(const string& filename, RevoltTheme* theme)
{
	Theme = theme;
	ofstream os(filename.c_str(), ios_base::out);

	if(os.is_open())
	{
		os << "# Track Unit Extraction Test Output File" << endl;
		
		for(U32 i = 0; i < Theme->VertexCount(); i++)
		{
			const RevoltVertex* vert = Theme->Vertex(i);
			os << "v " << vert->X << " " << vert->Y << " " << vert->Z << endl;
		}
		os << endl;
		OutputTheme(os);
	}
	Theme = NULL;
}

