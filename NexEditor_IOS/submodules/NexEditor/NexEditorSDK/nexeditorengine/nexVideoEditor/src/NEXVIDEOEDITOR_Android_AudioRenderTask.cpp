/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WaveOut_AudioRenderTask.cpp
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
#include "NEXVIDEOEDITOR_Android_AudioRenderTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_AudioGain.h"
#include <stdio.h>

void handlerRALAudioEvent(int msg, int ext1, int ext2, int ext3, void* ext4, void* userData)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] handlerRALAudioEvent(%d %d %d %d %p %p)", __LINE__, msg, ext1, ext2, ext3, ext4, userData);
	switch(msg)
	{
		case 0x60001:
		{
			/*
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor )
			{
				void* pTrack = pEditor->callbackGetAudioTrack(ext1, ext2);
				if( pTrack )
				{
					if( funcs && funcs->fnNexRALBody_Audio_prepareAudioTrack )
					{
						funcs->fnNexRALBody_Audio_prepareAudioTrack(pTrack);
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] couldn't prepare audio track!", __LINE__);
					}
				}
				SAFE_RELEASE(pEditor);
			}
			*/
			break;
		}
		case 0x60002:
		{
			/*
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor )
			{
				pEditor->callbackReleaseAudioTrack();
				SAFE_RELEASE(pEditor);
			}			
			*/
			break;
		}
	};
}

CNEXThread_AudioRenderTask::CNEXThread_AudioRenderTask(void* pAudioRenderFuncs, int iClipCount )
{
	m_uiCurrentTime		= 0;

	m_uiSamplingRate			= 0;
	m_uiChannels				= 0;
	m_uiBitsForSample			= 0;
	m_uiSampleForChannel		= 0;
	m_uiTotalSampleCount		= 0;
	m_iMuteAudio				= 0;
	m_iManualVolumeControl		= 0;
	m_uiAudioUserData			= (void*)1;
	m_uiInternalAudioCTS		= 0;
	m_ullOutPCMTotalCount		= 0;

	m_uiTotalTime				= 0;
	m_uiPreviewStartTime		= 0;

	m_bRenderPause				= FALSE;
	m_uiLastRenderTime			= 0;
	m_iValidTrackCount			= 0;

	m_pProjectManager			= NULL;

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	void* audioManager = NULL;
	if( pEditor )
	{
		audioManager = pEditor->callbackGetAudioManager();
		SAFE_RELEASE(pEditor);
	}

	m_pRALAudioHandle = (AUDIO_RALBODY_FUNCTION_ST*)pAudioRenderFuncs;
	if( m_pRALAudioHandle->fnNexRALBody_Audio_create )
	{
        NXINT32 iLevel=0;
        nexSAL_TraceGetCondition(NEX_TRACE_CATEGORY_P_AUDIO, &iLevel);
		m_pRALAudioHandle->fnNexRALBody_Audio_create(iLevel,  FALSE, audioManager, (FNRALCALLBACK)handlerRALAudioEvent, m_uiAudioUserData);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] couldn't create audio renderer!", __LINE__);
	}

	m_pPCMBuffer			= (unsigned char*)nexSAL_MemAlloc(DEFAULT_PCMBUFFER_SIZE);

	m_hAudioMixer			= NxMixerOpen(iClipCount);

	m_eTaskPriority			= NEXSAL_PRIORITY_HIGHEST;
	m_uTaskStackSize			= 1024*1024;
	m_eTaskOption			= NEXSAL_TASK_NO_OPTION;

	m_pFileWriter			= NULL;

	m_isVideoStarted		= FALSE;

	m_pAudioMuteBuffer		= (short*) nexSAL_MemAlloc(DEFAULT_PCMBUFFER_SIZE);
	memset(m_pAudioMuteBuffer, 0x00, DEFAULT_PCMBUFFER_SIZE);

	m_iProjectVolume		= 100;
	m_iFadeInTime			= 0;
	m_iFadeOutTime			= 0;
	m_iPreviousFadeVolume	= 0;

	m_iMasterVolume			= 100;
	m_iSlaveVolume			= 100;

	fPeakMeterRMS			= 0;
	fPeakMeter[0]			= 0;
	fPeakMeter[1]			= 0;
	m_AudioTrack.clear();

#ifdef FOR_TEST_AUDIO_MIX_DUMP
	m_pPCMOutFile = NEXSAL_INVALID_HANDLE;
#endif
	m_bDirectExport = 0;
	m_bSetPauseForVisual = 0;
	m_bPausedForVisual = 0;
    
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] CNEXThread_AudioRenderTask", __LINE__);
	
}

CNEXThread_AudioRenderTask::~CNEXThread_AudioRenderTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] ~~~~~CNEXThread_AudioRenderTask In", __LINE__);
	for(int i = 0; i < (int)m_AudioTrack.size(); i++)
	{
		m_AudioTrack[i]->clearAudioTrackData();
		m_AudioTrack[i]->Release();
	}
	m_AudioTrack.clear();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] ~~~~~CNEXThread_AudioRenderTask AudioInfo(%p)", __LINE__, m_uiAudioUserData);
	if( m_uiAudioUserData != (void*)1)
	{
		if( m_pRALAudioHandle->fnNexRALBody_Audio_deinit )
		{
			m_pRALAudioHandle->fnNexRALBody_Audio_deinit(m_uiAudioUserData);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] couldn't deinit audio renderer!", __LINE__);
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] ~~~~~CNEXThread_AudioRenderTask Audio deinit end", __LINE__);
	}

	m_uiAudioUserData = (void*)1;
	if( m_pRALAudioHandle->fnNexRALBody_Audio_delete )
	{
		m_pRALAudioHandle->fnNexRALBody_Audio_delete(m_uiAudioUserData);
		m_uiAudioUserData = (void*)1;
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] couldn't delete audio renderer!", __LINE__);
	}

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		pEditor->callbackReleaseAudioManager();
		SAFE_RELEASE(pEditor);
	}
	
	if( m_hAudioMixer )
	{
		NxMixerClose(m_hAudioMixer);
		m_hAudioMixer = NULL;
	}

	SAFE_RELEASE(m_pFileWriter);

	if( m_pPCMBuffer )
	{
		nexSAL_MemFree(m_pPCMBuffer);
		m_pPCMBuffer = NULL;
	}

	if( m_pAudioMuteBuffer )
	{
		nexSAL_MemFree(m_pAudioMuteBuffer);
		m_pAudioMuteBuffer = NULL;
	}
    
#ifdef FOR_TEST_AUDIO_MIX_DUMP
    if( m_pPCMOutFile != NEXSAL_INVALID_HANDLE )
    {
        nexSAL_FileClose(m_pPCMOutFile);
        m_pPCMOutFile = NEXSAL_INVALID_HANDLE;
    }
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] ~~~~~CNEXThread_AudioRenderTask Out", __LINE__);
}

void CNEXThread_AudioRenderTask::sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3)
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		pEditor->notifyEvent(uiEventType, uiParam1, uiParam2, uiParam3);
		SAFE_RELEASE(pEditor);
	}
}

void CNEXThread_AudioRenderTask::setDirectExport(NXBOOL bDirectExport)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] setDirectExport %d", __LINE__, bDirectExport);
	m_bDirectExport = bDirectExport;
}

void CNEXThread_AudioRenderTask::setPauseForVisual()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] setPauseForVisual", __LINE__);
	m_bSetPauseForVisual = TRUE;

	unsigned int tick = nexSAL_GetTickCount();
	while(m_bPausedForVisual == FALSE && m_isVideoStarted == TRUE)
	{
		if(nexSAL_GetTickCount() - tick > 200)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] setPauseForVisual timeout.... do something", __LINE__);
			break;
		}
		nexSAL_TaskSleep(5);
	}
}

void CNEXThread_AudioRenderTask::setResumeForVisual()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] setResumeForVisual", __LINE__);
	m_bSetPauseForVisual = FALSE;
}

NXBOOL CNEXThread_AudioRenderTask::getPausedForVisual()
{
	return m_bPausedForVisual;
}

unsigned int CNEXThread_AudioRenderTask::getCurrentTime()
{
	CAutoLock m(m_AudioUserDataLock);

	unsigned int uiTime = 0;

	switch( m_eThreadState )
	{
		case PLAY_STATE_RUN:
			if( m_isVideoStarted == FALSE )
			{
				uiTime = m_uiCurrentTime;
				break;
			}
			
			if( m_uiAudioUserData != (void*)1 )
			{
				if(  m_pRALAudioHandle->fnNexRALBody_Audio_getCurrentCTS )
				{
 					m_pRALAudioHandle->fnNexRALBody_Audio_getCurrentCTS(&m_uiCurrentTime, m_uiAudioUserData);
 					uiTime = m_uiCurrentTime;
 				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] couldn't get current cts!", __LINE__);
				}
			}
			break;
		case PLAY_STATE_RECORD:
			uiTime = m_uiCurrentTime;
			break;
		default:
		{
			uiTime = 0;
			break;
		}
	};
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] GetCurrentTime returnTime(%d) Cur(%d)", __LINE__, uiTime, m_uiCurrentTime);
	return uiTime;
}

unsigned int CNEXThread_AudioRenderTask::getInternalAudioTime()
{
	return m_uiInternalAudioCTS;
}

unsigned int CNEXThread_AudioRenderTask::getAudioSessionID()
{
	unsigned int uSessionID = 0;
	if( m_pRALAudioHandle->fnNexRALBody_Audio_getAudioSessionId )
	{
		m_pRALAudioHandle->fnNexRALBody_Audio_getAudioSessionId(&uSessionID, m_uiAudioUserData);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] getAudioSessionID %d", __LINE__, uSessionID);
	
	return uSessionID;
}

void CNEXThread_AudioRenderTask::setAudioTrackHandle(void* pAudioTrack)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] setAudioTrackHandle(%p)", __LINE__, pAudioTrack);
}

int CNEXThread_AudioRenderTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	switch(pMsg->m_nMsgType)
	{
		case MESSAGE_UPDATE_CURRENTIME:
		{
			CNxMsgUpdateCurrentTimeStamp* pCurTime = (CNxMsgUpdateCurrentTimeStamp*)pMsg;
			if( pCurTime )
			{
				m_uiCurrentTime 		= pCurTime->m_currentTimeStamp;
				m_uiInternalAudioCTS 	= 0;
				pCurTime->Release();
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_UPDATE_CURRENTIME(%d)", __LINE__, m_uiCurrentTime);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_AUDIO_RENDERING_INFO:
		{
			CAutoLock m(m_AudioUserDataLock);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_AUDIO_RENDERING_INFO", __LINE__);
			CNxMsgAudioRenderInfo* pAudioInitMsg = (CNxMsgAudioRenderInfo*)pMsg;
			
			m_uiSamplingRate		= pAudioInitMsg->m_uiSamplingRate;
			m_uiChannels			= pAudioInitMsg->m_uiChannels;
			m_uiBitsForSample		= pAudioInitMsg->m_uiBitsForSample;
			m_uiSampleForChannel	= pAudioInitMsg->m_uiSampleForChannel;

			m_iMuteAudio			= pAudioInitMsg->m_iMuteAudio;
			m_iManualVolumeControl	= pAudioInitMsg->m_iManualVolumeControl;

			m_uiCurrentTime		= pAudioInitMsg->m_uiCurrentTimeStamp;
			m_uiPreviewStartTime	= m_uiCurrentTime;
			m_uiTotalTime		= pAudioInitMsg->m_uiTotalTime;

			m_iProjectVolume		= pAudioInitMsg->m_iProjectVolume!=-1?pAudioInitMsg->m_iProjectVolume:100;
			m_iFadeInTime		= pAudioInitMsg->m_iFadeInTime;
			m_iFadeOutTime		= pAudioInitMsg->m_iFadeOutTime;

			// for mantis 5357 issue
 			m_ullOutPCMTotalCount			= (unsigned long long)m_uiCurrentTime;
			m_ullOutPCMTotalCount			= m_ullOutPCMTotalCount * m_uiSamplingRate / 1000;
			m_uiInternalAudioCTS			= (unsigned int)(m_ullOutPCMTotalCount * 1000 / m_uiSamplingRate);			

			m_uiTotalSampleCount = m_uiSampleForChannel * m_uiChannels * m_uiBitsForSample / 8;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] TS(%d) SR(%d) CH(%d) SC(%d) MuteMode(%d) ManualCTL(%d) CurTime(%d) InterTime(%d) ProjectVol(%d) Fade(%d %d)", __LINE__, 
							m_uiTotalSampleCount,
							m_uiSamplingRate,
							m_uiChannels,
							m_uiSampleForChannel, 
							m_iMuteAudio,
							m_iManualVolumeControl,
							m_uiCurrentTime,
							m_uiInternalAudioCTS,
							m_iProjectVolume,
							m_iFadeInTime,
							m_iFadeOutTime);

			if( m_hAudioMixer )
			{
				m_hAudioMixer->nOutChannel		= m_uiChannels;
				m_hAudioMixer->nOutSampleRate	= m_uiSamplingRate;
				m_hAudioMixer->nNum_Track2Mix	= 0;
			}

			if( pAudioInitMsg->m_iPreview )
			{
				if( m_pRALAudioHandle->fnNexRALBody_Audio_init )
				{
					if (0 != m_pRALAudioHandle->fnNexRALBody_Audio_init( eNEX_CODEC_A_AAC, m_uiSamplingRate, m_uiChannels, m_uiBitsForSample, m_uiSampleForChannel, (&m_uiAudioUserData)))
					{
						if(m_pProjectManager)
							m_pProjectManager->sendEvent(MESSAGE_ARAL_INIT_FAIL, m_pProjectManager->getCurrentState(), NEXVIDEOEDITOR_ERROR_RENDERER_INIT);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] couldn't init audio renderer!m_pProjectManager(%p)", __LINE__, m_pProjectManager);
					}
					else
					{
						if (0 != m_pRALAudioHandle->fnNexRALBody_Audio_setTime( m_uiCurrentTime, m_uiAudioUserData))
						{
							if(m_pProjectManager)
								m_pProjectManager->sendEvent(MESSAGE_ARAL_INIT_FAIL, m_pProjectManager->getCurrentState(), NEXVIDEOEDITOR_ERROR_RENDERER_INIT);
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] Audio_setTime Fail", __LINE__);
						}
						m_PeakMeterDetector.Init(m_uiChannels, m_uiSamplingRate);
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] couldn't init audio renderer!", __LINE__);
				}
			}
			else
			{
				m_uiAudioUserData = (void*)1;
			}
			
			pAudioInitMsg->setProcessDone();
			
			SAFE_RELEASE(pAudioInitMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Renderer init end(UD:%p) Time(%d)", __LINE__, m_uiAudioUserData, m_uiCurrentTime);

#ifdef FOR_TEST_AUDIO_MIX_DUMP
            if( m_pPCMOutFile != NEXSAL_INVALID_HANDLE )
            {
                nexSAL_FileClose(m_pPCMOutFile);
                m_pPCMOutFile = NEXSAL_INVALID_HANDLE;
            }
			#ifdef __APPLE__
			nexSAL_FileRemoveA("/Users/rooney/PCM_DUMP/Audio_Mix_Out.pcm");
			m_pPCMOutFile = nexSAL_FileOpen("/Users/rooney/PCM_DUMP/Audio_Mix_Out.pcm", (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
			#endif
            
			#ifdef _ANDROID
			nexSAL_FileRemoveA("/sdcard/Audio_Mix_Out.pcm");
			m_pPCMOutFile = nexSAL_FileOpen("/sdcard/Audio_Mix_Out.pcm", (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
			#endif
#endif
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_ADD_TRACK:
		{
			CNxMsgAddTrack* pAudioTrack = (CNxMsgAddTrack*)pMsg;
			CAudioTrackInfo* pTrack = (CAudioTrackInfo*)pAudioTrack->m_pTrack;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] TID(%d) Audio MESSAGE_ADD_TRACK", __LINE__, pTrack->m_uiTrackID);
			
			// Add Mixer Track
			SAFE_ADDREF(pTrack);
			m_AudioTrack.insert(m_AudioTrack.end(), pTrack);

			/*
			int iTrackCount = m_AudioTrack.size() >= 5 ? 5 : m_AudioTrack.size();

			m_hAudioMixer->nOutChannel		= m_uiChannels;
  			m_hAudioMixer->nOutSampleRate	= m_uiSamplingRate;
			m_hAudioMixer->nNum_Track2Mix	= (unsigned int)iTrackCount;
			for(int i = 0; i < (int)m_hAudioMixer->nNum_Track2Mix; i++)
			{
				m_hAudioMixer->TrackInfo[i]->nSamplePerFrame	= m_AudioTrack[i]->m_uiSampleForChannel;
				m_hAudioMixer->TrackInfo[i]->nInChannel		= m_AudioTrack[i]->m_uiChannels==0?2:m_AudioTrack[i]->m_uiChannels;

				m_hAudioMixer->TrackInfo[i]->nVolume			= -1;
				m_hAudioMixer->TrackInfo[i]->nGain				= getGain(m_iMuteAudio == 1 ? 0 : m_AudioTrack[i]->m_uiVolume);
				//m_hAudioMixer->TrackInfo[i]->nVolume			= m_iMuteAudio == 1 ? 0 : m_AudioTrack[i]->m_uiVolume;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Track info(%d, %d, %d, %d, %d)", __LINE__,
								m_hAudioMixer->TrackInfo[i]->nSamplePerFrame,
								m_hAudioMixer->TrackInfo[i]->nInChannel,
								m_hAudioMixer->TrackInfo[i]->nVolume,
								m_AudioTrack[i]->m_uiVolume,
								m_hAudioMixer->TrackInfo[i]->nGain, i);
				
			}
			NxMixerInit(m_hAudioMixer);
			m_uiTotalSampleCount = m_hAudioMixer->nOutFrameSize * m_hAudioMixer->nOutChannel * 2;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio info(%d %d %d)", __LINE__,
							m_hAudioMixer->nOutSampleRate,
							m_hAudioMixer->nOutChannel,
							m_hAudioMixer->nOutFrameSize);
			*/

			pTrack->m_bRenderRegistered = TRUE;
			SAFE_RELEASE(pAudioTrack);

			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_DELETE_TRACK:
		{
			CNxMsgDeleteTrack* pAudioTrack = (CNxMsgDeleteTrack*)pMsg;
			CAudioTrackInfo* pTrack = (CAudioTrackInfo*)pAudioTrack->m_pTrack;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_DELETE_TRACK ClipID(%d) TrackRefCount(%d)", __LINE__, pTrack->m_uiTrackID, pTrack->GetRefCnt());
			for(AudioTrackVecIter i = m_AudioTrack.begin(); i != m_AudioTrack.end(); i++)
			{
				CAudioTrackInfo* pTrackItem = (CAudioTrackInfo*)*i;
				if( pTrackItem->m_uiTrackID == pTrack->m_uiTrackID )
				{
					m_AudioTrack.erase(i);
					pTrackItem->clearAudioTrackData();
					SAFE_RELEASE(pTrackItem);
					/*
					m_hAudioMixer->nOutChannel		= m_uiChannels;
 					m_hAudioMixer->nOutSampleRate	= m_uiSamplingRate;
					m_hAudioMixer->nNum_Track2Mix	= (unsigned int)m_AudioTrack.size();
					for(int index = 0; index < (int)m_hAudioMixer->nNum_Track2Mix; index++)
					{
						m_hAudioMixer->TrackInfo[index]->nSamplePerFrame	= m_AudioTrack[index]->m_uiSampleForChannel;
						m_hAudioMixer->TrackInfo[index]->nInChannel		= m_AudioTrack[index]->m_uiChannels;
						m_hAudioMixer->TrackInfo[index]->nVolume			= -1;
						m_hAudioMixer->TrackInfo[index]->nGain				= getGain(m_iMuteAudio == 1 ? 0 : m_AudioTrack[index]->m_uiVolume);
						//m_hAudioMixer->TrackInfo[index]->nVolume			= m_iMuteAudio == 1 ? 0 : m_AudioTrack[index]->m_uiVolume;

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Track info(%d, %d, %d, %d, %d) idx(%d)", __LINE__,
										m_hAudioMixer->TrackInfo[index]->nSamplePerFrame,
										m_hAudioMixer->TrackInfo[index]->nInChannel,
										m_hAudioMixer->TrackInfo[index]->nVolume,
										m_AudioTrack[index]->m_uiVolume,
										m_hAudioMixer->TrackInfo[index]->nGain, index);
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Mixer(TotalSize:%d)", __LINE__, m_uiTotalSampleCount);
					if( m_hAudioMixer->nNum_Track2Mix == 0 )
					{
						m_uiTotalSampleCount = m_uiSampleForChannel * m_uiChannels * m_uiBitsForSample / 8;
					}
					else
					{
						NxMixerInit(m_hAudioMixer);
						m_uiTotalSampleCount = m_hAudioMixer->nOutFrameSize * m_hAudioMixer->nOutChannel * 2;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Mixer reinit end(TotalSize:%d)", __LINE__, m_uiTotalSampleCount);
					*/
					break;
				}
			}
			SAFE_RELEASE(pAudioTrack)
			// Delete Mixer Track
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_SET_FILEWRITER:
		{
			if( m_pFileWriter )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_SET_FILEWRITER Writer RefCnt(%d)", __LINE__, m_pFileWriter->GetRefCnt());
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_SET_FILEWRITER", __LINE__);
			CNxMsgSetFileWriter* pSetFileWriter = (CNxMsgSetFileWriter*)pMsg;
			SAFE_RELEASE(m_pFileWriter);
			if( pSetFileWriter )
			{
				m_pFileWriter = (CNexExportWriter*)pSetFileWriter->m_pWriter;
				SAFE_ADDREF(m_pFileWriter);
				SAFE_RELEASE(pSetFileWriter)
				return MESSAGE_PROCESS_OK;
			}
			break;
		}
		case MESSAGE_VIDEO_STARTED:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_VIDEO_STARTED", __LINE__);
			m_isVideoStarted = TRUE;
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_PAUSE_RENDERER:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Renderer pause message receive", __LINE__);
			m_bRenderPause = TRUE;
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_RESUME_RENDERER:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Renderer resume message receive", __LINE__);
			m_bRenderPause = FALSE;
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_VOLUME_WHILE_PLAYING:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_SET_VOLUME_WHILE_PLAYING", __LINE__);
			CNxMsgSetVolumeWhilePlay* pVolume = (CNxMsgSetVolumeWhilePlay*)pMsg;
			if( pVolume != NULL )
			{
				m_iMasterVolume = pVolume->m_iMasterVolume;
				m_iMasterVolume = m_iMasterVolume < 0 ? 0 : m_iMasterVolume;
				m_iMasterVolume = m_iMasterVolume > 200 ? 200 : m_iMasterVolume;

				m_iSlaveVolume = pVolume->m_iSlaveVolume;
				m_iSlaveVolume = m_iSlaveVolume < 0 ? 0 : m_iSlaveVolume;
				m_iSlaveVolume = m_iSlaveVolume > 200 ? 200 : m_iSlaveVolume;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] MESSAGE_SET_VOLUME_WHILE_PLAYING Vol(%d %d)", __LINE__, m_iMasterVolume, m_iSlaveVolume);
			}
			SAFE_RELEASE(pVolume);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_STATE_CHANGE:
		{
			CNxMsgChangeState* pStateChangeMsg = (CNxMsgChangeState*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] StateChange(%d %d)", __LINE__, m_eThreadState, pStateChangeMsg->m_currentState );
			if( m_eThreadState == pStateChangeMsg->m_currentState )
			{
				break;
			}
			if(pStateChangeMsg->m_currentState == PLAY_STATE_RESUME)
			{
				if(m_uiTotalTime > m_uiCurrentTime + 500)
				m_isVideoStarted = 0;

				m_bSetPauseForVisual = FALSE;
				m_bPausedForVisual = FALSE;
				m_uiCurrentTime = m_pFileWriter->getAudioTime();
			}
			m_ePreThreadState = m_eThreadState;
			m_eThreadState = pStateChangeMsg->m_currentState;
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}		
 		default:
		{
			break;
		}
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);
}

void CNEXThread_AudioRenderTask::ProcessPlayState()
{
	if( m_bRenderPause )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Renderer Paused while playing", __LINE__);
		return;
	}
	
	if( m_isVideoStarted == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] Wait Video Start(Audio : %d)", __LINE__, m_uiCurrentTime);
		if(m_bSetPauseForVisual)
			m_bPausedForVisual = TRUE;
		m_uiCurrentTime++;
		nexSAL_TaskSleep(100);
		return;
	}

	while(m_bIsWorking && GetMsgSize() == 0)
	{
		checkValidAudioTrack();

		if(m_bSetPauseForVisual)
		{
			if( m_bPausedForVisual == FALSE && m_pRALAudioHandle->fnNexRALBody_Audio_getEmptyBuffer )
			{
	 			if (0 != m_pRALAudioHandle->fnNexRALBody_Audio_pause(m_uiAudioUserData))
	 			{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] fnNexRALBody_Audio_pause failed.... do something", __LINE__);
	 			}
 			}
 			m_bPausedForVisual = TRUE;
			nexSAL_TaskSleep(10);
			return;
		}

		if(m_bSetPauseForVisual == FALSE && m_bPausedForVisual == TRUE)
		{
 			if (0 != m_pRALAudioHandle->fnNexRALBody_Audio_resume(m_uiAudioUserData))
 			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] fnNexRALBody_Audio_resume failed.... do something", __LINE__);
 			}
 			m_bPausedForVisual = FALSE;
		}


		unsigned int	uiSleepTime	= 0;
		unsigned int	uiTime		= 0;
		unsigned int	uiMixSize	= audioMix( m_pPCMBuffer, m_uiTotalSampleCount, (unsigned int*)&uiTime);
		if( uiMixSize > 0 )
		{
#ifdef FOR_TEST_AUDIO_MIX_DUMP
	        	if( m_pPCMOutFile != NEXSAL_INVALID_HANDLE )
       	 	{
		            	nexSAL_FileWrite(m_pPCMOutFile, m_pPCMBuffer, uiMixSize);
       	 	}
#endif
			unsigned char* 	pTBuffer = NULL;
			int				iTBufferSize = 0;

			if( m_pRALAudioHandle->fnNexRALBody_Audio_getEmptyBuffer )
			{
	 			if (0 != m_pRALAudioHandle->fnNexRALBody_Audio_getEmptyBuffer((void **)&pTBuffer, &iTBufferSize, m_uiAudioUserData))
	 			{
					if(m_pProjectManager)
						m_pProjectManager->sendEvent(MESSAGE_AUDIO_RENDER_FAIL, m_pProjectManager->getCurrentState(), NEXVIDEOEDITOR_ERROR_AUDIO_RENDERER);
	 			}
 			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] couldn't get empty buffer!", __LINE__);
			}
 			if( iTBufferSize >= uiMixSize )
			{
				// KM-3030 blocked since audio performance issue.
#if 0
				unsigned int uiRenderGap =  nexSAL_GetTickCount() - m_uiLastRenderTime;
				if(uiRenderGap < 10)
				{
					// for ZIRA 1924 on exynos5410 devices.(change sleep time 10 to 5)
					nexSAL_TaskSleep(5);
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] add Some wait time for start cts", __LINE__);
				}
#endif
			
				memcpy(pTBuffer, m_pPCMBuffer, uiMixSize);
				if( m_pRALAudioHandle->fnNexRALBody_Audio_consumeBuffer )
				{
#if defined(_ANDROID)
		 			if (0 != m_pRALAudioHandle->fnNexRALBody_Audio_consumeBuffer(pTBuffer, uiMixSize, uiTime, 0, 0, m_uiAudioUserData))
#elif defined(__APPLE__)
		 			if (0 != m_pRALAudioHandle->fnNexRALBody_Audio_consumeBuffer(pTBuffer, uiMixSize, uiTime + 23, 0, 0, m_uiAudioUserData))                      
#endif
		 			{
						if(m_pProjectManager)
							m_pProjectManager->sendEvent(MESSAGE_AUDIO_RENDER_FAIL, m_pProjectManager->getCurrentState(), NEXVIDEOEDITOR_ERROR_AUDIO_RENDERER);
	 				}
					m_uiLastRenderTime = nexSAL_GetTickCount();
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, (uiTime-m_uiCurrentTime < 200)?0:(m_uLogCnt++%50), "[ARTask.cpp %d] ConsumeBuffer Done! (%7u/%7u) Diff(%7u) \n", __LINE__, uiTime, m_uiCurrentTime, uiTime-m_uiCurrentTime);

					if (CNexVideoEditor::m_bSupportPeakMeter)
					{
						if (m_iValidTrackCount == 0)
						{
							fPeakMeterRMS = 0.0f;
							fPeakMeter[0] = 0.0f;
							fPeakMeter[1] = 0.0f;
						}
						else
						{
							m_PeakMeterDetector.EnvelopDetectProcess((short*)m_pPCMBuffer, &fPeakMeterRMS, fPeakMeter, uiMixSize/(m_uiChannels*(m_uiBitsForSample/8)));
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] PeakMeter valid(%d) cts(%u) value(%f, %f)", __LINE__, m_iValidTrackCount, uiTime, fPeakMeter[0], fPeakMeter[1]);
						sendNotifyEvent(MESSAGE_PREVIEW_PEAKMETER, uiTime, (unsigned int) (fPeakMeter[0]*100.0), (unsigned int)(fPeakMeter[1]*100.0));
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] couldn't write to audio renderer!", __LINE__);
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] RealGetEmptyBuffer is NULL", __LINE__);
				return;
			}
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] audioMix m_uiCurrentTime(MixTime:%d, Cur:%d)", __LINE__, uiTime, m_uiCurrentTime);
		}
	}
}

void CNEXThread_AudioRenderTask::ProcessRecordState()
{
	if( m_bRenderPause )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Renderer Paused while exporting", __LINE__);
		return;
	}
	
	// for mantis 9522 problem(20131223 log)
	if( m_pFileWriter == NULL  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] Writer did not set", __LINE__);
		m_uiCurrentTime++;
		nexSAL_TaskSleep(100);
		return;
	}	
    
	if( m_isVideoStarted == FALSE && m_pFileWriter->getVideoTime() <= m_uiCurrentTime)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ARTask.cpp %d] Wait Video Start(Audio : %d)", __LINE__, m_uiCurrentTime);
		if(m_bSetPauseForVisual)
			m_bPausedForVisual = TRUE;
		m_uiCurrentTime++;
		nexSAL_TaskSleep(100);
		return;
	}

	if( m_pFileWriter->isStarted() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ARTask.cpp %d] Wait Writer start", __LINE__, m_uiCurrentTime);
		m_uiCurrentTime++;
		nexSAL_TaskSleep(100);
		return;
	}


	while(m_bIsWorking && GetMsgSize() == 0)
	{
		checkValidAudioTrack();
		
		if(m_bSetPauseForVisual)
		{
 			m_bPausedForVisual = TRUE;
			nexSAL_TaskSleep(20);
			return;
		}

		if(m_bSetPauseForVisual == FALSE && m_bPausedForVisual == TRUE)
		{
 			m_bPausedForVisual = FALSE;
		}
		
		if (m_pFileWriter && !m_pFileWriter->isReadyForMoreAudio()) {
			break;
		}

		unsigned int	uiTime		= 0;
		unsigned int 	uiInterval 	= (AUDIO_ENCODE_MAX_SIZE * 1000)/(m_uiBitsForSample/8*m_uiChannels*m_uiSamplingRate) +1;			
		unsigned int	uiMixSize	= audioMix( m_pPCMBuffer, m_uiTotalSampleCount, (unsigned int*)&uiTime);

#ifdef FOR_TEST_AUDIO_MIX_DUMP
		if( m_pPCMOutFile != NEXSAL_INVALID_HANDLE && uiMixSize > 0 )
		{
			nexSAL_FileWrite(m_pPCMOutFile, m_pPCMBuffer, uiMixSize);
		}
#endif

		if( uiMixSize > 0 && m_pFileWriter )
		{
			unsigned int uiSize;
			unsigned int uiDuration;

			if( uiMixSize > AUDIO_ENCODE_MAX_SIZE )
			{
				unsigned char* pTmp = m_pPCMBuffer;
				unsigned int uiTmpSize = uiMixSize;
				while( uiTmpSize > AUDIO_ENCODE_MAX_SIZE)
				{
					uiTime += uiInterval;
					if( m_pFileWriter->setAudioFrame(uiTime++, pTmp, AUDIO_ENCODE_MAX_SIZE, &uiDuration, &uiSize) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] Audio Frame Write failed", __LINE__);
					}

					pTmp += AUDIO_ENCODE_MAX_SIZE;
					uiTmpSize -= AUDIO_ENCODE_MAX_SIZE;
				}
				uiTime +=uiInterval;
				if( m_pFileWriter->setAudioFrame(uiTime++, pTmp, uiTmpSize, &uiDuration, &uiSize) == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] Audio Frame Write failed", __LINE__);
				}
			}
			else
			{
				if( m_pFileWriter->setAudioFrame(uiTime, m_pPCMBuffer, uiMixSize, &uiDuration, &uiSize) == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ARTask.cpp %d] Audio Frame Write failed", __LINE__);
				}
			}

			//m_uiCurrentTime = uiTime;
			m_uiCurrentTime = m_pFileWriter->getAudioTime();
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio One Frame Write successed(%d)", __LINE__, m_uiCurrentTime);
		}
		else
		{
			break;
		}
	}
}

void CNEXThread_AudioRenderTask::ProcessPauseState()
{
	nexSAL_TaskSleep(50);
}

int CNEXThread_AudioRenderTask::checkValidAudioTrack()
{
	// int iPreviousTrackSize = m_hAudioMixer->nNum_Track2Mix;
	m_iValidTrackCount = 0;
	for(int i = 0; i < m_AudioTrack.size(); i++)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] TID(%d) Check valid track(s:%d e:%d c:%d)", __LINE__,
			m_AudioTrack[i]->m_uiTrackID, m_AudioTrack[i]->m_uiStartTime, m_AudioTrack[i]->m_uiEndTime, m_uiInternalAudioCTS);		

		if( m_AudioTrack[i]->m_uiStartTime <= m_uiInternalAudioCTS && m_AudioTrack[i]->m_uiEndTime > m_uiInternalAudioCTS )
		{
			m_AudioTrack[i]->m_bTrackValidTime = TRUE;
			m_iValidTrackCount++;
		}
		else
		{
			if( m_AudioTrack[i]->m_uiEndTime < m_uiInternalAudioCTS && m_AudioTrack[i]->getDecodingDone() )
			{
				m_AudioTrack[i]->clearAudioTrackData();
			}
			m_AudioTrack[i]->m_bTrackValidTime = FALSE;
		}
	}

	/*
	if( iPreviousTrackSize == m_iValidTrackCount )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Track info count not changed(p:%d, c:%d)", __LINE__, iPreviousTrackSize, m_iValidTrackCount);
		return 0;
	}
	*/

	int iIndex = 0;
	m_hAudioMixer->nOutChannel		= m_uiChannels;
	m_hAudioMixer->nOutSampleRate	= m_uiSamplingRate;
	m_hAudioMixer->bManualVolumeControl	= m_iManualVolumeControl;
	for(int i = 0; i < m_AudioTrack.size(); i++)
	{
		unsigned int uiVolume = 0;
		if( m_AudioTrack[i]->m_bTrackValidTime == FALSE )
			continue;

		m_hAudioMixer->TrackInfo[iIndex]->nSamplePerFrame	= m_AudioTrack[i]->m_uiSampleForChannel;
		m_hAudioMixer->TrackInfo[iIndex]->nInChannel		= m_AudioTrack[i]->m_uiChannels;
		m_hAudioMixer->TrackInfo[iIndex]->nVolume			= -1;

		uiVolume = m_AudioTrack[i]->m_uiVolume;
		CClipAudioRenderInfo* pRenderInfo = m_AudioTrack[i]->getActiveRenderInfo(m_uiInternalAudioCTS);
		if(pRenderInfo)
		{
			uiVolume = pRenderInfo->m_iClipVolume;
		}
		m_hAudioMixer->TrackInfo[iIndex]->nGain				= getGain(m_iMuteAudio == 1 ? 0 : uiVolume);

		for(int j = 0; j < m_AudioTrack[i]->m_uiChannels; j++)
		{
			if(pRenderInfo)
			{
				if(m_hAudioMixer->TrackInfo[iIndex]->nInChannel == 1)
				{
					if (pRenderInfo->m_iPanFactor[0] < -100 || pRenderInfo->m_iPanFactor[0] > 100)
					{
						m_hAudioMixer->TrackInfo[iIndex]->nPan[0] = 0;
						m_hAudioMixer->TrackInfo[iIndex]->nPan[1] = 0;
					}
					else
					{
						m_hAudioMixer->TrackInfo[iIndex]->nPan[0] = pRenderInfo->m_iPanFactor[0];
						m_hAudioMixer->TrackInfo[iIndex]->nPan[1] = pRenderInfo->m_iPanFactor[1];
					}
				}
				else
				{
					m_hAudioMixer->TrackInfo[iIndex]->nPan[j] = pRenderInfo->m_iPanFactor[j];
				}
			}
			else
			{
				m_hAudioMixer->TrackInfo[iIndex]->nPan[j] = m_AudioTrack[i]->m_iPanFactor[j];
			}
		}

		if( m_AudioTrack[i]->m_uiTrackID < 10000 )
		{
			if( m_iMasterVolume != 100 )
			{
				m_hAudioMixer->TrackInfo[iIndex]->nGain = getGain(m_iMasterVolume);
			}
		}
		else if( m_AudioTrack[i]->m_uiTrackID < 30000)
		{
			if( m_iSlaveVolume != 100 )
			{
				m_hAudioMixer->TrackInfo[iIndex]->nGain = getGain(m_iSlaveVolume);
			}
		}else{
			if( m_iMasterVolume != 100 )
			{
				m_hAudioMixer->TrackInfo[iIndex]->nGain = getGain(m_iMasterVolume);
			}
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Track info(s:%d, c:%d, v:%d, g:%d, pan:%d,%d inx:%d mv:%d sv:%d)", __LINE__,
						m_hAudioMixer->TrackInfo[iIndex]->nSamplePerFrame,
						m_hAudioMixer->TrackInfo[iIndex]->nInChannel,
						m_hAudioMixer->TrackInfo[iIndex]->nVolume,
						m_hAudioMixer->TrackInfo[iIndex]->nGain,
						m_hAudioMixer->TrackInfo[iIndex]->nPan[0],
						m_hAudioMixer->TrackInfo[iIndex]->nPan[1],
						iIndex,
						m_iMasterVolume,
						m_iSlaveVolume);
		iIndex++;
	}
	
	m_hAudioMixer->nNum_Track2Mix = m_iValidTrackCount;
	NxMixerInit(m_hAudioMixer);

	m_uiTotalSampleCount = m_iValidTrackCount == 0 ? (m_uiSampleForChannel * m_uiChannels * m_uiBitsForSample / 8) : 
					m_hAudioMixer->nOutFrameSize * m_hAudioMixer->nOutChannel * (m_uiBitsForSample>>3);
	return 0;
}

int CNEXThread_AudioRenderTask::audioMix(unsigned char* pBuf, unsigned int uiSize, unsigned int* pCTS)
{
	AudioTrackVecIter		i;
	int					iIndex				= 0;
	NXBOOL				bNeedReinitMixer	= FALSE;
	CAudioTrackInfo*		pAudioInfo		= NULL;
	unsigned int			uiTotalSample	= 0;

	int iStartTime = m_uiCurrentTime - m_uiPreviewStartTime;
	if( iStartTime < 2000 || m_uiCurrentTime > (m_uiTotalTime - 2000 ) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Audio Mix Track(I:%d C:%d T:%d)", __LINE__, m_uiInternalAudioCTS, m_uiCurrentTime, m_uiTotalTime);
	}
	else if( m_uiInternalAudioCTS > m_uiCurrentTime )
	{
		// Blow codec was added for audio track late started problem. but exynos 4412 device have bluetooth problem about it.
		if( (m_uiInternalAudioCTS - m_uiCurrentTime) > 1000 && m_eThreadState == PLAY_STATE_RUN)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] Wait next audio Track(I:%d C:%d T:%d)", __LINE__, m_uiInternalAudioCTS, m_uiCurrentTime, m_uiTotalTime);
			*pCTS = m_uiInternalAudioCTS;
			return 0;
		}
	}	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] audioMix Tracksize(%zu) pBuf(%p %d) interT(%d)", __LINE__, m_AudioTrack.size(), pBuf, uiSize, m_uiInternalAudioCTS);
	if( m_iValidTrackCount == 0 )
	{
		if(m_bDirectExport)
		{
			if(m_uiInternalAudioCTS  ==0 && m_uiTotalTime > m_uiInternalAudioCTS + 100)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] audioMix Valid Tracksize is 0 wait for DE %d, %d", 
				 	__LINE__, m_uiInternalAudioCTS, m_uiTotalTime);
				return 0;		
			}
		}

		uiTotalSample				= uiSize;
		m_ullOutPCMTotalCount		+= (uiTotalSample / 4);

		memset(pBuf, 0x00, uiSize);
		*pCTS = m_uiInternalAudioCTS;
		m_uiInternalAudioCTS	= (unsigned int)(m_ullOutPCMTotalCount * 1000 / m_uiSamplingRate);
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] audioMix Valid Tracksize is 0 pBuf(%p %d) Time(%d), SC(%d) TotalSample(%lld)", 
		 	__LINE__, pBuf, uiSize, m_uiInternalAudioCTS,uiTotalSample, m_ullOutPCMTotalCount);

		return uiTotalSample;
	}
	else if(m_bDirectExport && m_AudioTrack.size() == 1 && m_AudioTrack[0]->m_uiChannels == 2)
	{
		unsigned char* pTemp = NULL;
		if( m_AudioTrack[0]->m_bTrackValidTime == FALSE )
			return 0;
			
		int nRet = m_AudioTrack[0]->getDequeueAudioTrackBuf((unsigned char **)&pTemp, uiSize);
		if(  nRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] audioMix Track(%d) No data Time(S:%d E:%d I:%d)(Size:%d, Ret:%d)",
				__LINE__, m_AudioTrack[0]->m_uiTrackID, m_AudioTrack[0]->m_uiStartTime, m_AudioTrack[0]->m_uiEndTime, m_uiInternalAudioCTS, uiSize,nRet);

			if( m_AudioTrack[0]->getDecodingDone() )
			{
				uiTotalSample				= uiSize;
				m_ullOutPCMTotalCount		+= (uiTotalSample / 4);

				memset(pBuf, 0x00, uiSize);
				*pCTS = m_uiInternalAudioCTS;
				m_uiInternalAudioCTS	= (unsigned int)(m_ullOutPCMTotalCount * 1000 / m_uiSamplingRate);			
			}
			return uiTotalSample;
		}

		memcpy(pBuf, pTemp, uiSize);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ARTask.cpp %d] audioMix Mixing Track(Start:%d, End:%d internalTime:%d CH:%d BS :%d, S:%d)",
			__LINE__, m_AudioTrack[0]->m_uiStartTime, m_AudioTrack[0]->m_uiEndTime, m_uiInternalAudioCTS,m_AudioTrack[0]->m_uiChannels, uiSize, uiSize);

		uiTotalSample = uiSize;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] audioMix deqeue TID(%d) (Index:%d Start:%d, Cur:%d) size(%d)",
			__LINE__, m_AudioTrack[0]->m_uiTrackID, i, m_AudioTrack[0]->m_uiStartTime, m_uiInternalAudioCTS, uiTotalSample);
				
		m_AudioTrack[0]->dequeueAudioTrackBuf((unsigned char *)pTemp, uiTotalSample);
	}
	else
	{
		int iTrackIdx = 0;
		for(int i = 0; i < m_AudioTrack.size(); i++)
		{
			if( m_AudioTrack[i]->m_bTrackValidTime == FALSE )
				continue;
			
			unsigned int uiTmpSize = m_hAudioMixer->nOutFrameSize * m_AudioTrack[i]->m_uiChannels * (m_uiBitsForSample>>3);
			if( m_AudioTrack[i]->getWaitCountAtLastTime() > 20 )
			{
				m_hAudioMixer->pInputPCM[iTrackIdx] = m_pAudioMuteBuffer;
				m_AudioTrack[i]->clearAudioTrackData();
				m_AudioTrack[i]->m_bRenderFlag = FALSE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] audioMix Mixing Track with mute(Index:%d Start:%d, End:%d internalTime:%d CH:%d BS :%d, S:%d)",  
					__LINE__, i, m_AudioTrack[i]->m_uiStartTime, m_AudioTrack[i]->m_uiEndTime, m_uiInternalAudioCTS,m_AudioTrack[i]->m_uiChannels, uiTmpSize, uiSize);
			}
			else
			{
				int nRet = m_AudioTrack[i]->getDequeueAudioTrackBuf((unsigned char **)&m_hAudioMixer->pInputPCM[iTrackIdx], uiTmpSize/*uiSize*/);
				if(  nRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					if( m_AudioTrack[i]->getDecodingDone() )
						m_AudioTrack[i]->clearAudioTrackData();

					if( m_uiInternalAudioCTS > ( m_AudioTrack[i]->m_uiEndTime - 100 ) )
					{
						m_AudioTrack[i]->setWaitCountAtLastTime();
					}
					
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] audioMix(idx:%d) Track(%d) No data Time(S:%d E:%d I:%d)(Size:%d, Ret:%d)",
						__LINE__, i, m_AudioTrack[i]->m_uiTrackID, m_AudioTrack[i]->m_uiStartTime, m_AudioTrack[i]->m_uiEndTime, m_uiInternalAudioCTS, uiTmpSize,nRet);
					return 0;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] audioMix Mixing Track(idx:%d Start:%d, End:%d internalTime:%d CH:%d BS :%d, S:%d)",
					__LINE__, i, m_AudioTrack[i]->m_uiStartTime, m_AudioTrack[i]->m_uiEndTime, m_uiInternalAudioCTS,m_AudioTrack[i]->m_uiChannels, uiTmpSize, uiSize);

				m_AudioTrack[i]->m_bRenderFlag = TRUE;
				m_AudioTrack[i]->clearWaitCountAtLastTime();
			}
			iTrackIdx++;
		}

		m_hAudioMixer->pOutputPCM = (short *)pBuf;
		NxMixerProcess(m_hAudioMixer);

		iTrackIdx = 0;
		for( int i = 0; i < m_AudioTrack.size(); i++)
		{
			if( m_AudioTrack[i]->m_bTrackValidTime == FALSE )
			{
				continue;
			}
			
			if( m_AudioTrack[i]->m_bRenderFlag )
			{
				uiTotalSample = m_hAudioMixer->nOutFrameSize * m_AudioTrack[i]->m_uiChannels * (m_uiBitsForSample>>3);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] audioMix deqeue TID(%d) (Index:%d Start:%d, Cur:%d) size(%d)",
					__LINE__, m_AudioTrack[i]->m_uiTrackID, i, m_AudioTrack[i]->m_uiStartTime, m_uiInternalAudioCTS, uiTotalSample);
				
				m_AudioTrack[i]->dequeueAudioTrackBuf((unsigned char *)m_hAudioMixer->pInputPCM[iTrackIdx], uiTotalSample);
				m_AudioTrack[i]->m_bRenderFlag = FALSE;
			}
			iTrackIdx++;
		}
	}

	if( m_uiInternalAudioCTS <= m_iFadeInTime )
	{
		int iVol = calcVolume(m_uiInternalAudioCTS, 0, m_iFadeInTime, 0, m_iProjectVolume);
		processVolume(m_uiChannels, (short *)pBuf, m_hAudioMixer->nOutFrameSize * m_uiChannels * (m_uiBitsForSample>>3), m_iPreviousFadeVolume, iVol);
		m_iPreviousFadeVolume = iVol;
	}
	else if( m_uiInternalAudioCTS >= (m_uiTotalTime - m_iFadeOutTime) )
	{
		int iVol = calcVolume(m_uiInternalAudioCTS, m_uiTotalTime - m_iFadeOutTime, m_uiTotalTime, m_iProjectVolume, 0);
		processVolume(m_uiChannels, (short *)pBuf, m_hAudioMixer->nOutFrameSize * m_uiChannels * (m_uiBitsForSample>>3), m_iPreviousFadeVolume, iVol);
		m_iPreviousFadeVolume = iVol;
	}
	else
	{
		processVolume(m_uiChannels, (short *)pBuf, m_hAudioMixer->nOutFrameSize * m_uiChannels * (m_uiBitsForSample>>3), m_iProjectVolume, m_iProjectVolume);
		m_iPreviousFadeVolume = m_iProjectVolume;
	}

	*pCTS = m_uiInternalAudioCTS;
	uiTotalSample = m_hAudioMixer->nOutFrameSize * m_hAudioMixer->nOutChannel * (m_uiBitsForSample>>3);

	m_ullOutPCMTotalCount	+= m_hAudioMixer->nOutFrameSize;
	m_uiInternalAudioCTS	= (unsigned int)(m_ullOutPCMTotalCount * 1000 / m_hAudioMixer->nOutSampleRate);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ARTask.cpp %d] audioMix Tracksize(%zu) pBuf(%p %d) Time(%d), VOL(%d) SC(%d) TotalSample(%lld)",
	 	__LINE__, m_AudioTrack.size(), pBuf, uiSize, m_uiInternalAudioCTS, m_iPreviousFadeVolume, uiTotalSample, m_ullOutPCMTotalCount);

	return uiTotalSample;
}

