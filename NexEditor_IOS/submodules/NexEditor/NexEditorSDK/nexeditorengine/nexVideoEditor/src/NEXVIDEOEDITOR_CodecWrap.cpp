/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_CodecWrap.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2015/10/13	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_CodecWrap.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_Util.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NexCAL.h"

CNexCodecWrap::CNexCodecWrap( void )
{
	m_bInitDecoder = FALSE;
	m_hCodecVideo = NULL;
	m_uiCodecUserData= 0;
	m_ColorFormat = 0;

	m_iWidth = 0;
	m_iHeight = 0;
	m_iPitch = 0;

	m_pSurfaceTexture = NULL;

	m_pPostDisplayCallback = NULL;
	m_pPostDisplayCallbackUserData = NULL;
}

CNexCodecWrap::~CNexCodecWrap( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] ~~~~CNexCodecWraop In", __LINE__);

	deinitDecoder();

	m_bInitDecoder = FALSE;
	m_uiCodecUserData= 0;
	m_ColorFormat = 0;

	m_iWidth = 0;
	m_iHeight = 0;
	m_iPitch = 0;

	m_pSurfaceTexture = NULL;
	m_pPostDisplayCallback = NULL;
	m_pPostDisplayCallbackUserData = NULL;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] ~~~~CNexCodecWraop Out", __LINE__);
}

NXBOOL CNexCodecWrap::isInitDecoder()
{
	return m_bInitDecoder;
}

int CNexCodecWrap::getWidth()
{
	return m_iWidth;
}

int CNexCodecWrap::getHeight()
{
	return m_iHeight;
}

int CNexCodecWrap::getPitch()
{
	return m_iPitch;
}

NXINT64 CNexCodecWrap::getFormat()
{
	return m_ColorFormat;
}

NXBOOL CNexCodecWrap::setSurfaceTexture(void* pSurf)
{
	m_pSurfaceTexture = pSurf;
	return TRUE;
}

void* CNexCodecWrap::getSurfaceTexture()
{
	return m_pSurfaceTexture;
}

NXBOOL CNexCodecWrap::getCodec(NEXCALMediaType eType, NEXCALMode eMode, unsigned int uiOTI, int iWidth, int iHeight, int iPitch)
{
	m_hCodecVideo = CNexCodecManager::getCodec(		eType,
													eMode,
													uiOTI,
													iWidth,
													iHeight,
													iWidth);
	if( m_hCodecVideo == NULL )
	{
		return FALSE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] GetCodec m_hCodecVideo=(%p)", __LINE__, m_hCodecVideo);
	return TRUE;
}

NXBOOL CNexCodecWrap::setCodec(NEXCALCodecHandle hCodecVideo)
{
	deinitDecoder();
	m_hCodecVideo = hCodecVideo;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] setCodec m_hCodecVideo=(%p)", __LINE__, m_hCodecVideo);
	return TRUE;
}

NXINT32 CNexCodecWrap::initDecoder(	NEX_CODEC_TYPE a_eCodecType,
									NXUINT8 *a_pConfig,
									NXINT32 a_nLen,
									NXUINT8 *a_pConfigEnhance,
									NXINT32 a_nEnhLen,
									NXVOID *a_pInitInfo,
									NXVOID *a_pExtraInfo,
									NXINT32 a_nNALHeaderSize,
									NXINT32 *a_pnWidth,
									NXINT32 *a_pnHeight,
									NXINT32 *a_pnPitch,
									NXINT32 a_nFps,
									NXUINT32 a_uMode)
{
	unsigned int uiUserDataType = 0;
	m_uiCodecUserData = (NXINT64)this;

	if( CNexCodecManager::isHardwareCodec(m_hCodecVideo) )
	{
#if defined(_ANDROID)
		if(m_pSurfaceTexture != NULL)
		{
			uiUserDataType = (unsigned int)'NKMS';
			m_uiCodecUserData = (NXINT64)m_pSurfaceTexture;// surfaceTexuture
		}
#endif
	}
	else
	{
		nexCAL_VideoDecoderSetProperty( m_hCodecVideo, NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_VIDEO_BUFFER_NV12);
	}

	nexCAL_SetClientIdentifier(m_hCodecVideo, (void*)m_uiCodecUserData);
	nexCAL_VideoDecoderSetProperty( m_hCodecVideo, NEXCAL_PROPERTY_VIDEO_FPS, a_nFps);
	nexCAL_VideoDecoderSetProperty( m_hCodecVideo, NEXCAL_PROPERTY_VIDEO_DECODER_MODE, CNexVideoEditor::m_bVideoDecoderSW);

	NXINT32 iRet = nexCAL_VideoDecoderInit( m_hCodecVideo,
										    a_eCodecType,
											a_pConfig,
											a_nLen,
											a_pConfigEnhance,
											a_nEnhLen,
											a_pInitInfo,
											a_pExtraInfo,
											a_nNALHeaderSize,
											a_pnWidth,
											a_pnHeight,
											a_pnPitch,
											a_uMode,
											uiUserDataType,
											(void*)m_uiCodecUserData);


	m_ColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
	if( nexCAL_VideoDecoderGetProperty( m_hCodecVideo, NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, &m_ColorFormat) != NEXCAL_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] Video Decoder getProperty failed", __LINE__);
		m_ColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
	}

#if defined(_ANDROID)
	if(m_pSurfaceTexture == NULL)
    {
		m_ColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;		
    }
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] ColorFormat(0x%x)", __LINE__, m_ColorFormat);


	if( iRet == NEXCAL_ERROR_NONE )
	{
		m_bInitDecoder = TRUE;
		if( CNexCodecManager::isHardwareCodec(m_hCodecVideo) )
		{
#ifdef _ANDROID
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor )
			{
				m_pPostDisplayCallback = (postDisplayCallback_t)pEditor->getMediaCodecCallback();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] Get PostDisplayCallback(%p) sucessecd", __LINE__, m_pPostDisplayCallback);
				SAFE_RELEASE(pEditor);
			}

			NXINT64 uiCALDisplayFlagCallBackUserData = 0;
			if( nexCAL_VideoDecoderGetProperty( m_hCodecVideo, NEXCAL_PROPERTY_GET_DECODER_INFO, &uiCALDisplayFlagCallBackUserData) == NEXCAL_ERROR_NONE )
			{
				m_pPostDisplayCallbackUserData = (void*)uiCALDisplayFlagCallBackUserData;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] Get PostDisplayCallbackUserData(%p) sucessecd", __LINE__, m_pPostDisplayCallbackUserData);
			}

			if( m_pPostDisplayCallback == NULL || m_pPostDisplayCallbackUserData == NULL )
			{
				iRet = NEXCAL_ERROR_FAIL;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] Get PostDisplayCallback and user data failed(%p %p)", __LINE__, m_pPostDisplayCallback, m_pPostDisplayCallbackUserData);
				deinitDecoder();
				return iRet;
			}
#endif
		}
	}
	return iRet;
}

NXBOOL CNexCodecWrap::deinitDecoder()
{
	if( m_hCodecVideo != NULL )
	{
		if( m_bInitDecoder )
		{
			nexCAL_VideoDecoderDeinit( m_hCodecVideo );
			m_bInitDecoder = FALSE;
		}

		CNexCodecManager::releaseCodec(m_hCodecVideo);
		m_hCodecVideo = NULL;
	}

	m_pPostDisplayCallback = NULL;
	m_pPostDisplayCallbackUserData = NULL;
	return TRUE;
}

NEXCALCodecHandle CNexCodecWrap::getCodecHandle()
{
	return m_hCodecVideo;
}

NXBOOL CNexCodecWrap::isHardwareCodec()
{
	return CNexCodecManager::isHardwareCodec(m_hCodecVideo);
}
#ifdef _ANDROID
void CNexCodecWrap::postFrameRenderCallback(unsigned char* pBuff, NXBOOL bRender)
{
	if( m_pPostDisplayCallback == NULL || m_pPostDisplayCallbackUserData == NULL )
		return;

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] postFrameRenderCallback(%p %d)", __LINE__, pBuff, bRender);

	m_pPostDisplayCallback(pBuff, bRender, m_pPostDisplayCallbackUserData);
}
#endif

void CNexCodecWrap::renderDecodedFrame(unsigned char* pBuff)
{
#if defined(_ANDROID)

    if( m_pPostDisplayCallback == NULL || m_pPostDisplayCallbackUserData == NULL )
        return;

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] postFrameRenderCallback(%p TRUE)", __LINE__, pBuff);
    m_pPostDisplayCallback(pBuff, TRUE, m_pPostDisplayCallbackUserData);

#elif defined(__APPLE__)
    // nothing to do
#endif
}

extern "C" void releaseImageBuffer(void* imageBuffer);

void CNexCodecWrap::releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender)
{
#if defined(_ANDROID)

    if( m_pPostDisplayCallback == NULL || m_pPostDisplayCallbackUserData == NULL )
        return;

    if ( isAfterRender == FALSE )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecWrap.cpp %d] postFrameRenderCallback(%p FALSE)", __LINE__, pBuff);
        m_pPostDisplayCallback(pBuff, TRUE, m_pPostDisplayCallbackUserData);
    }
    
#elif defined(__APPLE__)
    releaseImageBuffer(pBuff);
#endif
    
}
