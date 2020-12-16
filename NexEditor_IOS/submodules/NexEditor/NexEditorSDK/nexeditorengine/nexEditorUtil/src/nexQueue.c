/******************************************************************************
* File Name   :	nexQueue.c
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/14	Draft.
-----------------------------------------------------------------------------*/

#include "nexQueue.h"
#include "string.h"

PNexQueueHandle			nexQueue_Create(unsigned int uiQueueSize)
{
	PNexQueueHandle pQueue = (PNexQueueHandle)nexSAL_MemAlloc(sizeof(NexQueueStruct));
	if( pQueue == NULL )
		return NULL;

	pQueue->m_pBuffer = (unsigned char*)nexSAL_MemAlloc(uiQueueSize);
	if( pQueue->m_pBuffer == NULL )
	{
		nexSAL_MemFree(pQueue);
		return NULL;
	}

	pQueue->m_hMutex = nexSAL_MutexCreate();
	if( pQueue->m_hMutex == NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MemFree(pQueue->m_pBuffer);
		nexSAL_MemFree(pQueue);
		return NULL;
	}

	pQueue->m_uiBufSize		= uiQueueSize;
	pQueue->m_uiBasePos		= 0;
	pQueue->m_uiUsedSize	= 0;
	pQueue->m_uiRemainSize	= uiQueueSize;
	return pQueue;
}

NexQueueRet				nexQueue_Destroy(PNexQueueHandle pHandle)
{
	if( pHandle == NULL )
		return NEXQUEUE_RET_ERROR;

	if( pHandle->m_hMutex != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MutexLock(pHandle->m_hMutex, NEXSAL_INFINITE );
		if( pHandle->m_pBuffer != NULL )
		{
			nexSAL_MemFree(pHandle->m_pBuffer);
			pHandle->m_pBuffer = NULL;
		}
		nexSAL_MutexUnlock(pHandle->m_hMutex);
		nexSAL_MutexDelete(pHandle->m_hMutex);
		pHandle->m_hMutex = NEXSAL_INVALID_HANDLE;
	}
	else
	{
		if( pHandle->m_pBuffer != NULL )
		{
			nexSAL_MemFree(pHandle->m_pBuffer);
			pHandle->m_pBuffer = NULL;
		}
	}
	nexSAL_MemFree(pHandle);
	return NEXQUEUE_RET_SUCCESS;
}

NexQueueRet				nexQueue_Clear(PNexQueueHandle pHandle)
{
	if( pHandle == NULL || pHandle->m_hMutex == NEXSAL_INVALID_HANDLE )
		return NEXQUEUE_RET_ERROR;

	nexSAL_MutexLock(pHandle->m_hMutex, NEXSAL_INFINITE );

	pHandle->m_uiBasePos	= 0;
	pHandle->m_uiUsedSize	= 0;
	pHandle->m_uiRemainSize	= pHandle->m_uiBufSize;

	nexSAL_MutexUnlock(pHandle->m_hMutex);
	return NEXQUEUE_RET_SUCCESS;
}

NexQueueRet				nexQueue_ExistData(PNexQueueHandle pHandle)
{
	if( pHandle == NULL )
		return NEXQUEUE_RET_ERROR;

	if( nexSAL_MutexLock(pHandle->m_hMutex, 10 ) != 0 )
	{
		return NEXQUEUE_RET_NODATA;
	}

	if( pHandle->m_uiUsedSize > 0 )
	{
		nexSAL_MutexUnlock(pHandle->m_hMutex);
		return NEXQUEUE_RET_SUCCESS;
	}

	nexSAL_MutexUnlock(pHandle->m_hMutex);
	return NEXQUEUE_RET_NODATA;
}

NexQueueRet				nexQueue_GetEnQueueBuf(PNexQueueHandle pHandle, unsigned char** ppBuf, unsigned int uiSize, unsigned int uiTimeOut)
{
	unsigned int uiRemain;
	int iTime = (int)uiTimeOut;

	if( pHandle == NULL || pHandle->m_pBuffer == NULL || pHandle->m_hMutex == NEXSAL_INVALID_HANDLE || ppBuf == NULL || uiSize == 0 )
		return NEXQUEUE_RET_ERROR;

	while( iTime >= 0 )
	{
		if( nexSAL_MutexLock(pHandle->m_hMutex, 10 ) != 0 )
		{
			uiTimeOut = uiTimeOut - 10;
			continue;
		}

		uiRemain = pHandle->m_uiBufSize - (pHandle->m_uiBasePos + pHandle->m_uiUsedSize);
		if( uiRemain >= uiSize )
		{
			*ppBuf = pHandle->m_pBuffer + pHandle->m_uiBasePos + pHandle->m_uiUsedSize;
			nexSAL_MutexUnlock(pHandle->m_hMutex);
			return NEXQUEUE_RET_SUCCESS;
		}

		if(pHandle->m_uiUsedSize < 4096)
		{
			memmove(pHandle->m_pBuffer, pHandle->m_pBuffer + pHandle->m_uiBasePos, pHandle->m_uiUsedSize);
			pHandle->m_uiBasePos = 0;			
		}

		iTime = iTime - 10;
		nexSAL_MutexUnlock(pHandle->m_hMutex);
#ifdef FOR_PROJECT_Tinno
		nexSAL_TaskSleep(10);
#endif
	}
	*ppBuf = NULL;
	return NEXQUEUE_RET_ERROR;
}

NexQueueRet				nexQueue_EnQueue(PNexQueueHandle pHandle, unsigned char* pBuf, unsigned int uiSize)
{
	if( pHandle == NULL || pHandle->m_pBuffer == NULL || pHandle->m_hMutex == NEXSAL_INVALID_HANDLE || pBuf == NULL || uiSize == 0 )
		return NEXQUEUE_RET_ERROR;

	nexSAL_MutexLock(pHandle->m_hMutex, NEXSAL_INFINITE );

	if( (pHandle->m_pBuffer + pHandle->m_uiBasePos + pHandle->m_uiUsedSize) != pBuf )
	{
		nexSAL_MutexUnlock(pHandle->m_hMutex);
		return NEXQUEUE_RET_ERROR;
	}

	pHandle->m_uiUsedSize += uiSize;
	pHandle->m_uiRemainSize -= uiSize;
	nexSAL_MutexUnlock(pHandle->m_hMutex);
	return NEXQUEUE_RET_SUCCESS;
}

NexQueueRet				nexQueue_GetDeQueueBuf(PNexQueueHandle pHandle, unsigned char** ppBuf, unsigned int uiSize, unsigned int uiTimeOut)
{
	int iTime = (int)uiTimeOut;
	if( pHandle == NULL || pHandle->m_pBuffer == NULL || pHandle->m_hMutex == NEXSAL_INVALID_HANDLE || ppBuf == NULL || uiSize == 0 )
		return NEXQUEUE_RET_ERROR;

	while( iTime >= 0 )
	{
		if( nexSAL_MutexLock(pHandle->m_hMutex, 10 ) != 0 )
		{
			uiTimeOut = uiTimeOut - 10;
			continue;
		}

		if( pHandle->m_uiUsedSize >= uiSize )
		{
			*ppBuf = pHandle->m_pBuffer + pHandle->m_uiBasePos;
			nexSAL_MutexUnlock(pHandle->m_hMutex);
			return NEXQUEUE_RET_SUCCESS;
		}
		iTime = iTime - 10;
		nexSAL_MutexUnlock(pHandle->m_hMutex);
#ifdef FOR_PROJECT_Tinno
		nexSAL_TaskSleep(10);
#endif
	}
	*ppBuf = NULL;
	return NEXQUEUE_RET_ERROR;
}

NexQueueRet				nexQueue_DeQueue(PNexQueueHandle pHandle, unsigned char* pBuf, unsigned int uiSize)
{
	if( pHandle == NULL || pHandle->m_pBuffer == NULL || pHandle->m_hMutex == NEXSAL_INVALID_HANDLE || pBuf == NULL || uiSize == 0 )
		return NEXQUEUE_RET_ERROR;

	if( nexSAL_MutexLock(pHandle->m_hMutex, NEXSAL_INFINITE ) != 0 )
		return NEXQUEUE_RET_ERROR;

	pHandle->m_uiBasePos	+= uiSize;
	pHandle->m_uiUsedSize	-= uiSize;
	pHandle->m_uiRemainSize	+= uiSize;

	if( (pHandle->m_uiBasePos * 100) / pHandle->m_uiBufSize > 60 )
	{
		memmove(pHandle->m_pBuffer, pHandle->m_pBuffer + pHandle->m_uiBasePos, pHandle->m_uiUsedSize);
		pHandle->m_uiBasePos = 0;
	}
	nexSAL_MutexUnlock(pHandle->m_hMutex);
	return NEXQUEUE_RET_SUCCESS;
}
