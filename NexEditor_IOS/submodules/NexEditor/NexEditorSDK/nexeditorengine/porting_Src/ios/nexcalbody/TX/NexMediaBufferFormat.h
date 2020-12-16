/******************************************************************************
 * File Name   :	NexMediaBufferFormat.h
 * Description :	The definition of BufferFormat used in CALBody.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#ifndef NexMediaBufferFormat_h
#define NexMediaBufferFormat_h

class NexMediaBufferFormat
{
public:
    NexMediaBufferFormat(int initialBufferSize = _DEFAULT_BUFFER_SIZE_);
    ~NexMediaBufferFormat();
    
    void smartCopy( void* pSrc, int nLen );
    void* getBufferPtr() { return mBufferPtr; }
    int getBufferLen() { return mBufferLen; };

private:
    void* mBufferPtr;
    int mBufferLen;
    int mMaxLen;
    
public:
    long long mPTS;
    long long mDTS;
    
    void* mUserData;
    
private:
    static const int _DEFAULT_BUFFER_SIZE_ = 1024*24;
};

#endif /* NexMediaBufferFormat_h */

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 15, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/