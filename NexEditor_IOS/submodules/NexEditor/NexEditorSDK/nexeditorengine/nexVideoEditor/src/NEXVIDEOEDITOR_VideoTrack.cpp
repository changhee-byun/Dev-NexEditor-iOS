/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoTrack.cpp
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

#include "NEXVIDEOEDITOR_VideoTrack.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "NEXVIDEOEDITOR_Util.h"
#include "NEXVIDEOEDITOR_CalcTime.h"
#ifdef _ANDROID
#include "NEXVIDEOEDITOR_PerformanceChecker.h"
#endif

#ifdef __APPLE__
extern "C" {
    unsigned int getPTSFromImageBuffer(void* imageBuffer);
    
}
#endif

int CVideoTrackInfo::is360Video( unsigned char * inUUID, int inUUIDSize )
{
	int iVideoRenderMode = 0;
	if( inUUID != NULL && inUUIDSize > 0 ){
		int rval = findString(inUUID,inUUIDSize,(unsigned char *)"rdf:SphericalVideo",strlen("rdf:SphericalVideo"));
	   if( rval < 0 ){
	       nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] is360Video 360video not found\n", __LINE__);
	       iVideoRenderMode = 0;
	   }else{
	   	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] is360Video 360video set\n", __LINE__);
	   	iVideoRenderMode = 1;
	   }
	}
	return iVideoRenderMode;

}	

//static
int CVideoTrackInfo::getHDRMetaData(unsigned int inCodecType,	NXUINT8*	inDSI , NXUINT32 inDSISize , NXUINT8* in1stFrame, NXUINT32 in1stFrameSize, int inNalSize ,NEXCODECUTIL_SEI_HDR_INFO *out)
{
	int iHdrType = 0;
	if( inCodecType == eNEX_CODEC_V_HEVC )
	{
		if( inDSISize > 0 && inDSI != NULL )
		{
			NEXCODECUTIL_SPS_INFO sps;
			memset(&sps,0,sizeof(NEXCODECUTIL_SPS_INFO));
			if( NexCodecUtil_HEVC_GetSPSInfo( (NXCHAR*)inDSI, inDSISize, &sps, NexCodecUtil_CheckByteFormat( inDSI, inDSISize)) == 0 )
			{
				iHdrType = sps.transfer_characteristics;
				if ( iHdrType == 1 || iHdrType == 2 || iHdrType == 6 ) // Fixed for CHC-319
				{
					iHdrType = 0;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d]getHDRMetaData HdrType(%d->%d)", __LINE__, sps.transfer_characteristics, iHdrType);	
			}
		}

		if( iHdrType != 0 && out != NULL )
		{
			NXINT32 len = in1stFrameSize;
			NXUINT8 * pTarget = in1stFrame; 
			int ismalloc = 0;
			if( NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW == NexCodecUtil_CheckByteFormat((unsigned char*)in1stFrame, in1stFrameSize) )
			{	
				ismalloc = 1;
				pTarget = (NXUINT8*)malloc(in1stFrameSize+100);
				len = NexCodecUtil_ConvertFormat( pTarget	, in1stFrameSize+80
									, in1stFrame, in1stFrameSize
									, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, inNalSize );
			}

			if( NexCodecUtil_HEVC_SEI_GetHDRInfo( pTarget, len, inNalSize, NULL , out ) == 0 )
			{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d]getHDRMetaData GetHDRInfo(%d,%d,%d)", __LINE__
						, out->MDCV_prsent_flag
						, out->CRI_present_flag
						, out->CLL_present_flag
						);	
			}
						
			if( ismalloc )
			{
				free(pTarget);
			}
		}	
	}	
	return iHdrType;
}	

CVideoTrackInfo::CVideoTrackInfo()
{
	m_uiTrackID				= 0;
	m_iWidth				= 0;
	m_iHeight				= 0;
	m_iPitch				= 0;

	m_iFrameInfoCount		= VIDEO_FRAME_MAX_COUNT;

	m_ClipType				= CLIPTYPE_NONE;

	m_uiStartTime			= 0;
	m_uiEndTime				= 0;

	m_uiEffectStartTime		= 0;
	m_uiEffectDuration		= 0;
	m_iEffectOffect			= 0;
	m_iEffectOverlap		= 0;
	m_bClipEffectStart		= FALSE;	
	m_pEffectID				= NULL;

	m_bEffectApplyEnd		= FALSE;

	m_pTitle				= NULL;
	m_pTitleEffectID		= NULL;
	m_bTitleEffectStart		= FALSE;
	m_uiTitleStartTime		= 0;
	m_uiTitleEndTime		= 0;

	m_iRotateState			= 0;

	m_iBrightness			= 0;
	m_iContrast				= 0;
	m_iSaturation			= 0;
    m_iHue                  = 0;
	m_iTintcolor			= 0;

	memcpy(m_StartMatrix, default_matrix, sizeof(float)*MATRIX_MAX_COUNT);
	memcpy(m_EndMatrix, default_matrix, sizeof(float)*MATRIX_MAX_COUNT);
	
	m_bTrackStart			= FALSE;

	m_uiTotalClipCount		= 0;
	m_uiCurrentClipIndex	= 0;

	m_pSource			= NULL;
	m_isNextFrameRead		= FALSE;

	m_FrameTimeChecker.clear();

	m_pSurfaceTexture		= NULL;
	m_pCodecWrap			= NULL;

	m_bDecodingEnd			= FALSE;
	m_bRenderingEnd			= FALSE;
	m_bFirstFrameFlag		= FALSE;//TRUE;

	m_iVideoLayerTextureID	= 0;
	m_iDrop = 0;

	setEffectID(THEME_RENDERER_DEFAULT_EFFECT);
	setTitleEffectID(THEME_RENDERER_DEFAULT_TITLE_EFFECT);

	m_iDropCount = 0;
	m_iDropStartTime = 0xffffffff;
	m_iDropLastCount = 0;

    m_iVideoRenderMode = 0; //yoon
    m_iHDRType = 0; //yoon
    memset(&m_stHdrInfo,0,sizeof(NEXCODECUTIL_SEI_HDR_INFO));//yoon

	m_FrameTimeChecker4Pause.clear();
	m_uiLastEncodedFrameCTS = 0;
	m_ClipVideoRenderInfoVec.clear();
	m_bFirstDisplay4Layer = 0;

	m_pDrawInfos = NULL;
	m_iTrackTextureID	= -1;
	m_iFeatureVersion = 0;

	m_isMotionTracked		= FALSE;
	m_bReUseSurfaceTexture = FALSE;   
	m_uLogCnt = 0;
	m_bTrackPaused = 0;
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor != NULL ) 
	{
		m_iFeatureVersion = pEditor->getPropertyInt("FeatureVersion", 0);
		SAFE_RELEASE(pEditor);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] CVideoTrackInfo ", __LINE__);
}

CVideoTrackInfo::~CVideoTrackInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ~~~~~~~~CVideoTrackInfo In(%d)", __LINE__, m_uiTrackID);
	if( m_pTitle )
	{
		nexSAL_MemFree(m_pTitle);
		m_pTitle = NULL;
	}

	for (int i = 0; i < VIDEO_FRAME_MAX_COUNT; i++)
	{
		m_FrameInfo[i].setCodecWrap(NULL);
	}

	if( m_pCodecWrap )
	{
		m_pCodecWrap->deinitDecoder();
		SAFE_RELEASE(m_pCodecWrap);
	}

	if( m_bReUseSurfaceTexture == FALSE && m_pSurfaceTexture )
	{
		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			pVideoRender->releaseSurfaceTexture(m_pSurfaceTexture);
			m_pSurfaceTexture = NULL;
			SAFE_RELEASE(pVideoRender);
		}
	}

	SAFE_RELEASE(m_pSource);

	if( m_pEffectID)
	{
		nexSAL_MemFree(m_pEffectID);
		m_pEffectID = NULL;
	}

	if( m_pTitleEffectID)
	{
		nexSAL_MemFree(m_pTitleEffectID);
		m_pTitleEffectID = NULL;
	}

	clearClipRenderInfoVec();

	SAFE_RELEASE(m_pDrawInfos);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ~~~~~~~~CVideoTrackInfo Out", __LINE__);
}

void CVideoTrackInfo::setTrackID(unsigned int uiTrackID)
{
	m_uiTrackID = uiTrackID;
}

NXBOOL CVideoTrackInfo::setClipRenderInfo(CClipVideoRenderInfo* pClipRenderInfo)
{
	if( pClipRenderInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setClipRenderInfo is failed because param is null(%d)", __LINE__, pClipRenderInfo);
		return FALSE;
	}

 	CAutoLock m(m_FrameQueueLock);
	NXBOOL bInitSucess = TRUE;

	m_uiTrackID		= pClipRenderInfo->muiTrackID;
	m_uiStartTime	= pClipRenderInfo->mStartTime;
	m_uiEndTime		= pClipRenderInfo->mEndTime;
	
	m_iWidth			= pClipRenderInfo->mWidth;
	m_iHeight		= pClipRenderInfo->mHeight;
	m_iPitch			= pClipRenderInfo->mPitch;

	m_iRotateState	= pClipRenderInfo->mRotateState;
	m_iBrightness		= pClipRenderInfo->mBrightness;
	m_iContrast		= pClipRenderInfo->mContrast;
	m_iSaturation		= pClipRenderInfo->mSaturation;
    m_iHue              = pClipRenderInfo->mHue;
	m_iTintcolor		= pClipRenderInfo->mTintcolor;
	m_iLUT 				= pClipRenderInfo->mLUT;
	m_iCustomLUT_A		= pClipRenderInfo->mCustomLUT_A;
	m_iCustomLUT_B		= pClipRenderInfo->mCustomLUT_B;
	m_iCustomLUT_Power	= pClipRenderInfo->mCustomLUT_Power;
	m_iVignette			= pClipRenderInfo->mVignette;
	m_iVideoRenderMode = pClipRenderInfo->mVideoRenderMode; //yoon
	m_iHDRType =  pClipRenderInfo->mHDRType; //yoon
	memcpy( &m_stHdrInfo, &pClipRenderInfo->mHdrInfo , sizeof(NEXCODECUTIL_SEI_HDR_INFO));
	m_ClipType		= pClipRenderInfo->mClipType;
	m_iLoadedType		= pClipRenderInfo->mLoadedType;
	
	memcpy(m_StartMatrix, pClipRenderInfo->mStartMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	memcpy(m_EndMatrix, pClipRenderInfo->mEndMatrix, sizeof(float)*MATRIX_MAX_COUNT);

	// printMatrixInfo(m_StartMatrix);
	// printMatrixInfo(m_EndMatrix);
	
	m_FrameInVec.clear();
	m_FrameOutVec.clear();

	if( m_ClipType == CLIPTYPE_IMAGE )
	{
		m_FrameInfo[0].deinitFrameInfo();
		if( m_FrameInfo[0].initClipRenderInfo(pClipRenderInfo) == FALSE )
		{
			m_FrameInfo[0].deinitFrameInfo();
			bInitSucess = FALSE;
		}

		if( bInitSucess )
		{
			m_FrameOutVec.insert(m_FrameOutVec.begin(), &m_FrameInfo[0]);
			m_bTrackStart	= TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setImageInfo(TrackID:%d Start:%d End:%d Width:%d Height:%d Pitch:%d)", 
				__LINE__, m_uiTrackID, m_uiStartTime, m_uiEndTime, m_iWidth, m_iHeight, m_iPitch);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setImageInfo(Color Info %d %d %d %d %d)", 
				__LINE__, m_iBrightness, m_iContrast, m_iSaturation, m_iTintcolor);
		}
	}
	else if( m_ClipType == CLIPTYPE_VIDEO || m_ClipType == CLIPTYPE_VIDEO_LAYER )
	{
		if( pClipRenderInfo->mIsPreview )
			m_iFrameInfoCount = 1;

		/*
		if( pClipRenderInfo->mColorFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
		{
			m_iFrameInfoCount = 2;
		}
		*/

		for( int i = 0; i < m_iFrameInfoCount; i++ )
		{
			m_FrameInfo[i].deinitFrameInfo();
			if( m_FrameInfo[i].initClipRenderInfo(pClipRenderInfo) == FALSE )
			{
				bInitSucess = FALSE;
				break;
			}
			m_FrameInfo[i].mVideoRenderMode = m_iVideoRenderMode; //yoon
			m_FrameInfo[i].mHDRType = m_iHDRType; //yoon
			memcpy( &m_FrameInfo[i].mHdrInfo , &m_stHdrInfo , sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon
			m_FrameInVec.insert(m_FrameInVec.end(), &m_FrameInfo[i]);
		}

		if( bInitSucess )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setVideoInfo(TrackID:%d Start:%d End:%d Width:%d Height:%d)", 
				__LINE__, m_uiTrackID, m_uiStartTime, m_uiEndTime, m_iWidth, m_iHeight);
		}
		else
		{
			for( int i = 0; i < VIDEO_FRAME_MAX_COUNT; i++ )
			{
				m_FrameInfo[i].deinitFrameInfo();
			}
			
			m_FrameInVec.clear();
			m_FrameOutVec.clear();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setVideoInfo failed(TrackID:%d Start:%d End:%d Width:%d Height:%d)", 
				__LINE__, m_uiTrackID, m_uiStartTime, m_uiEndTime, m_iWidth, m_iHeight);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setVideoInfo(Color Info %d %d %d %d)", 
				__LINE__, m_iBrightness, m_iContrast, m_iSaturation, m_iHue, m_iTintcolor);
		}
		
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setClipRenderInfo is failed because did not support clip type(%d)", __LINE__, m_ClipType);
		return FALSE;
	}
	return bInitSucess;
}

NXBOOL CVideoTrackInfo::clearClipRenderInfoVec()
{
	int i = 0;
	CAutoLock m(m_FrameQueueLock);

	for (i = 0; i < m_ClipVideoRenderInfoVec.size(); i++ ) {
		m_ClipVideoRenderInfoVec[i]->Release();
	}
	m_ClipVideoRenderInfoVec.clear();

	for( i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].clearClipRenderInfoVec();
	}
	return TRUE;
}

NXBOOL CVideoTrackInfo::setClipRenderInfoVec(CClipVideoRenderInfo* pClipRenderInfo)
{
	if( pClipRenderInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setClipRenderInfoVec is failed because param is null(%d)", __LINE__, pClipRenderInfo);
		return FALSE;
	}

 	CAutoLock m(m_FrameQueueLock);
	NXBOOL bInitSucess = TRUE;

	SAFE_ADDREF(pClipRenderInfo);
	m_ClipVideoRenderInfoVec.insert(m_ClipVideoRenderInfoVec.end(), pClipRenderInfo);

	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].setClipRenderInfoVec(pClipRenderInfo);
	}
	return bInitSucess;
}

NXBOOL CVideoTrackInfo::setSurfaceTexture(void* pSurface)
{
	if( pSurface == NULL ) return FALSE;

	if( m_pSurfaceTexture != NULL )
	{
		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			pVideoRender->releaseSurfaceTexture(m_pSurfaceTexture);
			SAFE_RELEASE(pVideoRender);
		}
	}

	m_pSurfaceTexture = pSurface;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		if( m_FrameInfo[i].setSurfaceTexture(m_pSurfaceTexture) == FALSE )
		{
		}
	}
	return TRUE;
}

void* CVideoTrackInfo::getSurfaceTexture()
{
	return m_pSurfaceTexture;
}

NXBOOL CVideoTrackInfo::setTrackIndexInfo( unsigned int uiCurrentIndex, unsigned int uiTotalClipCount)
{
	m_uiTotalClipCount	= uiTotalClipCount;
	m_uiCurrentClipIndex	= uiCurrentIndex;	
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setTrackIndexInfo ID(%d) Info(%d %d)", __LINE__, m_uiTrackID, m_uiCurrentClipIndex, m_uiTotalClipCount);
	return TRUE;
}

NXBOOL CVideoTrackInfo::isLastTrack(unsigned int uClipTotalTime)
{
	if( uClipTotalTime == m_uiEndTime)
		return TRUE;

	return FALSE;
}

NXBOOL CVideoTrackInfo::setEffectInfo( unsigned int uiEffectStartTime, unsigned int uiEffectDuration, int iEffectOffset, int iEffectOverlap, const char* pEffect)
{
	m_uiEffectStartTime	= uiEffectStartTime;
	m_uiEffectDuration		= uiEffectDuration;
	m_iEffectOffect		= iEffectOffset;
	m_iEffectOverlap		= iEffectOverlap;
	setEffectID(pEffect);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setEffectInfo(Start:%d Dur:%d Offset:%d Overlap:%d, Effect(%s))", 
		__LINE__, uiEffectStartTime, uiEffectDuration, iEffectOffset, iEffectOverlap, pEffect);	
	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setTitleInfo(char* pStrTitle, const char* pEffect, unsigned int uiStartTime, unsigned int uiEndTime)
{
	setTitleEffectID(pEffect);
	m_uiTitleStartTime		= uiStartTime;
	m_uiTitleEndTime		= uiEndTime;
	m_bTitleEffectStart		= FALSE;

	if( m_pTitle )
	{
		nexSAL_MemFree(m_pTitle);
		m_pTitle = NULL;
	}	
	
	if( pStrTitle != NULL )
	{
		m_pTitle = (char*)nexSAL_MemAlloc(strlen(pStrTitle) + 1);
		strcpy(m_pTitle, pStrTitle);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setTitleInfo(Title:%s, Start:%d End:%d Effect(%p))", 
			__LINE__, pStrTitle, uiStartTime, uiEndTime, pEffect);	
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setTitleInfo(Title:null, Start:%d End:%d Effect(%p))", 
	__LINE__, uiStartTime, uiEndTime, pEffect);	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setFilterInfo(void* pEffect)
{
	m_pFilterEffect = pEffect;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setFilterInfo(%p)", __LINE__, pEffect);	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setRotateState(int iRotate)
{
	m_iRotateState = iRotate;

	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mRotateState	= m_iRotateState;
	}	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setRotateState(%d)", __LINE__, m_iRotateState);	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setBrightness(int iBrightness)
{
	m_iBrightness = iBrightness;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mBrightness	= m_iBrightness;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setContrast(int iContrast)
{
	m_iContrast = iContrast;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mContrast	= m_iContrast;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setSaturation(int iSaturation)
{
	m_iSaturation = iSaturation;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mSaturation	= m_iSaturation;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setHue(int iHue)
{
	m_iHue = iHue;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mHue	= m_iHue;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setTintcolor(int iTintcolor)
{
	m_iTintcolor = iTintcolor;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mTintcolor	= m_iTintcolor;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setLUT(int iLUT)
{
	m_iLUT = iLUT;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mLUT	= m_iLUT;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setCustomLUTA(int iLUT)
{
	m_iCustomLUT_A = iLUT;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mCustomLUT_A	= m_iCustomLUT_A;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setCustomLUTB(int iLUT)
{
	m_iCustomLUT_B = iLUT;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mCustomLUT_B	= m_iCustomLUT_B;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setCustomLUTPower(int iPower)
{
	m_iCustomLUT_Power = iPower;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mCustomLUT_Power	= m_iCustomLUT_Power;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setVignette(int iVignette)
{
	m_iVignette = iVignette;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mVignette	= m_iVignette;
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setVideoRenerMode(int iRenderMode)
{
	m_iVideoRenderMode = iRenderMode;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].mVideoRenderMode	= m_iVideoRenderMode;
	}	
	return TRUE;
}

char*CVideoTrackInfo:: getEffectID()
{
	return m_pEffectID;
}
int	CVideoTrackInfo::setEffectID(const char* pEffect)
{
	if( pEffect == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] pEffect is invalid", __LINE__);	
		return 1;
	}
	if( m_pEffectID != NULL)
	{
		nexSAL_MemFree(m_pEffectID);
		m_pEffectID = NULL;
	}

	m_pEffectID = (char*)nexSAL_MemAlloc( strlen(pEffect)+1);
	if( m_pEffectID == NULL)
	{
		return 1;
	}
	strcpy(m_pEffectID, pEffect);

	return 0;
}

char* CVideoTrackInfo::getTitleEffectID()
{
	return m_pTitleEffectID;	
}
int 	CVideoTrackInfo::setTitleEffectID(const char* pTitleEffect)
{

	if( pTitleEffect == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] pTitleEffect is invalid", __LINE__);	
		return 1;
	}
	if( m_pTitleEffectID != NULL)
	{
		nexSAL_MemFree(m_pTitleEffectID);
		m_pTitleEffectID = NULL;
	}

	m_pTitleEffectID = (char*)nexSAL_MemAlloc( strlen(pTitleEffect)+1);
	if( m_pTitleEffectID == NULL)
	{
		return 1;
	}
	strcpy(m_pTitleEffectID, pTitleEffect);
	
	return 0;
}
	
void CVideoTrackInfo::setCodecWrap(CNexCodecWrap* pCodec)
{
	CAutoLock m(m_CallbackLock);
	SAFE_RELEASE(m_pCodecWrap);
	m_pCodecWrap = pCodec;
	SAFE_ADDREF(m_pCodecWrap);

	for (int i = 0; i < VIDEO_FRAME_MAX_COUNT; i++)
	{
		m_FrameInfo[i].setCodecWrap(pCodec);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setCodecWrap(%p)", __LINE__, pCodec);
}

#ifdef _ANDROID
void CVideoTrackInfo::callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender)
{
	CAutoLock m(m_CallbackLock);
	if( m_pCodecWrap != NULL )
	{
		m_pCodecWrap->postFrameRenderCallback(pBuff, bRender);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VTrack.cpp %d] ID(%d) Call Render callback func(%p, %d)", __LINE__, m_uiTrackID, pBuff, bRender);
	}
}
#endif

#ifdef __APPLE__
void CVideoTrackInfo::callCodecWrap_renderDecodedFrame(unsigned char* pBuff)
{
    CAutoLock m(m_CallbackLock);
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->renderDecodedFrame(pBuff);
    }
}

void CVideoTrackInfo::callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender)
{
    CAutoLock m(m_CallbackLock);
    if( m_pCodecWrap != NULL )
    {
        m_pCodecWrap->releaseDecodedFrame(pBuff, isAfterRender);
    }
}
#endif

void CVideoTrackInfo::setVideoDecodeEnd(NXBOOL bEnd)
{
	m_bDecodingEnd = bEnd;
}

NXBOOL CVideoTrackInfo::getVideoDecodeEnd()
{
	return m_bDecodingEnd;
}

void CVideoTrackInfo::setVideoRenderEnd(NXBOOL bEnd)
{
	m_bRenderingEnd = bEnd;
}

void CVideoTrackInfo::setFirstVideoFrameDrop(void* pSurface)
{
	if( m_bFirstFrameFlag )
	{
		CFrameInfo* pFrame = getFrameOutBuffer();
		if( pFrame != NULL )
		{
			NXT_ThemeRenderer_AquireContext((NXT_HThemeRenderer)pSurface);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) setFirstVideoFrameDrop(%p, %d)", __LINE__, m_uiTrackID, pSurface, m_bFirstFrameFlag);	
			pFrame->dropFirstVideoTexture(pSurface);
			m_bFirstFrameFlag = FALSE;
			NXT_ThemeRenderer_ReleaseContext((NXT_HThemeRenderer)pSurface, FALSE);	
			DequeueFrameOutBuffer();
		}
	}
}

void CVideoTrackInfo::setVideoLayerTextureID(int iTextureID)
{
	m_iVideoLayerTextureID = iTextureID;
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].setVideoLayerTextureID(iTextureID);
	}	
}

void CVideoTrackInfo::setVideoDropRenderTime(unsigned int uTime)
{
	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].setVideoDropRenderTime(uTime);
	}	
}

NXBOOL CVideoTrackInfo::initVideoSurface(void* pDDraw, int iBitCount)
{
	if( pDDraw == NULL ) return FALSE;

	for( int i = 0; i < m_iFrameInfoCount; i++ )
	{
		m_FrameInfo[i].initVideoSurface(pDDraw, iBitCount);
	}
	return TRUE;
}

NXBOOL CVideoTrackInfo::isImageClip()
{
	return m_ClipType == CLIPTYPE_IMAGE ? TRUE : FALSE;
}

NXBOOL CVideoTrackInfo::isVideoClip()
{
	return m_ClipType == CLIPTYPE_VIDEO ? TRUE : FALSE;
}

NXBOOL CVideoTrackInfo::isVideoLayer()
{
	return m_ClipType == CLIPTYPE_VIDEO_LAYER ? TRUE : FALSE;
}

int CVideoTrackInfo::applyClipOptions(NXT_HThemeRenderer hThemeRenderer, NXT_TextureID eTextureID)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d)applyClipOptions TextureID(%d)", __LINE__, m_uiTrackID, eTextureID);	

	NXT_ThemeRenderer_SetTextureRotation(hThemeRenderer, eTextureID, (unsigned int)m_iRotateState);
	NXT_ThemeRenderer_SetTextureColorAdjust(hThemeRenderer, eTextureID, m_iBrightness, m_iContrast, m_iSaturation, m_iTintcolor, m_iHue, m_iVignette);
	return 0;
}

// return value
// 0 : Effect End
// 1 : while Clip Effect apply
// 2 : while Title Effect apply
int CVideoTrackInfo::applyClipEffect(NXT_HThemeRenderer hThemeRenderer, unsigned int uiCurrentTime)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] applyEffect In(%d %d %d %d %p) %d", 
	// 	__LINE__, m_uiEffectStartTime, m_uiEffectDuration, m_iEffectOffect, m_iEffectOverlap, m_hEffect, uiCurrentTime);

	if( m_bEffectApplyEnd || m_uiEffectStartTime > uiCurrentTime )
	//	if( m_bEffectApplyEnd || m_uiEffectStartTime > uiCurrentTime || uiCurrentTime > (m_uiEffectStartTime + m_uiEffectDuration ))
	{
		// NXT_ThemeRenderer_ClearTransitionEffect(hThemeRenderer);
		// NXT_ThemeRenderer_ClearClipEffect(hThemeRenderer);		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VTrack.cpp %d] ID(%d) applyEffect not time(Cur:%d Start:%d Dur:%d)", 
				__LINE__, m_uiTrackID, uiCurrentTime, m_uiEffectStartTime, m_uiEffectDuration);	
		m_bClipEffectStart = FALSE;
		return 0;
	}

	float fTime = (float)(uiCurrentTime - m_uiEffectStartTime) / m_uiEffectDuration;
	fTime = fTime > 0.96 ? 0.99: fTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) applyEffect TimeInfo(Cur:%d Start:%d Dur:%d)", 
		__LINE__, m_uiTrackID, uiCurrentTime, m_uiEffectStartTime, m_uiEffectDuration);	

	if( m_bClipEffectStart )
	{
		if( uiCurrentTime > (m_uiEffectStartTime + m_uiEffectDuration) )
		{
			NXT_ThemeRenderer_ClearTransitionEffect(hThemeRenderer);
			// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);	
			NXT_ThemeRenderer_SwapTextures(hThemeRenderer, NXT_TextureID_Video_1, NXT_TextureID_Video_2);
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) applyEffect Effect End", __LINE__, m_uiTrackID);	
			m_bClipEffectStart = FALSE;
			m_bEffectApplyEnd	 = TRUE;
			return 100;
		}

		// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);	
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) applyEffect Effect process(T: %f)", __LINE__, m_uiTrackID, fTime);	

		return int(fTime * 100);
	}

	if( m_uiEffectStartTime <= uiCurrentTime &&  uiCurrentTime < (m_uiEffectStartTime +  m_uiEffectDuration) )
	{
		NXT_ThemeRenderer_SetTransitionEffect(	hThemeRenderer,
												m_pEffectID,  // ID of transition effect, or NULL to clear current effect
												m_pTitle,
												m_uiCurrentClipIndex,         // Index of the transition's clip (the clip before the transition), from 0
												m_uiTotalClipCount,    // Total number of clips in the project
												m_uiEffectStartTime,   // Start time for this effect
												m_uiEffectStartTime +  m_uiEffectDuration );    // End time for this effect

		// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);		

		m_bClipEffectStart = TRUE;
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) applyEffect Effect Start(%d %d %d) IndexInfo(%d %d)",
			__LINE__, m_uiTrackID, uiCurrentTime, m_uiEffectStartTime, m_uiEffectDuration, m_uiTotalClipCount, m_uiCurrentClipIndex);	
		return  int(fTime * 100);
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] applyEffect Out", __LINE__);	
	return 0;
}

int CVideoTrackInfo::applyTitleEffect(NXT_HThemeRenderer hThemeRenderer, unsigned int uiCurrentTime)
{
	 // nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) applyTitleEffect In(%d %d) Flag : %d, Cur(%d)", 
	 // 	 __LINE__, m_uiTrackID, m_uiTitleStartTime, m_uiTitleEndTime, m_bTitleEffectStart, uiCurrentTime);

	// for mantis 7765 problem
	unsigned int uiTitleEndTime = m_uiTitleEndTime;
	// if( m_uiTotalClipCount != (m_uiCurrentClipIndex + 1) )

	unsigned int uTotalTime = 0;
	CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
	if( pProjectMng )
	{
		CClipList* pList = pProjectMng ->getClipList();
		if( pList )
		{
			uTotalTime = pList->getClipTotalTime();
			SAFE_RELEASE(pList);
		}
		SAFE_RELEASE(pProjectMng);
	}

	if( isLastTrack(uTotalTime) == FALSE )
	{
		uiTitleEndTime = uiTitleEndTime < 20 ? 0 : uiTitleEndTime - 20;
	}

	if( m_uiStartTime > uiCurrentTime || uiCurrentTime > m_uiEndTime )
	{
		m_bTitleEffectStart = FALSE;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Title Effect Not Time(c:%d s:%d e:%d)",  __LINE__, m_uiTrackID, uiCurrentTime, m_uiTitleStartTime, uiTitleEndTime);
		return 0;
	}

	if( m_uiTitleStartTime > uiCurrentTime || uiCurrentTime > uiTitleEndTime )
	{
		m_bTitleEffectStart = FALSE;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Title Effect Not Time(c:%d s:%d e:%d)",  __LINE__, m_uiTrackID, uiCurrentTime, m_uiTitleStartTime, uiTitleEndTime);
		return 0;
	}

	if( m_bTitleEffectStart )
	{
		if( uiCurrentTime > uiTitleEndTime )
		{
			m_bTitleEffectStart = FALSE;
			NXT_ThemeRenderer_ClearClipEffect(hThemeRenderer);
			// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) apply Title Effect End(%d %d %d) (%s)",  __LINE__, m_uiTrackID, uiCurrentTime, m_uiTitleStartTime, m_uiTitleEndTime, m_pTitle);
			return 0;
		}
		// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) apply Title Effect Process(%d %d %d) (%s)",  __LINE__, m_uiTrackID, uiCurrentTime, m_uiTitleStartTime, m_uiTitleEndTime, m_pTitle);
		return 2;
	}
	NXT_ThemeRenderer_SetClipEffect(hThemeRenderer,
									m_pTitleEffectID,  // ID of clip effect, or NULL to clear current effect
									m_pTitle,
									m_uiCurrentClipIndex,         // Index of this clip, from 0
									m_uiTotalClipCount,    // Total number of clips in the project
									m_uiStartTime,     // Start time for this clip
									m_uiEndTime,       // End time for this clip
									m_uiTitleStartTime,   // Start time for this effect (must be >= clipStartTime)
									m_uiTitleEndTime );    // End time for this effect (must be <= clipEndTime)	
	                                     
		// NXT_ThemeRenderer_SetCTS(hThemeRenderer, uiCurrentTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) apply Title Effect Start(%d %d %d) (%s)",  __LINE__, m_uiTrackID, uiCurrentTime, m_uiTitleStartTime, m_uiTitleEndTime, m_pTitle);
	m_bTitleEffectStart = TRUE;
	return 2;
}

void CVideoTrackInfo::resetEffect(NXT_HThemeRenderer hThemeRenderer)
{
	if( hThemeRenderer == NULL ) return;

	NXT_ThemeRenderer_ClearTransitionEffect(hThemeRenderer);
	// NXT_ThemeRenderer_ClearClipEffect(hThemeRenderer);		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VTrack.cpp %d] resetEffect done", __LINE__);
	m_bClipEffectStart = FALSE;
}

char* CVideoTrackInfo::getAppliedTitleEffect()
{
	if( m_bTitleEffectStart )
		return m_pTitleEffectID;
	return NULL;
}

// 0 : draw Time
// 1 : no available frame
// 2 : fast frame
// 3 : before track time
// 4 : after track time
// 5 : after track time of last clip
// 6 : track not started
// 7 : track decode end
int CVideoTrackInfo::existDrawFrame(unsigned int uiCurrentTime, NXBOOL bWait)
{
	CFrameInfo* pFrame = NULL;

	if( isTrackTime(uiCurrentTime) == FALSE )
	{
		if( m_uiStartTime > uiCurrentTime )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, m_uLogCnt++%30, "[VTrack.cpp %d] ID(%d) Before Track Time(start:%d c:%d) ret 3", __LINE__, m_uiTrackID, m_uiStartTime, uiCurrentTime);
			return 3;
		}

		if( m_uiEndTime < uiCurrentTime )
		{
			unsigned int uTotalTime = 0;
			CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
			if( pProjectMng )
			{
				CClipList* pList = pProjectMng ->getClipList();
				if( pList )
				{
					uTotalTime = pList->getClipTotalTime();
					SAFE_RELEASE(pList);
				}
				SAFE_RELEASE(pProjectMng);
			}

			if( isLastTrack(uTotalTime) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VTrack.cpp %d] ID(%d) After Track Time of list track (c:%d) ret 5", __LINE__, m_uiTrackID, uiCurrentTime);
				return 5;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) After Track Time(c:%d) ret 4", __LINE__, m_uiTrackID, uiCurrentTime);
			return 4;
		}
	}

	if( isImageClip() )
	{
		return 0;
	}
	
	if( isTrackStarted() == FALSE )
	{
		if( getVideoDecodeEnd() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) Track decode end(c:%d) ret 7", __LINE__, m_uiTrackID, uiCurrentTime);
			return 7;
		}
		
		if( bWait )
		{
			int iWaitTime = 2000;
			while( iWaitTime > 0 && m_bTrackPaused == FALSE)
			{
				pFrame = getFrameOutBuffer();
				if( pFrame == NULL )
				{
					iWaitTime -= 10;
					nexSAL_TaskSleep(10);
					continue;
				}
				break;
			}
			
			if( pFrame == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) Track not  started(c:%d) ret 6", __LINE__, m_uiTrackID, uiCurrentTime);
				return 6;
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) Track not  started(c:%d) ret 6", __LINE__, m_uiTrackID, uiCurrentTime);
			return 6;
		}
	}
	
CHECK_NEXTFRAME_AFTER_DROP:
	if( bWait )
	{
		int iWaitTime = 3000;
		while( iWaitTime > 0 && m_bTrackPaused == FALSE)
		{
			pFrame = getFrameOutBuffer();
			if( pFrame == NULL )
			{
				if( getVideoDecodeEnd() )
				{
					if( m_pDrawInfos != NULL && m_pDrawInfos->isDrawTime(uiCurrentTime) )
					{
						return 0;
					}

					unsigned int uTotalTime = 0;
					CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
					if( pProjectMng )
					{
						CClipList* pList = pProjectMng ->getClipList();
						if( pList )
						{
							uTotalTime = pList->getClipTotalTime();
							SAFE_RELEASE(pList);
						}
						SAFE_RELEASE(pProjectMng);
					}

					if( isLastTrack(uTotalTime) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) After Track Time of list track (c:%d) ret 5", __LINE__, m_uiTrackID, uiCurrentTime);
						return 5;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) Track decode end(c:%d) ret 7", __LINE__, m_uiTrackID, uiCurrentTime);
					return 7;
				}
				iWaitTime -= 10;
				nexSAL_TaskSleep(10);
				continue;
			}
			break;
		}
	}
	else
	{
		pFrame = getFrameOutBuffer();
	}
	
	if( pFrame == NULL )
	{
		if( getVideoDecodeEnd() &&  m_pDrawInfos != NULL )
		{
			if( m_pDrawInfos->isDrawTime(uiCurrentTime) ) 
			{
				return 0;
			}
		}
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) VideoFrame not available ret 1", __LINE__, m_uiTrackID);
		return 1;
	}
	
	int iRet = pFrame->checkDrawTime(uiCurrentTime, TRUE);
	if( iRet == 1 )  // late frame
	{
		if(m_iDropStartTime == 0xffffffff)
			m_iDropStartTime = uiCurrentTime;

		m_iDropCount++;
		if(uiCurrentTime - m_iDropStartTime > 1000)
		{
			m_iDropLastCount = m_iDropCount;
			m_iDropCount = 0;
			m_iDropStartTime += 1000;
		}
	
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Drop VideoFrame(f:%d c:%d) Gap(%d)", __LINE__, m_uiTrackID, pFrame->m_uiTime, uiCurrentTime, uiCurrentTime - pFrame->m_uiTime);
		DequeueFrameOutBuffer();
		pFrame = NULL;
#ifdef _ANDROID
		PerformanceChecker_Catch(PC_ERROR_FRAME_DROP);
#endif
		goto CHECK_NEXTFRAME_AFTER_DROP;
	}
	else if( iRet == 2 ) // fast frame
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VTrack.cpp %d] ID(%d) Fast VideoFrame(f:%d c:%d) Gap(%d) ret 2", __LINE__, m_uiTrackID, pFrame->m_uiTime, uiCurrentTime, pFrame->m_uiTime - uiCurrentTime);
		return 2;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VTrack.cpp %d] ID(%d) Draw VideoFrame(f:%d c:%d) Gap(%d)", __LINE__, m_uiTrackID, pFrame->m_uiTime, uiCurrentTime, uiCurrentTime - pFrame->m_uiTime);
	return 0;
}

NXBOOL CVideoTrackInfo::isTrackTime(unsigned int uiTime)
{
	if( m_uiStartTime <= uiTime && uiTime <= (m_uiEndTime ) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VTrack.cpp %d] ID(%d) TrackTime(S(%d) E(%d) Cur(%d) EffectEnd(%d))", 
			__LINE__, m_uiTrackID, m_uiStartTime, m_uiEndTime, uiTime, m_bEffectApplyEnd);

		if( m_bEffectApplyEnd )
			return FALSE;
		return TRUE;
	}

	// For ZIRA 1925 about video freezing issue on image exported clip.
	if( (uiTime < m_uiEndTime + 100) &&  m_bClipEffectStart )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VTrack.cpp %d] ID(%d) TrackTime(S(%d) E(%d) Cur(%d) EffectEnd(%d))", 
			__LINE__, m_uiTrackID, m_uiStartTime, m_uiEndTime, uiTime, m_bEffectApplyEnd);
		return TRUE;
	}

	if( m_pDrawInfos != NULL )
	{
		if( m_pDrawInfos->isDrawTime(uiTime) )
		{
			return TRUE;
		}
	}

	m_bClipEffectStart = FALSE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VTrack.cpp %d] ID(%d) not TrackTime(Start(%d) End(%d) Cur(%d))", __LINE__, m_uiTrackID, m_uiStartTime, m_uiEndTime, uiTime);
	return FALSE;
}

NXBOOL CVideoTrackInfo::isFaceDetectProcessed(unsigned int uiTime){

	CClipVideoRenderInfo* pvideo_render_info = getActiveRenderInfo(uiTime);
	if(!pvideo_render_info)
		return TRUE;
	if(pvideo_render_info->mFaceDetectProcessed){

		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] TID(%d) isFaceDetectProcessed - false", 
			__LINE__, this->m_uiTrackID);

	return FALSE;
} 

NXBOOL CVideoTrackInfo::isTrackStarted()
{
	return m_bTrackStart;
}

NXBOOL CVideoTrackInfo::isTrackEnded()
{
	if( m_bTrackStart )
	{
		CFrameInfo* pFrameInfo = getFrameOutBuffer();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] TID(%d) isTrackEnded(Frame : %p,  ClipEffectStart : %d) Time(%d %d)", 
			__LINE__, this->m_uiTrackID, pFrameInfo, m_bClipEffectStart, m_uiStartTime, m_uiEndTime);	

		if( pFrameInfo == NULL && m_bClipEffectStart == FALSE )
		{
			return TRUE;
		}

		return FALSE;
	}

	if( m_uiEffectDuration <= 0 )
	{
		return TRUE;
	}
	
	if( m_bEffectApplyEnd && m_bClipEffectStart == FALSE )
		return TRUE;
	return FALSE;
}

 NXBOOL CVideoTrackInfo::isEffectApplyEnd()
 {
 	return m_bEffectApplyEnd;
 }
	
CFrameInfo* CVideoTrackInfo::getFrameInBuffer()
{
	CAutoLock m(m_FrameQueueLock);
	if( m_FrameInVec.size() <= 0 )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getFrameInBuffer InQueue is empty()", __LINE__);
		return NULL;
	}
	return m_FrameInVec[0];
}

void CVideoTrackInfo::EnqueueFrameInBuffer()
{
 	CAutoLock m(m_FrameQueueLock);
	if( m_FrameInVec.size() <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) EnqueueFrameInBuffer invalid operation id(%d) size(%zu)", __LINE__, m_uiTrackID, m_FrameInVec.size());
		return;
	}

	if(m_bRenderingEnd)
	{
#if defined(_ANDROID)
		callCodecWrapPostCallback(m_FrameInVec[0]->m_pMediaBuff - 1, m_FrameInVec[0]->m_bRenderMediaBuff);
#elif defined(__APPLE__)
		callCodecWrap_releaseDecodedFrame(m_FrameInVec[0]->m_pImageBuff, FALSE);
#endif
		// if m_FrameInVec[0]->m_bRenderMediaBuff may be m_FrameInVec[0]->m_bRenderMediaBuff = false.
		if ( m_FrameInVec[0]->m_bRenderMediaBuff )
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] EnqueueFrameInBuffer. render option is true. what happened!!!", __LINE__);
		m_FrameInVec[0]->m_pMediaBuff = NULL;
#ifdef __APPLE__
        m_FrameInVec[0]->m_pImageBuff = NULL;
#endif
		m_FrameInVec[0]->m_bRenderMediaBuff = FALSE;
		return;
	}

	m_FrameOutVec.insert(m_FrameOutVec.end(), m_FrameInVec[0]);
	m_FrameInVec.erase(m_FrameInVec.begin());

	if( m_bTrackStart == FALSE )
	{
		if( m_ClipType == CLIPTYPE_IMAGE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Image track was stated", __LINE__, m_uiTrackID);
			m_bTrackStart = TRUE;
		}
		else if( (m_ClipType == CLIPTYPE_VIDEO && m_FrameOutVec.size() >= 1) || (m_ClipType == CLIPTYPE_VIDEO_LAYER && m_FrameOutVec.size() >= 1) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Video track was started (%d)", __LINE__, m_uiTrackID, m_FrameOutVec[0]->m_uiTime);
			m_bTrackStart = TRUE;
		}
	}
}

void CVideoTrackInfo::ReorderFrameInBuffer()
{
 	CAutoLock m(m_FrameQueueLock);
	if( m_FrameInVec.size() <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) ReorderFrameInBuffer invalid operation id(%d) size(%zu)", __LINE__, m_uiTrackID, m_FrameInVec.size());
		return;
	}

	m_FrameInVec.insert(m_FrameInVec.end(), m_FrameInVec[0]);
	m_FrameInVec.erase(m_FrameInVec.begin());
}

CFrameInfo* CVideoTrackInfo::getFrameOutBuffer(NXBOOL bCheckTime)
{
	CAutoLock m(m_FrameQueueLock);
	if( m_FrameOutVec.size() <= 0 )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d)getFrameOutBuffer OutQueue is empty()", __LINE__, m_uiTrackID);
		return NULL;
	}

	if( bCheckTime == FALSE )
	{
		return m_FrameOutVec[0];
	}

	CFrameInfo* pFrame = m_FrameOutVec[0];
	for(int i = 1; i < m_FrameOutVec.size(); i++)
	{
		if( pFrame->m_uiTime > m_FrameOutVec[i]->m_uiTime )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d)getFrameOutBuffer TimeCheck(%d %d %d %zu)", 
				__LINE__, m_uiTrackID, pFrame->m_uiTime, m_FrameOutVec[i]->m_uiTime, i, m_FrameOutVec.size());
			pFrame = m_FrameOutVec[i];
		}
	}
	return pFrame;
}

void CVideoTrackInfo::DequeueAllFrameOutBuffer(CFrameInfo* pFrameInfo, unsigned int* pTime)
{
	CAutoLock m(m_FrameQueueLock);
	if (m_ClipType == CLIPTYPE_IMAGE) return;
	if (m_FrameOutVec.size() <= 0)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) DequeueFrameOutBuffer invalid operation size(%zu)", __LINE__, m_uiTrackID, m_FrameOutVec.size());
		return;
	}

	if (pFrameInfo == NULL)
	{
		while (m_FrameOutVec.size()){
			if (m_FrameOutVec[0]->mColorFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY &&
#if defined(_ANDROID)
                m_FrameOutVec[0]->m_pMediaBuff
#elif defined(__APPLE__)
                m_FrameOutVec[0]->m_pImageBuff
#endif
                )
			{
#if defined(_ANDROID)
				callCodecWrapPostCallback(m_FrameOutVec[0]->m_pMediaBuff - 1, m_FrameOutVec[0]->m_bRenderMediaBuff);
#elif defined(__APPLE__)
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) DequeueAllFrameOutBuffer ImageBuffer release pts=%d", __LINE__, m_uiTrackID, getPTSFromImageBuffer((void*)m_FrameOutVec[0]->m_pImageBuff));
                callCodecWrap_releaseDecodedFrame(m_FrameOutVec[0]->m_pImageBuff, FALSE);
#endif
				m_FrameOutVec[0]->m_pMediaBuff = NULL;
#ifdef __APPLE__
                m_FrameOutVec[0]->m_pImageBuff = NULL;
#endif
				m_FrameOutVec[0]->m_bRenderMediaBuff = FALSE;
			}

			if (pTime != NULL)
				*pTime = m_FrameOutVec[0]->m_uiTime;
			m_FrameInVec.insert(m_FrameInVec.end(), m_FrameOutVec[0]);
			m_FrameOutVec.erase(m_FrameOutVec.begin());
		}
		return;
	}

	if (pTime != NULL)
		*pTime = m_FrameOutVec[0]->m_uiTime;
	m_FrameInVec.insert(m_FrameInVec.end(), pFrameInfo);
	VideoFrameVec::iterator i = m_FrameOutVec.begin();
	for ( ; i != m_FrameOutVec.end(); i++)
	{
		CFrameInfo* pFrame = (CFrameInfo*)*i;
		if (pFrameInfo == pFrame)
		{
			pFrameInfo->mColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
			pFrameInfo->m_bPreviewThumb = 0;
			pFrameInfo->m_pY = NULL;
			m_FrameOutVec.erase(i);
			break;
		}
	}
}

void CVideoTrackInfo::DequeueFrameOutBuffer(CFrameInfo* pFrameInfo, unsigned int* pTime)
{
	CAutoLock m(m_FrameQueueLock);
	if( m_ClipType == CLIPTYPE_IMAGE ) return;
	if( m_FrameOutVec.size() <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) DequeueFrameOutBuffer invalid operation size(%zu)", __LINE__, m_uiTrackID, m_FrameOutVec.size());
		return;
	}

	if( pFrameInfo == NULL )
	{
		if(m_FrameOutVec[0]->m_bPreviewThumb)
		{
			m_FrameOutVec[0]->mColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
			m_FrameOutVec[0]->m_bPreviewThumb = 0;
			m_FrameOutVec[0]->m_pY = NULL;
		}
	
		if(	m_FrameOutVec[0]->mColorFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY &&
#if defined(_ANDROID)
                m_FrameOutVec[0]->m_pMediaBuff
#elif defined(__APPLE__)
                m_FrameOutVec[0]->m_pImageBuff
#endif
                )
		{
			if( m_FrameOutVec[0]->m_bRenderMediaBuff == FALSE )
			{
#if defined(_ANDROID)
				callCodecWrapPostCallback(m_FrameOutVec[0]->m_pMediaBuff - 1, m_FrameOutVec[0]->m_bRenderMediaBuff);
#elif defined(__APPLE__)
                if ( m_FrameOutVec[0]->m_pImageBuff)
                    callCodecWrap_releaseDecodedFrame(m_FrameOutVec[0]->m_pImageBuff, FALSE);
#endif
				// m_FrameInVec[0]->m_bRenderMediaBuff may be false.
				if ( m_FrameInVec[0]->m_bRenderMediaBuff )
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] EnqueueFrameInBuffer. render option is true. what happened!!!", __LINE__);
			}
			m_FrameOutVec[0]->m_pMediaBuff = NULL;
#ifdef __APPLE__
            m_FrameOutVec[0]->m_pImageBuff = NULL;
#endif
            
			m_FrameOutVec[0]->m_bRenderMediaBuff = FALSE;
		}

		if( pTime != NULL )
			*pTime = m_FrameOutVec[0]->m_uiTime;
		m_FrameInVec.insert(m_FrameInVec.end(), m_FrameOutVec[0]);
		m_FrameOutVec.erase(m_FrameOutVec.begin());
		return;
	}

	if( pTime != NULL )
		*pTime = m_FrameOutVec[0]->m_uiTime;
	m_FrameInVec.insert(m_FrameInVec.end(), pFrameInfo);
	VideoFrameVec::iterator i = m_FrameOutVec.begin();
	for( ; i != m_FrameOutVec.end(); i++)
	{
		CFrameInfo* pFrame = (CFrameInfo*)*i;
		if( pFrameInfo == pFrame )
		{
			if(pFrameInfo->m_bPreviewThumb)
			{
				pFrameInfo->mColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
				pFrameInfo->m_bPreviewThumb = 0;
				pFrameInfo->m_pY = NULL;
			}
			m_FrameOutVec.erase(i);
			break;
		}
	}
}

void CVideoTrackInfo::clearImageTrackDrawFlag()
{
	if( m_ClipType != CLIPTYPE_IMAGE ) return;

	m_FrameInfo[0].clearTextureID();

	m_bClipEffectStart		= FALSE;	
	m_bEffectApplyEnd		= FALSE;
	m_bTitleEffectStart		= FALSE;	
}

void CVideoTrackInfo::updateTrackTime(unsigned int uiStartTime, unsigned uiEndTime, unsigned int uiStartTrimTime, unsigned int uiEndTrimTime)
{
	m_uiStartTime	= uiStartTime;
	m_uiEndTime		= uiEndTime ;

	m_FrameInfo[0].updateFrameTime(uiStartTime, uiEndTime);

	if( m_pSource )
	{
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(uiStartTrimTime, uiEndTrimTime);
	}
}

void CVideoTrackInfo::updateImageTrackRect(RECT rcStartRect, RECT rcEndRect)
{
	m_FrameInfo[0].updateImageFrameRect(rcStartRect, rcEndRect);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Image Track position update(%d %d %d %d) (%d %d %d %d)", __LINE__, m_uiTrackID, rcStartRect.left, rcStartRect.top, rcStartRect.right, rcStartRect.bottom, rcEndRect.left, rcEndRect.top, rcEndRect.right, rcEndRect.bottom);
}

void CVideoTrackInfo::setMotionTracked(NXBOOL tracked)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) setMotionTracked(%d)", __LINE__, m_uiTrackID, tracked);
	m_isMotionTracked = tracked;
}

NXBOOL CVideoTrackInfo::getMotionTracked()
{
	return m_isMotionTracked;
}

void CVideoTrackInfo::setReUseSurfaceTexture(NXBOOL bReUse)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) setReUseSurfaceTexture(%d)", __LINE__, m_uiTrackID, bReUse);
	m_bReUseSurfaceTexture = bReUse;
}

void CVideoTrackInfo::printTrackInfo()
{
	const char* pClipType = NULL;
	switch(m_ClipType)
	{
		case CLIPTYPE_IMAGE:
			pClipType = "Image";
			break;
		case CLIPTYPE_TEXT:
			pClipType = "Text";
			break;
		case CLIPTYPE_AUDIO:
			pClipType = "Audio";
			break;
		case CLIPTYPE_VIDEO:
			pClipType = "Video";
			break;
		case CLIPTYPE_VIDEO_LAYER:
			pClipType = "Video Layer";
			break;
		default:
			pClipType = "unKnown";
			break;
	};
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] -------------- Track Info ------------------", __LINE__);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] RefCnt: %d", __LINE__, GetRefCnt());
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] m_uiClipID: %d", __LINE__, m_uiTrackID);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] m_ClipType: %s", __LINE__, pClipType);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] StartTime(%d) EndTime(%d)", __LINE__, m_uiStartTime, m_uiEndTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] m_iWidth(%d) m_iHeight(%d) ", __LINE__, m_iWidth, m_iHeight);
	if( m_pTitle )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] m_pTitle: %s", __LINE__, m_pTitle);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] m_uiTitle StartTime(%d) EndTime(%d)", __LINE__, m_uiTitleStartTime, m_uiTitleEndTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ClipEffect(%s) TitleEffect(%s)", __LINE__, m_pEffectID, m_pTitleEffectID);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Effect Time(Start:%d, Dur:%d, Offset:%d, Overlap:%d)", __LINE__, 
		m_uiEffectStartTime, m_uiEffectDuration, m_iEffectOffect, m_iEffectOverlap);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] -------------- Track Info End -----------------", __LINE__);
}

void CVideoTrackInfo::printMatrixInfo(float* pMatrix)
{
	if( pMatrix == NULL ) return;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] -------------- Matrix Info ------------------", __LINE__);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] | %3.3f %3.3f %3.3f |", __LINE__, pMatrix[0], pMatrix[1], pMatrix[2]);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] | %3.3f %3.3f %3.3f |", __LINE__, pMatrix[3], pMatrix[4], pMatrix[5]);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] | %3.3f %3.3f %3.3f |", __LINE__, pMatrix[6], pMatrix[7], pMatrix[8]);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] -------------- Matrix Info End -----------------", __LINE__);
}

NXBOOL CVideoTrackInfo::isReaderSeted()
{
	return m_pSource != NULL ? TRUE : FALSE;
}

NXBOOL CVideoTrackInfo::setSourceHandle(CClipItem* pClip, CNexSource* pSource)
{
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
	NXUINT8*	pEnhancedDSI				= NULL;
	NXUINT32	uiEnhancedDSISize			= 0;
	NXUINT32	uiH264ProfileLevelID		= 0;

	unsigned int	eRet					= 0;
	NXBOOL			bRet					= FALSE;

	int 			iDSINALHeaderSize		= 0;
	int 			iFrameNALHeaderSize		= 0;

	unsigned int	uiVideoType				= 0;
	int				iWidth					= 0;
	int				iHeight					= 0;
	int				iPitch					= 0;

	NXINT64			isUseIframeVDecInit		= 0;
	
	IRectangle*		pRec					= NULL;
	
	float*			pMatrix					= NULL;
	int				iMatrixSize				= 0;

	unsigned char* puuid = NULL;
	int uuid_size = 0;
/* comments related to iFirstVideoFrame will be removed.
#ifdef FOR_TEST_FIRST_DECODE_SKIP
	m_iFirstVideoFrameCounter = 0;
#else
	m_iFirstVideoFrameCounter					= 2;
#endif
*/
	CClipVideoRenderInfo videoInfo;
	//memset(&videoInfo, 0x00, sizeof(CClipVideoRenderInfo));

	videoInfo.mFaceDetectProcessed = 1;

	if( pSource == NULL )
	{
		SAFE_RELEASE(m_pSource);
		if( m_pCodecWrap != NULL )
		{
			m_pCodecWrap->deinitDecoder();
			SAFE_RELEASE(m_pCodecWrap);
		}
		return FALSE;
	}

	if( m_pSource == pSource )
	{
		return TRUE;
	}

	SAFE_RELEASE(m_pSource);
	m_pSource = pSource;
	SAFE_ADDREF(m_pSource);

	if( m_pSource->isVideoExist() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VTrack.cpp %d] This Clip do not has video track", __LINE__);
		goto SET_READER_FAIL;
	}	

	iDSINALHeaderSize	= m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();

	SAFE_RELEASE(m_pCodecWrap);

	m_pCodecWrap = new CNexCodecWrap();
	if( m_pCodecWrap == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Alloc CodecWrap failed", __LINE__);
		goto SET_READER_FAIL;
	}

	uiVideoType = m_pSource->getVideoObjectType();
	switch(uiVideoType)
	{
		case eNEX_CODEC_V_MPEG4V:
		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			break;
		default:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Not Support Video Codec(0x%x)", __LINE__, uiVideoType);
			goto SET_READER_FAIL;
	}

	if(	m_iWidth * m_iHeight > CNexCodecManager::getHardwareMemRemainSize() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] have to use sw video codec because codec not available", __LINE__);
		if( uiVideoType == eNEX_CODEC_V_H264 && CNexVideoEditor::m_bSuppoertSWH264Codec )
		{
			uiVideoType = eNEX_CODEC_V_H264_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Used AVC Decoder(0x%x)", __LINE__, uiVideoType);
		}
		else if( uiVideoType == eNEX_CODEC_V_MPEG4V && CNexVideoEditor::m_bSuppoertSWMP4Codec )
		{
			uiVideoType = eNEX_CODEC_V_MPEG4V_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Used Mpeg4V Decoder(0x%x)", __LINE__, uiVideoType);
		}
	}
	else
	{
#ifdef _ANDROID
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Used HW Decoder(0x%x)", __LINE__, uiVideoType);

		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			void* pSurfaceTexture = pVideoRender->getSurfaceTexture();
			if( pSurfaceTexture == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] get surface texture failed", __LINE__);
				goto SET_READER_FAIL;
			}
			m_pCodecWrap->setSurfaceTexture(pSurfaceTexture);
			setSurfaceTexture(pSurfaceTexture);
			SAFE_RELEASE(pVideoRender);
		}
#endif
	}
	m_pSource->getVideoResolution((unsigned int*)&iWidth, (unsigned int*)&iHeight);

	bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
									NEXCAL_MODE_DECODER,
									uiVideoType,
									iWidth,
									iHeight,
									iWidth);

	if( bRet == FALSE )
	{
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] Get Codec Failed(0x%x)", __LINE__, uiVideoType);
		goto SET_READER_FAIL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VTrack.cpp %d] Get Codec end(%p)", __LINE__, uiVideoType, m_pCodecWrap->getCodecHandle());

	if( nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	if( m_pCodecWrap->isHardwareCodec() == FALSE )
	{
		nexCAL_VideoDecoderSetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_VIDEO_BUFFER_NV12);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ColorFormat(0x%x) NAL Size(D:%d, F:%d)", __LINE__, m_pCodecWrap->getFormat(), iDSINALHeaderSize, iFrameNALHeaderSize);
	
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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] This Video frame is invalid.\n", __LINE__);
				nexSAL_TaskSleep(5);
				continue;
			}

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] This Video frame Info(%d %d %d)", __LINE__, m_pSource->getVideoObjectType(), iFrameNALHeaderSize, m_pSource->getFrameFormat());
			nexSAL_TaskSleep(5);

			// Codec �ʱ�ȭ�� IDR �������� ���� ���� �ʾ� IFrame�� ã���� ��.
			isIntraFrame = NexCodecUtil_IsSeekableFrame((NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
			if( isIntraFrame )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] I-frame searched...CTS(%u)\n", __LINE__, uiDTS);
				break;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] P-frame searched...CTS(%u)\n", __LINE__, uiDTS);
			}
		}
		else if( uiRet == _SRC_GETFRAME_NEEDBUFFERING )
		{
			nexSAL_TaskSleep(5);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] Need buffering Not normal condition", __LINE__, uiRet);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] Get Frame fail while decoder init(%d)\n", __LINE__, uiRet);
			goto SET_READER_FAIL;
		}
	}

	//yoon
	m_pSource->getVideoTrackUUID(&puuid,&uuid_size);
	m_iVideoRenderMode = is360Video(puuid,uuid_size);

	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );


			eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
												pBaseDSI,
												uiBaseDSISize,
												pFrame,
												uiFrameSize,
												NULL,
												NULL,
												0,
												&iWidth,
												&iHeight,
												&iPitch,
												m_pSource->getVideoFrameRate(),												
												NEXCAL_VDEC_MODE_NONE);

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp  %d] Video Codec init Error(%d)", __LINE__, eRet);
				goto SET_READER_FAIL;
			}

			m_iWidth	= iWidth;
			m_iHeight	= iHeight;
			m_iPitch	= 0;
				
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Video Codec init End(w:%d h:%d)", __LINE__, iWidth, iHeight);
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_iWidth		= iWidth;
			m_iHeight		= iHeight;
			m_iPitch		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Dec init(%p %d)", __LINE__, pBaseDSI, uiBaseDSISize);

			{
				unsigned char*	pTmpBuf		= NULL;
				unsigned int		uiTmpBufSize	= 0;
				if( NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES == isUseIframeVDecInit )
				{
					pTmpBuf			= pFrame;
					uiTmpBufSize	= uiFrameSize;
				}
				else
				{
					m_pSource->getDSI( NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, &pEnhancedDSI, &uiEnhancedDSISize );
					pTmpBuf			= pEnhancedDSI;
					uiTmpBufSize	= uiEnhancedDSISize;
				}

				m_iHDRType = getHDRMetaData(m_pSource->getVideoObjectType(),	pBaseDSI , uiBaseDSISize
				, pFrame, uiFrameSize, iFrameNALHeaderSize
				,&m_stHdrInfo);


				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
													pBaseDSI,
													uiBaseDSISize,
													pTmpBuf,
													uiTmpBufSize,
													NULL,
													NULL,
													iFrameNALHeaderSize,
													&iWidth,
													&iHeight,
													&iPitch,
													m_pSource->getVideoFrameRate(),													
													NEXCAL_VDEC_MODE_NONE);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Dec init End(%d)", __LINE__, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp  %d] Video Codec init Error(%d)", __LINE__, eRet);
				goto SET_READER_FAIL;
			}

			m_iWidth		= iWidth;
			m_iHeight		= iHeight;
			m_iPitch		= iPitch;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Video Codec init End(w:%d h:%d p:%d)", __LINE__, iWidth, iHeight, iPitch);
			break;

		default:
			goto SET_READER_FAIL;
	};	

	if( m_pCodecWrap->isHardwareCodec() == FALSE )
	{
		nexCAL_VideoDecoderSetProperty( m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE, NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE_THUMBNAIL);
	}

	m_isNextFrameRead			= FALSE;

	videoInfo.muiTrackID		= pClip->getClipID();
	videoInfo.mClipType			= pClip->getClipType();

	videoInfo.mStartTime		= pClip->getStartTime();
	videoInfo.mEndTime			= pClip->getEndTime();

	videoInfo.mWidth			= iWidth;
	videoInfo.mHeight			= iHeight;
	videoInfo.mPitch			= iPitch;
	videoInfo.mColorFormat		= m_pCodecWrap->getFormat();
	videoInfo.mIsPreview		= TRUE;

	videoInfo.mRotateState		= pClip->getRotateState();

	videoInfo.mBrightness		= pClip->getBrightness();
	videoInfo.mContrast			= pClip->getContrast();
	videoInfo.mSaturation		= pClip->getSaturation();
    videoInfo.mHue              = pClip->getHue();
	videoInfo.mTintcolor		= pClip->getTintcolor();
	videoInfo.mLUT				= pClip->getLUT();
	videoInfo.mVignette 		= pClip->getVignette();
	videoInfo.mVideoRenderMode = m_iVideoRenderMode;//pClip->getVideoRenderMode(); //yoon
	videoInfo.mHDRType =  m_iHDRType; //yoon
	memcpy(&videoInfo.mHdrInfo, &m_stHdrInfo , sizeof( NEXCODECUTIL_SEI_HDR_INFO )); //yoon

	pRec =  pClip->getStartPosition();
	if( pRec != NULL )
	{
		videoInfo.mStart.left		= pRec->getLeft();
		videoInfo.mStart.top		= pRec->getTop();
		videoInfo.mStart.right		= pRec->getRight();
		videoInfo.mStart.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pRec =  pClip->getEndPosition();
	if( pRec != NULL )
	{
		videoInfo.mEnd.left			= pRec->getLeft();
		videoInfo.mEnd.top			= pRec->getTop();
		videoInfo.mEnd.right		= pRec->getRight();
		videoInfo.mEnd.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}	
	
	pMatrix = pClip->getStartMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mStartMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}

	pMatrix = pClip->getEndMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mEndMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}		
	
 	if( setClipRenderInfo(&videoInfo) == FALSE )
	{
		goto SET_READER_FAIL;
	}

	for (int i = 0; i < VIDEO_FRAME_MAX_COUNT; i++)
	{
		m_FrameInfo[i].setCodecWrap(m_pCodecWrap);
	}

	m_FrameTimeChecker.clear();
	
	{
		CNexDrawInfoVec* pDrawInfos = pClip->getDrawInfos();
		setDrawInfos(pDrawInfos);
		SAFE_RELEASE(pDrawInfos);
	}

	return TRUE;

SET_READER_FAIL:
	if( m_pCodecWrap != NULL )
	{
		m_pCodecWrap->deinitDecoder();
		SAFE_RELEASE(m_pCodecWrap);
	}

	SAFE_RELEASE(m_pSource);
	return FALSE;
}

CNexSource* CVideoTrackInfo::getSourceHandle()
{
	return m_pSource;
}

NXBOOL CVideoTrackInfo::waitFinishTrackFrame(int iTime)
{
	CFrameInfo* pFrame = getFrameOutBuffer();
	while( iTime > 0 && pFrame )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) Draw wait for remain frame while previous set time", __LINE__, m_uiTrackID);
		pFrame = getFrameOutBuffer();
		iTime -= 30;
		nexSAL_TaskSleep(30);
	}

	if( pFrame == NULL )
		return TRUE;
	return FALSE;
}

NXBOOL CVideoTrackInfo::setPreviewTime(CClipItem* pClip, int iIDRFrame, unsigned int uiTime)
{
	NXBOOL bIDRFlag		= FALSE;
	NXBOOL isIDRFrame	= FALSE;
	NXBOOL isSeeking	= FALSE;

	CCalcTime calcTime;

	if( pClip == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime not this time(Start:%d End:%d Cur(%d)", __LINE__, m_uiStartTime, m_uiEndTime, uiTime);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime TrackID(%d) Time(%d) IDR(%d)", __LINE__, m_uiTrackID, uiTime, iIDRFrame);

	unsigned int uiClipBaseTime = pClip->getStartTime();
	unsigned int uiClipTrimTime = pClip->getStartTrimTime();
	int iClipSpeedFactor = pClip->getSpeedCtlFactor();
	
	unsigned int uiReaderTime		= 0;
	unsigned int uiSeekResultTime	= 0;
	NXBOOL bVideoEnd = FALSE;
	CClipVideoRenderInfo* pRenderInfo = NULL, *pCurRenderInfo = NULL, *pPrevRenderInfo = NULL;
	unsigned int uiUpdatedTime = 0;
	NXBOOL				bSupportFrameTimeChecker = 0;
	int    iRetryCount = 0;
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		bSupportFrameTimeChecker = pEditor->m_bSupportFrameTimeChecker;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VTrack.cpp %d] bSupportFrameTimeChecker (%d)", __LINE__, bSupportFrameTimeChecker);
		SAFE_RELEASE(pEditor);
	}

        unsigned int                        uiFlowLevel = 1;

#if defined( _ANDROID) && !defined(FOR_PROJECT_LGE)
        if(CNexVideoEditor::m_bNexEditorSDK && isQualcommChip())
        {
            int count = 0;
            CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
            if( pProjectMng )
            {
                CClipList* pList = pProjectMng ->getClipList();
                if( pList )
                {
                    count = pList->getVideoClipCount();
                    SAFE_RELEASE(pList);
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d]  getVideoClipCount(%d)",  __LINE__, count);
                }
                SAFE_RELEASE(pProjectMng);
            }

            if(count == 1 && m_iWidth*m_iHeight <= 1920*1088)
                uiFlowLevel = 4;
        }
#endif

	//RYU : Mantis 9404 : wait until rendertask render all frames before seek to new position and codec reset.
	if( waitFinishTrackFrame(1000) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] TrackID(%d) setPreviewTime is Fail because video frame is remained.", __LINE__, m_uiTrackID);
		return FALSE;
	}
	
	if( iIDRFrame == 2 )
	{
		bIDRFlag = TRUE;
		m_FrameTimeChecker.clear();
	}
	
	int size = 0;
	unsigned int uiVideoEditBoxTime = m_pSource->getVideoEditBox();

	if(bSupportFrameTimeChecker)
		size = m_FrameTimeChecker.size();
	m_pSource->isEndOfChannel(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &bVideoEnd);	
	m_pSource->setTrimTime(pClip->getStartTrimTime(), pClip->getEndTrimTime());
	m_pSource->getVideoFrameDTS(&uiReaderTime, TRUE);
	uiReaderTime = uiReaderTime >= uiVideoEditBoxTime ? uiReaderTime - uiVideoEditBoxTime : 0;
	
	int iCacheStartTime = 0;
	int iCacheEndTime = 0;

	if( size >= 2 )
	{
		iCacheStartTime = m_FrameTimeChecker[0].getPTS()*uiFlowLevel;
		iCacheEndTime = m_FrameTimeChecker[size-1].getPTS()*uiFlowLevel;

		iCacheStartTime = iCacheStartTime >= uiVideoEditBoxTime ? iCacheStartTime - uiVideoEditBoxTime : 0;
		iCacheEndTime = iCacheEndTime >= uiVideoEditBoxTime ? iCacheEndTime - uiVideoEditBoxTime : 0;
        
		if((pCurRenderInfo = pClip->getActiveVideoRenderInfo(iCacheStartTime, TRUE)) == NULL)
			pCurRenderInfo = m_ClipVideoRenderInfoVec[m_ClipVideoRenderInfoVec.size() - 1];
		iCacheStartTime = (int)calcTime.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, iCacheStartTime);
		if((pPrevRenderInfo = pClip->getActiveVideoRenderInfo(iCacheEndTime, TRUE)) == NULL)
			pPrevRenderInfo = m_ClipVideoRenderInfoVec[m_ClipVideoRenderInfoVec.size() - 1];
		iCacheEndTime = (int)calcTime.applySpeed(pPrevRenderInfo->mStartTime, pPrevRenderInfo->mStartTrimTime, pPrevRenderInfo->m_iSpeedCtlFactor, iCacheEndTime);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Dec Cache(time:%d start:%d end:%d spc:%d, %d)", 
			__LINE__, pClip->getClipID(), uiTime, iCacheStartTime, iCacheEndTime, pCurRenderInfo->m_iSpeedCtlFactor, pPrevRenderInfo->m_iSpeedCtlFactor);
		
		iCacheStartTime = iCacheStartTime < 0 ? 0 : iCacheStartTime;
		iCacheEndTime = iCacheEndTime < 0 ? 0 : iCacheEndTime;
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Dec Cache(time:%d start:%d end:%d spc:%d, %d)", 
			__LINE__, pClip->getClipID(), uiTime, iCacheStartTime, iCacheEndTime, pCurRenderInfo->m_iSpeedCtlFactor, pPrevRenderInfo->m_iSpeedCtlFactor);
	}

	pRenderInfo = pClip->getActiveVideoRenderInfo(uiTime, FALSE);
	int iOriTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiTime);
	iOriTime = iOriTime < 0 ? 0 : iOriTime;
	
	if((pRenderInfo = pClip->getActiveVideoRenderInfo(uiReaderTime, TRUE)) == NULL)
		pRenderInfo = pClip->m_ClipVideoRenderInfoVec[0];
	int uiCurReaderTime = (int)calcTime.applySpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiReaderTime);
	
	if( iIDRFrame == 2 )
	{
		m_pSource->seekToVideoWithAbsoluteTime(iOriTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV);
		// m_pSource->seekTo(uiTime, &uiSeekResultTime);
		m_isNextFrameRead = TRUE;
		isIDRFrame = TRUE;
		m_FrameTimeChecker.clear();

		// JIRA 2113
		resetVideoDecoder();

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime SeekTo(%d %d) readerTime(%d) CalcReaderTime(%d)", 
			__LINE__, uiTime, iOriTime, uiReaderTime, uiCurReaderTime);
	}
	else
	{
		if( 	size >= 2 && uiTime > 500 &&  uiTime >= iCacheStartTime && uiTime <= iCacheEndTime ) // for Mantis 9026
		{
			if(bVideoEnd)
			{
				m_isNextFrameRead = TRUE;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Codec has requested frame(Req:%d St:%d ed:%d) bVideoEnd(%d)", 
				__LINE__, uiTime, iCacheStartTime, iCacheEndTime, bVideoEnd);
		}
		else
		{
			if( uiCurReaderTime < 0 || uiTime < uiCurReaderTime )
			{
				m_pSource->seekToVideoWithAbsoluteTime(iOriTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV);
				// m_pSource->seekTo(uiTime, &uiSeekResultTime);
				m_isNextFrameRead = TRUE;
				isIDRFrame = TRUE;
				m_FrameTimeChecker.clear();

				// JIRA 2113
				resetVideoDecoder();

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime SeekTo(%d %d) readerTime(%d) CalcReaderTime(%d)", 
					__LINE__, uiTime, iOriTime, uiReaderTime, uiCurReaderTime);
				/*comments related to iFirstVideoFrame will be removed.
				if( uiTime <= 1 )
					m_iFirstVideoFrameCounter = 0;
				*/
			}
			else if( bVideoEnd == FALSE && (uiCurReaderTime <= uiTime && (uiCurReaderTime + 300 ) > uiTime) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime no seek operation setT(%d) readerTime(%d %d)", __LINE__, uiTime, uiReaderTime, uiCurReaderTime);
			}
			else
			{
				unsigned int uiResultTime = 0;
				int iRet = m_pSource->getFindIFramePos(uiReaderTime, iOriTime, &uiResultTime);
				switch(iRet)
				{
					case 1:
						m_pSource->seekToVideoWithAbsoluteTime(iOriTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV);
						// m_pSource->seekTo(uiResultTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
						m_isNextFrameRead = TRUE;
						isIDRFrame = TRUE;
						m_FrameTimeChecker.clear();
						resetVideoDecoder();
						// nexCAL_VideoDecoderReset(m_hCodecVideo);
						/* comments related to iFirstVideoFrame will be removed.
						if( uiTime <= 1 )
							m_iFirstVideoFrameCounter = 0;
						*/
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime SeekTo(%d %d) readerTime(%d) seekResult(%d)", 
							__LINE__, uiTime, iOriTime, uiReaderTime, uiSeekResultTime);					
						
						break;
					case 2:
						if(bVideoEnd)
						{
							m_pSource->seekToVideoWithAbsoluteTime(iOriTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV);
							m_isNextFrameRead = TRUE;
							isIDRFrame = TRUE;
							m_FrameTimeChecker.clear();
							resetVideoDecoder();
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime SeekTo(%d %d) readerTime(%d) seekResult(%d)",
							__LINE__, uiTime, iOriTime, uiReaderTime, uiSeekResultTime);
						}
						break;
					case 0:
					case 3:
					default:
						m_pSource->seekToVideoWithAbsoluteTime(iOriTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV);
						// m_pSource->seekTo(uiTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
						m_isNextFrameRead = TRUE;
						isIDRFrame = TRUE;
						m_FrameTimeChecker.clear();
						resetVideoDecoder();
						// nexCAL_VideoDecoderReset(m_hCodecVideo);
						/*comments related to iFirstVideoFrame will be removed.
						if( uiTime <= 1 )
							m_iFirstVideoFrameCounter = 0;
						*/
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setPreviewTime SeekTo(%d %d) readerTime(%d) seekResult(%d)", 
							__LINE__, uiTime, iOriTime, uiReaderTime, uiSeekResultTime);					
						break;
				}
			}
		}
	}

	NEXVIDEOEDITOR_ERROR	eRet			= NEXVIDEOEDITOR_ERROR_NONE;
	unsigned int				uiFrameDTS		= 0;
	unsigned int				uiFramePTS		= 0;
	unsigned char*			pFrame			= NULL;
	unsigned int				uiFrameSize		= 0;
	unsigned int				uiDecoderErrRet	= 0;
	unsigned int				uiEnhancement	= NEXCAL_VDEC_FLAG_NONE;

	CFrameInfo*				pFrameInfo		= NULL;
	
	int						iReadFrameRet	= 0;

	int 						iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();
	NXBOOL					bEndOfContent	= FALSE;
	NXBOOL					bCanGetOutputBuffer = FALSE;

	pCurRenderInfo = pClip->m_ClipVideoRenderInfoVec[0];
	pPrevRenderInfo = NULL;

	while( TRUE )
	{
		if( m_isNextFrameRead )
		{
			iReadFrameRet = m_pSource->getVideoFrame();
			if( iReadFrameRet != _SRC_GETFRAME_OK )
			{
				if( iReadFrameRet == _SRC_GETFRAME_END )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] End this contents", __LINE__);
					bEndOfContent = TRUE;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] contents read operation failed(0x%x)", __LINE__, iReadFrameRet);
					return FALSE;
				}
			}
			m_isNextFrameRead = FALSE;
		}

		if( bEndOfContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;

			if( m_pCodecWrap->isHardwareCodec() == FALSE )
			{
				// for mantis 6805
				if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 && bCanGetOutputBuffer )
				{
					unsigned char*	pY = NULL;
					unsigned char*	pU = NULL;
					unsigned char*	pV = NULL;
					
					unsigned int		uiOutPutCTS		= 0;

					while( getFrameOutBuffer() != NULL )
					{
						DequeueFrameOutBuffer();
					}

					pFrameInfo = getFrameInBuffer();
					if( pFrameInfo == NULL )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getFrame Input Buffer return null Preview failed", __LINE__);
						return FALSE;
					}

					if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pFrameInfo->m_pY,NULL, NULL, &uiOutPutCTS) != 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getOutFrame Frame is failed", __LINE__);
						return FALSE;
					}
					pFrameInfo->setVideoFrame(uiOutPutCTS, NULL, NULL, NULL);
					EnqueueFrameInBuffer();
				}
				return TRUE;
			}
		}
		else
		{
			m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );

			m_pSource->getVideoFrameDTS( &uiFrameDTS, TRUE );
			m_pSource->getVideoFramePTS( &uiFramePTS, TRUE );

			pRenderInfo = pClip->getActiveVideoRenderInfo(uiFramePTS, TRUE);
			if(pRenderInfo != NULL && pRenderInfo != pCurRenderInfo)
			{
				pPrevRenderInfo = pCurRenderInfo;
				pCurRenderInfo = pRenderInfo;
				uiUpdatedTime = uiFramePTS;
			}
		}

		if( bEndOfContent == FALSE &&  isIDRFrame )
		{
			if( NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), bIDRFlag) == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Find IDR read next frame CTS(%d %d)", __LINE__, uiFrameDTS, uiFramePTS);
				m_isNextFrameRead = TRUE;
				continue;
			}
			isIDRFrame = FALSE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] IDR Frame found CTS(%d %d) flag(%d)", __LINE__, uiFrameDTS, uiFramePTS, bIDRFlag);
		}

		if( bEndOfContent == FALSE && m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264 )
		{
			NXBOOL bBFrame = 0;
			bBFrame = !NexCodecUtil_AVC_IsAVCReferenceFrame(pFrame, uiFrameSize, m_pSource->getFrameFormat(), iFrameNALHeaderSize);

			if(bBFrame)
			{
				m_isNextFrameRead = TRUE;
				continue;
			}
		}

		unsigned int uiTick = nexSAL_GetTickCount();
		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
									pFrame,
									uiFrameSize,
									NULL,
									uiFrameDTS/uiFlowLevel,
									uiFramePTS/uiFlowLevel,
									uiEnhancement, 
									&uiDecoderErrRet );

		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Decode preview Frame Sucessed CTS(%d) Ret(%d) Time(%d)", __LINE__, uiFrameDTS, uiDecoderErrRet, uiTick);
		if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Decode preview Frame Sucessed CTS(%d) Time(%d)", __LINE__, uiFrameDTS, uiTick);
			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				m_isNextFrameRead = TRUE;
				if(bSupportFrameTimeChecker)
					m_FrameTimeChecker.addFrameCTSVec(uiFrameDTS/uiFlowLevel, uiFramePTS/uiFlowLevel);
			}

			if( m_pCodecWrap->isHardwareCodec() == FALSE )
			{
				int iFrameCTS = (int)uiFrameDTS;
				if( iIDRFrame == 0 && (iFrameCTS < 0 || iFrameCTS < uiTime) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Preview Frame skip(CTS:%d Time:%d)", __LINE__, uiFrameDTS, uiTime);
					if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
						bCanGetOutputBuffer = TRUE;
					continue;
				}
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char*	pY = NULL;
				unsigned char*	pU = NULL;
				unsigned char*	pV = NULL;
				
				unsigned int		uiDecOutCTS		= 0;
				int				iDecRealTime	= 0;
					
				unsigned int		uiRet			= 0;
				int				iWaitTime		= 1000;
				NXBOOL                bUsePrevInfo = FALSE;

				bCanGetOutputBuffer = FALSE;

				while( getFrameOutBuffer() != NULL )
				{
					DequeueFrameOutBuffer();
				}

				pFrameInfo = getFrameInBuffer();
				if( pFrameInfo == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getFrame Input Buffer return null Preview failed", __LINE__);
					return FALSE;
				}

				switch(m_pCodecWrap->getFormat())
				{
					case NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY:
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VTrack.cpp %d] ID(%d) Non Memory  nexCAL_VideoDecoderGetOutput failed(%p, %d)",__LINE__, m_uiTrackID, pY, uiDecOutCTS);
							continue;
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Non Memory getOutputFrame Time(%d) mem(%p)", __LINE__, m_uiTrackID, uiDecOutCTS, pY);

						//comments related to iFirstVideoFrame will be removed.
						if( bSupportFrameTimeChecker && m_FrameTimeChecker.isValidFrameCTS(uiDecOutCTS, TRUE) == FALSE /*|| m_iFirstVideoFrameCounter < 2 */)
						{
							// TRUE --> FALSE
#if defined(_ANDROID)
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
						        m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
							//m_iFirstVideoFrameCounter++;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Preview Frame skip(CTS:%d Time:%d)", __LINE__, m_uiTrackID, uiDecOutCTS, uiTime);
							continue;
						}

						if(bSupportFrameTimeChecker)
							m_FrameTimeChecker.removeSmallerFrameTime(uiDecOutCTS);

						uiDecOutCTS*=uiFlowLevel;

						if(uiDecOutCTS < uiVideoEditBoxTime)
						{
#if defined(_ANDROID)
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
						        m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
							//m_iFirstVideoFrameCounter++;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Preview Frame skip for editbox(CTS:%d Time:%d)", __LINE__, m_uiTrackID, uiDecOutCTS, uiTime);
							continue;						
						}
                        
						if(uiUpdatedTime > uiDecOutCTS)
							bUsePrevInfo = TRUE;

						if( bEndOfContent && (bSupportFrameTimeChecker && m_FrameTimeChecker.empty()) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Last Video Frame force display",__LINE__, m_uiTrackID);
							if(bUsePrevInfo)
								iDecRealTime = (int)calcTime.applySpeed(pPrevRenderInfo->mStartTime, pPrevRenderInfo->mStartTrimTime, pPrevRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);
							else
								iDecRealTime = (int)calcTime.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);                            
						}
						else
						{
							if(bUsePrevInfo)
								iDecRealTime = (int)calcTime.applySpeed(pPrevRenderInfo->mStartTime, pPrevRenderInfo->mStartTrimTime, pPrevRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);
							else
								iDecRealTime = (int)calcTime.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);                            

							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) GetOutput Time (outTime:%d realTime:%d)", 
								__LINE__, pClip->getClipID(), uiDecOutCTS, iDecRealTime);

							if( iDecRealTime < 0 )
							{
								//TRUE -> FALSE
#if defined(_ANDROID)
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
								m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
								continue;
							}

							if( iIDRFrame == 2 )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Preview IDR Frame (CTS:%d Time:%d)", __LINE__, m_uiTrackID, iDecRealTime, uiTime);
							}
							else if(bSupportFrameTimeChecker == FALSE)
							{
								if(!isEOSFlag(uiEnhancement) && iDecRealTime < uiTime)
								{
#if defined(_ANDROID)
									m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
									m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
									continue;
								}
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Preview Frame Disply Not Support Time & EOS (CTS:%d Time:%d), %d, %d", __LINE__, m_uiTrackID, iDecRealTime, uiTime, bSupportFrameTimeChecker, uiDecoderErrRet);                                
							}
							else if( iIDRFrame == 0 && iDecRealTime < uiTime ) 
							{
								//TRUE -> FALSE
#if defined(_ANDROID)
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
								m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] ID(%d) Preview Frame skip(CTS:%d Time:%d)", __LINE__, m_uiTrackID, iDecRealTime, uiTime);
								continue;
							}
						}

						pFrameInfo->setVideoFrame(iDecRealTime, pY, NULL, NULL);
						break;
					case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, &pU, &pV, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getOutFrame Frame is failed", __LINE__);
							return FALSE;
						}
						pFrameInfo->setVideoFrame(uiDecOutCTS, pY, pU, pV);
						break;
					case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] NV12Mode getOutputFrame(%p)", __LINE__, pFrameInfo->m_pY);
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pFrameInfo->m_pY,NULL, NULL, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getOutFrame Frame is failed", __LINE__);
							return FALSE;
						}
						pFrameInfo->setVideoFrame(uiDecOutCTS, NULL, NULL, NULL);
						break;
					default:
						break;
				};
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] get preview frame end(CTS:%d Time:%d)", __LINE__, uiDecOutCTS, uiTime);
				EnqueueFrameInBuffer();
				return TRUE;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) &&
				m_pCodecWrap->isHardwareCodec() &&
				(!bSupportFrameTimeChecker || m_FrameTimeChecker.size() <= 0) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] get preview frame No more frame", __LINE__);
				return FALSE;
			}
		}
		else
		{
			isIDRFrame = TRUE;
			m_isNextFrameRead = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] Video Dec failed(0x%x)", __LINE__, uiDecoderErrRet);

			iRetryCount++;
			if(iRetryCount > 3)
				return FALSE;

			if( 	bEndOfContent && m_pCodecWrap->isHardwareCodec() )
			{
				m_FrameTimeChecker.clear();
				nexCAL_VideoDecoderReset(m_pCodecWrap->getCodecHandle());
				
				while( getFrameOutBuffer() != NULL )
				{
					DequeueFrameOutBuffer();
				}

				pFrameInfo = getFrameInBuffer();
				if( pFrameInfo == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getFrame Input Buffer return null Preview failed", __LINE__);
					return FALSE;
				}
				EnqueueFrameInBuffer();
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] get preview frame No more frame", __LINE__);
				return TRUE;
			}
		}
	}	
	return TRUE;
}

NXBOOL CVideoTrackInfo::setPreviewThumb(unsigned char* pY, unsigned int uiTime)
{
	CFrameInfo*				pFrameInfo		= NULL;

	while( getFrameOutBuffer() != NULL )
	{
		DequeueFrameOutBuffer();
	}

	pFrameInfo = getFrameInBuffer();
	if( pFrameInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] getFrame Input Buffer return null Preview failed", __LINE__);
		return FALSE;
	}
	pFrameInfo->mColorFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
	pFrameInfo->m_pY = pY;
	pFrameInfo->m_bPreviewThumb = 1;
	pFrameInfo->setVideoFrame(uiTime, NULL, NULL, NULL);
	EnqueueFrameInBuffer();

    return TRUE;
}

void CVideoTrackInfo::resetVideoDecoder()
{
	if( m_pCodecWrap == NULL ) return;
	
#ifdef _ANDROID
	if( isNexus10Device() == FALSE )
#endif
	{
		nexCAL_VideoDecoderReset(m_pCodecWrap->getCodecHandle());
	}	
}

unsigned int CVideoTrackInfo::getDropVideoCount()
{
	return m_iDropLastCount;
}

NXBOOL CVideoTrackInfo::setPause()
{
	m_bTrackPaused = TRUE;
	return TRUE;
}

NXBOOL CVideoTrackInfo::setResume()
{
	m_bTrackPaused = FALSE;
	m_bTrackStart = FALSE;
	return TRUE;
}
CClipVideoRenderInfo* CVideoTrackInfo::getActiveRenderInfo(unsigned int uiTime)
{
	for(int i = 0; i < m_ClipVideoRenderInfoVec.size(); i++)
	{
		if( m_ClipVideoRenderInfoVec[i]->mStartTime <= uiTime && m_ClipVideoRenderInfoVec[i]->mEndTime >= uiTime )
		{
			return m_ClipVideoRenderInfoVec[i];
		}
	}
	
	return NULL;
}

void CVideoTrackInfo::setDrawInfos(CNexDrawInfoVec* pInfos)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VTrack.cpp %d] setDrawInfos(%p %p)", __LINE__, m_pDrawInfos, pInfos);
	SAFE_RELEASE(m_pDrawInfos);
	m_pDrawInfos = pInfos;
	SAFE_ADDREF(m_pDrawInfos)
}

void CVideoTrackInfo::applyDrawInfos(void* pRender, unsigned int uiTime)
{
	if( m_pDrawInfos != NULL )
	{
		m_pDrawInfos->applyDrawInfo(pRender, uiTime, m_uiTrackID);
	}
}

void CVideoTrackInfo::resetDrawInfos(void* pRender)
{
	if( m_pDrawInfos != NULL )
	{
		m_pDrawInfos->resetDrawInfos(pRender);
	}
}


void CVideoTrackInfo::applyPosition(void* pSurface, unsigned int uiTime, int iIndex)
{
	if( !(m_iFeatureVersion >= 3) || iIndex == -1)
		return;     

	unsigned int uiDur;
	float fPer, fleft, fright, ftop, fbottom;

	CClipVideoRenderInfo* pRenderInfo = getActiveRenderInfo(uiTime);

	if(pRenderInfo)
	{
		uiDur = pRenderInfo->mEndTime - pRenderInfo->mStartTime;
		fPer = (float)(uiTime - pRenderInfo->mStartTime) / uiDur;

		fleft 			= calcPos((float)pRenderInfo->mStart.left, (float)pRenderInfo->mEnd.left, fPer);
		fright 			= calcPos((float)pRenderInfo->mStart.right, (float)pRenderInfo->mEnd.right, fPer);
		
		ftop 			= calcPos((float)pRenderInfo->mStart.top, (float)pRenderInfo->mEnd.top, fPer);
		fbottom			= calcPos((float)pRenderInfo->mStart.bottom, (float)pRenderInfo->mEnd.bottom, fPer);
	
		if( m_ClipType == CLIPTYPE_VIDEO_LAYER )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] ID(%d) SetTextureInput Rect(%.2f %.2f %.2f %.2f) for video layer fPer(%f)", __LINE__, m_uiTrackID, fleft, ftop, fright, fbottom, fPer);
			NXT_ThemeRenderer_SetTextureInputRect((NXT_HThemeRenderer )pSurface, (NXT_TextureID)m_iVideoLayerTextureID, fleft, ftop, fright,  fbottom);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VFrame.cpp %d] ID(%d) TextureID(%d) SetTextureInput Rect(%.2f %.2f %.2f %.2f) for video frame fPer(%f)", __LINE__, m_uiTrackID, iIndex, fleft, ftop, fright, fbottom, fPer);
			NXT_ThemeRenderer_SetTextureInputRect((NXT_HThemeRenderer )pSurface, (NXT_TextureID)iIndex, fleft, ftop, fright,  fbottom);
		}
	}
}

float CVideoTrackInfo::calcPos(float fPos1, float fPos2, float fPer)
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

NXBOOL CVideoTrackInfo::isEOSFlag(unsigned int uiFlag)
{
	if( (uiFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
		return TRUE;
	return FALSE;
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/
