//-----------------------------------------------------------------------------
// File: MenuItem.h
//
// Desc: Menu item object
//
// Hist: 01.29.02 - Created
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef REVOLT_MENU_ITEM_H
#define REVOLT_MENU_ITEM_H

#include "Common.h"
#include <string>




//-----------------------------------------------------------------------------
// Name: class MenuItem
// Desc: Menu item object
//-----------------------------------------------------------------------------
class MenuItem
{
    std::wstring m_strText;
    DWORD m_dwID;
    BOOL m_bIsVisible;

public:

    MenuItem();
    MenuItem( const std::wstring&, DWORD );
    MenuItem( const MenuItem& );
    MenuItem& operator=( const MenuItem& );
    ~MenuItem();

    bool operator==( const MenuItem& ) const;

    VOID SetText( const std::wstring& );
    VOID SetID( DWORD );
    VOID SetVisible( BOOL = TRUE );

    const WCHAR* GetText() const;
    DWORD GetID() const;
    BOOL IsVisible() const;
};




#endif // REVOLTUI_MENU_ITEM_H
