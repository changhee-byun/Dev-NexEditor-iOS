/******************************************************************************
* File Name   : BinaryTree.c
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

#include "BinaryTree.h"

#ifdef __NOT_SUPPORT_NEXSAL__
#undef nexSAL_MemAlloc
#define nexSAL_MemAlloc malloc
#undef nexSAL_MemFree
#define nexSAL_MemFree free
#endif

BITREE* BiTree_Init(IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	BITREE *pTree;

	if (NULL == (pTree= nexSAL_MemAlloc(sizeof(BITREE))))
	{
		return NULL;
	}

	pTree->nSize = 0;
	pTree->DestroyCB = a_DestroyCB;
	pTree->pRoot = NULL;

	return pTree;
}

NXVOID BiTree_Destroy(IN BITREE *a_pTree)
{
	BiTree_RemoveLeft(a_pTree, NULL);

	SAFE_FREE(a_pTree);

	return;
}

DS_RET BiTree_InsertLeft(IN BITREE *a_pTree, IN BITREE_NODE *a_pNode, IN const NXVOID *a_pData)
{
	BITREE_NODE *pNewNode;
	BITREE_NODE **ppPosition;

	if (a_pNode == NULL)
	{
		if (a_pTree->nSize > 0)
		{
			return eDS_RET_FAIL;
		}

		ppPosition = &a_pTree->pRoot;
	}
	else
	{
		if (NULL != a_pNode->pLeft)
		{
			return eDS_RET_FAIL;
		}

		ppPosition = &a_pNode->pLeft;
	}

	if ((pNewNode = (BITREE_NODE*)nexSAL_MemAlloc(sizeof(BITREE_NODE))) == NULL)
	{
		return eDS_RET_FAIL;
	}

	pNewNode->pData = (NXVOID*)a_pData;
	pNewNode->pLeft = NULL;
	pNewNode->pRight = NULL;
	*ppPosition = pNewNode;

	a_pTree->nSize++;

	return eDS_RET_SUCCESS;
}

DS_RET BiTree_InsertRight(IN BITREE *a_pTree, IN BITREE_NODE *a_pNode, IN const NXVOID *a_pData)
{
	BITREE_NODE *pNewNode;
	BITREE_NODE **ppPosition;

	if (a_pNode == NULL)
	{
		if (a_pTree->nSize > 0)
		{
			return eDS_RET_FAIL;
		}

		ppPosition = &a_pTree->pRoot;
	}
	else
	{
		if (NULL != a_pNode->pRight)
		{
			return eDS_RET_FAIL;
		}

		ppPosition = &a_pNode->pRight;
	}

	if ((pNewNode = (BITREE_NODE *)nexSAL_MemAlloc(sizeof(BITREE_NODE))) == NULL)
	{
		return eDS_RET_FAIL;
	}

	pNewNode->pData = (NXVOID *)a_pData;
	pNewNode->pLeft = NULL;
	pNewNode->pRight = NULL;
	*ppPosition = pNewNode;

	a_pTree->nSize++;

	return eDS_RET_SUCCESS;
}

NXVOID BiTree_RemoveLeft(IN BITREE *a_pTree, IN BITREE_NODE *a_pNode)
{
	BITREE_NODE **ppPosition;

	if (a_pTree->nSize == 0)
	{
		return;
	}

	if (a_pNode == NULL)
	{
		ppPosition = &a_pTree->pRoot;
	}
	else
	{
		ppPosition = &a_pNode->pLeft;
	}

	if (*ppPosition != NULL)
	{
		BiTree_RemoveLeft(a_pTree, *ppPosition);
		BiTree_RemoveRight(a_pTree, *ppPosition);

		if (a_pTree->DestroyCB != NULL)
		{
			a_pTree->DestroyCB((*ppPosition)->pData);
		}

		SAFE_FREE(*ppPosition);

		a_pTree->nSize--;
	}

	return;
}

NXVOID BiTree_RemoveRight(IN BITREE *a_pTree, IN BITREE_NODE *a_pNode)
{
	BITREE_NODE **ppPosition;

	if (a_pTree->nSize == 0)
	{
		return;
	}

	if (a_pNode == NULL)
	{
		ppPosition = &a_pTree->pRoot;
	}
	else
	{
		ppPosition = &a_pNode->pRight;
	}

	if (*ppPosition != NULL)
	{

		BiTree_RemoveLeft(a_pTree, *ppPosition);
		BiTree_RemoveRight(a_pTree, *ppPosition);

		if (a_pTree->DestroyCB != NULL)
		{
			a_pTree->DestroyCB((*ppPosition)->pData);
		}

		SAFE_FREE(*ppPosition);

		a_pTree->nSize--;
	}

	return;
}

BITREE* BiTree_Merge(IN BITREE *a_pLeft, IN BITREE *a_pRight, IN const NXVOID *a_pData)
{
	BITREE *pMerge = BiTree_Init(a_pLeft->DestroyCB);

	if (BiTree_InsertLeft(pMerge, NULL, a_pData) != eDS_RET_SUCCESS)
	{
		BiTree_Destroy(pMerge);
		return NULL;
	}

	(pMerge->pRoot)->pLeft = a_pLeft->pRoot;
	(pMerge->pRoot)->pRight = a_pRight->pRoot;

	pMerge->nSize = pMerge->nSize + a_pLeft->nSize + a_pRight->nSize;

	SAFE_FREE(a_pLeft);
	SAFE_FREE(a_pRight);

	return pMerge;
}

NXINT32 BiTree_Size(IN BITREE *a_pTree)
{
	return a_pTree->nSize;
}

BITREE_NODE* BiTree_Root(IN BITREE *a_pTree)
{
	return a_pTree->pRoot;
}

NXBOOL BiTree_IsEOB(IN BITREE_NODE *a_pNode)
{
	return(NULL==a_pNode ? TRUE : FALSE);
}

NXBOOL BiTree_IsLeaf(IN BITREE_NODE *a_pNode)
{
	return(NULL==a_pNode->pLeft && NULL==a_pNode->pRight ? TRUE : FALSE);
}

NXVOID* BiTree_Data(IN BITREE_NODE *a_pNode)
{
	return a_pNode->pData;
}

BITREE_NODE* BiTree_Left(IN BITREE_NODE *a_pNode)
{
	return a_pNode->pLeft;
}

BITREE_NODE* BiTree_Right(IN BITREE_NODE *a_pNode)
{
	return a_pNode->pRight;
}

