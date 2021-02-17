//-----------------------------------------------------------------------------
// File: readinit.cpp
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include "readinit.h"
#include "geom.h"
#include "main.h"
#include "texture.h"
#include "competition.h"
#include "draw.h"
#include "ai_car.h"
#include "content.h"
#include "Settings.h"


bool ReadAIInfo(FILE *fp);
void InitCarInfo(CAR_INFO *allCarInfo, int nCars);


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
        string[iChar] = (char)toupper(string[iChar]);
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
//$MD: removed
#if 0
bool ReadAllCarInfo(char *fileName)
{
    char word[READ_MAX_WORDLEN];
    FILE *fp;

    int tInt;

    // Open the file (note: must be binary mode for Checksum to work
    if ((fp = fopen(fileName, "rb")) == NULL) {
        ShowErrorMessage("Could not open CarInit file");
        return FALSE;
    }

    // Check the checksum
    if (!CheckStreamChecksum(fp, FALSE)) {
        ModifiedCarInfo = TRUE;
    }

    // Re-open the file in ascii mode
    if ((fp = freopen(fileName, "r", fp)) == NULL) {
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

            // Clear Cheat flag for all cars (when carinfo read from a single file, use ModifiedCarInfo
            for (tInt = 0; tInt < NCarTypes; tInt++) {
                CarInfo[tInt].Modified = FALSE;
                CarInfo[tInt].Moved = FALSE;
            }

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
            if (HexStringToInt(word) == 0) {
                if (UnknownWordMessage(word) == IDNO) {
                    fclose(fp);
                    return FALSE;
                }
            }
        }
    }

    free(CarList);
    fclose(fp);


    // TEMPORARY - load AI info into carInfo structure
    //InitCarInfoAIData();

    return TRUE;
}
#endif


/////////////////////////////////////////////////////////////////////
//
// ReadCarInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadCarInfo(FILE *fp, const CHAR* szRoot)
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
//$MODIFIED(cprince) - insert szRoot at the start, if the filename is relative non-null
//                    strncpy(CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]], word, MAX_CAR_FILENAME);
                    if( 0 == strlen(word) || ':' == word[1] ) {
                        strncpy( CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]], word, MAX_CAR_FILENAME );
                    } else {
                        _snprintf( CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]], MAX_CAR_FILENAME, "%s\\%s", szRoot, word );
                        CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]][MAX_CAR_FILENAME] = '\0';
                    }
    //$PERF(cprince): no reason to do this check/fixup in shipping game.
//$END_MODIFICATIONS
                }
            }
        }

        // TPage file
        else if (Compare(word, "TPAGE")) {
            ReadFileName(word, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
///$MODIFIED(cprince) - insert szRoot at the start, if the filename is relative non-null
//                    strncpy(CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]], word, MAX_CAR_FILENAME);
                    if( 0 == strlen(word) || ':' == word[1] ) {
                    strncpy( CarInfo[CarList[iCar]].TPageFile, word, MAX_CAR_FILENAME );
                } else {
                    _snprintf( CarInfo[CarList[iCar]].TPageFile, MAX_CAR_FILENAME, "%s\\%s", szRoot, word );
                    CarInfo[CarList[iCar]].TPageFile[MAX_CAR_FILENAME] = '\0';
                }
    //$PERF(cprince): no reason to do this check/fixup in shipping game.
//$END_MODIFICATIONS
            }
        }

        // Carbox texture file
        else if (Compare(word, "TCARBOX")) {
            ReadFileName(word, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
///$MODIFIED(cprince) - insert szRoot at the start, if the filename is relative non-null
//                    strncpy(CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]], word, MAX_CAR_FILENAME);
                    if( 0 == strlen(word) || ':' == word[1] ) {
                    strncpy( CarInfo[CarList[iCar]].TCarBoxFile, word, MAX_CAR_FILENAME );
                } else {
                    _snprintf( CarInfo[CarList[iCar]].TCarBoxFile, MAX_CAR_FILENAME, "%s\\%s", szRoot, word );
                    CarInfo[CarList[iCar]].TCarBoxFile[MAX_CAR_FILENAME] = '\0';
                }
    //$PERF(cprince): no reason to do this check/fixup in shipping game.
//$END_MODIFICATIONS
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
///$MODIFIED(cprince) - insert szRoot at the start, if the filename is relative non-null
//                    strncpy(CarInfo[CarList[iCar]].ModelFile[ModelList[iModel]], word, MAX_CAR_FILENAME);
                    if( 0 == strlen(word) || ':' == word[1] ) {
                    strncpy( CarInfo[CarList[iCar]].CollFile, word, MAX_CAR_FILENAME );
                } else {
                    _snprintf( CarInfo[CarList[iCar]].CollFile, MAX_CAR_FILENAME, "%s\\%s", szRoot, word );
                    CarInfo[CarList[iCar]].CollFile[MAX_CAR_FILENAME] = '\0';
                }
    //$PERF(cprince): no reason to do this check/fixup in shipping game.
//$END_MODIFICATIONS
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

        // AI
        else if (Compare(word, "AI")) {
            ReadAIInfo(fp);
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
                CarInfo[CarList[iCar]].Selectable = tBool; // $MD: actually use selectable bit
            }
        }

        // OBTAIN
        else if (Compare(word, "OBTAIN")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].ObtainMethod = (CAR_OBTAIN)tInt;
            }
        }

        // CLASS
        else if (Compare(word, "CLASS")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].Class = (CAR_CLASS)tInt;
            }
        }

        // RATING
        else if (Compare(word, "RATING")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].Rating = (CAR_RATING)tInt;
            }
        }

        // TopEnd
        else if (Compare(word, "TOPEND")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].TopEnd = tReal;
            }
        }

        // Acc
        else if (Compare(word, "ACC")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].Acc = tReal;
            }
        }

        // Weight
        else if (Compare(word, "WEIGHT")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].Weight = tReal;
            }
        }

        // Handling
        else if (Compare(word, "Handling")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].Handling = tReal;
            }
        }

        // Transmission
        else if (Compare(word, "TRANS")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].Trans = tInt;
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
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar;

    int     tInt;
    REAL    tReal;
    VEC tVec;
    MAT tMat;

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
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar, iWheel;

    int     tInt;
    bool    tBool;
    REAL    tReal;
    VEC tVec;

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
                    /*if (tInt == CAR_MODEL_NONE) {
                        CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].IsPresent = FALSE;
                    } else {
                        CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].IsPresent = TRUE;
                    }*/
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

        // TOEIN
        else if (Compare(word, "TOEIN")) {
            ReadReal(&tReal, fp);
            //tReal *= OGU2GU_LENGTH;
            for (iCar = 0; iCar < CarListSize; iCar++) {
                for (iWheel = 0; iWheel < WheelListSize; iWheel++) {
                    //CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].ToeIn = tReal;
                    CarInfo[CarList[iCar]].Wheel[WheelList[iWheel]].ToeIn = ZERO;
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
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar, iSpring;

    int     tInt;
    REAL    tReal;
    VEC tVec;

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
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar, iAxle;

    int     tInt;
    REAL    tReal;
    VEC tVec;

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
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar, iPin;

    int     tInt;
    REAL    tReal;
    VEC tVec;

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
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar;

    int     tInt;
    REAL    tReal;
    VEC tVec;

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
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar;

    int     tInt;
    REAL    tReal;
    VEC tVec;

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
// ReadAIInfo:
//
/////////////////////////////////////////////////////////////////////

bool ReadAIInfo(FILE *fp)
{
    char    ch;
    char    word[READ_MAX_WORDLEN];
    int     iCar;

    int     tInt;
    REAL    tReal;

    // Find the opening braces
    while (isspace(ch = fgetc(fp)) && ch != EOF) {
    }
    if (ch == EOF || ch != '{') {
        return FALSE;
    }

    // Read in keywords and act on them
    while (ReadWord(word, fp) && !Compare(word, "}")) {

        // UNDERSTEER THRESHOLD
        if (Compare(word, "UNDERTHRESH")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.understeerThreshold = tReal;
            }
        }

        // UNDERSTEER RANGE
        else if (Compare(word, "UNDERRANGE")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.understeerRange = tReal;
            }
        }

        // UNDERSTEER FRONT
        else if (Compare(word, "UNDERFRONT")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.understeerFront = tReal;
            }
        }

        // UNDERSTEER REAR
        else if (Compare(word, "UNDERREAR")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.understeerRear = tReal;
            }
        }

        // UNDERSTEER MAX
        else if (Compare(word, "UNDERMAX")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.understeerMax = tReal;
            }
        }

        // OVERSTEER THRESHOLD
        else if (Compare(word, "OVERTHRESH")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.oversteerThreshold = tReal;
            }
        }

        // OVERSTEER RANGE
        else if (Compare(word, "OVERRANGE")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.oversteerRange = tReal;
            }
        }

        // OVERSTEER MAX
        else if (Compare(word, "OVERMAX")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.oversteerMax = tReal;
            }
        }

        // OVERSTEER FRONT
        else if (Compare(word, "OVERACCTHRESH")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.oversteerAccelThreshold = tReal;
            }
        }

        // OVERSTEER REAR
        else if (Compare(word, "OVERACCRANGE")) {
            ReadReal(&tReal, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.oversteerAccelRange = tReal;
            }
        }

        // PICKUP BIAS
        else if (Compare(word, "PICKUPBIAS")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.pickupBias = tInt;
            }
        }

        // BLOCK BIAS
        else if (Compare(word, "BLOCKBIAS")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.blockBias = tInt;
            }
        }

        // OVERTAKE BIAS
        else if (Compare(word, "OVERTAKEBIAS")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.overtakeBias = tInt;
            }
        }

        // SUSPENSION
        else if (Compare(word, "SUSPENSION")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.suspension = tInt;
            }
        }

        // AGGRESSION
        else if (Compare(word, "AGGRESSION")) {
            ReadInt(&tInt, fp);
            for (iCar = 0; iCar < CarListSize; iCar++) {
                CarInfo[CarList[iCar]].AI.aggression = tInt;
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
    sprintf(ErrorMessage, "\"%s\"\n\nContinue?", word);
//$MODIFIED
//    return Box("Unrecognised word:", ErrorMessage, MB_YESNO | MB_DEFBUTTON1 | MB_ICONQUESTION);
    //$CMP_NOTE: if we reach this point, something bad has happened.
    /// So always return value indicating that we don't want to continue.
    DumpMessage("Unrecognised word:", ErrorMessage);
    return IDNO;
    //$REVISIT -- Will we ever want option of continuing after an unrecognized word is found?
    /// If not, then remove this function.
//$END_MODIFICATIONS
}

void ShowErrorMessage(char *word)
{
    sprintf(ErrorMessage, "%s", word);
    DumpMessage("Initialisation Error", ErrorMessage);
}

void InvalidVariable(char *object)
{
    sprintf(ErrorMessage, "Invalid variable for%s", object);
    DumpMessage("Initialisation Error", ErrorMessage);
}

void InvalidNumberList(char *object)
{
    sprintf(ErrorMessage, "Invalid number list for\n%s", object);
    DumpMessage("Initialisation Error", ErrorMessage);
}



////////////////////////////////////////////////////////////////
//
// Write Car Info
//
////////////////////////////////////////////////////////////////
#define CARINFO_COMMENT_BAR "===================="

bool WriteOneCarInfo(CAR_INFO *carInfo, FILE *fp)
{

    // Opening braces
    fprintf(fp, "{\n\n");

    // Name comment
    fprintf(fp, ";%s\n", CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR);
    fprintf(fp, ";%s\n", CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR);
    fprintf(fp, "; %s\n", carInfo->Name);
    fprintf(fp, ";%s\n", CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR);
    fprintf(fp, ";%s\n", CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR CARINFO_COMMENT_BAR);

    // Car Number and name
    fprintf(fp, "Name      \t\"%s\"\n\n", carInfo->Name);



    // Model filenames
    fprintf(fp, "\n;%s\n", CARINFO_COMMENT_BAR);
    fprintf(fp, "; Model Filenames\n");
    fprintf(fp, ";%s\n\n", CARINFO_COMMENT_BAR);

    int iModel;
    for (iModel = 0; iModel < MAX_CAR_MODEL_TYPES; iModel++) {
        fprintf(fp, "MODEL \t%d \t\"%s\"\n",
            iModel,
            (carInfo->ModelFile[iModel][0] == '\0')? "NONE": carInfo->ModelFile[iModel]);
    }

    // TPage
    fprintf(fp, "TPAGE \t\"%s\"\n", (carInfo->TPageFile[0] == '\0')? "NONE": carInfo->TPageFile);

    // Collision skin
    fprintf(fp, "COLL \t\"%s\"\n", (carInfo->CollFile[0] == '\0')? "NONE": carInfo->CollFile);

    // Env RGB
    fprintf(fp, "EnvRGB \t%d %d %d\n", (carInfo->EnvRGB & RGB_RED_MASK)>>16, (carInfo->EnvRGB & RGB_GREEN_MASK)>>8, (carInfo->EnvRGB & RGB_BLUE_MASK));

    // Frontend variables
    fprintf(fp, "\n;%s\n; Stuff mainly for frontend display and car selectability\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    fprintf(fp, "BestTime   \t%s\n", (carInfo->AllowedBestTime)? "TRUE": "FALSE");
    fprintf(fp, "Selectable \t%s\n", (carInfo->Selectable)? "TRUE": "FALSE");
    fprintf(fp, "Class      \t%d \t\t\t; Engine type (0=Elec, 1=Glow, 2=Other)\n", carInfo->Class);
    fprintf(fp, "Obtain     \t%d \t\t\t; Obtain method\n", carInfo->ObtainMethod);
    fprintf(fp, "Rating     \t%d \t\t\t; Skill level (rookie, amateur, ...)\n", carInfo->Rating);
    fprintf(fp, "TopEnd     \t%f \t\t\t; Actual top speed (mph) for frontend bars\n", carInfo->TopEnd);
    fprintf(fp, "Acc        \t%f \t\t\t; Acceleration rating (empirical)\n", carInfo->Acc);
    fprintf(fp, "Weight     \t%f \t\t\t; Scaled weight (for frontend bars)\n", carInfo->Weight);
    fprintf(fp, "Handling   \t%f \t\t\t; Handling ability (empirical and totally subjective)\n", carInfo->Handling);
    fprintf(fp, "Trans      \t%d \t\t\t; Transmission type (calculate in game anyway...)\n", carInfo->Trans);
    fprintf(fp, "MaxRevs    \t%f \t\t\t; Max Revs (for rev counter)\n", carInfo->MaxRevs * OGU2MPH_SPEED);

    // Handling/ engine stuff
    fprintf(fp, "\n;%s\n; Handling related stuff\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    fprintf(fp, "SteerRate  \t%f \t\t\t; Rate at which steer angle approaches value from input\n", carInfo->SteerRate);
    fprintf(fp, "SteerMod   \t%f \t\t\t;\n", carInfo->SteerModifier);
    fprintf(fp, "EngineRate \t%f \t\t\t; Rate at which Engine voltage approaches set value\n", carInfo->EngineRate);
    fprintf(fp, "TopSpeed   \t%f \t\t\t; Car's theoretical top speed (not including friction...)\n", carInfo->TopSpeed * OGU2MPH_SPEED);
    fprintf(fp, "DownForceMod\t%f \t\t\t; Down force modifier when car on floor\n", carInfo->DownForceMod);
    fprintf(fp, "CoM        \t%f %f %f \t\t; Centre of mass relative to model centre\n", carInfo->CoMOffset.v[X], carInfo->CoMOffset.v[Y], carInfo->CoMOffset.v[Z]);
    fprintf(fp, "Weapon     \t%f %f %f \t\t; Weapon genration offset\n", carInfo->WeaponOffset.v[X], carInfo->WeaponOffset.v[Y], carInfo->WeaponOffset.v[Z]);

    // Body Stuff
    fprintf(fp, "\n;%s\n; Car Body details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    fprintf(fp, "BODY {\t\t; Start Body\n");
    fprintf(fp, "ModelNum   \t%d \t\t\t; Model Number in above list\n", carInfo->Body.ModelNum);
    fprintf(fp, "Offset     \t0, 0, 0 \t\t; Calculated in game\n");
    fprintf(fp, "Mass       \t%f\n", carInfo->Body.Mass);
    fprintf(fp, "Inertia    \t%f %f %f\n"
                "           \t%f %f %f\n"
                "           \t%f %f %f\n", 
                carInfo->Body.Inertia.m[XX], carInfo->Body.Inertia.m[XY], carInfo->Body.Inertia.m[XZ],
                carInfo->Body.Inertia.m[YX], carInfo->Body.Inertia.m[YY], carInfo->Body.Inertia.m[YZ],
                carInfo->Body.Inertia.m[ZX], carInfo->Body.Inertia.m[ZY], carInfo->Body.Inertia.m[ZZ]);
    fprintf(fp, "Gravity    \t2200 \t\t\t; No longer used\n");
    fprintf(fp, "Hardness   \t%f\n", carInfo->Body.Hardness);
    fprintf(fp, "Resistance \t%f \t\t\t; Linear air esistance\n", carInfo->Body.Resistance);
    fprintf(fp, "AngRes     \t%f \t\t\t; Angular air resistance\n", carInfo->Body.AngResistance);
    fprintf(fp, "ResMod     \t%f \t\t\t; Ang air resistnce scale when in air\n", carInfo->Body.ResModifier);
    fprintf(fp, "Grip       \t%f \t\t\t; Converts downforce to friction value\n", carInfo->Body.Grip);
    fprintf(fp, "StaticFriction %f\n", carInfo->Body.StaticFriction);
    fprintf(fp, "KineticFriction %f\n", carInfo->Body.KineticFriction);
    fprintf(fp, "}     \t\t; End Body\n");

    // Wheel stuff
    fprintf(fp, "\n;%s\n; Car Wheel details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    int iWheel;
    for (iWheel = 0; iWheel < CAR_NWHEELS; iWheel++) {
        WHEEL_INFO *wheel = &carInfo->Wheel[iWheel];
        fprintf(fp, "WHEEL %d { \t; Start Wheel\n", iWheel);
        fprintf(fp, "ModelNum \t%d\n", wheel->ModelNum);
        fprintf(fp, "Offset1  \t%f %f %f\n", wheel->Offset1.v[X], wheel->Offset1.v[Y], wheel->Offset1.v[Z]);
        fprintf(fp, "Offset2  \t%f %f %f\n", wheel->Offset2.v[X], wheel->Offset2.v[Y], wheel->Offset2.v[Z]);
        fprintf(fp, "IsPresent   \t%s\n", (wheel->IsPresent)? "TRUE": "FALSE");
        fprintf(fp, "IsPowered   \t%s\n", (wheel->IsPowered)? "TRUE": "FALSE");
        fprintf(fp, "IsTurnable  \t%s\n", (wheel->IsTurnable)? "TRUE": "FALSE");
        fprintf(fp, "SteerRatio  \t%f\n", wheel->SteerRatio);
        fprintf(fp, "EngineRatio \t%f\n", wheel->EngineRatio);
        fprintf(fp, "Radius      \t%f\n", wheel->Radius);
        fprintf(fp, "Mass        \t%f\n", wheel->Mass);
        fprintf(fp, "Gravity     \t%f\n", wheel->Gravity);
        fprintf(fp, "MaxPos      \t%f\n", wheel->MaxPos);
        fprintf(fp, "SkidWidth   \t%f\n", wheel->SkidWidth);
        fprintf(fp, "ToeIn       \t%f\n", wheel->ToeIn);
        fprintf(fp, "AxleFriction    \t%f\n", wheel->AxleFriction);
        fprintf(fp, "Grip            \t%f\n", wheel->Grip);
        fprintf(fp, "StaticFriction  \t%f\n", wheel->StaticFriction);
        fprintf(fp, "KineticFriction \t%f\n", wheel->KineticFriction);
        fprintf(fp, "}          \t; End Wheel\n\n");
    }

    // Spring stuff
    fprintf(fp, "\n;%s\n; Car Spring details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    int iSpring;
    for (iSpring = 0; iSpring < CAR_NWHEELS; iSpring++) {
        SPRING_INFO *spring = &carInfo->Spring[iSpring];
        fprintf(fp, "SPRING %d { \t; Start Spring\n", iSpring);
        fprintf(fp, "ModelNum    \t%d\n", spring->ModelNum);
        fprintf(fp, "Offset      \t%f %f %f\n", spring->Offset.v[X], spring->Offset.v[Y], spring->Offset.v[Z]);
        fprintf(fp, "Length      \t%f\n", spring->Length);
        fprintf(fp, "Stiffness   \t%f\n", spring->Stiffness);
        fprintf(fp, "Damping     \t%f\n", spring->Damping);
        fprintf(fp, "Restitution \t%f\n", spring->Restitution);
        fprintf(fp, "}           \t; End Spring\n\n");
    }

    // Pin stuff
    fprintf(fp, "\n;%s\n; Car Pin details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    int iPin;
    for (iPin = 0; iPin < CAR_NWHEELS; iPin++) {
        PIN_INFO *pin = &carInfo->Pin[iPin];
        fprintf(fp, "PIN %d {    \t; Start Pin\n", iPin);
        fprintf(fp, "ModelNum    \t%d\n", pin->ModelNum);
        fprintf(fp, "Offset      \t%f %f %f\n", pin->Offset.v[X], pin->Offset.v[Y], pin->Offset.v[Z]);
        fprintf(fp, "Length      \t%f\n", pin->Length);
        fprintf(fp, "}           \t; End Pin\n\n");
    }

    // Axle stuff
    fprintf(fp, "\n;%s\n; Car axle details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    int iAxle;
    for (iAxle = 0; iAxle < CAR_NWHEELS; iAxle++) {
        AXLE_INFO *axle = &carInfo->Axle[iAxle];
        fprintf(fp, "AXLE %d {   \t; Start Axle\n", iAxle);
        fprintf(fp, "ModelNum    \t%d\n", axle->ModelNum);
        fprintf(fp, "Offset      \t%f %f %f\n", axle->Offset.v[X], axle->Offset.v[Y], axle->Offset.v[Z]);
        fprintf(fp, "Length      \t%f\n", axle->Length);
        fprintf(fp, "}           \t; End axle\n\n");
    }

    // Spinner stuff
    fprintf(fp, "\n;%s\n; Car spinner details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    fprintf(fp, "SPINNER {   \t; Start spinner\n");
    fprintf(fp, "ModelNum    \t%d\n", carInfo->Spinner.ModelNum);
    fprintf(fp, "Offset      \t%f %f %f\n", carInfo->Spinner.Offset.v[X], carInfo->Spinner.Offset.v[Y], carInfo->Spinner.Offset.v[Z]);
    fprintf(fp, "Axis        \t%f %f %f\n", carInfo->Spinner.Axis.v[X], carInfo->Spinner.Axis.v[Y], carInfo->Spinner.Axis.v[Z]);
    fprintf(fp, "AngVel      \t%f\n", carInfo->Spinner.AngVel);
    fprintf(fp, "}           \t; End Spinner\n\n");

    // Aerial stuff
    fprintf(fp, "\n;%s\n; Car Aerial details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    fprintf(fp, "AERIAL {    \t; Start Aerial\n");
    fprintf(fp, "SecModelNum \t%d\n", carInfo->Aerial.SecModelNum);
    fprintf(fp, "TopModelNum \t%d\n", carInfo->Aerial.TopModelNum);
    fprintf(fp, "Offset      \t%f %f %f\n", carInfo->Aerial.Offset.v[X], carInfo->Aerial.Offset.v[Y], carInfo->Aerial.Offset.v[Z]);
    fprintf(fp, "Direction   \t%f %f %f\n", carInfo->Aerial.Direction.v[X], carInfo->Aerial.Direction.v[Y], carInfo->Aerial.Direction.v[Z]);
    fprintf(fp, "Length      \t%f\n", carInfo->Aerial.SecLen);
    fprintf(fp, "Stiffness   \t%f\n", carInfo->Aerial.Stiffness);
    fprintf(fp, "Damping     \t%f\n", carInfo->Aerial.Damping);
    fprintf(fp, "}           \t; End Aerial\n\n");

    // AI stuff
    fprintf(fp, "\n;%s\n; Car AI details\n;%s\n\n", CARINFO_COMMENT_BAR, CARINFO_COMMENT_BAR);
    fprintf(fp, "AI {        \t ;Start AI\n");
    fprintf(fp, "UnderThresh \t%f\n", carInfo->AI.understeerThreshold);
    fprintf(fp, "UnderRange  \t%f\n", carInfo->AI.understeerRange);
    fprintf(fp, "UnderFront  \t%f\n", carInfo->AI.understeerFront);
    fprintf(fp, "UnderRear   \t%f\n", carInfo->AI.understeerRear);
    fprintf(fp, "UnderMax    \t%f\n", carInfo->AI.understeerMax);
    fprintf(fp, "OverThresh  \t%f\n", carInfo->AI.oversteerThreshold);
    fprintf(fp, "OverRange   \t%f\n", carInfo->AI.oversteerRange);
    fprintf(fp, "OverMax     \t%f\n", carInfo->AI.oversteerMax);
    fprintf(fp, "OverAccThresh  \t%f\n", carInfo->AI.oversteerAccelThreshold);
    fprintf(fp, "OverAccRange   \t%f\n", carInfo->AI.oversteerAccelRange);
    fprintf(fp, "PickupBias     \t%d\n", carInfo->AI.pickupBias);
    fprintf(fp, "BlockBias      \t%d\n", carInfo->AI.blockBias);
    fprintf(fp, "OvertakeBias   \t%d\n", carInfo->AI.overtakeBias);
    fprintf(fp, "Suspension     \t%d\n", carInfo->AI.suspension);
    fprintf(fp, "Aggression     \t%d\n", carInfo->AI.aggression);
    fprintf(fp, "}           \t; End AI\n\n");

    // Closing braces
    fprintf(fp, "}\n\n");

    return TRUE;
}


////////////////////////////////////////////////////////////
//
// WritaAllCarInfoSingle: write all car infos to a single
// carinfo file
//
////////////////////////////////////////////////////////////

bool WriteAllCarInfoSingle(CAR_INFO *carInfo, int nCars, char *filename)
{
    int iCar;
    FILE *fp;

    // Open the file
    fp = fopen(filename, "w");
    if (fp == NULL) {
        return FALSE;
    }

    // Shift all car offsets back to their original positions (before CoM offset called)
    UnsetAllCarCoMs();

    // Write Header
    fprintf(fp, "NumCars \t\t\t%d\n\n", nCars);

    // Write info for each car
    for (iCar = 0; iCar < nCars; iCar++) {

        fprintf(fp, "CAR %d\n", iCar);

        WriteOneCarInfo(&carInfo[iCar], fp);
    }

    fclose(fp);

    // Put the cars CoMs back as they were
    SetAllCarCoMs();

    return TRUE;
}


////////////////////////////////////////////////////////////
//
// WriteAllCarInfoMultiple: write the carinfo's for each
// car to their own directories
//
////////////////////////////////////////////////////////////

//$NOTE(cprince): this code seems to be unreachable.
bool WriteAllCarInfoMultiple(CAR_INFO *carInfo, int nCars)
{
    int iCar, iChar, sLen;
    FILE *fp;
    char filename[MAX_CAR_FILENAME];
    char dirname[MAX_CAR_FILENAME];

    // Shift all car offsets back to their original positions (before CoM offset called)
    UnsetAllCarCoMs();

    //$BUGBUG(cprince): I think the uses of "5" below need to be changed to "8" (because we changed strings from "cars\..." to "D:\cars\...")  But might not matter if this function is unreachable, which it seems to be.
    for (iCar = 0; iCar < nCars; iCar++) {

        // get the filename for this car's carinfo (assumes car has a MODEL 0)
        sLen = strlen(carInfo[iCar].ModelFile[0]);
        if (sLen <= 5) {
            DumpMessage("Warning", "Could not get car directory");
            continue;                   // doesn't even have "cars\" at the start
        }

        iChar = 5;
        while ((iChar < sLen) && (iChar < MAX_CAR_FILENAME - 1) && (carInfo[iCar].ModelFile[0][iChar] != '\\')) {
            dirname[iChar - 5] = carInfo[iCar].ModelFile[0][iChar];
            iChar++;
        }
        dirname[iChar - 5] = '\0';

        sprintf(filename, "D:\\Cars\\%s\\Parameters.txt", dirname); //$MODIFIED: added "D:\\" at start

        // open the file
        fp = fopen(filename, "w");
        if (fp == NULL) {
            DumpMessage("Could not save", filename);
            continue;
        }

        // Write the info file
        if (!WriteOneCarInfo(&carInfo[iCar], fp)) {
            DumpMessage("Could not save", filename);
            continue;
        }
    }

    // Put the cars CoMs back as they were
    SetAllCarCoMs();

    return TRUE;
}


////////////////////////////////////////////////////////////
//
// ReadAllCarInfoMultiple: read the car info files for the
// cars from their own directories
//
////////////////////////////////////////////////////////////
bool ReadAllCarInfoMultiple()
{
    //$MD: Modified,  Only default cars are loaded here

    int iCar;
    FILE *fp;
    char szFilename[MAX_CAR_FILENAME];
    
    NCarTypes = CARID_NTYPES;

    // Create the car info array
    free(CarInfo);
    CarInfo = (CAR_INFO*) malloc(NCarTypes * sizeof(CAR_INFO));
    ZeroMemory( CarInfo, NCarTypes * sizeof(CAR_INFO) );
    InitCarInfo(CarInfo, NCarTypes);

    // Create car list
    // $MD: this list is not really needed.
    CarList = (int *)malloc(sizeof(int));
    
    ////////////////////////////////////////////////////////////
    // Load in the default cars
    //
    for (iCar = 0; iCar < CARID_NTYPES; iCar++) {

        // Build the filename
        sprintf(szFilename, "%s\\Parameters.txt", CarDirs[iCar]);

        // Open the file (note: must be binary mode for Checksum to work
        if ((fp = fopen(szFilename, "rb")) == NULL) {
            assert( FALSE );
            continue;
        }

        // Check the checksum
        // $MD: remove for now
        //if (!CheckStreamChecksum(fp, FALSE)) {
        //    CarInfo[iCar].Modified = TRUE;
        //} else {
        //    CarInfo[iCar].Modified = FALSE;
        //}

        // Re-open the file in ascii mode
        if ((fp = freopen(szFilename, "r", fp)) == NULL) {
            std::string Error("Could not load parameters file: ");
            Error += szFilename;
            DumpMessage("Warning", Error.c_str());
            assert( FALSE );
            continue;
        }

        // Read info into array
        CarListSize = 1;
        CarList[0] = iCar;
        ReadCarInfo(fp, "D:");

        // Make sure that the file had not been moved from one directory into another
        // $MD: checks bitmap dir with car dir
        // $MD: remove for now
        //if (_strnicmp(CarInfo[iCar].TPageFile, CarDirs[iCar], strlen(CarDirs[iCar])) != 0) { //$MODIFIED: changed strnicmp to _strnicmp
        //    CarInfo[iCar].Moved = TRUE;
        //    CarInfo[iCar].Modified = TRUE;
        //}

        fclose(fp);

    }

    free(CarList);
    CarList = NULL;

    // update the default cars center of masses
    for (int iCar = 0; iCar < NCarTypes; iCar++)
        MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);

    // set car selectability
    for(int iCar = 0; iCar < NCarTypes; iCar++)
        CarInfo[iCar].Selectable = BOOL (g_abIsDefaultCarSelectable[iCar]);


    return TRUE;
}

//-----------------------------------------------------------------------------
// Name: ReadAllCarPackagesMutliple
// Desc: Reads in all car packages.  Car packages have a bitflat of 2
//       Can be called mutlple times, with the CarInfo array growing or 
//       shrinking to accomdate
//-----------------------------------------------------------------------------
bool ReadAllCarPackagesMultiple()
{
    // $MD: $BUGBUG:
    // Not sure what will happen if the app registry stores the users current car,
    // the box is rebooted, and then the car is deleted.  When the app comes back
    // up, a car is assigned to a user but it does not exist!

    int iCar;
    FILE *fp;
    char szFilename[MAX_CAR_FILENAME];
    ModifiedCarInfo = FALSE;
    HANDLE hFind;
    XCONTENT_FIND_DATA Finddata;

    // clear out carinfo structures
    for(int i = CARID_NTYPES; i < NCarTypes; i++)
    {
        delete CarInfo[i].m_pCarBoxXBR;
        delete CarInfo[i].m_pXBR;
        
        assert(CarInfo[i].m_RefCountXBR == 0);
        delete CarInfo[i].m_pEffect;
    }


    NCarTypes = CARID_NTYPES;    

    // find out how many extra cars we have

    // shrink the car info array to the size of the original cars
    assert(CarInfo);
    CarInfo = (CAR_INFO*) realloc(CarInfo, NCarTypes * sizeof(CAR_INFO));
    
    // Create car list
    // $MD: this list is not really needed.
    CarList = (int *)malloc(sizeof(int));



    ////////////////////////////////////////////////////////////
    // Load in the downloaded cars
    //
    iCar = CARID_NTYPES; 
      
    // only look for cars
    hFind = XFindFirstContent("T:\\", CONTENT_CAR_FLAG, &Finddata);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            g_ContentManager.AddOwnedContent(Finddata.qwOfferingId, Finddata.szContentDirectory, true);

            // don't add corrupt content
            if(g_ContentManager.IsOwnedContentCorrupted(Finddata.qwOfferingId))
                continue;
            
            
            // Build the filename
            sprintf(szFilename, "%s\\Parameters.txt", Finddata.szContentDirectory);

            

            // Open the file (note: must be binary mode for Checksum to work
            if ((fp = fopen(szFilename, "rb")) == NULL)
            {
#ifdef _DEBUG
                std::string Error("Could not find car paramter file: ");
                Error += szFilename;
                DumpMessage("Warning", Error.c_str());
#endif
                continue;
                
            } 

            NCarTypes++;
            CarInfo = (CAR_INFO*) realloc(CarInfo, NCarTypes * sizeof(CAR_INFO));
            ZeroMemory(CarInfo + NCarTypes - 1, sizeof(CAR_INFO));
            InitCarInfo(CarInfo + NCarTypes - 1, 1);

            // Check the checksum
            // $MD: remove for now
            //if (!CheckStreamChecksum(fp, FALSE)) {
            //    CarInfo[iCar].Modified = TRUE;
            //} else {
            //    CarInfo[iCar].Modified = FALSE;
            //}

            // Re-open the file in ascii mode
            if ((fp = freopen(szFilename, "r", fp)) == NULL) {
#ifdef _DEBUG
                std::string Error("Warning -- Could not find car paramter file: ");
                Error += szFilename;
                Error += "\n";
                OutputDebugString(Error.c_str());
#endif
                assert( FALSE );
                continue;
            }

            // Read info into array
            CarListSize = 1;
            CarList[0] = iCar;
            ReadCarInfo(fp, Finddata.szContentDirectory);
            iCar++;

            // Make sure that the file had not been moved from one directory into another
            // $MD: checks bitmap dir with car dir
            // $MD: remove for now
            //if (_strnicmp(CarInfo[iCar].TPageFile, CarDirs[iCar], strlen(CarDirs[iCar])) != 0) { //$MODIFIED: changed strnicmp to _strnicmp
            //    CarInfo[iCar].Moved = TRUE;
            //    CarInfo[iCar].Modified = TRUE;
            //}
            

            fclose(fp);
        }
        while( XFindNextContent(hFind, &Finddata) );

        XFindClose(hFind); 
    }

    free(CarList);
    CarList = NULL;

    // update the new car center of masses
    for (int iCar = CARID_NTYPES; iCar < NCarTypes; iCar++)
        MoveCarCoM(&CarInfo[iCar], &CarInfo[iCar].CoMOffset);

    // if the car we had selected is not longer there (it is corrupt, was moved, etc), change it back to a car that exists
    if(RegistrySettings.CarType >= (UINT)NCarTypes)
    {
        RegistrySettings.CarType = 0;
    }




    return TRUE; 
}

//-----------------------------------------------------------------------------
// Name: ReadAllCarKeysMutliple
// Desc: Reads in all car key packages, setting the cars with keys as selectable
//-----------------------------------------------------------------------------
bool ReadAllCarKeysMultiple()
{
    // $MD: $BUGBUG:
    // Not sure what will happen if the app registry stores the users current car,
    // the box is rebooted, and then the keys are deleted.  When the app comes back
    // up, a car is assigned to a user but it should not be selectable!
    XCONTENT_FIND_DATA Finddata;
    HANDLE hFind;

    char szSearch[MAX_PATH];
    WIN32_FIND_DATA KeyFindData;
    HANDLE hKeyFind;


    
    ////////////////////////////////////////////////////////////
    // Load in the downloaded cars
    //
      
    // only look for cars keys
    hFind = XFindFirstContent("T:\\", CONTENT_CARKEY_FLAG, &Finddata);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            g_ContentManager.AddOwnedContent(Finddata.qwOfferingId, Finddata.szContentDirectory, true);

            // don't add corrupt content
            if(g_ContentManager.IsOwnedContentCorrupted(Finddata.qwOfferingId))
                continue;

            // look for .txt file
            sprintf(szSearch, "%s\\*.txt", Finddata.szContentDirectory);
            
            hKeyFind = FindFirstFile(szSearch, &KeyFindData);
            if(hKeyFind == INVALID_HANDLE_VALUE)
            {
#ifdef _DEBUG
                DumpMessage("Warning", "Car key package found with not .txt file");
#endif
            }
            else
            {
                FindClose(hKeyFind);

                // remove .txt
                KeyFindData.cFileName[strlen(KeyFindData.cFileName) - 4] = NULL;

                // key found, unlock its car
                for(int i = 0; i < NCarTypes; i++)
                {
                    // convert name
                    if(_stricmp(KeyFindData.cFileName, CarInfo[i].Name) == 0)
                    {
                        CarInfo[i].Selectable = TRUE;
                        break;
                    }

                }

#ifdef _DEBUG
                if(i == NCarTypes)
                {
                    DumpMessage("Warning", "Car keys found with no matching car");
                }
#endif
            }
        }
        while( XFindNextContent(hFind, &Finddata) );

        XFindClose(hFind);
    }
    return TRUE; 
}

// $MD: old code
/*

    int iCar, currentCar;
    FILE *fp;
    HANDLE handle;
    CONTENT_FIND_DATA data;
    char filename[MAX_CAR_FILENAME];

    ModifiedCarInfo = FALSE;


    ////////////////////////////////////////////////////////////
    // Count the directories with a valid "Parameters.txt"
    //
    NCarTypes = CARID_NTYPES;
    
    handle = FindFirstFile("D:\\cars\\*.*", &data); //$MODIFIED: added "D:\\" at start
    if (handle == INVALID_HANDLE_VALUE)
    {
        DumpMessage("ERROR", "Can't find any car directories");
        QuitGame();
        return FALSE;
    }

    while (TRUE)
    {
        if (!FindNextFile(handle, &data))
            break;

        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (!strcmp(data.cFileName, "."))
            continue;

        if (!strcmp(data.cFileName, ".."))
            continue;

        sprintf(filename, "D:\\cars\\%s\\Parameters.txt", data.cFileName); //$MODIFIED: added "D:\\" at start
        fp = fopen(filename, "r");
        if (fp == NULL)
            continue;

        // make sure its not a default car
        bool newCar = TRUE;
        for (iCar = 0; iCar < CARID_NTYPES; iCar++) {
            if (_strnicmp(&CarDirs[iCar][8], data.cFileName, strlen(CarDirs[iCar])) == 0) { //$MODIFIED: changed strnicmp to _strnicmp, and changed [5] to [8] to account for "D:\" at start of CarDirs entries
                newCar = FALSE;
                fclose(fp);
                break;
            }
        }

        if (newCar) {
            NCarTypes++;
        }

        fclose(fp);

    }
    FindClose(handle);

    //$TODO(MD): Add enumeration of downloaded cars here

    // Make sure there is at least enough space for the default cars (shouldn't be needed)
    if (NCarTypes < CARID_NTYPES)
        NCarTypes = CARID_NTYPES;

    // Create the carinfo array
    if (CarInfo == NULL) {
        CarInfo = CreateCarInfo(NCarTypes);
        InitCarInfo(CarInfo, NCarTypes);
    }
    CarList = (int *)malloc(sizeof(int) * NCarTypes);

    ////////////////////////////////////////////////////////////
    // Load in the default cars
    //
    for (iCar = 0; iCar < CARID_NTYPES; iCar++) {

        // Build the filename
        sprintf(filename, "%s\\Parameters.txt", CarDirs[iCar]);

        // Open the file (note: must be binary mode for Checksum to work
        if ((fp = fopen(filename, "rb")) == NULL) {
            continue;
        }

        // Check the checksum
        // $MD: remove for now
        //if (!CheckStreamChecksum(fp, FALSE)) {
        //    CarInfo[iCar].Modified = TRUE;
        //} else {
        //    CarInfo[iCar].Modified = FALSE;
        //}

        // Re-open the file in ascii mode
        if ((fp = freopen(filename, "r", fp)) == NULL) {
            char message[256];
            sprintf(message, "Could not load parameters file:\n\"%s\"\n", filename);
            DumpMessage("Warning", message);
            continue;
        }

        // Read info into array
        CarListSize = 1;
        CarList[0] = iCar;
        ReadCarInfo(fp, "D:");

        // Make sure that the file had not been moved from one directory into another
        // $MD: checks bitmap dir with car dir
        // $MD: remove for now
        //if (_strnicmp(CarInfo[iCar].TPageFile, CarDirs[iCar], strlen(CarDirs[iCar])) != 0) { //$MODIFIED: changed strnicmp to _strnicmp
        //    CarInfo[iCar].Moved = TRUE;
        //    CarInfo[iCar].Modified = TRUE;
        //}

        fclose(fp);

    }

    ////////////////////////////////////////////////////////////
    // Load in the extra cars
    //
    currentCar = CARID_NTYPES;

    handle = FindFirstFile("D:\\cars\\*.*", &data); //$MODIFIED: added "D:\\" at start
    while (TRUE)
    {
        if (currentCar >= NCarTypes)
            break;

        if (!FindNextFile(handle, &data))
            break;

        if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            continue;

        if (!strcmp(data.cFileName, "."))
            continue;

        if (!strcmp(data.cFileName, ".."))
            continue;

        // Build the filename
        sprintf(filename, "D:\\cars\\%s\\Parameters.txt", data.cFileName); //$MODIFIED: added "D:\\" at start

        // Open the file (note: must be binary mode for Checksum to work
        if ((fp = fopen(filename, "rb")) == NULL) {
            continue;
        }

        // Check the checksum
        // $MD: remove for now
        //if (!CheckStreamChecksum(fp, FALSE)) {
        //    CarInfo[currentCar].Modified = TRUE;
        //} else {
        //    CarInfo[currentCar].Modified = FALSE;
        //}

        // Re-open the file in ascii mode
        if ((fp = freopen(filename, "r", fp)) == NULL) {
            fclose(fp);
            continue;
        }

        // make sure its not a default car
        bool newCar = TRUE;
        for (iCar = 0; iCar < CARID_NTYPES; iCar++) {
            if (_strnicmp(&CarDirs[iCar][8], data.cFileName, strlen(CarDirs[iCar])) == 0) { //$MODIFIED: changed strnicmp to _strnicmp, and changed [5] to [8] to account for "D:\" at start of CarDirs entries
                newCar = FALSE;
                fclose(fp);
                break;
            }
        }

        if (newCar) {
            // New car, so load it
            CarListSize = 1;
            CarList[0] = currentCar;
            ReadCarInfo(fp, "D:");
        } else {
            // Normal car - skip
            fclose(fp);
            continue;
        }

        // Make sure that the file had not been moved from one directory into another
        // $MD: checks bitmap dir with car dir
        // $MD: remove for now
        //sprintf(filename, "D:\\cars\\%s", data.cFileName); //$MODIFIED: added "D:\\" at start
        //if (_strnicmp(CarInfo[currentCar].TPageFile, filename, strlen(filename)) != 0) { //$MODIFIED: changed strnicmp to _strnicmp
        //    CarInfo[currentCar].Moved = TRUE;
        //    CarInfo[currentCar].Modified = TRUE;
        //}

        currentCar++;

        fclose(fp);     

    }
    FindClose(handle);

    return TRUE;
*/


void InitCarInfo(CAR_INFO *allCarInfo, int nCars)
{
    int iCar;
    CAR_INFO *carInfo;

    for (iCar = 0; iCar < nCars; iCar++) {
        carInfo = &allCarInfo[iCar];

        carInfo->ObtainMethod = CAR_OBTAIN_NEVER;
        carInfo->Rating = CAR_RATING_PRO;
        carInfo->Class = CAR_CLASS_OTHER;
        carInfo->Acc = 0;
        carInfo->Handling = 0;
        carInfo->TopEnd = 0;
        carInfo->Trans = 0;
        carInfo->Weight = 0;
        carInfo->Modified = FALSE;
        carInfo->Moved = FALSE;
        carInfo->Name[0] = '\0';
        carInfo->Selectable = FALSE;

    }

}

