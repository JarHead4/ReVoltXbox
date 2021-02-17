
#ifndef DXERRORS_H
#define DXERRORS_H

// macros

typedef struct {
	HRESULT Result;
	char *Error;
} ERRORDX;

// globals

extern ERRORDX ErrorListDX[];

#endif
