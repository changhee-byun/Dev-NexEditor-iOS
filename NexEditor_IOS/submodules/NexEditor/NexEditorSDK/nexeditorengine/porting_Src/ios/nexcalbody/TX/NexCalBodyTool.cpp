/******************************************************************************
 * File Name   : NexCaalBodyTool.h
 * Description : The implementtation of tools for A/V Decoder/Encoder CAL Body
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
#include "NexCalBodyTool.h"

#if !defined(_TESTMODE_WITHOUT_CAL_)

const char* NexCalBodyTools::getNameFromOTI(NEX_CODEC_TYPE codecType)
{
    switch ( codecType ) {
        case eNEX_CODEC_A_MP3       : return "eNEX_CODEC_A_MP3";
        case eNEX_CODEC_A_AAC       : return "eNEX_CODEC_A_AAC";
        case eNEX_CODEC_A_AACPLUS   : return "eNEX_CODEC_A_AACPLUS";
        case eNEX_CODEC_A_AACPLUSV2 : return "eNEX_CODEC_A_AACPLUSV2";
        default : return "Unknown";
    }
}

#endif


void UserValueHolder::UserValuesMemoryClear()
{
    mMaxUserValueForInt = 0;
    if ( mUserValuesForInt )
        free( mUserValuesForInt );
    
    mMaxUserValueForPointer = 0;
    if ( mUserValuesForPointer )
        free( mUserValuesForPointer );
}

UserValueHolder::~UserValueHolder()
{
    UserValuesMemoryClear();
}


void UserValueHolder::setMaxUserValue( unsigned int maxEntryForInt, unsigned int maxEntryForPointer )
{
    UserValuesMemoryClear();
    
    if ( maxEntryForInt > 0 ) {
        mUserValuesForInt = (int*)malloc(sizeof(int)*maxEntryForInt);
        mMaxUserValueForInt = maxEntryForInt;
    }
    if ( maxEntryForInt > 0 ) {
        mUserValuesForPointer = (void**)malloc(sizeof(void*)*maxEntryForPointer);
        mMaxUserValueForPointer = maxEntryForPointer;
    }
}


void UserValueHolder::setUserValueInt( unsigned int index, int value )
{
    if ( index < mMaxUserValueForInt ) {
        mUserValuesForInt[index] = value;
     }
}


int UserValueHolder::getUserValueInt( unsigned int index )
{
    if ( index < mMaxUserValueForInt )
        return mUserValuesForInt[index];
    return 0;
}


void UserValueHolder::setUserValuePointer( unsigned int index, void* value )
{
    if ( index < mMaxUserValueForPointer ) {
        mUserValuesForPointer[index] = value;
    }

}
void* UserValueHolder::getUserValuePointer( unsigned int index )
{
    if ( index < mMaxUserValueForPointer )
        return mUserValuesForPointer[index];
    return (void*)0;
}
    
/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		   Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  April 8, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/
