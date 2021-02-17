/*********************************************************************************************
 *
 * ctrlread.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Controller reading code (for mouse, keyboard, etc)
 *
 *********************************************************************************************
 *
 * 03/03/98 Matt Taylor
 *	file inception.
 *
 *********************************************************************************************/


#ifndef _CTRLREAD_H_
#define _CTRLREAD_H_

#ifndef _PSX

#include "Control.h"

#endif
//
// Defines and macros
//

#define PHYSICS_DEBUG_KEYS TRUE

//
// Typedefs and structures
//

// CTRL_TYPE - platform specific - each platform should have it's own ctrlread functions

typedef enum
{
	CTRL_TYPE_NONE = 0,
	CTRL_TYPE_KBD,
	CTRL_TYPE_JOY,
	CTRL_TYPE_CPU,
	CTRL_TYPE_MOUSE
} CTRL_TYPE;

//
// External function prototypes
//

struct PlayerStruct;

extern void CRD_CheckLocalKeys(void);
extern void CRD_InitPlayerControl(struct PlayerStruct *player, CTRL_TYPE CtrlType);
extern void CRD_KeyboardInput(CTRL *Control);
extern void CRD_JoystickInput(CTRL *Control);

#endif /* _CTRLREAD_H_ */
