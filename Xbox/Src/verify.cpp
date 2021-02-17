//-----------------------------------------------------------------------------
// File: verify.cpp
//
// Desc: content signature verification functions
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#include "verify.h"


//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool g_bSigFailure = false;

//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool VerifySignature(const char* szFileName)
{
    assert(szFileName);

    bool bSuccess = true;
    FILE* fp = NULL;
    XCALCSIG_SIGNATURE SigFile, SigSaved;
    std::string FileName;
    

    // get signature
    if(!GetSignature(szFileName, &SigFile))
        goto Error;
    
    // create sig filename
    FileName = szFileName;
    int nPos = FileName.find_last_of('.');
    if(nPos != -1)
        FileName.erase(nPos);
    FileName += ".sig";

    // open sig filename
    if(!(fp = fopen(FileName.c_str(), "rb")))
        goto Error;

    // read
    if(fread(&SigSaved, sizeof(SigSaved), 1, fp) != 1)
        goto Error;

    // compare sigs
    if(memcmp(&SigFile, &SigSaved, sizeof(XCALCSIG_SIGNATURE)) != 0)
        bSuccess = false;

    if(fclose(fp) != 0)
    {
        fp = NULL;
        goto Error;
    }
    fp = NULL;

Cleanup:
    if(fp != NULL)
        fclose(fp);
    return bSuccess;

Error:
    bSuccess = false;
    goto Cleanup;
}


//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool SaveSignature(const char* szFileName)
{
    assert(szFileName);

    FILE* fp = NULL;
    XCALCSIG_SIGNATURE Sig;
    std::string FileName;
    bool bSuccess = true;
    

    // get signature
    if(!GetSignature(szFileName, &Sig))
        goto Error;
    
    // create sig filename
    FileName = szFileName;
    int nPos = FileName.find_last_of('.');
    if(nPos != -1)
        FileName.erase(nPos);
    FileName += ".sig";

    // open sig filename
    if(!(fp = fopen(FileName.c_str(), "wb")))
        goto Error;

    if(fwrite(&Sig, sizeof(Sig), 1, fp) != 1)
        goto Error;

    if(fclose(fp) != 0)
    {
        fp = NULL;
        goto Error;
    }
    fp = NULL;

Cleanup:
    if(fp != NULL)
        fclose(fp);
    return bSuccess;

Error:
    bSuccess = false;
    goto Cleanup;
};      


//-----------------------------------------------------------------------------
// Name: HasContent
// Desc: do we own a piece of contest
//-----------------------------------------------------------------------------
bool GetSignature(const char* szFileName, XCALCSIG_SIGNATURE* pSig)
{
    assert(szFileName);
    assert(pSig);
    
    FILE* fp = NULL;
    BYTE* pBuf = NULL;
    HANDLE hSig = INVALID_HANDLE_VALUE;
    bool bSuccess = true;


    // open file
    if(!(fp = fopen(szFileName, "rb")))
        goto Error;

    // create buffer
    pBuf = new BYTE[MAX_VERIFY_LENGTH];

    // begin cal sig
    hSig = XCalculateSignatureBegin(XCALCSIG_FLAG_NON_ROAMABLE);
    if(hSig == INVALID_HANDLE_VALUE)
        goto Error;

    // read in chucks and calc sig
    size_t BytesRead;
    while( BytesRead = fread(pBuf, 1, MAX_VERIFY_LENGTH, fp))
    {
        if(ferror(fp))
            goto Error;
        if(XCalculateSignatureUpdate(hSig, pBuf, BytesRead) != ERROR_SUCCESS)
            goto Error;
    }

    // close file
    if(fclose(fp) != 0)
    {
        fp = NULL;
        goto Error;
    }
    fp = NULL;

    // create sig
    if(XCalculateSignatureEnd(hSig, pSig) != ERROR_SUCCESS)
    {
        hSig = INVALID_HANDLE_VALUE;
        goto Error;
    }
    hSig = INVALID_HANDLE_VALUE;
    
Cleanup:
    if(fp != NULL)
        fclose(fp);
    delete [] pBuf;
    if(hSig != INVALID_HANDLE_VALUE)
        XCalculateSignatureEnd(hSig, NULL);
    return bSuccess;

Error:
    bSuccess = false;
    goto Cleanup;
}
