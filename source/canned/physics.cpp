
#include "revolt.h"
#include "coll.h"
#include "geom.h"
#include "3d.h"
#include "sound.h"
#include "input.h"
#include "model.h"
#include "car.h"
#include "physics.h"
#include "text.h"

extern IDirect3DDevice2 *D3Ddevice;

extern char Keys[];
extern float TimeFactor;

extern CAR Car[];

float Velocity[3];
float Friction;

char RevsSfx = 0;
short RevsHandle;

char CollisionDeadLock = 0;

float Debug0, Debug1, Debug2;

void BuildCarFromWheels2(CAR *car);


/************************************************************************************************************/

void ControlCar( CAR *car )
{

// Temp Sound effects for Revs

if (RevsSfx == 0)
{
	RevsHandle = PlaySample(2, -2500, 8000, DSBPAN_CENTER, 1);
	RevsSfx = 1;
}

if (RevsSfx == 1) 
	ChangeSample(2, RevsHandle, DSBVOLUME_MAX, (long)6000 + abs((long)(car->Revs*2.9f)), DSBPAN_CENTER);



// wheel turn

	float turn = 0.003f * (float)TimeFactor;
	char turned = 0;

	if (Keys[DIK_LEFT])
		car->WheelTurn -= turn, turned = 1;
	if (Keys[DIK_RIGHT]) 
		car->WheelTurn += turn, turned = 1;

// Limit max turn

	if (car->WheelTurn < -0.06f)
		car->WheelTurn = -0.06f;
	if (car->WheelTurn > 0.06f)
		car->WheelTurn = 0.06f;


// Auto level wheel turn 

if( turned == 0 )
{
	if( car->WheelTurn > 0 )
	{
		car->WheelTurn -=  0.003f*TimeFactor;
		if( car->WheelTurn < 0 )
			car->WheelTurn = 0;
	}

	if( car->WheelTurn < 0 )
	{
		car->WheelTurn +=  0.003f*TimeFactor;
		if( car->WheelTurn > 0 )
			car->WheelTurn = 0;
	}
}

	
	
}


/************************************************************************************************************/

void UpdateCarMotion( CAR *car )
{

	float Vect1[3], Vect2[3], Vel;

	// *** Velocity change (if any) due to stearing **

	// make 1-D Velocity

	Vel = GetScalar( car->Matrix, car->LastVelocity );

	// Update Stearing Matrix
	
	BuildTurnMatrices(car);


	// New 3-D Velocity based on new stearing matrix

	SetVector( Vect1, 0, 0, Vel );
	RotVector( car->TurnCarMatrix, Vect1, Vect2 );

	car->Velocity[0] = Vect2[0];
	car->Velocity[1] = Vect2[1];
	car->Velocity[2] = Vect2[2];


	// *** Accelerate due to Engine force ***

	if( Keys[DIK_UP] )
	{
		SetVector( Vect1, 0, 0, 0.5f*TimeFactor );
		RotVector(car->TurnCarMatrix, Vect1, Vect2 );
		car->Velocity[0] += Vect2[0];
		car->Velocity[1] += Vect2[1];
		car->Velocity[2] += Vect2[2];
	}

	if( Keys[DIK_DOWN] )
	{
		SetVector( Vect1, 0, 0, -0.5f*TimeFactor );
		RotVector(car->TurnCarMatrix, Vect1, Vect2 );
		car->Velocity[0] += Vect2[0];
		car->Velocity[1] += Vect2[1];
		car->Velocity[2] += Vect2[2];
	}



	// *** Acceleration due to gravity

	car->Gravity += (GRAVITY/100)*TimeFactor;


	// *** Drag ***

	float Drag[3];

	Drag[0] = car->Velocity[0]*car->Velocity[0]*COEFFOFDRAG;
	Drag[1] = car->Velocity[1]*car->Velocity[1]*COEFFOFDRAG;
	Drag[2] = car->Velocity[2]*car->Velocity[2]*COEFFOFDRAG;

	if( car->Velocity[0] > 0 )
		car->Velocity[0] -= Drag[0]*TimeFactor;
	if( car->Velocity[0] < 0 ) 
		car->Velocity[0] += Drag[0]*TimeFactor;

	if( car->Velocity[1] > 0 )
		car->Velocity[1] -= Drag[1]*TimeFactor;
	if( car->Velocity[1] < 0 ) 
		car->Velocity[1] += Drag[1]*TimeFactor;

	if( car->Velocity[2] > 0 )
		car->Velocity[2] -= Drag[2]*TimeFactor;
	if( car->Velocity[2] < 0 )
		car->Velocity[2] += Drag[2]*TimeFactor;


	// *** Update car position with new Velocitys ***

	// Store current car Pos
	
	car->LastPos[0] = car->WorldPos[0];
	car->LastPos[1] = car->WorldPos[1];
	car->LastPos[2] = car->WorldPos[2];
	
	// Move the car according to key presses, traction and circular motion

	CircularMotion( car );
	

	// Add Velocity produced from acceleration due to Gravity to the world pos

	car->WorldPos[1] += car->Gravity*TimeFactor;

	// Collision check

	CollisionMain( car );
	
	// Storing away 3-D Velocities for use next time

	car->LastVelocity[0] = car->Velocity[0];
	car->LastVelocity[1] = car->Velocity[1];
	car->LastVelocity[2] = car->Velocity[2];


	// Cosmetic wheel spin (to be done properly later, i.e based on engine revs as well as velocity)

	car->MasterVelocity = GetScalar( car->Matrix, car->Velocity );

	car->WheelBL.Spin += (float)(car->MasterVelocity/DRIVEWHEELCIRC);
	car->WheelBR.Spin += (float)(car->MasterVelocity/DRIVEWHEELCIRC);
	car->WheelFL.Spin += (float)(car->MasterVelocity/DRIVEWHEELCIRC);
	car->WheelFR.Spin += (float)(car->MasterVelocity/DRIVEWHEELCIRC);

	car->Revs = ( car->MasterVelocity / GEARRATIO * IDEALFRAMERATE * 60 ) / DRIVEWHEELCIRC;


}

/************************************************************************************************************/
// CircularMotion( CAR *car )
//
// Known problem:  w will be greater than v, thus after many iterations v will become greater and greater 
//					might go back and fix this later if it becomes a problem.
/************************************************************************************************************/

void CircularMotion( CAR *car )
{
	float d, r, v, w;
	float Temp[3], Temp2[3], Origin[3];
	float TempMatrix[9];
	
	// *** distance, d, between front and rear wheels

	d = car->WheelFL.Offset[2] - car->WheelBL.Offset[2];
	

	// *** radius of turning circle,   r = d * tan(90-WheelTurn Angle)

	double tanvalue;
	tanvalue = tan((1.570796327f)-(car->WheelTurn*3.14159f));

	if( tanvalue > 0 )
	{
		if( tanvalue > 20000 )
			tanvalue = 20000;
	}
	else
	{
		if( tanvalue < -20000 )
			tanvalue = -20000;
	}

	r = (float)tanvalue*d;


	// *** velocity, v  

	v = GetScalar( car->TurnCarMatrix, car->Velocity );

	// *** angular, w = v / r
	
	w = (v * TimeFactor) / r;
	
	// *** Move car

	SetVector( Temp2, -r, 0, -car->WheelBL.Offset[2] );
	RotVector( car->Matrix, Temp2, Temp );

	Origin[0] = car->WorldPos[0] - Temp[0];
	Origin[1] = car->WorldPos[1] - Temp[1];
	Origin[2] = car->WorldPos[2] - Temp[2];


	TempMatrix[1] = TempMatrix[2] = TempMatrix[3] = TempMatrix[5] = TempMatrix[6] = TempMatrix[7] = 0;
	TempMatrix[0] = TempMatrix[4] = TempMatrix[8] = 1;

	FBuildMatrix( 0, -w, 0, TempMatrix );
	
    SetVector( Temp2, -r, 0, -car->WheelBL.Offset[2] );
	RotVector( TempMatrix, Temp2, Temp );
	RotVector( car->Matrix, Temp, Temp2 );


	car->Velocity[0] = -car->WorldPos[0];
	car->Velocity[1] = -car->WorldPos[1];
	car->Velocity[2] = -car->WorldPos[2];


	car->WorldPos[0] = Origin[0] + Temp2[0];
	car->WorldPos[1] = Origin[1] + Temp2[1];
	car->WorldPos[2] = Origin[2] + Temp2[2];


	car->Velocity[0] += car->WorldPos[0];
	car->Velocity[1] += car->WorldPos[1];
	car->Velocity[2] += car->WorldPos[2];

	car->Velocity[0] = car->Velocity[0] / TimeFactor;
	car->Velocity[1] = car->Velocity[1] / TimeFactor;
	car->Velocity[2] = car->Velocity[2] / TimeFactor;


	// Limit acceleration due max force tires can take

	
	car->WorldPos[0] -=  car->Velocity[0]*TimeFactor;
	car->WorldPos[1] -=  car->Velocity[1]*TimeFactor;
	car->WorldPos[2] -=  car->Velocity[2]*TimeFactor;


	float AccelerationVect[3];
	float AccLimit;

	AccelerationVect[0] = (car->Velocity[0] - car->LastVelocity[0]);
	AccelerationVect[1] = (car->Velocity[1] - car->LastVelocity[1]);
	AccelerationVect[2] = (car->Velocity[2] - car->LastVelocity[2]);


	car->Acceleration = (float)sqrt( (AccelerationVect[0]*AccelerationVect[0])
					         	    +(AccelerationVect[1]*AccelerationVect[1])
			        			    +(AccelerationVect[2]*AccelerationVect[2]) );


	AccLimit = car->Acceleration;

	if( car->Acceleration > COEFFOFFRICTION *TimeFactor )
		AccLimit = COEFFOFFRICTION *TimeFactor;
	

	if( car->Acceleration > 0 )
	{
		AccelerationVect[0] = (AccLimit*AccelerationVect[0])/car->Acceleration;
		AccelerationVect[1] = (AccLimit*AccelerationVect[1])/car->Acceleration;
		AccelerationVect[2] = (AccLimit*AccelerationVect[2])/car->Acceleration;
	}


	car->Velocity[0] = car->LastVelocity[0] + AccelerationVect[0];
	car->Velocity[1] = car->LastVelocity[1] + AccelerationVect[1];
	car->Velocity[2] = car->LastVelocity[2] + AccelerationVect[2];

	car->WorldPos[0] +=  car->Velocity[0]*TimeFactor;
	car->WorldPos[1] +=  car->Velocity[1]*TimeFactor;
	car->WorldPos[2] +=  car->Velocity[2]*TimeFactor;
	

	// Update Rotation Matrix of car by adding angular speed

	FBuildMatrix( 0, -w, 0, car->Matrix );

}


/************************************************************************************************************/
// BounceAgainstPlane( float *Approach, float *Plane, float *Exit )
// 
// Plane must be normalised to unit vector 1
/************************************************************************************************************/

void BounceAgainstPlane( float *Approach, float *Plane, float *Exit )
{
	float DotProduct;
	DotProduct = (Approach[0]*Plane[0]) + (Approach[1]*Plane[1]) + (Approach[2]*Plane[2]);

	Exit[0] =  Approach[0] - (2*Plane[0]*DotProduct);
	Exit[1] =  Approach[1] - (2*Plane[1]*DotProduct);
	Exit[2] =  Approach[2] - (2*Plane[2]*DotProduct);

}


/************************************************************************************************************/
// BounceAgainstPlane2( float *Approach, float *Plane, float *Exit )
// 
// Plane must be normalised to unit vector 1
/************************************************************************************************************/

void BounceAgainstPlane2( float *Approach, float *Plane, float *Exit )
{
	float DotProduct;

	DotProduct = (Approach[0]*Plane[0]) + (Approach[1]*Plane[1]) + (Approach[2]*Plane[2]);

	Exit[0] =  Approach[0] - (Plane[0]*DotProduct);
	Exit[1] =  Approach[1] - (Plane[1]*DotProduct);
	Exit[2] =  Approach[2] - (Plane[2]*DotProduct);

}



/************************************************************************************************************/

void PhysicsCollision( CAR *car, float *Offset )
{

	float After[3];
	float ReturnedFriction;
	float Normal[3];
	float Exit[3];
	float Velocity[3];
	float TotalDisplacement, ActualDisplacement;
	float Old[3];
	float Current[3];


	CollisionDeadLock++;

	if( CollisionDeadLock > 7 )
		return;
	

	Old[0] = car->LastPos[0] + Offset[0];
	Old[1] = car->LastPos[1] + Offset[1];
	Old[2] = car->LastPos[2] + Offset[2];
	
	Current[0] = car->WorldPos[0] + Offset[0];
	Current[1] = car->WorldPos[1] + Offset[1];
	Current[2] = car->WorldPos[2] + Offset[2];

	if( SphereCollTestNorm( Old, Current, 10, After, &ReturnedFriction, Normal ) )
	{
	
		// Calc overall distance to travel and how much of that it actually did before collision

/*		TotalDisplacement = (float)sqrt( ((car->WorldPos[0]-car->LastPos[0])*(car->WorldPos[0]-car->LastPos[0])) + 
										 ((car->WorldPos[1]-car->LastPos[1])*(car->WorldPos[1]-car->LastPos[1])) + 
										 ((car->WorldPos[2]-car->LastPos[2])*(car->WorldPos[2]-car->LastPos[2]))  
										);

		ActualDisplacement = (float)sqrt( ((After[0]- Offset[0]-car->LastPos[0])*(After[0]- Offset[0]-car->LastPos[0])) + 
			     						  ((After[1]- Offset[1]-car->LastPos[1])*(After[1]- Offset[1]-car->LastPos[1])) + 
										  ((After[2]- Offset[2]-car->LastPos[2])*(After[2]- Offset[2]-car->LastPos[2]))  
										);
*/

		// Get the velocity of car

//		Velocity[0] = (car->WorldPos[0] - car->LastPos[0])/TimeFactor;
//		Velocity[1] = (car->WorldPos[1] - car->LastPos[1])/TimeFactor;
//		Velocity[2] = (car->WorldPos[2] - car->LastPos[2])/TimeFactor;

	
		// acceleration due to gravity is zero

		car->Gravity = 0;

		
		// Set World pos to point of collision
		
	  /*  car->LastPos[0] =*/ car->WorldPos[0] = After[0] - Offset[0]; 
	/*	car->LastPos[1] =*/ car->WorldPos[1] = After[1] - Offset[1];
	/*	car->LastPos[2] =*/ car->WorldPos[2] = After[2] - Offset[2];

		// Bounce off plane

		
/*		BounceAgainstPlane2( Velocity, Normal, Exit );
		
		car->WorldPos[0] += Exit[0]*abs(TotalDisplacement-ActualDisplacement)/TotalDisplacement*TimeFactor;
		car->WorldPos[1] += Exit[1]*abs(TotalDisplacement-ActualDisplacement)/TotalDisplacement*TimeFactor;
		car->WorldPos[2] += Exit[2]*abs(TotalDisplacement-ActualDisplacement)/TotalDisplacement*TimeFactor;
	

		car->Velocity[0] = Exit[0];
		car->Velocity[1] = Exit[1];
		car->Velocity[2] = Exit[2];
*/
		PhysicsCollision( car, Offset );

	}


}
	
/************************************************************************************************************/

void CollisionMain( CAR *car )
{


	
float Quim[3], Large[3];
float WP[3], LP[3];


	Quim[0] = car->WheelFL.Offset[0] + car->WheelFL.LocalPoint[0];
	Quim[1] = car->WheelFL.Offset[1] + car->WheelFL.LocalPoint[1];
	Quim[2] = car->WheelFL.Offset[2] + car->WheelFL.LocalPoint[2];

	RotVector( car->Matrix, Quim, Large );
	CollisionDeadLock = 0;
	WP[0] = car->WorldPos[0];
	WP[1] = car->WorldPos[1];
	WP[2] = car->WorldPos[2];
	LP[0] = car->LastPos[0];
	LP[1] = car->LastPos[1];
	LP[2] = car->LastPos[2];
	
	PhysicsCollision( car, Large);

	car->WheelFL.CollPoint[0] = car->WorldPos[0] + Large[0];
	car->WheelFL.CollPoint[1] = car->WorldPos[1] + Large[1];
	car->WheelFL.CollPoint[2] = car->WorldPos[2] + Large[2];

	car->WorldPos[0] = WP[0];
	car->WorldPos[1] = WP[1];
	car->WorldPos[2] = WP[2];
	car->LastPos[0] = LP[0];
	car->LastPos[1] = LP[1];
	car->LastPos[2] = LP[2];




	Quim[0] = car->WheelFR.Offset[0] + car->WheelFR.LocalPoint[0];
	Quim[1] = car->WheelFR.Offset[1] + car->WheelFR.LocalPoint[1];
	Quim[2] = car->WheelFR.Offset[2] + car->WheelFR.LocalPoint[2];

	RotVector( car->Matrix, Quim, Large );
	CollisionDeadLock = 0;
	WP[0] = car->WorldPos[0];
	WP[1] = car->WorldPos[1];
	WP[2] = car->WorldPos[2];
	LP[0] = car->LastPos[0];
	LP[1] = car->LastPos[1];
	LP[2] = car->LastPos[2];
	
	PhysicsCollision( car, Large);

	car->WheelFR.CollPoint[0] = car->WorldPos[0] + Large[0];
	car->WheelFR.CollPoint[1] = car->WorldPos[1] + Large[1];
	car->WheelFR.CollPoint[2] = car->WorldPos[2] + Large[2];
	
	car->WorldPos[0] = WP[0];
	car->WorldPos[1] = WP[1];
	car->WorldPos[2] = WP[2];
	car->LastPos[0] = LP[0];
	car->LastPos[1] = LP[1];
	car->LastPos[2] = LP[2];






	Quim[0] = car->WheelBL.Offset[0] + car->WheelBL.LocalPoint[0];
	Quim[1] = car->WheelBL.Offset[1] + car->WheelBL.LocalPoint[1];
	Quim[2] = car->WheelBL.Offset[2] + car->WheelBL.LocalPoint[2];

	RotVector( car->Matrix, Quim, Large );
	CollisionDeadLock = 0;
	WP[0] = car->WorldPos[0];
	WP[1] = car->WorldPos[1];
	WP[2] = car->WorldPos[2];
	LP[0] = car->LastPos[0];
	LP[1] = car->LastPos[1];
	LP[2] = car->LastPos[2];
	
	PhysicsCollision( car, Large);

	car->WheelBL.CollPoint[0] = car->WorldPos[0] + Large[0];
	car->WheelBL.CollPoint[1] = car->WorldPos[1] + Large[1];
	car->WheelBL.CollPoint[2] = car->WorldPos[2] + Large[2];

	car->WorldPos[0] = WP[0];
	car->WorldPos[1] = WP[1];
	car->WorldPos[2] = WP[2];
	car->LastPos[0] = LP[0];
	car->LastPos[1] = LP[1];
	car->LastPos[2] = LP[2];




	Quim[0] = car->WheelBR.Offset[0] + car->WheelBR.LocalPoint[0];
	Quim[1] = car->WheelBR.Offset[1] + car->WheelBR.LocalPoint[1];
	Quim[2] = car->WheelBR.Offset[2] + car->WheelBR.LocalPoint[2];

	RotVector( car->Matrix, Quim, Large );
	CollisionDeadLock = 0;
	WP[0] = car->WorldPos[0];
	WP[1] = car->WorldPos[1];
	WP[2] = car->WorldPos[2];
	LP[0] = car->LastPos[0];
	LP[1] = car->LastPos[1];
	LP[2] = car->LastPos[2];
	
	PhysicsCollision( car, Large);

	car->WheelBR.CollPoint[0] = car->WorldPos[0] + Large[0];
	car->WheelBR.CollPoint[1] = car->WorldPos[1] + Large[1];
	car->WheelBR.CollPoint[2] = car->WorldPos[2] + Large[2];

	car->WorldPos[0] = WP[0];
	car->WorldPos[1] = WP[1];
	car->WorldPos[2] = WP[2];
	car->LastPos[0] = LP[0];
	car->LastPos[1] = LP[1];
	car->LastPos[2] = LP[2];



//carBuildWheelCollision( car, &car->WheelFL );
//	CarBuildWheelCollision( car, &car->WheelFR );
//	CarBuildWheelCollision( car, &car->WheelBL );
//	CarBuildWheelCollision( car, &car->WheelBR );




//	CollisionDeadLock = 0;
//	WholeCollision( car );



	BuildCarFromWheels2( car);


//	FBuildMatrix( 0.01f, 0, 0, car->Matrix );

/*
	CarBuildWheelCollision( car, &car->WheelFL );
	CarBuildWheelCollision( car, &car->WheelFR );
	CarBuildWheelCollision( car, &car->WheelBL );
	CarBuildWheelCollision( car, &car->WheelBR );

	CollisionDeadLock = 0;
	WheelCollision( car, &car->WheelFL );
	CollisionDeadLock = 0;
	WheelCollision( car, &car->WheelFR );
	CollisionDeadLock = 0;
	WheelCollision( car, &car->WheelBL );
	CollisionDeadLock = 0;
	WheelCollision( car, &car->WheelBR );


	car->WorldPos[0] = (car->WheelFL.CollPoint[0]+car->WheelFR.CollPoint[0]+
						car->WheelBL.CollPoint[0]+car->WheelBR.CollPoint[0]	)/4;
	car->WorldPos[1] = (car->WheelFL.CollPoint[1]+car->WheelFR.CollPoint[1]+
						car->WheelBL.CollPoint[1]+car->WheelBR.CollPoint[1]	)/4;
	car->WorldPos[2] = (car->WheelFL.CollPoint[2]+car->WheelFR.CollPoint[2]+
						car->WheelBL.CollPoint[2]+car->WheelBR.CollPoint[2]	)/4;
*/
}


/************************************************************************************************************/


//////////////////////////////
// Draw       Graph         //
//////////////////////////////

D3DTLVERTEX Points[8];

void DrawGraph( CAR *car )
{


	short i;

	D3Ddevice->SetRenderState(D3DRENDERSTATE_TEXTUREHANDLE, NULL);
 
	for (i = 0 ; i < 4; i++ )
	{
		Points[i].dvSX = i;
		Points[i].dvSY = 100;
		Points[i].dvSZ = 1/(float)65536;
		Points[i].dcColor = RGB_MAKE( 255,255,255 );
	}

		for (i = 4 ; i < 8; i++ )
	{
		Points[i].dvSX = i;
		Points[i].dvSY = 100-(float)(car->Revs/300);
		Points[i].dvSZ = 1/(float)65536;
		Points[i].dcColor = RGB_MAKE( 255,0,0 );
	}



	float Vel = (float) sqrt( (car->Velocity[0]*car->Velocity[0])+
							  (car->Velocity[1]*car->Velocity[1])+
					   		  (car->Velocity[2]*car->Velocity[2]) );

	char buff[128];
	wsprintf(buff, "RPM: %5d  Actual MPH :%2d.%02d   Acc: %d  Debug: %3d,%3d,%3d",
		(int)car->Revs,
		 ((int)((car->MasterVelocity*IDEALFRAMERATE*3600)/(METERSPERMILE*WORLDMETER))),
		 abs((int)((car->MasterVelocity*IDEALFRAMERATE*3600)/(METERSPERMILE*WORLDMETER)*100) - (((int)((car->MasterVelocity*72*3600)/(METERSPERMILE*200)))*100))
		 ,(int)((car->Acceleration*100)/TimeFactor)
		 ,(int)Debug0,(int)Debug1,(int)Debug2

		 );
	DumpText(0, 16, 8, 16, 0x808080, buff);

	D3Ddevice->DrawPrimitive(D3DPT_POINTLIST, D3DVT_TLVERTEX, &Points, 8, NULL );


}


/************************************************************************************************************/

float GetScalar( float *Matrix, float *In )
{

	float Vect[3], Vect2[3];

	SetVector( Vect, 0, 0, 1 );
	RotVector( Matrix, Vect, Vect2 );

	return ((In[0]*Vect2[0]) +(In[1]*Vect2[1]) +(In[2]*Vect2[2]));

}

/************************************************************************************************************/

void WholeCollision( CAR *car )
{
		
	float After[3];
	float ReturnedFriction;
	float Normal[3];
	float Exit[3];
	float Velocity[3];
	float TotalDisplacement, ActualDisplacement;
	float Old[3];
	float Current[3];


	CollisionDeadLock++;

	if( CollisionDeadLock > 7 )
		return;
	



	Old[0] = car->LastPos[0];
	Old[1] = car->LastPos[1];
	Old[2] = car->LastPos[2];
	
	Current[0] = car->WorldPos[0];
	Current[1] = car->WorldPos[1];
	Current[2] = car->WorldPos[2];

	if( SphereCollTestNorm( Old, Current, 10, After, &ReturnedFriction, Normal ) )
	{


	
		car->Gravity = 0;






		// Set World pos to point of collision
		
		car->WorldPos[0] = After[0]; 
		car->WorldPos[1] = After[1];
		car->WorldPos[2] = After[2];
		

		WholeCollision( car );
		
	}

	Debug0 = car->WorldPos[0];
	Debug1 = car->WorldPos[1];
	Debug2 = car->WorldPos[2];

	

}
	
/************************************************************************************************************/

void GoodCollision( CAR *car )
{
		
	float After[3];
	float ReturnedFriction;
	float Normal[3];
	float Exit[3];
	float Velocity[3];
	float TotalDisplacement, ActualDisplacement;
	float Old[3];
	float Current[3];


	CollisionDeadLock++;

	if( CollisionDeadLock > 7 )
		return;
	
	Old[0] = car->LastPos[0];
	Old[1] = car->LastPos[1];
	Old[2] = car->LastPos[2];
	
	Current[0] = car->WorldPos[0];
	Current[1] = car->WorldPos[1];
	Current[2] = car->WorldPos[2];

	if( SphereCollTest( Old, Current, 10, After, &ReturnedFriction, Normal ) )
	{
	
		// Calc overall distance to travel and how much of that it actually did before collision

		TotalDisplacement = (float)sqrt( ((car->WorldPos[0]-car->LastPos[0])*(car->WorldPos[0]-car->LastPos[0])) + 
										 ((car->WorldPos[1]-car->LastPos[1])*(car->WorldPos[1]-car->LastPos[1])) + 
										 ((car->WorldPos[2]-car->LastPos[2])*(car->WorldPos[2]-car->LastPos[2]))  
										);

		ActualDisplacement = (float)sqrt( ((After[0]-car->LastPos[0])*(After[0]-car->LastPos[0])) + 
			     						  ((After[1]-car->LastPos[1])*(After[1]-car->LastPos[1])) + 
										  ((After[2]-car->LastPos[2])*(After[2]-car->LastPos[2]))  
										);


		// Get the velocity of car

		Velocity[0] = (car->WorldPos[0] - car->LastPos[0])/TimeFactor;
		Velocity[1] = (car->WorldPos[1] - car->LastPos[1])/TimeFactor;
		Velocity[2] = (car->WorldPos[2] - car->LastPos[2])/TimeFactor;

		

		// acceleration due to gravity is zero

		car->Gravity = 0;

		
		// Set World pos to point of collision
		
		car->LastPos[0] = car->WorldPos[0] = After[0]; 
		car->LastPos[1] = car->WorldPos[1] = After[1];
		car->LastPos[2] = car->WorldPos[2] = After[2];
		

		// Bounce off plane

		
		BounceAgainstPlane2( Velocity, Normal, Exit );
		
		car->WorldPos[0] += Exit[0]*abs(TotalDisplacement-ActualDisplacement)/TotalDisplacement*TimeFactor;
		car->WorldPos[1] += Exit[1]*abs(TotalDisplacement-ActualDisplacement)/TotalDisplacement*TimeFactor;
		car->WorldPos[2] += Exit[2]*abs(TotalDisplacement-ActualDisplacement)/TotalDisplacement*TimeFactor;
	
		car->Velocity[0] = Exit[0];
		car->Velocity[1] = Exit[1];
		car->Velocity[2] = Exit[2];

		GoodCollision( car );


	}


}
	
/************************************************************************************************************/

////////////////////////////////////////
// build car matrix / pos from wheels //
////////////////////////////////////////

void BuildCarFromWheels2(CAR *car)
	{
	float f[3], b[3], l[3], r[3], forward[3], right[3], up[3];

// get car world pos from wheel world positions

	car->WorldPos[0] = (car->WheelFL.CollPoint[0] + car->WheelFR.CollPoint[0] + car->WheelBL.CollPoint[0] + car->WheelBR.CollPoint[0]) / 4;
	car->WorldPos[1] = (car->WheelFL.CollPoint[1] + car->WheelFR.CollPoint[1] + car->WheelBL.CollPoint[1] + car->WheelBR.CollPoint[1]) / 4;
	car->WorldPos[2] = (car->WheelFL.CollPoint[2] + car->WheelFR.CollPoint[2] + car->WheelBL.CollPoint[2] + car->WheelBR.CollPoint[2]) / 4;

// get four mid points between wheels

	f[0] = (car->WheelFL.CollPoint[0] + car->WheelFR.CollPoint[0]) / 2;
	f[1] = (car->WheelFL.CollPoint[1] + car->WheelFR.CollPoint[1]) / 2;
	f[2] = (car->WheelFL.CollPoint[2] + car->WheelFR.CollPoint[2]) / 2;

	b[0] = (car->WheelBL.CollPoint[0] + car->WheelBR.CollPoint[0]) / 2;
	b[1] = (car->WheelBL.CollPoint[1] + car->WheelBR.CollPoint[1]) / 2;
	b[2] = (car->WheelBL.CollPoint[2] + car->WheelBR.CollPoint[2]) / 2;

	l[0] = (car->WheelFL.CollPoint[0] + car->WheelBL.CollPoint[0]) / 2;
	l[1] = (car->WheelFL.CollPoint[1] + car->WheelBL.CollPoint[1]) / 2;
	l[2] = (car->WheelFL.CollPoint[2] + car->WheelBL.CollPoint[2]) / 2;

	r[0] = (car->WheelFR.CollPoint[0] + car->WheelBR.CollPoint[0]) / 2;
	r[1] = (car->WheelFR.CollPoint[1] + car->WheelBR.CollPoint[1]) / 2;
	r[2] = (car->WheelFR.CollPoint[2] + car->WheelBR.CollPoint[2]) / 2;

// get forward / right / up vectors

	SubVector(f, b, forward);
	SubVector(r, l, right);

	CrossProduct(forward, right, up);
	CrossProduct(up, forward, right);

// normalize vectors for matrix

	Normalize(right)
	car->Matrix[0] = right[0];
	car->Matrix[1] = right[1];
	car->Matrix[2] = right[2];

	Normalize(up);
	car->Matrix[3] = up[0];
	car->Matrix[4] = up[1];
	car->Matrix[5] = up[2];

	Normalize(forward);
	car->Matrix[6] = forward[0];
	car->Matrix[7] = forward[1];
	car->Matrix[8] = forward[2];
	}

