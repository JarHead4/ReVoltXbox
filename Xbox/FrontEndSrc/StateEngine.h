//-----------------------------------------------------------------------------
// File: StateEngine.h
//
// Desc: Base class for managing the state engines that are used to implement
//       the UI
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef STATEENGINE_H
#define STATEENGINE_H




class CStateEngine;
extern CStateEngine* g_pActiveStateEngine;


//-----------------------------------------------------------------------------
// States for the state engine
//-----------------------------------------------------------------------------
enum
{
	STATEENGINE_STATE_BEGIN = 0,
};




//-----------------------------------------------------------------------------
// Name: class CStateEngine
// Desc: Base class for managing the state engines that are used to implement
//       the UI
//-----------------------------------------------------------------------------
class CStateEngine
{
protected:
	CStateEngine* m_pParent;
	DWORD         m_State;

public:
	virtual HRESULT Process() = 0;

	VOID MakeActive( CStateEngine* pParent )
	{
		g_pActiveStateEngine = this;
		m_pParent            = pParent;
		m_State              = STATEENGINE_STATE_BEGIN;
	}

	VOID Call( CStateEngine* pNext )
	{
		pNext->MakeActive( this );
	}

	VOID Return()
	{
		g_pActiveStateEngine = m_pParent;
	}
};


	

#endif // STATEENGINE_H
