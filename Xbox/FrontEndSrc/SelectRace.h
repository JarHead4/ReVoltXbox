//-----------------------------------------------------------------------------
// File: SelectRace.h
//
// Desc: 
//
// Copyright (c)  Microsoft Corporation.  All rights reserved.
//-----------------------------------------------------------------------------
#ifndef SELECTRACE_H
#define SELECTRACE_H




//-----------------------------------------------------------------------------
// Select Race state engine
//-----------------------------------------------------------------------------
class CSelectRaceStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
};


// Access to the Select Race state engine
extern CSelectRaceStateEngine g_SelectRaceStateEngine;




#endif // SELECTRACE_H
