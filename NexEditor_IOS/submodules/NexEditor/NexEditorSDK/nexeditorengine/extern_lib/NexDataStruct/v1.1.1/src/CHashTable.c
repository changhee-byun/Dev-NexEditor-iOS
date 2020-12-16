/******************************************************************************
* File Name   : CHashTable.c
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
#include "CHashTable.h"

#ifdef __NOT_SUPPORT_NEXSAL__
#undef nexSAL_MemAlloc
#define nexSAL_MemAlloc malloc
#undef nexSAL_MemFree
#define nexSAL_MemFree free
#endif


CHASHTABLE* CHashTable_Init(IN NXINT32 a_nBuckets, IN NXINT32 (*a_HashCB)(const NXVOID *a_pKey), 
								IN NXINT32(*a_MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2), IN NXVOID (*a_DestroyCB)(NXVOID*a_pData)) 
{
	NXINT32 i;
	LINKEDLIST *pTemp = NULL;
	CHASHTABLE *pHT = NULL;

	if ((pHT = (CHASHTABLE*)nexSAL_MemAlloc(sizeof(CHASHTABLE))) == NULL)
	{
		return NULL;
	}

	if ((pHT->ppTable = (LINKEDLIST**)nexSAL_MemAlloc(a_nBuckets * sizeof(LINKEDLIST*))) == NULL)
	{
		return NULL;
	}

	pHT->nBuckets = a_nBuckets;

	for (i=0; i<pHT->nBuckets; i++)
	{
		pHT->ppTable[i] = LinkedList_Init(a_DestroyCB);
	}

	pHT->HashCB = a_HashCB;
	pHT->MatchCB = a_MatchCB;
	pHT->DestroyCB = a_DestroyCB;
	pHT->nSize = 0;

	return pHT;
}

NXVOID CHashTable_Destroy(IN CHASHTABLE *a_pHT)
{
	NXINT32 i;

	for (i=0; i<a_pHT->nBuckets; i++)
	{
		LinkedList_Destroy(a_pHT->ppTable[i]);
	}

	SAFE_FREE(a_pHT->ppTable);
	SAFE_FREE(a_pHT);

	return;
}

DS_RET CHashTable_Insert(IN CHASHTABLE *a_pHT, IN const NXVOID *a_pData)
{
	NXVOID *pTemp = NULL;
	NXINT32 nBucket = 0;
	DS_RET eRetVal = eDS_RET_FAIL;
	
	pTemp = (NXVOID*)a_pData;

	if (CHashTable_Lookup(a_pHT, pTemp) == eDS_RET_SUCCESS)
	{
		return eDS_RET_DUPLICATION;
	}

	nBucket = a_pHT->HashCB(a_pData) % a_pHT->nBuckets;

	if ((eRetVal = LinkedList_InsertNext(a_pHT->ppTable[nBucket], NULL, a_pData)) == eDS_RET_SUCCESS)
	{
		a_pHT->nSize++;
	}

	return eRetVal;
}

DS_RET CHashTable_Remove(IN CHASHTABLE *a_pHT, IN NXVOID *a_pData)
{
	LINKEDLIST_ELEMENT *pElement = NULL;
	LINKEDLIST_ELEMENT *pPrev = NULL;
	NXINT32 nBucket = 0;

	nBucket = a_pHT->HashCB(a_pData) % a_pHT->nBuckets;
	pPrev = NULL;

	for (pElement = LinkedList_Head(a_pHT->ppTable[nBucket]); pElement != NULL; pElement = LinkedList_Next(pElement))
	{
		if (a_pHT->MatchCB(a_pData, LinkedList_Data(pElement)))
		{
			if (LinkedList_RemoveNext(a_pHT->ppTable[nBucket], pPrev, NULL) == eDS_RET_SUCCESS)
			{
				a_pHT->nSize--;
				return eDS_RET_SUCCESS;
			}
			else
			{
				return eDS_RET_FAIL;
			}
		}
		pPrev = pElement;
	}

	return eDS_RET_FAIL;
}

DS_RET CHashTable_Lookup(IN const CHASHTABLE *a_pHT, IN NXVOID *a_pData)
{
	LINKEDLIST_ELEMENT *pElement = NULL;
	NXINT32 nBucket = 0;

	nBucket = a_pHT->HashCB(a_pData) % a_pHT->nBuckets;

	for (pElement = LinkedList_Head(a_pHT->ppTable[nBucket]); pElement != NULL; pElement = LinkedList_Next(pElement))
	{
		NXVOID *pCurData = LinkedList_Data(pElement);
		
		if (a_pHT->MatchCB(a_pData, pCurData))
		{
			return eDS_RET_SUCCESS;
		}
	}

	return eDS_RET_FAIL;
}

NXINT32 CHashTable_Size(IN CHASHTABLE *a_pHT)
{
	return a_pHT->nSize;
}

