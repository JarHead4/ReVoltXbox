
#include "ReadInit.h"
#include "Geom.h"
#include "main.h"
#include "texture.h"

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

static char ErrorMessage[READ_MAX_WORDLEN];

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
			NULL;
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
		NULL;
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
		NULL;
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
			NULL;
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

bool ReadAllCarInfo(char *fileName)
{
	char word[READ_MAX_WORDLEN];
	FILE *fp;

	int tInt;

	// Open the file
	if ((fp = fopen(fileName, "r")) == NULL) {
		ShowErrorMessage("Could not open CarInit file");
		return FALSE;
	}

	// Read in keywords and act on them
	while (ReadWord(word, fp)) {

		// NCARS
		if (Compare(word, "NUMCARS")) {
			// read in the number of cars
			ReadInt(&tInt, fp);
			NCarTypes = tInt;
			if (CarInfo == NULL) {
				CarInfo = CreateCarInfo(NCarTypes);
			}
			CarList = (int *)malloc(sizeof(int) * NCarTypes);
		}

		// CAR
		else if (Compare(word, "CAR")) {
			if ((CarListSize = ReadNumberList(CarList, NCarTypes, fp)) < 0) {
				InvalidNumberList("Car");
				fclose(fp); 
				return FALSE;
			}
			if (!ReadCarInfo(fp)) {
				ShowErrorMessage("Error in CarInit file");
				fclose(fp);
				return FALSE;
			}
		}

		// DEFAULT
		else {
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
		}
	}

	free(CarList);
	fclose(fp);

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
	bool tBool;
	//char *fileName;

	REAL tReal;
	int tInt, tInt2, tInt3;
	VEC tVec;

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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
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
	VEC	tVec;
	MAT	tMat;

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

		// RESISTANCE
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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
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
	VEC	tVec;

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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
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
	VEC	tVec;

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
			if (UnknownWordMessage(word) == IDNO) {
				fclose(fp);
				return FALSE;
			}
		}
	}
		
	return TRUE;
}


/////////////////////////////////////////////////////////////////////
//
// UnknownWordMessage:
//
/////////////////////////////////////////////////////////////////////

int UnknownWordMessage(char *word)
{

	wsprintf(ErrorMessage, "\"%s\"\n\nContinue?", word);
	return Box("Unrecognised word:", ErrorMessage, MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION);

}

void ShowErrorMessage(char *word)
{

	wsprintf(ErrorMessage, "%s", word);
	Box("Initialisation Error", ErrorMessage, MB_OK);

}

void InvalidVariable(char *object)
{

	wsprintf(ErrorMessage, "Invalid variable for%s", object);
	Box("Initialisation Error", ErrorMessage, MB_OK);

}

void InvalidNumberList(char *object)
{

	wsprintf(ErrorMessage, "Invalid number list for\n%s", object);
	Box("Initialisation Error", ErrorMessage, MB_OK);

}
