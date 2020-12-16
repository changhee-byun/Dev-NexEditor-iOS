/******************************************************************************
* File Name   : LinkedList.c
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

#include "LinkedList.h"

#ifdef __NOT_SUPPORT_NEXSAL__
#undef nexSAL_MemAlloc
#define nexSAL_MemAlloc malloc
#undef nexSAL_MemFree
#define nexSAL_MemFree free
#endif


LINKEDLIST* LinkedList_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	LINKEDLIST *pList = nexSAL_MemAlloc(sizeof(LINKEDLIST));

	if (pList)
	{
		pList->nSize = 0;
		pList->DestroyCB = a_DestroyCB;
		pList->pHead = NULL;
		pList->pTail = NULL;

		return pList;
	}

	return NULL;
}

NXVOID LinkedList_Destroy(IN LINKEDLIST *a_pList)
{
	while (a_pList->nSize > 0)
	{
		LinkedList_RemoveNext(a_pList, NULL, NULL);
	}

	SAFE_FREE(a_pList);

	return;
}

DS_RET LinkedList_InsertNext(IN LINKEDLIST *a_pList, IN LINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData)
{
	LINKEDLIST_ELEMENT *pNewElem;

	if ((pNewElem = (LINKEDLIST_ELEMENT *)nexSAL_MemAlloc(sizeof(LINKEDLIST_ELEMENT))) == NULL)
	{
		return eDS_RET_FAIL;
	}

	pNewElem->pData = (NXVOID*)a_pData;

	if (a_pElem == NULL)
	{
		if (0 == a_pList->nSize)
		{
			a_pList->pTail = pNewElem;
		}

		pNewElem->pNext = a_pList->pHead;
		a_pList->pHead = pNewElem;
	}
	else
	{
		if (a_pElem->pNext == NULL)
		{
			a_pList->pTail = pNewElem;
		}

		pNewElem->pNext = a_pElem->pNext;
		a_pElem->pNext = pNewElem;
	}

	a_pList->nSize++;

	return eDS_RET_SUCCESS;
}

DS_RET LinkedList_RemoveNext(IN LINKEDLIST *a_pList, IN LINKEDLIST_ELEMENT *a_pElem, OUT NXVOID **a_ppData)
{
	LINKEDLIST_ELEMENT *pOldElem;

	if (0 == a_pList->nSize)
	{
		return eDS_RET_FAIL;
	}

	if (a_pElem == NULL)
	{
		if (NULL == a_ppData)
		{
			if (a_pList->DestroyCB)
			{
				a_pList->DestroyCB(a_pList->pHead->pData);
			}
		}
		else
		{
			*a_ppData = a_pList->pHead->pData;
		}
		
		pOldElem = a_pList->pHead;
		a_pList->pHead = a_pList->pHead->pNext;

		if (1 == a_pList->nSize)
		{
			a_pList->pTail = NULL;
		}
	}
	else
	{
		if (a_pElem->pNext == NULL)
		{
			return eDS_RET_FAIL;
		}

		if (NULL == a_ppData)
		{
			if (a_pList->DestroyCB)
			{
				a_pList->DestroyCB(a_pElem->pNext->pData);
			}
		}
		else
		{
			*a_ppData = a_pElem->pNext->pData;
		}
		
		pOldElem = a_pElem->pNext;
		a_pElem->pNext = a_pElem->pNext->pNext;

		if (a_pElem->pNext == NULL)
		{
			a_pList->pTail = a_pElem;
		}
	}

	SAFE_FREE(pOldElem);
	a_pList->nSize--;

	return eDS_RET_SUCCESS;
}

NXINT32 LinkedList_Size(IN LINKEDLIST *a_pList)
{
	return a_pList->nSize;
}

LINKEDLIST_ELEMENT* LinkedList_Head(IN LINKEDLIST *a_pList)
{
	return a_pList->pHead;
}

LINKEDLIST_ELEMENT* LinkedList_Tail(IN LINKEDLIST *a_pList)
{
	return a_pList->pTail;
}

NXBOOL LinkedList_IsHead(IN LINKEDLIST *a_pList, IN LINKEDLIST_ELEMENT *a_pElem)
{
	return(a_pElem == a_pList->pHead ? TRUE : FALSE);
}

NXBOOL LinkedList_IsTail(IN LINKEDLIST_ELEMENT *a_pElem)
{
	return(a_pElem->pNext == NULL ? TRUE : FALSE);
}

NXVOID* LinkedList_Data(IN LINKEDLIST_ELEMENT *a_pElem)
{
	return a_pElem->pData;
}

LINKEDLIST_ELEMENT* LinkedList_Next(IN LINKEDLIST_ELEMENT *a_pElem)
{
	return a_pElem->pNext;
}

