/*-----------------------------------------------------------------------------
	File Name   :	NexCodecEntry.c
	Description :	See the following code. ^^
 ------------------------------------------------------------------------------

 NexStreaming Confidential Proprietary
 Copyright (C) 2007 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

#if defined(WIN32) || defined(WINCE)
	#include "windows.h"
#endif

#include "only4CAL.h"
#include "NexCodecEntry.h"

NEXCALCodecEntry * _CE_New()
{
	NEXCALCodecEntry *pCE = (NEXCALCodecEntry*)nexSAL_MemAlloc(sizeof(NEXCALCodecEntry));
	if (pCE)
	{
		memset(pCE, 0, sizeof(NEXCALCodecEntry));
	}

	return pCE;
}

NXBOOL _IsSameEntry(NEXCALCodecEntry *a_pA, NEXCALCodecEntry *a_pB)
{
	if ((a_pA->m_eType != a_pB->m_eType) 
		|| (a_pA->m_eMode != a_pB->m_eMode) 
		|| (a_pA->m_eCodecType != a_pB->m_eCodecType) 
		|| (a_pA->m_pOnLoad != a_pB->m_pOnLoad) 
		|| (a_pA->m_pUserDataForOnLoad != a_pB->m_pUserDataForOnLoad)
		|| (a_pA->m_uStaticProperties != a_pB->m_uStaticProperties))
	{
		return(NXBOOL)FALSE;
	}

	return(NXBOOL)TRUE;
}

//
// return value :
//		-1	: fail
//		0	: added
//		1	: find the same entry, and updated
// Comments:
//		a_pEntry must be created by _CE_New()
//
NXINT32 _CE_Add(NEXCALCodecEntry **a_pStart, NEXCALCodecEntry *a_pEntry)
{
	NXINT32 nLoopCnt;
	NXBOOL bSameTypeFinded = FALSE;
	NEXCALCodecEntry *pPre, *pTemp = NULL, *pFirst = NULL;

	a_pEntry->m_nNumOfSameType = 1;

	if (*a_pStart == 0)
	{
		*a_pStart = a_pEntry;
		return NEXCAL_ERROR_NONE;
	}

	pPre = (*a_pStart);

	for (nLoopCnt = 0; pPre && nLoopCnt < NEXCAL_MAX_CODECS_ENTRY; nLoopCnt++)
	{
		if (_IsSameEntry(pPre, a_pEntry))
		{
			SAFE_FREE(a_pEntry);
			return NEXCAL_ERROR_NONE;
		}

		if (_IS_SAME_TYPE_(pPre, a_pEntry->m_eType, a_pEntry->m_eMode, a_pEntry->m_eCodecType))
		{
			if (FALSE == bSameTypeFinded) // first met the same type.
			{
				pFirst = pPre;
				bSameTypeFinded = TRUE;
			}

			pFirst->m_nNumOfSameType++;
		}
		else if (TRUE == bSameTypeFinded)
		{
			break;
		}
		else
		{
			bSameTypeFinded = FALSE;
		}

		pTemp = pPre;
		pPre = pPre->m_pNext;
	}

	if (nLoopCnt == NEXCAL_MAX_CODECS_ENTRY)
	{
		return NEXCAL_ERROR_FAIL;
	}

	a_pEntry->m_pNext = pTemp->m_pNext;
	pTemp->m_pNext = a_pEntry;

	return NEXCAL_ERROR_NONE;
}

NXINT32 _CE_Remove(NEXCALCodecEntry **a_pStart, NEXCALCodecEntry *a_pEntry)
{
	NXINT32 nLoopCnt;
	NEXCALCodecEntry *pPrev, *pCur;

	if (*a_pStart == 0)
	{
		return NEXCAL_ERROR_INVALID_PARAMETER;
	}

	pCur = (*a_pStart);
	pPrev = NULL;

	for (nLoopCnt = 0; pCur && nLoopCnt < NEXCAL_MAX_CODECS_ENTRY; nLoopCnt++)
	{
		if (_IsSameEntry(pCur, a_pEntry))
		{
			//이전 Item의 pNext Pointer를 변갱해줘야 한다.
			if (pPrev)
			{
				pPrev->m_pNext = pCur->m_pNext;
			}
			else
			{
				(*a_pStart) = pCur->m_pNext;
			}

			SAFE_FREE(pCur);
			break;
		}

		pPrev = pCur;
		pCur = pCur->m_pNext;
	}

	if (nLoopCnt == NEXCAL_MAX_CODECS_ENTRY)
	{
		return NEXCAL_ERROR_FAIL;
	}

	return NEXCAL_ERROR_NONE;
}

NEXCALCodecEntry* _CE_Find(NEXCALCodecEntry *a_pStart, NEXCALMediaType a_eType, NEXCALMode a_eMode, NEX_CODEC_TYPE a_eCodec)
{
	NXINT32 nLoopCnt;
	NEXCALCodecEntry *a_pEntry = a_pStart;

	for (nLoopCnt = 0 ; a_pEntry && nLoopCnt < NEXCAL_MAX_CODECS_ENTRY ; nLoopCnt++)
	{
		if (_IS_SAME_TYPE_(a_pEntry, a_eType, a_eMode, a_eCodec))
		{
			break;
		}
		a_pEntry = a_pEntry->m_pNext;
	}

	if (nLoopCnt == NEXCAL_MAX_CODECS_ENTRY)
	{
		return(NXVOID*)0;
	}

	return a_pEntry;
}

NXVOID _CE_FreeListAll(NEXCALCodecEntry *a_pStart)
{
	NEXCALCodecEntry* pTemp;

	while (a_pStart)
	{
		pTemp = a_pStart;
		a_pStart = a_pStart->m_pNext;

		SAFE_FREE(pTemp);
	}
}

/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date		Description of Changes
 ------------------------------------------------------------------------------
 ysh		07-04-07	Draft.
 ----------------------------------------------------------------------------*/
