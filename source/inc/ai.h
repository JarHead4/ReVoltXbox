/*********************************************************************************************
 *
 * ai.h
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 * 			Utility AI functions, used by car and general AIs
 *
 *********************************************************************************************
 *
 * 01/07/98 Matt Taylor
 *	File inception.
 *
 *********************************************************************************************/

#ifndef _AI_H_
#define _AI_H_

//
// External global variables
//

extern long	AI_Testing;

//
// External function prototypes
//

extern void AI_ProcessAllAIs(void);
extern void AI_CarAiHandler(OBJECT *obj);
extern void AI_RemoteAiHandler(OBJECT *obj);
extern void AI_GhostCarAiHandler(OBJECT *obj);
extern void AI_BarrelHandler(OBJECT *obj);
extern void AI_PlanetHandler(OBJECT *obj);
extern void AI_PlaneHandler(OBJECT *obj);
extern void AI_CopterHandler(OBJECT *obj);
extern void AI_DragonHandler(OBJECT *obj);
extern void AI_WaterHandler(OBJECT *obj);
extern void AI_BoatHandler(OBJECT *obj);
extern void AI_RadarHandler(OBJECT *obj);
extern void AI_BalloonHandler(OBJECT *obj);
extern void AI_HorseRipper(OBJECT *obj);
extern void NewCopterDest(OBJECT *obj);
extern void AI_TrainHandler(OBJECT *obj);
extern void AI_StrobeHandler(OBJECT *obj);
extern void SparkGenHandler(OBJECT *obj);
extern void AI_SpacemanHandler(OBJECT *obj);
extern void AI_PickupHandler(OBJECT *obj);
extern void AI_DissolveModelHandler(OBJECT *obj);
extern void AI_LaserHandler(OBJECT *obj);
extern void AI_SplashHandler(OBJECT *obj);
extern void SpeedupImpulse(CAR *car);
extern void AI_SpeedupAIHandler(OBJECT *obj);

#endif