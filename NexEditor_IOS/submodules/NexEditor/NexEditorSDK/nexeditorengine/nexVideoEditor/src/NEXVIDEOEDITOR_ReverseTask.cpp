/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ReverseTask.cpp
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

// #include <GLES2/gl2.h>
// #include <GLES2/gl2ext.h>

#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_ReverseTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"

CNEXThread_ReverseTask::CNEXThread_ReverseTask( void ) : 
	m_perfVideoY2UV2YUV((char*)"ReverseY2UVAtoYUV420"),
	m_perfGLDraw((char*)"ReverseGLDraw"),
	m_perfGetPixels((char*)"ReverseGetPixels"),
	m_perfSwapbuffer((char*)"ReverseSwapBuffer"),
	m_perfDrawAll((char*)"ReverseDrawAll")
{
	m_pTranscodingTask		= NULL;

	m_pUserData = NULL;

	m_hCodecAudio = NULL;
	m_isResampling = 0;
	m_uiAudioDecodeBufSize = 0;
	m_pAudioDecodeBuf = NULL;
	m_uiAudioReSampleBufSize = 0;
	m_pAudioReSampleBuf = NULL;
	m_hNexResamplerHandle = NULL;
	m_isNextVideoFrameRead =  FALSE;
	m_isNextAudioFrameRead = FALSE;

	m_uiTotalPlayTime = 0;
	m_isVideo = 0;
	m_isAudio = 0;

	m_iFlag = 0;

	m_pFileReader = NULL;
	m_pFileWriter = NULL;

	m_bUserCancel = FALSE;

	m_pPCMBuffer = NULL;
	m_uPCMBufferSize = 0;
	m_iErrorType = 0;
	m_bAudioFinished = 0;
	m_uEncodingTime = 0;
	m_uEncodingCount = 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d]CNEXThread_ReverseTask Create Done", __LINE__);
}

CNEXThread_ReverseTask::CNEXThread_ReverseTask( CNEXThread_TranscodingTask* pPM ) : 
	m_perfVideoY2UV2YUV((char*)"ReverseY2UVAtoYUV420"),
	m_perfGLDraw((char*)"ReverseGLDraw"),
	m_perfGetPixels((char*)"ReverseGetPixels"),
	m_perfSwapbuffer((char*)"ReverseSwapBuffer"),
	m_perfDrawAll((char*)"ReverseDrawAll")
{
	SAFE_ADDREF(pPM);
	m_pTranscodingTask = pPM;

	m_pUserData = NULL;

	m_hCodecAudio = NULL;
	m_uiAudioDecodeBufSize = 0;
	m_pAudioDecodeBuf = NULL;
	m_isResampling = 0;
	m_uiAudioReSampleBufSize = 0;
	m_pAudioReSampleBuf = NULL;
	m_hNexResamplerHandle = NULL;

	m_isNextVideoFrameRead =  FALSE;
	m_isNextAudioFrameRead = FALSE;

	m_uiTotalPlayTime = 0;
	m_isVideo = 0;
	m_isAudio = 0;

	m_iFlag = 0;

	m_pFileReader = NULL;
	m_pFileWriter = NULL;

	m_bUserCancel = FALSE;

	m_pPCMBuffer = NULL;
	m_uPCMBufferSize = 0;
	m_iErrorType = 0;
	m_bAudioFinished = 0;
	m_uEncodingTime = 0;
	m_uEncodingCount = 0;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d]CNEXThread_ReverseTask Create Done", __LINE__);
}

CNEXThread_ReverseTask::~CNEXThread_ReverseTask( void )
{
	deinitAudioDecoder();
	deinitFileReader();

	if( m_pUserData )
	{
		nexSAL_MemFree(m_pUserData);
		m_pUserData = NULL;
	}

	SAFE_RELEASE(m_pFileReader);
	SAFE_RELEASE(m_pFileWriter);
	SAFE_RELEASE(m_pTranscodingTask);

	if(m_pPCMBuffer)
	{
		nexSAL_MemFree(m_pPCMBuffer);
		m_pPCMBuffer = NULL;
		m_uPCMBufferSize = 0;
	}

	if( m_pAudioDecodeBuf )
	{
		nexSAL_MemFree(m_pAudioDecodeBuf);
		m_pAudioDecodeBuf = NULL;
		m_uiAudioDecodeBufSize = 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d]~~~~CNEXThread_ReverseTask Destroy Done", __LINE__ );
}

int CNEXThread_ReverseTask::setReverseInfo(const char* pTempFile, unsigned int uStartTime, unsigned int uEndTime)
{
	m_uStartTime = uStartTime;
	m_uEndTime = uEndTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] setReverseInfo start %d, end %d", __LINE__, m_uStartTime, m_uEndTime);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNEXThread_ReverseTask::setFileReader(CNexFileReader* pFileReader)
{
	if( pFileReader == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pFileReader);
	m_pFileReader = pFileReader;
	SAFE_ADDREF(m_pFileReader);

	initFileReader();

	return TRUE;
}

NXBOOL CNEXThread_ReverseTask::setFileWriter(CNexExportWriter* pFileWriter)
{
	if( pFileWriter == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pFileWriter);
	m_pFileWriter = pFileWriter;
	SAFE_ADDREF(m_pFileWriter);

	return TRUE;
}

int CNEXThread_ReverseTask::getErrorCheck()
{
	return m_iErrorType;
}

int CNEXThread_ReverseTask::getAudioFinished()
{
	return m_bAudioFinished;
}

int CNEXThread_ReverseTask::getAudioEncodingTime()
{
	return m_uEncodingTime;
}

int	CNEXThread_ReverseTask::initFileReader()
{
	NXUINT32				uTotalPlayTime	= 0;
	NEXVIDEOEDITOR_ERROR	eRet			= NEXVIDEOEDITOR_ERROR_NONE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] initFileReader In", __LINE__);
	unsigned int uFormatTag = 0;
	unsigned int uBitsPerSample = 0;
	unsigned int uBlockAlign = 0;
	unsigned int uAvgBytesPerSec = 0;
	unsigned int uSamplesPerBlock = 0;
	unsigned int uEnCodeOpt = 0;
	unsigned int uExtraSize = 0;
	unsigned char ExtraData[256];
	unsigned char* pExtraData = ExtraData;
	memset(pExtraData, 0, 256);

	m_pFileReader->getTotalPlayTime(&uTotalPlayTime);
	m_uiTotalPlayTime = uTotalPlayTime;
 
	if( m_pFileReader->isAudioExist())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] before Audio Sample(%d) Channels(%d), BitsPerSample(%d) SamplePerCh(%d)",
				__LINE__, m_uiSampleRate, m_uiChannels, m_uiBitsPerSample, m_uiSamplePerChannel);

		m_pFileReader->getWAVEFormat(&uFormatTag, &uBitsPerSample, &uBlockAlign, &uAvgBytesPerSec, &uSamplesPerBlock, &uEnCodeOpt, &uExtraSize, &pExtraData);
		m_pFileReader->getDSI(NXFF_MEDIA_TYPE_AUDIO, &m_pDSI, &m_uiDSISize );
		m_pFileReader->getSamplingRate(&m_uiSampleRate);
		m_pFileReader->getNumberOfChannel(&m_uiChannels);
		m_pFileReader->getSamplesPerChannel(&m_uiSamplePerChannel);

		// m_uiDSISize = 2;

		m_pFileReader->m_uiAudioFrameInterval		= _GetFrameInterval(m_uiSamplePerChannel, m_uiSampleRate);
		m_pFileReader->m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiSamplePerChannel, m_uiSampleRate);
		m_pFileReader->m_uiMaxAudioFrameInterval	= (m_pFileReader->m_uiAudioFrameInterval*3) >> 1;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio Sample(%d) Channels(%d), BitsPerSample(%d) SamplePerCh(%d)",
				__LINE__, m_uiSampleRate, m_uiChannels, m_uiBitsPerSample, m_uiSamplePerChannel);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio is not exist", __LINE__);

	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] initFileReader Out(%p)", __LINE__, m_pFileReader);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int	CNEXThread_ReverseTask::deinitFileReader()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] deinitFileReader In", __LINE__);
	SAFE_RELEASE(m_pFileReader);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] deinitFileReader Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNEXThread_ReverseTask::initAudioDecoder(unsigned int uiStartTS)
{
	if( m_pFileReader == NULL )
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;

	if( m_pFileReader->isAudioExist() == FALSE )
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;

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
	int				iRet				= 0;
	unsigned int		uiReaderRet		= 0;
	int nProfile = 0;
	unsigned int uiAudioObjectType =  m_pFileReader->m_uiAudioObjectType;


	m_pFileReader->getWAVEFormat(&uFormatTag, &uBitsPerSample, &uBlockAlign, &uAvgBytesPerSec, &uSamplesPerBlock, &uEnCodeOpt, &uExtraSize, &pExtraData);
	m_pFileReader->getDSI(NXFF_MEDIA_TYPE_AUDIO, &pDSI, &uDSISize );

	m_pFileReader->getSamplingRate(&uSamplingRate);
	m_pFileReader->getNumberOfChannel(&uNumOfChannels);
	m_pFileReader->getSamplesPerChannel(&uNumOfSamplesPerChannel);


	while( 1 )
	{
		uiReaderRet = m_pFileReader->getAudioFrame();

		if( uiReaderRet == _SRC_GETFRAME_OK )
		{
 			m_pFileReader->getAudioFrameData(&pFrame, &uFrameSize );
			m_pFileReader->getAudioFrameCTS(&uiCTS);
			
			/*
			**Nx_robin__110111 HLS¿¡¼­ ResetÇÏ´Â °æ¿ì¸¸ uiStartTS°¡ 0ÀÌ ¾Æ´Ï¶ó¼­ ¾Æ·¡ Á¶°ÇÀ» Ã¼Å©ÇÑ´Ù. 
			** Video Only¿¡¼­ A/V·Î ÀüÈ¯µÇ´Âµ¥, InitÀ» À§ÇØ¼­ ÀÐ¾î¿Â Audio CTS°¡, ÀÌ¹Ì PlayÇÑ ºÎºÐÀÌ¶ó¸é, µ¥ÀÌÅÍ¸¦ ÀÐ¾î¼­ ¹ö¸°´Ù.
			** Video Only·Î 5ÃÊ±îÁö PlayÇÑÈÄ¿¡ A/V·Î ÀüÈ¯µÇ¾ú´Âµ¥, Audio Ã¹¹øÂ° µ¥ÀÌÅÍ°¡ 2ÃÊ¿´´Ù¸é, 2~5ÃÊ±îÁöÀÇ µ¥ÀÌÅÍ¸¦ ÀÐ¾î¼­ ¹ö¸°´Ù.
			*/
			if ( uiStartTS > uiCTS )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ReverseTask.cpp %d] Discard Audio Data(%u), Target(%u)\n", __LINE__, uiCTS, uiStartTS);
				continue;
			}
			else if( uiStartTS+300 < uiCTS ) //[shoh][2011.08.24] While Audio DTS is smaller than the first frame CTS, audio needs mute process.
			{
				// hPlayer->m_uRealStartCTS = uiCTS;
				uiCTS = uiStartTS;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ReverseTask.cpp %d] Audio Initialize Start(%u), Target(%u)\n", __LINE__, uiCTS, uiStartTS);
			}

			if( m_hCodecAudio )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ReverseTask.cpp %d] Audio Initialize %p\n", __LINE__, m_hCodecAudio);			
				nexCAL_AudioDecoderDeinit( m_hCodecAudio );
				CNexCodecManager::releaseCodec( m_hCodecAudio );
				m_hCodecAudio = NULL;
			}

			m_hCodecAudio = CNexCodecManager::getCodec(NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, uiAudioObjectType);

			if( m_hCodecAudio == NULL )
			{
				goto DECODER_INIT_ERROR;
			}

			break;
		}
		else if( uiReaderRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ReverseTask.cpp %d]Need Buffering....\n", __LINE__);
			nexSAL_TaskSleep(20);
			continue;
		}
		// JDKIM 2011/02/03 : Lost FrameÀÌ ÀÖ´Â °æ¿ì OK°¡ µÉ ¶§±îÁö ¹ö¸°´Ù.
		else if( uiReaderRet == _SRC_GETFRAME_OK_LOSTFRAME)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ReverseTask.cpp %d] Remove Lost Frame..\n", __LINE__);
			continue;
		}
		
		/*
		else if( uiReaderRet == _SRC_GETFRAME_END)
		{
			NXBOOL bRepeat = isRepeat(uiCTS == 0 ? uiStartTS + 500 : uiCTS + 500);
			if( bRepeat )
			{
				if( m_pFileReader->setRepeatAudioClip(uiCTS, m_pClipItem->getStartTrimTime()) )
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Audio Data Read Error[0x%X].\n", __LINE__, uiReaderRet);
			goto DECODER_INIT_ERROR;
		}
	}


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] nexCAL_AudioDecoderInit Frame Data!",  __LINE__);
	nexSAL_MemDump(pDSI, uDSISize);
	//nexSAL_MemDump(pFrame, uFrameSize);


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Audio Decoder Init Start(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	MPEG_AUDIO_INFO mpa_info;
	memset(&mpa_info, 0x00, sizeof(MPEG_AUDIO_INFO));
	NexCodecUtil_Get_MPEG_Audio_Info( pFrame, uFrameSize, &mpa_info );
	if (mpa_info.Version == NEX_MPEG_AUDIO_VER_1 && mpa_info.Layer == NEX_MPEG_AUDIO_LAYER_2)
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 1);
	else
		nexCAL_AudioDecoderSetProperty(m_hCodecAudio, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 0);

	iRet = nexCAL_AudioDecoderInit(		m_hCodecAudio,
										(NEX_CODEC_TYPE)m_pFileReader->m_uiAudioObjectType, 
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

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Audio Decoder Init End(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	if(m_pAudioDecodeBuf != NULL)
	{
		nexSAL_MemFree(m_pAudioDecodeBuf);
		m_pAudioDecodeBuf = NULL;
	}

	if( uNumOfChannels == 0 || uBitsPerSample == 0 || uNumOfSamplesPerChannel == 0)
	{
		m_uiAudioDecodeBufSize = 384*1024;
		m_pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioDecodeBufSize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ReverseTask.cpp %d] Default  PCMBuffer(%p, %d)\n", __LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
	}
	else
	{
		m_uiAudioDecodeBufSize = (uNumOfSamplesPerChannel)*uNumOfChannels*(uBitsPerSample>>3);
		m_pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioDecodeBufSize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ReverseTask.cpp %d] init PCMBuffer(%p, %d)\n", __LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
	}
	

	
	if( iRet != NEXCAL_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Audio Decoder Init Failed(%d)",  __LINE__, iRet);
		goto DECODER_INIT_ERROR;
	}

	m_uiSampleRate		= uSamplingRate;
	m_uiChannels			= uNumOfChannels;
	m_uiBitsPerSample		= uBitsPerSample;
	m_uiSamplePerChannel	= uNumOfSamplesPerChannel;

	m_pFileReader->m_uiNumOfChannels				= m_uiChannels;
	m_pFileReader->m_uiSamplingRate				= m_uiSampleRate;
	m_pFileReader->m_uiNumOfSamplesPerChannel	= m_uiSamplePerChannel;

	m_pFileReader->m_uiAudioFrameInterval		= _GetFrameInterval(m_uiSamplePerChannel, m_uiSampleRate);
	m_pFileReader->m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiSamplePerChannel, m_uiSampleRate);
	m_pFileReader->m_uiMaxAudioFrameInterval	= (m_pFileReader->m_uiAudioFrameInterval*3) >> 1;			

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Interval(%d %f %d)",  __LINE__, 
		m_pFileReader->m_uiAudioFrameInterval, 
		m_pFileReader->m_dbAudioFrameInterval, 
		m_pFileReader->m_uiMaxAudioFrameInterval);

	if ((m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AAC || m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AACPLUS) && m_pDSI != NULL )
	{
		unsigned int    uSamplingFreqIndex = 0;
		const unsigned int puAACSampleRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};

		uSamplingFreqIndex = ((m_pDSI[0] & 0x07) << 1) | ((m_pDSI[1] & 0x80) >> 7);

		if ( uSamplingFreqIndex > 11 )//Nx_robin__090827 uSamplingFreqIndexï¿½ï¿½ Ã¼Å©
			uSamplingFreqIndex = 0;
		m_uiSampleRate = puAACSampleRates[uSamplingFreqIndex];
		m_uiChannels = (m_pDSI[1] & 0x78) >> 3;
		if ( m_uiChannels == 0 )//Nx_robin__090827 DSIï¿½ï¿½ï¿½ï¿½ uNumOfChannelsï¿½ï¿½ 0ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿?File MetaDataï¿½ï¿½ï¿½ï¿½ ï¿½ï¿½ï¿½ï¿½ï¿½Í¾ï¿½ ï¿½Ñ´ï¿½.
			m_pFileReader->getNumberOfChannel(&m_uiChannels);
		m_uiBitsPerSample = 16;

		m_pFileReader->m_uiAudioObjectType = NexCodecUtil_GetAACType((NEX_CODEC_TYPE)m_pFileReader->m_uiAudioObjectType, m_uiSampleRate, m_uiChannels, pFrame, uFrameSize);

		if(m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AACPLUSV2)
			m_pFileReader->m_uiAudioObjectType = eNEX_CODEC_A_AACPLUS;

		if ( m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AACPLUS )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] This contents is AAC+\n", __LINE__);
			m_uiSamplePerChannel = 2048;
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[ADTask.cpp %d] This contents is AAC\n", __LINE__);
			m_uiSamplePerChannel = 1024;
		}

		m_pFileReader->m_uiAudioFrameInterval		= _GetFrameInterval(m_uiSamplePerChannel, m_uiSampleRate);
		m_pFileReader->m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiSamplePerChannel, m_uiSampleRate);
		m_pFileReader->m_uiMaxAudioFrameInterval	= (m_pFileReader->m_uiAudioFrameInterval*3) >> 1;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio Sample(%d) Channels(%d), BitsPerSample(%d) SamplePerCh(%d)",
				__LINE__, m_uiSampleRate, m_uiChannels, m_uiBitsPerSample, m_uiSamplePerChannel);
	}

	m_isNextAudioFrameRead = FALSE;

	// Case : mantis 9267 when decoder channel info was wrong.
	if( m_uiSampleRate == 0 || m_uiChannels == 0 || m_uiBitsPerSample == 0 || m_uiSamplePerChannel == 0 )
	{
		if(m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AMR)
		{
			m_uiSampleRate = 8000;
			m_uiChannels = 1;
			m_uiBitsPerSample = 16;
			m_uiSamplePerChannel = 160;
		}
		else if(m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AMRWB)
		{
			m_uiSampleRate = 16000;
			m_uiChannels = 1;
			m_uiBitsPerSample = 16;
			m_uiSamplePerChannel = 320;
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Wait decode config change",  __LINE__);
			goto DECODER_INIT_ERROR;
		}
	}

	// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_FUNC, (unsigned int)_GetCurrentMediaCTS );
	// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_USERDATA, (unsigned int)this );

	if( m_uiSampleRate != EDITOR_DEFAULT_SAMPLERATE )
	{
		m_isResampling = TRUE;
	}

	m_uiAudioDecodeBufSize = 384*1024;
	m_pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioDecodeBufSize);

	m_uiAudioReSampleBufSize = 384*1024;
	m_pAudioReSampleBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioReSampleBufSize);

	if( m_isResampling )
	{
		if(FALSE == initResampler())
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask %d] initResampler open fail", __LINE__);
			goto DECODER_INIT_ERROR;
		}
	}
	
	return NEXVIDEOEDITOR_ERROR_NONE;

DECODER_INIT_ERROR:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Audio Decoder init failed",  __LINE__);
	deinitAudioDecoder();
	return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
}

NXBOOL CNEXThread_ReverseTask::deinitAudioDecoder()
{
	if( m_hCodecAudio )
	{
		nexCAL_VideoDecoderDeinit( m_hCodecAudio );
		CNexCodecManager::releaseCodec(m_hCodecAudio);
		m_hCodecAudio = NULL;
	}

	if( m_pAudioDecodeBuf )
	{
		nexSAL_MemFree(m_pAudioDecodeBuf);
		m_pAudioDecodeBuf = NULL;
		m_uiAudioDecodeBufSize = 0;
	}

	if( m_pAudioReSampleBuf )
	{
		nexSAL_MemFree(m_pAudioReSampleBuf);
		m_pAudioReSampleBuf = NULL;
		m_uiAudioReSampleBufSize = 0;
	}

	deinitResampler();

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_ReverseTask::getWriterError()
{
	if( m_pFileWriter == NULL )
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;

	
	int ret = m_pFileWriter->getErrorStatus();
	if( ret == 0 || ret == 1 )
	{
		return NEXVIDEOEDITOR_ERROR_ENCODE_VIDEO_FAIL;
	}
	else if( ret == 2 )
	{
		return NEXVIDEOEDITOR_ERROR_TRANSCODING_NOT_ENOUGHT_DISK_SPACE;
	}

	return NEXVIDEOEDITOR_ERROR_UNKNOWN;
}

void CNEXThread_ReverseTask::sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3)
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		pEditor->notifyEvent(uiEventType, uiParam1, uiParam2, uiParam3);
		SAFE_RELEASE(pEditor);
	}
}

void CNEXThread_ReverseTask::cancelReverseAudio()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0," [ReverseTask.cpp %d] cancelReverseAudio In(%d)", __LINE__, m_bUserCancel);
	m_bUserCancel = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] cancelReverseAudio Out(%d)", __LINE__, m_bUserCancel);
}

int CNEXThread_ReverseTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	switch( pMsg->m_nMsgType )
	{
		case MESSAGE_MAKE_REVERSE_START:
		{
			CNxMsgReverseInfo* pReverseInfo = (CNxMsgReverseInfo*)pMsg;
			if( pReverseInfo == NULL )
			{
				m_bAudioFinished = TRUE;
				m_iErrorType = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
				SAFE_RELEASE(pReverseInfo);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp  %d] Reverse task recevie message is null", __LINE__);
				return MESSAGE_PROCESS_OK;
			}
			
			processReverseAudioFrame();
			
			SAFE_RELEASE(pReverseInfo);
			return MESSAGE_PROCESS_OK;
		}
		default:
		{
			nexSAL_TaskSleep(10);
			break;
		}
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);	
}

void CNEXThread_ReverseTask::processReverseAudioFrame()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processReverseAudioFrame Start!!!", __LINE__);

	NXBOOL	bWriteAudio = FALSE;
	NXBOOL	bAudioFrameEnd = FALSE;
	NXBOOL	bFinishedAudio = FALSE;

	NXINT64 uiAudioDTS = 0;
	unsigned int uiAudioFrameSize = 0;
	unsigned char* pAudioFrame = NULL;

	NXBOOL bEncodeFailed = FALSE;
	NXBOOL bDecodeFailed = FALSE;

	unsigned int uTempStart = 0;
	unsigned int uTempEnd = 0xffffffff;

	NXBOOL bTempFileReady = FALSE;
	NXBOOL bLastDecFlag = FALSE;
	unsigned int uBaseReverseTime = 0;
	NXINT64	uFirstAudioFrame = 0;
	
	unsigned int uAdjustAudioTime = 0;
	unsigned int uSeekTime = 0;

	unsigned int uiDecoderErrRet = 0;	
	unsigned int uLastAudioDTS = 0;
	unsigned int uSectionStartTime = 0;
	unsigned int uSectionDropCount = 0;
	unsigned int uSectionDropSize = 0;
	unsigned int uUsedByte = 0;
	int iRet = 0;

	int iCurrProgress = 0;
	int iPrevProgress = -1;

	//NEXSALFileHandle hPCMFile = nexSAL_FileOpen("/storage/emulated/0/PCM.dump",  (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_READWRITE));

	if( m_pFileReader->isAudioExist() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] No Audio track", __LINE__);
		//m_iErrorType = NEXVIDEOEDITOR_ERROR_UNSUPPORT_FORMAT;		
		m_bAudioFinished = TRUE;		
		bFinishedAudio = TRUE;
		deinitFileReader();	
		return;
	}

	// Init Audio Decoder.
	iRet = initAudioDecoder(0);
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		m_iErrorType = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
		m_bAudioFinished = TRUE;
		bFinishedAudio = TRUE;
		deinitFileReader();
		return;
	}

	m_pPCMBuffer = (unsigned char*)nexSAL_MemAlloc(1024*1024);
	m_uPCMBufferSize = 0;

	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfGLDraw.CheckModuleStart();
	m_perfGetPixels.CheckModuleStart();
	m_perfSwapbuffer.CheckModuleStart();
	m_perfDrawAll.CheckModuleStart();

	if(m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AMR || m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AMRWB)
		uAdjustAudioTime = m_uiSamplePerChannel*1000/m_uiSampleRate*8;
	else
		uAdjustAudioTime = m_uiSamplePerChannel*1000/m_uiSampleRate*3;

	if(m_uEndTime - m_uStartTime > 2000 + uAdjustAudioTime)
		uSeekTime = m_uEndTime - 2000 - uAdjustAudioTime;
	else if(m_uStartTime > uAdjustAudioTime)
		uSeekTime = m_uStartTime - uAdjustAudioTime;
	else
		uSeekTime = 0;

	m_pFileReader->getAudioFrameCTS( &uFirstAudioFrame );
	m_pFileReader->seekToAudio(uSeekTime, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
	m_isNextAudioFrameRead = TRUE;	
	uTempEnd = m_uEndTime;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] ready to seek (%d, %d, %d, %d, %d)", __LINE__, m_uStartTime, m_uEndTime ,uAdjustAudioTime, uSeekTime, uTempStart);

	if(m_uStartTime >= uTempStart)
	{
		bLastDecFlag = TRUE;
	}

	uTempStart = 0xFFFFFFFF;
	
	while ( m_bIsWorking )
	{
		if( m_bUserCancel || bEncodeFailed || bDecodeFailed )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] task stop because state was changed(%d, %d %d)", 
				__LINE__, m_bUserCancel, bEncodeFailed ,bDecodeFailed);
			break;
		}

		if(bTempFileReady == FALSE)
		{
			if( m_isNextAudioFrameRead )
			{
				iRet = m_pFileReader->getAudioFrame();
				
				if( iRet  == _SRC_GETFRAME_END || iRet  == _SRC_GETFRAME_ERROR )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] getAudioFrame End", __LINE__);
					bAudioFrameEnd = TRUE;
				}
				m_isNextAudioFrameRead = FALSE;
			}

			m_pFileReader->getAudioFrameCTS( &uiAudioDTS );
			m_pFileReader->getAudioFrameData( &pAudioFrame, &uiAudioFrameSize );

			if((unsigned int)uiAudioDTS >= uTempEnd) 
			{
				bAudioFrameEnd = TRUE;
			}					

			if((unsigned int)uiAudioDTS + uAdjustAudioTime < m_uStartTime)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] drop audio frame before Start Time (%lld, %d)", __LINE__, uiAudioDTS, m_uStartTime);
				m_isNextAudioFrameRead = TRUE;
				continue;						
			}

			m_uiAudioDecodeBufSize = 384*1024;
			nexCAL_AudioDecoderDecode(	m_hCodecAudio,
										pAudioFrame, 
										uiAudioFrameSize, 
										NULL, 
										m_pAudioDecodeBuf,
										(int *)&m_uiAudioDecodeBufSize,
										(unsigned int)uiAudioDTS,
										NULL,
										bAudioFrameEnd == FALSE?NEXCAL_ADEC_FLAG_NONE:NEXCAL_ADEC_FLAG_END_OF_STREAM,
										&uiDecoderErrRet );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[ReverseTask.cpp %d] Audio Decode. (Buf %p, BufSize:%d, TS:%lld Ret:0x%x)", 
				__LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize , uiAudioDTS, uiDecoderErrRet);

			if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_NEXT_FRAME))
			{
				//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] getNextFrame is OK!", __LINE__);
				m_isNextAudioFrameRead = TRUE;

				if(bLastDecFlag)
				{
					if(m_uStartTime > (unsigned int)uiAudioDTS)
					{
						uSectionDropCount++;
					}
					else
					{
						if(uTempStart == 0xFFFFFFFF)
						{
							uTempStart = (unsigned int)uiAudioDTS;
							uSectionDropSize = m_uiSamplePerChannel*m_uiBitsPerSample/8*m_uiChannels*uSectionDropCount;	
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] uTempStart %d, uSectionDropSize %d", __LINE__, uTempStart, uSectionDropSize);						
						}
					}
				}
				else
				{	
					if(uTempStart == 0xFFFFFFFF)
					{
						if(uSectionDropCount == 3)
						{
							uTempStart = (unsigned int)uiAudioDTS;
							uSectionDropSize = m_uiSamplePerChannel*m_uiBitsPerSample/8*m_uiChannels*uSectionDropCount;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] uTempStart %d, uSectionDropSize %d", __LINE__, uTempStart, uSectionDropSize);							
						}
						uSectionDropCount++;						
					}
				}
				
				if(bAudioFrameEnd && uBaseReverseTime == 0)
					uBaseReverseTime = (unsigned int)uiAudioDTS;
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

					if( m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
					{
						if(CNexVideoEditor::m_bTrustAudioMonoInfoFromCodec == FALSE)
						{
							uiChannels = m_uiChannels;
						}
						else
						{
							m_uiChannels = uiChannels;
						}
					}	
					else
					{
						m_uiChannels = uiChannels;					
					}

					m_uiSampleRate = uiSamplingRate;
					if(m_isResampling)
					{
						deinitResampler();
					}
						
					if( m_uiSampleRate != EDITOR_DEFAULT_SAMPLERATE )
					{
						m_isResampling = TRUE;
					}

					if( m_isResampling )
					{
						if(FALSE == initResampler())
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask %d] NexSound resampler open fail", __LINE__);
							bDecodeFailed = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
							m_bAudioFinished = TRUE;			
							bFinishedAudio = TRUE;	
							continue;
						}
					}
						
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio Output was changed!(S(%d) C(%d) BpS(%d) SC(%d)", 
						__LINE__, uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
				}
				
				if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_EXIST))
				{
					unsigned char* pBuf = NULL;
					unsigned int uBufSize = 0;

					if(uTempStart == 0xFFFFFFFF)
					{
						memcpy(m_pPCMBuffer + m_uPCMBufferSize, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
						m_uPCMBufferSize += m_uiAudioDecodeBufSize;			
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] add PCM before TempStart %d, %d", __LINE__, m_uPCMBufferSize, m_uiAudioDecodeBufSize);						
						continue;						
					}
					else
					{
						if(uSectionDropSize != 0 && m_uPCMBufferSize != 0)
						{
							if(m_uPCMBufferSize > uSectionDropSize)
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] m_uPCMBufferSize %d, uSectionDropSize %d", __LINE__, m_uPCMBufferSize, uSectionDropSize);
								memmove(m_pPCMBuffer, m_pPCMBuffer + uSectionDropSize, m_uPCMBufferSize - uSectionDropSize);
								m_uPCMBufferSize -= uSectionDropSize;
								uSectionDropSize = 0;

								if(m_uPCMBufferSize + m_uiAudioDecodeBufSize > 384*1024)
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] please check m_uPCMBufferSize %d, uSectionDropSize %d", __LINE__, m_uPCMBufferSize, uSectionDropSize);
									m_uPCMBufferSize = 0;
								}
								else
								{
									memcpy(m_pPCMBuffer + m_uPCMBufferSize, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
									m_uPCMBufferSize += m_uiAudioDecodeBufSize;

									memcpy(m_pAudioDecodeBuf, m_pPCMBuffer, m_uPCMBufferSize);
									m_uiAudioDecodeBufSize = m_uPCMBufferSize;
									m_uPCMBufferSize = 0;
								}
							}
							else
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] m_uPCMBufferSize %d, uSectionDropSize %d", __LINE__, m_uPCMBufferSize, uSectionDropSize);							
								uSectionDropSize -= m_uPCMBufferSize; 
								m_uPCMBufferSize = 0;
							}
						}
					}

					if(uSectionDropSize >= m_uiAudioDecodeBufSize)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] uSectionDropSize %d, m_uiAudioDecodeBufSize %d", __LINE__, uSectionDropSize, m_uiAudioDecodeBufSize);									
						uSectionDropSize -= m_uiAudioDecodeBufSize;
						continue;
					}
					else
					{
						if(uSectionDropSize != 0)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] uSectionDropSize %d, m_uiAudioDecodeBufSize %d", __LINE__, uSectionDropSize, m_uiAudioDecodeBufSize);				
							memmove(m_pAudioDecodeBuf, m_pAudioDecodeBuf + uSectionDropSize, m_uiAudioDecodeBufSize - uSectionDropSize);
							m_uiAudioDecodeBufSize-=uSectionDropSize;
							uSectionDropSize = 0;
						}
					}
					
					if(m_isResampling)
					{
						unsigned int uTotalOutBufSize=0, OutBufSize=0, uLastSample;
						unsigned int uInputSamplesPerChannel = m_uiAudioDecodeBufSize / (m_uiBitsPerSample >> 3) /  m_uiChannels;

						unsigned char *pInBuffer = m_pAudioDecodeBuf;
						unsigned char* pOutBuffer = m_pAudioReSampleBuf;

						pBuf = m_pAudioReSampleBuf;

						while (uInputSamplesPerChannel > 0)
						{
							uLastSample = (uInputSamplesPerChannel > EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE)?EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE:uInputSamplesPerChannel;

							// Processing Resampler.
							m_hNexResamplerHandle->NexSoundProcess(Processor::Resampler, (Int16*)pInBuffer, (Int16*)pOutBuffer, uLastSample);
							m_hNexResamplerHandle->NexSoundGetParam(Processor::Resampler, ParamCommand::Resampler_Output_NumberOfSamples, &OutBufSize);

							pInBuffer += (uLastSample*m_uiChannels*sizeof(short));
							pOutBuffer += (OutBufSize*m_uiChannels*sizeof(short));
							uInputSamplesPerChannel -= uLastSample;
							uTotalOutBufSize += OutBufSize;
						}

						uBufSize = uTotalOutBufSize*m_uiChannels*sizeof(short);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ReverseTask.cpp %d] resampling AudioDecodeBufSize(%d)->ResamplerOutputBufSize(%d)", __LINE__, m_uiAudioDecodeBufSize, uBufSize);
					}
					else
					{
						pBuf = m_pAudioDecodeBuf;
						uBufSize = m_uiAudioDecodeBufSize;
					}

					if(m_uPCMBufferSize + uBufSize > 1024*1024)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Addio PcmBuffer overflow", __LINE__);					
						bTempFileReady = TRUE;
						continue;
					}

					if(m_uiChannels == 1)
					{
						int samples = uBufSize/(m_uiBitsPerSample/8);
						unsigned char* pNew = m_pPCMBuffer + m_uPCMBufferSize;
					
						for(int i=0; i<samples; i++)
						{
							memcpy(	pNew + i*4, pBuf + i*2, m_uiBitsPerSample/8);
							memcpy(	pNew + i*4 + 2, pBuf + i*2, m_uiBitsPerSample/8);
						}
						m_uPCMBufferSize += uBufSize*2;
					}
					else
					{
						memcpy(m_pPCMBuffer + m_uPCMBufferSize, pBuf, uBufSize);
						m_uPCMBufferSize += uBufSize;					
					}				
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] add Audio PCMBuffer %d, %d, %d, %lld", __LINE__, m_uPCMBufferSize, m_uiAudioDecodeBufSize, uBufSize, uiAudioDTS);					
					
					if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
					{
						bTempFileReady = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Addio Task EOS received", __LINE__);
					}
				}
				else
				{
					if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
					{
						bTempFileReady = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio Task end received", __LINE__);
					}
					
					if( bAudioFrameEnd == TRUE)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio is End!!", __LINE__);
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[ReverseTask.cpp %d] Audio Output is empty!!", __LINE__);
					}
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio Dec Failed(%p %d %lld Ret:%d)", __LINE__, pAudioFrame, uiAudioFrameSize, uiAudioDTS, uiDecoderErrRet);
				m_isNextAudioFrameRead = TRUE;
				nexSAL_TaskSleep(5);

				if( bAudioFrameEnd  == TRUE) // Frame End.
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] TAudio Dec Failed(%p %d %lld Ret:%d)", __LINE__, pAudioFrame, uiAudioFrameSize, uiAudioDTS, uiDecoderErrRet);
				}
				bDecodeFailed = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				m_bAudioFinished = TRUE;			
				bFinishedAudio = TRUE;
			}
			continue;
		}

		if(m_pFileReader->isVideoExist() && m_uEncodingTime > m_pTranscodingTask->getVideoEncodingTime() && m_pTranscodingTask->getVideoFinished() == FALSE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Wait Videoframe(A:%d VD:%d)", __LINE__, m_uEncodingTime, m_pTranscodingTask->getVideoEncodingTime());
			bWriteAudio = FALSE;
		}
		else
		{
			if(!m_pFileReader->isVideoExist() || m_pFileWriter->isVideoFrameWriteStarted())
				bWriteAudio = TRUE;
			else
				bWriteAudio = FALSE;
		}
		
		if(bWriteAudio == FALSE)
		{
			nexSAL_TaskSleep(100);
		}
		else
		{
			unsigned int uiSize = 0;
			unsigned int uiDuration = 0;
			//unsigned int uiInterval = m_uEncodingCount*(AUDIO_ENCODE_MAX_SIZE * 1000)/(m_uiBitsPerSample/8*2*44100);
			unsigned int uiInterval = m_uEncodingCount*(1024*1000/44100);
		
			if(m_uPCMBufferSize > 0)
			{
				if(m_uPCMBufferSize > AUDIO_ENCODE_MAX_SIZE)
				{
					unsigned int copyByte = m_uiBitsPerSample/8*2;
					unsigned int endCount = AUDIO_ENCODE_MAX_SIZE/copyByte;
					unsigned char* pTemp = m_pPCMBuffer + m_uPCMBufferSize;

					#if 1
					for(int i =0; i < endCount; i++)
					{
						memcpy(m_pAudioDecodeBuf + i*copyByte, pTemp - (i+1)*copyByte, copyByte);
					}
					#else
					nexSAL_FileWrite(hPCMFile, m_pPCMBuffer+uUsedByte, AUDIO_ENCODE_MAX_SIZE);
					memcpy(m_pAudioDecodeBuf, m_pPCMBuffer + uUsedByte, AUDIO_ENCODE_MAX_SIZE);					
					uUsedByte += AUDIO_ENCODE_MAX_SIZE;
					#endif

					if( m_pFileWriter->setAudioFrame(uBaseReverseTime - (uBaseReverseTime - uiInterval), m_pAudioDecodeBuf, AUDIO_ENCODE_MAX_SIZE, &uiDuration, &uiSize) == FALSE )
					{
						m_iErrorType = getWriterError();
						bEncodeFailed = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Audio Frame Write failed", __LINE__);
						continue;
					}
					
					m_uPCMBufferSize -= AUDIO_ENCODE_MAX_SIZE;
				}
				else 
				{
					unsigned int copyByte = m_uiBitsPerSample/8*2;
					unsigned int endCount = m_uPCMBufferSize/copyByte;
					unsigned char* pTemp = m_pPCMBuffer + m_uPCMBufferSize - copyByte;

					#if 1
					for(int i =0; i < endCount; i++)
					{
						memcpy(m_pAudioDecodeBuf + i*copyByte, pTemp - i*copyByte, copyByte);
					}
					#else
					nexSAL_FileWrite(hPCMFile, m_pPCMBuffer+uUsedByte, m_uPCMBufferSize);					
					memcpy(m_pAudioDecodeBuf, m_pPCMBuffer + uUsedByte, m_uPCMBufferSize);	
					uUsedByte += m_uPCMBufferSize;
					#endif

					//uiInterval = (m_uPCMBufferSize * 1000)/(m_uiBitsPerSample/8*m_uiChannels*m_uiSampleRate);
					if( m_pFileWriter->setAudioFrame(uBaseReverseTime - (uBaseReverseTime - uiInterval), m_pAudioDecodeBuf, m_uPCMBufferSize, &uiDuration, &uiSize) == FALSE )
					{
						m_iErrorType = getWriterError();
						bEncodeFailed = TRUE;						
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ReverseTask.cpp %d] Audio Frame Write failed", __LINE__);
						continue;
					}
					
					m_uPCMBufferSize = 0;		
				}

				m_uEncodingCount++;
				m_uEncodingTime = m_pFileWriter->getAudioTime();
					
				if(!m_pFileReader->isVideoExist())
				{
					iCurrProgress = ((uBaseReverseTime - (uBaseReverseTime - uiInterval))*100 / (m_uEndTime - m_uStartTime));
					if( iCurrProgress > iPrevProgress)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] REVERSE PROGRESS(%d %d %d, %d, %d)", __LINE__, iCurrProgress, uBaseReverseTime, uBaseReverseTime - uiInterval, m_uStartTime, m_uEndTime);
						sendNotifyEvent(MESSAGE_MAKE_REVERSE_PROGRESS, iCurrProgress, m_uEncodingTime, m_uEndTime - m_uStartTime);
						iPrevProgress = iCurrProgress;
					}				
				}
				else if(m_pTranscodingTask->getVideoFinished())
				{
					if(m_uEncodingTime > m_pTranscodingTask->getVideoEncodingTime())
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] video is finished and audio time is over, %d, %d)", __LINE__, m_uEncodingTime, m_pTranscodingTask->getVideoEncodingTime());
						bFinishedAudio = TRUE;
						break;
					}
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] NexReverseAudioBuffer(%d, %d, %d, %d, %d, %d)", __LINE__, m_uPCMBufferSize, m_uEncodingCount, uBaseReverseTime, (uBaseReverseTime - uiInterval), m_uEncodingTime, m_pTranscodingTask->getVideoEncodingTime());
				continue;
			}
			else
			{
				if(uFirstAudioFrame == uTempStart || bLastDecFlag)
				{				
					bFinishedAudio = TRUE;
				}
				else
				{
					uTempEnd = uTempStart;
					uSeekTime = uTempStart > 2000+uAdjustAudioTime?uTempStart-2000-uAdjustAudioTime:0;
					if(m_uStartTime > uSeekTime)
					{
						if(m_uStartTime - uSeekTime > uAdjustAudioTime)
							uSeekTime = m_uStartTime - uAdjustAudioTime;
						else if(m_uStartTime > uAdjustAudioTime)
							uSeekTime = m_uStartTime - uAdjustAudioTime;
						else
							uSeekTime = 0;
					}
					m_pFileReader->seekToAudio(uSeekTime, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
					if(m_uStartTime > uTempStart || uFirstAudioFrame == uTempStart)
					{
						bLastDecFlag = TRUE;
					}
					nexCAL_AudioDecoderReset(m_hCodecAudio);
					m_isNextAudioFrameRead = TRUE;
					bTempFileReady = FALSE;
					bAudioFrameEnd = 0;
					uiDecoderErrRet = 0;
					uTempStart = 0xFFFFFFFF;
					uSectionDropCount = 0;
				}
			}

			if( bFinishedAudio == TRUE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Audio is finished!!!",__LINE__);
				break;
			}
		}
	}

	//nexSAL_FileClose(hPCMFile);
	m_bAudioFinished = TRUE;

	if(m_pPCMBuffer)
	{
		nexSAL_MemFree(m_pPCMBuffer);
		m_pPCMBuffer = NULL;
		m_uPCMBufferSize = 0;
	}
	
	if( m_bUserCancel )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Detect user cancel while Reverse(%d)", __LINE__, m_uiTotalPlayTime);
		m_iErrorType = NEXVIDEOEDITOR_ERROR_TRANSCODING_USER_CANCEL;
	}
	else if( bEncodeFailed )
	{
		m_bUserCancel = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Detect encode failed(%d)", __LINE__, m_uiTotalPlayTime);
	}
	else if(bDecodeFailed)
	{
		m_bUserCancel = TRUE;
		m_iErrorType = NEXVIDEOEDITOR_ERROR_TRANSCODING_CODEC_FAILED;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ReverseTask.cpp %d] Detect decode failed(%d)", __LINE__, m_uiTotalPlayTime);
	}
	else
	{
		// sendNotifyEvent(MESSAGE_TRANSCODING_PROGRESS, 100, m_uiTotalPlayTime, m_uiTotalPlayTime);
	}

	deinitAudioDecoder();
	deinitFileReader();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processReverse Ended!!!", __LINE__);
	return;	
}

NXBOOL CNEXThread_ReverseTask::initResampler()
{
	NXBOOL bRet = TRUE;

	if(m_hNexResamplerHandle)
	{
		m_hNexResamplerHandle->NexSoundRelease(Processor::Resampler);
		DestroyNexSound(m_hNexResamplerHandle);
		m_hNexResamplerHandle	= 0;
	}

	m_hNexResamplerHandle = CreateNexSound();
	if( m_hNexResamplerHandle == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] initResampler CreateNexSound failed",__LINE__);
		bRet = FALSE;
		goto InitResampler_Error;
	}

	if(m_hNexResamplerHandle->NexSoundInitialize(Processor::Resampler))
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] initResampler NexSoundInitialize failed",__LINE__);
		bRet = FALSE;
		goto InitResampler_Error;
	}

	m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Input_SamplingRate, (int)m_uiSampleRate);
	m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Input_Channels, (int)m_uiChannels);
	m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Input_NumberOfSamples, EDITOR_DEFAULT_RESAMPLER_BLOCK_SIZE);
	m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_BitPerSample, (int)m_uiBitsPerSample);
	m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Output_SamplingRate, EDITOR_DEFAULT_SAMPLERATE);
	m_hNexResamplerHandle->NexSoundSetParam(Processor::Resampler, ParamCommand::Resampler_Quality_Level, NEX_RESAMPLER_DEF_QUALITY);

InitResampler_Error:
	if (FALSE == bRet)
	{
		deinitResampler();
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initResampler NexResamplerHandle(%p) ", __LINE__, m_hNexResamplerHandle);
	return bRet;
}

NXBOOL CNEXThread_ReverseTask::deinitResampler()
{
	if( m_hNexResamplerHandle )
	{
		m_hNexResamplerHandle->NexSoundRelease(Processor::Resampler);
		DestroyNexSound(m_hNexResamplerHandle);
		m_hNexResamplerHandle	= 0;
		m_isResampling = 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask %d] NxSound ResamplerClose", __LINE__);
	}

	return TRUE;
}
