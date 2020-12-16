/******************************************************************************
* File Name   :	nexQueue.h
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

//---------------------------------------------------------------------------
#ifndef __NEXQUEUE_H__
#define __NEXQUEUE_H__
//---------------------------------------------------------------------------

#include "NexSAL_Internal.h"

typedef enum
{
	NEXQUEUE_RET_SUCCESS		= 0x00000000,
	NEXQUEUE_RET_ERROR			= 0x00000001,
	NEXQUEUE_RET_NODATA			= 0x00000002,
	NEXQUEUE_RET_NEEDMORE		= 0x00000003
} NexQueueRet;

typedef struct _NexQueueStruct
{
	unsigned char*		m_pBuffer;
	unsigned int		m_uiBufSize;
	unsigned int		m_uiBasePos;
	unsigned int		m_uiUsedSize;
	unsigned int		m_uiRemainSize;
	NXBOOL				m_bEnqueueState;
	NXBOOL				m_bDequeueState;
	NEXSALMutexHandle	m_hMutex;
} NexQueueStruct, *PNexQueueHandle;

#ifdef __cplusplus
extern "C"
{
#endif

PNexQueueHandle			nexQueue_Create(unsigned int uiQueueSize);
NexQueueRet				nexQueue_Destroy(PNexQueueHandle pHandle);

NexQueueRet				nexQueue_Clear(PNexQueueHandle pHandle);
NexQueueRet				nexQueue_ExistData(PNexQueueHandle pHandle);

NexQueueRet				nexQueue_GetEnQueueBuf(PNexQueueHandle pHandle, unsigned char** ppBuf, unsigned int uiSize, unsigned int uiTimeOut);
NexQueueRet				nexQueue_EnQueue(PNexQueueHandle pHandle, unsigned char* pBuf, unsigned int uiSize);

NexQueueRet				nexQueue_GetDeQueueBuf(PNexQueueHandle pHandle, unsigned char** ppBuf, unsigned int uiSize, unsigned int uiTimeOut);
NexQueueRet				nexQueue_DeQueue(PNexQueueHandle pHandle, unsigned char* pBuf, unsigned int uiSize);

#ifdef __cplusplus
}
#endif

#endif // __NEXQUEUE_H__