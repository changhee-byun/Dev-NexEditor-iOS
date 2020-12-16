/******************************************************************************
* File Name   : BinaryTree.h
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

#ifndef NEXDATASTRUCT_BINARYTREE_H
#define NEXDATASTRUCT_BINARYTREE_H

#include <stdlib.h>

#include "NexDataStruct.h"


/*****************************************************************************
*                                                                            *
*  Define a structure for binary tree nodes.                                 *
*                                                                            *
*****************************************************************************/
typedef struct _BITREE_NODE_
{
	NXVOID *pData;
	struct _BITREE_NODE_ *pLeft;
	struct _BITREE_NODE_ *pRight;
} BITREE_NODE;

/*****************************************************************************
*                                                                            *
*  Define a structure for binary trees.                                      *
*                                                                            *
*****************************************************************************/
typedef struct _BITREE_
{
	NXINT32 nSize;

	NXINT32 (*CompareCB)(const NXVOID *a_pKey1, const NXVOID *a_pKey2);
	NXVOID (*DestroyCB)(NXVOID *a_pData);

	BITREE_NODE *pRoot;
} BITREE;

/*****************************************************************************
*                                                                            *
*  --------------------------- Public Interface ---------------------------  *
*                                                                            *
*****************************************************************************/
BITREE* BiTree_Init(IN NXVOID (*a_pDestroy)(NXVOID *a_pData));

NXVOID BiTree_Destroy(IN BITREE *a_pTree);

DS_RET BiTree_InsertLeft(IN BITREE *a_pTree, IN BITREE_NODE *a_pNode, IN const NXVOID *a_pData);

DS_RET BiTree_InsertRight(IN BITREE *a_pTree, IN BITREE_NODE IN *a_pNode, IN const NXVOID *a_pData);

NXVOID BiTree_RemoveLeft(IN BITREE *a_pTree, IN BITREE_NODE *a_pNode);

NXVOID BiTree_RemoveRight(IN BITREE *a_pTree, IN BITREE_NODE *a_pNode);

BITREE* BiTree_Merge(IN BITREE *a_pLeft, IN BITREE *a_pRight, IN const NXVOID *a_pData);

NXINT32 BiTree_Size(IN BITREE *a_pTree);

BITREE_NODE* BiTree_Root(IN BITREE *a_pTree);

NXBOOL BiTree_IsEOB(IN BITREE_NODE *a_pNode);

NXBOOL BiTree_IsLeaf(IN BITREE_NODE *a_pNode);

NXVOID* BiTree_Data(IN BITREE_NODE *a_pNode);

BITREE_NODE* BiTree_Left(IN BITREE_NODE *a_pNode);

BITREE_NODE* BiTree_Right(IN BITREE_NODE *a_pNode);

#endif // NEXDATASTRUCT_BINARYTREE_H
