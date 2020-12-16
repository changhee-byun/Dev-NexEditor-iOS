/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_AudioTask.h
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

#ifndef __NEXVIDEOEDITOR_AUDIOTASK_H__
#define __NEXVIDEOEDITOR_AUDIOTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_Thumbnail.h"
#include <vector>
#include "NexSound.h"
#include "NxResamplerAPI.h"

#include "NEXVIDEOEDITOR_Android_AudioRenderTask.h"

#define AUDIO_TASK_NAME "NEXVIDEOEDITOR Audio task"
#define AUDIO_DECODER_BUFFER_MAXSIZE 10*1024

//#define _ADTASK_PCM_DUMP_

class CNEXThread_AudioTask : public CNEXThreadBase
{
public:
	CNEXThread_AudioTask( void );
	virtual ~CNEXThread_AudioTask( void );

	virtual const char* Name()
	{	return AUDIO_TASK_NAME;
	}

	NXBOOL setClipItem(CClipItem* pClipItem);
	NXBOOL setClipBaseTime(unsigned int uiBaseTime);
	NXBOOL setSource(CNexSource* pSource);
	NXBOOL setFileWriter(CNexExportWriter* pFileWriter);
	NXBOOL setAudioRenderer(CNEXThread_AudioRenderTask* pAudioRenderer);
	NXBOOL setThumbnailHandle(CThumbnail* pThumbnail);
	NXBOOL setThumbnailMultiThreadTime(unsigned int uTime);
	
	NXBOOL setAutoEnvelop(NXBOOL bAutoEnvelop = FALSE);
	NXBOOL setStartCTS(unsigned int uiCTS);

	virtual void End( unsigned int uiTimeout );
	virtual void WaitTask();

	NXBOOL setSpeedFactor(int iFactor);
	NXBOOL setKeepPitch(NXBOOL bFactor);
	NXBOOL setPitchFactor(int iFactor);
	NXBOOL setCompressorFactor(int iFactor);
	NXBOOL setMusicEffectFactor(int iFactor, int iProcessorStrength, int iBassStrength);
	NXBOOL setVoiceChangerFactor(int iFactor);
	NXBOOL setPCMRenderWaitFlag(NXBOOL bWait);
	NXBOOL setEnhancedAudioFilter(char* pEnhancedAudioFilter);
	NXBOOL setEqualizer(char* pEqualizer);

	NXBOOL checkDecoding();

	NXBOOL setStopThumb();
	NXBOOL GetThumbWithMemory(CNexSource* pSource);
	NXBOOL setUseSkipCount(unsigned int uUseCount, unsigned int uSkipCount);

protected:
	virtual int OnThreadMain( void );

private:
	NXBOOL initAudioDecoder(unsigned int uiStartTS);
	NXBOOL deinitAudioDecoder();

	NXBOOL reinitAudioTrack(unsigned int  uiSamplingRate, unsigned int uiChannels, unsigned int uiBitsPerSample, unsigned int uiNumOfSamplesPerChannel);

	NXBOOL registTrack();
	NXBOOL deregistTrack();

	void initAutoEnvelopVolume(unsigned int uiCTS);
	NXBOOL checkAutoEnvelopProcess(unsigned int uiCTS);

	void processEnvelop(short* pPCM, int iPCMCount, unsigned int uiCTS);
	void processEnvelopForRenderInfo(short* pPCM, int iPCMCount, unsigned int uiCTS);

	NXBOOL initResampler();
	NXBOOL deinitResampler();
	NXBOOL getResamplerOutBufferSize(unsigned char* pInBuf, unsigned int uInBufSize, unsigned int* pOutSize);
	int processResampler(unsigned char** ppInPCM, unsigned int uiPCMSize);
	NXBOOL initSpeedCtl();
	NXBOOL deinitSpeedCtl();
	int processNexSound(unsigned char* pInPCM, int iInPCMSize, unsigned int uiCTS);
	NXBOOL isRepeat(unsigned int uiTime);
	NXBOOL initMusicEffect();
	NXBOOL deinitMusicEffect();
	NXBOOL initPitchContorl();
	NXBOOL deinitPitchContorl();
	NXBOOL initCompressor();
	NXBOOL deinitCompressor();
	NXBOOL initVoiceChanger();
	NXBOOL deinitVoiceChanger();
	NXBOOL initEnhancedAudioFilter();
	NXBOOL deinitEnhancedAudioFilter();
	NXBOOL initEqualizer();
	NXBOOL deinitEqualizer();	
	int initNexSound();
	NXBOOL deinitNexSound();
	NXBOOL saveToPauseBuffer(unsigned char* pBuffer, unsigned int uSize, unsigned int uCTS);
	unsigned int downChannels( unsigned char* pDest, unsigned int iLen, unsigned int* piWrittenPCMSize);
	int checkAudioRenderInfo(CClipAudioRenderInfo* pCurrent, CClipAudioRenderInfo* pPrevious);
	int updateAudioRenderInfo();

public:
	int				m_iTemp;
	CAudioTrackInfo*	m_pAudioTrack;

private:
	CClipItem*			m_pClipItem;
	CNexSource*			m_pSource;	
	CNexExportWriter*		m_pFileWriter;
	CNEXThread_AudioRenderTask* m_pAudioRenderer;

	int					m_isStopThumb;

	NEXCALCodecHandle	m_hCodecAudio;
	unsigned int			m_uiAudioObjectType;
	
	CThumbnail*			m_pThumbnail;
	unsigned int			m_uMultiThreadTime;
	
	NXBOOL				m_bAutoEnvelop;
	int					m_iAutoEnvelopVol;
	int					m_iAutoEnvelopVolTemp;

	int					m_iTrackID;
	int					m_iSampleRate;
	int					m_iChannels;
	int					m_iOutChannels;
	int					m_iBitsForSample;
	int					m_iSampleForChannel;
	int					m_iAACSbr;
	unsigned int		m_uiOutputSampleForChannel;

	NXBOOL				m_isNextFrameRead;

	unsigned char*		m_pAudioDecodeBuf;
	unsigned int			m_uiAudioDecodeBufSize;
	unsigned int			m_uiClipBaseTime;

	unsigned long long		m_ullRenderPCMSize;
	unsigned int			m_uiRenderPCMDuration;

	int					m_iSpeedFactor;
	int					m_iSpeedCtlInSize;
	int					m_iSpeedCtlOutputSize;

	int					m_iVoiceChangerFactor;

	int					m_iPitchIndex;
	int					m_iCompressorFactor;
	int 				m_iMusicEffector;
	int					m_iProcessorStrength;
	int					m_iBassStrength;
	NXBOOL				m_bKeepPitch;

	char*				m_pEnhancedAudioFilter;
	char*				m_pEqualizer;

	NXBOOL				m_isNeedResampling;
	unsigned int		m_uLastInputPCMSize;
	unsigned int		m_uResamplerOutputBufSize;
	unsigned char*		m_pResamplerOutBuffer;

	LP_INEXSOUND		m_hNexResamplerHandle;
	LP_INEXSOUND		m_hNexSountHandle;
	unsigned char*		m_pSpeedCtlInBuffer;
	int					m_iSpeedCtlInBufferRemainSize;

	unsigned char*		m_pSpeedCtlOutBuffer;

	unsigned int			m_uiAudioDecoderInitTime;

	unsigned int			m_uiClipEndCTS;
	unsigned int			m_uiClipStartCTS;
	PNXRESAMPLERHANDLE	m_hResamplerForSpeed;
#ifdef FOR_TEST_AUDIOENVELOP
	// RYU 201130604
	int					m_iAudioEnvelopSize;
	int					m_iAudioEnvelopIndex;
	unsigned int			m_iAudioEnvelopStartCTS;
	unsigned int			m_iAudioEnvelopEndCTS;
	unsigned int			m_iAudioEnvelopStartVolume;
	unsigned int			m_iAudioEnvelopEndVolume;
	NXBOOL				m_bUpdateAudioEnvelop;
	int					m_iAudioEnvelopPrevVolume;
 	unsigned int 			m_uiVolume;
	unsigned int			m_uiFirstFrameCTS;
	unsigned int			m_uiStartCTSFromSeek;
#endif

#ifdef FOR_TEST_AUDIO_DEC_DUMP
    NEXSALFileHandle 		m_pDecPCMDump;
#endif
#ifdef FOR_TEST_AUDIO_RESAMPLER_DUMP
    NEXSALFileHandle 		m_pResamplerPCMDump;
#endif
#ifdef FOR_TEST_AUDIO_SPEED_DUMP
    NEXSALFileHandle 		m_pSpeedPCMDump;
#endif

	std::vector<unsigned char>		m_PCMVec;
	std::vector<unsigned char>		m_PCMVec2;

	NXBOOL				m_bWaitPCMRender;

	unsigned char*		m_pBuffer4Pause;
	unsigned int			m_uiBuffer4PauseSize;
	unsigned int			m_uiBuffer4PauseCTS;

	int					m_iExportSamplingRate;

	CClipAudioRenderInfo* m_pPreviousRenderInfo;
	CClipAudioRenderInfo* m_pCurrentRenderInfo;	

	unsigned char*		m_pBufferRenderUpdate;
	int					m_iBufferRenderUpdateSize;
	unsigned int			m_uiRenderInfoStartCTS;

	unsigned int		m_uBufferSizeForSpeed;
	unsigned char*		m_pBufferForSpeed;
	
	unsigned int			m_uiLevelForSpeed;

	unsigned int			m_uUseCount;
	unsigned int			m_uSkipCount;	
};

#endif // __NEXVIDEOEDITOR_AUDIOTASK_H__
