/******************************************************************************
* File Name   : Stack.h
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

#ifndef NEXDATASTRUCT_STACK_H
#define NEXDATASTRUCT_STACK_H

#include <stdlib.h>

#include "NexDataStruct.h"
#include "LinkedList.h"

/*****************************************************************************
*                                                                            *
*  Implement stacks as linked lists.                                         *
*                                                                            *
*****************************************************************************/

typedef LINKEDLIST STACK;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

	STACK* Stack_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

	NXVOID Stack_Destroy(IN STACK *a_pStack);

	DS_RET Stack_Push(IN STACK *a_pStack, IN const NXVOID *a_pData);

	DS_RET Stack_Pop(IN STACK *a_pStack, OUT NXVOID **a_ppData);

	NXVOID* Stack_Peek(IN STACK *a_pStack);

	NXINT32 Stack_Size(IN STACK *a_pStack);

#ifdef __cplusplus
}
#endif

#endif //NEXDATASTRUCT_STACK_H
