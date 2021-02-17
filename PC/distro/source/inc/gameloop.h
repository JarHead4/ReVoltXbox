/*********************************************************************************************
 *
 * gameloop.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Main game loop code
 *
 *********************************************************************************************
 *
 * 05/04/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _GAMELOOP_H_
#define _GAMELOOP_H_

#define MAX_TIMESTEP	(150)			// 1 / max time step allowed for physics

#define GHOST_TAKEOVER_TIME 10000000

extern int NPhysicsLoops;
extern bool DrawGridCollSkin;

// defines + macros

enum {
	PAWS_MENU_RESUME,
	PAWS_MENU_QUIT,

	PAWS_MENU_NUM
};

// External function prototpyes

extern void GLP_GameLoop(void);
extern long PawsMenu(void);

#endif /* _GAMELOOP_H_ */