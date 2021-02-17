//-----------------------------------------------------------------------------
// File: Menu.cpp
//
// Desc: Menu code
//
// Hist: 01.29.02 - Created 
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "Menu.h"
#include "MenuItem.h"
#include <algorithm>




//-----------------------------------------------------------------------------
// Local struct for searching for menu item by ID
//-----------------------------------------------------------------------------
struct ById
{
    DWORD m_dwID;

    ById( DWORD dwID )
    :
        m_dwID( dwID )
    {
    }

    bool operator()( const MenuItem& menuItem ) const
    {
        return( menuItem.GetID() == m_dwID );
    }
};




//-----------------------------------------------------------------------------
// Name: Menu()
// Desc: Constructor
//-----------------------------------------------------------------------------
Menu::Menu( DWORD dwID, const std::wstring& strTitle )
:
    m_Menu(),
    m_dwID( dwID ),
    m_strTitle( strTitle ),
    m_dwCurrPos( 0 )
{
}




//-----------------------------------------------------------------------------
// Name: Menu()
// Desc: Copy Constructor
//-----------------------------------------------------------------------------
Menu::Menu( const Menu& menu )
:
    m_Menu( menu.m_Menu ),
    m_dwID( menu.m_dwID ),
    m_strTitle( menu.m_strTitle ),
    m_dwCurrPos( menu.m_dwCurrPos )
{
}



//-----------------------------------------------------------------------------
// Name: operator=()
// Desc: Assignment operator
//-----------------------------------------------------------------------------
Menu& Menu::operator=( const Menu& menu )
{
    Menu tmp( menu );
    m_Menu.swap( tmp.m_Menu );
    std::swap( m_dwID, tmp.m_dwID );
    m_strTitle.swap( tmp.m_strTitle );
    std::swap( m_dwCurrPos, tmp.m_dwCurrPos );
    return *this;
}




//-----------------------------------------------------------------------------
// Name: ~Menu()
// Desc: Destructor
//-----------------------------------------------------------------------------
Menu::~Menu()
{
}




//-----------------------------------------------------------------------------
// Name: SetID()
// Desc: Set menu ID
//-----------------------------------------------------------------------------
VOID Menu::SetID( DWORD dwID )
{
    m_dwID = dwID;
}




//-----------------------------------------------------------------------------
// Name: SetTitle()
// Desc: Set menu title
//-----------------------------------------------------------------------------
VOID Menu::SetTitle( const std::wstring& strTitle )
{
    m_strTitle = strTitle;
}




//-----------------------------------------------------------------------------
// Name: AddItem()
// Desc: Add item to menu at postion dwPos. If dwPos == EndMenu, item is added
//       to end.
//-----------------------------------------------------------------------------
VOID Menu::AddItem( const MenuItem& menuItem, DWORD dwPos )
{
    if( dwPos >= m_Menu.size() )
        m_Menu.push_back( menuItem );
    else
        m_Menu.insert( m_Menu.begin() + dwPos, menuItem );
}




//-----------------------------------------------------------------------------
// Name: RemoveItem()
// Desc: Remove given item from menu. O(N).
//-----------------------------------------------------------------------------
VOID Menu::RemoveItem( const MenuItem& menuItem )
{
    MenuList::iterator i = std::find( m_Menu.begin(), m_Menu.end(), menuItem );
    if( i != m_Menu.end() )
        m_Menu.erase( i );

    if( DWORD( i - m_Menu.begin() ) < m_dwCurrPos )
        --m_dwCurrPos;
}




//-----------------------------------------------------------------------------
// Name: RemoveItem()
// Desc: Remove the item at the given position. If dwPos == EndMenu, last
//       item is removed.
//-----------------------------------------------------------------------------
VOID Menu::RemoveItem( DWORD dwPos )
{
    assert( m_Menu.size() > 0 );
    if( dwPos >= m_Menu.size() )
        m_Menu.pop_back();
    else
        m_Menu.erase( m_Menu.begin() + dwPos );

    if( dwPos < m_dwCurrPos )
        --m_dwCurrPos;
}




//-----------------------------------------------------------------------------
// Name: RemoveAll()
// Desc: Remove the item at the given position. If dwPos == EndMenu, last
//       item is removed.
//-----------------------------------------------------------------------------
VOID Menu::RemoveAll()
{
    m_Menu.clear();
    m_dwCurrPos = 0;
}





//-----------------------------------------------------------------------------
// Name: SetCurrPos()
// Desc: Set the current position
//-----------------------------------------------------------------------------
VOID Menu::SetCurrPos( DWORD dwCurrPos )
{
    assert( dwCurrPos < m_Menu.size() );
    m_dwCurrPos = dwCurrPos;
}




//-----------------------------------------------------------------------------
// Name: SetCurrByID()
// Desc: Set the current position to the item with matching ID
//-----------------------------------------------------------------------------
VOID Menu::SetCurrByID( DWORD dwID )
{
    MenuList::iterator i = std::find_if( m_Menu.begin(), m_Menu.end(), ById( dwID ) );
    assert( i != m_Menu.end() );
    m_dwCurrPos = i - m_Menu.begin();
}




//-----------------------------------------------------------------------------
// Name: MoveUp()
// Desc: Move the cursor one item up
//-----------------------------------------------------------------------------
VOID Menu::MoveUp()
{
    do
    {
        if( m_dwCurrPos == 0 )
            m_dwCurrPos = GetCount() - 1;
        else
            --m_dwCurrPos;
    }
    while( !GetItem( m_dwCurrPos ).IsVisible() );
}




//-----------------------------------------------------------------------------
// Name: MoveDown()
// Desc: Move the cursor one item down
//-----------------------------------------------------------------------------
VOID Menu::MoveDown()
{
    do
    {
        ++m_dwCurrPos;
        if( m_dwCurrPos >= GetCount() )
            m_dwCurrPos = 0;
    }
    while( !GetItem( m_dwCurrPos ).IsVisible() );
}




//-----------------------------------------------------------------------------
// Name: GetID()
// Desc: Returns the unique menu ID
//-----------------------------------------------------------------------------
DWORD Menu::GetID() const
{
    return m_dwID;
}




//-----------------------------------------------------------------------------
// Name: GetTitle()
// Desc: Returns the menu title
//-----------------------------------------------------------------------------
const WCHAR* Menu::GetTitle() const
{
    return m_strTitle.c_str();
}




//-----------------------------------------------------------------------------
// Name: GetCurrPos()
// Desc: Returns current position
//-----------------------------------------------------------------------------
DWORD Menu::GetCurrPos() const
{
    return m_dwCurrPos;
}




//-----------------------------------------------------------------------------
// Name: GetItem()
// Desc: Returns the item at position dwPos.
//-----------------------------------------------------------------------------
MenuItem& Menu::GetItem( DWORD dwPos )
{
    assert( dwPos < m_Menu.size() );
    return m_Menu[ dwPos ];
}




//-----------------------------------------------------------------------------
// Name: GetItem()
// Desc: Returns the item at position dwPos
//-----------------------------------------------------------------------------
const MenuItem& Menu::GetItem( DWORD dwPos ) const
{
    assert( dwPos < m_Menu.size() );
    return m_Menu[ dwPos ];
}




//-----------------------------------------------------------------------------
// Name: GetCurrItem()
// Desc: Returns the item at the current position
//-----------------------------------------------------------------------------
MenuItem& Menu::GetCurrItem()
{
    return m_Menu[ m_dwCurrPos ];
}




//-----------------------------------------------------------------------------
// Name: GetCurrItem()
// Desc: Returns the item at the current position
//-----------------------------------------------------------------------------
const MenuItem& Menu::GetCurrItem() const
{
    return m_Menu[ m_dwCurrPos ];
}




//-----------------------------------------------------------------------------
// Name: GetCount()
// Desc: Returns the number of items in the menu
//-----------------------------------------------------------------------------
DWORD Menu::GetCount() const
{
    return m_Menu.size();
}




