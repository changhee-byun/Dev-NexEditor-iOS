/******************************************************************************
 * File Name   :	NexMediaBufferFormat.cpp
 * Description :	The Implementation of BufferFormat used in CALBody.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "NexMediaBufferFormat.h"

NexMediaBufferFormat::NexMediaBufferFormat(int initialBufferSize)
: mBufferPtr(nullptr)
, mBufferLen(0)
, mMaxLen(initialBufferSize)
, mUserData(nullptr)
, mDTS(0)
, mPTS(0)
{
    if ( mMaxLen > 0 )
        mBufferPtr = (void*)malloc( mMaxLen );
}

NexMediaBufferFormat::~NexMediaBufferFormat()
{
    if ( mBufferPtr )
        free( mBufferPtr );
}

void NexMediaBufferFormat::smartCopy( void* pSrc, int nLen )
{
    if ( mMaxLen < nLen ) {
        free( mBufferPtr );
        mBufferPtr = malloc( nLen );
        mMaxLen = nLen;
    }
    
    if ( mBufferPtr ) {
        memcpy( mBufferPtr, pSrc, nLen );
        mBufferLen = nLen;
    }
}

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 15, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/