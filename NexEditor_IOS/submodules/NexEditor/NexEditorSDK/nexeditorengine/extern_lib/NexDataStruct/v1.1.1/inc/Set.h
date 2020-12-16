/******************************************************************************
* File Name   : Set.h
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

#ifndef NEXDATASTRUCT_SET_H
#define NEXDATASTRUCT_SET_H

#include <stdlib.h>

#include "NexDataStruct.h"
#include "LinkedList.h"


/*****************************************************************************
*                                                                            *
*  Implement sets as linked lists.                                           *
*                                                                            *
*****************************************************************************/

typedef LINKEDLIST SET;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

	SET* Set_Init(IN NXINT32 (*a_MatchCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2), IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

	NXVOID Set_Destroy(IN SET *a_pSet);

	DS_RET Set_Insert(IN SET *a_pSet, IN NXVOID *a_pData);

	DS_RET Set_Remove(IN SET *a_pSet, IN NXVOID *a_pData);

	SET* Set_Union(IN SET *a_pSet1, IN SET *a_pSet2); //A¡úB

	SET* Set_Intersection(IN SET *a_pSet1, IN SET *a_pSet2); //A¡ûB

	SET* Set_Difference(IN SET *a_pSet1, IN SET *a_pSet2);	//A-B

	NXBOOL Set_IsMember(IN SET *a_pSet, IN NXVOID *a_pData);

	NXBOOL Set_IsSubset(IN SET *a_pSet1, IN SET *a_pSet2); //A¡øB

	NXBOOL Set_IsEqual(IN SET *a_pSet1, IN SET *a_pSet2); //A=B

	NXINT32 Set_Size(IN SET *a_pSet);

#ifdef __cplusplus
}
#endif

#endif //NEXDATASTRUCT_SET_H

