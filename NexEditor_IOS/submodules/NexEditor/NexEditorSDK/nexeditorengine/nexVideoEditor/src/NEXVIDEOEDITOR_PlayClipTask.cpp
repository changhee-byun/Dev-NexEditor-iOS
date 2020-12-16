/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_PlayClipTask.cpp
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
#include "NEXVIDEOEDITOR_PlayClipTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"

CNEXThread_PlayClipTask::CNEXThread_PlayClipTask( void )
{
	m_pProjectMng		= NULL;
	m_pCliplist			= NULL;
	m_pVideoRenderer	= NULL;
	m_pAudioRenderer	= NULL;
	m_pFileWriter		= NULL;

	m_pUserData			= NULL;
	m_iOption			= 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "CNEXThread_PlayClipTask Create Done" );
}

CNEXThread_PlayClipTask::~CNEXThread_PlayClipTask( void )
{
	SAFE_RELEASE(m_pProjectMng);
	SAFE_RELEASE(m_pVideoRenderer);
	SAFE_RELEASE(m_pAudioRenderer);
	SAFE_RELEASE(m_pFileWriter);
	SAFE_RELEASE(m_pCliplist);
	
	if( m_pUserData != NULL )
	{
		nexSAL_MemFree(m_pUserData);
		m_pUserData = NULL;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "~~~~CNEXThread_PlayClipTask Destroy Done" );
}

int CNEXThread_PlayClipTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ---------- PM ProcessCommandMsg(%d)", __LINE__, pMsg->m_nMsgType);
	switch( pMsg->m_nMsgType )
	{
		/*
		case MESSAGE_UPDATE_CURRENTIME:
		{
			CNxMsgUpdateCurrentTimeStamp* pUpdateTime = (CNxMsgUpdateCurrentTimeStamp*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlayClipTask.cpp %d] playingClip Porcess(MsgRef:%d Time:%d)", __LINE__, pUpdateTime->GetRefCnt(), pUpdateTime->m_currentTimeStamp);			
			if( pUpdateTime )
			{
				if( playingClip(pUpdateTime->m_currentTimeStamp) == FALSE )
				{
					// clip.cpp에서 이벤트를 보내도록 수정함.
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlayClipTask.cpp %d] playingClip Error", __LINE__);
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[PlayClipTask.cpp %d] playingClip Porcess(%d)", __LINE__, pUpdateTime->m_currentTimeStamp);
				pUpdateTime->Release();
			}
			nexSAL_TaskSleep(5);
			return MESSAGE_PROCESS_OK;
		}
		*/
		case MESSAGE_STOP_PLAYTASK:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlayClipTask.cpp %d] MESSAGE_STOP_PLAYTASK(%p %p)", __LINE__, m_pProjectMng, m_pCliplist);
			if( m_pCliplist )
				m_pCliplist->stopPlay();
			break;
		}

		case MESSAGE_STATE_CHANGE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlayClipTask.cpp %d] MESSAGE_STATE_CHANGE(%p)", __LINE__, m_pProjectMng);
			CNxMsgChangeState* pStateChangeMsg = (CNxMsgChangeState*)pMsg;
			// The second image will be start before 2 sec after drawing first image in order to improve the performance
			CNexCodecManager::setImagePreparationTime(2000);
			if(pStateChangeMsg->m_currentState == PLAY_STATE_PAUSE)
			{
				pauseClip();
			}
			else if(pStateChangeMsg->m_currentState == PLAY_STATE_RESUME)
			{
				resumeClip();
			}
			break;
		}

		case MESSAGE_UPDATE_CLIP_FOR_EXPORT:
		{
			CNxMsgUpdateClip4Export* pUpdateMsg = (CNxMsgUpdateClip4Export*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlayClipTask.cpp %d] MESSAGE_UPDATE_CLIP_FOR_EXPORT(%d)", __LINE__, pUpdateMsg->m_uCurrentTime);
			updatePlay4Export(pUpdateMsg->m_uCurrentTime);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);
}

void CNEXThread_PlayClipTask::ProcessIdleState()
{
	nexSAL_TaskSleep(5);
}

void CNEXThread_PlayClipTask::ProcessPlayState()
{
	if( m_uiCurrentTime == CNexProjectManager:: getCurrentTimeStamp() )
	{
		nexSAL_TaskSleep(10);
		return;
	}

	if( playingClip(CNexProjectManager:: getCurrentTimeStamp()) == FALSE )
	{
		// clip.cpp에서 이벤트를 보내도록 수정함.
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlayClipTask.cpp %d] playingClip Error", __LINE__);
	}
}

void CNEXThread_PlayClipTask::ProcessRecordState()
{
	if( m_pAudioRenderer != NULL && m_uiCurrentTime == CNexProjectManager:: getCurrentTimeStamp() && m_uiCurrentTime != 0)
	{
		nexSAL_TaskSleep(10);
		return;
	}

	if( playingClip(CNexProjectManager:: getCurrentTimeStamp()) == FALSE )
	{
		// clip.cpp에서 이벤트를 보내도록 수정함.
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlayClipTask.cpp %d] playingClip Error", __LINE__);
	}
}

NXBOOL CNEXThread_PlayClipTask::setProjectManager(CNexProjectManager* pMng)
{
	SAFE_RELEASE(m_pProjectMng);
	m_pProjectMng = pMng;
	SAFE_ADDREF(m_pProjectMng);
	return TRUE;
}
	
NXBOOL CNEXThread_PlayClipTask::setRenderer(CNEXThreadBase* pVideo, CNEXThreadBase* pAudio)
{
	SAFE_ADDREF(pVideo);
	SAFE_RELEASE(m_pVideoRenderer);
	m_pVideoRenderer = pVideo;
	
	SAFE_ADDREF(pAudio);
	SAFE_RELEASE(m_pAudioRenderer);
	m_pAudioRenderer = pAudio;
	return TRUE;
}

NXBOOL CNEXThread_PlayClipTask::setFileWriter(CNexExportWriter* pFileWriter)
{
	SAFE_RELEASE(m_pFileWriter);
	m_pFileWriter = pFileWriter;
	SAFE_ADDREF(m_pFileWriter);
	return TRUE;
}

NXBOOL CNEXThread_PlayClipTask::setClipList(CClipList* pCliplist)
{
	SAFE_RELEASE(m_pCliplist);
	m_pCliplist = pCliplist;
	SAFE_ADDREF(m_pCliplist);
	return TRUE;
}

NXBOOL CNEXThread_PlayClipTask::setDirectEncodeUserData(char* pUserData)
{
	if( pUserData )
	{
		if( m_pUserData )
		{
			nexSAL_MemFree(m_pUserData);
			m_pUserData = NULL;
		}		
		m_pUserData = (char*)nexSAL_MemAlloc(strlen(pUserData)+1);
		strcpy(m_pUserData, pUserData);
	}
	return TRUE;
}

NXBOOL CNEXThread_PlayClipTask::setDirectExportOption(int option)
{
	m_iOption = option;
	return TRUE;
}

NXBOOL CNEXThread_PlayClipTask::playingClip(unsigned int uiCurrentTime)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[playClipTask.cpp %d] playingClip(%p %p %p %p %d %d)", __LINE__, m_pCliplist, m_pFileWriter, m_pVideoRenderer, m_pAudioRenderer, m_uiCurrentTime, uiCurrentTime);
	if( m_pProjectMng == NULL || m_pCliplist == NULL ) return FALSE;

	if( m_pFileWriter != NULL )
	{
	}
	else
	{
		if( m_pVideoRenderer == NULL || m_pAudioRenderer  == NULL )
		{
			return FALSE;
		}
	}
		
	if( m_uiCurrentTime > uiCurrentTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[playClipTask.cpp %d] Playing Done Project(%d %d)", __LINE__, m_uiCurrentTime, uiCurrentTime);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[playClipTask.cpp %d] playingClip(%d %d)", __LINE__, m_uiCurrentTime, uiCurrentTime);
	m_uiCurrentTime = uiCurrentTime;
	if( m_uiCurrentTime >= m_pCliplist->getClipTotalTime() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[playClipTask.cpp %d] Project play End(Cur:%d Total : %d)", __LINE__, m_uiCurrentTime, m_pCliplist->getClipTotalTime());
		// for loop play
		// m_pProjectMng->sendEvent(MESSAGE_CLIP_PLAY_END, m_uiCurrentTime);
		// m_pProjectMng->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		return TRUE;
	}
	
	if( m_pFileWriter != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[playClipTask.cpp %d] playingClip with FileWriter (Cur:%d Total : %d)", __LINE__, m_uiCurrentTime, m_pCliplist->getClipTotalTime());
		return m_pCliplist->updatePlay(m_uiCurrentTime, m_pAudioRenderer, m_pFileWriter, m_pUserData, m_iOption);
	}
	return m_pCliplist->updatePlay(m_uiCurrentTime, m_pVideoRenderer, m_pAudioRenderer);
}

NXBOOL CNEXThread_PlayClipTask::pauseClip()
{
	if( m_pProjectMng == NULL || m_pVideoRenderer == NULL || m_pAudioRenderer == NULL ) return FALSE;
	return m_pProjectMng->m_pClipList->pausePlay();		
}

NXBOOL CNEXThread_PlayClipTask::resumeClip()
{
	if( m_pProjectMng == NULL || m_pVideoRenderer == NULL || m_pAudioRenderer == NULL ) return FALSE;
	m_pProjectMng->m_pClipList->resumePlay(m_pAudioRenderer, m_pVideoRenderer);		
	return playingClip(CNexProjectManager:: getCurrentTimeStamp());
}

NXBOOL CNEXThread_PlayClipTask::updatePlay4Export(unsigned int uCurrentTime)
{
	NXBOOL bFail = TRUE;
	if( m_pProjectMng == NULL || m_pVideoRenderer == NULL || m_pAudioRenderer == NULL ) return FALSE;

	if(m_eThreadState != PLAY_STATE_PAUSE && m_pProjectMng->m_pClipList->getClipTotalTime() > uCurrentTime)
	{
		bFail = m_pCliplist->updatePlay4Export(uCurrentTime, m_pVideoRenderer, m_pAudioRenderer);
	}

	return bFail;
}

