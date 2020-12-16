//
//  SALBody_File.c
//  ApiDemos
//
//  Created by rooney on 2016. 1. 27..
//  Copyright © 2016년 rooney. All rights reserved.
//

#include "SALBody_File.h"
#include "SALBody_Debug.h"
#include <stdio.h>


NEXSALFileHandle nexSALBody_FileOpen( char * pFilename, NEXSALFileMode iMode )
{
    NEXSALFileHandle hRetFile;
    
    if(pFilename)
    {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[nexSALBody_FileOpen] %s  (%d)\n", pFilename, iMode);
    }
    else {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[nexSALBody_FileOpen] Invalid file name.\n");
    }
    
    if ( iMode & NEXSAL_FILE_CREATE )
    {
        if ( ( iMode & 0x03 ) == NEXSAL_FILE_WRITE || ( iMode & 0x03 ) == NEXSAL_FILE_READWRITE )
            hRetFile = (NEXSALFileHandle)fopen( pFilename, "wb" );
        else
        {
            
            hRetFile = (NEXSALFileHandle)0;
        }
    }
    else
    {
        if ( ( iMode & 0x03 ) == NEXSAL_FILE_READ )
            hRetFile = (NEXSALFileHandle)fopen( pFilename, "rb" );
        else if ( ( iMode & 0x03 ) == NEXSAL_FILE_WRITE || ( iMode & 0x03 ) == NEXSAL_FILE_READWRITE )
            hRetFile = (NEXSALFileHandle)fopen( pFilename, "ab" );
        else
            hRetFile = (NEXSALFileHandle)0;
    }
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[nexSALBody_FileOpen] hRetFile : %d\n", hRetFile);
    
    return hRetFile;
}

int nexSALBody_FileClose( NEXSALFileHandle hFile )
{
    return fclose( (FILE*)hFile );
}

int nexSALBody_FileRead( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToRead )
{
    return (int)fread( pBuf, 1, uiBytesToRead, (FILE*)hFile );
}

int nexSALBody_FileWrite( NEXSALFileHandle hFile, void *pBuf, unsigned int uiBytesToWrite )
{
    return (int)fwrite( pBuf, 1, uiBytesToWrite, (FILE*)hFile );
}

NXINT32 nexSALBody_FileSeek( NEXSALFileHandle hFile, NXINT32 iOffset, NEXSALFileSeekOrigin iOrigin )
{
    int origin;
    int nPos = 0;
    
    switch ( iOrigin )
    {
        case NEXSAL_SEEK_BEGIN :	origin = SEEK_SET; break;
        case NEXSAL_SEEK_CUR :		origin = SEEK_CUR; break;
        case NEXSAL_SEEK_END :		origin = SEEK_END; break;
    }
    
    fseek( (FILE*)hFile, (long)iOffset, origin );
    
    nPos = (int)ftell( (FILE*)hFile );
    
    //NEXSAL_TRACE("[nexSALBody_FileSeek] iOffset(%d) iOrigin(%d), nPos(%d)\n", iOffset, iOrigin, nPos);
    
    return nPos;
}

NXINT64 nexSALBody_FileSeek64( NEXSALFileHandle hFile, NXINT64 iOffset, NEXSALFileSeekOrigin iOrigin )
{
    int origin;
    NXINT64 nPos = 0;
    
    
    
    switch ( iOrigin )
    {
        case NEXSAL_SEEK_BEGIN :	origin = SEEK_SET; break;
        case NEXSAL_SEEK_CUR :		origin = SEEK_CUR; break;
        case NEXSAL_SEEK_END :		origin = SEEK_END; break;
    }
    
    fseeko( (FILE*)hFile, (off_t)iOffset, origin );
    
    nPos = (NXINT64)ftell( (FILE*)hFile );
    
    //NEXSAL_TRACE("[nexSALBody_FileSeek64] iOffset(%d) iOrigin(%d), nPos(%d)\n", (int)iOffset, iOrigin, (int)nPos);
    
    return nPos;
}

NXINT64 nexSALBody_FileSize(NEXSALFileHandle hFile)
{
    NXINT64 nCurrentPos;
    NXINT64 nSize;
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[nexSALBody_FileSize] hFile=0x%x.\n", hFile);
    
    nCurrentPos = nexSALBody_FileSeek64(hFile, 0, NEXSAL_SEEK_CUR);
    nSize = nexSALBody_FileSeek64(hFile, 0, NEXSAL_SEEK_END);
    nexSALBody_FileSeek(hFile, (NXINT32) nCurrentPos, NEXSAL_SEEK_BEGIN);
    
    return nSize;
}
