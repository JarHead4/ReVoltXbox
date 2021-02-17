//-----------------------------------------------------------------------------
// File: ui_Content.cpp
//
// Desc: content implimentation
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "revolt.h"
#include <xonline.h>
#include "content.h"
#include "main.h"
#include <string>


//-----------------------------------------------------------------------------
// Name: g_ContentManager
// Desc: global content manager
//-----------------------------------------------------------------------------
CContentManager            g_ContentManager;

//-----------------------------------------------------------------------------
// Name: g_bHasSubscrition
// Desc: do we have the premium subscription or not
// NOTE: set when you sign on and cleared when you sign off
//-----------------------------------------------------------------------------
BOOL g_bHasSubscription = FALSE;

//-----------------------------------------------------------------------------
// Name: g_bHadCorruptContent
// Desc: used as a signal that corrrupt content was detected and removed
//-----------------------------------------------------------------------------
BOOL g_bHadCorruptContent = FALSE;

//-----------------------------------------------------------------------------
// Name: CContentManager
// Desc: constructr
//-----------------------------------------------------------------------------
CContentManager::CContentManager()
: m_hContentTask(NULL),  // must init to null so Init() function will not assert
  m_dwBuildNumber(BUILD_NUMBER),
  m_bNewContent(FALSE) 
{
}

//-----------------------------------------------------------------------------
// Name: GetPackage
// Desc: Gets a package
//-----------------------------------------------------------------------------
ContentInfo* CContentManager::GetInfo( DWORD dwItem)
{
    if(dwItem < m_ContentInfoList.size())
        return &m_ContentInfoList[dwItem];
    return NULL;
}

//-----------------------------------------------------------------------------
// Name: GetPackage
// Desc: Gets a package
//-----------------------------------------------------------------------------
VOID CContentManager::RemoveInfo( DWORD dwItem)
{
    // assert that we are not doing something else
    assert( m_hContentTask == NULL);

    // assert that we have this itme
    assert( dwItem < m_ContentInfoList.size());

    // remove info
    m_ContentInfoList.erase(m_ContentInfoList.begin() + dwItem);
}

//-----------------------------------------------------------------------------
// Name: Init
// Desc: Initalizes the global intance
//-----------------------------------------------------------------------------
VOID CContentManager::Init()
{
    // assert that the task isn't open
    assert( m_hContentTask == NULL );   

    // assert the working set is empy
    assert (m_ContentInfoList.size() == 0);

    // $MD: $BUGBUG: the first controller isn't always logged on
    m_dwUserIndex = 0;

    m_dwWorkingItem = DWORD(-1);

    m_qwTotalBytes = 0;
    m_qwCurrentBytes = 0;
    m_bDownloadAll = FALSE;
    m_fDownloadPercent = 0.0f;

    m_bError = FALSE;

}

//-----------------------------------------------------------------------------
// Name: CleanUp
// Desc: Free's up the "majority" of memory used by the class
//-----------------------------------------------------------------------------
VOID CContentManager::CleanUp()
{
    // clear the content list
    m_ContentInfoList.clear();

    // free details blob
    m_WorkingDetails.FreeBlob();
    
    // NOTE car offerind id list is not cleared

    // close content task if needed
    // NOTE: this cleans up any memory reserver for download, enum, etc.
    if(m_hContentTask != NULL)
    {
        XOnlineTaskClose(m_hContentTask);
        m_hContentTask = NULL;
    }
}



//-----------------------------------------------------------------------------
// Name: Begin Enum
// Desc: begins content enumeration
//-----------------------------------------------------------------------------    
VOID CContentManager::BeginEnum( DWORD dwBitFilter, DWORD dwOfferingType, BOOL bIncludePay)
{
    // assert that we are not doing something else
    assert( m_hContentTask == NULL);

    // set up enum structure
    m_EnumParams.dwOfferingType = dwOfferingType;
    m_EnumParams.dwBitFilter = dwBitFilter; 
    //m_EnumParams.dwBitFilter = 0xffffffff; //0xffffffff for testing all offers
    m_EnumParams.wStartingIndex = 0; 
    m_EnumParams.wMaxResults = ENUM_REQUEST_SIZE; 

    //$MD: $BUGBUG: we need to support more languages so time in the future.
    //              the enum and data blobs need to be localized to more than
    //              just Japansese
    m_EnumParams.dwDescriptionIndex = 0; // ENGLISH
    if(XGetLanguage() == XC_LANGUAGE_JAPANESE)
        m_EnumParams.dwDescriptionIndex = 1; // JAPANESE
    

    // Determine the buffer size required for enumeration
    DWORD dwBufferSize = XOnlineOfferingEnumerateMaxSize( &m_EnumParams, 0 ); 

    // Initiate the enumeration on the selected device, using the
    // credentials of the user on the current controller
    HRESULT hr = XOnlineOfferingEnumerate(  
        m_dwUserIndex, &m_EnumParams, 
        NULL, dwBufferSize,
        NULL, &m_hContentTask );

    if( FAILED(hr) )
    {
#ifdef _DEBUG
        DumpMessage("Error", "Enumeration failed to start");
#endif
        Error(hr);
        return;
    }

    // don't download pay items
    m_bIncludePay = bIncludePay;
    return;
}

//-----------------------------------------------------------------------------
// Name: UpdateEnum
// Desc: Updates content enumeration
// Note: Intializes the enumeration also
//       Content is returned from server in newest to oldest order.
//         Once we see a peice of content we already own, we stop enumerating
//       We only update new content
//-----------------------------------------------------------------------------    
VOID CContentManager::UpdateEnum()
{
    // assert that we are doing something
    assert(m_hContentTask != NULL);

    HRESULT hr = XOnlineTaskContinue(m_hContentTask);
    if( hr != XONLINETASK_S_RUNNING )
    {
        // Handle pump errors
        if( FAILED(hr) )
        {
#ifdef _DEBUG
            DumpMessage("Error", "Enumeration failed");
#endif
            Error( hr );
            return;
        }
        
        // Extract the results
        PXONLINEOFFERING_INFO* ppInfo;
        DWORD dwItems;
        BOOL bPartial;
        
        hr = XOnlineOfferingEnumerateGetResults( m_hContentTask,
            &ppInfo, &dwItems, &bPartial );
        
        // Handle errors
        if( FAILED(hr) )
        {
#ifdef _DEBUG
            DumpMessage("Error", "Enumeration failed to get results");
#endif
            Error(hr);
            return;
        }
        
        // Save the results
        for( DWORD i = 0; i < dwItems; ++i )
        {
            // see if we already own this cotnet

            // $MD: $BUGBUG: hack to not enum the default content
            if((*ppInfo[i]).OfferingId & 0x00000000FFFF0000)
                continue;

            if(OwnContent((*ppInfo[i]).OfferingId))
            {
                continue;
            }

            // skip pay content if requrested
            if(!m_bIncludePay && !XOnlineOfferingIsFree((*ppInfo[i]).fOfferingFlags))
                continue;
            
            // if the content is new, add it to the working list
            m_ContentInfoList.resize(m_ContentInfoList.size() + 1);
            m_ContentInfoList.back().InitFromEnumInfo((*ppInfo[i]));
            
        }
        
        // If enumeration is not complete, continue enumerating
        if( bPartial)
        {
            // see if we have found anything that we already have.  if so, abort
            // Enumeration is not complete, keep pumping for more results
            return;
        }
        
        // Enumeration is complete
        XOnlineTaskClose(m_hContentTask);
        m_hContentTask = NULL;
    }
}


//-----------------------------------------------------------------------------
// Name: BeginDownloadCurrent
// Desc: Downloads a specific item
// Note: Parital downloads are left so we may resume later
//-----------------------------------------------------------------------------    
VOID CContentManager::BeginDownloadWorking( )
{
    // assert that we are not doing anything else
    assert( m_hContentTask == NULL);

    // assert that this 
    assert(m_dwWorkingItem < m_ContentInfoList.size() );

    XOFFERING_ID id = m_ContentInfoList[ m_dwWorkingItem ].GetId();
    
    // Initiate the installation of the selected content
    HRESULT hr = XOnlineContentInstall( id, NULL, &m_hContentTask );
    
    if( FAILED(hr) )
    {
#ifdef _DEBUG
        DumpMessage("Error", "Installation failed to start");
#endif
        Error(hr);
        return;
    }
}

//-----------------------------------------------------------------------------
// Name: BeginDownload
// Desc: Downloads a specific item
// Note: Parital downloads are left so we may resume later
//-----------------------------------------------------------------------------    
VOID CContentManager::BeginDownload( DWORD dwItem )
{
    assert(dwItem < m_ContentInfoList.size());
    
    m_dwWorkingItem = dwItem;
    
    m_fDownloadPercent = 0.0f;
    m_qwCurrentBytes = 0;
    m_qwTotalBytes = m_ContentInfoList[dwItem].GetPackageSize();

    m_bDownloadAll = FALSE;

    BeginDownloadWorking();
}

//-----------------------------------------------------------------------------
// Name: BeginDownload
// Desc: Downloads all items
// Note: Parital downloads are left so we may resume later
//-----------------------------------------------------------------------------    
VOID CContentManager::BeginDownloadAll()
{
    assert( m_ContentInfoList.size() != 0 );

    m_dwWorkingItem = 0;

    m_fDownloadPercent = 0.0f;
    m_qwCurrentBytes = 0;
    m_qwTotalBytes = 0;
    for(UINT i = 0; i < m_ContentInfoList.size(); i++)
    {
        m_qwTotalBytes += m_ContentInfoList[i].GetPackageSize();
    }

    m_bDownloadAll = TRUE;

    BeginDownloadWorking();

}

//-----------------------------------------------------------------------------
// Name: UpdateDownload
// Desc: Downloads everything in the working set
// Note: Parital downloads are left so we may resume later
//-----------------------------------------------------------------------------    
VOID CContentManager::UpdateDownload()
{
    // assert that we have somthing to do
    assert(m_hContentTask != NULL);

    // Determine the download progress
    DWORD dwPercent;
    ULONGLONG qwBytesInstalled;
    ULONGLONG qwBytesTotal;
    
    HRESULT hr = XOnlineContentInstallGetProgress( m_hContentTask,
        &dwPercent, &qwBytesInstalled, &qwBytesTotal );

    if( FAILED(hr) )
    {
        #ifdef _DUBUG
            DumpMessage("Error","Get download progress failed");
        #endif 
            Error(hr);
            return;
    }



    m_fDownloadPercent = (FLOAT(qwBytesInstalled + m_qwCurrentBytes) / FLOAT(m_qwTotalBytes)) * 100.0f;

   
    
    hr = XOnlineTaskContinue(m_hContentTask);
    if( hr != XONLINETASK_S_RUNNING )
    {
        // close taks
        XOnlineTaskClose(m_hContentTask);
        m_hContentTask = NULL;
        
        // Handle errors
        if( FAILED(hr) )
        {
#ifdef _DUBUG
            DumpMessage("Error","Installation failed");
#endif 
            Error(hr);
            return;
        }

        // NOTE: $BUGBUG: we assume that the content has been verified by the installer at this point.
        //                is this correct?
        AddOwnedContent(m_ContentInfoList[m_dwWorkingItem].GetId(),
                        m_ContentInfoList[m_dwWorkingItem].GetInstallDirectory(), false);

        if(m_bDownloadAll)
        {
            m_qwCurrentBytes += qwBytesInstalled;
            m_dwWorkingItem++;
            if( m_dwWorkingItem < m_ContentInfoList.size())
                BeginDownloadWorking();
            
        }
    }
}


//-----------------------------------------------------------------------------
// Name: BeginDetials
// Desc: Initiate the get details process
//-----------------------------------------------------------------------------
VOID CContentManager::BeginGetDetails( DWORD dwItem)
{
    // assert that we are not doing something else
    assert( m_hContentTask == NULL);

    // assert corrent index
    assert( dwItem < m_ContentInfoList.size());

    // set working item
    m_dwWorkingItem = dwItem;

    // Determine the buffer size required for details.  
    DWORD dwBufferSize = XOnlineOfferingDetailsMaxSize( 0 ); 
    
    
    //$MD: $BUGBUG: the desciption index is always 0
    //              it should be set using the language
    HRESULT hr = XOnlineOfferingDetails( m_dwUserIndex,
                                         m_ContentInfoList[m_dwWorkingItem].GetId(),
                                         XGetLanguage(), 0,
                                         NULL, dwBufferSize,
                                         NULL, &m_hContentTask );
    
    if( FAILED(hr) )
    {
#ifdef _DEBUG
        DumpMessage("Error", "Get Details failed to start" );
#endif
        Error(hr);
        return;
    }
}

//-----------------------------------------------------------------------------
// Name: UpdateStateGetDetails()
// Desc: Spin in get details
//-----------------------------------------------------------------------------
VOID CContentManager::UpdateGetDetails()
{
    // assert that we have somthing to do
    assert(m_hContentTask != NULL);
    
    HRESULT hr = XOnlineTaskContinue(m_hContentTask);
    if( hr != XONLINETASK_S_RUNNING )
    {
        // Handle pump errors
        if( FAILED(hr) )
        {
#ifdef _DEBUG
            DumpMessage("Error", "Get Details failed" );
#endif
            Error(hr);
            return;
        }
        
        // Extract the results
        XONLINEOFFERING_DETAILS Details;
        
        hr = XOnlineOfferingDetailsGetResults( m_hContentTask, &Details);
                
        // Handle errors
        if( FAILED(hr) )
        {
#ifdef _DEGUG
            DumpMessage("Error", "Get Details results failed" );
#endif
            Error(hr);
            return;
        }

        // save details 
        m_WorkingDetails.InitFromDetails( Details );
        
        // get details is complete
        XOnlineTaskClose(m_hContentTask);
        m_hContentTask = NULL;
    }
}

//-----------------------------------------------------------------------------
// Name: BeginPurchase()
// Desc: Initiate the purchase process
//-----------------------------------------------------------------------------
bool CContentManager::CurrentUserCanPurchase()
{
    XONLINE_USER* pUsers = XOnlineGetLogonUsers();

    if(FAILED(pUsers[m_dwUserIndex].hr))
        return false;

    return XOnlineIsUserPurchaseAllowed(pUsers[m_dwUserIndex].dwUserOptions); 
}


//-----------------------------------------------------------------------------
// Name: BeginPurchase()
// Desc: Initiate the purchase process
//-----------------------------------------------------------------------------
VOID CContentManager::BeginPurchase( DWORD dwItem )
{
    // assert that we are not doing something else
    assert( m_hContentTask == NULL);

    // assert corrent index
    assert( dwItem < m_ContentInfoList.size());

    // set working item
    m_dwWorkingItem = dwItem;

    HRESULT hr = XOnlineOfferingPurchase( m_dwUserIndex,
                                          m_ContentInfoList[m_dwWorkingItem].GetId(),
                                          NULL, &m_hContentTask );

    if( FAILED(hr) )
    {
#ifdef _DEBUG
        DumpMessage("Error", "Purchase failed to start" );
#endif
        Error(hr);
    }
}

//-----------------------------------------------------------------------------
// Name: UpdateStatePurchase()
// Desc: Spin during purchase
//-----------------------------------------------------------------------------
VOID CContentManager::UpdatePurchase() 
{
    // assert that we have somthing to do
    assert(m_hContentTask != NULL);
        
    HRESULT hr = XOnlineTaskContinue(m_hContentTask);
    if( hr != XONLINETASK_S_RUNNING )
    {
        // Handle errors
        if( FAILED(hr) )
        {
#ifdef _DEBUG
            DumpMessage("Error", "Purchase failed" );
#endif
            Error(hr);
            return;
        }

        // purchase is complete
        XOnlineTaskClose(m_hContentTask);
        m_hContentTask = NULL;


        
    }
}

//-----------------------------------------------------------------------------
// Name: CancelPurchase()
// Desc: Initiate the cancel purchase process
//-----------------------------------------------------------------------------
VOID CContentManager::BeginCancelSubscription( DWORD dwItem)
{
    // assert that we are not doing something else
    assert( m_hContentTask == NULL);

    // assert corrent index
    assert( dwItem < m_ContentInfoList.size());

    // set working item
    m_dwWorkingItem = dwItem;

    HRESULT hr = XOnlineOfferingCancel(m_dwUserIndex,
                                       m_ContentInfoList[m_dwWorkingItem].GetId(),
                                       NULL, &m_hContentTask);

    if( FAILED(hr) )
    {
#ifdef _DEBUG
        DumpMessage("Error", "Cancel subscritption failed to start" );
#endif
        Error(hr);
    }
}



//-----------------------------------------------------------------------------
// Name: UpdateStateCancelSub()
// Desc: Spin during cancel subscription
//-----------------------------------------------------------------------------
VOID CContentManager::UpdateCancelSubscription()
{
    // assert that we have somthing to do
    assert(m_hContentTask != NULL);

    HRESULT hr = XOnlineTaskContinue(m_hContentTask);
    if( hr != XONLINETASK_S_RUNNING )
    {
        // Handle errors
        if( FAILED(hr) )
        {
#ifdef _DEBUG
            DumpMessage("Error", "Cancel subscription failed" );
#endif
            Error(hr);
            return;
        }

        // cancel subscripton is complete
        XOnlineTaskClose(m_hContentTask);
        m_hContentTask = NULL;

        
    }
}

//-----------------------------------------------------------------------------
// Name: BeginError
// Desc: Begins the error state, sets a message
//-----------------------------------------------------------------------------  
VOID CContentManager::Error( HRESULT hr )
{
    WCHAR strBuf[20];
    swprintf(strBuf, L"HR=%08x", hr);
    m_Err = strBuf;
    m_bError = TRUE;
}


//-----------------------------------------------------------------------------
// Name: BeginError
// Desc: Begins the error state, sets a message
//-----------------------------------------------------------------------------  
VOID CContentManager::CancelTask()
{
    if(m_hContentTask != NULL)
    {
        XOnlineTaskClose(m_hContentTask);
        m_hContentTask = NULL;
    } 
}

//-----------------------------------------------------------------------------
// Name: AddContent
// Desc: Adds a pieceof content to the current content set
//-----------------------------------------------------------------------------
void CContentManager::AddOwnedContent(XOFFERING_ID Id, const char* szContentDirectory, bool bVerify)
{
    // check for content dups
    UINT uiNumContent = m_OwnedContent.size();
    for(UINT i = 0; i < uiNumContent; i++)
    {
        if(m_OwnedContent[i].GetId() == Id)
        {
#ifdef _DEBUG
            DumpMessageVar("Warning", "Duplicate content ID not added to content set: %I64X", Id);
#endif
            return;
        }
    }

    bool bCorrupt = false;

    if(bVerify)
    {
        bCorrupt = !VerifyContent(szContentDirectory);
#ifdef _DEBUG
        if(bCorrupt)
            DumpMessageVar("Warning", "Corrupt content detected: %I64X", Id);
#endif

    }

    m_OwnedContent.push_back(ContentOwned());
    m_OwnedContent.back().Init(Id, szContentDirectory, bCorrupt); 
    
}

//-----------------------------------------------------------------------------
// Name: AddContent
// Desc: recursivly verifys content in a directory
//-----------------------------------------------------------------------------
bool CContentManager::VerifyContent( const char* szContentDirectory )
{
    assert( szContentDirectory );

    // get signatures handle 
    HANDLE hSig = XLoadContentSignatures( szContentDirectory );
    if ( hSig == INVALID_HANDLE_VALUE )
    {
        Error( HRESULT_FROM_WIN32(GetLastError()) );
        return FALSE;
    }

    // create vector of search directories
    std::vector< std::string > Directories;

    // add initial search directory
    std::string RootDir( szContentDirectory );
    RootDir += "\\";
    Directories.push_back( RootDir );
    DWORD dwRootLen = RootDir.size();

    // check the content of root and child directories
    BOOL bSuccess = TRUE;
    while( Directories.size() > 0 && bSuccess )
    {
           
        std::string CurDir = Directories.back();
        Directories.pop_back();
        
        // find files
        WIN32_FIND_DATA FindData;
        HANDLE hFind = FindFirstFile( (CurDir + "*.*").c_str(), &FindData );
        if( hFind == INVALID_HANDLE_VALUE )
        {
            // no content in this directory
            continue;
        }

        do
        {
            // create local path name
            std::string LocalPath =
                CurDir.substr(dwRootLen) + std::string(FindData.cFileName);
            
            // create full path name
            std::string FullPath =
                CurDir + FindData.cFileName;

            // skip the metadata file
            if( _stricmp( FindData.cFileName, "contentmeta.xbx" ) == 0 )
                continue;
            
            // skip . and .. directories
            if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            {
                if(  FindData.cFileName[0] != '.' )
                {
                    FullPath += std::string("\\");
                    Directories.push_back( FullPath );
                    continue;
                }
            }
            
            // verify content of file
            HANDLE hFile = CreateFile( FullPath.c_str(), GENERIC_READ,
                                       FILE_SHARE_READ, NULL,
                                       OPEN_EXISTING,
                                       FILE_ATTRIBUTE_NORMAL, NULL );
            if( hFile != INVALID_HANDLE_VALUE )
            {
                // read in file
                DWORD dwFileSize = GetFileSize(hFile, NULL);
                
                //skip files of 0 sizle
                if(dwFileSize == 0)
                    continue;

                BYTE* pbyBuffer = new BYTE[dwFileSize];
                DWORD dwNumBytesRead;
                if(!ReadFile( hFile, pbyBuffer,
                              dwFileSize, &dwNumBytesRead, NULL ) )
                {
                    Error( HRESULT_FROM_WIN32(GetLastError()) );
                    bSuccess = FALSE;
                }
                else
                {
            
                    // locate signature
                    BYTE* pbySignature = NULL;
                    DWORD dwSignatureSize = XCALCSIG_SIGNATURE_SIZE;
                    if(! XLocateSignatureByName( hSig, LocalPath.c_str(), 
                                                 0, dwFileSize,
                                                 &pbySignature, &dwSignatureSize ) )
                    {
                        Error( HRESULT_FROM_WIN32(GetLastError()) );
                        bSuccess = FALSE;
                    }
                    else
                    {
                        // compare signatures
                        DWORD dwVerifySize =  XCALCSIG_SIGNATURE_SIZE;
                        BYTE abyVerify[XCALCSIG_SIGNATURE_SIZE];
                        if( !XCalculateContentSignature( pbyBuffer, dwNumBytesRead,
                                                     abyVerify, &dwVerifySize ) )
                        {
                            Error( HRESULT_FROM_WIN32(GetLastError()) );
                            bSuccess = FALSE;
                        }
                        else
                        {
                            if( ( dwVerifySize != dwSignatureSize ) ||
                                ( memcmp(abyVerify, pbySignature, dwVerifySize) != 0))
                            {
                                Error(ERROR_INVALID_DATA);
                                bSuccess = FALSE;
                            }
                        }
                    }
                }

                // cleanup 
                CloseHandle( hFile );
                delete [] pbyBuffer;
            
            }
            else
            {
                Error( ERROR_FILE_NOT_FOUND );
                bSuccess = FALSE;
            }
        
        } while( FindNextFile( hFind, &FindData ) && bSuccess );

        // cleanup
        CloseHandle( hFind );
    }

    XCloseContentSignatures( hSig );

    return bSuccess;
}


//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool CContentManager::OwnContent(XOFFERING_ID Id)
{
    // check for content dups
    UINT uiNumContent = m_OwnedContent.size();
    for(UINT i = 0; i < uiNumContent; i++)
    {
        if(m_OwnedContent[i].GetId() == Id)
            return true;
    }

    return false;
}

//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool CContentManager::IsOwnedContentCorrupted(XOFFERING_ID Id)
{
    // check for content dups
    UINT uiNumContent = m_OwnedContent.size();
    for(UINT i = 0; i < uiNumContent; i++)
    {
        if(m_OwnedContent[i].GetId() == Id && m_OwnedContent[i].IsCorrupt())
            return true;
        else
            return false;
    }

    return true;
}

//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool CContentManager::OwnCorruptContent()
{
    // check for content dups
    UINT uiNumContent = m_OwnedContent.size();
    for(UINT i = 0; i < uiNumContent; i++)
    {
        if(m_OwnedContent[i].IsCorrupt())
            return true;
    }

    return false;
}


//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool CContentManager::DeleteCorruptContent()
{
    bool bRemovedAll = true;
    
    for(UINT i = 0; i < m_OwnedContent.size(); i++)
    {
        if(m_OwnedContent[i].IsCorrupt())
        {
            if(!XRemoveContent(m_OwnedContent[i].GetInstallDirectory()))
            {
                // we have corrupt content but cannot remove it
                if(bRemovedAll)
                {
#ifdef _DEBUG
                    DumpMessageVar("Warning", "Could not remove corrupt content: %I64X", m_OwnedContent[i].GetId());
#endif
                    bRemovedAll = false;
                    Error(HRESULT_FROM_WIN32(GetLastError()));
                }
            }

            m_OwnedContent.erase(m_OwnedContent.begin() + i);
        }
            
    }

    return bRemovedAll;
}


