/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoTask.cpp
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

#include "NEXVIDEOEDITOR_Thumbnail.h"
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

int makeBigendian( unsigned int uiData ) 
{
	unsigned int value = 0;
	unsigned char *pByte = (unsigned char*)&value;
	pByte[0] = ((uiData >> 24)&0xFF);
	pByte[1] = ((uiData >> 16)&0xFF);
	pByte[2] = ((uiData >> 8)&0xFF);
	pByte[3] = (uiData&0xFF);
	return (int)value;
}

CThumbnail::CThumbnail()
{
	m_eThumbMode		= THUMBNAIL_MODE_NONE;
	m_iDuration			= 0;
	m_isVideo			= 0;
	m_isAudio			= 0;

	m_iMaxThumbnailCount	= THUMBNAIL_MAX_COUNT;
	m_iThumbnailFlag		= 0;
	m_iThumbnailTag		= 0;

	m_iSrcWidth			= 0;
	m_iSrcHeight			= 0;
	m_iSrcPitch			= 0;

	m_iHexWidth			= 0;
	m_iHexHeight			= 0;
	m_iHexPitch			= 0;
	
	m_iDstWidth			= 0;
	m_iDstHeight			= 0;
	m_iDstPitch			= 0;
	m_iBitsForPixel		= 0;
	m_iBufferSize			= 0;

	m_bScaleMode		= FALSE;

	m_uiCurrentFilePos		= 0;
	m_uiThumbnailCount	= 0;

	m_pThumbnailInfo		= NULL;

	m_strPath = NULL;

	m_pHexY				= NULL;
	m_pHexU				= NULL;
	m_pHexV				= NULL;

	m_pTempY			= NULL;
	m_pTempU			= NULL;
	m_pTempV			= NULL;

	m_pRGBBuffer		= NULL;

	m_pScaler			= NULL;
	m_pFile				= NEXSAL_INVALID_HANDLE;

	m_pSeekTable		= NULL;
	m_uSeekTableCount	= 0;

	m_pPCMTable			= NULL;
	m_uPCMTableCount	= 0;

	m_pLargeThumbnail	= NULL;
	m_iLargeThumbnailSize	= 0;

	m_iLargeThumbnailWidth	= 0;
	m_iLargeThumbnailHeight	= 0;

	m_hThumbRenderer		= NULL;
	m_pThumbDecSurface		= NULL;
	m_pOutputSurface		= NULL;

#ifdef YUV_TEST_DUMP
	m_pDumpYUV			= NULL;
#endif

	m_bSkipFirstFrame		= FALSE;//TRUE;

	m_pCodecWrap = NULL;

	m_uStartTime = 0;
	m_uEndTime = 0;
	m_bRequestedRawData = 0;

	m_pRAWData = NULL;
	m_uRAWSize = 0;	
	m_pPCMData = NULL;
	m_uPCMSize = 0; 	
	
	m_pTimeTable = NULL;
	m_iTimeTableCount = 0;
	
	m_vecThumbTime.clear();
	
}

CThumbnail::~CThumbnail()
{
	SAFE_RELEASE(m_pCodecWrap);
#ifdef _ANDROID
	if( m_hThumbRenderer != NULL && m_pThumbDecSurface != NULL )
	{
	 	NXT_ThemeRenderer_DestroySurfaceTexture( m_hThumbRenderer, (ANativeWindow*) m_pThumbDecSurface);
	}
#endif

	m_hThumbRenderer	= NULL;
	m_pThumbDecSurface	= NULL;
	m_pOutputSurface 	= NULL;
	deinit();

	m_vecThumbTime.clear();
}

NXBOOL CThumbnail::existThumbnailFile(const char* pThumbnailPath)
{
	NEXSALFileHandle pFile = nexSAL_FileOpen((char*)pThumbnailPath, (NEXSALFileMode)(NEXSAL_FILE_READ));
	if( pFile != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] CheckThumbnail return failed because thumbnail already existed(%p)", __LINE__, pFile);
		nexSAL_FileClose(pFile);
		return TRUE;
	}
	return FALSE;
}

NXBOOL CThumbnail::init(const char* pThumbnailPath, int iDuration, int isVideo, int isAudio)
{
	if( pThumbnailPath == NULL ) return FALSE;

	if(m_strPath)
	{
		nexSAL_MemFree(m_strPath);
		m_strPath = NULL;
	}

	m_strPath = (char*)nexSAL_MemAlloc(strlen(pThumbnailPath)+1);
	strcpy(m_strPath, pThumbnailPath);

	m_bSkipFirstFrame 		= FALSE;//TRUE;

	m_iDuration			= iDuration;
	m_isVideo			= isVideo;
	m_isAudio			= isAudio;

	m_pFile = nexSAL_FileOpen(m_strPath, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
	if( m_pFile == NEXSAL_INVALID_HANDLE  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init(%s) failed because file existed", __LINE__, m_strPath);
		goto INIT_FAIL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init(%s)", __LINE__, m_strPath);

	return TRUE;
INIT_FAIL:

	if(m_strPath)
	{
		nexSAL_MemFree(m_strPath);
		m_strPath = NULL;
	}

	m_iDuration	= 0;
	m_isVideo	= 0;
	m_isAudio	= 0;
	
	if( m_pFile != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_FileClose(m_pFile);
		m_pFile = NEXSAL_INVALID_HANDLE;
	}
	
	return FALSE;
}

NXBOOL CThumbnail::init(NXBOOL bRawData, THUMBNAIL_MODE eMode, void* pThumbRender, void* pOutputSurface, int* pTimeTable, int iTimeTableCount, int iFlag)
{
	m_bSkipFirstFrame		= FALSE;//TRUE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init(%d) mode(%d) render(%p) iFlag(0x%x)", __LINE__, bRawData, eMode, pThumbRender, iFlag);

	m_bRequestedRawData = bRawData;
	m_iThumbnailFlag = iFlag;
	m_eThumbMode = eMode;

	if(m_pTimeTable)
	{
		nexSAL_MemFree(m_pTimeTable);
		m_pTimeTable = NULL;
		m_iTimeTableCount = 0;
	}

	m_pTimeTable = (int*)nexSAL_MemAlloc(sizeof(int)*iTimeTableCount);
	if( m_pTimeTable == NULL )
	{
		return FALSE;
	}
	memcpy(m_pTimeTable, pTimeTable, sizeof(int)*iTimeTableCount);
	m_iTimeTableCount = iTimeTableCount;
	m_iMaxThumbnailCount = iTimeTableCount;

	if( m_eThumbMode == THUMBNAIL_MODE_VIDEO )
	{
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
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Created SurfaceTexture for thumb decoding(%p)", __LINE__, surface);
		if( surface )
		{
			m_pThumbDecSurface	= (void*)surface;
		}			

		NXT_ThemeRenderer_AquireContext(m_hThumbRenderer);
		NXT_ThemeRenderer_GLDraw(m_hThumbRenderer, NXT_RendererOutputType_JUST_CLEAR, 1);
		NXT_ThemeRenderer_ReleaseContext(m_hThumbRenderer, 1);		
		
	}
	return TRUE;
INIT_FAIL:
	return FALSE;
}

NXBOOL CThumbnail::init(const char* pThumbnailPath, THUMBNAIL_MODE eMode, void* pThumbRender, void* pOutputSurface, int iDstWidth, int iDstHeight, int iMaxCount, int iFlag, int iTag)
{
	if( pThumbnailPath == NULL ) return FALSE;

	m_bSkipFirstFrame		= FALSE;//TRUE;

	if( pThumbnailPath != NULL && strlen(pThumbnailPath) > 0 )
	{
		if(m_strPath)
		{
			nexSAL_MemFree(m_strPath);
			m_strPath = NULL;
		}
		m_strPath = (char*)nexSAL_MemAlloc(strlen(pThumbnailPath)+1);

		strcpy(m_strPath, pThumbnailPath);

		m_pFile = nexSAL_FileOpen(m_strPath, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
		if( m_pFile == NEXSAL_INVALID_HANDLE  )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init(%s) failed because file existed", __LINE__, m_strPath);
			goto INIT_FAIL;
		}
	}

	m_iMaxThumbnailCount = iMaxCount == 0 ? THUMBNAIL_MAX_COUNT : iMaxCount;
	m_iThumbnailFlag = iFlag;
	m_iThumbnailTag = iTag;

	if( (m_iThumbnailFlag & GET_THUMBRAW_RGB_CALLBACK) == GET_THUMBRAW_RGB_CALLBACK )
	{
		m_bRequestedRawData = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init with callback mode", __LINE__);
	}

#ifdef FOR_TEST_THUMBNAIL_FILE_FAILED
	{
		char buffer[2];
		int iExpectSize = iDstWidth * iDstHeight * (m_iMaxThumbnailCount) * 4;
		int iSeekRet = nexSAL_FileSeek(m_pFile, iExpectSize, NEXSAL_SEEK_BEGIN);

		int iWriteWrite = nexSAL_FileWrite(m_pFile, buffer, 2);
		if( iWriteWrite != 2 )
		{
			m_bRequestedRawData = 1;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init with callback mode(%d %d %d)", __LINE__, iExpectSize, iSeekRet, iWriteWrite);
		}
		
		nexSAL_FileSeek(m_pFile, 0, NEXSAL_SEEK_BEGIN);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init check write(%d %d %d)", __LINE__, iExpectSize, iSeekRet, iWriteWrite);
	}
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init(%s) mode(%d) render(%p) size(%d %d) flag(%d)", __LINE__, m_strPath, eMode, pThumbRender, iDstWidth, iDstHeight, iFlag);

	m_iDstWidth = iDstWidth;
	m_iDstHeight = iDstHeight;

	if( iDstWidth == 0 || iDstHeight == 0 )
	{
		m_iDstWidth = THUMBNAIL_VIDEO_MODE_VIDEO_WIDTH;
		m_iDstHeight = THUMBNAIL_VIDEO_MODE_VIDEO_HEIGHT;
	}

	m_eThumbMode = eMode;
	if( m_eThumbMode == THUMBNAIL_MODE_VIDEO )
	{
		ANativeWindow* surface = NULL;
		m_hThumbRenderer = (NXT_HThemeRenderer)pThumbRender;
		m_pOutputSurface  = pOutputSurface;
#if defined(__APPLE__)
		NXT_ThemeRenderer_SetRenderTargetInformation( m_hThumbRenderer, m_iDstWidth, m_iDstHeight );
#elif defined(_ANDROID)
		NXT_ThemeRenderer_SetNativeWindow(	m_hThumbRenderer,
												(ANativeWindow*)m_pOutputSurface,
												m_iDstWidth,
												m_iDstHeight );
#endif
		// Case : for mantis 9228
		NXT_ThemeRenderer_ClearTransitionEffect(m_hThumbRenderer);
		NXT_ThemeRenderer_ClearClipEffect(m_hThumbRenderer);
		
#ifdef ANDROID
	 	NXT_ThemeRenderer_CreateSurfaceTexture( m_hThumbRenderer, &surface );
#endif
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Created SurfaceTexture for thumb decoding(%p)", __LINE__, surface);
		if( surface )
		{
			m_pThumbDecSurface	= (void*)surface;
		}			

		NXT_ThemeRenderer_AquireContext(m_hThumbRenderer);
		NXT_ThemeRenderer_GLDraw(m_hThumbRenderer, NXT_RendererOutputType_JUST_CLEAR, 1);
		NXT_ThemeRenderer_ReleaseContext(m_hThumbRenderer, 1);		
		
		m_iRotateState = 0;
		if( (iFlag & GET_FLAG_ROTATE_90_MODE) == GET_FLAG_ROTATE_90_MODE )
		{
			m_iRotateState = 90;
		}
		else if( (iFlag & GET_FLAG_ROTATE_180_MODE) == GET_FLAG_ROTATE_180_MODE )
		{
			m_iRotateState = 180;
		}
		else if( (iFlag & GET_FLAG_ROTATE_270_MODE) == GET_FLAG_ROTATE_270_MODE )
		{
			m_iRotateState = 270;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] RotateState(%d)", __LINE__, m_iRotateState);		
	}

	m_pThumbnailInfo		= (THUMBNAILINFO*)nexSAL_MemAlloc(sizeof(THUMBNAILINFO) * m_iMaxThumbnailCount);
	if( m_pThumbnailInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail buffer alloc failed", __LINE__);
		goto INIT_FAIL;
	}
	memset(m_pThumbnailInfo, 0x00, sizeof(THUMBNAILINFO)*m_iMaxThumbnailCount);
	
	return TRUE;
INIT_FAIL:

	if(m_strPath)
	{
		nexSAL_MemFree(m_strPath);
		m_strPath = NULL;
	}
	return FALSE;
}

void* CThumbnail::getThumbSurfaceTexture()
{
	return m_pThumbDecSurface;
}

NXBOOL CThumbnail::setThumbnailInfo(int iWidth, int iHeight, int iPitch, NXBOOL bSoftwareCodec)
{
	if( m_eThumbMode == THUMBNAIL_MODE_AUDIO )
		return TRUE;
	int nWrite = 0; //yoon
	m_iSrcWidth			= iWidth;
	m_iSrcHeight			= iHeight;
	m_iSrcPitch			= iPitch;

	// Case : for mantis 9348 problem.
	m_iHexWidth			= (m_iSrcWidth >> 5)  << 5;
	m_iHexHeight			= (m_iSrcHeight >> 5)  << 5;
	m_iHexPitch			= (m_iSrcPitch >> 5) << 5;

	if( bSoftwareCodec )
	{
		if( (m_iSrcPitch * m_iSrcHeight) < (m_iDstWidth * m_iDstHeight) )
		{
			m_iDstWidth		= m_iHexWidth;
			m_iDstHeight		= m_iHexHeight;
			m_iDstPitch		= m_iHexPitch;
		}
		else
		{
			m_iDstWidth			= (m_iSrcWidth / (m_iSrcWidth / m_iDstWidth)) / 16 * 16;
			m_iDstHeight			= (m_iSrcHeight / (m_iSrcHeight / m_iDstHeight )) / 16 * 16;
			m_iDstPitch			= m_iDstWidth;
			m_bScaleMode		= TRUE;
		}

		m_iBitsForPixel = VIDEO_DEFAULT_THUMBNAIL_BITCOUNT16;
		
		nWrite = nexSAL_FileWrite(m_pFile, &m_iBitsForPixel, sizeof(m_iBitsForPixel));
		if( nWrite <= 0 )	goto INIT_FAIL;
		nWrite = nexSAL_FileWrite(m_pFile, &m_iDstWidth, sizeof(m_iDstWidth));
		if( nWrite <= 0 )	goto INIT_FAIL;
		nWrite = nexSAL_FileWrite(m_pFile, &m_iDstHeight, sizeof(m_iDstHeight));		
		if( nWrite <= 0 )	goto INIT_FAIL;
	}
	else
	{
		m_iBitsForPixel = VIDEO_DEFAULT_THUMBNAIL_BITCOUNT32;
		if( (m_iThumbnailFlag & GET_THUMBRAW_YUV) == GET_THUMBRAW_YUV || (m_iThumbnailFlag & GET_THUMBRAW_Y_ONLY) == GET_THUMBRAW_Y_ONLY)
		{
			int BitsForPixel = 8;	// for detail thumbnail YUV or Y Only
			nWrite = nexSAL_FileWrite(m_pFile, &BitsForPixel, sizeof(BitsForPixel));
			if( nWrite <= 0 )	goto INIT_FAIL;
		}
		else
		{
			nWrite = nexSAL_FileWrite(m_pFile, &m_iBitsForPixel, sizeof(m_iBitsForPixel));
			if( nWrite <= 0 )	goto INIT_FAIL;
		}
		
		nWrite = nexSAL_FileWrite(m_pFile, &m_iDstWidth, sizeof(m_iDstWidth));
		if( nWrite <= 0 )	goto INIT_FAIL;
		nWrite = nexSAL_FileWrite(m_pFile, &m_iDstHeight, sizeof(m_iDstHeight));
		if( nWrite <= 0 )	goto INIT_FAIL;
		return TRUE;
	}

	// m_iBitsForPixel		= VIDEO_DEFAULT_THUMBNAIL_BITCOUNT16;
	m_iBufferSize			= m_iDstWidth * m_iDstHeight * m_iBitsForPixel / 8;

	m_pRGBBuffer		= (unsigned char*)nexSAL_MemAlloc(m_iBufferSize);

	m_pTempY			= (unsigned char*)nexSAL_MemAlloc(m_iDstPitch * m_iDstHeight);
	m_pTempU			= (unsigned char*)nexSAL_MemAlloc(m_iDstPitch * m_iDstHeight / 4);
	m_pTempV			= (unsigned char*)nexSAL_MemAlloc(m_iDstPitch * m_iDstHeight / 4);

	m_pHexY				= (unsigned char*)nexSAL_MemAlloc(m_iHexPitch * m_iHexHeight);
	m_pHexU				= (unsigned char*)nexSAL_MemAlloc(m_iHexPitch * m_iHexHeight / 4);
	m_pHexV				= (unsigned char*)nexSAL_MemAlloc(m_iHexPitch * m_iHexHeight / 4);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init Src(%d %d %d) HexSrc(%d %d %d) Dest(%d %d %d) ", __LINE__, 
		m_iSrcWidth, m_iSrcHeight, m_iSrcPitch, m_iHexWidth, m_iHexHeight, m_iHexPitch, m_iDstWidth, m_iDstHeight, m_iDstPitch);

	if( m_pRGBBuffer == NULL || m_pTempY == NULL || m_pTempU == NULL || m_pTempV == NULL || m_pHexY == NULL || m_pHexU == NULL || m_pHexV == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail failed", __LINE__);
		goto INIT_FAIL;
	}

	memset(m_pHexY, 0x00, m_iHexPitch * m_iHexHeight);
	memset(m_pHexU, 0x80, m_iHexPitch * m_iHexHeight / 4);
	memset(m_pHexV, 0x80, m_iHexPitch * m_iHexHeight / 4);

	if( m_bScaleMode )
	{
		m_pScaler = FUNC_ARM(NxVScalerInit)(m_iDstWidth, m_iDstHeight, m_iDstWidth, m_iHexWidth, m_iHexHeight, m_iHexPitch);
		if( m_pScaler == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail failed", __LINE__);
			goto INIT_FAIL;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail init(%s)", __LINE__, m_strPath);

#ifdef YUV_TEST_DUMP
	char strTemp[256];
	sprintf(strTemp, "/sdcard/%d_%d_%d_%d.yuv", m_iHexWidth, m_iHexHeight, m_iHexPitch, nexSAL_GetTickCount());

	m_pDumpYUV = fopen(strTemp, "wb");
#endif

	return TRUE;
INIT_FAIL:
	if( m_pRGBBuffer )
	{
		nexSAL_MemFree(m_pRGBBuffer);
		m_pRGBBuffer = NULL;
	}
	if( m_pTempY )
	{
		nexSAL_MemFree(m_pTempY);
		m_pTempY = NULL;
	}
	if( m_pTempU )
	{
		nexSAL_MemFree(m_pTempU);
		m_pTempU = NULL;
	}
	if( m_pTempV )
	{
		nexSAL_MemFree(m_pTempV);
		m_pTempV = NULL;
	}

	if( m_pHexY )
	{
		nexSAL_MemFree(m_pHexY);
		m_pHexY = NULL;
	}
	if( m_pHexU )
	{
		nexSAL_MemFree(m_pHexU);
		m_pHexU = NULL;
	}
	if( m_pHexV )
	{
		nexSAL_MemFree(m_pHexV);
		m_pHexV = NULL;
	}
	
	if( m_pScaler )
	{
		FUNC_ARM(NxVScalerClose)(m_pScaler);
		m_pScaler = NULL;
	}

	return FALSE;
}
#if 0
NXBOOL CThumbnail::addThumbInfo(unsigned int uiCTS, unsigned char* pNV12Buffer)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Write Thumbnail info Write In Count(%d)", __LINE__, m_uiThumbnailCount);
	
	if( m_uiThumbnailCount > m_iMaxThumbnailCount || m_pFile == NEXSAL_INVALID_HANDLE )
		return FALSE;
	
	int nWrite = 0;
	
#if 0	
	for(int i = 0; i < m_iHexHeight; i++)
	{
		memcpy(m_pHexY + (m_iHexPitch*i), pNV12Buffer +  (m_iSrcPitch*i), m_iHexWidth);
	}

	unsigned char* pTmpUV = pNV12Buffer + (m_iSrcPitch*m_iSrcHeight);
	unsigned char* pU = m_pHexU;
	unsigned char* pV = m_pHexV;

	for( int j = 0; j < m_iHexHeight / 2; j++)
	{
		for(int i = 0; i < m_iHexWidth / 2; i++ )
		{
			*(pU + i) = *(pTmpUV + (i*2));
			*(pV + i) = *(pTmpUV + (i*2) + 1);
		}
		pTmpUV += m_iSrcPitch;
		pU += m_iHexPitch / 2;
		pV += m_iHexPitch / 2;
	}
#endif	

	int iHalfPitch = m_iHexPitch/2;
	unsigned char* pTmpY = m_pHexY + (m_iHexHeight * m_iHexPitch) - m_iHexPitch;

	for(int i = 0; i < m_iHexHeight; i++)
	{
		memcpy(pTmpY, pNV12Buffer +  (m_iSrcPitch*i), m_iHexWidth);
		pTmpY -= m_iHexPitch;
	}

	unsigned char* pTmpUV = pNV12Buffer + (m_iSrcPitch*m_iSrcHeight);
	unsigned char* pU = m_pHexU + (m_iHexHeight * m_iHexPitch/4) - (m_iHexPitch/2);
	unsigned char* pV = m_pHexV + (m_iHexHeight * m_iHexPitch/4) - (m_iHexPitch/2);

	for( int j = 0; j < m_iHexHeight / 2; j++)
	{
		for(int i = 0; i < m_iHexWidth / 2; i++ )
		{
			*(pU + i) = *(pTmpUV + (i*2));
			*(pV + i) = *(pTmpUV + (i*2) + 1);
		}
		pTmpUV += m_iSrcPitch;
		pU -= iHalfPitch;
		pV -= iHalfPitch;
	}
	
	if( m_pScaler && m_bScaleMode )
	{
		FUNC_ARM(NxVScalerResizeYUV)(m_pScaler, m_pTempY, m_pTempU, m_pTempV, m_pHexY, m_pHexU, m_pHexV);

		if( m_iBitsForPixel == VIDEO_DEFAULT_THUMBNAIL_BITCOUNT16 )
		{
			NXCC_YUV420ToRGB565_EX((unsigned short *)m_pRGBBuffer, m_pTempY, m_pTempU, m_pTempV, m_iDstWidth, m_iDstHeight, m_iDstWidth, m_iDstWidth);
		}
		else
		{
			NXCC_YUV420ToRGB888_EX((NXUINT32*)m_pRGBBuffer, m_pTempY, m_pTempU, m_pTempV, m_iDstWidth, m_iDstHeight, m_iDstPitch, m_iDstWidth);
		}
	}
	else
	{
		if( m_iBitsForPixel == VIDEO_DEFAULT_THUMBNAIL_BITCOUNT16 )
		{
			NXCC_YUV420ToRGB565_EX((unsigned short *)m_pRGBBuffer, m_pHexY, m_pHexU, m_pHexV, m_iHexWidth, m_iHexHeight, m_iHexPitch, m_iHexWidth);
		}
		else
		{
			NXCC_YUV420ToRGB888_EX((NXUINT32*)m_pRGBBuffer, m_pHexY, m_pHexU, m_pHexV, m_iHexWidth, m_iHexHeight, m_iHexPitch, m_iHexWidth);
		}
	}

	if( m_pLargeThumbnail == NULL && m_uiThumbnailCount == 2 )
	{
		setLargeThumbnail(m_pHexY, m_pHexU, m_pHexV);
	}	

	if( m_eThumbMode == THUMBNAIL_MODE_VIDEO )
	{
		nWrite = nexSAL_FileWrite(m_pFile, &uiCTS, sizeof(uiCTS));
		if( nWrite <= 0 )return FALSE;
		
		nWrite = nexSAL_FileWrite(m_pFile, m_pRGBBuffer, m_iBufferSize);
		if( nWrite <= 0 )return FALSE;
			
		m_uiThumbnailCount++;
		return TRUE;
	}

	m_pThumbnailInfo[m_uiThumbnailCount].m_uiTime	= makeBigendian((unsigned int)uiCTS);
	m_pThumbnailInfo[m_uiThumbnailCount].m_iFilePos	= makeBigendian((unsigned int)m_uiCurrentFilePos);

	nWrite = nexSAL_FileWrite(m_pFile, m_pRGBBuffer, m_iBufferSize);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Write Thumbnail info Write(%d, %d)", __LINE__, m_uiThumbnailCount,nWrite);
	m_uiThumbnailCount++;
	m_uiCurrentFilePos += m_iBufferSize;
	if( nWrite <= 0 )return FALSE;
	return TRUE;
}

NXBOOL CThumbnail::addThumbInfo(unsigned int uiCTS, unsigned char* pY, unsigned char* pU, unsigned char* pV)
{
	if( m_uiThumbnailCount > m_iMaxThumbnailCount || m_pFile == NEXSAL_INVALID_HANDLE )
		return FALSE;
	int nWrite = 0;
	for(int i = 0; i < m_iHexHeight; i++)
	{
		memcpy(m_pHexY + (m_iHexPitch*i), pY +  (m_iSrcPitch*i), m_iHexWidth);
	}

	for(int i = 0; i < m_iHexHeight / 2; i++)
	{
		memcpy(m_pHexU + (m_iHexPitch/2*i), pU +  (m_iSrcPitch/2*i), m_iHexWidth/2);
		memcpy(m_pHexV + (m_iHexPitch/2*i), pV +  (m_iSrcPitch/2*i), m_iHexWidth/2);
	}

#ifdef YUV_TEST_DUMP
	if( m_pDumpYUV )
	{
		fwrite(m_pHexY, m_iHexPitch*m_iHexHeight, 1, m_pDumpYUV);
		fwrite(m_pHexU, m_iHexPitch*m_iHexHeight/4, 1, m_pDumpYUV);
		fwrite(m_pHexV, m_iHexPitch*m_iHexHeight/4, 1, m_pDumpYUV);
	}
#endif

	if( m_pLargeThumbnail == NULL && m_uiThumbnailCount == 2 )
	{
		setLargeThumbnail(m_pHexY, m_pHexU, m_pHexV);
	}

	FUNC_ARM(NxVScalerResizeYUV)(m_pScaler, m_pTempY, m_pTempU, m_pTempV, m_pHexY, m_pHexU, m_pHexV);

	if( m_iBitsForPixel == VIDEO_DEFAULT_THUMBNAIL_BITCOUNT16 )
	{
		NXCC_YUV420ToRGB565_EX((unsigned short *)m_pRGBBuffer, m_pTempY, m_pTempU, m_pTempV, m_iDstWidth, m_iDstHeight, m_iDstWidth, m_iDstWidth);
		// File Write
	}
	else
	{
		NXCC_YUV420ToRGB888_EX((NXUINT32*)m_pRGBBuffer, m_pTempY, m_pTempU, m_pTempV, m_iDstWidth, m_iDstHeight, m_iDstPitch, m_iDstWidth);
		// File Write
	}

	if( m_eThumbMode == THUMBNAIL_MODE_VIDEO )
	{
		nWrite = nexSAL_FileWrite(m_pFile, &uiCTS, sizeof(uiCTS));
		if( nWrite <= 0 ) return FALSE;
		nWrite = nexSAL_FileWrite(m_pFile, m_pRGBBuffer, m_iBufferSize);
		if( nWrite <= 0 ) return FALSE;
		return TRUE;
	}

	m_pThumbnailInfo[m_uiThumbnailCount].m_uiTime	= makeBigendian((unsigned int)uiCTS);
	m_pThumbnailInfo[m_uiThumbnailCount].m_iFilePos	= makeBigendian((unsigned int)m_uiCurrentFilePos);

	nWrite = nexSAL_FileWrite(m_pFile, m_pRGBBuffer, m_iBufferSize);
		
	m_uiThumbnailCount++;
	m_uiCurrentFilePos += m_iBufferSize;
	if( nWrite <= 0 ) return FALSE;
	return TRUE;
}
#endif
NXBOOL CThumbnail::addThumbInfo(void* pMediaBuffer, unsigned int uiCTS)
{
	if( m_uiThumbnailCount > m_iMaxThumbnailCount )
	{
#if defined(__APPLE__)
		callCodecWrap_releaseDecodedFrame((unsigned char*)pMediaBuffer, FALSE);
#elif defined(_ANDROID)
		callCodecWrapPostCallback((unsigned char*)pMediaBuffer, FALSE);
#endif
		return FALSE;
	}

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
        
		NXT_ThemeRenderer_SetTextureRotation(m_hThumbRenderer, NXT_TextureID_Video_1, (unsigned int)m_iRotateState);

		if( (m_iThumbnailFlag & GET_THUMBRAW_YUV) == GET_THUMBRAW_YUV || (m_iThumbnailFlag & GET_THUMBRAW_Y_ONLY) == GET_THUMBRAW_Y_ONLY )
		{
			NXT_ThemeRenderer_GLDraw(m_hThumbRenderer, NXT_RendererOutputType_Y2CrA_8888, 1);
		}
		else
		{
			NXT_ThemeRenderer_GLDraw(m_hThumbRenderer, NXT_RendererOutputType_RGBA_8888, 1);
		}

		NXT_ThemeRenderer_GLWaitToFinishRendering(m_hThumbRenderer);

		int iThumbWidth			= 0;
		int iThumbHeight		= 0;
		int iThumbSize			= 0;
		unsigned char* pThumb	= NULL;
		
		if( NXT_ThemeRenderer_GetPixels(m_hThumbRenderer, &iThumbWidth, &iThumbHeight, &iThumbSize, &pThumb, FALSE) == NXT_Error_None )
		{
			if( m_bSkipFirstFrame )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] first frame skip for black thumbnail", __LINE__);
				m_bSkipFirstFrame = FALSE;
			}
			else
			{
				m_uiThumbnailCount++;
				if( (m_iThumbnailFlag & GET_THUMBRAW_YUV) == GET_THUMBRAW_YUV || (m_iThumbnailFlag & GET_THUMBRAW_Y_ONLY) == GET_THUMBRAW_Y_ONLY)
				{
					unsigned int uiBufferSize = iThumbWidth * iThumbHeight * 3 / 2;
					if(m_pRAWData)
					{
						nexSAL_MemFree(m_pRAWData);
					}
					m_pRAWData = (unsigned char*)nexSAL_MemAlloc(uiBufferSize);

					unsigned char* pY = m_pRAWData;
					unsigned char* pUV = m_pRAWData + (iThumbWidth * iThumbHeight);
					unsigned char* pU = m_pRAWData + (iThumbWidth * iThumbHeight);
					unsigned char* pV = m_pRAWData + (iThumbWidth * iThumbHeight + (iThumbWidth * iThumbHeight/4));

					nexYYUVAtoYUV420(iThumbWidth, iThumbHeight, pThumb, pY, pU, pV);

					if( (m_iThumbnailFlag & GET_THUMBRAW_YUV) == GET_THUMBRAW_YUV)
						m_uRAWSize = uiBufferSize;
					else if( (m_iThumbnailFlag & GET_THUMBRAW_Y_ONLY) == GET_THUMBRAW_Y_ONLY)
						m_uRAWSize = iThumbWidth*iThumbHeight;

					if(!m_bRequestedRawData)
					{
						nWrite = nexSAL_FileWrite(m_pFile, &uiCTS, sizeof(uiCTS));
						if( nWrite <= 0 ) RetVal = FALSE;
						nWrite = nexSAL_FileWrite(m_pFile, m_pRAWData, m_uRAWSize);
						if( nWrite <= 0 ) RetVal = FALSE;
					}
					else
					{
						CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
						if( pEditor )
						{
							pEditor->callbackThumb(1, m_iThumbnailTag, uiCTS, iThumbWidth, iThumbHeight, m_uiThumbnailCount, m_iMaxThumbnailCount, m_uRAWSize, (char*)m_pRAWData);
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] after callbackThumb", __LINE__);
							SAFE_RELEASE(pEditor);
							m_pRAWData = NULL;
						}
					}
				}
				else
				{
					if(!m_bRequestedRawData)
					{
						nWrite = nexSAL_FileWrite(m_pFile, &uiCTS, sizeof(uiCTS));
						if( nWrite <= 0 ) RetVal = FALSE;
						nWrite = nexSAL_FileWrite(m_pFile, pThumb, iThumbSize);
						if( nWrite <= 0 ) RetVal = FALSE;
					}
					else 
					{
						if(m_pRAWData)
						{
							nexSAL_MemFree(m_pRAWData);
						}
							
						m_pRAWData = (unsigned char*)nexSAL_MemAlloc(iThumbSize);
						m_uRAWSize = iThumbSize;
						memcpy(m_pRAWData, pThumb, iThumbSize);

						CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
						if( pEditor )
						{
							pEditor->callbackThumb(1, m_iThumbnailTag, uiCTS, iThumbWidth, iThumbHeight, m_uiThumbnailCount, m_iMaxThumbnailCount, m_uRAWSize, (char*)m_pRAWData);
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] after callbackThumb", __LINE__);
							SAFE_RELEASE(pEditor);
							m_pRAWData = NULL;
						}
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] one frame thumbnail writen size(%d) time(%d) (%d %d)", 
					__LINE__, iThumbSize, uiCTS, m_uiThumbnailCount, m_iMaxThumbnailCount);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] read thumbnail failed", __LINE__);
		}

		NXT_ThemeRenderer_ReleaseContext(m_hThumbRenderer, 1);
	}
	return RetVal;
}
	
NXBOOL CThumbnail::setSeekTableInfo(int iSeekTableCount, unsigned int* pSeekTable)
{
	if( iSeekTableCount <= 0 || pSeekTable == NULL )
		return FALSE;
	
	if( m_pSeekTable )
	{
		nexSAL_MemFree(m_pSeekTable);
		m_pSeekTable = NULL;
		m_uSeekTableCount = 0;
	}

	m_pSeekTable = (unsigned int*)nexSAL_MemAlloc(sizeof(unsigned int)*iSeekTableCount);
	if( m_pSeekTable == NULL )
	{
		return FALSE;
	}

#if 0	
	memcpy(m_pSeekTable, pSeekTable, sizeof(unsigned int)*iSeekTableCount);
#else
	for(int i = 0; i < iSeekTableCount; i++)
	{
		m_pSeekTable[i] = makeBigendian((unsigned int)pSeekTable[i]);
	}
#endif
	m_uSeekTableCount = iSeekTableCount;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setSeekTableInfo(Count %d)", __LINE__, iSeekTableCount);
	return TRUE;
}

NXBOOL CThumbnail::setPCMTableInfo(int iPCMTableCount, unsigned char* pPCMTable)
{
	if( iPCMTableCount <= 0 || pPCMTable == NULL )
		return FALSE;
	
#if 0    
	if( m_pPCMTable )
	{
		nexSAL_MemFree(m_pPCMTable);
		m_pPCMTable = NULL;
		m_uPCMTableCount = 0;
	}
#endif

	if(m_pPCMTable == NULL)
	{
		m_pPCMTable = (unsigned char*)nexSAL_MemAlloc(sizeof(unsigned char)*iPCMTableCount*5);
		if( m_pPCMTable == NULL )
		{
			return FALSE;
		}

		memcpy(m_pPCMTable, pPCMTable, sizeof(unsigned char)*iPCMTableCount);
		m_uPCMTableCount = iPCMTableCount;
	}
	else
	{
		memcpy(m_pPCMTable + m_uPCMTableCount, pPCMTable, sizeof(unsigned char)*iPCMTableCount);
		m_uPCMTableCount += iPCMTableCount;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setPCMTableInfo(Count %d)", __LINE__, iPCMTableCount);
	return TRUE;
}
#if 0
NXBOOL CThumbnail::setLargeThumbnail(unsigned char* pY, unsigned char* pU, unsigned char* pV)
{
	if( pY == NULL || pU == NULL || pV == NULL)
		return FALSE;

	
	if( m_pLargeThumbnail )
	{
		nexSAL_MemFree(m_pLargeThumbnail);
		m_pLargeThumbnail	= NULL;
		m_iLargeThumbnailSize	= 0;
	}

	int iLargeWidth	= m_iHexWidth;
	int iLargeHeight	= m_iHexHeight;
	int iLargePitch	= m_iHexPitch;

	if( iLargePitch > 640 || iLargeHeight > 640)
	{
		iLargeWidth /= 2;
		iLargeHeight /= 2;
		iLargePitch /= 2;
	}
	else
	{
		// for small contents about mantis 5491
		int iBufferSize = iLargeWidth * iLargeHeight * 2;
		
		m_pLargeThumbnail = (unsigned char*)nexSAL_MemAlloc(iBufferSize);
		if( m_pLargeThumbnail == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setLargeThumbnail failed(Width :%d Height : %d size : %d)", __LINE__, iLargeWidth, iLargeHeight, iBufferSize);
			return FALSE;
		}

		NXCC_YUV420ToRGB565_EX((unsigned short *)m_pLargeThumbnail, pY, pU, pV, iLargeWidth, iLargeHeight, iLargePitch, iLargeWidth);
		
		m_iLargeThumbnailSize		= iBufferSize;
		m_iLargeThumbnailWidth	= iLargeWidth;
		m_iLargeThumbnailHeight	= iLargeHeight;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setLargeThumbnail(Width :%d Height : %d size : %d)", __LINE__, m_iHexWidth, m_iHexHeight, m_iLargeThumbnailSize);
		return TRUE;
	}

	int iBufferSize = iLargeWidth * iLargeHeight * 2;
	
	NxVScalerStruct*	hScaler = FUNC_ARM(NxVScalerInit)(iLargeWidth, iLargeHeight, iLargePitch, m_iHexWidth, m_iHexHeight, m_iHexPitch);
	if( hScaler == NULL )
		return FALSE;

	unsigned char* pLargeScaleTemp = (unsigned char*)nexSAL_MemAlloc(iLargePitch * iLargeHeight *3/2);
	if( pLargeScaleTemp == NULL )
	{
		FUNC_ARM(NxVScalerClose)(hScaler);
		return FALSE;
	}

	unsigned char* pLargeTempY = pLargeScaleTemp;
	unsigned char* pLargeTempU = pLargeTempY + (iLargePitch * iLargeHeight);
	unsigned char* pLargeTempV = pLargeTempU + (iLargePitch * iLargeHeight / 4);
	

	m_pLargeThumbnail = (unsigned char*)nexSAL_MemAlloc(iBufferSize);
	if( m_pLargeThumbnail == NULL )
	{
		nexSAL_MemFree(pLargeScaleTemp);
		FUNC_ARM(NxVScalerClose)(hScaler);
		return FALSE;
	}

	FUNC_ARM(NxVScalerResizeYUV)(hScaler, pLargeTempY , pLargeTempU, pLargeTempV, m_pHexY, m_pHexU, m_pHexV);
	FUNC_ARM(NxVScalerClose)(hScaler);
	
	NXCC_YUV420ToRGB565_EX((unsigned short *)m_pLargeThumbnail, pLargeTempY, pLargeTempU, pLargeTempV, iLargeWidth, iLargeHeight, iLargePitch, iLargeWidth);
	nexSAL_MemFree(pLargeScaleTemp);
	
	m_iLargeThumbnailSize		= iBufferSize;
	m_iLargeThumbnailWidth	= iLargeWidth;
	m_iLargeThumbnailHeight	= iLargeHeight;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setLargeThumbnail(Width :%d Height : %d size : %d)", __LINE__, m_iHexWidth, m_iHexHeight, m_iLargeThumbnailSize);
	return TRUE;
}
#endif
unsigned int CThumbnail::getThumbnailcount()
{
	return m_uiThumbnailCount;
}

int CThumbnail::getMaxThumbnailCount()
{
	return m_iMaxThumbnailCount;
}

int CThumbnail::getThumbnailFlag()
{
	return m_iThumbnailFlag;
}

NXBOOL CThumbnail::deinit()
{
	if( m_pTempY )
	{
		nexSAL_MemFree(m_pTempY);
		m_pTempY = NULL;
	}

	if( m_pTempU )
	{
		nexSAL_MemFree(m_pTempU);
		m_pTempU = NULL;
	}

	if( m_pTempV )
	{
		nexSAL_MemFree(m_pTempV);
		m_pTempV = NULL;
	}

	if( m_pHexY )
	{
		nexSAL_MemFree(m_pHexY);
		m_pHexY = NULL;
	}
	if( m_pHexU )
	{
		nexSAL_MemFree(m_pHexU);
		m_pHexU = NULL;
	}
	if( m_pHexV )
	{
		nexSAL_MemFree(m_pHexV);
		m_pHexV = NULL;
	}

	if( m_pRGBBuffer )
	{
		nexSAL_MemFree(m_pRGBBuffer);
		m_pRGBBuffer = NULL;
	}

	if( m_pScaler )
	{
		FUNC_ARM(NxVScalerClose)(m_pScaler);
		m_pScaler = NULL;
	}

	if( m_eThumbMode == THUMBNAIL_MODE_VIDEO )
	{
		if( m_pFile != NEXSAL_INVALID_HANDLE)
		{
			nexSAL_FileClose(m_pFile);
			m_pFile = NEXSAL_INVALID_HANDLE;		
		}		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] close video thumbnail ", __LINE__);

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
	}
	else if( m_eThumbMode == THUMBNAIL_MODE_AUDIO )
	{
		if( m_pFile != NEXSAL_INVALID_HANDLE)
		{
			if( m_uPCMTableCount > 0 && m_pPCMTable )
			{
				nexSAL_FileWrite(m_pFile, m_pPCMTable, sizeof(unsigned char)*m_uPCMTableCount);
			}
			

			nexSAL_FileClose(m_pFile);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] close Audio thumbnail (Count:%d)", __LINE__, m_uPCMTableCount);
			m_pFile = NEXSAL_INVALID_HANDLE;		
		}

		if(m_bRequestedRawData)
		{
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor )
			{
				pEditor->callbackThumb(0, m_iThumbnailTag, 0, 0, 0, 0, 0, m_uPCMSize, (char*)m_pPCMData);
				SAFE_RELEASE(pEditor);
				m_pPCMData = NULL;
			}
		}
	}
	else
	{
		if( m_pFile != NEXSAL_INVALID_HANDLE )
		{
			THUMBNAIL thumbnail;

			thumbnail.m_iDuration				= makeBigendian((unsigned int)m_iDuration);
			thumbnail.m_isVideo				= makeBigendian((unsigned int)m_isVideo);
			thumbnail.m_isAudio				= makeBigendian((unsigned int)m_isAudio);
			thumbnail.m_iVideoWidth			= makeBigendian((unsigned int)m_iSrcWidth);
			thumbnail.m_iVideoHeight			= makeBigendian((unsigned int)m_iSrcHeight);
			thumbnail.m_iThumbnailWidth		= makeBigendian((unsigned int)m_iDstWidth);
			thumbnail.m_iThumbnailHeight		= makeBigendian((unsigned int)m_iDstHeight);

			int iseektalbpos = m_uiCurrentFilePos;
			if( m_uSeekTableCount > 0 && m_pSeekTable )
			{
				thumbnail.m_iSeekTablePos = makeBigendian((unsigned int)m_uiCurrentFilePos);
				thumbnail.m_iSeekTableCount = makeBigendian((unsigned int)m_uSeekTableCount);
				
				nexSAL_FileWrite(m_pFile, m_pSeekTable, sizeof(unsigned int)*m_uSeekTableCount);
				m_uiCurrentFilePos += sizeof(unsigned int)*m_uSeekTableCount;
			}
			else
			{
				thumbnail.m_iSeekTablePos = makeBigendian((unsigned int)m_uiCurrentFilePos);
				thumbnail.m_iSeekTableCount = 0;
			}

			int ipcmlevelpos = m_uiCurrentFilePos;
			if( m_uPCMTableCount > 0 && m_pPCMTable )
			{
				thumbnail.m_iPCMLevelPos = makeBigendian((unsigned int)m_uiCurrentFilePos);
				thumbnail.m_iPCMLevelCount = makeBigendian((unsigned int)m_uPCMTableCount);
				
				nexSAL_FileWrite(m_pFile, m_pPCMTable, sizeof(unsigned char)*m_uPCMTableCount);
				m_uiCurrentFilePos += sizeof(unsigned char)*m_uPCMTableCount;
			}
			else
			{
				thumbnail.m_iPCMLevelPos = makeBigendian((unsigned int)m_uiCurrentFilePos);
				thumbnail.m_iPCMLevelCount = 0;
			}

			int ilargeThumbpos = m_uiCurrentFilePos;
			if( m_iLargeThumbnailSize > 0 && m_pLargeThumbnail )
			{
				thumbnail.m_iLargeThumbWidth	= makeBigendian((unsigned int)m_iLargeThumbnailWidth);
				thumbnail.m_iLargeThumbHeight	= makeBigendian((unsigned int)m_iLargeThumbnailHeight);
				thumbnail.m_iLargeThumbPos	= makeBigendian((unsigned int)m_uiCurrentFilePos);
				thumbnail.m_iLargeThumbSize	= makeBigendian((unsigned int)m_iLargeThumbnailSize);
				
				nexSAL_FileWrite(m_pFile, m_pLargeThumbnail, sizeof(unsigned char)*m_iLargeThumbnailSize);
				m_uiCurrentFilePos += sizeof(unsigned char)*m_iLargeThumbnailSize;
			}
			else
			{
				thumbnail.m_iLargeThumbWidth	= 0;
				thumbnail.m_iLargeThumbHeight	= 0;
				thumbnail.m_iLargeThumbPos	= makeBigendian((unsigned int)m_uiCurrentFilePos);
				thumbnail.m_iLargeThumbSize	= 0;
			}		
		
			thumbnail.m_iThumbnailInfoCount	= makeBigendian((unsigned int)m_uiThumbnailCount);
			thumbnail.m_iThumbnailInfoPos		= makeBigendian((unsigned int)m_uiCurrentFilePos);
			
			nexSAL_FileWrite(m_pFile, m_pThumbnailInfo, sizeof(THUMBNAILINFO)*m_uiThumbnailCount);
			nexSAL_FileWrite(m_pFile, &thumbnail, sizeof(THUMBNAIL));

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Thumbnail Info", __LINE__);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Audio Exist : %d", __LINE__, m_isAudio);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Video Exist : %d", __LINE__, m_isVideo);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Clip Duration : %d", __LINE__, m_iDuration);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Width : %d", __LINE__, m_iSrcWidth);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Height : %d", __LINE__, m_iSrcHeight);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Thumb Width : %d", __LINE__, m_iDstWidth);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Thumb Height : %d", __LINE__, m_iDstHeight);
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t SeekTablePos : %d", __LINE__, iseektalbpos);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t SeekTableCount : %d", __LINE__, m_uSeekTableCount);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t PCM Level Pos : %d", __LINE__, ipcmlevelpos);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t PCM Level Count : %d", __LINE__, m_uPCMTableCount);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Large Thumbnail width : %d", __LINE__, m_iLargeThumbnailWidth);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Large Thumbnail height : %d", __LINE__, m_iLargeThumbnailHeight);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Large Thumbnail Pos : %d", __LINE__, ilargeThumbpos);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Large Thumbnail Size : %d", __LINE__, m_iLargeThumbnailSize);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Thumb Pos : %d", __LINE__, m_uiCurrentFilePos);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] \t Thumb Count : %d", __LINE__, m_uiThumbnailCount);
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Write Thumbnail index End", __LINE__);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] Total Thumbnail Count(%d)", __LINE__, m_uiThumbnailCount);
			
			nexSAL_FileClose(m_pFile);
			m_pFile = NEXSAL_INVALID_HANDLE;
		}
	}

	if( m_pSeekTable )
	{
		nexSAL_MemFree(m_pSeekTable);
		m_pSeekTable = NULL;
	}

	if( m_pPCMTable )
	{
		nexSAL_MemFree(m_pPCMTable);
		m_pPCMTable = NULL;
	}

	if( m_pLargeThumbnail )
	{
		nexSAL_MemFree(m_pLargeThumbnail);
		m_pLargeThumbnail = NULL;
	}

	if( m_pThumbnailInfo )
	{
		nexSAL_MemFree(m_pThumbnailInfo);
		m_pThumbnailInfo = NULL;
	}

#ifdef YUV_TEST_DUMP
	if( m_pDumpYUV )
	{
		fclose(m_pDumpYUV);
		m_pDumpYUV = NULL;
	}
#endif

	if(m_pRAWData)
	{
		nexSAL_MemFree(m_pRAWData);
		m_pRAWData = NULL;
	}

	if(m_pPCMData)
	{
		nexSAL_MemFree(m_pPCMData);
		m_pPCMData = NULL;
	}

	if(m_strPath)
	{
		nexSAL_MemFree(m_strPath);
		m_strPath = NULL;
	}
	return TRUE;
}

void CThumbnail::setStartEndTime(unsigned int uStartTime, unsigned int uEndTime)
{
	m_uStartTime = uStartTime;
	m_uEndTime = uEndTime;
}

void CThumbnail::getStartEndTime(unsigned int *puStartTime, unsigned int *puEndTime)
{
	*puStartTime = m_uStartTime;
	*puEndTime = m_uEndTime;	
}

NXBOOL CThumbnail::getRequestRawData()
{
	if( (m_iThumbnailFlag & GET_THUMBRAW_RGB_CALLBACK) == GET_THUMBRAW_RGB_CALLBACK )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] getRequestRawData return false because callback mode", __LINE__);
		return FALSE;
	}
	
	return m_bRequestedRawData;
}

void CThumbnail::setAudioPCMSize(unsigned int uSize)
{
	if(m_pPCMData)
		nexSAL_MemFree(m_pPCMData);

	m_pPCMData = (unsigned char*)nexSAL_MemAlloc(uSize);
}

NXBOOL CThumbnail::addAudioPCM(unsigned int uSize, unsigned char* pData)
{
	if(!m_pPCMData)
		return FALSE;

	memcpy(m_pPCMData, pData, uSize);
	m_uPCMSize += uSize;

	return TRUE;
}

int* CThumbnail::getTimeTable(int* piTimeTableCount)
{
	*piTimeTableCount = m_iTimeTableCount;
	return m_pTimeTable;
}

int CThumbnail::getThumbTimeTableCount()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] getThumbTimeTableCount(%zu)", __LINE__, m_vecThumbTime.size());
	return m_vecThumbTime.size();
}

int CThumbnail::getThumbTimeTable(int iIndex)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] getThumbTimeTable In(%d)", __LINE__, iIndex);
	if( iIndex < 0 || iIndex >= m_vecThumbTime.size() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] getThumbTimeTable failed(%d %zu)", __LINE__, iIndex, m_vecThumbTime.size());
		return -1;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] getThumbTimeTable Out(%d %p)", __LINE__, iIndex, &m_vecThumbTime[iIndex]);
	return m_vecThumbTime[iIndex];
}

NXBOOL CThumbnail::setThumbTimeTable(int iSize, int* pTable)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setThumbTimeTable In(%d %p)", __LINE__, iSize, pTable);
	if( iSize <= 0 || pTable == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setThumbTimeTable failed because param error", __LINE__);
		return FALSE;
    }

	m_iMaxThumbnailCount = iSize;
	m_vecThumbTime.clear();

	for(int i = 0; i < iSize; i++)
    {
		m_vecThumbTime.insert(m_vecThumbTime.end(), pTable[i]);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] setThumbTimeTable Out(%zu)", __LINE__, m_vecThumbTime.size());
    return TRUE;
}

NXBOOL CThumbnail::needSeekNextTimeTableItem(unsigned int uiDecOutTime, unsigned int uiReaderTime, int iNextTime)
{
	if( uiReaderTime > iNextTime )
	{
		if( uiDecOutTime > iNextTime )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] isSeekOrNotNextTimeTableItem need seek(%d %d %d)", 
				__LINE__, uiDecOutTime, uiReaderTime, iNextTime);
			return TRUE;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] isSeekOrNotNextTimeTableItem need more dec(%d %d %d)", 
			__LINE__, uiDecOutTime, uiReaderTime, iNextTime);
		return FALSE;
	}

	if( m_pSeekTable == NULL || m_uSeekTableCount <= 0 )
	{
		if( uiReaderTime + 1000 < iNextTime )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] isSeekOrNotNextTimeTableItem need seek(%d %d %d)", 
				__LINE__, uiDecOutTime, uiReaderTime, iNextTime);
			return TRUE;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] isSeekOrNotNextTimeTableItem need more dec(%d %d %d)", 
			__LINE__, uiDecOutTime, uiReaderTime, iNextTime);
		return FALSE;
	}

	int iPreSeekTime = 0;
	int iNextSeekTime = 0;
	for( int i = 0; i < m_uSeekTableCount; i++)
	{
		if( m_pSeekTable[i] < iNextTime )
		{
			iPreSeekTime = m_pSeekTable[i];
			continue;
		}

		iNextSeekTime = m_pSeekTable[i];
		break;
	}

	if( uiReaderTime < iPreSeekTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] isSeekOrNotNextTimeTableItem need seek(%d %d %d)", 
			__LINE__, uiReaderTime, iPreSeekTime, iNextTime);
		return TRUE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Thumbnail.cpp %d] isSeekOrNotNextTimeTableItem need more dec(%d %d %d)", 
		__LINE__, uiReaderTime, iPreSeekTime, iNextTime);
	return FALSE;
}

void CThumbnail::setCodecWrap(CNexCodecWrap* pCodec)
{
	SAFE_RELEASE(m_pCodecWrap);
	m_pCodecWrap = pCodec;
	SAFE_ADDREF(m_pCodecWrap);
}

#if defined(_ANDROID)
void CThumbnail::callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender)
{
	if( m_pCodecWrap != NULL )
		m_pCodecWrap->postFrameRenderCallback(pBuff, bRender);
}
#elif defined(__APPLE__)
void CThumbnail::callCodecWrap_renderDecodedFrame(unsigned char* pBuff)
{
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->renderDecodedFrame(pBuff);
    }
}

void CThumbnail::callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender)
{
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->releaseDecodedFrame(pBuff, isAfterRender);
    }
}
#endif
