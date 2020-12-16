/******************************************************************************
* File Name   : CLinkedList.c
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
#include <string.h>

#include "CLinkedList.h"

#ifdef __NOT_SUPPORT_NEXSAL__
#undef nexSAL_MemAlloc
#define nexSAL_MemAlloc malloc
#undef nexSAL_MemFree
#define nexSAL_MemFree free
#endif


CLINKEDLIST* CLinkedList_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	CLINKEDLIST *pCList = nexSAL_MemAlloc(sizeof(CLINKEDLIST));

	if (pCList)
	{
		pCList->nSize = 0;
		pCList->DestroyCB = a_DestroyCB;
		pCList->pHead = NULL;

		return pCList;
	}

	return NULL;
}

NXVOID CLinkedList_Destroy(IN CLINKEDLIST *a_pCList)
{
	while (a_pCList->nSize > 0)
	{
		CLinkedList_RemoveNext(a_pCList, a_pCList->pHead);
	}

	SAFE_FREE(a_pCList);

	return;
}

DS_RET CLinkedList_InsertNext(IN CLINKEDLIST *a_pCList, IN CLINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData)
{
	CLINKEDLIST_ELEMENT *pNewElem;

	if ((pNewElem = (CLINKEDLIST_ELEMENT *)nexSAL_MemAlloc(sizeof(CLINKEDLIST_ELEMENT))) == NULL)
	{
		return eDS_RET_FAIL;
	}

	pNewElem->pData = (NXVOID*)a_pData;

	if (0 == a_pCList->nSize)
	{
		pNewElem->pNext = pNewElem;
		a_pCList->pHead = pNewElem;
	}
	else
	{
		pNewElem->pNext = a_pElem->pNext;
		a_pElem->pNext = pNewElem;
	}

	a_pCList->nSize++;

	return eDS_RET_SUCCESS;
}

DS_RET CLinkedList_RemoveNext(IN CLINKEDLIST *a_pCList, IN CLINKEDLIST_ELEMENT *a_pElem)
{
	CLINKEDLIST_ELEMENT *a_pOldElem;

	if (0 == a_pCList->nSize)
	{
		return eDS_RET_FAIL;
	}

	if (a_pCList->DestroyCB)
	{
		a_pCList->DestroyCB(a_pElem->pNext->pData);
	}

	if (1 == a_pCList->nSize)
	{
		a_pOldElem = a_pElem->pNext;
		a_pCList->pHead = NULL;
	}
	else
	{
		a_pOldElem = a_pElem->pNext;
		a_pElem->pNext = a_pElem->pNext->pNext;
	}

	SAFE_FREE(a_pOldElem);

	a_pCList->nSize--;

	return eDS_RET_SUCCESS;
}

NXINT32 CLinkedList_Size(IN CLINKEDLIST *a_pCList)
{
	return a_pCList->nSize;
}

CLINKEDLIST_ELEMENT* CLinkedList_Head(IN CLINKEDLIST *a_pCList)
{
	return a_pCList->pHead;
}

CLINKEDLIST_ELEMENT* CLinkedList_Data(IN CLINKEDLIST_ELEMENT *a_pElem)
{
	return a_pElem->pData;
}

CLINKEDLIST_ELEMENT* CLinkedList_Next(IN CLINKEDLIST_ELEMENT *a_pElem)
{
	return a_pElem->pNext;
}

