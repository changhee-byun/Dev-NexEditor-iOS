/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Audio_AudioRenderTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/12	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_AUDIO_AUDIORENDERTASK_H__
#define __NEXVIDEOEDITOR_AUDIO_AUDIORENDERTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "nexQueue.h"
#include "NxMixerAPI.h"
#include "EnvelopDetector.h"

#include "NEXVIDEOEDITOR_WrapFileWriter.h"

#include "nexRALBody_Audio_API.h"

#define AUDIO_RENDER_INVALID_TIME 0xFFFFFFFF

class CNexProjectManager;

#define AUDIO_RENDER_TASK_NAME "NEXVIDEOEDITOR Android Audio Renader task"

#define DEFAULT_PCMBUFFER_SIZE	100*1024

// #define PCM_DEBUG_DUMP

class CAudioTrackInfo : public CNxRef<INxRefObj>
{
public:
	CAudioTrackInfo()
	{
		m_uiTrackID				= 0;
		m_isResampling			= FALSE;
		m_uiSamplingRate		= 0;
		m_uiChannels			= 0;
		m_uiBitsForSample		= 0;
		m_uiSampleForChannel	= 0;
		m_uiLastTime			= 0;

		m_AudioQueue			= NULL;
		m_uiVolume				= 0;
		m_uiStartTime			= 0;
		m_uiEndTime				= 0;
		m_bRenderRegistered		= FALSE;
		m_bRenderFlag			= FALSE;

		m_bDecodingDone			= FALSE;
		m_iWaitCountAtLastTime	= 0;
		m_bTrackValidTime		= FALSE;

		m_iPanFactor[0]			= -111;
		m_iPanFactor[1]			= 111;

#ifdef FOR_TEST_AUDIO_TRACK_DUMP
		m_pPCMDUMP              = NEXSAL_INVALID_HANDLE;
#endif
		m_ClipAudioRenderInfoVec.clear();
	}

	~CAudioTrackInfo()
	{
		for(int i = 0 ; i < m_ClipAudioRenderInfoVec.size() ; i++)
			m_ClipAudioRenderInfoVec[i]->Release();
		m_ClipAudioRenderInfoVec.clear();	
		if( m_AudioQueue )
		{
			nexQueue_Destroy(m_AudioQueue);
			m_AudioQueue = NULL;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack %d] nexQueue_Destroy", __LINE__);
		}

#ifdef FOR_TEST_AUDIO_TRACK_DUMP
		if( m_pPCMDUMP != NEXSAL_INVALID_HANDLE )
		{
			nexSAL_FileClose(m_pPCMDUMP);
			m_pPCMDUMP = NEXSAL_INVALID_HANDLE;
		}
#endif
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack %d] ~~~~~~CAudioTrackInfo", __LINE__);
		
	}

	int setAudioTrackInfo(	unsigned int uiTrackID, 
							unsigned int uiSamplingRate, 
							unsigned int uiChannels, 
							unsigned int uiBitForSample, 
							unsigned int uiSampleForChannel, 
							unsigned int uiStartTime,
							unsigned int uiEndTime,
							unsigned int iVolume,
							int* pPanFactor)
	{
		m_uiTrackID				= uiTrackID;
		m_uiSamplingRate		= uiSamplingRate;
		m_uiChannels			= uiChannels;
		m_uiBitsForSample		= uiBitForSample;
		m_uiSampleForChannel	= uiSampleForChannel;
		m_uiVolume				= iVolume;

		if (m_uiChannels == 1)
		{
			if (pPanFactor[0] < -100 || pPanFactor[0] > 100)
			{
				m_iPanFactor[0] = 0;
				m_iPanFactor[1] = 0;
			}
			else
			{
				m_iPanFactor[0] = pPanFactor[0];
				m_iPanFactor[1] = pPanFactor[1];
			}
		}
		else if (m_uiChannels == 2)
		{
			for(int i=0; i<m_uiChannels; i++)
			{
				if (pPanFactor[i] < -100 || pPanFactor[i] > 100)
					m_iPanFactor[i] = (i==0?-100:100);
				else
					m_iPanFactor[i] = pPanFactor[i];
			}
		}

		m_uiStartTime			= uiStartTime;
		m_uiEndTime				= uiEndTime;
		m_bTrackValidTime		= FALSE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack %d] setAudioTrackInfo(TrackID(%d), S(%d) C(%d) BpS(%d) SpC(%d) StartTime(%d) Vol(%d) PanFactor(%d, %d))",
				__LINE__, uiTrackID, uiSamplingRate, uiChannels, uiBitForSample, uiSampleForChannel, uiStartTime, iVolume, m_iPanFactor[0], m_iPanFactor[1]);

		m_uiLastTime	= 0;

		
		if(m_uiSampleForChannel < 1024)
		{
			m_uiTotalSize	= m_uiChannels * 1024* (m_uiBitsForSample>>3);
		}
		else
		{
			m_uiTotalSize	= m_uiChannels * m_uiSampleForChannel * (m_uiBitsForSample>>3);
		}

		m_uiBufferDuration =  m_uiTotalSize*42*1000/m_uiSamplingRate/m_uiChannels/(m_uiBitsForSample>>3);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack %d]  before create audio Queue(0x%x)(%d, D:%d)", __LINE__, m_AudioQueue, m_uiTotalSize, m_uiBufferDuration);
		m_AudioQueue	= nexQueue_Create( m_uiTotalSize * 42);
		if( m_AudioQueue == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack %d] audio queue create fail", __LINE__);
			return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack %d] create audio Queue(0x%x, 0x%x)(%d)", __LINE__, m_AudioQueue, m_AudioQueue->m_pBuffer, m_uiTotalSize);

#ifdef FOR_TEST_AUDIO_TRACK_DUMP
		if( m_pPCMDUMP != NEXSAL_INVALID_HANDLE )
		{
			nexSAL_FileClose(m_pPCMDUMP);
			m_pPCMDUMP = NEXSAL_INVALID_HANDLE;
		}

		char strFile[256];
		sprintf(strFile, "/sdcard/%d_Clip_Track.pcm", uiTrackID);
		nexSAL_FileRemoveA(strFile);
		m_pPCMDUMP = nexSAL_FileOpen(strFile, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
#endif
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack %d] setAudioTrackInfo Out", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	NXBOOL setClipRenderInfoVec(CClipAudioRenderInfo* pClipRenderInfo)
	{
		if( pClipRenderInfo == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[AudioTrack.cpp %d] setClipRenderInfoVec is failed because param is null(%d)", __LINE__, pClipRenderInfo);
			return FALSE;
		}

		NXBOOL bInitSucess = TRUE;
		SAFE_ADDREF(pClipRenderInfo);
		m_ClipAudioRenderInfoVec.insert(m_ClipAudioRenderInfoVec.end(), pClipRenderInfo);
		return bInitSucess;
	}

	CClipAudioRenderInfo* getActiveRenderInfo(unsigned int uiTime)
	{
		for(int i = 0; i < m_ClipAudioRenderInfoVec.size(); i++)
		{
			if( m_ClipAudioRenderInfoVec[i]->mStartTime <= uiTime && m_ClipAudioRenderInfoVec[i]->mEndTime >= uiTime )
			{
				return m_ClipAudioRenderInfoVec[i];
			}
		}
		
		return NULL;
	}
	
	int EnqueueAudioTrackData(unsigned char* pPCMBuf, unsigned int uiPCMSize, unsigned int uiTime)
	{
		unsigned char* pBuf =  NULL;

		NexQueueRet	ret = nexQueue_GetEnQueueBuf(m_AudioQueue, &pBuf, uiPCMSize, 40);
		if( ret != NEXQUEUE_RET_SUCCESS )
		{
			return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
		}

		memcpy(pBuf, pPCMBuf, uiPCMSize);
		nexQueue_EnQueue(m_AudioQueue, pBuf, uiPCMSize);
#ifdef FOR_TEST_AUDIO_TRACK_DUMP
		if( m_pPCMDUMP != NEXSAL_INVALID_HANDLE  )
		{
			nexSAL_FileWrite(m_pPCMDUMP, pBuf, uiPCMSize);
		}
#endif

		m_TimeStampQueue.push(uiTime);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	int getDequeueAudioTrackBuf(unsigned char** pPCMBuf, unsigned int uiPCMSize)
	{
		NexQueueRet	ret = nexQueue_GetDeQueueBuf(m_AudioQueue, pPCMBuf, uiPCMSize, 100);
		if( ret != NEXQUEUE_RET_SUCCESS )
		{
				return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
		}
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	int dequeueAudioTrackBuf(unsigned char* pPCMBuf, unsigned int uiPCMSize)
	{
		NexQueueRet	ret = nexQueue_DeQueue(m_AudioQueue, pPCMBuf, uiPCMSize);
		if( ret != NEXQUEUE_RET_SUCCESS )
		{
			return NEXVIDEOEDITOR_ERROR_GENERAL;
		}
		if( m_TimeStampQueue.empty() == FALSE )
			m_TimeStampQueue.pop();
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	NXBOOL existAudioTrackData()
	{
		NexQueueRet	ret = nexQueue_ExistData(m_AudioQueue);
		if( ret == NEXQUEUE_RET_SUCCESS )
		{
			return TRUE;
		}
		
		return FALSE;
	}

	void clearAudioTrackData()
	{
		nexQueue_Clear(m_AudioQueue);
	}

	void setDecodingDone(NXBOOL bEnd)
	{
		m_bDecodingDone = bEnd;
	}

	NXBOOL getDecodingDone()
	{
		return m_bDecodingDone;
	}	

	void clearWaitCountAtLastTime()
	{
		m_iWaitCountAtLastTime = 0;
	}
	
	void setWaitCountAtLastTime()
	{
		m_iWaitCountAtLastTime++;
	}

	int getWaitCountAtLastTime()
	{
		return m_iWaitCountAtLastTime;
	}		

	int getCurrentTime()
	{
		if( m_TimeStampQueue.size() <= 0 )
			return m_uiLastTime;
		m_uiLastTime = m_TimeStampQueue.front();
		return m_uiLastTime;
	}

	int getBufferDuration()
	{
		return m_uiBufferDuration;
	}


	unsigned int		m_uiTrackID;
	NXBOOL				m_isResampling;
	unsigned int		m_uiSamplingRate;
	unsigned int		m_uiChannels;
	unsigned int		m_uiBitsForSample;
	unsigned int		m_uiSampleForChannel;
	unsigned int		m_uiTotalSize;
	unsigned int		m_uiBufferDuration;
	unsigned int		m_uiVolume;
	int					m_iPanFactor[2];

	NXBOOL				m_bRenderRegistered;
	unsigned int 		m_uiStartTime;
	unsigned int		m_uiEndTime;
	unsigned int		m_uiLastTime;
	NXBOOL				m_bRenderFlag;

	NXBOOL				m_bDecodingDone;
	int					m_iWaitCountAtLastTime;
	NXBOOL				m_bTrackValidTime;

	PNexQueueHandle		m_AudioQueue;
	std::queue<unsigned int> m_TimeStampQueue;

	CClipAudioRenderInfoVec		m_ClipAudioRenderInfoVec;

#ifdef FOR_TEST_AUDIO_TRACK_DUMP
	NEXSALFileHandle	m_pPCMDUMP;
#endif
};

typedef std::vector<CAudioTrackInfo*>	AudioTrackVec;
typedef AudioTrackVec::iterator			AudioTrackVecIter;

class CNEXThread_AudioRenderTask : public CNEXThreadBase
{

public:
	CNEXThread_AudioRenderTask( void* pAudioRenderFuncs, int iClipCount );
	virtual ~CNEXThread_AudioRenderTask( void );

	virtual const char* Name()
	{	return AUDIO_RENDER_TASK_NAME;
	}

	virtual unsigned int getCurrentTime();
	
	unsigned int getInternalAudioTime();
	unsigned int getAudioSessionID();

	void setProjectManager(CNexProjectManager* pProjectManager) {m_pProjectManager = pProjectManager;}
	void setAudioTrackHandle(void* pAudioTrack);

	void sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3);
	void setDirectExport(NXBOOL bDirectExport);
	void setPauseForVisual();
	void setResumeForVisual();
	NXBOOL getPausedForVisual();

protected:
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	virtual void ProcessPlayState();
	virtual void ProcessRecordState();
	virtual void ProcessPauseState();


	int audioMix(unsigned char* pBuf, unsigned int uiSize, unsigned int* pCTS);
	int checkValidAudioTrack();

	CNexProjectManager* m_pProjectManager;
public:
	int					m_iTemp;
	NXBOOL				m_isVideoStarted;
	
	unsigned int		m_uiSamplingRate;
	unsigned int		m_uiChannels;
	unsigned int		m_uiBitsForSample;
	unsigned int		m_uiSampleForChannel;
	unsigned int		m_uiTotalSampleCount;

	unsigned int		m_uiPreviewStartTime;
	unsigned int		m_uiTotalTime;

	unsigned char*		m_pPCMBuffer;

	AudioTrackVec		m_AudioTrack;
	PNXMIXERHANDLE		m_hAudioMixer;

	CNexExportWriter*		m_pFileWriter;

	short*				m_pAudioMuteBuffer;

	int					m_iMuteAudio;
	int					m_iManualVolumeControl;
	unsigned int		m_uiLastRenderTime;

#ifdef FOR_TEST_AUDIO_MIX_DUMP
	NEXSALFileHandle			m_pPCMOutFile;
#endif
	CNexLock			m_AudioUserDataLock;
	void*		m_uiAudioUserData;
	unsigned int		m_uiInternalAudioCTS;
	unsigned long long	m_ullOutPCMTotalCount;

	NXBOOL				m_bRenderPause;
	
	AUDIO_RALBODY_FUNCTION_ST*	m_pRALAudioHandle;
	
	int					m_iFadeInTime;
	int					m_iFadeOutTime;
	int					m_iPreviousFadeVolume;
	int					m_iValidTrackCount;
	
	CNexLock			m_Lock;

	int					m_iProjectVolume;
	int					m_iMasterVolume;
	int					m_iSlaveVolume;

	float				fPeakMeterRMS;
	float				fPeakMeter[2];
	EnvelopDetector		m_PeakMeterDetector;
	NXBOOL		m_bDirectExport;
	NXBOOL		m_bSetPauseForVisual;
	NXBOOL		m_bPausedForVisual;
};

#endif // __NEXVIDEOEDITOR_AUDIO_AUDIORENDERTASK_H__
