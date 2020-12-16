/******************************************************************************
* File Name   : DLinkedList.c
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

#include "DLinkedList.h"

#ifdef __NOT_SUPPORT_NEXSAL__
#undef nexSAL_MemAlloc
#define nexSAL_MemAlloc malloc
#undef nexSAL_MemFree
#define nexSAL_MemFree free
#endif


DLINKEDLIST* DLinkedList_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	DLINKEDLIST *pDList = (DLINKEDLIST*)nexSAL_MemAlloc(sizeof(DLINKEDLIST));

	if (pDList)
	{
		pDList->nSize = 0;
		pDList->DestroyCB = a_DestroyCB;
		pDList->pHead = NULL;
		pDList->pTail = NULL;

		return pDList;
	}

	return NULL;
}

NXVOID DLinkedList_Destroy(IN DLINKEDLIST *a_pDList)
{
	while (a_pDList->nSize > 0)
	{
		DLinkedList_Remove(a_pDList, DLinkedList_Tail(a_pDList));
	}

	SAFE_FREE(a_pDList);

	return;

}

DS_RET DLinkedList_InsertNext(IN DLINKEDLIST *a_pDList, IN DLINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData)
{

	DLINKEDLIST_ELEMENT *pNewElem;

	if (a_pElem == NULL && 0 != a_pDList->nSize)
	{
		return eDS_RET_FAIL;
	}

	if ((pNewElem = (DLINKEDLIST_ELEMENT*)nexSAL_MemAlloc(sizeof(DLINKEDLIST_ELEMENT))) == NULL)
	{
		return eDS_RET_FAIL;
	}

	pNewElem->a_pData = (NXVOID*)a_pData;

	if (0 == a_pDList->nSize)
	{
		a_pDList->pHead = pNewElem;
		a_pDList->pHead->pPrev = NULL;
		a_pDList->pHead->pNext = NULL;
		a_pDList->pTail = pNewElem;
	}
	else
	{
		pNewElem->pNext = a_pElem->pNext;
		pNewElem->pPrev = a_pElem;

		if (a_pElem->pNext == NULL)
		{
			a_pDList->pTail = pNewElem;
		}
		else
		{
			a_pElem->pNext->pPrev = pNewElem;
		}

		a_pElem->pNext = pNewElem;
	}

	a_pDList->nSize++;

	return eDS_RET_SUCCESS;
}

DS_RET DLinkedList_InsertPrev(IN DLINKEDLIST *a_pDList, IN DLINKEDLIST_ELEMENT *a_pElem, IN const NXVOID *a_pData)
{
	DLINKEDLIST_ELEMENT *pNewElem;

	if (a_pElem == NULL && 0 != a_pDList->nSize)
	{
		return eDS_RET_FAIL;
	}

	if ((pNewElem = (DLINKEDLIST_ELEMENT *)nexSAL_MemAlloc(sizeof(DLINKEDLIST_ELEMENT))) == NULL)
	{
		return eDS_RET_FAIL;
	}

	pNewElem->a_pData = (NXVOID*)a_pData;

	if (0 == a_pDList->nSize)
	{
		a_pDList->pHead = pNewElem;
		a_pDList->pHead->pPrev = NULL;
		a_pDList->pHead->pNext = NULL;
		a_pDList->pTail = pNewElem;
	}
	else
	{
		pNewElem->pNext = a_pElem; 
		pNewElem->pPrev = a_pElem->pPrev;

		if (a_pElem->pPrev == NULL)
		{
			a_pDList->pHead = pNewElem;
		}
		else
		{
			a_pElem->pPrev->pNext = pNewElem;
		}

		a_pElem->pPrev = pNewElem;
	}

	a_pDList->nSize++;

	return eDS_RET_SUCCESS;
}

DS_RET DLinkedList_Remove(IN DLINKEDLIST *a_pDList, IN DLINKEDLIST_ELEMENT *a_pElem)
{
	if (a_pElem == NULL || 0 == a_pDList->nSize)
	{
		return eDS_RET_FAIL;
	}

	if (a_pDList->DestroyCB)
	{
		a_pDList->DestroyCB(a_pElem->a_pData);
	}

	if (a_pElem == a_pDList->pHead)
	{
		a_pDList->pHead = a_pElem->pNext;

		if (a_pDList->pHead == NULL)
		{
			a_pDList->pTail = NULL;
		}
		else
		{
			a_pElem->pNext->pPrev = NULL;
		}
	}
	else
	{
		a_pElem->pPrev->pNext = a_pElem->pNext;

		if (a_pElem->pNext == NULL)
		{
			a_pDList->pTail = a_pElem->pPrev;
		}
		else
		{
			a_pElem->pNext->pPrev = a_pElem->pPrev;
		}
	}

	SAFE_FREE(a_pElem);

	a_pDList->nSize--;

	return eDS_RET_SUCCESS;
}

NXINT32 DLinkedList_Size(IN DLINKEDLIST *a_pDList)
{
	return a_pDList->nSize;
}

DLINKEDLIST_ELEMENT* DLinkedList_Head(IN DLINKEDLIST *a_pDList)
{
	return a_pDList->pHead;
}

DLINKEDLIST_ELEMENT* DLinkedList_Tail(IN DLINKEDLIST *a_pDList)
{
	return a_pDList->pTail;
}

NXBOOL DLinkedList_IsHead(IN DLINKEDLIST_ELEMENT *a_pElem)
{
	return(a_pElem->pPrev == NULL ? TRUE : FALSE);
}

NXBOOL DLinkedList_IsTail(IN DLINKEDLIST_ELEMENT *a_pElem)
{
	return(a_pElem->pNext == NULL ? TRUE : FALSE);
}

NXVOID* DLinkedList_Data(IN DLINKEDLIST_ELEMENT *a_pElem)
{
	return a_pElem->a_pData;
}

DLINKEDLIST_ELEMENT* DLinkedList_Next(IN DLINKEDLIST_ELEMENT *a_pElem)
{
	return a_pElem->pNext;
}

DLINKEDLIST_ELEMENT* DLinkedList_Prev(IN DLINKEDLIST_ELEMENT *a_pElem)
{
	return a_pElem->pPrev;
}

