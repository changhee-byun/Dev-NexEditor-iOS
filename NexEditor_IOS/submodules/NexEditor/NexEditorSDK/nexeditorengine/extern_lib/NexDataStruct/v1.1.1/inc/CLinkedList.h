/******************************************************************************
* File Name   : CLinkedList.h
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

#ifndef NEXDATASTRUCT_CLIST_H
#define NEXDATASTRUCT_CLIST_H

#include <stdlib.h>

#include "NexDataStruct.h"

/*****************************************************************************
*                                                                            *
*  Define a structure for circular list elements.                            *
*                                                                            *
*****************************************************************************/
typedef struct _CLINKEDLIST_ELEMENT_
{
	NXVOID *pData;
	struct _CLINKEDLIST_ELEMENT_ *pNext;
} CLINKEDLIST_ELEMENT;

/*****************************************************************************
*                                                                            *
*  Define a structure for circular lists.                                    *
*                                                                            *
*****************************************************************************/
typedef struct _CLINKEDLIST_
{
	NXINT32 nSize;

	NXINT32 (*MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2);
	NXVOID (*DestroyCB)(NXVOID *a_pData);

	CLINKEDLIST_ELEMENT *pHead;
} CLINKEDLIST;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

	CLINKEDLIST* CLinkedList_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

	NXVOID CLinkedList_Destroy(IN CLINKEDLIST *a_pCList);

	DS_RET CLinkedList_InsertNext(IN CLINKEDLIST *a_pCList, IN CLINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData);

	DS_RET CLinkedList_RemoveNext(IN CLINKEDLIST *a_pCList, IN CLINKEDLIST_ELEMENT *a_pElem);

	NXINT32 CLinkedList_Size(IN CLINKEDLIST *a_pCList);

	CLINKEDLIST_ELEMENT* CLinkedList_Head(IN CLINKEDLIST *a_pCList);

	CLINKEDLIST_ELEMENT* CLinkedList_Data(IN CLINKEDLIST_ELEMENT *a_pElem);

	CLINKEDLIST_ELEMENT* CLinkedList_Next(IN CLINKEDLIST_ELEMENT *a_pElem); 

#ifdef __cplusplus
}
#endif

#endif//NEXDATASTRUCT_CLIST_H
