/*********************************************************************************************
 *
 * object.cpp
 *
 * Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Object processing code
 *
 *********************************************************************************************
 *
 * 03/03/98 Matt Taylor
 *  File inception.
 *
 *********************************************************************************************/

#include "revolt.h"
#ifndef _PSX
#include "main.h"
#endif
#include "geom.h"
#include "particle.h"
#include "model.h"
#include "aerial.h"
#include "newcoll.h"
#include "body.h"
#include "car.h"
#ifndef _PSX
#include "ctrlread.h"
#endif
#include "object.h"
#include "field.h"
#ifdef _N64
#include "spark.h"
#endif

//
// Static variables
//

static OBJECT *s_NextFreeObj;

//
// Global variables
//

OBJECT *OBJ_ObjectList;
OBJECT *OBJ_ObjectHead = NULL;
OBJECT *OBJ_ObjectTail = NULL;

long	OBJ_NumObjects;

// Array giving contact info about object pairs
PAIRCOLLINFO	OBJ_PairCollInfo[MAX_OBJECTS][MAX_OBJECTS];

//
// Global function prototypes
//

long	OBJ_InitObjSys(void);
void	OBJ_KillObjSys(void);
OBJECT *OBJ_AllocObject(void);
OBJECT *OBJ_ReplaceObject(void);
long	OBJ_FreeObject(OBJECT *Obj);

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_InitObjsys
//
// Initialises the object processing system
//

long OBJ_InitObjSys(void)
{
	long	ii;

	OBJ_ObjectList = (OBJECT *)malloc(sizeof(OBJECT) * MAX_OBJECTS);
	s_NextFreeObj = OBJ_ObjectList;

	OBJ_ObjectList[0].prev = NULL;							// Setup first object in linked list
	OBJ_ObjectList[0].next = &(OBJ_ObjectList[1]);

	for (ii = 1; ii < (MAX_OBJECTS - 1); ii++)				// Initialise bulk of object list links
	{
		OBJ_ObjectList[ii].prev = &(OBJ_ObjectList[ii - 1]);
		OBJ_ObjectList[ii].next = &(OBJ_ObjectList[ii + 1]);
	}
															// Initialise last object
	OBJ_ObjectList[MAX_OBJECTS - 1].prev = &(OBJ_ObjectList[MAX_OBJECTS - 2]);
	OBJ_ObjectList[MAX_OBJECTS - 1].next = NULL;

	// initialise object IDs
	for (ii = 0; ii < MAX_OBJECTS; ii++) {
		OBJ_ObjectList[ii].ObjID = ii;
	}

	// Initialise pair contact/ tested list
	ClearAllPairInfo();


	OBJ_NumObjects = 0;
	OBJ_ObjectHead = NULL;
	OBJ_ObjectTail = NULL;

	return(1);			// Success
}

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_KillObjSys(void)
//
// Removes resources used by the object processing system
//

void OBJ_KillObjSys(void)
{
    if (OBJ_ObjectList != NULL)
	{
		while (OBJ_ObjectHead)	// free all alive objects
		{
			OBJ_FreeObject(OBJ_ObjectHead);
		}

        free(OBJ_ObjectList);
	}

    OBJ_ObjectList = NULL;
    OBJ_ObjectHead = NULL;
    OBJ_ObjectTail = NULL;
    s_NextFreeObj = NULL;

	OBJ_NumObjects = 0;
}

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_AllocObject
//
// Alocates an object from the object buffer
//

OBJECT *OBJ_AllocObject(void)
{
	OBJECT *newobj;

	newobj = s_NextFreeObj;
	if (newobj == NULL)
	{
		return(NULL);										// Could not allocate object (buffer full)
	}

	s_NextFreeObj = s_NextFreeObj->next;					// Update free object list
	if (s_NextFreeObj != NULL)
	{
		s_NextFreeObj->prev = NULL;
	}

	newobj->prev = OBJ_ObjectTail;

	if (OBJ_ObjectHead == NULL)
	{
		OBJ_ObjectHead = newobj;							// newobj is the first to be allocated
	}
	else
	{
        OBJ_ObjectTail->next = newobj;
	}
	OBJ_ObjectTail = newobj;
	
	newobj->next = NULL;

	// Set some defaults
	newobj->player = NULL;
	newobj->objref = NULL;
	newobj->creator = NULL;
	newobj->flag.IsInGrid = 0;								// Mark object as not in grid yet
	newobj->Data = NULL;
	newobj->Field = NULL;
	newobj->FieldPriority = FIELD_PRIORITY_MIN;				// Default - affected by all fields
#ifndef _PSX
	newobj->SparkGen = NULL;
	newobj->Light = NULL;
#endif
#ifdef _PC
	newobj->Sfx3D = NULL;
#endif
	newobj->movehandler = NULL;
	newobj->collhandler = NULL;
	newobj->aihandler = NULL;
	newobj->renderhandler = NULL;
	newobj->freehandler = NULL;

	// Clear contact testing flags
	ClearThisObjPairInfo(newobj);

	OBJ_NumObjects++;
	return(newobj);
}

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_ReplaceObject(void)
//
// Searches the list of allocated objects for low priority objects
// and returns a pointer to the oldest one it can find.
//
// !MT! Not used - yet. Only required if number of objects required exceeds
//      availble free

OBJECT *OBJ_ReplaceObject(void)
{
	OBJECT *newobj = NULL;
	long	found = 0;

	newobj = OBJ_ObjectTail;								// Start at the end of the list and work towards "newer" objects

#if 0	/* Examples: */
	while ((!found) && (newobj != NULL))
	{
		switch(newobj->type)
		{
			case TYPE_EYECANDY:
			case TYPE_PICKUP:
			found = 1;
			break;

			default:
			newobj = newobj->prev;
			break
		}
	}

	#ifndef _PSX
	GRD_RemoveObject(newobj);
	#endif

	return(newobj);
#else
	return(NULL);
#endif
}

//--------------------------------------------------------------------------------------------------------------------------

//
// OBJ_FreeObject
//
// Frees an allocated object from the object buffer
//

long OBJ_FreeObject(OBJECT *Obj)
{

	// Free ram allocated in object
	FreeCollSkin(&Obj->body.CollSkin);
	if (Obj->freehandler)
	{
		Obj->freehandler(Obj);
	}

#ifndef _PSX

	if (Obj->SparkGen) 
	{
		//FreeSparkGen(Obj->SparkGen);
		Obj->SparkGen = NULL;
	}

#ifdef _PC
	if (Obj->Sfx3D)
	{
		FreeSfx3D(Obj->Sfx3D);
	}
#endif

	if (Obj->Light)
	{
		FreeLight(Obj->Light);
	}

#endif

	if (Obj->Data)
	{
		free(Obj->Data);
		Obj->Data = NULL;
	}
	
	if (Obj->Field)
	{
		RemoveField(Obj->Field);
		Obj->Field = NULL;
	}

	// Free the collision skin if necessary
	FreeCollSkin(&Obj->body.CollSkin);
	if (IsBodySphere(&Obj->body)) {
		free(Obj->body.CollSkin.Sphere);
		Obj->body.CollSkin.Sphere = NULL;
		Obj->body.CollSkin.NSpheres = 0;
	}

#ifndef _PSX
	GRD_RemoveObject(Obj);
#endif

	// Nullify all the handlers
	Obj->renderhandler = NULL;
	Obj->movehandler = NULL;
	Obj->collhandler = NULL;
	Obj->aihandler = NULL;
	Obj->freehandler = NULL;


	if (Obj->prev != NULL)									// Update next and prev pointers of adjacent objects
	{														// to close up list
		(Obj->prev)->next = Obj->next;
	}
	else
    {
        OBJ_ObjectHead = Obj->next;
	}

	if (Obj->next != NULL)
	{
		(Obj->next)->prev = Obj->prev;
	}
	else 
	{
		OBJ_ObjectTail = Obj->prev;
	}


	if (s_NextFreeObj != NULL)								// Add object to free list
	{
		s_NextFreeObj->prev = Obj;
	}

	Obj->next = s_NextFreeObj;
	Obj->prev = NULL;
	s_NextFreeObj = Obj;
	
	OBJ_NumObjects--;

	return(1);
}

/////////////////////////////////////////////////////////////////////
//
// ClearPairCollInfo:
//
/////////////////////////////////////////////////////////////////////
void ClearThisObjPairInfo(OBJECT *obj2)
{
	OBJECT *obj1;

	for (obj1 = OBJ_ObjectHead; obj1 != NULL; obj1 = obj1->next) {

			ClearPairInfo(obj1, obj2);
	
	}
}

void ClearActivePairInfo()
{
	OBJECT *obj1, *obj2;

	for (obj1 = OBJ_ObjectHead; obj1 != NULL; obj1 = obj1->next) {
		for (obj2 = obj1->next; obj2 != NULL; obj2 = obj2->next) {

			ClearPairInfo(obj1, obj2);

		}
	}
}

void ClearAllPairInfo()
{
	int ii, jj;

	for (ii = 0; ii < MAX_OBJECTS; ii++) {
		for (jj = 0; jj < MAX_OBJECTS; jj++) {

			ClearPairInfo(&OBJ_ObjectList[ii], &OBJ_ObjectList[jj]);

		}
	}
}
