/******************************************************************************
* File Name   : Set.c
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
#include "Set.h"

SET* Set_Init(IN NXINT32 (*a_MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2), IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	SET *pSet = NULL;
	pSet = (SET*)LinkedList_Init(a_DestroyCB);
	pSet->MatchCB = a_MatchCB;

	return pSet;
}

NXVOID Set_Destroy(IN SET *a_pSet)
{
	LinkedList_Destroy((LINKEDLIST*)a_pSet);
}

DS_RET Set_Insert(IN SET *a_pSet, IN NXVOID *a_pData)
{
	if (TRUE == Set_IsMember(a_pSet, a_pData))
	{
		return eDS_RET_DUPLICATION;
	}

	return LinkedList_InsertNext((LINKEDLIST*)a_pSet, LinkedList_Tail((LINKEDLIST*)a_pSet), a_pData);
}

DS_RET Set_Remove(IN SET *a_pSet, IN NXVOID *a_pData)
{
	LINKEDLIST_ELEMENT *pMember = NULL;
	LINKEDLIST_ELEMENT *pPrev = NULL;

	for (pMember = LinkedList_Head((LINKEDLIST*)a_pSet); pMember != NULL; pMember = LinkedList_Next(pMember))
	{
		if (a_pSet->MatchCB(a_pData, LinkedList_Data(pMember)))
		{
			break;
		}

		pPrev = pMember;
	}

	if (pMember == NULL)
	{
		return eDS_RET_FAIL;
	}

	return LinkedList_RemoveNext(a_pSet, pPrev, NULL);
}

SET* Set_Union(IN SET *a_pSet1, IN SET *a_pSet2)
{
	LINKEDLIST_ELEMENT *pMember = NULL;
	NXVOID *pData = NULL;
	SET *pSetU = NULL;

	pSetU = Set_Init(a_pSet1->MatchCB, NULL);

	for (pMember = LinkedList_Head((LINKEDLIST*)a_pSet1); pMember != NULL; pMember = LinkedList_Next(pMember))
	{
		pData = LinkedList_Data(pMember);

		if (LinkedList_InsertNext((LINKEDLIST*)pSetU, LinkedList_Tail((LINKEDLIST*)pSetU), pData) != 0)
		{
			Set_Destroy(pSetU);
			return NULL;
		}
	}

	for (pMember = LinkedList_Head((LINKEDLIST*)a_pSet2); pMember != NULL; pMember = LinkedList_Next(pMember))
	{
		pData = LinkedList_Data(pMember);

		if (TRUE == Set_IsMember(a_pSet1, pData))
		{
			continue;
		}
		else
		{
			if (LinkedList_InsertNext((LINKEDLIST*)pSetU, LinkedList_Tail((LINKEDLIST*)pSetU), pData) != 0)
			{
				Set_Destroy(pSetU);
				return NULL;
			}
		}
	}

	return pSetU;
}

SET* Set_Intersection(IN SET *a_pSet1, IN SET *a_pSet2)
{
	LINKEDLIST_ELEMENT *pMember = NULL;
	NXVOID *pData = NULL;
	SET *pSetI = NULL;

	pSetI = Set_Init(a_pSet1->MatchCB, NULL);

	for (pMember = LinkedList_Head((LINKEDLIST*)a_pSet1); pMember != NULL; pMember = LinkedList_Next(pMember))
	{
		pData = LinkedList_Data(pMember);

		if (TRUE == Set_IsMember(a_pSet2, pData))
		{
			if (LinkedList_InsertNext((LINKEDLIST*)pSetI, LinkedList_Tail((LINKEDLIST*)pSetI), pData) != 0)
			{
				Set_Destroy(pSetI);
				return NULL;
			}
		}
	}

	return pSetI;
}

SET* Set_Difference(IN SET *a_pSet1, IN SET *a_pSet2)
{
	LINKEDLIST_ELEMENT *pMember = NULL;
	NXVOID *pData = NULL;
	SET *pSetD = NULL;

	pSetD = Set_Init(a_pSet1->MatchCB, NULL);

	for (pMember = LinkedList_Head((LINKEDLIST*)a_pSet1); pMember != NULL; pMember = LinkedList_Next(pMember))
	{
		pData = LinkedList_Data(pMember);

		if (FALSE == Set_IsMember(a_pSet2, pData))
		{
			if (LinkedList_InsertNext((LINKEDLIST*)pSetD, LinkedList_Tail((LINKEDLIST*)pSetD), pData) != 0)
			{
				Set_Destroy(pSetD);
				return NULL;
			}
		}
	}

	return pSetD;
}

NXBOOL Set_IsMember(IN SET *a_pSet, IN NXVOID *a_pData)
{
	LINKEDLIST_ELEMENT *pMember = NULL;

	for (pMember = LinkedList_Head((LINKEDLIST*)a_pSet); pMember != NULL; pMember = LinkedList_Next(pMember))
	{
		if (a_pSet->MatchCB(a_pData, LinkedList_Data(pMember)))
		{
			return TRUE;
		}
	}

	return FALSE;
}

NXBOOL Set_IsSubset(IN SET *a_pSet1, IN SET *a_pSet2)
{
	LINKEDLIST_ELEMENT *pMember = NULL;

	if (Set_Size(a_pSet1) > Set_Size(a_pSet2))
	{
		return FALSE;
	}

	for (pMember = LinkedList_Head(a_pSet1); pMember != NULL; pMember = LinkedList_Next(pMember))
	{
		if (!Set_IsMember(a_pSet2, LinkedList_Data(pMember)))
		{
			return FALSE;
		}
	}

	return TRUE;
}

NXBOOL Set_IsEqual(IN SET *a_pSet1, IN SET *a_pSet2)
{
	if (Set_Size(a_pSet1) != Set_Size(a_pSet2))
	{
		return 0;
	}

	return Set_IsSubset(a_pSet1, a_pSet2);
}

NXINT32 Set_Size(IN SET *a_pSet)
{
	return a_pSet->nSize;
}

