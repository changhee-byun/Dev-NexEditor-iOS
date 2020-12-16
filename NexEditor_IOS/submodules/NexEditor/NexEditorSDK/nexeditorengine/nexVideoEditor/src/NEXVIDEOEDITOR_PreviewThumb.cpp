/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_PreviewThumb.cpp
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

#include "NEXVIDEOEDITOR_PreviewThumb.h"
#include "yuv420_to_rgb16_ex.h"
#include "NexTheme.h"
#include "NexThemeRenderer.h"
#ifdef ANDROID
#include "NexThemeRenderer_SurfaceTexture.h"
#endif
#ifdef __APPLE__
#include "NexThemeRenderer_Platform_iOS-CAPI.h"
#endif
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "nexYYUV2YUV420.h"
#include <algorithm>

CPreviewInfo::CPreviewInfo()
{
	m_uiTime = 0;
	m_pBuffer = NULL;
}

CPreviewInfo::CPreviewInfo(unsigned int uiTime, unsigned char* pBuffer)
{
	m_uiTime = uiTime;
	m_pBuffer = pBuffer;
}

unsigned int CPreviewInfo::getTime()
{
	return m_uiTime;
}

unsigned char* CPreviewInfo::getBuffer()
{
	return m_pBuffer;
}

CPreviewThumb::CPreviewThumb()
{
	m_iSrcWidth			= 0;
	m_iSrcHeight			= 0;
	m_iSrcPitch			= 0;

	m_hThumbRenderer		= NULL;
	m_pThumbDecSurface		= NULL;
	m_pOutputSurface		= NULL;

	m_pCodecWrap = NULL;
	clear();
}

CPreviewThumb::~CPreviewThumb()
{
	SAFE_RELEASE(m_pCodecWrap);
#ifdef _ANDROID
	if( m_hThumbRenderer != NULL && m_pThumbDecSurface != NULL )
	{
	 	NXT_ThemeRenderer_DestroySurfaceTexture( m_hThumbRenderer, (ANativeWindow*) m_pThumbDecSurface);
	}
#endif

	removePreviewData(0, 0xffffffff);

	m_hThumbRenderer	= NULL;
	m_pThumbDecSurface	= NULL;
	m_pOutputSurface 	= NULL;
	deinit();

	clear();
}

NXBOOL CPreviewThumb::init(void* pThumbRender, void* pOutputSurface)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] PreviewThumb render(%p) outSurface(%p)", __LINE__, pThumbRender, pOutputSurface);

	ANativeWindow* surface = NULL;
	m_hThumbRenderer = (NXT_HThemeRenderer)pThumbRender;
	m_pOutputSurface  = pOutputSurface;
#if defined(__APPLE__)
	NXT_ThemeRenderer_SetRenderTargetInformation( m_hThumbRenderer, THUMBNAIL_VIDEO_MODE_RAW_WIDTH, THUMBNAIL_VIDEO_MODE_RAW_HEIGHT );
#elif defined(_ANDROID)
	NXT_ThemeRenderer_SetNativeWindow(	m_hThumbRenderer,
											(ANativeWindow*)m_pOutputSurface,
											THUMBNAIL_VIDEO_MODE_RAW_WIDTH,
											THUMBNAIL_VIDEO_MODE_RAW_HEIGHT );
#endif
	// Case : for mantis 9228
	NXT_ThemeRenderer_ClearTransitionEffect(m_hThumbRenderer);
	NXT_ThemeRenderer_ClearClipEffect(m_hThumbRenderer);
		
#ifdef _ANDROID
 	NXT_ThemeRenderer_CreateSurfaceTexture( m_hThumbRenderer, &surface );
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] Created SurfaceTexture for thumb decoding(%p)", __LINE__, surface);
	if( surface )
	{
		m_pThumbDecSurface	= (void*)surface;
	}			

	NXT_ThemeRenderer_AquireContext(m_hThumbRenderer);
	NXT_ThemeRenderer_GLDraw(m_hThumbRenderer, NXT_RendererOutputType_RGBA_8888, 1);
	NXT_ThemeRenderer_ReleaseContext(m_hThumbRenderer, 1);		
		
	return TRUE;
INIT_FAIL:
	return FALSE;
}

void* CPreviewThumb::getPreviewThumbSurfaceTexture()
{
	return m_pThumbDecSurface;
}

unsigned int CPreviewThumb::getSize()
{
	return size();
}

NXBOOL CPreviewThumb::addPreviewData(unsigned int uiTime, unsigned char* pData)
{
	CPreviewInfo info(uiTime, pData);

	if(size())
	{
		vecPreviewInfoItr i = begin();
    
		for(; i != end(); i++)
		{
			CPreviewInfo info = (CPreviewInfo&)*i;

			if(uiTime < info.getTime())
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] addPreviewData (%d, %d, 0x%x)", __LINE__, uiTime, info.getTime(), info.getBuffer());
				break;
			}
		}
		insert(++i, info);			        
	}
	else
	{
		insert(end(), info);	
	}

	return TRUE;
}

unsigned char* CPreviewThumb::getPreviewData(unsigned int uiTime)
{
	vecPreviewInfoItr i = begin();
	unsigned int uiStartTime = 0, uiEndTime = 0;
	getStartEndTime(&uiStartTime, &uiEndTime);

	if(!(uiStartTime <= uiTime && uiEndTime >= uiTime))
	{
		return NULL;	
	}
    
	for(; i != end(); i++)
	{
		CPreviewInfo info = (CPreviewInfo&)*i;

		if(uiTime <= info.getTime())
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] getPreviewData (%d, %d, 0x%x)", __LINE__, uiTime, info.getTime(), info.getBuffer());
			return info.getBuffer();
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] getPreviewData there is not available data %d", __LINE__, uiTime);
	return NULL;
}

NXBOOL CPreviewThumb::getStartEndTime(unsigned int* puiStartTime, unsigned int* puiEndTime)
{
if(size() == 0)
{
*puiStartTime = 0;
*puiEndTime = 0;
return TRUE;
}
	vecPreviewInfoItr i = begin();
	CPreviewInfo info = (CPreviewInfo&)*i;
	*puiStartTime = info.getTime();

	i = end();
	i--;
	info = (CPreviewInfo&)*i;
	*puiEndTime = info.getTime();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] getStartEndTime (%d, %d)", __LINE__, *puiStartTime, *puiEndTime);

	return TRUE;
}

NXBOOL CPreviewThumb::removePreviewData(unsigned int uiStartTime, unsigned int uiEndTime)
{
	int iSize = size();
	vecPreviewInfoItr i = begin();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] removePreviewData %d, %d", __LINE__, uiStartTime, uiEndTime );

	for(int j=0; j < iSize; j++)
	{
		CPreviewInfo info = (CPreviewInfo&)*i;
		if(uiStartTime > info.getTime() || uiEndTime < info.getTime())
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] removePreviewData delete %d", __LINE__, info.getTime() );
			nexSAL_MemFree(info.getBuffer());
			erase(i);
		}
		else
		{
			i++;
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] removePreviewData remain %d", __LINE__, info.getTime() );
		}
	}
	return TRUE;
}

bool cmp_Time(CPreviewInfo a, CPreviewInfo b) { return a.getTime() < b.getTime();; }
void CPreviewThumb::sortWithTime()
{
	std::sort(begin(), end(), cmp_Time);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] sortWithTime", __LINE__ );
}

NXBOOL CPreviewThumb::setPreviewThumbInfo(int iWidth, int iHeight, int iPitch)
{
	m_iSrcWidth			= iWidth;
	m_iSrcHeight			= iHeight;
	m_iSrcPitch			= iPitch;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] PreviewThumb init(%d, %d, %d)", __LINE__, m_iSrcWidth, m_iSrcHeight, m_iSrcPitch);

	return TRUE;
}

NXBOOL CPreviewThumb::addPreviewThumbInfo(void* pMediaBuffer, unsigned int uiCTS, NXBOOL bReverse)
{
	NXBOOL RetVal = TRUE;	
	int nWrite = 0;
	if( m_hThumbRenderer )
	{
		NXT_ThemeRenderer_AquireContext(m_hThumbRenderer);
#if defined(_ANDROID)
		callCodecWrapPostCallback((unsigned char*)pMediaBuffer, TRUE);
#ifdef FOR_TEST_MEDIACODEC_DEC
		NXT_ThemeRenderer_WaitForFrameToBeAvailable(m_hThumbRenderer, (ANativeWindow*)m_pThumbDecSurface, -1);
#endif

		// KM-3035 call render callback inside Render Context.
//		callVideoFrameRenderCallback((unsigned char*)pMediaBuffer, TRUE);

		NXT_ThemeRenderer_SetSurfaceTexture(m_hThumbRenderer,
											0, //track_id ..... there's no need for specifying it in this situation
											NXT_TextureID_Video_1,
											m_iSrcWidth,
											m_iSrcHeight,
											m_iSrcPitch,
											m_iSrcHeight,
											(ANativeWindow *)m_pThumbDecSurface,
											0,   //convert_rgb_flag
											0,   //tex_id_for_lut
											0,   //tex_id_for_customlut_a
											0,   //tex_id_for_customlut_b
											0   //tex_id_for_customlut_power
		);
#elif defined(__APPLE__)
        callCodecWrap_renderDecodedFrame((unsigned char*)pMediaBuffer);
        
        NXT_ThemeRenderer_UploadTextureForHWDecoder(m_hThumbRenderer,
                                                    NXT_TextureID_Video_1,
                                                    (unsigned char*)pMediaBuffer,
                                                    NULL,
                                                    NULL,
                                                    0,0);
#endif
        
//		NXT_ThemeRenderer_SetTextureRotation(m_hThumbRenderer, NXT_TextureID_Video_1, (unsigned int)m_iRotateState);
		NXT_ThemeRenderer_GLDraw(m_hThumbRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);
		NXT_ThemeRenderer_GLWaitToFinishRendering(m_hThumbRenderer);

		int iThumbWidth			= 0;
		int iThumbHeight		= 0;
		int iThumbSize			= 0;
		unsigned char* pThumb	= NULL;
		
		if( NXT_ThemeRenderer_GetPixels(m_hThumbRenderer, &iThumbWidth, &iThumbHeight, &iThumbSize, &pThumb, FALSE) == NXT_Error_None )
		{
				unsigned int uiBufferSize = iThumbWidth * iThumbHeight * 3 / 2;
				unsigned char* pRAWData = (unsigned char*)nexSAL_MemAlloc(uiBufferSize);
				unsigned char* pY = pRAWData;
				unsigned char* pUV = pRAWData + (iThumbWidth * iThumbHeight);
				unsigned char* pU = pRAWData + (iThumbWidth * iThumbHeight);
				unsigned char* pV = pRAWData + (iThumbWidth * iThumbHeight + (iThumbWidth * iThumbHeight/4));

				nexYYUVtoY2UV(iThumbWidth, iThumbHeight,  pThumb, pY, pUV);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] one frame PreviewThumb writen %d, %d size(%d, %d) time(%d), 0x%x", 	__LINE__, iThumbWidth, iThumbHeight, iThumbSize, uiBufferSize, uiCTS, pRAWData);

				//addPreviewData(uiCTS, pRAWData);
				CPreviewInfo info(uiCTS, pRAWData);
				if(bReverse)
					insert(begin(), info);
				else
					insert(end(), info);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] read PreviewThumb failed", __LINE__);
		}

		NXT_ThemeRenderer_ReleaseContext(m_hThumbRenderer, 1);
	}
	return RetVal;
}
	
NXBOOL CPreviewThumb::deinit()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PreviewThumb.cpp %d] close video PreviewThumb ", __LINE__);

#if defined(__APPLE__)
        if( m_hThumbRenderer != NULL ) {
            NXT_ThemeRenderer_SetRenderTargetInformation( m_hThumbRenderer, 0, 0 );
        }
        
#elif defined(ANDROID)

	if( m_hThumbRenderer != NULL && m_pThumbDecSurface != NULL  )
	{
	 	NXT_ThemeRenderer_DestroySurfaceTexture( m_hThumbRenderer, (ANativeWindow*) m_pThumbDecSurface);
		// clear output surface
		NXT_ThemeRenderer_SetNativeWindow(m_hThumbRenderer, (ANativeWindow*)NULL, 0, 0);
		m_pThumbDecSurface = NULL;
	}
#endif

	return TRUE;
}

void CPreviewThumb::setCodecWrap(CNexCodecWrap* pCodec)
{
	SAFE_RELEASE(m_pCodecWrap);
	m_pCodecWrap = pCodec;
	SAFE_ADDREF(m_pCodecWrap);
}

#if defined(_ANDROID)
void CPreviewThumb::callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender)
{
	if( m_pCodecWrap != NULL )
		m_pCodecWrap->postFrameRenderCallback(pBuff, bRender);
}
#elif defined(__APPLE__)
void CPreviewThumb::callCodecWrap_renderDecodedFrame(unsigned char* pBuff)
{
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->renderDecodedFrame(pBuff);
    }
}

void CPreviewThumb::callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender)
{
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->releaseDecodedFrame(pBuff, isAfterRender);
    }
}
#endif
