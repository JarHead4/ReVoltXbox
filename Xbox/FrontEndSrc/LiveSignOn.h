//-----------------------------------------------------------------------------
// File: LiveSignOn.h
//
// Desc: 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef LIVESIGNON_H
#define LIVESIGNON_H




//-----------------------------------------------------------------------------
// Live Sign On state engine
//-----------------------------------------------------------------------------
class CLiveSignOnStateEngine : public CStateEngine
{
public:
	virtual HRESULT Process();
};


extern CLiveSignOnStateEngine g_LiveSignOnStateEngine;




#endif //  LIVESIGNON_H
