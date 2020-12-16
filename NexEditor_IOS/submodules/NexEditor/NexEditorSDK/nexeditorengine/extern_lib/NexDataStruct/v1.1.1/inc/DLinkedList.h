/******************************************************************************
* File Name   : DLinkedList.h
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

#ifndef NEXDATASTRUCT_DLIST_H
#define NEXDATASTRUCT_DLIST_H

#include <stdlib.h>

#include "NexDataStruct.h"

/*****************************************************************************
*                                                                            *
*  Define a structure for doubly-linked list elements.                       *
*                                                                            *
*****************************************************************************/

typedef struct _DLINKEDLIST_ELEMENT_
{
	NXVOID *a_pData;
	struct _DLINKEDLIST_ELEMENT_ *pPrev;
	struct _DLINKEDLIST_ELEMENT_ *pNext;
} DLINKEDLIST_ELEMENT;

/*****************************************************************************
*                                                                            *
*  Define a structure for doubly-linked lists.                               *
*                                                                            *
*****************************************************************************/

typedef struct _DLINKEDLIST_
{
	NXINT32 nSize;

	NXINT32 (*MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2);
	NXVOID (*DestroyCB)(NXVOID *pData);

	DLINKEDLIST_ELEMENT *pHead;
	DLINKEDLIST_ELEMENT *pTail;
} DLINKEDLIST;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

	DLINKEDLIST* DLinkedList_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

	NXVOID DLinkedList_Destroy(IN DLINKEDLIST *a_pDList);

	DS_RET DLinkedList_InsertNext(IN DLINKEDLIST *a_pDList, IN DLINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData);

	DS_RET DLinkedList_InsertPrev(IN DLINKEDLIST *a_pDList, IN DLINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData);

	DS_RET DLinkedList_Remove(IN DLINKEDLIST *a_pDList, IN DLINKEDLIST_ELEMENT *a_pElem);

	NXINT32 DLinkedList_Size(IN DLINKEDLIST *a_pDList);

	DLINKEDLIST_ELEMENT* DLinkedList_Head(IN DLINKEDLIST *a_pDList);

	DLINKEDLIST_ELEMENT* DLinkedList_Tail(IN DLINKEDLIST *a_pDList);

	NXBOOL DLinkedList_IsHead(IN DLINKEDLIST_ELEMENT *a_pElem);

	NXBOOL DLinkedList_IsTail(IN DLINKEDLIST_ELEMENT *a_pElem);

	NXVOID* DLinkedList_Data(IN DLINKEDLIST_ELEMENT *a_pElem);

	DLINKEDLIST_ELEMENT* DLinkedList_Next(IN DLINKEDLIST_ELEMENT *a_pElem);

	DLINKEDLIST_ELEMENT* DLinkedList_Prev(IN DLINKEDLIST_ELEMENT *a_pElem);

#ifdef __cplusplus
}
#endif

#endif//NEXDATASTRUCT_DLIST_H
