//-----------------------------------------------------------------------------
// File: MenuItem.cpp
//
// Desc: Menu item code
//
// Hist: 01.29.02 - Created 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "MenuItem.h"




//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: MenuItem()
// Desc: Constructor
//-----------------------------------------------------------------------------
MenuItem::MenuItem()
:
    m_strText(),
    m_dwID( DWORD(-1) ),
    m_bIsVisible( TRUE )
{
}




//-----------------------------------------------------------------------------
// Name: MenuItem()
// Desc: Construct from menu data
//-----------------------------------------------------------------------------
MenuItem::MenuItem( const std::wstring& str, DWORD dwID )
:
    m_strText( str ),
    m_dwID( dwID ),
    m_bIsVisible( TRUE )
{
}




//-----------------------------------------------------------------------------
// Name: MenuItem()
// Desc: Copy Constructor
//-----------------------------------------------------------------------------
MenuItem::MenuItem( const MenuItem& menuItem )
:
    m_strText( menuItem.m_strText ),
    m_dwID( menuItem.m_dwID ),
    m_bIsVisible( menuItem.m_bIsVisible )
{
}




//-----------------------------------------------------------------------------
// Name: operator=()
// Desc: Assignment operator
//-----------------------------------------------------------------------------
MenuItem& MenuItem::operator=( const MenuItem& menuItem )
{
    m_strText = menuItem.m_strText;
    m_dwID = menuItem.m_dwID;
    m_bIsVisible = menuItem.m_bIsVisible;
    return *this;
}




//-----------------------------------------------------------------------------
// Name: ~MenuItem()
// Desc: Destructor
//-----------------------------------------------------------------------------
MenuItem::~MenuItem()
{
}




//-----------------------------------------------------------------------------
// Name: operator==()
// Desc: Comparison operator
//-----------------------------------------------------------------------------
bool MenuItem::operator==( const MenuItem& menuItem ) const
{
    return( m_strText == menuItem.m_strText && 
            m_dwID == menuItem.m_dwID );
}




//-----------------------------------------------------------------------------
// Name: SetText()
// Desc: Set menu item text
//-----------------------------------------------------------------------------
VOID MenuItem::SetText( const std::wstring& str )
{
    m_strText = str;
}




//-----------------------------------------------------------------------------
// Name: SetID()
// Desc: Set menu item ID
//-----------------------------------------------------------------------------
VOID MenuItem::SetID( DWORD dwID )
{
    m_dwID = dwID;
}




//-----------------------------------------------------------------------------
// Name: SetVisible()
// Desc: Set menu item visibility state
//-----------------------------------------------------------------------------
VOID MenuItem::SetVisible( BOOL bIsVisible )
{
    m_bIsVisible = bIsVisible;
}




//-----------------------------------------------------------------------------
// Name: GetText()
// Desc: Get menu item text
//-----------------------------------------------------------------------------
const WCHAR* MenuItem::GetText() const
{
    return m_strText.c_str();
}




//-----------------------------------------------------------------------------
// Name: GetID()
// Desc: Get menu item ID
//-----------------------------------------------------------------------------
DWORD MenuItem::GetID() const
{
    return m_dwID;
}




//-----------------------------------------------------------------------------
// Name: IsVisible()
// Desc: Get menu item visibility state
//-----------------------------------------------------------------------------
BOOL MenuItem::IsVisible() const
{
    return m_bIsVisible;
}
