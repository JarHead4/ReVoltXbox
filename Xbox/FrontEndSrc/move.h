//-----------------------------------------------------------------------------
// File: move.h
//
// Desc: Object movement code
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef MOVE_H
#define MOVE_H


#define MOVE_MIN_VEL            TO_LENGTH(Real(1))
#define MOVE_MAX_NOMOVETIME     TO_TIME(Real(0.2))
#define MOVE_MAX_NOCONTACTTIME  TO_TIME(Real(5.0))

//
// Typedefs and structures
//


//
// External function prototypes
//

extern void MOV_MoveObjects(void);
extern void MOV_MoveCarNew(OBJECT *CarObj);
extern void MOV_DropCar(OBJECT *carObj);
extern void MOV_MoveGhost(OBJECT *CarObj);
extern void MOV_MoveBody(OBJECT *bodyObj);
extern void MOV_MoveBodyClever(OBJECT *bodyObj);
extern void MOV_DropBody(OBJECT *obj);
extern void MOV_RightCar(OBJECT *obj);
extern void MOV_RepositionCar(OBJECT *obj);
extern void MOV_MoveTrain(OBJECT *obj);
extern void MOV_MoveReplayCar(OBJECT *obj);
extern void MOV_MoveUFO(OBJECT *obj);

#endif // MOVE_H

