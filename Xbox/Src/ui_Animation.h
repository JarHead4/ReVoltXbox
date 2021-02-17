//-----------------------------------------------------------------------------
// File: ui_Animation.h
//
// Desc: 
//
// Re-Volt (Generic) Copyright (c) Probe Entertainment 1998
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef UI_ANIMATION_H
#define UI_ANIMATION_H

// misplaced externs
extern void DrawNewTitleBox(FLOAT xPos, FLOAT yPos, FLOAT xSize, FLOAT ySize, int col, int trans);
extern void DrawNewSpruBox( FLOAT, FLOAT, FLOAT, FLOAT );
extern void DrawNewSpruBoxWithTabs( FLOAT, FLOAT, FLOAT, FLOAT );


class CUIListHelper
{
public:
    int m_nNumberOfVisibleItems;
    int m_nFirstVisible;
    int m_nCurrentSelection;
    int m_nNumberInList;

    CUIListHelper()
    {
        Clear( 0 );
    }

    void Clear( int nNumberOfVisibleItems = -1)
    {
        if ( nNumberOfVisibleItems != -1 )
        {
            m_nNumberOfVisibleItems = nNumberOfVisibleItems;
        }

        m_nFirstVisible = 0;
        m_nCurrentSelection = 0;
        m_nNumberInList = 0;

    }

    bool ScrollUp()
    {
        if ( m_nCurrentSelection > 0 )
        {
            m_nCurrentSelection--;
            if ( m_nCurrentSelection < m_nFirstVisible )
            {
                m_nFirstVisible = m_nCurrentSelection;
                return TRUE;
            }
        }

        return FALSE;
    }

    bool ScrollDown()
    {
        if ( m_nCurrentSelection < m_nNumberInList - 1 )
        {
            m_nCurrentSelection++;

            if ( m_nCurrentSelection > ( m_nFirstVisible + m_nNumberOfVisibleItems - 1) )
            {
                m_nFirstVisible++;
                return TRUE;
            }
        }

        return FALSE;
    }
};

class CUIRenderContext
{
public:
    virtual bool Animate() = 0;
};

typedef std::list<CUIRenderContext*> RenderList;

class CUIRCSlidingAnimation : public CUIRenderContext
{
protected:
    float   m_fXShiftNow,   m_fYShiftNow;   // Current Screen offset
    float   m_fXShiftDone,  m_fYShiftDone;  // Screen offset when animation complete

    float   m_fXSize, m_fYSize;             // Current Screen Position and dimensions
    float   m_fXMin,  m_fYMin;          // Desired position and size
    float   m_fXMax,  m_fYMax;         // Desired position and size

    bool    m_bMeasureExtents;
    bool    m_bAllowRender;

    static const float c_fXScreenWidth;
    static const float c_fYScreenHeight;

public:
    CUIRCSlidingAnimation()
    {
        Reset();
    }

    void Reset()
    {
        m_fXShiftNow = 0;
        m_fXShiftDone = 0;
        m_fXMin = 0;
        m_fXMax = 0;
        m_fXSize = c_fXScreenWidth;

        m_fYShiftNow = 0;
        m_fYShiftDone = 0;
        m_fYMin = 0;
        m_fYMax = 0;
        m_fYSize = c_fYScreenHeight;

        m_bMeasureExtents = true;
        m_bAllowRender = true;
    }

    void UpdateExtents( float xPos, float yPos, float xSize, float ySize )
    {
        m_fXMin = min( m_fXMin, xPos );
        m_fXMax = max( m_fXMax, (xPos + xSize) );
        m_fXSize = m_fXMax - m_fXMin;

        m_fYMin = min( m_fYMin, yPos );
        m_fYMax = max( m_fYMax, (yPos + ySize) );
        m_fYSize = m_fYMax - m_fYMin;
    }

    void BeginEnterFromLeft( )
    {
        m_fXShiftNow = 0 - m_fXSize;
        m_fYShiftNow = 0;

        m_fXShiftDone = 0;
        m_fYShiftDone = 0;

    }

    void BeginExitToLeft()
    {
        m_fXShiftNow = 0;
        m_fYShiftNow = 0;

        m_fXShiftDone = 0 - m_fXSize;
        m_fYShiftDone = 0;

    }

    void BeginEnterFromRight()
    {
        m_fXShiftNow = c_fXScreenWidth - m_fXMin;
        m_fYShiftNow = 0;

        m_fXShiftDone = 0;
        m_fYShiftDone = 0;

    }

    void BeginExitToRight()
    {
        m_fXShiftNow = 0;
        m_fYShiftNow = 0;

        m_fXShiftDone = c_fXScreenWidth - m_fXMin;
        m_fYShiftDone = 0;

    }

    void BeginEnterFromBottom()
    {
        m_fXShiftNow = 0;
        m_fYShiftNow = c_fYScreenHeight - m_fYMin;

        m_fXShiftDone = 0;
        m_fYShiftDone = 0;

    }

    void BeginExitToBottom()
    {
        m_fXShiftNow = 0;
        m_fYShiftNow = 0;

        m_fXShiftDone = 0;
        m_fYShiftDone = c_fYScreenHeight - m_fYMin;

    }

    void BeginExitToTop()
    {
        m_fXShiftNow = 0;
        m_fYShiftNow = 0;

        m_fXShiftDone = 0;
        m_fYShiftDone = c_fYScreenHeight - m_fYSize;

    }

    virtual bool Animate()
    {
        bool  bReachedDest = true;
        float xVel = MENU_MOVE_VEL * TimeStep;
        float yVel = MENU_MOVE_VEL * TimeStep;

        // Move along the X axis
        if( m_fXShiftNow != m_fXShiftDone ) 
        {
            FLOAT delta = m_fXShiftDone - m_fXShiftNow;
            if( fabsf(delta) < xVel ) 
            {
                m_fXShiftNow = m_fXShiftDone;
            } 
            else 
            {
                if( delta < 0 ) 
                    m_fXShiftNow -= xVel;
                else 
                    m_fXShiftNow += xVel;
                bReachedDest = FALSE;
            }
        }

        // Move along the Y axis
        if( m_fYShiftNow != m_fYShiftDone ) 
        {
            FLOAT delta = m_fYShiftDone - m_fYShiftNow;
            if( fabsf(delta) < yVel ) 
            {
                m_fYShiftNow = m_fYShiftDone;
            } 
            else 
            {
                if( delta < 0 ) 
                    m_fYShiftNow -= yVel;
                else
                    m_fYShiftNow += yVel;
                bReachedDest = FALSE;
            }
        }

        return !bReachedDest;
    }

    void DrawNewTitleBox(float xPos, float yPos, float xSize, float ySize, int col, int trans)
    {
        if ( m_bMeasureExtents )
        {
            UpdateExtents( xPos, yPos, xSize, ySize );
        }

        if ( m_bAllowRender )
        {
            ::DrawNewTitleBox( m_fXShiftNow + xPos, m_fYShiftNow + yPos, xSize, ySize, col, trans );
        }
    }

    void DrawNewSpruBox(float xPos, float yPos, float xSize, float ySize )
    {
        if ( m_bMeasureExtents )
        {
            UpdateExtents( xPos, yPos, xSize, ySize );
        }

        if ( m_bAllowRender )
        {
            ::DrawNewSpruBox( m_fXShiftNow + xPos, m_fYShiftNow + yPos, xSize, ySize );
        }
    }

    HRESULT DrawText( CXBFont* pFont, float fOriginX, float fOriginY, DWORD dwColor,
                           const WCHAR* strText, DWORD dwFlags )
    {
        Assert( pFont );
        HRESULT hr = S_OK;
        if ( m_bMeasureExtents )
        {
            float fWidth;
            float fHeight;
            hr = pFont->GetTextExtent( strText, &fWidth, &fHeight, FALSE);
            UpdateExtents( fOriginX, fOriginY, fWidth, fHeight );
            m_bMeasureExtents = FALSE;  // extra execution time should be avoided, possible problems on dynamic text
        }

        if ( m_bAllowRender )
        {
            hr = pFont->DrawText( m_fXShiftNow + fOriginX, m_fYShiftNow + fOriginY, dwColor, strText, dwFlags );
        }

        return hr;
    }

    HRESULT DrawMenuText( float fOriginX, float fOriginY, DWORD dwColor,
                           const WCHAR* strText, DWORD dwFlags )
    {
        return DrawText( g_pFont, fOriginX, fOriginY, dwColor, strText, dwFlags );
    }

    HRESULT DrawMenuTitleText( float fOriginX, float fOriginY, DWORD dwColor,
                           const WCHAR* strText, DWORD dwFlags )
    {
        g_pFont->SetScaleFactors( 1.5f, 1.5f );
        HRESULT hr = DrawText( g_pFont, fOriginX, fOriginY, dwColor, strText, dwFlags );
        g_pFont->SetScaleFactors( 1.0f, 1.0f );
        return hr;
    }
};


class CUIAnimationQueue : public RenderList
{
public:
    bool Animate()
    {
        if ( empty() )
            return false;

        CUIRenderContext* pRC = front();
        Assert( pRC );

        if ( !pRC->Animate() )
        {
            // remove finished animation
            pop_front();
        }

        return true;
    }

    void Add( CUIRenderContext* pNewAnimation)
    {
        push_back( pNewAnimation );
    }
};
extern CUIAnimationQueue g_AnimationQueue;


class CUIScreen : public CUIStateEngine
{
public:
    CUIScreen()
    {
    }

    virtual HRESULT Process()
    {
        // Update the camera
        g_pTitleScreenCamera->Update();

        // Animate menus and handle input
        g_pMenuHeader->MoveResizeMenu();

        // Draw menu
        if( g_pMenuHeader->m_pMenu != NULL ) 
        {
            g_pMenuHeader->DrawMenuTitle();
            g_pMenuHeader->DrawMenu();
        }

        //g_AnimationQueue.Animate();

        //m_pActiveScreen->Process();
        MenuInputTypeEnum input = GetFullScreenMenuInput( );
        HandleInput( input );

        // if nothing in animation queue, we'd better draw ourselves
        if ( !g_AnimationQueue.Animate() )
            Draw( );

        return S_OK;
    }

    virtual WCHAR*  DebugGetName()   { return L"UIScreens"; }
    virtual bool HandleInput( MenuInputTypeEnum input) = 0;
    virtual void Draw() = 0;

};


class CUIRCStatPanel : public CUIRenderContext
{
public:
    CUIRCSlidingAnimation   m_Title;
    CUIRCSlidingAnimation   m_Panel;
    bool                    m_bDone;

    CUIScreen              *m_pUIThisScreen;    // UI to keep drawing during any transitory animations

    CUIRCStatPanel( )
    {
        m_pUIThisScreen = NULL;
    }

    void Init( CUIScreen *pUIThisScreen )
    {
        m_pUIThisScreen = pUIThisScreen;
    }

    void BeginEnterFromSides()
    {
        Assert( m_pUIThisScreen );
        m_Title.BeginEnterFromLeft();
        m_Panel.BeginEnterFromRight();
        g_AnimationQueue.Add( this );
    }

    void BeginExitToSides()
    {
        Assert( m_pUIThisScreen );
        m_Title.BeginExitToLeft();
        m_Panel.BeginExitToRight();
        g_AnimationQueue.Add( this );
    }


    void BeginEnterFromBottom()
    {
        Assert( m_pUIThisScreen );
        m_Title.BeginEnterFromBottom();
        m_Panel.BeginEnterFromBottom();
        g_AnimationQueue.Add( this );
    }


    void BeginExitToBottom()
    {
        Assert( m_pUIThisScreen );
        m_Title.BeginExitToBottom();
        m_Panel.BeginExitToBottom();
        g_AnimationQueue.Add( this );
    }

    virtual bool Animate()
    {
        int nNotDone = 0;

        if ( m_Title.Animate() )
            nNotDone++;

        if ( m_Panel.Animate() )
            nNotDone++;

        m_bDone = (nNotDone == 0);

        if ( m_pUIThisScreen )
            m_pUIThisScreen->Draw();

        return !m_bDone;
    }
};


extern CUIAnimationQueue g_AnimationQueue;

#endif // UI_ANIMATION_H
