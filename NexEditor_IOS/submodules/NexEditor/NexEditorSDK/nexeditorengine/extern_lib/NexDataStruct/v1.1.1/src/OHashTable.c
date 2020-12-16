/******************************************************************************
* File Name   : OHashTable.c
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

#include "OHashTable.h"

#ifdef __NOT_SUPPORT_NEXSAL__
#undef nexSAL_MemAlloc
#define nexSAL_MemAlloc malloc
#undef nexSAL_MemFree
#define nexSAL_MemFree free
#endif

static NXCHAR s_cVacated;


OHASHTABLE* OHashTbl_Init(IN NXINT32 a_nPositions, IN NXINT32 (*a_Hash1CB)(const NXVOID *a_pKey), IN NXINT32(*a_Hash2CB)(const NXVOID *a_pKey),
							IN NXINT32 (*a_MatchCB)(const NXVOID *a_Key1, const NXVOID *a_Key2), IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	NXINT32 i;
	OHASHTABLE *pHashTbl;

	if ((pHashTbl = (OHASHTABLE*)nexSAL_MemAlloc(sizeof(OHASHTABLE))) == NULL)
	{
		return NULL;
	}

	if ((pHashTbl->ppTable = (NXVOID**)nexSAL_MemAlloc(a_nPositions*sizeof(NXVOID*))) == NULL)
	{
		free(pHashTbl);
		return NULL;
	}

	pHashTbl->nPositions = a_nPositions;

	for (i = 0; i < pHashTbl->nPositions; i++)
	{
		pHashTbl->ppTable[i] = NULL;
	}

	pHashTbl->pVacated = &s_cVacated;

	pHashTbl->Hash1CB = a_Hash1CB;
	pHashTbl->Hash2CB = a_Hash2CB;
	pHashTbl->MatchCB = a_MatchCB;
	pHashTbl->DestroyCB = a_DestroyCB;

	pHashTbl->nSize = 0;

	return pHashTbl;
}

NXVOID OHashTbl_Destroy(IN OHASHTABLE *a_pHashTbl)
{
	NXINT32 i;

	if (a_pHashTbl->DestroyCB != NULL)
	{
		for (i=0; i<a_pHashTbl->nPositions; i++)
		{
			if (a_pHashTbl->ppTable[i] != NULL && a_pHashTbl->ppTable[i] != a_pHashTbl->pVacated)
			{
				a_pHashTbl->DestroyCB(a_pHashTbl->ppTable[i]);
			}
		}
	}

	SAFE_FREE(a_pHashTbl->ppTable);
	memset(a_pHashTbl, 0, sizeof(OHASHTABLE));
	
	return;
}

DS_RET OHashTbl_Insert(IN OHASHTABLE *a_pHashTbl, IN const NXVOID *a_pData)
{
	NXVOID *pTemp;
	NXINT32 nPosition;
	NXINT32 i;

	if (a_pHashTbl->nSize == a_pHashTbl->nPositions)
	{
		return eDS_RET_FAIL;
	}

	pTemp = (NXVOID*)a_pData;

	if (OHashTbl_Lookup(a_pHashTbl, pTemp) == eDS_RET_SUCCESS)
	{
		return eDS_RET_DUPLICATION;
	}

	for (i=0; i<a_pHashTbl->nPositions; i++)
	{
		nPosition = (a_pHashTbl->Hash1CB(a_pData) + (i * a_pHashTbl->Hash2CB(a_pData))) % a_pHashTbl->nPositions;

		if (a_pHashTbl->ppTable[nPosition] == NULL || a_pHashTbl->ppTable[nPosition] == a_pHashTbl->pVacated)
		{
			a_pHashTbl->ppTable[nPosition] = (NXVOID*)a_pData;
			a_pHashTbl->nSize++;
			return eDS_RET_SUCCESS;
		}
	}

	return eDS_RET_FAIL;
}

DS_RET OHashTbl_Remove(IN OHASHTABLE *a_pHashTbl, IN NXVOID *a_pData)
{
	NXINT32 nPosition;
	NXINT32 i;

	for (i = 0; i < a_pHashTbl->nPositions; i++)
	{
		nPosition = (a_pHashTbl->Hash1CB(a_pData) + (i * a_pHashTbl->Hash2CB(a_pData))) % a_pHashTbl->nPositions;

		if (a_pHashTbl->ppTable[nPosition] == NULL)
		{
			return eDS_RET_FAIL;
		}
		else if (a_pHashTbl->ppTable[nPosition] == a_pHashTbl->pVacated)
		{
			continue;
		}
		else if (a_pHashTbl->MatchCB(a_pHashTbl->ppTable[nPosition], a_pData))
		{
			if (a_pHashTbl->DestroyCB)
			{
				a_pHashTbl->DestroyCB(a_pHashTbl->ppTable[nPosition]);
			}
			a_pHashTbl->ppTable[nPosition] = a_pHashTbl->pVacated;
			a_pHashTbl->nSize--;
			return eDS_RET_SUCCESS;
		}
	}
	
	return eDS_RET_FAIL;
}

DS_RET OHashTbl_Lookup(IN const OHASHTABLE *a_pHashTbl, INOUT NXVOID *a_pData)
{
	NXINT32 nPosition;
	NXINT32 i;

	for (i = 0; i<a_pHashTbl->nPositions; i++)
	{
		nPosition = (a_pHashTbl->Hash1CB(a_pData) + (i * a_pHashTbl->Hash2CB(a_pData))) % a_pHashTbl->nPositions;

		if (a_pHashTbl->ppTable[nPosition] == NULL)
		{
			return eDS_RET_FAIL;
		}
		else if (a_pHashTbl->MatchCB(a_pHashTbl->ppTable[nPosition], a_pData))
		{
			return eDS_RET_SUCCESS;
		}
	}

	return eDS_RET_FAIL;
}

NXINT32 OHashTbl_Size(IN OHASHTABLE *a_pHashTbl)
{
	return a_pHashTbl->nSize;
}

