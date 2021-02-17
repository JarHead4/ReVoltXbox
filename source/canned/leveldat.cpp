/*********************************************************************************************
 *
 * leveldat.cpp
 *
 * Re-Volt (PC) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Level specific data
 *
 *********************************************************************************************
 *
 * 28/02/98 Matt Taylor
 *  File inception.
 *
 *********************************************************************************************/

#include "revolt.h"
#include "level.h"

// Track info

/*LEVEL_INFO LevelInfo[MAX_LEVEL] =
{
	// Toy World
	{
		"Toy World",
		"levels\\toy\\toy.w",
		"levels\\toy\\toy.col",
		"",							// No convex hulls yet
		"levels\\toy\\toy.lit",
		"levels\\toy\\toy.vis",
		"levels\\toy\\toy.fob",
		"levels\\toy\\toy.fin",
		2800, -448, 8500, 0.25f,
		6000, 5500, 0x000000, 0, 0,
		MAX_TP_TOYWORLD, ToyPages,
		1, ToyInstanceModels,
		50
	},

	// Rooftop
	{
		"Rooftops",
		"levels\\roof\\roof.w",
		"levels\\roof\\roof.col",
		"",							// No convex hulls yet
		"levels\\roof\\roof.lit",
		"levels\\roof\\roof.vis",
		"levels\\roof\\roof.fob",
		"levels\\roof\\roof.fin",
		-128, 512, 0, 0.25f,
		7000, 5000, 0xa09090, 768, 1024,
		MAX_TP_ROOFTOP, RoofPages,
		1, RoofInstanceModels,
		50
	},

	// Cave
	{
		"Spirit Cave",
		"levels\\cave\\cave.w",
		"levels\\cave\\cave.col",
		"",							// No convex hulls yet
		"levels\\cave\\cave.lit",
		"levels\\cave\\cave.vis",
		"levels\\cave\\cave.fob",
		"levels\\cave\\cave.fin",
		0, -100, 0, -0.25f,
		7000, 6000, 0x200000, 0, 0,
		MAX_TP_CAVE, CavePages,
		1, CaveInstanceModels,
		100
	},

	// Museum
	{
		"Museum",
		"levels\\muse\\muse.w",
		"levels\\muse\\muse.col",
		"",							// No convex hulls yet
		"levels\\muse\\muse.lit",
		"levels\\muse\\muse.vis",
		"levels\\muse\\muse.fob",
		"levels\\muse\\muse.fin",
		0, -128, 0, 0.5f,
		7000, 6000, 0x000000, 0, 0,
		MAX_TP_MUSEUM, MusePages,
		1, MuseInstanceModels,
		100
	},

	// Dirt Track
	{
		"Dirt 1",
		"levels\\dirt1\\dirt1.w",
		"levels\\dirt1\\dirt1.col",
		"",							// No convex hulls yet
		"levels\\dirt1\\dirt1.lit",
		"levels\\dirt1\\dirt1.vis",
		"levels\\dirt1\\dirt1.fob",
		"levels\\dirt1\\dirt1.fin",
		0, -256, 0, 0,
		8000, 7000, 0x000000, 0, 0,
		MAX_TP_DIRT, Dirt1Pages,
		1, Dirt1InstanceModels,
		100
	},

	// bloc 1
	{
		"Council Estate 1",
		"levels\\bloc1\\bloc1.w",
		"levels\\bloc1\\bloc1.col",
		"",							// No convex hulls yet
		"levels\\bloc1\\bloc1.lit",
		"levels\\bloc1\\bloc1.vis",
		"levels\\bloc1\\bloc1.fob",
		"levels\\bloc1\\bloc1.fin",
		0, -128, 2700, 0.75f,
		16384, 16384, 0x4080c0, 0, 0,
		MAX_TP_BLOC, BlocPages,
		1, BlocInstanceModels,
		100
	},

	// bloc 2
	{
		"Council Estate 2",
		"levels\\bloc2\\bloc2.w",
		"levels\\bloc2\\bloc2.col",
		"levels\\bloc2\\bloc2.hul",
		"levels\\bloc2\\bloc2.lit",
		"levels\\bloc2\\bloc2.vis",
		"levels\\bloc2\\bloc2.fob",
		"levels\\bloc2\\bloc2.fin",
		0, -128, 2700, 0.75f,
		16384, 16384, 0x4080c0, 0, 0,
		MAX_TP_BLOC, BlocPages,
		1, BlocInstanceModels,
		100
	},

	// bloc 3
	{
		"Council Estate 3",
		"levels\\bloc3\\bloc3.w",
		"levels\\bloc3\\bloc3.col",
		"",							// No convex hulls yet
		"levels\\bloc3\\bloc3.lit",
		"levels\\bloc3\\bloc3.vis",
		"levels\\bloc3\\bloc3.fob",
		"levels\\bloc3\\bloc3.fin",
		-4000, -128, 2700, 0.75f,
		16384, 16384, 0x4080c0, 0, 0,
		MAX_TP_BLOC, BlocPages,
		1, BlocInstanceModels,
		100
	},

	// Grand Prix (urrrgh.. you said.. 'Grand')
	{
		"Prix 01",
		"levels\\prix01\\prix01.w",
		"levels\\prix01\\prix01.col",
		"",							// No convex hulls yet
		"levels\\prix01\\prix01.lit",
		"levels\\prix01\\prix01.vis",
		"levels\\prix01\\prix01.fob",
		"levels\\prix01\\prix01.fin",
		0, -256, 0, 0,
		8000, 7000, 0x000000, 0, 0,
		MAX_TP_PRIX, Prix01Pages,
		1, Prix01InstanceModels,
		100
	}
};
*/