/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ThumbnailTask.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2012/09/19	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_ThumbnailTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"

CNEXThread_ThumbnailTask::CNEXThread_ThumbnailTask( void )
{
	m_pProjectMng		= NULL;
	m_iUserTag			= 0;
	m_pItem = NULL;
	m_isParseClip = 0;	
	m_isStopParse = 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "CNEXThread_ThumbnailTask Create Done" );
}

CNEXThread_ThumbnailTask::CNEXThread_ThumbnailTask( CNexProjectManager* pPM )
{
	SAFE_ADDREF(pPM);
	m_pProjectMng = pPM;
	m_pItem = NULL;	
	m_isParseClip = 0;
	m_isStopParse = 0;	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "CNEXThread_ThumbnailTask Create Done" );
}

CNEXThread_ThumbnailTask::~CNEXThread_ThumbnailTask( void )
{
	SAFE_RELEASE(m_pProjectMng);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "~~~~CNEXThread_ThumbnailTask Destroy Done" );
}

int CNEXThread_ThumbnailTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	switch( pMsg->m_nMsgType )
	{
		case MESSAGE_GETCLIPINFO_BACKGROUND:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] MESSAGE_GETCLIPINFO_BACKGROUND", __LINE__);
			CNxMsgBackgroundGetInfo* pGetInfo = (CNxMsgBackgroundGetInfo*)pMsg;

			m_pItem = new CClipItem(INVALID_CLIP_ID);
			if( m_pItem == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] creat clip failed", __LINE__);
				pGetInfo->m_nResult = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
				m_pProjectMng->SendCommand(pGetInfo);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;				
			}

			m_pItem->setThumbnailPath((const char*)pGetInfo->m_strThumbFileName);
			
			NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
			if( pGetInfo->m_pThumbRenderer == NULL )
			{
				eRet = (NEXVIDEOEDITOR_ERROR)m_pItem->parseClipFile(pGetInfo->m_strFileName, pGetInfo->m_iVideoThumbnail, pGetInfo->m_iAudioPCMLevel);
			}
			else
			{
				if(m_isStopParse)
				{
					m_pItem->parseClipStop();				
				}
				
				m_isParseClip = TRUE;			
				if( pGetInfo->m_iStartTime > pGetInfo->m_iEndTime )
				{
					eRet = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
				}
				else
				{
					eRet = (NEXVIDEOEDITOR_ERROR)m_pItem->parseClipFile(		pGetInfo->m_strFileName, 
																			pGetInfo->m_iVideoThumbnail, 
																			pGetInfo->m_iAudioPCMLevel,
																			pGetInfo->m_pThumbRenderer,
																			pGetInfo->m_pOutputSurface,
																			pGetInfo->m_iWidth,
																			pGetInfo->m_iHeight,
																			pGetInfo->m_iStartTime,
																			pGetInfo->m_iEndTime,
																			pGetInfo->m_iCount,
																			pGetInfo->m_iTimeTableCount,
																			pGetInfo->m_pTimeTable,
																			pGetInfo->m_iFlag,
																			pGetInfo->m_iUserTag,
																			pGetInfo->m_iUseCount,
																			pGetInfo->m_iSkipCount);
				}
			}
			SAFE_RELEASE(m_pItem);
			if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] clip parse failed", __LINE__);
				pGetInfo->m_nMsgType = MESSAGE_GETCLIPINFO_BACKGROUND_DONE;
				pGetInfo->m_nResult = eRet;
				m_pProjectMng->SendCommand(pGetInfo);
				SAFE_RELEASE(pGetInfo);
				m_isParseClip = FALSE;							
				return MESSAGE_PROCESS_OK;				
			}

			pMsg->m_nMsgType = MESSAGE_GETCLIPINFO_BACKGROUND_DONE;
			if(m_isStopParse)
				pMsg->m_nResult = NEXVIDEOEDITOR_ERROR_BACKGROUND_USER_CANCEL;	
			else
				pMsg->m_nResult = NEXVIDEOEDITOR_ERROR_NONE;
			m_pProjectMng->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] clip parse end(%d)", __LINE__, m_isStopParse);
			m_isParseClip = FALSE;										
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_GETTHUMB_RAWDATA:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] MESSAGE_GETTHUMB_RAWDATA", __LINE__);
			CNxMsgBackgroundGetInfo* pGetInfo = (CNxMsgBackgroundGetInfo*)pMsg;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] MESSAGE_GETTHUMB_RAWDATA %d, %d, 0x%x", __LINE__, pGetInfo->m_iVideoThumbnail, pGetInfo->m_iAudioPCMLevel, pGetInfo->m_iFlag);

			m_pItem = new CClipItem(INVALID_CLIP_ID);
			if( m_pItem == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] creat clip failed", __LINE__);
				pGetInfo->m_nResult = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
				m_pProjectMng->SendCommand(pGetInfo);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;				
			}

			NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
			if(m_isStopParse)
			{
				m_pItem->parseClipStop();				
			}			
			
			m_isParseClip = TRUE;
			if( pGetInfo->m_pThumbRenderer == NULL )
			{
				eRet = (NEXVIDEOEDITOR_ERROR)m_pItem->parseClipFile(pGetInfo->m_strFileName, pGetInfo->m_iVideoThumbnail, pGetInfo->m_iAudioPCMLevel);
			}
			else
			{
				eRet = (NEXVIDEOEDITOR_ERROR)m_pItem->parseClipFile(	pGetInfo->m_strFileName, 
																	pGetInfo->m_iVideoThumbnail, 
																	pGetInfo->m_iAudioPCMLevel,
																	pGetInfo->m_pTimeTable,
																	pGetInfo->m_iTimeTableCount,
																	pGetInfo->m_iStartTime,
																	pGetInfo->m_iEndTime,																	
																	pGetInfo->m_pThumbRenderer,
																	pGetInfo->m_pOutputSurface,
																	pGetInfo->m_iFlag);				
			}
			SAFE_RELEASE(m_pItem);
			
			if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] clip parse failed", __LINE__);
				pGetInfo->m_nMsgType = MESSAGE_GETTHUMB_RAWDATA_DONE;
				pGetInfo->m_nResult = eRet;
				m_pProjectMng->SendCommand(pGetInfo);
				SAFE_RELEASE(pGetInfo);
				m_isParseClip = FALSE;				
				return MESSAGE_PROCESS_OK;				
			}

			pMsg->m_nMsgType = MESSAGE_GETTHUMB_RAWDATA_DONE;
			if(m_isStopParse)
				pMsg->m_nResult = NEXVIDEOEDITOR_ERROR_BACKGROUND_USER_CANCEL;	
			else
				pMsg->m_nResult = NEXVIDEOEDITOR_ERROR_NONE;
			m_pProjectMng->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] clip parse end", __LINE__);
			m_isParseClip = FALSE;			
			return MESSAGE_PROCESS_OK;
		}
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);
}

void CNEXThread_ThumbnailTask::ProcessIdleState()
{
	nexSAL_TaskSleep(50);
}

void CNEXThread_ThumbnailTask::ProcessPlayState()
{
	nexSAL_TaskSleep(50);
}

void CNEXThread_ThumbnailTask::ProcessRecordState()
{
	nexSAL_TaskSleep(50);
}

NXBOOL CNEXThread_ThumbnailTask::setProjectManager(CNexProjectManager* pMng)
{
	SAFE_RELEASE(m_pProjectMng);
	m_pProjectMng = pMng;
	SAFE_ADDREF(m_pProjectMng);
	return TRUE;
}

void CNEXThread_ThumbnailTask::setUserTag(int iUserTag)
{
	m_iUserTag = iUserTag;
}

int CNEXThread_ThumbnailTask::getUserTag()
{
	return m_iUserTag;
}

void CNEXThread_ThumbnailTask::setStopThumb()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ThumbnailTask.cpp %d] setStopThumb", __LINE__);
	m_isStopParse = TRUE;
	if(m_isParseClip && m_pItem)
	{
		m_pItem->parseClipStop();
	}
}


