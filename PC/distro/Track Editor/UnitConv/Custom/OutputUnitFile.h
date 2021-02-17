#ifndef _OUTPUTUNITFILE_H
#define _OUTPUTUNITFILE_H

#include "MappedVector.h"
#include "Primitives.h"
#include <Fname.h>
#include <endstrm.h>

//#define RTU_WRITE_VERSION 1		//version 1 - now includes UV information
//#define RTU_WRITE_VERSION 2		//version 2 - now includes surface material information
//#define RTU_WRITE_VERSION 3		//version 3 - now includes track zone information
#define RTU_WRITE_VERSION 4		//version 4 - no longer contains VALIDEXIT fields

class UnitFileOutput
{
	protected:
		RevoltTheme* Theme;
		EndianOutputStream* Output;
	public:
		UnitFileOutput();
		void OutputVertices(void);
		void OutputPolygons(void);
		void OutputMeshes(void);
		void OutputUVCoords(void);
		void OutputUVPolys(void);
		void OutputUnits(void);
		void OutputModules(void);
		void OutputTPages(void);
		void OutputPolySet(const RevoltPolySet* polyset);
		void OutputMesh(const RevoltMesh* mesh);
		void OutputModule(const RevoltModule* module);
		void OutputUnitFile(const string& filename, RevoltTheme* theme);
};

#endif
