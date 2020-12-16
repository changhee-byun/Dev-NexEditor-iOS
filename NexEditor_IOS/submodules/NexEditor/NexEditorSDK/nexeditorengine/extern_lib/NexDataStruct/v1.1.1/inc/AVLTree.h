/******************************************************************************
* File Name   : AVLTree.h
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

#ifndef NEXDATASTRUCT_AVLTREE_H
#define NEXDATASTRUCT_AVLTREE_H

#include "NexDataStruct.h"


/*****************************************************************************
*																			 *
*  Define a structure for binary tree nodes.								 *
*																			 *
*****************************************************************************/
typedef struct _AVLTREE_NODE_
{
	NXVOID *pData;
	struct _AVLTREE_NODE_ *pLeft;
	struct _AVLTREE_NODE_ *pRight;
	NXINT32 nHeight;
} AVLTREE_NODE;

/*****************************************************************************
*																			 *
*  Define a structure for binary trees. 									 *
*																			 *
*****************************************************************************/
typedef struct _AVLTREE_
{
	NXINT32 nSize;

	NXINT32 (*CompareCB)(NXVOID *a_pKey1, NXVOID *a_pKey2);
	NXVOID (*DestroyCB)(NXVOID *a_pData);

	AVLTREE_NODE *pRoot;
} AVLTREE;


/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/

AVLTREE* AVLTree_Init(IN NXINT32 (*a_CompareCB)(NXVOID *a_pKey1, NXVOID *a_pKey2), IN NXVOID (*a_DestroyCB)(NXVOID *a_pData));

NXVOID AVLTree_Destroy(IN AVLTREE *a_pTree);

DS_RET AVLTree_Insert(IN AVLTREE *a_pTree, IN NXVOID *a_pData);

DS_RET AVLTree_Remove(IN AVLTREE *a_pTree, IN NXVOID *a_pData);

DS_RET AVLTree_Lookup(IN AVLTREE *a_pTree, IN NXVOID *a_pData);

NXINT32 AVLTree_Size(IN AVLTREE *a_pTree);

NXVOID* AVLTree_Data(IN AVLTREE_NODE *a_pNode);

AVLTREE_NODE* AVLTree_Left(IN AVLTREE_NODE *a_pNode);

AVLTREE_NODE* AVLTree_Right(IN AVLTREE_NODE *a_pNode);

NXINT32 AVLTree_Height(IN AVLTREE_NODE *a_pNode);

#endif //NEXDATASTRUCT_AVLTREE_H
