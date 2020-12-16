/******************************************************************************
* File Name   : Queue.h
* Description : Data Structure
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2015 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef NEXDATASRUCT_QUEUE_H
#define NEXDATASRUCT_QUEUE_H

#include <stdlib.h>

#include "NexDataStruct.h"
#include "LinkedList.h"

/*****************************************************************************
*                                                                            *
*  Implement queues as linked lists.                                         *
*                                                                            *
*****************************************************************************/

typedef LINKEDLIST QUEUE;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

	QUEUE* Queue_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

	NXVOID Queue_Destroy(IN QUEUE *a_pQueue);

	DS_RET Queue_Enqueue(IN QUEUE *a_pQueue, IN const NXVOID *a_pData);

	DS_RET Queue_Dequeue(IN QUEUE *a_pQueue, OUT NXVOID **a_ppData);

	NXVOID* Queue_Peek(IN QUEUE *a_pQueue);

	NXINT32 Queue_Size(IN QUEUE *a_pQueue); 

#ifdef __cplusplus
}
#endif

#endif//NEXDATASRUCT_QUEUE_H
