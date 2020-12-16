/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_PFrameProcessTask.cpp
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

// #include <GLES2/gl2.h>
// #include <GLES2/gl2ext.h>

#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_PFrameProcessTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_Util.h"

CNEXThread_PFrameProcessTask::CNEXThread_PFrameProcessTask( void )
{
	m_pSource		= NULL;
	m_pFileWriter		= NULL;
	m_pClipItem			= NULL;

	m_pCodecWrap		= NULL;
	m_bFirstOutputDrop	= TRUE;

	m_iSrcWidth			= 0;
	m_iSrcHeight		= 0;
	m_iSrcPitch			= 0;

	m_iVideoBitrate		= 0;
	m_iVideoFramerate	= 0;
	m_iVideoNalHeaderSize	= 0;
 	
	m_isNextVideoFrameRead	= FALSE;

	m_uiClipID			= INVALID_CLIP_ID;
	m_uiBaseTime		= 0;
	m_uiStartTrimTime	= 0;
	m_iSpeedFactor		= 0;
	m_uiDstTime			= 0;
	
	m_hVideoEnc			= NULL;
	m_MediaCodecInputSurf	= NULL;
	m_uiMediaCodecUserData	= 0;
	m_fnSetMediaCodecTimeStamp		= NULL;
	m_fnResetMediaCodecVideoEncoder	= NULL;

	m_bOutputDSI				= FALSE;
	m_iEncodeFrameDSINALSize	= 0;
	m_bPFrameEncodeDone			= FALSE;
	m_uiNextIDRTime				= 0;;
	
	m_pRawBuffer				= (unsigned char*)nexSAL_MemAlloc(WRITER_RAW_FRAME_TEMP_SIZE);

	m_hRenderer				= NULL;
	m_pDecSurface			= NULL;
	m_pOutputSurface		= NULL;

	m_bUserCancel			= FALSE;

	m_pUserData				= NULL;

	m_bCheckEncoderDSI		= FALSE;
	m_pEncoderDSI			= NULL;
	m_iEncoderDSISize		= 0;

	m_bPFrameAndAudio		= FALSE;
	m_bDirectExportEnd		= FALSE;

	m_vDTS.clear();
}

CNEXThread_PFrameProcessTask::~CNEXThread_PFrameProcessTask( void )
{
	deinitVideoDecoder();
	deinitRenderer();
	deinitVideoEncoder();

	SAFE_RELEASE(m_pCodecWrap);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pFileWriter);

	m_vDTS.clear();

	if( m_pRawBuffer != NULL )
	{
		nexSAL_MemFree(m_pRawBuffer);
		m_pRawBuffer = NULL;
	}

	if( m_pUserData )
	{
		nexSAL_MemFree(m_pUserData);
		m_pUserData = NULL;
	}	

	if( m_pEncoderDSI )
	{
		nexSAL_MemFree(m_pEncoderDSI);
		m_pEncoderDSI = NULL;
	}
	m_bDirectExportEnd	= FALSE;
}

void CNEXThread_PFrameProcessTask::End( unsigned int uiTimeout )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) End In(%d)", __LINE__, m_uiClipID, m_bIsWorking);
	if( m_bIsWorking == FALSE ) return;

	if( m_vDTS.size() > 0 )
	{
		NXUINT32 uVideoDTS = m_vDTS[0];
		m_vDTS.erase(m_vDTS.begin());
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Video No dts time(%d, %u)", __LINE__, m_vDTS.size(), uVideoDTS);
	}

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
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) End Out", __LINE__, m_uiClipID);
}

void CNEXThread_PFrameProcessTask::WaitTask()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] WaitTask In", __LINE__, m_uiClipID);
	if( m_bCheckEncoderDSI  )
	{
		nexSAL_TaskWait(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] WaitTask Wait end", __LINE__, m_uiClipID);
		nexSAL_TaskDelete(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] WaitTask delete end", __LINE__, m_uiClipID);
		m_hThread = NEXSAL_INVALID_HANDLE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] WaitTask Out", __LINE__, m_uiClipID);
}


NXBOOL CNEXThread_PFrameProcessTask::setSource(CNexSource* pSource)
{
	SAFE_RELEASE(m_pSource);
	m_pSource = pSource;
	SAFE_ADDREF(m_pSource);

	if( m_pSource != NULL )
 	{
		m_pSource->getVideoResolution((NXUINT32*)&m_iSrcWidth, (NXUINT32*)&m_iSrcHeight);
		m_iSrcPitch = m_iSrcWidth;

		m_pSource->getVideoBitrate((NXUINT32*)&m_iVideoBitrate);
		m_iVideoFramerate = m_pSource->getVideoFrameRate();

		m_iVideoNalHeaderSize = m_pSource->getFrameNALHeaderLength();

		if( m_iSrcWidth == 0 || m_iSrcHeight == 0 ||m_iSrcPitch == 0 ||m_iVideoBitrate == 0 || m_iVideoFramerate == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] setFileReader info failed size(%d %d %d) Bitrate(%d) FrameRate(%d)", 
				__LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch, m_iVideoBitrate, m_iVideoFramerate);
			return FALSE;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] setFileReader size(%d %d %d) Bitrate(%d) FrameRate(%d)", 
			__LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch, m_iVideoBitrate, m_iVideoFramerate);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] setFileReader clear", __LINE__);
	}
	return TRUE;;
}

NXBOOL CNEXThread_PFrameProcessTask::setFileWriter(CNexExportWriter* pFileWriter)
{
	SAFE_RELEASE(m_pFileWriter);
	m_pFileWriter = pFileWriter;
	SAFE_ADDREF(m_pFileWriter);
	return TRUE;
}

NXBOOL CNEXThread_PFrameProcessTask::setClipItem(CClipItem* pClip)
{
	SAFE_RELEASE(m_pClipItem);
	m_pClipItem = pClip;
	SAFE_ADDREF(m_pClipItem);

	if( m_pClipItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] setClip failed(%p)", __LINE__, m_pClipItem);
		return FALSE;
	}

	m_uiClipID			= m_pClipItem->getClipID();
	
	m_uiStartTime		= m_pClipItem->getStartTime();
	m_uiEndTime			= m_pClipItem->getEndTime();	
	
	m_uiBaseTime		= m_pClipItem->getStartTime();
	m_uiStartTrimTime	= m_pClipItem->getStartTrimTime();
	m_iSpeedFactor		= m_pClipItem->getSpeedCtlFactor();

	m_uiDstTime = m_uiNextIDRTime = m_uiEndTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) setClip end Time(%d %d %d) (%d %d)", __LINE__, m_uiClipID, m_uiStartTime, m_uiEndTime, m_uiBaseTime, m_uiStartTrimTime, m_iSpeedFactor);
	return TRUE;
}

NXBOOL CNEXThread_PFrameProcessTask::setUserData(char* pUserData)
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

void CNEXThread_PFrameProcessTask::setPFrameAndAudioMode(NXBOOL bFlag)
{
	m_bPFrameAndAudio = bFlag;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] setPFrameAndAudioMode(%d)", __LINE__, m_bPFrameAndAudio);
}

void CNEXThread_PFrameProcessTask::setCheckEncoderDSIMode(NXBOOL bCheck)
{
	m_bCheckEncoderDSI = bCheck;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] setCheckEncoderDSIMode(%d)", __LINE__, m_bCheckEncoderDSI);
}

int CNEXThread_PFrameProcessTask::getEncoderDSISize()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] getEncoderDSISize(%d)", __LINE__, m_iEncoderDSISize);
	return m_iEncoderDSISize;
}

unsigned char* CNEXThread_PFrameProcessTask::getEncoderDSI()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] getEncoderDSI(%p)", __LINE__, m_pEncoderDSI);
	return m_pEncoderDSI;
}

NXBOOL CNEXThread_PFrameProcessTask::isEndDirectExport()
{
	return m_bDirectExportEnd;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::initVideoDecoder()
{
	CAutoLock m(CNexCodecManager::getVideoDecInitLock());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] initVideoDecoder() In", __LINE__);

	deinitVideoDecoder();

	m_pCodecWrap = new CNexCodecWrap();
	if( m_pCodecWrap == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Allocate codecwrap failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_DEC_INIT_ERROR;
	}

	while( m_bIsWorking )
	{
		if( CNexCodecManager::isHardwareDecodeAvailable() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d]Now available hardware codec", __LINE__);
			break;
		}
		nexSAL_TaskSleep(30);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Wait available hardware codec", __LINE__);
	}

	if( m_bIsWorking == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d]Task exit before video decoder init", __LINE__);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	NXBOOL bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
											NEXCAL_MODE_DECODER,
											m_pSource->getVideoObjectType(),
											m_iSrcWidth,
											m_iSrcHeight,
											m_iSrcWidth);

	if( bRet == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Get Codec Failed(0x%x)", __LINE__, m_pSource->getVideoObjectType());
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_DEC_INIT_ERROR;
	}

	NXUINT8*	pFrame			= NULL;
	NXUINT32	uiFrameSize		= 0;
	NXUINT32	uiDTS			= 0;
	NXUINT32	uiPTS			= 0;
	NXBOOL		isValidVideo	= FALSE;
	NXBOOL		isIntraFrame	= FALSE;

	NXUINT8*	pBaseDSI		= NULL;
	NXUINT32	uiBaseDSISize	= 0;
	NXUINT8*	pNewConfig		= NULL;
	NXUINT32	uiNewConfigSize	= 0;
	NXUINT8*	pEnhancedDSI				= NULL;
	NXUINT32	uiEnhancedDSISize			= 0;

	NXINT64			isUseIframeVDecInit		= FALSE;
	unsigned int	eRet 					= 0;
	int 			iDSINALHeaderSize		= 0;
	int 			iFrameNALHeaderSize		= 0;

	unsigned int	iWidth					= 0;
	unsigned int	iHeight					= 0;

	if( nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	iDSINALHeaderSize = m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

	if( m_pCodecWrap->isHardwareCodec() )
	{
		if(m_hRenderer != NULL)
		{
			if( m_pDecSurface == NULL )
			{
				ANativeWindow* surface = NULL;
				NXT_ThemeRenderer_CreateSurfaceTexture( m_hRenderer, &surface );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] Created SurfaceTexture for transcoding decoding(%p)", __LINE__, surface);
				if( surface == NULL )
				{
					SAFE_RELEASE(m_pCodecWrap);
					return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_DEC_INIT_SURFACE_ERROR;
				}
				m_pDecSurface = (void*)surface;
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Use surfaceTexture(%p)", __LINE__, m_pDecSurface);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Can't use PFrame re-encode with SoftwareCodec", __LINE__);
		SAFE_RELEASE(m_pCodecWrap);
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_DEC_INIT_ERROR;
	}

	if( m_pDecSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Can't create dec surface", __LINE__);
		SAFE_RELEASE(m_pCodecWrap);
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_DEC_INIT_ERROR;
	}

	m_pCodecWrap->setSurfaceTexture(m_pDecSurface);

	while(1)
	{
		unsigned int uiRet = m_pSource->getVideoFrame();
		if( uiRet == _SRC_GETFRAME_OK )
		{
			m_pSource->getVideoFrameData(&pFrame, &uiFrameSize);
			m_pSource->getVideoFrameDTS(&uiDTS);
			m_pSource->getVideoFramePTS(&uiPTS);

			isValidVideo = NexCodecUtil_IsValidVideo( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize );
			if( isValidVideo == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] This Video frame is invalid", __LINE__);
				nexSAL_TaskSleep(20);
				continue;
			}

			isIntraFrame = NexCodecUtil_IsSeekableFrame((NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] Check I-frame End CTS(%u) Nal Size(%d)", __LINE__, uiDTS, iDSINALHeaderSize);
			if( isIntraFrame )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] I-frame searched...CTS(%u)", __LINE__, uiDTS);
				break;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] P-frame searched...CTS(%u)", __LINE__, uiDTS);
			}
		}
		else if( uiRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TaskSleep(20);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Need buffering Not normal condition(0x%x)", __LINE__, uiRet);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d]  Get Frame fail while decoder init(0x%x)", __LINE__, uiRet);
			goto DECODER_INIT_ERROR;
		}
	}

	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoder Inint", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Dec init(%p %p %d)",
							__LINE__, m_uiClipID, m_pCodecWrap->getCodecHandle(), pBaseDSI, uiBaseDSISize);

			eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
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
												m_pSource->getVideoFrameRate(),
												NEXCAL_VDEC_MODE_NONE);

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp  %d] ID(%d) Video Codec initialize Error(%d)", __LINE__, m_uiClipID, eRet);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp  %d] ID(%d) Video Codec initialize Error. So retry[%d]\n", __LINE__, m_uiClipID);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iSrcWidth		= iWidth;
			m_iSrcHeight	= iHeight;
			m_iSrcPitch		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoder Init", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Dec init %p %d",
							__LINE__, m_uiClipID, pBaseDSI, uiBaseDSISize);

			if( NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES == isUseIframeVDecInit )
			{
				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
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
													m_pSource->getVideoFrameRate(),
													NEXCAL_VDEC_MODE_NONE);
			}
			else
			{
				m_pSource->getDSI( NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, &pEnhancedDSI, &uiEnhancedDSISize );

				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
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
													m_pSource->getVideoFrameRate(),
													NEXCAL_VDEC_MODE_NONE);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp  %d] ID(%d) Video Codec initialize Error(%d)", __LINE__, m_uiClipID, eRet);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n",
							__LINE__, m_uiClipID, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch );
			break;

		default:
			goto DECODER_INIT_ERROR;
	};

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	if( m_pCodecWrap->getFormat() != NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] Not support ColorFormat(0x%x)", __LINE__, m_pCodecWrap->getFormat());
		goto DECODER_INIT_ERROR;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ColorFormat(0x%x) HeaderSize(%d)", __LINE__, m_pCodecWrap->getFormat(), iDSINALHeaderSize);
	m_isNextVideoFrameRead = FALSE;
	m_bFirstOutputDrop	= TRUE;

	return NEXVIDEOEDITOR_ERROR_NONE;

DECODER_INIT_ERROR:
	deinitVideoDecoder();
	return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_DEC_INIT_ERROR;	
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::deinitVideoDecoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] deinitVideoDecoder In", __LINE__);
	if( m_pCodecWrap )
	{
		m_pCodecWrap->deinitDecoder();
		SAFE_RELEASE(m_pCodecWrap);
	}

	if( m_pDecSurface != NULL )
	{
		NXT_ThemeRenderer_DestroySurfaceTexture( m_hRenderer, (ANativeWindow*)m_pDecSurface );
		m_pDecSurface = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] deinitVideoDecoder Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::initVideoEncoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] initVideoEncoder In", __LINE__);

	switch( m_pSource->getVideoObjectType() )
	{
		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
		case eNEX_CODEC_V_MPEG4V:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] initVideoEncoder codec type (0x%x)", __LINE__, m_pSource->getVideoObjectType());
			break;
		default:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] initVideoEncoder codec type failed(0x%x)", __LINE__, m_pSource->getVideoObjectType() );
			return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
	}

	while ( m_bIsWorking )
	{
		if( CNexCodecManager::isHardwareEncodeAvailable() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] initVideoEncoder can use HW encoder at this time", __LINE__);
			break;
		}
		nexSAL_TaskSleep(30);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] initVideoEncoder Wait for HW encoder because can't use encoder at this time", __LINE__);
	}
	
	m_hVideoEnc = CNexCodecManager::getCodec( 	NEXCAL_MEDIATYPE_VIDEO, 
												NEXCAL_MODE_ENCODER, 
												m_pSource->getVideoObjectType(), 
												m_iSrcWidth, 
												m_iSrcHeight, 
												m_iSrcPitch);
	if( m_hVideoEnc ==  NULL  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] initVideoEncoder get codec failed(%p)", __LINE__, m_hVideoEnc );
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_INIT_ERROR;
	}

	unsigned char*	pConfig		= NULL;
	int				iConfigLen	= 0;


	if( CNexVideoEditor::m_bNexEditorSDK )
	{
		NXINT64 lProfileValue = 0, lLevelValue = 0;
		unsigned int uiProfile = 0, uiLevel = 0;
		m_pSource->getH264ProfileLevelID(&uiProfile);
		m_pSource->getH264Level(&uiLevel);

		if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264)
		{
			switch(uiProfile)
			{
				case eNEX_AVC_PROFILE_BASELINE:
					lProfileValue = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_BASELINE;
					break;
				case eNEX_AVC_PROFILE_MAIN:
					lProfileValue = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN;
					break;
				case eNEX_AVC_PROFILE_EXTENDED:
					lProfileValue = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_EXTENDED;
					break;
				case eNEX_AVC_PROFILE_HIGH:
					lProfileValue = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH;
					break;
				case eNEX_AVC_PROFILE_HIGH10:
					lProfileValue = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH10;
					break;
				case eNEX_AVC_PROFILE_HIGH422:
					lProfileValue = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH422;
					break;
				case eNEX_AVC_PROFILE_HIGH444:
					lProfileValue = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH444;
					break;
				case eNEX_HEVC_PROFILE_MAIN:
					lProfileValue = NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN;
					break;
				case eNEX_HEVC_PROFILE_MAIN10:
					lProfileValue = NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN10;
					break;
				case eNEX_HEVC_PROFILE_MAINSTILLPICTURE:
					lProfileValue = NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAINSTILLPICTURE;
					break;                
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] can't use encoder property(%p %d)", __LINE__, m_hVideoEnc, lProfileValue);
					break;
			}
			switch(uiLevel)
			{
				case 10:
					lLevelValue = 0x1 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel1*/;
					break;
				case 11:
					lLevelValue = 0x4 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel11*/;
					break;
				case 12:
					lLevelValue = 0x8 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel12*/;
					break;
				case 13:
					lLevelValue = 0x10 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel13*/;
					break;
				case 20:
					lLevelValue = 0x20 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel2*/;
					break;
				case 21:
					lLevelValue = 0x40 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel21*/;
					break;
				case 22:
					lLevelValue = 0x80 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel22*/;
					break;
				case 30:
					lLevelValue = 0x100 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel3*/;
					break;
				case 31:
					lLevelValue = 0x200 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel31*/;
					break;
				case 32:
					lLevelValue = 0x400 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel32*/;
					break;
				case 40:
					lLevelValue = 0x800 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel4*/;
					break;
				case 41:
					lLevelValue = 0x1000 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel41*/;
					break;
				case 42:
					lLevelValue = 0x2000 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel42*/;
					break;
				case 50:
					lLevelValue = 0x4000 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel5*/;
					break;
				case 51:
					lLevelValue = 0x8000 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel51*/;
					break;
				case 52:
					lLevelValue = 0x10000 /*NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL_AVCLevel52*/;
					break;	

				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] can't use encoder property(%p %lld)", __LINE__, m_hVideoEnc, lLevelValue);
					break;
			}
		}
        
		if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_HEVC)
		{
			switch(uiProfile)
			{
				case eNEX_HEVC_PROFILE_MAIN:
					lProfileValue = NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN;
					break;
				case eNEX_HEVC_PROFILE_MAIN10:
					lProfileValue = NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN10;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] can't use encoder property(%p %d)", __LINE__, m_hVideoEnc, lProfileValue);
					break;
			}
		    
			lLevelValue = uiLevel;
		}

		if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_MPEG4V)
		{
			lProfileValue = uiProfile;
			lLevelValue = uiLevel;
		}
#ifdef FOR_TEST_ENCODER_PROFILE	

		if( (m_iSrcWidth*m_iSrcHeight) >= (SUPPORTED_ENCODE_WIDTH *SUPPORTED_ENCODE_HEIGHT_1080) )
		{
			nexCAL_SetClientIdentifier(m_hVideoEnc, (void*)this);
			if( m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264)
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH);
			else if( m_pSource->getVideoObjectType() == eNEX_CODEC_V_HEVC)        
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE, NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN);
			else if( m_pSource->getVideoObjectType() == eNEX_CODEC_V_MPEG4V)
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_PROFILE, 0x8);     
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] FOR_TEST_ENCODER_PROFILE use encoder property(%p %lld)", __LINE__, m_hVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH);
		}
/*
		if( lProfileValue > 0 )
		{
			nexCAL_SetClientIdentifier(m_hVideoEnc, (void*)this);
			if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264)
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE, lProfileValue);
			else if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_HEVC)
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE, lProfileValue);
			else if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_MPEG4V)
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_PROFILE, lProfileValue);
        
      nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] use encoder property(%p %lld)", __LINE__, m_hVideoEnc, lProfileValue);
		}
*/		
#else
		if( lProfileValue > 0 )
		{
			nexCAL_SetClientIdentifier(m_hVideoEnc, (void*)this);

			if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264)
			{
            			nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE, lProfileValue);
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL, lLevelValue);
			}
			else if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_HEVC)
			{
            			nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE, lProfileValue);
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_HEVC_ENCODER_SET_LEVEL, lLevelValue);
			}
			else if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_MPEG4V)
			{
            			nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE, lProfileValue);
				nexCAL_VideoEncoderSetProperty(m_hVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL, lLevelValue);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [PFrameProcessTask.cpp %d] use encoder property(%p) profile:%d->%lld, level:%d->%lld)", __LINE__, m_hVideoEnc, uiProfile, lProfileValue, uiLevel, lLevelValue);
		}
#endif
	}

	unsigned int uiRet = nexCAL_VideoEncoderInit(	m_hVideoEnc, 
												(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), 
												&pConfig,
												&iConfigLen,
												10,	// int iQuality
												m_iSrcWidth, 
												m_iSrcHeight,
												m_iSrcHeight,
												m_iVideoFramerate,
												1,	// unsignec int bCBR
												(int)m_iVideoBitrate,
												(void*)this);	// unsigned int uClientIdentifier

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder width(%d %d %d %d %d %d) Ret(%d)", 
		__LINE__, 10, m_iSrcWidth, m_iSrcHeight, m_iVideoFramerate, 1, m_iVideoBitrate, uiRet );
	if( uiRet != 0 )
	{
		CNexCodecManager::releaseCodec( m_hVideoEnc );
		m_hVideoEnc = NULL;
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_INIT_ERROR;
	}
	nexCAL_VideoDecoderGetProperty( m_hVideoEnc, NEXCAL_PROPERTY_GET_DECODER_INFO, &m_uiMediaCodecUserData); //yoon
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder Get MediaCodec UserData(%lld)", __LINE__, m_uiMediaCodecUserData); //yoon

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		getMediaCodecInputSurf fInputSurf = (getMediaCodecInputSurf)pEditor->getMediaCodecInputSurf();
		if( fInputSurf )
		{
			m_MediaCodecInputSurf = fInputSurf((void*)m_uiMediaCodecUserData); //yoon
			if( m_MediaCodecInputSurf != NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder Media Codec input Surface(%p)", __LINE__, m_MediaCodecInputSurf);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder Media Codec input Surface failed(%p)", __LINE__, m_MediaCodecInputSurf);
				deinitVideoEncoder();
				SAFE_RELEASE(pEditor);
				return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_INPUT_SURFACE_ERROR;
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder Media Codec input function failed(%p)", __LINE__, fInputSurf);
			deinitVideoEncoder();
			SAFE_RELEASE(pEditor);
			return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_INPUT_SURFACE_ERROR;
		}
		m_fnSetMediaCodecTimeStamp = (setMediaCodecsetTimeStamp)pEditor->getMediaCodecSetTimeStampOnSurf();
		m_fnResetMediaCodecVideoEncoder = (resetMediaCodecVideoEncoder)pEditor->getMediaCodecResetVideoEncoder();
		if( m_fnSetMediaCodecTimeStamp == NULL || m_fnResetMediaCodecVideoEncoder == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder codec time stamp function failed(%p) (%p)", 
				__LINE__, m_fnSetMediaCodecTimeStamp, m_fnResetMediaCodecVideoEncoder);
			deinitVideoEncoder();
			SAFE_RELEASE(pEditor);
			return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_FUNCTION_ERROR;
		}
		SAFE_RELEASE(pEditor);
	}

	if( pConfig != NULL || iConfigLen > 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder Video DSI(%p, %d)", __LINE__, pConfig, iConfigLen );
		nexSAL_MemDump(pConfig, iConfigLen);
	}

 	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] initVideoEncoder End", __LINE__);
	m_bOutputDSI = FALSE;
	m_iEncodeFrameDSINALSize = 4;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::deinitVideoEncoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] deinitVideoEncoder In(0x%d)", __LINE__, m_hVideoEnc);	
	if( m_hVideoEnc )
	{
		nexCAL_VideoEncoderDeinit( m_hVideoEnc );
		CNexCodecManager::releaseCodec( m_hVideoEnc );	
		m_hVideoEnc = NULL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] deinitVideoEncoder End", __LINE__);	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::initRenderer()
{
	NXT_ThemeRenderer_SetNativeWindow(	m_hRenderer, (ANativeWindow*)NULL, m_iSrcWidth, m_iSrcHeight );	
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) setTranscodingInfo:: setting Output Window.", __LINE__, m_uiClipID);
#if 0
	// SET OUTPUT RENDER SURFACE.
	NXT_ThemeRenderer_SetNativeWindow(	m_hRenderer,
											(ANativeWindow*)m_pOutputSurface,
											m_iSrcWidth,
											m_iSrcHeight );
#endif

	NXT_Error eThemeRet = NXT_ThemeRenderer_BeginExport( m_hRenderer, m_iSrcWidth, m_iSrcHeight,  (ANativeWindow*)m_MediaCodecInputSurf );
	if( eThemeRet != NXT_Error_None )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] BeginExport is fail(%d)", __LINE__, eThemeRet);
		deinitVideoEncoder();
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_RENDER_INIT_ERROR;
	}

	// Case : for mantis 9228
	NXT_ThemeRenderer_ClearTransitionEffect(m_hRenderer);
	NXT_ThemeRenderer_ClearClipEffect(m_hRenderer);

	if( m_pUserData )
	{
		NXT_ThemeRenderer_SetWatermarkEffect(m_hRenderer, m_pUserData, 0);
	}

#if 0 // delete by case.
	//Jeff-------------------------------------------------------------------------------------------------------
	NXT_ThemeRenderer_SetHQScale(m_hRenderer, m_iSrcWidth, m_iSrcHeight, m_iSrcWidth, m_iSrcHeight);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] SetHQScaler:SrcW:%d SrcH:%d DstW:%d DstH:%d.", __LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcWidth, m_iSrcHeight);
	//-----------------------------------------------------------------------------------------------------------
#endif
	return NEXVIDEOEDITOR_ERROR_NONE;	
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::deinitRenderer()
{
	if(m_MediaCodecInputSurf != NULL)
	{
		NXT_Error eRet = NXT_ThemeRenderer_EndExport( m_hRenderer);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) EndExport with mediacodec's input Surf", __LINE__, m_uiClipID);
	}
	
	if( m_hRenderer )
	{
		NXT_ThemeRenderer_SetNativeWindow( m_hRenderer, (ANativeWindow*)NULL, 0, 0);
		m_hRenderer = NULL;
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNEXThread_PFrameProcessTask::convertFrame(unsigned char* pSrc, unsigned int uiSize, unsigned char** ppDst, unsigned int* pDstSize)
{
	if( pSrc == NULL || uiSize <= 0 || ppDst == NULL || pDstSize == NULL )
		return FALSE;
	
	if( m_iEncodeFrameDSINALSize == 0 )
	{
		nexSAL_MemDump((unsigned char*)pSrc, uiSize);
		m_iEncodeFrameDSINALSize = NexCodecUtil_GuessNalHeaderLengthSize((unsigned char*)pSrc, uiSize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] get Frame Nal Size(%d)", __LINE__, m_iEncodeFrameDSINALSize);
	}

	unsigned int uiNewSize = 0;
	if( NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == NexCodecUtil_CheckByteFormat((unsigned char*)pSrc, uiSize) )
	{
		uiNewSize = NexCodecUtil_ConvertFormat( m_pRawBuffer, WRITER_RAW_FRAME_TEMP_SIZE, (unsigned char*)pSrc, uiSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, m_iEncodeFrameDSINALSize);
		if( uiNewSize == -1 )
		{
			*ppDst = NULL;
			*pDstSize = 0;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] Convert Annex To Raw Failed ErrorCnt", __LINE__);
			return FALSE;
		}
		else
		{
			*ppDst = m_pRawBuffer;
			*pDstSize = uiNewSize;
		}
	}
	else
	{
		*ppDst =  pSrc;
		*pDstSize = uiSize;
	}
	return TRUE;
}

NXBOOL CNEXThread_PFrameProcessTask::checkDSI_ClipAndEncoder(unsigned char* pDSI, int iSize)
{
	if( pDSI == NULL || iSize <= 0 || m_pClipItem == NULL )
		return FALSE;

#if 0
	return TRUE;
#else
	unsigned char* pClipDSI = NULL;
	unsigned int iClipSize = 0;

	int iRet = m_pClipItem->getDSIofClip(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pClipDSI, &iClipSize);	
	if( iRet == NEXVIDEOEDITOR_ERROR_NONE && pClipDSI != NULL && iClipSize > 0 )
	{
		nexSAL_MemDump(pClipDSI, iClipSize);
		nexSAL_MemDump(pDSI, iSize);
#if 0
		NXBOOL bResult = NexCodecUtil_AVC_IsDirectMixable(g_p1s_dsiInfo_1280_720, sizeof(g_p1s_dsiInfo_1280_720), g_p1s_dsiInfo_1280_720_120, sizeof(g_p1s_dsiInfo_1280_720_120));
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] check compareVideoDSIofClip DSI (%d)", __LINE__, bResult);
#endif		
        int isMixable = NexCodecUtil_AVC_IsDirectMixable(pClipDSI, iClipSize, pDSI, iSize);
        
		if( isMixable == NEXCODECUTIL_DIRECT_MIXABLE || isMixable == NEXCODECUTIL_DIRECT_MIXABLE_ONLY_DECODING )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] compareVideoDSIofClip DSI match", __LINE__);
			return TRUE;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] compareVideoDSIofClip DSI un - match", __LINE__);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] compareVideoDSIofClip get DSI of clip Failed", __LINE__);
	return FALSE;
#endif	
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::writeOneFrameWithEncode(NXBOOL bEnd, unsigned int uiCTS)
{
	unsigned char		pTempBuffer[50];
	unsigned int 		uiVEncRet		= 0;
	unsigned int		uiRet			= 0;
	NXBOOL			bNextFrame		= FALSE;
	NXBOOL			bEOS			= FALSE;
	unsigned char*	pBuffer = bEnd ? NULL : pTempBuffer;

	m_vDTS.insert(m_vDTS.end(), uiCTS);

	while( m_bIsWorking )
	{
		if( bEnd )
		{
			if( bEOS )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeOneFrameWithEncode end(%d %d)", 
					__LINE__, m_uiClipID, bEnd , bEOS);
				break;
			}
		}
		else
		{
			if( bNextFrame == TRUE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeOneFrameWithEncode need next frame(%d %d)", 
					__LINE__, m_uiClipID, bEnd , bEOS);
				break;
			}
			
		}

		uiRet = nexCAL_VideoEncoderEncode(m_hVideoEnc, pBuffer, 0, 0, uiCTS, &uiVEncRet);

		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc Frame success Frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc Frame failed Frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
			return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_ENCODE_ERROR;
		}
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_NEXT_INPUT) )
		{
			bNextFrame = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc next frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
		}
		if(NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_EOS))
		{
			bEOS = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Encode is finished.(EOS) result(%d %d)", __LINE__, m_uiClipID, uiRet, uiVEncRet);
		}
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_TIMEOUT) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc TimeOut(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
		}
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
		{
			unsigned char* pBitStream = NULL;
			int uiBitStreamLen = 0;
			NXUINT32 uVEncPTS = 0;
			NXUINT32 uVEncDTS = 0;

			uiRet = nexCAL_VideoEncoderGetOutput(m_hVideoEnc, &pBitStream, &uiBitStreamLen, &uVEncPTS);
			if( uiRet != 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc getoutput Failed(%d %d)", __LINE__, m_uiClipID, uiRet, uiVEncRet);
				return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_ENCODE_ERROR;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc getoutput sucessed pFrame(%p) Size(%d) CTS(%d)", 
				__LINE__, m_uiClipID, pBitStream, uiBitStreamLen, uVEncPTS);

			if( m_bOutputDSI == FALSE )
			{
				m_bOutputDSI = TRUE;
				/*
				if( checkDSI_ClipAndEncoder(pBitStream, uiBitStreamLen) == FALSE )
				{
					return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_DSI_DIFF_ERROR;
				}
				*/
				continue;
			}

			if( m_vDTS.size() > 0 )
			{
				uVEncDTS = m_vDTS[0];
				m_vDTS.erase(m_vDTS.begin());
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] VideoTime to enter writer (%d, %u)", __LINE__, m_vDTS.size(), uVEncDTS);
			}

			if( m_uiNextIDRTime > 0 &&  uVEncPTS >= m_uiDstTime )
			{
				m_bPFrameEncodeDone = TRUE;
				unsigned int uiSeekResultTime = 0;
				m_pSource->seekToVideoWithAbsoluteTime(m_uiNextIDRTime + 3, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV);
				m_isNextVideoFrameRead = TRUE;
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) PFrame encode End and write more frame(%d %d %d)",
					__LINE__, m_uiClipID, m_uiNextIDRTime, uVEncPTS, m_uiDstTime);
				return NEXVIDEOEDITOR_ERROR_NONE;
			}
			
			unsigned char* pNewBuff = NULL;
			unsigned int uiNewSize = 0;
			NXBOOL bConvert = convertFrame(pBitStream, uiBitStreamLen, &pNewBuff, &uiNewSize);
			if( bConvert )
			{
				while( m_bIsWorking )
				{
					unsigned int uiDuration = 0;
					unsigned int uiSize = 0;	
				
					int iWriterRet = m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, uVEncPTS, uVEncDTS, pNewBuff, uiNewSize, &uiDuration, &uiSize);
					if( iWriterRet == 0 )		// write sucessed
					{
						// m_isNextFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Write sucess(%u, %d)", __LINE__, m_uiClipID, uVEncPTS, iWriterRet);
						break;
					}
					else if( iWriterRet == 1 )	// write failed
					{
						// m_isNextFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Write failed(%u, %d)", __LINE__, m_uiClipID, uVEncPTS, iWriterRet);
						return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_WRITER_WRITE_ERROR;
					}
					else if( iWriterRet == 2 )	// write wait
					{
						nexSAL_TaskSleep(5);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Write failed(%u, %d)", __LINE__, m_uiClipID, uVEncPTS, iWriterRet);
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Writer unknown(%u, %d)", __LINE__, m_uiClipID, uVEncPTS, iWriterRet);
						return NEXVIDEOEDITOR_ERROR_NONE;
					}
				}								
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Encoded frame was not invalid frame(%u)", __LINE__, m_uiClipID, uVEncPTS);
				return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_FRAME_CONVERT_ERROR;
			}			
		}

	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::writeOneFrameWithEncode2(NXBOOL bEnd, unsigned int uiCTS)
{
	unsigned char		pTempBuffer[50];
	unsigned int 		uiVEncRet		= 0;
	unsigned int		uiRet			= 0;
	NXBOOL			bNextFrame		= FALSE;
	NXBOOL			bEOS			= FALSE;
	unsigned char*	pBuffer = bEnd ? NULL : pTempBuffer;

	m_vDTS.insert(m_vDTS.end(), uiCTS);

	while( m_bIsWorking )
	{
		if( bEnd )
		{
			if( bEOS )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeOneFrameWithEncode end(%d %d)", 
					__LINE__, m_uiClipID, bEnd , bEOS);
				break;
			}
		}
		else
		{
			if( bNextFrame == TRUE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeOneFrameWithEncode need next frame(%d %d)", 
					__LINE__, m_uiClipID, bEnd , bEOS);
				break;
			}
			
		}

		uiRet = nexCAL_VideoEncoderEncode(m_hVideoEnc, pBuffer, 0, 0, uiCTS, &uiVEncRet);

		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc Frame success Frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc Frame failed Frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
			return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_ENCODE_ERROR;
		}
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_NEXT_INPUT) )
		{
			bNextFrame = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc next frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
		}
		if(NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_EOS))
		{
			bEOS = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Encode is finished.(EOS) result(%d %d)", __LINE__, m_uiClipID, uiRet, uiVEncRet);
		}
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_TIMEOUT) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc TimeOut(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pBuffer, uiCTS, uiVEncRet);
		}
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
		{
			unsigned char* pBitStream = NULL;
			int uiBitStreamLen = 0;
			NXUINT32 uiVEncDTS = 0;
			NXUINT32 uiVEncPTS = 0;

			uiRet = nexCAL_VideoEncoderGetOutput(m_hVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
			if( uiRet != 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc getoutput Failed(%d %d)", __LINE__, m_uiClipID, uiRet, uiVEncRet);
				return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_ENCODE_ERROR;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc getoutput sucessed pFrame(%p) Size(%d) CTS(%d)", 
				__LINE__, m_uiClipID, pBitStream, uiBitStreamLen, uiVEncPTS);

			if( m_bOutputDSI == FALSE )
			{
				m_bOutputDSI = TRUE;
				continue;
			}

			if( m_vDTS.size() > 0 )
			{
				uiVEncDTS = m_vDTS[0];
				m_vDTS.erase(m_vDTS.begin());
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] VideoTime to enter writer (%d, %u)", __LINE__, m_vDTS.size(), uiVEncDTS);
			}
            
			if( bEOS && uiBitStreamLen <= 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Write VideoFrame End with EOS", __LINE__);
				return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_WRITER_UNKNOWN_ERROR;
			}

			if( m_uiNextIDRTime > 0 &&  uiVEncPTS >= m_uiDstTime )
			{
				m_bPFrameEncodeDone = TRUE;
				unsigned int uiSeekResultTime = 0;
				m_pSource->seekToVideoWithAbsoluteTime(m_uiNextIDRTime + 3, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV);
				m_isNextVideoFrameRead = TRUE;
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) PFrame encode End and write more frame(%d %d %d)", 
					__LINE__, m_uiClipID, m_uiNextIDRTime, uiVEncPTS, m_uiDstTime);
				return NEXVIDEOEDITOR_ERROR_NONE;
			}

			if( m_uiEndTime < (uiVEncPTS + 10) )
			{
				unsigned int uiDuration = 0;
				unsigned int uiSize = 0;	
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Write VideoFrame End(%d %d)", __LINE__, m_uiClipID, m_uiEndTime, uiVEncPTS);
				m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, m_uiEndTime, m_uiEndTime, NULL, 0, &uiDuration, &uiSize);
				return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_WRITER_UNKNOWN_ERROR;
			}
			
			unsigned char* pNewBuff = NULL;
			unsigned int uiNewSize = 0;
			NXBOOL bConvert = convertFrame(pBitStream, uiBitStreamLen, &pNewBuff, &uiNewSize);
			if( bConvert )
			{
				unsigned int uiDuration = 0;
				unsigned int uiSize = 0;	
				m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, uiVEncPTS, uiVEncDTS, pNewBuff, uiNewSize, &uiDuration, &uiSize);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Encoded frame was not invalid frame (%u, %u)", __LINE__, m_uiClipID, uiVEncPTS, uiVEncDTS);
				return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_FRAME_CONVERT_ERROR;
			}			
		}
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_PFrameProcessTask::OnThreadMain( void )
{
	if( m_bCheckEncoderDSI )
	{
		prodessEncoderDSI();
		return 0;
	}
	
	if( m_bPFrameAndAudio )
	{
		NEXVIDEOEDITOR_ERROR eRet = processPFrameAndAudio();
		if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor )
			{
				pEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, eRet);
				SAFE_RELEASE(pEditor);
			}
		}
		deinitVideoDecoder();
		deinitRenderer();
		deinitVideoEncoder();	
		return 0;
	}
	
	NEXVIDEOEDITOR_ERROR eRet = processPFrameProcess();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
		if( pEditor )
		{
			pEditor->notifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, eRet);
			SAFE_RELEASE(pEditor);
		}
	}
	deinitVideoDecoder();
	deinitRenderer();
	deinitVideoEncoder();	
	return 0;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::processPFrameProcess()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processPFrameProcess Start!!!", __LINE__);

	NXBOOL	bFinishedVideoDec = FALSE;
	NXBOOL	bVideoEndOfContent = FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	unsigned int uiVideoDecoderErrRet = 0;
	unsigned int uiEnhancement = 0;
 
	unsigned int uiVideoPTS = 0;
	unsigned int uiVideoDTS = 0;
	unsigned int uiVideoLastDTS = 0;
	
	unsigned int uiVideoDecOutCTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	unsigned int uiCheckDecTime = 0;

	unsigned int	uiLastPTS	= 0;
	int			iCheckDTS	= 0;
	int			iCheckCTS	= 0;
	
	NXBOOL bDecodeFailed = FALSE;

	int iIDRFrameCount = 0;

	int iRet = 0;
	CClipVideoRenderInfo* pRenderInfo = NULL, *pCurRenderInfo = NULL, *pPrevRenderInfo = NULL;
	unsigned int uiUpdatedTime = 0;

	int                                 iSkipLevelForSlow = 0;
	int                                 iSkipCountForSlow = 0;

	if( m_pSource == NULL )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEREADER_INIT_ERROR;

	if( m_pSource->isVideoExist() == FALSE )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEREADER_INIT_ERROR;
	
	if( m_pFileWriter == NULL )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEWRITER_INIT_ERROR;

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		m_hRenderer = (NXT_HThemeRenderer)pEditor->getExportRenderer();
		m_pOutputSurface = pEditor->getExportOutputSurface();
		SAFE_RELEASE(pEditor)
	}

	if( m_hRenderer == NULL || m_pOutputSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess Can't use export renderer and output surface", __LINE__, m_uiClipID);
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_RENDER_INIT_ERROR;
	}

	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	eRet = initVideoEncoder();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess init video encoder failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}

	eRet = initRenderer();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess init renderer failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}

	eRet = initVideoDecoder();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess init video decoder failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}
	
	unsigned int uiSeekResultTime = 0;
	m_pSource->seekToVideo(m_uiBaseTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);

	if((pCurRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiSeekResultTime, TRUE)) == NULL)
		pCurRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[0];
    
	if(m_pClipItem->getSlowMotion())
	{
		if(pCurRenderInfo->m_iSpeedCtlFactor == 3)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/32;
		else if(pCurRenderInfo->m_iSpeedCtlFactor == 6)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/16;
		else if(pCurRenderInfo->m_iSpeedCtlFactor == 13)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/8;
		else
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30*pCurRenderInfo->m_iSpeedCtlFactor/100;
		iSkipCountForSlow = 0;
	}
    
	m_isNextVideoFrameRead = TRUE;
	
	while ( m_bIsWorking )
	{
		if( m_bUserCancel || bDecodeFailed )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) task stop because state changed(%d, %d)", 
				__LINE__, m_uiClipID, m_bUserCancel, bDecodeFailed);
			break;
		}
		
		if( m_isNextVideoFrameRead )
		{
			bVideoEndOfContent = FALSE;
			int iReaderRet = m_pSource->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				case _SRC_GETFRAME_END:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame End", __LINE__, m_uiClipID);
					bVideoEndOfContent = TRUE;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame error(%d) so End clip", __LINE__, m_uiClipID, iReaderRet);
					bVideoEndOfContent = TRUE;
					break;
			};

			m_isNextVideoFrameRead = FALSE;
		}

		if( bVideoEndOfContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
		}
		else
		{
			m_pSource->getVideoFrameDTS( &uiVideoDTS, TRUE );
			m_pSource->getVideoFramePTS( &uiVideoPTS, TRUE );
			m_pSource->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );

			int iVideoUITime = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);
			if(iVideoUITime > pCurRenderInfo->mEndTime)
			{
				pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiVideoPTS, TRUE);
				if(pRenderInfo != NULL && pRenderInfo != pCurRenderInfo)
				{
					pPrevRenderInfo = pCurRenderInfo;
					pCurRenderInfo = pRenderInfo;
					uiUpdatedTime = uiVideoPTS;

					if(m_pClipItem->getSlowMotion())
					{
						if(pCurRenderInfo->m_iSpeedCtlFactor == 3)
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/32;
						else if(pCurRenderInfo->m_iSpeedCtlFactor == 6)
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/16;
						else if(pCurRenderInfo->m_iSpeedCtlFactor == 13)
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/8;
						else
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30*pCurRenderInfo->m_iSpeedCtlFactor/100;
						isFindIDRFrame = TRUE;
					}
				}
			}
		}

		bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pVideoFrame, uiVideoFrameSize, (void*)&m_iVideoNalHeaderSize, m_pSource->getFrameFormat(), TRUE);
		if( isFindIDRFrame )
		{
			if( bIDRFrame == FALSE )
			{
				m_isNextVideoFrameRead = TRUE;
				continue;
			}
			isFindIDRFrame = FALSE;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) IDR Frame(%d) Time(%d)", __LINE__, m_uiClipID, bIDRFrame, uiVideoDTS);

		if( bIDRFrame )
		{
			iIDRFrameCount++;
			int iCurTime = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);
			if( (m_uiNextIDRTime == m_uiDstTime || m_uiNextIDRTime == 0) && iCurTime > 0 && iCurTime >= m_uiBaseTime )
			{
				m_uiNextIDRTime = uiVideoDTS;
				m_uiDstTime = iCurTime;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) needEncodeTime(%d) Dst Time(%d) curTime(%d)", 
					__LINE__, m_uiClipID, m_uiNextIDRTime, m_uiDstTime, iCurTime);
			}
			iSkipCountForSlow = 0;
		}

		uiVideoLastDTS = uiVideoDTS;

		if(m_pClipItem->getSlowMotion() && iSkipLevelForSlow > 1 && iSkipCountForSlow%iSkipLevelForSlow != 0)
		{
			iSkipCountForSlow++;
			continue;
		}
		iSkipCountForSlow++;

		if( m_bPFrameEncodeDone  )
		{
			unsigned char* pTemp = NULL;		
			unsigned int uiDuration = 0;
			unsigned int uiSize = 0;
			iCheckDTS = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoDTS);
			iCheckCTS = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);
			if( uiVideoDTS < m_uiNextIDRTime )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) check Not Video Time (%d, %d)", __LINE__, m_uiClipID, uiVideoDTS, m_uiNextIDRTime);
				m_isNextVideoFrameRead = TRUE;
				continue;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) check Video Time for direct write Time(%d, %d)", __LINE__, m_uiClipID, iCheckDTS, uiVideoPTS);

			if( m_pClipItem->getEndTime() < iCheckDTS || bVideoEndOfContent)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame End Time(%d, %d %d)", __LINE__, m_uiClipID, iCheckDTS, uiVideoPTS, bVideoEndOfContent);
				m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, iCheckCTS, iCheckDTS, NULL, 0, &uiDuration, &uiSize);
				break;
			}

			if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264 || m_pSource->getVideoObjectType() == eNEX_CODEC_V_HEVC)
			{
				if(m_iVideoNalHeaderSize < 4)	// consider 1 or 2 byte nalsize
				{
					pTemp = (unsigned char*)nexSAL_MemAlloc(uiVideoFrameSize);

					for(int i=0; i<4-m_iVideoNalHeaderSize; i++)
					{
						pTemp[i] = 0;
					}

					memcpy(pTemp + (4- m_iVideoNalHeaderSize), pVideoFrame, uiVideoFrameSize);
					pVideoFrame = pTemp;
					uiVideoFrameSize = uiVideoFrameSize + (4 - m_iVideoNalHeaderSize);
				}
			}
			
			int iWriterRet = m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, iCheckCTS, iCheckDTS, pVideoFrame, uiVideoFrameSize, &uiDuration, &uiSize);

			if(pTemp)
			{
				nexSAL_MemFree(pTemp);
				pTemp = NULL;
			}
			
			if( iWriterRet == 0 )		// write sucessed
			{
				m_isNextVideoFrameRead = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Write sucess(%d, %d)", __LINE__, m_uiClipID, iCheckDTS, iWriterRet);
			}
			else if( iWriterRet == 1 )	// write failed
			{
				eRet = NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_WRITER_WRITE_ERROR;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Write failed(%d, %d)", __LINE__, m_uiClipID, iCheckDTS, iWriterRet);
				break;
			}
			else if( iWriterRet == 2 )	// write wait
			{
				nexSAL_TaskSleep(5);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Write failed(%d, %d)", __LINE__, m_uiClipID, iCheckDTS, iWriterRet);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video One frame Writer unknown(%d, %d)", __LINE__, m_uiClipID, iCheckDTS, iWriterRet);
				break;
			}
			continue;
		}

		uiCheckDecTime = nexSAL_GetTickCount();
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
									pVideoFrame,
									uiVideoFrameSize,
									NULL,
									uiVideoDTS,
									uiVideoPTS,
									uiEnhancement,
									&uiVideoDecoderErrRet );

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) DecFrame End(%p, %d) DTS(%d) PTS(%d)uiRet(%d) time(%d)",
			__LINE__, m_uiClipID, pVideoFrame, uiVideoFrameSize, uiVideoDTS, uiVideoPTS, uiVideoDecoderErrRet, nexSAL_GetTickCount() - uiCheckDecTime);

		if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			uiVideoDecOutCTS = 0;
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				m_isNextVideoFrameRead = TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Receive decoder EOS", __LINE__, m_uiClipID);
				bFinishedVideoDec = TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char* pY = NULL;
				NXBOOL bUsePrevInfo = FALSE;                

				if( m_uiCheckVideoDecInit != 0 )
				{
					m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) VideoDecoder_First output( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
					m_uiCheckVideoDecInit = 0;
				}
				
				if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed",__LINE__, m_uiClipID);
					continue;
				}

				if(uiUpdatedTime > uiVideoDecOutCTS)
					bUsePrevInfo = TRUE;

				if(bUsePrevInfo)
					iCheckCTS = (int)m_TimeCalc.applySpeed(pPrevRenderInfo->mStartTime, pPrevRenderInfo->mStartTrimTime, pPrevRenderInfo->m_iSpeedCtlFactor, uiVideoDecOutCTS);
				else
	    				iCheckCTS = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoDecOutCTS);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Dec GetOutput *m_pY=0x%08x outTime(%d) realTime(%d) Base(%d)",
					__LINE__, m_uiClipID, pY, uiVideoDecOutCTS, iCheckCTS, m_uiBaseTime);

				if( m_bFirstOutputDrop )
				{
					m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
					NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hRenderer, (ANativeWindow*)m_pDecSurface, -1);
#endif
					
					NXT_ThemeRenderer_AquireContext(m_hRenderer);
					NXT_ThemeRenderer_DiscardSurfaceTexture(m_hRenderer, (ANativeWindow*)m_pDecSurface);					
					NXT_ThemeRenderer_ReleaseContext(m_hRenderer, FALSE);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) m_bFirstOutputDrop(%d)",__LINE__, m_uiClipID, m_bFirstOutputDrop);
					m_bFirstOutputDrop = FALSE;
					continue;
				}

				if( iCheckCTS < 0 || iCheckCTS < m_uiBaseTime )
				{
					m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#ifdef FOR_TEST_MEDIACODEC_DEC
					// NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hRenderer, (ANativeWindow*)m_pDecSurface, -1);
#endif
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) dec frame drop(%d %d)", __LINE__, m_uiClipID, iCheckCTS, m_uiBaseTime);

					continue;
				}
				m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
				NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hRenderer, (ANativeWindow*)m_pDecSurface, -1);
#endif

				NXT_ThemeRenderer_AquireContext(m_hRenderer);

				NXT_ThemeRenderer_SetSurfaceTexture(	m_hRenderer,
														0,//track_id ..... there's no need for specifying it in this situation
														NXT_TextureID_Video_1,
														m_iSrcWidth,
														m_iSrcHeight,
														m_iSrcPitch,
														m_iSrcHeight,
														(ANativeWindow*)m_pDecSurface,
														0, //convert_rgb_flag
														0, //tex_id_for_lut
														0, //tex_id_for_customlut_a
														0, //tex_id_for_customlut_b
														0 //tex_id_for_customlut_power
														);

				NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hRenderer, NXT_TextureID_Video_1, 0, 0);

				NXT_ThemeRenderer_GLDraw(m_hRenderer, NXT_RendererOutputType_RGBA_8888, 1);
				m_fnSetMediaCodecTimeStamp(iCheckCTS, (void*)m_uiMediaCodecUserData); //yoon

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame Begin Swap(%d)", __LINE__, m_uiClipID, iCheckDTS);
				NXT_ThemeRenderer_SwapBuffers(m_hRenderer);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame end Swap", __LINE__, m_uiClipID);
				
				NXT_ThemeRenderer_ReleaseContext(m_hRenderer, 0);

				NXBOOL bEnd = FALSE;
				if( (m_uiNextIDRTime > 0 && iCheckCTS > (m_uiDstTime + 500)) || bFinishedVideoDec )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) PFrame encode End and encode with EOS", __LINE__, m_uiClipID);
					bEnd = TRUE;
				}

				eRet = writeOneFrameWithEncode(bEnd, iCheckCTS);
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeOneFrameWithEncode failed(%d)", __LINE__, m_uiClipID, eRet);
					
					return eRet;
				}
			}
		}
		else
		{
			NXBOOL bNeedMoreDecode = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoding Failed(%d)",__LINE__, m_uiClipID, uiVideoDecoderErrRet);
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Decode Frame Failed and next frame", __LINE__, m_uiClipID);
				m_isNextVideoFrameRead = TRUE;
				bNeedMoreDecode = TRUE;
			}

			if(NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Decode Frame Failed (Output timeout)", __LINE__, m_uiClipID);
				bNeedMoreDecode = TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Receive decoder EOS", __LINE__);
				bFinishedVideoDec = TRUE;
			}

			if( bNeedMoreDecode )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoding Failed but try decode again",__LINE__, m_uiClipID);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoding Failed and exit ",__LINE__, m_uiClipID);
				bDecodeFailed = TRUE;
			}
		}

	}

	if(m_pFileWriter->getDECurrentVideoClipID() == m_uiClipID)
	{
		unsigned int uiDuration = 0;
		unsigned int uiSize = 0;
		m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, iCheckDTS, iCheckDTS, NULL, 0, &uiDuration, &uiSize);
	}

	deinitVideoDecoder();
	deinitRenderer();
	deinitVideoEncoder();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) PFrame processEnded!!", __LINE__, m_uiClipID);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::processPFrameAndAudio()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "processPFrameAndAudio Start!!!", __LINE__);

	NXBOOL					bAudioEnd		= FALSE;
	NXBOOL					bVideoEnd		= FALSE;

	NXBOOL					bAudioReadEnd	= FALSE;
	NXBOOL					bVideoReadEnd	= FALSE;

	unsigned char*			pFrame			= NULL;
	unsigned int			uiFrameSize		= 0;
	
	NXINT64				uiAudioDTS		= 0;
	
	unsigned int			uiVideoDTS		= 0;
	unsigned int			uiVideoPTS		= 0;

	int						iNewVideoDTS	= 0;
	int						iNewVideoPTS	= 0;

	unsigned int			uiWriteDuration	= 0;
	unsigned int			uiWriteSize		= 0;

	

	NXBOOL	bFinishedVideoDec = FALSE;
	NXBOOL	bVideoEndOfContent = FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	unsigned int uiVideoDecoderErrRet = 0;
	unsigned int uiEnhancement = 0;
 
	unsigned int uiVideoLastDTS = 0;
	
	unsigned int uiVideoDecOutCTS = 0;
	unsigned int uiVideoFrameSize = 0;
	unsigned char* pVideoFrame = NULL;

	unsigned int uiCheckDecTime = 0;

	unsigned int	uiLastPTS	= 0;
	int			iCheckDTS	= 0;
	int			iCheckCTS	= 0;
	
	NXBOOL	bDecodeFailed = FALSE;
	NXBOOL	bReadError = FALSE;

	int iRet = 0;
	CClipVideoRenderInfo* pRenderInfo = NULL, *pCurRenderInfo = NULL, *pPrevRenderInfo = NULL;
	unsigned int uiUpdatedTime = 0;

	int                                 iSkipLevelForSlow = 0;
	int                                 iSkipCountForSlow = 0;
    
	if( m_pSource == NULL )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEREADER_INIT_ERROR;

	if( m_pSource->isVideoExist() == FALSE )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEREADER_INIT_ERROR;
	
	if( m_pFileWriter == NULL )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEWRITER_INIT_ERROR;

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		m_hRenderer = (NXT_HThemeRenderer)pEditor->getExportRenderer();
		m_pOutputSurface = pEditor->getExportOutputSurface();
		SAFE_RELEASE(pEditor)
	}

	if( m_hRenderer == NULL || m_pOutputSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess Can't use export renderer and output surface", __LINE__, m_uiClipID);
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_RENDER_INIT_ERROR;
	}

	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	eRet = initVideoEncoder();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess init video encoder failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}

	eRet = initRenderer();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess init renderer failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}

	eRet = initVideoDecoder();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameProcess init video decoder failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}
	
	unsigned int uiSeekResult = 0;
	m_pSource->seekTo(m_uiBaseTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
    
	if((pCurRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiSeekResult, TRUE)) == NULL)
		pCurRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[0];

	if(m_pClipItem->getSlowMotion())
	{
		if(pCurRenderInfo->m_iSpeedCtlFactor == 3)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/32;
		else if(pCurRenderInfo->m_iSpeedCtlFactor == 6)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/16;
		else if(pCurRenderInfo->m_iSpeedCtlFactor == 13)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/8;
		else
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30*pCurRenderInfo->m_iSpeedCtlFactor/100;
		iSkipCountForSlow = 0;
	}

	m_isNextVideoFrameRead = TRUE;

	if( m_pSource->isAudioExist() ==  FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Audio Track not exist", __LINE__);
		bAudioEnd = TRUE;
	}	
	
	while ( m_bIsWorking )
	{
		if( bReadError )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) DE Task error exit with frame read failed", __LINE__, m_uiClipID);
			CNexProjectManager::sendEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, 0, 0);
			break;
		}

		if( m_bUserCancel || bDecodeFailed )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) task stop because state changed(%d, %d)", 
				__LINE__, m_uiClipID, m_bUserCancel, bDecodeFailed);
			break;
		}
		
		if( bAudioEnd && bVideoEnd )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) DE Task process audio and video end(%d %d)", __LINE__, m_uiClipID, bAudioEnd, bVideoEnd);
			break;
		}
		
		if( bAudioEnd == FALSE )
		{
			if( iNewVideoPTS > (unsigned int)uiAudioDTS || bVideoEnd )
			{
				int iReaderRet = m_pSource->getAudioFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getAudioFrame End\n", __LINE__, m_uiClipID);
						bAudioReadEnd = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getAudioFrame error(%d) so End clip", __LINE__, m_uiClipID, iReaderRet);
						bReadError = TRUE;
						continue;
				};

				if( bAudioReadEnd )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getAudioFrame EOS(%d)", __LINE__, m_uiClipID, m_uiEndTime);
					m_pFileWriter->writeAudioFrameWithoutEncode(m_uiClipID, m_uiEndTime, NULL, 0);
					bAudioEnd = TRUE;
					continue;
				}
				else
				{
					m_pSource->getAudioFrameCTS( &uiAudioDTS );

					if( m_uiStartTime > uiAudioDTS )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Write AudioFrame Skip(%d %lld)", __LINE__, m_uiClipID, m_uiStartTime, uiAudioDTS);
						continue;
					}
					
					if( m_uiEndTime < ((unsigned int)uiAudioDTS + 10) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Write AudioFrame End(%d %d %lld)", __LINE__, m_uiClipID, m_uiEndTime, uiAudioDTS);
						m_pFileWriter->writeAudioFrameWithoutEncode(m_uiClipID, m_uiEndTime, NULL, 0);
						bAudioEnd = TRUE;
						continue;
					}

					m_pSource->getAudioFrameData( &pFrame, &uiFrameSize );
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) read video AudioFrame time info(%p %d) (%lld)",
						__LINE__, m_uiClipID, pFrame, uiFrameSize, uiAudioDTS);
					m_pFileWriter->writeAudioFrameWithoutEncode(m_uiClipID, (unsigned int)uiAudioDTS, pFrame, uiFrameSize);
				}
				continue;
			}
		}	

		if( m_bPFrameEncodeDone )
		{
			if( bVideoEnd == FALSE )
			{
				int iReaderRet = m_pSource->getVideoFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame End\n", __LINE__, m_uiClipID);
						bVideoReadEnd = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame error(%d) so End clip", __LINE__, m_uiClipID, iReaderRet);
						bReadError = TRUE;
						continue;
				};

				if( bVideoReadEnd )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame EOS(%d)", __LINE__, m_uiClipID, m_uiEndTime);
					m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, m_uiEndTime, m_uiEndTime, NULL, 0, &uiWriteDuration, &uiWriteSize);
					bVideoEnd = TRUE;
					continue;;
				}
				else
				{
					m_pSource->getVideoFrameDTS( &uiVideoDTS, TRUE );
					m_pSource->getVideoFramePTS( &uiVideoPTS, TRUE);
					m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );

					iNewVideoDTS = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoDTS);
					iNewVideoPTS= (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);
					if(iNewVideoPTS > pCurRenderInfo->mEndTime)
					{
						pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiVideoPTS, TRUE);
						if(pRenderInfo != NULL && pRenderInfo != pCurRenderInfo)
						{
							pPrevRenderInfo = pCurRenderInfo;
							pCurRenderInfo = pRenderInfo;
                            
							iNewVideoDTS = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoDTS);
							iNewVideoPTS= (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);

							if(m_pClipItem->getSlowMotion())
							{
								if(pCurRenderInfo->m_iSpeedCtlFactor == 3)
									iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/32;
								else if(pCurRenderInfo->m_iSpeedCtlFactor == 6)
									iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/16;
								else if(pCurRenderInfo->m_iSpeedCtlFactor == 13)
									iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/8;
								else
									iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30*pCurRenderInfo->m_iSpeedCtlFactor/100;
								isFindIDRFrame = TRUE;
							}
						}
					}

					if( m_uiEndTime < (iNewVideoDTS + 10) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Write VideoFrame End(%d %d %d)", __LINE__, m_uiClipID, m_uiEndTime, iNewVideoDTS, uiVideoDTS);
						m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, m_uiEndTime, m_uiEndTime, NULL, 0, &uiWriteDuration, &uiWriteSize);
						bVideoEnd = TRUE;
						continue;;
					}
					
					bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pVideoFrame, uiVideoFrameSize, (void*)&m_iVideoNalHeaderSize, m_pSource->getFrameFormat(), TRUE);
					if( isFindIDRFrame )
					{
						if( bIDRFrame == FALSE )
						{
							continue;
						}
						isFindIDRFrame = FALSE;
					}

					if(bIDRFrame)
						iSkipCountForSlow = 0;
                    
					if(m_pClipItem->getSlowMotion() && iSkipLevelForSlow > 1 && iSkipCountForSlow%iSkipLevelForSlow != 0)
					{
						iSkipCountForSlow++;
						continue;
					}
					iSkipCountForSlow++;
					
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) read video frame time info(%p %d) (%d %d) (%d %d)", 
						__LINE__, m_uiClipID, pFrame, uiFrameSize, uiVideoDTS, uiVideoPTS, iNewVideoDTS, iNewVideoDTS);

					m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, iNewVideoPTS, iNewVideoDTS, pFrame, uiFrameSize, &uiWriteDuration, &uiWriteSize);
				}		
			}
			continue;
		}	

		if( bFinishedVideoDec )
		{
			NXT_ThemeRenderer_AquireContext(m_hRenderer);

			NXT_ThemeRenderer_GLDraw(m_hRenderer, NXT_RendererOutputType_RGBA_8888, 1);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame Begin Swap(%d)", __LINE__, m_uiClipID, iCheckCTS);
			NXT_ThemeRenderer_SwapBuffers(m_hRenderer);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame end Swap", __LINE__, m_uiClipID);
			
			NXT_ThemeRenderer_ReleaseContext(m_hRenderer, 0);

			eRet = writeOneFrameWithEncode2(TRUE, iCheckCTS);
			if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
			{
				if( eRet == NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_WRITER_UNKNOWN_ERROR )
				{
					m_bPFrameEncodeDone = TRUE;
					bVideoEnd = TRUE;
					continue;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeOneFrameWithEncode failed(%d)", __LINE__, m_uiClipID, eRet);
				return eRet;
			}
			continue;
		}

		if( m_isNextVideoFrameRead )
		{
			bVideoEndOfContent = FALSE;
			int iReaderRet = m_pSource->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				case _SRC_GETFRAME_END:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame End", __LINE__, m_uiClipID);
					bVideoEndOfContent = TRUE;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) getVideoFrame error(%d) so End clip", __LINE__, m_uiClipID, iReaderRet);
					bVideoEndOfContent = TRUE;
					break;
			};

			m_isNextVideoFrameRead = FALSE;
		}

		if( bVideoEndOfContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
		}
		else
		{
			m_pSource->getVideoFrameDTS( &uiVideoDTS, TRUE );
			m_pSource->getVideoFramePTS( &uiVideoPTS, TRUE );
			m_pSource->getVideoFrameData( &pVideoFrame, &uiVideoFrameSize );

			int iVideoUITime = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);
			if(iVideoUITime > pCurRenderInfo->mEndTime)
			{
				pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiVideoPTS, TRUE);
				if(pRenderInfo != NULL && pRenderInfo != pCurRenderInfo)
				{
					pPrevRenderInfo = pCurRenderInfo;
					pCurRenderInfo = pRenderInfo;
					uiUpdatedTime = uiVideoPTS;

					if(m_pClipItem->getSlowMotion())
					{
						if(pCurRenderInfo->m_iSpeedCtlFactor == 3)
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/32;
						else if(pCurRenderInfo->m_iSpeedCtlFactor == 6)
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/16;
						else if(pCurRenderInfo->m_iSpeedCtlFactor == 13)
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/8;
						else
							iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30*pCurRenderInfo->m_iSpeedCtlFactor/100;
						isFindIDRFrame = TRUE;
					}
				}
			}
            
		}

		bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pVideoFrame, uiVideoFrameSize, (void*)&m_iVideoNalHeaderSize, m_pSource->getFrameFormat(), TRUE);
		if( isFindIDRFrame )
		{
			if( bIDRFrame == FALSE )
			{
				m_isNextVideoFrameRead = TRUE;
				continue;
			}
			isFindIDRFrame = FALSE;
		}

		if(bIDRFrame)
			iSkipCountForSlow = 0;

		if(m_pClipItem->getSlowMotion() && iSkipLevelForSlow > 1 && iSkipCountForSlow%iSkipLevelForSlow != 0)
		{
			iSkipCountForSlow++;
			continue;
		}
		iSkipCountForSlow++;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) IDR Frame(%d) Time(%d)", __LINE__, m_uiClipID, bIDRFrame, uiVideoDTS);

		if( bIDRFrame )
		{
			int iCurTime = (int)m_TimeCalc.applySpeed(m_uiBaseTime, m_uiStartTrimTime, m_iSpeedFactor, uiVideoPTS);

			if( (m_uiNextIDRTime == m_uiDstTime || m_uiNextIDRTime == 0) && iCurTime > 0 && iCurTime >= m_uiBaseTime )
			{
				m_uiNextIDRTime = uiVideoDTS;
				m_uiDstTime = (int)m_TimeCalc.applySpeed(m_uiBaseTime, m_uiStartTrimTime, m_iSpeedFactor, uiVideoPTS);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) needEncodeTime(%d) Dst Time(%d) curTime(%d)", 
					__LINE__, m_uiClipID, m_uiNextIDRTime, m_uiDstTime, iCurTime);
			}
		}

		uiVideoLastDTS = uiVideoDTS;

		uiCheckDecTime = nexSAL_GetTickCount();
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
									pVideoFrame,
									uiVideoFrameSize,
									NULL,
									uiVideoDTS,
									uiVideoPTS,
									uiEnhancement,
									&uiVideoDecoderErrRet );

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) DecFrame End(%p, %d) DTS(%d) PTS(%d)uiRet(%d) time(%d)",
			__LINE__, m_uiClipID, pVideoFrame, uiVideoFrameSize, uiVideoDTS, uiVideoPTS, uiVideoDecoderErrRet, nexSAL_GetTickCount() - uiCheckDecTime);

		if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			uiVideoDecOutCTS = 0;
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				m_isNextVideoFrameRead = TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Receive decoder EOS", __LINE__, m_uiClipID);
				bFinishedVideoDec = TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char* pY = NULL;
				NXBOOL bUsePrevInfo = FALSE;

				if( m_uiCheckVideoDecInit != 0 )
				{
					m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) VideoDecoder_First output( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
					m_uiCheckVideoDecInit = 0;
				}
				
				if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiVideoDecOutCTS) != 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed",__LINE__, m_uiClipID);
					continue;
				}

				if(uiUpdatedTime > uiVideoDecOutCTS)
					bUsePrevInfo = TRUE;

				if(bUsePrevInfo)
					iCheckCTS = (int)m_TimeCalc.applySpeed(pPrevRenderInfo->mStartTime, pPrevRenderInfo->mStartTrimTime, pPrevRenderInfo->m_iSpeedCtlFactor, uiVideoDecOutCTS);
				else
	    				iCheckCTS = (int)m_TimeCalc.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoDecOutCTS);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Dec GetOutput *m_pY=0x%08x outTime(%d) realTime(%d) Base(%d)",
					__LINE__, m_uiClipID, pY, uiVideoDecOutCTS, iCheckCTS, m_uiBaseTime);

				// For KMSA - 347 issue.
				if( m_bFirstOutputDrop )
				{
					m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
					// NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hRenderer, (ANativeWindow*)m_pDecSurface, -1);

					/* for KMSA - 363 issue.
					NXT_ThemeRenderer_AquireContext(m_hRenderer);
					NXT_ThemeRenderer_DiscardSurfaceTexture(m_hRenderer, (ANativeWindow*)m_pDecSurface);					
					NXT_ThemeRenderer_ReleaseContext(m_hRenderer, FALSE);
					*/
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) m_bFirstOutputDrop(%d)",__LINE__, m_uiClipID, m_bFirstOutputDrop);
					m_bFirstOutputDrop = FALSE;
					continue;
				}

				if( iCheckCTS < 0 || iCheckCTS < m_uiBaseTime )
				{
					m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
					// NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hRenderer, (ANativeWindow*)m_pDecSurface, -1);
					continue;
				}
				NXT_ThemeRenderer_AquireContext(m_hRenderer);

				m_pCodecWrap->postFrameRenderCallback(pY, TRUE);
				NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hRenderer, (ANativeWindow*)m_pDecSurface, -1);

				NXT_ThemeRenderer_SetSurfaceTexture(m_hRenderer,
													0, //track_id ..... there's no need for specifying it in this situation
													NXT_TextureID_Video_1,
													m_iSrcWidth,
													m_iSrcHeight,
													m_iSrcPitch,
													m_iSrcHeight,
													(ANativeWindow *)m_pDecSurface,
													0,   //convert_rgb_flag
													0,   //tex_id_for_lut
													0,   //tex_id_for_customlut_a
													0,   //tex_id_for_customlut_b
													0   //tex_id_for_customlut_power
				);

				NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hRenderer, NXT_TextureID_Video_1, 0, 0);

				NXT_ThemeRenderer_GLDraw(m_hRenderer, NXT_RendererOutputType_RGBA_8888, 1);
				m_fnSetMediaCodecTimeStamp(iCheckCTS, (void*)m_uiMediaCodecUserData); //yoon

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame Begin Swap(%d)", __LINE__, m_uiClipID, iCheckCTS);
				NXT_ThemeRenderer_SwapBuffers(m_hRenderer);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame end Swap", __LINE__, m_uiClipID);
				
				NXT_ThemeRenderer_ReleaseContext(m_hRenderer, 0);

				NXBOOL bEnd = FALSE;
				if( ( m_uiNextIDRTime > 0 && iCheckCTS > (m_uiDstTime + 500) ) || bFinishedVideoDec )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) PFrame encode End and encode with EOS", __LINE__, m_uiClipID);
					bEnd = TRUE;
				}

				eRet = writeOneFrameWithEncode2(bEnd, iCheckCTS);
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					if( eRet == NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_WRITER_UNKNOWN_ERROR )
					{
						m_bPFrameEncodeDone = TRUE;
						bVideoEnd = TRUE;
						continue;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeOneFrameWithEncode failed(%d)", __LINE__, m_uiClipID, eRet);
					return eRet;
				}
			}
		}
		else
		{
			NXBOOL bNeedMoreDecode = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoding Failed(%d)",__LINE__, m_uiClipID, uiVideoDecoderErrRet);
			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Decode Frame Failed and next frame", __LINE__, m_uiClipID);
				m_isNextVideoFrameRead = TRUE;
				bNeedMoreDecode = TRUE;
			}

			if(NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Decode Frame Failed (Output timeout)", __LINE__, m_uiClipID);
				bNeedMoreDecode = TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiVideoDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Receive decoder EOS", __LINE__);
				bFinishedVideoDec = TRUE;
			}

			if( bNeedMoreDecode )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoding Failed but try decode again",__LINE__, m_uiClipID);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Decoding Failed and exit ",__LINE__, m_uiClipID);
				bDecodeFailed = TRUE;
			}
		}

	}

	if(m_pFileWriter->getDECurrentVideoClipID() == m_uiClipID)
	{
		unsigned int uiDuration = 0;
		unsigned int uiSize = 0;
		m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, iCheckCTS, uiVideoLastDTS, NULL, 0, &uiDuration, &uiSize);
	}

	deinitVideoDecoder();
	deinitRenderer();
	deinitVideoEncoder();
	m_bDirectExportEnd = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) processPFrameAndAudio Ended!!", __LINE__, m_uiClipID);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_PFrameProcessTask::prodessEncoderDSI()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "prodessEncoderDSI Start!!!", __LINE__);

	if( m_pSource == NULL )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEREADER_INIT_ERROR;

	if( m_pSource->isVideoExist() == FALSE )
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_FILEREADER_INIT_ERROR;
	
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		m_hRenderer = (NXT_HThemeRenderer)pEditor->getExportRenderer();
		m_pOutputSurface = pEditor->getExportOutputSurface();
		SAFE_RELEASE(pEditor)
	}

	if( m_hRenderer == NULL || m_pOutputSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) prodessEncoderDSI Can't use export renderer and output surface", __LINE__, m_uiClipID);
		return NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_RENDER_INIT_ERROR;
	}

	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	eRet = initVideoEncoder();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) prodessEncoderDSI init video encoder failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}

	eRet = initRenderer();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) prodessEncoderDSI init renderer failed(%d)", __LINE__, m_uiClipID, eRet);
		return eRet;
	}

	unsigned char* pTextureBuffer = (unsigned char*)nexSAL_MemAlloc(16*9*3/2);
	if( pTextureBuffer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) prodessEncoderDSI texture clear buffer alloc failed(%d)", __LINE__, m_uiClipID);
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	memset(pTextureBuffer, 0x00, 16*9);
	memset(pTextureBuffer+(16*9), 0x80, 16*9/2);

	unsigned int uiCheckTime = nexSAL_GetTickCount();
	int iTime = 0;
	while ( m_bIsWorking && (nexSAL_GetTickCount() - uiCheckTime) < 1000 )
	{
		NXT_ThemeRenderer_AquireContext(m_hRenderer);

		NXT_ThemeRenderer_UploadTexture(	m_hRenderer,
											0,
											NXT_TextureID_Video_1,
											16,
											9,
											16,
											NXT_PixelFormat_NV12,
											pTextureBuffer,
											pTextureBuffer+(16*9),
											NULL,
											0, //lut
											0, //custom_lut_a
											0, //custom_lut_b
											0, //custom_lut_power
											NULL); //render_info

		NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )m_hRenderer, NXT_TextureID_Video_1, 0, 0);

		NXT_ThemeRenderer_GLDraw(m_hRenderer, NXT_RendererOutputType_RGBA_8888, 1);
		m_fnSetMediaCodecTimeStamp(iTime, (void*)m_uiMediaCodecUserData); //yoon

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame Begin Swap(%d)", __LINE__, m_uiClipID, iTime);
		NXT_ThemeRenderer_SwapBuffers(m_hRenderer);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) writeVideoFrame end Swap", __LINE__, m_uiClipID);
		
		NXT_ThemeRenderer_ReleaseContext(m_hRenderer, 0);

		unsigned char		pTempBuffer[50];
		unsigned int 		uiVEncRet		= 0;
		unsigned int		uiRet			= 0;
		NXBOOL			bNextFrame		= FALSE;
		NXBOOL			bEOS			= FALSE;

		uiRet = nexCAL_VideoEncoderEncode(m_hVideoEnc, pTempBuffer, 0, 0, iTime, &uiVEncRet);
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc Frame success Frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pTempBuffer, iTime, uiVEncRet);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc Frame failed Frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pTempBuffer, iTime, uiVEncRet);
			eRet = NEXVIDEOEDITOR_ERROR_DIRECTEXPORT_ENC_ENCODE_ERROR;
			break;
		}
		
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_NEXT_INPUT) )
		{
			iTime += 33;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc next frame(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pTempBuffer, iTime, uiVEncRet);
		}
		
		if(NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_EOS))
		{
			bEOS = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Encode is finished.(EOS) result(%d %d)", __LINE__, m_uiClipID, uiRet, uiVEncRet);
		}
		
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_TIMEOUT) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc TimeOut(%p) CTS(%d) result(%d)", 
				__LINE__, m_uiClipID, pTempBuffer, iTime, uiVEncRet);
		}
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
		{
			unsigned char* pBitStream = NULL;
			int uiBitStreamLen = 0;
			unsigned int uiVEncPTS = 0;
			
			uiRet = nexCAL_VideoEncoderGetOutput(m_hVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
			if( uiRet != 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc getoutput Failed(%d %d)", __LINE__, m_uiClipID, uiRet, uiVEncRet);
				continue;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[PFrameProcessTask.cpp %d] ID(%d) Video Enc getoutput sucessed pFrame(%p) Size(%d) CTS(%d)", 
				__LINE__, m_uiClipID, pBitStream, uiBitStreamLen, uiVEncPTS);

			if( m_pEncoderDSI == NULL && m_iEncoderDSISize == 0 )
			{
				m_pEncoderDSI = (unsigned char*)nexSAL_MemAlloc(uiBitStreamLen);
				if( m_pEncoderDSI == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) prodessEncoderDSI buffer alloc failed(%d)", __LINE__, m_uiClipID);
					eRet = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
					break;
				}
				memcpy(m_pEncoderDSI, pBitStream, uiBitStreamLen);
				m_iEncoderDSISize = uiBitStreamLen;
				eRet = NEXVIDEOEDITOR_ERROR_NONE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) prodessEncoderDSI end while", __LINE__, m_uiClipID);
				break;
			}
		}
	}

	nexSAL_MemFree(pTextureBuffer);

	deinitRenderer();
	deinitVideoEncoder();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PFrameProcessTask.cpp %d] ID(%d) prodessEncoderDSI!!", __LINE__, m_uiClipID);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2012/09/19	Draft.
-----------------------------------------------------------------------------*/
