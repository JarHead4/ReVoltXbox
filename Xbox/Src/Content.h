//-----------------------------------------------------------------------------
// File: Common.h
//
// Desc: content download global classes, constants, etc
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#ifndef CONTENTDOWNLOAD_COMMON_H
#define CONTENTDOWNLOAD_COMMON_H

#include "XBOnline.h"

// we have DWORD, right?
#define QWORD ULONGLONG

//-----------------------------------------------------------------------------
// constants
//-----------------------------------------------------------------------------
#define CONTENT_MANDATORY_FLAG    1  // bit flag for mandatory content
#define CONTENT_CAR_FLAG          2  // bit flag for cars
#define CONTENT_LEVEL_FLAG        4  // bit flag for levels
#define CONTENT_CARKEY_FLAG       8  // bit flag for car keys
#define CONTENT_LEVELKEY_FLAG     16 // bit flag for level keys

#define ENUM_REQUEST_SIZE   5           // number of content enum data
                                        //    structures to request at a time
// $MD: $BUGBUG: is this duped anywhere else??
#define BUILD_NUMBER        0           // current build number

// $MD: Added
// preimum subscription serivce id
#define REVOLT_SUBSCRIPTION_ID 0x584C0001


//-----------------------------------------------------------------------------
// Name: g_bHasSubscrition
// Desc: do we have the premium subscription or not
// NOTE: set when you sign on and cleared when you sign off
//-----------------------------------------------------------------------------
extern BOOL g_bHasSubscription;

//-----------------------------------------------------------------------------
// Name: g_bHadCorruptContent
// Desc: used as a signal that corrrupt content was detected and removed
//-----------------------------------------------------------------------------
extern BOOL g_bHadCorruptContent;


//-----------------------------------------------------------------------------
// Name: struct Content
// Desc: Describes a particular piece of content
//-----------------------------------------------------------------------------
class ContentOwned
{
public:
    VOID Init(XOFFERING_ID id, const char* szInstallDirectory, bool bCorrupt)
    {
        m_Id = id;
        m_InstallDirectory = szInstallDirectory;
        m_bCorrupt = bCorrupt;
    }

    const char*     GetInstallDirectory() {return m_InstallDirectory.c_str();}
    XOFFERING_ID    GetId() {return m_Id;}
    bool            IsCorrupt() {return m_bCorrupt;}

private:
        
    XOFFERING_ID    m_Id;
    std::string     m_InstallDirectory;
    bool            m_bCorrupt;
};

//-----------------------------------------------------------------------------
// Name: class ContentInfo
// Desc: Content information from the functions; simplifies access to
//-----------------------------------------------------------------------------
class ContentInfo
{
public:
    XOFFERING_ID                m_ID;
    DWORD                       m_dwRating;
    DWORD                       m_dwOfferingType;
    DWORD                       m_dwBitFlags;
    WORD                        m_wOfferingFlags;
    
    DWORD                       m_dwPackageSize;   // package size (bytes)
    DWORD                       m_dwInstallSize;   // size of package on disk
                                                   //   (blocks)
    FILETIME                    m_ftActivationDate;// When this content was
                                                   // activated
    FILETIME                    m_ftDownloadDate;  // When this content was
                                                   // downloaded
    std::basic_string< BYTE >   m_EnumBlob;        // Title-specific enum data
    std::string                 m_InstallDirectory; // directory content was
    std::wstring                m_DisplayName;      // content display name


public:

    VOID InitFromContentFindData( const XCONTENT_FIND_DATA& xContentFindData)
    {
        m_ID                = xContentFindData.qwOfferingId;
        m_dwBitFlags        = xContentFindData.dwFlags;
        m_ftDownloadDate    = xContentFindData.wfd.ftCreationTime;
        m_dwInstallSize     = XGetDisplayBlocks( xContentFindData.szContentDirectory );
        m_InstallDirectory  = xContentFindData.szContentDirectory;
        m_DisplayName       = xContentFindData.szDisplayName;
    }


    VOID InitFromEnumInfo( const XONLINEOFFERING_INFO& xOnInfo )
    {
        m_dwPackageSize    = xOnInfo.dwPackageSize;
        m_dwInstallSize    = xOnInfo.dwInstallSize; 
        m_dwOfferingType   = xOnInfo.dwOfferingType;
        m_dwBitFlags       = xOnInfo.dwBitFlags;
        m_wOfferingFlags   = xOnInfo.fOfferingFlags;
        m_dwRating         = xOnInfo.dwRating; 
        m_ftActivationDate = xOnInfo.ftActivationDate; 
        m_ID               = xOnInfo.OfferingId; 
        m_EnumBlob.assign( xOnInfo.pbTitleSpecificData, xOnInfo.dwTitleSpecificData);

        char szBuf[MAX_PATH];
        sprintf(szBuf, "T:\\$C\\%I64X", m_ID);
        m_InstallDirectory = szBuf;
    }

    DWORD GetPackageSize() const       { return m_dwPackageSize; }
    DWORD GetInstallSize() const       { return m_dwInstallSize; }
    DWORD GetOfferingType() const      { return m_dwOfferingType; }
    DWORD GetBitFlags() const          { return m_dwBitFlags; }
    WORD GetOfferingFlags() const      { return m_wOfferingFlags;}

    DWORD GetRating() const            { return m_dwRating; }
    FILETIME GetActivationDate() const { return m_ftActivationDate; }
    FILETIME GetDownloadDate() const   { return m_ftDownloadDate; }
    XOFFERING_ID GetId() const         { return m_ID; }
    const BYTE* GetEnumBlob() const    { return &(*m_EnumBlob.begin());}
    DWORD GetEnumBlobSize() const      { return m_EnumBlob.size();}

    const CHAR* GetInstallDirectory() { return m_InstallDirectory.c_str(); }
    const WCHAR* GetDisplayName()     { return m_DisplayName.c_str(); }
};


//-----------------------------------------------------------------------------
// Name: class ContentDetails
// Desc: Content information from the functions; simplifies access to
//-----------------------------------------------------------------------------
class ContentDetails
{
    std::basic_string< BYTE >   m_DetailsBlob;     // Title-specific details data
    DWORD                       m_dwNumInstances;  // Number of instances already owned
    XONLINE_PRICE               m_Price;
    DWORD                       m_dwFreeMonths;     // free months before charge
    DWORD                       m_dwDuration;
    XONLINE_OFFERING_FREQUENCY  m_Frequency;        // how often charges are made
public:

    VOID InitFromDetails( const XONLINEOFFERING_DETAILS& xOnDetails)
    {
        m_dwNumInstances = xOnDetails.dwInstances;
        m_Price          = xOnDetails.Price;
        m_dwFreeMonths   = xOnDetails.dwFreeMonthsBeforeCharge;
        m_dwDuration     = xOnDetails.dwDuration;
        m_Frequency      = xOnDetails.Frequency;
        m_DetailsBlob.assign( xOnDetails.pbDetailsBuffer, xOnDetails.dwDetailsBuffer);
    }
    
    DWORD GetNumInstances() const               { return m_dwNumInstances; }
    XONLINE_PRICE& GetPrice()                   { return m_Price; }
    DWORD GetFreeMonths() const                 { return m_dwFreeMonths; }
    DWORD GetDuration() const                   { return m_dwDuration; }
    const XONLINE_OFFERING_FREQUENCY& GetFrequency()  { return m_Frequency; }
    const BYTE* GetDetailsBlob() const          { return &m_DetailsBlob[0];}
    DWORD GetDetailsBlobSize() const            { return m_DetailsBlob.size();}

    VOID  FreeBlob() {m_DetailsBlob.erase();}
};


//-----------------------------------------------------------------------------
// Name: CContentManager
// Desc: Handles content enum and download
//-----------------------------------------------------------------------------
class CContentManager
{
public:
    CContentManager();

    // init and cleanup
    VOID    Init();
    VOID    CleanUp();

    // working
    BOOL    Working()                 {return m_hContentTask != NULL;}

    // cancels current working task
    VOID    CancelTask();

    // enum 
    VOID    BeginEnum( DWORD dwBitFilter, DWORD dwContentType, BOOL bIncludePay);
    VOID    UpdateEnum();

    // new content 
    // NOT WORKING
    //VOID    BeginCheckForNew();
    //VOID    UpdateCheckForNew();
    //BOOL    NewContentAvailable() {return m_bNewContent;}
    
    // details
    VOID    BeginGetDetails( DWORD dwItem );
    VOID    UpdateGetDetails( );

    // purchase
    bool    CurrentUserCanPurchase();
    VOID    BeginPurchase( DWORD dwItem );
    VOID    UpdatePurchase();

    // cancel sub
    VOID    BeginCancelSubscription( DWORD dwItem);
    VOID    UpdateCancelSubscription();

    // downloads
    VOID    BeginDownload( DWORD dwItem );
    VOID    BeginDownloadAll();
    VOID    UpdateDownload();
    FLOAT   GetDownloadPercent()                 {return m_fDownloadPercent;}


    // working packages (after enum)
    DWORD         GetNumInfos()             {return m_ContentInfoList.size();}
    ContentInfo*  GetInfo(DWORD i);
    VOID          RemoveInfo( DWORD i);
    ContentInfo*  GetWorkingInfo()          {return GetInfo(m_dwWorkingItem);}

    // details
    ContentDetails* GetWorkingDetails()     {return &m_WorkingDetails;}

    // message string for message UI
    BOOL    Error()                   {return m_bError;}
    const   WCHAR* GetErrorString()   {return m_Err.c_str(); }

    // owned content
    VOID    AddOwnedContent(XOFFERING_ID Id, const char* szContentDirectory, bool bVerify);
    bool    OwnContent(XOFFERING_ID Id);
    bool    IsOwnedContentCorrupted(XOFFERING_ID Id);
    bool    OwnCorruptContent();
    bool    DeleteCorruptContent();


protected:
    
    // Sets Error
    VOID Error(HRESULT hr);

    // helper download function
    VOID BeginDownloadWorking();

    // verifcation helper
    bool VerifyContent(const char* szContentDirectory);
    
    XONLINETASK_HANDLE              m_hContentTask;       // content task handle
    XONLINEOFFERING_ENUM_PARAMS     m_EnumParams;         // enumerate params
    //BYTE*                         m_pEnumBuffer;        // enumeration buffer
    //BYTE*                         m_pDetailsBuffer;     // details buffer
    DWORD                           m_dwUserIndex;        // user who is downloading
    DWORD                           m_dwWorkingItem;      // index into content list for current working item
    BOOL                            m_bError;
    FLOAT                           m_fDownloadPercent;
    QWORD                           m_qwTotalBytes;
    QWORD                           m_qwCurrentBytes;
    BOOL                            m_bDownloadAll;
    BOOL                            m_bIncludePay;

    BOOL                            m_bNewContent;
    
    std::vector<ContentInfo>        m_ContentInfoList;       // working content list
    ContentDetails                  m_WorkingDetails;
    std::wstring                    m_Err;                   // Error string

    std::vector<ContentOwned>       m_OwnedContent;
    DWORD                           m_dwBuildNumber;    // owned content

};

//-----------------------------------------------------------------------------
// Name: g_ContentManager
// Desc: global content manager
//-----------------------------------------------------------------------------
extern CContentManager g_ContentManager;


#endif // CONTENTDOWNLOAD_COMMON_H
