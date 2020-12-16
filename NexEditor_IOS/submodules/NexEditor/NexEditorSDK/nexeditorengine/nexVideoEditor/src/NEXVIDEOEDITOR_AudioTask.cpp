/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_AudioTask.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/09/06	Draft.
-----------------------------------------------------------------------------*/
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_AudioTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_AudioGain.h"
#include "NEXVIDEOEDITOR_Util.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "EnvelopDetector.h"

#define UNUSED_CLIP_BASE_TIME 0xFFFFFFFF

#define CAL_PCMDEC_MIX_GAIN_3DB_Q15		23170		//Q(15) 0.7079
#define CAL_PCMDEC_MIX_GAIN_6DB_Q15		16384		//Q(15) 0.5

CNEXThread_AudioTask::CNEXThread_AudioTask( void )
{
	m_pClipItem				= NULL;
	m_pSource				= NULL;
	m_pFileWriter			= NULL;

	m_hCodecAudio			= NULL;

	m_uiAudioObjectType		= 0;

	m_iSampleRate			= 0;
	m_iChannels				= 0;
	m_iOutChannels			= 0;
	m_iBitsForSample		= 0;
	m_iSampleForChannel		= 0;
	m_uiOutputSampleForChannel	= 0;
	m_iAACSbr				= 0;
	m_isNextFrameRead		= TRUE;
	m_pAudioTrack			= NULL;
	m_pAudioRenderer		= NULL;

	m_pThumbnail			= NULL;
	m_uMultiThreadTime = 0;
	
	m_bAutoEnvelop			= FALSE;
	m_iAutoEnvelopVol		= 100;
	m_iAutoEnvelopVolTemp	= 100;
	
	m_PCMVec.clear();
	m_PCMVec2.clear();

	m_uiClipBaseTime		= UNUSED_CLIP_BASE_TIME;
	m_eTaskPriority			= NEXSAL_PRIORITY_URGENT;

	m_pAudioDecodeBuf		=  NULL;
	m_uiAudioDecodeBufSize	= 0;
	m_iTrackID				= 0; // ?„ì‹œ ?¸ëž™ ?„ì´??

	m_ullRenderPCMSize				= 0;
	m_uiRenderPCMDuration			= 0;

	m_iSpeedFactor					= 100;
	m_iSpeedCtlInSize				= 0;
	m_iSpeedCtlOutputSize			= 0;
	
	m_isNeedResampling			= FALSE;
	m_uLastInputPCMSize			= 0;
	m_uResamplerOutputBufSize		= 0;
	m_pResamplerOutBuffer			= NULL;
	m_hNexResamplerHandle			= NULL;
	m_hNexSountHandle				= NULL;
	m_pSpeedCtlInBuffer				= NULL;
	m_iSpeedCtlInBufferRemainSize	= 0;

	m_uiAudioDecoderInitTime		= 0;

	m_pSpeedCtlOutBuffer			= NULL;

	m_iPitchIndex					= 0;
	m_iCompressorFactor				= 0;
	m_iMusicEffector				= 0;
	m_iProcessorStrength			= -1;
	m_iBassStrength					= -1;
	m_bKeepPitch					= TRUE;

	m_pEnhancedAudioFilter			= NULL;
	m_pEqualizer					= NULL;

	m_uiClipEndCTS					= 0;
	m_uiClipStartCTS				= 0;
	m_hResamplerForSpeed			= NULL;
#ifdef FOR_TEST_AUDIOENVELOP
	m_iAudioEnvelopSize				= 0;
	m_iAudioEnvelopIndex			= 0;
	m_iAudioEnvelopStartCTS			= 0;
	m_iAudioEnvelopEndCTS			= 0;
	m_iAudioEnvelopStartVolume		= 0;
	m_iAudioEnvelopEndVolume		= 0;
	m_bUpdateAudioEnvelop			= FALSE;
	m_iAudioEnvelopPrevVolume		= 0;
 	m_uiVolume						= 0;
	m_uiFirstFrameCTS				= 0xFFFFFFFF;
	m_uiStartCTSFromSeek			= 0;
#endif	
    
#ifdef FOR_TEST_AUDIO_DEC_DUMP
    m_pDecPCMDump = NEXSAL_INVALID_HANDLE;
#endif
#ifdef FOR_TEST_AUDIO_RESAMPLER_DUMP
    m_pResamplerPCMDump = NEXSAL_INVALID_HANDLE;
#endif
#ifdef FOR_TEST_AUDIO_SPEED_DUMP
    m_pSpeedPCMDump = NEXSAL_INVALID_HANDLE;
#endif

	m_bWaitPCMRender				= TRUE;

	m_isStopThumb = 0;

	m_pBuffer4Pause = NULL;
	m_uiBuffer4PauseSize = 0;	
	m_uiBuffer4PauseCTS = 0;
	m_iExportSamplingRate = EDITOR_DEFAULT_SAMPLERATE;

	m_pPreviousRenderInfo = NULL;
	m_pCurrentRenderInfo = NULL; 	

	m_pBufferRenderUpdate = NULL;
	m_iBufferRenderUpdateSize = 0;
	m_uiRenderInfoStartCTS = 0;    

	m_uBufferSizeForSpeed = 0;
	m_pBufferForSpeed =  NULL;
	m_uiLevelForSpeed = 0;
	m_uUseCount = 0;
	m_uSkipCount = 0;	
}

CNEXThread_AudioTask::~CNEXThread_AudioTask( void )
{
	unsigned int uiClipID = 0;
	if( m_pClipItem )
		uiClipID = m_pClipItem->getClipID();
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] ClipID:%d ~~~~CNEXThread_AudioTask In", __LINE__, uiClipID);
	SAFE_RELEASE(m_pClipItem);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pFileWriter);

	deinitAudioDecoder();

	SAFE_RELEASE(m_pAudioRenderer);

	if( m_pAudioDecodeBuf )
	{
		nexSAL_MemFree(m_pAudioDecodeBuf);
		m_pAudioDecodeBuf = NULL;
	}

	if(m_pBufferForSpeed)
	{
		nexSAL_MemFree(m_pBufferForSpeed);
		m_pBufferForSpeed = NULL;
	}

	if(m_pBuffer4Pause)
	{
		nexSAL_MemFree(m_pBuffer4Pause);
		m_pBuffer4Pause = NULL;
	}

	if(m_pBufferRenderUpdate)
	{
		nexSAL_MemFree(m_pBufferRenderUpdate);
		m_pBufferRenderUpdate = NULL;
	}

	if( m_pThumbnail && m_PCMVec.size() > 0 )
	{
		unsigned char* pTempBuff = (unsigned char*)nexSAL_MemAlloc(sizeof(unsigned char)*(m_PCMVec.size() + m_PCMVec2.size()));
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] m_PCMVec.size() %d, %d", __LINE__, m_PCMVec.size(), m_PCMVec2.size());
        
		if( pTempBuff )
		{
			for(int i = 0; i < m_PCMVec.size(); i++)
			{
				pTempBuff[i] = m_PCMVec[i];
			}

			for(int i = 0; i < m_PCMVec2.size(); i++)
			{
				pTempBuff[i + m_PCMVec.size()] = m_PCMVec2[i];
			}

			m_pThumbnail->setPCMTableInfo((int)m_PCMVec.size() + (int)m_PCMVec2.size(), pTempBuff);
			nexSAL_MemFree(pTempBuff);
		}
	}
    
	m_pThumbnail = NULL;
	m_PCMVec.clear();
	m_PCMVec2.clear();    
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] ClipID:%d ~~~~CNEXThread_AudioTask Out", __LINE__, uiClipID);
}

void CNEXThread_AudioTask::End( unsigned int uiTimeout )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] End In(ClipID:%d)", __LINE__, m_pClipItem->getClipID());
	if( m_bIsWorking == FALSE ) return;

	m_bIsWorking = FALSE;

	if(m_bIsTaskPaused)
		nexSAL_SemaphoreRelease(m_hSema);

	if( nexSAL_TaskWait(m_hThread) != 0 )
	{
		nexSAL_TaskTerminate(m_hThread);
	}
	nexSAL_TaskDelete(m_hThread);
	m_hThread = NEXSAL_INVALID_HANDLE;

	if( m_hSema != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_SemaphoreDelete(m_hSema);
		m_hSema = NEXSAL_INVALID_HANDLE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] End Out(ClipID:%d)", __LINE__, m_pClipItem->getClipID());
}

void CNEXThread_AudioTask::WaitTask()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] WaitTask In(%d)", __LINE__, m_pClipItem->getClipID());
	if( m_pThumbnail  )
	{
		nexSAL_TaskWait(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] WaitTask Wait end(%d)", __LINE__, m_pClipItem->getClipID());
		nexSAL_TaskDelete(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] WaitTask delete end(%d)", __LINE__, m_pClipItem->getClipID());
		m_hThread = NEXSAL_INVALID_HANDLE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] WaitTask Out(%d)", __LINE__, m_pClipItem->getClipID());
}

NXBOOL CNEXThread_AudioTask::setSpeedFactor(int iFactor)
{
	
	if( iFactor < 3 || iFactor > 1600 )
		return FALSE;
	m_iSpeedFactor = iFactor;
	return TRUE;
}
NXBOOL CNEXThread_AudioTask::setVoiceChangerFactor(int iFactor)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] setVoiceChangerFactor(%d)", __LINE__, iFactor);
	if( iFactor < 0 || iFactor > 4 )
		return FALSE;
	m_iVoiceChangerFactor= iFactor;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setPitchFactor(int iFactor)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] setPitchFactor(%d)", __LINE__, iFactor);
	if( iFactor < -12 || iFactor > 12 )
		return FALSE;
	m_iPitchIndex = iFactor;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setKeepPitch(NXBOOL bKeepPitch)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] setPitchFactor(%d)", __LINE__, bKeepPitch);
	m_bKeepPitch = bKeepPitch;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setCompressorFactor(int iFactor)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] setCompressorFactor(%d)", __LINE__, iFactor);
	if( iFactor < 0 || iFactor > 7 )
		return FALSE;
	m_iCompressorFactor = iFactor;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setMusicEffectFactor(int iFactor, int iProcessorStrength, int iBassStrength)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) setMusicEffectFactor(%d, %d, %d)", __LINE__, m_pClipItem?m_pClipItem->getClipID():0, iFactor, iProcessorStrength, iBassStrength);
	if( iFactor < 0 || iFactor > 3 )
		return FALSE;

	m_iMusicEffector		= iFactor;
	m_iProcessorStrength	= iProcessorStrength;
	m_iBassStrength		= iBassStrength;

	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setEqualizer(char* pEqualizer)
{
	NXBOOL bRet = FALSE;
#if 0
	if (m_pCurrentRenderInfo->m_pEqualizer == NULL && pEqualizer)
	{
		if (m_pEqualizer)
		{
			nexSAL_MemFree(m_pEqualizer);
			m_pEqualizer = NULL;
		}
		m_pEqualizer = (char*)nexSAL_MemAlloc(strlen(pEqualizer)+1);
		strcpy(m_pEqualizer, pEqualizer);
	}
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] setEqualizer(%s)", __LINE__, pEqualizer?pEqualizer:"NULL");
	return bRet;
}

NXBOOL CNEXThread_AudioTask::setEnhancedAudioFilter(char* pEnhancedAudioFilter)
{
	NXBOOL bRet = FALSE;
#if 0
	if (m_pCurrentRenderInfo->m_pEnhancedAudioFilter == NULL && pEnhancedAudioFilter)
	{
		if (m_pEnhancedAudioFilter)
		{
			nexSAL_MemFree(m_pEnhancedAudioFilter);
			m_pEnhancedAudioFilter = NULL;
		}
		m_pEnhancedAudioFilter = (char*)nexSAL_MemAlloc(strlen(pEnhancedAudioFilter)+1);
		strcpy(m_pEnhancedAudioFilter, pEnhancedAudioFilter);
	}
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] setEnhancedAudioFilter(%s)", __LINE__, pEnhancedAudioFilter?pEnhancedAudioFilter:"NULL");
	return bRet;
}

NXBOOL CNEXThread_AudioTask::setPCMRenderWaitFlag(NXBOOL bWait)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Clip(%d) setPCMRenderWaitFlag(%d)", __LINE__, m_pClipItem->getClipID(), bWait);
	m_bWaitPCMRender = bWait;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setThumbnailHandle(CThumbnail* pThumbnail)
{
	m_pThumbnail = pThumbnail;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setThumbnailMultiThreadTime(unsigned int uTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Clip(%d) setThumbnailMultiThreadTime(%d)", __LINE__, m_pClipItem->getClipID(), uTime);
	m_uMultiThreadTime = uTime;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setAutoEnvelop(NXBOOL bAutoEnvelop)
{
	m_bAutoEnvelop = bAutoEnvelop;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Clip(%d) setAutoEnvelop(%d)", __LINE__, m_pClipItem->getClipID(), m_bAutoEnvelop);
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setStartCTS(unsigned int uiCTS)
{
	m_uiStartCTSFromSeek = uiCTS;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] setStartFrameCTS(%d)",__LINE__, m_uiStartCTSFromSeek);
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setUseSkipCount(unsigned int uUseCount, unsigned int uSkipCount)
{
	m_uUseCount = uUseCount;
	m_uSkipCount = uSkipCount;    
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] setUseSkipCount(%d, %d)",__LINE__, uUseCount, uSkipCount);
	return TRUE;
}

int CNEXThread_AudioTask::OnThreadMain( void )
{
	NEXVIDEOEDITOR_ERROR	eRet				= NEXVIDEOEDITOR_ERROR_NONE;
	unsigned int			uiPCMBufSize		= m_uiAudioDecodeBufSize;
	unsigned int			uiResultTime		= 0;
	NXINT64				uiCTS				= 0;
	unsigned char*			m_pAudioPCMBuf		= NULL;
	unsigned char*			pFrame				= NULL;
	unsigned int			uiFrameSize			= 0;
	unsigned int			uiDecoderErrRet		= 0;

	EnvelopDetector			pcmDetector;

	NXBOOL					bAutoEnvelopProcess	= FALSE;

	unsigned int			uiPCMLevelCTS		= 0;
	unsigned int			uiClipTotalTime		= 0;
	unsigned int			uiTotalDuration 	= 0;
	
	unsigned long long		ulDecodedPCMSize 	= 0;
	unsigned int			uiDecodedPCMDuration	= 0;
	unsigned int			uiFrameDuration 	= 0;

	NXBOOL					bFrameEnd 			= FALSE;

	unsigned int 			uiSkipPCMSize		= 0;	
	unsigned int			uiSkipFrameCount	= 0;
	
	unsigned int			uStartTime			= 0;
	unsigned int			uEndTime 			= 0;
	unsigned int 			uiAudioDuration 	= 0;
	unsigned int			uThumbPCMSize		= 0;
	NXINT64				uiLastCTS = 0;
	m_uiFirstFrameCTS		= 0xFFFFFFFF;

	unsigned int uUsedCound = 0;
	unsigned int uSkipedCount = 0;

	m_uiClipEndCTS			= m_uiClipBaseTime + m_pClipItem->getEndTime();
	m_uiClipStartCTS		= m_uiClipBaseTime + m_pClipItem->getStartTime();
	uiAudioDuration			= m_pClipItem->getTotalTime();

	CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
	if( pProjectMng )
	{
		uiTotalDuration = pProjectMng->getDuration();	
		if(CNexProjectManager::getCurrentState() == PLAY_STATE_RECORD)
			m_iExportSamplingRate = pProjectMng->getExportSamplingRate();
		m_uiClipEndCTS = m_uiClipEndCTS < uiTotalDuration ? m_uiClipEndCTS : uiTotalDuration;		
		SAFE_RELEASE(pProjectMng);
	}

	if(m_pClipItem->getClipType() == CLIPTYPE_AUDIO)
	{
		// m_uiClipEndCTS	-= (m_pClipItem->getStartTrimTime()+m_pClipItem->getEndTrimTime());
		uiAudioDuration = m_pClipItem->getTotalTime() - (m_pClipItem->getStartTrimTime()+m_pClipItem->getEndTrimTime());

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ClipType is Audio :  Total(%d) StartTrim(%d) EndTrim(%d) audioDuration(%d)", __LINE__, m_pClipItem->getTotalTime(), m_uiClipStartCTS, m_uiClipEndCTS, uiAudioDuration);
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] m_ClipAudioRenderInfoVec.size (%d, %zu)\n", __LINE__,  m_pClipItem->getClipID(),m_pClipItem->m_ClipAudioRenderInfoVec.size());
	if(m_pClipItem->m_ClipAudioRenderInfoVec.size())
	{
		m_pCurrentRenderInfo = m_pClipItem->m_ClipAudioRenderInfoVec[0];
		m_pPreviousRenderInfo = m_pClipItem->m_ClipAudioRenderInfoVec[0];		
	}
		
	if( m_pFileWriter != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ClipID(%d) ADTask Task without Decoder", __LINE__, m_pClipItem->getClipID());
	}
	else
	{
		if( initAudioDecoder(m_uiCurrentTime) == FALSE )
		{
			CNexProjectManager::sendEvent(MESSAGE_ADEC_INIT_FAIL, CNexProjectManager::getCurrentState(), NEXVIDEOEDITOR_ERROR_CODEC_INIT, m_pClipItem->getClipID(), 0);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] ClipID(%d) ADTask Task init Decoder failed ", __LINE__, m_pClipItem->getClipID());
			return 181818;
		}
	}
	
#ifdef FOR_TEST_AUDIOENVELOP
	if( m_pClipItem->isAudioEnvelopEnabled() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  Audio Clip(%d) enable SetAudioEnvelop Size(%d)", __LINE__,m_pCurrentRenderInfo->muiTrackID, m_pCurrentRenderInfo->m_iEnvelopAudioSize);
		m_iAudioEnvelopSize = m_pCurrentRenderInfo->m_iEnvelopAudioSize;
		m_iAudioEnvelopIndex = 1;
		m_bUpdateAudioEnvelop = TRUE;

		for( m_iAudioEnvelopIndex = 0 ; m_iAudioEnvelopIndex < m_iAudioEnvelopSize ; m_iAudioEnvelopIndex++)
		{
			m_iAudioEnvelopEndCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[m_iAudioEnvelopIndex];		
			m_iAudioEnvelopEndVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[m_iAudioEnvelopIndex];
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  (Index: %d, Volume: %d, CTS:%d)", __LINE__, m_iAudioEnvelopIndex, m_iAudioEnvelopEndVolume, m_iAudioEnvelopEndCTS );
		}
		m_iAudioEnvelopIndex = 1;
	}
#endif
	// for JIRA 3195 issue
	initAutoEnvelopVolume(m_pClipItem->getStartTime());
	
	unsigned int			uiThumbnailTime		= nexSAL_GetTickCount();

	m_pSource->getTotalPlayTime(&uiClipTotalTime);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Clip(%d) : Base time(%d) ClipStart(%d) ClipEnd(%d) Cur(%lld)", __LINE__, m_pClipItem->getClipID(), m_uiClipBaseTime, m_uiClipStartCTS, m_uiClipEndCTS, uiCTS);

#ifdef FOR_TEST_AUDIO_DEC_DUMP
    char strFile[256];
    sprintf(strFile, "/sdcard/%d_clip_dec.pcm" , m_pClipItem->getClipID());
    m_pDecPCMDump = nexSAL_FileOpen(strFile, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
#endif
#ifdef FOR_TEST_AUDIO_RESAMPLER_DUMP
    char strFile1[256];
    sprintf(strFile1, "/sdcard/%d_clip_resampler.pcm" , m_pClipItem->getClipID());
    m_pResamplerPCMDump = nexSAL_FileOpen(strFile, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
#endif
#ifdef FOR_TEST_AUDIO_SPEED_DUMP
    char strFile2[256];
    sprintf(strFile2, "/sdcard/%d_clip_speed.pcm" , m_pClipItem->getClipID());
    m_pSpeedPCMDump = nexSAL_FileOpen(strFile2, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
#endif
    
#if 0
	// Case : mantis 9267 when decoder channel info was wrong.
	while( m_pThumbnail == NULL && m_bIsWorking && m_pAudioTrack->m_bRenderRegistered == FALSE )
	{
		nexSAL_TaskSleep(10);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] wait regist audio track(start %d) ", __LINE__, m_pAudioTrack->m_bRenderRegistered );
	}
#endif	

	if( m_pThumbnail )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb PCMDetector Init(S(%d) C(%d)", 
			__LINE__, m_iSampleRate, m_iChannels);
		pcmDetector.Init(m_iChannels, m_iSampleRate);

		if(m_pThumbnail->getRequestRawData())
		{
			int pcmSize = 0;

			m_pThumbnail->getStartEndTime(&uStartTime, &uEndTime);
			if(uStartTime != uEndTime)
			{
				pcmSize = (uEndTime - uStartTime)*m_iSampleRate/1000;
			}

			m_pThumbnail->setAudioPCMSize(pcmSize + 384*1024);
		}
	}

#ifdef FOR_PROJECT_LGE
	m_uiLevelForSpeed = 75;
#else
	m_uiLevelForSpeed = 12;
#endif

	if (m_iOutChannels)
	{
		initSpeedCtl();
		initMusicEffect();
		initCompressor();
		initPitchContorl();
		initVoiceChanger();
		initEnhancedAudioFilter();
		initEqualizer();
	}

	while ( m_bIsWorking )
	{
		if(m_bIsActive)
		{

			if (m_bIsTaskResumed == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] AudioDecTask RESUMED!!!\n",  __LINE__);
				if(m_bIsTaskPaused)
				{
					#if 0
					while(1)
					{
						if(m_pSource->getAudioFrame() != _SRC_GETFRAME_OK)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] AudioDecTask resume audio frame end!!!\n",  __LINE__);					
							break;
						}
						m_pSource->getAudioFrameCTS( &uiCTS );


						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] ID(%d) AudioDecTask resume audio frame drop %lld, %lld!!!\n",  __LINE__, m_pClipItem->getClipID(), uiCTS, uiLastCTS);					
						if(uiLastCTS < uiCTS )
						{
							m_isNextFrameRead = FALSE;					
							break;
						}
					}
					#endif
				}
				m_bIsTaskResumed = TRUE;
				m_bIsTaskPaused = FALSE;
			}		

		if( m_pThumbnail )
		{
			if(m_isStopThumb)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] ID(%d) ADTask Task thumb stop flag set ", __LINE__, m_pClipItem->getClipID());			
				break;
			}

			if(m_isNextFrameRead)
			{
				if( m_pSource->getAudioFrame() != _SRC_GETFRAME_OK )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb Audio DecTask get pcm level end Time(%d)", __LINE__, nexSAL_GetTickCount() - uiThumbnailTime);
					bFrameEnd = TRUE;
				}
				m_isNextFrameRead = FALSE;
			}

			if( m_uiAudioObjectType == eNEX_CODEC_A_AAC_S )
			{
				if( uiPCMLevelCTS > uiClipTotalTime )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb Audio DecTask get pcm level end Time(%d)", __LINE__, nexSAL_GetTickCount() - uiThumbnailTime);
						break;
				}

				if( bFrameEnd )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb Audio DecTask EOS received and Exit using software codec(%d)", __LINE__, nexSAL_GetTickCount() - uiThumbnailTime);
					break;
				}
			}

			if( bFrameEnd == FALSE)
			{
				m_pSource->getAudioFrameCTS( &uiCTS );
				m_pSource->getAudioFrameData( &pFrame, &uiFrameSize );
			}

			if((unsigned int)uiCTS >= m_uMultiThreadTime && m_uMultiThreadTime != 0)
			{
				bFrameEnd = TRUE;
			}

			if(bFrameEnd == FALSE)
			{
				if(m_uUseCount != 0 && m_uUseCount == uUsedCound)
				{
					if(m_uSkipCount == uSkipedCount)
					{
						uUsedCound = 0;
						uSkipedCount = 0;
					}
					else
					{
						uSkipedCount++;
						m_isNextFrameRead = TRUE;
						continue;
					}
				}
			}

			if( uiCTS < 0 )
			{
				m_isNextFrameRead = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Drop frame before EditBox Time(%lld)",__LINE__, uiCTS);
				continue;
			}

			uiPCMBufSize = m_uiAudioDecodeBufSize;
			nexCAL_AudioDecoderDecode(	m_hCodecAudio,
										pFrame, 
										uiFrameSize, 
										NULL, 
										m_pAudioDecodeBuf,
										(int *)&uiPCMBufSize,
										(unsigned int)uiCTS,
										NULL,
										bFrameEnd == FALSE?NEXCAL_ADEC_FLAG_NONE:NEXCAL_ADEC_FLAG_END_OF_STREAM,
										&uiDecoderErrRet );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d] Thumb After Decode. (Buf %p, BufSize:%d, TS:%lld Ret:0x%x)", 	__LINE__, m_pAudioDecodeBuf, uiPCMBufSize , uiCTS, uiDecoderErrRet);

			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_NEXT_FRAME))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d] Thumb getNextFrame is OK!", __LINE__);
				m_isNextFrameRead = TRUE;
				uUsedCound++;
			}

			if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_DECODING_SUCCESS) )
			{
				if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_CHANGED))
				{
					unsigned int uiSamplingRate = 0;
					unsigned int uiChannels = 0;
					unsigned int uiBitPerSample = 0;
					unsigned int uiNumOfSamplesPerChannel = 0;
					nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_SAMPLINGRATE, &uiSamplingRate);
					nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS, &uiChannels);
					nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE, &uiBitPerSample);
					nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL, &uiNumOfSamplesPerChannel);

					if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
					{
						if(CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec == FALSE && m_iChannels == 1)
						{
							uiChannels = m_iChannels;
						}
					}

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb Audio Output was changed!(S(%d) C(%d) BpS(%d) SC(%d)", 
						__LINE__, uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
					reinitAudioTrack(uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
					pcmDetector.Init(m_iOutChannels, m_iSampleRate);
				}

				if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_EXIST))
				{
					float					fEnvelopRMS			= 0.0;
					float				fEnvelop[m_iChannels];
					int						iPCMRMS				= 0;
					int						iPCM[m_iChannels];
					unsigned int			uiPCMLevelFrameCnt	= 0;
					int iPCMSamplePerChannel = 128;
				
					if(m_iChannels > 2 && m_iOutChannels == 2)
					{
						downChannels(m_pAudioDecodeBuf, uiPCMBufSize, &uiPCMBufSize);
					}

#if 1
					m_pAudioPCMBuf = m_pAudioDecodeBuf;
					while(uiPCMBufSize > iPCMSamplePerChannel*(m_iOutChannels *m_iBitsForSample / 8))
					{
						pcmDetector.AudioThumbProcess((short*)m_pAudioPCMBuf, &fEnvelopRMS, fEnvelop,  iPCMSamplePerChannel);
						iPCMRMS		= (int)(fEnvelopRMS * 255.0);
						m_PCMVec.insert(m_PCMVec.end(), (unsigned char)iPCMRMS);
						uiPCMBufSize -= iPCMSamplePerChannel*(m_iOutChannels *m_iBitsForSample / 8);
						m_pAudioPCMBuf += iPCMSamplePerChannel*(m_iOutChannels *m_iBitsForSample / 8);
						uiPCMLevelFrameCnt++;                    
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 2, "[ADTask.cpp %d] Thumb Audio PCM (%d)", __LINE__, iPCMRMS);
					}
#else
					pcmDetector.AudioThumbProcess((short*)m_pAudioDecodeBuf, &fEnvelopRMS, fEnvelop,  uiPCMBufSize/(m_iOutChannels *m_iBitsForSample / 8));
					iPCMRMS		= (int)(fEnvelopRMS * 255.0);
					m_PCMVec.insert(m_PCMVec.end(), (unsigned char)iPCMRMS);
					uiPCMLevelFrameCnt++;                    
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 2, "[ADTask.cpp %d] Thumb Audio PCM (%d)", __LINE__, iPCMRMS);
#endif
					if(m_pThumbnail->getRequestRawData())
					{
						unsigned int pcmTime = 0;

						m_pThumbnail->addAudioPCM(uiPCMBufSize, m_pAudioDecodeBuf);
						uThumbPCMSize += uiPCMBufSize;

						pcmTime = uThumbPCMSize/(m_iOutChannels *m_iBitsForSample/8)*1000/m_iSampleRate;

						if(pcmTime > uEndTime - uStartTime)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ready Raw Thumb Data %d, %d, %d", __LINE__, pcmTime, uStartTime, uEndTime);
							break;
						}
					}

					if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Task EOS received", __LINE__);
						break;
					}
				}
				else
				{
					if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Task EOS received", __LINE__);
						break;
					}

					if( bFrameEnd == TRUE)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb Audio is End!!", __LINE__);
						break;
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d] Thumb Audio Output is empty!!", __LINE__);
					}
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb Audio Dec Failed(%p %d %lld Ret:%d)", __LINE__, pFrame, uiFrameSize, uiCTS, uiDecoderErrRet);
				m_isNextFrameRead = TRUE;

				if( bFrameEnd  == TRUE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Thumb FrameEnd. Audio Dec Failed(%p %d %lld Ret:%d)", __LINE__, pFrame, uiFrameSize, uiCTS, uiDecoderErrRet);
					break;
				}
			}

			continue;
		}
		
		if( m_pFileWriter != NULL )
		{
			if( m_isNextFrameRead )
			{
				if( m_pSource->getAudioFrame() != _SRC_GETFRAME_OK )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) getAudioFrame End", __LINE__, m_pClipItem->getClipID());
					// bFrameEnd = TRUE;
					int iRet = m_pFileWriter->setAudioFrameWithoutEncode(m_pClipItem->getClipID(), m_pClipItem->getEndTime(), NULL, 0);
					if( iRet == 2 )
					{
						nexSAL_TaskSleep(10);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ADTask.cpp %d] ID(%d) Audio One frame Write Wait(%lld, %d)", __LINE__, m_pClipItem->getClipID(), uiCTS, iRet);
						continue;
					}					
					break;
				}
				m_isNextFrameRead = FALSE;

				m_pSource->getAudioFrameCTS( &uiCTS );
				m_pSource->getAudioFrameData( &pFrame, &uiFrameSize );

				if( (unsigned int)uiCTS < m_pClipItem->getStartTime() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) DirectExport Audio Frame before time(ST:%d CUR:%lld)", __LINE__, m_pClipItem->getClipID(), m_pClipItem->getStartTime(), uiCTS);
					m_isNextFrameRead = TRUE;
					continue;
				}

				if( (unsigned int)uiCTS > m_pClipItem->getEndTime() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) DirectExport Audio Frame after time(ET:%d CUR:%lld)", __LINE__, m_pClipItem->getClipID(), m_pClipItem->getEndTime(), uiCTS);
					m_pFileWriter->setAudioFrameWithoutEncode(m_pClipItem->getClipID(), (unsigned int)uiCTS, NULL, 0);
					break;					
				}
			}

			int iRet = m_pFileWriter->setAudioFrameWithoutEncode(m_pClipItem->getClipID(), (unsigned int)uiCTS, pFrame, uiFrameSize);
			if( iRet == 0 )
			{
				m_isNextFrameRead = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ADTask.cpp %d] ID(%d) Audio One frame Write sucess(%lld, %d)", __LINE__, m_pClipItem->getClipID(), uiCTS, iRet);
			}
			else if( iRet == 1 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) Audio One frame Write failed(%lld, %d)", __LINE__, m_pClipItem->getClipID(), uiCTS, iRet);
				m_isNextFrameRead = TRUE;
			}
			else if( iRet == 2 )
			{
				nexSAL_TaskSleep(3);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ADTask.cpp %d] ID(%d) Audio One frame Write Wait(%lld, %d)", __LINE__, m_pClipItem->getClipID(), uiCTS, iRet);
			}
			else if( iRet == 3 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) Audio One frame Write Drop(%lld, %d)", __LINE__, m_pClipItem->getClipID(), uiCTS, iRet);
				m_isNextFrameRead = TRUE;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) Audio One frame Write unknown(%lld, %d)", __LINE__, m_pClipItem->getClipID(), uiCTS, iRet);
			}
			continue;
		}
		
		if( m_isNextFrameRead )
		{
			int iReaderRet = m_pSource->getAudioFrame();
			if( iReaderRet == _SRC_GETFRAME_END )
			{
				NXBOOL bRepeat = isRepeat((unsigned int)uiCTS + 500);
				if( bRepeat )
				{
					if( m_pSource->setRepeatAudioClip((unsigned int)uiCTS, m_pClipItem->getStartTrimTime()) )
					{
						m_isNextFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) Audio Clip Repeat Start", __LINE__, m_pClipItem->getClipID());
						continue;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) Audio Clip Repeat failed", __LINE__, m_pClipItem->getClipID());
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) getAudioFrame End", __LINE__, m_pClipItem->getClipID());
				bFrameEnd = TRUE;
				//break;
			}
			else if( iReaderRet != _SRC_GETFRAME_OK )
			{
				if(iReaderRet == _SRC_GETFRAME_ERROR)
				{
					if(CNexProjectManager::getCurrentState() == PLAY_STATE_RECORD)
						CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_pClipItem->getClipID(), 0);			
					else
						CNexProjectManager::sendEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_pClipItem->getClipID(), 0);								
					bFrameEnd = TRUE;
				}
				m_isNextFrameRead = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) Frame read error %d", __LINE__, m_pClipItem->getClipID(), iReaderRet);
				continue;
			}

			m_isNextFrameRead = FALSE;
		}

		if( bFrameEnd == FALSE)
		{
			m_pSource->getAudioFrameCTS( &uiCTS );
			m_pSource->getAudioFrameData( &pFrame, &uiFrameSize );
		}

		// for KMSA 380 issue.
		if( uiCTS < 0 )
		{
			m_isNextFrameRead = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Drop frame because not play time(%lld)",__LINE__, uiCTS);
			continue;
		}

 
		if( m_uiFirstFrameCTS == 0xFFFFFFFF)
		{
			m_uiFirstFrameCTS  = (unsigned int)uiCTS;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] SetFirstFrame(%d), iStartCTSFromSeek(%d)",__LINE__, m_uiFirstFrameCTS, m_uiStartCTSFromSeek);

			if(m_uiFirstFrameCTS > m_uiStartCTSFromSeek)
				m_uiStartCTSFromSeek = 0;

			NXINT64 iTimeForUI = m_uiStartCTSFromSeek == 0?uiCTS:m_uiStartCTSFromSeek;
			if(m_pClipItem->m_ClipAudioRenderInfoVec.size())
			{
				CClipAudioRenderInfo* pRenderInfo = NULL;
				iTimeForUI = iTimeForUI - m_pClipItem->getStartTime() + m_pClipItem->getStartTrimTime();
				pRenderInfo = m_pClipItem->getActiveAudioRenderInfo(iTimeForUI, TRUE);

				if(pRenderInfo != NULL && pRenderInfo != m_pCurrentRenderInfo)
				{
					m_pPreviousRenderInfo = m_pCurrentRenderInfo;
					m_pCurrentRenderInfo = pRenderInfo;

					if(checkAudioRenderInfo(m_pCurrentRenderInfo, m_pPreviousRenderInfo))
						updateAudioRenderInfo();

#ifdef FOR_TEST_AUDIOENVELOP
					if( m_pClipItem->isAudioEnvelopEnabled() )
					{
						m_iAudioEnvelopSize = m_pCurrentRenderInfo->m_iEnvelopAudioSize;
						m_iAudioEnvelopIndex = 1;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  Audio Clip(%d) enable SetAudioEnvelop Size(%d)", __LINE__,m_pCurrentRenderInfo->muiTrackID, m_iAudioEnvelopSize);		
						m_bUpdateAudioEnvelop = TRUE;

						for( m_iAudioEnvelopIndex = 0 ; m_iAudioEnvelopIndex < m_iAudioEnvelopSize ; m_iAudioEnvelopIndex++)
						{
							m_iAudioEnvelopEndCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[m_iAudioEnvelopIndex];		
							m_iAudioEnvelopEndVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[m_iAudioEnvelopIndex];
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  (Index: %d, Volume: %d, CTS:%d)", __LINE__, m_iAudioEnvelopIndex, m_iAudioEnvelopEndVolume, m_iAudioEnvelopEndCTS );
						}
						m_iAudioEnvelopIndex = 1;
					}
#endif
				}
				CCalcTime		calcTime;
				m_uiRenderInfoStartCTS = calcTime.applySpeed(m_pCurrentRenderInfo->mStartTime, m_pCurrentRenderInfo->mStartTrimTime, m_pCurrentRenderInfo->m_iSpeedCtlFactor, (unsigned int)iTimeForUI);
			}
		}

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) Audio Decode start",__LINE__, m_pClipItem->getClipID());
		
		uiPCMBufSize = m_uiAudioDecodeBufSize;// m_iSampleForChannel*m_iOutChannels*(m_iBitsForSample/8);

		nexCAL_AudioDecoderDecode(	m_hCodecAudio,
									pFrame, 
									uiFrameSize, 
									NULL, 
									m_pAudioDecodeBuf,
									(int *)&uiPCMBufSize,
									(unsigned int)uiCTS,
									NULL,
									bFrameEnd == FALSE?NEXCAL_ADEC_FLAG_NONE:NEXCAL_ADEC_FLAG_END_OF_STREAM,
									&uiDecoderErrRet );

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, (m_uLogCnt++)%50, "[ADTask.cpp %d] Clip(%d) Audio Decode. (Buf %p, BufSize:%d, TS:%lld Time:%d, Ret:0x%x)",
			__LINE__, m_pClipItem->getClipID(), m_pAudioDecodeBuf, uiPCMBufSize , uiCTS, uiDecodedPCMDuration, uiDecoderErrRet);
		
		if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_NEXT_FRAME))
		{
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] getNextFrame is OK!", __LINE__);

			if(m_uiStartCTSFromSeek > (unsigned int)uiCTS)
				uiSkipFrameCount ++;
			
			m_isNextFrameRead = TRUE;
			uiLastCTS = uiCTS;
		}

		if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_DECODING_SUCCESS))
		{
			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_CHANGED))
			{
				unsigned int uiSamplingRate = 0;
				unsigned int uiChannels = 0;
				unsigned int uiBitPerSample = 0;
				unsigned int uiNumOfSamplesPerChannel = 0;
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_SAMPLINGRATE, &uiSamplingRate);
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS, &uiChannels);
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE, &uiBitPerSample);
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL, &uiNumOfSamplesPerChannel);

				if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
				{
					if(CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec == FALSE && m_iChannels == 1)
					{
						uiChannels = m_iChannels;
					}	
				}	
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip ID(%d) Audio Output was changed!(S(%d) C(%d) BpS(%d) SC(%d)", 
					__LINE__, m_pClipItem->getClipID(), uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
				reinitAudioTrack(uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
			}
			
			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_EXIST))
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) Audio Decode. (Buf %p, BufSize:%d, TS:%lld Time:%d, Ret:0x%x)", __LINE__, m_pClipItem->getClipID(), m_pAudioDecodeBuf, uiPCMBufSize , uiCTS, uiDecodedPCMDuration, uiDecoderErrRet);
				if(m_iChannels > 2 && m_iOutChannels == 2)
				{
					downChannels(m_pAudioDecodeBuf, uiPCMBufSize, &uiPCMBufSize);
				}
				if(m_uiStartCTSFromSeek != 0)
				{
					unsigned int uiSamplingRate = m_iSampleRate;
					unsigned int uiChannels = m_iOutChannels;
					unsigned int uiBitPerSample = m_iBitsForSample;					
					unsigned int uiNumOfSamplesPerChannel = 0;

					uiSkipPCMSize += uiPCMBufSize;

					if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
					{					
						if(CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec == FALSE && m_iChannels == 1)
						{
							uiChannels = m_iChannels;
						}	
					}	

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip ID(%d) Audio Output was changed!m_uiStartCTSFromSeek (S(%d) C(%d) BpS(%d) SC(%d)", 
						__LINE__, m_pClipItem->getClipID(), m_uiStartCTSFromSeek, uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
					
					if(m_uiFirstFrameCTS + uiSkipPCMSize/uiChannels/(uiBitPerSample/8)*1000/uiSamplingRate < m_uiStartCTSFromSeek)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) Adjust Audio PCM %d", 
							__LINE__, m_pClipItem->getClipID(), uiSkipPCMSize);	
						continue;
					}
					else
					{
						if ( (m_pSource->getAudioObjectType() == eNEX_CODEC_A_AAC || m_pSource->getAudioObjectType() == eNEX_CODEC_A_AACPLUS))
						{
							if(m_iSampleForChannel == 1024) uiNumOfSamplesPerChannel = 1024;
							else if(m_iSampleForChannel == 2048) uiNumOfSamplesPerChannel = 2048;
							else if(m_iAACSbr) uiNumOfSamplesPerChannel = 2048;
							else uiNumOfSamplesPerChannel = 1024;
						}
						else
						{
							m_pSource->getSamplesPerChannel(&uiNumOfSamplesPerChannel);
						}

						unsigned int remain_size = 0;
						//unsigned int abandon_size = uiSkipFrameCount*uiNumOfSamplesPerChannel*uiChannels*(uiBitPerSample/8);
						unsigned int abandon_size = (m_uiStartCTSFromSeek- m_uiFirstFrameCTS)*uiSamplingRate/1000;
						abandon_size &= 0xfffffff0;
						abandon_size = abandon_size * uiChannels*(uiBitPerSample/8);
						remain_size = uiSkipPCMSize - abandon_size;

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) Adjust Audio PCM finished %d, %d", 
							__LINE__, m_pClipItem->getClipID(), uiSkipPCMSize, abandon_size);	

						if(uiPCMBufSize > remain_size  && remain_size != 0)
						{
							memmove(m_pAudioDecodeBuf, m_pAudioDecodeBuf + (uiPCMBufSize - remain_size), remain_size);
							uiPCMBufSize = remain_size;
						}

						m_uiFirstFrameCTS = m_uiStartCTSFromSeek;
						m_uiStartCTSFromSeek = 0;

						if(remain_size == 0)
						{
							continue;
						}
					}
				}

			
				// for mantis 10756
				if( m_bAutoEnvelop )
				{
					unsigned int uiNewTime = m_uiRenderInfoStartCTS + m_uiRenderPCMDuration + m_uiBuffer4PauseSize*1000/m_iExportSamplingRate;
				
					if( bAutoEnvelopProcess == FALSE && checkAutoEnvelopProcess((unsigned int)uiNewTime) )
					{
						bAutoEnvelopProcess = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) Auto Envelop Start(%d -> %d %d)", 
									__LINE__, m_pClipItem->getClipID(), m_iAutoEnvelopVolTemp, m_iAutoEnvelopVol, bAutoEnvelopProcess);								
					}

					if( bAutoEnvelopProcess )
					{
						if( m_iAutoEnvelopVolTemp < m_iAutoEnvelopVol )
						{
							if( (m_iAutoEnvelopVolTemp + 5) > m_iAutoEnvelopVol )
							{
								bAutoEnvelopProcess = FALSE;
								processVolume(m_iOutChannels, (short*)m_pAudioDecodeBuf, uiPCMBufSize, m_iAutoEnvelopVolTemp, m_iAutoEnvelopVol);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) Auto Envelop End(%d %d)", 
									__LINE__, m_pClipItem->getClipID(), m_iAutoEnvelopVol, bAutoEnvelopProcess);								
							}
							else
							{
								processVolume(m_iOutChannels, (short*)m_pAudioDecodeBuf, uiPCMBufSize, m_iAutoEnvelopVolTemp, m_iAutoEnvelopVolTemp + 5);
								m_iAutoEnvelopVolTemp += 5;
							}
						}
						else
						{
							if( (m_iAutoEnvelopVolTemp - 5) < m_iAutoEnvelopVol )
							{
								bAutoEnvelopProcess = FALSE;
								processVolume(m_iOutChannels, (short*)m_pAudioDecodeBuf, uiPCMBufSize, m_iAutoEnvelopVolTemp, m_iAutoEnvelopVol);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) Auto Envelop End(%d %d)", 
									__LINE__, m_pClipItem->getClipID(), m_iAutoEnvelopVol, bAutoEnvelopProcess);								
							}
							else
							{
								processVolume(m_iOutChannels, (short*)m_pAudioDecodeBuf, uiPCMBufSize, m_iAutoEnvelopVolTemp, m_iAutoEnvelopVolTemp - 5);
								m_iAutoEnvelopVolTemp -= 5;
							}
						}
					}
					else
					{
						processVolume(m_iOutChannels, (short*)m_pAudioDecodeBuf, uiPCMBufSize, m_iAutoEnvelopVol, m_iAutoEnvelopVol);
						// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) BGM Volume process(%d)", __LINE__, m_pClipItem->getClipID(), m_iAutoEnvelopVol);
					}
				}

#ifdef FOR_TEST_AUDIO_DEC_DUMP
				if( m_pDecPCMDump != NEXSAL_INVALID_HANDLE )
					nexSAL_FileWrite(m_pDecPCMDump, m_pAudioDecodeBuf, uiPCMBufSize);
#endif
				if (m_isNeedResampling)
				{
					processResampler(&m_pAudioDecodeBuf, uiPCMBufSize);
					m_pAudioPCMBuf = m_pResamplerOutBuffer;
					uiPCMBufSize = m_uResamplerOutputBufSize;
#ifdef FOR_TEST_AUDIO_RESAMPLER_DUMP
					if( m_pResamplerPCMDump != NEXSAL_INVALID_HANDLE )
						nexSAL_FileWrite(m_pResamplerPCMDump, m_pAudioPCMBuf, uiPCMBufSize);
#endif
				}
				else
				{
					m_pAudioPCMBuf = m_pAudioDecodeBuf;
				}
				processNexSound(m_pAudioPCMBuf, uiPCMBufSize, (unsigned int)uiCTS);
				if( m_uiAudioDecoderInitTime != 0 )
				{
					m_uiAudioDecoderInitTime = nexSAL_GetTickCount() - m_uiAudioDecoderInitTime;

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip ID(%d) Audio Frame First output elapsed Time after init(%d)", 
						__LINE__, m_pClipItem->getClipID(), m_uiAudioDecoderInitTime);
					m_uiAudioDecoderInitTime = 0;
				}

#if 0
				if( m_uiRenderPCMDuration > ( uiAudioDuration + 100))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] finished decoding Audio Frame!(PCM dur:%d, Total:%d Start:%d, End:%d) TotalSample:%lld)", __LINE__, m_uiRenderPCMDuration, m_uiClipEndCTS - m_uiClipStartCTS, m_uiClipStartCTS, m_uiClipEndCTS, m_ullRenderPCMSize);
					break;
				}
#else				
				if( (m_uiRenderPCMDuration + m_uiClipBaseTime) > ( m_uiClipEndCTS + 100))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] finished decoding Audio Frame!(PCM dur:%d, BaseTime:%d ClipDur:%d Start:%d, End:%d) TotalSample:%lld)", __LINE__, m_uiRenderPCMDuration, m_uiClipBaseTime, m_uiClipEndCTS - m_uiClipStartCTS, m_uiClipStartCTS, m_uiClipEndCTS, m_ullRenderPCMSize);
					break;
				}
#endif				
				
				if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Addio Task EOS received", __LINE__);
					break;
				}
				
			}
			else
			{
				if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Task end received", __LINE__);
					break;
				}
				
				if( bFrameEnd == TRUE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio is End!!", __LINE__);
					break;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d] Audio Output is empty!!", __LINE__);
				}
			}
			//nexSAL_TaskSleep(1);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Dec Failed(%p %d %lld Ret:%d, bFrameEnd:%d)", __LINE__, pFrame, uiFrameSize, uiCTS, uiDecoderErrRet, bFrameEnd);
			if (FALSE == bFrameEnd)
				CNexProjectManager::sendEvent(MESSAGE_ADEC_DEC_FAIL, CNexProjectManager::getCurrentState(), NEXVIDEOEDITOR_ERROR_CODEC_DECODE, m_pClipItem->getClipID(), 0);
			break;
		}

	}
		else
		{
			if(m_bIsTaskPaused == FALSE)
			{
				m_bIsTaskPaused = TRUE; 
				m_bIsTaskResumed = FALSE;
			}

			if (m_bIsTaskPaused && m_hSema != NEXSAL_INVALID_HANDLE )
			{
				m_isSemaState = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] Audio Task Semaphore Wait!\n", __LINE__);
				nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask %d] Audio Task PauseMode Semaphore is NULL!!!\n", __LINE__);
			}			
		}
	}

	if( m_pAudioTrack )
	{
		int iWaitTime = 0;
		m_pAudioTrack->setDecodingDone(TRUE);
		while ( m_bIsWorking )
		{
			if(m_bIsActive)
			{
				if (m_bIsTaskResumed == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] AudioDecTask RESUMED!!!\n",  __LINE__);
					m_bIsTaskResumed = TRUE;
					m_bIsTaskPaused = FALSE;
				}	

				if( m_pAudioTrack->existAudioTrackData() == FALSE  )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Render End(ID:%d)",__LINE__, m_pClipItem->getClipID());
					break;
				}
				nexSAL_TaskSleep(20);
				iWaitTime  += 20;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Wait Audio Render(ID:%d)(T:%d)",__LINE__, m_pClipItem->getClipID(),iWaitTime);
			}
			else
			{
				if(m_bIsTaskPaused == FALSE)
				{
					m_bIsTaskPaused = TRUE; 
					m_bIsTaskResumed = FALSE;
				}

				if( m_bIsTaskPaused && m_hSema != NEXSAL_INVALID_HANDLE )
				{
					m_isSemaState = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] Audio Task Semaphore Wait!\n", __LINE__);
					nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask %d] Audio Task PauseMode Semaphore is NULL!!!\n", __LINE__);
				}			
			}		
				
		}

		if( m_bIsWorking == FALSE && m_bWaitPCMRender && m_pAudioTrack->existAudioTrackData() )
		{
			int iWaitTime = 1000;
			while( iWaitTime > 0 )
			{
				if(m_bIsActive)
				{
					if (m_bIsTaskResumed == FALSE)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] AudioDecTask RESUMED!!!\n",  __LINE__);
						m_bIsTaskResumed = TRUE;
						m_bIsTaskPaused = FALSE;
					}					
					
					if( m_pAudioTrack->existAudioTrackData() == FALSE  )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Render End(ID:%d)",__LINE__, m_pClipItem->getClipID());
						break;
					}
					nexSAL_TaskSleep(20);
					iWaitTime  -= 20;						
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Wait Audio Render(ID:%d)(T:%d)",__LINE__, m_pClipItem->getClipID(),iWaitTime);
				}
				else
				{
					if(m_bIsTaskPaused == FALSE)
					{
						m_bIsTaskPaused = TRUE; 
						m_bIsTaskResumed = FALSE;
					}

					if( m_bIsTaskPaused && m_hSema != NEXSAL_INVALID_HANDLE )
					{
						m_isSemaState = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] Audio Task Semaphore Wait!\n", __LINE__);
						nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask %d] Audio Task PauseMode Semaphore is NULL!!!\n", __LINE__);
					}					
				}
			}			
		}
	}
	m_bThreadFuncEnd = TRUE;
	deregistTrack();

	deinitResampler();
	deinitSpeedCtl();
	deinitMusicEffect();
	deinitCompressor();
	deinitPitchContorl();
	deinitVoiceChanger();
	deinitEnhancedAudioFilter();
	deinitEqualizer();

#ifdef FOR_TEST_AUDIO_DEC_DUMP
    if( m_pDecPCMDump != NEXSAL_INVALID_HANDLE )
    {
        nexSAL_FileClose(m_pDecPCMDump);
        m_pDecPCMDump = NEXSAL_INVALID_HANDLE;
    }
#endif
#ifdef FOR_TEST_AUDIO_RESAMPLER_DUMP
    if( m_pResamplerPCMDump != NEXSAL_INVALID_HANDLE )
    {
        nexSAL_FileClose(m_pResamplerPCMDump);
        m_pResamplerPCMDump = NEXSAL_INVALID_HANDLE;
    }
#endif
#ifdef FOR_TEST_AUDIO_SPEED_DUMP
    if( m_pSpeedPCMDump != NEXSAL_INVALID_HANDLE )
    {
        nexSAL_FileClose(m_pSpeedPCMDump);
        m_pSpeedPCMDump = NEXSAL_INVALID_HANDLE;
    }
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) Audio Task End",__LINE__, m_pClipItem->getClipID());
    return 0;
}

NXBOOL CNEXThread_AudioTask::setClipItem(CClipItem* pClipItem)
{
	if( pClipItem == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pClipItem);
	pClipItem->AddRef();
	m_pClipItem	= pClipItem;

	m_iTrackID = m_pClipItem->getClipID();
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setClipBaseTime(unsigned int uiBaseTime)
{
	m_uiClipBaseTime = uiBaseTime;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setSource(CNexSource* pSource)
{
	if( pSource == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pSource);
	SAFE_ADDREF(pSource);
	m_pSource = pSource;

	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setFileWriter(CNexExportWriter* pFileWriter)
{
	if( pFileWriter == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pFileWriter);
	SAFE_ADDREF(pFileWriter);
	m_pFileWriter = pFileWriter;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setAudioRenderer(CNEXThread_AudioRenderTask* pAudioRenderer)
{
	if( pAudioRenderer == NULL ) return FALSE;

	SAFE_RELEASE(m_pAudioRenderer);
	SAFE_ADDREF(pAudioRenderer)
	m_pAudioRenderer = pAudioRenderer;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::setStopThumb()
{
	m_isStopThumb = TRUE;
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::initAudioDecoder(unsigned int uiStartTS)
{
	if( m_pSource == NULL )
		return FALSE;

	if( m_pSource->isAudioExist() == FALSE )
		return FALSE;

	unsigned int	uSamplingRate			= 0;
	unsigned int	uNumOfChannels			= 0;
	unsigned int	uNumOfSamplesPerChannel	= 0;

	NXINT64	uiCTS				= 0;
	unsigned int	uFormatTag			= 0;
	unsigned int	uBitsPerSample		= 0;
	unsigned int	uBlockAlign			= 0;
	unsigned int	uAvgBytesPerSec	= 0;
	unsigned int	uSamplesPerBlock	= 0;
	unsigned int	uEnCodeOpt			= 0;
	unsigned int	uExtraSize			= 0;
	unsigned char*	pExtraData		= NULL;

	NXUINT8*		pDSI			= NULL;
	NXUINT8*		pFrame			= NULL;
	NXUINT32		uDSISize		= 0;
	NXUINT32		uFrameSize		= 0;
	unsigned int*		pTempDSI		= NULL;
	int				iRet				= 0;
	unsigned int		uiReaderRet		= 0;
	int nProfile = 0;
	unsigned int uiAudioObjectType =  m_pSource->getAudioObjectType();
	int	iAudioMultiChannelOut = 0;

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		iAudioMultiChannelOut = pEditor->m_iAudioMultiChannelOut;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] AudioMultiChannelOut (%d)", __LINE__, iAudioMultiChannelOut);		
		SAFE_RELEASE(pEditor);
	}

	m_pSource->getWAVEFormat(&uFormatTag, &uBitsPerSample, &uBlockAlign, &uAvgBytesPerSec, &uSamplesPerBlock, &uEnCodeOpt, &uExtraSize, &pExtraData);
	m_pSource->getDSI(NXFF_MEDIA_TYPE_AUDIO, &pDSI, &uDSISize );

	m_pSource->getSamplingRate(&uSamplingRate);
	m_pSource->getNumberOfChannel(&uNumOfChannels);
	m_pSource->getSamplesPerChannel(&uNumOfSamplesPerChannel);

#ifdef __APPLE__ // NESI-129
    // uBitsPerSample of reader is not same as our meaning.
    if(uBitsPerSample == 0)
        uBitsPerSample = 16;
#endif    

	while( 1 )
	{
		uiReaderRet = m_pSource->getAudioFrame();

		if( uiReaderRet == _SRC_GETFRAME_OK )
		{
 			m_pSource->getAudioFrameData(&pFrame, &uFrameSize );
			m_pSource->getAudioFrameCTS(&uiCTS);

#if 0
			if( uFrameSize < 32)
				nexSAL_MemDump(pFrame, uFrameSize);
			else
				nexSAL_MemDump(pFrame, 32);
#endif
			
			/*
			**Nx_robin__110111 HLS????? Reset????? ?????????uiStartTS??? 0???????????? ????? ???????????????????. 
			** Video Only????? A/V??????????????? Init????????????????Audio CTS???, ???? Play????????????????, ????????? ???????????????
			** Video Only???5????????? Play???????A/V??????????????????, Audio ?????????????????? 2??????????, 2~5???????????????????? ???????????????
			*/
			if ( uiStartTS > uiCTS )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Discard Audio Data(%lld), Target(%u)\n", __LINE__, uiCTS, uiStartTS);
				continue;
			}
			else if( uiStartTS+300 < uiCTS ) //[shoh][2011.08.24] While Audio DTS is smaller than the first frame CTS, audio needs mute process.
			{
				// hPlayer->m_uRealStartCTS = uiCTS;
				uiCTS = uiStartTS;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Audio Initialize Start(%lld), Target(%u)\n", __LINE__, uiCTS, uiStartTS);
			}

			if( m_hCodecAudio )
			{
				nexCAL_AudioDecoderDeinit( m_hCodecAudio );
				CNexCodecManager::releaseCodec( m_hCodecAudio );
				m_hCodecAudio = NULL;
			}

			// RYU 20130620 
			if ( (uiAudioObjectType == eNEX_CODEC_A_AAC || uiAudioObjectType == eNEX_CODEC_A_AACPLUS))
			{
				getAACProfile(pFrame, uFrameSize, &nProfile);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] AAC Profile(%d) (%d, 0x%x)\n", __LINE__, nProfile,
					CNexVideoEditor::m_bSupportAACSWCodec,  CNexVideoEditor::m_iSupportAACProfile);
				
				switch( nProfile)
				{
				case 0: // main
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE  && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_MAIN) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Using Software Codec for AAC Main\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 2: // SSR
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_SSR) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Using Software Codec for AAC SSR\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 3: // LTP
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_LTP) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Using Software Codec for AAC LTP\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 1: // LC
					//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] support AAC LC", __LINE__);
				default :
					//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] AAC Unknown ", __LINE__);
					break;
				}
			}

			m_hCodecAudio = CNexCodecManager::getCodec(NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, uiAudioObjectType);

			if( m_hCodecAudio == NULL )
			{
				return FALSE;
			}

			m_uiAudioObjectType = uiAudioObjectType;

			if ( (m_pSource->getAudioObjectType() == eNEX_CODEC_A_AAC || m_pSource->getAudioObjectType() == eNEX_CODEC_A_AACPLUS) && pDSI != NULL )
			{
				unsigned int    uSamplingFreqIndex = 0;
				const unsigned int puAACSampleRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};

				NXINT32 isSBR;
				NexCodecUtil_AAC_ParseDSI(pDSI, uDSISize, &uSamplingFreqIndex, &uNumOfSamplesPerChannel, &isSBR);
				if (isSBR)
				{
					m_iAACSbr = isSBR;
				}

				uSamplingFreqIndex = ((pDSI[0] & 0x07) << 1) | ((pDSI[1] & 0x80) >> 7);

				if ( uSamplingFreqIndex > 11 )//Nx_robin__090827 uSamplingFreqIndexê°?ì²´í¬
					uSamplingFreqIndex = 0;
				uSamplingRate = puAACSampleRates[uSamplingFreqIndex];
				uNumOfChannels = (pDSI[1] & 0x78) >> 3;
				if ( uNumOfChannels == 0 )//Nx_robin__090827 DSI????? uNumOfChannels??? 0????? ????????????? File MetaData????? ??????????????.
					m_pSource->getNumberOfChannel(&uNumOfChannels);
				uBitsPerSample = 16;

				uiAudioObjectType = NexCodecUtil_GetAACType((NEX_CODEC_TYPE)m_pSource->getAudioObjectType(), uSamplingRate, uNumOfChannels, pFrame, uFrameSize);							

				if(uiAudioObjectType == eNEX_CODEC_A_AACPLUSV2)
					m_pSource->setAudioObjectType(eNEX_CODEC_A_AACPLUS);

				if ( m_pSource->getAudioObjectType()== eNEX_CODEC_A_AACPLUS )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] This contents is AAC+\n", __LINE__);
					uNumOfSamplesPerChannel = 2048;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] This contents is AAC\n", __LINE__);
					uNumOfSamplesPerChannel = 1024;
				}
			}
			break;
		}
		else if( uiReaderRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d]Need Buffering....\n", __LINE__);
			nexSAL_TaskSleep(20);
			continue;
		}
		// JDKIM 2011/02/03 : Lost Frame??????? ?????? OK??? ?????????? ????????
		else if( uiReaderRet == _SRC_GETFRAME_OK_LOSTFRAME)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Remove Lost Frame..\n", __LINE__);
			continue;
		}
		
		/*
		else if( uiReaderRet == _SRC_GETFRAME_END)
		{
			NXBOOL bRepeat = isRepeat(uiCTS == 0 ? uiStartTS + 500 : uiCTS + 500);
			if( bRepeat )
			{
				if( m_pSource->setRepeatAudioClip(uiCTS, m_pClipItem->getStartTrimTime()) )
				{
					m_isNextFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Clip Repeat Start", __LINE__);
					continue;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Clip Repeat failed", __LINE__);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] getAudioFrame End", __LINE__);
			goto DECODER_INIT_ERROR;
		}
		*/
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Data Read Error[0x%X].\n", __LINE__, uiReaderRet);
			goto DECODER_INIT_ERROR;
		}
	}


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] nexCAL_AudioDecoderInit Frame Data!",  __LINE__);
	nexSAL_MemDump(pDSI, uDSISize);
	//nexSAL_MemDump(pFrame, uFrameSize);


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Decoder Init Start(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	MPEG_AUDIO_INFO mpa_info;
	memset(&mpa_info, 0x00, sizeof(MPEG_AUDIO_INFO));
	NexCodecUtil_Get_MPEG_Audio_Info( pFrame, uFrameSize, &mpa_info );
	if (mpa_info.Version == NEX_MPEG_AUDIO_VER_1 && mpa_info.Layer == NEX_MPEG_AUDIO_LAYER_2)
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 1);
	else
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 0);

	iRet = nexCAL_AudioDecoderInit(		m_hCodecAudio,
										(NEX_CODEC_TYPE)m_pSource->getAudioObjectType(), 
										pDSI, 
										uDSISize, 
										pFrame, 
										uFrameSize, 
										NULL,
										NULL,
										&uSamplingRate,
										&uNumOfChannels, 
										&uBitsPerSample, 
										&uNumOfSamplesPerChannel, 
										NEXCAL_ADEC_MODE_NONE,
										0, // USERDATATYPE
										(NXVOID *)this );

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Decoder Init End(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	m_uiAudioDecoderInitTime = nexSAL_GetTickCount();

	if(m_pAudioDecodeBuf != NULL)
	{
		nexSAL_MemFree(m_pAudioDecodeBuf);
		m_pAudioDecodeBuf = NULL;
	}

	m_uiAudioDecodeBufSize = 384*1024;
	m_pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioDecodeBufSize);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Default  PCMBuffer(%p, %d)\n", __LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
	
	if( iRet != NEXCAL_ERROR_NONE )
	{
		if ( pTempDSI )
		{
			nexSAL_MemFree(pTempDSI);
			pTempDSI = NULL;
			pDSI = NULL;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Decoder Init Failed(%d)",  __LINE__, iRet);
		goto DECODER_INIT_ERROR;
	}
	if ( pTempDSI )
	{
		nexSAL_MemFree(pTempDSI);
		pTempDSI = NULL;
		pDSI = NULL;
	}

	m_iSampleRate		= uSamplingRate;
	m_iChannels			= uNumOfChannels;

	if(iAudioMultiChannelOut)
	{
		if(m_iChannels == 6)
			m_iOutChannels = 6;
		else if(m_iChannels > 2)
			m_iOutChannels = 2;
		else
			m_iOutChannels = m_iChannels;
	}
	else
	{
		if(m_iChannels > 2)
			m_iOutChannels = 2;
		else
			m_iOutChannels = m_iChannels;
	}
	m_iBitsForSample		= uBitsPerSample;
	m_iSampleForChannel	= uNumOfSamplesPerChannel;

	m_pSource->setNumOfChannels(m_iChannels);
	m_pSource->setSamplingRate(m_iSampleRate);
	m_pSource->setNumOfSamplesPerChannel(m_iSampleForChannel);

	m_isNextFrameRead = FALSE;

	if( m_pThumbnail )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] PCM detector mode",  __LINE__);
		return TRUE;
	}

	// Case : mantis 9267 when decoder channel info was wrong.
	if( m_iSampleRate == 0 || m_iChannels == 0 || m_iBitsForSample == 0 || m_iSampleForChannel == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Wait decode config change",  __LINE__);
		return TRUE;
	}

	m_pAudioTrack = new CAudioTrackInfo;
	if( m_pAudioTrack == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Track Create failed(Track %p)",  __LINE__, m_pAudioTrack);
		goto DECODER_INIT_ERROR;
	}

	if (m_iOutChannels) initResampler();

	m_pAudioTrack->setAudioTrackInfo(	m_iTrackID, 
									m_iExportSamplingRate,
									m_iOutChannels, 
									m_iBitsForSample, 
									1024, 
									(unsigned int) (m_uiClipBaseTime + m_pClipItem->getStartTime()), 
									(unsigned int) (m_uiClipBaseTime + m_pClipItem->getEndTime()), 
									(unsigned int)m_pClipItem->getAudioVolume(),
									(int*) m_pClipItem->getPanFactor());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] m_ClipAudioRenderInfoVec.size (%d, %zu)\n", __LINE__,  m_pClipItem->getClipID(),m_pClipItem->m_ClipAudioRenderInfoVec.size());
	if(m_pClipItem->m_ClipAudioRenderInfoVec.size())
	{
		for(int i=0; i<m_pClipItem->m_ClipAudioRenderInfoVec.size(); i++)
		{
			m_pAudioTrack->setClipRenderInfoVec(m_pClipItem->m_ClipAudioRenderInfoVec[i]);
		}
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Clip baseTime(%d) Start(%d) End(%d) Trim(%d %d)",  __LINE__, 
		m_uiClipBaseTime, m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), m_pClipItem->getStartTrimTime(), m_pClipItem->getEndTrimTime());
	if( registTrack() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Track regist failed (Track %p)",  __LINE__, m_pAudioTrack);
		goto DECODER_INIT_ERROR;
	}
	// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_FUNC, (unsigned int)_GetCurrentMediaCTS );
	// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_USERDATA, (unsigned int)this );
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Track regist End (Track %p), %d",  __LINE__, m_pAudioTrack, m_uiOutputSampleForChannel?m_uiOutputSampleForChannel:m_iSampleForChannel);
	return TRUE;

DECODER_INIT_ERROR:
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Decoder failed(Track %p)",  __LINE__, m_pAudioTrack);
	if( m_hCodecAudio )
	{
		CNexCodecManager::releaseCodec(m_hCodecAudio);
		m_hCodecAudio = NULL;
	}
	if ( pTempDSI )
	{
		nexSAL_MemFree(pTempDSI);
		pTempDSI = NULL;
		pDSI = NULL;
	}

	SAFE_RELEASE(m_pAudioTrack);
	// Event FILE INVALID ERROR
	// Error Report
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::reinitAudioTrack(unsigned int  uiSamplingRate, unsigned int uiChannels, unsigned int uiBitsPerSample, unsigned int uiNumOfSamplesPerChannel)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d]reinitAudioTrack START!(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uiSamplingRate, uiChannels, uiBitsPerSample, uiNumOfSamplesPerChannel);

	unsigned int bNeedReinitAudioTrack = FALSE;
	unsigned int uiAudioDecodeBufsize = 0;
	int iAudioMultiChannelOut = 0;
	// for mantis 8851(sampleforchannel is 0 problem)
	// if( m_iSampleRate != uiSamplingRate || m_iChannels != uiChannels || m_iSampleForChannel == 0 || m_iSampleForChannel < uiNumOfSamplesPerChannel )
	if( m_iSampleRate != uiSamplingRate || m_iChannels != uiChannels || m_iSampleForChannel == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d]update AudioTrack!(%d, %d)(%d, %d), (%d,%d)",__LINE__,  m_iSampleRate, uiSamplingRate, m_iChannels, uiChannels, m_iSampleForChannel, uiNumOfSamplesPerChannel);
		bNeedReinitAudioTrack = TRUE;
	}

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		iAudioMultiChannelOut = pEditor->m_iAudioMultiChannelOut;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] AudioMultiChannelOut (%d)", __LINE__, iAudioMultiChannelOut);
		SAFE_RELEASE(pEditor);
	}

	m_iSampleRate		= uiSamplingRate;
	m_iChannels			= uiChannels;
	
	if(iAudioMultiChannelOut)
	{
		if(m_iChannels == 6)
			m_iOutChannels = 6;
		else if(m_iChannels > 2)
			m_iOutChannels = 2;
		else
			m_iOutChannels = m_iChannels;			
	}
	else
	{
		if(m_iChannels > 2)
			m_iOutChannels = 2;
		else
			m_iOutChannels = m_iChannels;	
	}
	
	m_iBitsForSample		= uiBitsPerSample;
	m_iSampleForChannel	= uiNumOfSamplesPerChannel;

	deinitResampler();
	deinitSpeedCtl();
	deinitMusicEffect();
	deinitCompressor();
	deinitPitchContorl();
	deinitVoiceChanger();
	deinitEnhancedAudioFilter();
	deinitEqualizer();
	initResampler();
	initSpeedCtl();
	initMusicEffect();
	initCompressor();
	initPitchContorl();
	initVoiceChanger();
	initEnhancedAudioFilter();
	initEqualizer();

	m_pSource->setNumOfChannels(m_iChannels);
	m_pSource->setSamplingRate(m_iSampleRate);
	m_pSource->setNumOfSamplesPerChannel(m_iSampleForChannel);

	uiAudioDecodeBufsize = (uiNumOfSamplesPerChannel)*uiChannels*(uiBitsPerSample>>3);
	
	// KM-2384 need double buffer since calbody still use wrong info. (2 channel)
	if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
	{	
		if(CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec == FALSE && m_iChannels == 1)
		{
			uiAudioDecodeBufsize *= 2;
		}	
	}	
	
	if( uiAudioDecodeBufsize > m_uiAudioDecodeBufSize)
	{
		if(m_pAudioDecodeBuf)
		{
			nexSAL_MemFree(m_pAudioDecodeBuf);
			m_pAudioDecodeBuf = NULL;
		}

		m_uiAudioDecodeBufSize = uiAudioDecodeBufsize;
		m_pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioDecodeBufSize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] reinit PCMBuffer(%p, %d)\n", __LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);

		if( m_pAudioDecodeBuf == NULL)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] reinit malloc fail!\n", __LINE__);
			goto AUDIOTRACK_REINIT_ERROR;
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Don't need to update PCMBuffer(%p, %d)", __LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
	}

	if( m_pThumbnail )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] PCM detector mode",  __LINE__);
		return TRUE;
	}

	if(bNeedReinitAudioTrack == TRUE)
	{
		deregistTrack();
		SAFE_RELEASE(m_pAudioTrack);
		m_pAudioTrack = new CAudioTrackInfo;
		
		if( m_pAudioTrack == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Track Create failed(Track %p)",  __LINE__, m_pAudioTrack);
			goto AUDIOTRACK_REINIT_ERROR;
		}

		m_pAudioTrack->setAudioTrackInfo(	m_iTrackID, 
										m_iExportSamplingRate, 
										m_iOutChannels, 
										m_iBitsForSample, 
										1024, 
										(unsigned int) (m_uiClipBaseTime + m_pClipItem->getStartTime()), 
										(unsigned int) (m_uiClipBaseTime + m_pClipItem->getEndTime()), 
										(unsigned int)m_pClipItem->getAudioVolume(),
										(int*) m_pClipItem->getPanFactor());

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] m_ClipAudioRenderInfoVec.size (%d, %zu)\n", __LINE__,  m_pClipItem->getClipID(),m_pClipItem->m_ClipAudioRenderInfoVec.size());
		if(m_pClipItem->m_ClipAudioRenderInfoVec.size())
		{
			for(int i=0; i<m_pClipItem->m_ClipAudioRenderInfoVec.size(); i++)
			{
				m_pAudioTrack->setClipRenderInfoVec(m_pClipItem->m_ClipAudioRenderInfoVec[i]);
			}
		}
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] Clip basTime(%d) Start(%d) End(%d)",  __LINE__, m_uiClipBaseTime, m_pClipItem->getStartTime(), m_pClipItem->getEndTime());
		if( registTrack() == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Track regist failed (Track %p)",  __LINE__, m_pAudioTrack);
			goto AUDIOTRACK_REINIT_ERROR;

		}
		// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_FUNC, (unsigned int)_GetCurrentMediaCTS );
		// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_USERDATA, (unsigned int)this );
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Track regist End (Track %p)",  __LINE__, m_pAudioTrack);

	return TRUE;
AUDIOTRACK_REINIT_ERROR:
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] Audio Track reinit failed(Track %p)",  __LINE__, m_pAudioTrack);
	SAFE_RELEASE(m_pAudioTrack);
	// Event FILE INVALID ERROR
	// Error Report
	return FALSE;
	
}


NXBOOL CNEXThread_AudioTask::deinitAudioDecoder()
{
	deregistTrack();

	SAFE_RELEASE(m_pAudioTrack);

	if( m_hCodecAudio )
	{
		nexCAL_AudioDecoderDeinit( m_hCodecAudio );
		CNexCodecManager::releaseCodec(m_hCodecAudio);
		m_hCodecAudio = NULL;
	}

	return TRUE;
}

NXBOOL CNEXThread_AudioTask::registTrack()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] registAudio Track(render: %p, Track: %p)",  __LINE__, m_pAudioRenderer, m_pAudioTrack);
	if( m_pAudioRenderer && m_pAudioTrack )
	{
		CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pAudioTrack);
		if( pMsg )
		{
			m_pAudioRenderer->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] registAudio Track(id(%d) render: %p, Track: %p) Out",  __LINE__, m_pAudioTrack->m_uiTrackID, m_pAudioRenderer, m_pAudioTrack);
			return TRUE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] registAudio Track(render: %p, Track: %p) FailOut",  __LINE__, m_pAudioRenderer, m_pAudioTrack);
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::deregistTrack()
{
	if( m_pAudioRenderer && m_pAudioTrack )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deregist Audio Track RefCnt(%d)",  __LINE__, m_pAudioTrack->m_uiTrackID, m_pAudioTrack->GetRefCnt());
		CNxMsgDeleteTrack* pMsg = new CNxMsgDeleteTrack(m_pAudioTrack);
		if( pMsg )
		{
			m_pAudioRenderer->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] deregist Audio Track(id(%d) render: %p, Track: %p) Out",  __LINE__, m_pAudioTrack->m_uiTrackID, m_pAudioRenderer, m_pAudioTrack);
			return TRUE;
		}
	}
	SAFE_RELEASE(m_pAudioTrack);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] deregist Audio Track(render: %p, Track: %p) Fail Out",  __LINE__, m_pAudioRenderer, m_pAudioTrack);
	return FALSE;
}

#define __abs(x)	x<0?-x:x

// for JIRA 3195 issue
void CNEXThread_AudioTask::initAutoEnvelopVolume(unsigned int uiCTS)
{
	CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
	if( pProjectMng )
	{
		CClipList* pList = pProjectMng ->getClipList();
		if( pList )
		{
			int iBGMVol = pList->getBGMVolumeOfVisualClipAtCurrentTime(uiCTS);
			m_iAutoEnvelopVolTemp = m_iAutoEnvelopVol = iBGMVol;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  ID(%d) getAutoEnvelopVolume(%d)",  __LINE__, m_pClipItem->getClipID(), iBGMVol);
			SAFE_RELEASE(pList);
		}
		SAFE_RELEASE(pProjectMng);
	}
}

NXBOOL CNEXThread_AudioTask::checkAutoEnvelopProcess(unsigned int uiCTS)
{
	NXBOOL bFlag = FALSE;
	CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
	if( pProjectMng )
	{
		CClipList* pList = pProjectMng ->getClipList();
		if( pList )
		{
			int iBGMVol = pList->getBGMVolumeOfVisualClipAtCurrentTime(uiCTS);
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  ID(%d) checkAutoEnvelopProcess(%d %d)",  __LINE__, m_pClipItem->getClipID(), m_iAutoEnvelopVol, iBGMVol);
			if( iBGMVol != m_iAutoEnvelopVol )
			{
				m_iAutoEnvelopVolTemp 	= m_iAutoEnvelopVol;
				m_iAutoEnvelopVol			= iBGMVol;
				bFlag = TRUE;
			}
			SAFE_RELEASE(pList);
		}
		SAFE_RELEASE(pProjectMng);
	}
	return bFlag;
}

void CNEXThread_AudioTask::processEnvelop(short* pPCM, int iPCMCount, unsigned int uiCTS)
{
#ifdef FOR_TEST_AUDIOENVELOP
	if( m_iAudioEnvelopSize > 0 )
	{
		if( m_bUpdateAudioEnvelop == TRUE )
		{
			if( m_iAudioEnvelopSize == 1 )
			{
				m_pClipItem->getAudioEnvelop( 0, &m_iAudioEnvelopStartCTS, &m_iAudioEnvelopStartVolume );
				m_pClipItem->getAudioEnvelop( 0, &m_iAudioEnvelopEndCTS, &m_iAudioEnvelopEndVolume );
			}
			else
			{
				if( m_iAudioEnvelopIndex == 1 )
				{
					// skip previous frame.
					for( int nStartEnvelopIndex = 1 ; nStartEnvelopIndex < m_iAudioEnvelopSize ; nStartEnvelopIndex++)
					{
						m_pClipItem->getAudioEnvelop( nStartEnvelopIndex-1, &m_iAudioEnvelopStartCTS, &m_iAudioEnvelopStartVolume );
						m_pClipItem->getAudioEnvelop( nStartEnvelopIndex, &m_iAudioEnvelopEndCTS, &m_iAudioEnvelopEndVolume );

						m_iAudioEnvelopStartCTS += m_uiClipStartCTS;
						m_iAudioEnvelopEndCTS += m_uiClipStartCTS;
						// m_iAudioEnvelopEndCTS += m_uiClipEndCTS;
						
						if( uiCTS >= m_iAudioEnvelopStartCTS && uiCTS <= m_iAudioEnvelopEndCTS)
						{
							m_iAudioEnvelopIndex = nStartEnvelopIndex;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) set FirstEnvelop Position(%d, %d, %d, %d)", __LINE__,
								m_pClipItem->getClipID(), m_iAudioEnvelopIndex, m_iAudioEnvelopStartCTS, uiCTS, m_iAudioEnvelopEndCTS);
							break;
						}
					}
				}

				m_pClipItem->getAudioEnvelop(m_iAudioEnvelopIndex-1, &m_iAudioEnvelopStartCTS, &m_iAudioEnvelopStartVolume);
				m_pClipItem->getAudioEnvelop(m_iAudioEnvelopIndex, &m_iAudioEnvelopEndCTS, &m_iAudioEnvelopEndVolume);
				m_iAudioEnvelopIndex++;
			}

			// RYU 20130827 : m_uiClipBaseTime change to m_uiClipStartCTS for considering the trim time.
			m_iAudioEnvelopStartCTS += m_uiClipStartCTS;
			m_iAudioEnvelopEndCTS += m_uiClipStartCTS;
			m_bUpdateAudioEnvelop = FALSE;
			m_iAudioEnvelopPrevVolume = m_iAudioEnvelopStartVolume;

		}

		m_uiVolume = calcVolume(uiCTS, m_iAudioEnvelopStartCTS, m_iAudioEnvelopEndCTS, m_iAudioEnvelopStartVolume, m_iAudioEnvelopEndVolume);
		processVolume(m_iOutChannels, (short*)pPCM, iPCMCount, m_iAudioEnvelopPrevVolume, m_uiVolume);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ClipID(%d) BufSize(%d) AudioEnvelop Process(S:%d, E:%d)(%d, %d, %d)", __LINE__,
		// m_pClipItem->getClipID(), iPCMCount, m_iAudioEnvelopPrevVolume, m_uiVolume, m_iAudioEnvelopStartCTS, uiCTS, m_iAudioEnvelopEndCTS);

		m_iAudioEnvelopPrevVolume = m_uiVolume;
		if( uiCTS >= m_iAudioEnvelopEndCTS)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 2, "[ADTask.cpp %d]  Audio Clip(%d)  Need Update AudioEnvelop", __LINE__,m_pClipItem->getClipID());
			m_bUpdateAudioEnvelop = TRUE;
		}
	}
#endif
}

void CNEXThread_AudioTask::processEnvelopForRenderInfo(short* pPCM, int iPCMCount, unsigned int uiCTS)
{
#ifdef FOR_TEST_AUDIOENVELOP
	if( m_iAudioEnvelopSize > 0 )
	{
		if( m_bUpdateAudioEnvelop == TRUE )
		{
			if( m_iAudioEnvelopSize == 1 )
			{
				m_iAudioEnvelopStartCTS = m_iAudioEnvelopEndCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[0];
				m_iAudioEnvelopStartVolume = m_iAudioEnvelopEndVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[0];
			}
			else
			{
				if( m_iAudioEnvelopIndex == 1 )
				{
					// skip previous frame.
					for( int nStartEnvelopIndex = 1 ; nStartEnvelopIndex < m_iAudioEnvelopSize ; nStartEnvelopIndex++)
					{
						m_iAudioEnvelopStartCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[nStartEnvelopIndex-1];		
						m_iAudioEnvelopStartVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[nStartEnvelopIndex-1];
						m_iAudioEnvelopEndCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[nStartEnvelopIndex];		
						m_iAudioEnvelopEndVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[nStartEnvelopIndex];
						m_iAudioEnvelopStartCTS += m_pCurrentRenderInfo->mStartTime;
						m_iAudioEnvelopEndCTS += m_pCurrentRenderInfo->mStartTime;
						
						if( uiCTS >= m_iAudioEnvelopStartCTS && uiCTS <= m_iAudioEnvelopEndCTS)
						{
							m_iAudioEnvelopIndex = nStartEnvelopIndex;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Clip(%d) set FirstEnvelop Position(%d, %d, %d, %d)", __LINE__,
								m_pClipItem->getClipID(), m_iAudioEnvelopIndex, m_iAudioEnvelopStartCTS, uiCTS, m_iAudioEnvelopEndCTS);
							break;
						}
					}
				}

				if(m_iAudioEnvelopIndex >= m_iAudioEnvelopSize)
				{
						m_iAudioEnvelopStartCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[m_iAudioEnvelopSize-1];
						m_iAudioEnvelopStartVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[m_iAudioEnvelopSize-1];
						m_iAudioEnvelopEndCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[m_iAudioEnvelopSize - 1];
						m_iAudioEnvelopEndVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[m_iAudioEnvelopSize - 1];
				}
				else
				{
						m_iAudioEnvelopStartCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[m_iAudioEnvelopIndex-1];
						m_iAudioEnvelopStartVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[m_iAudioEnvelopIndex-1];
						m_iAudioEnvelopEndCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[m_iAudioEnvelopIndex];
						m_iAudioEnvelopEndVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[m_iAudioEnvelopIndex];
				}
				m_iAudioEnvelopIndex++;
			}

			// RYU 20130827 : m_uiClipBaseTime change to m_uiClipStartCTS for considering the trim time.
			m_iAudioEnvelopStartCTS += m_pCurrentRenderInfo->mStartTime;
			m_iAudioEnvelopEndCTS += m_pCurrentRenderInfo->mStartTime;
			m_bUpdateAudioEnvelop = FALSE;
			m_iAudioEnvelopPrevVolume = m_iAudioEnvelopStartVolume;

		}

		m_uiVolume = calcVolume(uiCTS, m_iAudioEnvelopStartCTS, m_iAudioEnvelopEndCTS, m_iAudioEnvelopStartVolume, m_iAudioEnvelopEndVolume);
		processVolume(m_iOutChannels, (short*)pPCM, iPCMCount, m_iAudioEnvelopPrevVolume, m_uiVolume);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ClipID(%d) BufSize(%d) AudioEnvelop Process(S:%d, E:%d)(%d, %d, %d)", __LINE__,
		// m_pClipItem->getClipID(), iPCMCount, m_iAudioEnvelopPrevVolume, m_uiVolume, m_iAudioEnvelopStartCTS, uiCTS, m_iAudioEnvelopEndCTS);

		m_iAudioEnvelopPrevVolume = m_uiVolume;
		if( uiCTS >= m_iAudioEnvelopEndCTS)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 2, "[ADTask.cpp %d]  Audio Clip(%d)  Need Update AudioEnvelop", __LINE__,m_pClipItem->getClipID());
			m_bUpdateAudioEnvelop = TRUE;
		}
	}
#endif
}

NXBOOL CNEXThread_AudioTask::getResamplerOutBufferSize(unsigned char* pInBuf, unsigned int uInBufSize, unsigned int* pOutSize)
{
	NXBOOL bRet = FALSE;
	*pOutSize = 0;

	if (uInBufSize > 0)
	{
		unsigned int uInputSamplesPerChannel, uLastSample, uOutBufSize, uOutSize, uTotalOutSize = 0;
		unsigned char *pInputBuffer, *pOutputBuffer;
		unsigned char *pInputBuffer1, *pOutputBuffer1;

		// to get the output SampleForChannel value after resampling
		if (pInBuf)
		{
			pInputBuffer = pInputBuffer1 = pInBuf;
			uInputSamplesPerChannel = uInBufSize / (m_iBitsForSample >> 3) /  m_iOutChannels;
		}
		// to get the output resampling buffer size in the first time, send dummy data to Resampler lib.
		else
		{
			int iInputBufferSize = uInBufSize*(m_iBitsForSample >> 3)*m_iOutChannels+64;	// 64 is dummy
			pInputBuffer = pInputBuffer1 = (unsigned char*)nexSAL_MemAlloc(iInputBufferSize);
			memset(pInputBuffer, 0x00, iInputBufferSize);
			uInputSamplesPerChannel = uInBufSize;
		}

		if (m_iExportSamplingRate > m_iSampleRate)
		{
			uOutBufSize = ceil((float)m_iExportSamplingRate / (float)m_iSampleRate);
			uOutBufSize *= (uInputSamplesPerChannel*m_iOutChannels*(m_iBitsForSample>>3));
		}
		else
		{
			uOutBufSize = m_iSampleRate / m_iExportSamplingRate;
			uOutBufSize = (uInputSamplesPerChannel*m_iOutChannels*(m_iBitsForSample>>3) / uOutBufSize);
		}
		uOutBufSize += 64;	// 64 is dummy

		pOutputBuffer = pOutputBuffer1 = (unsigned char*)nexSAL_MemAlloc(uOutBufSize);

		while (uInputSamplesPerChannel > 0)
		{
			uLastSample = (uInputSamplesPerChannel > EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE)?EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE:uInputSamplesPerChannel;

			m_hNexResamplerHandle->NexSoundProcess(Processor::Resampler, (Int16*)pInputBuffer, (Int16*)pOutputBuffer, uLastSample);
			m_hNexResamplerHandle->NexSoundGetParam(Processor::Resampler, ParamCommand::Resampler_Output_NumberOfSamples, &uOutSize);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ADTask.cpp %d] getResamplerOutBufferSize pOutputBuffer:%p, uOutSize:%u", __LINE__, pOutputBuffer, uOutSize);
			uInputSamplesPerChannel -= uLastSample;
			pInputBuffer += (uLastSample*m_iOutChannels*sizeof(short));
			pOutputBuffer += (uOutSize*m_iOutChannels*sizeof(short));
			uTotalOutSize += uOutSize;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] uTotalOutSize(%d) buffer Size(In:%d->Out:%d)", __LINE__, uTotalOutSize, uInBufSize, uTotalOutSize*m_iOutChannels*sizeof(short));

		// reallocte output buffer
		if (pInBuf)
		{
			m_uResamplerOutputBufSize = (uTotalOutSize*m_iOutChannels*sizeof(short));
			m_pResamplerOutBuffer = (unsigned char*)nexSAL_MemAlloc(m_uResamplerOutputBufSize + 64);	//64 is dummy
			memcpy(m_pResamplerOutBuffer, pOutputBuffer1, m_uResamplerOutputBufSize);
		}
		else
			nexSAL_MemFree(pInputBuffer1);
		nexSAL_MemFree(pOutputBuffer1);

		*pOutSize = uTotalOutSize;
		bRet = TRUE;
	}

	return bRet;
}

NXBOOL CNEXThread_AudioTask::initResampler()
{
	NXBOOL bRet = TRUE;

	if( m_iSampleRate != m_iExportSamplingRate )
	{
		m_isNeedResampling = TRUE;

		if(m_hNexResamplerHandle)
		{
			m_hNexResamplerHandle->NexSoundRelease(Processor::Resampler);
			DestroyNexSound(m_hNexResamplerHandle);
			m_hNexResamplerHandle	= NULL;
		}

		m_hNexResamplerHandle = CreateNexSound();
		if( m_hNexResamplerHandle == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[AudioTrack %d] initResampler CreateNexSound failed",__LINE__);
			bRet = FALSE;
			goto initResampler_Err;
		}

		if(NEXSOUND_SUCCESS != m_hNexResamplerHandle->NexSoundInitialize(Processor::Resampler))
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[AudioTrack %d] initResampler NexSoundInitialize failed",__LINE__);
			bRet = FALSE;
			goto initResampler_Err;
		}

		m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Input_SamplingRate, m_iSampleRate);
		m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Input_Channels, m_iOutChannels);
		m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_BitPerSample, m_iBitsForSample);
		m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Input_NumberOfSamples, EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE);
		m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Output_SamplingRate, m_iExportSamplingRate);
		m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Quality_Level, NEX_RESAMPLER_DEF_QUALITY);

		getResamplerOutBufferSize(NULL, m_iSampleForChannel, &m_uiOutputSampleForChannel);
	}

initResampler_Err:
	if(FALSE == bRet)
	{
		deinitResampler();
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] initResampler m_hNexResamplerHandle(%p) SampleForChannel(%d->%d )", __LINE__, m_hNexResamplerHandle, m_iSampleForChannel, m_uiOutputSampleForChannel );    
	return bRet;
}

NXBOOL CNEXThread_AudioTask::deinitResampler()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] deinitResampler In(b:%p, h:%p )", __LINE__, m_pResamplerOutBuffer, m_hNexResamplerHandle );

	m_isNeedResampling = FALSE;	
	m_uLastInputPCMSize = 0;

	 if (m_pResamplerOutBuffer)
	{
		m_uResamplerOutputBufSize	 = 0;
		nexSAL_MemFree(m_pResamplerOutBuffer);
		m_pResamplerOutBuffer = NULL;
	}

	if(m_hNexResamplerHandle)
	{
		m_hNexResamplerHandle->NexSoundRelease(Processor::Resampler);
		DestroyNexSound(m_hNexResamplerHandle);
		m_hNexResamplerHandle = NULL;
	}

	m_uiOutputSampleForChannel = 0;
 	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] deinitResampler Out(b:%p, h:%p )", __LINE__, m_pResamplerOutBuffer, m_hNexResamplerHandle );
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::processResampler( unsigned char** ppInPCM, unsigned int uiPCMSize)
{
	unsigned int uTime = nexSAL_GetTickCount();

	if( m_hNexResamplerHandle )
	{
		unsigned int uInputSamplesPerChannel = uiPCMSize / (m_iBitsForSample >> 3) /  m_iOutChannels;
		unsigned int uLastSample, uOutputSize = 0, uTotalOutputBufferSize = 0;
		unsigned char* pInputBuffer = *ppInPCM;
		unsigned char* pOutputBuffer = m_pResamplerOutBuffer;

		// can get output buffer size after calling NexSoundProcess fuction at least once.
		if (m_pResamplerOutBuffer == NULL || uiPCMSize > m_uLastInputPCMSize)
		{
			unsigned int uSize;

			if (m_pResamplerOutBuffer)
			{
				nexSAL_MemFree(m_pResamplerOutBuffer);
				m_pResamplerOutBuffer = NULL;
			}

			getResamplerOutBufferSize(*ppInPCM, uiPCMSize, &uTotalOutputBufferSize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] processResampler  uiPCMSize:%u(last:%d), ResamplerOutBuffer(%p) ResamplerOutBufferSize=%u", __LINE__, uiPCMSize, m_uLastInputPCMSize, m_pResamplerOutBuffer, m_uResamplerOutputBufSize);
		}
		else	 // works after getting output buffer size
		{
			while (uInputSamplesPerChannel > 0)
			{
				uLastSample = (uInputSamplesPerChannel > EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE)?EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE:uInputSamplesPerChannel;

				m_hNexResamplerHandle->NexSoundProcess(Processor::Resampler, (Int16*)pInputBuffer, (Int16*)pOutputBuffer, uLastSample);
				m_hNexResamplerHandle->NexSoundGetParam(Processor::Resampler, ParamCommand::Resampler_Output_NumberOfSamples, &uOutputSize);
				//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] processResampler pOutputBuffer:%p, uOutputSize:%u", __LINE__, pOutputBuffer, uOutputSize);
				uInputSamplesPerChannel -= uLastSample;
				pInputBuffer += (uLastSample*m_iOutChannels*sizeof(short));
				uTotalOutputBufferSize += uOutputSize;
				pOutputBuffer += (uOutputSize*m_iOutChannels*sizeof(short));
			}

			m_uResamplerOutputBufSize = uTotalOutputBufferSize*m_iOutChannels*sizeof(short);
		}
	}

	if (uiPCMSize > m_uLastInputPCMSize)
		m_uLastInputPCMSize = uiPCMSize;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ADTask.cpp %d] processResampler isNeedResampling:%d,  uiPCMSize:%u(last:%u),  m_uResamplerOutputBufSize=%u", __LINE__, m_isNeedResampling, uiPCMSize, m_uLastInputPCMSize, m_uResamplerOutputBufSize);
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::initSpeedCtl()
{
	if(m_pCurrentRenderInfo)
	{
		m_iSpeedFactor = m_pCurrentRenderInfo->m_iSpeedCtlFactor;
		m_bKeepPitch = m_pCurrentRenderInfo->m_bKeepPitch;
	}

	if( m_pThumbnail != NULL  || m_iSpeedFactor == 100 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound Speed control did not use",__LINE__);
		return TRUE;
	}

#ifdef _ANDROID // No NEXResampler library for iOS
	if(m_uiLevelForSpeed > m_iSpeedFactor)
	{
		m_hResamplerForSpeed = NxResamplerOpen();

		if( m_hResamplerForSpeed == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] resampler open fail", __LINE__);
			return FALSE;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] create resampler(%p)", __LINE__, m_hResamplerForSpeed);
		m_hResamplerForSpeed->nInSampleRate		= EDITOR_DEFAULT_SAMPLEFORCHANNEL;
		m_hResamplerForSpeed->nInChannels		= m_iOutChannels;
		m_hResamplerForSpeed->nInput				= m_iOutChannels * EDITOR_DEFAULT_SAMPLEFORCHANNEL;

		if(m_iSpeedFactor == 3) //yoon
			m_hResamplerForSpeed->nOutSampleRate	= EDITOR_DEFAULT_SAMPLEFORCHANNEL*32; //yoon
		else if(m_iSpeedFactor == 6) //yoon
			m_hResamplerForSpeed->nOutSampleRate	= EDITOR_DEFAULT_SAMPLEFORCHANNEL*16; //yoon
		else if( m_iSpeedFactor == 13 )	
			m_hResamplerForSpeed->nOutSampleRate	= EDITOR_DEFAULT_SAMPLEFORCHANNEL*8;
		else
			m_hResamplerForSpeed->nOutSampleRate	= EDITOR_DEFAULT_SAMPLEFORCHANNEL*100/m_iSpeedFactor;
		m_hResamplerForSpeed->Quality_Level		= NEX_RESAMPLER_DEF_QUALITY;	/* 0 ~ 3 */

		int iRet = NxResamplerInit(m_hResamplerForSpeed); 
		if( iRet < 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] resampler init fail(%d)", __LINE__, iRet);
			NxResamplerClose(m_hResamplerForSpeed);
			m_hResamplerForSpeed = NULL;
			return FALSE;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask %d] init resampler(%p)", __LINE__, m_hResamplerForSpeed);

		m_iSpeedCtlInBufferRemainSize = 0;
		m_uBufferSizeForSpeed = 384*1024;
		m_pBufferForSpeed = (unsigned char*)nexSAL_MemAlloc(384*1024);
		if( m_pBufferForSpeed == NULL )
		{
			NxResamplerClose(m_hResamplerForSpeed);
			m_hResamplerForSpeed = NULL;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] resampler init sucessed but buffer alloc failed",__LINE__);
			return FALSE;
		}
	}
	else
#endif
	{
		if(initNexSound())
		{
			float fSpeedFactor = (float)(m_iSpeedFactor==13?0.125:m_iSpeedFactor/100.0);
				
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound init sucessed(%p) and Set SpeedCtl(%f) KeepPitch(%d)",__LINE__, m_hNexSountHandle, fSpeedFactor, m_bKeepPitch);

			m_hNexSountHandle->NexSoundSetParam(Processor::SpeedControl, ParamCommand::Enable, TRUE);
			m_hNexSountHandle->NexSoundSetParam(Processor::SpeedControl, ParamCommand::SpeedControl_PlaySpeed, fSpeedFactor);
			m_hNexSountHandle->NexSoundSetParam(Processor::SpeedControl, ParamCommand::SpeedControl_Maintain_Pitch, m_bKeepPitch);

			int iSampleSize = 0;
			// if you have changed the playing speed, you should get the new BLOCK SIZE changed by new playing speed.
			m_hNexSountHandle->NexSoundGetParam(Processor::SpeedControl, ParamCommand::SpeedControl_Input_SamplePerChannel, &iSampleSize);
			m_iSpeedCtlInSize			= iSampleSize * m_iOutChannels * m_iBitsForSample / 8;
			m_iSpeedCtlOutputSize = (m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel)) * m_iOutChannels * (m_iBitsForSample >> 3);
			//m_iSpeedCtlOutputSize		= m_iSampleForChannel * m_iOutChannels * m_iBitsForSample / 8;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound init sucessed(InSize:%d OutSize:%d)",__LINE__, m_iSpeedCtlInSize, m_iSpeedCtlOutputSize);
			m_pSpeedCtlInBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInSize);
			if( m_pSpeedCtlInBuffer == NULL )
			{
				m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
				DestroyNexSound(m_hNexSountHandle);
				m_hNexSountHandle	= 0;
				m_iSpeedCtlInSize		= 0;
				m_iSpeedCtlOutputSize	= 0;
				m_iSpeedCtlInBufferRemainSize = 0;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound init sucessed but buffer alloc failed",__LINE__);
				return FALSE;
			}
			m_iSpeedCtlInBufferRemainSize = 0;

			m_pSpeedCtlOutBuffer = (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlOutputSize);
			if( m_pSpeedCtlOutBuffer == NULL )
			{
				m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
				DestroyNexSound(m_hNexSountHandle);
				m_hNexSountHandle	= 0;
				m_iSpeedCtlInSize		= 0;
				m_iSpeedCtlOutputSize	= 0;
				m_iSpeedCtlInBufferRemainSize = 0;

				nexSAL_MemFree(m_pSpeedCtlInBuffer);
				m_pSpeedCtlInBuffer = NULL;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound init sucessed but buffer alloc failed",__LINE__);
				return FALSE;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound init sucessed(h:%p o:%p i:%p) Out",
				__LINE__, m_hNexSountHandle, m_pSpeedCtlOutBuffer, m_pSpeedCtlInBuffer);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] initSpeedCtl initNexSound Fail", __LINE__);
			return FALSE;
		}
		
	}
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::deinitSpeedCtl()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] deinitSpeedCtl In(h:%p, o:%p i:%p)",
		__LINE__, m_hNexSountHandle, m_pSpeedCtlOutBuffer, m_pSpeedCtlInBuffer);
#ifdef _ANDROID // No NEXResampler library for iOS
	if(m_hResamplerForSpeed)
	{
		if(m_pBufferForSpeed)
		{
			nexSAL_MemFree(m_pBufferForSpeed);
			m_pBufferForSpeed = NULL;
		}
	
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] resampler Release(%p)",__LINE__, m_hResamplerForSpeed);
		NxResamplerClose(m_hResamplerForSpeed);
		m_hResamplerForSpeed = NULL;
	}

#endif
	if( m_hNexSountHandle )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound Release(%p)",__LINE__, m_hNexSountHandle);
		m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
		DestroyNexSound(m_hNexSountHandle);
		m_hNexSountHandle = NULL;
	}

	if( m_pSpeedCtlOutBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlOutBuffer);
		m_pSpeedCtlOutBuffer = NULL;
	}

	if( m_pSpeedCtlInBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlInBuffer);
		m_pSpeedCtlInBuffer = NULL;
		m_iSpeedCtlInBufferRemainSize = 0;
	}

	m_iSpeedCtlInSize					= 0;
	m_iSpeedCtlOutputSize				= 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] deinitSpeedCtl Out(%p)",__LINE__, m_hNexSountHandle);
	return TRUE;
}

int CNEXThread_AudioTask::processNexSound(unsigned char* pInPCM, int iInPCMSize, unsigned int uiCTS)
{
	int sample =  (m_iBitsForSample / 8)*m_iOutChannels;

	if(m_pClipItem->m_ClipAudioRenderInfoVec.size())
	{
		CClipAudioRenderInfo* pRenderInfo = NULL;
		unsigned int uiNewTime = m_uiRenderInfoStartCTS + m_uiRenderPCMDuration + m_uiBuffer4PauseSize*1000/m_iExportSamplingRate;

		if(uiNewTime > m_pCurrentRenderInfo->mEndTime)
     			pRenderInfo = m_pClipItem->getActiveAudioRenderInfo(uiNewTime, FALSE);

		if(pRenderInfo != NULL && pRenderInfo != m_pCurrentRenderInfo)
		{
			m_pPreviousRenderInfo = m_pCurrentRenderInfo;
			m_pCurrentRenderInfo = pRenderInfo;

			if(checkAudioRenderInfo(m_pCurrentRenderInfo, m_pPreviousRenderInfo))
				updateAudioRenderInfo();

#ifdef FOR_TEST_AUDIOENVELOP
			if( m_pClipItem->isAudioEnvelopEnabled() )
			{
				m_iAudioEnvelopSize = m_pCurrentRenderInfo->m_iEnvelopAudioSize;
				m_iAudioEnvelopIndex = 1;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  Audio Clip(%d) enable SetAudioEnvelop Size(%d)", __LINE__,m_pCurrentRenderInfo->muiTrackID, m_iAudioEnvelopSize);
				m_bUpdateAudioEnvelop = TRUE;

				for( m_iAudioEnvelopIndex = 0 ; m_iAudioEnvelopIndex < m_iAudioEnvelopSize ; m_iAudioEnvelopIndex++)
				{
					m_iAudioEnvelopEndCTS = m_pCurrentRenderInfo->m_puiEnvelopAudioTime[m_iAudioEnvelopIndex];		
					m_iAudioEnvelopEndVolume = m_pCurrentRenderInfo->m_puiEnvelopAudioVolume[m_iAudioEnvelopIndex];
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d]  (Index: %d, Volume: %d, CTS:%d)", __LINE__, m_iAudioEnvelopIndex, m_iAudioEnvelopEndVolume, m_iAudioEnvelopEndCTS );
				}
				m_iAudioEnvelopIndex = 1;
			}
#endif
		}
		processEnvelopForRenderInfo((short*)pInPCM, iInPCMSize, uiNewTime);
	}
	else
	{
		processEnvelop((short*)pInPCM, iInPCMSize, uiCTS);
	}

	// Case : mantis 9267 when decoder channel info was wrong.
	if( m_pAudioTrack == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio track did not prepare yet(Buffer info : %p %d %d)",__LINE__, pInPCM, iInPCMSize, uiCTS);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio track did not prepare yet(Channel info : %d %d %d)",__LINE__, m_iExportSamplingRate, m_iOutChannels, m_iBitsForSample);
		return 0;
	}

	if(m_pBuffer4Pause)
	{
		unsigned char* pTemp = m_pBuffer4Pause;
		int size;
		while( m_bIsWorking && m_bIsActive)
		{
			size = m_uiBuffer4PauseSize > 4096 ? 4096:m_uiBuffer4PauseSize;
			if( m_pAudioTrack->EnqueueAudioTrackData( pTemp, size, m_uiRenderPCMDuration ) == NEXVIDEOEDITOR_ERROR_NONE )
			{
				m_ullRenderPCMSize += size / (m_iOutChannels *m_iBitsForSample / 8);
				m_uiRenderPCMDuration = m_ullRenderPCMSize*1000/m_iExportSamplingRate;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] saved Paused Buffer (%d, %d)",__LINE__, m_uiBuffer4PauseSize, m_uiBuffer4PauseCTS);
				m_uiBuffer4PauseSize -= size;
				pTemp += size;

				if(m_uiBuffer4PauseSize)
				{
					nexSAL_TaskSleep(1);
					continue;
				}
				nexSAL_MemFree(m_pBuffer4Pause);
				m_pBuffer4Pause = NULL;
				m_uiBuffer4PauseSize = 0;
				m_uiBuffer4PauseCTS = 0;
				break;
			}
			nexSAL_TaskSleep(1);
		}

		if(m_uiBuffer4PauseSize)
		{
			memcpy(m_pBuffer4Pause, pTemp, m_uiBuffer4PauseSize);
		}
	}

#ifdef _ANDROID // No NEXResampler library for iOS
	if (m_hResamplerForSpeed)
	{
		if(m_uiLevelForSpeed > m_iSpeedFactor)
		{
			int a = 0;
    
			m_hResamplerForSpeed->pInputBuffer = pInPCM;
			m_hResamplerForSpeed->nInput = iInPCMSize/(m_iBitsForSample / 8); //EDITOR_DEFAULT_SAMPLEFORCHANNEL*m_iOutChannels;

			
 			if(m_iSpeedFactor == 3)
 				m_hResamplerForSpeed->nOutput = iInPCMSize/(m_iBitsForSample / 8)*32;
			else if(m_iSpeedFactor == 6)
				m_hResamplerForSpeed->nOutput = iInPCMSize/(m_iBitsForSample / 8)*16;
			else if(m_iSpeedFactor == 13)
				m_hResamplerForSpeed->nOutput = iInPCMSize/(m_iBitsForSample / 8)*8;
			else
				m_hResamplerForSpeed->nOutput = iInPCMSize/(m_iBitsForSample / 8)*100/m_iSpeedFactor;					

			if(m_uBufferSizeForSpeed < m_hResamplerForSpeed->nOutput * (m_iBitsForSample / 8))
			{
				nexSAL_MemFree(m_pBufferForSpeed);
				m_uBufferSizeForSpeed = m_hResamplerForSpeed->nOutput * (m_iBitsForSample / 8);
				m_pBufferForSpeed = (unsigned char*)nexSAL_MemAlloc(m_uBufferSizeForSpeed);
			}

			m_hResamplerForSpeed->pOutputBuffer	= m_pBufferForSpeed;
			a = NxResamplerResample(m_hResamplerForSpeed);

			pInPCM = m_pBufferForSpeed;
			iInPCMSize = m_hResamplerForSpeed->nOutput * (m_iBitsForSample / 8);
		}
	}

#endif
	if( m_hNexSountHandle == 0 || m_iSpeedCtlInSize <= 0 || m_iSpeedCtlOutputSize <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d] NexSound Speed control did not initailize(%d %d %d)",__LINE__, m_iSampleRate, m_iOutChannels, m_iBitsForSample);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d] NexSound Speed control did not initailize(%p %d %d)",__LINE__, pInPCM, iInPCMSize, uiCTS);

#ifdef FOR_TEST_AUDIO_SPEED_DUMP
		if( m_pSpeedPCMDump != NEXSAL_INVALID_HANDLE )
		{
			nexSAL_FileWrite(m_pSpeedPCMDump, pInPCM, iInPCMSize);
		}
#endif
		unsigned char* pTemp = NULL;
		if(m_pBufferRenderUpdate)
		{
			pTemp = m_pBufferRenderUpdate;		
			while( m_bIsWorking && m_iBufferRenderUpdateSize > 0)
			{
				int size = (m_iBufferRenderUpdateSize>8192)?8192:m_iBufferRenderUpdateSize;
                
				if(m_bIsActive == FALSE)
				{
					if(saveToPauseBuffer(pTemp, m_iBufferRenderUpdateSize, uiCTS))
					{	
						break;
					}
				}
				else if( m_pAudioTrack->EnqueueAudioTrackData( pTemp, size, uiCTS ) == NEXVIDEOEDITOR_ERROR_NONE )
				{
					m_ullRenderPCMSize += size / (m_iOutChannels *m_iBitsForSample / 8);
					m_uiRenderPCMDuration = m_ullRenderPCMSize*1000/m_iExportSamplingRate;
					m_iBufferRenderUpdateSize -= size;
					pTemp += size;
				}
				else
				{
					nexSAL_TaskSleep(1);
				}
			}	
			nexSAL_MemFree(m_pBufferRenderUpdate);
			m_pBufferRenderUpdate = NULL;
			m_iBufferRenderUpdateSize = 0;
		}
			
		pTemp = pInPCM;
		while( m_bIsWorking && iInPCMSize > 0)
		{
			int size = (iInPCMSize>8192)?8192:iInPCMSize;
			if(m_bIsActive == FALSE)
			{
				if(saveToPauseBuffer(pTemp, iInPCMSize, uiCTS))
				{
					break;
				}
			}
			else if( m_pAudioTrack->EnqueueAudioTrackData( pTemp, size, uiCTS ) == NEXVIDEOEDITOR_ERROR_NONE )
			{
				m_ullRenderPCMSize += size / (m_iOutChannels *m_iBitsForSample / 8);
				m_uiRenderPCMDuration = m_ullRenderPCMSize*1000/m_iExportSamplingRate;
				iInPCMSize -= size;
				pTemp += size;
			}
			else
			{
				nexSAL_TaskSleep(1);
			}
		}	
		return 0;
	}

	while(m_iSpeedCtlInBufferRemainSize >= m_iSpeedCtlInSize && m_bIsWorking)
	{
		
		m_hNexSountHandle->NexSoundProcess(Processor::NexSound, (Int16*)m_pSpeedCtlInBuffer, (Int16*)m_pSpeedCtlOutBuffer, m_iSpeedCtlInSize / m_iOutChannels / (m_iBitsForSample / 8));
		memmove(m_pSpeedCtlInBuffer, m_pSpeedCtlInBuffer + m_iSpeedCtlInSize, m_iSpeedCtlInBufferRemainSize-m_iSpeedCtlInSize);	
		m_iSpeedCtlInBufferRemainSize -= m_iSpeedCtlInSize;

		//processEnvelop((short*)m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS);
#ifdef FOR_TEST_AUDIO_SPEED_DUMP
		if( m_pSpeedPCMDump != NEXSAL_INVALID_HANDLE )
		{
			nexSAL_FileWrite(m_pSpeedPCMDump, m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize);
		}
#endif
		while( m_bIsWorking )
		{
			if(m_bIsActive == FALSE)
			{
				if(saveToPauseBuffer(m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS))
				{
					//memcpy(m_pSpeedCtlInBuffer + m_iSpeedCtlInBufferRemainSize, pInPCM, iInPCMSize);
					//m_iSpeedCtlInBufferRemainSize += iInPCMSize;
					//return 0;
					continue;
				}
			}
			else if( m_pAudioTrack->EnqueueAudioTrackData( m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS ) == NEXVIDEOEDITOR_ERROR_NONE )
			{
				m_ullRenderPCMSize += m_iSpeedCtlOutputSize / (m_iOutChannels *m_iBitsForSample / 8);
				m_uiRenderPCMDuration = m_ullRenderPCMSize*1000/m_iExportSamplingRate;
				break;
			}
			nexSAL_TaskSleep(5);
		}
	}

	if( m_iSpeedCtlInSize > iInPCMSize )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound Speed ctl(InSize:%d PCM:%d, Remain:%d)",__LINE__, m_iSpeedCtlInSize, iInPCMSize, m_iSpeedCtlInBufferRemainSize);
		if( m_iSpeedCtlInSize > m_iSpeedCtlInBufferRemainSize + iInPCMSize )
		{
			memcpy(m_pSpeedCtlInBuffer + m_iSpeedCtlInBufferRemainSize, pInPCM, iInPCMSize);
			m_iSpeedCtlInBufferRemainSize += iInPCMSize;
			return 0;
		}
		else
		{
			int iAddSize = m_iSpeedCtlInSize - m_iSpeedCtlInBufferRemainSize;
			memcpy(m_pSpeedCtlInBuffer + m_iSpeedCtlInBufferRemainSize, pInPCM, iAddSize);

			m_hNexSountHandle->NexSoundProcess(Processor::NexSound, (Int16*)m_pSpeedCtlInBuffer, (Int16*)m_pSpeedCtlOutBuffer, m_iSpeedCtlInSize / m_iOutChannels / (m_iBitsForSample / 8));
			m_iSpeedCtlInBufferRemainSize = iInPCMSize - iAddSize;
			memcpy(m_pSpeedCtlInBuffer, pInPCM + iAddSize, m_iSpeedCtlInBufferRemainSize);

#ifdef FOR_TEST_AUDIO_SPEED_DUMP
			if( m_pSpeedPCMDump != NEXSAL_INVALID_HANDLE )
			{
				nexSAL_FileWrite(m_pSpeedPCMDump, m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize);
			}
#endif
			while( m_bIsWorking )
			{
				if(m_bIsActive == FALSE)
				{
					if(saveToPauseBuffer(m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS))
					{
						break;
					}
				}				
				else if( m_pAudioTrack->EnqueueAudioTrackData( m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS ) == NEXVIDEOEDITOR_ERROR_NONE )
				{
					m_ullRenderPCMSize += m_iSpeedCtlOutputSize / (m_iOutChannels *m_iBitsForSample / 8);
					m_uiRenderPCMDuration = m_ullRenderPCMSize*1000/m_iExportSamplingRate;
					break;
				}
				nexSAL_TaskSleep(5);
			}
		}
	}
	else
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] NexSound Speed ctl(InSize:%d PCM:%d, Remain:%d)",__LINE__, m_iSpeedCtlInSize, iInPCMSize, m_iSpeedCtlInBufferRemainSize);
		if( m_iSpeedCtlInBufferRemainSize > 0 )
		{
			int iAddSize = m_iSpeedCtlInSize - m_iSpeedCtlInBufferRemainSize;
			memcpy(m_pSpeedCtlInBuffer + m_iSpeedCtlInBufferRemainSize, pInPCM, iAddSize);
			pInPCM += iAddSize;
			iInPCMSize -= iAddSize;
			m_iSpeedCtlInBufferRemainSize = 0;
			m_hNexSountHandle->NexSoundProcess(Processor::NexSound, (Int16*)m_pSpeedCtlInBuffer, (Int16*)m_pSpeedCtlOutBuffer, m_iSpeedCtlInSize / m_iOutChannels / (m_iBitsForSample / 8));

#ifdef FOR_TEST_AUDIO_SPEED_DUMP
            if( m_pSpeedPCMDump != NEXSAL_INVALID_HANDLE )
            {
                nexSAL_FileWrite(m_pSpeedPCMDump, m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize);
            }
#endif
			while( m_bIsWorking )
			{
				if(m_bIsActive == FALSE)
				{
					if(saveToPauseBuffer(m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS))
					{
						break;
					}
				}				
				else if( m_pAudioTrack->EnqueueAudioTrackData( m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS ) == NEXVIDEOEDITOR_ERROR_NONE )
				{
					m_ullRenderPCMSize += m_iSpeedCtlOutputSize / (m_iOutChannels *m_iBitsForSample / 8);
					m_uiRenderPCMDuration = m_ullRenderPCMSize*1000/m_iExportSamplingRate;
					break;
				}		
				nexSAL_TaskSleep(5);
			}
		}

		while(iInPCMSize >= m_iSpeedCtlInSize && m_bIsWorking)
		{
			m_hNexSountHandle->NexSoundProcess(Processor::NexSound, (Int16*)pInPCM, (Int16*)m_pSpeedCtlOutBuffer, m_iSpeedCtlInSize / m_iOutChannels / (m_iBitsForSample / 8));
			pInPCM += m_iSpeedCtlInSize;
			iInPCMSize -= m_iSpeedCtlInSize;

			// processEnvelop((short*)m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS);
#ifdef FOR_TEST_AUDIO_SPEED_DUMP
			if( m_pSpeedPCMDump != NEXSAL_INVALID_HANDLE )
			{
				nexSAL_FileWrite(m_pSpeedPCMDump, m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize);
			}
#endif
			while( m_bIsWorking )
			{
				if(m_bIsActive == FALSE)
				{
					if(saveToPauseBuffer(m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS))
					{
						break;
					}
				}
				else if( m_pAudioTrack->EnqueueAudioTrackData( m_pSpeedCtlOutBuffer, m_iSpeedCtlOutputSize, uiCTS ) == NEXVIDEOEDITOR_ERROR_NONE )
				{
					m_ullRenderPCMSize += m_iSpeedCtlOutputSize / (m_iOutChannels *m_iBitsForSample / 8);
					m_uiRenderPCMDuration = m_ullRenderPCMSize*1000/m_iExportSamplingRate;
					break;
				}
				nexSAL_TaskSleep(5);
			}
		}

		if( iInPCMSize > 0 && m_bIsWorking )
		{
			memcpy(m_pSpeedCtlInBuffer, pInPCM, iInPCMSize);
			m_iSpeedCtlInBufferRemainSize = iInPCMSize;
		}
	}
	return 0;
}

NXBOOL CNEXThread_AudioTask::isRepeat(unsigned int uiTime)
{
	if( m_pClipItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] did not reg a clip", __LINE__);
		return FALSE;
	}

	CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
	if( pProjectMng == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] ID(%d) Can't get projectmanager", __LINE__, m_pClipItem->getClipID());
		return FALSE;
	}

	NXBOOL bRepeat = FALSE;
	CClipList* pClipList = pProjectMng->getClipList();
	if( pClipList != NULL )
	{
		bRepeat = pClipList->isAudioClipTime(m_pClipItem->getClipID(), uiTime);
		SAFE_RELEASE(pClipList);
	}

	SAFE_RELEASE(pProjectMng);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] ID(%d) Check repeat audio(%d)", __LINE__, m_pClipItem->getClipID(), bRepeat);
	return bRepeat;
}

NXBOOL CNEXThread_AudioTask::checkDecoding()
{
	if( m_pSource == NULL )
		return FALSE;

	if( m_pSource->isAudioExist() == FALSE )
		return FALSE;

	unsigned int	uSamplingRate			= 0;
	unsigned int	uNumOfChannels			= 0;
	unsigned int	uNumOfSamplesPerChannel	= 0;

	NXINT64		uiCTS				= 0;
	unsigned int	uFormatTag			= 0;
	unsigned int	uBitsPerSample		= 0;
	unsigned int	uBlockAlign			= 0;
	unsigned int	uAvgBytesPerSec	= 0;
	unsigned int	uSamplesPerBlock	= 0;
	unsigned int	uEnCodeOpt			= 0;
	unsigned int	uExtraSize			= 0;
	unsigned char*	pExtraData		= NULL;

	NXUINT8*		pDSI			= NULL;
	NXUINT8*		pFrame			= NULL;
	NXUINT32		uDSISize		= 0;
	NXUINT32		uFrameSize		= 0;
	unsigned int*		pTempDSI		= NULL;
	int				iRet				= 0;
	unsigned int		uiReaderRet		= 0;
	int nProfile = 0;
	unsigned int uiAudioObjectType =  m_pSource->getAudioObjectType();

	NXBOOL				bFrameEnd = FALSE;
	unsigned int			uiDecoderErrRet		= 0;
	unsigned int			uiPCMBufSize		= 0;

	m_pSource->getWAVEFormat(&uFormatTag, &uBitsPerSample, &uBlockAlign, &uAvgBytesPerSec, &uSamplesPerBlock, &uEnCodeOpt, &uExtraSize, &pExtraData);
	m_pSource->getDSI(NXFF_MEDIA_TYPE_AUDIO, &pDSI, &uDSISize );

	m_pSource->getSamplingRate(&uSamplingRate);
	m_pSource->getNumberOfChannel(&uNumOfChannels);
	m_pSource->getSamplesPerChannel(&uNumOfSamplesPerChannel);


	while( 1 )
	{
		uiReaderRet = m_pSource->getAudioFrame();

		if( uiReaderRet == _SRC_GETFRAME_OK )
		{
 			m_pSource->getAudioFrameData(&pFrame, &uFrameSize );
			m_pSource->getAudioFrameCTS(&uiCTS);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding Audio Initialize Start(%lld)\n", __LINE__, uiCTS);

			if( m_hCodecAudio )
			{
				nexCAL_AudioDecoderDeinit( m_hCodecAudio );
				CNexCodecManager::releaseCodec( m_hCodecAudio );
				m_hCodecAudio = NULL;
			}

			// RYU 20130620 
			if ( (uiAudioObjectType == eNEX_CODEC_A_AAC || uiAudioObjectType == eNEX_CODEC_A_AACPLUS))
			{
				getAACProfile(pFrame, uFrameSize, &nProfile);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding AAC Profile(%d) (%d, 0x%x)\n", __LINE__, nProfile,
					CNexVideoEditor::m_bSupportAACSWCodec,  CNexVideoEditor::m_iSupportAACProfile);
				
				switch( nProfile)
				{
				case 0: // main
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE  && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_MAIN) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding Using Software Codec for AAC Main\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 2: // SSR
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_SSR) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding Using Software Codec for AAC SSR\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 3: // LTP
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_LTP) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding Using Software Codec for AAC LTP\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 1: // LC
					//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] support AAC LC", __LINE__);
				default :
					//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] AAC Unknown ", __LINE__);
					break;
				}
			}

			m_hCodecAudio = CNexCodecManager::getCodec(NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, uiAudioObjectType);

			if( m_hCodecAudio == NULL )
			{
				return FALSE;
			}

			m_uiAudioObjectType = uiAudioObjectType;

			if ( (m_pSource->getAudioObjectType() == eNEX_CODEC_A_AAC || m_pSource->getAudioObjectType() == eNEX_CODEC_A_AACPLUS) && pDSI != NULL )
			{
				unsigned int    uSamplingFreqIndex = 0;
				const unsigned int puAACSampleRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};

				NXINT32 isSBR;
				NexCodecUtil_AAC_ParseDSI(pDSI, uDSISize, &uSamplingFreqIndex, &uNumOfSamplesPerChannel, &isSBR);
				if (isSBR)
				{
					m_iAACSbr = isSBR;
				}

				uSamplingFreqIndex = ((pDSI[0] & 0x07) << 1) | ((pDSI[1] & 0x80) >> 7);

				if ( uSamplingFreqIndex > 11 )//Nx_robin__090827 uSamplingFreqIndexê°?ì²´í¬
					uSamplingFreqIndex = 0;
				uSamplingRate = puAACSampleRates[uSamplingFreqIndex];
				uNumOfChannels = (pDSI[1] & 0x78) >> 3;
				if ( uNumOfChannels == 0 )//Nx_robin__090827 DSI????? uNumOfChannels??? 0????? ????????????? File MetaData????? ??????????????.
					m_pSource->getNumberOfChannel(&uNumOfChannels);
				uBitsPerSample = 16;

				uiAudioObjectType = NexCodecUtil_GetAACType((NEX_CODEC_TYPE)m_pSource->getAudioObjectType(), uSamplingRate, uNumOfChannels, pFrame, uFrameSize);							

				if(uiAudioObjectType == eNEX_CODEC_A_AACPLUSV2)
					m_pSource->setAudioObjectType(eNEX_CODEC_A_AACPLUS);
				
				if ( m_pSource->getAudioObjectType() == eNEX_CODEC_A_AACPLUS )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding This contents is AAC+\n", __LINE__);
					uNumOfSamplesPerChannel = 2048;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding This contents is AAC\n", __LINE__);
					uNumOfSamplesPerChannel = 1024;
				}
			}
			break;
		}
		else if( uiReaderRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d]checkDecoding Need Buffering....\n", __LINE__);
			nexSAL_TaskSleep(20);
			continue;
		}
		// JDKIM 2011/02/03 : Lost Frame??????? ?????? OK??? ?????????? ????????
		else if( uiReaderRet == _SRC_GETFRAME_OK_LOSTFRAME)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding Remove Lost Frame..\n", __LINE__);
			continue;
		}
		
		/*
		else if( uiReaderRet == _SRC_GETFRAME_END)
		{
			NXBOOL bRepeat = isRepeat(uiCTS == 0 ? uiStartTS + 500 : uiCTS + 500);
			if( bRepeat )
			{
				if( m_pSource->setRepeatAudioClip(uiCTS, m_pClipItem->getStartTrimTime()) )
				{
					m_isNextFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Clip Repeat Start", __LINE__);
					continue;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] Audio Clip Repeat failed", __LINE__);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] getAudioFrame End", __LINE__);
			goto DECODER_INIT_ERROR;
		}
		*/
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding Audio Data Read Error[0x%X].\n", __LINE__, uiReaderRet);
			goto DECODER_INIT_ERROR;
		}
	}


	//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding nexCAL_AudioDecoderInit Frame Data!",  __LINE__);
	//nexSAL_MemDump(pDSI, uDSISize);
	//nexSAL_MemDump(pFrame, uFrameSize);


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding Audio Decoder Init Start(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	MPEG_AUDIO_INFO mpa_info;
	memset(&mpa_info, 0x00, sizeof(MPEG_AUDIO_INFO));
	NexCodecUtil_Get_MPEG_Audio_Info( pFrame, uFrameSize, &mpa_info );
	if (mpa_info.Version == NEX_MPEG_AUDIO_VER_1 && mpa_info.Layer == NEX_MPEG_AUDIO_LAYER_2)
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 1);
	else
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 0);

	iRet = nexCAL_AudioDecoderInit(		m_hCodecAudio,
										(NEX_CODEC_TYPE)m_pSource->getAudioObjectType(), 
										pDSI, 
										uDSISize, 
										pFrame, 
										uFrameSize, 
										NULL,
										NULL,
										&uSamplingRate,
										&uNumOfChannels, 
										&uBitsPerSample, 
										&uNumOfSamplesPerChannel, 
										NEXCAL_ADEC_MODE_NONE,
										0, // USERDATATYPE
										(NXVOID *)this );

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding Audio Decoder Init End(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	m_uiAudioDecoderInitTime = nexSAL_GetTickCount();

	if(m_pAudioDecodeBuf != NULL)
	{
		nexSAL_MemFree(m_pAudioDecodeBuf);
		m_pAudioDecodeBuf = NULL;
	}

	m_uiAudioDecodeBufSize = 384*1024;
	m_pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioDecodeBufSize);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] checkDecoding Default  PCMBuffer(%p, %d)\n", __LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
	
	if( iRet != NEXCAL_ERROR_NONE )
	{
		if ( pTempDSI )
		{
			nexSAL_MemFree(pTempDSI);
			pTempDSI = NULL;
			pDSI = NULL;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding Audio Decoder Init Failed(%d)",  __LINE__, iRet);
		goto DECODER_INIT_ERROR;
	}
	if ( pTempDSI )
	{
		nexSAL_MemFree(pTempDSI);
		pTempDSI = NULL;
		pDSI = NULL;
	}

	m_iSampleRate		= uSamplingRate;
	m_iChannels			= uNumOfChannels;
	m_iBitsForSample		= uBitsPerSample;
	m_iSampleForChannel	= uNumOfSamplesPerChannel;

	m_pSource->setNumOfChannels(m_iChannels);
	m_pSource->setSamplingRate(m_iSampleRate);
	m_pSource->setNumOfSamplesPerChannel(m_iSampleForChannel);

	// Case : mantis 9267 when decoder channel info was wrong.
	if( m_iSampleRate == 0 || m_iChannels == 0 || m_iBitsForSample == 0 || m_iSampleForChannel == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding Wait decode config change",  __LINE__);
	}

	while(1)
	{
		nexSAL_TaskSleep(1);

		uiPCMBufSize = m_uiAudioDecodeBufSize;
		nexCAL_AudioDecoderDecode(	m_hCodecAudio,
									pFrame, 
									uFrameSize, 
									NULL, 
									m_pAudioDecodeBuf,
									(int *)&uiPCMBufSize,
									(unsigned int)uiCTS,
									NULL,
									bFrameEnd == FALSE?NEXCAL_ADEC_FLAG_NONE:NEXCAL_ADEC_FLAG_END_OF_STREAM,
									&uiDecoderErrRet );

		bFrameEnd = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d]  checkDecoding After Decode. (Buf %p, BufSize:%d, TS:%lld Ret:0x%x)", 	__LINE__, m_pAudioDecodeBuf, uiPCMBufSize , uiCTS, uiDecoderErrRet);

		if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_DECODING_SUCCESS))
		{
			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_CHANGED))
			{
				unsigned int uiSamplingRate = 0;
				unsigned int uiChannels = 0;
				unsigned int uiBitPerSample = 0;
				unsigned int uiNumOfSamplesPerChannel = 0;
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_SAMPLINGRATE, &uiSamplingRate);
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS, &uiChannels);
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE, &uiBitPerSample);
				nexCAL_AudioDecoderGetInfo(m_hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL, &uiNumOfSamplesPerChannel);

				if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
				{
					if(CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec == FALSE && m_iChannels == 1)
					{
						uiChannels = m_iChannels;
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] checkDecoding Audio Output was changed!(S(%d) C(%d) BpS(%d) SC(%d)", 
					__LINE__, uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
			}

			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_EXIST))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] checkDecoding Audio Output size(%d)", __LINE__, uiPCMBufSize);
				break;
			}

			if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] checkDecoding Audio Task EOS received", __LINE__);
				goto DECODER_INIT_ERROR;	
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] checkDecoding Audio Dec Failed(%p %d %lld Ret:%d)", __LINE__, pFrame, uFrameSize, uiCTS, uiDecoderErrRet);
			goto DECODER_INIT_ERROR;
		}
	}

	return TRUE;

DECODER_INIT_ERROR:
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding Audio Decoder failed(Track %p)",  __LINE__, m_pAudioTrack);
	if( m_hCodecAudio )
	{
		CNexCodecManager::releaseCodec(m_hCodecAudio);
		m_hCodecAudio = NULL;
	}
	if ( pTempDSI )
	{
		nexSAL_MemFree(pTempDSI);
		pTempDSI = NULL;
		pDSI = NULL;
	}

	SAFE_RELEASE(m_pAudioTrack);
	// Event FILE INVALID ERROR
	// Error Report
	return FALSE;	
}

NXBOOL CNEXThread_AudioTask::initEqualizer()
{
	if(m_pCurrentRenderInfo)
		m_pEqualizer =  m_pCurrentRenderInfo->m_pEqualizer;

	if( m_pThumbnail != NULL || m_pEqualizer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound Equalizer did not use (%x, %x)",__LINE__, m_pClipItem->getClipID(), m_pThumbnail, m_pEqualizer);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initEqualizer In", __LINE__, m_pClipItem->getClipID());
	
	if(initNexSound())
	{
		m_hNexSountHandle->NexSoundSetParam(Processor::Equalizer, ParamCommand::JsonDoc, m_pEqualizer);
		m_hNexSountHandle->NexSoundSetParam(Processor::Equalizer, ParamCommand::Enable, TRUE);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed(0x%x) and Set Equalizer(%d,%s)",__LINE__, m_pClipItem->getClipID(), m_hNexSountHandle, strlen(m_pEqualizer), m_pEqualizer);

		if(m_iSpeedFactor == 100)
		{
			m_iSpeedCtlOutputSize = (m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel)) * m_iOutChannels * (m_iBitsForSample >> 3);
			m_iSpeedCtlInSize = m_iSpeedCtlOutputSize; 

			if (m_pSpeedCtlInBuffer == NULL)
			{
				m_pSpeedCtlInBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInSize);
			}
			if (m_pSpeedCtlOutBuffer == NULL)
			{
				m_pSpeedCtlOutBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlOutputSize);
			}
			m_iSpeedCtlInBufferRemainSize = 0;

			if( m_pSpeedCtlInBuffer == NULL || m_pSpeedCtlOutBuffer == NULL)
			{
				m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
				DestroyNexSound(m_hNexSountHandle);
				m_hNexSountHandle	= 0;
				m_iSpeedCtlInSize		= 0;
				m_iSpeedCtlOutputSize	= 0;
				m_iSpeedCtlInBufferRemainSize = 0;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed but buffer alloc failed",__LINE__, m_pClipItem->getClipID());
				return FALSE;
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initEqualizer Success", __LINE__, m_pClipItem->getClipID());
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initEqualizer initNexSound Fail", __LINE__, m_pClipItem->getClipID());
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::deinitEqualizer()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit Equalizer In", __LINE__, m_pClipItem->getClipID());

	if( m_hNexSountHandle )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound Release(0x%x)", __LINE__, m_pClipItem->getClipID(), m_hNexSountHandle);

		m_hNexSountHandle->NexSoundRelease(Processor::Equalizer);
		m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
		DestroyNexSound(m_hNexSountHandle);
		m_hNexSountHandle = NULL;
	}

	if( m_pSpeedCtlOutBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlOutBuffer);
		m_pSpeedCtlOutBuffer = NULL;
	}

	if( m_pSpeedCtlInBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlInBuffer);
		m_pSpeedCtlInBuffer = NULL;
		m_iSpeedCtlInBufferRemainSize = 0;
	}
#if 0
	if (m_pEqualizer)
	{
		nexSAL_MemFree(m_pEqualizer);
		m_pEqualizer = NULL;
	}
#endif	
	m_iSpeedCtlInSize		= 0;
	m_iSpeedCtlOutputSize	= 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit Equalizer Out", __LINE__, m_pClipItem->getClipID());

	return TRUE;
}

NXBOOL CNEXThread_AudioTask::initEnhancedAudioFilter()
{
	if(m_pCurrentRenderInfo)
		m_pEnhancedAudioFilter =  m_pCurrentRenderInfo->m_pEnhancedAudioFilter;

	if( m_pThumbnail != NULL || m_pEnhancedAudioFilter == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound EnhancedAudioFilter did not use (%x, %x)",__LINE__, m_pClipItem->getClipID(), m_pThumbnail, m_pEnhancedAudioFilter);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initEnhancedAudioFilter In", __LINE__, m_pClipItem->getClipID());
	
	if(initNexSound())
	{
		m_hNexSountHandle->NexSoundSetParam(Processor::VoiceChanger, ParamCommand::JsonDoc, m_pEnhancedAudioFilter);
		m_hNexSountHandle->NexSoundSetParam(Processor::VoiceChanger, ParamCommand::Enable, TRUE);
		//m_hNexSountHandle->NexSoundSetParam(Processor::NexSound, ParamCommand::NexSound_Headroom, -6);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed(0x%x) and Set EnhancedAudioFilter(%d,%s)",__LINE__, m_pClipItem->getClipID(), m_hNexSountHandle, strlen(m_pEnhancedAudioFilter), m_pEnhancedAudioFilter);

		if(m_iSpeedFactor == 100)
		{
			m_iSpeedCtlOutputSize = (m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel)) * m_iOutChannels * (m_iBitsForSample >> 3);
			m_iSpeedCtlInSize = m_iSpeedCtlOutputSize; 

			if (m_pSpeedCtlInBuffer == NULL)
			{
				m_pSpeedCtlInBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInSize);
			}
			if (m_pSpeedCtlOutBuffer == NULL)
			{
				m_pSpeedCtlOutBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlOutputSize);
			}
			m_iSpeedCtlInBufferRemainSize = 0;

			if( m_pSpeedCtlInBuffer == NULL || m_pSpeedCtlOutBuffer == NULL)
			{
				m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
				DestroyNexSound(m_hNexSountHandle);
				m_hNexSountHandle	= 0;
				m_iSpeedCtlInSize		= 0;
				m_iSpeedCtlOutputSize	= 0;
				m_iSpeedCtlInBufferRemainSize = 0;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed but buffer alloc failed",__LINE__, m_pClipItem->getClipID());
				return FALSE;
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initEnhancedAudioFilter Success", __LINE__, m_pClipItem->getClipID());
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initEnhancedAudioFilter initNexSound Fail", __LINE__, m_pClipItem->getClipID());
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::deinitEnhancedAudioFilter()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit EnhancedAudioFilter In", __LINE__, m_pClipItem->getClipID());

	if( m_hNexSountHandle )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound Release(0x%x)", __LINE__, m_pClipItem->getClipID(), m_hNexSountHandle);

		m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
		DestroyNexSound(m_hNexSountHandle);
		m_hNexSountHandle = NULL;
	}

	if( m_pSpeedCtlOutBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlOutBuffer);
		m_pSpeedCtlOutBuffer = NULL;
	}

	if( m_pSpeedCtlInBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlInBuffer);
		m_pSpeedCtlInBuffer = NULL;
		m_iSpeedCtlInBufferRemainSize = 0;
	}
#if 0
	if (m_pEnhancedAudioFilter)
	{
		nexSAL_MemFree(m_pEnhancedAudioFilter);
		m_pEnhancedAudioFilter = NULL;
	}
#endif	
	m_iSpeedCtlInSize		= 0;
	m_iSpeedCtlOutputSize	= 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit EnhancedAudioFilter Out", __LINE__, m_pClipItem->getClipID());

	return TRUE;
}

NXBOOL CNEXThread_AudioTask::initVoiceChanger()
{
	if(m_pCurrentRenderInfo)
		m_iVoiceChangerFactor = m_pCurrentRenderInfo->m_iVoiceChangerFactor;
	
	if( m_pThumbnail != NULL || m_iVoiceChangerFactor <= 0 || m_iVoiceChangerFactor >= 5 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound VoiceChanager did not use (%p, %d)",__LINE__, m_pClipItem->getClipID(), m_pThumbnail, m_iPitchIndex);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initVoiceChanger In", __LINE__, m_pClipItem->getClipID());
	
	if(initNexSound())
	{			
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed(%p) and Set VoiceChanger(%d)",__LINE__, m_pClipItem->getClipID(), m_hNexSountHandle, m_iVoiceChangerFactor);
		m_hNexSountHandle->NexSoundSetParam(Processor::VoiceChanger, ParamCommand::Enable, TRUE);		
		m_hNexSountHandle->NexSoundSetParam(Processor::VoiceChanger, ParamCommand::VoiceChanger_VoiceType, m_iVoiceChangerFactor-1);

		m_hNexSountHandle->NexSoundSetParam(Processor::NexSound, ParamCommand::NexSound_Headroom, -6);

		if(m_iSpeedFactor == 100)
		{
			m_iSpeedCtlOutputSize = (m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel)) * m_iOutChannels * (m_iBitsForSample >> 3);
			m_iSpeedCtlInSize = m_iSpeedCtlOutputSize; 

			if (m_pSpeedCtlInBuffer == NULL)
			{
				m_pSpeedCtlInBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInSize);
			}
			if (m_pSpeedCtlOutBuffer == NULL)
			{
				m_pSpeedCtlOutBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlOutputSize);
			}
			m_iSpeedCtlInBufferRemainSize = 0;

			if( m_pSpeedCtlInBuffer == NULL || m_pSpeedCtlOutBuffer == NULL)
			{
				m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
				DestroyNexSound(m_hNexSountHandle);
				m_hNexSountHandle	= 0;
				m_iSpeedCtlInSize		= 0;
				m_iSpeedCtlOutputSize	= 0;
				m_iSpeedCtlInBufferRemainSize = 0;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed but buffer alloc failed",__LINE__, m_pClipItem->getClipID());
				return FALSE;
			}
			
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initVoiceChanger Success", __LINE__, m_pClipItem->getClipID());
		return TRUE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initVoiceChanger initNexSound Fail", __LINE__, m_pClipItem->getClipID());
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::deinitVoiceChanger()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit VoiceChanger In", __LINE__, m_pClipItem->getClipID());

	if( m_hNexSountHandle )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound Release(%p)", __LINE__, m_pClipItem->getClipID(), m_hNexSountHandle);
		m_hNexSountHandle->NexSoundSetParam(Processor::VoiceChanger, ParamCommand::Enable, FALSE);		
		m_hNexSountHandle->NexSoundSetParam(Processor::VoiceChanger, ParamCommand::VoiceChanger_VoiceType, -1);
		m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
		DestroyNexSound(m_hNexSountHandle);
		m_hNexSountHandle = NULL;
	}

	if( m_pSpeedCtlOutBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlOutBuffer);
		m_pSpeedCtlOutBuffer = NULL;
	}

	if( m_pSpeedCtlInBuffer )
	{
		nexSAL_MemFree(m_pSpeedCtlInBuffer);
		m_pSpeedCtlInBuffer = NULL;
		m_iSpeedCtlInBufferRemainSize = 0;
	}

	m_iSpeedCtlInSize					= 0;
	m_iSpeedCtlOutputSize				= 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit VoiceChanger Out", __LINE__, m_pClipItem->getClipID());
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::initPitchContorl()
{
	if(m_pCurrentRenderInfo)
		m_iPitchIndex = m_pCurrentRenderInfo->m_iPitchIndex;

	if( m_pThumbnail != NULL  || (m_iPitchIndex == 0 || m_iPitchIndex < -12 || m_iPitchIndex > 12) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound PitchContorl did not use (%p, %d)",__LINE__, m_pClipItem->getClipID(), m_pThumbnail, m_iPitchIndex);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initPitchContorl In", __LINE__, m_pClipItem->getClipID());
	
	if(initNexSound())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed(%p) and Set PitchContorl(%d)", __LINE__, m_pClipItem->getClipID(), m_hNexSountHandle, m_iPitchIndex);
		m_hNexSountHandle->NexSoundSetParam(Processor::PitchControl, ParamCommand::Enable, TRUE);		
		m_hNexSountHandle->NexSoundSetParam(Processor::PitchControl, ParamCommand::PitchControl_PitchIndex, m_iPitchIndex);

		if(m_iSpeedFactor == 100)
		{
			m_iSpeedCtlOutputSize = (m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel)) * m_iOutChannels * (m_iBitsForSample >> 3);
			m_iSpeedCtlInSize = m_iSpeedCtlOutputSize; 

			if (m_pSpeedCtlInBuffer == NULL)
			{
				m_pSpeedCtlInBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInSize);
			}
			if (m_pSpeedCtlOutBuffer == NULL)
			{
				m_pSpeedCtlOutBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlOutputSize);
			}
			m_iSpeedCtlInBufferRemainSize = 0;

			if( m_pSpeedCtlInBuffer == NULL || m_pSpeedCtlOutBuffer == NULL)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed but buffer alloc failed",__LINE__, m_pClipItem->getClipID());
				deinitPitchContorl();
				return FALSE;
			}
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initPitchContorl Success", __LINE__, m_pClipItem->getClipID());
		return TRUE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initPitchContorl initNexSound Fail", __LINE__, m_pClipItem->getClipID());
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::deinitPitchContorl()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit PitchContorl In", __LINE__, m_pClipItem->getClipID());

	deinitNexSound();
	if(m_iSpeedFactor == 100)
	{
		if( m_pSpeedCtlOutBuffer )
		{
			nexSAL_MemFree(m_pSpeedCtlOutBuffer);
			m_pSpeedCtlOutBuffer = NULL;
		}

		if( m_pSpeedCtlInBuffer )
		{
			nexSAL_MemFree(m_pSpeedCtlInBuffer);
			m_pSpeedCtlInBuffer = NULL;
			m_iSpeedCtlInBufferRemainSize = 0;
		}

		m_iSpeedCtlInSize		= 0;
		m_iSpeedCtlOutputSize	= 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit PitchContorl Out", __LINE__, m_pClipItem->getClipID());
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::initCompressor()
{
	if(m_pCurrentRenderInfo)
		m_iCompressorFactor = m_pCurrentRenderInfo->m_iCompressorFactor;

	if( m_pThumbnail != NULL  || (m_iCompressorFactor < 1 || m_iCompressorFactor > 7) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound Compressor did not use (%p, %d) ",__LINE__, m_pClipItem->getClipID(), m_pThumbnail, m_iCompressorFactor);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initCompressor In", __LINE__, m_pClipItem->getClipID());
	
	if(initNexSound())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed(%p) and Set Compressor(%d)",__LINE__ , m_pClipItem->getClipID(), m_hNexSountHandle, m_iCompressorFactor);
		m_hNexSountHandle->NexSoundSetParam(Processor::MaxVolume, ParamCommand::Enable, TRUE);
		m_hNexSountHandle->NexSoundSetParam(Processor::MaxVolume, ParamCommand::MaxVolume_Strength, m_iCompressorFactor-1);
		m_hNexSountHandle->NexSoundSetParam(Processor::MaxVolume, ParamCommand::MaxVolume_Release, 3);

		if(m_iSpeedFactor == 100)
		{
			m_iSpeedCtlOutputSize = (m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel)) * m_iOutChannels * (m_iBitsForSample >> 3);
			m_iSpeedCtlInSize = m_iSpeedCtlOutputSize;

			if (m_pSpeedCtlInBuffer == NULL)
			{
				m_pSpeedCtlInBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInSize);
			}
			if (m_pSpeedCtlOutBuffer == NULL)
			{
				m_pSpeedCtlOutBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlOutputSize);
			}
			m_iSpeedCtlInBufferRemainSize = 0;

			if( m_pSpeedCtlInBuffer == NULL || m_pSpeedCtlOutBuffer == NULL)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed but buffer alloc failed",__LINE__, m_pClipItem->getClipID());
				deinitCompressor();
				return FALSE;
			}
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initCompressor Success", __LINE__, m_pClipItem->getClipID());
		return TRUE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initCompressor initNexSound Fail", __LINE__, m_pClipItem->getClipID());
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::deinitCompressor()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinitCompressor In", __LINE__, m_pClipItem->getClipID());

	deinitNexSound();
	if(m_iSpeedFactor == 100)
	{
		if( m_pSpeedCtlOutBuffer )
		{
			nexSAL_MemFree(m_pSpeedCtlOutBuffer);
			m_pSpeedCtlOutBuffer = NULL;
		}

		if( m_pSpeedCtlInBuffer )
		{
			nexSAL_MemFree(m_pSpeedCtlInBuffer);
			m_pSpeedCtlInBuffer = NULL;
			m_iSpeedCtlInBufferRemainSize = 0;
		}

		m_iSpeedCtlInSize		= 0;
		m_iSpeedCtlOutputSize	= 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinitCompressor Out", __LINE__, m_pClipItem->getClipID());
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::initMusicEffect()
{
	if(m_pCurrentRenderInfo)
	{
		m_iMusicEffector = m_pCurrentRenderInfo->m_iMusicEffector;
		m_iProcessorStrength = m_pCurrentRenderInfo->m_iProcessorStrength;
		m_iBassStrength = m_pCurrentRenderInfo->m_iBassStrength;		
	}	
	if( m_pThumbnail != NULL  || (m_iMusicEffector < 1 || m_iMusicEffector > 3) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound MusicEffect did not use (%p, %d)",__LINE__, m_pClipItem->getClipID(), m_pThumbnail, m_iMusicEffector);
		return TRUE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initMusicEffect In", __LINE__, m_pClipItem->getClipID());

	if(initNexSound())
	{
		Processor::TAG eProcessor = Processor::Normal;
		int nResult = NEXSOUND_SUCCESS;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed(%p) and Set MusicEffect(%d,%d,%d)",__LINE__ , m_pClipItem->getClipID(), m_hNexSountHandle, m_iMusicEffector, m_iProcessorStrength, m_iBassStrength);

		if (m_iMusicEffector == 1)
			eProcessor = (Processor::TAG)Processor::LiveConcert;
		else if (m_iMusicEffector == 2)
			eProcessor = (Processor::TAG)Processor::StereoChorus;
		else if (m_iMusicEffector == 3)
			eProcessor = (Processor::TAG)Processor::MusicEnhancer;

		if (NEXSOUND_SUCCESS != m_hNexSountHandle->NexSoundSetParam(eProcessor, ParamCommand::Enable, TRUE))
			goto MusicEffectError;
		if (NEXSOUND_SUCCESS != m_hNexSountHandle->NexSoundSetParam(eProcessor, ParamCommand::ProcessorStrength, m_iProcessorStrength))
			goto MusicEffectError;
		if (NEXSOUND_SUCCESS != m_hNexSountHandle->NexSoundSetParam(eProcessor, ParamCommand::BassStrength, m_iBassStrength))
			goto MusicEffectError;

		m_hNexSountHandle->NexSoundSetParam(Processor::NexSound, ParamCommand::NexSound_Headroom, -6);

		if(m_iSpeedFactor == 100)
		{
			m_iSpeedCtlOutputSize = (m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel)) * m_iOutChannels * (m_iBitsForSample >> 3);
			m_iSpeedCtlInSize = m_iSpeedCtlOutputSize; 

			if (m_pSpeedCtlInBuffer == NULL)
			{
				m_pSpeedCtlInBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInSize);
			}
			if (m_pSpeedCtlOutBuffer == NULL)
			{
				m_pSpeedCtlOutBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlOutputSize);
			}
			m_iSpeedCtlInBufferRemainSize = 0;

			if( m_pSpeedCtlInBuffer == NULL || m_pSpeedCtlOutBuffer == NULL)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound init sucessed but buffer alloc failed",__LINE__, m_pClipItem->getClipID());
				deinitMusicEffect();
				goto MusicEffectError;
			}
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initMusicEffect Success", __LINE__, m_pClipItem->getClipID());
		return TRUE;
	}

MusicEffectError:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) initPitchContorl initNexSound Fail", __LINE__, m_pClipItem->getClipID());
	return FALSE;
}

NXBOOL CNEXThread_AudioTask::deinitMusicEffect()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinitMusicEffect In", __LINE__, m_pClipItem->getClipID());
#if 0
	Processor::TAG eProcessor = Processor::Normal;
	if (m_iMusicEffector == 1)
		eProcessor = (Processor::TAG)Processor::LiveConcert;
	else if (m_iMusicEffector == 2)
		eProcessor = (Processor::TAG)Processor::StereoChorus;
	else if (m_iMusicEffector == 3)
		eProcessor = (Processor::TAG)Processor::MusicEnhancer;
	m_hNexSountHandle->NexSoundSetParam(eProcessor, ParamCommand::Enable, FALSE);
#endif
	if(m_iSpeedFactor == 100)
	{
		deinitNexSound();
		if(m_iSpeedFactor == 100)
		{
			if( m_pSpeedCtlOutBuffer )
			{
				nexSAL_MemFree(m_pSpeedCtlOutBuffer);
				m_pSpeedCtlOutBuffer = NULL;
			}

			if( m_pSpeedCtlInBuffer )
			{
				nexSAL_MemFree(m_pSpeedCtlInBuffer);
				m_pSpeedCtlInBuffer = NULL;
				m_iSpeedCtlInBufferRemainSize = 0;
			}

			m_iSpeedCtlInSize		= 0;
			m_iSpeedCtlOutputSize	= 0;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) deinit MusicEffect Out", __LINE__, m_pClipItem->getClipID());
	return TRUE;
}

int CNEXThread_AudioTask::initNexSound()
{
	if(m_hNexSountHandle)
		return TRUE;
	
	m_hNexSountHandle = CreateNexSound();
	if( m_hNexSountHandle == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound create failed",__LINE__, m_pClipItem->getClipID());
		return FALSE;
	}

	if(m_hNexSountHandle->NexSoundInitialize(Processor::NexSound, m_iOutChannels, m_iExportSamplingRate, m_iBitsForSample, m_iSampleForChannel > 1152 ? 1024 : (m_iSampleForChannel < 768 ? 768 : m_iSampleForChannel), NEXSOUND_OUTPUT_MODE_SPEAKER) != 0)
	{
		DestroyNexSound(m_hNexSountHandle);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound NexSoundInitialize failed",__LINE__, m_pClipItem->getClipID());
		m_hNexSountHandle = NULL;
		return FALSE;
	}

	m_hNexSountHandle->NexSoundSetParam(Processor::NexSound, ParamCommand::NexSound_Headroom, 0);
	m_hNexSountHandle->NexSoundSetParam(Processor::NexSound, ParamCommand::NexSound_ConvertStereo, 0);

	return TRUE;
}

int CNEXThread_AudioTask::deinitNexSound()
{
	if (m_hNexSountHandle)
	{
		m_hNexSountHandle->NexSoundRelease(Processor::NexSound);
		DestroyNexSound(m_hNexSountHandle);
		m_hNexSountHandle	= 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) NexSound deinit sucess",__LINE__, m_pClipItem->getClipID());
	return TRUE;
}

NXBOOL CNEXThread_AudioTask::saveToPauseBuffer(unsigned char* pBuffer, unsigned int uSize, unsigned int uCTS)
{
	unsigned char* pTemp;
	pTemp = (unsigned char*)nexSAL_MemAlloc(m_uiBuffer4PauseSize + uSize);
	memcpy(pTemp, m_pBuffer4Pause, m_uiBuffer4PauseSize);
	memcpy(pTemp + m_uiBuffer4PauseSize, pBuffer, uSize);

	if(m_pBuffer4Pause)
	{
		nexSAL_MemFree(m_pBuffer4Pause);
	}
	m_pBuffer4Pause = pTemp;
	m_uiBuffer4PauseSize += uSize;	
	m_uiBuffer4PauseCTS = uCTS;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] ID(%d) saveToPauseBuffer (%d, %d)",__LINE__, m_pClipItem->getClipID(), m_uiBuffer4PauseSize, m_uiBuffer4PauseCTS);
	return TRUE;
}

unsigned int CNEXThread_AudioTask::downChannels( unsigned char* pDest, unsigned int iLen, unsigned int* piWrittenPCMSize)
{
	unsigned int uActualSize, uChanIdx;
	unsigned int i;
	int nOutBps;

	nOutBps = 16;

	if(pDest == NULL || iLen == 0)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] downChannels check Buffer(%p, %d)",__LINE__, pDest, iLen);
		return 0;
	}
	//uActualSize ´Â ¼ÀÇÃ¼ö
	uActualSize = iLen * 8 / m_iBitsForSample;
	*piWrittenPCMSize = uActualSize *2;

	// bit º¯°æ ¿Ï·á

	//piWrittenPCMSize Ãâ·Â ¹ÙÀÌÆ® Å©±â
	{

		*piWrittenPCMSize *= 2;
		/* channel down-mix */
		switch (m_iChannels)
		{
			case 3 :
				{
					NXINT16 *pPCM16 = (NXINT16*) pDest;
					NXINT16 *pMixedPCM = (NXINT16*) pDest;
					NXINT32   iPCM32 = 0;

					for (i=0; i<uActualSize; i += m_iChannels)
					{
						NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15);

						iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;

						iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;
					}
				}
				break;

			case 4 :
				{
					NXINT16 *pPCM16 = (NXINT16*) pDest;
					NXINT16 *pMixedPCM = (NXINT16*) pDest;
					NXINT32   iPCM32 = 0;

					for (i=0; i<uActualSize; i += m_iChannels)
					{
						iPCM32 = ( ((NXINT32)pPCM16[i+0]) + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) ) >> 15) );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;

						iPCM32 = ( ((NXINT32)pPCM16[i+1]) + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+3]) ) >> 15) );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;
					}
				}
				break;

			case 5 :		// 5 ch : L,R,C,Ls,Rs
				{
					NXINT16 *pPCM16 = (NXINT16*) pDest;
					NXINT16 *pMixedPCM = (NXINT16*) pDest;
					NXINT32   iPCM32 = 0;

					for (i=0; i<uActualSize; i += m_iChannels)
					{
						NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15 );

						iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+3]) ) >> 15) );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;

						iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+4]) ) >> 15) );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;
					}
				}
				break;
			case 6 :	/* ignore LFE */ // 5.1 ch ·Î : L,R,C,LFE,Ls,Rs
				{
					NXINT16 *pPCM16 = (NXINT16*) pDest;
					NXINT16 *pMixedPCM = (NXINT16*) pDest;
					NXINT32	  iPCM32 = 0;

					for(i=0; i<uActualSize; i += m_iChannels)
					{
						NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15 );

						iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+4]) ) >> 15) );
						if(iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if(iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;

						iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+5]) ) >> 15) );
						if(iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if(iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;
					}
				}
				break;

			case 7 :	/* ignore rLs, rRs */			/* 3/4     : L,R,C,Ls,rLs,rRs,Rs		*/
			case 8 :	/* ignore rLs, rRs, LFE */		/* 3/4+lfe : L,R,C,Ls,rLs,rRs,Rs,LFE	*/
				{
					NXINT16 *pPCM16 = (NXINT16*) pDest;
					NXINT16 *pMixedPCM = (NXINT16*) pDest;
					NXINT32   iPCM32 = 0;

					for (i=0; i<uActualSize; i += m_iChannels)
					{
						NXINT32 CenterGain = ( ((NXINT32)CAL_PCMDEC_MIX_GAIN_3DB_Q15 * pPCM16[i+2]) >> 15 );

						iPCM32 = ( ((NXINT32)pPCM16[i+0]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+3]) ) >> 15) );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;

						iPCM32 = ( ((NXINT32)pPCM16[i+1]) + CenterGain + ((((NXINT32)CAL_PCMDEC_MIX_GAIN_6DB_Q15 * pPCM16[i+6]) ) >> 15) );
						if (iPCM32 > 32767)
							*pMixedPCM++ = 32767;
						else if (iPCM32 < -32768)
							*pMixedPCM++ = -32768;
						else
							*pMixedPCM++ = (NXINT16)iPCM32;
					}
				}
				break;

			default :
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] unKnown channels !!! %d \n", __LINE__, m_iBitsForSample);	
				break;
		}

		*piWrittenPCMSize /= m_iChannels;
	}

	if ((*piWrittenPCMSize%2) != 0)
	{
		*piWrittenPCMSize += 1;
	}

	return NEXCAL_ERROR_NONE;
}

int CNEXThread_AudioTask::checkAudioRenderInfo(CClipAudioRenderInfo* pCurrent, CClipAudioRenderInfo* pPrevious)
{
	if(pCurrent == NULL || pPrevious == NULL)
		return 0;

	if(pCurrent->m_iSpeedCtlFactor != pPrevious->m_iSpeedCtlFactor)
		return 1;

	if(pCurrent->m_bKeepPitch != pPrevious->m_bKeepPitch)
		return 1;

	if(pCurrent->m_iVoiceChangerFactor != pPrevious->m_iVoiceChangerFactor)
		return 1;

	if(pCurrent->m_iPitchIndex != pPrevious->m_iPitchIndex)
		return 1;

	if(pCurrent->m_iCompressorFactor != pPrevious->m_iCompressorFactor)
		return 1;

	if(pCurrent->m_iMusicEffector != pPrevious->m_iMusicEffector)
		return 1;

	if(pCurrent->m_iProcessorStrength != pPrevious->m_iProcessorStrength)
		return 1;
	
	if(pCurrent->m_iBassStrength != pPrevious->m_iBassStrength)
		return 1;

	return 0;	
}

int CNEXThread_AudioTask::updateAudioRenderInfo()
{
	unsigned char* pTemp = NULL;
	unsigned int uTemp = m_iSpeedCtlInBufferRemainSize;
	if(m_iSpeedCtlInBufferRemainSize)
	{
		pTemp = (unsigned char*)nexSAL_MemAlloc(m_iSpeedCtlInBufferRemainSize);
		memcpy(pTemp, m_pSpeedCtlInBuffer, m_iSpeedCtlInBufferRemainSize);
	}
	deinitSpeedCtl();
	deinitMusicEffect();
	deinitCompressor();
	deinitPitchContorl();
	deinitVoiceChanger();
	deinitEnhancedAudioFilter();
	deinitEqualizer();
	initSpeedCtl();
	initMusicEffect();
	initCompressor();
	initPitchContorl();
	initVoiceChanger();
	initEnhancedAudioFilter();
	initEqualizer();
	if(pTemp)
	{
		if(m_pSpeedCtlInBuffer)
		{
			if(m_iSpeedCtlInSize < uTemp)
			{
				nexSAL_MemFree(m_pSpeedCtlInBuffer);
				m_pSpeedCtlInBuffer = (unsigned char*)nexSAL_MemAlloc(uTemp);
			}
			memcpy(m_pSpeedCtlInBuffer, pTemp, uTemp);	
			m_iSpeedCtlInBufferRemainSize = uTemp;
		}
		else
		{
			if(m_pBufferRenderUpdate)
			{
				nexSAL_MemFree(m_pBufferRenderUpdate);
			}
			m_pBufferRenderUpdate = (unsigned char*)nexSAL_MemAlloc(uTemp);
			memcpy(m_pBufferRenderUpdate, pTemp, uTemp);
			m_iBufferRenderUpdateSize = uTemp;
		}
		nexSAL_MemFree(pTemp);		
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] updateAudioRenderInfo %d, %d \n", __LINE__, m_pClipItem->getClipID(), m_pCurrentRenderInfo->muiTrackID);	
	return 0;
}

NXBOOL CNEXThread_AudioTask::GetThumbWithMemory(CNexSource* pSource)
{
	if( pSource == NULL )
		return FALSE;

	while(m_isStopThumb == FALSE && m_bThreadFuncEnd == FALSE)
	{
		nexSAL_TaskSleep(1);
		if(m_PCMVec.size())
			break;          
	}

	NEXCALCodecHandle	hCodecAudio;

	unsigned int	uSamplingRate			= 0;
	unsigned int	uNumOfChannels			= 0;
	unsigned int	uNumOfSamplesPerChannel	= 0;

	NXINT64		uiCTS				= 0;
	unsigned int	uFormatTag			= 0;
	unsigned int	uBitsPerSample		= 0;
	unsigned int	uBlockAlign			= 0;
	unsigned int	uAvgBytesPerSec	= 0;
	unsigned int	uSamplesPerBlock	= 0;
	unsigned int	uEnCodeOpt			= 0;
	unsigned int	uExtraSize			= 0;
	unsigned char*	pExtraData		= NULL;

	NXUINT8*		pDSI			= NULL;
	NXUINT8*		pFrame			= NULL;
	NXUINT32		uDSISize		= 0;
	NXUINT32		uFrameSize		= 0;
	unsigned int*		pTempDSI		= NULL;
	int				iRet				= 0;
	unsigned int		uiReaderRet		= 0;
	int nProfile = 0;
	unsigned int uiAudioObjectType =  pSource->getAudioObjectType();

	NXBOOL				bFrameEnd = FALSE;
	unsigned int			uiDecoderErrRet		= 0;
	unsigned int			uiPCMBufSize		= 0;
	unsigned int                  uPos = 0;

	unsigned int uiAudioDecodeBufSize = 0;
	unsigned char* pAudioDecodeBuf = NULL;
	unsigned int uiTempBufSize = 0;
	unsigned char* pTempBuf = NULL;

	EnvelopDetector			pcmDetector;
	NXBOOL isNextFrameRead = 0;
    
	unsigned int            iUsedCount = 0;
	unsigned int            iSkipCount = 0;    
    
	pSource->getWAVEFormat(&uFormatTag, &uBitsPerSample, &uBlockAlign, &uAvgBytesPerSec, &uSamplesPerBlock, &uEnCodeOpt, &uExtraSize, &pExtraData);
	pSource->getDSI(NXFF_MEDIA_TYPE_AUDIO, &pDSI, &uDSISize );

	pSource->getSamplingRate(&uSamplingRate);
	pSource->getNumberOfChannel(&uNumOfChannels);
	pSource->getSamplesPerChannel(&uNumOfSamplesPerChannel);

	while( 1 )
	{
		uiReaderRet = pSource->getAudioFrame();

		if( uiReaderRet == _SRC_GETFRAME_OK )
		{
 			pSource->getAudioFrameData(&pFrame, &uFrameSize );
			pSource->getAudioFrameCTS(&uiCTS);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Initialize Start(%lld)\n", __LINE__, uiCTS);

			// RYU 20130620 
			if ( (uiAudioObjectType == eNEX_CODEC_A_AAC || uiAudioObjectType == eNEX_CODEC_A_AACPLUS))
			{
				getAACProfile(pFrame, uFrameSize, &nProfile);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory AAC Profile(%d) (%d, 0x%x)\n", __LINE__, nProfile,
					CNexVideoEditor::m_bSupportAACSWCodec,  CNexVideoEditor::m_iSupportAACProfile);
				
				switch( nProfile)
				{
				case 0: // main
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE  && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_MAIN) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory Using Software Codec for AAC Main\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 2: // SSR
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_SSR) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory Using Software Codec for AAC SSR\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 3: // LTP
					if(	CNexVideoEditor::m_bSupportAACSWCodec == TRUE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_LTP) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory Using Software Codec for AAC LTP\n", __LINE__);
						uiAudioObjectType = eNEX_CODEC_A_AAC_S;
					}
					break;
				case 1: // LC
					//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] support AAC LC", __LINE__);
				default :
					//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] AAC Unknown ", __LINE__);
					break;
				}
			}

			hCodecAudio = CNexCodecManager::getCodec(NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, uiAudioObjectType);

			if( hCodecAudio == NULL )
			{
				return FALSE;
			}

			m_uiAudioObjectType = uiAudioObjectType;

			if ( (pSource->getAudioObjectType() == eNEX_CODEC_A_AAC || pSource->getAudioObjectType() == eNEX_CODEC_A_AACPLUS) && pDSI != NULL )
			{
				unsigned int    uSamplingFreqIndex = 0;
				const unsigned int puAACSampleRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};

				NXINT32 isSBR;
				NexCodecUtil_AAC_ParseDSI(pDSI, uDSISize, &uSamplingFreqIndex, &uNumOfSamplesPerChannel, &isSBR);
				if (isSBR)
				{
					m_iAACSbr = isSBR;
				}

				uSamplingFreqIndex = ((pDSI[0] & 0x07) << 1) | ((pDSI[1] & 0x80) >> 7);

				if ( uSamplingFreqIndex > 11 )//Nx_robin__090827 uSamplingFreqIndexê°?ì²´í¬
					uSamplingFreqIndex = 0;
				uSamplingRate = puAACSampleRates[uSamplingFreqIndex];
				uNumOfChannels = (pDSI[1] & 0x78) >> 3;
				if ( uNumOfChannels == 0 )//Nx_robin__090827 DSI????? uNumOfChannels??? 0????? ????????????? File MetaData????? ??????????????.
					pSource->getNumberOfChannel(&uNumOfChannels);
				uBitsPerSample = 16;

				uiAudioObjectType = NexCodecUtil_GetAACType((NEX_CODEC_TYPE)pSource->getAudioObjectType(), uSamplingRate, uNumOfChannels, pFrame, uFrameSize);							

				if(uiAudioObjectType == eNEX_CODEC_A_AACPLUSV2)
					pSource->setAudioObjectType(eNEX_CODEC_A_AACPLUS);
				
				if ( pSource->getAudioObjectType() == eNEX_CODEC_A_AACPLUS )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory This contents is AAC+\n", __LINE__);
					uNumOfSamplesPerChannel = 2048;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory This contents is AAC\n", __LINE__);
					uNumOfSamplesPerChannel = 1024;
				}
			}
			break;
		}
		else if( uiReaderRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d]GetThumbWithMemory Need Buffering....\n", __LINE__);
			nexSAL_TaskSleep(20);
			continue;
		}
		else if( uiReaderRet == _SRC_GETFRAME_OK_LOSTFRAME)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory Remove Lost Frame..\n", __LINE__);
			continue;
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Data Read Error[0x%X].\n", __LINE__, uiReaderRet);
			goto DECODER_INIT_ERROR;
		}
	}

	//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] checkDecoding nexCAL_AudioDecoderInit Frame Data!",  __LINE__);
	//nexSAL_MemDump(pDSI, uDSISize);
	//nexSAL_MemDump(pFrame, uFrameSize);


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Decoder Init Start(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	MPEG_AUDIO_INFO mpa_info;
	memset(&mpa_info, 0x00, sizeof(MPEG_AUDIO_INFO));
	NexCodecUtil_Get_MPEG_Audio_Info( pFrame, uFrameSize, &mpa_info );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] NexCodecUtil_Get_MPEG_Audio_Info %d, %d",  __LINE__, mpa_info.Version, mpa_info.Layer);
	if (mpa_info.Version == NEX_MPEG_AUDIO_VER_1 && mpa_info.Layer == NEX_MPEG_AUDIO_LAYER_2)
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 1);
	else
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 0);

	iRet = nexCAL_AudioDecoderInit(		hCodecAudio,
										(NEX_CODEC_TYPE)pSource->getAudioObjectType(), 
										pDSI, 
										uDSISize, 
										pFrame, 
										uFrameSize, 
										NULL,
										NULL,
										&uSamplingRate,
										&uNumOfChannels, 
										&uBitsPerSample, 
										&uNumOfSamplesPerChannel, 
										NEXCAL_ADEC_MODE_NONE,
										0, // USERDATATYPE
										(NXVOID *)this );

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Decoder Init End(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	uiAudioDecodeBufSize = 384*1024;
	pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(uiAudioDecodeBufSize);
	pTempBuf	= (unsigned char*)nexSAL_MemAlloc(uiAudioDecodeBufSize);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] GetThumbWithMemory Default  PCMBuffer(%p, %d)\n", __LINE__, pAudioDecodeBuf, uiAudioDecodeBufSize);
	
	if( iRet != NEXCAL_ERROR_NONE )
	{
		if ( pTempDSI )
		{
			nexSAL_MemFree(pTempDSI);
			pTempDSI = NULL;
			pDSI = NULL;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Decoder Init Failed(%d)",  __LINE__, iRet);
		goto DECODER_INIT_ERROR;
	}
	if ( pTempDSI )
	{
		nexSAL_MemFree(pTempDSI);
		pTempDSI = NULL;
		pDSI = NULL;
	}

	m_iSampleRate		= uSamplingRate;
	m_iChannels			= uNumOfChannels;
	m_iBitsForSample		= uBitsPerSample;
	m_iSampleForChannel	= uNumOfSamplesPerChannel;

	pSource->setNumOfChannels(m_iChannels);
	pSource->setSamplingRate(m_iSampleRate);
	pSource->setNumOfSamplesPerChannel(m_iSampleForChannel);

	// Case : mantis 9267 when decoder channel info was wrong.
	if( m_iSampleRate == 0 || m_iChannels == 0 || m_iBitsForSample == 0 || m_iSampleForChannel == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] GetThumbWithMemory Wait decode config change",  __LINE__);
	}

	pcmDetector.Init(m_iOutChannels, m_iSampleRate);

	while(1)
	{
		if(m_isStopThumb)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] ID(%d) ADTask Task thumb stop flag set ", __LINE__, m_pClipItem->getClipID());			
			break;
		}

		if(isNextFrameRead)
		{
			if( pSource->getAudioFrame() != _SRC_GETFRAME_OK )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] GetThumbWithMemory get pcm level end Time", __LINE__);
				bFrameEnd = TRUE;
			}
			isNextFrameRead = FALSE;
		}

		if( uiAudioObjectType == eNEX_CODEC_A_AAC_S )
		{
			if( bFrameEnd )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] GetThumbWithMemory EOS received and Exit using software codec", __LINE__);
				break;
			}
		}

		if( bFrameEnd == FALSE)
		{
			if(iSkipCount == 25)
			{
				iUsedCount = 0;
				iSkipCount = 0;
			}
			if(iUsedCount == 5)
			{
				isNextFrameRead = TRUE;
				iSkipCount++;        
				continue;
			}
			pSource->getAudioFrameCTS( &uiCTS );
			pSource->getAudioFrameData( &pFrame, &uFrameSize );
		}
    
		uiPCMBufSize = uiAudioDecodeBufSize;
		nexCAL_AudioDecoderDecode(	hCodecAudio,
									pFrame, 
									uFrameSize, 
									NULL, 
									pAudioDecodeBuf,
									(int *)&uiPCMBufSize,
									(unsigned int)uiCTS,
									NULL,
									bFrameEnd == FALSE?NEXCAL_ADEC_FLAG_NONE:NEXCAL_ADEC_FLAG_END_OF_STREAM,
									&uiDecoderErrRet );

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ADTask.cpp %d]  GetThumbWithMemory After Decode. (Buf %p, BufSize:%d, TS:%lld Ret:0x%x)", 	__LINE__, pAudioDecodeBuf, uiPCMBufSize , uiCTS, uiDecoderErrRet);

		if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_DECODING_SUCCESS))
		{
			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_NEXT_FRAME))
			{
				isNextFrameRead = TRUE;
 				iUsedCount++;
			}
		
			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_CHANGED))
			{
				unsigned int uiSamplingRate = 0;
				unsigned int uiChannels = 0;
				unsigned int uiBitPerSample = 0;
				unsigned int uiNumOfSamplesPerChannel = 0;
				nexCAL_AudioDecoderGetInfo(hCodecAudio , NEXCAL_AUDIO_GETINFO_SAMPLINGRATE, &uiSamplingRate);
				nexCAL_AudioDecoderGetInfo(hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS, &uiChannels);
				nexCAL_AudioDecoderGetInfo(hCodecAudio , NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE, &uiBitPerSample);
				nexCAL_AudioDecoderGetInfo(hCodecAudio , NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL, &uiNumOfSamplesPerChannel);

				if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
				{
					if(CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec == FALSE && m_iChannels == 1)
					{
						uiChannels = m_iChannels;
					}
				}
				pcmDetector.Init(m_iOutChannels, m_iSampleRate);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Output was changed!(S(%d) C(%d) BpS(%d) SC(%d)", 
					__LINE__, uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
			}

			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_EXIST))
			{
				float					fEnvelopRMS			= 0.0;
				float				fEnvelop[m_iChannels];
				int						iPCMRMS				= 0;
				int						iPCM[m_iChannels];
				unsigned int			uiPCMLevelFrameCnt	= 0;
			
				if(m_iChannels > 2 && m_iOutChannels == 2)
				{
					downChannels(pAudioDecodeBuf, uiPCMBufSize, &uiPCMBufSize);
				}

#if 1
				memcpy(pTempBuf + uiTempBufSize, pAudioDecodeBuf, uiPCMBufSize);
				uiTempBufSize += uiPCMBufSize;

				while(uiTempBufSize > m_iSampleForChannel*(m_iOutChannels *m_iBitsForSample / 8))
				{
					pcmDetector.EnvelopDetectProcess((short*)pTempBuf, &fEnvelopRMS, fEnvelop,  m_iSampleForChannel);
					iPCMRMS		= (int)(fEnvelopRMS * 100.0);
					for(int i=0; i<m_iOutChannels; i++)
						iPCM[i] = (int)(fEnvelop[i] * 100.0);
					m_PCMVec2.insert(m_PCMVec2.end(), (unsigned char)iPCMRMS);
					uiTempBufSize -= m_iSampleForChannel*(m_iOutChannels *m_iBitsForSample / 8);
					memmove(pTempBuf, pTempBuf + (m_iSampleForChannel*(m_iOutChannels *m_iBitsForSample / 8)), uiTempBufSize);
					//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Output size(%d)", __LINE__, uiPCMBufSize);
				}
#else
				pcmDetector.EnvelopDetectProcess((short*)pAudioDecodeBuf, &fEnvelopRMS, fEnvelop,  uiPCMBufSize/(m_iOutChannels *m_iBitsForSample / 8));
				iPCMRMS		= (int)(fEnvelopRMS * 100.0);
				for(int i=0; i<m_iOutChannels; i++)
					iPCM[i] = (int)(fEnvelop[i] * 100.0);
				m_PCMVec2.insert(m_PCMVec2.end(), (unsigned char)iPCMRMS);
#endif
			}

			if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Task EOS received", __LINE__);
				break;
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Dec Failed(%p %d %lld Ret:%d)", __LINE__, pFrame, uFrameSize, uiCTS, uiDecoderErrRet);
			goto DECODER_INIT_ERROR;
		}
	}

	if( hCodecAudio )
	{
		CNexCodecManager::releaseCodec(hCodecAudio);
		hCodecAudio = NULL;
	}

	if ( pTempDSI )
	{
		nexSAL_MemFree(pTempDSI);
		pTempDSI = NULL;
		pDSI = NULL;
	}

	if ( pAudioDecodeBuf )
	{
		nexSAL_MemFree(pAudioDecodeBuf);
		pAudioDecodeBuf = NULL;
		pDSI = NULL;
	}

	if ( pTempBuf )
	{
		nexSAL_MemFree(pTempBuf);
		pTempBuf = NULL;
	}

	return TRUE;

DECODER_INIT_ERROR:
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ADTask.cpp %d] GetThumbWithMemory Audio Decoder failed",  __LINE__);
	if( hCodecAudio )
	{
		CNexCodecManager::releaseCodec(hCodecAudio);
		hCodecAudio = NULL;
	}
	if ( pTempDSI )
	{
		nexSAL_MemFree(pTempDSI);
		pTempDSI = NULL;
		pDSI = NULL;
	}

	if ( pAudioDecodeBuf )
	{
		nexSAL_MemFree(pAudioDecodeBuf);
		pAudioDecodeBuf = NULL;
		pDSI = NULL;
	}

	if ( pTempBuf )
	{
		nexSAL_MemFree(pTempBuf);
		pTempBuf = NULL;
	}
    
	return FALSE;	
}
