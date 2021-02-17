//-----------------------------------------------------------------------------
// File: Menu.h
//
// Desc: Menu object
//
// Hist: 01.29.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef REVOLT_MENU_H
#define REVOLT_MENU_H

#include "Common.h"
#include <vector>
#include <string>

// Forward declarations
class MenuItem;




//-----------------------------------------------------------------------------
// Name: class Menu
// Desc: Menu object
//-----------------------------------------------------------------------------
class Menu
{
    static const DWORD EndMenu = DWORD(-1);

    // The list of menu items
    typedef std::vector< MenuItem > MenuList;
    MenuList     m_Menu;

    DWORD        m_dwID;        // unique menu identifier
    std::wstring m_strTitle;    // menu header
    DWORD        m_dwCurrPos;   // current position in menu (cursor)

public:

    Menu( DWORD = 0, const std::wstring& = std::wstring() );
    Menu( const Menu& );
    Menu& operator=( const Menu& );
    ~Menu();

    VOID SetID( DWORD dwID );
    VOID SetTitle( const std::wstring& );
    VOID AddItem( const MenuItem&, DWORD dwPos = EndMenu );
    VOID RemoveItem( const MenuItem& );
    VOID RemoveItem( DWORD dwPos );
    VOID RemoveAll();
    VOID SetCurrPos( DWORD dwPos );
    VOID SetCurrByID( DWORD dwID );
    VOID MoveUp();
    VOID MoveDown();

    DWORD GetID() const;
    const WCHAR* GetTitle() const;
    DWORD GetCurrPos() const;

    MenuItem& GetItem( DWORD dwPos );
    const MenuItem& GetItem( DWORD dwPos ) const;
    MenuItem& GetCurrItem();
    const MenuItem& GetCurrItem() const;

    DWORD GetCount() const;
};




#endif // REVOLT_MENU_H
