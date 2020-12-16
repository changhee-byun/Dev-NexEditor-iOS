/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_SetTimeTask.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/07/26	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_SetTimeTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_ClipList.h"

CNEXThread_SetTimeTask::CNEXThread_SetTimeTask( void )
{
	m_pProjectMng			= NULL;
	m_pVideoRenderer		= NULL;
	m_bThumbMode = 0;
}

CNEXThread_SetTimeTask::~CNEXThread_SetTimeTask( void )
{
	SAFE_RELEASE(m_pProjectMng);
	SAFE_RELEASE(m_pVideoRenderer);
}

NXBOOL CNEXThread_SetTimeTask::setProjectManager(CNexProjectManager* pMng)
{
	SAFE_RELEASE(m_pProjectMng);
	m_pProjectMng = pMng;
	SAFE_ADDREF(m_pProjectMng);
	return TRUE;
}

NXBOOL CNEXThread_SetTimeTask::setRenderer(CNEXThreadBase* pVideo)
{
	SAFE_RELEASE(m_pVideoRenderer);
	m_pVideoRenderer = pVideo;
	SAFE_ADDREF(m_pVideoRenderer);
	return TRUE;
}

int CNEXThread_SetTimeTask::OnThreadMain( void )
{
	m_bIsWorking = TRUE;
	while ( TRUE )
	{
		CNxMsgInfo* pCmdMsg = m_MsgQ.PopCommand(NEXSAL_INFINITE);
		if( pCmdMsg != NULL )
		{
			if ( pCmdMsg->m_nMsgType == CNEXThreadBase_Exit )
			{
				SAFE_RELEASE(pCmdMsg);
				break;
			}

			if( ProcessCommandMsg(pCmdMsg) == MESSAGE_PROCESS_OK )
				continue;
			SAFE_RELEASE(pCmdMsg);
		}

		switch(m_eThreadState)
		{
			case PLAY_STATE_IDLE:
			{
				ProcessIdleState();
				break;
			}
			case PLAY_STATE_RUN:
			{
				ProcessPlayState();
				break;
			}
			case PLAY_STATE_RECORD:
			{
				ProcessRecordState();
				break;
			}
			default:
			{
				nexSAL_TaskSleep(100);
				break;
			}
		};
	}
	m_bIsWorking = FALSE;
	return 0;
}

int CNEXThread_SetTimeTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	switch( pMsg->m_nMsgType )
	{
		case MESSAGE_STATE_CHANGE:
		{
			CNxMsgChangeState* pStateChangeMsg = (CNxMsgChangeState*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] StateChange(%s : %d %d)", __LINE__, Name(), m_eThreadState, pStateChangeMsg->m_currentState );
			if( m_eThreadState == pStateChangeMsg->m_currentState )
			{
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}
			m_ePreThreadState = m_eThreadState;
			m_eThreadState = pStateChangeMsg->m_currentState;
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_WAIT_COMMAND:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] MESSAGE_WAIT_COMMAND", __LINE__, m_pProjectMng);
			CNxMsgWaitCmd* pWaitCmdMsg = (CNxMsgWaitCmd*)pMsg;
			if( pWaitCmdMsg != NULL )
			{
				pWaitCmdMsg->setProcessDone();
				SAFE_RELEASE(pWaitCmdMsg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] MESSAGE_WAIT_COMMAND Done", __LINE__, m_pProjectMng);
				return MESSAGE_PROCESS_OK; 
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] MESSAGE_WAIT_COMMAND Done", __LINE__, m_pProjectMng);
			SAFE_RELEASE(pWaitCmdMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_SET_TIME:
		{
			CNxMsgSetTime* pSetTime = (CNxMsgSetTime*)pMsg;
			CClipList* pCliplist = (CClipList*)pSetTime->getClipList();
			int iMsgSize = GetMsgSize();
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] MESSAGE_SET_TIME(%p %p), %d %d", __LINE__, m_pProjectMng, pCliplist, iMsgSize, pSetTime->GetRefCnt());

			if( pCliplist == NULL || iMsgSize )
			{
				CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
				SAFE_RELEASE(pCliplist);				
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}				

#ifdef FOR_CHECK_HW_RESOURCE_SIZE
			int iPreviewHWSize = pCliplist->getPreviewHWSize(pSetTime->m_uiTime);
			if(CNexCodecManager::getHardwareMemRemainSize() < iPreviewHWSize)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] video HW not enough resource(%d %d)", __LINE__, CNexCodecManager::getHardwareMemRemainSize(), iPreviewHWSize);			
				CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_HW_NOT_ENOUGH_MEMORY, 0);
				SAFE_RELEASE(pCliplist);				
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}				
#endif

			CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
			if( pVideoRender )
			{
				if( pVideoRender->isLostNativeWindow() ) {
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] native Window did not set", __LINE__);
					CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, 0);
					SAFE_RELEASE(pCliplist);
					SAFE_RELEASE(pMsg);
					SAFE_RELEASE(pVideoRender);
					return MESSAGE_PROCESS_OK;
				}
				SAFE_RELEASE(pVideoRender);
			}

			if( pSetTime->m_iDisplay )
			{
				// case, for mantis 9094		
				if( m_pVideoRenderer )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] MESSAGE_DELETE_TRACK_ALL send", __LINE__);
					m_pVideoRenderer->SendSimpleCommand(MESSAGE_DELETE_TRACK_ALL);
				}
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] Receive setTime Command Time(%d) prepare image", __LINE__, pSetTime->m_uiTime);
				if( pSetTime->m_iIDRFrame == 4 )
				{
					m_bThumbMode = TRUE;
					if( pCliplist->setPreviewThumb(pSetTime->m_uiTime, pSetTime->m_pThumbRender, pSetTime->m_pOutputSurface, m_pVideoRenderer) == 0 )
					{
						CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
						m_bThumbMode = FALSE;
						pCliplist->unsetPreviewThumb();
						SAFE_RELEASE(pCliplist);			
						SAFE_RELEASE(pSetTime);
						return MESSAGE_PROCESS_OK; 
					}
				}
				else
				{
					if( m_bThumbMode )
					{
						m_bThumbMode = FALSE;
						pCliplist->unsetPreviewThumb();
					}
					pCliplist->setPreviewTime(pSetTime->m_uiTime, pSetTime->m_iIDRFrame, m_pVideoRenderer);
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] Receive setTime Command Time(%d) ignore image", __LINE__, pSetTime->m_uiTime);
				//m_pClipList->setPreviewTime(pSetTime->m_uiTime, pSetTime->m_iIDRFrame, m_pVideoRender);
			}
			
			if( m_pVideoRenderer )
			{
                if (pSetTime->createNexEvent())
                {
                    m_pVideoRenderer->SendCommand(pSetTime);
                    if (pSetTime->waitProcessDone(2000))
                    {
                        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] Set Time Wait Done(%d) (%p %d)", __LINE__, pSetTime->m_uiTime, pSetTime, pSetTime->GetRefCnt());
                    }
                    else
                    {
                        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[SetTimeTask.cpp %d] Set Time Wait Timeout(%d) (%p %d)", __LINE__, pSetTime->m_uiTime, pSetTime, pSetTime->GetRefCnt());
                        
                        CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, 0);
                    }
                }
                else
                {
                    CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY, 0);
                }
			}
			pCliplist->clearImageTask();
			SAFE_RELEASE(pCliplist);			
			SAFE_RELEASE(pSetTime);
			return MESSAGE_PROCESS_OK; 
		}
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);	
}
