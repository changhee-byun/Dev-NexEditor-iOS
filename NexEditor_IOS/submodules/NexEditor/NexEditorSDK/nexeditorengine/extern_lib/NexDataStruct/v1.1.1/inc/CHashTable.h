/******************************************************************************
* File Name   : CHashTable.h
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

#ifndef NEXDATASTRUCT_CHASHTABLE_H
#define NEXDATASTRUCT_CHASHTABLE_H

#include <stdlib.h>

#include "NexDataStruct.h"
#include "LinkedList.h"

/*****************************************************************************
*                                                                            *
*  Define a structure for chained hash tables.                               *
*                                                                            *
*****************************************************************************/

typedef struct 
_CHASHTABLE_
{
	NXINT32 nBuckets;

	NXINT32 (*HashCB)(const NXVOID *a_pKey);
	NXINT32 (*MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2);
	NXVOID (*DestroyCB)(NXVOID *a_pData);

	NXINT32 nSize;
	LINKEDLIST **ppTable;
} CHASHTABLE;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

CHASHTABLE* CHashTable_Init(IN NXINT32 a_nBuckets, IN NXINT32 (*a_HashCB)(const NXVOID *a_pKey), 
								IN NXINT32(*a_MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2), IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

NXVOID CHashTable_Destroy(IN CHASHTABLE *a_pHT);

DS_RET CHashTable_Insert(IN CHASHTABLE *a_pHT, IN const NXVOID *a_pData);

DS_RET CHashTable_Remove(IN CHASHTABLE *a_pHT, IN NXVOID *a_pData);

DS_RET CHashTable_Lookup(IN const CHASHTABLE *a_pHT, IN NXVOID *a_pData);

NXINT32 CHashTable_Size(IN CHASHTABLE *a_pHT);

#endif //NEXDATASTRUCT_CHASHTABLE_H

