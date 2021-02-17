/*********************************************************************************************
 *
 * cardata.c
 *
 * Re-Volt (PC) Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Car data structures
 *
 *********************************************************************************************
 *
 * xx/xx/97 Balor Knight
 *  File inception.
 *
 * 12/02/98 Matt Taylor
 *	Split out from car.cpp
 *
 *********************************************************************************************/

#include "revolt.h"
#include "NewColl.h"
#include "Particle.h"
#include "Body.h"
#include "car.h"


//
// CarInfo
//
// Setup information for each car
//

CAR_INFO CarInfo[MAX_CAR_TYPES] = {
	{
		// renault 5
		"cars\\r5\\body.prm",				// Model filenames
		"cars\\r5\\wheelfl.prm",
		"cars\\r5\\wheelfr.prm",
		"cars\\r5\\wheelbl.prm",
		"cars\\r5\\wheelbr.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		CAR_MODEL_BODY,						// Body model
		0, 0, 0,							// Model Offset
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction

		// Wheel Info
		CAR_MODEL_WHEEL1, 
		-16, 14, 32, 
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2, 
		16, 14, 32,
		4.5, 0, 0,	
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3, 
		-16, 14, -38,
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4, 
		16, 14, -38,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio

		// Spring Info
		CAR_MODEL_NONE,	
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,	
		0, 0, 0,
		0,

		// Axle Info
		CAR_MODEL_NONE,	
		0, 0, 0,
		0,
		CAR_MODEL_NONE, 
		0, 0, 0,
		0,
		CAR_MODEL_NONE, 
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		
		// Pin Info
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE, 
		0, 0, 0,
		0,
		CAR_MODEL_NONE, 
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,

		// Aerial Info
		CAR_MODEL_AERIAL_SEC1,
		CAR_MODEL_AERIAL_TOP1,
		12, -33, -27,						// Aerial Base Offset
		0, -1, 0,							// Aerial pointing direction
		13,

	},
	{
		// rc
		"cars\\rc\\body.prm",
		"cars\\rc\\wheelfl.prm",
		"cars\\rc\\wheelfr.prm",
		"cars\\rc\\wheelbl.prm",
		"cars\\rc\\wheelbr.prm",
		"cars\\rc\\spring.prm",
		NULL,
		NULL,
		NULL,
		"cars\\rc\\axle.prm",
		NULL,
		NULL,
		NULL,
		"cars\\rc\\pin.prm",
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		// Body Info
		CAR_MODEL_BODY,						// Body Model
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction

		// Wheel Info
		CAR_MODEL_WHEEL1,
		-18, 8, 42,
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2, 
		18, 8, 42,
		4.5, 0, 0,							
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3, 
		-18, 8, -30,
		-6, 0, 0,							
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4, 
		18, 8, -30,
		6, 0, 0, 							
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		

		// Spring Info
		CAR_MODEL_SPRING1,
		-6, -10, 41,
		16,
		CAR_MODEL_SPRING1, 
		6, -10, 41,
		16,
		CAR_MODEL_SPRING1, 
		-8, -12, -38,
		16,
		CAR_MODEL_SPRING1, 
		8, -12, -38,
		16,

		// Axle info
		CAR_MODEL_AXLE1,
		-7, 7, 42,
		12,
		CAR_MODEL_AXLE1,
		7, 7, 42,
		12,
		CAR_MODEL_AXLE1,
		-7, 7, -30,
		12,
		CAR_MODEL_AXLE1,
		7, 7, -30,
		12,
		
		// Pin Info
		CAR_MODEL_PIN1,
		0, 0, 0,
		-1.35f,
		CAR_MODEL_PIN1,
		0, 0, 0,
		-1.35f,
		CAR_MODEL_NONE,
		0, 0, 0,
		-1.5f,
		CAR_MODEL_NONE,
		0, 0, 0,
		-1.5f,

		// Aerial Info
		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-10, -20, -50,						// Aerial Base Offset		
		0, -1, 0,							// Aerial pointing direction
		20									// Aerial Length
	},
	{
		// flag
		"cars\\flag\\body.prm",
		"cars\\flag\\wheelfl.prm",
		"cars\\flag\\wheelfr.prm",
		"cars\\flag\\wheelbl.prm",
		"cars\\flag\\wheelbr.prm",
		"cars\\flag\\spring.prm",
		NULL,
		NULL,
		NULL,
		"cars\\flag\\axle.prm",
		NULL,
		NULL,
		NULL,
		"cars\\flag\\pin.prm",
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		// Body Info
		CAR_MODEL_BODY,
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction
		
		// Wheel Info
		CAR_MODEL_WHEEL1, 
		-20, 4, 46, 
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2,
		20, 4, 46, 
		4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3,
		-20, 4, -34, 
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4,
		20, 4, -34,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		
		// Spring Info
		CAR_MODEL_SPRING1, 
		-7, -15, 41, 
		16,
		CAR_MODEL_SPRING1,
		7, -15, 41, 
		16,
		CAR_MODEL_SPRING1,
		-11, -22, -38, 
		16,
		CAR_MODEL_SPRING1,
		11, -22, -38,
		16,
		
		// Axle Info
		CAR_MODEL_AXLE1, 
		-7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		-11, 3, -34, 
		12,
		CAR_MODEL_AXLE1,
		11, 3, -34,
		12,
		
		// Pin Info
		CAR_MODEL_PIN1, 
		0, 0, 0,
		-1.55f,
		CAR_MODEL_PIN1, 
		0, 0, 0,
		-1.55f,
		CAR_MODEL_NONE, 
		0, 0, 0,
		-1.8f,
		CAR_MODEL_NONE, 
		0, 0, 0,
		-1.8f,

		// Aerial Info
		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-10, -20, -50,						// Aerial Base Offset		
		0, -1, 0,							// Aerial pointing direction
		20									// Aerial Length
	},
	{
		// dino
		"cars\\dino\\body.prm",
		"cars\\dino\\wheelfl.prm",
		"cars\\dino\\wheelfr.prm",
		"cars\\dino\\wheelbl.prm",
		"cars\\dino\\wheelbr.prm",
		"cars\\dino\\spring.prm",
		NULL,
		NULL,
		NULL,
		"cars\\dino\\axle.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		CAR_MODEL_BODY,
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction
		
		CAR_MODEL_WHEEL1,
		-20, 4, 37,
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2,
		20, 4, 37,
		4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3,
		-20, 4, -36,
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4,
		20, 4, -36,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		
		CAR_MODEL_SPRING1, 
		-15, -4, 37,
		16,
		CAR_MODEL_SPRING1,
		15, -4, 37,
		16,
		CAR_MODEL_SPRING1,
		-17, -4, -36,
		16,
		CAR_MODEL_SPRING1, 
		17, -4, -36,
		16,

		CAR_MODEL_AXLE1,
		-13, 2, 35,
		12,
		CAR_MODEL_AXLE1,
		13, 2, 35,
		12,
		CAR_MODEL_AXLE1,
		-13, 1, -36,
		12,
		CAR_MODEL_AXLE1,
		13, 1, -36,
		12,

		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		
		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-10, -20, -50,						// Aerial Base Offset		
		0, -1, 0,							// Aerial pointing direction
		20									// Aerial Length
	},
	{
		// phat
		"cars\\phat\\body.prm",
		"cars\\phat\\wheelfl.prm",
		"cars\\phat\\wheelfr.prm",
		"cars\\phat\\wheelbl.prm",
		"cars\\phat\\wheelbr.prm",
		"cars\\phat\\spring.prm",
		NULL,
		NULL,
		NULL,
		"cars\\phat\\axle.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		// Body Info
		CAR_MODEL_BODY,
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction

		// Wheel Info
		CAR_MODEL_WHEEL1, 
		-28, -1, 33, 
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2,
		28, -1, 33, 
		4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3,
		-28, -1, -42, 
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4,
		28, -1, -42,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio

		// Spring Info
		CAR_MODEL_SPRING1, 
		-10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		-12, -29, -44, 
		16,
		CAR_MODEL_SPRING1,
		12, -29, -44,
		16,
		
		// Axle Info
		CAR_MODEL_AXLE1, 
		-7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		-11, 3, -34, 
		12,
		CAR_MODEL_AXLE1,
		11, 3, -34,
		12,
		
		// Pin Info
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,

		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-20, -26, -20,						// Aerial Base Offset		
		-0.31f, -0.9f, -0.31f,					// Aerial pointing direction
		20									// Aerial Length
	},
	{
		// moss
		"cars\\moss\\body.prm",
		"cars\\moss\\wheelfl.prm",
		"cars\\moss\\wheelfr.prm",
		"cars\\moss\\wheelbl.prm",
		"cars\\moss\\wheelbr.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\moss\\axle.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		// Body Info
		CAR_MODEL_BODY, 
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction

		// Wheel Info
		CAR_MODEL_WHEEL1, 
		-28, -1, 33, 
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2,
		28, -1, 33, 
		4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3,
		-28, -1, -42, 
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4,
		28, -1, -42,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		
		// Spring Info
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,

		// Axle Info
		CAR_MODEL_AXLE1, 
		-7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		-11, 3, -34, 
		12,
		CAR_MODEL_AXLE1,
		11, 3, -34,
		12,

		// Pin Info
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,

		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-10, -20, -50,						// Aerial Base Offset		
		0, -1, 0,							// Aerial pointing direction
		20									// Aerial Length
	},
	{
		// beatall
		"cars\\beatall\\body.prm",
		"cars\\beatall\\wheelfl.prm",
		"cars\\beatall\\wheelfr.prm",
		"cars\\beatall\\wheelbl.prm",
		"cars\\beatall\\wheelbr.prm",
		"cars\\beatall\\spring.prm",
		NULL,
		NULL,
		NULL,
		"cars\\beatall\\axle.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		// Body Info
		CAR_MODEL_BODY, 
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction
		
		// Wheel Info
		CAR_MODEL_WHEEL1, 
		-28, -1, 33,
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2, 
		28, -1, 33, 
		4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3,
		-28, -1, -42, 
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4,
		28, -1, -42,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		
		// Spring Info
		CAR_MODEL_SPRING1, 
		-10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		-12, -29, -44, 
		16,
		CAR_MODEL_SPRING1,
		12, -29, -44,
		16,

		// Axle Info
		CAR_MODEL_AXLE1, 
		-7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		-11, 3, -34, 
		12,
		CAR_MODEL_AXLE1,
		11, 3, -34,
		12,
		
		// Pin Info
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,

		// Aerial Info
		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-10, -20, -50,						// Aerial Base Offset		
		0, -1, 0,							// Aerial pointing direction
		20									// Aerial Length
	},
	{
		// mite
		"cars\\mite\\body.prm",
		"cars\\mite\\wheelfl.prm",
		"cars\\mite\\wheelfr.prm",
		"cars\\mite\\wheelbl.prm",
		"cars\\mite\\wheelbr.prm",
		"cars\\mite\\spring.prm",
		NULL,
		NULL,
		NULL,
		"cars\\mite\\axle.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		// Body Info
		CAR_MODEL_BODY, 
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction

		// Wheel Info
		CAR_MODEL_WHEEL1, 
		-28, -1, 33, 
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2,
		28, -1, 33, 
		4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3,
		-28, -1, -42, 
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4,
		28, -1, -42,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		
		// Spring Info
		CAR_MODEL_SPRING1, 
		-10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		-12, -29, -44, 
		16,
		CAR_MODEL_SPRING1,
		12, -29, -44,
		16,

		// Axle Info
		CAR_MODEL_AXLE1, 
		-7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		-11, 3, -34, 
		12,
		CAR_MODEL_AXLE1,
		11, 3, -34,
		12,

		// Pin Info
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,

		// Aerial Info
		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-10, -20, -50,						// Aerial Base Offset		
		0, -1, 0,							// Aerial pointing direction
		20									// Aerial Length
	},
	{
		// gencar
		"cars\\gencar\\body.prm",
		"cars\\gencar\\wheelfl.prm",
		"cars\\gencar\\wheelfr.prm",
		"cars\\gencar\\wheelbl.prm",
		"cars\\gencar\\wheelbr.prm",
		"cars\\gencar\\spring.prm",
		NULL,
		NULL,
		NULL,
		"cars\\gencar\\axle.prm",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL,
		"cars\\misc\\Aerial.m",
		"cars\\misc\\AerialTop.m",

		// Body Info
		CAR_MODEL_BODY, 
		0, 0, 0,
		1.0f,								// Mass
		1767.0f, 0.0f, 0.0f,				// Inertia Matrix
		0.0f, 4967.0f, 0.0f, 
		0.0f, 0.0f, 3467.0f,	
		1000.0f,							// Acc due to gravity
		0.0f,								// Hardness
		0.003f,								// Air resistance
		0.3f,								// Static Friction
		0.3f,								// Kinetic Friction

		// Wheel Info
		CAR_MODEL_WHEEL1, 
		-28, -1, 33, 
		-4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL2,
		28, -1, 33, 
		4.5, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL3,
		-28, -1, -42, 
		-6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		CAR_MODEL_WHEEL4,
		28, -1, -42,
		6, 0, 0,
		10,
		0.01f,								// Mass
		1000.0f,							// Gravity
		0.4f,								// Static Friciton
		0.3f,								// Kinetic Friction
		0.1f,								// Axle Friction
		0.5f,								// Steer Ratio
		
		// Spring Info
		CAR_MODEL_SPRING1, 
		-10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		10, -15, 34, 
		16,
		CAR_MODEL_SPRING1,
		-12, -29, -44, 
		16,
		CAR_MODEL_SPRING1,
		12, -29, -44,
		16,
		
		// Axle Info
		CAR_MODEL_AXLE1, 
		-7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		7, 3, 46, 
		12,
		CAR_MODEL_AXLE1,
		-11, 3, -34, 
		12,
		CAR_MODEL_AXLE1,
		11, 3, -34,
		12,
		
		// Pin Info
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		CAR_MODEL_NONE,
		0, 0, 0,
		0,
		
		// Aerial Info
		CAR_MODEL_AERIAL_SEC1,				// Aerial Models
		CAR_MODEL_AERIAL_TOP1,
		-10, -20, -50,						// Aerial Base Offset		
		0, -1, 0,							// Aerial pointing direction
		20									// Aerial Length
	}
};

