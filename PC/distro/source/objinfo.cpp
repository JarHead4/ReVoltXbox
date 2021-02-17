
#include "revolt.h"
#include "editobj.h"

// file object model list

char *FileObjectModelList[] = {
	"models\\barrel.m",
	"models\\beachball.m",
	"models\\mercury.m",
	"models\\venus.m",
	"models\\earth.m",
	"models\\mars.m",
	"models\\jupiter.m",
	"models\\saturn.m",
	"models\\uranus.m",
	"models\\neptune.m",
	"models\\pluto.m",
	"models\\moon.m",
	"models\\rings.m",
	"models\\plane.m",
	"models\\copter.m",
	"models\\dragon1.m",
	"models\\water.m",
	"models\\trolley.m",
	"models\\boat1.m",
	"models\\speedup.m",
	"models\\radar.m",
	"models\\balloon.m",
	"models\\horse.m",
	"models\\train.m",
	"models\\light1.m",
	"models\\light2.m",
	"models\\football.m",	// Football
	"edit\\spot.m",			// Spark generator - no model
	"models\\spaceman.m",
	"models\\pickup.m",
	"models\\flap.m",
	"edit\\spot.m",

	NULL
};

// type lists

static char *TypeYesNo[] = {
	"No",
	"Yes",
};

static char *TypeOnOff[] = {
	"Off",
	"On",
};

static char *TypeAxis[] = {
	"X",
	"Y",
	"Z",
};

static char *TypePlanet[] = {
	"Mercury",
	"Venus",
	"Earth",
	"Mars",
	"Jupiter",
	"Saturn",
	"Uranus",
	"Neptune",
	"Pluto",
	"Moon",
	"Rings",
	"Sun",
};

static char *TypeBoat[] = {
	"Sail",
	"Tug",
};

static char *TypeStrobe[] = {
	"Muse Post",
	"Muse Wall",
};

static char *TypeSparkGen[] = {
	"Scratch",
	"Snow",
	"PopCorn",
	"Gravel",
	"Sand",
	"Mud",
	"Electric",
	"Water",
	"Dirt",
	"Smoke1",
	"Smoke2",
};

static char *TypePickup[] = {
	"Random",
	"Clone",
};

// file object info

FILE_OBJECT_INFO FileObjectInfo[] = {

// barrel

	{
		0,												// model ID
		"Spinning Barrel",								// name
		"Speed", NULL, -255, 255,						// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// Beachball

	{
		1,												// model ID
		"Beachball",									// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// planet

	{
		2,												// model ID
		"Planet",										// name
		"Name", TypePlanet, 0, 11,						// flag 1
		"Orbit", TypePlanet, 0, 11,						// flag 2
		"Orbit Speed", NULL, -255, 255,					// flag 3
		"Spin speed", NULL, -255, 255,					// flag 4
	},

// plane

	{
		13,												// model ID
		"Plane",										// name
		"Speed", NULL, -256, 256,						// flag 1
		"Radius", NULL, 0, 1024,						// flag 2
		"Bank", NULL, -256, 256,						// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// copter

	{
		14,												// model ID
		"Copter",										// name
		"X range", NULL, 0, 256,						// flag 1
		"Y range", NULL, 0, 256,						// flag 1
		"Z range", NULL, 0, 256,						// flag 1
		"Y offset", NULL, -256, 256,					// flag 4
	},

// dragon

	{
		15,												// model ID
		"Dragon",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// water

	{
		16,												// model ID
		"Water",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// Trolley

	{
		17,												// model ID
		"Trolley",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// boat

	{
		18,												// model ID
		"Boat",											// name
		"Type", TypeBoat, 0, 1,							// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// speedup

	{
		19,												// model ID
		"Speedup",										// name
		"Width        ", NULL, 10, 100,					// flag 2
		"LoSpeed (mph)", NULL, 0, 100,					// flag 1
		"HiSpeed (mph)", NULL, 0, 100,					// flag 1
		"Time (s)     ", NULL, 0, 50,					// flag 4
	},

// radar

	{
		20,												// model ID
		"Radar",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// balloon

	{
		21,												// model ID
		"Balloon",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// horse

	{
		22,												// model ID
		"Horse",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// train

	{
		23,												// model ID
		"Train",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// strobe

	{
		24,												// model ID
		"Strobe",										// name
		"Type", TypeStrobe, 0, 1,						// flag 1
		"Sequence Num", NULL, 0, 500,					// flag 2
		"Sequence Count", NULL, 0, 500,					// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// Football

	{
		26,												// model ID
		"Football",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// Spark Generator

	{
		27,												// model ID
		"Spark Generator",								// name
		"Type", TypeSparkGen, 0, 10,					// flag 1
		"Av. Speed", NULL, 0, 200,						// flag 2
		"Var. Speed", NULL, 0, 200,						// flag 3
		"Frequency", NULL, 1, 200,						// flag 4
	},

// space man

	{
		28,												// model ID
		"Space Man",									// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// shockwave

	{
		-2,												// model ID
	},

// firework

	{
		-2,												// model ID
	},

// putty bomb

	{
		-2,												// model ID
	},

// water bomb

	{
		-2,												// model ID
	},

// electro pulse

	{
		-2,												// model ID
	},

// oil slick

	{
		-2,												// model ID
	},

// oil slick dropper

	{
		-2,												// model ID
	},

// chrome ball

	{
		-2,												// model ID
	},

// clone

	{
		-2,												// model ID
	},

// turbo

	{
		-2,												// model ID
	},

// electro zapped

	{
		-2,												// model ID
	},

// spring

	{
		-2,												// model ID
	},

// pickup generator

	{
		29,												// model ID
		"Pickup Generator",								// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// dissolve model

	{
		-2,												// model ID
	},

// flap

	{
		30,												// model ID
		"Flappage",										// name
		NULL, NULL, 0, 0,								// flag 1
		NULL, NULL, 0, 0,								// flag 2
		NULL, NULL, 0, 0,								// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// laser

	{
		31,												// model ID
		"Laser",										// name
		"Width", NULL, 1, 10,							// flag 1
		"Rand", NULL, 1, 10,							// flag 2
		"Object", TypeYesNo, 0, 1,						// flag 3
		NULL, NULL, 0, 0,								// flag 4
	},

// splash

	{
		-2,												// model ID
	},

// bomb glow

	{
		-2,												// model ID
	},

// end of list

	{
		-1
	}
};
