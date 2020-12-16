/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_TranscodingTask.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

//#define TEST_REVERSE_YUV

#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_TranscodingTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "nexSeperatorByte_.h"
#include "nexYYUV2YUV420.h"

// #define DUMP_YUV_FRAME

#define ROUNDUP(x, y) (((x)+((y)-1)) & (-1 * (y)))

#define IS_PCM_Codec(type) \
			( (type == eNEX_CODEC_A_PCM_S16LE) || (type == eNEX_CODEC_A_PCM_S16BE) || (type == eNEX_CODEC_A_PCM_RAW) || (type == eNEX_CODEC_A_PCM_FL32LE) \
			 || (type == eNEX_CODEC_A_PCM_FL32BE) || (type == eNEX_CODEC_A_PCM_FL64LE) || (type == eNEX_CODEC_A_PCM_FL64BE) || (type == eNEX_CODEC_A_PCM_IN24LE) \
			 || (type == eNEX_CODEC_A_PCM_IN24BE) || (type == eNEX_CODEC_A_PCM_IN32LE) || (type == eNEX_CODEC_A_PCM_IN32BE) || (type == eNEX_CODEC_A_PCM_LPCMLE) \
			 || (type == eNEX_CODEC_A_PCM_LPCMBE))

#define IS_AMR_Codec(type) \
			( (type == eNEX_CODEC_A_AMR) || (type == eNEX_CODEC_A_AMRWB) || (type == eNEX_CODEC_A_EAMRWB))

#define IS_AUDIO_DECODING(type) \
			( IS_PCM_Codec(type) || IS_AMR_Codec(type) || type == eNEX_CODEC_A_FLAC)
			
CNEXThread_TranscodingTask::CNEXThread_TranscodingTask( void ) : 
	m_perfVideoY2UV2YUV((char*)"TranscodingY2UVAtoYUV420"),
	m_perfGLDraw((char*)"TranscodingGLDraw"),
	m_perfGetPixels((char*)"TranscodingGetPixels"),
	m_perfSwapbuffer((char*)"TranscodingSwapBuffer"),
	m_perfDrawAll((char*)"TranscodingDrawAll")
{
	m_pProjectMng		= NULL;

	m_strSrcFilePath = NULL;
	m_strDstFilePath = NULL;
	m_strTempFilePath = NULL;

	m_pUserData = NULL;

	m_pCodecWrap = NULL;
	m_uiCheckVideoDecInit = 0;

	m_hCodecAudio = NULL;
	m_isResampling = 0;
	m_hNexResamplerHandle = NULL;
	m_uiAudioDecodeBufSize = 0;
	m_pAudioDecodeBuf = NULL;
	m_uiAudioReSampleBufSize = 0;
	m_pAudioReSampleBuf = NULL;
	m_pAudioUpChannelBuf = NULL;

	m_isNextVideoFrameRead =  FALSE;
	m_isNextAudioFrameRead = FALSE;

	m_uiTotalPlayTime = 0;
	m_isVideo = 0;
	m_isAudio = 0;

	m_iSrcWidth = 0;
	m_iSrcHeight = 0;
	m_iSrcPitch = 0;

	m_iDstWidth = 0;
	m_iDstHeight = 0;
	m_iDstPitch = 0;
	m_iDstDisplayWidth = 0;
	m_iDstDisplayHeight = 0;

	m_iSettingWidth = 0;
	m_iSettingHeight = 0;
	m_iSettingPitch = 0;

	m_iFlag = 0;
	m_iSpeedFactor = 100;

	m_uiEncodedVideoCount = 0;
	m_uiReqEncodeVideoCount = 0;

	m_hTranscodingRenderer = NULL;
	m_pTranscodingDecSurface = NULL;
	m_pOutputSurface = NULL;
	m_pMediaSurface = NULL;
	m_uiEncoderInputFormat = 0;

	m_pFileReader = NULL;
	m_pFileWriter = NULL;

	m_pSoftwareTempBuffer = NULL;

	m_bUserCancel = FALSE;

	m_FrameTimeChecker.clear();

	m_pYUVFRAME	= NULL;

	m_iHighLightOutputMode = 0;
	m_pHighLightBackBuffer = NULL;

	m_iSrcOrientation = 0;

	m_uStartTime = 0;
	m_uEndTime = 0;
	m_vecNexReverseYUVInfo.clear();
	m_pYuvFile = NULL;
	m_uYuvTotalSize = 0;
	m_pReverseAudioTask = NULL;
	m_uReverseMode = 0;

	m_bVideoFinished = 0;
	m_uEncodingTime = 0;
	m_uBaseReverseTime = 0;

    m_bNeedRenderFinish = FALSE;
	m_iDecodeMode = 0;
	m_bSupportFrameTimeChecker = 0;

	m_pTempBuffer = NULL;

	m_iOutputFormat = 21;
	CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
	if( pVideoEditor != NULL )
	{
		m_bNeedRenderFinish = pVideoEditor->getPropertyBoolean("WaitGLRenderFinish", FALSE);
		m_bSupportFrameTimeChecker = pVideoEditor->m_bSupportFrameTimeChecker;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] WaitGLRenderFinish(%d), bSupportFrameTimeChecker(%d)", __LINE__, m_bNeedRenderFinish, m_bSupportFrameTimeChecker);
		SAFE_RELEASE(pVideoEditor);
	}

	m_pFrameYUVInfo = NULL;
#ifdef DUMP_YUV_FRAME
	m_pYUVFRAME	= fopen("/sdcard/TranscodingFrame.yuv", "wb");
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d]CNEXThread_TranscodingTask Create Done", __LINE__);
}

CNEXThread_TranscodingTask::CNEXThread_TranscodingTask( CNexProjectManager* pPM ) : 
	m_perfVideoY2UV2YUV((char*)"TranscodingY2UVAtoYUV420"),
	m_perfGLDraw((char*)"TranscodingGLDraw"),
	m_perfGetPixels((char*)"TranscodingGetPixels"),
	m_perfSwapbuffer((char*)"TranscodingSwapBuffer"),
	m_perfDrawAll((char*)"TranscodingDrawAll")
{
	SAFE_ADDREF(pPM);
	m_pProjectMng = pPM;

	m_strSrcFilePath = NULL;
	m_strDstFilePath = NULL;
	m_strTempFilePath = NULL; 

	m_pUserData = NULL;

	m_pCodecWrap = NULL;
	m_uiCheckVideoDecInit = 0;

	m_hCodecAudio = NULL;
	m_uiAudioDecodeBufSize = 0;
	m_pAudioDecodeBuf = NULL;
	m_isResampling = 0;
	m_hNexResamplerHandle = NULL;
	m_uiAudioReSampleBufSize = 0;
	m_pAudioReSampleBuf = NULL;
	m_pAudioUpChannelBuf = NULL;

	m_isNextVideoFrameRead =  FALSE;
	m_isNextAudioFrameRead = FALSE;

	m_uiTotalPlayTime = 0;
	m_isVideo = 0;
	m_isAudio = 0;

	m_iSrcWidth = 0;
	m_iSrcHeight = 0;
	m_iSrcPitch = 0;

	m_iDstWidth = 0;
	m_iDstHeight = 0;
	m_iDstPitch = 0;
	m_iDstDisplayWidth = 0;
	m_iDstDisplayHeight = 0;

	m_iSettingWidth = 0;
	m_iSettingHeight = 0;
	m_iSettingPitch = 0;

	m_iFlag = 0;
	m_iSpeedFactor = 100;

	m_uiEncodedVideoCount = 0;
	m_uiReqEncodeVideoCount = 0;

	m_hTranscodingRenderer = NULL;
	m_pTranscodingDecSurface = NULL;
	m_pOutputSurface = NULL;
	m_pMediaSurface = NULL;

	m_pFileReader = NULL;
	m_pFileWriter = NULL;

	m_pSoftwareTempBuffer = NULL;
	
	m_bUserCancel = FALSE;

	m_FrameTimeChecker.clear();

	m_pYUVFRAME	= NULL;

	m_iHighLightOutputMode = 0;
	m_pHighLightBackBuffer = NULL;
	
	m_iSrcOrientation = 0;

	m_uStartTime = 0;
	m_uEndTime = 0;
	m_vecNexReverseYUVInfo.clear();
	m_pYuvFile = NULL;
	m_uYuvTotalSize = 0;
	m_pReverseAudioTask = NULL;
	m_uReverseMode = 0;

	m_bVideoFinished = 0;
	m_uEncodingTime = 0;
	m_uBaseReverseTime = 0;
	
	m_bNeedRenderFinish = FALSE;
	m_iDecodeMode = 0;
	m_bSupportFrameTimeChecker = 0;

	m_pTempBuffer = NULL;

	m_iOutputFormat = 21;
	CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
	if( pVideoEditor != NULL )
	{
		m_bNeedRenderFinish = pVideoEditor->getPropertyBoolean("WaitGLRenderFinish", FALSE);
		m_bSupportFrameTimeChecker = pVideoEditor->m_bSupportFrameTimeChecker;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] WaitGLRenderFinish(%d), bSupportFrameTimeChecker(%d)", __LINE__, m_bNeedRenderFinish, m_bSupportFrameTimeChecker);
		SAFE_RELEASE(pVideoEditor);
	}	
	m_pFrameYUVInfo = NULL;    
#ifdef DUMP_YUV_FRAME
	m_pYUVFRAME	= fopen("/sdcard/TranscodingFrame.yuv", "wb");
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d]CNEXThread_TranscodingTask Create Done", __LINE__);
}

CNEXThread_TranscodingTask::~CNEXThread_TranscodingTask( void )
{
	deinitAudioDecoder();
	deinitVideoDecoder();
	deinitWriter();
	deinitRenderer();
	deinitFileReader();

	m_hTranscodingRenderer = NULL;
	m_pTranscodingDecSurface = NULL;
	m_pOutputSurface = NULL;

	SAFE_RELEASE(m_pCodecWrap);

	m_pProjectMng->clearCacheTranscodingDecSurf();
	SAFE_RELEASE(m_pProjectMng);

	if( m_pReverseAudioTask )
	{
		if( m_pReverseAudioTask->IsWorking() )
		{
			m_pReverseAudioTask->End(1000);
		}
		SAFE_RELEASE(m_pReverseAudioTask);
	}

	if( m_pYUVFRAME != NULL )
	{
		fclose(m_pYUVFRAME);
		m_pYUVFRAME = NULL;
	}

	if( m_pUserData )
	{
		nexSAL_MemFree(m_pUserData);
		m_pUserData = NULL;
	}

	if( m_pSoftwareTempBuffer != NULL )
	{
		nexSAL_MemFree(m_pSoftwareTempBuffer);
		m_pSoftwareTempBuffer = NULL;
	}

	if( m_pHighLightBackBuffer != NULL )
	{
		nexSAL_MemFree(m_pHighLightBackBuffer);
		m_pHighLightBackBuffer = NULL;
	}

	m_vecNexReverseYUVInfo.clear();
	if(m_pYuvFile)
	{
		nexSAL_FileClose(m_pYuvFile);
		m_pYuvFile = NULL;
		nexSAL_FileRemove(m_strTempFilePath);		
	}

	if( m_strSrcFilePath )
	{
		nexSAL_MemFree(m_strSrcFilePath);
		m_strSrcFilePath = NULL;
	}

	if( m_strDstFilePath )
	{
		nexSAL_MemFree(m_strDstFilePath);
		m_strDstFilePath = NULL;
	}

	if(m_strTempFilePath)
	{
		nexSAL_MemFree(m_strTempFilePath);
		m_strTempFilePath = NULL;
	}
    
	delete m_pFrameYUVInfo;
    
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d]~~~~CNEXThread_TranscodingTask Destroy Done", __LINE__ );
}

int CNEXThread_TranscodingTask::setInfo(const char* pSrcFile, const char* pDstFile, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, int iBitrate, long long llMaxFileSize, int iFPS, int iFlag, int iSpeedFactor, void* pTranscodingRenderer, void* pOutputSurface, void* pDecSurface, char* pUserData)
{
	if( pSrcFile == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setInfo Src is NULL!", __LINE__ );
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
//jbcho 20180806
/*
	if( pDstFile == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setInfo Dst is NULL!", __LINE__ );
	//	return  NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
*/
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setInfo SRC(%s)", __LINE__, pSrcFile);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setInfo DST(%s)", __LINE__, pDstFile);

	if( pTranscodingRenderer == NULL || pOutputSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setInfo render info failed", __LINE__ );
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

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

	m_iDstWidth = iWidth;
	m_iDstHeight = iHeight;
	m_iDstDisplayWidth = iDisplayWidth;
	m_iDstDisplayHeight = iDisplayHeight;
	m_iBitrate = iBitrate;
	m_llMaxFileSize = llMaxFileSize;
	m_iFPS = iFPS;
	m_hTranscodingRenderer =  (NXT_HThemeRenderer)pTranscodingRenderer;
	m_pOutputSurface = pOutputSurface;
	m_pTranscodingDecSurface = pDecSurface;

	m_iFlag = iFlag;
	m_iSpeedFactor = iSpeedFactor;

	m_iEncodeProfile = 0x01;
	m_iEncodeLevel = 0x800;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] DST W: %d H: %d DisW: %d DisH: %d MAX:%lld Bitrate :%d FPS:%d Flag:0x%x, %d", __LINE__, m_iDstWidth, m_iDstHeight, m_iDstDisplayWidth, m_iDstDisplayHeight, m_llMaxFileSize, m_iBitrate, iFPS, iFlag, iSpeedFactor);

	if(m_strSrcFilePath)
	{
		nexSAL_MemFree(m_strSrcFilePath);
		m_strSrcFilePath = NULL;
	}
	m_strSrcFilePath = (char*)nexSAL_MemAlloc(strlen(pSrcFile)+1);
	strcpy(m_strSrcFilePath, pSrcFile);

	if( pDstFile != NULL){
		if(m_strDstFilePath)
		{
			nexSAL_MemFree(m_strDstFilePath);
			m_strDstFilePath = NULL;
		}
		
		m_strDstFilePath = (char*)nexSAL_MemAlloc(strlen(pDstFile)+1);
		strcpy(m_strDstFilePath, pDstFile);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_TranscodingTask::setReverseInfo(const char* pTempFile, unsigned int uStartTime, unsigned int uEndTime)
{
	if( pTempFile == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setReverseInfo Src is NULL!", __LINE__ );
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setReverseInfo TEMP(%s)", __LINE__, pTempFile);

	m_uStartTime = uStartTime;
	m_uEndTime = uEndTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] start %d, end %d", __LINE__, m_uStartTime, m_uEndTime);

	if(m_strTempFilePath)
	{
		nexSAL_MemFree(m_strTempFilePath);
		m_strTempFilePath = NULL;
	}
	m_strTempFilePath = (char*)nexSAL_MemAlloc(strlen(pTempFile)+1);

	strcpy(m_strTempFilePath, pTempFile);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_TranscodingTask::setVideoResolution(int iDstWidth, int iDstHeight)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "setVideoResolution DstWidth : %d, DstHeight: %d", __LINE__, iDstWidth, iDstHeight);

	m_iDstWidth = iDstWidth;
	m_iDstHeight = iDstHeight;
	return NEXVIDEOEDITOR_ERROR_NONE;
}
int CNEXThread_TranscodingTask::setMaxFileSize(long long llMaxFileSize)
{
	m_llMaxFileSize = llMaxFileSize;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_TranscodingTask::getVideoFinished()
{
	return m_bVideoFinished;
}

unsigned int CNEXThread_TranscodingTask::getVideoEncodingTime()
{
	return m_uEncodingTime;
}

unsigned int CNEXThread_TranscodingTask::getVideoBaseReverseTime()
{
	return m_uBaseReverseTime;
}

int CNEXThread_TranscodingTask::initRenderer()
{
	if( m_hTranscodingRenderer == NULL || m_pOutputSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initRenderer(setting Output Window) failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	NXT_ThemeRenderer_SetNativeWindow(	m_hTranscodingRenderer, (ANativeWindow*)NULL, m_iDstWidth, m_iDstHeight );	
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] setTranscodingInfo:: setting Output Window.", __LINE__);
	// SET OUTPUT RENDER SURFACE.
	NXT_ThemeRenderer_SetNativeWindow(	m_hTranscodingRenderer,
											(ANativeWindow*)m_pOutputSurface,
											m_iDstWidth,
											m_iDstHeight );

	// Case : for mantis 9228
	NXT_ThemeRenderer_ClearTransitionEffect(m_hTranscodingRenderer);
	NXT_ThemeRenderer_ClearClipEffect(m_hTranscodingRenderer);

	if( m_pUserData )
	{
		NXT_ThemeRenderer_SetWatermarkEffect(m_hTranscodingRenderer, m_pUserData, 0);
	}

	m_pTempBuffer = (unsigned char*)nexSAL_MemAlloc(ROUNDUP(m_iDstWidth, 16)*ROUNDUP(m_iDstHeight, 16)*3/2);
	if( m_pTempBuffer == NULL)
	{
		NXT_ThemeRenderer_SetNativeWindow(	m_hTranscodingRenderer, (ANativeWindow*)NULL, m_iDstWidth, m_iDstHeight );	
		return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
	}
	//Jeff-------------------------------------------------------------------------------------------------------
	if( (m_iFlag & HQ_SCALE_FLAG ) ==  HQ_SCALE_FLAG )
	{
		NXT_ThemeRenderer_AquireContext(m_hTranscodingRenderer);
		NXT_ThemeRenderer_SetHQScale(m_hTranscodingRenderer, m_iSrcWidth, m_iSrcHeight, m_iDstWidth, m_iDstHeight);
		NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 0);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NexThemeRenderer TranscodingTask.cpp %d] SetHQScaler:SrcW:%d SrcH:%d DstW:%d DstH:%d.", __LINE__, m_iSrcWidth, m_iSrcHeight, m_iDstWidth, m_iDstHeight);
	}
	//-----------------------------------------------------------------------------------------------------------

	if( m_pHighLightBackBuffer != NULL )
	{
		nexSAL_MemFree(m_pHighLightBackBuffer);
		m_pHighLightBackBuffer = NULL;
	}
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_TranscodingTask::deinitRenderer()
{
	if( m_pTempBuffer)
	{
		nexSAL_MemFree(m_pTempBuffer);
		m_pTempBuffer = NULL;
	}

	if( m_hTranscodingRenderer )
	{
		NXT_ThemeRenderer_SetNativeWindow( m_hTranscodingRenderer, (ANativeWindow*)NULL, 0, 0);
		m_hTranscodingRenderer = NULL;
	}

	if( m_pHighLightBackBuffer != NULL )
	{
		nexSAL_MemFree(m_pHighLightBackBuffer);
		m_pHighLightBackBuffer = NULL;
	}

	return 0;
}

int	CNEXThread_TranscodingTask::initFileReader()
{
	NXUINT32				uTotalPlayTime	= 0;
	NEXVIDEOEDITOR_ERROR	eRet			= NEXVIDEOEDITOR_ERROR_NONE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initFileReader In", __LINE__);
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

	CNexFileReader* pFileReader = new CNexFileReader;

	if( pFileReader == NULL )
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;

	eRet = (NEXVIDEOEDITOR_ERROR)pFileReader->createSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
		goto initFile_Error;
	}

	pFileReader->setEncodeToTranscode(CNexProjectManager::getEncodeToTranscode());
	eRet = (NEXVIDEOEDITOR_ERROR)pFileReader->openFile(m_strSrcFilePath, (unsigned int)strlen(m_strSrcFilePath));
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
		goto initFile_Error;
	}

	pFileReader->getTotalPlayTime(&uTotalPlayTime);
	m_uiTotalPlayTime = uTotalPlayTime;

	// setAudioOnOff(TRUE);
	if( !pFileReader->isVideoExist())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video is not existed. not supported format", __LINE__);
		if(m_uReverseMode == 0)
			return NEXVIDEOEDITOR_ERROR_TRANSCODING_NOT_SUPPORTED_FORMAT;
	}
	else
	{
		NXUINT32 iWidth		= 0;
		NXUINT32 iHeight	= 0;
		if( pFileReader->getVideoResolution(&iWidth, &iHeight) != NEXVIDEOEDITOR_ERROR_NONE )
		{
			eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
			goto initFile_Error;
		}
		m_iSrcWidth = iWidth;
		m_iSrcHeight = iHeight;
	}

	if( pFileReader->isAudioExist())
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] before Audio Sample(%d) Channels(%d), BitsPerSample(%d) SamplePerCh(%d)",
				__LINE__, m_uiSampleRate, m_uiChannels, m_uiBitsPerSample, m_uiSamplePerChannel);

		pFileReader->getWAVEFormat(&uFormatTag, &uBitsPerSample, &uBlockAlign, &uAvgBytesPerSec, &uSamplesPerBlock, &uEnCodeOpt, &uExtraSize, &pExtraData);
		pFileReader->getDSI(NXFF_MEDIA_TYPE_AUDIO, &m_pDSI, &m_uiDSISize );
		pFileReader->getSamplingRate(&m_uiSampleRate);
		pFileReader->getNumberOfChannel(&m_uiChannels);
		pFileReader->getSamplesPerChannel(&m_uiSamplePerChannel);

		// m_uiDSISize = 2;

		pFileReader->m_uiAudioFrameInterval		= _GetFrameInterval(m_uiSamplePerChannel, m_uiSampleRate);
		pFileReader->m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiSamplePerChannel, m_uiSampleRate);
		pFileReader->m_uiMaxAudioFrameInterval	= (pFileReader->m_uiAudioFrameInterval*3) >> 1;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio Sample(%d) Channels(%d), BitsPerSample(%d) SamplePerCh(%d)",
				__LINE__, m_uiSampleRate, m_uiChannels, m_uiBitsPerSample, m_uiSamplePerChannel);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio is not exist", __LINE__);

	}
	m_pFileReader = pFileReader;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initFileReader Out(%p)", __LINE__, m_pFileReader);
	return NEXVIDEOEDITOR_ERROR_NONE;


initFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initFileReader Failed Out(%p)", __LINE__, m_pFileReader);
	SAFE_RELEASE(pFileReader);
	return eRet;
}

int	CNEXThread_TranscodingTask::deinitFileReader()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] deinitFileReader In", __LINE__);
	SAFE_RELEASE(m_pFileReader);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] deinitFileReader Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int	CNEXThread_TranscodingTask::initWriter(unsigned int uDuration)   // iMode  0:transcoding 1: highlight 2:reverse
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initWriter In", __LINE__);
	m_pFileWriter = new CNexFileWriter;
	if( m_pFileWriter == NULL)
	{
		eRet = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [TranscodingTask.cpp %d] FileWriter create failed", __LINE__);
		goto ErrReturn;
	}

	if(uDuration == m_uiTotalPlayTime)
	{
		m_pFileWriter->setTotalDuration(m_pFileReader->getTotalPlayAudioTime(), m_pFileReader->getTotalPlayVideoTime());
	}

	if( m_pFileWriter->initFileWriter(m_llMaxFileSize, uDuration) == FALSE )
	{
		eRet = NEXVIDEOEDITOR_ERROR_FILEWRITER_CREATE_FAIL;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] FileWrite initFileWriter failed", __LINE__);
		goto ErrReturn;
	}

	if( m_pFileWriter->setFilePath(m_strDstFilePath) == FALSE )
	{
		eRet = NEXVIDEOEDITOR_ERROR_FILEWRITER_CREATE_FAIL;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] FileWrite setFilePath failed", __LINE__);
		goto ErrReturn;
	}

	if(m_pFileReader->isVideoExist())
	{
		if(m_uReverseMode == 1)
		{
			#ifdef TEST_REVERSE_YUV	
			if( m_pFileWriter->setVideoCodecInfo(0x50010301, m_iDstWidth, m_iDstHeight, m_iDstDisplayWidth, m_iDstDisplayHeight, m_pFileReader->getVideoFrameRate()*100, m_iBitrate, m_iEncodeProfile, m_iEncodeLevel ) == FALSE)
			#else
			if( m_pFileWriter->setVideoCodecInfo(eNEX_CODEC_V_H264, m_iDstWidth, m_iDstHeight, m_iDstDisplayWidth, m_iDstDisplayHeight, m_pFileReader->getVideoFrameRate()*100, m_iBitrate, m_iEncodeProfile, m_iEncodeLevel ) == FALSE)
			#endif
			{
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] FileWrite setVideoCodecInfo failed", __LINE__);
				goto ErrReturn;
			}
		}
		else if(m_uReverseMode == 2)
		{
			unsigned char* pBaseDSI = NULL;
			unsigned int uiBaseDSISize = 0;
			m_pFileReader->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize);	
			if( m_pFileWriter->setVideoCodecInfo(m_pFileReader->getVideoObjectType(), pBaseDSI, uiBaseDSISize) == FALSE)
			{
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] FileWrite setVideoCodecInfo failed", __LINE__);
				goto ErrReturn;
			}
		}
		else
		{
			if( m_pFileWriter->setVideoCodecInfo(eNEX_CODEC_V_H264, m_iDstWidth, m_iDstHeight, m_iDstDisplayWidth, m_iDstDisplayHeight, EDITOR_DEFAULT_FRAME_RATE, m_iBitrate, m_iEncodeProfile, m_iEncodeLevel ) == FALSE)
			{
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] FileWrite setVideoCodecInfo failed", __LINE__);
				goto ErrReturn;
			}
		}

		//
		NXT_RendererOutputType typeRenderOutput;
		m_pMediaSurface = m_pFileWriter->getMediaCodecInputSurface();
		if( m_pMediaSurface == NULL)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] MediaSurface is NULL", __LINE__);
			typeRenderOutput = NXT_RendererOutputType_Y2CrA_8888;
		}
		else
		{
			NXT_Error eThemeRet = NXT_ThemeRenderer_BeginExport( m_hTranscodingRenderer, m_iDstWidth, m_iDstHeight,  (ANativeWindow*)m_pMediaSurface );
			if( eThemeRet != NXT_Error_None )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] BeginExport is fail(%d)", __LINE__, eThemeRet);
				eRet = NEXVIDEOEDITOR_ERROR_RENDERER_INIT;
				goto ErrReturn;
			}

			typeRenderOutput = NXT_RendererOutputType_RGBA_8888;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] BeginExpot with mediacodec's input Surf", __LINE__);

		}
	}
	else
	{
		m_pFileWriter->setAudioOnlyMode(TRUE);
	}

	if( m_pFileReader->isAudioExist() && m_iSpeedFactor == 100)
	{
		if(m_uReverseMode)
		{
			if( m_pFileWriter->setAudioCodecInfo(eNEX_CODEC_A_AAC, EDITOR_DEFAULT_SAMPLERATE, EDITOR_DEFAULT_CHANNELS, 128*1024) == FALSE)
			//if( m_pFileWriter->setAudioCodecInfo(eNEX_CODEC_A_AAC, m_uiSampleRate, m_uiChannels, 128*1024) == FALSE)
			{
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] FileWrite setAudioCodecInfo failed", __LINE__);
				goto ErrReturn;
			}		
		}
		else if(IS_AUDIO_DECODING(m_pFileReader->m_uiAudioObjectType))
		{
			if( m_pFileWriter->setAudioCodecInfo(eNEX_CODEC_A_AAC, EDITOR_DEFAULT_SAMPLERATE, EDITOR_DEFAULT_CHANNELS, 128*1024) == FALSE)
			{
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] FileWrite setAudioCodecInfo failed", __LINE__);
				goto ErrReturn;
			}
		}
		else
		{
			if( m_pFileWriter->setAudioCodecInfo(m_pFileReader->m_uiAudioObjectType, m_pDSI, m_uiDSISize) == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] FileWrite setAudioCodecInfo failed", __LINE__);
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				goto ErrReturn;
			}
			m_pFileWriter->setAudioSamplingRate(m_uiSampleRate);
		}

		nexSAL_MemDump(m_pDSI, m_uiDSISize);
	}
	else
	{
		m_pFileWriter->setVideoOnlyMode(TRUE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Set Video only transcoding mode", __LINE__);
	}

	if(m_pFileReader->isVideoExist())
	{
		m_iSrcOrientation = m_pFileReader->getRotateState();
		if( (m_iFlag & 0x00000001 ) ==  0x00000001 && m_iSrcOrientation != 0 )
		{
			m_pFileWriter->setVideoRotate(0);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] dst orientataion set to %d->0", __LINE__, m_pFileReader->getReaderRotateState());
		}
		else
		{
			m_pFileWriter->setVideoRotate(m_pFileReader->getReaderRotateState());
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] src orientation info pass to dst(%d)", __LINE__, m_pFileReader->getReaderRotateState());
		}
		
		m_uiEncoderInputFormat = m_pFileWriter->getEncodeInputFormat();
	}

	if( m_pFileWriter->startFileWriter() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_VideoEditor.cpp %d] FileWrite initFileWriter failed", __LINE__);
		eRet = NEXVIDEOEDITOR_ERROR_FILEWRITER_CREATE_FAIL;
		goto ErrReturn;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initWriter Out", __LINE__);
	return eRet;
ErrReturn:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initWriter Fail Out", __LINE__);
	return eRet;
}

int	CNEXThread_TranscodingTask::deinitWriter()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] deinitWriter In", __LINE__);

	if( m_pFileWriter == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Writer instance is NULL", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	unsigned int uiDuration	= 0;
	int FWRet = 0;

	if(m_pMediaSurface != NULL)
	{
		NXT_Error eRet = NXT_ThemeRenderer_EndExport( m_hTranscodingRenderer);
		m_pMediaSurface = NULL;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] EndExport with mediacodec's input Surf", __LINE__);
	}

	FWRet = m_pFileWriter->endFileWriter(&uiDuration, m_bUserCancel );
	SAFE_RELEASE(m_pFileWriter);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] deinitWriter Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;

ErrReturn:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] deinitWriter fail Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_GENERAL;
}

NXBOOL CNEXThread_TranscodingTask::initAudioDecoder(unsigned int uiStartTS)
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

#if 0
			if( uFrameSize < 32)
				nexSAL_MemDump(pFrame, uFrameSize);
			else
				nexSAL_MemDump(pFrame, 32);
#endif
			
			/*
			**Nx_robin__110111 HLS���� Reset�ϴ� ���츸 uiStartTS�� 0�� �ƴ϶��� �Ʒ� ������ üũ�Ѵ�. 
			** Video Only���� A/V�� ��ȯ�Ǵµ�, Init�� ���ؼ� �о��� Audio CTS��, �̹� Play�� �κ��̶���, �����͸� �о ������.
			** Video Only�� 5�ʱ��� Play���Ŀ� A/V�� ��ȯ�Ǿ��µ�, Audio ù��° �����Ͱ� 2�ʿ��ٸ�, 2~5�ʱ����� �����͸� �о ������.
			*/
			if ( uiStartTS > uiCTS )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d] Discard Audio Data(%u), Target(%u)\n", __LINE__, uiCTS, uiStartTS);
				continue;
			}
			else if( uiStartTS+300 < uiCTS ) //[shoh][2011.08.24] While Audio DTS is smaller than the first frame CTS, audio needs mute process.
			{
				// hPlayer->m_uRealStartCTS = uiCTS;
				uiCTS = uiStartTS;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d] Audio Initialize Start(%u), Target(%u)\n", __LINE__, uiCTS, uiStartTS);
			}

			if( m_hCodecAudio )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d] Audio Initialize %p\n", __LINE__, m_hCodecAudio);			
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d]Need Buffering....\n", __LINE__);
			nexSAL_TaskSleep(20);
			continue;
		}
		// JDKIM 2011/02/03 : Lost Frame�� �ִ� ���� OK�� �� ������ ������.
		else if( uiReaderRet == _SRC_GETFRAME_OK_LOSTFRAME)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d] Remove Lost Frame..\n", __LINE__);
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Audio Data Read Error[0x%X].\n", __LINE__, uiReaderRet);
			goto DECODER_INIT_ERROR;
		}
	}


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_AudioDecoderInit Frame Data!",  __LINE__);
	nexSAL_MemDump(pDSI, uDSISize);
	//nexSAL_MemDump(pFrame, uFrameSize);


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Audio Decoder Init Start(S(%d) C(%d) BpS(%d) SC(%d)", 
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

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Audio Decoder Init End(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);

	if(m_pAudioDecodeBuf != NULL)
	{
		nexSAL_MemFree(m_pAudioDecodeBuf);
		m_pAudioDecodeBuf = NULL;
	}

	if( uSamplingRate == 0 || uNumOfChannels == 0 || uBitsPerSample == 0 || uNumOfSamplesPerChannel == 0 )
	{
		if(m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AMR)
		{
			uSamplingRate = 8000;
			uNumOfChannels = 1;
			uBitsPerSample = 16;
			uNumOfSamplesPerChannel = 160;
		}
		else if(m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AMRWB)
		{
			uSamplingRate = 16000;
			uNumOfChannels = 1;
			uBitsPerSample = 16;
			uNumOfSamplesPerChannel = 320;
		}
	}

	m_uiAudioDecodeBufSize = 384*1024;
	m_pAudioDecodeBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioDecodeBufSize);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d] Default  PCMBuffer(%p, %d)\n", __LINE__, m_pAudioDecodeBuf, m_uiAudioDecodeBufSize);
	
	if( iRet != NEXCAL_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Audio Decoder Init Failed(%d)",  __LINE__, iRet);
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

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Interval(%d %f %d)",  __LINE__, 
		m_pFileReader->m_uiAudioFrameInterval, 
		m_pFileReader->m_dbAudioFrameInterval, 
		m_pFileReader->m_uiMaxAudioFrameInterval);

	m_isNextAudioFrameRead = FALSE;

	// Case : mantis 9267 when decoder channel info was wrong.
	if( m_uiSampleRate == 0 || m_uiChannels == 0 || m_uiBitsPerSample == 0 || m_uiSamplePerChannel == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Wait decode config change",  __LINE__);
		goto DECODER_INIT_ERROR;
	}

	// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_FUNC, (unsigned int)_GetCurrentMediaCTS );
	// nexCAL_AudioDecoderSetProperty( hPlayer->m_hCodecAudio, NEXCAL_SET_PROPERTY_MEDIA_CLOCK_USERDATA, (unsigned int)this );

	if( m_uiSampleRate != EDITOR_DEFAULT_SAMPLERATE )
	{
		m_isResampling = TRUE;
	}

	if (m_uiChannels == 1)
	{
		m_pAudioUpChannelBuf = (unsigned char*)nexSAL_MemAlloc(384*1024);
	}

	if( m_isResampling )
	{
		m_uiAudioReSampleBufSize = 384*1024;
		m_pAudioReSampleBuf	= (unsigned char*)nexSAL_MemAlloc(m_uiAudioReSampleBufSize);

		if(FALSE == initResampler())
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask %d] initResampler open fail", __LINE__);
			goto DECODER_INIT_ERROR;
		}
	}

	return NEXVIDEOEDITOR_ERROR_NONE;

DECODER_INIT_ERROR:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Audio Decoder init failed",  __LINE__);
	deinitAudioDecoder();
	return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
}

NXBOOL CNEXThread_TranscodingTask::deinitAudioDecoder()
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

	if( m_pAudioUpChannelBuf )
	{
		nexSAL_MemFree(m_pAudioUpChannelBuf);
		m_pAudioUpChannelBuf = NULL;
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

int	CNEXThread_TranscodingTask::initVideoDecoder()
{
	CAutoLock m(CNexCodecManager::getVideoDecInitLock());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] initVideoDecoder() In", __LINE__);
	if( m_pFileReader == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( m_pFileReader->isVideoExist() == FALSE )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	deinitVideoDecoder();

	m_pCodecWrap = new CNexCodecWrap();
	if( m_pCodecWrap == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] codedwrap allocate failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
	}

	int iUseSoftCodec = 0;

	int iChipType = getCheckChipsetType();
	if( iChipType == CHIPSET_MSM8226 || iChipType == CHIPSET_MSM8926 )
	{
		if( m_iSrcWidth * m_iSrcHeight > CONTENT_720P_WIDTH * CONTENT_720P_HEIGHT )
		{
			if( CNexVideoEditor::m_bSupportSWMCH264 )
			{
				iUseSoftCodec = 2;
			}
			else if( CNexVideoEditor::m_bSuppoertSWH264Codec )
			{
				iUseSoftCodec = 1;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Can't support transcoding on this device", __LINE__);
				return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
			}
		}
	}

	while( iUseSoftCodec == 0 && m_bIsWorking )
	{
		if( CNexCodecManager::isHardwareDecodeAvailable() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d]Now available hardware codec", __LINE__);
			break;
		}
		nexSAL_TaskSleep(30);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Wait available hardware codec", __LINE__);
	}

	if( m_bIsWorking == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d]Task exit before video decoder init", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	unsigned int uiVideoType = m_pFileReader->m_uiVideoObjectType;
	if(  iUseSoftCodec  == 1 )
	{
		// Use Software Codec;
		if( uiVideoType  == eNEX_CODEC_V_MPEG4V )
		{
			uiVideoType = eNEX_CODEC_V_MPEG4V_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Used Mpeg4V Software Decoder(0x%x)", __LINE__, uiVideoType);
		}
		else if( uiVideoType  == eNEX_CODEC_V_H264 )
		{
			uiVideoType = eNEX_CODEC_V_H264_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Used AVC Software Decoder(0x%x)", __LINE__, uiVideoType);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Not Support Video Codec(0x%x)", __LINE__, uiVideoType);
			return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
		}
	}
	else if(  iUseSoftCodec  == 2 )
	{
		// Use Software Codec;
		if( uiVideoType  == eNEX_CODEC_V_H264 )
		{
			uiVideoType = eNEX_CODEC_V_H264_MC_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Used AVC MC Software Decoder(0x%x)", __LINE__, uiVideoType);
		}
		/*
		else if( uiVideoType  == eNEX_CODEC_V_MPEG4V )
		{
			uiVideoType = eNEX_CODEC_V_MPEG4V_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Used Mpeg4V Software Decoder(0x%x)", __LINE__, uiVideoType);
		}
		*/
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Not Support Video Codec(0x%x)", __LINE__, uiVideoType);
			return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
		}
	}	
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] ID(%d) Used Hardware Decoder(0x%x)", __LINE__, uiVideoType);
	}	

	NXBOOL bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
											NEXCAL_MODE_DECODER,
											uiVideoType,
											m_iSrcWidth,
											m_iSrcHeight,
											m_iSrcWidth);

	if( bRet == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Get Codec Failed(0x%x)", __LINE__, uiVideoType);
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
	}


	NXUINT8*	pFrame			= NULL;
	NXUINT32	uiFrameSize		= 0;
	NXUINT32	uiDTS			= 0;
	NXUINT32	uiPTS			= 0;
	NXBOOL		isValidVideo	= FALSE;
	NXBOOL		isIntraFrame	= FALSE;

	NXUINT32	uiRetryCnt		= 0;
	NXUINT8*	pBaseDSI		= NULL;
	NXUINT32	uiBaseDSISize	= 0;
	NXUINT8*	pNewConfig		= NULL;
	NXUINT32	uiNewConfigSize	= 0;
	NXUINT8*	pEnhancedDSI			= NULL;
	NXUINT32	uiEnhancedDSISize		= 0;
	NXUINT32	uiH264ProfileLevelID		= 0;

	NXINT64		isUseIframeVDecInit			= FALSE;
	unsigned int	eRet 					= 0;
	int 			iDSINALHeaderSize		= 0;
	int 			iFrameNALHeaderSize		= 0;

	unsigned int	uiUserDataType			= 0;

	unsigned int	iWidth					= 0;
	unsigned int	iHeight					= 0;

	if( nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	iDSINALHeaderSize = m_pFileReader->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pFileReader->getFrameNALHeaderLength();

	if( m_pCodecWrap->isHardwareCodec() )
	{
		if( m_pTranscodingDecSurface == NULL )
		{
			ANativeWindow* surface = NULL;
			NXT_ThemeRenderer_CreateSurfaceTexture( m_hTranscodingRenderer, &surface );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Transcoding.cpp %d] Created SurfaceTexture for transcoding decoding(%p)", __LINE__, surface);
			if( surface == NULL )
			{
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
			}
			m_pTranscodingDecSurface = (void*)surface;
		}

		if( m_pTranscodingDecSurface == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Transcoding.cpp %d] Created SurfaceTexture for transcoding decoder failed(%p)", __LINE__, m_pTranscodingDecSurface);
			m_pCodecWrap->deinitDecoder();
			SAFE_RELEASE(m_pCodecWrap);
			return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
		}
		m_pProjectMng->setCacheTranscodingDecSurf((void*)m_hTranscodingRenderer, (void*) m_pTranscodingDecSurface);
		m_pCodecWrap->setSurfaceTexture(m_pTranscodingDecSurface);
		if(m_iDecodeMode)
		{
			m_pCodecWrap->setSurfaceTexture(NULL);		
		}	
	}
	else
	{
		nexCAL_VideoDecoderSetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_VIDEO_BUFFER_NV12);
	}		

RETRY_INIT_DECODER:
	while(1)
	{
		unsigned int uiRet = m_pFileReader->getVideoFrame();
		if( uiRet == _SRC_GETFRAME_OK )
		{
			m_pFileReader->getVideoFrameData(&pFrame, &uiFrameSize);
			m_pFileReader->getVideoFrameDTS(&uiDTS);
			m_pFileReader->getVideoFramePTS(&uiPTS);

			isValidVideo = NexCodecUtil_IsValidVideo( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pFrame, uiFrameSize );
			if( isValidVideo == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] This Video frame is invalid", __LINE__);
				nexSAL_TaskSleep(20);
				continue;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Check I-frame CTS(%u %d)\n", __LINE__, uiDTS, iDSINALHeaderSize);
			isIntraFrame = NexCodecUtil_IsSeekableFrame((NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Check I-frame End CTS(%u)\n", __LINE__, uiDTS);
			if( isIntraFrame )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] I-frame searched...CTS(%u)", __LINE__, uiDTS);
				break;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] P-frame searched...CTS(%u)", __LINE__, uiDTS);
			}
		}
		else if( uiRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TaskSleep(20);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Need buffering Not normal condition(0x%x)", __LINE__, uiRet);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d]  Get Frame fail while decoder init(0x%x)", __LINE__, uiRet);
			goto DECODER_INIT_ERROR;
		}
	}

	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pFileReader->m_uiVideoObjectType)
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pFileReader->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Dec init(%p %p %d)", __LINE__, m_pCodecWrap->getCodecHandle(), pBaseDSI, uiBaseDSISize);

			eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType,
												pBaseDSI,
												uiBaseDSISize,
												pFrame,
												uiFrameSize,
												NULL,
												NULL,
												0,
												&m_iSrcWidth,
												&m_iSrcHeight,
												&m_iSrcPitch,
												m_pFileReader->getVideoFrameRate(),
												NEXCAL_VDEC_MODE_NONE);

			if( eRet != NEXCAL_ERROR_NONE )
			{
				if ( eRet == NEXCAL_ERROR_CORRUPTED_FRAME && uiRetryCnt < 1 )
				{
					uiRetryCnt++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Video Codec initialize Error. So retry[%d]", __LINE__, uiRetryCnt);
					goto RETRY_INIT_DECODER;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Video Codec initialize Error.", __LINE__);
					m_pCodecWrap->deinitDecoder();
					SAFE_RELEASE(m_pCodecWrap);
					goto DECODER_INIT_ERROR;
				}
			}
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pFileReader->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pFileReader->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pFileReader->getVideoResolution(&iWidth, &iHeight);
			m_iSrcWidth		= iWidth;
			m_iSrcHeight	= iHeight;
			m_iSrcPitch		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Dec init %p %d", __LINE__, pBaseDSI, uiBaseDSISize);

			if( NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES == isUseIframeVDecInit )
			{
				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType,
													pBaseDSI,
													uiBaseDSISize,
													pFrame,
													uiFrameSize,
													NULL,
													NULL,
													iFrameNALHeaderSize,
													&m_iSrcWidth,
													&m_iSrcHeight,
													&m_iSrcPitch,
													m_pFileReader->getVideoFrameRate(),													
													NEXCAL_VDEC_MODE_NONE);
			}
			else
			{
				m_pFileReader->getDSI( NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, &pEnhancedDSI, &uiEnhancedDSISize );

				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType,
													pBaseDSI,
													uiBaseDSISize,
													pEnhancedDSI,
													uiEnhancedDSISize,
													NULL,
													NULL,
													iFrameNALHeaderSize,
													&m_iSrcWidth,
													&m_iSrcHeight,
													&m_iSrcPitch,
													m_pFileReader->getVideoFrameRate(),													
													NEXCAL_VDEC_MODE_NONE);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Dec init End(%d)", __LINE__, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				if ( uiRetryCnt < 1 )
				{
					uiRetryCnt++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Video Codec initialize Error. So retry[%d]", __LINE__, uiRetryCnt);
					goto RETRY_INIT_DECODER;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Video Codec initialize Error.", __LINE__);
					m_pCodecWrap->deinitDecoder();
					SAFE_RELEASE(m_pCodecWrap);
					goto DECODER_INIT_ERROR;
				}
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] VideoDecoder_Initialized() : W[%d], H[%d], P[%d]", __LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch);
			break;
		default:
			goto DECODER_INIT_ERROR;
	};

	m_iSettingWidth = m_iSrcWidth;
	m_iSettingHeight = m_iSrcHeight;
	m_iSettingPitch = m_iSrcPitch;

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	if( m_pCodecWrap->isHardwareCodec() == FALSE )
	{
		if( m_pSoftwareTempBuffer != NULL )
		{
			nexSAL_MemFree(m_pSoftwareTempBuffer);
			m_pSoftwareTempBuffer = NULL;
		}

		m_pSoftwareTempBuffer = (unsigned char*)nexSAL_MemAlloc(m_iSrcPitch * m_iSrcHeight * 3 / 2);
		if( m_pSoftwareTempBuffer == NULL )
		{
			goto DECODER_INIT_ERROR;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] ColorFormat(0x%x) HeaderSize(%d)", __LINE__, m_pCodecWrap->getFormat(), iDSINALHeaderSize);
	m_isNextVideoFrameRead = FALSE;

	if( m_pMediaSurface == NULL )
	{
		NXT_ThemeRenderer_AquireContext(m_hTranscodingRenderer);
		NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_Y2CrA_8888 , 1);
		NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
	}

	return NEXVIDEOEDITOR_ERROR_NONE;

DECODER_INIT_ERROR:
	deinitVideoDecoder();
	return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
}

NXBOOL CNEXThread_TranscodingTask::reinitVideoDecoder(unsigned char* pIDRFrame, unsigned int uiSize)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] reinitVideoDecoder In", __LINE__);

	if( m_pCodecWrap == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] reinitVideoDecoder failed(%p)", __LINE__, m_pCodecWrap);
		return FALSE;
	}
	
	NEXCALCodecHandle hCodecVideo = CNexCodecManager::getCodec( 	NEXCAL_MEDIATYPE_VIDEO,
																	NEXCAL_MODE_DECODER,
																	m_pFileReader->m_uiVideoObjectType,
																	m_iSrcWidth,
																	m_iSrcHeight,
																	m_iSrcWidth);
	if( hCodecVideo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] get HW Decoder failed", __LINE__);
		return FALSE;
	}

	deinitVideoDecoder();

	m_pCodecWrap = new CNexCodecWrap();

	m_pCodecWrap->setCodec(hCodecVideo);

	NXUINT32	uiDTS			= 0;
	NXBOOL		isValidVideo	= FALSE;
	NXBOOL		isIntraFrame	= FALSE;

	NXUINT8*	pBaseDSI		= NULL;
	NXUINT32	uiBaseDSISize	= 0;
	NXUINT8*	pNewConfig		= NULL;
	NXUINT32	uiNewConfigSize	= 0;
	NXUINT8*	pEnhancedDSI			= NULL;
	NXUINT32	uiEnhancedDSISize		= 0;
	NXUINT32	uiH264ProfileLevelID	= 0;

	NXINT64			isUseIframeVDecInit			= 0;
	unsigned int	eRet						= 0;

	int 			iDSINALHeaderSize			= 0;
	int 			iFrameNALHeaderSize			= 0;

	unsigned int	uiUserDataType				= 0;
	unsigned int	iWidth						= 0;
	unsigned int	iHeight						= 0;

	void*			pSurfaceTexture				= NULL;

	iDSINALHeaderSize = m_pFileReader->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pFileReader->getFrameNALHeaderLength();

	if( m_pCodecWrap->isHardwareCodec() )
	{
		if( m_pTranscodingDecSurface == NULL )
		{
			ANativeWindow* surface = NULL;
			NXT_ThemeRenderer_CreateSurfaceTexture( m_hTranscodingRenderer, &surface );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Transcoding.cpp %d] Created SurfaceTexture for transcoding decoding(%p)", __LINE__, surface);
			if( surface == NULL )
			{
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				return FALSE;
			}
			m_pTranscodingDecSurface = (void*)surface;
		}

		m_pProjectMng->setCacheTranscodingDecSurf((void*)m_hTranscodingRenderer, (void*) m_pTranscodingDecSurface);
		m_pCodecWrap->setSurfaceTexture(m_pTranscodingDecSurface);
		if(m_iDecodeMode)
		{
			m_pCodecWrap->setSurfaceTexture(NULL);		
		}	
	}
	else
	{
		nexCAL_VideoDecoderSetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_VIDEO_BUFFER_NV12);
	}		

	switch(m_pFileReader->m_uiVideoObjectType)
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pFileReader->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Dec init(%p %p %d)", __LINE__, m_pCodecWrap->getCodecHandle(), pBaseDSI, uiBaseDSISize);

			eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType,
												pBaseDSI,
												uiBaseDSISize,
												pIDRFrame,
												uiSize,
												NULL,
												NULL,
												0,
												&m_iSrcWidth,
												&m_iSrcHeight,
												&m_iSrcPitch,
												m_pFileReader->getVideoFrameRate(),												
												NEXCAL_VDEC_MODE_NONE);

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Video Codec initialize Error(%d).", __LINE__, eRet);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pFileReader->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pFileReader->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pFileReader->getVideoResolution(&iWidth, &iHeight);
			m_iSrcWidth		= iWidth;
			m_iSrcHeight	= iHeight;
			m_iSrcPitch		= 0;

			isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;
			nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Dec init %p %d", __LINE__, pBaseDSI, uiBaseDSISize);

			if( isUseIframeVDecInit ==  NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES )
			{
				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType,
													pBaseDSI,
													uiBaseDSISize,
													pIDRFrame,
													uiSize,
													NULL,
													NULL,
													iFrameNALHeaderSize,
													&m_iSrcWidth,
													&m_iSrcHeight,
													&m_iSrcPitch,
													m_pFileReader->getVideoFrameRate(),													
													NEXCAL_VDEC_MODE_NONE);
			}
			else
			{
				m_pFileReader->getDSI( NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, &pEnhancedDSI, &uiEnhancedDSISize );

				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType,
													pBaseDSI,
													uiBaseDSISize,
													pEnhancedDSI,
													uiEnhancedDSISize,
													NULL,
													NULL,
													iFrameNALHeaderSize,
													&m_iSrcWidth,
													&m_iSrcHeight,
													&m_iSrcPitch,
													m_pFileReader->getVideoFrameRate(),													
													NEXCAL_VDEC_MODE_NONE);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Dec init End(%d)", __LINE__, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Video Codec initialize Error(%d).", __LINE__, eRet);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] VideoDecoder_Initialized() : W[%d], H[%d], P[%d]", __LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch);
			break;
		default:
			goto DECODER_INIT_ERROR;
	};

	m_iSettingWidth = m_iSrcWidth;
	m_iSettingHeight = m_iSrcHeight;
	m_iSettingPitch = m_iSrcPitch;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] reinitVideoDecoder Codec Reinit Sucessed colorFormat(%d)", __LINE__, m_pCodecWrap->getFormat());
	m_isNextVideoFrameRead	= FALSE;
	return NEXVIDEOEDITOR_ERROR_NONE;

DECODER_INIT_ERROR:
	deinitVideoDecoder();
	return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
}
int	CNEXThread_TranscodingTask::deinitVideoDecoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] deinitVideoDecoder In", __LINE__);
	if( m_pCodecWrap != NULL )
	{
		m_pCodecWrap->deinitDecoder();
		SAFE_RELEASE(m_pCodecWrap);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] deinitVideoDecoder Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

void CNEXThread_TranscodingTask::processEnhancedFrame(unsigned int* pEnhancement)
{
	if( m_pFileReader->m_isEnhancedExist )
	{
		unsigned int uiBaseTS		= 0;
		unsigned int uiEnhanceTS	= 0;
		m_pFileReader->getTS( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &uiBaseTS );
		m_pFileReader->getTS( NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, &uiEnhanceTS );
		if( uiBaseTS > uiEnhanceTS )
		{
			*pEnhancement |= NEXCAL_VDEC_FLAG_ENHANCEMENT;
			m_pFileReader->getVideoFrame(TRUE);
		}
	}
}

int CNEXThread_TranscodingTask::completeTranscoding()
{
	// Make Clip.
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] completeTranscoding In", __LINE__);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] completeTranscoding Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNEXThread_TranscodingTask::isEOSFlag(unsigned int uiFlag)
{
	if( (uiFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
		return TRUE;
	return FALSE;
}

NXBOOL CNEXThread_TranscodingTask::writeAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
	return m_pFileWriter->setAudioFrame(uiCTS, pFrame, uiFrameSize);
}

NXBOOL CNEXThread_TranscodingTask::writeVideoFrame(void* pMediaBuffer, unsigned int uiCTS, unsigned int bWriteEnd)
{
	//if( pMediaBuffer == NULL ) return FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame Begin bEnd(%d)", __LINE__, bWriteEnd);

	if( m_hTranscodingRenderer )
	{
		//	for jira KM-4015
		if(uiCTS == 1)
			uiCTS = 0;
	
		unsigned int uiDuration	= 0;
		unsigned int uiSize		= 0;
		NXBOOL bEncodeEnd		= FALSE;
		unsigned int uiLastTime = uiCTS;
		int iRetry = nexSAL_GetTickCount();
		if(bWriteEnd)
		{
			if(m_pMediaSurface)
			{
				while( (nexSAL_GetTickCount() -  iRetry) < 1000 && m_uiReqEncodeVideoCount != m_uiEncodedVideoCount )
				{
					bEncodeEnd = FALSE;
					uiLastTime += 33;
					m_pFileWriter->setBaseVideoFrame(uiLastTime, FALSE, &uiDuration, &uiSize, &bEncodeEnd);
					if(bEncodeEnd)
						m_uiEncodedVideoCount++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] remained buffer Count(%d %d)", __LINE__, m_uiReqEncodeVideoCount, m_uiEncodedVideoCount);
					// NXT_ThemeRenderer_ReleaseContext(m_hExportThemeRenderer, 0);
					nexSAL_TaskSleep(30);
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Flush Video Frames", __LINE__);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] No Need to Flush Video Frames", __LINE__);
			}
		}
		else
		{
			m_perfDrawAll.CheckModuleUnitStart();
			NXT_ThemeRenderer_AquireContext(m_hTranscodingRenderer);

			NXT_ThemeRenderer_SetSurfaceTexture(m_hTranscodingRenderer,
												0, //track_id ..... there's no need for specifying it in this situation
												NXT_TextureID_Video_1,
												m_iSrcWidth,
												m_iSrcHeight,
												m_iSrcPitch,
												m_iSrcHeight,
												(ANativeWindow *)m_pTranscodingDecSurface,
												0,   //convert_rgb_flag
												0,   //tex_id_for_lut
												0,   //tex_id_for_customlut_a
												0,   //tex_id_for_customlut_b
												0   //tex_id_for_customlut_power
			);

			NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, 0, 0);
			// It is a function of applyPosition() when exporting.
			NXT_ThemeRenderer_SetTextureInputRect((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, 0, 0, 100000,  100000);

			if( (m_iFlag & 0x00000001 ) ==  0x00000001 )
			{
				NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, (unsigned int)m_iSrcOrientation);
			}
			
			if( m_pMediaSurface)
			{
				if( m_pYUVFRAME != NULL )
				{
					int iTranscodingWidth			= 0;
					int iTranscodingHeight		= 0;
					int iTranscodingSize			= 0;
					unsigned char* pTranscodingBuffer	= NULL;

					NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);

					/*
					glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
					glEnable(GL_SCISSOR_TEST);
					glScissor(0.75, 0.75, 1, 1);
					glClearColor(1.0, 0, 0, 1.0);
					glClear(GL_COLOR_BUFFER_BIT);	
					glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
					*/
					
					NXT_Error eRet = NXT_ThemeRenderer_GetPixels(m_hTranscodingRenderer, &iTranscodingWidth, &iTranscodingHeight, &iTranscodingSize, &pTranscodingBuffer, FALSE);
					if( eRet == NXT_Error_None )
					{
						unsigned int uiBufferSize = m_iDstWidth * m_iDstHeight * 3 / 2;
						unsigned char* pY = m_pTempBuffer;
						unsigned char* pUV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
						unsigned char* pU = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
						unsigned char* pV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight + (m_iDstWidth * m_iDstHeight/4));

						nexYYUVAtoYUV420(m_iDstWidth, m_iDstHeight, pTranscodingBuffer, pY, pU, pV);
						fwrite(m_pTempBuffer, sizeof(char), uiBufferSize, m_pYUVFRAME);	
					}
				}
				
				m_perfGLDraw.CheckModuleUnitStart();
				NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_RGBA_8888, 1);

				/*
				glEnable(GL_SCISSOR_TEST);
				glScissor(640, 0, 640, 360);
				glClearColor(1.0, 0, 0, 1.0);
				glClear(GL_COLOR_BUFFER_BIT);	
				glDisable(GL_SCISSOR_TEST);
				*/

				if( m_bNeedRenderFinish )
					NXT_ThemeRenderer_GLWaitToFinishRendering(m_hTranscodingRenderer);

				m_perfGLDraw.CheckModuleUnitEnd();

				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] DrawEnd", __LINE__);
				m_pFileWriter->setMediaCodecTimeStamp(uiCTS);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame Begin Swap", __LINE__);
				m_perfSwapbuffer.CheckModuleUnitStart();
				NXT_ThemeRenderer_SwapBuffers(m_hTranscodingRenderer);
				m_perfSwapbuffer.CheckModuleUnitEnd();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame end Swap", __LINE__);
				
				NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 0);
				
				if( m_pFileWriter->setBaseVideoFrame(uiCTS, FALSE, &uiDuration, &uiSize, &bEncodeEnd) == FALSE )
				{
					return FALSE;
				}

				m_uiReqEncodeVideoCount++;
				if(bEncodeEnd)
					m_uiEncodedVideoCount++;
				m_perfDrawAll.CheckModuleUnitEnd();
				return TRUE;
			}
			else
			{
				int iTranscodingWidth			= 0;
				int iTranscodingHeight			= 0;
				int iTranscodingSize			= 0;
				unsigned char* pTranscodingBuffer	= NULL;

				m_perfGLDraw.CheckModuleUnitStart();
				NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);
				NXT_ThemeRenderer_GLWaitToFinishRendering(m_hTranscodingRenderer);
				m_perfGLDraw.CheckModuleUnitEnd();

				m_perfGetPixels.CheckModuleUnitStart();
				NXT_Error eRet = NXT_ThemeRenderer_GetPixels(m_hTranscodingRenderer, &iTranscodingWidth, &iTranscodingHeight, &iTranscodingSize, &pTranscodingBuffer, FALSE);
				m_perfGetPixels.CheckModuleUnitEnd();
				if( eRet == NXT_Error_None )
				{
					unsigned int uiDuration = 0;
					unsigned int uiSize = 0;

					unsigned int uiBufferSize = m_iDstWidth * m_iDstHeight * 3 / 2;
					unsigned char* pY = m_pTempBuffer;
					unsigned char* pUV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
					unsigned char* pU = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
					unsigned char* pV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight + (m_iDstWidth * m_iDstHeight/4));
					m_perfVideoY2UV2YUV.CheckModuleUnitStart();
					switch(m_uiEncoderInputFormat)
					{
						case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
#if defined(__ARM_ARCH_7__)
							seperatorByte( m_iDstWidth, m_iDstHeight,  pTranscodingBuffer, pY, pUV);
#else
							nexYYUVtoY2UV(m_iDstWidth, m_iDstHeight,  pTranscodingBuffer, pY, pUV);
#endif
							break;
						case NEXCAL_PROPERTY_VIDEO_BUFFER_NV21:
							nexYYUVtoY2VU(m_iDstWidth, m_iDstHeight,  pTranscodingBuffer, pY, pUV);
							break;
						case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
							nexYYUVAtoYUV420(m_iDstWidth, m_iDstHeight, pTranscodingBuffer, pY, pU, pV);
							break;
						case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420:
							nexYYUVAtoYUV420(m_iDstWidth, m_iDstHeight, pTranscodingBuffer, pY, pV, pU);
							break;
						default:
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Not support encode format", __LINE__);
							break;
					};
					m_perfVideoY2UV2YUV.CheckModuleUnitEnd();
					if( m_pFileWriter->setBaseVideoFrame(uiCTS, m_pTempBuffer, uiBufferSize, &uiDuration, &uiSize) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Video Frame Write failed", __LINE__);
						NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
						return FALSE;
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] read Transcoding failed", __LINE__);
					NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
					return FALSE;
				}
			}

			NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
			m_perfDrawAll.CheckModuleUnitEnd();
		}

	}
	return TRUE;
}

NXBOOL CNEXThread_TranscodingTask::writeVideoFrame(unsigned int uiCTS, unsigned char* pNV12Buffer)
{
	if( pNV12Buffer == NULL ) return FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame Begin bEnd", __LINE__);

	//	for jira KM-4015
	if(uiCTS == 1)
		uiCTS = 0;

	if( m_hTranscodingRenderer )
	{
		unsigned int uiDuration	= 0;
		unsigned int uiSize		= 0;
		NXBOOL bEncodeEnd		= FALSE;
		unsigned int uiLastTime = uiCTS;
		int iRetry = nexSAL_GetTickCount();

		m_perfDrawAll.CheckModuleUnitStart();
		NXT_ThemeRenderer_AquireContext(m_hTranscodingRenderer);

		NXT_ThemeRenderer_UploadTexture(	(NXT_HThemeRenderer )m_hTranscodingRenderer,
											0,
											NXT_TextureID_Video_1,
											m_iSrcWidth,
											m_iSrcHeight,
											m_iSrcPitch,
											NXT_PixelFormat_NV12,
											pNV12Buffer,
											pNV12Buffer + (m_iSrcPitch * m_iSrcHeight),
											NULL,
											0, //lut
											0, //custom_lut_a
											0, //custom_lut_b
											0, //custom_lut_power
											NULL); //render_info

		NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, 0, 0);

		if( m_pMediaSurface)
		{
			if( m_pYUVFRAME != NULL )
			{
				int iTranscodingWidth			= 0;
				int iTranscodingHeight		= 0;
				int iTranscodingSize			= 0;
				unsigned char* pTranscodingBuffer	= NULL;

				NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);

				/*
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
				glEnable(GL_SCISSOR_TEST);
				glScissor(0.75, 0.75, 1, 1);
				glClearColor(1.0, 0, 0, 1.0);
				glClear(GL_COLOR_BUFFER_BIT);	
				glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
				*/
				
				NXT_Error eRet = NXT_ThemeRenderer_GetPixels(m_hTranscodingRenderer, &iTranscodingWidth, &iTranscodingHeight, &iTranscodingSize, &pTranscodingBuffer, FALSE);
				if( eRet == NXT_Error_None )
				{
					unsigned int uiBufferSize = m_iDstWidth * m_iDstHeight * 3 / 2;
					unsigned char* pY = m_pTempBuffer;
					unsigned char* pUV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
					unsigned char* pU = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
					unsigned char* pV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight + (m_iDstWidth * m_iDstHeight/4));

					nexYYUVAtoYUV420(m_iDstWidth, m_iDstHeight, pTranscodingBuffer, pY, pU, pV);
					fwrite(m_pTempBuffer, sizeof(char), uiBufferSize, m_pYUVFRAME);	
				}
			}
			
			m_perfGLDraw.CheckModuleUnitStart();
			NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_RGBA_8888, 1);

			if( m_bNeedRenderFinish )
				NXT_ThemeRenderer_GLWaitToFinishRendering(m_hTranscodingRenderer);

			m_perfGLDraw.CheckModuleUnitEnd();

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] DrawEnd", __LINE__);
			m_pFileWriter->setMediaCodecTimeStamp(uiCTS);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame Begin Swap", __LINE__);
			m_perfSwapbuffer.CheckModuleUnitStart();
			NXT_ThemeRenderer_SwapBuffers(m_hTranscodingRenderer);
			m_perfSwapbuffer.CheckModuleUnitEnd();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame end Swap", __LINE__);
			
			NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 0);
			
			if( m_pFileWriter->setBaseVideoFrame(uiCTS, FALSE, &uiDuration, &uiSize, &bEncodeEnd) == FALSE )
			{
				return FALSE;
			}

			m_uiReqEncodeVideoCount++;
			if(bEncodeEnd)
				m_uiEncodedVideoCount++;
			m_perfDrawAll.CheckModuleUnitEnd();
			return TRUE;
		}
		else
		{
			int iTranscodingWidth			= 0;
			int iTranscodingHeight			= 0;
			int iTranscodingSize			= 0;
			unsigned char* pTranscodingBuffer	= NULL;

			m_perfGLDraw.CheckModuleUnitStart();
			NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);
			NXT_ThemeRenderer_GLWaitToFinishRendering(m_hTranscodingRenderer);
			m_perfGLDraw.CheckModuleUnitEnd();

			m_perfGetPixels.CheckModuleUnitStart();
			NXT_Error eRet = NXT_ThemeRenderer_GetPixels(m_hTranscodingRenderer, &iTranscodingWidth, &iTranscodingHeight, &iTranscodingSize, &pTranscodingBuffer, FALSE);
			m_perfGetPixels.CheckModuleUnitEnd();
			if( eRet == NXT_Error_None )
			{
				unsigned int uiDuration = 0;
				unsigned int uiSize = 0;

				unsigned int uiBufferSize = m_iDstWidth * m_iDstHeight * 3 / 2;
				unsigned char* pY = m_pTempBuffer;
				unsigned char* pUV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
				unsigned char* pU = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
				unsigned char* pV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight + (m_iDstWidth * m_iDstHeight/4));
				m_perfVideoY2UV2YUV.CheckModuleUnitStart();
				switch(m_uiEncoderInputFormat)
				{
					case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
#if defined(__ARM_ARCH_7__)
						seperatorByte( m_iDstWidth, m_iDstHeight,  pTranscodingBuffer, pY, pUV);
#else
						nexYYUVtoY2UV(m_iDstWidth, m_iDstHeight,  pTranscodingBuffer, pY, pUV);
#endif
						break;
					case NEXCAL_PROPERTY_VIDEO_BUFFER_NV21:
						nexYYUVtoY2VU(m_iDstWidth, m_iDstHeight,  pTranscodingBuffer, pY, pUV);
						break;
					case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
						nexYYUVAtoYUV420(m_iDstWidth, m_iDstHeight, pTranscodingBuffer, pY, pU, pV);
						break;
					case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420:
						nexYYUVAtoYUV420(m_iDstWidth, m_iDstHeight, pTranscodingBuffer, pY, pV, pU);
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Not support encode format", __LINE__);
						break;
				};
				m_perfVideoY2UV2YUV.CheckModuleUnitEnd();
				if( m_pFileWriter->setBaseVideoFrame(uiCTS, m_pTempBuffer, uiBufferSize, &uiDuration, &uiSize) == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Video Frame Write failed", __LINE__);
					NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
					return FALSE;
					
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] read Transcoding failed", __LINE__);
				NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
				return FALSE;
			}
		}

		NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
		m_perfDrawAll.CheckModuleUnitEnd();

	}
	return TRUE;
}

NXBOOL CNEXThread_TranscodingTask::saveVideoYUV(void* pMediaBuffer, unsigned int uiCTS)
{
	//if( pMediaBuffer == NULL ) return FALSE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] saveVideoYUV Begin (%d)", __LINE__, uiCTS);

	if( m_hTranscodingRenderer )
	{
		m_perfDrawAll.CheckModuleUnitStart();
		NXT_ThemeRenderer_AquireContext(m_hTranscodingRenderer);

		NXT_ThemeRenderer_SetSurfaceTexture(m_hTranscodingRenderer,
											0, //track_id ..... there's no need for specifying it in this situation
											NXT_TextureID_Video_1,
											m_iSrcWidth,
											m_iSrcHeight,
											m_iSrcPitch,
											m_iSrcHeight,
											(ANativeWindow *)m_pTranscodingDecSurface,
											0,   //convert_rgb_flag
											0,   //tex_id_for_lut
											0,   //tex_id_for_customlut_a
											0,   //tex_id_for_customlut_b
											0   //tex_id_for_customlut_power
		);

		//NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, 0, 0);

		if( (m_iFlag & 0x00000001 ) ==  0x00000001 )
		{
			NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, (unsigned int)m_iSrcOrientation);
		}
		
		if( m_pMediaSurface)
		{
			int iTranscodingWidth			= 0;
			int iTranscodingHeight		= 0;
			int iTranscodingSize			= 0;
			unsigned char* pTranscodingBuffer	= NULL;

			NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);

			NXT_ThemeRenderer_GLWaitToFinishRendering(m_hTranscodingRenderer);
			
			NXT_Error eRet = NXT_ThemeRenderer_GetPixels(m_hTranscodingRenderer, &iTranscodingWidth, &iTranscodingHeight, &iTranscodingSize, &pTranscodingBuffer, FALSE);
			if( eRet == NXT_Error_None )
			{
				unsigned int uiBufferSize = m_iSrcWidth*m_iSrcHeight*3/2;
				unsigned char* pY = m_pTempBuffer;
				unsigned char* pUV = m_pTempBuffer + (m_iSrcWidth * m_iSrcHeight);
				unsigned int uSize_a, uSize_b, uSize_c;
				unsigned int uRetryCount = 0;
				nexYYUVtoY2UV(m_iSrcWidth, m_iSrcHeight,  pTranscodingBuffer, pY, pUV);

				m_vecNexReverseYUVInfo.insert(m_vecNexReverseYUVInfo.end(), m_uYuvTotalSize);

				while(uRetryCount < 50)
				{
					uSize_a = nexSAL_FileWrite(m_pYuvFile, &uiCTS, sizeof(uiCTS));
					uSize_b = nexSAL_FileWrite(m_pYuvFile, &uiBufferSize, sizeof(uiBufferSize));
					uSize_c = nexSAL_FileWrite(m_pYuvFile, m_pTempBuffer, uiBufferSize);

					if(uSize_a == sizeof(uiCTS) && uSize_b == sizeof(uiBufferSize) && uSize_c == uiBufferSize)
						break;
					else
						nexSAL_TaskSleep(5);

					nexSAL_FileSeek(m_pYuvFile, m_uYuvTotalSize, NEXSAL_SEEK_BEGIN);
					uRetryCount++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] saveVideoYUV retry (%d), (%d), (%d), %d, %d, %d", __LINE__, uiCTS, uiBufferSize, m_uYuvTotalSize, uSize_a, uSize_b, uSize_c);					
				}

				if(uRetryCount == 50)
					return FALSE;

				m_uYuvTotalSize += (sizeof(uiCTS) + sizeof(uiBufferSize) + uiBufferSize);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] saveVideoYUV Begin (%d), (%d), (%d), (%zu)", __LINE__, uiCTS, uiBufferSize, m_uYuvTotalSize, m_vecNexReverseYUVInfo.size());
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] saveVideoYUV failed %d", __LINE__, eRet);	
				return FALSE;
			}
		}

		NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 0);
		m_perfDrawAll.CheckModuleUnitEnd();
	}
	return TRUE;
}

NXBOOL CNEXThread_TranscodingTask::saveVideoYUV(unsigned char* pData, unsigned int uSize, unsigned int uiCTS)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] saveVideoYUV Begin (%d)", __LINE__, uiCTS);
	unsigned int uSize_a, uSize_b ,uSize_c , uSize_d;
	unsigned uRetryCount = 0;
	m_vecNexReverseYUVInfo.insert(m_vecNexReverseYUVInfo.end(), m_uYuvTotalSize);

	while(uRetryCount < 50)
	{
		uSize_a = nexSAL_FileWrite(m_pYuvFile, &uiCTS, sizeof(uiCTS));
		uSize_b = nexSAL_FileWrite(m_pYuvFile, &uSize, sizeof(uSize));
		uSize_c = nexSAL_FileWrite(m_pYuvFile, pData, m_iSettingWidth*m_iSettingHeight);
		uSize_d = nexSAL_FileWrite(m_pYuvFile, pData + m_iSettingWidth*m_iSettingHeight, m_iSettingWidth*m_iSettingHeight/2);	
		
		if(uSize_a == sizeof(uiCTS) && uSize_b == sizeof(uSize) && uSize_c == m_iSettingWidth*m_iSettingHeight && uSize_d == m_iSettingWidth*m_iSettingHeight/2)
			break;
		else
			nexSAL_TaskSleep(5);

		nexSAL_FileSeek(m_pYuvFile, m_uYuvTotalSize, NEXSAL_SEEK_BEGIN);
		uRetryCount++;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] saveVideoYUV retry (%d), (%d), (%d), %d, %d, %d, %d", __LINE__, uiCTS, uSize, m_uYuvTotalSize, uSize_a, uSize_b, uSize_c, uSize_d);
	}

	if(uRetryCount == 50)
		return FALSE;

	m_uYuvTotalSize += (sizeof(uiCTS) + sizeof(uSize) + uSize);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] saveVideoYUV Begin (%d), (%d), (%d), (%zu)", __LINE__, uiCTS, uSize, m_uYuvTotalSize, m_vecNexReverseYUVInfo.size());

	return TRUE;
}

NXBOOL CNEXThread_TranscodingTask::writeVideoYUV(unsigned int uiCTS, unsigned char* pNV12Buffer, unsigned int uSize, unsigned int bWriteEnd)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoYUV Begin %d, %p", __LINE__, uiCTS, pNV12Buffer);

	unsigned int uiDuration	= 0;
	unsigned int uiSize		= 0;
	NXBOOL bEncodeEnd		= FALSE;
	unsigned int uiLastTime = uiCTS;
	int iRetry = nexSAL_GetTickCount();
	unsigned int uvSize = (m_iDecodeMode == 1)?m_iSettingWidth * m_iSettingHeight:m_iSrcWidth*m_iSrcHeight;

	unsigned char* pY = pNV12Buffer;
	unsigned char* pU = pNV12Buffer + uvSize;
	unsigned char* pV = NULL;    

	if( m_pMediaSurface )
	{
		if(bWriteEnd)
		{
			if(m_pMediaSurface)
			{
				while( (nexSAL_GetTickCount() -  iRetry) < 1000 && m_uiReqEncodeVideoCount != m_uiEncodedVideoCount )
				{
					bEncodeEnd = FALSE;
					uiLastTime += 33;
					m_pFileWriter->setBaseVideoFrame(uiLastTime, FALSE, &uiDuration, &uiSize, &bEncodeEnd);
					if(bEncodeEnd)
						m_uiEncodedVideoCount++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] remained buffer Count(%d %d)", __LINE__, m_uiReqEncodeVideoCount, m_uiEncodedVideoCount);
					// NXT_ThemeRenderer_ReleaseContext(m_hExportThemeRenderer, 0);
					nexSAL_TaskSleep(30);
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Flush Video Frames", __LINE__);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] No Need to Flush Video Frames", __LINE__);
			}
			return TRUE;
		}

		if(m_iOutputFormat == 19)
		{
			pV = pU + uvSize/4;
		}
    
		m_perfDrawAll.CheckModuleUnitStart();
		NXT_ThemeRenderer_AquireContext(m_hTranscodingRenderer);

		NXT_ThemeRenderer_SetTextureInputRect((NXT_HThemeRenderer )m_hTranscodingRenderer, (NXT_TextureID)NXT_TextureID_Video_1, 0, 0, 100000,  100000);

		NXT_ThemeRenderer_UploadTexture(	(NXT_HThemeRenderer )m_hTranscodingRenderer,
											0,
											NXT_TextureID_Video_1,
											(m_iDecodeMode == 1)?m_iSettingPitch:m_iSrcWidth,
											(m_iDecodeMode == 1)?m_iSettingHeight:m_iSrcHeight,
											(m_iDecodeMode == 1)?m_iSettingWidth:m_iSrcWidth,
											(m_iOutputFormat == 19 && m_iDecodeMode == 1)?NXT_PixelFormat_YUV:NXT_PixelFormat_NV12,
											pY,
											pU,
											pV,
											0, //lut
											0, //custom_lut_a
											0, //custom_lut_b
											0, //custom_lut_power
											NULL); //render_info

		NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, 0, 0);

		if( m_pMediaSurface)
		{			
			m_perfGLDraw.CheckModuleUnitStart();
			NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_RGBA_8888, 1);
			NXT_ThemeRenderer_GLWaitToFinishRendering(m_hTranscodingRenderer);
			m_perfGLDraw.CheckModuleUnitEnd();

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] DrawEnd", __LINE__);
			m_pFileWriter->setMediaCodecTimeStamp(uiCTS);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame Begin Swap", __LINE__);
			m_perfSwapbuffer.CheckModuleUnitStart();
			NXT_ThemeRenderer_SwapBuffers(m_hTranscodingRenderer);
			m_perfSwapbuffer.CheckModuleUnitEnd();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] writeVideoFrame end Swap", __LINE__);
			
			NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 0);
			
			if( m_pFileWriter->setBaseVideoFrame(uiCTS, FALSE, &uiDuration, &uiSize, &bEncodeEnd) == FALSE )
			{
				return FALSE;
			}

			m_uiReqEncodeVideoCount++;
			if(bEncodeEnd)
				m_uiEncodedVideoCount++;
			m_perfDrawAll.CheckModuleUnitEnd();
			return TRUE;
		}
		NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
		m_perfDrawAll.CheckModuleUnitEnd();
	}
	else
	{
		if(bWriteEnd)
		{
			unsigned int uiLastTime = uiCTS;	
			int iRetry = nexSAL_GetTickCount();
		
			while( (nexSAL_GetTickCount() -  iRetry) < 1000 && m_uiReqEncodeVideoCount != m_uiEncodedVideoCount )
			{
				bEncodeEnd = FALSE;
				uiLastTime += 33;
				m_pFileWriter->setBaseVideoFrame(uiLastTime, NULL, 0, &uiDuration, &uiSize, &bEncodeEnd);
				if(bEncodeEnd)
					m_uiEncodedVideoCount++;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] remained buffer Count(%d %d)", __LINE__, m_uiReqEncodeVideoCount, m_uiEncodedVideoCount);
				// NXT_ThemeRenderer_ReleaseContext(m_hExportThemeRenderer, 0);
				nexSAL_TaskSleep(30);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Flush Video Frames", __LINE__);
			return TRUE;
		}

		m_perfDrawAll.CheckModuleUnitStart();

		if( m_pFileWriter->setBaseVideoFrame(uiCTS, pNV12Buffer, uSize, &uiDuration, &uiSize, &bEncodeEnd) == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] encoding failed", __LINE__);		
			return FALSE;
		}

		m_uiReqEncodeVideoCount++;
		if(bEncodeEnd)
			m_uiEncodedVideoCount++;
		m_perfDrawAll.CheckModuleUnitEnd();		
	}
	return TRUE;
}

#define __ABS(x)	x<0?-x:x

NXBOOL CNEXThread_TranscodingTask::processVideoFrameForHighLight(void* pMediaBuffer, unsigned int uiCTS)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] processVideoFrameForHighLight Begin", __LINE__);

	unsigned char* pY = NULL;
	unsigned char* pUV = NULL;
	unsigned char* pU = NULL;
	unsigned char* pV = NULL;
	if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
	{
		if( m_hTranscodingRenderer == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] processVideoFrameForHighLight failed(%p)", 
				__LINE__, m_hTranscodingRenderer);
			return FALSE;
		}
		
		unsigned int uiDuration	= 0;
		unsigned int uiSize		= 0;
		NXBOOL bEncodeEnd		= FALSE;
		unsigned int uiLastTime = uiCTS;


		m_perfDrawAll.CheckModuleUnitStart();
		NXT_ThemeRenderer_AquireContext(m_hTranscodingRenderer);

		NXT_ThemeRenderer_SetSurfaceTexture(m_hTranscodingRenderer,
											0, //track_id ..... there's no need for specifying it in this situation
											NXT_TextureID_Video_1,
											m_iSrcWidth,
											m_iSrcHeight,
											m_iSrcPitch,
											m_iSrcHeight,
											(ANativeWindow *)m_pTranscodingDecSurface,
											0,   //convert_rgb_flag
											0,   //tex_id_for_lut
											0,   //tex_id_for_customlut_a
											0,   //tex_id_for_customlut_b
											0   //tex_id_for_customlut_power
		);

		NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hTranscodingRenderer, NXT_TextureID_Video_1, 0, 0);

		int iTranscodingWidth			= 0;
		int iTranscodingHeight		= 0;
		int iTranscodingSize			= 0;
		unsigned char* pTranscodingBuffer	= NULL;
		m_perfGLDraw.CheckModuleUnitStart();
		NXT_ThemeRenderer_GLDraw(m_hTranscodingRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);
		m_perfGLDraw.CheckModuleUnitEnd();

		NXT_Error eRet = NXT_ThemeRenderer_GetPixels(m_hTranscodingRenderer, &iTranscodingWidth, &iTranscodingHeight, &iTranscodingSize, &pTranscodingBuffer, FALSE);
		if( eRet != NXT_Error_None )
		{
			NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] NXT_ThemeRenderer_GetPixels failed(%d)", __LINE__, eRet);
			return FALSE;
		}
		NXT_ThemeRenderer_ReleaseContext(m_hTranscodingRenderer, 1);
		
		unsigned int uiBufferSize = m_iDstWidth * m_iDstHeight * 3 / 2;
		pY = m_pTempBuffer;
		pUV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
		pU = m_pTempBuffer + (m_iDstWidth * m_iDstHeight);
		pV = m_pTempBuffer + (m_iDstWidth * m_iDstHeight + (m_iDstWidth * m_iDstHeight/4));

		nexYYUVAtoYUV420(m_iDstWidth, m_iDstHeight, pTranscodingBuffer, pY, pU, pV);
		m_perfDrawAll.CheckModuleUnitEnd();
	}
	else
	{
		pY = (unsigned char*)pMediaBuffer;
	}

	if( m_pHighLightBackBuffer == NULL )
	{
		m_pHighLightBackBuffer = (unsigned char*)nexSAL_MemAlloc(m_iDstWidth*m_iDstHeight*3/2);
		if( m_pHighLightBackBuffer == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] mem alloc failed", __LINE__);
			return FALSE;
		}
		memcpy(m_pHighLightBackBuffer, pY, m_iDstWidth*m_iDstHeight*3/2);
		m_HighlightDiffChecker.clear();
		m_HighlightDiffChecker.addHighlightDiff(uiCTS+1, 0);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] set first highlight index frame", __LINE__);
	}
	else
	{
#if 0		
		int iDiffCount = 0;
		for(int i = 0; i < m_iDstWidth*m_iDstHeight; i++)
		{
			if(  __ABS(m_pHighLightBackBuffer[i] - m_pTempBuffer[i]) > 25 ) // 10%
			{
				iDiffCount++;
			}
		}
#else
		int firstAvg = 0;
		int secondAvg = 0;

		int iDiffCount = 0;
		for(int i = 0; i < m_iDstHeight - 4; i+=4 )
		{
			for(int j = 0; j < m_iDstWidth - 4; j+=4 )
			{
				int pos = (i*m_iDstWidth) + j;
				firstAvg = m_pHighLightBackBuffer[pos] + m_pHighLightBackBuffer[pos + 1] + m_pHighLightBackBuffer[pos + 2] + m_pHighLightBackBuffer[pos + 3];
				secondAvg = pY[pos] + pY[pos + 1] + pY[pos + 2] + pY[pos + 3];
				pos += m_iDstWidth;
				firstAvg = m_pHighLightBackBuffer[pos] + m_pHighLightBackBuffer[pos + 1] + m_pHighLightBackBuffer[pos + 2] + m_pHighLightBackBuffer[pos + 3];
				secondAvg = pY[pos] + pY[pos + 1] + pY[pos + 2] + pY[pos + 3];
				pos += m_iDstWidth;
				firstAvg = m_pHighLightBackBuffer[pos] + m_pHighLightBackBuffer[pos + 1] + m_pHighLightBackBuffer[pos + 2] + m_pHighLightBackBuffer[pos + 3];
				secondAvg = pY[pos] + pY[pos + 1] + pY[pos + 2] + pY[pos + 3];
				pos += m_iDstWidth;
				firstAvg = m_pHighLightBackBuffer[pos] + m_pHighLightBackBuffer[pos + 1] + m_pHighLightBackBuffer[pos + 2] + m_pHighLightBackBuffer[pos + 3];
				secondAvg = pY[pos] + pY[pos + 1] + pY[pos + 2] + pY[pos + 3];

				firstAvg = firstAvg / 16;
				secondAvg = secondAvg / 16;

				if(  __ABS(firstAvg - secondAvg) > 128 ) // 50%
				{
					iDiffCount++;
				}
			}
		}
#endif
		m_HighlightDiffChecker.addHighlightDiff(uiCTS+1, iDiffCount);
		
		memcpy(m_pHighLightBackBuffer, pY, m_iDstWidth*m_iDstHeight*3/2);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] processVideoFrameForHighLight(DiffCount:%d Time:%d)", __LINE__, iDiffCount, uiCTS);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] processVideoFrameForHighLight end", __LINE__);
	return TRUE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_TranscodingTask::getWriterError()
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

void CNEXThread_TranscodingTask::sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3)
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		pEditor->notifyEvent(uiEventType, uiParam1, uiParam2, uiParam3);
		SAFE_RELEASE(pEditor);
	}
}

void CNEXThread_TranscodingTask::cancelTranscoding()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0," [TranscodingTask.cpp %d] cancelTranscoding In(%d)", __LINE__, m_bUserCancel);

	m_bUserCancel = TRUE;
	if(m_pReverseAudioTask)
	{
		m_pReverseAudioTask->cancelReverseAudio();
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] cancelTranscoding Out(%d)", __LINE__, m_bUserCancel);
}

int CNEXThread_TranscodingTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	switch( pMsg->m_nMsgType )
	{
		case MESSAGE_TRANSCODING_START:
		{
			CNxMsgTranscodingInfo* pTranscodingInfo = (CNxMsgTranscodingInfo*)pMsg;
			if( pTranscodingInfo == NULL )
			{
				pTranscodingInfo->m_nMsgType = MESSAGE_TRANSCODING_DONE;
				pTranscodingInfo->m_nResult = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
				m_pProjectMng->SendCommand(pTranscodingInfo);				
				SAFE_RELEASE(pTranscodingInfo);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Transcoding task recevie message is null", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			// SetExport Render.
			setInfo(		pTranscodingInfo->m_strSrcClipPath, 
							pTranscodingInfo->m_strDstClipPath,
							pTranscodingInfo->m_iWidth,
							pTranscodingInfo->m_iHeight,
							pTranscodingInfo->m_iDisplayWidth,
							pTranscodingInfo->m_iDisplayHeight,
							pTranscodingInfo->m_iBitrate,
							pTranscodingInfo->m_llMaxFileSize,
							pTranscodingInfo->m_iFPS,
							pTranscodingInfo->m_iFlag,
							pTranscodingInfo->m_iSpeedFactor,
							pTranscodingInfo->m_pFrameRenderer,
							pTranscodingInfo->m_pOutputSurface,
							pTranscodingInfo->m_pDecSurface,
							pTranscodingInfo->m_pUserData);

			processTransCoding(pTranscodingInfo);
			
			pTranscodingInfo->m_nMsgType = MESSAGE_TRANSCODING_DONE;
			m_pProjectMng->SendCommand(pTranscodingInfo);
			SAFE_RELEASE(pTranscodingInfo);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_MAKE_HIGHLIGHT_START:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] MESSAGE_MAKE_HIGHLIGHT", __LINE__);

			CNxMsgHighLightIndex* pHighLight = (CNxMsgHighLightIndex*)pMsg;
			if( pHighLight == NULL )
			{
				pHighLight->m_nMsgType = MESSAGE_MAKE_HIGHLIGHT_START_DONE;
				pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
				m_pProjectMng->SendCommand(pHighLight);				
				SAFE_RELEASE(pHighLight);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Transcoding task recevie message is null", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			m_iDecodeMode = pHighLight->m_iDecodeMode;
			m_iHighLightOutputMode = pHighLight->m_iOutputMode;
			setInfo(		pHighLight->m_strSrcClipPath,
							pHighLight->m_strDstClipPath,
							320,
							240,
							320,
							240,
							pHighLight->m_iBitrate,
							pHighLight->m_llMaxFileSize,
							EDITOR_DEFAULT_FRAME_RATE,
							0, 100,
							pHighLight->m_pFrameRenderer,
							pHighLight->m_pOutputSurface,
							pHighLight->m_pDecSurface,
							pHighLight->m_pUserData);
			processHighLightIndex(pHighLight);
			
			if( m_HighlightDiffChecker.getHighlightTimeCount() <= 0 )
			{
				pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_GET_INDEX_ERROR;
				pHighLight->m_nMsgType = MESSAGE_MAKE_HIGHLIGHT_START_DONE;
				m_pProjectMng->SendCommand(pHighLight);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;
			}

			if(pHighLight->m_nResult)
			{
				pHighLight->m_nMsgType = MESSAGE_MAKE_HIGHLIGHT_START_DONE;
				m_pProjectMng->SendCommand(pHighLight);
				SAFE_RELEASE(pHighLight);
				return MESSAGE_PROCESS_OK;			
			}

			if(m_iHighLightOutputMode == 1)
			{
				setInfo(		pHighLight->m_strSrcClipPath,
							pHighLight->m_strDstClipPath,
							pHighLight->m_iWidth,
							pHighLight->m_iHeight,
							pHighLight->m_iWidth,
							pHighLight->m_iHeight,
							pHighLight->m_iBitrate,
							pHighLight->m_llMaxFileSize,
							EDITOR_DEFAULT_FRAME_RATE,
							0, 100,
							pHighLight->m_pFrameRenderer,
							pHighLight->m_pOutputSurface,
							pHighLight->m_pDecSurface,
							pHighLight->m_pUserData);
				
				processHighLight(pHighLight);
			}
			else
			{
				CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
				if( pEditor )
				{
					int iCount = m_HighlightDiffChecker.getHighlightTimeCount();
					int* pData = (int*)malloc(sizeof(int)*iCount);

					for(int i=0; i< iCount; i++)
					{
						pData[i] = m_HighlightDiffChecker.getHighlightTime(i);
					}

					pEditor->callbackHighLightIndex(iCount, pData);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] after callbackHighLightIndex", __LINE__);
					SAFE_RELEASE(pEditor);
				}
			}
			
			pHighLight->m_nMsgType = MESSAGE_MAKE_HIGHLIGHT_START_DONE;
			m_pProjectMng->SendCommand(pHighLight);
			SAFE_RELEASE(pHighLight);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_MAKE_REVERSE_START:
		{
			CNxMsgReverseInfo* pReverseInfo = (CNxMsgReverseInfo*)pMsg;
			if( pReverseInfo == NULL )
			{
				pReverseInfo->m_nMsgType = MESSAGE_MAKE_REVERSE_START_DONE;
				pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
				m_pProjectMng->SendCommand(pReverseInfo);				
				SAFE_RELEASE(pReverseInfo);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] Transcoding task recevie message is null", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			m_iDecodeMode = pReverseInfo->m_iDecodeMode;			
			// SetExport Render.
			setInfo(		pReverseInfo->m_strSrcClipPath,
							pReverseInfo->m_strDstClipPath,
							pReverseInfo->m_iWidth,
							pReverseInfo->m_iHeight,
							pReverseInfo->m_iWidth,
							pReverseInfo->m_iHeight,
							pReverseInfo->m_iBitrate,
							pReverseInfo->m_llMaxFileSize,
							EDITOR_DEFAULT_FRAME_RATE,
							pReverseInfo->m_iFlag,
							100,
							pReverseInfo->m_pFrameRenderer,
							pReverseInfo->m_pOutputSurface,
							pReverseInfo->m_pDecSurface,
							pReverseInfo->m_pUserData);

			setReverseInfo(pReverseInfo->m_strTempClipPath, pReverseInfo->m_uStartTime, pReverseInfo->m_uEndTime);
			if(m_iDecodeMode == 2)
				processReverseDirect(pReverseInfo);
			else
				processReverse(pReverseInfo);
			
			pReverseInfo->m_nMsgType = MESSAGE_MAKE_REVERSE_START_DONE;
			m_pProjectMng->SendCommand(pReverseInfo);
			SAFE_RELEASE(pReverseInfo);
			return MESSAGE_PROCESS_OK;
		}		
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);	
}

void CNEXThread_TranscodingTask::processTransCoding(CNxMsgTranscodingInfo* pTranscodingInfo)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processTransCoding Start!!!", __LINE__);

	CFrameInfo FrameInfo;
	CFrameInfo*	pFrameInfo		= &FrameInfo;
	NXBOOL	bWriteAudio = FALSE;
	NXBOOL	bWriteVideo = FALSE;
	NXBOOL	bAudioFrameEnd = FALSE;
	NXBOOL	bFinishedAudio = FALSE;
	NXBOOL	bFinishedVideo = FALSE;
	NXBOOL	bVideoEndOfContent = FALSE;
	NXBOOL	bAudioEndOfContent = FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	NXBOOL	bFirstAudioFrame = TRUE;
	unsigned int uiVideoDecoderErrRet = 0;
	unsigned int uiEnhancement = 0;
	unsigned int iVideoFrameNALHeaderSize = 0;
	NXINT64	uiAudioDTS = 0;
	unsigned int uiAudioFrameSize = 0;
	unsigned char* pAudioFrame = NULL;

	unsigned int uiVideoPTS = 0;
	unsigned int uiVideoDTS = 0;
	unsigned int uiDecDTS = 0;
	unsigned int uiDecPTS = 0;
	unsigned int uiCheckGetOutputTime = 0;
	unsigned int uiVideoDecOutCTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	unsigned int uiCheckDecTime = 0;

	NXBOOL bFPSMode = FALSE;
	unsigned int  uiLastPTS = 0;
	unsigned int uiTotalFrameCount = 0;

	int iRetryCountForDecTimeOut = 0;

	int iTimeStampControlForQCDevice = 0;

	NXBOOL bEncodeFailed = FALSE;
	NXBOOL bDecodeFailed = FALSE;

	int iFPS_Transcoding_Gap = 1000/m_iFPS;
	int iFPS_Transcoding_Gap_Interval = 100000/m_iFPS;

	int iCurrProgress = 0;
	int iPrevProgress = -1;

	int iRet = 0;

	// Init File Reader
	iRet = initFileReader();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pTranscodingInfo->m_nResult = iRet;
		return;
	}
	// JIRA 2372
	/*
	// for portrait comcorded content.
	if( m_iSrcWidth < m_iSrcHeight )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Change dest width and height", __LINE__);

		int tmpWidth = m_iDstWidth;
		m_iDstWidth = m_iDstHeight;
		m_iDstHeight = tmpWidth;
	}
	*/

	if( m_pFileReader->isAudioExist() == FALSE || m_iSpeedFactor > 100 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] No Audio track", __LINE__);
		bFinishedAudio = TRUE;
	}

	iVideoFrameNALHeaderSize = m_pFileReader->getFrameNALHeaderLength();

	iRet = initRenderer();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pTranscodingInfo->m_nResult = iRet;
		return;
	}

	iRet = initWriter(m_uiTotalPlayTime);
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pTranscodingInfo->m_nResult = iRet;
		return;
	}
	
	if(IS_AUDIO_DECODING(m_pFileReader->m_uiAudioObjectType))
	{
		// Init Audio Decoder.
		iRet = initAudioDecoder(0);
		if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
		{
			pTranscodingInfo->m_nResult = iRet;
			return;
		}
	}	
	else
	{
		m_isNextAudioFrameRead = TRUE;
	}
	// Init Video Decoder.
	iRet = initVideoDecoder();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pTranscodingInfo->m_nResult = iRet;
		return;
	}

#ifdef FOR_RESAMPLING_FRAMERATE
	if( m_iFPS < m_pFileReader->getVideoFrameRate() )
	{
		bFPSMode = TRUE;
		uiLastPTS = 0;
		uiTotalFrameCount = 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] FPS re-encoding mode current FPS(%d)", __LINE__, m_pFileReader->getVideoFrameRate());
	}
#endif	

#ifdef FOR_TEST_QUALCOMM_CHIP
	if( m_pCodecWrap->isHardwareCodec() && isQualcommChip()  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Time Stamp control for QualComm devices", __LINE__);
		// do not use flow control KM-4162
		//iTimeStampControlForQCDevice = 1;
	}
#endif

	m_FrameTimeChecker.clear();

	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfGLDraw.CheckModuleStart();
	m_perfGetPixels.CheckModuleStart();
	m_perfSwapbuffer.CheckModuleStart();
	m_perfDrawAll.CheckModuleStart();

	while ( m_bIsWorking )
	{
		if( m_bUserCancel || bEncodeFailed || bDecodeFailed )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] task stop because state was changed(%d, %d %d)", 
				__LINE__, m_bUserCancel, bEncodeFailed ,bDecodeFailed);
			break;
		}
		
		// CheckStatus
		{
			iCurrProgress = (uiVideoDTS*100 / m_uiTotalPlayTime);
			if( iCurrProgress > iPrevProgress)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] TRANSCODING PROGRESS(%d %d %d)", __LINE__, iCurrProgress, uiVideoDTS, m_uiTotalPlayTime);
				sendNotifyEvent(MESSAGE_TRANSCODING_PROGRESS, iCurrProgress, uiVideoDTS, m_uiTotalPlayTime);
				iPrevProgress = iCurrProgress;
			}
		}

		if( m_pFileWriter->isVideoFrameWriteStarted() && bFinishedAudio == FALSE  && uiVideoDecOutCTS > uiAudioDTS+100 )
		{
			bWriteVideo = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Wait AudioFrame(Audio End:%d Video Time:%d audio Time:%lld)", 
				__LINE__, bFinishedAudio, uiVideoDecOutCTS, uiAudioDTS);
		}
		else
		{
			bWriteVideo = TRUE;
		}

		if(uiAudioDTS >= uiVideoDecOutCTS && bFinishedVideo == FALSE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Wait Videoframe(A:%lld VD:%d)", __LINE__, uiAudioDTS, uiVideoDecOutCTS);
			bWriteAudio = FALSE;
		}
		else
		{
			if( m_pFileWriter->isVideoFrameWriteStarted() )
				bWriteAudio = TRUE;
			else
				bWriteAudio = FALSE;
		}

		if(bWriteVideo == TRUE && bFinishedVideo == FALSE)
		{
			// Video
			if( m_isNextVideoFrameRead )
			{
				int iReaderRet = m_pFileReader->getVideoFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame End", __LINE__);
						bVideoEndOfContent = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
						bVideoEndOfContent = TRUE;
						break;
				};

				processEnhancedFrame(&uiEnhancement);
				m_isNextVideoFrameRead = FALSE;
			}

			if( bVideoEndOfContent )
			{
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}
			else
			{
				// for sync problem.
				m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
				m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );
				m_pFileReader->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );
			}

			uiDecDTS = uiVideoDTS;
			uiDecPTS = uiVideoPTS;
			if( iTimeStampControlForQCDevice && bVideoEndOfContent == FALSE )
			{
				uiDecDTS = uiDecDTS * 2;
				uiDecPTS = uiDecPTS * 2;
			}

			if( isFindIDRFrame )
			{
				bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
				if( bIDRFrame == FALSE )
				{
					m_isNextVideoFrameRead = TRUE;
					continue;
				}

				isFindIDRFrame = FALSE;
			}

			uiCheckDecTime = nexSAL_GetTickCount();
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
			nexCAL_VideoDecoderDecode(		m_pCodecWrap->getCodecHandle(),
											pVideoFrame,
											uiVideoFrameSize,
											NULL,
											uiDecDTS,
											uiDecPTS,
											uiEnhancement,
											&uiVideoDecoderErrRet );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] DecFrame End(%p, %d) DTS(%d) PTS(%d)uiRet(%d) time(%d)",
				__LINE__, pVideoFrame, uiVideoFrameSize, uiVideoDTS, uiVideoPTS, uiVideoDecoderErrRet, nexSAL_GetTickCount() - uiCheckDecTime);


			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
			{
				uiVideoDecOutCTS			= 0;
				iRetryCountForDecTimeOut	= 0;

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame End and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;

					// whenever decoding frames, add PTS on the FrameCTSVector

					if(m_bSupportFrameTimeChecker)
					{
						if( iTimeStampControlForQCDevice && bVideoEndOfContent == FALSE)
						{
							uiDecDTS = uiDecDTS / 2;
							uiDecPTS = uiDecPTS / 2;
							
							m_FrameTimeChecker.addFrameCTSVec(uiDecDTS, uiDecPTS);
						}
						else
						{
							m_FrameTimeChecker.addFrameCTSVec(uiDecDTS, uiDecPTS);
						}
					}
				}

				// for msm8x26
				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					bFinishedVideo = TRUE;
				}				

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
				{
					unsigned char*	pY			= NULL;
					unsigned char*	pU			= NULL;
					unsigned char*	pV			= NULL;
					unsigned int 	uiCheckCTS 	= 0;

					//pFrameInfo					= NULL;

					if( m_uiCheckVideoDecInit != 0 )
					{
						m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] VideoDecoder_First output( Time : %d )", __LINE__, m_uiCheckVideoDecInit );
						m_uiCheckVideoDecInit = 0;
					}

					if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
					{
						uiCheckGetOutputTime = nexSAL_GetTickCount();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
							/*
							//delete for mantis 6764
							if( (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] ID(%d) getOutFrame End(%d %d)",__LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
								break;
							}
							*/
							continue;
						}

						if( iTimeStampControlForQCDevice )
							uiVideoDecOutCTS = uiVideoDecOutCTS / 2;

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
							__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);

						if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2  ||
							( m_bSupportFrameTimeChecker && m_FrameTimeChecker.isValidFrameCTS(uiVideoDecOutCTS, TRUE) == FALSE ) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}

						if(m_pFileReader->m_isEditBox && m_pFileReader->m_uiVideoEditBoxTime)
						{
							uiVideoDecOutCTS -= m_pFileReader->m_uiVideoEditBoxTime;

							if((int)uiVideoDecOutCTS < 0) 
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput drop before EditBoxTime (%d)", __LINE__, uiVideoDecOutCTS);
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
								continue;
							}
						}

						if( m_iSpeedFactor > 100)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] check pts for down sampling frame rate(%d) (%d) (%d) (%d)", __LINE__, uiVideoDecOutCTS, uiTotalFrameCount, uiLastPTS, m_iSpeedFactor);
							uiVideoDecOutCTS = (uiVideoDecOutCTS/(m_iSpeedFactor/100));
							if( uiTotalFrameCount != 0 && (uiVideoDecOutCTS < uiLastPTS || (uiVideoDecOutCTS - uiLastPTS) < iFPS_Transcoding_Gap) )
							{
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Skip for down sampling frame rate(%d) (%d) (%d)", __LINE__, uiVideoDecOutCTS, uiLastPTS, m_iSpeedFactor);
							}
							else
							{
								m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
								NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif
								
								if( writeVideoFrame(pY, uiVideoDecOutCTS) == FALSE )
								{
									pTranscodingInfo->m_nResult = getWriterError();
									bEncodeFailed = TRUE;
									continue;
								}
								uiLastPTS = (uiTotalFrameCount*iFPS_Transcoding_Gap_Interval)/100;
								uiTotalFrameCount++;
							}
						}
						else if( bFPSMode )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] check pts for down sampling frame rate(%d) (%d) (%d)", __LINE__, uiVideoDecOutCTS, uiTotalFrameCount, uiLastPTS);
							if( uiTotalFrameCount != 0 && (uiVideoDecOutCTS - uiLastPTS) < iFPS_Transcoding_Gap )
							{
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Skip for down sampling frame rate(%d) (%d)", __LINE__, uiVideoDecOutCTS, uiLastPTS);
							}
							else
							{
								m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
								NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif
								
								if( writeVideoFrame(pY, uiVideoDecOutCTS) == FALSE )
								{
									pTranscodingInfo->m_nResult = getWriterError();
									bEncodeFailed = TRUE;
									continue;
								}
								uiLastPTS = (uiTotalFrameCount*iFPS_Transcoding_Gap_Interval)/100;
								uiTotalFrameCount++;
							}
						}
						else
						{
							m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
								NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif
							if( writeVideoFrame(pY, uiVideoDecOutCTS) == FALSE )
							{
								pTranscodingInfo->m_nResult = getWriterError();
								bEncodeFailed = TRUE;
								continue;
							}
							uiLastPTS = uiVideoDecOutCTS;
						}
					}
					else if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getOutFrame Thumbnail Frame NV12", __LINE__);
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &m_pSoftwareTempBuffer,NULL, NULL, &uiVideoDecOutCTS) != 0 )
						{
							if( ( uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM ) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
								bFinishedVideo = TRUE;
							}
							
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getOutFrame failed", __LINE__);
							continue;
						}

						if( iTimeStampControlForQCDevice )
							uiVideoDecOutCTS = uiVideoDecOutCTS / 2;

						if( 	(int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2  ||
							( isEOSFlag(uiEnhancement)  == FALSE && (m_bSupportFrameTimeChecker && m_FrameTimeChecker.isValidFrameCTS(uiVideoDecOutCTS, TRUE) == FALSE )) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
							continue;
						}

						if( bFPSMode )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] check pts for down sampling frame rate(%d) (%d) (%d)", __LINE__, uiVideoDecOutCTS, uiTotalFrameCount, uiLastPTS);
							if( uiTotalFrameCount != 0 && (uiVideoDecOutCTS - uiLastPTS) < iFPS_Transcoding_Gap )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Skip for down sampling frame rate(%d) (%d)", __LINE__, uiVideoDecOutCTS, uiLastPTS);
							}
							else
							{
								if( writeVideoFrame(uiVideoDecOutCTS, m_pSoftwareTempBuffer) == FALSE )
								{
									pTranscodingInfo->m_nResult = getWriterError();
									bEncodeFailed = TRUE;
									continue;
								}
								uiLastPTS = (uiTotalFrameCount*iFPS_Transcoding_Gap_Interval)/100;
								uiTotalFrameCount++;
							}
						}
						else
						{
							if( writeVideoFrame(uiVideoDecOutCTS, m_pSoftwareTempBuffer) == FALSE )
							{
								pTranscodingInfo->m_nResult = getWriterError();
								bEncodeFailed = TRUE;
								continue;
							}
							uiLastPTS = uiVideoDecOutCTS;
						}						

					}

					if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
						bFinishedVideo = TRUE;
					}

					if( (uiVideoDecOutCTS & 0x80000000 ) == 0x80000000 )
					{
						if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
						{
							m_isNextVideoFrameRead = TRUE;
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Skip current frame because invalid frame CTS(%d)", __LINE__, uiVideoDecOutCTS);
						continue;
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput did not exist", __LINE__);
					nexSAL_TaskSleep(1);
				}
			}
			else
			{
				if( m_pCodecWrap->isHardwareCodec() == FALSE )
				{
					isFindIDRFrame = TRUE;
					m_isNextVideoFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] SW Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);
					continue;
				}
				
				NXBOOL bNeedMoreDecode = FALSE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;
					bNeedMoreDecode = TRUE;
				}

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					bFinishedVideo = TRUE;
				}				

				if(NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[TranscodingTask.cpp %d] Video Output timeout cnt=%d ", __LINE__, ++iRetryCountForDecTimeOut);
					bNeedMoreDecode = TRUE;
					if (iRetryCountForDecTimeOut > 3)
					{
						if (isEOSFlag(uiEnhancement))
							bFinishedVideo = TRUE;
						else
							bDecodeFailed = TRUE;
					}
					else
						continue;
				}

				if( bNeedMoreDecode == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Video Task Decoding Error", __LINE__);
					bDecodeFailed = TRUE;
				}

				nexSAL_TaskSleep(1);
			}

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Check VideoEnd(%d %d %d)",__LINE__, uiCTS, uiDecOutCTS, (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM);
			// if( bFinishedVideo || (uiVideoDTS <= uiVideoDecOutCTS || uiVideoDecOutCTS == 0) && isEOSFlag(uiEnhancement))
			if( bFinishedVideo || ((uiVideoDTS <= uiVideoDecOutCTS ) && isEOSFlag(uiEnhancement)))
			{
				bFinishedVideo = TRUE;
				//flush video frames
				writeVideoFrame(NULL, 0, TRUE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getOutFrame End(%d %d)",__LINE__, uiVideoDTS, uiVideoDecOutCTS);
			}
		}
		// Audio
		// without decoding. write audio frame directly to writer.
		if( bFinishedVideo == TRUE && bFinishedAudio == TRUE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video and Audio is finished!!!",__LINE__);
			break;
		}

		if(bFinishedAudio == FALSE)
		{
			if(bWriteAudio)
			{
				if( m_isNextAudioFrameRead )
				{
					int iRet = 0;
					iRet = m_pFileReader->getAudioFrame();
					
					if( iRet  == _SRC_GETFRAME_END || iRet  == _SRC_GETFRAME_ERROR )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getAudioFrame End", __LINE__);
						bAudioFrameEnd = TRUE;
					}
					m_isNextAudioFrameRead = FALSE;
				}

				if( bAudioFrameEnd == FALSE)
				{
					m_pFileReader->getAudioFrameCTS( &uiAudioDTS );
					m_pFileReader->getAudioFrameData( &pAudioFrame, &uiAudioFrameSize );

					if (bFirstAudioFrame && (m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AAC || m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AACPLUS) && m_pDSI != NULL )
					{
						unsigned int    uSamplingFreqIndex = 0;
						const unsigned int puAACSampleRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};

						uSamplingFreqIndex = ((m_pDSI[0] & 0x07) << 1) | ((m_pDSI[1] & 0x80) >> 7);

						if ( uSamplingFreqIndex > 11 )//Nx_robin__090827 uSamplingFreqIndex�� üũ
							uSamplingFreqIndex = 0;
						m_uiSampleRate = puAACSampleRates[uSamplingFreqIndex];
						m_uiChannels = (m_pDSI[1] & 0x78) >> 3;
						if ( m_uiChannels == 0 )//Nx_robin__090827 DSI���� uNumOfChannels�� 0���� ������ ����?File MetaData���� �����;� �Ѵ�.
							m_pFileReader->getNumberOfChannel(&m_uiChannels);
						m_uiBitsPerSample = 16;

						m_pFileReader->m_uiAudioObjectType = NexCodecUtil_GetAACType((NEX_CODEC_TYPE)m_pFileReader->m_uiAudioObjectType, m_uiSampleRate, m_uiChannels, pAudioFrame, uiAudioFrameSize);

						if(m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AACPLUSV2)
							m_pFileReader->m_uiAudioObjectType = eNEX_CODEC_A_AACPLUS;

						if ( m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AACPLUS )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d] This contents is AAC+\n", __LINE__);
							m_uiSamplePerChannel = 2048;
						}
						else
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[TranscodingTask.cpp %d] This contents is AAC\n", __LINE__);
							m_uiSamplePerChannel = 1024;
						}

						m_pFileReader->m_uiAudioFrameInterval		= _GetFrameInterval(m_uiSamplePerChannel, m_uiSampleRate);
						m_pFileReader->m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiSamplePerChannel, m_uiSampleRate);
						m_pFileReader->m_uiMaxAudioFrameInterval	= (m_pFileReader->m_uiAudioFrameInterval*3) >> 1;

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio Sample(%d) Channels(%d), BitsPerSample(%d) SamplePerCh(%d)",
								__LINE__, m_uiSampleRate, m_uiChannels, m_uiBitsPerSample, m_uiSamplePerChannel);

						bFirstAudioFrame = FALSE;
					}

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getAudioFrame (%lld, %p, %d)", __LINE__, uiAudioDTS, pAudioFrame, uiAudioFrameSize);
					if((int)uiAudioDTS < 0)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] drop AudioFrame before edit box time(%lld, %p, %d)", __LINE__, uiAudioDTS, pAudioFrame, uiAudioFrameSize);					
						m_isNextAudioFrameRead = TRUE;
						continue;
					}
					
					if(IS_AUDIO_DECODING(m_pFileReader->m_uiAudioObjectType))
					{
						int uRet;
						if( (uRet = decodingPCM(bAudioFrameEnd, pAudioFrame, uiAudioFrameSize, (unsigned int)uiAudioDTS)) != 0)
						{
							if(uRet == 1)
							{
								bDecodeFailed = TRUE;
							}
							else if (uRet == 2)
							{
								pTranscodingInfo->m_nResult = getWriterError();
								bEncodeFailed = TRUE;
							}
							continue;
						}
					}
					else
					{
						writeAudioFrame((unsigned int)uiAudioDTS, pAudioFrame, uiAudioFrameSize);	
						m_isNextAudioFrameRead = TRUE;
					}
				}
				if( bAudioFrameEnd == TRUE)
					bFinishedAudio = TRUE;
			}
		}
	}

	
	if( m_bUserCancel )
	{
		writeVideoFrame(NULL, 0, TRUE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect user cancel while transcoding(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		pTranscodingInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_USER_CANCEL;
	}
	else if( bEncodeFailed )
	{
		m_bUserCancel = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect encode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}
	else if(bDecodeFailed)
	{
		m_bUserCancel = TRUE;
		pTranscodingInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_CODEC_FAILED;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect decode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}
	else
	{
		sendNotifyEvent(MESSAGE_TRANSCODING_PROGRESS, 100, uiVideoDTS, m_uiTotalPlayTime);
		// sendNotifyEvent(MESSAGE_TRANSCODING_PROGRESS, 100, m_uiTotalPlayTime, m_uiTotalPlayTime);
	}
	// deinit File Reader
	deinitFileReader();
	// deinit Video Decoder
	deinitVideoDecoder();
	deinitAudioDecoder();	
	// deinit file writer.
	deinitWriter();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processTransCoding Ended!!!", __LINE__);
	return;	
}

void CNEXThread_TranscodingTask::processHighLightIndex(CNxMsgHighLightIndex* pHighLightIndex)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "TranscodingTask.cpp[%d] processHighLightIndex Start!!!", __LINE__);

	CFrameInfo FrameInfo;
	CFrameInfo*	pFrameInfo		= &FrameInfo;
	NXBOOL	bWriteAudio = FALSE;
	NXBOOL	bWriteVideo = FALSE;
	NXBOOL	bAudioFrameEnd = FALSE;
	NXBOOL	bFinishedAudio = FALSE;
	NXBOOL	bFinishedVideo = FALSE;
	NXBOOL	bVideoEndOfContent = FALSE;
	NXBOOL	bAudioEndOfContent = FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	NXBOOL	bFirstAudioFrame = TRUE;
	unsigned int uiVideoDecoderErrRet = 0;
	unsigned int uiEnhancement = 0;
	unsigned int iVideoFrameNALHeaderSize = 0;
	unsigned int uiAudioDTS = 0;
	unsigned int uiAudioFrameSize = 0;
	unsigned char* pAudioFrame = NULL;

	unsigned int uiVideoPTS = 0;
	unsigned int uiVideoDTS = 0;
	unsigned int uiDecDTS = 0;
	unsigned int uiDecPTS = 0;
	unsigned int uiCheckGetOutputTime = 0;
	unsigned int uiVideoDecOutCTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	unsigned int uiCheckDecTime = 0;

	unsigned int  uiLastPTS = 0;
	unsigned int uiTotalFrameCount = 0;

	int iRetryCountForDecFail = 0;

	NXBOOL bDecodeFailed = FALSE;

	int iCurrProgress = 0;
	int iPrevProgress = -1;

	int iRet = 0;
	int iSeekTableCount = 0;
	int iSeekCurIndex = 0;
	unsigned int* pSeekTable = NULL;
	int iIndexMode = pHighLightIndex->m_iIndexMode;
	

	m_HighlightDiffChecker.clear();
	m_FrameTimeChecker.clear();

	// Init File Reader
	iRet = initFileReader();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_FILEREADER_INIT_ERROR;
		return;
	}

	iVideoFrameNALHeaderSize = m_pFileReader->getFrameNALHeaderLength();

	iRet = initRenderer();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_RENDER_INIT_ERROR;
		return;
	}

	// Init Video Decoder.
	iRet = initVideoDecoder();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_CODEC_INIT_ERROR;
		return;
	}

	if( m_pCodecWrap->isHardwareCodec() == FALSE )
	{
		pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Can't suppoted HW Codec",__LINE__);
		return;
	}

	if(iIndexMode == 1)
	{
		if( m_pFileReader->getSeekTable(0, &iSeekTableCount, &pSeekTable) != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] getSeekTable failed", __LINE__);
			pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_FILEREADER_INIT_ERROR;
			return;
		}
		m_isNextVideoFrameRead = TRUE;
	}
       isFindIDRFrame = TRUE;

	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfGLDraw.CheckModuleStart();
	m_perfGetPixels.CheckModuleStart();
	m_perfSwapbuffer.CheckModuleStart();
	m_perfDrawAll.CheckModuleStart();

	while ( m_bIsWorking )
	{
		if( m_bUserCancel || bDecodeFailed || bFinishedVideo )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] task stop because state was changed(%d, %d %d)", 
				__LINE__, m_bUserCancel ,bDecodeFailed, bFinishedVideo);
			break;
		}

		// CheckStatus
		{
			iCurrProgress = (uiVideoDTS*100 / m_uiTotalPlayTime);
			if( iCurrProgress > iPrevProgress)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] TRANSCODING PROGRESS(%d %d %d)", __LINE__, iCurrProgress, uiVideoDTS, m_uiTotalPlayTime);
				sendNotifyEvent(MESSAGE_MAKE_HIGHLIGHT_PROGRESS_INDEX, iCurrProgress, uiVideoDTS, m_uiTotalPlayTime);
				iPrevProgress = iCurrProgress;
			}
		}

		// Video
		if( m_isNextVideoFrameRead )
		{
			if( iIndexMode == 1 && !bVideoEndOfContent && iSeekCurIndex < iSeekTableCount )
			{
				unsigned int uiSeekResult = 0;
				if( m_pFileReader->seekTo(pSeekTable[iSeekCurIndex], &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
				{
					iSeekCurIndex++;
					continue;
				}
			}

			int iReaderRet = m_pFileReader->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				case _SRC_GETFRAME_END:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame End", __LINE__);
					bVideoEndOfContent = TRUE;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
					bVideoEndOfContent = TRUE;
					break;
			};

			processEnhancedFrame(&uiEnhancement);
			m_isNextVideoFrameRead = FALSE;

			if( bVideoEndOfContent )
			{
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}
			else
			{
				// for sync problem.
				m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
				m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );
				m_pFileReader->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );
			}
			
		}

		uiDecDTS = uiVideoDTS;
		uiDecPTS = uiVideoPTS;
		if( isFindIDRFrame && !bVideoEndOfContent)
		{
			bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), TRUE);
			if( bIDRFrame == FALSE )
			{
				m_isNextVideoFrameRead = TRUE;
				iSeekCurIndex++;
				continue;
			}

                	if(iIndexMode != 1)
			isFindIDRFrame = FALSE;
		}

		uiCheckDecTime = nexSAL_GetTickCount();
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
									pVideoFrame,
									uiVideoFrameSize,
									NULL,
									uiDecDTS,
									uiDecPTS,
									uiEnhancement,
									&uiVideoDecoderErrRet );

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] DecFrame End(%p, %d) DTS(%d) PTS(%d)uiRet(%d) time(%d)",
			__LINE__, pVideoFrame, uiVideoFrameSize, uiVideoDTS, uiVideoPTS, uiVideoDecoderErrRet, nexSAL_GetTickCount() - uiCheckDecTime);

		if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			uiVideoDecOutCTS = 0;
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame End and next frame", __LINE__);
				m_isNextVideoFrameRead = TRUE;
				if(iIndexMode == 1)
				{
					iSeekCurIndex++;
				}

				if(m_bSupportFrameTimeChecker)
					m_FrameTimeChecker.addFrameCTSVec(uiDecDTS, uiDecPTS);				
			}

			// for msm8x26
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
				bFinishedVideo = TRUE;
			}				

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char*	pY			= NULL;
				unsigned char*	pU			= NULL;
				unsigned char*	pV			= NULL;
				unsigned int 	uiCheckCTS 	= 0;

				//pFrameInfo					= NULL;

				if( m_uiCheckVideoDecInit != 0 )
				{
					m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] VideoDecoder_First output( Time : %d )", __LINE__, m_uiCheckVideoDecInit );
					m_uiCheckVideoDecInit = 0;
				}

				if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
				{
					uiCheckGetOutputTime = nexSAL_GetTickCount();
					if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
						/*
						//delete for mantis 6764
						if( (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] ID(%d) getOutFrame End(%d %d)",__LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
							break;
						}
						*/
						continue;
					}

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
						__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);
					
					if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
						m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
						continue;
					}

					m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
					NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif

					unsigned int uNewCTS = uiVideoDecOutCTS;
					if(m_bSupportFrameTimeChecker)
					{
						if (m_FrameTimeChecker.isValidFrameCTS(uiVideoDecOutCTS, 0))
						{
							m_FrameTimeChecker.getValidFrameCTSWithApplyMutipleCTS(uiVideoDecOutCTS, &uNewCTS, 1);
						}
					}

					if( processVideoFrameForHighLight(pY, uNewCTS) == FALSE)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] processVideoFrameForHighLight failed", __LINE__);
						continue;						
					}
					uiTotalFrameCount++;
					uiLastPTS = uiVideoDecOutCTS;

					if(iIndexMode == 1)
					{
						if(uiTotalFrameCount >= iSeekTableCount)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] processVideoFrameForHighLight decoding done", __LINE__);					
							break;
						}
					}
				}
				else
				{
					uiCheckGetOutputTime = nexSAL_GetTickCount();
					if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
						continue;
					}

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
						__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);
					
					if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
						continue;
					}

					unsigned int uNewCTS = uiVideoDecOutCTS;
					if(m_bSupportFrameTimeChecker)
					{
						if (m_FrameTimeChecker.isValidFrameCTS(uiVideoDecOutCTS, 0))
						{
							m_FrameTimeChecker.getValidFrameCTSWithApplyMutipleCTS(uiVideoDecOutCTS, &uNewCTS, 1);
						}
					}

					if( processVideoFrameForHighLight(pY, uNewCTS) == FALSE)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] processVideoFrameForHighLight failed", __LINE__);
						continue;						
					}
					uiTotalFrameCount++;
					uiLastPTS = uiVideoDecOutCTS;

					if(iIndexMode == 1)
					{
						if(uiTotalFrameCount >= iSeekTableCount)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] processVideoFrameForHighLight decoding done", __LINE__);					
							break;
						}
					}
				}
			}
		}
		else
		{
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame End and next frame", __LINE__);
				m_isNextVideoFrameRead = TRUE;
			}
			
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
				bFinishedVideo = TRUE;
				continue;
			}
			
			NXBOOL bNeedMoreDecode = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);

			if(NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed (Output timeout)", __LINE__);
				bNeedMoreDecode = TRUE;
			}

			if( bNeedMoreDecode == FALSE )
			{
				NXBOOL bIDR = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
				if( bIDR == FALSE )
				{
					m_isNextVideoFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task retry again for decoder reinit with next frame", __LINE__);
					continue;
				}

				if( iRetryCountForDecFail > 1 )
				{
					bDecodeFailed = TRUE;
					continue;
				}
				
				deinitVideoDecoder();
				// nexSAL_TaskSleep(500);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task reinit codec for codec repare", __LINE__);
				if( reinitVideoDecoder(pVideoFrame, uiVideoFrameSize) == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task Exit because reinit failed", __LINE__);
					bDecodeFailed = TRUE;
					continue;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task re start after reinit codec(%d)", __LINE__, iRetryCountForDecFail);
				iRetryCountForDecFail++;

				// CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, VIDEOEDITOR_EVENT_PLAY_ERROR, NEXVIDEOEDITOR_ERROR_CODEC_INIT);
				// break;
			}
			nexSAL_TaskSleep(1);
		}

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Check VideoEnd(%d %d %d)",__LINE__, uiCTS, uiDecOutCTS, (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM);
		// if( bFinishedVideo || (uiVideoDTS <= uiVideoDecOutCTS || uiVideoDecOutCTS == 0) && isEOSFlag(uiEnhancement))
		if( bFinishedVideo || ((uiVideoDTS <= uiVideoDecOutCTS ) && isEOSFlag(uiEnhancement)))
		{
			bFinishedVideo = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getOutFrame End(%d %d)",__LINE__, uiVideoDTS, uiVideoDecOutCTS);
		}
	}

	
	if( m_bUserCancel )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect user cancel while transcoding(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_USER_CANCEL;
	}
	else if(bDecodeFailed)
	{
		m_bUserCancel = TRUE;
		pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_CODEC_DECODE_ERROR;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect decode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}
	else
	{
		sendNotifyEvent(MESSAGE_MAKE_HIGHLIGHT_PROGRESS_INDEX, 100, uiVideoDTS, m_uiTotalPlayTime);
		// sendNotifyEvent(MESSAGE_MAKE_HIGHLIGHT_PROGRESS_INDEX, 100, m_uiTotalPlayTime, m_uiTotalPlayTime);

		pHighLightIndex->m_nResult = NEXVIDEOEDITOR_ERROR_NONE;
	}

	m_HighlightDiffChecker.sortWithDiff();
	m_HighlightDiffChecker.printDiff();

	m_HighlightDiffChecker.sortWithTime();
	m_HighlightDiffChecker.printDiff();

	unsigned int uInterval = m_pFileReader->getTotalPlayVideoTime()/(pHighLightIndex->m_iRequestCount*2);
	if(uInterval < pHighLightIndex->m_iRequestInterval)
	{
		uInterval = pHighLightIndex->m_iRequestInterval;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processHighLightIndex calcHighlight %d, %d", uInterval, iSeekTableCount);		
	m_HighlightDiffChecker.calcHighlight(uInterval, pHighLightIndex->m_iRequestCount, m_uiTotalPlayTime);		

	// deinit File Reader
	deinitFileReader();
	// deinit Video Decoder
	deinitVideoDecoder();
	deinitRenderer();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processHighLightIndex Ended!!!", __LINE__);
	return;	
}

void CNEXThread_TranscodingTask::processHighLight(CNxMsgHighLightIndex* pHighLight)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processHighLight Start!!!", __LINE__);

	CFrameInfo FrameInfo;
	CFrameInfo*	pFrameInfo		= &FrameInfo;
	NXBOOL	bWriteAudio = FALSE;
	NXBOOL	bWriteVideo = FALSE;
	NXBOOL	bAudioFrameEnd = FALSE;
	NXBOOL	bFinishedAudio = FALSE;
	NXBOOL	bFinishedVideo = FALSE;
	NXBOOL	bVideoEndOfContent = FALSE;
	NXBOOL	bAudioEndOfContent = FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	NXBOOL	bFirstAudioFrame = TRUE;
	unsigned int uiVideoDecoderErrRet = 0;
	unsigned int uiEnhancement = 0;
	unsigned int iVideoFrameNALHeaderSize = 0;
	NXINT64	uiAudioDTS = 0;
	unsigned int uiAudioFrameSize = 0;
	unsigned char* pAudioFrame = NULL;

	unsigned int uiVideoPTS = 0;
	unsigned int uiVideoDTS = 0;
	unsigned int uiDecDTS = 0;
	unsigned int uiDecPTS = 0;
	unsigned int uiCheckGetOutputTime = 0;
	unsigned int uiVideoDecOutCTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	unsigned int uiCheckDecTime = 0;

	int iRetryCountForDecFail = 0;

	int iTimeStampControlForQCDevice = 0;

	NXBOOL bEncodeFailed = FALSE;
	NXBOOL bDecodeFailed = FALSE;

	unsigned int  uiLastPTS = 0;

	int iCurrProgress = 0;
	int iPrevProgress = -1;

	int iRet = 0;

	// Init File Reader
	iRet = initFileReader();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_FILEREADER_INIT_ERROR;
		return;
	}
	// JIRA 2372
	/*
	// for portrait comcorded content.
	if( m_iSrcWidth < m_iSrcHeight )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Change dest width and height", __LINE__);

		int tmpWidth = m_iDstWidth;
		m_iDstWidth = m_iDstHeight;
		m_iDstHeight = tmpWidth;
	}
	*/

	if( m_pFileReader->isAudioExist() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] No Audio track", __LINE__);
		bFinishedAudio = TRUE;
	}

	iVideoFrameNALHeaderSize = m_pFileReader->getFrameNALHeaderLength();

	iRet = initRenderer();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_RENDER_INIT_ERROR;
		return;
	}

	iRet = initWriter(m_HighlightDiffChecker.getHighlightTimeCount()*pHighLight->m_iRequestInterval);
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_WRITER_INIT_ERROR;
		return;
	}
	
	if(IS_AUDIO_DECODING(m_pFileReader->m_uiAudioObjectType))
	{
		// Init Audio Decoder.
		iRet = initAudioDecoder(0);
		if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
		{
			pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_CODEC_INIT_ERROR;
			return;
		}
	}	
	else
	{
		m_isNextAudioFrameRead = TRUE;
	}
	// Init Video Decoder.
	iRet = initVideoDecoder();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_HIGHLIGHT_CODEC_INIT_ERROR;
		return;
	}

#ifdef FOR_TEST_QUALCOMM_CHIP
	if( m_pCodecWrap->isHardwareCodec() && isQualcommChip()  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Time Stamp control for QualComm devices", __LINE__);
		// do not use flow control KM-4162
		//iTimeStampControlForQCDevice = 1;
	}
#endif

	int iHighligntIndex = 0;
	int iHighlightCount = m_HighlightDiffChecker.getHighlightTimeCount();
	unsigned int uiHighlightStartTime = m_HighlightDiffChecker.getHighlightTime(iHighligntIndex++);
	unsigned int uiHighlightEndTime = uiHighlightStartTime + pHighLight->m_iRequestInterval;
	unsigned int uiHighlightTrimTime = uiHighlightStartTime;
	unsigned int uiHighlightBaseTime = 0;
	unsigned int uiCheckTime = 0;

	unsigned int uiSeekResult = 0;
	int iReaderRet = m_pFileReader->seekTo(uiHighlightStartTime, &uiSeekResult);
	m_isNextVideoFrameRead = TRUE;


	m_FrameTimeChecker.clear();

	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfGLDraw.CheckModuleStart();
	m_perfGetPixels.CheckModuleStart();
	m_perfSwapbuffer.CheckModuleStart();
	m_perfDrawAll.CheckModuleStart();

	while ( m_bIsWorking )
	{
		if( m_bUserCancel || bEncodeFailed || bDecodeFailed )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] procHighlight task stop because state was changed(%d, %d %d)", 
				__LINE__, m_bUserCancel, bEncodeFailed ,bDecodeFailed);
			break;
		}
		
		// CheckStatus
		{
			iCurrProgress = (uiVideoDTS*100 / m_uiTotalPlayTime);
			if( iCurrProgress > iPrevProgress)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] TRANSCODING PROGRESS(%d %d %d)", __LINE__, iCurrProgress, uiVideoDTS, m_uiTotalPlayTime);
				sendNotifyEvent(MESSAGE_MAKE_HIGHLIGHT_PROGRESS, iCurrProgress, uiVideoDTS, m_uiTotalPlayTime);
				iPrevProgress = iCurrProgress;
			}
		}

		if( uiVideoDecOutCTS > uiHighlightEndTime )
		{
			uiHighlightBaseTime = uiCheckTime;
			uiHighlightStartTime = m_HighlightDiffChecker.getHighlightTime(iHighligntIndex++);
			uiHighlightEndTime = uiHighlightStartTime + pHighLight->m_iRequestInterval;
			uiHighlightTrimTime = uiHighlightStartTime;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] procHighlight  Highlight get next node time(hst:%d hct:%d)", __LINE__, uiHighlightStartTime, uiCheckTime);

			if( uiHighlightStartTime == 0xFFFFFFFF )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] procHighlight  task stop because highlight end", 
					__LINE__, m_bUserCancel, bEncodeFailed ,bDecodeFailed);
				break;
			}

			unsigned int uiSeekResult = 0;
			int iReaderRet = m_pFileReader->seekTo(uiHighlightStartTime, &uiSeekResult);
			m_isNextVideoFrameRead = TRUE;
		}
		

		if( m_pFileWriter->isVideoFrameWriteStarted() && bFinishedAudio == FALSE  && uiVideoDecOutCTS > uiAudioDTS+100 )
		{
			bWriteVideo = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Wait AudioFrame(Audio End:%d Video Time:%d audio Time:%lld)", 
				__LINE__, bFinishedAudio, uiVideoDecOutCTS, uiAudioDTS);
		}
		else
		{
			bWriteVideo = TRUE;
		}

		if(uiAudioDTS >= uiVideoDecOutCTS && bFinishedVideo == FALSE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Wait Videoframe(A:%lld VD:%d)", __LINE__, uiAudioDTS, uiVideoDecOutCTS);
			bWriteAudio = FALSE;
		}
		else
		{
			if( m_pFileWriter->isVideoFrameWriteStarted() )
				bWriteAudio = TRUE;
			else
				bWriteAudio = FALSE;
		}

		if(bWriteVideo == TRUE && bFinishedVideo == FALSE)
		{
			// Video
			if( m_isNextVideoFrameRead )
			{
				int iReaderRet = m_pFileReader->getVideoFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame End", __LINE__);
						bVideoEndOfContent = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
						bVideoEndOfContent = TRUE;
						break;
				};

				processEnhancedFrame(&uiEnhancement);
				m_isNextVideoFrameRead = FALSE;
			}

			if( bVideoEndOfContent )
			{
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}
			else
			{
				// for sync problem.
				m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
				m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );
				m_pFileReader->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );
			}

			uiDecDTS = uiVideoDTS;
			uiDecPTS = uiVideoPTS;
			if( iTimeStampControlForQCDevice && bVideoEndOfContent == FALSE )
			{
				uiDecDTS = uiDecDTS * 2;
				uiDecPTS = uiDecPTS * 2;
			}

			if( isFindIDRFrame )
			{
				bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
				if( bIDRFrame == FALSE )
				{
					m_isNextVideoFrameRead = TRUE;
					continue;
				}

				isFindIDRFrame = FALSE;
			}

			uiCheckDecTime = nexSAL_GetTickCount();
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
			nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
										pVideoFrame,
										uiVideoFrameSize,
										NULL,
										uiDecDTS,
										uiDecPTS,
										uiEnhancement,
										&uiVideoDecoderErrRet );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] DecFrame End(%p, %d) DTS(%d) PTS(%d)uiRet(%d) time(%d)",
				__LINE__, pVideoFrame, uiVideoFrameSize, uiVideoDTS, uiVideoPTS, uiVideoDecoderErrRet, nexSAL_GetTickCount() - uiCheckDecTime);


			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
			{
				uiVideoDecOutCTS = 0;
				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame End and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;

					// whenever decoding frames, add PTS on the FrameCTSVector
					if(m_bSupportFrameTimeChecker)
					{
						if( iTimeStampControlForQCDevice && bVideoEndOfContent == FALSE)
						{
							uiDecDTS = uiDecDTS / 2;
							uiDecPTS = uiDecPTS / 2;
							
							m_FrameTimeChecker.addFrameCTSVec(uiDecDTS, uiDecPTS);
						}
						else
						{
							m_FrameTimeChecker.addFrameCTSVec(uiDecDTS, uiDecPTS);
						}
					}
				}

				// for msm8x26
				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					bFinishedVideo = TRUE;
				}				

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
				{
					unsigned char*	pY			= NULL;
					unsigned int 	uiCheckCTS 	= 0;

					if( m_uiCheckVideoDecInit != 0 )
					{
						m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] VideoDecoder_First output( Time : %d )", __LINE__, m_uiCheckVideoDecInit );
						m_uiCheckVideoDecInit = 0;
					}
					if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
					{
						uiCheckGetOutputTime = nexSAL_GetTickCount();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
							/*
							//delete for mantis 6764
							if( (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] ID(%d) getOutFrame End(%d %d)",__LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
								break;
							}
							*/
							continue;
						}

						if( iTimeStampControlForQCDevice )
							uiVideoDecOutCTS = uiVideoDecOutCTS / 2;

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
							__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);

						if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2  ||
							( isEOSFlag(uiEnhancement)  == FALSE && (m_bSupportFrameTimeChecker && m_FrameTimeChecker.isValidFrameCTS(uiVideoDecOutCTS, TRUE) == FALSE )) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}

						if( uiVideoDecOutCTS < uiHighlightTrimTime )
						{
							m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
							NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] procHighlight  skip video frame with trim gap(vot:%d htt:%d)", __LINE__, uiVideoDecOutCTS, uiHighlightTrimTime);
							continue;
						}

						uiCheckTime = uiVideoDecOutCTS - uiHighlightTrimTime + uiHighlightBaseTime;


						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] procHighlight write frame(hct:%d hst:%d hbt:%d htt:%d dot:%d )",
							__LINE__, uiCheckTime, uiHighlightStartTime, uiHighlightBaseTime, uiHighlightTrimTime, uiVideoDecOutCTS);
						m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
						NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif
						if( writeVideoFrame(pY, uiCheckTime) == FALSE )
						{
							pHighLight->m_nResult = getWriterError();
							bEncodeFailed = TRUE;
							continue;
						}
						uiLastPTS = uiVideoDecOutCTS;
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput did not exist", __LINE__);
					nexSAL_TaskSleep(1);
				}
			}
			else
			{
				if( m_pCodecWrap->isHardwareCodec() == FALSE )
				{
					isFindIDRFrame = TRUE;
					m_isNextVideoFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] SW Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);
					continue;
				}
				
				NXBOOL bNeedMoreDecode = FALSE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;
					bNeedMoreDecode = TRUE;
				}

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					bFinishedVideo = TRUE;
				}				

				if(NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed (Output timeout)", __LINE__);
					bNeedMoreDecode = TRUE;
				}

				if( bNeedMoreDecode == FALSE )
				{
					NXBOOL bIDR = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
					if( bIDR == FALSE )
					{
						m_isNextVideoFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task retry again for decoder reinit with next frame", __LINE__);
						continue;
					}

					if( iRetryCountForDecFail > 1 )
					{
						bDecodeFailed = TRUE;
						continue;
					}
					
					deinitVideoDecoder();
					// nexSAL_TaskSleep(500);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task reinit codec for codec repare", __LINE__);
					if( reinitVideoDecoder(pVideoFrame, uiVideoFrameSize) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task Exit because reinit failed", __LINE__);
						bDecodeFailed = TRUE;
						continue;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task re start after reinit codec(%d)", __LINE__, iRetryCountForDecFail);
					iRetryCountForDecFail++;

					// CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, VIDEOEDITOR_EVENT_PLAY_ERROR, NEXVIDEOEDITOR_ERROR_CODEC_INIT);
					// break;
				}
				nexSAL_TaskSleep(1);
			}

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Check VideoEnd(%d %d %d)",__LINE__, uiCTS, uiDecOutCTS, (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM);
			// if( bFinishedVideo || (uiVideoDTS <= uiVideoDecOutCTS || uiVideoDecOutCTS == 0) && isEOSFlag(uiEnhancement))
			if( bFinishedVideo || ((uiVideoDTS <= uiVideoDecOutCTS ) && isEOSFlag(uiEnhancement)))
			{
				bFinishedVideo = TRUE;
				//flush video frames
				writeVideoFrame(NULL, 0, TRUE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getOutFrame End(%d %d)",__LINE__, uiVideoDTS, uiVideoDecOutCTS);
			}
		}
		// Audio
		// without decoding. write audio frame directly to writer.
		if( bFinishedVideo == TRUE && bFinishedAudio == TRUE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video and Audio is finished!!!",__LINE__);
			break;
		}

		if(bFinishedAudio == FALSE)
		{
			if(bWriteAudio)
			{
				if( m_isNextAudioFrameRead )
				{
					int iRet = 0;
					iRet = m_pFileReader->getAudioFrame();
					
					if( iRet  == _SRC_GETFRAME_END || iRet  == _SRC_GETFRAME_ERROR )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getAudioFrame End", __LINE__);
						bAudioFrameEnd = TRUE;
					}
					m_isNextAudioFrameRead = FALSE;
				}

				if( bAudioFrameEnd == FALSE)
				{
					m_pFileReader->getAudioFrameCTS( &uiAudioDTS );
					m_pFileReader->getAudioFrameData( &pAudioFrame, &uiAudioFrameSize );

					if(uiAudioDTS < 0)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Highlight drop before editbox time %lld", __LINE__, uiAudioDTS);						
					}

					if( (unsigned int)uiAudioDTS < uiHighlightTrimTime )
					{
						m_isNextAudioFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Highlight skip audio frame before start node time(s:%d) (c:%d)", __LINE__, uiHighlightStartTime, uiAudioDTS);
						continue;
					}

					if( (unsigned int)uiAudioDTS > uiHighlightEndTime )
					{
						m_isNextAudioFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Highlight skip audio frame after start node time(s:%d) (c:%d)", __LINE__, uiHighlightEndTime, uiAudioDTS);
						continue;
					}
					
					uiCheckTime = (unsigned int)uiAudioDTS - uiHighlightTrimTime + uiHighlightBaseTime;
					uiAudioDTS = uiCheckTime;
					
					if (bFirstAudioFrame && (m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AAC || m_pFileReader->m_uiAudioObjectType == eNEX_CODEC_A_AACPLUS) && m_pDSI != NULL )
					{
						unsigned int    uSamplingFreqIndex = 0;
						const unsigned int puAACSampleRates[] = { 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000};

						uSamplingFreqIndex = ((m_pDSI[0] & 0x07) << 1) | ((m_pDSI[1] & 0x80) >> 7);

						if ( uSamplingFreqIndex > 11 )//Nx_robin__090827 uSamplingFreqIndex�� üũ
							uSamplingFreqIndex = 0;
						m_uiSampleRate = puAACSampleRates[uSamplingFreqIndex];
						m_uiChannels = (m_pDSI[1] & 0x78) >> 3;
						if ( m_uiChannels == 0 )//Nx_robin__090827 DSI���� uNumOfChannels�� 0���� ������ ����?File MetaData���� �����;� �Ѵ�.
							m_pFileReader->getNumberOfChannel(&m_uiChannels);
						m_uiBitsPerSample = 16;

						m_pFileReader->m_uiAudioObjectType = NexCodecUtil_GetAACType((NEX_CODEC_TYPE)m_pFileReader->m_uiAudioObjectType, m_uiSampleRate, m_uiChannels, pAudioFrame, uiAudioFrameSize);

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

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio Sample(%d) Channels(%d), BitsPerSample(%d) SamplePerCh(%d)",
								__LINE__, m_uiSampleRate, m_uiChannels, m_uiBitsPerSample, m_uiSamplePerChannel);

						bFirstAudioFrame = FALSE;
					}

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getAudioFrame (%lld, %p, %d)", __LINE__, uiAudioDTS, pAudioFrame, uiAudioFrameSize);
					if(IS_AUDIO_DECODING(m_pFileReader->m_uiAudioObjectType))
					{
						int uRet;
						if((uRet = decodingPCM(bAudioFrameEnd, pAudioFrame, uiAudioFrameSize, (unsigned int)uiAudioDTS)) != 0)
						{
							if(uRet == 1)
							{
								bDecodeFailed = TRUE;
							}
							else if (uRet == 2)
							{
								pHighLight->m_nResult = getWriterError();
								bEncodeFailed = TRUE;
							}
							continue;
						}
					}
					else
					{
						writeAudioFrame((unsigned int)uiAudioDTS, pAudioFrame, uiAudioFrameSize);	
						m_isNextAudioFrameRead = TRUE;
					}
				}
				if( bAudioFrameEnd == TRUE)
					bFinishedAudio = TRUE;
			}
		}
	}

	
	if( m_bUserCancel )
	{
		writeVideoFrame(NULL, 0, TRUE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect user cancel while transcoding(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_USER_CANCEL;
	}
	else if( bEncodeFailed )
	{
		m_bUserCancel = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect encode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}
	else if(bDecodeFailed)
	{
		m_bUserCancel = TRUE;
		pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_CODEC_FAILED;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect decode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}
	else
	{
		sendNotifyEvent(MESSAGE_MAKE_HIGHLIGHT_PROGRESS, 100, uiVideoDTS, m_uiTotalPlayTime);
		// sendNotifyEvent(MESSAGE_MAKE_HIGHLIGHT_PROGRESS, 100, m_uiTotalPlayTime, m_uiTotalPlayTime);
	}
	// deinit File Reader
	deinitFileReader();
	// deinit Video Decoder
	deinitVideoDecoder();
	deinitAudioDecoder();
	// deinit file writer.
	deinitWriter();
	deinitRenderer();

	pHighLight->m_nResult = NEXVIDEOEDITOR_ERROR_NONE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "highlight Ended!!!", __LINE__);
	return;	
}

void CNEXThread_TranscodingTask::processReverse(CNxMsgReverseInfo* pReverseInfo)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processReverse Start!!!", __LINE__);

	CFrameInfo FrameInfo;
	CFrameInfo*	pFrameInfo		= &FrameInfo;
	NXBOOL	bWriteVideo = FALSE;
	NXBOOL	bFinishedVideo = FALSE;
	NXBOOL	bVideoEndOfContent = FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	unsigned int uiVideoDecoderErrRet = 0;
	unsigned int uiEnhancement = 0;
	unsigned int iVideoFrameNALHeaderSize = 0;

	unsigned int uiVideoPTS = 0;
	unsigned int uiVideoDTS = 0;
	unsigned int uiDecDTS = 0;
	unsigned int uiDecPTS = 0;
	unsigned int uiCheckGetOutputTime = 0;
	unsigned int uiVideoDecOutCTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	unsigned int uiCheckDecTime = 0;

	int iRetryCountForDecFail = 0;

	NXBOOL bEncodeFailed = FALSE;
	NXBOOL bDecodeFailed = FALSE;

	int iCurrProgress = 0;
	int iPrevProgress = -1;

	int iRet = 0;

	NXBOOL bTempFileReady = FALSE;
	unsigned int uTempStart = 0;
	unsigned int uTempEnd = 0xffffffff;
	unsigned int uFirstVideoFrame = 0;
	NXBOOL bLastDecFlag = FALSE;
	int iMultipleFactor = 1;
	unsigned int uFPS = 0;

	// Init File Reader
	iRet = initFileReader();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pReverseInfo->m_nResult = iRet;
		return;
	}

	m_pFileReader->seekTo(0, &uFirstVideoFrame);

	m_uReverseMode = 1;
	iVideoFrameNALHeaderSize = m_pFileReader->getFrameNALHeaderLength();

	if(m_pFileReader->isVideoExist())
	{
		iRet = initRenderer();
		if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
		{
			pReverseInfo->m_nResult = iRet;
			return;
		}
	}

	#if 0
	if(!m_pFileReader->isAudioExist())
	{
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_NOT_SUPPORTED_FORMAT;
		return;
	}
	#endif

	iRet = initWriter(m_uEndTime - m_uStartTime);
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pReverseInfo->m_nResult = iRet;
		return;
	}
	
	// Init Video Decoder.
	if(m_pFileReader->isVideoExist())
	{
		iRet = initVideoDecoder();
		if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
		{
			pReverseInfo->m_nResult = iRet;
			return;
		}
		m_pFileReader->getVideoFrameDTS( &uFirstVideoFrame, TRUE );

		m_pYuvFile = nexSAL_FileOpen(m_strTempFilePath, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_READWRITE));
		if(m_pYuvFile == NULL)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] processReverse m_pYuvFile is NULL (%s)", __LINE__, m_strTempFilePath);
			pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
			return;
		}
		nexSAL_FileSeek(m_pYuvFile, 0, NEXSAL_SEEK_BEGIN);

		uFPS = m_pFileReader->getVideoFrameRate();

		uTempEnd = m_uEndTime;
		m_pFileReader->seekToVideo(m_uEndTime - 100, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
		m_isNextVideoFrameRead = TRUE;

		if(m_uStartTime >= uTempStart)
		{
			bLastDecFlag = TRUE;
		}
	}

	m_FrameTimeChecker.clear();

	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfGLDraw.CheckModuleStart();
	m_perfGetPixels.CheckModuleStart();
	m_perfSwapbuffer.CheckModuleStart();
	m_perfDrawAll.CheckModuleStart();

	if(m_pFileReader->isAudioExist())
	{
		m_pReverseAudioTask = new CNEXThread_ReverseTask(this);
		if( m_pReverseAudioTask == NULL )
		{
			pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] m_pReverseAudioTask is NULL", __LINE__);
			return;
		}

		m_pReverseAudioTask->setReverseInfo(m_strTempFilePath, m_uStartTime, m_uEndTime);
		m_pReverseAudioTask->setFileReader(m_pFileReader);
		m_pReverseAudioTask->setFileWriter(m_pFileWriter);
			
		m_pReverseAudioTask->Begin();
		m_pReverseAudioTask->SendCommand(pReverseInfo);
	}

	while (m_bIsWorking && m_pFileReader->isVideoExist())
	{
		int err = 0;

		if(m_pReverseAudioTask)
		{
			err = m_pReverseAudioTask->getErrorCheck();
		}

		if( m_bUserCancel || bEncodeFailed || bDecodeFailed)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] task stop because state was changed(%d, %d, %d, %d)", 
				__LINE__, m_bUserCancel, bEncodeFailed ,bDecodeFailed, err);
			break;
		}

		if(err)
		{
			break;
		}

		if(bTempFileReady == FALSE)
		{
			// Video
			if( m_isNextVideoFrameRead )
			{
				int iReaderRet = m_pFileReader->getVideoFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame End", __LINE__);
						bVideoEndOfContent = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
						bVideoEndOfContent = TRUE;
						break;
				};

				processEnhancedFrame(&uiEnhancement);
				m_isNextVideoFrameRead = FALSE;
			}

			if( bVideoEndOfContent )
			{
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}
			else
			{
				// for sync problem.
				m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
				m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );
				m_pFileReader->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );
			}

			if(uiVideoDTS >= uTempEnd && uiVideoPTS >= uTempEnd) {
				bVideoEndOfContent = TRUE;
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}

			uiDecDTS = uiVideoDTS;
			uiDecPTS = uiVideoPTS;
			
			if( CNexVideoEditor::m_bNexEditorSDK ) 
			{
				if(m_bSupportFrameTimeChecker && CNexCodecManager::getHardwareDecoderUseCount() == 1)				
				{
					if(uFPS < 45)
					{
						iMultipleFactor = 200;
						uiDecDTS = uiDecDTS/2;
						uiDecPTS = uiDecPTS/2;
					}
				}
			}

			if( isFindIDRFrame )
			{
				bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
				if( bIDRFrame == FALSE )
				{
					m_isNextVideoFrameRead = TRUE;
					continue;
				}

				isFindIDRFrame = FALSE;
			}

			uiCheckDecTime = nexSAL_GetTickCount();
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
			nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
										pVideoFrame,
										uiVideoFrameSize,
										NULL,
										uiDecDTS,
										uiDecPTS,
										uiEnhancement,
										&uiVideoDecoderErrRet );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] DecFrame End(%p, %d) DTS(%d) PTS(%d)uiRet(%d) time(%d)",
				__LINE__, pVideoFrame, uiVideoFrameSize, uiVideoDTS, uiVideoPTS, uiVideoDecoderErrRet, nexSAL_GetTickCount() - uiCheckDecTime);

			if (NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_SETTING_CHANGE))
			{
				CheckVideoSettingChange();
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
			{
				uiVideoDecOutCTS = 0;
				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame End and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;
					if(m_bSupportFrameTimeChecker)
					m_FrameTimeChecker.addFrameCTSVec(uiDecDTS, uiDecPTS, iMultipleFactor);
				}

				// for msm8x26
				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					//bFinishedVideo = TRUE;
					bTempFileReady = TRUE;
				}				

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
				{
					unsigned char*	pY			= NULL;
					unsigned char*	pU			= NULL;
					unsigned char*	pV			= NULL;
					unsigned int 	uiCheckCTS 	= 0;

					if( m_uiCheckVideoDecInit != 0 )
					{
						m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] VideoDecoder_First output( Time : %d )", __LINE__, m_uiCheckVideoDecInit );
						m_uiCheckVideoDecInit = 0;
					}
					if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
					{
						uiCheckGetOutputTime = nexSAL_GetTickCount();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
							continue;
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
							__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);

						if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2  ||
							( m_bSupportFrameTimeChecker && m_FrameTimeChecker.getValidFrameCTSWithApplyMutiplePTS(uiVideoDecOutCTS, &uiVideoDecOutCTS, TRUE) == FALSE ) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}

						if(uiVideoDecOutCTS >= uTempEnd) {
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}

						if(uiVideoDecOutCTS < m_uStartTime)
						{
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}
						m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
							NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif
						if( saveVideoYUV(pY, uiVideoDecOutCTS) == FALSE )
						{
							pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED;
							bEncodeFailed = TRUE;
							continue;
						}

						if(bVideoEndOfContent == TRUE)
						{
							if(m_FrameTimeChecker.size() == 0)
							{
								bTempFileReady = TRUE;
							}
						}
					}
					else
					{
						uiCheckGetOutputTime = nexSAL_GetTickCount();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
							continue;
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
							__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);
						
						if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2  ||
							( m_bSupportFrameTimeChecker && m_FrameTimeChecker.getValidFrameCTSWithApplyMutiplePTS(uiVideoDecOutCTS, &uiVideoDecOutCTS, TRUE) == FALSE ) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
							continue;
						}

						if(uiVideoDecOutCTS >= uTempEnd) {
							continue;
						}

						if(uiVideoDecOutCTS < m_uStartTime)
						{
							continue;
						}

						if( saveVideoYUV(pY, m_iSettingWidth*m_iSettingHeight*3/2, uiVideoDecOutCTS) == FALSE )
						{
							pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED;
							bEncodeFailed = TRUE;
							continue;
						}

						if(bVideoEndOfContent == TRUE)
						{
							if(m_FrameTimeChecker.size() == 0)
								bTempFileReady = TRUE;
						}
					}

					if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
						//bFinishedVideo = TRUE;
						bTempFileReady = TRUE;					
					}

					if( (uiVideoDecOutCTS & 0x80000000 ) == 0x80000000 )
					{
						if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
						{
							m_isNextVideoFrameRead = TRUE;
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Skip current frame because invalid frame CTS(%d)", __LINE__, uiVideoDecOutCTS);
						continue;
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput did not exist", __LINE__);
					nexSAL_TaskSleep(1);
				}
			}
			else
			{
				if( m_pCodecWrap->isHardwareCodec() == FALSE )
				{
					isFindIDRFrame = TRUE;
					m_isNextVideoFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] SW Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);
					continue;
				}
				
				NXBOOL bNeedMoreDecode = FALSE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;
					bNeedMoreDecode = TRUE;
				}

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					bFinishedVideo = TRUE;
				}				

				if(NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed (Output timeout)", __LINE__);
					bNeedMoreDecode = TRUE;
				}

				if( bNeedMoreDecode == FALSE )
				{
					NXBOOL bIDR = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
					if( bIDR == FALSE )
					{
						m_isNextVideoFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task retry again for decoder reinit with next frame", __LINE__);
						continue;
					}

					if( iRetryCountForDecFail > 1 )
					{
						bDecodeFailed = TRUE;
						continue;
					}
					
					deinitVideoDecoder();
					// nexSAL_TaskSleep(500);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task reinit codec for codec repare", __LINE__);
					if( reinitVideoDecoder(pVideoFrame, uiVideoFrameSize) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task Exit because reinit failed", __LINE__);
						bDecodeFailed = TRUE;
						continue;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task re start after reinit codec(%d)", __LINE__, iRetryCountForDecFail);
					iRetryCountForDecFail++;

					// CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, VIDEOEDITOR_EVENT_PLAY_ERROR, NEXVIDEOEDITOR_ERROR_CODEC_INIT);
					// break;
				}
				nexSAL_TaskSleep(1);
			}
			continue;
		}

		#if 0
		if( m_pFileWriter->isVideoFrameWriteStarted() && m_pReverseAudioTask->getAudioFinished() == FALSE  && uiVideoDecOutCTS > m_pReverseAudioTask->getAudioEncodingTime()+100 )
		{
			bWriteVideo = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Wait AudioFrame(Audio End:%d Video Time:%d audio Time:%d)", 
				__LINE__, m_pReverseAudioTask->getAudioFinished(), uiVideoDecOutCTS, m_pReverseAudioTask->getAudioEncodingTime());
		}
		else
		#endif
		{
			bWriteVideo = TRUE;
		}
		
		if(bWriteVideo == TRUE && bFinishedVideo == FALSE)
		{
			if(m_vecNexReverseYUVInfo.size())
			{
				unsigned int uCTS = 0;
				unsigned int uSize = 0;
				unsigned int uPos = 0;
				unsigned int uRetryCount = 0;
				vecNexReverseYUVInfoItr j = m_vecNexReverseYUVInfo.end();
				j--;

				while(uRetryCount < 50)
				{
					unsigned int uSize_a, uSize_b, uSize_c, uSize_d;
					uSize_a = nexSAL_FileSeek(m_pYuvFile, *j, NEXSAL_SEEK_BEGIN);
					uSize_b = nexSAL_FileRead(m_pYuvFile, &uCTS, sizeof(uCTS));
					uSize_c = nexSAL_FileRead(m_pYuvFile, &uSize, sizeof(uSize));
					uSize_d = nexSAL_FileRead(m_pYuvFile, m_pTempBuffer, uSize);

					if(uSize_a == *j && uSize_b == sizeof(uCTS) && uSize_c == sizeof(uSize) && uSize_d == uSize)
						break;
					else
						nexSAL_TaskSleep(5);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] read file fail and try again(%zu, %d, %d, %d), (%d, %d, %d, %d)", __LINE__, m_vecNexReverseYUVInfo.size(), (unsigned int&)*j, uCTS, uSize, uSize_a, uSize_b, uSize_c, uSize_d);
					uRetryCount++;
				}

				if(uRetryCount == 50)
				{
					pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED;
					bEncodeFailed = TRUE;
					continue;
				}

				m_uEncodingTime = m_pFileWriter->getVideoTime();
				unsigned int uAudioTime = 0;
				if(m_pReverseAudioTask)
				{
					uAudioTime = m_pReverseAudioTask->getAudioEncodingTime();
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] m_vecNexReverseYUVInfo(%zu, %d, %d, %d), %d, %d", __LINE__, m_vecNexReverseYUVInfo.size(), (unsigned int&)*j, uCTS, uSize, m_uEncodingTime, uAudioTime);
	
				if(m_uBaseReverseTime == 0)
					m_uBaseReverseTime = uCTS;

				m_vecNexReverseYUVInfo.erase(j);		
				if(writeVideoYUV(m_uBaseReverseTime - uCTS, m_pTempBuffer, uSize) == FALSE)
				{
					pReverseInfo->m_nResult = getWriterError();
					bEncodeFailed = TRUE;
					continue;
				}
				
	 			iCurrProgress = ((m_uBaseReverseTime - uCTS)*100 / (m_uEndTime - m_uStartTime));
				if( iCurrProgress > iPrevProgress)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] REVERSE PROGRESS(%d %d %d, %d, %d)", __LINE__, iCurrProgress, m_uBaseReverseTime, uCTS, m_uStartTime, m_uEndTime);
					sendNotifyEvent(MESSAGE_MAKE_REVERSE_PROGRESS, iCurrProgress, m_uEncodingTime, m_uEndTime - m_uStartTime);
					iPrevProgress = iCurrProgress;
				} 				
				continue;
			}
			else
			{
				if(uFirstVideoFrame == uTempStart || bLastDecFlag)
				{
					writeVideoYUV(m_uBaseReverseTime, NULL, 0, TRUE);					
					bFinishedVideo = TRUE;
				}
				else
				{
					unsigned int uSeekTime = 0;
					m_vecNexReverseYUVInfo.clear();
					m_uYuvTotalSize = 0;					
					nexSAL_FileSeek(m_pYuvFile, 0, NEXSAL_SEEK_BEGIN);

					uTempEnd = uTempStart;
					uSeekTime = uTempStart > 1000?uTempStart-1000:0;
					if(m_uStartTime > uSeekTime)
					{
						uSeekTime = m_uStartTime;
					}
					m_pFileReader->seekToVideo(uSeekTime, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
					if(m_uStartTime > uTempStart)
					{
						bLastDecFlag = TRUE;
					}
					m_isNextVideoFrameRead = TRUE;
					nexCAL_VideoDecoderReset(m_pCodecWrap->getCodecHandle());
					bTempFileReady = FALSE;
					uiVideoDecoderErrRet = 0;
					bVideoEndOfContent = 0;
					uiEnhancement = 0;
				}
			}
		}

		if( bFinishedVideo == TRUE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video is finished!!!",__LINE__);
			break;
		}
 	}

	m_bVideoFinished = TRUE;
	
	if( m_bUserCancel )
	{
		writeVideoFrame(NULL, 0, TRUE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect user cancel while transcoding(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_USER_CANCEL;
	}
	else if( bEncodeFailed )
	{
		m_bUserCancel = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect encode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		if(m_pReverseAudioTask)
		{
			m_pReverseAudioTask->cancelReverseAudio();
		}
	}
	else if(bDecodeFailed)
	{
		m_bUserCancel = TRUE;
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_CODEC_FAILED;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect decode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		if(m_pReverseAudioTask)
		{
			m_pReverseAudioTask->cancelReverseAudio();
		}
	}
	else if(m_pReverseAudioTask && m_pReverseAudioTask->getErrorCheck())
	{
		m_bUserCancel = TRUE;
		pReverseInfo->m_nResult = m_pReverseAudioTask->getErrorCheck();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}	
	else
	{
		if(m_pReverseAudioTask)
		{
			while(!m_pReverseAudioTask->getAudioFinished() && m_pReverseAudioTask->IsWorking())
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] wait finish m_pReverseAudioTask!!!", __LINE__);
				nexSAL_TaskSleep(10);
			}
		}
		sendNotifyEvent(MESSAGE_MAKE_REVERSE_PROGRESS, 100, uiVideoDTS, m_uiTotalPlayTime);
	}
	
	if( m_pReverseAudioTask )
	{
		if( m_pReverseAudioTask->IsWorking() )
		{
			m_pReverseAudioTask->End(1000);
		}
		SAFE_RELEASE(m_pReverseAudioTask);
	}			

	if(m_pYuvFile)
	{
		nexSAL_FileClose(m_pYuvFile);
		m_pYuvFile = NULL;
		nexSAL_FileRemove(m_strTempFilePath);
	}
			
	// deinit File Reader
	deinitFileReader();
	// deinit Video Decoder
	deinitVideoDecoder();
	// deinit file writer.
	deinitWriter();
	deinitRenderer();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processTransCoding Ended!!!", __LINE__);
	return;	
}

void CNEXThread_TranscodingTask::processReverseMemory(CNxMsgReverseInfo* pReverseInfo)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processReverse Start!!!", __LINE__);

	NXBOOL	bWriteVideo = FALSE;
	NXBOOL	bFinishedVideo = FALSE;
	NXBOOL	bVideoEndOfContent = FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	unsigned int uiVideoDecoderErrRet = 0;
	unsigned int uiEnhancement = 0;
	unsigned int iVideoFrameNALHeaderSize = 0;

	unsigned int uiVideoPTS = 0;
	unsigned int uiVideoDTS = 0;
	unsigned int uiDecDTS = 0;
	unsigned int uiDecPTS = 0;
	unsigned int uiCheckGetOutputTime = 0;
	unsigned int uiVideoDecOutCTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	unsigned int uiCheckDecTime = 0;

	int iRetryCountForDecFail = 0;

	NXBOOL bEncodeFailed = FALSE;
	NXBOOL bDecodeFailed = FALSE;

	int iCurrProgress = 0;
	int iPrevProgress = -1;

	int iRet = 0;

	NXBOOL bTempFileReady = FALSE;
	unsigned int uTempStart = 0;
	unsigned int uTempEnd = 0xffffffff;
	unsigned int uFirstVideoFrame = 0;
	NXBOOL bLastDecFlag = FALSE;
	int iMultipleFactor = 1;
	unsigned int uFPS = 0;
	unsigned char* pYUVData = NULL;    
	NXBOOL bCheckStart = FALSE;
	unsigned int uLastDecTime = 0xffffffff;                    


	// Init File Reader
	iRet = initFileReader();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pReverseInfo->m_nResult = iRet;
		return;
	}

	m_pFileReader->seekTo(0, &uFirstVideoFrame);

	m_uReverseMode = 1;
	iVideoFrameNALHeaderSize = m_pFileReader->getFrameNALHeaderLength();

	if(m_pFileReader->isVideoExist())
	{
		iRet = initRenderer();
		if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
		{
			pReverseInfo->m_nResult = iRet;
			return;
		}
	}

	#if 0
	if(!m_pFileReader->isAudioExist())
	{
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_NOT_SUPPORTED_FORMAT;
		return;
	}
	#endif

	iRet = initWriter(m_uEndTime - m_uStartTime);
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pReverseInfo->m_nResult = iRet;
		return;
	}
	
	// Init Video Decoder.
	if(m_pFileReader->isVideoExist())
	{
		pReverseInfo->m_uFrameMemoryCount = 5;
	
		iRet = initVideoDecoder();
		if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
		{
			pReverseInfo->m_nResult = iRet;
			return;
		}
		m_pFileReader->getVideoFrameDTS( &uFirstVideoFrame, TRUE );

		m_pYuvFile = nexSAL_FileOpen(m_strTempFilePath, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_READWRITE));
		if(m_pYuvFile == NULL)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] processReverse m_pYuvFile is NULL (%s)", __LINE__, m_strTempFilePath);
			pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
			return;
		}
		nexSAL_FileSeek(m_pYuvFile, 0, NEXSAL_SEEK_BEGIN);

		uFPS = m_pFileReader->getVideoFrameRate();
		m_pFrameYUVInfo = new CFrameYUVInfo;

		uTempEnd = m_uEndTime;
		m_pFileReader->seekToVideo(m_uEndTime - 1000/uFPS*pReverseInfo->m_uFrameMemoryCount, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
		m_isNextVideoFrameRead = TRUE;

		if(uTempStart <= m_uStartTime)
			bCheckStart = TRUE;
		
	}

	m_FrameTimeChecker.clear();
	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfGLDraw.CheckModuleStart();
	m_perfGetPixels.CheckModuleStart();
	m_perfSwapbuffer.CheckModuleStart();
	m_perfDrawAll.CheckModuleStart();

	if(m_pFileReader->isVideoExist())
	{
		while(1)
		{

			int iReaderRet = m_pFileReader->getVideoFrame();
			if(iReaderRet == _SRC_GETFRAME_OK)
			{
				m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
				m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );
				if(uiVideoDTS >= uTempEnd && uiVideoPTS >= uTempEnd)
				{
					break;			
				}

				if(m_uStartTime <= uiVideoPTS)
				{
					if(bCheckStart && uLastDecTime == 0xffffffff)
					{
						uLastDecTime = uiVideoPTS;
					}

					if( CNexVideoEditor::m_bNexEditorSDK ) 
					{
						if(m_bSupportFrameTimeChecker && CNexCodecManager::getHardwareDecoderUseCount() == 1)				
						{
							if(uFPS < 45)
							{
								uiVideoPTS = uiVideoPTS/2;
								uiVideoPTS = uiVideoPTS*2;
							}
						}
					}
					m_pFrameYUVInfo->addTime(uiVideoPTS);
				}
			}
			else
			{
				break;
			}
		}
		m_pFrameYUVInfo->sortWithTime();
	    	while(m_pFrameYUVInfo->size()>pReverseInfo->m_uFrameMemoryCount)
		{
			m_pFrameYUVInfo->removeFirst();
		}
		m_pFrameYUVInfo->printTime();
		if(m_pFrameYUVInfo->isExistTime(uLastDecTime) == TRUE)
		{
			bLastDecFlag = TRUE;		                    
		}

		m_pFileReader->seekToVideo(m_uEndTime - 1000/uFPS*pReverseInfo->m_uFrameMemoryCount, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
	}

	if(m_pFileReader->isAudioExist())
	{
		m_pReverseAudioTask = new CNEXThread_ReverseTask(this);
		if( m_pReverseAudioTask == NULL )
		{
			pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] m_pReverseAudioTask is NULL", __LINE__);
			return;
		}

		m_pReverseAudioTask->setReverseInfo(m_strTempFilePath, m_uStartTime, m_uEndTime);
		m_pReverseAudioTask->setFileReader(m_pFileReader);
		m_pReverseAudioTask->setFileWriter(m_pFileWriter);
			
		m_pReverseAudioTask->Begin();
		m_pReverseAudioTask->SendCommand(pReverseInfo);
	}

	while ( m_bIsWorking && m_pFileReader->isVideoExist())
	{
		int err = 0;

		if(m_pReverseAudioTask)
		{
			err = m_pReverseAudioTask->getErrorCheck();
		}

		if( m_bUserCancel || bEncodeFailed || bDecodeFailed)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] task stop because state was changed(%d, %d, %d, %d)", 
				__LINE__, m_bUserCancel, bEncodeFailed ,bDecodeFailed, err);
			break;
		}

		if(err)
		{
			break;
		}

		if(bTempFileReady == FALSE)
		{
			// Video
			if( m_isNextVideoFrameRead )
			{
				int iReaderRet = m_pFileReader->getVideoFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame End", __LINE__);
						bVideoEndOfContent = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
						bVideoEndOfContent = TRUE;
						break;
				};

				processEnhancedFrame(&uiEnhancement);
				m_isNextVideoFrameRead = FALSE;
			}

			if( bVideoEndOfContent )
			{
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}
			else
			{
				// for sync problem.
				m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
				m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );
				m_pFileReader->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );
			}

			if(uiVideoDTS >= uTempEnd && uiVideoPTS >= uTempEnd) {
				bVideoEndOfContent = TRUE;
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}

			uiDecDTS = uiVideoDTS;
			uiDecPTS = uiVideoPTS;

			if( CNexVideoEditor::m_bNexEditorSDK ) 
			{
				if(m_bSupportFrameTimeChecker && CNexCodecManager::getHardwareDecoderUseCount() == 1)				
				{
					if(uFPS < 45)
					{
						iMultipleFactor = 200;
						uiDecDTS = uiDecDTS/2;
						uiDecPTS = uiDecPTS/2;
					}
				}
			}

			if( isFindIDRFrame )
			{
				bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
				if( bIDRFrame == FALSE )
				{
					m_isNextVideoFrameRead = TRUE;
					continue;
				}

				isFindIDRFrame = FALSE;
			}

			uiCheckDecTime = nexSAL_GetTickCount();
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
			nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
										pVideoFrame,
										uiVideoFrameSize,
										NULL,
										uiDecDTS,
										uiDecPTS,
										uiEnhancement,
										&uiVideoDecoderErrRet );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] DecFrame End(%p, %d) DTS(%d) PTS(%d)uiRet(%d) time(%d)",
				__LINE__, pVideoFrame, uiVideoFrameSize, uiVideoDTS, uiVideoPTS, uiVideoDecoderErrRet, nexSAL_GetTickCount() - uiCheckDecTime);

			if (NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_SETTING_CHANGE))
			{
				CheckVideoSettingChange();
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
			{
				uiVideoDecOutCTS = 0;
				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame End and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;
					if(m_bSupportFrameTimeChecker)
					{
						m_FrameTimeChecker.addFrameCTSVec(uiDecDTS, uiDecPTS, iMultipleFactor);
					}
				}

				// for msm8x26
				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					//bFinishedVideo = TRUE;
					bTempFileReady = TRUE;
				}

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
				{
					unsigned char*	pY			= NULL;
					unsigned char*	pU			= NULL;
					unsigned char*	pV			= NULL;
					unsigned int 	uiCheckCTS 	= 0;

					if( m_uiCheckVideoDecInit != 0 )
					{
						m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] VideoDecoder_First output( Time : %d )", __LINE__, m_uiCheckVideoDecInit );
						m_uiCheckVideoDecInit = 0;
					}
					if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
					{
						uiCheckGetOutputTime = nexSAL_GetTickCount();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
							continue;
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
							__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);

						if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2  ||
							( m_bSupportFrameTimeChecker && m_FrameTimeChecker.getValidFrameCTSWithApplyMutiplePTS(uiVideoDecOutCTS, &uiVideoDecOutCTS, TRUE) == FALSE ) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}

						if(uiVideoDecOutCTS >= uTempEnd) {
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}

						if(uiVideoDecOutCTS < m_uStartTime)
						{
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}
						m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
							NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hTranscodingRenderer, (ANativeWindow*)m_pTranscodingDecSurface, -1);
#endif
						if( saveVideoYUV(pY, uiVideoDecOutCTS) == FALSE )
						{
							pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED;
							bEncodeFailed = TRUE;
							continue;
						}

						if(bVideoEndOfContent == TRUE)
						{
							if(m_FrameTimeChecker.size() == 0)
							{
								bTempFileReady = TRUE;
							}
						}
					}
					else
					{
						uiCheckGetOutputTime = nexSAL_GetTickCount();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] nexCAL_VideoDecoderGetOutput failed",__LINE__);
							continue;
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput *m_pY=0x%08x CTS(%d) Time(%d)",
							__LINE__, pY, uiVideoDecOutCTS, nexSAL_GetTickCount() - uiCheckGetOutputTime);
						
						if( (int)uiVideoDecOutCTS < 0 || uiVideoDecOutCTS == 2  ||
							( m_bSupportFrameTimeChecker && m_FrameTimeChecker.getValidFrameCTSWithApplyMutiplePTS(uiVideoDecOutCTS, &uiVideoDecOutCTS, TRUE) == FALSE ) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, uiVideoDTS, uiVideoDecOutCTS);
							continue;
						}

						if(uiVideoDecOutCTS < m_pFrameYUVInfo->getFirstTime())
						{
							continue;
						}

						if(uiVideoDecOutCTS >= uTempEnd) {
							continue;
						}

						pYUVData = (unsigned char*)nexSAL_MemAlloc(m_iSettingWidth*m_iSettingHeight*3/2);
						memcpy(pYUVData, pY, m_iSettingWidth*m_iSettingHeight*3/2);
						m_pFrameYUVInfo->addYUVData(uiVideoDecOutCTS, pYUVData);
                    				m_pFrameYUVInfo->removeFirst();

						if(bVideoEndOfContent == TRUE)
						{
							if(m_FrameTimeChecker.size() == 0)
								bTempFileReady = TRUE;
						}
					}

					if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
						//bFinishedVideo = TRUE;
						bTempFileReady = TRUE;					
					}

					if( (uiVideoDecOutCTS & 0x80000000 ) == 0x80000000 )
					{
						if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
						{
							m_isNextVideoFrameRead = TRUE;
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Skip current frame because invalid frame CTS(%d)", __LINE__, uiVideoDecOutCTS);
						continue;
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] GetOutput did not exist", __LINE__);
					nexSAL_TaskSleep(1);
				}
			}
			else
			{
				if( m_pCodecWrap->isHardwareCodec() == FALSE )
				{
					isFindIDRFrame = TRUE;
					m_isNextVideoFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] SW Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);
					continue;
				}
				
				NXBOOL bNeedMoreDecode = FALSE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Decoding Failed(%d)",__LINE__, uiVideoDecoderErrRet);

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed and next frame", __LINE__);
					m_isNextVideoFrameRead = TRUE;
					bNeedMoreDecode = TRUE;
				}

				if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Receive decoder EOS", __LINE__);
					bFinishedVideo = TRUE;
				}				

				if(NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Decode Frame Failed (Output timeout)", __LINE__);
					bNeedMoreDecode = TRUE;
				}

				if( bNeedMoreDecode == FALSE )
				{
					NXBOOL bIDR = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pFileReader->m_uiVideoObjectType, pVideoFrame, uiVideoFrameSize, (void*)&iVideoFrameNALHeaderSize, m_pFileReader->getFrameFormat(), FALSE);
					if( bIDR == FALSE )
					{
						m_isNextVideoFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task retry again for decoder reinit with next frame", __LINE__);
						continue;
					}

					if( iRetryCountForDecFail > 1 )
					{
						bDecodeFailed = TRUE;
						continue;
					}
					
					deinitVideoDecoder();
					// nexSAL_TaskSleep(500);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task reinit codec for codec repare", __LINE__);
					if( reinitVideoDecoder(pVideoFrame, uiVideoFrameSize) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task Exit because reinit failed", __LINE__);
						bDecodeFailed = TRUE;
						continue;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video Task re start after reinit codec(%d)", __LINE__, iRetryCountForDecFail);
					iRetryCountForDecFail++;

					// CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, VIDEOEDITOR_EVENT_PLAY_ERROR, NEXVIDEOEDITOR_ERROR_CODEC_INIT);
					// break;
				}
				nexSAL_TaskSleep(1);
			}
			continue;
		}

		if(bWriteVideo == FALSE)
		{
			bWriteVideo = TRUE;
		}
		
		if(bWriteVideo == TRUE && bFinishedVideo == FALSE)
		{
			if(m_pFrameYUVInfo->getSize() > 0)
			{
				m_uEncodingTime = m_pFileWriter->getVideoTime();
				unsigned int uAudioTime = 0;
				if(m_pReverseAudioTask)
				{
					uAudioTime = m_pReverseAudioTask->getAudioEncodingTime();
				}

				uiVideoPTS = m_pFrameYUVInfo->getLastTime();
                
				if(m_pFrameYUVInfo->getLastData() == NULL)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] m_pFrameMemory data is NULL %d, %d", __LINE__, m_pFrameYUVInfo->getSize(), uiVideoPTS);
					m_pFrameYUVInfo->removeLast();
					continue;
				}
                
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] m_pFrameMemory(%d, %d), %d, %d", __LINE__, m_pFrameYUVInfo->getSize(), uiVideoPTS, m_uEncodingTime, uAudioTime);
	
				if(m_uBaseReverseTime == 0)
					m_uBaseReverseTime = uiVideoPTS;

				if(writeVideoYUV(m_uBaseReverseTime - uiVideoPTS, m_pFrameYUVInfo->getLastData(), m_iSettingWidth*m_iSettingHeight*3/2) == FALSE)
				{
					pReverseInfo->m_nResult = getWriterError();
					bEncodeFailed = TRUE;
					continue;
				}
				
	 			iCurrProgress = ((m_uBaseReverseTime - uiVideoPTS)*100 / (m_uEndTime - m_uStartTime));
				if( iCurrProgress > iPrevProgress)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] REVERSE PROGRESS(%d %d %d, %d, %d)", __LINE__, iCurrProgress, m_uBaseReverseTime, m_pFrameYUVInfo->getFirstTime(), m_uStartTime, m_uEndTime);
					sendNotifyEvent(MESSAGE_MAKE_REVERSE_PROGRESS, iCurrProgress, m_uEncodingTime, m_uEndTime - m_uStartTime);
					iPrevProgress = iCurrProgress;
				}
				m_pFrameYUVInfo->removeLast();
				continue;
			}
			else
			{
				if(bLastDecFlag || uiVideoPTS == 0 || uiVideoPTS == 1)
				{
					writeVideoYUV(m_uBaseReverseTime, NULL, 0, TRUE);					
					bFinishedVideo = TRUE;
				}
				else
				{
					unsigned int uSeekTime = 0;
					bCheckStart = FALSE;
					uLastDecTime = 0xffffffff;
					uTempEnd = uiVideoPTS;
					uSeekTime = uiVideoPTS > 1000/uFPS*pReverseInfo->m_uFrameMemoryCount?uiVideoPTS-1000/uFPS*pReverseInfo->m_uFrameMemoryCount:0;
					if(m_uStartTime > uSeekTime)
					{
						uSeekTime = m_uStartTime;
					}
					m_pFileReader->seekToVideo(uSeekTime, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
					if(uTempStart <= m_uStartTime)
						bCheckStart = TRUE;
					while(1)
					{

						int iReaderRet = m_pFileReader->getVideoFrame();
						if(iReaderRet == _SRC_GETFRAME_OK)
						{
							m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
							m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );

							if(uiVideoDTS >= uTempEnd && uiVideoPTS >= uTempEnd)
							{
								break;			
							}

							if(uTempEnd != uiVideoPTS && m_uStartTime <= uiVideoPTS)
							{
								if( CNexVideoEditor::m_bNexEditorSDK ) 
								{
									if(m_bSupportFrameTimeChecker && CNexCodecManager::getHardwareDecoderUseCount() == 1)				
									{
										if(uFPS < 45)
										{
											uiVideoPTS = uiVideoPTS/2;
											uiVideoPTS = uiVideoPTS*2;
										}
									}
								}

								if(bCheckStart && uLastDecTime == 0xffffffff)
								{
									uLastDecTime = uiVideoPTS;
								}
								m_pFrameYUVInfo->addTime(uiVideoPTS);
							}
						}
						else
						{
							break;
						}
					}
					m_pFrameYUVInfo->sortWithTime();
				    	while(m_pFrameYUVInfo->size()>pReverseInfo->m_uFrameMemoryCount)
					{
						m_pFrameYUVInfo->removeFirst();
					}
					m_pFrameYUVInfo->printTime();

					if(m_pFrameYUVInfo->isExistTime(uLastDecTime) == TRUE)
					{
						bLastDecFlag = TRUE;		                    
					}

					m_pFileReader->seekToVideo(uSeekTime, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);

					m_isNextVideoFrameRead = TRUE;
					nexCAL_VideoDecoderReset(m_pCodecWrap->getCodecHandle());
					bTempFileReady = FALSE;
					uiVideoDecoderErrRet = 0;
					bVideoEndOfContent = 0;
					uiEnhancement = 0;
                    			bWriteVideo = FALSE;
				}
			}
		}

		if( bFinishedVideo == TRUE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video is finished!!!",__LINE__);
			break;
		}
 	}

	m_bVideoFinished = TRUE;
	
	if( m_bUserCancel )
	{
		writeVideoFrame(NULL, 0, TRUE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect user cancel while transcoding(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_USER_CANCEL;
	}
	else if( bEncodeFailed )
	{
		m_bUserCancel = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect encode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		if(m_pReverseAudioTask)
		{
			m_pReverseAudioTask->cancelReverseAudio();
		}
	}
	else if(bDecodeFailed)
	{
		m_bUserCancel = TRUE;
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_CODEC_FAILED;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect decode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		if(m_pReverseAudioTask)
		{
			m_pReverseAudioTask->cancelReverseAudio();
		}
	}
	else if(m_pReverseAudioTask && m_pReverseAudioTask->getErrorCheck())
	{
		m_bUserCancel = TRUE;
		pReverseInfo->m_nResult = m_pReverseAudioTask->getErrorCheck();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}	
	else
	{
		if(m_pReverseAudioTask)
		{
			while(!m_pReverseAudioTask->getAudioFinished() && m_pReverseAudioTask->IsWorking())
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] wait finish m_pReverseAudioTask!!!", __LINE__);
				nexSAL_TaskSleep(10);
			}
		}
		sendNotifyEvent(MESSAGE_MAKE_REVERSE_PROGRESS, 100, uiVideoDTS, m_uiTotalPlayTime);
	}
	
	if( m_pReverseAudioTask )
	{
		if( m_pReverseAudioTask->IsWorking() )
		{
			m_pReverseAudioTask->End(1000);
		}
		SAFE_RELEASE(m_pReverseAudioTask);
	}			

	if(m_pYuvFile)
	{
		nexSAL_FileClose(m_pYuvFile);
		m_pYuvFile = NULL;
		nexSAL_FileRemove(m_strTempFilePath);
	}

	delete m_pFrameYUVInfo;

	// deinit File Reader
	deinitFileReader();
	// deinit Video Decoder
	deinitVideoDecoder();
	// deinit file writer.
	deinitWriter();
	deinitRenderer();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processTransCoding Ended!!!", __LINE__);
	return;	
}

void CNEXThread_TranscodingTask::processReverseDirect(CNxMsgReverseInfo* pReverseInfo)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processReverse Start!!!", __LINE__);

	NXBOOL	bVideoEndOfContent = FALSE;
	unsigned int uiVideoPTS = 0;
	unsigned int uiVideoDTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	NXBOOL bEncodeFailed = FALSE;
	NXBOOL bDecodeFailed = FALSE;

	int iCurrProgress = 0;
	int iPrevProgress = -1;

	int iRet = 0;

	unsigned int uTempStart = 0;
	unsigned int uTempEnd = 0xffffffff;
	unsigned int uFirstVideoFrame = 0;
	unsigned int uFPS = 0;
	unsigned int uCount = 0;    
	unsigned int uLastVideoDTS = 0;    
	unsigned int uFirstWriteDTS = 0;    
	unsigned int uFirstWritePTS = 0;    

	unsigned int uiWriteDuration = 0;
	unsigned int uiWriteSize = 0;
    
	// Init File Reader
	iRet = initFileReader();
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pReverseInfo->m_nResult = iRet;
		return;
	}

	m_pFileReader->seekTo(0, &uFirstVideoFrame);

	m_uReverseMode = 2;

//	iRet = initRenderer();
//	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
//	{
//		pReverseInfo->m_nResult = iRet;
//		return;
//	}

	iRet = initWriter(m_uEndTime - m_uStartTime);
	if(iRet != NEXVIDEOEDITOR_ERROR_NONE)
	{
		pReverseInfo->m_nResult = iRet;
		return;
	}
	
	m_FrameTimeChecker.clear();

	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfGLDraw.CheckModuleStart();
	m_perfGetPixels.CheckModuleStart();
	m_perfSwapbuffer.CheckModuleStart();
	m_perfDrawAll.CheckModuleStart();

	m_pFileReader->getVideoFrameDTS( &uFirstVideoFrame, TRUE );

	uTempEnd = m_uEndTime;
	m_pFileReader->seekToVideo(m_uEndTime, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
	m_isNextVideoFrameRead = TRUE;

	uFPS = m_pFileReader->getVideoFrameRate();

	if(m_pFileReader->isAudioExist())
	{
		m_pReverseAudioTask = new CNEXThread_ReverseTask(this);
		if( m_pReverseAudioTask == NULL )
		{
			pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp  %d] m_pReverseAudioTask is NULL", __LINE__);
			return;
		}

		m_pReverseAudioTask->setReverseInfo(m_strTempFilePath, m_uStartTime, m_uEndTime);
		m_pReverseAudioTask->setFileReader(m_pFileReader);
		m_pReverseAudioTask->setFileWriter(m_pFileWriter);
			
		m_pReverseAudioTask->Begin();
		m_pReverseAudioTask->SendCommand(pReverseInfo);
	}

	while ( m_bIsWorking )
	{
		int err = 0;

		if(m_pReverseAudioTask)
		{
			err = m_pReverseAudioTask->getErrorCheck();
		}

		if( m_bUserCancel || bEncodeFailed || bDecodeFailed || err)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] task stop because state was changed(%d, %d, %d, %d)", 
				__LINE__, m_bUserCancel, bEncodeFailed ,bDecodeFailed, err);
			break;
		}

		// Video
		if( m_isNextVideoFrameRead )
		{
			int iReaderRet = m_pFileReader->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				case _SRC_GETFRAME_END:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame End", __LINE__);
					bVideoEndOfContent = TRUE;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
					bVideoEndOfContent = TRUE;
					break;
			};
		}

		// for sync problem.
		m_pFileReader->getVideoFrameDTS( &uiVideoDTS, TRUE );
		m_pFileReader->getVideoFramePTS( &uiVideoPTS, TRUE );
		m_pFileReader->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );

		if(m_uEndTime < uiVideoDTS || (uLastVideoDTS <= uiVideoDTS && uLastVideoDTS != 0))
		{
			uCount++;
			m_pFileReader->seekToVideo(m_uEndTime - 1000/uFPS*uCount, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
			continue;
		}

		if ( ( uiVideoDTS < m_uStartTime && uiVideoPTS < m_uStartTime ) || bVideoEndOfContent ) 
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video is finished!!!",__LINE__);
			break;
		}

		if(uFirstWriteDTS == 0)
		{
			uFirstWriteDTS = uiVideoDTS;
			uFirstWritePTS = uiVideoPTS;
		}
        
		bEncodeFailed = m_pFileWriter->writeVideoFrameWithoutEncode(0, uFirstWritePTS - uiVideoPTS, uFirstWriteDTS- uiVideoDTS, pVideoFrame, uiVideoFrameSize, &uiWriteDuration, &uiWriteSize);
		m_uEncodingTime = uFirstWritePTS - uiVideoPTS;
		uLastVideoDTS = uiVideoDTS;
		uCount++;
		m_pFileReader->seekToVideo(m_uEndTime - 1000/uFPS*uCount, &uTempStart, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);

		iCurrProgress = ((uFirstWriteDTS - uiVideoDTS)*100 / (m_uEndTime - m_uStartTime));
		if( iCurrProgress > iPrevProgress)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] REVERSE PROGRESS(%d %d %d, %d, %d)", __LINE__, iCurrProgress, m_uBaseReverseTime, uFirstWriteDTS - uiVideoDTS, m_uStartTime, m_uEndTime);
			sendNotifyEvent(MESSAGE_MAKE_REVERSE_PROGRESS, iCurrProgress, m_uEncodingTime, m_uEndTime - m_uStartTime);
			iPrevProgress = iCurrProgress;
		} 				

		if(uiVideoDTS == 0)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Video is finished!!!",__LINE__);
			break;
		}
	}
		
	m_bVideoFinished = TRUE;
	
	if( m_bUserCancel )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect user cancel while transcoding(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_USER_CANCEL;
	}
	else if( bEncodeFailed )
	{
		m_bUserCancel = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect encode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		if(m_pReverseAudioTask)
		{
			m_pReverseAudioTask->cancelReverseAudio();
		}
	}
	else if(bDecodeFailed)
	{
		m_bUserCancel = TRUE;
		pReverseInfo->m_nResult = NEXVIDEOEDITOR_ERROR_TRANSCODING_CODEC_FAILED;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect decode failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
		if(m_pReverseAudioTask)
		{
			m_pReverseAudioTask->cancelReverseAudio();
		}
	}
	else if(m_pReverseAudioTask && m_pReverseAudioTask->getErrorCheck())
	{
		m_bUserCancel = TRUE;
		pReverseInfo->m_nResult = m_pReverseAudioTask->getErrorCheck();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Detect failed(%d %d)", __LINE__, uiVideoDTS, m_uiTotalPlayTime);
	}	
	else
	{
		sendNotifyEvent(MESSAGE_MAKE_REVERSE_PROGRESS, 100, uiVideoDTS, m_uiTotalPlayTime);

		if(m_pReverseAudioTask)
		{
			while(!m_pReverseAudioTask->getAudioFinished() && m_pReverseAudioTask->IsWorking())
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] wait finish m_pReverseAudioTask!!!", __LINE__);
				nexSAL_TaskSleep(10);
			}
		}
	}
	
	if( m_pReverseAudioTask )
	{
		if( m_pReverseAudioTask->IsWorking() )
		{
			m_pReverseAudioTask->End(1000);
		}
		SAFE_RELEASE(m_pReverseAudioTask);
	}			

	// deinit File Reader
	deinitFileReader();
	// deinit file writer.
	deinitWriter();
//	deinitRenderer();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processTransCoding Ended!!!", __LINE__);
	return;	
}
int CNEXThread_TranscodingTask::decodingPCM( NXBOOL bFrameEnd, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int uiCTS)
{
	unsigned int uiPCMBufSize = m_uiAudioDecodeBufSize;// m_iSampleForChannel*m_iChannels*(m_iBitsForSample/8);
	unsigned int uiDecoderErrRet = 0;
	
	nexCAL_AudioDecoderDecode(	m_hCodecAudio,
								pFrame, 
								uiFrameSize, 
								NULL, 
								m_pAudioDecodeBuf,
								(int *)&uiPCMBufSize,
								uiCTS,
								NULL,
								bFrameEnd == FALSE?NEXCAL_ADEC_FLAG_NONE:NEXCAL_ADEC_FLAG_END_OF_STREAM,
								&uiDecoderErrRet );

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[TranscodingTask.cpp %d] Audio Decode. (Buf %p, BufSize:%d, TS:%d Ret:0x%x)", 
		__LINE__, m_pAudioDecodeBuf, uiPCMBufSize , uiCTS, uiDecoderErrRet);
	
	if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_NEXT_FRAME))
	{
		//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ADTask.cpp %d] getNextFrame is OK!", __LINE__);
		m_isNextAudioFrameRead = TRUE;
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

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio Output was changed!(S(%d) C(%d) BpS(%d) SC(%d)", 
				__LINE__, uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);
		}
		
		if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_EXIST))
		{
			unsigned char* pBuf = NULL;
			unsigned int uBufSize = 0;
			unsigned int uiSize = 0;
			unsigned int uiDuration = 0;
			unsigned int	uiTime		= 0;
			unsigned int 	uiInterval 	= (AUDIO_ENCODE_MAX_SIZE * 1000)/(m_uiBitsPerSample/8*m_uiChannels*m_uiSampleRate) + 1;

			if(m_isResampling)
			{
				unsigned int uTotalOutBufSize=0, OutBufSize=0, uLastSample;
				unsigned int uInputSamplesPerChannel = uiPCMBufSize / (m_uiBitsPerSample >> 3) /  m_uiChannels;

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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[TranscodingTask.cpp %d] resampling uiPCMSize(%d)->ResamplerOutputBufSize(%d)", __LINE__, uiPCMBufSize, uBufSize);
			}
			else
			{
				pBuf = m_pAudioDecodeBuf;
				uBufSize = uiPCMBufSize;
			}

			// to set 2 channels
			if(m_uiChannels == 1)
			{
				int samples = uBufSize/(m_uiBitsPerSample/8);

				for(int i=0; i<samples; i++)
				{
					memcpy(	m_pAudioUpChannelBuf + i*4, pBuf + i*2, m_uiBitsPerSample/8);
					memcpy(	m_pAudioUpChannelBuf + i*4 + 2, pBuf + i*2, m_uiBitsPerSample/8);
				}

				pBuf = m_pAudioUpChannelBuf;
				uBufSize *= 2;
			}

			uiTime = uiCTS;

			while( uBufSize > AUDIO_ENCODE_MAX_SIZE)
			{
				if( m_pFileWriter->setAudioFrame(uiTime, pBuf, AUDIO_ENCODE_MAX_SIZE, &uiDuration, &uiSize) == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Audio Frame Write failed", __LINE__);
				}

				uiTime += uiInterval;
				pBuf += AUDIO_ENCODE_MAX_SIZE;
				uBufSize -= AUDIO_ENCODE_MAX_SIZE;
			}

			if( m_pFileWriter->setAudioFrame(uiTime, pBuf, uBufSize, &uiDuration, &uiSize) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TranscodingTask.cpp %d] Audio Frame Write failed", __LINE__);
				return 2;
			}


			if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Addio Task EOS received", __LINE__);
			}
		}
		else
		{
			if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio Task end received", __LINE__);
			}
			
			if( bFrameEnd == TRUE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio is End!!", __LINE__);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[TranscodingTask.cpp %d] Audio Output is empty!!", __LINE__);
			}
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] Audio Dec Failed(%p %d %d Ret:%d)", __LINE__, pFrame, uiFrameSize, uiCTS, uiDecoderErrRet);
		m_isNextAudioFrameRead = TRUE;
		nexSAL_TaskSleep(5);

		if( bFrameEnd  == TRUE) // Frame End.
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] TAudio Dec Failed(%p %d %d Ret:%d)", __LINE__, pFrame, uiFrameSize, uiCTS, uiDecoderErrRet);
		}
		else
		{
			return 1;
		}
	}
	return 0;
}

int CNEXThread_TranscodingTask::CheckVideoSettingChange()
{
	NXUINT32 uNewWidth = 0, uNewHeight = 0;
	NXUINT32 uNewWidthPitch = 0;
	NXUINT32 uResetRenderer = 0;
	NXUINT32 uGIRet = 0;
	NXUINT32 uNewcolorFormat = 0;    

	uGIRet = nexCAL_VideoDecoderGetInfo(m_pCodecWrap->getCodecHandle(), NEXCAL_VIDEO_GETINFO_WIDTH, &uNewWidth);
	m_iSettingWidth = NEXCAL_ERROR_NONE == uGIRet ? uNewWidth : m_iSrcWidth;
	uGIRet = nexCAL_VideoDecoderGetInfo(m_pCodecWrap->getCodecHandle(), NEXCAL_VIDEO_GETINFO_HEIGHT, &uNewHeight);
	m_iSettingHeight = NEXCAL_ERROR_NONE == uGIRet ? uNewHeight : m_iSrcHeight;
	uGIRet = nexCAL_VideoDecoderGetInfo(m_pCodecWrap->getCodecHandle(), NEXCAL_VIDEO_GETINFO_WIDTHPITCH, &uNewWidthPitch);               
	m_iSettingPitch = NEXCAL_ERROR_NONE == uGIRet ? uNewWidthPitch : m_iSrcPitch;
	nexCAL_VideoDecoderGetInfo(m_pCodecWrap->getCodecHandle(), NEXCAL_VIDEO_GETINFO_RENDERER_RESET ,&uResetRenderer);

	uGIRet = nexCAL_VideoDecoderGetInfo(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, &uNewcolorFormat);               
	m_iOutputFormat = NEXCAL_ERROR_NONE == uGIRet ? uNewcolorFormat : m_iOutputFormat;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[TranscodingTask.cpp %d] CheckVideoSettingChange(%d %d, %d, %d, %d)", __LINE__, uNewWidth, uNewHeight, uNewWidthPitch, uResetRenderer, uNewcolorFormat);
	return 0;
}

NXBOOL CNEXThread_TranscodingTask::initResampler()
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

NXBOOL CNEXThread_TranscodingTask::deinitResampler()
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
