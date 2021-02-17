/*********************************************************************************************
 *
 * CARCONV.c
 *
 * Copyright (c) Probe Entertainment 1998
 * 
 * Contents:
 *			Converts CARINFO.TXT text file into 
 *
 *********************************************************************************************
 *
 * 10/06/98 Matt Taylor
 *  File inception.
 *
 * 09/11/98 Matt Taylor
 *	v1.3 - updated, added DownForceMod
 *
 *********************************************************************************************/

#define _CARCONV
#define _PC

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#define MAX_NUM_PLAYERS 12
#define READ_MAX_WORDLEN	256
#define READ_COMMENT_CHAR	';'
#define MAX_SPLIT_TIMES	10


#include "typedefs.h"
#include "geom.h"
#include "model.h"
#include "particle.h"
#include "newcoll.h"
#include "body.h"
#include "wheel.h"
#include "aerial.h"
#include "car.h"

#undef EOF
#define EOF ((char)(-1))

//
// Structures
//

//
// Global variables
//

char		TXTfname[256];
char		CDTfname[256];
FILE		*TXTfile;
FILE		*CDTfile;
long		Anal;
long		n64;
long		Verbose;

// Variables ripped from ReadInit.c

long		NCarTypes;
CAR_INFO	*CarInfo;

int *CarList;
int CarListSize = 0;

int WheelList[CAR_NWHEELS];
int WheelListSize = 0;

int SpringList[CAR_NWHEELS];
int SpringListSize = 0;

int AxleList[CAR_NWHEELS];
int AxleListSize = 0;

int PinList[CAR_NWHEELS];
int PinListSize = 0;

int ModelList[MAX_CAR_MODEL_TYPES];
int ModelListSize = 0;


//
// Function prototypes
//

void ParseCmdLine(int argc, char *argv[]);
long EndConvertL(long value);
short EndConvertS(short value);

void CopyMat(MAT *src, MAT *dest);
bool ReadWord(char *buf, FILE *fp);
bool ReadFileName(char *name, FILE *fp);
bool ReadInt(int *n, FILE *fp);
bool ReadBool(bool *b, FILE *fp);
bool ReadReal(REAL *r, FILE *fp);
bool ReadVec(VEC *vec, FILE *fp);
bool ReadMat(MAT *mat, FILE *fp);
bool StringToUpper(char *string);
bool Compare (char *word, char *token);
int ReadNumberList(int *numList, int maxNum, FILE *fp);
bool ReadAllCarInfo(void);
bool ReadCarInfo(FILE *fp);
bool ReadBodyInfo(FILE *fp);
bool ReadWheelInfo(FILE *fp);
bool ReadSpringInfo(FILE *fp);
bool ReadAxleInfo(FILE *fp);
bool ReadPinInfo(FILE *fp);
bool ReadSpinnerInfo(FILE *fp);
bool ReadAerialInfo(FILE *fp);
void UnknownWordMessage(char *word);
void ShowErrorMessage(char *word);
void InvalidVariable(char *object);
void InvalidNumberList(char *object);

void fputfloat(float value, FILE *file);
void fputshort(short value, FILE *file);
void fputlong(long value, FILE *file);
void fputreal(float value, FILE *file);

//======
// CODE
//======

void main(int argc, char *argv[])
{
	long	ii, jj, kk;
	long	size;

	printf("\nCARCONV V1.3 - M. Taylor - 09/11/98\n\n");
	if (argc == 1)
	{
		printf("Usage: CARCONV [switches] <src> <dest>\n\n");
		printf("Switches:\n");
		printf("      /a - Anal mode. All unknown words or values are treated as errors.\n");
		printf("      /n - Output N64 data.\n");
		printf("      /v - Verbose output.\n");
		exit(0);
	}

	ParseCmdLine(argc, argv);

	ReadAllCarInfo();

	if (Verbose)
	{
		printf("Number of car types = %d\n", NCarTypes);
		if (n64)
		{
			printf("Size of N64 CAR_INFO structure = %d bytes\n", sizeof(CAR_INFO) - ((MAX_CAR_MODEL_TYPES + 2) * MAX_CAR_FILENAME) + (sizeof(long) * (MAX_CAR_MODEL_TYPES + 1)));
			printf("Size of N64 data without file IDs = %d bytes\n", sizeof(CAR_INFO) - ((MAX_CAR_MODEL_TYPES + 2) * MAX_CAR_FILENAME));
		}
		else
		{
			printf("Size of PSX CAR_INFO structure = %d bytes\n", sizeof(CAR_INFO));
		}
	}

	if (n64)
	{
		fputlong(NCarTypes, CDTfile);								// Write number of cars in file
		size = sizeof(CAR_INFO) - ((MAX_CAR_MODEL_TYPES + 2) * MAX_CAR_FILENAME) + (sizeof(long) * (MAX_CAR_MODEL_TYPES + 1));
		fputlong(size, CDTfile);									// Write sizeof CAR_INFO structure
		size = sizeof(CAR_INFO) - ((MAX_CAR_MODEL_TYPES + 2) * MAX_CAR_FILENAME);
		for (ii = 0; ii < NCarTypes; ii++)
		{
			fputlong(CarInfo[ii].EnvRGB, CDTfile);
			fwrite(CarInfo[ii].Name, 1, CAR_NAMELEN, CDTfile);

			fputfloat(CarInfo[ii].SteerRate, CDTfile);
			fputfloat(CarInfo[ii].SteerModifier, CDTfile);
			fputfloat(CarInfo[ii].EngineRate, CDTfile);
			fputfloat(CarInfo[ii].TopSpeed, CDTfile);
			fputfloat(CarInfo[ii].MaxRevs, CDTfile);
			fputfloat(CarInfo[ii].DownForceMod, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputfloat(CarInfo[ii].CoMOffset.v[kk], CDTfile);
			}
			fputlong(CarInfo[ii].AllowedBestTime, CDTfile);
			fputlong(CarInfo[ii].Selectable, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputfloat(CarInfo[ii].WeaponOffset.v[kk], CDTfile);
			}
			
			// Write BODY_INFO
			fputlong(CarInfo[ii].Body.ModelNum, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputfloat(CarInfo[ii].Body.Offset.v[kk], CDTfile);
			}
			fputfloat(CarInfo[ii].Body.Mass, CDTfile);
			for (kk = 0; kk < 9; kk++)
			{
				fputfloat(CarInfo[ii].Body.Inertia.m[kk], CDTfile);
			}
			fputfloat(CarInfo[ii].Body.Gravity, CDTfile);
			fputfloat(CarInfo[ii].Body.Hardness, CDTfile);
			fputfloat(CarInfo[ii].Body.Resistance, CDTfile);
			fputfloat(CarInfo[ii].Body.AngResistance, CDTfile);
			fputfloat(CarInfo[ii].Body.ResModifier, CDTfile);
			fputfloat(CarInfo[ii].Body.Grip, CDTfile);
			fputfloat(CarInfo[ii].Body.StaticFriction, CDTfile);
			fputfloat(CarInfo[ii].Body.KineticFriction, CDTfile);

			// Write out WHEEL_INFOs
			for (jj = 0; jj < 4; jj++)
			{
				fputlong(CarInfo[ii].Wheel[jj].ModelNum, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputfloat(CarInfo[ii].Wheel[jj].Offset1.v[kk], CDTfile);
				}
				for (kk = 0; kk < 3; kk++)
				{
					fputfloat(CarInfo[ii].Wheel[jj].Offset2.v[kk], CDTfile);
				}
				fputfloat(CarInfo[ii].Wheel[jj].Radius, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].Mass, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].Gravity, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].Grip, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].StaticFriction, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].KineticFriction, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].AxleFriction, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].SteerRatio, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].EngineRatio, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].MaxPos, CDTfile);
				fputfloat(CarInfo[ii].Wheel[jj].SkidWidth, CDTfile);
				fputlong(CarInfo[ii].Wheel[jj].IsPresent, CDTfile);
				fputlong(CarInfo[ii].Wheel[jj].IsTurnable, CDTfile);
				fputlong(CarInfo[ii].Wheel[jj].IsPowered, CDTfile);
			}				

			// Write out SPRING_INFO
			for (jj = 0; jj < 4; jj++)
			{
				fputlong(CarInfo[ii].Spring[jj].ModelNum, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputfloat(CarInfo[ii].Spring[jj].Offset.v[kk], CDTfile);
				}
				fputfloat(CarInfo[ii].Spring[jj].Length, CDTfile);
				fputfloat(CarInfo[ii].Spring[jj].Stiffness, CDTfile);
				fputfloat(CarInfo[ii].Spring[jj].Damping, CDTfile);
				fputfloat(CarInfo[ii].Spring[jj].Restitution, CDTfile);
			}

			// Write out AXLE_INFO
			for (jj = 0; jj < 4; jj++)
			{
				fputlong(CarInfo[ii].Axle[jj].ModelNum, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputfloat(CarInfo[ii].Axle[jj].Offset.v[kk], CDTfile);
				}
				fputfloat(CarInfo[ii].Axle[jj].Length, CDTfile);
			}

			// Write out PIN_INFO
			for (jj = 0; jj < 4; jj++)
			{
				fputlong(CarInfo[ii].Pin[jj].ModelNum, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputfloat(CarInfo[ii].Pin[jj].Offset.v[kk], CDTfile);
				}
				fputfloat(CarInfo[ii].Pin[jj].Length, CDTfile);
			}

			// Write out SPINNER_INFO
			fputlong(CarInfo[ii].Spinner.ModelNum, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputfloat(CarInfo[ii].Spinner.Offset.v[kk], CDTfile);
			}
			for (kk = 0; kk < 3; kk++)
			{
				fputfloat(CarInfo[ii].Spinner.Axis.v[kk], CDTfile);
			}
			fputfloat(CarInfo[ii].Spinner.AngVel, CDTfile);

			// Write out AERIAL_INFO
			fputlong(CarInfo[ii].Aerial.SecModelNum, CDTfile);
			fputlong(CarInfo[ii].Aerial.TopModelNum, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputfloat(CarInfo[ii].Aerial.Offset.v[kk], CDTfile);
			}
			for (kk = 0; kk < 3; kk++)
			{
				fputfloat(CarInfo[ii].Aerial.Direction.v[kk], CDTfile);
			}
			fputfloat(CarInfo[ii].Aerial.SecLen, CDTfile);
			fputfloat(CarInfo[ii].Aerial.Stiffness, CDTfile);
			fputfloat(CarInfo[ii].Aerial.Damping, CDTfile);
		}
	}
	else
	{
		fwrite((char *)&NCarTypes, sizeof(long), 1, CDTfile); 		// Write number of cars in file
		size = sizeof(CAR_INFO) - ((MAX_CAR_MODEL_TYPES + 2) * MAX_CAR_FILENAME) + (sizeof(long) * (MAX_CAR_MODEL_TYPES + 1));
		fwrite((char *)&size, sizeof(long), 1, CDTfile);			// Write sizeof CAR_INFO structure
		size = sizeof(CAR_INFO) - ((MAX_CAR_MODEL_TYPES + 2) * MAX_CAR_FILENAME);
		for (ii = 0; ii < NCarTypes; ii++)
		{
			fwrite((char *)&CarInfo[ii].EnvRGB, sizeof(long), 1, CDTfile);
			fwrite(CarInfo[ii].Name, 1, CAR_NAMELEN, CDTfile);

			fputreal(CarInfo[ii].SteerRate, CDTfile);
			fputreal(CarInfo[ii].SteerModifier, CDTfile);
			fputreal(CarInfo[ii].EngineRate, CDTfile);
			fputreal(CarInfo[ii].TopSpeed, CDTfile);
			fputreal(CarInfo[ii].MaxRevs, CDTfile);
			fputreal(CarInfo[ii].DownForceMod, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputreal(CarInfo[ii].CoMOffset.v[kk], CDTfile);
			}
			fputreal(CarInfo[ii].AllowedBestTime, CDTfile);
			fputreal(CarInfo[ii].Selectable, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputreal(CarInfo[ii].WeaponOffset.v[kk], CDTfile);
			}
				
			// Write BODY_INFO
			fwrite((char *)&CarInfo[ii].Body.ModelNum, sizeof(long), 1, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputreal(CarInfo[ii].Body.Offset.v[kk], CDTfile);
			}
			fputreal(CarInfo[ii].Body.Mass, CDTfile);
			for (kk = 0; kk < 9; kk++)
			{
				fputreal(CarInfo[ii].Body.Inertia.m[kk], CDTfile);
			}
			fputreal(CarInfo[ii].Body.Gravity, CDTfile);
			fputreal(CarInfo[ii].Body.Hardness, CDTfile);
			fputreal(CarInfo[ii].Body.Resistance, CDTfile);
			fputreal(CarInfo[ii].Body.AngResistance, CDTfile);
			fputreal(CarInfo[ii].Body.ResModifier, CDTfile);
			fputreal(CarInfo[ii].Body.Grip, CDTfile);
			fputreal(CarInfo[ii].Body.StaticFriction, CDTfile);
			fputreal(CarInfo[ii].Body.KineticFriction, CDTfile);

			// Write out WHEEL_INFOs
			for (jj = 0; jj < 4; jj++)
			{
				fwrite((char *)&CarInfo[ii].Wheel[jj].ModelNum, sizeof(long), 1, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputreal(CarInfo[ii].Wheel[jj].Offset1.v[kk], CDTfile);
				}
				for (kk = 0; kk < 3; kk++)
				{
					fputreal(CarInfo[ii].Wheel[jj].Offset2.v[kk], CDTfile);
				}
				fputreal(CarInfo[ii].Wheel[jj].Radius, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].Mass, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].Gravity, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].Grip, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].StaticFriction, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].KineticFriction, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].AxleFriction, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].SteerRatio, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].EngineRatio, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].MaxPos, CDTfile);
				fputreal(CarInfo[ii].Wheel[jj].SkidWidth, CDTfile);
				fputlong(CarInfo[ii].Wheel[jj].IsPresent, CDTfile);
				fputlong(CarInfo[ii].Wheel[jj].IsTurnable, CDTfile);
				fputlong(CarInfo[ii].Wheel[jj].IsPowered, CDTfile);
			}				

			// Write out SPRING_INFO
			for (jj = 0; jj < 4; jj++)
			{
				fwrite((char *)&CarInfo[ii].Spring[jj].ModelNum, sizeof(long), 1, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputreal(CarInfo[ii].Spring[jj].Offset.v[kk], CDTfile);
				}
				fputreal(CarInfo[ii].Spring[jj].Length, CDTfile);
				fputreal(CarInfo[ii].Spring[jj].Stiffness, CDTfile);
				fputreal(CarInfo[ii].Spring[jj].Damping, CDTfile);
				fputreal(CarInfo[ii].Spring[jj].Restitution, CDTfile);
			}

			// Write out AXLE_INFO
			for (jj = 0; jj < 4; jj++)
			{
				fwrite((char *)&CarInfo[ii].Axle[jj].ModelNum, sizeof(long), 1, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputreal(CarInfo[ii].Axle[jj].Offset.v[kk], CDTfile);
				}
				fputreal(CarInfo[ii].Axle[jj].Length, CDTfile);
			}

			// Write out PIN_INFO
			for (jj = 0; jj < 4; jj++)
			{
				fwrite((char *)&CarInfo[ii].Pin[jj].ModelNum, sizeof(long), 1, CDTfile);
				for (kk = 0; kk < 3; kk++)
				{
					fputreal(CarInfo[ii].Pin[jj].Offset.v[kk], CDTfile);
				}
				fputreal(CarInfo[ii].Pin[jj].Length, CDTfile);
			}

			// Write out SPINNER_INFO
			fwrite((char *)&CarInfo[ii].Spinner.ModelNum, sizeof(long), 1, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputreal(CarInfo[ii].Spinner.Offset.v[kk], CDTfile);
			}
			for (kk = 0; kk < 3; kk++)
			{
				fputreal(CarInfo[ii].Spinner.Axis.v[kk], CDTfile);
			}
			fputreal(CarInfo[ii].Spinner.AngVel, CDTfile);

			// Write out AERIAL_INFO
			fwrite((char *)&CarInfo[ii].Aerial.SecModelNum, sizeof(long), 1, CDTfile);
			fwrite((char *)&CarInfo[ii].Aerial.TopModelNum, sizeof(long), 1, CDTfile);
			for (kk = 0; kk < 3; kk++)
			{
				fputreal(CarInfo[ii].Aerial.Offset.v[kk], CDTfile);
			}
			for (kk = 0; kk < 3; kk++)
			{
				fputreal(CarInfo[ii].Aerial.Direction.v[kk], CDTfile);
			}
			fputreal(CarInfo[ii].Aerial.SecLen, CDTfile);
			fputreal(CarInfo[ii].Aerial.Stiffness, CDTfile);
			fputreal(CarInfo[ii].Aerial.Damping, CDTfile);
		}
	}
	fclose(TXTfile);
	fclose(CDTfile);
	exit(0);
}


//
// ParseCmdLine
//

void ParseCmdLine(int argc, char *argv[])
{
	int 	ii;
	int		FoundFname1;
	int		FoundFname2;

	FoundFname1 = FALSE;
	FoundFname2 = FALSE;
	Anal = FALSE;
	n64 = FALSE;
	Verbose = FALSE;

	for (ii = 1; ii < argc; ii++)
	{
		if (argv[ii][0] == '/')
		{				 
			if (argv[ii][2] != 0)
			{
				printf("\nERROR: Invalid switch - %s\n", argv[ii]);
				exit(1);
			}

			switch (argv[ii][1])
			{
				case 'a':
				case 'A':
				Anal = TRUE;
				break;
				
				case 'n':
				case 'N':
				n64 = TRUE;
				break;

				case 'v':
				case 'V':
				Verbose = TRUE;
				break;

				default:
				printf("\nERROR: Invalid switch - %s\n", argv[ii]);
				break;
			}
		}
		else
		{
			if (FoundFname1 != TRUE)
			{
				strcpy(TXTfname, argv[ii]);
				FoundFname1 = TRUE;
			}
			else if (FoundFname2 != TRUE)
			{
				strcpy(CDTfname, argv[ii]);
				FoundFname2 = TRUE;
			}
			else
			{
				printf("\nERROR: Only two filenames may be specified.\n");
				exit(1);
			}
		}
	}

	if (FoundFname1 == FALSE)
	{
		printf("\nERROR: You must specify a source filename.\n");
		exit(1);
	}

	if (FoundFname2 == FALSE)
	{
		printf("\nERROR: You must specify a destination filename.\n");
		exit(1);
	}
}


//============================================================================================

void CopyMat(MAT *src, MAT *dest) 
{
	dest->m[XX] = src->m[XX];
	dest->m[XY] = src->m[XY];
	dest->m[XZ] = src->m[XZ];

	dest->m[YX] = src->m[YX];
	dest->m[YY] = src->m[YY];
	dest->m[YZ] = src->m[YZ];

	dest->m[ZX] = src->m[ZX];
	dest->m[ZY] = src->m[ZY];
	dest->m[ZZ] = src->m[ZZ];
}

/////////////////////////////////////////////////////////////////////
//
// ReadWord: get the next word in buf, return FALSE if no more words
//
/////////////////////////////////////////////////////////////////////

bool ReadWord(char *buf, FILE *fp)
{
	char *pChar = buf;

	// Skip white space and commas
	while (isspace(*pChar = fgetc(fp)) || (*pChar == ',')) {
	}

	// Check for comments
	while (*pChar == READ_COMMENT_CHAR) {
		// Go to next line
		while ((*pChar = fgetc(fp)) != '\n' && (*pChar != EOF)) {
		}

		// Skip the white space again
		if (*pChar != EOF) {
			while (isspace(*pChar = fgetc(fp))) {
			}
		}
	}

	// Check for EOF
	if (*pChar == EOF) {
		buf[0] = NULL;
		return FALSE;
	}

	// Read in chars up to next white space, brace or comma
	while (!isspace(*++pChar = fgetc(fp)) && 
		(*pChar != EOF) && 
		(*pChar != '{') &&
		(*pChar != '}') &&
		(*pChar != ',')) 
	{
	}
	ungetc(*pChar, fp);
	*pChar = '\0';

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadFileName:
//
/////////////////////////////////////////////////////////////////////

bool ReadFileName(char *name, FILE *fp)
{
	int iCh, nameLen;
	char delimiter;
	char *endOfName;

	// Read next word
	if (!ReadWord(name, fp)) {
		ShowErrorMessage("Could not find file name");
		return FALSE;
	}

	// If the first letter is " or ' read words until the last letter
	// is also " or '
	delimiter = name[0];
	endOfName = name + strlen(name);
	while (((delimiter == '\"') || (delimiter == '\'')) && (*(endOfName-1) != delimiter)) {
		while (isspace(*endOfName = fgetc(fp))) {
			endOfName++;
		}
		ungetc(*endOfName, fp);
		if (!ReadWord(endOfName, fp)) {
			ShowErrorMessage("Could not find file name");
			return FALSE;
		}
		endOfName = name + strlen(name);
	}
	
	// Remove the quotes
	if (delimiter == '\'' || delimiter == '\"') {
		nameLen = strlen(name);
		for (iCh = 0; iCh < nameLen - 2; iCh++) {
			name[iCh] = name[iCh+1];
		}
		name[nameLen - 2] = '\0';
	}

	// Check for NULL keywords
	if (Compare(name, "NULL") || Compare(name, "NONE") || Compare(name, "0")) {
		name[0] = '\0';
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadInt:
//
/////////////////////////////////////////////////////////////////////

bool ReadInt(int *n, FILE *fp)
{
	if (fscanf(fp, "%d", n) == EOF) {
		ShowErrorMessage("Could not read integer");
		return FALSE;
	} else {
		return TRUE;
	}
}


/////////////////////////////////////////////////////////////////////
//
// ReadBool:
//
/////////////////////////////////////////////////////////////////////

bool ReadBool(bool *b, FILE *fp)
{
	char word[READ_MAX_WORDLEN];

	*b = FALSE;

	if (!ReadWord(word, fp)) {
		return FALSE;
	}

	if (Compare(word, "TRUE") || Compare(word, "1") || Compare(word, "YES")) {
		*b = TRUE;
	}
	else if (Compare(word, "FALSE") || Compare(word, "0") || Compare(word, "NO")) {
		*b = FALSE;
	}
	else {
		return FALSE;
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadReal:
//
/////////////////////////////////////////////////////////////////////

#if (defined(_PC) || defined(_N64)) 
bool ReadReal(REAL *r, FILE *fp)
{
	/*char ch;
	// Skip white space and commas
	while (isspace(ch = fgetc(fp)) || (ch == ',')) {
	};

	if (fscanf(fp, "%f", r) == EOF) {
		return FALSE;
	} else {
		return TRUE;
	}*/
	char word[READ_MAX_WORDLEN];

	if (!ReadWord(word, fp)) {
		return FALSE;
	} else {
		*r = (REAL)atof(word);
		return TRUE;
	}
}
#else
bool ReadReal(REAL *r, FILE *fp)
{
	/*char ch;
	// Skip white space and commas
	while (isspace(ch = fgetc(fp)) || (ch == ',')) {
	};

	if (fscanf(fp, "[,]%d", r) == EOF) {
		return FALSE;
	} else {
		return TRUE;
	}*/
	char word[READ_MAX_WORDLEN];

	if (!ReadWord(word, fp)) {
		return FALSE;
	} else {
		*r = ONE * atol(word);
		return TRUE;
	}

}
#endif


/////////////////////////////////////////////////////////////////////
//
// ReadVec:
//
/////////////////////////////////////////////////////////////////////

bool ReadVec(VEC *vec, FILE *fp)
{
	int iR;

	for (iR = 0; iR < 3; iR++) {
		if (!ReadReal(&vec->v[iR], fp)) {
			ShowErrorMessage("Could not read vector");
			return FALSE;
		}
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadMat:
//
/////////////////////////////////////////////////////////////////////

bool ReadMat(MAT *mat, FILE *fp)
{
	int iV;

	for (iV = 0; iV < 3; iV++) {
		if (!ReadVec(&mat->mv[iV], fp)) {
			ShowErrorMessage("Could not read Matrix");
			return FALSE;
		}
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// ToUpper: Convert a string to upper case
//
/////////////////////////////////////////////////////////////////////

bool StringToUpper(char *string)
{
	int iChar;
	int sLen = strlen(string);

	if (sLen > READ_MAX_WORDLEN) return FALSE;
	
	for (iChar = 0; iChar < sLen; ++iChar) {
		string[iChar] = toupper(string[iChar]);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// Compare: compare two strings without checking case
//
/////////////////////////////////////////////////////////////////////

/*bool Compare(char *word, char *token)
{
	// Convert strings to upper case
	if (!StringToUpper(word) || !StringToUpper(token)) {
		return FALSE;
	}

	// Compare them
	if (strcmp(word, token) != 0) {
		return FALSE;
	}

	return TRUE;
}*/

bool Compare (char *word, char *token) 
{
	unsigned int wordLen;
	unsigned int iChar;

	wordLen = strlen(word);

	if (wordLen != strlen(token)) return FALSE;

	for (iChar = 0; iChar < wordLen; iChar++) {
		if (toupper(word[iChar]) != toupper(token[iChar])) {
			return FALSE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// ReadNumberList: Translate a comma, hyphen number list
//
/////////////////////////////////////////////////////////////////////

int ReadNumberList(int *numList, int maxNum, FILE *fp)
{
	int iN = 0;
	int startNum, endNum, tmpNum;
	bool doneList;
	char ch;

	// Skip white space
	while (isspace(ch = fgetc(fp))) {
	}
	ungetc(ch, fp);

	// Read the first number
	if ((!ReadInt(&numList[iN++], fp))) {
		return 0;
	}

	if (numList[0] < 0) {
		return -1;
	}

	// Read in the rest of the numbers
	doneList = FALSE;
	while (!doneList) {

		// Make sure there is no overflow
		if (iN > maxNum) {
			return -1;
		}

		// skip white space
		while (isspace(ch = fgetc(fp))){
		}

		// parse the list
		switch (ch) {

		// comma separated list
		case ',':
			if (!ReadInt(&numList[iN++], fp)) {
				return iN - 1;
			}
			break;

		// dash separated list
		case '-':
			if (!ReadInt(&endNum, fp)) {
				return -1;
			} else {
				if (endNum < 0 || endNum >= maxNum) return -1;
				startNum = numList[--iN];
				if (endNum < startNum) {
					tmpNum = startNum;
					startNum = endNum;
					endNum = tmpNum;
				}
				while(startNum <= endNum) {
					numList[iN++] = startNum++;
				}
			}
			break;
		
		// end of list
		default:
			ungetc(ch, fp);
			doneList = TRUE;
			break;
		
		}
	}
	return iN;
}


/////////////////////////////////////////////////////////////////////
//
// ReadAllCarInfo: 
//
/////////////////////////////////////////////////////////////////////

bool ReadAllCarInfo(void)
{
	char word[READ_MAX_WORDLEN];
	FILE *fp;

	int tInt;

	TXTfile = fopen(TXTfname, "rb");										// Open the input file
	if (TXTfile == NULL)
	{
		printf("\nERROR: Could not open %s for reading.\n", TXTfname);
		exit(1);
	}

	CDTfile = fopen(CDTfname, "wb");										// Open the destination file for writing
	if (CDTfile == NULL)
	{
		printf("\nERROR: Could not open %s for writing.\n", CDTfname);
		exit(1);
	}

	fp = TXTfile;

	// Read in keywords and act on them
	while (ReadWord(word, fp)) {

		// NCARS
		if (Compare(word, "NUMCARS")) {
			// read in the number of cars
			ReadInt(&tInt, fp);
			NCarTypes = tInt;
			CarInfo = malloc(sizeof(CAR_INFO) * NCarTypes);
			CarList = (int *)malloc(sizeof(int) * NCarTypes);
		}

		// CAR
		else if (Compare(word, "CAR")) {
			if ((CarListSize = ReadNumberList(CarList, NCarTypes, fp)) < 0) {
				InvalidNumberList("Car");
				return FALSE;
			}
			if (!ReadCarInfo(fp)) {
				ShowErrorMessage("Error in CarInit file");
				return FALSE;
			}
		}

		// DEFAULT
		else {
			UnknownWordMessage(word);
		}
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadCarInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadCarInfo(FILE *fp)
{
	char ch;
	char word[READ_MAX_WORDLEN];
	int iCar, iModel;
	REAL tReal;
	int tInt, tInt2, tInt3;
	VEC tVec;
	bool tBool;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
		if (ch == EOF || ch != '{') {
			return FALSE;
		}
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {

		// MODEL FILE
 		if (Compare(word, "MODEL")) {
			if ((ModelListSize = ReadNumberList(ModelList, MAX_CAR_MODEL_TYPES, fp)) < 0) {
		 		InvalidNumberList("Model");
		 		return FALSE;
			}
			ReadFileName(word, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iModel = 0; iModel < ModelListSize; iModel++) {
					if (ModelList[iModel] >= MAX_CAR_MODEL_TYPES) {
						return FALSE;
					}
					strncpy(CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]], word, MAX_CAR_FILENAME);
				}
			}
		}

		// TPage file
		else if (Compare(word, "TPAGE")) {
			ReadFileName(word, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				strncpy(CarInfo[CarList[iCar]].TPageFile, word, MAX_CAR_FILENAME);
			}
		}

		// ENV RGB
		else if (Compare(word, "ENVRGB")) {
			ReadInt(&tInt, fp);
			ReadInt(&tInt2, fp);
			ReadInt(&tInt3, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].EnvRGB = (tInt << 16) | (tInt2 << 8) | tInt3;
			}
		}

		// COLLSKIN file
		else if (Compare(word, "COLL")) {
			ReadFileName(word, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				strncpy(CarInfo[CarList[iCar]].CollFile, word, MAX_CAR_FILENAME);
			}
		}

		// CoM position
		else if (Compare(word, "COM")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH)
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyVec(&tVec, &CarInfo[CarList[iCar]].CoMOffset);
			}
		}

		// Weapon Offset
		else if (Compare(word, "WEAPON")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH)
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyVec(&tVec, &CarInfo[CarList[iCar]].WeaponOffset);
			}
		}

		// BODY
		else if (Compare(word, "BODY")) {
			ReadBodyInfo(fp);
		}

		// WHEEL
		else if (Compare(word, "WHEEL")) {
			if ((WheelListSize = ReadNumberList(WheelList, CAR_NWHEELS, fp)) < 1) {
				InvalidNumberList("Wheel");
				return FALSE;
			}
			ReadWheelInfo(fp);
		}

		// AERIAL
		else if (Compare(word, "AERIAL")) {
			ReadAerialInfo(fp);
		}

		// SPRING
		else if (Compare(word, "SPRING")) {
			if ((SpringListSize = ReadNumberList(SpringList, CAR_NWHEELS, fp)) < 1) {
				InvalidNumberList("Spring");
				return FALSE;
			}
			ReadSpringInfo(fp);
		}

		// AXLE
		else if (Compare(word, "AXLE")) {
			if ((AxleListSize = ReadNumberList(AxleList, CAR_NWHEELS, fp)) < 1) {
				InvalidNumberList("Axle");
				return FALSE;
			}
			ReadAxleInfo(fp);
		}

		// PIN
		else if (Compare(word, "PIN")) {
			if ((PinListSize = ReadNumberList(PinList, CAR_NWHEELS, fp)) < 1) {
				InvalidNumberList("Pin");
				return FALSE;
			}
			ReadPinInfo(fp);
		}

		// SPINNER
		else if (Compare(word, "SPINNER")) {
			ReadSpinnerInfo(fp);
		}

		// STEERRATE
		else if (Compare(word, "STEERRATE")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_FREQ;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].SteerRate = tReal;
			}
		}

		// STEERMOD
		else if (Compare(word, "STEERMOD")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].SteerModifier = tReal;
			}
		}

		// ENGINERATE
		else if (Compare(word, "ENGINERATE")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_FREQ;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].EngineRate = tReal;
			}
		}

		// TopSpeed
		else if (Compare(word, "TOPSPEED")) {
			ReadReal(&tReal, fp);
			tReal *= MPH2OGU_SPEED;// * OGU2GU_VEL;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].TopSpeed = tReal;
			}
		}

		// MaxRevs
		else if (Compare(word, "MAXREVS")) {
			ReadReal(&tReal, fp);
			tReal *= MPH2OGU_SPEED;// * OGU2GU_VEL;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].MaxRevs = tReal;
			}
		}

		// DownForcMod
		else if (Compare(word, "DOWNFORCEMOD")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].DownForceMod = tReal;
			}
		}
		
		// NAME
		else if (Compare(word, "NAME")) {
			ReadFileName(word, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				strncpy(CarInfo[CarList[iCar]].Name, word, CAR_NAMELEN - 1);
			}
		}

		// ALLOWED BEST TIME
		else if (Compare(word, "BESTTIME")) {
			ReadBool(&tBool, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].AllowedBestTime = tBool;
			}
		}

		// SELECTABLE
		else if (Compare(word, "SELECTABLE")) {
			ReadBool(&tBool, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Selectable = tBool;
			}
		}

		// DEFAULT
		else {
			UnknownWordMessage(word);
		}
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadBodyInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadBodyInfo(FILE *fp)
{
	char	ch;
	char	word[READ_MAX_WORDLEN];
	int		iCar;

	int		tInt;
	REAL	tReal;
	VEC		tVec;
	MAT		tMat;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
	}
	if (ch == EOF || ch != '{') {
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {
	
		// MODEL NUMBER
		if (Compare(word, "MODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.ModelNum = tInt;
			}
		}

		// OFFSET
		else if (Compare(word, "OFFSET")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyVec(&tVec, &CarInfo[CarList[iCar]].Body.Offset);
			}
		}

		// MASS
		else if (Compare(word, "MASS")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_MASS;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.Mass = tReal;
			}
		}

		// INERTIA
		else if (Compare(word, "INERTIA")) {
			ReadMat(&tMat, fp);
			//tReal *= OGU2GU_INERTIA;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyMat(&tMat, &CarInfo[CarList[iCar]].Body.Inertia);
			}
		}

		// GRAVITY
		else if (Compare(word, "GRAVITY")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_ACC;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.Gravity = tReal;
			}
		}

		// HARDNESS
		else if (Compare(word, "HARDNESS")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.Hardness = tReal;
			}
		}

		// RESISTANCE
		else if (Compare(word, "RESISTANCE")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.Resistance = tReal;
			}
		}

		// ANGRESISTANCE
		else if (Compare(word, "ANGRES")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.AngResistance = tReal;
			}
		}

		// RESMOD
		else if (Compare(word, "RESMOD")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.ResModifier = tReal;
			}
		}

		// STATICFRICTION
		else if (Compare(word, "STATICFRICTION")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.StaticFriction = tReal;
			}
		}

		// KINETICFRICTION
		else if (Compare(word, "KINETICFRICTION")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.KineticFriction = tReal;
			}
		}

		// GRIP
		else if (Compare(word, "GRIP")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Body.Grip = tReal;
			}
		}

		// Default
		else {
			UnknownWordMessage(word);
		}
	}

	return TRUE;
}



/////////////////////////////////////////////////////////////////////
//
// ReadWheelInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadWheelInfo(FILE *fp)
{
	char	ch;
	char	word[READ_MAX_WORDLEN];
	int		iCar, iWheel;

	int		tInt;
	bool	tBool;
	REAL	tReal;
	VEC	tVec;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
	}
	if (ch == EOF || ch != '{') {
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {
	
		// MODEL NUMBER
		if (Compare(word, "MODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].ModelNum = tInt;
					if (tInt == CAR_MODEL_NONE) {
						CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].IsPresent = FALSE;
					} else {
						CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].IsPresent = TRUE;
					}
				}
			}
		}

		// OFFSETS
		else if (Compare(word, "OFFSET1")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CopyVec(&tVec, &CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].Offset1);
				}
			}
		}
		else if (Compare(word, "OFFSET2")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CopyVec(&tVec, &CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].Offset2);
				}
			}
		}

		// RADIUS
		else if (Compare(word, "RADIUS")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].Radius = tReal;
				}
			}
		}

		// MASS
		else if (Compare(word, "MASS")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_MASS;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].Mass = tReal;
				}
			}
		}

		// GRAVITY
		else if (Compare(word, "GRAVITY")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_ACC;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].Gravity = tReal;
				}
			}
		}

		// MAXPOS
		else if (Compare(word, "MAXPOS")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].MaxPos = tReal;
				}
			}
		}
		
		// Grip
		else if (Compare(word, "GRIP")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_GRIP;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].Grip = tReal;
				}
			}
		}

		// STATIC FRICTION
		else if (Compare(word, "STATICFRICTION")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].StaticFriction = tReal;
				}
			}
		}

		// KINETIC FRICTION
		else if (Compare(word, "KINETICFRICTION")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].KineticFriction = tReal;
				}
			}
		}

		// AXLE FRICTION
		else if (Compare(word, "AXLEFRICTION")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].AxleFriction = tReal;
				}
			}
		}

		// STEER RATIO
		else if (Compare(word, "STEERRATIO")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_FORCE;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].SteerRatio = tReal;
				}
			}
		}

		// ENGINE RATIO
		else if (Compare(word, "ENGINERATIO")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_FORCE;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].EngineRatio = tReal;
				}
			}
		}

		// WHEEL STATUS
		else if (Compare(word, "ISTURNABLE")) {
			ReadBool(&tBool, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].IsTurnable = tBool;
				}
			}
		}
		else if (Compare(word, "ISPOWERED")) {
			ReadBool(&tBool, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].IsPowered = tBool;
				}
			}
		}
		else if (Compare(word, "ISPRESENT")) {
			ReadBool(&tBool, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].IsPresent = tBool;
				}
			}
		}

		// SKIDWIDTH
		else if (Compare(word, "SKIDWIDTH")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
					CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].SkidWidth = tReal;
				}
			}
		}


		// Default
		else {
			UnknownWordMessage(word);
		}

	}

	return TRUE;
}



/////////////////////////////////////////////////////////////////////
//
// ReadSpringInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadSpringInfo(FILE *fp)
{
	char	ch;
	char	word[READ_MAX_WORDLEN];
	int		iCar, iSpring;

	int		tInt;
	REAL	tReal;
	VEC		tVec;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
	}
	if (ch == EOF || ch != '{') {
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {

		// MODEL NUMBER
		if (Compare(word, "MODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iSpring = 0; iSpring < SpringListSize; iSpring++) {
					CarInfo[CarList[iCar]].Spring[SpringList[iSpring]].ModelNum = tInt;
				}
			}
		}

		// OFFSETS
		else if (Compare(word, "OFFSET")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iSpring = 0; iSpring < SpringListSize; iSpring++) {
					CopyVec(&tVec, &CarInfo[CarList[iCar]].Spring[SpringList[iSpring]].Offset);
				}
			}
		}
	
		// LENGTH
		else if (Compare(word, "LENGTH")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iSpring = 0; iSpring < SpringListSize; iSpring++) {
					CarInfo[CarList[iCar]].Spring[SpringList[iSpring]].Length = tReal;
				}
			}
		}

		// Stiffness
		else if (Compare(word, "STIFFNESS")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_STIFFNESS;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iSpring = 0; iSpring < SpringListSize; iSpring++) {
					CarInfo[CarList[iCar]].Spring[SpringList[iSpring]].Stiffness = tReal;
				}
			}
		}

		// Damping
		else if (Compare(word, "DAMPING")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_DAMPING;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iSpring = 0; iSpring < SpringListSize; iSpring++) {
					CarInfo[CarList[iCar]].Spring[SpringList[iSpring]].Damping = tReal;
				}
			}
		}

		// Restitution
		else if (Compare(word, "RESTITUTION")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iSpring = 0; iSpring < SpringListSize; iSpring++) {
					CarInfo[CarList[iCar]].Spring[SpringList[iSpring]].Restitution = tReal;
				}
			}
		}

		// Default
		else {
			UnknownWordMessage(word);
		}
	}
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadAxleInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadAxleInfo(FILE *fp)
{
	char	ch;
	char	word[READ_MAX_WORDLEN];
	int		iCar, iAxle;

	int		tInt;
	REAL	tReal;
	VEC		tVec;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
	}
	if (ch == EOF || ch != '{') {
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {

		// MODEL NUMBER
		if (Compare(word, "MODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iAxle = 0; iAxle < AxleListSize; iAxle++) {
					CarInfo[CarList[iCar]].Axle[AxleList[iAxle]].ModelNum = tInt;
				}
			}
		}

		// OFFSET
		else if (Compare(word, "OFFSET")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iAxle = 0; iAxle < AxleListSize; iAxle++) {
					CopyVec(&tVec, &CarInfo[CarList[iCar]].Axle[AxleList[iAxle]].Offset);
				}
			}
		}
	
		// LENGTH
		else if (Compare(word, "LENGTH")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iAxle = 0; iAxle < AxleListSize; iAxle++) {
					CarInfo[CarList[iCar]].Axle[AxleList[iAxle]].Length = tReal;
				}
			}
		}

		// Default
		else {
			UnknownWordMessage(word);
		}
	}
	
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadPinInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadPinInfo(FILE *fp)
{
	char	ch;
	char	word[READ_MAX_WORDLEN];
	int		iCar, iPin;

	int		tInt;
	REAL	tReal;
	VEC		tVec;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
	}
	if (ch == EOF || ch != '{') {
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {

		// MODEL NUMBER
		if (Compare(word, "MODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iPin = 0; iPin < PinListSize; iPin++) {
					CarInfo[CarList[iCar]].Pin[PinList[iPin]].ModelNum = tInt;
				}
			}
		}

		// OFFSET
		else if (Compare(word, "OFFSET")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iPin = 0; iPin < PinListSize; iPin++) {
					CopyVec(&tVec, &CarInfo[CarList[iCar]].Pin[PinList[iPin]].Offset);
				}
			}
		}
	
		// LENGTH
		else if (Compare(word, "LENGTH")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				for (iPin = 0; iPin < PinListSize; iPin++) {
					CarInfo[CarList[iCar]].Pin[PinList[iPin]].Length = tReal;
				}
			}
		}

		// Default
		else {
			UnknownWordMessage(word);
		}
	}
	
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
//
// ReadSpinnerInfo
//
/////////////////////////////////////////////////////////////////////

bool ReadSpinnerInfo(FILE *fp)
{
	char	ch;
	char	word[READ_MAX_WORDLEN];
	int		iCar;

	int		tInt;
	REAL	tReal;
	VEC		tVec;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
	}
	if (ch == EOF || ch != '{') {
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {

		// MODEL NUMBERS
		if (Compare(word, "MODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Spinner.ModelNum = tInt;
			}
		}

		// OFFSET
		else if (Compare(word, "OFFSET")) {
			ReadVec(&tVec, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyVec(&tVec, &CarInfo[CarList[iCar]].Spinner.Offset);
			}
		}

		// Axis of spin
		else if (Compare(word, "AXIS")) {
			ReadVec(&tVec, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyVec(&tVec, &CarInfo[CarList[iCar]].Spinner.Axis);
			}
		}

		// Spin angular velocity
		else if (Compare(word, "ANGVEL")) {
			ReadReal(&tReal, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Spinner.AngVel = tReal;
			}
		}

		// Default
		else {
			UnknownWordMessage(word);
		}

	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// ReadAerialInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadAerialInfo(FILE *fp)
{
	char	ch;
	char	word[READ_MAX_WORDLEN];
	int		iCar;

	int		tInt;
	REAL	tReal;
	VEC		tVec;

	// Find the opening braces
	while (isspace(ch = fgetc(fp)) && ch != EOF) {
	}
	if (ch == EOF || ch != '{') {
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp) && !Compare(word, "}")) {

		// MODEL NUMBERS
		if (Compare(word, "SECMODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Aerial.SecModelNum = tInt;
			}
		}
		else if (Compare(word, "TOPMODELNUM")) {
			ReadInt(&tInt, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Aerial.TopModelNum = tInt;
			}
		}

		// OFFSET
		else if (Compare(word, "OFFSET")) {
			ReadVec(&tVec, fp);
			//VecMulScalar(&tVec, OGU2GU_LENGTH);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyVec(&tVec, &CarInfo[CarList[iCar]].Aerial.Offset);
			}
		}
	
		// LENGTH
		else if (Compare(word, "LENGTH")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Aerial.SecLen = tReal;
			}
		}

		// DIRECTION
		else if (Compare(word, "DIRECTION")) {
			ReadVec(&tVec, fp);
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CopyVec(&tVec, &CarInfo[CarList[iCar]].Aerial.Direction);
			}
		}

		// STIFFNESS
		else if (Compare(word, "STIFFNESS")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_FORCE / OGU2GU_LENGTH;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Aerial.Stiffness = tReal;
			}
		}

		// DAMPING
		else if (Compare(word, "DAMPING")) {
			ReadReal(&tReal, fp);
			//tReal *= OGU2GU_DAMPING;
			for (iCar = 0; iCar < CarListSize; iCar++) {
				CarInfo[CarList[iCar]].Aerial.Damping = tReal;
			}
		}

		// Default
		else {
			UnknownWordMessage(word);
		}
	}
		
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// UnknownWordMessage:
//
/////////////////////////////////////////////////////////////////////

void UnknownWordMessage(char *word)
{
	printf("ERROR: Unrecognised word: \"%s\"\n", word);
	if (Anal)
	{
		exit(1);
	}
}

void ShowErrorMessage(char *word)
{
	printf("ERROR: Initialsation error: %s\n", word);
	if (Anal)
	{
		exit(1);
	}
}

void InvalidVariable(char *object)
{
	printf("ERROR: Invalid variable for %s\n", object);
	if (Anal)
	{
		exit(1);
	}
}

void InvalidNumberList(char *object)
{
	printf("ERROR: Invalid number list for %s\n", object);
	if (Anal)
	{
		exit(1);
	}
}


//============================================================================================

//
// fputlong
//
// Write big endian long value to a file
//

void fputlong(long value, FILE *file)
{
	long	temp;

	temp = value >> 24;
	fwrite(&temp, 1, 1, file);
	temp = value >> 16;
	fwrite(&temp, 1, 1, file);
	temp = value >> 8;
	fwrite(&temp, 1, 1, file);
	fwrite(&value, 1, 1, file);
}


//
// fputshort
//
// Write big endian short value to a file
//

void fputshort(short value, FILE *file)
{
	long	temp;

	temp = value >> 8;
	fwrite(&temp, 1, 1, file);
	fwrite(&value, 1, 1, file);
}


//
// Write big endian float value to a file
//

void fputfloat(float value, FILE *file)
{
	char	*temp;

	temp = (char *)&value;	
	fwrite(temp + 3, 1, 1, file);
	fwrite(temp + 2, 1, 1, file);
	fwrite(temp + 1, 1, 1, file);
	fwrite(temp + 0, 1, 1, file);
}


//
// Write little endian REAL value
//

void fputreal(float value, FILE *file)
{
	long	temp;

	temp = (long)(value * 65536);
	fwrite((char *)&temp, sizeof(long), 1, file);
}
