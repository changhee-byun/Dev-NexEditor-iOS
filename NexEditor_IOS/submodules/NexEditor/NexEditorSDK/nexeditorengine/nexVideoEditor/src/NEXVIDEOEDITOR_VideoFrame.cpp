/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoFrame.cpp
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

#include "NEXVIDEOEDITOR_VideoFrame.h"
#include "RGB_To_YUV420.h"
#include "NexCAL.h"

#include "NEXVIDEOEDITOR_Def.h"
#ifdef ANDROID
#include "NexThemeRenderer_SurfaceTexture.h"
#include "NEXVIDEOEDITOR_PerformanceChecker.h"
#endif
#ifdef __APPLE__
#include "NexThemeRenderer_Platform_iOS-CAPI.h"
#endif

#include "NEXVIDEOEDITOR_VideoEditor.h"

// #define IMAGE_YUV_DUMP

#ifdef VIDEO_FRAME_DUMP
#include "nexNV12TileToNV12.h"
#endif

CFrameInfo::CFrameInfo() : m_performanceMonitor((char*)"UpdateTexImage"), m_perfWaitFrame((char*)"WaitFrame")
{
	m_uiTime					= 0;
	
	m_uiGraphicBufferPitch			= 0;
	m_uiGraphicBufferAlignHeight	= 0;
	
	m_pY						= NULL;
	m_pU						= NULL;
	m_pV						= NULL;

	m_pMediaBuff				= NULL;
#ifdef __APPLE__
    m_pImageBuff                = NULL;
#endif
	m_bRenderMediaBuff 			= FALSE;

	m_isUsed					= FALSE;

	m_iTextureID				= -1;

	m_iImageFrameUploadFlag		= 0;

	m_iFaceDetectFlag			= 0;

	m_pSurfaceTexture			= NULL;

	mByteCount					= 0;
	mRGB						= NULL;

	mImageY						= NULL;
	mImageU						= NULL;
	mImageV						= NULL;

	mImageNV12					= NULL;
	// mantis 6883 variable init.
	mIsNV12Jepg					= FALSE;

	memset(&mSrc, 0x00, sizeof(RECT));
	memset(&mStart, 0x00, sizeof(RECT));
	memset(&mEnd, 0x00, sizeof(RECT));

	memcpy(mStartMatrix, default_matrix, sizeof(float)*MATRIX_MAX_COUNT);
	memcpy(mEndMatrix, default_matrix, sizeof(float)*MATRIX_MAX_COUNT);

	m_iFeatureVersion			= 0;
	m_iVideoLayerTextureID		= 0;

#ifdef VIDEO_FRAME_DUMP
	m_pVIDEOFRAMEInFile			= NULL;
	m_pVideoFrameBuffer			= NULL;
	m_iDumpFrameCount			= 0;
#endif
	
	m_pCodecWrap				= NULL;
    mVideoRenderMode = 0; //yoon
    mHDRType = 0; //yoon
    memset(&mHdrInfo,0,sizeof(NEXCODECUTIL_SEI_HDR_INFO));
	m_uiDropRenderTime			= DROP_RENDER_TIME_EXPORT;
	m_ClipVideoRenderInfoVec.clear();
	m_bPreviewThumb = 0;
	m_uLogCnt = 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] Create CFrameInfo(this:%p)", __LINE__, this);
}

CFrameInfo::~CFrameInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] ~~~~~~~~CFrameInfo(this:%p %p %p %p)", __LINE__, this, m_pY, m_pU, m_pV);
	deinitFrameInfo();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] ~~~~~~~~CFrameInfo", __LINE__);	

#ifdef VIDEO_FRAME_DUMP
	if( m_pVIDEOFRAMEInFile )
	{
		fclose(m_pVIDEOFRAMEInFile);
		m_pVIDEOFRAMEInFile = NULL;
	}
	if( m_pVideoFrameBuffer )
	{
		nexSAL_MemFree( m_pVideoFrameBuffer );
		m_pVideoFrameBuffer = NULL;
	}
#endif
	clearClipRenderInfoVec();

	SAFE_RELEASE(m_pCodecWrap);
}

void CFrameInfo::setCodecWrap(CNexCodecWrap* pCodecWrap)
{
	CAutoLock m(m_FrameLock);
	SAFE_RELEASE(m_pCodecWrap);
	m_pCodecWrap = pCodecWrap;
	SAFE_ADDREF(m_pCodecWrap);
}

#if defined(_ANDROID)
void CFrameInfo::callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender)
{
	if( m_pCodecWrap != NULL )
	{
		m_pCodecWrap->postFrameRenderCallback(pBuff, bRender);
	}
}
#elif defined(__APPLE__)
void CFrameInfo::callCodecWrap_renderDecodedFrame(unsigned char* pBuff)
{
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->renderDecodedFrame(pBuff);
    }
}

void CFrameInfo::callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender)
{
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->releaseDecodedFrame(pBuff, isAfterRender);
    }
}
#endif
NXBOOL CFrameInfo::initClipRenderInfo(CClipVideoRenderInfo* pClipRenderInfo)
{
	if( pClipRenderInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] initClipRenderInfo is failed because param was null(%p)", __LINE__, pClipRenderInfo);
		return FALSE;
	}

	CAutoLock m(m_FrameLock);
	
	if( m_pY )
		nexSAL_MemFree(m_pY);
	
	if( m_pU )
		nexSAL_MemFree(m_pU);

	if( m_pV )
		nexSAL_MemFree(m_pV);
	
	m_pY = NULL;
	m_pU = NULL;
	m_pV = NULL;

	m_pMediaBuff			= NULL;
#ifdef __APPLE__
    m_pImageBuff			= NULL;
#endif
    m_bRenderMediaBuff 	= FALSE;

	muiTrackID		= pClipRenderInfo->muiTrackID;

	mClipType		= pClipRenderInfo->mClipType;

	mStartTime		= pClipRenderInfo->mStartTime;
	mEndTime		= pClipRenderInfo->mEndTime;

	mRotateState	= pClipRenderInfo->mRotateState;
	mBrightness		= pClipRenderInfo->mBrightness;
	mContrast		= pClipRenderInfo->mContrast;
	mSaturation		= pClipRenderInfo->mSaturation;
    mHue            = pClipRenderInfo->mHue;
	mTintcolor		= pClipRenderInfo->mTintcolor;
	mLUT 			= pClipRenderInfo->mLUT;
	mCustomLUT_A	= pClipRenderInfo->mCustomLUT_A;
	mCustomLUT_B	= pClipRenderInfo->mCustomLUT_B;
	mCustomLUT_Power	= pClipRenderInfo->mCustomLUT_Power;
	mVignette		= pClipRenderInfo->mVignette;

   	mWidth			= pClipRenderInfo->mWidth;
	mHeight			= pClipRenderInfo->mHeight;
	mPitch			= pClipRenderInfo->mPitch;

	memcpy(mStartMatrix, pClipRenderInfo->mStartMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	memcpy(mEndMatrix, pClipRenderInfo->mEndMatrix, sizeof(float)*MATRIX_MAX_COUNT);

	if( mClipType == CLIPTYPE_IMAGE )
	{
		mSrc.left			= pClipRenderInfo->mSrc.left;
		mSrc.top			= pClipRenderInfo->mSrc.top;
		mSrc.right			= pClipRenderInfo->mSrc.right;
		mSrc.bottom			= pClipRenderInfo->mSrc.bottom;

		mStart.left			= pClipRenderInfo->mStart.left;
		mStart.top			= pClipRenderInfo->mStart.top;
		mStart.right		= pClipRenderInfo->mStart.right;
		mStart.bottom		= pClipRenderInfo->mStart.bottom;

		mEnd.left			= pClipRenderInfo->mEnd.left;
		mEnd.top			= pClipRenderInfo->mEnd.top;
		mEnd.right			= pClipRenderInfo->mEnd.right;
		mEnd.bottom			= pClipRenderInfo->mEnd.bottom;

		mFace.left	 		= pClipRenderInfo->mFace.left;
		mFace.top	 		= pClipRenderInfo->mFace.top;
		mFace.right 		= pClipRenderInfo->mFace.right;
		mFace.bottom 		= pClipRenderInfo->mFace.bottom;

		m_isUsed			= TRUE;

		if( pClipRenderInfo->mImageY && pClipRenderInfo->mImageU && pClipRenderInfo->mImageV )
		{
			mColorFormat		= NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
			mIsNV12Jepg			= pClipRenderInfo->mIsNV12Jepg;

			m_pY = (unsigned char*)nexSAL_MemAlloc(mWidth * mHeight * 3 / 2);
			if( m_pY == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] initClipRenderInfo malloc fail", __LINE__);
				deinitFrameInfo();
				return FALSE;
			}

			unsigned char* pY = m_pY;
			unsigned char* pUV = m_pY + (mWidth*mHeight);

			unsigned char* pSrcY = pClipRenderInfo->mImageY;
			unsigned char* pSrcU = pClipRenderInfo->mImageU;
			unsigned char* pSrcV = pClipRenderInfo->mImageV;
			
			for(int y = 0; y < mHeight; y++)
			{
				memcpy(pY, pSrcY, mWidth);
				pY += mWidth;
				pSrcY += mPitch;
			}

			for(int y = 0; y < mHeight/2; y++)
			{
				for(int x = 0; x<mWidth/2; x++)
				{
					*(pUV) 		= *(pSrcU + x);
					*(pUV+1) 	= *(pSrcV + x);
					pUV += 2;
				}
				pSrcU += mPitch/2;
				pSrcV += mPitch/2;
			}
			mPitch = mWidth;
		}
		else if( pClipRenderInfo->mRGB )
		{
#if 1
			mColorFormat		= NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
			mIsNV12Jepg			= pClipRenderInfo->mIsNV12Jepg;

			m_pY = (unsigned char*)nexSAL_MemAlloc(mWidth * mHeight * 3 / 2);
			if( m_pY == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] initClipRenderInfo malloc fail", __LINE__);
				deinitFrameInfo();
				return FALSE;
			}

			if( NXCC_RGBToYUV420Semi(m_pY, pClipRenderInfo->mRGB, mWidth, mHeight, pClipRenderInfo->mByteCount) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] initClipRenderInfo Color convert Error", __LINE__);
				deinitFrameInfo();
				return FALSE;
			}
#else
			mColorFormat		= NEXCAL_PROPERTY_VIDEO_BUFFER_RGB888;
			mIsNV12Jepg			= pClipRenderInfo->mIsNV12Jepg;

			m_pY = (unsigned char*)nexSAL_MemAlloc(mWidth * mHeight * pClipRenderInfo->mByteCount);
			if( m_pY == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] initClipRenderInfo malloc fail", __LINE__);
				deinitFrameInfo();
				return FALSE;
			}

			memcpy(m_pY, pClipRenderInfo->mRGB, mWidth*mHeight*pClipRenderInfo->mByteCount);
#endif
		}
		else if( pClipRenderInfo->mImageNV12 )
		{
			mColorFormat		= NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
			mIsNV12Jepg			= pClipRenderInfo->mIsNV12Jepg;

			m_pY = (unsigned char*)nexSAL_MemAlloc(mWidth * mHeight * 3 / 2);
			if( m_pY == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] initClipRenderInfo malloc fail", __LINE__);
				deinitFrameInfo();
				return FALSE;
			}

			memcpy(m_pY, pClipRenderInfo->mImageNV12, mWidth*mHeight*3/2);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] not se image date", __LINE__);
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] initImageInfo Image frame(%d %d)", __LINE__, mWidth, mHeight);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] initImageInfo Start(%d %d %d %d) End(%d %d %d %d)", 
			__LINE__, mStart.left, mStart.top, mStart.right, mStart.bottom, mEnd.left, mEnd.top, mEnd.right, mEnd.bottom);
	}
	else if( mClipType == CLIPTYPE_VIDEO || mClipType == CLIPTYPE_VIDEO_LAYER )
	{
		mIsPreview		= pClipRenderInfo->mIsPreview;
		mColorFormat		= pClipRenderInfo->mColorFormat;
		mVideoRenderMode	= pClipRenderInfo->mVideoRenderMode; //yoon
		mHDRType	= pClipRenderInfo->mHDRType; //yoon
		memcpy(&mHdrInfo,&pClipRenderInfo->mHdrInfo,sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon
		
		mSrc.left			= 0;
		mSrc.top			= 0;
		mSrc.right		= mWidth;
		mSrc.bottom		= mHeight;

		mStart.left		= pClipRenderInfo->mStart.left;
		mStart.top		= pClipRenderInfo->mStart.top;
		mStart.right		= pClipRenderInfo->mStart.right;
		mStart.bottom		= pClipRenderInfo->mStart.bottom;

		mEnd.left			= pClipRenderInfo->mEnd.left;
		mEnd.top			= pClipRenderInfo->mEnd.top;
		mEnd.right		= pClipRenderInfo->mEnd.right;
		mEnd.bottom		= pClipRenderInfo->mEnd.bottom;		
		
		m_isUsed				= FALSE;
	
		switch(mColorFormat)
		{
			case NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY:
				m_pMediaBuff					= NULL;
#ifdef __APPLE__
                m_pImageBuff                = NULL;
#endif
                
				m_bRenderMediaBuff 			= FALSE;
				
#ifdef VIDEO_FRAME_DUMP
				{
					char strFilePath[256];
					sprintf(strFilePath, "/sdcard/H264Frame_ID_%d_0x%x_%d.yuv", muiTrackID, this, nexSAL_GetTickCount());
					m_pVIDEOFRAMEInFile = fopen(strFilePath, "wb");
					m_iDumpFrameCount = 0;

					if( m_pVideoFrameBuffer )
					{
						nexSAL_MemFree( m_pVideoFrameBuffer );
						m_pVideoFrameBuffer = NULL;
					}

					m_pVideoFrameBuffer = (unsigned char*)nexSAL_MemAlloc(mPitch * mHeight * 3);
				}
#endif				
				break;
			case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
				m_pY				= (unsigned char*)nexSAL_MemAlloc(mPitch * mHeight * 3 / 2);
				m_pU				= (unsigned char*)nexSAL_MemAlloc(mPitch * mHeight /4);
				m_pV				= (unsigned char*)nexSAL_MemAlloc(mPitch * mHeight /4);
				if( m_pY == NULL || m_pU == NULL || m_pV == NULL )
				{
					deinitFrameInfo();
					return FALSE;
				}
				memset(m_pY, 0x00, mPitch * mHeight * 3 / 2);		
				memset(m_pU, 0x00, mPitch * mHeight /4);
				memset(m_pV, 0x00, mPitch * mHeight /4);
				break;
			case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
				m_pY				= (unsigned char*)nexSAL_MemAlloc(mPitch * mHeight * 3 / 2);
				if( m_pY == NULL )
				{
					deinitFrameInfo();
					return FALSE;
				}
				memset(m_pY, 0x00, mPitch * mHeight * 3 / 2);
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] initVideoFrameInfo failed because unknow color format", __LINE__);
				break;
		};

		m_performanceMonitor.CheckModuleStart();
        m_perfWaitFrame.CheckModuleStart();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] initVideoInfo(%p %p %p %d %d %d)", __LINE__, m_pY, m_pU, m_pV, mWidth, mHeight, mPitch);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[VFrame.cpp %d] initClipRenderInfo is failed because did not support clip type(%d)", __LINE__, mClipType);
		return FALSE;
	}

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor != NULL ) 
	{
		m_iFeatureVersion = pEditor->getPropertyInt("FeatureVersion", 0);
		SAFE_RELEASE(pEditor);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] initClipRenderInfo is sucess clip type(%d) FeatureVersion(%d)", __LINE__, mClipType, m_iFeatureVersion);
	return TRUE;
}

NXBOOL CFrameInfo::clearClipRenderInfoVec()
{
	CAutoLock m(m_FrameLock);
	for (int i = 0; i < m_ClipVideoRenderInfoVec.size(); i++ ) {
		m_ClipVideoRenderInfoVec[i]->Release();
	}
	m_ClipVideoRenderInfoVec.clear();
	return TRUE;
}

NXBOOL CFrameInfo::setClipRenderInfoVec(CClipVideoRenderInfo* pClipRenderInfo)
{
	if( pClipRenderInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] initClipRenderInfo is failed because param was null(%p)", __LINE__, pClipRenderInfo);
		return FALSE;
	}

	CAutoLock m(m_FrameLock);
	SAFE_ADDREF(pClipRenderInfo);
	m_ClipVideoRenderInfoVec.insert(m_ClipVideoRenderInfoVec.end(), pClipRenderInfo);
	
	return TRUE;
}

NXBOOL CFrameInfo::setSurfaceTexture(void* pSurface)
{
	m_pSurfaceTexture = pSurface;
	return TRUE;
}

void CFrameInfo::deinitFrameInfo()
{
	if( m_pY )
	{
		nexSAL_MemFree(m_pY);
		m_pY = NULL;
	}
	if( m_pU )
	{
		nexSAL_MemFree(m_pU);
		m_pU = NULL;
	}
	if( m_pV )
	{
		nexSAL_MemFree(m_pV);
		m_pV = NULL;
	}

	m_uiTime			= 0;
	mStartTime			= 0;
	mEndTime			= 0;
	

	mWidth				= 0;
	mHeight				= 0;
	mPitch				= 0;

	m_isUsed			= FALSE;

	mClipType			= CLIPTYPE_NONE;
	m_iTextureID		= -1;
	mRotateState		= 0;
  	mBrightness			= 0;
 	mContrast			= 0;
 	mSaturation			= 0;
    mHue                = 0;
 	mTintcolor			= 0;
 	mLUT 				= 0;
 	mVignette			= 0;
    mVideoRenderMode = 0; //yoon
        mHDRType = 0; //yoon
    memset(&mHdrInfo,0,sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon

	memset(&mSrc, 0x00, sizeof(RECT));
	memset(&mStart, 0x00, sizeof(RECT));
	memset(&mEnd, 0x00, sizeof(RECT));
	memset(&mFace, 0x00, sizeof(RECT));
}

NXBOOL CFrameInfo::setVideoFrame(unsigned int uiTime, unsigned char* pY, unsigned char* pU, unsigned char* pV)
{
	switch(mColorFormat)
	{
		case NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY:
			{

#if defined(ANDROID)
#ifdef FOR_TEST_MEDIACODEC_DEC
				m_pMediaBuff					= pY + 1;
				m_bRenderMediaBuff 			= FALSE;
#else
				m_pMediaBuff					= pY;
				m_bRenderMediaBuff 			= FALSE;
#endif
#elif defined(__APPLE__)
                m_pImageBuff = pY;
#else
#error "not supported platform."
#endif
                
#ifdef VIDEO_FRAME_DUMP
				android::MediaBuffer* pMedia = (android::MediaBuffer*)pY;
				if( pMedia)
				{
					// int iPixelFormat = pMedia->graphicBuffer()->getPixelFormat();
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] Graphic Buffer Format(0x%x)", __LINE__, iPixelFormat);
					if( m_pVIDEOFRAMEInFile )
					{
						void* pBuffer = NULL;
						int iRet = (int)pMedia->graphicBuffer()->lock(GRALLOC_USAGE_SW_READ_OFTEN, &pBuffer);
						if( iRet == 0 )
						{
							unsigned int uiWidth = pMedia->graphicBuffer()->getWidth();
							unsigned int uiHeight = pMedia->graphicBuffer()->getHeight();
							unsigned int uiStride = pMedia->graphicBuffer()->getStride();
							unsigned int uiPixelFormat = (unsigned int)pMedia->graphicBuffer()->getPixelFormat();
							
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] Lock() sRet(%d) Out(%p) (%d %d %d %d)", 
								__LINE__, iRet, pBuffer, uiWidth, uiHeight, uiStride, uiPixelFormat);

							if( uiPixelFormat == 257 || uiPixelFormat == 259)
							{
								fwrite(pBuffer, sizeof(char), uiStride*uiHeight*3/2, m_pVIDEOFRAMEInFile);
							}
							else
							{
								convertNV12TiledToNV12(uiWidth, uiHeight, uiStride, uiHeight, pBuffer, m_pVideoFrameBuffer);
								fwrite(m_pVideoFrameBuffer, sizeof(char), uiStride*uiHeight*3/2, m_pVIDEOFRAMEInFile);
							}

							m_iDumpFrameCount++;
							if( m_iDumpFrameCount > 100 )
							{
								fclose(m_pVIDEOFRAMEInFile);
								m_pVIDEOFRAMEInFile = NULL;
								m_iDumpFrameCount = 0;
								nexSAL_MemFree(m_pVideoFrameBuffer);
								m_pVideoFrameBuffer = NULL;
							}
							pMedia->graphicBuffer()->unlock();
						}
						else
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] Lock() failed sRet(%d) Out(%p)", __LINE__, iRet, pBuffer);
						}
					}
				}
#endif
				break;
			}
		case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
			if( pY == NULL || pU == NULL || pV == NULL || m_pY == NULL || m_pU == NULL || m_pV == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] YUV pointer failed(%p, %p, %p, %p, %p, %p)", 
					__LINE__, pY, pU, pV , m_pY , m_pU , m_pV );
				return FALSE;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] setVideoFrame(%d %d %d)", __LINE__, mWidth, mHeight, mPitch);

			memcpy(m_pY, pY, mPitch*mHeight);
			memcpy(m_pU, pU, mPitch*mHeight/4);
			memcpy(m_pV, pV, mPitch*mHeight/4);		
			break;
		case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
			if( m_pY == NULL )
				return FALSE;
			break;
		case NEXCAL_PROPERTY_VIDEO_BUFFER_RGB888:
			if( m_pY == NULL )
				return FALSE;
			break;
		default:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] setVideoFrame failed because unknow color format", __LINE__);
			break;
	};
	m_uiTime = uiTime;
	return TRUE;
}

NXBOOL CFrameInfo::initVideoSurface(void* pDDraw, int iBitCount)
{
	return TRUE;
}

void CFrameInfo::applyPosition(void* pSurface, unsigned int uiTime, int iIndex)
{
	if( !(m_iFeatureVersion >= 3))
		return;     

	unsigned int uiDur = mEndTime - mStartTime;
	float fPer = (float)(uiTime - mStartTime) / uiDur;

	float fleft 			= calcPos((float)mStart.left, (float)mEnd.left, fPer);
	float fright 			= calcPos((float)mStart.right, (float)mEnd.right, fPer);
	
	float ftop				= calcPos((float)mStart.top, (float)mEnd.top, fPer);
	float fbottom			= calcPos((float)mStart.bottom, (float)mEnd.bottom, fPer);

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] ID(%d) Pos(S(%d %d %d %d) E(%d %d %d %d))", __LINE__, muiTrackID, mStart.left, mStart.top, mStart.right, mStart.bottom, mEnd.left, mEnd.top, mEnd.right, mEnd.bottom);

	CClipVideoRenderInfo* pRenderInfo = getActiveRenderInfo(uiTime);

	if(pRenderInfo)
	{
		uiDur = pRenderInfo->mEndTime - pRenderInfo->mStartTime;
		fPer = (float)(uiTime - pRenderInfo->mStartTime) / uiDur;

		fleft 			= calcPos((float)pRenderInfo->mStart.left, (float)pRenderInfo->mEnd.left, fPer);
		fright 			= calcPos((float)pRenderInfo->mStart.right, (float)pRenderInfo->mEnd.right, fPer);
		
		ftop 			= calcPos((float)pRenderInfo->mStart.top, (float)pRenderInfo->mEnd.top, fPer);
		fbottom			= calcPos((float)pRenderInfo->mStart.bottom, (float)pRenderInfo->mEnd.bottom, fPer);
	}
	
	if( mClipType == CLIPTYPE_VIDEO_LAYER )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, m_uLogCnt++%30, "[VFrame.cpp %d] ID(%d) SetTextureInput Rect(%.2f %.2f %.2f %.2f) for video layer fPer(%f)", __LINE__, muiTrackID, fleft, ftop, fright, fbottom, fPer);
		NXT_ThemeRenderer_SetTextureInputRect((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, fleft, ftop, fright,  fbottom);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, m_uLogCnt++%30, "[VFrame.cpp %d] ID(%d) TextureID(%d) SetTextureInput Rect(%.2f %.2f %.2f %.2f) for video frame fPer(%f)", __LINE__, muiTrackID, m_iTextureID, fleft, ftop, fright, fbottom, fPer);
		NXT_ThemeRenderer_SetTextureInputRect((NXT_HThemeRenderer )pSurface, (NXT_TextureID)((iIndex == -1)?m_iTextureID: iIndex), fleft, ftop, fright,  fbottom);
	}
}

float CFrameInfo::calcPos(float fPos1, float fPos2, float fPer)
{
	float fNewPos = 0;
	if( fPos1 < fPos2 )
	{
		fNewPos = fPos1 + ((fPos2 - fPos1) * fPer);
	}
	else
	{
		fNewPos = fPos1 - ((fPos1 - fPos2) * fPer);
	}
	return fNewPos;
}

bool CFrameInfo::uploadTexture(void* pSurface, int iTimeOut, CClipVideoRenderInfo* pRenderInfo)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VFrame.cpp %d] Texture upload TrackID(%d) TextureID(%d) F(0x%x)", 
		__LINE__, muiTrackID, m_iTextureID, mColorFormat);
	float face_box[] = {0.0f, 0.0f, 0.0f, 0.0f};

	if(pRenderInfo){

		face_box[0] = (float)pRenderInfo->mFace.left;
    	face_box[1] = (float)pRenderInfo->mFace.top;
    	face_box[2] = (float)pRenderInfo->mFace.right;
    	face_box[3] = (float)pRenderInfo->mFace.bottom;
	}

	int lut = pRenderInfo?pRenderInfo->mLUT:mLUT;
	int custom_lut_a = pRenderInfo?pRenderInfo->mCustomLUT_A:mCustomLUT_A;
	int custom_lut_b = pRenderInfo?pRenderInfo->mCustomLUT_B:mCustomLUT_B;
	int custom_lut_power = pRenderInfo?pRenderInfo->mCustomLUT_Power:mCustomLUT_Power;

	int iHdrType = 0;
	if( CNexVideoEditor::m_bHDR2SDR ){
		iHdrType = mHDRType;
	}
	int maxCll = 10000;
	if (iHdrType == HDR_TYPE_PQ)
	{
		if (mHdrInfo.CLL_present_flag)
		{
			maxCll = mHdrInfo.stCLLinfo.max_content_light_level;
			/*
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] drawVideo Hdr CLL ( max cll=%d, max pall=%d)", __LINE__
				,mHdrInfo.stCLLinfo.max_content_light_level
				,mHdrInfo.stCLLinfo.max_pic_average_light_level
			);
			*/
			if (maxCll == 0)
				maxCll = 10000;
		}
	}

	if( mClipType == CLIPTYPE_VIDEO_LAYER )
	{
		switch(mColorFormat)
		{
			case NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY:
#if defined(ANDROID)
                if( m_pSurfaceTexture )
				{
					if(	m_pMediaBuff )
					{
						m_bRenderMediaBuff = TRUE;
						callCodecWrapPostCallback(m_pMediaBuff-1, m_bRenderMediaBuff);
#ifdef FOR_TEST_MEDIACODEC_DEC
                        m_perfWaitFrame.CheckModuleUnitStart();
						NXT_Error waitResult = NXT_ThemeRenderer_WaitForFrameToBeAvailable((NXT_HThemeRenderer )pSurface, (ANativeWindow*)m_pSurfaceTexture, iTimeOut);
                        m_perfWaitFrame.CheckModuleUnitEnd();
                        if( waitResult != NXT_Error_None ) {
                            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] WaitForFrameToBeAvailable failed (%d)", __LINE__, waitResult);
							PerformanceChecker_Catch(PC_ERROR_FRAME_WAIT_TIMEOUT);
                        }
#endif
						
					}
                    m_performanceMonitor.CheckModuleUnitStart();
					NXT_Error nxtRet = NXT_ThemeRenderer_SetSurfaceTexture(	(NXT_HThemeRenderer )pSurface,
																			muiTrackID,
																			(NXT_TextureID)m_iVideoLayerTextureID,
																			mWidth,
																			mHeight,
																			mPitch,
																			mHeight,
																			(ANativeWindow*)m_pSurfaceTexture,
																			CNexCodecManager::isVideoLayerAvailable(),
																			lut,
																			custom_lut_a,
																			custom_lut_b,
																			custom_lut_power,
																			iHdrType,
																			maxCll);

					m_performanceMonitor.CheckModuleUnitEnd();
					if( nxtRet != NXT_Error_None )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] SurfaceTexture can't bind texture (%d)", __LINE__, nxtRet);
					}
				}
#elif defined(__APPLE__)
                m_bRenderMediaBuff = TRUE;
                callCodecWrap_renderDecodedFrame(m_pImageBuff);
				m_performanceMonitor.CheckModuleUnitStart();
                NXT_ThemeRenderer_UploadTextureForHWDecoder((NXT_HThemeRenderer )pSurface,
                                                            (NXT_TextureID)m_iVideoLayerTextureID,
                                                            m_pImageBuff,
                                                            NULL,
                                                            NULL,
                                                            CNexCodecManager::isVideoLayerAvailable(),
                                                            lut);
				m_performanceMonitor.CheckModuleUnitEnd();
                
#endif
                break;
			case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] uploadTexture not supported separated yuv420 format", __LINE__);
				break;
			case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] uploadTexture not supported nv12 format", __LINE__);
				break;
			case NEXCAL_PROPERTY_VIDEO_BUFFER_RGB888:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] uploadTexture not supported RGB8888 format", __LINE__);
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] uploadTexture failed because unknow color format", __LINE__);
				break;
		};

		return true;
	}

	switch(mColorFormat)
	{
		case NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY:
#if defined(ANDROID)
            if( m_pSurfaceTexture )
			{
#ifdef FOR_TEST_MEDIACODEC_DEC
#if 1
				if(	m_pMediaBuff )
				{
					m_bRenderMediaBuff = TRUE;
					callCodecWrapPostCallback(m_pMediaBuff-1, m_bRenderMediaBuff);
                    m_perfWaitFrame.CheckModuleUnitStart();
					NXT_ThemeRenderer_WaitForFrameToBeAvailable((NXT_HThemeRenderer )pSurface, (ANativeWindow*)m_pSurfaceTexture, iTimeOut);
                    m_perfWaitFrame.CheckModuleUnitEnd();
				}
                
#endif
                m_performanceMonitor.CheckModuleUnitStart();
				NXT_Error nxtRet = NXT_ThemeRenderer_SetSurfaceTexture((NXT_HThemeRenderer)pSurface,
																	   muiTrackID,
																	   (NXT_TextureID)m_iTextureID,
																	   mWidth,
																	   mHeight,
																	   mPitch,
																	   mHeight,
																	   (ANativeWindow *)m_pSurfaceTexture,
																	   CNexCodecManager::isVideoLayerAvailable(),
																	   lut,
																	   custom_lut_a,
																	   custom_lut_b,
																	   custom_lut_power,
																	   iHdrType,
																	   maxCll);

				m_performanceMonitor.CheckModuleUnitEnd();
				if( nxtRet != NXT_Error_None )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] SurfaceTexture can't bind texture", __LINE__);
				}
#else
				android::MediaBuffer* pMedia = (android::MediaBuffer*)m_pMediaBuff;
				if( pMedia)
				{
					m_bRenderMediaBuff = TRUE;
					void* pMediaBuffer = (void*)pMedia->graphicBuffer()->getNativeBuffer();
					android::sp<ANativeWindow> pSurf = (ANativeWindow*)m_pSurfaceTexture;
					pSurf->queueBuffer(pSurf.get(), (ANativeWindowBuffer*)pMediaBuffer);
					NXT_Error nxtRet = NXT_ThemeRenderer_SetSurfaceTexture((NXT_HThemeRenderer)pSurface,
																		   (NXT_TextureID)m_iTextureID,
																		   mWidth,
																		   mHeight,
																		   mPitch,
																		   mHeight,
																		   (ANativeWindow *)m_pSurfaceTexture,
																		   CNexCodecManager::isVideoLayerAvailable(),
																		   lut,
																		   custom_lut_a,
																		   custom_lut_b,
																		   custom_lut_power,
																		   iHdrType,
																		   maxCll);

					if( nxtRet != NXT_Error_None )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] SurfaceTexture can't bind texture", __LINE__);
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 1, "[VFrame.cpp %d] SurfaceTexture queue end(buf:%p, Time:%d)", __LINE__, m_pMediaBuff, m_uiTime);
				}
#endif
			}
#elif defined(__APPLE__)
            {
                m_bRenderMediaBuff = TRUE;
                callCodecWrap_renderDecodedFrame(m_pImageBuff);
                m_performanceMonitor.CheckModuleUnitStart();
                NXT_ThemeRenderer_UploadTextureForHWDecoder((NXT_HThemeRenderer )pSurface,
                                                            (NXT_TextureID)m_iTextureID,
                                                            m_pImageBuff,
                                                            NULL,
                                                            NULL,
															CNexCodecManager::isVideoLayerAvailable(), 
                                                            lut);
				m_performanceMonitor.CheckModuleUnitEnd();
                                                            
            }
#endif
			break;
		case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
		{
            		m_performanceMonitor.CheckModuleUnitStart();

			NXT_Error err = NXT_ThemeRenderer_UploadTexture(	(NXT_HThemeRenderer )pSurface,
												muiTrackID,
												(NXT_TextureID)m_iTextureID,
												mWidth,
												mHeight,
												mPitch,
												NXT_PixelFormat_YUV,
												m_pY,
												m_pU,
												m_pV,
												lut,
												custom_lut_a,
												custom_lut_b,
												custom_lut_power,
													face_box);
			m_performanceMonitor.CheckModuleUnitEnd();
			if(err != NXT_Error_None)
				return false;
			break;
		}
		case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
		{
			NXT_Error err;
			m_performanceMonitor.CheckModuleUnitStart();

			if(m_bPreviewThumb)
			{
				 err = NXT_ThemeRenderer_UploadTexture(	(NXT_HThemeRenderer )pSurface,
				 									muiTrackID,
													(NXT_TextureID)m_iTextureID,
													320,
													240,
													320,
													NXT_PixelFormat_NV12,
													m_pY,
													m_pY + (320 * 240),
													NULL,
													lut,
													custom_lut_a,
													custom_lut_b,
													custom_lut_power,
													face_box);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] uploadTexture 0x%x", __LINE__, m_pY);
            
			}
			else
			{
				err = NXT_ThemeRenderer_UploadTexture(	(NXT_HThemeRenderer )pSurface,
												muiTrackID,
												(NXT_TextureID)m_iTextureID,
												mWidth,
												mHeight,
												mPitch,
												mIsNV12Jepg ? NXT_PixelFormat_NV12_JPEG : NXT_PixelFormat_NV12,
												m_pY,
												m_pY + (mPitch * mHeight),
												NULL,
												lut,
												custom_lut_a,
												custom_lut_b,
												custom_lut_power,
												face_box);
			}
			m_performanceMonitor.CheckModuleUnitEnd();
			if(err != NXT_Error_None)
				return false;

			break;
		}
		
		case NEXCAL_PROPERTY_VIDEO_BUFFER_RGB888:
			m_performanceMonitor.CheckModuleUnitStart();

			NXT_ThemeRenderer_UploadTexture(	(NXT_HThemeRenderer )pSurface,
											muiTrackID,
											(NXT_TextureID)m_iTextureID,
											mWidth,
											mHeight,
											mPitch,
											NXT_PixelFormat_RGBA8888,
											m_pY,
											NULL,
											NULL,
											lut,
											custom_lut_a,
											custom_lut_b,
											custom_lut_power,
											face_box);
			m_performanceMonitor.CheckModuleUnitEnd();
			break;

		default:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] uploadTexture failed because unknow color format", __LINE__);
			break;
	};

	return true;
}

void CFrameInfo::dropFirstVideoTexture(void* pSurface)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VFrame.cpp %d] dropFirstVideoTexture(%d) TextureID(%d) F(0x%x)", 
		__LINE__, muiTrackID, m_iTextureID, mColorFormat);

	if( mColorFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY)
	{
#if defined(ANDROID)
        if ( m_pSurfaceTexture == NULL )
            return;

#ifdef FOR_TEST_MEDIACODEC_DEC
		m_bRenderMediaBuff = TRUE;
		NXT_Error nxtRet = NXT_ThemeRenderer_DiscardSurfaceTexture(	(NXT_HThemeRenderer )pSurface, (ANativeWindow*)m_pSurfaceTexture);
		if(	m_pMediaBuff )
		{
			callCodecWrapPostCallback(m_pMediaBuff-1, m_bRenderMediaBuff);
			NXT_ThemeRenderer_WaitForFrameToBeAvailable((NXT_HThemeRenderer )pSurface, (ANativeWindow*)m_pSurfaceTexture, -1);
		}
#endif
#elif defined(__APPLE__)
        if ( m_pImageBuff )
            callCodecWrap_renderDecodedFrame(m_pImageBuff);
#endif
	}
}

void CFrameInfo::setVideoLayerTextureID(int iTextureID)
{
	m_iVideoLayerTextureID = iTextureID;
}

void CFrameInfo::setVideoDropRenderTime(unsigned int uTime)
{
	m_uiDropRenderTime = (uTime == 0?DROP_RENDER_TIME_EXPORT:uTime);
}

NXBOOL CFrameInfo::drawVideo(void* pSurface, RECT* pRC, int iBitCount)
{
	return TRUE;
}

int CFrameInfo::drawVideo(void* pSurface, int iIndex, unsigned int uiTime, NXBOOL bForceDisplay, NXBOOL bExport, int iTimeOut)
{
	CAutoLock m(m_FrameLock);
	CClipVideoRenderInfo* pRenderInfo = getActiveRenderInfo(uiTime);

	if( mClipType == CLIPTYPE_VIDEO_LAYER )
	{
		if(pRenderInfo)
		{
			if( bForceDisplay )
			{
				uploadTexture(pSurface, iTimeOut, pRenderInfo);
				NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, (unsigned int)pRenderInfo->mRotateState);
				NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, pRenderInfo->mBrightness, pRenderInfo->mContrast, pRenderInfo->mSaturation, pRenderInfo->mTintcolor, pRenderInfo->mHue, pRenderInfo->mVignette);
				NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);

				// test code for Matrix effect
				NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, pRenderInfo->mStartMatrix, pRenderInfo->mStartTime, pRenderInfo->mEndMatrix, pRenderInfo->mEndTime);
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	
				applyPosition(pSurface, uiTime);
				return 0;
			}

			int iCheckStatus = checkDrawTime(uiTime, bExport);
			if( iCheckStatus != 0 )
				return iCheckStatus;
			uploadTexture(pSurface, iTimeOut, pRenderInfo);
			NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, (unsigned int)pRenderInfo->mRotateState);
			NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, pRenderInfo->mBrightness, pRenderInfo->mContrast, pRenderInfo->mSaturation, pRenderInfo->mTintcolor, pRenderInfo->mHue, pRenderInfo->mVignette);
			NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	

			// test code for Matrix effect
			NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, pRenderInfo->mStartMatrix, pRenderInfo->mStartTime, pRenderInfo->mEndMatrix, pRenderInfo->mEndTime);
		}
		else
		{
			if( bForceDisplay )
			{
				uploadTexture(pSurface, iTimeOut, pRenderInfo);
				NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, (unsigned int)mRotateState);
				NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, mBrightness, mContrast, mSaturation, mTintcolor, mHue, mVignette);
				NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);

				// test code for Matrix effect
				NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, mStartMatrix, mStartTime, mEndMatrix, mEndTime);
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	
				applyPosition(pSurface, uiTime);
				return 0;
			}

			int iCheckStatus = checkDrawTime(uiTime, bExport);
			if( iCheckStatus != 0 )
				return iCheckStatus;
			uploadTexture(pSurface, iTimeOut, pRenderInfo);
			NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, (unsigned int)mRotateState);
			NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, mBrightness, mContrast, mSaturation, mTintcolor, mHue, mVignette);
			NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	

			// test code for Matrix effect
			NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, mStartMatrix, mStartTime, mEndMatrix, mEndTime);
		}

		applyPosition(pSurface, uiTime);

		return 0;
	}

	NXBOOL isUploadTexture = TRUE;
	NXT_TextureID eID = iIndex == 0 ? NXT_TextureID_Video_1 : NXT_TextureID_Video_2;
    
	if( mClipType == CLIPTYPE_IMAGE )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] drawImage(%d) iIndex(%d) Time(%d %d %d)", __LINE__, muiTrackID, iIndex, mStartTime, mEndTime, uiTime);
		
		int iTexture = m_iImageFrameUploadFlag | (iIndex == 0 ? 1 : 2);
		if( m_iImageFrameUploadFlag == iTexture )
		{
			isUploadTexture = FALSE;
		}

		if(NXT_ThemeRenderer_CheckUploadOk((NXT_HThemeRenderer)pSurface)) {

			m_iImageFrameUploadFlag |= iTexture;
        }
			
		m_iTextureID = eID;

		int face_detected = 0;
		if (pRenderInfo)
		{
			face_detected = (m_iFaceDetectFlag != pRenderInfo->mFaceDetected)?pRenderInfo->mFaceDetected:0;
			m_iFaceDetectFlag = pRenderInfo->mFaceDetected;
		}

		if( isUploadTexture || face_detected )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] uploadTexture image(%d) index(%d) ItemStart(%d)Cur Time(%d)", __LINE__, muiTrackID, m_iTextureID, mStartTime, uiTime );
			uploadTexture(pSurface, iTimeOut, pRenderInfo);

			if(pRenderInfo)
			{
				NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, (unsigned int)pRenderInfo->mRotateState);
				NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, pRenderInfo->mBrightness, pRenderInfo->mContrast, pRenderInfo->mSaturation, pRenderInfo->mTintcolor, pRenderInfo->mHue, pRenderInfo->mVignette);
				NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);

				// test code for Matrix effect
				NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, pRenderInfo->mStartMatrix, pRenderInfo->mStartTime, pRenderInfo->mEndMatrix, pRenderInfo->mEndTime);
			}
			else
			{
				NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, (unsigned int)mRotateState);
				NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, mBrightness, mContrast, mSaturation, mTintcolor, mHue, mVignette);
				NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);

				// test code for Matrix effect
				NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, mStartMatrix, mStartTime, mEndMatrix, mEndTime);
			}
		}

		applyPosition(pSurface, uiTime);
		return 0;
	}

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] drawVideo(%d) iIndex(%d) Time(F:%d C:%d)", __LINE__, muiTrackID, iIndex, m_uiTime, uiTime);
	
	//TODO : 360video yoon
	//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] drawVideo m_iVideoRenderMode=%d, m_iHDRType=%d //yoon", __LINE__,mVideoRenderMode, mHDRType);
//test 
//m_iVideoRenderMode = 1;
	
    if( mVideoRenderMode == RENDERMODE_360VIDEO ){
     
       	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
    	if( pEditor != NULL ) 
    	{
    	    if( pEditor->get360VideoTrackPositionRadian(3) )
    	    {    
    	        int xRadian , yRadian;
        	    xRadian = pEditor->get360VideoTrackPositionRadian(1);
        	    yRadian = pEditor->get360VideoTrackPositionRadian(2);
        		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] drawVideo xRadian=%d, yRadian=%d //yoon", __LINE__,xRadian, yRadian);
        		NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, 1);
        		NXT_ThemeRenderer_set360View((NXT_HThemeRenderer)pSurface,xRadian ,yRadian );
        	}
        	else
        	{
        	    NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, 0);
        	}
    		SAFE_RELEASE(pEditor);
    	}
    }
    else
    {
        NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, 0);
    }
	
	m_iTextureID = eID;

	if(pRenderInfo)
	{
		if( bForceDisplay )
		{
			uploadTexture(pSurface, iTimeOut, pRenderInfo);
			NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, (unsigned int)pRenderInfo->mRotateState);
			NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, pRenderInfo->mBrightness, pRenderInfo->mContrast, pRenderInfo->mSaturation, pRenderInfo->mTintcolor, pRenderInfo->mHue, pRenderInfo->mVignette);
			NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);

			// test code for Matrix effect
			NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, pRenderInfo->mStartMatrix, pRenderInfo->mStartTime, pRenderInfo->mEndMatrix, pRenderInfo->mEndTime);
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	
			applyPosition(pSurface, uiTime);
			return 0;
		}

		int iCheckStatus = checkDrawTime(uiTime, bExport);
		if( iCheckStatus != 0 )
		{
			return iCheckStatus;
		}

		uploadTexture(pSurface, iTimeOut, pRenderInfo);
		NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, (unsigned int)pRenderInfo->mRotateState);
		NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, pRenderInfo->mBrightness, pRenderInfo->mContrast, pRenderInfo->mSaturation, pRenderInfo->mTintcolor, pRenderInfo->mHue, pRenderInfo->mVignette);
		NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	

		// test code for Matrix effect
		NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, pRenderInfo->mStartMatrix, pRenderInfo->mStartTime, pRenderInfo->mEndMatrix, pRenderInfo->mEndTime);
	}	
	else
	{
		if( bForceDisplay )
		{
			uploadTexture(pSurface, iTimeOut, pRenderInfo);
			NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, (unsigned int)mRotateState);
	NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, mBrightness, mContrast, mSaturation, mTintcolor, mHue, mVignette);
			NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);

			// test code for Matrix effect
			NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, mStartMatrix, mStartTime, mEndMatrix, mEndTime);
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	
			applyPosition(pSurface, uiTime);
			return 0;
		}

		int iCheckStatus = checkDrawTime(uiTime, bExport);
		if( iCheckStatus != 0 )
		{
			return iCheckStatus;
		}

		uploadTexture(pSurface, iTimeOut, pRenderInfo);
		NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, (unsigned int)mRotateState);
		NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, mBrightness, mContrast, mSaturation, mTintcolor, mHue, mVignette);
		NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);	

		// test code for Matrix effect
		NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iTextureID, mStartMatrix, mStartTime, mEndMatrix, mEndTime);
	}

	applyPosition(pSurface, uiTime);

	return 0;
}

int CFrameInfo::drawVideo(void* pSurface, unsigned int uiTime, int iTimeOut)
{
    CAutoLock m(m_FrameLock);
	int iHdrType = 0 ;
	if( CNexVideoEditor::m_bHDR2SDR ){
		iHdrType = mHDRType;
	}
	
	int maxCll = 10000;
	if (iHdrType == HDR_TYPE_PQ)
	{
		if (mHdrInfo.CLL_present_flag)
		{
			maxCll = mHdrInfo.stCLLinfo.max_content_light_level;
			/*
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] drawVideo Hdr CLL ( max cll=%d, max pall=%d)", __LINE__
				,mHdrInfo.stCLLinfo.max_content_light_level
				,mHdrInfo.stCLLinfo.max_pic_average_light_level
			);
			*/
			if (maxCll == 0)
				maxCll = 10000;
		}
	}
	
#ifdef _ANDROID

    if(	m_pMediaBuff )
    {
        m_bRenderMediaBuff = TRUE;
		callCodecWrapPostCallback(m_pMediaBuff-1, m_bRenderMediaBuff);
        NXT_ThemeRenderer_WaitForFrameToBeAvailable((NXT_HThemeRenderer)pSurface, (ANativeWindow*)m_pSurfaceTexture, iTimeOut);
    }

    NXT_Error nxtRet = NXT_ThemeRenderer_SetSurfaceTexture(	(NXT_HThemeRenderer)pSurface,
															muiTrackID,
                                                            NXT_TextureID_Video_1,
                                                            mWidth,
                                                            mHeight,
                                                            mPitch,
                                                            mHeight,
                                                            (ANativeWindow*)m_pSurfaceTexture,
															0, //convert_rgb_flag
															0, //tex_id_for_lut
															0, //tex_id_for_customlut_a
															0, //tex_id_for_customlut_b
															0, //tex_id_for_customlut_power
															iHdrType,
															maxCll);
    if( nxtRet != NXT_Error_None )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] SurfaceTexture can't bind texture", __LINE__);
    }

	//TODO : 360video yoon
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] drawVideo VideoRenderMode=%d, HDRType=(%d,%d) //yoon", __LINE__,mVideoRenderMode, mHDRType,iHdrType);

    if( mVideoRenderMode == RENDERMODE_360VIDEO ){
     
       	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
    	if( pEditor != NULL ) 
    	{
    		
    		NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, 1);
    		NXT_ThemeRenderer_set360View((NXT_HThemeRenderer)pSurface, pEditor->get360VideoTrackPositionRadian(1), pEditor->get360VideoTrackPositionRadian(2));
    		SAFE_RELEASE(pEditor);
    	}
    }
    else
    {
        NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, 0);
    }
    
    NXT_ThemeRenderer_SetTextureRotation((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, (unsigned int)mRotateState);
    NXT_ThemeRenderer_SetTextureColorAdjust((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, mBrightness, mContrast, mSaturation, mTintcolor, mHue, mVignette);
    NXT_ThemeRenderer_SetTextureFitInFrame((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, SUPPORTED_VIEW_WIDTH, SUPPORTED_VIEW_HEIGHT);
    
    // test code for Matrix effect
    NXT_ThemeRenderer_SetTextureTransform((NXT_HThemeRenderer)pSurface, NXT_TextureID_Video_1, mStartMatrix, mStartTime, mEndMatrix, mEndTime);
    // nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[VFrame.cpp %d] display color(%d %d %d %d) rotate(%d)", __LINE__, mBrightness, mContrast, mContrast, mTintcolor, mRotateState);

    m_iTextureID = NXT_TextureID_Video_1;
    applyPosition(pSurface, uiTime);

    // NXT_ThemeRenderer_GLDraw((NXT_HThemeRenderer)pSurface, NXT_RendererOutputType_RGBA_8888, 0);

    NXT_ThemeRenderer_AddFastPreviewFrame((NXT_HThemeRenderer)pSurface, NXT_RendererOutputType_RGBA_8888, 0, m_uiTime);
#endif
    return 0;
}

int CFrameInfo::drawVideo(void* pSurface, unsigned int uiTime, int textureID, NXBOOL bExport, NXBOOL bForced, int iTimeOut)
{
	CAutoLock m(m_FrameLock);

	NXBOOL isUploadTexture = TRUE;

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] TID(%d) drawVideo with texture id(%d)", __LINE__,  muiTrackID, textureID);

	if( mClipType == CLIPTYPE_IMAGE )

	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] drawImage(%d) iIndex(%d) Time(%d %d %d)", __LINE__, muiTrackID, iIndex, mStartTime, mEndTime, uiTime);

		if( m_iImageFrameUploadFlag == 0 && NXT_ThemeRenderer_CheckUploadOk((NXT_HThemeRenderer)pSurface))
		{
			m_iTextureID = textureID;
			m_iImageFrameUploadFlag = 1;

			m_performanceMonitor.CheckModuleUnitStart();

			NXT_ThemeRenderer_UploadTexture(	(NXT_HThemeRenderer )pSurface,
											muiTrackID,
											(NXT_TextureID)m_iTextureID,
											mWidth,
											mHeight,
											mPitch,
											mIsNV12Jepg ? NXT_PixelFormat_NV12_JPEG : NXT_PixelFormat_NV12,
											m_pY,
											m_pY + (mPitch * mHeight),
											NULL,
											mLUT,
											mCustomLUT_A,
											mCustomLUT_B,
											mCustomLUT_Power,
											NULL);
			m_performanceMonitor.CheckModuleUnitEnd();
		}
		return 0;
	}

	int iCheckStatus = checkDrawTime(uiTime, bExport);
	if( bForced == FALSE && iCheckStatus != 0 )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] TID(%d) drawVideo checkDrawTime(%d)", __LINE__, muiTrackID, iCheckStatus);
		return iCheckStatus;
	}

	m_iTextureID = textureID;

	//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] TID(%d) drawVideo m_iVideoRenderMode=%d , m_iHDRType=%d", __LINE__, muiTrackID, mVideoRenderMode,mHDRType);
	int iHdrType = 0 ;
	if( CNexVideoEditor::m_bHDR2SDR ){
		iHdrType = mHDRType;
	}

	int maxCll = 10000;
	if( iHdrType == HDR_TYPE_PQ )
	{
		if( mHdrInfo.CLL_present_flag )
		{	
			maxCll = mHdrInfo.stCLLinfo.max_content_light_level;
			/*
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] drawVideo Hdr CLL ( max cll=%d, max pall=%d)", __LINE__
				,mHdrInfo.stCLLinfo.max_content_light_level
				,mHdrInfo.stCLLinfo.max_pic_average_light_level
			);
			*/
			if (maxCll == 0)
				maxCll = 10000;
		}
	}
	//TODO : 360video yoon
	if( mVideoRenderMode == RENDERMODE_360VIDEO ){

		CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
		if( pEditor != NULL )
		{
			if( pEditor->get360VideoTrackPositionRadian(3) )
			{
				int xRadian , yRadian;
				xRadian = pEditor->get360VideoTrackPositionRadian(1);
				yRadian = pEditor->get360VideoTrackPositionRadian(2);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] TID(%d) drawVideo xRadian=%d, yRadian=%d //yoon", muiTrackID, __LINE__,xRadian, yRadian);
				NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, (NXT_TextureID)m_iTextureID, 1);
				NXT_ThemeRenderer_set360View((NXT_HThemeRenderer)pSurface,xRadian ,yRadian );
			}
			else
			{
				NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, (NXT_TextureID)m_iTextureID, 0);
			}
			SAFE_RELEASE(pEditor);
		}
	}
	else
	{
		NXT_Theme_SetTexture360Flag((NXT_HThemeRenderer)pSurface, (NXT_TextureID)m_iTextureID, 0);
	}

#if defined(ANDROID)
	if( m_pSurfaceTexture )
	{
		if(	m_pMediaBuff )
		{
			m_bRenderMediaBuff = TRUE;
			callCodecWrapPostCallback(m_pMediaBuff-1, m_bRenderMediaBuff);

			m_perfWaitFrame.CheckModuleUnitStart();
			NXT_Error waitResult = NXT_ThemeRenderer_WaitForFrameToBeAvailable((NXT_HThemeRenderer )pSurface, (ANativeWindow*)m_pSurfaceTexture, iTimeOut);
			m_perfWaitFrame.CheckModuleUnitEnd();
			if( waitResult != NXT_Error_None ) {
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] TID(%d) WaitForFrameToBeAvailable failed (%d)", __LINE__, muiTrackID, waitResult);
			}
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] TID(%d) WaitForFrameToBeAvailable (%d)", __LINE__, muiTrackID, waitResult);
		}
		m_performanceMonitor.CheckModuleUnitStart();
		NXT_Error nxtRet = NXT_ThemeRenderer_SetSurfaceTexture(	(NXT_HThemeRenderer )pSurface,
															   muiTrackID,
															   (NXT_TextureID)m_iTextureID,
															   mWidth,
															   mHeight,
															   mPitch,
															   mHeight,
															   (ANativeWindow*)m_pSurfaceTexture,
															   CNexCodecManager::isVideoLayerAvailable(),
															   mLUT,
															   mCustomLUT_A,
															   mCustomLUT_B,
															   mCustomLUT_Power,
															   iHdrType,
															   maxCll);

		m_performanceMonitor.CheckModuleUnitEnd();
		if( nxtRet != NXT_Error_None )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] TID(%d) SurfaceTexture can't bind texture (%d)", __LINE__, muiTrackID, nxtRet);
		}
	}
#elif defined(__APPLE__)
	m_bRenderMediaBuff = TRUE;
	callCodecWrap_renderDecodedFrame(m_pImageBuff);
	m_performanceMonitor.CheckModuleUnitStart();
	NXT_ThemeRenderer_UploadTextureForHWDecoder((NXT_HThemeRenderer )pSurface,
												(NXT_TextureID)m_iTextureID,
												m_pImageBuff,
												NULL,
												NULL,
												CNexCodecManager::isVideoLayerAvailable(),
												mLUT);
	m_performanceMonitor.CheckModuleUnitEnd();

#endif
	return 0;
}

int CFrameInfo::checkDrawTime(unsigned int uiTime, NXBOOL bExport)
{
	// return value 
	// 0 : exist draw item;
	// 1 : drop draw item
	// 2 : wait draw item
	int isDraw = TRUE;
	if( mClipType == CLIPTYPE_IMAGE )
	{
		if( mStartTime < uiTime && uiTime > mEndTime )
			return 0;
		return 1;
	}

	if( bExport )
	{
		if( m_uiTime < uiTime )
		{
			if( uiTime < IGNORE_RENDER_TIME )
			{
			}
			else if( m_uiTime < (uiTime - m_uiDropRenderTime/*DROP_RENDER_TIME_EXPORT*/) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VFrame.cpp %d] Video is late(FrameT:%d CurT:%d Gap:%d)", __LINE__, m_uiTime, uiTime, m_uiTime - uiTime);
				return 1;
			}
		}
		else
		{
			if( m_uiTime > (uiTime + WAIT_RENDER_TIME_EXPORT) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VFrame.cpp %d] Video is fast(FrameT:%d CurT:%d Gap:%d)", __LINE__, m_uiTime, uiTime, m_uiTime - uiTime);
				return 2;
			}
		}		
	}
	else
	{
		if( m_uiTime < uiTime )
		{
			if( uiTime < IGNORE_RENDER_TIME )
			{
			}
			else if( m_uiTime < (uiTime - m_uiDropRenderTime) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VFrame.cpp %d] Video is late(FrameT:%d CurT:%d Gap:%d)", __LINE__, m_uiTime, uiTime, m_uiTime - uiTime);
				return 1;
			}
		}
		else
		{
			if( m_uiTime > (uiTime + WAIT_RENDER_TIME_PREVIEW) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VFrame.cpp %d] Video is fast(FrameT:%d CurT:%d Gap:%d)", __LINE__, m_uiTime, uiTime, m_uiTime - uiTime);
				return 2;
			}
		}
	}

	return 0;
}

NXBOOL CFrameInfo::isImageClip()
{
	return mClipType == CLIPTYPE_IMAGE ? TRUE : FALSE;
}

void CFrameInfo::clearTextureID()
{
	CAutoLock m(m_FrameLock);
	m_iTextureID = -1;
	m_iImageFrameUploadFlag = 0;
}

void CFrameInfo::updateFrameTime(unsigned int uiStartTime, unsigned uiEndTime)
{
	CAutoLock m(m_FrameLock);
	mStartTime = uiStartTime;
	mEndTime = uiEndTime;
}

void CFrameInfo::updateImageFrameRect(RECT rcStartRect, RECT rcEndRect)
{
	CAutoLock m(m_FrameLock);
	mStart.left		= rcStartRect.left;
	mStart.top		= rcStartRect.top;
	mStart.right		= rcStartRect.right;
	mStart.bottom		= rcStartRect.bottom;
	
	mEnd.left			= rcEndRect.left;
	mEnd.top			= rcEndRect.top;
	mEnd.right		= rcEndRect.right;
	mEnd.bottom		= rcEndRect.bottom;	
}

unsigned int CFrameInfo::getHighestPowerOfTwo( unsigned int v )
{
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

CClipVideoRenderInfo* CFrameInfo::getActiveRenderInfo(unsigned int uiTime)
{
	for(int i = 0; i < m_ClipVideoRenderInfoVec.size(); i++)
	{
		if( m_ClipVideoRenderInfoVec[i]->mStartTime <= uiTime && m_ClipVideoRenderInfoVec[i]->mEndTime > uiTime )
		{
			return m_ClipVideoRenderInfoVec[i];
		}
	}
	
	return NULL;
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/
