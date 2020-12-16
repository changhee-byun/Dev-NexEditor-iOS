/******************************************************************************
* File Name   : Stack.c
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
#include "Stack.h"

STACK* Stack_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	return(STACK*)LinkedList_Init(a_DestroyCB);
}

NXVOID Stack_Destroy(IN STACK *a_pStack)
{
	LinkedList_Destroy((LINKEDLIST*)a_pStack);
}

DS_RET Stack_Push(IN STACK *a_pStack, IN const NXVOID *a_pData)
{
	return LinkedList_InsertNext((LINKEDLIST*)a_pStack, NULL, a_pData);
}

DS_RET Stack_Pop(IN STACK *a_pStack, OUT NXVOID **a_ppData)
{
	return LinkedList_RemoveNext((LINKEDLIST*)a_pStack, NULL, a_ppData);
}

NXVOID* Stack_Peek(IN STACK *a_pStack)
{
	return(a_pStack->pHead == NULL ? NULL : a_pStack->pHead->pData);
}

NXINT32 Stack_Size(IN STACK *a_pStack)
{
	return LinkedList_Size((LINKEDLIST*)a_pStack);
}

