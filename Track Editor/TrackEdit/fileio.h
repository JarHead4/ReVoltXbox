#ifndef _FILELOAD_H
#define _FILELOAD_H

//Function prototypes exported from fileload.cpp

#include "UnitInfo.h"

using namespace std;

bool ReadRTUFile(const string& pathname, TRACKTHEME* theme);
bool NamedFileExists(const char* filename);
bool GetFileDescription(const char* filename, char* description);
bool DescribedFileExists(const char* description);
bool GetDescribedFilename(const char* description, char* filename);
bool NextFreeFile(char* filename);
bool NextFreeDescription(char* description);
bool NamedLevelExists(const char* levelname);
bool GetLevelDescription(const char* levelname, char* description);
bool GetDescribedLevelName(const char* description, char* levelname);
bool DescribedLevelExists(const char* description);
bool NextFreeLevel(char* levelname);
bool WriteTDF(const char* filename, const TRACKDESC* track);
bool ReadTDF(const char* filename, TRACKDESC* track);
bool MakeFileList(void);
void EraseFileList(void);

//const U16 RTU_READ_VERSION = 1;		//now includes UV information
//const U16 RTU_READ_VERSION = 2;		//now includes surface material information
//const U16 RTU_READ_VERSION = 3;		//now includes track zone information
const U16 RTU_READ_VERSION = 4;		//no longer includes VALID_EXITS info
const U16 TDF_READ_VERSION = 1;		



#endif //_FILELOAD_H