/******************************************************************************
* File Name   : Queue.c
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

#include <stdlib.h>

#include "LinkedList.h"
#include "Queue.h"


QUEUE* Queue_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	return(QUEUE*)LinkedList_Init(a_DestroyCB);
}

NXVOID Queue_Destroy(IN QUEUE *a_pQueue)
{
	LinkedList_Destroy((LINKEDLIST*)a_pQueue);
}

DS_RET Queue_Enqueue(IN QUEUE *a_pQueue, IN const NXVOID *a_pData)
{
	return LinkedList_InsertNext((LINKEDLIST*)a_pQueue, LinkedList_Tail(a_pQueue), a_pData);
}

DS_RET Queue_Dequeue(IN QUEUE *a_pQueue, OUT NXVOID **a_ppData)
{
	return LinkedList_RemoveNext((LINKEDLIST*)a_pQueue, NULL, a_ppData);
}

NXVOID* Queue_Peek(IN QUEUE *a_pQueue)
{
	return(a_pQueue->pHead == NULL ? NULL : a_pQueue->pHead->pData);
}

NXINT32 Queue_Size(IN QUEUE *a_pQueue)
{
	return LinkedList_Size((LINKEDLIST*)a_pQueue);
}

