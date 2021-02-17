#ifndef _OUTPUTWAVEFRONT_H
#define _OUTPUTWAVEFRONT_H

#include "MappedVector.h"
#include "Primitives.h"
#include <fstream>

class WaveFrontOutput
{
	protected:
		RevoltTheme* Theme;
	public:
		void OutputPolySet(ofstream& os, RevoltPolySet* polyset);
		void OutputMesh(ofstream& os, const RevoltMesh* mesh);
		void OutputTheme(ofstream& os);
		void OutputWaveFrontFile(const string& filename, RevoltTheme* theme);
};

#endif
