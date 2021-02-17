/////////////////////////////////////////////////////////////////////
// Aerial - describe the motion and state of a wobbly aerial
//
// An aerial is divided into up to ARIAL_MAX_SECTIONS stright sections
// which are attached to the previous section. 
//
//

#ifndef __AERIAL_H__
#define __AERIAL_H__

#include "particle.h"

#ifndef _PSX
#define AERIAL_NSECTIONS	3			// Odd number > 2
#define AERIAL_LASTSECTION	2			// AERIAL_NSECTIONS - 1
#define AERIAL_SKIP			1			// (AERIAL_NSECTIONS - 1) / 2
#define AERIAL_START		1			// always = AERIAL_SKIP

#define AERIAL_NTOTSECTIONS	5			// Total number of sections to draw
#define AERIAL_UNITLEN		(1.0f / 4.0f)	// 1.0 / (AERIAL_NTOTSECTIONS - 1)
#else
#define AERIAL_NSECTIONS	3			// Odd number > 2
#define AERIAL_LASTSECTION	2			// AERIAL_NSECTIONS - 1
#define AERIAL_SKIP			1			// (AERIAL_NSECTIONS - 1) / 2
#define AERIAL_START		1			// always = AERIAL_SKIP

#define AERIAL_NTOTSECTIONS	5			// Total number of sections to draw
#define AERIAL_UNITLEN		(1.0f / 4.0f)	// 1.0 / (AERIAL_NTOTSECTIONS - 1)
#endif


typedef PARTICLE AERIALSECTION;

typedef struct {
	//VEC BasePos;				// Position of aerial base relative to car CoM
	VEC Direction;
	REAL Length;				// Length of each aerial section in world coords
	REAL Stiffness;				// Stiffness of the linear spring connecting nodes
	REAL Damping;				// Damping of the linear spring
	REAL AntiGrav;				// "gravity" which keeps aerial straight
	AERIALSECTION Section[AERIAL_NSECTIONS];		// array of aerial sections
} AERIAL;

// prototypes

#ifdef _PC
extern AERIAL *CreateAerial();
extern void DestroyAerial(AERIAL *aerial);
#endif
extern void SetAerialSprings(AERIAL *aerial, REAL stiffness, REAL damping, REAL antiGrav);
extern void InitAerial(AERIAL *aerial, VEC *direction, REAL secLength, REAL mass, REAL hardness, REAL resistance, REAL gravity);

// globals


#endif
