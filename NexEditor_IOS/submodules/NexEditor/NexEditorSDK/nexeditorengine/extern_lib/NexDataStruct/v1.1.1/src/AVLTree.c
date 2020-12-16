/******************************************************************************
* File Name   : AVLTree.c
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

#include<stdio.h>
#include<stdlib.h>

#include "AVLTree.h"

#ifdef __NOT_SUPPORT_NEXSAL__
#undef nexSAL_MemAlloc
#define nexSAL_MemAlloc malloc
#undef nexSAL_MemFree
#define nexSAL_MemFree free
#endif


// A utility function to get height of the tree
static NXINT32 _GetHeight(AVLTREE_NODE *a_pNode)
{
	if (a_pNode == NULL)
	{
		return 0;
	}
	
	return a_pNode->nHeight;
}

/* Helper function that allocates a new AVLTREE_NODE with the given key and
	NULL left and right poNXINT32ers. */
static AVLTREE_NODE* _NewNode(NXVOID *a_pData)
{
	AVLTREE_NODE *pNode = (AVLTREE_NODE*)nexSAL_MemAlloc(sizeof(AVLTREE_NODE));

	if (pNode)
	{
		pNode->pData = a_pData;
		pNode->pLeft = NULL;
		pNode->pRight = NULL;
		pNode->nHeight = 1;  // new pNode is initially added at leaf
	}
	
	return pNode;
}

// A utility function to right rotate subtree rooted with y
// See the diagram given above.
static AVLTREE_NODE* _RightRotate(AVLTREE_NODE *a_pNode)
{
	AVLTREE_NODE *pLeft = a_pNode->pLeft;
	AVLTREE_NODE *pTemp = pLeft->pRight;

	// Perform rotation
	pLeft->pRight = a_pNode;
	a_pNode->pLeft = pTemp;

	// Update heights
	a_pNode->nHeight = MAX(_GetHeight(a_pNode->pLeft), _GetHeight(a_pNode->pRight)) + 1;
	pLeft->nHeight = MAX(_GetHeight(pLeft->pLeft), _GetHeight(pLeft->pRight)) + 1;

	// Return new root
	return pLeft;
}

// A utility function to left rotate subtree rooted with x
// See the diagram given above.
static AVLTREE_NODE* _LeftRotate(AVLTREE_NODE *a_pNode)
{
	AVLTREE_NODE *pRight = a_pNode->pRight;
	AVLTREE_NODE *pTemp = pRight->pLeft;

	// Perform rotation
	pRight->pLeft = a_pNode;
	a_pNode->pRight = pTemp;

	//	Update heights
	a_pNode->nHeight = MAX(_GetHeight(a_pNode->pLeft), _GetHeight(a_pNode->pRight)) + 1;
	pRight->nHeight = MAX(_GetHeight(pRight->pLeft), _GetHeight(pRight->pRight)) + 1;

	// Return new root
	return pRight;
}

// Get Balance factor of AVLTREE_NODE a_pNode
static NXINT32 _GetBalance(AVLTREE_NODE *a_pNode)
{
	if (a_pNode == NULL)
	{
		return 0;
	}
	
	return _GetHeight(a_pNode->pLeft) - _GetHeight(a_pNode->pRight);
}

static DS_RET _InsertNode(AVLTREE *a_pTree, AVLTREE_NODE **a_ppNode, NXVOID *a_pData)
{
	NXINT32 nCmpVal = 0;
	NXINT32 nBalance = 0;
	DS_RET eRet = eDS_RET_FAIL;
	
	/* 1.  Perform the normal BST rotation */
	if (NULL == *a_ppNode)
	{
		*a_ppNode = _NewNode(a_pData);

		if (*a_ppNode)
		{
			return eDS_RET_SUCCESS;
		}
		else
		{
			return eDS_RET_FAIL;
		}
	}

	nCmpVal = a_pTree->CompareCB(a_pData, (*a_ppNode)->pData);

	if (nCmpVal < 0)
	{
		if ((eRet = _InsertNode(a_pTree, &((*a_ppNode)->pLeft), a_pData)) != eDS_RET_SUCCESS)
		{
			return eRet;
		}
	}
	else if (nCmpVal > 0)
	{
		if ((eRet = _InsertNode(a_pTree, &((*a_ppNode)->pRight), a_pData)) != eDS_RET_SUCCESS)
		{
			return eRet;
		}
	}
	else
	{
		return eDS_RET_DUPLICATION;
	}

	/* 2. Update height of this ancestor a_pNode */
	(*a_ppNode)->nHeight = MAX(_GetHeight((*a_ppNode)->pLeft), _GetHeight((*a_ppNode)->pRight)) + 1;

	/* 3. Get the balance factor of this ancestor a_pNode to check whether
	   this a_pNode became unbalanced */
	nBalance = _GetBalance((*a_ppNode));

	// If this AVLTREE_NODE becomes unbalanced, then there are 4 cases
	if (nBalance > 1 && nCmpVal < 0) // Left Left Case
	{
		(*a_ppNode) = _RightRotate(*a_ppNode);
	}
	else if (nBalance < -1 && nCmpVal > 0) // Right Right Case
	{
		(*a_ppNode) = _LeftRotate(*a_ppNode);
	}
	else if (nBalance > 1 && nCmpVal > 0) // Left Right Case
	{
		(*a_ppNode)->pLeft = _LeftRotate((*a_ppNode)->pLeft);
		*a_ppNode = _RightRotate((*a_ppNode));
	}
	else if (nBalance < -1 && nCmpVal < 0) // Right Left Case
	{
		(*a_ppNode)->pRight = _RightRotate((*a_ppNode)->pRight);
		*a_ppNode = _LeftRotate(*a_ppNode);
	}

	return eDS_RET_SUCCESS;
}

/* Given a non-empty binary search tree, return the a_pNode with minimum
   key value found in that tree. Note that the entire tree does not
   need to be searched. */
static AVLTREE_NODE* _MinValueNode(AVLTREE_NODE *a_pNode)
{
	AVLTREE_NODE *pCurNode = a_pNode;

	/* loop down to find the leftmost leaf */
	while (pCurNode->pLeft != NULL)
	{
		pCurNode = pCurNode->pLeft;
	}

	return pCurNode;
}

static DS_RET _RemoveNode(AVLTREE *a_pTree, AVLTREE_NODE **a_ppNode, NXVOID *a_pData)
{
	NXINT32 nCmpVal = 0;
	NXINT32 nBalance = 0;
	DS_RET eRet = eDS_RET_FAIL;
	
	// STEP 1: PERFORM STANDARD BST DELETE
	if (NULL == *a_ppNode)
	{
		return eDS_RET_FAIL;
	}

	nCmpVal = a_pTree->CompareCB(a_pData, (*a_ppNode)->pData);

	if (nCmpVal < 0) // If the key to be deleted is smaller than the root's key, then it lies in left subtree
	{
		if ((eRet = _RemoveNode(a_pTree, &((*a_ppNode)->pLeft), a_pData)) != eDS_RET_SUCCESS)
		{
			return eRet;
		}		
	}
	else if (nCmpVal > 0) // If the key to be deleted is greater than the root's key, then it lies in right subtree
	{
		if ((eRet = _RemoveNode(a_pTree, &((*a_ppNode)->pRight), a_pData)) != eDS_RET_SUCCESS)
		{
			return eRet;
		}		
	}
	else // if key is same as root's key, then This is the a_pNode to be deleted
	{
		// a_pNode with only one child or no child
		if (((*a_ppNode)->pLeft == NULL) || ((*a_ppNode)->pRight == NULL))
		{
			AVLTREE_NODE *pTemp = (*a_ppNode)->pLeft ? (*a_ppNode)->pLeft : (*a_ppNode)->pRight;

			// No child case
			if (pTemp == NULL)
			{
				pTemp = *a_ppNode;
				*a_ppNode = NULL;
			}
			else // One child case
			{
				**a_ppNode = *pTemp; // Copy the contents of the non-empty child
			}

			a_pTree->DestroyCB(pTemp->pData);
			SAFE_FREE(pTemp);
		}
		else
		{
			// a_pNode with two children: Get the inorder successor (smallest in the right subtree)
			AVLTREE_NODE* pTemp = _MinValueNode((*a_ppNode)->pRight);

			// Copy the inorder successor's data to this a_pNode
			(*a_ppNode)->pData = pTemp->pData;

			// Delete the inorder successor
			if ((eRet = _RemoveNode(a_pTree, &((*a_ppNode)->pRight), pTemp->pData)) != eDS_RET_SUCCESS)
			{
				return eRet;
			}				
		}
	}

	if (NULL == (*a_ppNode)) // If the tree had only one a_pNode then return
	{
		return eDS_RET_SUCCESS;
	}

	// STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
	(*a_ppNode)->nHeight = MAX(_GetHeight((*a_ppNode)->pLeft), _GetHeight((*a_ppNode)->pRight)) + 1;

	// STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to check whether
	//	this AVLTREE_NODE became unbalanced)
	nBalance = _GetBalance((*a_ppNode));

	// If this a_pNode becomes unbalanced, then there are 4 cases
	if (nBalance > 1 && _GetBalance((*a_ppNode)->pLeft) >= 0) // Left Left Case
	{
		*a_ppNode = _RightRotate((*a_ppNode));
	}
	else if (nBalance > 1 && _GetBalance((*a_ppNode)->pLeft) < 0) // Left Right Case
	{
		(*a_ppNode)->pLeft =  _LeftRotate((*a_ppNode)->pLeft);
		*a_ppNode = _RightRotate(*a_ppNode);
	}
	else if (nBalance < -1 && _GetBalance((*a_ppNode)->pRight) <= 0) // Right Right Case
	{
		*a_ppNode = _LeftRotate(*a_ppNode);
	}
	else if (nBalance < -1 && _GetBalance((*a_ppNode)->pRight) > 0) // Right Left Case
	{
		(*a_ppNode)->pRight = _RightRotate((*a_ppNode)->pRight);
		*a_ppNode = _LeftRotate(*a_ppNode);
	}

	return eDS_RET_SUCCESS;
}

static DS_RET _Lookup(AVLTREE *a_pTree, AVLTREE_NODE *a_pNode, NXVOID *a_pData)
{
	NXINT32 nCmpVal = 0;
	DS_RET eRet;

	if (NULL == a_pNode)
	{
		return eDS_RET_FAIL;
	}

	nCmpVal = a_pTree->CompareCB(a_pData, a_pNode->pData);

	if (nCmpVal < 0)
	{
		eRet = _Lookup(a_pTree, a_pNode->pLeft, a_pData);
	}
	else if (nCmpVal > 0)
	{
		eRet = _Lookup(a_pTree, a_pNode->pRight, a_pData);
	}
	else
	{
		eRet = eDS_RET_SUCCESS;
	}

	return eRet;
}

static DS_RET _DestroyTree(AVLTREE *a_pTree, AVLTREE_NODE *a_pNode)
{
	if (a_pTree->nSize < 1 || NULL == a_pNode)
	{
		return eDS_RET_FAIL;
	}
	
	_DestroyTree(a_pTree, a_pNode->pLeft);
	_DestroyTree(a_pTree, a_pNode->pRight);

	if (a_pTree->DestroyCB)
	{
		a_pTree->DestroyCB(a_pNode->pData);
	}

	SAFE_FREE(a_pNode);
	a_pTree->nSize--;

	return eDS_RET_SUCCESS;
}

AVLTREE* AVLTree_Init(IN NXINT32 (*a_CompareCB)(NXVOID *a_pKey1, NXVOID *a_pKey2), IN NXVOID (*a_DestroyCB)(NXVOID *a_pData))
{
	AVLTREE *pTree = NULL;

	pTree = nexSAL_MemAlloc(sizeof(AVLTREE));

	if (NULL == pTree)
	{
		return NULL;
	}

	pTree->nSize = 0;
	pTree->pRoot = NULL;
	pTree->CompareCB = a_CompareCB;
	pTree->DestroyCB = a_DestroyCB;

	return pTree;
}

NXVOID AVLTree_Destroy(IN AVLTREE *a_pTree)
{
	_DestroyTree(a_pTree, a_pTree->pRoot);
	
	SAFE_FREE(a_pTree);
}

DS_RET AVLTree_Insert(IN AVLTREE *a_pTree, IN NXVOID *a_pData)
{
	DS_RET eRet;
	
	if ((eRet = _InsertNode(a_pTree, &(a_pTree->pRoot), a_pData)) == eDS_RET_SUCCESS)
	{
		a_pTree->nSize++;
	}
	
	return eRet;
}

DS_RET AVLTree_Remove(IN AVLTREE *a_pTree, IN NXVOID *a_pData)
{
	DS_RET eRet;

	if ((eRet = _RemoveNode(a_pTree, &(a_pTree->pRoot), a_pData)) == eDS_RET_SUCCESS)
	{
		a_pTree->nSize--;
	}

	return eRet;
}

DS_RET AVLTree_Lookup(IN AVLTREE *a_pTree, IN NXVOID *a_pData)
{
	return _Lookup(a_pTree, a_pTree->pRoot, a_pData);
}

NXINT32 AVLTree_Size(IN AVLTREE *a_pTree)
{
	return a_pTree->nSize;
}

NXVOID* AVLTree_Data(IN AVLTREE_NODE *a_pNode)
{
	return a_pNode->pData;
}

AVLTREE_NODE* AVLTree_Left(IN AVLTREE_NODE *a_pNode)
{
	return a_pNode->pLeft;
}

AVLTREE_NODE* AVLTree_Right(IN AVLTREE_NODE *a_pNode)
{
	return a_pNode->pRight;
}

NXINT32 AVLTree_Height(IN AVLTREE_NODE *a_pNode)
{
	return a_pNode->nHeight;
}

