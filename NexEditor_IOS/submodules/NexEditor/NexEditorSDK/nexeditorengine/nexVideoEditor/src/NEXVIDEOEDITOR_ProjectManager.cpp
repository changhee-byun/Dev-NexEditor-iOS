/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ProjectManager.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/05	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NexMediaDef.h"
#include "NEXVIDEOEDITOR_Util.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"

CNexProjectManager*	CNexProjectManager::m_pProjectManager = NULL;

//---------------------------------------------------------------------------
CNexProjectManager::CNexProjectManager( void )
{
	m_pClipList			= new CClipList;		
	m_pAudioRender	= NULL;
	m_pVideoEditor	= NULL;
	m_pPlayClipTask	= NULL;
	m_pThumbnailTask = NULL;
#ifdef _ANDROID
	m_pTranscodingTask = NULL;
    m_pFastPreviewTask = NULL;
#endif
	m_pFileWriter		= NULL;

	m_uiLastNotiTime	= 0;
	m_uLastTickCount    = 0xffffffff;
	m_uCurrentTickTime  = 0xffffffff;
	m_uAudioStartTime = 0;

	m_iRenderLeft		= 0;
	m_iRenderTop		= 0;
	m_iRenderRight	= 0;
	m_iRenderBottom	= 0;

	m_bPlayEnd		= FALSE;
	m_bMuteAudio	= FALSE;

#ifdef _ANDROID
	m_pTranscodingRenderer	= NULL;
	m_pTranscodingDecSurf	= NULL;
#endif
	m_pProjectManager = this;
	
	m_pVideoRender	= new CNEXThread_VideoRenderTask;
	m_pVideoRender->Begin();

	CNxMsgVideoRenderInfo* pVideoRenderInfo = new CNxMsgVideoRenderInfo;
	// Default Render Info
	pVideoRenderInfo->m_iLeft				= m_iRenderLeft;
	pVideoRenderInfo->m_iTop				= m_iRenderTop;
	pVideoRenderInfo->m_iRight			= m_iRenderRight;
	pVideoRenderInfo->m_iBottom			= m_iRenderBottom;

	m_pVideoRender->SendCommand(pVideoRenderInfo);
	SAFE_RELEASE(pVideoRenderInfo);

	m_pFileWriter	= new CNexFileWriter;

	int iHardwareMemSize = CNexVideoEditor::m_iSupportedMaxMemSize;
	int iHardwareDecMaxCount = CNexVideoEditor::m_iSupportDecoderMaxCount;
	int iHardwareEncMaxCount = CNexVideoEditor::m_iSupportEncoderMaxCount;
	m_CodecManager.setHardwareResource(	iHardwareMemSize, 
										iHardwareDecMaxCount,
										iHardwareEncMaxCount);	
	
	m_ImageTrackCache.clear();
	m_VideoTrackCache.clear();
	m_VideoCodecCache.clear();

	m_ThumbAudioVec.clear();
	m_uiExportingTime = 0;
	m_lMaxFileDuration = 0;

	m_iProjectVolumeFadeInTime	= 0;
	m_iProjectVolumeFadeOutTime	= 0;

	m_iProjectVolume			= -1;
	m_iMasterVolume				= 100;
	m_iSlaveVolume				= 100;
	m_bManualVolumeControl		= FALSE;

	m_pSetTimeTask = new CNEXThread_SetTimeTask();
	if( m_pSetTimeTask == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Play Clip Task Create failed(%p)", __LINE__, m_pSetTimeTask);
	}
	m_pSetTimeTask->setProjectManager(this);
	m_pSetTimeTask->setRenderer((CNEXThreadBase*)m_pVideoRender);
	
	m_pSetTimeTask->Begin();
	m_pSetTimeTask->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
	m_RecordToIdle 			= 0;
	m_iExportSamplingRate 	= EDITOR_DEFAULT_SAMPLERATE;
	m_iExportAudioBitRate 	= EDITOR_DEFAULT_AUDIO_BITRATE;
	m_iEncodeToTranscode 	= 0;

	m_isGIFMode				= FALSE;
}

CNexProjectManager::CNexProjectManager( CNexVideoEditor* pVE, CNexExportWriter *pWriter )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ---------- CNexProjectManager", __LINE__);

	m_pClipList		= new CClipList;
	m_pAudioRender	= NULL;
	m_pVideoEditor	= NULL;
	m_pPlayClipTask	= NULL;
	m_pThumbnailTask = NULL;
#ifdef _ANDROID
	m_pTranscodingTask = NULL;
    m_pFastPreviewTask = NULL;
#endif
 	m_uiLastNotiTime	= 0;
	m_uLastTickCount    = 0xffffffff;
	m_uCurrentTickTime  = 0xffffffff;
	m_uAudioStartTime = 0;
	
	m_iRenderLeft		= 0;
	m_iRenderTop		= 0;
	m_iRenderRight	= 0;
	m_iRenderBottom	= 0;

	m_bPlayEnd		= FALSE;
	m_bMuteAudio	= FALSE;
#ifdef _ANDROID
	m_pTranscodingRenderer	= NULL;
	m_pTranscodingDecSurf	= NULL;
#endif
	m_pProjectManager = this;
	
	m_pVideoEditor = pVE;
	SAFE_ADDREF(m_pVideoEditor);

	m_pVideoRender	= new CNEXThread_VideoRenderTask;

	NXBOOL bUseCache = TRUE;
#ifdef _ANDROID
	if( isMSM8974Device() )
	{
		bUseCache = FALSE;
	}
#endif
	m_pVideoRender->useSufaceTextureCache(bUseCache);
	m_pVideoRender->Begin();

	CNxMsgVideoRenderInfo* pVideoRenderInfo = new CNxMsgVideoRenderInfo;
	// Default Render Info
	pVideoRenderInfo->m_iLeft			= m_iRenderLeft;
	pVideoRenderInfo->m_iTop			= m_iRenderTop;
	pVideoRenderInfo->m_iRight		= m_iRenderRight;
	pVideoRenderInfo->m_iBottom		= m_iRenderBottom;

	m_pVideoRender->SendCommand(pVideoRenderInfo);
	SAFE_RELEASE(pVideoRenderInfo);

    m_pFileWriter	= pWriter != NULL ? pWriter : new CNexFileWriter;

	int iHardwareMemSize = CNexVideoEditor::m_iSupportedMaxMemSize;
	int iHardwareDecMaxCount = CNexVideoEditor::m_iSupportDecoderMaxCount;
	int iHardwareEncMaxCount = CNexVideoEditor::m_iSupportEncoderMaxCount;
	if( m_pVideoEditor )
	{
		char pValue[64];
		if( m_pVideoEditor->getProperty("HardWareCodecMemSize", pValue) == NEXVIDEOEDITOR_ERROR_NONE )
		{
			iHardwareMemSize = atoi(pValue);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] set HardWareCodecMemSize(%d)", __LINE__, iHardwareMemSize);

			// iHardwareMemSize = SUPPORTED_CONTENT_WIDTH * SUPPORTED_CONTENT_HEIGHT;
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] set HardWareCodecMemSize(%d)", __LINE__, iHardwareMemSize);
		}
		if( m_pVideoEditor->getProperty("HardWareDecMaxCount", pValue) == NEXVIDEOEDITOR_ERROR_NONE )
		{
			iHardwareDecMaxCount = atoi(pValue);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] set HardWareDecMaxCount(%d)", __LINE__, iHardwareDecMaxCount);
		}
		if( m_pVideoEditor->getProperty("HardWareEncMaxCount", pValue) == NEXVIDEOEDITOR_ERROR_NONE )
		{
			iHardwareEncMaxCount = atoi(pValue);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] set HardWareEncMaxCount(%d)", __LINE__, iHardwareEncMaxCount);
		}
		
	}

	m_CodecManager.setHardwareResource(	iHardwareMemSize, 
										iHardwareDecMaxCount	, 
										iHardwareEncMaxCount);

	//if( m_pVideoEditor->getPropertyBoolean("DeviceExtendMode", FALSE) )
	{
		m_pVideoRender->setSeparateEffect(TRUE);
		m_pClipList->setSeparateEffect(TRUE);
	}
	
	m_ImageTrackCache.clear();
	m_VideoTrackCache.clear();
	m_VideoCodecCache.clear();
	m_uiExportingTime = 0;
	m_lMaxFileDuration = 0;

	m_iProjectVolumeFadeInTime	= 0;
	m_iProjectVolumeFadeOutTime	= 0;

	m_iProjectVolume			= -1;
	m_iMasterVolume				= 100;
	m_iSlaveVolume				= 100;
	m_bManualVolumeControl		= FALSE;

	m_pSetTimeTask = new CNEXThread_SetTimeTask();
	if( m_pSetTimeTask == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Play Clip Task Create failed(%p)", __LINE__, m_pSetTimeTask);
	}
	m_pSetTimeTask->setProjectManager(this);
	m_pSetTimeTask->setRenderer((CNEXThreadBase*)m_pVideoRender);
	
	m_pSetTimeTask->Begin();
	m_pSetTimeTask->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
	m_RecordToIdle			= 0;
	m_iExportSamplingRate	= EDITOR_DEFAULT_SAMPLERATE;
	m_iExportAudioBitRate	= EDITOR_DEFAULT_AUDIO_BITRATE;
	m_iEncodeToTranscode	= 0;

	m_isGIFMode				= FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ---------- CNexProjectManager", __LINE__);
}

CNexProjectManager::~CNexProjectManager( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ~~~~~~CNexProjectManager", __LINE__);

	clearCacheTranscodingDecSurf();

	if( m_pAudioRender )
	{
		if( m_pAudioRender->IsWorking() )
			m_pAudioRender->End(1000);
		SAFE_RELEASE(m_pAudioRender);
	}

	if( m_pVideoRender )
	{
		if( m_pVideoRender->IsWorking() )
			m_pVideoRender->End(1000);
		SAFE_RELEASE(m_pVideoRender);
	}

	if( m_pPlayClipTask )
	{
		if( m_pPlayClipTask->IsWorking() )
		{
			m_pPlayClipTask->SendSimpleCommand(MESSAGE_STOP_PLAYTASK);
			m_pPlayClipTask->End(1000);
		}
		SAFE_RELEASE(m_pPlayClipTask);
	}

	if( m_pThumbnailTask )
	{
		if( m_pThumbnailTask->IsWorking() )
		{
			m_pThumbnailTask->End(1000);
		}
		SAFE_RELEASE(m_pThumbnailTask);
	}
#ifdef _ANDROID
	if( m_pTranscodingTask)
	{
		if( m_pTranscodingTask->IsWorking())
		{
			m_pTranscodingTask->End(1000);
		}
		SAFE_RELEASE(m_pTranscodingTask);
	}
#endif
	if( m_pSetTimeTask)
	{
		if( m_pSetTimeTask->IsWorking())
		{
			m_pSetTimeTask->End(1000);
		}
		SAFE_RELEASE(m_pSetTimeTask);
	}
#ifdef _ANDROID
    if( m_pFastPreviewTask)
    {
        if( m_pFastPreviewTask->IsWorking())
        {
            m_pFastPreviewTask->End(1000);
        }
        SAFE_RELEASE(m_pFastPreviewTask);
    }
#endif
	SAFE_RELEASE(m_pFileWriter);
	SAFE_RELEASE(m_pVideoEditor);

	clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE);
	clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
	clearCachedVisualCodecInfo();

	m_pProjectManager	= NULL;
	m_RecordToIdle = 0;
	SAFE_RELEASE(m_pClipList);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ~~~~~~CNexProjectManager End", __LINE__);
}

CNexProjectManager* CNexProjectManager::getProjectManager()
{
	if( m_pProjectManager == NULL )
		return NULL;

	SAFE_ADDREF(m_pProjectManager);
	return m_pProjectManager;
}

CNEXThread_VideoRenderTask* CNexProjectManager::getVideoRenderer()
{
	if( m_pProjectManager == NULL )
		return NULL;

	if( m_pProjectManager->m_pVideoRender == NULL )
		return NULL;

 	SAFE_ADDREF(m_pProjectManager->m_pVideoRender);
	return m_pProjectManager->m_pVideoRender;
}

CNEXThread_AudioRenderTask* CNexProjectManager::getAudioRenderer()
{
	if( m_pProjectManager == NULL )
		return NULL;

	if( m_pProjectManager->m_pAudioRender == NULL )
		return NULL;

	SAFE_ADDREF(m_pProjectManager->m_pAudioRender);
	return m_pProjectManager->m_pAudioRender;
}

PLAY_STATE CNexProjectManager::getCurrentState()
{
	if( m_pProjectManager == NULL )
		return PLAY_STATE_NONE;

	return m_pProjectManager->m_eThreadState;
}

unsigned int CNexProjectManager:: getCurrentTimeStamp()
{
	if( m_pProjectManager == NULL )
	{
		return 0;
	}
	return m_pProjectManager->getCurrentTime();
}

CClipList* CNexProjectManager::getClipList()
{
	CAutoLock m(m_LockClipList);
	SAFE_ADDREF(m_pClipList);
	return m_pClipList;
}

CVideoEffectItemVec* CNexProjectManager::getEffectItemVec()
{
	CAutoLock m(m_LockClipList);
	if( m_pClipList == NULL )
		return NULL;	
	return m_pClipList->getEffectItemVec();
}

int CNexProjectManager::getExportSamplingRate()
{
	if( m_pProjectManager == NULL )
		return PLAY_STATE_NONE;

	return m_pProjectManager->m_iExportSamplingRate;
}

int CNexProjectManager::getTotalAudioVolumeWhilePlay(int* pMasterVolume, int* pSlaveVolume)
{
	if( m_pProjectManager == NULL )
		return NEXVIDEOEDITOR_ERROR_GENERAL;

	if( m_pProjectManager->m_pAudioRender == NULL )
		return NEXVIDEOEDITOR_ERROR_GENERAL;

	if (pMasterVolume && pSlaveVolume)
	{
		*pMasterVolume = m_pProjectManager->m_pAudioRender->m_iMasterVolume;
		*pSlaveVolume = m_pProjectManager->m_pAudioRender->m_iSlaveVolume;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] getTotalAudioVolumeWhilePlay MasterVolume(%d) SlaveVolume(%d)", __LINE__, *pMasterVolume, *pSlaveVolume);
	}
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexProjectManager::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	if( m_pVideoEditor == NULL ) return MESSAGE_PROCESS_PASS;


#if 0
	unsigned char* pMemTestBuffer1 = (unsigned char*)nexSAL_MemAlloc(10*1024*1024);
	unsigned char* pMemTestBuffer2 = (unsigned char*)nexSAL_MemAlloc(10*1024*1024);
	if( pMemTestBuffer1 && pMemTestBuffer2 )
	{
		CNexPerformanceMonitor	checkPerformance("memCheck");
		memset(pMemTestBuffer1, 0x00, 10*1024*1024);
		memset(pMemTestBuffer2, 0x02, 10*1024*1024);

		checkPerformance.CheckModuleStart();
		for(int i = 0; i < 100; i++)
		{
			checkPerformance.CheckModuleUnitStart();
			memcpy(pMemTestBuffer1, pMemTestBuffer2, 10*1024*1024);
			checkPerformance.CheckModuleUnitEnd();
		}
		checkPerformance.CheckModuleLog();
		
		nexSAL_MemFree(pMemTestBuffer1);
		nexSAL_MemFree(pMemTestBuffer2);
	}
#endif
	
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ---------- PM ProcessCommandMsg(%d)", __LINE__, pMsg->m_nMsgType);
	switch( pMsg->m_nMsgType )
	{
		case MESSAGE_STATE_CHANGE:
		{
			CNxMsgChangeState* pStateChangeMsg = (CNxMsgChangeState*)pMsg;

			if( pStateChangeMsg->m_currentState == PLAY_STATE_PAUSE || pStateChangeMsg->m_currentState == PLAY_STATE_RESUME )
			{
				if(getPlayEnd())
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] No Action Pause/Resume since already finished", __LINE__);
					m_pVideoEditor->notifyEvent(MESSAGE_STATE_CHANGE_DONE, pStateChangeMsg->m_currentState, PLAY_STATE_RESUME, NEXVIDEOEDITOR_ERROR_NO_ACTION);	
					SAFE_RELEASE(pMsg);	
					if(pStateChangeMsg->m_currentState == PLAY_STATE_PAUSE)
						m_bIsTaskPaused = TRUE;
					else if(pStateChangeMsg->m_currentState == PLAY_STATE_RESUME)
						m_bIsTaskResumed = TRUE;
					return MESSAGE_PROCESS_OK;
				}
			}

			// for CHC-32 issue.
			if( pStateChangeMsg->m_currentState == PLAY_STATE_RUN || pStateChangeMsg->m_currentState == PLAY_STATE_RECORD )
			{
				if( m_isGIFMode )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] No Action state change because gif mode", __LINE__);
					m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
					SAFE_RELEASE(pMsg);
					return MESSAGE_PROCESS_OK;
				}

				if( m_eThreadState == PLAY_STATE_IDLE || m_eThreadState == PLAY_STATE_NONE )
				{
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] No Action state change with invalid state", __LINE__);
					m_pVideoEditor->notifyEvent(MESSAGE_STATE_CHANGE_DONE, m_eThreadState, m_eThreadState, NEXVIDEOEDITOR_ERROR_NO_ACTION);
					SAFE_RELEASE(pMsg);
					return MESSAGE_PROCESS_OK;
				}
			}

			if((m_eThreadState == PLAY_STATE_RECORD || m_eThreadState == PLAY_STATE_PAUSE) && pStateChangeMsg->m_currentState == PLAY_STATE_IDLE )
				m_RecordToIdle = 1;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_STATE_CHANGE Received State( %d -> %d)", 
				__LINE__, m_eThreadState, pStateChangeMsg->m_currentState);
					
			CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
			if( waitCmd != NULL )
			{
				if( CNexVideoEditor::m_bNexEditorSDK && 
					(pStateChangeMsg->m_currentState == PLAY_STATE_RUN || pStateChangeMsg->m_currentState == PLAY_STATE_RECORD) )
				{
					if( m_pSetTimeTask == NULL )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change failed because set time task invalid", __LINE__);
						return MESSAGE_PROCESS_OK;
					}

					m_pSetTimeTask->SendCommand(waitCmd);
					if( waitCmd->waitProcessDone(3000) == FALSE )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change failed because wait set time failed", __LINE__);
						return MESSAGE_PROCESS_OK; 					
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change set time wait end", __LINE__);
				}
				SAFE_RELEASE(waitCmd);
			}
 			changeState(pStateChangeMsg->m_currentState, pStateChangeMsg->m_iFlag);

			SAFE_RELEASE(pStateChangeMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_SET_TIME:
		{
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Time Event", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			if( m_pClipList == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Time Event", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			CNxMsgSetTime* pSetTime = (CNxMsgSetTime*)pMsg;

			m_pClipList->lockClipList();
			if( m_pClipList->getClipCount() == 0 )
			{
				m_pClipList->unlockClipList();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Time Event SetTime(%d) TotalTime(%d)", __LINE__, pSetTime->m_uiTime, m_pClipList->getClipTotalTime());
				m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			if( pSetTime->m_uiTime > m_pClipList->getClipTotalTime() )
			{
				m_pClipList->unlockClipList();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Time Event SetTime(%d) TotalTime(%d)", __LINE__, pSetTime->m_uiTime, m_pClipList->getClipTotalTime());
				m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}
			// RYU : if the currentTime is same with End time because setPreviewTime() skip to display the last video/image frame so it force to make smaller time than end time.   for Mantis 9322
			else if(pSetTime->m_uiTime == m_pClipList->getClipTotalTime() && m_pClipList->getClipTotalTime() > 0)
			{
				pSetTime->m_uiTime-=1;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive setTime (%d)", __LINE__, pSetTime->GetRefCnt());				
			
			m_uiCurrentTime = pSetTime->m_uiTime;

			if( CNexVideoEditor::m_bNexEditorSDK )
			{
				if( m_pSetTimeTask == NULL )
				{
					m_pClipList->unlockClipList();
					m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED, 1);
					SAFE_RELEASE(pMsg);
					return MESSAGE_PROCESS_OK;
				}
				m_pClipList->unlockClipList();
				pSetTime->setClipList((INxRefObj*)m_pClipList);
				m_pSetTimeTask->SendCommand(pSetTime);
				// m_pSetTimeTask->SendCommandWithClear(pMsg);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive setTime Command Time(%d) (%d)", __LINE__, m_uiCurrentTime, pSetTime->GetRefCnt());
				SAFE_RELEASE(pSetTime);		
				return MESSAGE_PROCESS_OK; 
			}
			else
			{
				if( m_pThumbnailTask && m_pThumbnailTask->IsWorking() )
				{
					m_pClipList->unlockClipList();
					m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_THUMBNAIL_BUSY, 1);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Time Event while the thumbnail processing", __LINE__);
					SAFE_RELEASE(pMsg);
					return MESSAGE_PROCESS_OK;
				}

#ifdef _ANDROID
				if( m_pTranscodingTask && m_pTranscodingTask->IsWorking())
				{
					m_pClipList->unlockClipList();
					m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY, 1);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Time Event while the transcoding processing", __LINE__);
					SAFE_RELEASE(pMsg);
					return MESSAGE_PROCESS_OK;
				}
#endif

				clearCacheTranscodingDecSurf();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive setTime Command Time(%d)", __LINE__, m_uiCurrentTime);
                
                pSetTime->setClipList((INxRefObj*)m_pClipList);
				if( pSetTime->m_iDisplay )
				{
					// case, for mantis 9094		
					if( m_pVideoRender )
						m_pVideoRender->SendSimpleCommand(MESSAGE_DELETE_TRACK_ALL);
					
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive setTime Command Time(%d) prepare image", __LINE__, m_uiCurrentTime);
					if (FALSE == m_pClipList->setPreviewTime(pSetTime->m_uiTime, pSetTime->m_iIDRFrame, m_pVideoRender))
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] setPreviewTime error m_uiTime(%u)", __LINE__, pSetTime->m_uiTime);
						m_pClipList->unlockClipList();
						m_pVideoEditor->notifyEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_IMAGE_PROCESS, 1);
						SAFE_RELEASE(pMsg);
						return MESSAGE_PROCESS_OK;
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive setTime Command Time(%d) ignore image", __LINE__, m_uiCurrentTime);
					//m_pClipList->setPreviewTime(pSetTime->m_uiTime, pSetTime->m_iIDRFrame, m_pVideoRender);
				}
				
				if( m_pVideoRender )
				{
					m_pVideoRender->SendCommand(pMsg);
				}

				m_pClipList->clearImageTask();
				m_pClipList->unlockClipList();
				SAFE_RELEASE(pSetTime);
			}
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_CAPTURE:
		{
			if( m_pVideoRender == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_CAPTURE_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}
			m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_PREPARE_CLIP_LOADING:
		{
			CNxMsgPrepareClipLoading* pClipLoading = (CNxMsgPrepareClipLoading*)pMsg;
			if( pClipLoading )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_PREPARE_CLIP_LOADING, pClipLoading->m_iClipID);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_UPDATE_CURRENTIME:
		{
			CNxMsgUpdateCurrentTimeStamp* pUpdateTime = (CNxMsgUpdateCurrentTimeStamp*)pMsg;
			if( pUpdateTime )
			{
				SAFE_RELEASE(pUpdateTime);
				return MESSAGE_PROCESS_OK; 
			}
			return MESSAGE_PROCESS_PASS;
 		}
		case MESSAGE_CLOSE_PROJECT:
		{
			CNxMsgCloseProject* pCloseMsg = (CNxMsgCloseProject*)pMsg;
			if( pCloseMsg == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_OPEN_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED);
				SAFE_RELEASE(pCloseMsg);
				return MESSAGE_PROCESS_OK; 
			}
			NEXVIDEOEDITOR_ERROR eErr = (NEXVIDEOEDITOR_ERROR)closeProject();
			if( eErr != NEXVIDEOEDITOR_ERROR_NONE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_CLOSE_PROJECT_DONE, eErr);
				SAFE_RELEASE(pCloseMsg);
				return MESSAGE_PROCESS_OK; 
			}
			m_pVideoEditor->notifyEvent(MESSAGE_CLOSE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_NONE);
			SAFE_RELEASE(pCloseMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_ADD_VISUAL_CLIP:
		{
			CNxMsgAddClip* pAddClipMsg = (CNxMsgAddClip*)pMsg;
			if( pAddClipMsg == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_ADD_CLIP_DONE, NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED, 0, -1, 0);
				SAFE_RELEASE(pAddClipMsg);
				return MESSAGE_PROCESS_OK; 
			}

			NEXVIDEOEDITOR_ERROR eErr = (NEXVIDEOEDITOR_ERROR)addVisualClip(pAddClipMsg->m_uiNextToClipID, pAddClipMsg->m_pClipPath, pAddClipMsg->m_iNewClipID);
			if( eErr != NEXVIDEOEDITOR_ERROR_NONE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_ADD_CLIP_DONE, eErr, 0, pAddClipMsg->m_iNewClipID, 0);
				SAFE_RELEASE(pAddClipMsg);
				return MESSAGE_PROCESS_OK; 
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ProjectManager.cpp %d] Add Visual clip done duration(%d)", __LINE__, getDuration());
			SAFE_RELEASE(pAddClipMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_ADD_AUDIO_CLIP:
		{
			CNxMsgAddClip* pAddClipMsg = (CNxMsgAddClip*)pMsg;
			if( pAddClipMsg == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_ADD_CLIP_DONE, NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED, 0, -1, 0);
				SAFE_RELEASE(pAddClipMsg);
				return MESSAGE_PROCESS_OK; 
			}


			NEXVIDEOEDITOR_ERROR eErr = (NEXVIDEOEDITOR_ERROR)addAudioClip(pAddClipMsg->m_uiStartTime, pAddClipMsg->m_pClipPath, pAddClipMsg->m_iNewClipID);
			if( eErr != NEXVIDEOEDITOR_ERROR_NONE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_ADD_CLIP_DONE, eErr, 0, pAddClipMsg->m_iNewClipID, 0);
				SAFE_RELEASE(pAddClipMsg);
				return MESSAGE_PROCESS_OK; 
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ProjectManager.cpp %d] Add Audio clip done duration(%d)", __LINE__, getDuration());
			SAFE_RELEASE(pAddClipMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_MOVE_CLIP:
		{
			CNxMsgMoveClip* pMoveClipMsg = (CNxMsgMoveClip*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Move clip (%p)", __LINE__, pMoveClipMsg);
			if( pMoveClipMsg == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Move Audio clip failed(%p)", __LINE__, pMoveClipMsg);
				m_pVideoEditor->notifyEvent(MESSAGE_MOVE_CLIP_DONE, NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED);
				SAFE_RELEASE(pMoveClipMsg);
				return MESSAGE_PROCESS_OK; 
			}

			if( pMoveClipMsg->m_uiNextToID == INVALID_CLIP_ID )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Move Audio clip (%d)", __LINE__, pMoveClipMsg->m_uiMoveClipID);
				NEXVIDEOEDITOR_ERROR eErr = (NEXVIDEOEDITOR_ERROR)moveAudioClip(pMoveClipMsg->m_uiMoveTime, pMoveClipMsg->m_uiMoveClipID);
				if( eErr != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Move Audio clip failed (%d)", __LINE__, pMoveClipMsg->m_uiMoveClipID);
					m_pVideoEditor->notifyEvent(MESSAGE_MOVE_CLIP_DONE, eErr);
					SAFE_RELEASE(pMoveClipMsg);
					return MESSAGE_PROCESS_OK; 
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Move Visual clip (%d %d)", __LINE__, pMoveClipMsg->m_uiNextToID, pMoveClipMsg->m_uiMoveClipID);
				NEXVIDEOEDITOR_ERROR eErr = (NEXVIDEOEDITOR_ERROR)moveVisualClip(pMoveClipMsg->m_uiNextToID, pMoveClipMsg->m_uiMoveClipID);
				if( eErr != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Move Visual clip failed (%d)", __LINE__, pMoveClipMsg->m_uiMoveClipID);
					m_pVideoEditor->notifyEvent(MESSAGE_MOVE_CLIP_DONE, eErr);
					SAFE_RELEASE(pMoveClipMsg);
					return MESSAGE_PROCESS_OK; 
				}
			}
			if( pMoveClipMsg->m_isEventSend )
				m_pVideoEditor->notifyEvent(MESSAGE_MOVE_CLIP_DONE, NEXVIDEOEDITOR_ERROR_NONE, pMoveClipMsg->m_uiMoveClipID);
			SAFE_RELEASE(pMoveClipMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_DELETE_CLIP:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP", __LINE__);

			CNxMsgDeleteClip* pDeleteClipMsg = (CNxMsgDeleteClip*)pMsg;
			if( pDeleteClipMsg == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_DELETE_CLIP_DONE, NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP error End", __LINE__);

				SAFE_RELEASE(pDeleteClipMsg);
				return MESSAGE_PROCESS_OK; 
			}

			if( m_pSetTimeTask != NULL )
			{
				CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
				if( waitCmd != NULL )
				{
					m_pSetTimeTask->SendCommand(waitCmd);
					if( waitCmd->waitProcessDone(3000) == FALSE ) {
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP setTime task wait Timeout", __LINE__);
					}
					else {
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP setTime task wait OK", __LINE__);
					}
					SAFE_RELEASE(waitCmd);
				}
			}

			if( m_pSetTimeTask != NULL )
			{
				CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
				if( waitCmd != NULL )
				{
					m_pSetTimeTask->SendCommand(waitCmd);
					if( waitCmd->waitProcessDone(3000) == FALSE ) {
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP setTime task wait Timeout", __LINE__);
					}
					else {
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP setTime task wait OK", __LINE__);
					}
					SAFE_RELEASE(waitCmd);
				}
			}

			NEXVIDEOEDITOR_ERROR eErr = (NEXVIDEOEDITOR_ERROR)deleteClip(pDeleteClipMsg->m_iClipID);
			if( eErr != NEXVIDEOEDITOR_ERROR_NONE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_DELETE_CLIP_DONE, eErr);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP error End(%d)", __LINE__, pDeleteClipMsg->m_iClipID);			
				SAFE_RELEASE(pDeleteClipMsg);
				return MESSAGE_PROCESS_OK; 
			}
			m_pVideoEditor->notifyEvent(MESSAGE_DELETE_CLIP_DONE, NEXVIDEOEDITOR_ERROR_NONE);


			CVideoTrackInfo* pClipTrack = getCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pDeleteClipMsg->m_iClipID);
			if( pClipTrack != NULL )
			{
				clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pClipTrack);
				SAFE_RELEASE(pClipTrack);
			}
			else {
				pClipTrack = getCachedVisualTrackInfo(CACHE_TYPE_IMAGE, pDeleteClipMsg->m_iClipID);
				if( pClipTrack != NULL ) {
					clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE, pClipTrack);
					SAFE_RELEASE(pClipTrack);	
				}
				
			}

			if( m_pVideoRender != NULL )
			{
				CNxMsgDeleteTrack* pDeleteMsg = new CNxMsgDeleteTrack(pDeleteClipMsg->m_iClipID);
				if( pDeleteMsg != NULL ) 
				{
					m_pVideoRender->SendCommand(pDeleteMsg);

					if( pDeleteMsg->waitProcessDone(1000) == FALSE ) {
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Delete Track Time out from Renderer(%d)", __LINE__, pDeleteClipMsg->m_iClipID);
					}

					SAFE_RELEASE(pDeleteMsg);
				}
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_DELETE_CLIP End(%d)", __LINE__, pDeleteClipMsg->m_iClipID);

			SAFE_RELEASE(pDeleteClipMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_UPDATE_CLIP:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CLIP", __LINE__);

			CNxMsgUpdateClipInfo* pUpdateClipInfo = (CNxMsgUpdateClipInfo*)pMsg;
			if( pUpdateClipInfo )
			{
				if(pUpdateClipInfo->m_pUpdatedClip )
				{
					m_pClipList->updateClipInfo((IClipItem*)pUpdateClipInfo->m_pUpdatedClip);
				}
				m_pClipList->reCalcTime();
				m_pVideoEditor->notifyEvent(MESSAGE_UPDATE_CLIP_DONE, NEXVIDEOEDITOR_ERROR_NONE);
				SAFE_RELEASE(pUpdateClipInfo);
			}
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_ENCODE_PROJECT:
		{
			CNxMsgEncodeProject* pEncode = (CNxMsgEncodeProject*)pMsg;
			if( pEncode == NULL )
			{
				return MESSAGE_PROCESS_PASS;
			}
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ENCODE_PROJECT state(%d) fileMax(%lld) (w:%d h:%d target:%s)", 
				__LINE__, m_eThreadState, pEncode->m_llMaxFileSize, pEncode->m_iWidth, pEncode->m_iHeight, pEncode->m_strPath);
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Invalid State(%d)", __LINE__,m_eThreadState);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK; 
			}
			
			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking() )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_THUMBNAIL_BUSY);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Play command while the thumbnail processing", __LINE__);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK;
			}
#ifdef _ANDROID
			if( m_pTranscodingTask && m_pTranscodingTask->IsWorking())
			{
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore encode project command while the transcoding processing", __LINE__);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK;
			}			
#endif			
			if( m_pFileWriter == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] File Writer handle is null", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INVAILED_HANDLE);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK; 
			}

			// Fixed GIONEE JIRA 337 issue
			CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
			if( waitCmd != NULL )
			{
				if( CNexVideoEditor::m_bNexEditorSDK )
				{
					if( m_pSetTimeTask == NULL )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change failed because set time task invalid", __LINE__);
						return MESSAGE_PROCESS_OK;
					}

					m_pSetTimeTask->SendCommand(waitCmd);
					if( waitCmd->waitProcessDone(3000) == FALSE )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change failed because wait set time failed", __LINE__);
						return MESSAGE_PROCESS_OK; 					
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change set time wait end", __LINE__);
				}
				SAFE_RELEASE(waitCmd);
			}

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();

			if( m_pVideoRender != NULL ) {
				m_pVideoRender->SendSimpleCommand(MESSAGE_DELETE_TRACK_ALL);

				CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
				if( waitCmd != NULL )
				{
					m_pVideoRender->SendCommand(waitCmd);
					if( waitCmd->waitProcessDone(1000) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Video render task command wait timeout", __LINE__);
					}
					else {
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Video render task command waiting ok", __LINE__);
					}
					SAFE_RELEASE(waitCmd);
				}
			}

			m_lMaxFileDuration = 0;
			m_lMaxFileDuration = pEncode->m_lMaxFileDuration;

			if( m_pFileWriter->initFileWriter(pEncode->m_llMaxFileSize, getDuration()) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite initFileWriter failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INIT_FAIL);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK; 
			}
			
			if( m_pFileWriter->setFilePath(pEncode->m_strPath) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite setFilePath failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INIT_FAIL);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK; 
			}

			m_iExportSamplingRate = pEncode->m_iSamplingRate;
			m_iExportAudioBitRate = pEncode->m_iAudioBitRate;
			if( m_pFileWriter->setAudioCodecInfo(eNEX_CODEC_A_AAC, m_iExportSamplingRate, (m_pVideoEditor->m_iAudioMultiChannelOut == 1) ? 6 : EDITOR_DEFAULT_CHANNELS, m_iExportAudioBitRate) == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite setAudioCodecInfo failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_AUDIO_DEC_INIT_FAIL);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pFileWriter->setVideoCodecInfo(pEncode->m_iVideoCodecType, pEncode->m_iWidth, pEncode->m_iHeight, pEncode->m_iWidth, pEncode->m_iHeight, pEncode->m_iFPS, pEncode->m_iBitRate, pEncode->m_iProfile, pEncode->m_iLevel) == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite setVideoCodecInfo failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_VIDEO_DEC_INIT_FAIL);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK; 
			}

			int iRotateState = 0;
			if( (pEncode->m_iFlag & GET_FLAG_ROTATE_90_MODE) == GET_FLAG_ROTATE_90_MODE )
			{
				iRotateState = 90;
			}
			else if( (pEncode->m_iFlag & GET_FLAG_ROTATE_180_MODE) == GET_FLAG_ROTATE_180_MODE )
			{
				iRotateState = 180;
			}
			else if( (pEncode->m_iFlag & GET_FLAG_ROTATE_270_MODE) == GET_FLAG_ROTATE_270_MODE )
			{
				iRotateState = 270;
			}
			
			if( (pEncode->m_iFlag & ENCODE2TRANSCODE) == ENCODE2TRANSCODE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ENCODE2TRANSCODE (%d)", __LINE__, pEncode->m_iFlag);
				m_iEncodeToTranscode = 1;
			}
			
			m_pFileWriter->setVideoRotate(iRotateState);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Rotate(%d)", __LINE__, iRotateState);

			if( m_pFileWriter->startFileWriter() == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite initFileWriter failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_START_FAIL);
				SAFE_RELEASE(pEncode);
				return MESSAGE_PROCESS_OK; 
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Change Record state", __LINE__);
			m_bMuteAudio = FALSE;
			changeState(PLAY_STATE_RECORD, 0);
			
			SAFE_RELEASE(pEncode);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_GET_DURATION:
		{
			unsigned int uiTotalDuration = getDuration();
			m_pVideoEditor->notifyEvent(MESSAGE_GET_DURATION_DONE, NEXVIDEOEDITOR_ERROR_NONE, uiTotalDuration);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_VIDEO_RENDER_POSITION:
		{
			CNxMsgVideoRenderPos* pPos = (CNxMsgVideoRenderPos*)pMsg;

			m_iRenderLeft	= pPos->m_iLeft;
			m_iRenderTop	= pPos->m_iTop;
			m_iRenderRight	= pPos->m_iRight;
			m_iRenderBottom	= pPos->m_iBottom;

			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pPos);

			m_pVideoEditor->notifyEvent(MESSAGE_VIDEO_RENDER_POSITION_DONE, NEXVIDEOEDITOR_ERROR_NONE);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_SET_THEME:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_THEME", __LINE__);
			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_SEND_EVENT:
		{
			CNxMsgSendEvent* pSendEvent  = (CNxMsgSendEvent*)pMsg;
			m_pVideoEditor->notifyEvent(pSendEvent->m_uiEventType, pSendEvent->m_uiParam1, pSendEvent->m_uiParam2, pSendEvent->m_uiParam3, pSendEvent->m_uiParam4);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_VIDEO_STARTED:
		{
#ifdef FOR_TEST_PREPARE_VIDEO
			CNexCodecManager::clearPrepareHardwareDecoderFlag();
#endif
			// restore to 4sec from 2sec. The second image is only started.
			CNexCodecManager::setImagePreparationTime(4000);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_VIDEO_STARTED(%p)", __LINE__, m_pAudioRender);
			if( m_pAudioRender )
				m_pAudioRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			m_pVideoEditor->notifyEvent(MESSAGE_VIDEO_STARTED);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_VIDEO_STARTED(%p)", __LINE__, m_pAudioRender);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_AUDIO_ON:
		{
			m_bMuteAudio = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_AUDIO_ON(%d)", __LINE__, m_bMuteAudio);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_AUDIO_OFF:
		{
			m_bMuteAudio = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_AUDIO_OFF(%d)", __LINE__, m_bMuteAudio);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_BACKGROUND_MUSIC:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_BACKGROUND_MUSIC", __LINE__);
			CNxMsgBackgroundMusic* pMusic = (CNxMsgBackgroundMusic*)pMsg;
			CClipItem* pClip = NULL;
			if( pMusic )
			{
				// for mantis 6748
				if( pMusic->m_pFilePath && strlen(pMusic->m_pFilePath) > 0 )
				{
					pClip = createClip(pMusic->m_iNewClipID, pMusic->m_pFilePath);
				}
				SAFE_RELEASE(pMusic);
			}
			m_pClipList->setBackGroundMusic(pClip);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_BACKGROUND_MUSIC Out(%p)", __LINE__, pClip);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_BACKGROUND_MUSIC_VOLUME:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_BACKGROUND_MUSIC_VOLUME", __LINE__);
			CNxMsgBackgroundMusicVolume* pMusicVol = (CNxMsgBackgroundMusicVolume*)pMsg;
			if( pMusicVol )
			{
				m_pClipList->setBackGroundMusicVolume(pMusicVol->m_iVolume, pMusicVol->m_iFadeInTime, pMusicVol->m_iFadeOutTime);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_BACKGROUND_MUSIC_VOLUME Out(%d)", __LINE__, pMusicVol->m_iVolume);
				SAFE_RELEASE(pMusicVol);
			}

			/* delete for mantis 5495
			switch( m_eThreadState )
			{
				case PLAY_STATE_RUN:
				case PLAY_STATE_RECORD:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_BACKGROUND_MUSIC_VOLUME Out(%d)", __LINE__, pMusicVol->m_iVolume);
					break;
			};
			*/
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_CMD_MARKER:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive MESSAGE_COMMAND_MARKER(%d)", __LINE__, pMsg->m_nResult);
			m_pVideoEditor->notifyEvent(MESSAGE_CMD_MARKER_CHECKED, pMsg->m_nResult);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_CMD_CLEAR_PROJECT:
		{
			m_uiCurrentTime = 0;
			changeState(PLAY_STATE_IDLE, 0);
			clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE);
			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();

			// Fixed for renderer clear problem when undo and redo.
			/*
			if( m_pVideoRender )
			{
				m_pVideoRender->SendSimpleCommand(MESSAGE_VIDEO_RENDER_CLEAR);
			}
			 */
			break;
		}
		case MESSAGE_SET_PROJECT_EFFECT:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_PROJECT_EFFECT", __LINE__);
			CNxMsgSetProjectEffect* pProjectEffect = (CNxMsgSetProjectEffect*)pMsg;
			if( pProjectEffect != NULL && m_pVideoRender != NULL )
			{
				m_pVideoRender->SendCommand(pProjectEffect);
				SAFE_RELEASE(pProjectEffect);
				return MESSAGE_PROCESS_OK;
			}
			break;
		}
		case MESSAGE_SET_PROJECT_VOLUME_FADE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_PROJECT_VOLUME_FADE", __LINE__);
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Time Event", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_PROJECT_VOLUME_FADE, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			CNxMsgProjectVolumeFade* pFadeVolTime =  (CNxMsgProjectVolumeFade*)pMsg;
			if( pFadeVolTime != NULL )
			{
				m_iProjectVolumeFadeInTime	= pFadeVolTime->m_iFadeInTime;
				m_iProjectVolumeFadeOutTime	= pFadeVolTime->m_iFadeOutTime;
				SAFE_RELEASE(pFadeVolTime);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_PROJECT_VOLUME_FADE(%d %d)", __LINE__, m_iProjectVolumeFadeInTime, m_iProjectVolumeFadeOutTime);
			}
			m_pVideoEditor->notifyEvent(MESSAGE_SET_PROJECT_VOLUME_FADE, 0);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_VDEC_INIT_FAIL:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive Dec init Failed event(%d)", __LINE__, pMsg->m_nMsgType);
			m_pVideoEditor->notifyEvent(MESSAGE_VDEC_INIT_FAIL, NEXVIDEOEDITOR_ERROR_CODEC_INIT, 1, 0);
			break;
		}
		case MESSAGE_ADEC_INIT_FAIL:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Receive Dec init Failed event(%d)", __LINE__, pMsg->m_nMsgType);
			m_pVideoEditor->notifyEvent(MESSAGE_ADEC_INIT_FAIL, NEXVIDEOEDITOR_ERROR_CODEC_INIT, 0, 0);
			break;
		}
		case MESSAGE_GETCLIPINFO_BACKGROUND:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_GETCLIPINFO_BACKGROUND", __LINE__);
			
			CNxMsgBackgroundGetInfo* pGetInfo = (CNxMsgBackgroundGetInfo*)pMsg;

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, pGetInfo->m_iUserTag, 0);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( startAudioThumbTask(pGetInfo) )
			{
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking() )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_DONE, NEXVIDEOEDITOR_ERROR_INPROGRESS_GETCLIPINFO, pGetInfo->m_iUserTag, 0);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore get clip info command while the thumbnail inprogress", __LINE__);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}
#ifdef _ANDROID
			if( m_pTranscodingTask && m_pTranscodingTask->IsWorking())
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY, pGetInfo->m_iUserTag, 0);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore get clip info command while the transcoding processing", __LINE__);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}
#endif
			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();

			m_pThumbnailTask = new CNEXThread_ThumbnailTask(this);
			if( m_pThumbnailTask == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED, pGetInfo->m_iUserTag, 0);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}

			m_pThumbnailTask->Begin();
			m_pThumbnailTask->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_GETCLIPINFO_BACKGROUND_DONE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_GETINFO_BACKGROUND_DONE", __LINE__);

			CNxMsgBackgroundGetInfo* pGetInfo = (CNxMsgBackgroundGetInfo*)pMsg;

			if( endAudioThumbTask(pGetInfo) )
			{
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;				
			}
			
			if( m_pThumbnailTask )
			{
				if( m_pThumbnailTask->IsWorking() )
				{
					m_pThumbnailTask->End(1000);
				}
				SAFE_RELEASE(m_pThumbnailTask);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_GETINFO_BACKGROUND_DONE(%d)", __LINE__, pGetInfo->m_nResult);
			m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_DONE, pGetInfo->m_nResult, pGetInfo->m_iUserTag, 0);
			if(pGetInfo->m_nResult == NEXVIDEOEDITOR_ERROR_BACKGROUND_USER_CANCEL)
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_STOP_DONE, pGetInfo->m_nResult, pGetInfo->m_iUserTag, 0);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_GETCLIPINFO_BACKGROUND_STOP:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_GETCLIPINFO_BACKGROUND_STOP", __LINE__);
			
			CNxMsgBackgroundGetInfoStop* pGetInfo = (CNxMsgBackgroundGetInfoStop*)pMsg;

			if( stopAudioThumbTask(pGetInfo) )
			{
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;				
			}
			
			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking() )
			{
				m_pThumbnailTask->setStopThumb();
			}
			else
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_STOP_DONE, NEXVIDEOEDITOR_ERROR_NONE, pGetInfo->m_iUserTag, 0);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore get clip info command while the thumbnail inprogress", __LINE__);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
#ifdef _ANDROID
		case MESSAGE_TRANSCODING_START:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_TRANSCODING_START", __LINE__);

			CNxMsgTranscodingInfo* pTranscodingInfo = (CNxMsgTranscodingInfo*)pMsg;
			if( pTranscodingInfo == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_TRANSCODING_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN, pTranscodingInfo->m_iUserTag);
				SAFE_RELEASE(pTranscodingInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pVideoRender == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_TRANSCODING_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN, pTranscodingInfo->m_iUserTag);
				SAFE_RELEASE(pTranscodingInfo);
				return MESSAGE_PROCESS_OK;
			}			

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_TRANSCODING_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, pTranscodingInfo->m_iUserTag);
				SAFE_RELEASE(pTranscodingInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pTranscodingTask && m_pTranscodingTask->IsWorking() )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_TRANSCODING_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY, pTranscodingInfo->m_iUserTag);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore transcoding command while the transcoding processing", __LINE__);
				SAFE_RELEASE(pTranscodingInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking())
			{
				m_pVideoEditor->notifyEvent(MESSAGE_TRANSCODING_DONE, NEXVIDEOEDITOR_ERROR_THUMBNAIL_BUSY, pTranscodingInfo->m_iUserTag);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore transcoding commandwhile the thumbnail processing", __LINE__);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo(); 
            
			m_pTranscodingTask = new CNEXThread_TranscodingTask(this);
			if( m_pTranscodingTask == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_TRANSCODING_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED, pTranscodingInfo->m_iUserTag);
				SAFE_RELEASE(pTranscodingInfo);
				return MESSAGE_PROCESS_OK;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_TRANSCODING_START(Dec surface : %p)", __LINE__, m_pTranscodingDecSurf);
			pTranscodingInfo->m_pDecSurface = m_pTranscodingDecSurf;
			m_iEncodeToTranscode = 1;
			m_pTranscodingTask->Begin();
			m_pTranscodingTask->SendCommand(pTranscodingInfo);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_TRANSCODING_DONE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_TRANSCODING_DONE", __LINE__);

			CNxMsgTranscodingInfo* pTranscodingInfo = (CNxMsgTranscodingInfo*)pMsg;

			if( m_pTranscodingTask )
			{
				if( m_pTranscodingTask->IsWorking() )
				{
					m_pTranscodingTask->End(1000);
				}
				SAFE_RELEASE(m_pTranscodingTask);
			}
			
			m_iEncodeToTranscode = 0;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_TRANSCODING_DONE", __LINE__);
			m_pVideoEditor->notifyEvent(MESSAGE_TRANSCODING_DONE, pTranscodingInfo->m_nResult, pTranscodingInfo->m_iUserTag);

			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_TRANSCODING_STOP:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_TRANSCODING_STOP", __LINE__);

			CNxMsgTranscodingInfo* pTranscodingInfo = (CNxMsgTranscodingInfo*)pMsg;

			if( m_pTranscodingTask )
			{
				m_pTranscodingTask->cancelTranscoding();
				}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_TRANSCODING_STOP", __LINE__);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
#endif
		case MESSAGE_CLEAR_SCREEN:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_CLEAR_SCREEN", __LINE__);
			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_FAST_OPTION_PREVIEW:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_OPTION_PREVIEW", __LINE__);
			if( m_pVideoRender )
			{
				if(m_pClipList->getClipCount() > 0)
					m_pVideoRender->SendCommand(pMsg);
				else
					m_pVideoEditor->notifyEvent(MESSAGE_FAST_OPTION_PREVIEW_DONE, 1, 0, 0, 0);
			}
			else
			{
				m_pVideoEditor->notifyEvent(MESSAGE_FAST_OPTION_PREVIEW_DONE, 1, 0, 0, 0);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_CLEAR_TEXTURE:
		{
			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_PAUSE_RENDERER:
		case MESSAGE_RESUME_RENDERER:
		{
			if( m_pAudioRender )
				m_pAudioRender->SendCommand(pMsg);
			
			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_CLEAR_TRACK_CACHE:
		{
			clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE);
			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_ADD_LAYER_ITEM:
		{
			CNxMsgAddLayerItem* pAddLayerItem = (CNxMsgAddLayerItem*)pMsg;
			if( pAddLayerItem != NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_ADD_LAYER_ITEM(%p)", __LINE__, pAddLayerItem->m_pItem);
				m_pClipList->addLayerItem((ILayerItem*)pAddLayerItem->m_pItem);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_DELETE_LAYER_ITEM:
		{
			CNxMsgDeleteLayerItem* pDeleteLayerItem = (CNxMsgDeleteLayerItem*)pMsg;
			if( pDeleteLayerItem != NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_DELETE_LAYER_ITEM(%d)", __LINE__, pDeleteLayerItem->m_iLayerID);
				m_pClipList->deleteLayerItem(pDeleteLayerItem->m_iLayerID);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_CLEAR_LAYER_ITEM:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_CLEAR_LAYER_ITEM", __LINE__);
			m_pClipList->clearLayerItem();
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}		
		case MESSAGE_UPDATE_LOADLIST:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_LOADLIST", __LINE__);
            std::map<int, int>* mapper = m_pVideoRender ? new std::map<int,int> : nullptr;
            int preClipCount = m_pClipList->getClipCount();
            m_pClipList->reCalcTimeForLoadList(FALSE, mapper);
            int postClipCount = m_pClipList->getClipCount();                    
            SAFE_RELEASE(pMsg);
            if(preClipCount == postClipCount) {

                SAFE_DELETE(mapper);
            }
            if(m_pVideoRender) {

                m_pVideoRender->setMapper(mapper);
            }
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_GETTHUMB_RAWDATA:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_GETTHUMB_RAWDATA", __LINE__);
			
			CNxMsgBackgroundGetInfo* pGetInfo = (CNxMsgBackgroundGetInfo*)pMsg;

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETTHUMB_RAWDATA_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, pGetInfo->m_iUserTag, 0);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( pGetInfo->m_iAudioPCMLevel == 1 )
			{
				CNEXThread_ThumbnailTask* pTask = new CNEXThread_ThumbnailTask(this);
				if( pTask == NULL )
				{
					m_pVideoEditor->notifyEvent(MESSAGE_GETTHUMB_RAWDATA_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED, pGetInfo->m_iUserTag, 0);
					return TRUE;
				}
				pTask->setUserTag(pGetInfo->m_iUserTag);
				m_ThumbAudioVec.insert(m_ThumbAudioVec.end(), pTask);
				pTask->Begin();
				pTask->SendCommand(pGetInfo);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Start Audio Thumb task(%d)", __LINE__, pGetInfo->m_iUserTag);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}


			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking() )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETTHUMB_RAWDATA_DONE, NEXVIDEOEDITOR_ERROR_INPROGRESS_GETCLIPINFO, pGetInfo->m_iUserTag, 0);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore get clip info command while the thumbnail inprogress", __LINE__);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}

#ifdef _ANDROID
			if( m_pTranscodingTask && m_pTranscodingTask->IsWorking())
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETTHUMB_RAWDATA_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY, pGetInfo->m_iUserTag, 0);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore get clip info command while the transcoding processing", __LINE__);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}
#endif

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();

			m_pThumbnailTask = new CNEXThread_ThumbnailTask(this);
			if( m_pThumbnailTask == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETTHUMB_RAWDATA_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED, pGetInfo->m_iUserTag, 0);
				SAFE_RELEASE(pGetInfo);
				return MESSAGE_PROCESS_OK;
			}

			m_pThumbnailTask->Begin();
			m_pThumbnailTask->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_GETTHUMB_RAWDATA_DONE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_GETTHUMB_RAWDATA_DONE", __LINE__);

			CNxMsgBackgroundGetInfo* pGetInfo = (CNxMsgBackgroundGetInfo*)pMsg;

			if( endAudioThumbTask(pGetInfo) )
			{
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;				
			}
			
			if( m_pThumbnailTask )
			{
				if( m_pThumbnailTask->IsWorking() )
				{
					m_pThumbnailTask->End(1000);
				}
				SAFE_RELEASE(m_pThumbnailTask);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_GETTHUMB_RAWDATA_DONE", __LINE__);
			m_pVideoEditor->notifyEvent(MESSAGE_GETTHUMB_RAWDATA_DONE, pGetInfo->m_nResult, pGetInfo->m_iUserTag, 0);
			if(pGetInfo->m_nResult == NEXVIDEOEDITOR_ERROR_BACKGROUND_USER_CANCEL)
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_STOP_DONE, pGetInfo->m_nResult, pGetInfo->m_iUserTag, 0);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}		

		case MESSAGE_LOAD_THEMEANDEFFECT:
		{
			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_LOAD_RENDERITEM:
		{
			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_CLEAR_RENDERITEMS:
		{
			if( m_pVideoRender )
				m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}

#ifdef _ANDROID
		case MESSAGE_MAKE_HIGHLIGHT_START:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_HIGHLIGHT", __LINE__);

			CNxMsgHighLightIndex* pHighLight = (CNxMsgHighLightIndex*)pMsg;
			if( pHighLight == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_HIGHLIGHT_START_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pVideoRender == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_HIGHLIGHT_START_DONE, NEXVIDEOEDITOR_ERROR_RENDERER_INIT);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;
			}			

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_HIGHLIGHT_START_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pTranscodingTask && m_pTranscodingTask->IsWorking() )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_HIGHLIGHT_START_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore highlight command while the transcoding processing", __LINE__);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking())
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_HIGHLIGHT_START_DONE, NEXVIDEOEDITOR_ERROR_THUMBNAIL_BUSY);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore highlight commandwhile the thumbnail processing", __LINE__);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;
			}

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();            
			m_pTranscodingTask = new CNEXThread_TranscodingTask(this);
			if( m_pTranscodingTask == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_HIGHLIGHT_START_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_TRANSCODING_START(Dec surface : %p)", __LINE__, m_pTranscodingDecSurf);
			pHighLight->m_pDecSurface = m_pTranscodingDecSurf;
			
			m_pTranscodingTask->Begin();
			m_pTranscodingTask->SendCommand(pHighLight);
			SAFE_RELEASE(pMsg);			
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_MAKE_HIGHLIGHT_START_DONE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_HIGHLIGHT_START_DONE", __LINE__);
			CNxMsgHighLightIndex* pHighLight = (CNxMsgHighLightIndex*)pMsg;

			if( m_pTranscodingTask )
			{
				if( m_pTranscodingTask->IsWorking() )
				{
					m_pTranscodingTask->End(1000);
				}
				SAFE_RELEASE(m_pTranscodingTask);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_HIGHLIGHT_START_DONE", __LINE__);
			m_pVideoEditor->notifyEvent(MESSAGE_MAKE_HIGHLIGHT_START_DONE, pHighLight->m_nResult, pHighLight->m_iUserTag);
			
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_MAKE_HIGHLIGHT_STOP:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_HIGHLIGHT_STOP", __LINE__);

			if( m_pTranscodingTask != NULL ) // need checking about the transcoding task run with highlight
			{
				m_pTranscodingTask->cancelTranscoding();
			}
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_HIGHLIGHT_STOP", __LINE__);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_MAKE_HIGHLIGHT_STOP_DONE:
		{
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
#endif		
		case MESSAGE_ASYNC_UPDATE_CLIPLIST:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_ASYNC_UPDATE_CLIPLIST", __LINE__);
			CNxMsgAsyncUpdateCliplist* pAyncUpdate = (CNxMsgAsyncUpdateCliplist*)pMsg;
			if( pAyncUpdate == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore async load list because msg failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore async load list because invalid state", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			if( (pAyncUpdate->m_iOption & 0x00000001 ) == 0x00000001 )
			{
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clear track cache", __LINE__);
				clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE);
				clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
				clearCachedVisualCodecInfo();                
			}

			if( (pAyncUpdate->m_iOption & 0x00000010 ) == 0x00000010 )
			{
			}
			else
			{
				if( m_pVideoRender )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] send  MESSAGE_CLEAR_TEXTURE to video renderer", __LINE__);
					m_pVideoRender->SendSimpleCommand(MESSAGE_CLEAR_TEXTURE);
					// m_pVideoRender->SendCommand(pMsg);				
				}
			}

			CClipList* pCliplist = (CClipList*)pAyncUpdate->m_pClipList;

			if( m_pClipList != NULL )
			{
				CNxMsgReleaseRendererCache* rrc = new CNxMsgReleaseRendererCache();
				if( rrc != NULL )
				{
					if( pCliplist == NULL )
					{
						for(int i = 0; i < m_pClipList->getClipCount(); i++)
						{
							IClipItem* pitem_of_old = m_pClipList->getClip(i);
							if( pitem_of_old != NULL )
								rrc->addID(pitem_of_old->getClipID());
							SAFE_RELEASE(pitem_of_old);
						}						
					}
					else
					{
						for(int i = 0; i < m_pClipList->getClipCount(); i++)
						{
							IClipItem* pitem_of_old = m_pClipList->getClip(i);
							if( pitem_of_old != NULL && (pitem_of_old->getClipType() == CLIPTYPE_VIDEO || pitem_of_old->getClipType() == CLIPTYPE_VIDEO_LAYER ))
							{
								IClipItem* pitem = pCliplist->getClipUsingID(pitem_of_old->getClipID());
								if( pitem == NULL )
								{
									rrc->addID(pitem_of_old->getClipID());
								}
								SAFE_RELEASE(pitem);
							}
							SAFE_RELEASE(pitem_of_old);
						}
					}

					if( m_pVideoRender != NULL )
					{
						m_pVideoRender->SendCommand(rrc);
					}
					SAFE_RELEASE(rrc);
				}
			}

			// For cliplist by case.
			CAutoLock m(m_LockClipList);

			SAFE_RELEASE(m_pClipList);
			m_pClipList = pCliplist;
			SAFE_ADDREF(m_pClipList);
			
			CNexVideoEditor::m_iVideoLayerProject = 0;

			if( m_pClipList != NULL ){

				for(int i = 0; i < m_pClipList->getClipCount(); ++i){

					IClipItem* pitem = m_pClipList->getClip(i);
					if(pitem->getClipType() == CLIPTYPE_VIDEO_LAYER){

						CNexVideoEditor::m_iVideoLayerProject = 1;
						SAFE_RELEASE(pitem);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp list %d] is videolayer project", __LINE__);
						break;
					}
					else{

						SAFE_RELEASE(pitem);
					}
				}
			}

			//NXBOOL bDeviceExtend = m_pVideoEditor->getPropertyBoolean("DeviceExtendMode", FALSE);
			//if( bDeviceExtend )
			{
				if( m_pVideoRender != NULL )
				{
					m_pVideoRender->setSeparateEffect(TRUE);
				}

				if( m_pClipList != NULL )
				{
					m_pClipList->setSeparateEffect(TRUE);
				}
			}

			if( (pAyncUpdate->m_iOption & 0x00000100 ) == 0x00000100 )
			{
				if( m_pClipList )
				{
					m_pClipList->reCalcTimeForLoadList(TRUE);
					// m_pClipList->printClipInfo();;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] loadlist", __LINE__);
				}
			}
			else
			{
				if( m_pClipList )
				{
                    std::map<int, int>* mapper = m_pVideoRender ? new std::map<int,int> : nullptr;
                    int preClipCount = m_pClipList->getClipCount();
                    m_pClipList->reCalcTimeForLoadList(FALSE, mapper);
                    int postClipCount = m_pClipList->getClipCount();                    
                    SAFE_RELEASE(pMsg);
                    if(preClipCount == postClipCount) {

                        SAFE_DELETE(mapper);
                    }
                    if(m_pVideoRender) {

                        m_pVideoRender->setMapper(mapper);
                    }
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] loadlist recalc time end", __LINE__);
				}
			}

			m_uiCurrentTime = 0;
			m_pVideoEditor->notifyEvent(MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE, NEXVIDEOEDITOR_ERROR_NONE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_ASYNC_UPDATE_CLIPLIST End", __LINE__);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		
		case MESSAGE_ASYNC_UPDATE_EFFECTLIST:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_ASYNC_UPDATE_EFFECTLIST", __LINE__);
			CNxMsgAsyncUpdateEffectlist* pAyncUpdate = (CNxMsgAsyncUpdateEffectlist*)pMsg;
			if( pAyncUpdate == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore async effect list because msg failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore async effect list because invalid state", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			CClipList* pCliplist = (CClipList*)pAyncUpdate->m_pClipList;

			if( m_pClipList != NULL )
			{
				pCliplist->m_pEffectItemVec->lock();
				m_pClipList->m_pEffectItemVec->lock();
				for(int i=0; i<pCliplist->m_pEffectItemVec->getCount(); i++)
				{
					CVideoEffectItem* pEffectItem = pCliplist->m_pEffectItemVec->getEffetItem(i);

					CVideoEffectItem* pNewEffectItem = new CVideoEffectItem();
					if( pNewEffectItem )
					{
						pNewEffectItem->setEffectInfo(pEffectItem->m_uiStartTime, pEffectItem->m_uiEndTime, pEffectItem->m_uiEffectStartTime, pEffectItem->m_uiEffectDuration, pEffectItem->m_iEffectOffset, pEffectItem->m_iEffectOverlap, pEffectItem->m_pEffectID, pEffectItem->m_pTitle, pEffectItem->m_iCurrentIdx, pEffectItem->m_iTotalCount);
						pNewEffectItem->setTitleEffectInfo(pEffectItem->m_uiStartTime, pEffectItem->m_uiEndTime, pEffectItem->m_uiTitleStartTime, pEffectItem->m_uiTitleEndTime, pEffectItem->m_pTitleEffectID, pEffectItem->m_pTitle, pEffectItem->m_iCurrentIdx, pEffectItem->m_iTotalCount);
						
						m_pClipList->m_pEffectItemVec->addEffectItem(pEffectItem);
						SAFE_RELEASE(pNewEffectItem);
					}
				}
				m_pClipList->m_pEffectItemVec->unlock();
				pCliplist->m_pEffectItemVec->unlock();
			}

			//m_pVideoEditor->notifyEvent(MESSAGE_ASYNC_UPDATE_CLIPLIST_DONE, NEXVIDEOEDITOR_ERROR_NONE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_ASYNC_UPDATE_EFFECTLIST End", __LINE__);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
#ifdef _ANDROID		
		case MESSAGE_MAKE_CHECK_DIRECT_EXPORT:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_CHECK_DIRECT_EXPORT", __LINE__);

			int iRet = 1;

			CNxMsgCheckDirectExport* pCheck = (CNxMsgCheckDirectExport*)pMsg;
			if( pCheck != NULL )
			{
				if( m_pClipList == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_CHECK_DIRECT_EXPORT failed(%d)", __LINE__);
					m_pVideoEditor->notifyEvent(MESSAGE_MAKE_CHECK_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_CHECK_ERROR, 2);
					SAFE_RELEASE(pCheck);
					return MESSAGE_PROCESS_OK;
				}

    			if( m_eThreadState != PLAY_STATE_IDLE )
    			{
    				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START invalid state(%d)", __LINE__, m_eThreadState);
    				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_CHECK_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, m_eThreadState);
    				SAFE_RELEASE(pCheck);
    				return MESSAGE_PROCESS_OK;
    			}

				iRet = m_pClipList->checkDirectExport(pCheck->m_iFlag);
				if( iRet == 0 )
				{
					m_pVideoEditor->notifyEvent(MESSAGE_MAKE_CHECK_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_NONE, iRet);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_CHECK_DIRECT_EXPORT End(%d)", __LINE__, iRet);
					SAFE_RELEASE(pCheck);
					return MESSAGE_PROCESS_OK;
				}
    			if( iRet < 0 ) //yoon
    			{
    			    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_CHECK_DIRECT_EXPORT Cancel (%d)", __LINE__, iRet);
    				SAFE_RELEASE(pCheck);
    				return MESSAGE_PROCESS_OK;
    			}
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_CHECK_DIRECT_EXPORT failed(%d)", __LINE__, iRet);
			m_pVideoEditor->notifyEvent(MESSAGE_MAKE_CHECK_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_CHECK_ERROR, iRet);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		
		case MESSAGE_MAKE_DIRECT_EXPORT_START:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START", __LINE__);
			CNxMsgDirectExport* pDirect = (CNxMsgDirectExport*)pMsg;

			CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
			if( waitCmd != NULL )
			{
				if( CNexVideoEditor::m_bNexEditorSDK )
				{
					if( m_pSetTimeTask == NULL )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change failed because set time task invalid", __LINE__);
						return MESSAGE_PROCESS_OK;
					}

					m_pSetTimeTask->SendCommand(waitCmd);
					if( waitCmd->waitProcessDone(3000) == FALSE )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change failed because wait set time failed", __LINE__);
						return MESSAGE_PROCESS_OK; 					
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] State change set time wait end", __LINE__);
				}
				SAFE_RELEASE(waitCmd);
			}

			if( pDirect == NULL ) 
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pClipList == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START failed(%d)", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_CHECK_ERROR, 2);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK;
			}

			int iRet = m_pClipList->checkDirectExport(pDirect->m_iFlag);
			if( iRet != 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START failed(%d)", __LINE__, iRet);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_CHECK_ERROR, iRet);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK;
			}

			/*
			if( m_pClipList->checkDirectExportForEncoder() == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_DSI_DIFF_ERROR);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK;
			}			
			*/

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START invalid state(%d)", __LINE__, m_eThreadState);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, m_eThreadState);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START processing for thumbnail", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_THUMBNAIL_BUSY);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pTranscodingTask && m_pTranscodingTask->IsWorking())
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START processing for transcoding", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pFileWriter == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START File Writer handle is null", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INVAILED_HANDLE);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();                

			m_lMaxFileDuration = 0;
			m_lMaxFileDuration = pDirect->m_lMaxFileDuration;

			if( m_pFileWriter->initFileWriter(pDirect->m_llMaxFileSize, getDuration()) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START FileWrite initFileWriter failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INIT_FAIL);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}
			
			if( m_pFileWriter->setFilePath(pDirect->m_strPath) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START FileWrite setFilePath failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INIT_FAIL);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}

			unsigned char* pDSI = NULL;
			unsigned int iDSISize = 0;

			/*
				return val
					0 : Did not need audio Encode
					1 : Need audio Encode
					2 : Audio not exist
			*/
			int iCheckAudio = m_pClipList->checkAudioDirectExport();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START CheckAudioDirectExport(%d)", __LINE__, iCheckAudio);
			if( iCheckAudio == 0 )
			{
				int iAudioCodecType = m_pClipList->getFirstClipCodecType(NXFF_MEDIA_TYPE_AUDIO);
				if( iAudioCodecType == 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START get Audio codec type failed(%d)", __LINE__, iAudioCodecType);
					m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_GENERAL);
					SAFE_RELEASE(pDirect);
					return MESSAGE_PROCESS_OK; 
				}

				int iRet = m_pClipList->getFirstClipDSIofClip(NXFF_MEDIA_TYPE_AUDIO, &pDSI, &iDSISize);
				if( iRet != 0 || pDSI == NULL || iDSISize <= 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START get Audio DSI failed(%d %p %d)", __LINE__, iRet, pDSI, iDSISize);
					m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_GENERAL);
					SAFE_RELEASE(pDirect);
					return MESSAGE_PROCESS_OK; 
				}

				if( m_pFileWriter->setAudioCodecInfo(iAudioCodecType, pDSI, iDSISize) == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite setAudioCodecInfo failed", __LINE__);
					m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INIT_FAIL);
					SAFE_RELEASE(pDirect);
					return MESSAGE_PROCESS_OK; 
				}
			}
			else if( iCheckAudio == 1 )
			{
				if( m_pFileWriter->setAudioCodecInfo(eNEX_CODEC_A_AAC, EDITOR_DEFAULT_SAMPLERATE, (m_pVideoEditor->m_iAudioMultiChannelOut == 1) ? 6 : EDITOR_DEFAULT_CHANNELS, 128*1024) == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START FileWrite setAudioCodecInfo failed", __LINE__);
					m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_AUDIO_DEC_INIT_FAIL);
					SAFE_RELEASE(pDirect);
					return MESSAGE_PROCESS_OK; 
				}				
			}
			else
			{
				m_pFileWriter->setVideoOnlyMode(TRUE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START With Video Only Mode", __LINE__);
			}

			int iVideoCodecType = m_pClipList->getFirstClipCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO);
			if( iVideoCodecType == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START get Video codec type failed(%d)", __LINE__, iVideoCodecType);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_GENERAL);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}

			pDSI = NULL;
			iDSISize = 0;

			iRet = m_pClipList->getFirstClipDSIofClip(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pDSI, &iDSISize);
			if( iRet != 0 || pDSI == NULL || iDSISize <= 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START get Video DSI failed(%d %p %d)", __LINE__, iRet, pDSI, iDSISize);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_GENERAL);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}
						
			if( m_pFileWriter->setVideoCodecInfo(iVideoCodecType, pDSI, iDSISize) == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite setVideoCodecInfo failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_INIT_FAIL);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}

			int iOrientation = 0;
			if( m_pClipList->getFirstClipRotateInfo(&iOrientation) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				iOrientation = 0;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START FileWrite Orientataion failed(%d)", __LINE__, iOrientation);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_DIRECT_EXPORT_START FileWrite Orientataion(%d)", __LINE__, iOrientation);
			m_pFileWriter->setVideoRotate(iOrientation);
			
			if( m_pFileWriter->startFileWriter() == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FileWrite initFileWriter failed", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_START_FAIL);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}

#ifdef FOR_TEST_PREPARE_VIDEO
			CNexCodecManager::setPrepareHardwareDecoderFlag(1);
#endif
						
			m_bPlayEnd = FALSE;
			
			m_uiCurrentTime = 0;
			m_uiLastNotiTime = m_uiCurrentTime;

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();                            
			m_pClipList->resetClipReader();

			m_uiExportingTime = nexSAL_GetTickCount();
		
			SAFE_RELEASE(m_pAudioRender);
			if( iCheckAudio == 1 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] DirectExport with audio renderer", __LINE__);

				void* pAudioRenderFuncs = m_pVideoEditor->getAudioRenderFuncs();
				if( pAudioRenderFuncs == NULL )
				{
					m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_RENDERER_INIT);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] StateChange failed because audio renderer did not initialized", __LINE__);
					SAFE_RELEASE(pDirect);
					return MESSAGE_PROCESS_OK; 
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] audiorenderFuncs(%p)", __LINE__, pAudioRenderFuncs);

				m_pAudioRender	= new CNEXThread_AudioRenderTask(pAudioRenderFuncs, m_pClipList->m_ClipItemVec.size()+m_pClipList->m_LayerItemVec.size()+m_pClipList->m_AudioClipItemVec.size());
				if( m_pAudioRender == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] get Audio Task Create failed(%p)", __LINE__, m_pAudioRender);
					m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_PLAYSTART_FAILED, 0, 0);
					SAFE_RELEASE(pDirect);
					return MESSAGE_PROCESS_OK;
				}
			
				m_pAudioRender->Begin();
				m_pAudioRender->setProjectManager(this);
				m_pAudioRender->setDirectExport(TRUE);

				CNxMsgAudioRenderInfo* pAudioRenderInfo = new CNxMsgAudioRenderInfo;
				// Default Render Info
				
				pAudioRenderInfo->m_uiCurrentTimeStamp		= m_uiCurrentTime;
				pAudioRenderInfo->m_uiTotalTime				= m_pClipList->getClipTotalTime();
				
				pAudioRenderInfo->m_uiSamplingRate			= EDITOR_DEFAULT_SAMPLERATE;
				pAudioRenderInfo->m_uiChannels				= (m_pVideoEditor->m_iAudioMultiChannelOut == 1) ? 6 : EDITOR_DEFAULT_CHANNELS;
				pAudioRenderInfo->m_uiBitsForSample			= EDITOR_DEFAULT_BITFORSAMPLE;
				pAudioRenderInfo->m_uiSampleForChannel		= EDITOR_DEFAULT_SAMPLEFORCHANNEL;
				pAudioRenderInfo->m_iMuteAudio				= (int)m_bMuteAudio;
				pAudioRenderInfo->m_iManualVolumeControl	= (int)m_bManualVolumeControl;
				pAudioRenderInfo->m_iPreview				= 0;

				pAudioRenderInfo->m_iProjectVolume			= m_iProjectVolume;
				pAudioRenderInfo->m_iFadeInTime				= m_iProjectVolumeFadeInTime;
				pAudioRenderInfo->m_iFadeOutTime			= m_iProjectVolumeFadeOutTime;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Audio Info(S:%d C:%d B:%d S:%d) PV(%d) FV(%d %d)", 
					__LINE__, 
					pAudioRenderInfo->m_uiSamplingRate,
					pAudioRenderInfo->m_uiChannels, 
					pAudioRenderInfo->m_uiBitsForSample, 
					pAudioRenderInfo->m_uiSampleForChannel,
					pAudioRenderInfo->m_iProjectVolume,
					pAudioRenderInfo->m_iFadeInTime,
					pAudioRenderInfo->m_iFadeOutTime);

				m_pAudioRender->SendCommand(pAudioRenderInfo);
				// for JIRA 2059
				if( pAudioRenderInfo->waitProcessDone(1000) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Audio Render Info Done", __LINE__);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Audio Render Info TimeOut", __LINE__);
				}
				
				SAFE_RELEASE(pAudioRenderInfo);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Current Time Send to Audio Renderer(%d)", __LINE__, m_uiCurrentTime);
				
				CNxMsgSetFileWriter* pFileWriter = new CNxMsgSetFileWriter((INxRefObj*)m_pFileWriter);

				m_pAudioRender->SendCommand(pFileWriter);
				SAFE_RELEASE(pFileWriter);

				m_pAudioRender->SendSimpleStateChangeCommand(PLAY_STATE_RECORD);
				m_pAudioRender->SendSimpleCommand(MESSAGE_VIDEO_STARTED);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] DirectExport without audio renderer", __LINE__);
			}

			SAFE_RELEASE(m_pPlayClipTask);
			m_pPlayClipTask = new CNEXThread_PlayClipTask();
			if( m_pPlayClipTask == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Play Clip Task Create failed(%p)", __LINE__, m_pPlayClipTask);
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_PLAYSTART_FAILED, 0, 0);
				SAFE_RELEASE(m_pAudioRender);
				SAFE_RELEASE(pDirect);
				return MESSAGE_PROCESS_OK; 
			}
			m_pPlayClipTask->setProjectManager(this);
			m_pPlayClipTask->setRenderer((CNEXThreadBase*)m_pVideoRender, (CNEXThreadBase*)m_pAudioRender);
			m_pPlayClipTask->setFileWriter(m_pFileWriter);
			m_pPlayClipTask->setClipList(m_pClipList);
			m_pPlayClipTask->setDirectExportOption(pDirect->m_iFlag);
			m_pPlayClipTask->Begin();			

			m_pPlayClipTask->SendSimpleStateChangeCommand(PLAY_STATE_RECORD);

			m_ePreThreadState = m_eThreadState;
			m_eThreadState = PLAY_STATE_RECORD;

			m_pVideoEditor->notifyEvent(MESSAGE_STATE_CHANGE_DONE, m_ePreThreadState, m_eThreadState);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Change State process End(pre:%d Cur:%d)", __LINE__, m_ePreThreadState, m_eThreadState);					

			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		
		case MESSAGE_FAST_PREVIEW_START:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_START START", __LINE__);
			CNxMsgFastPreview* pFastPreview = (CNxMsgFastPreview*)pMsg;

			if(pFastPreview == NULL) {
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] pFastPreview == NULL", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_START_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN);
				SAFE_RELEASE(pFastPreview);
				return MESSAGE_PROCESS_OK;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] startTime(%d) endTime(%d)", __LINE__, pFastPreview->m_uiStartTime, pFastPreview->m_uiEndTime);
            
            if( m_pFastPreviewTask != NULL )
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] pFastPreview is already running", __LINE__);
                m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_START_DONE, NEXVIDEOEDITOR_ERROR_FASTPREVIEW_BUSY);
                SAFE_RELEASE(pFastPreview);
                return MESSAGE_PROCESS_OK;
            }

			m_pFastPreviewTask = new CNEXThread_FastPreviewTask();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Create m_pFastPreviewTask(%p)", __LINE__, m_pFastPreviewTask);
			
			if(m_pFastPreviewTask == NULL) {
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] m_pFastPreviewTask == NULL", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_START_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN);
				SAFE_RELEASE(pFastPreview);
				return MESSAGE_PROCESS_OK;
			}

			if(m_pVideoRender == NULL)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] m_pVideoRender == NULL", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_START_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN);
				SAFE_RELEASE(pFastPreview);
				return MESSAGE_PROCESS_OK;
			}		

			if(m_pClipList == NULL)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] m_pClipList == NULL", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_START_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN);
				SAFE_RELEASE(pFastPreview);
				return MESSAGE_PROCESS_OK;
			}
						
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FastPreviewTask START", __LINE__);


			m_pFastPreviewTask->setProjectManager(this);
			m_pFastPreviewTask->setRenderer(m_pVideoRender);
			m_pFastPreviewTask->setClipList(m_pClipList);
			
			m_pFastPreviewTask->Begin();

			
			m_pFastPreviewTask->SendCommand(pFastPreview);
			                                
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] FastPreviewTask END", __LINE__);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_START END", __LINE__);
			
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}	

		case MESSAGE_FAST_PREVIEW_START_DONE:
		{
            CNxMsgFastPreview* pFastPreview = (CNxMsgFastPreview*)pMsg;
            m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_START_DONE, NEXVIDEOEDITOR_ERROR_NONE, pFastPreview->m_uiStartTime, pFastPreview->m_uiEndTime);
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_START_DONE", __LINE__);
			SAFE_RELEASE(pFastPreview);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_FAST_PREVIEW_STOP:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_STOP", __LINE__);
			
            if( m_pFastPreviewTask != NULL )
            {
                m_pFastPreviewTask->End(1000);
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] fastpreview end", __LINE__);
                SAFE_RELEASE(m_pFastPreviewTask);
            }
            m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_STOP_DONE, NEXVIDEOEDITOR_ERROR_NONE);
            SAFE_RELEASE(pMsg);
            return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_FAST_PREVIEW_TIME:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_TIME START", __LINE__);
			CNxMsgFastPreview* pFastPreview = (CNxMsgFastPreview*)pMsg;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] fastPreView Time(%d)", __LINE__, pFastPreview->m_uiTime);

			if( pFastPreview == NULL )
            {
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] pFastPreview == NULL", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_TIME_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN);
				SAFE_RELEASE(pFastPreview);
				return MESSAGE_PROCESS_OK;
			}

			if(m_pFastPreviewTask == NULL)
            {
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] m_pFastPreviewTask == NULL", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_TIME_DONE, NEXVIDEOEDITOR_ERROR_FASTPREVIEW_TIME_ERROR);
				SAFE_RELEASE(pFastPreview);
				return MESSAGE_PROCESS_OK;
			}

			m_pFastPreviewTask->SendCommand(pFastPreview);
			SAFE_RELEASE(pMsg);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_TIME END", __LINE__);
			return MESSAGE_PROCESS_OK;
		}
        case MESSAGE_FAST_PREVIEW_TIME_DONE:
        {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_TIME_DONE START", __LINE__);
            CNxMsgFastPreview* pFastPreview = (CNxMsgFastPreview*)pMsg;

            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] fastPreView Time(%d)", __LINE__, pFastPreview->m_uiTime);

            if( pFastPreview == NULL )
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] pFastPreview == NULL", __LINE__);
                m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_TIME_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN);
                SAFE_RELEASE(pFastPreview);
                return MESSAGE_PROCESS_OK;
            }

            m_pVideoEditor->notifyEvent(MESSAGE_FAST_PREVIEW_TIME_DONE, pFastPreview->m_nResult);

            SAFE_RELEASE(pFastPreview);
            
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_FAST_PREVIEW_TIME END", __LINE__);
            return MESSAGE_PROCESS_OK;
        }
		case MESSAGE_MAKE_REVERSE_START:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_REVERSE", __LINE__);

			CNxMsgReverseInfo* pReverseInfo = (CNxMsgReverseInfo*)pMsg;
			if( pReverseInfo == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_REVERSE_START_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN, pReverseInfo->m_iUserTag);
				SAFE_RELEASE(pReverseInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pVideoRender == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_REVERSE_START_DONE, NEXVIDEOEDITOR_ERROR_UNKNOWN, pReverseInfo->m_iUserTag);
				SAFE_RELEASE(pReverseInfo);
				return MESSAGE_PROCESS_OK;
			}			

			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_REVERSE_START_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE, pReverseInfo->m_iUserTag);
				SAFE_RELEASE(pReverseInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pTranscodingTask && m_pTranscodingTask->IsWorking() )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_REVERSE_START_DONE, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY, pReverseInfo->m_iUserTag);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore highlight command while the transcoding processing", __LINE__);
				SAFE_RELEASE(pReverseInfo);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pThumbnailTask && m_pThumbnailTask->IsWorking())
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_REVERSE_START_DONE, NEXVIDEOEDITOR_ERROR_THUMBNAIL_BUSY, pReverseInfo->m_iUserTag);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore highlight commandwhile the thumbnail processing", __LINE__);
				SAFE_RELEASE(pReverseInfo);
				return MESSAGE_PROCESS_OK;
			}

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();                            
			m_pTranscodingTask = new CNEXThread_TranscodingTask(this);
			if( m_pTranscodingTask == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_MAKE_REVERSE_START_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED, pReverseInfo->m_iUserTag);
				SAFE_RELEASE(pReverseInfo);
				return MESSAGE_PROCESS_OK;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_REVERSE_START(Dec surface : %p)", __LINE__, m_pTranscodingDecSurf);
			pReverseInfo->m_pDecSurface = m_pTranscodingDecSurf;
			
			m_pTranscodingTask->Begin();
			m_pTranscodingTask->SendCommand(pReverseInfo);
			SAFE_RELEASE(pMsg);			
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_MAKE_REVERSE_START_DONE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_REVERSE_START_DONE", __LINE__);
			CNxMsgReverseInfo* pHighLight = (CNxMsgReverseInfo*)pMsg;

			if( m_pTranscodingTask )
			{
				if( m_pTranscodingTask->IsWorking() )
				{
					m_pTranscodingTask->End(1000);
				}
				SAFE_RELEASE(m_pTranscodingTask);
			}
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_REVERSE_START_DONE, %d", __LINE__, pHighLight->m_nResult);
			m_pVideoEditor->notifyEvent(MESSAGE_MAKE_REVERSE_START_DONE, pHighLight->m_nResult, pHighLight->m_iUserTag);
			
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_MAKE_REVERSE_STOP:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_REVERSE_STOP", __LINE__);

			if( m_pTranscodingTask != NULL ) // need checking about the transcoding task run with highlight
			{
				m_pTranscodingTask->cancelTranscoding();
			}
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_MAKE_REVERSE_STOP", __LINE__);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_MAKE_REVERSE_STOP_DONE:
		{
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}

#endif
		case MESSAGE_SET_VOLUME_WHILE_PLAYING:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_VOLUME_WHILE_PLAYING In", __LINE__);

			if( m_pAudioRender != NULL)
			{
				m_pAudioRender->SendCommand(pMsg);
			}
			CNxMsgSetVolumeWhilePlay* pVolumeMsg = (CNxMsgSetVolumeWhilePlay*) pMsg;
			m_iMasterVolume = pVolumeMsg->m_iMasterVolume;
			m_iSlaveVolume = pVolumeMsg->m_iSlaveVolume;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_VOLUME_WHILE_PLAYING Out", __LINE__);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_PROJECT_VOLUME:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_PROJECT_VOLUME", __LINE__);
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Project Volume Event", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_PROJECT_VOLUME, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			CNxMsgProjectVolume* pProjectVol =  (CNxMsgProjectVolume*)pMsg;
			if( pProjectVol != NULL )
			{
				m_iProjectVolume	= pProjectVol->m_iProjectVolume;
				SAFE_RELEASE(pProjectVol);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_PROJECT_VOLUME(%d)", __LINE__, m_iProjectVolume);
			}
			m_pVideoEditor->notifyEvent(MESSAGE_SET_PROJECT_VOLUME, 0);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_MANUAL_VOLUME_CONTROL:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_MANUAL_VOLUME_CONTROL", __LINE__);
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Set Manual Volume Event", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_MANUAL_VOLUME_CONTROL, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}

			CNxMsgSetManualVolumeControl* pManualVolumeControl =  (CNxMsgSetManualVolumeControl*)pMsg;
			if( pManualVolumeControl != NULL )
			{
				m_bManualVolumeControl	= pManualVolumeControl->m_bManualVolumeControl;
				SAFE_RELEASE(pManualVolumeControl);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_MANUAL_VOLUME_CONTROL(%d)", __LINE__, m_bManualVolumeControl);
			}
			m_pVideoEditor->notifyEvent(MESSAGE_SET_MANUAL_VOLUME_CONTROL, 0);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_UPDATE_RENDERINFO:
		{
			if(NULL != m_pClipList){

				CNxMsgUpdateRenderInfo* pUpdateRenderInfo = (CNxMsgUpdateRenderInfo*)pMsg;
				m_pClipList->updateRenderInfo(pUpdateRenderInfo->m_uiClipID, pUpdateRenderInfo->m_uiFaceDetected, pUpdateRenderInfo->m_StartRect, pUpdateRenderInfo->m_EndRect, pUpdateRenderInfo->m_FaceRect);
				SAFE_RELEASE(pMsg);
			}
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_RESET_FACEDETECTINFO:
		{
			if(NULL != m_pClipList){

				CNxMsgResetFaceDetectInfo* pUpdateRenderInfo = (CNxMsgResetFaceDetectInfo*)pMsg;
				m_pClipList->resetFaceDetectInfo(pUpdateRenderInfo->m_uiClipID);
				SAFE_RELEASE(pMsg);
			}
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_DRAWINFO_LIST:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_DRAWINFO_LIST", __LINE__);
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set drawinfo on invalid state", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_LIST_DONE, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pVideoRender == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set drawinfo because video render is null", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_LIST_DONE, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pClipList == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set drawinfo because cliplist is null", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_LIST_DONE, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}


			m_pVideoRender->SendCommand(pMsg);

			CNxMsgSetDrawInfoList* pInfos = (CNxMsgSetDrawInfoList*)pMsg;
			if( pInfos != NULL )
			{
				CNexDrawInfoVec* pMaster = (CNexDrawInfoVec*)pInfos->masterList;
				if( pMaster != NULL ) {
					pMaster->printDrawInfo();
				}
				CNexDrawInfoVec* pSub = (CNexDrawInfoVec*)pInfos->subList;
				if( pSub != NULL ) {
					pSub->printDrawInfo();
				}

				m_pClipList->setDrawInfos((CNexDrawInfoVec*)pInfos->subList);
			}

			// update clip list;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_DRAWINFO_LIST", __LINE__);

			SAFE_RELEASE(pMsg);
			m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_LIST_DONE, 0);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_DRAWINFO:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_DRAWINFO", __LINE__);
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set drawinfo on invalid state", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_LIST, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pVideoRender == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set drawinfo because video render is null", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_LIST, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pClipList == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set drawinfo because cliplist is null", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_LIST, 1);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}


			CNxMsgSetDrawInfo* pInfos = (CNxMsgSetDrawInfo*)pMsg;
			if( pInfos != NULL )
			{
				CNexDrawInfo* pDrawInfo = (CNexDrawInfo*)pInfos->drawInfo;
				if( pDrawInfo != NULL ) {
					pDrawInfo->printDrawInfo();
				}

				m_pClipList->updateDrawInfo(pDrawInfo);
			}

			m_pVideoRender->SendCommand(pMsg);

			// update clip list;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_DRAWINFO", __LINE__);

			SAFE_RELEASE(pMsg);
			m_pVideoEditor->notifyEvent(MESSAGE_SET_DRAWINFO_DONE, 0);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_ENCODE_PROJECT_JPEG:
		{
			if( m_eThreadState != PLAY_STATE_IDLE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set capture on invalid state", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_JPEG_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pVideoRender == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set capture with invalid renderer", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_JPEG_DONE, NEXVIDEOEDITOR_ERROR_RENDERER_INIT);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			if( m_pClipList == NULL ) {
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set capture with invalid clip list", __LINE__);
				m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_JPEG_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
			if( waitCmd != NULL )
			{
				if( CNexVideoEditor::m_bNexEditorSDK )
				{
					if( m_pSetTimeTask == NULL )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_JPEG_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set capture with invalid setTimeTask", __LINE__);
						return MESSAGE_PROCESS_OK;
					}

					m_pSetTimeTask->SendCommand(waitCmd);
					if( waitCmd->waitProcessDone(3000) == FALSE )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_JPEG_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
						SAFE_RELEASE(waitCmd);
						SAFE_RELEASE(pMsg);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set capture with setTimeTask timeout", __LINE__);
						return MESSAGE_PROCESS_OK; 					
					}
				}
				SAFE_RELEASE(waitCmd);
			}			

			CNxMsgEncodeProjectJpeg* pJpeg = (CNxMsgEncodeProjectJpeg*)pMsg;

			if( pJpeg->m_iFlag != 0 )
			{
				CNexDrawInfo* pInfo = m_pClipList->getDrawInfo(pJpeg->m_iFlag);
				if( pInfo == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore set capture with invalid draw info", __LINE__);
					m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_JPEG_DONE, NEXVIDEOEDITOR_ERROR_INVALID_STATE);
					SAFE_RELEASE(pMsg);
					return MESSAGE_PROCESS_OK;
				}
				pJpeg->setDrawInfo(pInfo);
				SAFE_RELEASE(pInfo);
			}
			m_pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		
		case MESSAGE_LOAD_ORIGINAL_IMAGE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_LOAD_ORIGINAL_IMAGE", __LINE__);
			// m_pClipList->setPreviewTime(m_uiCurrentTime, 0, m_pVideoRender, TRUE, TRUE);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_SET_GIF_MODE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_GIF_MODE", __LINE__);

			CNxMsgSetGIFMode* gifMode = (CNxMsgSetGIFMode*)pMsg;
			if( gifMode == NULL )
			{
				SAFE_RELEASE(pMsg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_GIF_MODE error message", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
			if( waitCmd == NULL )
			{
				gifMode->m_nResult = NEXVIDEOEDITOR_ERROR_INVALID_STATE;
				gifMode->setProcessDone();

				SAFE_RELEASE(gifMode);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_GIF_MODE malloc error", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			if( CNexVideoEditor::m_bNexEditorSDK == FALSE || m_pSetTimeTask == NULL )
			{
				gifMode->m_nResult = NEXVIDEOEDITOR_ERROR_INVALID_STATE;
				gifMode->setProcessDone();
				SAFE_RELEASE(waitCmd);
				SAFE_RELEASE(gifMode);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_GIF_MODE not support GIF mode(%p)", __LINE__, m_pSetTimeTask);
				return MESSAGE_PROCESS_OK;
			}

			m_pSetTimeTask->SendCommand(waitCmd);
			if( waitCmd->waitProcessDone(1000) == FALSE )
			{
				gifMode->m_nResult = NEXVIDEOEDITOR_ERROR_INVALID_STATE;
				gifMode->setProcessDone();
				SAFE_RELEASE(waitCmd);
				SAFE_RELEASE(gifMode);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_GIF_MODE settime wait timeout", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			SAFE_RELEASE(waitCmd);

			if( m_eThreadState == PLAY_STATE_IDLE ) {
				m_isGIFMode = gifMode->m_mode == 1 ? TRUE : FALSE;
				gifMode->m_nResult = NEXVIDEOEDITOR_ERROR_NONE;
			}
			else
			{
				gifMode->m_nResult = NEXVIDEOEDITOR_ERROR_INVALID_STATE;
			}
			gifMode->setProcessDone();
			SAFE_RELEASE(gifMode);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_SET_GIF_MODE Out", __LINE__);
			return MESSAGE_PROCESS_OK;
		}
		default:
		{
			break;
		}
	}
	return MESSAGE_PROCESS_PASS;
}

void CNexProjectManager::ProcessIdleState()
{
	nexSAL_TaskSleep(5);
}

void CNexProjectManager::ProcessPlayState()
{
	if( m_pAudioRender == NULL )
	{
		nexSAL_TaskSleep(50);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Audio Renderer handle is null while PlayState", __LINE__);
		return;
	}

	if(m_pVideoEditor->callbackCheckImageWorkDone() > 0){

		nexSAL_TaskSleep(50);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] wait for image loading while PlayState", __LINE__);
		return;
	}

	unsigned int uiAudioTime = m_pAudioRender->getCurrentTime();
	/*
	if( uiAudioTime == AUDIO_RENDER_INVALID_TIME )
	{
		nexSAL_TaskSleep(20);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Audio Renderer invalid time stamp", __LINE__);
		return;
	}
	*/

	if( uiAudioTime < m_uiCurrentTime )
	{
		m_uLastTickCount = nexSAL_GetTickCount();
		nexSAL_TaskSleep(20);
		m_uLastTickCount = nexSAL_GetTickCount();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] AudioRenderer invalid timestamp(Cur:%d Aud:%d)", __LINE__, m_uiCurrentTime, uiAudioTime);
		return;
	}
	
	if(m_pAudioRender->getPausedForVisual())
	{
		m_uLastTickCount = nexSAL_GetTickCount();
		nexSAL_TaskSleep(10);
		m_uLastTickCount = nexSAL_GetTickCount();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] AudioRenderer paused for visual(Cur:%d Aud:%d)", __LINE__, m_uiCurrentTime, uiAudioTime);
		return;
	}
	
	if( uiAudioTime == m_uiCurrentTime )
	{
		nexSAL_TaskSleep(5);
		return;
	}

	int iTaskSleepTime = (uiAudioTime - m_uiCurrentTime) > 24 ? 1 : 24 - (uiAudioTime - m_uiCurrentTime);
	
	if( uiAudioTime >= getDuration() )
	{
		// for KMSA-477 play time issue.
		uiAudioTime = getDuration();
		// m_uiCurrentTime = uiAudioTime;
		if( m_bPlayEnd )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CURRENTIME EndPlay", __LINE__, uiAudioTime, getDuration());
			nexSAL_TaskSleep(20);
			return;
		}
		SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY_END, m_uiCurrentTime);
		m_bPlayEnd = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CURRENTIME EndPlay(%d %d)", __LINE__, m_uiCurrentTime, getDuration());
	}
	m_uiCurrentTime = uiAudioTime;
	
	m_uiLastNotiTime = m_uiCurrentTime;

	if(uiAudioTime > m_uAudioStartTime + 200)//LG harmony-303 audio tick time adjustment problem
	{
		m_uLastTickCount    = nexSAL_GetTickCount();

		if(m_uCurrentTickTime == 0xffffffff)
	   		m_uCurrentTickTime  = m_uiCurrentTime;
	}
	m_pVideoEditor->notifyEvent(MESSAGE_UPDATE_CURRENTIME, m_uiLastNotiTime);
	// nexSAL_TaskSleep(iTaskSleepTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, m_uLogCnt++%20, "[ProjectManager.cpp %d] Play StateTotalTime(%d) Update Time(A:%7u) (%7u) (%2d) (%7u), (V:%7u)", __LINE__, getDuration(), uiAudioTime, m_uCurrentTickTime, iTaskSleepTime, nexSAL_GetTickCount(), m_pVideoRender->getLastTime() );
}

void CNexProjectManager::ProcessRecordState()
{
	unsigned int uiAudioTime = 0;
	if( m_pAudioRender != NULL )
	{
		uiAudioTime = m_pAudioRender->getCurrentTime();
	}
	else
	{
		if( m_pFileWriter == NULL )
		{
			nexSAL_TaskSleep(50);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Audio Renderer handle is null while RecordState", __LINE__);
			return;
		}
		if( m_pFileWriter->getVideoOnlyMode() )
		{
			uiAudioTime = m_pFileWriter->getVideoTime();
			if( uiAudioTime == m_uiCurrentTime )
			{
				if( m_pFileWriter->isDirectExportWriteEndVideo(getDuration()) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] ProcessRecordState Direct Video Export End", __LINE__);
					uiAudioTime = getDuration();
				}
			}
		}
		else
		{
			uiAudioTime = m_pFileWriter->getAudioTime();
		}
	}
#ifdef __APPLE__
	// m_uiCurrentTime := exportTime := MIN(audioTime, videoTime)
	unsigned int exportTime = uiAudioTime;
	unsigned int lastFrameDuration = 0; // Should we take audio frame duration into account?
	if (m_pFileWriter) {
		unsigned int videoTime = m_pFileWriter->getVideoTime();
		if (exportTime > videoTime) {
			exportTime = videoTime;
		}
		lastFrameDuration = m_pFileWriter->getEncoderFrameInterval() / 100;
	}
	
	unsigned long durationLimit = getDuration();
	if ( m_lMaxFileDuration > 0 && m_lMaxFileDuration < durationLimit) {
		durationLimit = m_lMaxFileDuration;
	}
	
	if( exportTime == m_uiCurrentTime ) {
		nexSAL_TaskSleep(5);
		return;
	}
	
	m_uiCurrentTime = exportTime;
	if ((exportTime + lastFrameDuration) >= durationLimit)
	{
		if( m_bPlayEnd )
		{
			return;
		}
		SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY_END, m_uiCurrentTime);
		m_bPlayEnd = TRUE;
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CURRENTIME EndPlay (%lu <= %lu)", __LINE__, (unsigned long) m_uiCurrentTime, (unsigned long) durationLimit);
	}
#else
	if( uiAudioTime >= getDuration() )
	{
		m_uiCurrentTime = uiAudioTime;
		if( m_bPlayEnd )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CURRENTIME EndPlay", __LINE__, uiAudioTime, getDuration());
			return;
		}
		SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY_END, m_uiCurrentTime);
		m_bPlayEnd = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CURRENTIME EndPlay", __LINE__, m_uiCurrentTime, getDuration());
	}

	if( m_lMaxFileDuration != 0 && uiAudioTime > m_lMaxFileDuration )
	{
		m_uiCurrentTime = uiAudioTime;
		if( m_bPlayEnd )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CURRENTIME EndPlay", __LINE__, uiAudioTime, getDuration());
			return;
		}
		SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
		m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY_END, m_uiCurrentTime);
		m_bPlayEnd = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] MESSAGE_UPDATE_CURRENTIME EndPlay", __LINE__, m_uiCurrentTime, getDuration());
	}

	m_uiCurrentTime = uiAudioTime;
#endif

	//m_uiLastNotiTime = m_uiCurrentTime;
	if( m_bPlayEnd )
	{//yoon
		m_uiLastNotiTime = m_uiCurrentTime;
		m_pVideoEditor->notifyEvent(MESSAGE_UPDATE_CURRENTIME, m_uiLastNotiTime);
	}
	else
	{
		int old_percent = (m_uiLastNotiTime*100)/getDuration();
		int new_percent = (m_uiCurrentTime*100)/getDuration();
		
		if( old_percent != new_percent )
		{
			m_uiLastNotiTime = m_uiCurrentTime;
			m_pVideoEditor->notifyEvent(MESSAGE_UPDATE_CURRENTIME, m_uiLastNotiTime);
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Record State TotalTime(%d) Update Time(%d) (%u)\n", __LINE__, getDuration(), uiAudioTime, nexSAL_GetTickCount() );
}

unsigned int CNexProjectManager::getCurrentTime()
{
	unsigned int uGap = 0;
	unsigned int newCurrentTime = 0;

	if(m_uCurrentTickTime == 0xffffffff)
	{
		newCurrentTime = m_uiCurrentTime;
	}
	else
	{
		if(nexSAL_GetTickCount() > m_uLastTickCount)
			uGap = nexSAL_GetTickCount() - m_uLastTickCount;

		if(m_uCurrentTickTime < m_uiCurrentTime + uGap)
			newCurrentTime = m_uiCurrentTime + uGap;
		else
			newCurrentTime = m_uCurrentTickTime;

		m_uCurrentTickTime = newCurrentTime;
	}

	//return m_uiCurrentTime;
	return newCurrentTime;
}

NXBOOL CNexProjectManager::changeState(PLAY_STATE eState, int iFlag)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] chagneState In(Cur:%d To:%d)", __LINE__, m_eThreadState, eState);
	if( m_eThreadState == eState )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] StateChange Ignored(Cur:%d To:%d)", __LINE__, m_eThreadState, eState);
		m_pVideoEditor->notifyEvent(MESSAGE_STATE_CHANGE_DONE, m_eThreadState, eState, NEXVIDEOEDITOR_ERROR_NO_ACTION);
		return TRUE;
	}

	if( getDuration() <= 0 && ( eState == PLAY_STATE_RUN || eState == PLAY_STATE_RECORD ) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] any visual clip. do not change state(%d)", __LINE__, m_eThreadState);
		m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_NO_ACTION);
		return FALSE; 
	}

	if( CNexVideoEditor::m_bNexEditorSDK && (eState == PLAY_STATE_RUN || eState == PLAY_STATE_RECORD))
	{
#ifdef FOR_CHECK_HW_RESOURCE_SIZE
		clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
		clearCachedVisualCodecInfo(); 
		unsigned int uiMaxClipUseMemSize = 0;
		if( m_pClipList != NULL )
			uiMaxClipUseMemSize = m_pClipList ->getMaxHWUseSize(m_uiCurrentTime);
		
		if( CNexCodecManager::getHardwareMemRemainSize() < uiMaxClipUseMemSize )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] video HW not enough resource (%d) (%d)", __LINE__, CNexCodecManager::getHardwareMemRemainSize(), uiMaxClipUseMemSize);	
			m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_HW_NOT_ENOUGH_MEMORY);
			return FALSE; 
		}				
#endif		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Don't check thumbnail process in to preview ", __LINE__);
	}
	else
	{
   		if( m_pThumbnailTask && m_pThumbnailTask->IsWorking() )
   		{
   		    if( eState != PLAY_STATE_IDLE ){
   			    m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_THUMBNAIL_BUSY);
   			    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore Play command while the thumbnail processing", __LINE__);
   		    	return FALSE; 
   		    }else{
   		        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] force stop command while the thumbnail processing", __LINE__);
   		    }
   		}
	}

#ifdef _ANDROID
	if( m_pTranscodingTask && m_pTranscodingTask->IsWorking())
	{
		m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_TRANSCODING_BUSY);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Ignore change state command while the transcoding processing", __LINE__);
		return FALSE; 
	}
#endif

#ifdef FOR_TEST_PREPARE_VIDEO
	CNexCodecManager::setPrepareHardwareDecoderFlag(1);
#endif		

	switch( eState )
	{
		case PLAY_STATE_RUN:
		case PLAY_STATE_RECORD:
		{
			m_RecordToIdle = 0;			
			m_uiExportingTime = nexSAL_GetTickCount();
		
			void* pAudioRenderFuncs = m_pVideoEditor->getAudioRenderFuncs();
			if( pAudioRenderFuncs == NULL )
			{
				m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_RENDERER_INIT);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] StateChange failed because audio renderer did not initialized", __LINE__);
				return FALSE; 				
			}
			
			m_bPlayEnd = FALSE;
			
			m_uiLastNotiTime = m_uiCurrentTime;
			m_uAudioStartTime = m_uiCurrentTime;
			m_uLastTickCount    = 0xffffffff;
			m_uCurrentTickTime  = 0xffffffff;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, eState);

			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
			clearCachedVisualCodecInfo();                

			m_pClipList->resetClipReader();

			if( eState == PLAY_STATE_RUN )
			{
				if( m_pVideoRender != NULL ) {
					m_pVideoRender->SendSimpleCommand(MESSAGE_DELETE_TRACK_ALL);
				}
			}

			if( eState == PLAY_STATE_RECORD )
			{
				// clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE);
				if( m_pVideoRender != NULL ) {
					m_pVideoRender->SendSimpleCommand(MESSAGE_CLEAR_SURFACETEXTURE);
					m_pVideoRender->SendSimpleCommand(MESSAGE_DELETE_TRACK_ALL);
				}
			}			

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d) audiorenderFuncs(%p) totalClipCount(%d)", __LINE__, eState, pAudioRenderFuncs, m_pClipList->m_ClipItemVec.size()+m_pClipList->m_LayerItemVec.size()+m_pClipList->m_AudioClipItemVec.size());

			SAFE_RELEASE(m_pAudioRender);
			m_pAudioRender	= new CNEXThread_AudioRenderTask(pAudioRenderFuncs, m_pClipList->m_ClipItemVec.size()+m_pClipList->m_LayerItemVec.size()+m_pClipList->m_AudioClipItemVec.size());
			if( m_pAudioRender == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] get Audio Task Create failed(%p)", __LINE__, m_pAudioRender);
				m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_PLAYSTART_FAILED, 0, 0);
				return FALSE; 
			}

			SAFE_RELEASE(m_pPlayClipTask);
			m_pPlayClipTask = new CNEXThread_PlayClipTask();
			if( m_pPlayClipTask == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Play Clip Task Create failed(%p)", __LINE__, m_pPlayClipTask);
				m_pVideoEditor->notifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_PLAYSTART_FAILED, 0, 0);
				SAFE_RELEASE(m_pAudioRender);
				return FALSE; 
			}
			m_pPlayClipTask->setProjectManager(this);
			m_pPlayClipTask->setRenderer((CNEXThreadBase*)m_pVideoRender, (CNEXThreadBase*)m_pAudioRender);
			m_pPlayClipTask->setClipList(m_pClipList);
			m_pPlayClipTask->Begin();

			m_pAudioRender->Begin();
			m_pAudioRender->setProjectManager(this);

			CNxMsgAudioRenderInfo* pAudioRenderInfo = new CNxMsgAudioRenderInfo;
			// Default Render Info
			
			pAudioRenderInfo->m_uiCurrentTimeStamp		= m_uiCurrentTime;
			pAudioRenderInfo->m_uiTotalTime				= m_pClipList->getClipTotalTime();
			pAudioRenderInfo->m_uiSamplingRate			= eState == PLAY_STATE_RECORD ? m_iExportSamplingRate : EDITOR_DEFAULT_SAMPLERATE;
			pAudioRenderInfo->m_uiChannels				= (m_pVideoEditor->m_iAudioMultiChannelOut == 1) ? 6 : EDITOR_DEFAULT_CHANNELS;
			pAudioRenderInfo->m_uiBitsForSample			= EDITOR_DEFAULT_BITFORSAMPLE;
			pAudioRenderInfo->m_uiSampleForChannel		= EDITOR_DEFAULT_SAMPLEFORCHANNEL;
			pAudioRenderInfo->m_iMuteAudio				= (int)m_bMuteAudio;
			pAudioRenderInfo->m_iManualVolumeControl	= (int)m_bManualVolumeControl;
			pAudioRenderInfo->m_iPreview				= eState == PLAY_STATE_RECORD ? 0 : 1;

			pAudioRenderInfo->m_iProjectVolume			= m_iProjectVolume;
			pAudioRenderInfo->m_iFadeInTime				= m_iProjectVolumeFadeInTime;
			pAudioRenderInfo->m_iFadeOutTime			= m_iProjectVolumeFadeOutTime;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Audio Info(S:%d C:%d B:%d S:%d) PV(%d) FV(%d %d)", 
				__LINE__, 
				pAudioRenderInfo->m_uiSamplingRate,
				pAudioRenderInfo->m_uiChannels, 
				pAudioRenderInfo->m_uiBitsForSample, 
				pAudioRenderInfo->m_uiSampleForChannel,
				pAudioRenderInfo->m_iProjectVolume,
				pAudioRenderInfo->m_iFadeInTime,
				pAudioRenderInfo->m_iFadeOutTime);

			m_pAudioRender->SendCommand(pAudioRenderInfo);

#if defined(FOR_PROJECT_Vivo)
			CNxMsgSetVolumeWhilePlay* pMsg = new CNxMsgSetVolumeWhilePlay;
			pMsg->m_iMasterVolume = m_iMasterVolume;
			pMsg->m_iSlaveVolume = m_iSlaveVolume;
			m_pAudioRender->SendCommand((CNxMsgInfo*)pMsg);
			SAFE_RELEASE(pMsg);            
#endif
			// for JIRA 2059
			if( pAudioRenderInfo->waitProcessDone(1000) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Audio Render Info Done", __LINE__);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[ProjectManager.cpp %d] Audio Render Info TimeOut", __LINE__);
			}
			
			SAFE_RELEASE(pAudioRenderInfo);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Current Time Send to Audio Renderer(%d)", __LINE__, m_uiCurrentTime);
			
			if( eState == PLAY_STATE_RECORD)
			{
				CNxMsgSetFileWriter* pFileWriter = new CNxMsgSetFileWriter((INxRefObj*)m_pFileWriter);
				m_pVideoRender->SendCommand(pFileWriter);
				if( pFileWriter->waitProcessDone(3000) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Video Render set FileWriter end", __LINE__);
					if( pFileWriter->m_nResult == 1 )
					{
						m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_PLAYSTART_FAILED, 0, 0);
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ProjectManager.cpp %d] Video Render set FileWriter failed", __LINE__);
				}
				m_pAudioRender->SendCommand(pFileWriter);
				SAFE_RELEASE(pFileWriter);
			}

			m_pVideoRender->setVisualCountAtTime(m_pClipList->getAllVisualClipCountAtTime(m_uiCurrentTime), m_pClipList->getVisualLayerCountAtTime(m_uiCurrentTime));
            
			m_pVideoRender->SendSimpleStateChangeCommand(eState);
			m_pPlayClipTask->SendSimpleStateChangeCommand(eState);
			m_pAudioRender->SendSimpleStateChangeCommand(eState);
			break;
		}
		case PLAY_STATE_PAUSE:
		{
			m_pAudioRender->SendPauseCommand(eState);
			m_pPlayClipTask->SendPauseCommand(eState);
			m_pVideoRender->SendPauseCommand(eState);
			break;
		}
		case PLAY_STATE_RESUME:
		{
			m_pVideoRender->setVisualCountAtTime(m_pClipList->getVisualClipCountAtTime(m_uiCurrentTime), m_pClipList->getVisualLayerCountAtTime(m_uiCurrentTime));
			
			m_pVideoRender->SendResumeCommand(eState);
			m_pAudioRender->SendResumeCommand(eState);
			m_pPlayClipTask->SendResumeCommand(eState);
			break;
		}
		default:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
			m_iEncodeToTranscode = 0;
			// for preview stop delay problem
			if( m_pAudioRender )
			{
				m_pAudioRender->SendSimpleStateChangeCommand(eState);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
			
			if( m_pVideoRender )
			{
				m_pVideoRender->SendSimpleStateChangeCommand(eState);				

				CNxMsgWaitCmd* waitCmd = new CNxMsgWaitCmd();
				m_pVideoRender->SendCommand(waitCmd);
				if( waitCmd->waitProcessDone(3000) == FALSE ) {
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] videoRender task's state change Timeout", __LINE__);
				}
				else {
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] videoRender task's state change ok", __LINE__);
				}
				SAFE_RELEASE(waitCmd);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
			
			if( m_pPlayClipTask && m_pPlayClipTask->IsWorking() )
			{
				m_pPlayClipTask->SendSimpleStateChangeCommand(PLAY_STATE_IDLE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
				m_pPlayClipTask->SendSimpleCommand(MESSAGE_STOP_PLAYTASK);
				m_pPlayClipTask->End(1000);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
				m_pPlayClipTask->setProjectManager(NULL);
				m_pPlayClipTask->setRenderer(NULL, NULL);
				SAFE_RELEASE(m_pPlayClipTask);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
				m_pClipList->stopPlay();
			}

			CNxMsgSetFileWriter* pFileWriter = new CNxMsgSetFileWriter;
			if( m_pAudioRender )
			{
				m_pAudioRender->SendCommand(pFileWriter);
				m_pAudioRender->End(1000);
				SAFE_RELEASE(m_pAudioRender);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
			if( m_pVideoRender )
			{
				m_pVideoRender->SendCommand(pFileWriter);
				pFileWriter->waitProcessDone(3000);
			}
			SAFE_RELEASE(pFileWriter);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
			
			m_uiLastNotiTime = 0;
			m_uLastTickCount    = 0xffffffff;
			m_uCurrentTickTime  = 0xffffffff;
			m_uAudioStartTime = 0;			

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
			if( m_eThreadState == PLAY_STATE_RECORD || m_eThreadState == PLAY_STATE_PAUSE)
			{
				unsigned int uiDuration = 0;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Wait Renderer State(Renderer(%d) CurState(%d))",
					__LINE__, m_pVideoRender->getState(), eState);	
				while( eState != m_pVideoRender->getState() )
				{
					nexSAL_TaskSleep(10);
				}

				clearCachedVisualCodecInfo();
				m_pVideoRender->SendSimpleCommand(MESSAGE_CLEAR_SURFACETEXTURE);

				//NXBOOL bCancel = m_bPlayEnd || iFlag ? FALSE : TRUE;
				NXBOOL bCancel = TRUE;
				if( m_bPlayEnd || iFlag == 1 )
				{
				    bCancel = FALSE;
				}    
				int uRet = m_pFileWriter->endFileWriter(&uiDuration, bCancel);

				if(uRet == 0)
				{
					CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_START_FAIL);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] m_bPlayEnd(%d) iFlag(%d) ", __LINE__, m_bPlayEnd, iFlag);
					if( m_bPlayEnd == TRUE)
						m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_NONE, uiDuration);
					if( m_bPlayEnd == FALSE && iFlag != 0 )
						m_pVideoEditor->notifyEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_USER_CANCEL, uiDuration, 1);
				}		
				m_uiExportingTime = nexSAL_GetTickCount() - m_uiExportingTime;
				unsigned int uiMS = m_uiExportingTime % 1000;
				unsigned int uiSec = (m_uiExportingTime /1000 ) % 60;
				unsigned int uiMin = (m_uiExportingTime / 60000);
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Recording End(Dur : %d, Elapsed(%d:%d:%d)", __LINE__, uiDuration, uiMin, uiSec, uiMS);
			}
			m_pClipList->setVideoDecodingEnd(FALSE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process(%d)", __LINE__, m_eThreadState);
			break;
		}
	};

	m_ePreThreadState = m_eThreadState;
	m_eThreadState = eState;

	m_pVideoEditor->notifyEvent(MESSAGE_STATE_CHANGE_DONE, m_ePreThreadState, m_eThreadState);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ProjectManager.cpp %d] Change State process End(pre:%d Cur:%d)", __LINE__, m_ePreThreadState, m_eThreadState);
	return TRUE;
}

int CNexProjectManager::closeProject()
{
	clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE);
	clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO);
	clearCachedVisualCodecInfo();                

	if( m_pThumbnailTask )
	{
		if( m_pThumbnailTask->IsWorking() )
		{
            m_pThumbnailTask->setStopThumb();
			m_pThumbnailTask->End(1000);
		}
		SAFE_RELEASE(m_pThumbnailTask);
	}
#ifdef _ANDROID
	if( m_pTranscodingTask )
	{
		if( m_pTranscodingTask->IsWorking() )
			m_pTranscodingTask->End(1000);
		SAFE_RELEASE(m_pTranscodingTask);
	}
#endif
	if( m_pSetTimeTask )
	{
		if( m_pSetTimeTask->IsWorking() )
			m_pSetTimeTask->End(1000);
		SAFE_RELEASE(m_pSetTimeTask);
	}	
	
	if( m_pPlayClipTask )
	{
		if( m_pPlayClipTask->IsWorking() )
			m_pPlayClipTask->End(1000);
		SAFE_RELEASE(m_pPlayClipTask);
	}

	if( m_pAudioRender )
	{
		if( m_pAudioRender->IsWorking() )
			m_pAudioRender->End(1000);
		SAFE_RELEASE(m_pAudioRender);
	}

	if( m_pVideoRender )
	{
		if( m_pVideoRender->IsWorking() )
		{
			m_pVideoRender->SendSimpleCommand(MESSAGE_DELETE_TRACK_ALL);
			m_pVideoRender->End(1000);
		}
		SAFE_RELEASE(m_pVideoRender);
	}

	SAFE_RELEASE(m_pFileWriter);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

CClipItem* CNexProjectManager::createClip(int iNewClipID, char* pFile, NXBOOL bAudio )
{
	if( pFile == NULL ) return NULL;

	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	CClipItem* pItem = new CClipItem(iNewClipID);
	if( pItem == NULL ) return NULL;

	int iWidth			= 0;
	int iHeight			= 0;
	int iPitch				= 0;
	int iBitForPixel		= 0;

	IMAGE_TYPE imageType = getImageType(pFile);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] is image format(%d)", __LINE__, imageType);

    // KM-2964
    if( imageType == IMAGE_JPEG && m_pVideoEditor->getPropertyBoolean("UseAndroidJPEG", FALSE) )
    {
        imageType = IMAGE_JPEG_ANDROID;
    }
#ifdef __APPLE__
    else if( imageType == IMAGE_JPEG )
    {
        imageType = IMAGE_JPEG_IOS;
    }
#endif
	switch(imageType)
	{
		case NOT_IMAGE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] It was not image file", __LINE__);
			break;
			
		case IMAGE_JPEG:
		{
			if( m_pVideoEditor->existProperty("JpegMaxWidthFactor") && m_pVideoEditor->existProperty("JpegMaxHeightFactor") && m_pVideoEditor->existProperty("JpegMaxSizeFactor") )
			{
				iWidth = m_pVideoEditor->getPropertyInt("JpegMaxWidthFactor", 0);
				iHeight = m_pVideoEditor->getPropertyInt("JpegMaxHeightFactor", 0);
				iPitch = m_pVideoEditor->getPropertyInt("JpegMaxSizeFactor", 0);
			}
			
			if( m_pVideoEditor->getJPEGImageHeader(pFile, &iWidth, &iHeight, &iPitch) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] open Jpeg Image Failed", __LINE__);
				SAFE_RELEASE(pItem);
				return pItem;
			}
			break;
		}
		
		case IMAGE_JPEG_ANDROID:
        case IMAGE_JPEG_IOS:
		case IMAGE_RGB:
		case IMAGE_PNG:
		case IMAGE_SVG:
		case IMAGE_WEBP:
		case IMAGE_GIF:
		case IMAGE_BMP:
		{
			if( m_pVideoEditor->callbackGetImageHeaderUsingFile(pFile, (unsigned int)strlen(pFile), &iWidth, &iHeight, &iBitForPixel) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] openImage Failed", __LINE__);
				SAFE_RELEASE(pItem);
				return pItem;
			}
			break;
		}
		
		default:
			break;
	};
	
	if( iWidth > 0 && iHeight > 0 )
	{
		pItem->setClipPath(pFile);
		pItem->setClipType(CLIPTYPE_IMAGE);
		pItem->setWidth(iWidth);
		pItem->setHeight(iHeight);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] add Image File(%d %d)", __LINE__, iWidth, iHeight);

		RAND_RATIO ratio = RAND_RATIO_16_9;
		int iRatio = (CNexVideoEditor::m_iSupportedWidth * 10) / CNexVideoEditor::m_iSupportedHeight;
		switch(iRatio)
		{
			case 16:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Clip Rand Rect 5:3 Ratio", __LINE__);
				ratio = RAND_RATIO_5_3;
				break;
			case 13:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Clip Rand Rect 4:3 Ratio", __LINE__);
				ratio = RAND_RATIO_4_3;
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Clip Rand Rect 16:9 Ratio", __LINE__);
				ratio = RAND_RATIO_16_9;
				break;
		}
		
		CRectangle* pRect = (CRectangle*)pItem->getStartPosition();
		if( pRect )
		{
			pRect->setRect(0, 0, iWidth, iHeight);
			pRect->randRect( ratio );
			pRect->Release();
		}

		// Set End Rect
		pRect = (CRectangle*)pItem->getEndPosition();
		if( pRect )
		{
			pRect->setRect(0, 0, iWidth, iHeight);
			pRect->randRect( ratio );
			pRect->Release();
		}
		pItem->setTotalTime(6000);		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] openImage sucessed", __LINE__);
		return pItem;
	}


	if( bAudio )
	{
		pItem->setClipType(CLIPTYPE_AUDIO);
		eRet = (NEXVIDEOEDITOR_ERROR)pItem->parseClipFileWithoutVideo(pFile);
	}
	else
	{
	eRet = (NEXVIDEOEDITOR_ERROR)pItem->parseClipFile(pFile, FALSE, FALSE);
	}

	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] open file failed", __LINE__);
		SAFE_RELEASE(pItem);
		return pItem;
	}
	return pItem;
}

int CNexProjectManager::addVisualClip(unsigned int uiNextToClipID, char* pFile, int iNewClipID)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] addVisualClip(%d %s)", __LINE__, uiNextToClipID, pFile);
	CClipItem* pItem = createClip(iNewClipID, pFile);
	if( pItem == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED;
	}

	m_pClipList->addVisualClip(uiNextToClipID, pItem);
	m_pVideoEditor->notifyEvent(MESSAGE_ADD_CLIP_DONE, NEXVIDEOEDITOR_ERROR_NONE, getDuration(), pItem->getClipID(), pItem->getClipType());
	pItem->printClipInfo();
	SAFE_RELEASE(pItem);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexProjectManager::addAudioClip(unsigned int uiStartTime, char* pFile, int iNewClipID)
{
	CClipItem* pItem = createClip(iNewClipID, pFile, TRUE);
	if( pItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] fail to addAudioClip(%d %s)", __LINE__, uiStartTime, pFile);
		return NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] addAudioClip(%d %s)", __LINE__, uiStartTime, pFile);
	if( m_pClipList->addAudioClip(uiStartTime, pItem) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		SAFE_RELEASE(pItem);
		return NEXVIDEOEDITOR_ERROR_ADD_CLIP_FAIL;
	}
	pItem->setAudioOnOff(TRUE);
	m_pVideoEditor->notifyEvent(MESSAGE_ADD_CLIP_DONE, NEXVIDEOEDITOR_ERROR_NONE, getDuration(), pItem->getClipID(), pItem->getClipType());
	pItem->printClipInfo();
	SAFE_RELEASE(pItem);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexProjectManager::deleteClip(int iClipID)
{
	CVideoTrackInfo* pTrack = CNexProjectManager::getCachedVisualTrackInfo(CACHE_TYPE_VIDEO, iClipID);
	if( pTrack != NULL )
	{
		clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pTrack);
		SAFE_RELEASE(pTrack);
	}
	else
	{
		pTrack = CNexProjectManager::getCachedVisualTrackInfo(CACHE_TYPE_IMAGE, iClipID);
		if( pTrack != NULL )
		{
			clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE, pTrack);
			SAFE_RELEASE(pTrack);
		}
	}
	return m_pClipList->deleteClipItem(iClipID);
}

int CNexProjectManager::moveVisualClip(unsigned int uiNextToClipID, unsigned int uiMovedClipID)
{
	return m_pClipList->moveVisualClip(uiNextToClipID, uiMovedClipID);
}

int CNexProjectManager::moveAudioClip(unsigned int uiMoveTime, unsigned int uiMovedClipID)
{
	return m_pClipList->moveAudioClip(uiMoveTime, uiMovedClipID);
}

unsigned int CNexProjectManager::getDuration()
{
	if( m_pClipList == NULL )
		return 0;
	return m_pClipList->getClipTotalTime();
}

CVideoTrackInfo* CNexProjectManager::getCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, unsigned int uiClipID)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualTrackInfo In( type:%s id:%d)", __LINE__,
		eType == CACHE_TYPE_VIDEO ? "Video" : "Image" , uiClipID);
	
	VideoTrackVec* pTrackVec = &m_ImageTrackCache;
	if( eType == CACHE_TYPE_VIDEO )
		pTrackVec = &m_VideoTrackCache;	
	
	if( pTrackVec->size() <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualTrackInfo did not cached", __LINE__);
		return NULL;
	}

	CAutoLock m(m_CachedLock);
	VideoTrackVecIter i;
	for(i = pTrackVec->begin(); i != pTrackVec->end(); i++)
	{
		CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
		if( pTrackItem == NULL )
			continue;

		if( pTrackItem->m_uiTrackID == uiClipID )
		{
			if( i != pTrackVec->begin() )
			{
				pTrackVec->erase(i);
				pTrackVec->insert(pTrackVec->begin(), pTrackItem);
			}
			SAFE_ADDREF(pTrackItem);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualTrackInfo Sucessed(%p Ref:%d)", __LINE__, pTrackItem, pTrackItem->GetRefCnt());
			return pTrackItem;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualTrackInfo did not cached", __LINE__);
	return NULL;
}

void CNexProjectManager::setCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pTrack)
{
	if( pTrack == NULL ) return;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualTrackInfo In( type:%s track:%p size(I:%zu, V:%zu)", __LINE__,
		eType == CACHE_TYPE_VIDEO ? "Video" : "Image" , pTrack, m_ImageTrackCache.size(), m_VideoTrackCache.size());
	
	CAutoLock m(m_CachedLock);
	VideoTrackVecIter i, tmpi ;
	int iCacheCount = 9;

	VideoTrackVec* pTrackVec = &m_ImageTrackCache;
	if( eType == CACHE_TYPE_VIDEO )
	{
		pTrackVec = &m_VideoTrackCache;
		if( CNexVideoEditor::m_bSuppoertSWH264Codec == FALSE )
#if defined(_ANDROID)
			iCacheCount = 2;
#elif defined(__APPLE__)
			iCacheCount = 0;
#endif
//#ifdef FOR_PROJECT_Kinemaster
//			iCacheCount = CNexCodecManager::getHardwareDecodeMaxCount();
//#else
//			iCacheCount = CNexVideoEditor::m_iSupportDecoderMaxCount;
//#endif
	}
	

	SAFE_ADDREF(pTrack);
	for(i = pTrackVec->begin(); i != pTrackVec->end(); i++)
	{
		CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
		if( pTrackItem == NULL )
			continue;

		if( pTrackItem->m_uiTrackID == pTrack->m_uiTrackID )
		{
			SAFE_RELEASE(pTrackItem);
			pTrackVec->erase(i);
			break;
		}
	}

	pTrackVec->insert(pTrackVec->begin(), pTrack);
	if( pTrackVec->size() > iCacheCount )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualTrackInfo Delete last cached track(size:%d)", __LINE__, pTrackVec->size());
		for( i = pTrackVec->begin(); i != pTrackVec->end(); i++)
			tmpi = i;

		CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*tmpi;
		pTrackVec->erase(tmpi);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualTrackInfo Out and Delete last cached track(TrackID:%d)", __LINE__, pTrackItem->m_uiTrackID);
		SAFE_RELEASE(pTrackItem);
		return;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualTrackInfo Out(TrackID:%d)", __LINE__, pTrack->m_uiTrackID);
}

void CNexProjectManager::clearCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pTrack)
{
	if( pTrack == NULL ) return;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualTrackInfo In( type:%s track:%p size(I:%zu, V:%zu)", __LINE__,
		eType == CACHE_TYPE_VIDEO ? "Video" : "Image" , pTrack, m_ImageTrackCache.size(), m_VideoTrackCache.size());
	
	CAutoLock m(m_CachedLock);
	VideoTrackVecIter i, tmpi ;

	VideoTrackVec* pTrackVec = &m_ImageTrackCache;
	if( eType == CACHE_TYPE_VIDEO )
	{
		pTrackVec = &m_VideoTrackCache;
	}

	for(i = pTrackVec->begin(); i != pTrackVec->end(); i++)
	{
		CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
		if( pTrackItem == NULL )
			continue;

		if( pTrackItem->m_uiTrackID == pTrack->m_uiTrackID )
		{
			SAFE_RELEASE(pTrackItem);
			pTrackVec->erase(i);
			break;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualTrackInfo Out(TrackID:%d) size(%d)", __LINE__, pTrack->m_uiTrackID, pTrackVec->size());
}

void CNexProjectManager::clearCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType)
{
	CAutoLock m(m_CachedLock);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualTrackInfo In", __LINE__);
	VideoTrackVec* pTrackVec = &m_ImageTrackCache;

	if( eType == CACHE_TYPE_VIDEO )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualTrackInfo Video Track", __LINE__);
		pTrackVec = &m_VideoTrackCache;
	}
		
	while(pTrackVec->size() > 0 )
	{
		VideoTrackVecIter i = pTrackVec->begin();
		CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
		if( pTrackItem == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualTrackInfo cached track is null", __LINE__);
			pTrackVec->clear();
			continue;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualTrackInfo Track erase(ID:%d Ref:%d)", __LINE__, pTrackItem->m_uiTrackID, pTrackItem->GetRefCnt());
			
		SAFE_RELEASE(pTrackItem);
		pTrackVec->erase(i);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualTrackInfo Out(%d)", __LINE__, pTrackVec->size());
	// pTrackVec->clear();
}

void CNexProjectManager::clearCacheTranscodingDecSurf()
{
#ifdef _ANDROID
	if( m_pTranscodingRenderer == NULL || m_pTranscodingDecSurf == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCacheTranscodingDecSurf failed because render instance was wrong(%p %p)", 
			__LINE__, m_pTranscodingRenderer, m_pTranscodingDecSurf);
		return;
	}
	
	if( m_pTranscodingRenderer )
	{
		m_pVideoRender->destroySurfaceTexture(m_pTranscodingRenderer, m_pTranscodingDecSurf);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCacheTranscodingDecSurf Done", __LINE__);
	}
	m_pTranscodingRenderer = NULL;
	m_pTranscodingDecSurf = NULL;
#endif
}

void CNexProjectManager::setCacheTranscodingDecSurf(void * pRender, void* pDecSurface)
{
#ifdef _ANDROID
	clearCacheTranscodingDecSurf();
	m_pTranscodingRenderer = pRender;
	m_pTranscodingDecSurf = pDecSurface;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCacheTranscodingDecSurf Done(%p)", __LINE__, m_pTranscodingDecSurf);
#endif
}

int CNexProjectManager::getCachedVisualCodecSize()
{
	CAutoLock m(m_CachedLock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualCodecSize In( size %d )", __LINE__, m_VideoCodecCache.size());
	return m_VideoCodecCache.size();
}

CCodecCacheInfo* CNexProjectManager::getCachedVisualCodecInfo(unsigned int uiIndex)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualCodecInfo In( size %d, Index %d )", __LINE__, m_VideoCodecCache.size(), uiIndex);
	
	if( m_VideoCodecCache.size() <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualCodecInfo did not cached", __LINE__);
		return NULL;
	}

	CAutoLock m(m_CachedLock);
	CCodecCacheInfo* pCodecCache = m_VideoCodecCache[uiIndex];
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] getCachedVisualCodecInfo Sucessed(%p Ref:%d)", __LINE__, pCodecCache, pCodecCache->GetRefCnt());
	return pCodecCache;
}

void CNexProjectManager::setCachedVisualCodecInfo(CCodecCacheInfo* pCodec)
{
	if( pCodec == NULL ) return;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualCodecInfo In( codec:%p size(V:%zu)", __LINE__, pCodec, m_VideoCodecCache.size());
	
	CAutoLock m(m_CachedLock);
	vecCodecCacheInfoItr i, tmpi ;
	int iCacheCount = 9;

	vecCodecCacheInfo* pCodecVec = &m_VideoCodecCache;
	if( CNexVideoEditor::m_bSuppoertSWH264Codec == FALSE )
		iCacheCount = CNexCodecManager::getHardwareDecodeMaxCount();

	SAFE_ADDREF(pCodec);
	for(i = pCodecVec->begin(); i != pCodecVec->end(); i++)
	{
		CCodecCacheInfo* pCodecCache = (CCodecCacheInfo*)*i;
		if( pCodecCache == NULL )
			continue;

		if( pCodecCache == pCodec )
		{
			SAFE_RELEASE(pCodecCache);
			pCodecVec->erase(i);
			break;
		}
	}

	pCodecVec->insert(pCodecVec->begin(), pCodec);
	if( pCodecVec->size() > iCacheCount )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualCodecInfo Delete last cached codec(size:%d)", __LINE__, pCodecVec->size());
		for( i = pCodecVec->begin(); i != pCodecVec->end(); i++)
			tmpi = i;

		CCodecCacheInfo* pCodecCache = (CCodecCacheInfo*)*tmpi;
		pCodecVec->erase(tmpi);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualCodecInfo Out and Delete last cached codec(size:%d)", __LINE__, pCodecCache->getSize());
		SAFE_RELEASE(pCodecCache);
		return;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setCachedVisualCodecInfo Out(size():%d)", __LINE__, pCodec->getSize());
}

void CNexProjectManager::clearCachedVisualCodecInfo(CCodecCacheInfo* pCodec)
{
	if( pCodec == NULL ) return;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualCodecInfo In( codec:%p size(V:%zu)", __LINE__, pCodec, m_VideoCodecCache.size());
	
	CAutoLock m(m_CachedLock);
	vecCodecCacheInfoItr i ;

	vecCodecCacheInfo* pCodecVec = &m_VideoCodecCache;

	for(i = pCodecVec->begin(); i != pCodecVec->end(); i++)
	{
		CCodecCacheInfo* pCodecCache = (CCodecCacheInfo*)*i;
		if( pCodecCache == NULL )
			continue;

		if( pCodecCache == pCodec )
		{
			SAFE_RELEASE(pCodecCache);
			pCodecVec->erase(i);
			break;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualCodecInfo Out(size:%d) size(%d)", __LINE__, pCodec->getSize(), pCodecVec->size());
}

void CNexProjectManager::clearCachedVisualCodecInfo()
{
	CAutoLock m(m_CachedLock);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualCodecInfo In", __LINE__);
	vecCodecCacheInfo* pCodecVec = &m_VideoCodecCache;

	while(pCodecVec->size() > 0 )
	{
		vecCodecCacheInfoItr i = pCodecVec->begin();
		CCodecCacheInfo* pCodecCache = (CCodecCacheInfo*)*i;
		if( pCodecCache == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualCodecInfo cached codec is null", __LINE__);
			pCodecVec->clear();
			continue;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualCodecInfo Codec erase(size:%d Ref:%d)", __LINE__, pCodecCache->getSize(), pCodecCache->GetRefCnt());
			
		SAFE_RELEASE(pCodecCache);
		pCodecVec->erase(i);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] clearCachedVisualCodecInfo Out(%d)", __LINE__, pCodecVec->size());
	// pTrackVec->clear();
}

int CNexProjectManager::getPathHash( char * path ) 
{
	int h = 16769023;
	while( *path ) 
	{
		h = 31 * h + *path;
		path++;
	}
	return h;
}

NXBOOL CNexProjectManager::getFileName(char* pFilePath, char* pFileName)
{
	if( pFilePath == NULL || pFileName == NULL )
	{
		return FALSE;
	}

	int i = (int)strlen(pFilePath) - 1;
	for(; i >= 0; i--)
	{
		if( pFilePath[i] == '/')
		{
			strcpy(pFileName, &pFilePath[i+1]);
			i = (int)strlen(pFileName) - 1;
			for(; i >= 0; i--)
			{
				if( pFileName[i] == '.')
				{
					pFileName[i] = '\0';
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}

unsigned int CNexProjectManager::setThemeRenderer(void* pRender)
{
	if( m_pVideoRender == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setThemeRenderer failed because video handle is null", __LINE__);
		return NEXVIDEOEDITOR_ERROR_GENERAL;
	}
	return m_pVideoRender->setThemeRender(pRender);
}

unsigned int CNexProjectManager::setExportThemeRenderer(void* pRender)
{
	if( m_pVideoRender == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] setExportThemeRenderer failed because video handle is null", __LINE__);
		return NEXVIDEOEDITOR_ERROR_GENERAL;
	}
	return m_pVideoRender->setExportThemeRender(pRender);
}

void CNexProjectManager::sendEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3, unsigned int uiParam4)
{
	if( m_pProjectManager )
	{
		CNxMsgSendEvent* pMsg = new CNxMsgSendEvent;

		pMsg->m_uiEventType	= uiEventType;
		pMsg->m_uiParam1	= uiParam1;
		pMsg->m_uiParam2	= uiParam2;
		pMsg->m_uiParam3	= uiParam3;
		pMsg->m_uiParam4	= uiParam4;

		m_pProjectManager->SendCommand(pMsg);
		pMsg ->Release();
	}
}

NXBOOL CNexProjectManager::startAudioThumbTask(CNxMsgBackgroundGetInfo* pGetInfo)
{
	if( pGetInfo->m_iAudioPCMLevel != 1 )
		return FALSE;
	
	CNEXThread_ThumbnailTask* pTask = new CNEXThread_ThumbnailTask(this);
	if( pTask == NULL )
	{
		m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_DONE, NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED, pGetInfo->m_iUserTag, 0);
		return TRUE;
	}
	pTask->setUserTag(pGetInfo->m_iUserTag);
	m_ThumbAudioVec.insert(m_ThumbAudioVec.end(), pTask);
	pTask->Begin();
	pTask->SendCommand(pGetInfo);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] Start Audio Thumb task(%d)", __LINE__, pGetInfo->m_iUserTag);
	return TRUE;
}

NXBOOL CNexProjectManager::endAudioThumbTask(CNxMsgBackgroundGetInfo* pGetInfo)
{
	ThumbnailTaskIterator i;
	for( i = m_ThumbAudioVec.begin(); i != m_ThumbAudioVec.end(); i++)
	{
		CNEXThread_ThumbnailTask* pTask = (CNEXThread_ThumbnailTask*)*i;
		if( pTask && (pTask->getUserTag() == pGetInfo->m_iUserTag) )
		{
			if( pTask->IsWorking() )
			{
				pTask->End(1000);
			}
			SAFE_RELEASE(pTask);
			m_ThumbAudioVec.erase(i);
			m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_DONE, pGetInfo->m_nResult, pGetInfo->m_iUserTag, 0);
			if(pGetInfo->m_nResult == NEXVIDEOEDITOR_ERROR_BACKGROUND_USER_CANCEL)
			{
				m_pVideoEditor->notifyEvent(MESSAGE_GETCLIPINFO_BACKGROUND_STOP_DONE, pGetInfo->m_nResult, pGetInfo->m_iUserTag, 0);
			}			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] end Audio Thumb task(%d)", __LINE__, pGetInfo->m_iUserTag);
			return TRUE;
		}
	}
	return FALSE;
}

NXBOOL CNexProjectManager::stopAudioThumbTask(CNxMsgBackgroundGetInfoStop* pGetInfo)
{
	ThumbnailTaskIterator i;
	for( i = m_ThumbAudioVec.begin(); i != m_ThumbAudioVec.end(); i++)
	{
		CNEXThread_ThumbnailTask* pTask = (CNEXThread_ThumbnailTask*)*i;
		if( pTask && (pTask->getUserTag() == pGetInfo->m_iUserTag) )
		{
			if( pTask->IsWorking() )
			{
				pTask->setStopThumb();
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] stop Audio Thumb task(%d)", __LINE__, pGetInfo->m_iUserTag);
			return TRUE;
		}
	}
	return FALSE;
}

NXBOOL CNexProjectManager::getPlayEnd()
{
	return m_bPlayEnd || m_RecordToIdle;
}

unsigned int CNexProjectManager::getAudioSessionID()
{
	if( m_pAudioRender == NULL )
		return 0;
	return m_pAudioRender->getAudioSessionID();
}

int CNexProjectManager::getEncodeToTranscode()
{
	if( m_pProjectManager == NULL )
		return 0;

	return m_pProjectManager->m_iEncodeToTranscode;
}

void CNexProjectManager::closePreExecute(){
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] closePreExecute()", __LINE__);
    if( m_pClipList != NULL ){
        m_pClipList->cancelcheckDirectExport();
    }
}

CNEXThread_PlayClipTask* CNexProjectManager::getPlayClipTask()
{
	if( m_pProjectManager == NULL )
		return NULL;

	SAFE_ADDREF(m_pProjectManager->m_pPlayClipTask);
	return m_pProjectManager->m_pPlayClipTask;
}

