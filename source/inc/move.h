/*********************************************************************************************
 *
 * move.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Object movement code
 *
 *********************************************************************************************
 *
 * 04/03/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _MOVE_H_
#define _MOVE_H_

//
// Typedefs and structures
//


//
// External function prototypes
//

extern void MOV_MoveObjects(void);
extern void MOV_MoveCarNew(OBJECT *CarObj);
extern void MOV_MoveGhost(OBJECT *CarObj);
extern void MOV_MoveBody(OBJECT *bodyObj);
extern void MOV_RightCar(OBJECT *obj);
extern void MOV_MoveTrain(OBJECT *obj);

#endif /* _MOVE_H_ */