/******************************************************************************
 * File Name   :	NexCaalBodyTool.h
 * Description :	The definition of tools for A/V Decoder/Encoder CAL Body
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#ifndef NexCalBodyTool_h
#define NexCalBodyTool_h

//#define _TESTMODE_WITHOUT_CAL_

#include "nexCAL.h"

class NexCalBodyTools
{
public:
#if !defined(_TESTMODE_WITHOUT_CAL_)
    static const char* getNameFromOTI(NEX_CODEC_TYPE codecType);
#endif
};

class UserValueHolder
{
public:
    UserValueHolder() {};
    ~UserValueHolder();
    
    void setMaxUserValue( unsigned int maxEntryForInt, unsigned int maxEntryForPointer );
    void setUserValueInt( unsigned int index, int value );
    int getUserValueInt( unsigned int index );
    void setUserValuePointer( unsigned int index, void* value );
    void* getUserValuePointer( unsigned int index );
    
private:
    unsigned int mMaxUserValueForInt = 0;
    unsigned int mMaxUserValueForPointer = 0;
    int* mUserValuesForInt = NULL;
    void** mUserValuesForPointer = NULL;
    void UserValuesMemoryClear();
};


#endif /* NexCalBodyTool_h */

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		   Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  April 8, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
