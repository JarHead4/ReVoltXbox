// **********************************************************************************************************************/

#include <revolt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <dinput.h>
#include <geom.h>

float FNewX, FNewY, FNewZ;

float RotatedPosX,RotatedPosY,RotatedPosZ;

float FMatrixObjectCamera[9];

float FMatrixCamera[9] = { 1,0,0,0,1,0,0,0,1 };

float CameraPos[3] = { 0,-384,0};

float FMatrixObject[9] = { 1,0,0,0,1,0,0,0,1 };
float ObjectPos[3] = { 0,-16,-6800 };


float FMatrixZero[9] = { 1,0,0,0,1,0,0,0,1 };
float ZeroPos[9] = { 0,0,0 };

float CameraHomePos[3], CameraHomeHeight;

// **** Input.cpp stuff  **************************************************************************************************/

extern char Keys[256];
extern short KeyStatus;

extern DIMOUSESTATE Mouse;

// **** Temporary cobblers  ***********************************************************************************************/

DWORD FrameTimeOld = 0;
DWORD FrameTime = 0;

float Speed = 0;

float SpeedOld[3] = { 0,0,0 };
char StartSound = 0;
short GaySHandle;

#include "Sound.h"
#include "revolt.h"

#include "text.h"

short Collision[16*20]={ 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  
        	    	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  				 
  			      
                    99, 99, 99, 00, 00, 00, 00, 00, 00, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  				 
				    99, 99, 00, 00, 15, 15, 15, 15, 13, 00, 00,210, 00, 12, 00, 00, 99, 99, 99, 99,  				 
				    99, 00, 00, 15, 00, 00, 00, 00, 13, 15, 15,211, 15, 12, 15, 15, 00, 99, 99, 99,  				 
				    99, 00, 15, 00, 00, 99, 99, 99, 13, 00, 00,212, 00, 12, 00, 15, 00, 00, 99, 99,  				 
				 	99, 00, 15, 00, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 00, 00, 15, 00, 00, 99,  				 
				  	99, 00, 15, 00, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 00, 00, 15, 00, 99,  				 
				  	99, 00, 15, 00, 00, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 00, 15, 00, 99,  				 
				  	99, 00, 00, 15, 15, 00, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 00, 15, 00, 99,  				 
				  	99, 99, 00, 00, 15, 00, 00, 00, 00, 00, 00, 00, 99, 99, 99, 00, 00, 15, 00, 99,  				 
				  	99, 99, 99, 00, 15, 15, 15, 15, 00, 15, 15, 00, 00, 00, 00, 15, 15, 00, 00, 99,  				 
				    99, 99, 99, 99, 00, 00, 00, 00, 00, 00, 00, 15, 15, 15, 15, 00, 00, 00, 99, 99,  				 
				  	99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 00, 00, 00, 00, 00, 00, 99, 99, 99,  				 
				  
                    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,  				 
				    99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99  };				 



LARGE_INTEGER TimerFreq;
LARGE_INTEGER TimerLast;
LARGE_INTEGER TimerThis;
unsigned long TimeTaken;

float TimeFactor;

/***********************************************************************************************/

void FBuildMatrix( float x, float y, float z, float *FMatrix )
{
char i,j,k;
double divide;

double FMatrixAdd[9];
float FMatrixTemp[9];


// Move Current Coeffs to Temp Matrix


FMatrixTemp[0] = FMatrix[0];
FMatrixTemp[1] = FMatrix[1];
FMatrixTemp[2] = FMatrix[2];
FMatrixTemp[3] = FMatrix[3];
FMatrixTemp[4] = FMatrix[4];
FMatrixTemp[5] = FMatrix[5];
FMatrixTemp[6] = FMatrix[6];
FMatrixTemp[7] = FMatrix[7];
FMatrixTemp[8] = FMatrix[8];


// Reset Current Camera Matrix	

for( i = 0; i < 9; i++ )
	FMatrix[i] = 0;


// Build Add Matrix

FMatrixAdd[0] = (cos(z) * cos(y)) + (sin(x) * sin(z) * sin(y));

FMatrixAdd[1] = -(sin(z) * cos(y)) + (sin(x) * cos(z) * sin(y));

FMatrixAdd[2] = cos(x) * sin(y);

FMatrixAdd[3] = cos(x) * sin(z);

FMatrixAdd[4] = cos(x) * cos(z);

FMatrixAdd[5] = -sin(x);

FMatrixAdd[6] = -(cos(z) * sin(y)) + (sin(x) * sin(z) * cos(y));

FMatrixAdd[7] = (sin(z) * sin(y)) + (sin(x) * cos(z) * cos(y));

FMatrixAdd[8] = cos(x) * cos(y);


// Dot product of New Add Matrix and Temp Matrix to give new Camera Matrix

for( i = 0; i < 3; i++ )
	for( j = 0; j < 3; j++ )
		for( k = 0; k < 3; k++ )
			FMatrix[j*3+i] += (float)( FMatrixAdd[j*3+k] * FMatrixTemp[k*3+i] );



// Normalise X column of Camera Matrix

divide = sqrt( (FMatrix[0]*FMatrix[0]) + (FMatrix[3]*FMatrix[3]) + (FMatrix[6]*FMatrix[6]) );
if( divide != 0 )
{	
	FMatrix[0] = (float)(FMatrix[0] / divide);
	FMatrix[3] = (float)(FMatrix[3] / divide);
	FMatrix[6] = (float)(FMatrix[6] / divide);
}

// Normalise Y column of Camera Matrix

divide = sqrt( (FMatrix[1]*FMatrix[1]) + (FMatrix[4]*FMatrix[4]) + (FMatrix[7]*FMatrix[7]) );
if( divide != 0 )
{	
	FMatrix[1] = (float)(FMatrix[1] / divide);
	FMatrix[4] = (float)(FMatrix[4] / divide);
	FMatrix[7] = (float)(FMatrix[7] / divide);
}

// Normalise Z column of Camera Matrix

divide = sqrt( (FMatrix[2]*FMatrix[2]) + (FMatrix[5]*FMatrix[5]) + (FMatrix[8]*FMatrix[8]) );
if( divide != 0 )
{	
	FMatrix[2] = (float)(FMatrix[2] / divide);
	FMatrix[5] = (float)(FMatrix[5] / divide);
	FMatrix[8] = (float)(FMatrix[8] / divide);
}

}



/***********************************************************************************************/

void FMakeMatrixObjectCamera(float *cam, float *FMatrix, float *result)
{
	int i, j, k;

	for( i = 0; i < 3; i++ )
		for( j = 0; j < 3; j++ )
			result[j*3+i] =  0;

	for( i = 0; i < 3; i++ )
		for( j = 0; j < 3; j++ )
			for( k = 0; k < 3; k++ )
				result[j*3+i] += (float)( FMatrix[j*3+k] * cam[i*3+k] );


}

/***********************************************************************************************/

void MakeLightSource( float *LightPos, float *LookAt, float *FMatrix, float *Dir )
{

float Normalise;
float LightDirX, LightDirY, LightDirZ;

LightDirX = (float)-LookAt[0] + LightPos[0];
LightDirY = (float)-LookAt[1] + LightPos[1];
LightDirZ = (float)-LookAt[2] + LightPos[2];

Normalise = (float)sqrt((LightDirX*LightDirX)+(LightDirY*LightDirY)+(LightDirZ*LightDirZ));

LightDirX = LightDirX / Normalise;
LightDirY = LightDirY / Normalise;
LightDirZ = LightDirZ / Normalise;

Dir[0] = LightDirX*FMatrix[0] + LightDirY*FMatrix[1] + LightDirZ*FMatrix[2];
Dir[1] = LightDirX*FMatrix[3] + LightDirY*FMatrix[4] + LightDirZ*FMatrix[5];
Dir[2] = LightDirX*FMatrix[6] + LightDirY*FMatrix[7] + LightDirZ*FMatrix[8];

}



/***********************************************************************************************/

void FInvRotation( float X, float Y, float Z, float *FMatrix, float *result )
{
result[0] = (float)X*FMatrixCamera[0] + (float)Y*FMatrixCamera[1] + (float)Z*FMatrixCamera[2];
result[1] = (float)X*FMatrixCamera[3] + (float)Y*FMatrixCamera[4] + (float)Z*FMatrixCamera[5];
result[2] = (float)X*FMatrixCamera[6] + (float)Y*FMatrixCamera[7] + (float)Z*FMatrixCamera[8];
}

/***********************************************************************************************/

void FRotation( float  X, float Y, float Z, float *FMatrix, float *result )
{
result[0] = (float)X*FMatrix[0] + (float)Y*FMatrix[3] + (float)Z*FMatrix[6];
result[1] = (float)X*FMatrix[1] + (float)Y*FMatrix[4] + (float)Z*FMatrix[7];
result[2] = (float)X*FMatrix[2] + (float)Y*FMatrix[5] + (float)Z*FMatrix[8];
}

/***********************************************************************************************/

char CameraLook( float *CameraPos, float *LookAt, float *FMatrix)
{
	double Dir[3], Right[3], Up[3], Mod;

	Dir[0] = LookAt[0] - CameraPos[0];
	Dir[1] = LookAt[1] - CameraPos[1];
	Dir[2] = LookAt[2] - CameraPos[2];

	Mod = sqrt((Dir[0]*Dir[0])+(Dir[1]*Dir[1])+(Dir[2]*Dir[2]));
	
	if( Mod == 0)
		Mod = 1;
	
	Dir[0] = Dir[0] / Mod;
	Dir[1] = Dir[1] / Mod;
	Dir[2] = Dir[2] / Mod;

	Right[0] = Dir[2];
	Right[1] = 0;
	Right[2] = -Dir[0];

	Up[0] = (Dir[1]*Right[2]) - (Right[1]*Dir[2]);
	Up[1] = (Dir[2]*Right[0]) - (Right[2]*Dir[0]);
	Up[2] = (Dir[0]*Right[1]) - (Right[0]*Dir[1]);

	Mod = sqrt((Right[0]*Right[0])+(Right[1]*Right[1])+(Right[2]*Right[2]));
	
	if( Mod == 0)
		Mod = 1;
	
		Right[0] = Right[0] / Mod;
		Right[1] = Right[1] / Mod;
		Right[2] = Right[2] / Mod;
	
	Mod = sqrt((Up[0]*Up[0])+(Up[1]*Up[1])+(Up[2]*Up[2]));
	
	if( Mod == 0)
		Mod = 1;
	
		Up[0] = Up[0] / Mod;
		Up[1] = Up[1] / Mod;
		Up[2] = Up[2] / Mod;

	FMatrix[0] = (float)Right[0];
	FMatrix[1] = (float)Right[1];
	FMatrix[2] = (float)Right[2];
	FMatrix[3] = (float)Up[0];
	FMatrix[4] = (float)Up[1];
	FMatrix[5] = (float)Up[2];
	FMatrix[6] = (float)Dir[0];
	FMatrix[7] = (float)Dir[1];
	FMatrix[8] = (float)Dir[2];



	return TRUE;


}

// ***********************************************************************************************************************

char UpdateGame(void)
{

    
	QueryPerformanceFrequency( &TimerFreq );
	TimerLast.LowPart =  TimerThis.LowPart;
	QueryPerformanceCounter( &TimerThis );

	TimeTaken = TimerThis.LowPart - TimerLast.LowPart;

	if( TimeTaken == 0 )
		TimeTaken = 1;

	TimeFactor = TimeTaken/((float)TimerFreq.LowPart/72.0f); // Now Syncing at 72 fps 


//	SillyGame();


//	char buff[256];
//	wsprintf(buff, "Freq %d Time Taken: %d Time Ratio %d:100 " , TimerFreq.LowPart, TimeTaken, (long)(TimeFactor*100) );
//	DumpText(0, 128, 8, 16, 0x808080, buff);


	// Add to Camera Pos

//   short ShiftIt = 1;
//
//	if( Keys[42] )
//		ShiftIt = 4;
//
//    if( Keys[54] )
//		ShiftIt = 4;
//
//	if( Keys[199] )
//		CameraPos[1] -=(5*ShiftIt);
//	
//	if( Keys[207] )
//		CameraPos[1] += (5*ShiftIt);

	return TRUE;

}


// ***********************************************************************************************************************

char SillyGame(void)
{
	float FNew[3];


	// Add to Object Pos

	if( Keys[200] )
		Speed += 0.16f*(float)TimeFactor;
	else if( Keys[208] )
		Speed -= 0.16f*(float)TimeFactor;
	else
	{
		if( Speed > 0 )
			Speed -= 0.08f*(float)TimeFactor;
		if( Speed < 0 )
			Speed += 0.08f*(float)TimeFactor;
	}


	if( Speed < -4.0f )
		Speed = -4.0f;


	if( Speed > 16.0f )
		Speed = 16.0f;

		FRotation( 0, 0, (float)TimeFactor*Speed , FMatrixObject, FNew  );
		
		SpeedOld[0] = FNew[0];
		SpeedOld[1] = FNew[1];
		SpeedOld[2] = FNew[2];


//		if( Collision[ (((int)((ObjectPos[2]+62)/-125)+8)*20) +
//						(int)((ObjectPos[0]+SpeedOld[0]-50)/100) +10
//						] != 99 )
		{
		ObjectPos[0] += SpeedOld[0];
		}

//		if( Collision[ (((int)((ObjectPos[2]+SpeedOld[2]+62)/-125)+8)*20) +
//						(int)((ObjectPos[0]-50)/100) +10
//						] != 99 )
		{
		ObjectPos[2] += SpeedOld[2];
		}



	if( StartSound == 0 )
	{
	 PlaySample( 0, DSBVOLUME_MAX, 44100, DSBPAN_CENTER, 1 );
	 GaySHandle = PlaySample( 2, -2500, 8000, DSBPAN_CENTER, 1 );
	 StartSound = 1;
	}

	if(  StartSound == 1  )
		ChangeSample( 2, GaySHandle, DSBVOLUME_MAX, (long)6000+abs((long)(Speed*5000)), DSBPAN_CENTER );
	


// Add to Object Matrix 

	if( Keys[205] )
		FBuildMatrix( 0, (float)(-0.01f*TimeFactor*Speed)/3, 0, FMatrixObject );

	if( Keys[203] )
		FBuildMatrix( 0, (float)(0.01f*TimeFactor*Speed)/3, 0, FMatrixObject );


//	CameraPos[0] = ObjectPos[0];
//	CameraPos[2] = ObjectPos[2]-500;


	char buff[256];
	wsprintf(buff, "Freq %d  Time Taken: %d  Time Ratio %d:100" ,
		TimerFreq.LowPart, TimeTaken, (long)(TimeFactor*100) );




	DumpText(160, 0, 8, 16, 0x808080, buff);




	return TRUE;
}

///////////////////////////////
// home camera behind object //
///////////////////////////////

void HomeCamera(float *cpos, float *cmatrix, float *opos, float *omatrix)
	{
	float out[3], mat[9];
	float dx, dy, dz, dist, posdist, mul;

// set position / distance behind car

	posdist = (float)sqrt((double)(CameraHomePos[0] * CameraHomePos[0] + CameraHomePos[1] * CameraHomePos[1] + CameraHomePos[2] * CameraHomePos[2]));

// get world pos of dest

	mat[0] = omatrix[8];
	mat[1] = 0;
	mat[2] = -omatrix[6];

	if (!mat[0] && !mat[2]) mat[0] = 1;

	Normalize(&mat[0]);

	mat[3] = 0;
	mat[4] = 1;
	mat[5] = 0;

	CrossProduct(&mat[0], &mat[3], &mat[6]);

	RotVector(mat, CameraHomePos, out);
	out[0] += opos[0];
	out[1] += opos[1];
	out[2] += opos[2];

// home to

	dx = out[0] - cpos[0];
	dy = out[1] - cpos[1];
	dz = out[2] - cpos[2];

	if ((mul = TimeFactor / 16) > 1) mul = 1;
	cpos[0] += dx * mul;
	if ((mul = TimeFactor / 4) > 1) mul = 1;
	cpos[1] += dy * mul;
	if ((mul = TimeFactor / 16) > 1) mul = 1;
	cpos[2] += dz * mul;

// normalize distance from car

	dx = cpos[0] - opos[0];
	dy = cpos[1] - opos[1];
	dz = cpos[2] - opos[2];

	dist = (float)sqrt((float)(dx * dx + dy * dy + dz * dz));

	cpos[0] = opos[0] + (dx * posdist / dist);
	cpos[1] = opos[1] + (dy * posdist / dist);
	cpos[2] = opos[2] + (dz * posdist / dist);

// look at car

	SetVector(out, opos[0], opos[1] + CameraHomeHeight, opos[2]);
	CameraLook(cpos, out, cmatrix);
	}

///////////////////////
// put camera in car //
///////////////////////

void CarCamera(float *cpos, float *cmatrix, float *opos, float *omatrix)
	{
	float vec[3];

// apply car matrix to an offset vector to get camera pos

	SetVector(vec, 0, -24, 0);
	RotVector(omatrix, vec, cpos);
	AddVector(cpos, opos, cpos);

// copy car matrix

	memcpy(cmatrix, omatrix, sizeof(float) * 9);
	}
