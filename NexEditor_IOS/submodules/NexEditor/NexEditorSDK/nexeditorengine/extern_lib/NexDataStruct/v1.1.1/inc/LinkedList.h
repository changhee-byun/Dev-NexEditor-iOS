/******************************************************************************
* File Name   : LinkedList.h
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

#ifndef NEXDATASTRUCT_LIST_H
#define NEXDATASTRUCT_LIST_H

#include <stdlib.h>

#include "NexDataStruct.h"

typedef struct _LINKEDLIST_ELEMENT_
{
	NXVOID *pData;
	struct _LINKEDLIST_ELEMENT_ *pNext;
} LINKEDLIST_ELEMENT;

typedef struct _LINKEDLIST_
{
	NXINT32 nSize;

	NXINT32 (*MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2);
	NXVOID (*DestroyCB)(NXVOID *a_pData);

	LINKEDLIST_ELEMENT *pHead;
	LINKEDLIST_ELEMENT *pTail;
} LINKEDLIST;


/*****************************************************************************
*																			 *
*  --------------------------- Public Interface ---------------------------  *
*																			 *
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

	LINKEDLIST* LinkedList_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

	NXVOID LinkedList_Destroy(IN LINKEDLIST *a_pList);

	DS_RET LinkedList_InsertNext(IN LINKEDLIST *a_pList, IN LINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData);

	DS_RET LinkedList_RemoveNext(IN LINKEDLIST *a_pList, IN LINKEDLIST_ELEMENT *a_pElem, INOUT NXVOID **a_ppData);

	NXINT32 LinkedList_Size(IN LINKEDLIST *a_pList);

	LINKEDLIST_ELEMENT* LinkedList_Head(IN LINKEDLIST *a_pList);

	LINKEDLIST_ELEMENT* LinkedList_Tail(IN LINKEDLIST *a_pList);

	NXBOOL LinkedList_IsHead(IN LINKEDLIST *a_pList, IN LINKEDLIST_ELEMENT *a_pElem);

	NXBOOL LinkedList_IsTail(IN LINKEDLIST_ELEMENT *a_pElem);

	NXVOID* LinkedList_Data(IN LINKEDLIST_ELEMENT *a_pElem);

	LINKEDLIST_ELEMENT* LinkedList_Next(IN LINKEDLIST_ELEMENT *a_pElem);

#ifdef __cplusplus
}
#endif

#endif //NEXDATASTRUCT_LIST_H
