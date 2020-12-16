/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ImageTask.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_ImageTask.h"
#include "NEXVIDEOEDITOR_Util.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"

CNEXThread_ImageTask::CNEXThread_ImageTask( void )
{
	m_pClipItem			= NULL;
	m_pVideoRenderer	= NULL;
	m_isSetTime			= FALSE;
}

CNEXThread_ImageTask::~CNEXThread_ImageTask( void )
{
	if( m_pClipItem != NULL )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] ~CNEXThread_ImageTask In", __LINE__, m_pClipItem->getClipID());
	else
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] ~CNEXThread_ImageTask In", __LINE__);

//	if( m_pClipItem != NULL )
//	{
//		m_pClipItem->deregisteImageTrackInfo(m_pVideoRenderer);
//		SAFE_RELEASE(m_pClipItem);
//	}
	SAFE_RELEASE(m_pClipItem);
	SAFE_RELEASE(m_pVideoRenderer);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] ~CNEXThread_ImageTask Out", __LINE__);
}

NXBOOL CNEXThread_ImageTask::setClipItem(CClipItem* pClipItem)
{
	if( pClipItem == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pClipItem);

	SAFE_ADDREF(pClipItem);
	m_pClipItem	= pClipItem;
	return TRUE;
}

NXBOOL CNEXThread_ImageTask::setVideoRenderer(CNEXThread_VideoRenderTask* pVideoRenderer)
{
	if( pVideoRenderer == NULL ) return FALSE;

	SAFE_ADDREF(pVideoRenderer);
	SAFE_RELEASE(m_pVideoRenderer);
	m_pVideoRenderer = pVideoRenderer;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d]setVideoRenderer(%p)", __LINE__, m_pVideoRenderer);	
	return TRUE;
}

NXBOOL CNEXThread_ImageTask::waitLoadEvent(int timeOut)
{
	return m_eventLoad.WaitEvent(timeOut);
}

void CNEXThread_ImageTask::setSetTimeMode(NXBOOL isSetTime)
{
	m_isSetTime = isSetTime;
}

int CNEXThread_ImageTask::OnThreadMain( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] start %d", __LINE__, m_pClipItem->getClipID());	
	int editorImageThumbnailLoadFlag = 0;
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor != NULL )
	{
		editorImageThumbnailLoadFlag = pEditor->getThumbnailRoutine();
		SAFE_RELEASE(pEditor);
	}

	if( editorImageThumbnailLoadFlag == 0 ) // Thumbnail to original
	{
		CVideoTrackInfo* pImageTrackInfo = NULL;
		pImageTrackInfo = m_pClipItem->getCachedVisualTrackInfo(CACHE_TYPE_IMAGE, m_pClipItem->getClipID());
		if( pImageTrackInfo == NULL )
		{
			NXBOOL success = m_pClipItem->registeThumbImageTrackInfo(m_pVideoRenderer, m_isSetTime);
			if( success == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] reload original %d", __LINE__, m_pClipItem->getClipID());
				m_pClipItem->registeImageTrackInfo(m_pVideoRenderer, m_isSetTime);
				m_eventLoad.Set();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] end %d", __LINE__, m_pClipItem->getClipID());
				return TRUE;
			}
			m_eventLoad.Set();

			nexSAL_TaskSleep(500);

			m_pClipItem->registeThumbImageTrackInfo(m_pVideoRenderer, m_isSetTime);
		}
		else
		{
			if( m_pClipItem->registeThumbImageTrackInfo(m_pVideoRenderer, m_isSetTime) )
			{
				m_eventLoad.Set();
			}
			else
			{
				m_pClipItem->registeImageTrackInfo(m_pVideoRenderer, m_isSetTime);
				m_eventLoad.Set();
			}
		}
		SAFE_RELEASE(pImageTrackInfo);




//		while( m_bIsWorking )
//		{
//			pImageTrackInfo = m_pClipItem->getCachedVisualTrackInfo(CACHE_TYPE_IMAGE, m_pClipItem->getClipID());
//			if( pImageTrackInfo == NULL || pImageTrackInfo->m_iLoadedType == 2 )
//			{
//				if( pImageTrackInfo != NULL && pImageTrackInfo->m_iLoadedType == 2 )
//				{
//					nexSAL_TaskSleep(2000);
//				}
//
//				NXBOOL success = m_pClipItem->registeThumbImageTrackInfo(m_pVideoRenderer, m_isSetTime);
//				if( success == FALSE)
//				{
//					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] reload original %d", __LINE__, m_pClipItem->getClipID());
//					m_pClipItem->registeImageTrackInfo(m_pVideoRenderer, m_isSetTime);
//				}
//				m_eventLoad.Set();
//				SAFE_RELEASE(pImageTrackInfo);
//			}
//			else
//			{
//				if( m_pClipItem->registeThumbImageTrackInfo(m_pVideoRenderer, m_isSetTime) )
//				{
//					m_eventLoad.Set();
//					SAFE_RELEASE(pImageTrackInfo);
//					break;
//				}
//				else
//				{
//					m_pClipItem->registeImageTrackInfo(m_pVideoRenderer, m_isSetTime);
//					m_eventLoad.Set();
//					SAFE_RELEASE(pImageTrackInfo);
//					break;
//				}
//			}
//		}
	}
	else if( editorImageThumbnailLoadFlag == 1 ) // Thumbnail only
	{
		if( m_pClipItem->registeThumbImageTrackInfo(m_pVideoRenderer, m_isSetTime) == FALSE )
		{
			m_pClipItem->registeImageTrackInfo(m_pVideoRenderer, m_isSetTime);
			m_eventLoad.Set();
		}
	}
	else // Original only
	{
		m_pClipItem->registeImageTrackInfo(m_pVideoRenderer, m_isSetTime);
		m_eventLoad.Set();
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ImageTask.cpp %d] end %d", __LINE__, m_pClipItem->getClipID());
	return TRUE;
}

void CNEXThread_ImageTask::End( unsigned int uiTimeout )
{
	if( m_bIsWorking == FALSE ) return;
	if( m_hThread == NEXSAL_INVALID_HANDLE ) return;

	// for NESA-1298
	m_bIsWorking = FALSE;
	if( nexSAL_TaskWait(m_hThread) != 0 )
	{
		// nexSAL_TaskTerminate(m_hThread);
	}
	nexSAL_TaskDelete(m_hThread);
	m_hThread = NEXSAL_INVALID_HANDLE;

	if( m_hSema != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_SemaphoreDelete(m_hSema);
		m_hSema = NEXSAL_INVALID_HANDLE;
	}
}

