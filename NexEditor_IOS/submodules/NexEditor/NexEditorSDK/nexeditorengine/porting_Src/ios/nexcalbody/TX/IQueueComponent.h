/******************************************************************************
 * File Name   :	IQueueComponent.h
 * Description :	The interface of QueueComponent used in CALBody.
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#ifndef IQueueComponent_h
#define IQueueComponent_h

class IQueueComponent {
public:
    virtual ~IQueueComponent() {};
    
    virtual int enqueue( void* pBuff, int nLen, long long timeInUs, long long bufferSpecificData ) = 0;
    virtual int setInputEOS() = 0;
    virtual int dequeue( void* pBuff, int* pnLen, long long *pTimeInUs, bool *isConfig, long long * pBufferSpecificData ) = 0;
    virtual bool checkOutputAvailable() = 0;
    
};

#endif /* IQueueComponent_h */

/*-----------------------------------------------------------------------------
 Revision   History:
 Author		Date		    Version		Description of Changes
 ------------------------------------------------------------------------------
 Eric(ysh)  Apr 15, 2016				Draft.
 ------------------------------------------------------------------------------
 ----------------------------------------------------------------------------*/