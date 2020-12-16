/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_FastPreviewTask.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2015/07/22	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_FastPreviewTask.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"
#include "NEXVIDEOEDITOR_Util.h"

CNEXThread_FastPreviewTask::CNEXThread_FastPreviewTask( void ) : 
	m_perforDecode((char*)"FastPreviewTask Decode"),
	m_perforGetOutput((char*)"FastPreviewTask GetOutPut")
{
	m_pProjectMng		= NULL;
	m_pCliplist			= NULL;
	m_pVideoRenderer	= NULL;
	m_pSource			= NULL;

	m_pCodecWrap		= NULL;
	m_pDecSurface		= NULL;

	m_isInitEnd			= FALSE;

	m_iSrcWidth			= 0;
	m_iSrcHeight		= 0;
	m_iSrcPitch			= 0;

	m_uiStartTime		= 0;
	m_uiEndTime			= 0;

	m_iVideoBitrate		= 0;
	m_iVideoFramerate	= 0;
	m_iVideoNalHeaderSize	= 0;
 	
	m_isNextVideoFrameRead			= FALSE;
	m_isSupportFrameTimeChecker		= FALSE;

	m_uiVideoBaseTime	= 0;
	m_uiVideoStartTrim	= 0;

	m_iDecOutRealTime	= 0;
	m_uiDecOutCTS		= 0;

	m_iFrameNALHeaderSize	= 0;
	m_iFPS					= 0;

// add in VideoTask
	m_uiClipID			= INVALID_CLIP_ID;
	m_iSpeedFactor		= 100;
	m_FrameTimeChecker.clear();
	m_pVideoTrack		= NULL;
	m_uiEffectStartTime	= 0;

    m_uiFrameStartTime  = 0;
    m_uiFrameEndTime    = 0;

	m_uiEndDecTime		= 0;
	m_isIDRFrame		= TRUE;
	m_isDecodeMore		= FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "CNEXThread_FastPreviewTask Create Done" );
}

CNEXThread_FastPreviewTask::~CNEXThread_FastPreviewTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "~CNEXThread_FastPreviewTask Destroy Done" );
	deregistTrack();

	if( m_pVideoRenderer )
		m_pVideoRenderer->SendSimpleCommand(MESSAGE_FAST_PREVIEW_RENDER_DEINIT);

	SAFE_RELEASE(m_pCodecWrap);
	SAFE_RELEASE(m_pProjectMng);
	SAFE_RELEASE(m_pVideoRenderer);
	SAFE_RELEASE(m_pCliplist);
	SAFE_RELEASE(m_pSource);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "~CNEXThread_FastPreviewTask Destroy Done" );
}

NEXVIDEOEDITOR_ERROR CNEXThread_FastPreviewTask::initVideoDecoder()
{
	if( m_pSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] initVideoDecoder failed because reader was not set", __LINE__);
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}

	if( m_pVideoRenderer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] initVideoDecoder failed because renderer was not set", __LINE__);
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}
	
	CAutoLock m(CNexCodecManager::getVideoDecInitLock());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] initVideoDecoder() In", __LINE__);

	deinitVideoDecoder();

	m_pCodecWrap = new CNexCodecWrap();
	if( m_pCodecWrap == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Allocate CodecWrap failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
	}

	while( m_bIsWorking )
	{
		if( CNexCodecManager::isHardwareDecodeAvailable() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d]Now available hardware codec", __LINE__);
			break;
		}
		nexSAL_TaskSleep(30);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Wait available hardware codec", __LINE__);
	}

	if( m_bIsWorking == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d]Task exit before video decoder init", __LINE__);
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
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Get Codec Failed(0x%x)", __LINE__, m_pSource->getVideoObjectType());
		return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Get Codec end(%p)", __LINE__, m_pSource->getVideoObjectType(), m_pCodecWrap->getCodecHandle());

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
	NXUINT8*	pEnhancedDSI			= NULL;
	NXUINT32	uiEnhancedDSISize		= 0;

	NXINT64			isUseIframeVDecInit			= FALSE;
	unsigned int	eRet 						= 0;
	int 			iDSINALHeaderSize			= 0;

	unsigned int	uiUserDataType				= 0;

	unsigned int	iWidth						= 0;
	unsigned int	iHeight						= 0;

	NXINT64 uiCALDisplayFlagCallBackUserData	= 0;

	IRectangle*	pRec							= NULL;
	
	float* pMatrix = NULL;
	int iMatrixSize = 0;

	CClipVideoRenderInfo videoInfo;
	//memset(&videoInfo, 0x00, sizeof(CClipVideoRenderInfo));
	
	videoInfo.mFaceDetectProcessed = 1;
	
	if( nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	iDSINALHeaderSize		= m_pSource->getDSINALHeaderLength();
	m_iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();
	m_iFPS					= m_pSource->getVideoFrameRate();

	if( m_pCodecWrap->isHardwareCodec() )
	{
		if( m_pDecSurface == NULL )
		{
			m_pDecSurface = m_pVideoRenderer->getSurfaceTexture();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] get SurfaceTexture for fast preivew(%p) from renderer", __LINE__, m_pDecSurface);
			if( m_pDecSurface == NULL )
			{
				SAFE_RELEASE(m_pCodecWrap);
				return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_SURFACE_ERROR;
			}
		}

		m_pCodecWrap->setSurfaceTexture(m_pDecSurface);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Use surfaceTexture(%p)", __LINE__, m_pDecSurface);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Can't use PFrame re-encode with SoftwareCodec", __LINE__);
		SAFE_RELEASE(m_pCodecWrap);
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;
	}		

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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] This Video frame is invalid", __LINE__);
				nexSAL_TaskSleep(20);
				continue;
			}

			isIntraFrame = NexCodecUtil_IsSeekableFrame((NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&m_iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] Check I-frame End CTS(%u) Nal Size(%d)", __LINE__, uiDTS, iDSINALHeaderSize);
			if( isIntraFrame )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] I-frame searched...CTS(%u)", __LINE__, uiDTS);
				break;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] P-frame searched...CTS(%u)", __LINE__, uiDTS);
			}
		}
		else if( uiRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TaskSleep(20);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Need buffering Not normal condition(0x%x)", __LINE__, uiRet);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d]  Get Frame fail while decoder init(0x%x)", __LINE__, uiRet);

			deinitVideoDecoder();
			return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
		}
	}

	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoder Inint", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Dec init(%p %p %d)",
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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp  %d] Video Codec initialize Error(%d)", __LINE__, eRet);
				deinitVideoDecoder();
				return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch);
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iSrcWidth		= iWidth;
			m_iSrcHeight	= iHeight;
			m_iSrcPitch		= 0;


			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoder Init", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Dec init %p %d",
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
													m_iFrameNALHeaderSize,
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
													m_iFrameNALHeaderSize,
													&m_iSrcWidth,
													&m_iSrcHeight,
													&m_iSrcPitch,
													m_pSource->getVideoFrameRate(),
													NEXCAL_VDEC_MODE_NONE);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				deinitVideoDecoder();
				return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] VideoDecoder_Initialized() : W[%d], H[%d], P[%d]", __LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch);
			break;

		default:
			deinitVideoDecoder();
			return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
	};

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	if( m_pCodecWrap->getFormat() != NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] Not support ColorFormat(%p)", __LINE__, m_pCodecWrap->getFormat());
		deinitVideoDecoder();
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
	}

	m_pVideoTrack = new CVideoTrackInfo;
	if( m_pVideoTrack == NULL )
	{
		deinitVideoDecoder();
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
	}

	CClipItem* pCurrentItem = m_pCliplist->getVideoClipItem(m_uiStartTime, m_uiEndTime);


	videoInfo.muiTrackID		= pCurrentItem->getClipID();
	videoInfo.mClipType         = pCurrentItem->getClipType();

	videoInfo.mStartTime		= pCurrentItem->getStartTime();
	videoInfo.mEndTime          = pCurrentItem->getEndTime();

	videoInfo.mWidth			= m_iSrcWidth;
	videoInfo.mHeight			= m_iSrcHeight;
	videoInfo.mPitch			= m_iSrcPitch;
	videoInfo.mColorFormat      = m_pCodecWrap->getFormat();
	videoInfo.mIsPreview		= FALSE;
	videoInfo.mRotateState		= pCurrentItem->getRotateState();
	
	videoInfo.mBrightness		= pCurrentItem->getBrightness();
	videoInfo.mContrast         = pCurrentItem->getContrast();
	videoInfo.mSaturation		= pCurrentItem->getSaturation();
	videoInfo.mTintcolor		= pCurrentItem->getTintcolor();
	videoInfo.mHue				= pCurrentItem->getHue();


	pRec =  pCurrentItem->getStartPosition();
	if( pRec != NULL )
	{
		videoInfo.mStart.left		= pRec->getLeft();
		videoInfo.mStart.top		= pRec->getTop();
		videoInfo.mStart.right		= pRec->getRight();
		videoInfo.mStart.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pRec =  pCurrentItem->getEndPosition();
	if( pRec != NULL )
	{
		videoInfo.mEnd.left			= pRec->getLeft();
		videoInfo.mEnd.top			= pRec->getTop();
		videoInfo.mEnd.right		= pRec->getRight();
		videoInfo.mEnd.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pMatrix = pCurrentItem->getStartMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mStartMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}

	pMatrix = pCurrentItem->getEndMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mEndMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}	
	
	if( m_pVideoTrack->setClipRenderInfo(&videoInfo) == FALSE )
	{
		deinitVideoDecoder();
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
	}

	if( m_pCodecWrap->isHardwareCodec() &&
		m_pDecSurface &&
		m_pVideoTrack->setSurfaceTexture(m_pDecSurface) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] Regist surfacetexture failed(%p)", __LINE__, m_pDecSurface);

		deinitVideoDecoder();
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
	}

	m_pVideoTrack->setTrackIndexInfo(pCurrentItem->getClipIndex(), pCurrentItem->getClipTotalCount());

	m_pVideoTrack->setCodecWrap(m_pCodecWrap);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] setEffect(%p, %d)\n", __LINE__,  pCurrentItem, pCurrentItem->getEndTime());
	if( pCurrentItem )
	{
		if( pCurrentItem->getClipEffectEnable() )
		{
			char* pEffect = (char*)pCurrentItem->getClipEffectID();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] setEffect(%s)\n", __LINE__, pEffect);
			if( pEffect )
			{
				m_uiEffectStartTime = pCurrentItem->getEndTime() - (pCurrentItem->getClipEffectDuration() * pCurrentItem->getClipEffectOffset() / 100);
				m_pVideoTrack->setEffectInfo(m_uiEffectStartTime, 
											pCurrentItem->getClipEffectDuration(), 
											pCurrentItem->getClipEffectOffset(), 
											pCurrentItem->getClipEffectOverlap(),
											pEffect);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] setEffect(%p)\n", __LINE__, pEffect);
			}
		}

		// set title for Effect option, mantis 5524
		char* pTitleEffect = (char*) pCurrentItem->getTitleEffectID();
		m_pVideoTrack->setTitleInfo(pCurrentItem->getTitle(),
									pTitleEffect,
									pCurrentItem->getTitleStartTime(),
									pCurrentItem->getTitleEndTime());

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] setTitleInfo(%s, %d %d)", 
			__LINE__, pTitleEffect, pCurrentItem->getTitleStartTime(), pCurrentItem->getTitleEndTime());
	}
	
	if( registTrack() == FALSE )
	{
		deinitVideoDecoder();
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_DEC_INIT_ERROR;	
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ColorFormat(0x%x) HeaderSize(%d)", __LINE__, m_pCodecWrap->getFormat(), iDSINALHeaderSize);
	m_isNextVideoFrameRead = FALSE;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNEXThread_FastPreviewTask::registTrack()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] registTrack() In",__LINE__);	

	if( m_pVideoRenderer && m_pVideoTrack )
	{
		CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pVideoTrack);
		if( pMsg )
		{
			m_pVideoRenderer->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] registTrack() Out",__LINE__);	
			return TRUE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] registTrack() Out",__LINE__);	
	return FALSE;
}

NXBOOL CNEXThread_FastPreviewTask::deregistTrack()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] deregistTrack() In",__LINE__);

	if( m_pVideoRenderer && m_pVideoTrack )
	{
		CNxMsgDeleteTrack* pMsg = new CNxMsgDeleteTrack(m_pVideoTrack);
		if( pMsg )
		{
			m_pVideoRenderer->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] deregistTrack() Out",__LINE__);
			return TRUE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] deregistTrack() Out",__LINE__);
	return FALSE;
}

NEXVIDEOEDITOR_ERROR CNEXThread_FastPreviewTask::deinitVideoDecoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] deinitVideoDecoder In");

	deregistTrack();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] deinitVideoDecoder deregist track");

	if( m_pCodecWrap != NULL )
	{
		m_pCodecWrap->deinitDecoder();
		SAFE_RELEASE(m_pCodecWrap);
	}

	if( m_pVideoRenderer != NULL )
	{
		if( m_pDecSurface != NULL )
		{
			m_pVideoRenderer->releaseSurfaceTexture(m_pDecSurface);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] releaseSurfaceTexture (%p, %p)", __LINE__, m_pVideoRenderer, m_pDecSurface);
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] deinitVideoDecoder Out", __LINE__);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_FastPreviewTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[ProjectManager.cpp %d] ---------- PM ProcessCommandMsg(%d)", __LINE__, pMsg->m_nMsgType);
	switch( pMsg->m_nMsgType )
	{
		case MESSAGE_FAST_PREVIEW_START:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] MESSAGE_FAST_PREVIEW_START", __LINE__);
			CNxMsgFastPreview* pFastMsg = (CNxMsgFastPreview*)pMsg;

			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor )
			{
				m_isSupportFrameTimeChecker = pEditor->m_bSupportFrameTimeChecker;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] bSupportFrameTimeChecker (%d)", __LINE__, m_isSupportFrameTimeChecker);
				SAFE_RELEASE(pEditor);
			}

			m_uiStartTime = pFastMsg->m_uiStartTime;
			m_uiEndTime = pFastMsg->m_uiEndTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] uiStartTime: %d uiEndTime: %d", __LINE__, m_uiStartTime, m_uiEndTime);

			int iRet = FastPreview(pFastMsg->m_iWidth, pFastMsg->m_iHeight);

			pFastMsg->m_nMsgType = MESSAGE_FAST_PREVIEW_START_DONE;
			pFastMsg->m_nResult = iRet;
            pFastMsg->m_uiStartTime = m_uiFrameStartTime;
            pFastMsg->m_uiEndTime = m_uiFrameEndTime;

			if( m_pProjectMng )
			{
				m_pProjectMng->SendCommand(pFastMsg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] MESSAGE_FAST_PREVIEW_START End(%d)", __LINE__, iRet);
			}

			// m_vecFrameTime.clear();
			
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] MESSAGE_FAST_PREVIEW_START End(%d)", __LINE__, iRet);
			SAFE_RELEASE(pFastMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_FAST_PREVIEW_TIME:
		{
            CNxMsgFastPreview* pFastMsg = (CNxMsgFastPreview*)pMsg;
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] MESSAGE_FAST_PREVIEW_TIME In(%d)", __LINE__, pFastMsg->m_uiTime);

			if( pFastMsg->m_uiTime > m_uiFrameEndTime - 300 ) {
				FastPreviewTime(m_uiFrameEndTime + 100);
			}
			else if( pFastMsg->m_uiTime < m_uiFrameStartTime + 300) {
				FastPreviewTime(m_uiFrameStartTime - 1);
			}

            if( m_isInitEnd && m_pVideoRenderer )
            {
                m_pVideoRenderer->SendCommand(pFastMsg);
            }
            else
            {
                pFastMsg->m_nMsgType = MESSAGE_FAST_PREVIEW_TIME_DONE;
                pFastMsg->m_nResult = NEXVIDEOEDITOR_ERROR_RENDERER_INIT;

                m_pProjectMng->SendCommand(pFastMsg);
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] MESSAGE_FAST_PREVIEW_TIME End", __LINE__);
            }

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Call SetTime(%d)", __LINE__, pFastMsg->m_uiTime);
            SAFE_RELEASE(pFastMsg);
			return MESSAGE_PROCESS_OK;
		}
		
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);
}

NXBOOL CNEXThread_FastPreviewTask::setProjectManager(CNexProjectManager* pMng)
{
	SAFE_RELEASE(m_pProjectMng);
	m_pProjectMng = pMng;
	SAFE_ADDREF(m_pProjectMng);
	return TRUE;
}
	
NXBOOL CNEXThread_FastPreviewTask::setRenderer(CNEXThread_VideoRenderTask* pVideo)
{
	SAFE_ADDREF(pVideo);
	SAFE_RELEASE(m_pVideoRenderer);
	m_pVideoRenderer = pVideo;
	return TRUE;
}


NXBOOL CNEXThread_FastPreviewTask::setClipList(CClipList* pCliplist)
{
	SAFE_RELEASE(m_pCliplist);
	m_pCliplist = pCliplist;
	SAFE_ADDREF(m_pCliplist);
	return TRUE;
}

int CNEXThread_FastPreviewTask::FastPreview(int iDisplayWidth, int iDisplayHeight)
{
	NEXVIDEOEDITOR_ERROR	eRet				= NEXVIDEOEDITOR_ERROR_NONE;

	unsigned int			uiDTS				= 0;
	unsigned int			uiPTS				= 0;
	unsigned char*			pFrame				= NULL;
	unsigned int			uiFrameSize			= 0;
	unsigned int			uiDecoderErrRet		= 0;
	unsigned int			uiEnhancement		= NEXCAL_VDEC_FLAG_NONE;

	unsigned int			uiSeekResult		= 0;
	NXBOOL					isIDRFrame			= TRUE;
	NXBOOL					bIDRFrame			= FALSE;

	NXBOOL					bEndContent			= FALSE;

	unsigned int			uiCheckDecTime		= 0;

	if( m_pVideoRenderer == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] FastPreview video renderer error(%d)", __LINE__, m_pVideoRenderer);
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_VIDEO_RENDERER_ERROR;
	}

	if( m_pCliplist == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] FastPreview clip list error(%p)", __LINE__, m_pCliplist);
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_CLIPLIST_ERROR;
	}

	if( m_pCliplist->getClipCount() <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] FastPreview clip list size is 0(%d)", __LINE__, m_pCliplist->getClipCount());
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_CLIPLIST_ERROR;
	}

	CClipItem* pCurrentItem = m_pCliplist->getVideoClipItem(m_uiStartTime, m_uiEndTime);
	if( pCurrentItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] FastPreview clip find failed with time info(%d %d)", __LINE__, m_uiStartTime, m_uiEndTime);
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_FIND_CLIP_ERROR;
	}
	
	if( m_pSource == NULL )
	{
		m_pSource = pCurrentItem->getSource();
		if( m_pSource == NULL )
		{
			SAFE_RELEASE(pCurrentItem);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] FastPreview init reader of clip failed", __LINE__);
			return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_FILEREADER_INIT_ERROR;
		}

		if( m_pSource->isVideoExist() == FALSE )
            return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_FILEREADER_INIT_ERROR;
	}

	if( pCurrentItem )
	{
		m_uiClipID = pCurrentItem->getClipID();

		m_uiVideoBaseTime = pCurrentItem->getStartTime();
		m_uiVideoStartTrim = pCurrentItem->getStartTrimTime();
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] pCurrentItem is NULL", __LINE__);
		return NEXVIDEOEDITOR_ERROR_FASTPREVIEW_CLIPLIST_ERROR;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] Video Task Start ClipID(%d) StartTime(%d) StartTrim(%d) Speed(%d)", 
		__LINE__, m_uiClipID, m_uiStartTime, m_uiVideoStartTrim, m_iSpeedFactor);
	
	if( initVideoDecoder() != NEXVIDEOEDITOR_ERROR_NONE )
	{
		CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, CNexProjectManager::getCurrentState(), NEXVIDEOEDITOR_ERROR_CODEC_INIT, m_uiClipID, 1);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ClipID(%d) Video Task init Decoder failed ", __LINE__, m_uiClipID);
		return 181818;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) START seek to(%d) -> seekResult(%d) ", __LINE__, m_uiClipID, m_uiStartTime, uiSeekResult);
	int iSeekReturn = m_pSource->seekToVideo(m_uiStartTime, &uiSeekResult, NXFF_RA_MODE_PREV);
	if( iSeekReturn != NEXVIDEOEDITOR_ERROR_NONE)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] SEEK(%d) Fail Ret(%d) !!", __LINE__,  m_uiStartTime, iSeekReturn);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) END seek to(%d) -> seekResult(%d) ", __LINE__, m_uiClipID, m_uiStartTime, uiSeekResult);

	m_isNextVideoFrameRead = TRUE;

    CNxMsgFastPreview* pFastPreviewInit = new CNxMsgFastPreview;
    if( pFastPreviewInit )
    {
        pFastPreviewInit->m_nMsgType = MESSAGE_FAST_PREVIEW_RENDER_INIT;

        pFastPreviewInit->m_iWidth = iDisplayWidth == 0 ? m_iSrcWidth : iDisplayWidth;
        pFastPreviewInit->m_iHeight = iDisplayHeight == 0 ? m_iSrcHeight : iDisplayHeight;
        pFastPreviewInit->m_iMaxCount = FOR_FASTPREVIEW_MAX_BUFFER_COUNT;
        
        m_pVideoRenderer->SendCommand(pFastPreviewInit);
        SAFE_RELEASE(pFastPreviewInit);
    }
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Task init end and start ", __LINE__, m_uiClipID);
	m_perforDecode.CheckModuleStart();
	m_perforGetOutput.CheckModuleStart();

	// initialize Vector of FrameCTS
	m_FrameTimeChecker.clear();

	while ( m_bIsWorking )
	{
		if(m_iDecOutRealTime > m_uiEndTime)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] m_iDecOutRealTime(%d) > m_uiEndTime(%d). so break end loop", __LINE__, m_iDecOutRealTime, m_uiEndTime);
			break;
		}

		if( m_isNextVideoFrameRead )
		{
			int iReaderRet = m_pSource->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				case _SRC_GETFRAME_END:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] getVideoFrame End\n", __LINE__);
					bEndContent = TRUE;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
					bEndContent = TRUE;
					break;
			};
			
			m_isNextVideoFrameRead = FALSE;
		
		}

		if( bEndContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
		}
		else
		{
			m_pSource->getVideoFrameDTS(&uiDTS, TRUE);
			m_pSource->getVideoFramePTS(&uiPTS, TRUE);
			m_pSource->getVideoFrameData(&pFrame, &uiFrameSize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) read video frame time info (%d %d)", __LINE__, m_uiClipID, uiDTS, uiPTS);
		}

		if( m_pVideoTrack == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] m_pVideoTrack is null\n", __LINE__);
			break;
		}

		bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&m_iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
		if( isIDRFrame )
		{
			if( bIDRFrame == FALSE )
			{
				m_isNextVideoFrameRead = TRUE;
				continue;
			}

			isIDRFrame = FALSE;
		}
	
		uiCheckDecTime = nexSAL_GetTickCount();
		
		int uiCTS = (int)m_CalcTime.applySpeed(m_uiVideoBaseTime, m_uiVideoStartTrim, m_iSpeedFactor, uiPTS);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
		m_perforDecode.CheckModuleUnitStart();

		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
									pFrame,
									uiFrameSize,
									NULL,
									uiDTS,
									uiPTS,
									uiEnhancement, 
									&uiDecoderErrRet );

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) DecFrame End(%p, %d) DTS(%d) PTS(%d) uiRet(%d) IDR(%d) DecTime(%d)", 
		 	__LINE__, m_uiClipID, pFrame, uiFrameSize, uiDTS, uiPTS, uiDecoderErrRet, bIDRFrame, nexSAL_GetTickCount() - uiCheckDecTime);

		if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			m_uiDecOutCTS = 0;
			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] Decode Frame End and next frame", __LINE__);
				// whenever decoding frames, add PTS on the FrameCTSVector

				if( m_isSupportFrameTimeChecker )
					m_FrameTimeChecker.addFrameCTSVec(uiDTS, uiPTS, 1);
				m_isNextVideoFrameRead = TRUE;
				m_perforDecode.CheckModuleUnitEnd();
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char*	pY			= NULL;
				unsigned char*	pU			= NULL;
				unsigned char*	pV			= NULL;

				CFrameInfo*		pFrameInfo	= NULL;

				if( m_uiCheckVideoDecInit != 0 )
				{
					m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) VideoDecoder_First output( Time : %d )", __LINE__, 
						m_uiClipID, m_uiCheckVideoDecInit );
					m_uiCheckVideoDecInit = 0;
				}

				while( m_bIsWorking && pFrameInfo == NULL)
				{
					pFrameInfo = m_pVideoTrack->getFrameInBuffer();
					if( pFrameInfo == NULL )
					{
						if(m_iFPS * m_iSpeedFactor/100 > 60)
							nexSAL_TaskSleep(1);
						else
							nexSAL_TaskSleep(5);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] getFrame Input Buffer return null wait buffer", __LINE__);
						continue;
					}
				}
				

				if( pFrameInfo == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) getFrame Input Buffer return null wait buffer", __LINE__, m_uiClipID);
					// Case : mantis 9240
					// callVideoFrameRenderCallback(pY, FALSE);
					continue;
				}

				if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
				{
					m_perforGetOutput.CheckModuleUnitStart();
					if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &m_uiDecOutCTS) != 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed",__LINE__, m_uiClipID);
						continue;
					}
					m_perforGetOutput.CheckModuleUnitEnd();

					unsigned int uiNewPTS = 0;
					// Video split ±?�¥�?ªÁøÎ¡�?Decoder ø¿?�˘∑Œ ?�ﬂ?�°«�?
					if( m_isSupportFrameTimeChecker && m_FrameTimeChecker.getValidFrameCTSWithApplyMutiplePTS(m_uiDecOutCTS, &uiNewPTS, TRUE) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, m_uiClipID, uiDTS, m_uiDecOutCTS);
						m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
						continue;
					}
					
					if( CNexVideoEditor::m_bNexEditorSDK ) 
					{
						unsigned int uiRestoretime = m_CalcTime.restoreSpeed(m_iSpeedFactor, uiNewPTS);
						int iVideoT = (int)m_CalcTime.applySpeed(m_uiVideoBaseTime, m_uiVideoStartTrim, m_iSpeedFactor, uiRestoretime);

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput Time (outT:%d restoreT:%d realT:%d, base:%d trim:%d, spc:%d))", 
							__LINE__, m_uiClipID, uiNewPTS, uiRestoretime, iVideoT, m_uiVideoBaseTime, m_uiStartTime, m_iSpeedFactor);
						
						if( iVideoT < 0 )
						{
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
							continue;
						}

						m_iDecOutRealTime = iVideoT;
					}
					else
					{
						m_iDecOutRealTime = m_CalcTime.applySpeed(m_uiVideoBaseTime, m_uiVideoStartTrim, m_iSpeedFactor, uiNewPTS);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput Time (outT:%d realT:%d, base:%d trim:%d, spc:%d))", 
							__LINE__, m_uiClipID, uiNewPTS, m_iDecOutRealTime, m_uiVideoBaseTime, m_uiStartTime, m_iSpeedFactor);
						
					}
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput *m_pY=0x%08x PTS(%d %d) real PTS(%d)", __LINE__, m_uiClipID, pY, m_uiDecOutCTS, uiNewPTS, m_iDecOutRealTime);

					if( m_iDecOutRealTime < 0 || m_uiStartTime > m_iDecOutRealTime || m_iDecOutRealTime > m_uiEndTime)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Frame drop before or after time(Start:%d End:%d real frame:%d)",__LINE__,
							m_uiClipID, m_uiStartTime, m_uiEndTime, m_iDecOutRealTime);
						m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
						continue;
					}					

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] setVideoFrame m_iDecOutRealTime(%d) m_uiEndTime(%d)", __LINE__, m_iDecOutRealTime, m_uiEndTime);
					pFrameInfo->setVideoFrame(m_iDecOutRealTime, pY, NULL, NULL);
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] m_pVideoTrack->EnqueueFrameInBuffer: m_uiDecOutCTS(%d)", __LINE__, m_iDecOutRealTime);

				m_pVideoTrack->EnqueueFrameInBuffer();

				CNxMsgFastPreviewTime* pMsg = new CNxMsgFastPreviewTime();
				m_pVideoRenderer->SendCommand(pMsg);
				if( pMsg->waitProcessDone(500) ) {
					m_uiFrameStartTime = pMsg->m_uiStartTime;
					m_uiFrameEndTime = pMsg->m_uiEndTime;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] start(%d) end(%d)", __LINE__, m_uiFrameStartTime, m_uiFrameEndTime);
				}
				SAFE_RELEASE(pMsg)
			}
			else
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput did not exist", __LINE__, m_uiClipID);
				nexSAL_TaskSleep(1);
			}
		}
		else
		{
			NXBOOL bNeedMoreDecode = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoding Failed(%d)",__LINE__, m_uiClipID, uiDecoderErrRet);

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Decode Frame Failed and next frame", __LINE__, m_uiClipID);
				m_isNextVideoFrameRead = TRUE;
				bNeedMoreDecode = TRUE;
			}
			
			if(NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Decode Frame Failed (Output timeout)", __LINE__, m_uiClipID);
				bNeedMoreDecode = TRUE;
			}

			if( bNeedMoreDecode == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoding Failed and exit ",__LINE__, m_uiClipID);
				break;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoding Failed but try decode again",__LINE__, m_uiClipID);
			}
			
		}
	}

//    if( m_pVideoTrack )
//        m_pVideoTrack->setVideoDecodeEnd();

    int iWaitTime = 2000;
    while ( m_bIsWorking && iWaitTime > 0 && m_pVideoTrack != NULL )
    {
        if( m_pVideoTrack->isTrackEnded() )
        {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Frame Draw End",__LINE__, m_uiClipID);
            break;
        }
        nexSAL_TaskSleep(20);
        iWaitTime -= 20;
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Wait Video Frame Draw",__LINE__, m_uiClipID);
    }
//	m_pVideoTrack->setCodecWrap(NULL);
//
//	deinitVideoDecoder();

	m_isInitEnd = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Task End and Release Codec", __LINE__, m_uiClipID);
    
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_FastPreviewTask::FastPreviewTime(unsigned int uiTime)
{
	NEXVIDEOEDITOR_ERROR	eRet				= NEXVIDEOEDITOR_ERROR_NONE;

	unsigned int			uiDTS				= 0;
	unsigned int			uiPTS				= 0;
	unsigned char*			pFrame				= NULL;
	unsigned int			uiFrameSize			= 0;
	unsigned int			uiDecoderErrRet		= 0;
	unsigned int			uiEnhancement		= NEXCAL_VDEC_FLAG_NONE;

	NXBOOL					bEndContent			= FALSE;

	unsigned int			uiCheckDecTime		= 0;
	int						decodeCount			= 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) FastPreviewTime (cur:%d StartTime:%d EndTime:%d FrameStart:%d FrameEnd:%d)", __LINE__, m_uiClipID, uiTime, m_uiStartTime, m_uiEndTime, m_uiFrameStartTime, m_uiFrameEndTime);

	
	if( m_isDecodeMore ) {
		m_isIDRFrame = FALSE;
	}
	else {
		if( (uiTime == m_uiFrameStartTime - 1) && (m_uiFrameStartTime - m_uiVideoBaseTime < 20) ) {
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
		else if( uiTime > m_uiFrameEndTime ) {
			m_uiEndDecTime = uiTime;
		}
		else {
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) START seek to(%d)", __LINE__, m_uiClipID, uiTime);
			unsigned int uiSeekResult = 0;
			int iSeekReturn = m_pSource->seekToVideo(uiTime, &uiSeekResult, NXFF_RA_MODE_PREV);
			if( iSeekReturn != NEXVIDEOEDITOR_ERROR_NONE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] SEEK(%d) Fail Ret(%d) !!", __LINE__,  uiTime, iSeekReturn);
			}
			nexCAL_VideoDecoderReset(m_pCodecWrap->getCodecHandle());
			m_iDecOutRealTime = 0;
			m_isNextVideoFrameRead = TRUE;
			m_isIDRFrame = TRUE;
			m_uiEndDecTime = uiTime;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) END seek to(%d) -> seekResult(%d) ", __LINE__, m_uiClipID, m_uiStartTime, uiSeekResult);
		}
	}

	while ( m_bIsWorking )
	{
		if( m_iDecOutRealTime >= m_uiEndDecTime )
		{
			m_isDecodeMore = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] m_iDecOutRealTime(%d) > m_uiEndDecTime(%d). so break end loop", __LINE__, m_iDecOutRealTime, m_uiEndDecTime);
			break;
		}

		if( m_isDecodeMore && decodeCount > 3 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] continue dec next loop m_iDecOutRealTime(%d) > m_uiEndDecTime(%d)", __LINE__, m_iDecOutRealTime, m_uiEndDecTime);
			break;
		}

		if( m_isNextVideoFrameRead )
		{
			int iReaderRet = m_pSource->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				case _SRC_GETFRAME_END:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] getVideoFrame End\n", __LINE__);
					bEndContent = TRUE;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
					bEndContent = TRUE;
					break;
			};

			m_isNextVideoFrameRead = FALSE;

		}

		if( bEndContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
		}
		else
		{
			m_pSource->getVideoFrameDTS(&uiDTS, TRUE);
			m_pSource->getVideoFramePTS(&uiPTS, TRUE);
			m_pSource->getVideoFrameData(&pFrame, &uiFrameSize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) read video frame time info (%d %d)", __LINE__, m_uiClipID, uiDTS, uiPTS);
		}

		NXBOOL bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&m_iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
		if( m_isIDRFrame )
		{
			if( bIDRFrame == FALSE )
			{
				m_isNextVideoFrameRead = TRUE;
				continue;
			}

			m_isIDRFrame = FALSE;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) DecFrame(%p, %d) DTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiDTS);
		uiCheckDecTime = nexSAL_GetTickCount();
		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
								  pFrame,
								  uiFrameSize,
								  NULL,
								  uiDTS,
								  uiPTS,
								  uiEnhancement,
								  &uiDecoderErrRet );

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) DecFrame End(%p, %d) DTS(%d) PTS(%d) uiRet(%d) IDR(%d) DecTime(%d)",
						__LINE__, m_uiClipID, pFrame, uiFrameSize, uiDTS, uiPTS, uiDecoderErrRet, bIDRFrame, nexSAL_GetTickCount() - uiCheckDecTime);

		if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			m_uiDecOutCTS = 0;
			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] Decode Frame End and next frame", __LINE__);
				// whenever decoding frames, add PTS on the FrameCTSVector

				if( m_isSupportFrameTimeChecker )
					m_FrameTimeChecker.addFrameCTSVec(uiDTS, uiPTS, 1);
				m_isNextVideoFrameRead = TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char*	pY			= NULL;
				unsigned char*	pU			= NULL;
				unsigned char*	pV			= NULL;

				CFrameInfo*		pFrameInfo	= NULL;

				if( m_uiCheckVideoDecInit != 0 )
				{
					m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) VideoDecoder_First output( Time : %d )", __LINE__,
									m_uiClipID, m_uiCheckVideoDecInit );
					m_uiCheckVideoDecInit = 0;
				}

				while( m_bIsWorking && pFrameInfo == NULL)
				{
					pFrameInfo = m_pVideoTrack->getFrameInBuffer();
					if( pFrameInfo == NULL )
					{
						if(m_iFPS * m_iSpeedFactor/100 > 60)
							nexSAL_TaskSleep(1);
						else
							nexSAL_TaskSleep(5);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] getFrame Input Buffer return null wait buffer", __LINE__);
						continue;
					}
				}


				if( pFrameInfo == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) getFrame Input Buffer return null wait buffer", __LINE__, m_uiClipID);
					// Case : mantis 9240
					// callVideoFrameRenderCallback(pY, FALSE);
					continue;
				}

				if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
				{
					if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &m_uiDecOutCTS) != 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed",__LINE__, m_uiClipID);
						continue;
					}

					unsigned int uiNewPTS = 0;
					if( m_isSupportFrameTimeChecker && m_FrameTimeChecker.getValidFrameCTSWithApplyMutiplePTS(m_uiDecOutCTS, &uiNewPTS, TRUE) == FALSE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, m_uiClipID, uiDTS, m_uiDecOutCTS);
						m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
						continue;
					}

					if( CNexVideoEditor::m_bNexEditorSDK )
					{
						m_iDecOutRealTime = (int)m_CalcTime.applySpeed(m_uiVideoBaseTime, m_uiVideoStartTrim, m_iSpeedFactor, uiNewPTS);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput Time (outT:%d realT:%d, base:%d trim:%d, spc:%d))",
										__LINE__, m_uiClipID, uiNewPTS, m_iDecOutRealTime, m_uiVideoBaseTime, m_uiVideoStartTrim, m_iSpeedFactor);

					}
					else
					{
						m_iDecOutRealTime = m_CalcTime.applySpeed(m_uiVideoBaseTime, m_uiVideoStartTrim, m_iSpeedFactor, uiNewPTS);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput Time (outT:%d realT:%d, base:%d trim:%d, spc:%d))", __LINE__, m_uiClipID, uiNewPTS, m_iDecOutRealTime, m_uiVideoBaseTime, m_uiVideoStartTrim, m_iSpeedFactor);

					}

					if( m_iDecOutRealTime < 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Frame drop before or after time(Start:%d End:%d real frame:%d)",__LINE__, m_uiClipID, m_uiStartTime, m_uiEndTime, m_iDecOutRealTime);
						m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
						continue;
					}

					if( m_iDecOutRealTime >= m_uiEndDecTime || (m_uiEndDecTime >= m_uiFrameEndTime && m_iDecOutRealTime <= m_uiFrameEndTime) ) {
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Frame drop already has frame(DecOutRealTime:%d DecEndTime:%d FrameStart:%d FrameEnd:%d)",__LINE__, m_uiClipID, m_iDecOutRealTime, m_uiEndDecTime, m_uiFrameStartTime, m_uiFrameEndTime);
						m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
						continue;
					}

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] setVideoFrame m_iDecOutRealTime(%d) m_uiEndDecTime(%d)", __LINE__, m_iDecOutRealTime, m_uiEndDecTime);
					pFrameInfo->setVideoFrame(m_iDecOutRealTime, pY, NULL, NULL);
					decodeCount++;
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] m_pVideoTrack->EnqueueFrameInBuffer: m_uiDecOutCTS(%d)", __LINE__, m_iDecOutRealTime);

				m_pVideoTrack->EnqueueFrameInBuffer();

				CNxMsgFastPreviewTime* pMsg = new CNxMsgFastPreviewTime();
				m_pVideoRenderer->SendCommand(pMsg);
				if( pMsg->waitProcessDone(500) ) {
					m_uiFrameStartTime = pMsg->m_uiStartTime;
					m_uiFrameEndTime = pMsg->m_uiEndTime;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] start(%d) end(%d)", __LINE__, m_uiFrameStartTime, m_uiFrameEndTime);
				}
				SAFE_RELEASE(pMsg)
			}
			else
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) GetOutput did not exist", __LINE__, m_uiClipID);
				nexSAL_TaskSleep(1);
			}
		}
		else
		{
			NXBOOL bNeedMoreDecode = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoding Failed(%d)",__LINE__, m_uiClipID, uiDecoderErrRet);

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Decode Frame Failed and next frame", __LINE__, m_uiClipID);
				m_isNextVideoFrameRead = TRUE;
				bNeedMoreDecode = TRUE;
			}

			if(NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Decode Frame Failed (Output timeout)", __LINE__, m_uiClipID);
				bNeedMoreDecode = TRUE;
			}

			if( bNeedMoreDecode == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoding Failed and exit ",__LINE__, m_uiClipID);
				break;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Decoding Failed but try decode again",__LINE__, m_uiClipID);
			}

		}
	}

	//    if( m_pVideoTrack )
	//        m_pVideoTrack->setVideoDecodeEnd();

	int iWaitTime = 2000;
	while ( m_bIsWorking && iWaitTime > 0 && m_pVideoTrack != NULL )
	{
		if( m_pVideoTrack->isTrackEnded() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Frame Draw End",__LINE__, m_uiClipID);
			break;
		}
		nexSAL_TaskSleep(20);
		iWaitTime -= 20;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Wait Video Frame Draw",__LINE__, m_uiClipID);
	}
	// m_pVideoTrack->setCodecWrap(NULL);

	// deinitVideoDecoder();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[FastPreviewTask.cpp %d] ID(%d) Video Task End and Release Codec", __LINE__, m_uiClipID);

	return NEXVIDEOEDITOR_ERROR_NONE;
}


