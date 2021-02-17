//-----------------------------------------------------------------------------
// File: ui_StateEngine.h
//
// Desc: Base class for managing the state engines that are used to implement
//       the UI
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_STATEENGINE_H
#define UI_STATEENGINE_H

#include <assert.h>
#include "ui_menu.h"         // for MENU definition
#include "ui_TitleScreen.h"  // for g_pTitleScreenCamera


class CStateEngine;
extern CStateEngine* g_pActiveStateEngine;


//-----------------------------------------------------------------------------
// States for state engines
//-----------------------------------------------------------------------------
enum
{
    STATEENGINE_STATE_BEGIN = 0,
    STATEENGINE_STATE_EXIT  = ~0, //$REVISIT: Is this always safe?  Should we use "-1" or "0xFFFFFFFF" instead?
};




//-----------------------------------------------------------------------------
// Competion states for state engines
//-----------------------------------------------------------------------------
enum
{
    STATEENGINE_TERMINATED = -1, //$REVISIT: might rename to _BACK(ED)OUT or _EXITTOPARENT
    STATEENGINE_PROCESSING =  0,
    STATEENGINE_COMPLETED  =  1, //$REVISIT: might rename to _PROCEED(ED) or _EXITTOCHILD
};




//-----------------------------------------------------------------------------
// Name: class CStateEngine
// Desc: Base class for managing a state engine
//-----------------------------------------------------------------------------
class CStateEngine
{
public:
    inline CStateEngine* GetParent() { return m_pParent; }
    virtual WCHAR*       DebugGetName() = 0;
    
protected:
    CStateEngine*       m_pParent;
    DWORD               m_State;

    // Internal function to move the state engine to another state, and
    // immediately start processing that new state.
    virtual HRESULT GotoState( DWORD dwState )
    {
        m_State = dwState;
        return Process();
    }

    // Functions that will get called whenever we enter/exit a state
    // engine.  Having a single entry/exit point is important for doing
    // certain init/uninit work correctly.
    virtual VOID HandleEnterFromParent() {}
    virtual VOID HandleExitToParent()    {}
    virtual VOID HandleEnterFromChild() {}
    virtual VOID HandleExitToChild()    {}

public:
    // Public function to call a child state engine and pass control to the
    // new state engine.
    virtual VOID Call( CStateEngine* pNext )
    {
        pNext->MakeActive( this );
    }

    // Public function to make this state engine the currently active one.
    virtual VOID MakeActive( CStateEngine* pParent = g_pActiveStateEngine )
    {
#if _DEBUG
        // We'd better not push the same engine on the stack twice,
        // or all hell will break lose
        CStateEngine* pTraverse = pParent;
        while( pTraverse != NULL )
        {
            assert( pTraverse != this );
            pTraverse = pTraverse->m_pParent;
        }
#endif // _DEBUG

        if( NULL != pParent ) pParent->HandleExitToChild();
        g_pActiveStateEngine = this;
        this->HandleEnterFromParent();
        m_pParent            = pParent;
        m_State              = STATEENGINE_STATE_BEGIN;
    }

    // Public function to tell this state engine to return (i.e. give up
    // control) to its parent state engine.
    virtual VOID Return()
    {
        // We assert that this state engine is the active state engine.
        // Otherwise this function could accidentally bypass cleanup code in
        // a child state engine!
        ///$NOTE: For example, if you call a function like JoinMessage, and it
        /// activates a ShowSimpleMessage state engine, and then you call
        /// Return when JoinSession returns.
        assert( this == g_pActiveStateEngine );
        this->HandleExitToParent();
        g_pActiveStateEngine = m_pParent;
        if( NULL != m_pParent ) m_pParent->HandleEnterFromChild();
    }

    // Public function to implement logic based on the current state of this
    // state engine.
    virtual HRESULT Process() = 0;
};




//-----------------------------------------------------------------------------
// Name: class CStateEngineWithStatus
// Desc: Base class for managing a state engine
//-----------------------------------------------------------------------------
class CStateEngineWithStatus : public CStateEngine
{
public:
    inline  DWORD  GetStatus()    { return m_dwStatus; };
    virtual WCHAR* DebugGetName() { return L"<unknown>"; }
    
protected:
    DWORD m_dwStatus;

public:
    virtual HRESULT Process() = 0;

    VOID NextState( int eNextState)
    {
        m_State = eNextState;
    }

    virtual VOID MakeActive( CStateEngine* pParent = g_pActiveStateEngine )
    {
        m_dwStatus = STATEENGINE_PROCESSING;
        CStateEngine::MakeActive( pParent );
    }

    virtual VOID Return( DWORD dwExitStatus = STATEENGINE_COMPLETED )
    {
        m_dwStatus = dwExitStatus;
        CStateEngine::Return();
    }
};


    


//-----------------------------------------------------------------------------
// Name: class CUIStateEngine
// Desc: Base class for managing the state engines that are used to implement
//       the UI
//-----------------------------------------------------------------------------
class CUIStateEngine : public CStateEngineWithStatus
{
protected:
    LONG   m_OldCamPosIndex;
    MENU*  m_pOldMenu;

public:
    virtual VOID MakeActive( CStateEngine* pParent = g_pActiveStateEngine )
    {
        // Save the current camera and menu states
        m_OldCamPosIndex = g_pTitleScreenCamera->GetPosIndex();
        m_pOldMenu       = g_pMenuHeader->m_pNextMenu;

        // Call the base class function to make this state engine active
        CStateEngineWithStatus::MakeActive( pParent );
    }

    virtual VOID Return( DWORD dwExitStatus = STATEENGINE_COMPLETED )
    {
        // Restore camera and menu states
        g_pTitleScreenCamera->SetNewPos( m_OldCamPosIndex );
        g_pMenuHeader->SetNextMenu( m_pOldMenu );

        // Call the base class function to return from this state engine
        CStateEngineWithStatus::Return( dwExitStatus );
    }
};


    


#endif //UI_STATEENGINE_H

