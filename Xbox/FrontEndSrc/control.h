//-----------------------------------------------------------------------------
// File: control.h
//
// Desc: Control input processing code
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef CONTROL_H
#define CONTROL_H

//
// Defines and macros
//

typedef enum {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_FWD,
    KEY_BACK,
    KEY_FIRE,
    KEY_RESET,
    KEY_REPOSITION,
    KEY_HONKA,
//  KEY_HANDBRAKE,
    KEY_REARVIEW,
    KEY_SELWEAPON,
    KEY_SELCAMERAMODE,
    KEY_FULLBRAKE,
    KEY_PAUSE,

    KEY_CATCHUP_1,
    KEY_CATCHUP_2,
    KEY_CATCHUP_DIR,

    KEY_NUM
} DIGITAL_KEYS;

#define CTRL_RANGE_MAX      127

#define CTRL_LEFT       (1 << KEY_LEFT)
#define CTRL_RIGHT      (1 << KEY_RIGHT)
#define CTRL_FWD        (1 << KEY_FWD)
#define CTRL_BACK       (1 << KEY_BACK)
#define CTRL_FIRE       (1 << KEY_FIRE)
#define CTRL_RESET      (1 << KEY_RESET)
#define CTRL_REPOSITION (1 << KEY_REPOSITION)
#define CTRL_HONKA      (1 << KEY_HONKA)
//#define CTRL_HANDBRAKE    (1 << KEY_HANDBRAKE)
#define CTRL_REARVIEW   (1 << KEY_REARVIEW)
#define CTRL_SELWEAPON  (1 << KEY_SELWEAPON)
#define CTRL_CHANGECAMERA (1 << KEY_SELCAMERAMODE)
#define CTRL_FULLBRAKE  (1 << KEY_FULLBRAKE)
#define CTRL_PAUSE  (1 << KEY_PAUSE)

#define CTRL_LR         (CTRL_LEFT | CTRL_RIGHT)
#define CTRL_FB         (CTRL_FWD  | CTRL_BACK)

#define CTRL_CATCHUP_MASK   (7 << KEY_CATCHUP_1)
#define CTRL_CATCHUP_DIR    (1 << KEY_CATCHUP_DIR)
#define CTRL_CATCHUP_SHIFT  KEY_CATCHUP_1
#define CTRL_CATCHUP_DEC    3
#define CTRL_CATCHUP_INC    4


#define CTRL_CHANGE_NONE    0x0
#define CTRL_CHANGE_DX      0x1
#define CTRL_CHANGE_DY      0x2
#define CTRL_CHANGE_DIGITAL 0x4

//
// Typedefs and structures
//

typedef struct CtrlStruct {
    unsigned long changes;
    signed char dx, lastdx;
    signed char dy, lastdy;
    unsigned short digital, idigital, adigital, lastdigital;
} CTRL;


//
// External function prototypes
//
struct object_def;

extern void CON_DoPlayerControl(void);
extern void CON_RationaliseControl(unsigned short *digital);
extern void CON_LocalCarControl(CTRL *Control, struct object_def *CarObj);
extern long CON_ControlCompare(CTRL *ctrl1, CTRL *ctrl2);
extern void CON_CopyControls(CTRL *src, CTRL *dest);
extern void CON_InitControls(CTRL *ctrl);
extern void FirePlayerWeapon(struct object_def *obj);


extern bool NonLinearSteering;

#endif // CONTROL_H
