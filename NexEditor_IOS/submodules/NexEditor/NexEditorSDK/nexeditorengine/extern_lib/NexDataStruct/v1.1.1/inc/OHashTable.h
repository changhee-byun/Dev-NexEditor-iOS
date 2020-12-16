/******************************************************************************
* File Name   : OHashTable.h
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

#ifndef NEXDATASTRUCT_OHASHTABLE_H
#define NEXDATASTRUCT_OHASHTABLE_H

#include <stdlib.h>

#include "NexDataStruct.h"

/*****************************************************************************
*                                                                            *
*  Define a structure for open-addressed hash tables.                        *
*                                                                            *
*****************************************************************************/

typedef struct _OHASHTABLE_
{
	NXINT32 nPositions;
	NXVOID *pVacated;

	NXINT32 (*Hash1CB)(const NXVOID *a_pKey);
	NXINT32 (*Hash2CB)(const NXVOID *a_pKey);
	NXINT32 (*MatchCB)(const NXVOID *a_Key1, const NXVOID *a_pKey2);
	NXVOID (*DestroyCB)(NXVOID *a_pData);

	NXINT32 nSize;
	NXVOID **ppTable;
} OHASHTABLE;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

OHASHTABLE* OHashTbl_Init(IN NXINT32 a_nPositions, IN NXINT32 (*a_Hash1CB)(const NXVOID *a_pKey), IN NXINT32(*a_Hash2CB)(const NXVOID *a_pKey),
							IN NXINT32 (*a_MatchCB)(const NXVOID *a_Key1, const NXVOID *a_Key2), IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

NXVOID OHashTbl_Destroy(IN OHASHTABLE *a_pHashTbl);

DS_RET OHashTbl_Insert(IN OHASHTABLE *a_pHashTbl, IN const NXVOID *a_pData);

DS_RET OHashTbl_Remove(IN OHASHTABLE *a_pHashTbl, IN NXVOID *a_pData);

DS_RET OHashTbl_Lookup(IN const OHASHTABLE *a_pHashTbl, IN NXVOID *a_pData);

NXINT32 OHashTbl_Size(IN OHASHTABLE *a_pHashTbl);

#endif //NEXDATASTRUCT_OHASHTABLE_H

