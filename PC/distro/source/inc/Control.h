/*********************************************************************************************
 *
 * control.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Control input processing code
 *
 *********************************************************************************************
 *
 * 03/03/98 Matt Taylor
 *	file inception.
 *
 *********************************************************************************************/


#ifndef _CONTROL_H_
#define _CONTROL_H_

//
// Defines and macros
//

#define CTRL_RANGE_MAX		127

#define CTRL_LEFT		1
#define CTRL_RIGHT		(1 << 1)
#define CTRL_FWD 		(1 << 2)
#define CTRL_BACK		(1 << 3)
#define CTRL_FIRE		(1 << 4)
#define CTRL_RESET		(1 << 5)
#define CTRL_SELWEAPON	(1 << 14)
#define CTRL_RESTART	(1 << 15)

#define CTRL_LR			(CTRL_LEFT | CTRL_RIGHT)
#define CTRL_FB			(CTRL_FWD  | CTRL_BACK)

//
// Typedefs and structures
//

typedef struct CtrlStruct {
	signed char	dx;
	signed char	dy;
	unsigned short	digital, idigital;
} CTRL;


//
// External function prototypes
//
struct object_def;

extern void CON_DoPlayerControl(void);
extern void CON_LocalCarControl(CTRL *Control, struct object_def *CarObj);

#endif
