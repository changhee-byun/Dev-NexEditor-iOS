/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Clip.cpp
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

#include "NEXVIDEOEDITOR_Interface.h"
#include "NEXVIDEOEDITOR_Error.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NexSAL_Internal.h"
#include "NEXVIDEOEDITOR_Def.h"

#include "NEXVIDEOEDITOR_AudioTask.h"
#include "NEXVIDEOEDITOR_VideoTask.h"
#include "NEXVIDEOEDITOR_ImageTask.h"
#include "NEXVIDEOEDITOR_PFrameProcessTask.h"
#include "NEXVIDEOEDITOR_DirectExportTask.h"
#include "NEXVIDEOEDITOR_VideoThumbTask.h"
#include "NEXVIDEOEDITOR_VideoPreviewTask.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_Thumbnail.h"
#include "NEXVIDEOEDITOR_FileMissing.h"
#include "NEXVIDEOEDITOR_Util.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"

#include "stdio.h"
#include "string.h"

#ifdef __APPLE__
#include "NexThemeRenderer_Platform_iOS-CAPI.h"
#endif

//---------------------------------------------------------------------------
unsigned int CClipItem::m_uiUniqueIDCounter = 1;
unsigned int CClipItem::getUniqueClipID()
{
	return m_uiUniqueIDCounter++;
}

unsigned int CClipItem::getUniqueClipID(unsigned int uiClipID)
{
	m_uiUniqueIDCounter = m_uiUniqueIDCounter > uiClipID ? m_uiUniqueIDCounter : uiClipID;
	m_uiUniqueIDCounter++;
	return m_uiUniqueIDCounter;
}

CClipItem::CClipItem()
{
	m_uiClipID		= getUniqueClipID();
	m_pSource		= NULL;

	m_pAudioTask		= NULL;
	m_pVideoTask		= NULL;
	m_pImageTask 		= NULL;
#ifdef _ANDROID
	m_pPFrameTask		= NULL;
	m_pDirectExportTask	= NULL;
#endif
	m_pVideoThumbTask	= NULL;
	m_pVideoPreviewTask	= NULL;

	m_iFaceDetectProcessed = 1;

	m_pStartRect		= new CRectangle;
	m_pEndRect		= new CRectangle;
	m_pDstRect		= new CRectangle;
	m_pFaceRect		= new CRectangle;

	m_pImageTrackInfo		= NULL;
	m_pTitle					= NULL;

	m_pEnhancedAudioFilter		= NULL;
	m_pEqualizer				= NULL;

	m_strFilePath				= NULL;
	m_strThumbnailPath		= NULL;

	m_iThumb				= 0;

	m_puiEnvelopAudioVolume	= NULL;
	m_puiEnvelopAudioTime		= NULL;
	m_iEnvelopAudioSize		= 0;
	m_bEnableEnvelopAudio		= TRUE;
	
	m_iVideoCodecType		= 0;
	m_pVideoDSI				= NULL;
	m_iVideoDSISize			= 0;

	m_iAudioCodecType		= 0;
	m_pAudioDSI				= NULL;
	m_iAudioDSISize			= 0;	
	m_isStartIDRFrame		= FALSE;
	m_iContentOrientation		= 0;
	m_iAudioTrackCount		= 0;
	m_iVideoTrackCount		= 0;
	m_isParseClip				= FALSE;
	m_isParseClipStop			= FALSE;
	m_iVideoDecodeEnd		= FALSE;

	//yoon
	m_pVideoTrackUUID		= NULL;
	m_iVideoTrackUUIDSize	= -1;

	m_iHDRType = 0; //yoon
	memset(&m_stHDRInfo,0,sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon

	m_uiAudioEditBoxTime	= 0;
	m_uiVideoEditBoxTime 	= 0;

	m_iRenderItemCount 		= 0; //yoon
	m_isMotionTracked		= FALSE;

	m_pDrawInfos = NULL;
	m_bReverse = 0;    
	clearClipInfo();

	m_iFreezeDuration		= 0;
}

CClipItem::CClipItem(unsigned int uiClipID)
{
	m_uiClipID = uiClipID;
	if( uiClipID != INVALID_CLIP_ID )
	{
		getUniqueClipID(uiClipID);
	}
		
	m_pSource			= NULL;

	m_pAudioTask		= NULL;
	m_pAudioTask2		= NULL;
	m_pAudioTask3		= NULL;
    	m_pAudioTask4		= NULL;
    
	m_pVideoTask		= NULL;
	m_pImageTask 		= NULL;
#ifdef _ANDROID
	m_pPFrameTask		= NULL;
	m_pDirectExportTask	= NULL;
#endif
	m_pVideoThumbTask	= NULL;
	m_pVideoPreviewTask	= NULL;

	m_iFaceDetectProcessed = 1;

	m_pStartRect		= new CRectangle;
	m_pEndRect		= new CRectangle;
	m_pDstRect		= new CRectangle;
	m_pFaceRect		= new CRectangle;

	m_pImageTrackInfo		= NULL;
	m_pTitle					= NULL;

	m_pEnhancedAudioFilter		= NULL;
	m_pEqualizer				= NULL;

	m_strFilePath				= NULL;
	m_strThumbnailPath		= NULL;
	m_iThumb				= 0;

	m_puiEnvelopAudioVolume	= NULL;
	m_puiEnvelopAudioTime		= NULL;
	m_iEnvelopAudioSize		= 0;
	m_bEnableEnvelopAudio		= TRUE;
	
	m_iVideoCodecType		= 0;
	m_pVideoDSI				= NULL;
	m_iVideoDSISize			= 0;

	m_iAudioCodecType		= 0;
	m_pAudioDSI				= NULL;
	m_iAudioDSISize			= 0;
	m_isStartIDRFrame		= FALSE;
	m_iContentOrientation		= 0;
	m_iAudioTrackCount		= 0;
	m_iVideoTrackCount		= 0;
	m_isParseClip				= FALSE;
	m_isParseClipStop			= FALSE;

	//yoon
	m_pVideoTrackUUID = NULL;
	m_iVideoTrackUUIDSize = -1;

	m_iHDRType = 0; //yoon
	memset(&m_stHDRInfo,0,sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon

	m_iVideoDecodeEnd		= FALSE;
	
	m_uiAudioEditBoxTime = 0;
	m_uiVideoEditBoxTime = 0;

	m_iRenderItemCount = 0; //yoon

	m_isMotionTracked		= FALSE;

	m_pDrawInfos = NULL;
	m_bReverse = 0;    
	clearClipInfo();

	m_iFreezeDuration		= 0;
}

CClipItem::~CClipItem()
{
	stopPlay();

	if(m_pSource)
	{
		m_pSource->deleteSource();
		m_pSource->Release();
		m_pSource = NULL;
	}

	if( m_AudioClipVec.size() > 0 )
	{
		for(int i = 0; i < (int)m_AudioClipVec.size(); i++)
		{
			m_AudioClipVec[i]->Release();
		}
		m_AudioClipVec.clear();
	}

	if( m_SubVideoClipVec.size() > 0 )
	{
		for( int i = 0; i < (int)m_SubVideoClipVec.size(); i++)
		{
			m_SubVideoClipVec[i]->Release();
		}
		m_SubVideoClipVec.clear();
	}

	SAFE_RELEASE(m_pStartRect);
	SAFE_RELEASE(m_pEndRect);
	SAFE_RELEASE(m_pDstRect);
	SAFE_RELEASE(m_pFaceRect);
	SAFE_RELEASE(m_pImageTrackInfo);

	if( m_pEnhancedAudioFilter )
	{
		nexSAL_MemFree(m_pEnhancedAudioFilter);
		m_pEnhancedAudioFilter = NULL;
	}

	if ( m_pEqualizer )
	{
		nexSAL_MemFree(m_pEqualizer);
		m_pEqualizer = NULL;
	}

	if( m_pTitle )
	{
		nexSAL_MemFree(m_pTitle);
		m_pTitle = NULL;
	}
	
	if( m_pImageBuffer )
	{
		nexSAL_MemFree(m_pImageBuffer);
		m_pImageBuffer = NULL;
	}

	if( m_strFilePath )
	{
		nexSAL_MemFree(m_strFilePath);
		m_strFilePath = NULL;
	}

	if( m_strThumbnailPath )
	{
		nexSAL_MemFree(m_strThumbnailPath);
		m_strThumbnailPath = NULL;
	}
	
	if( m_pVideoDSI )
	{
		nexSAL_MemFree(m_pVideoDSI);
		m_pVideoDSI = NULL;
	}
	
	if( m_pAudioDSI )
	{
		nexSAL_MemFree(m_pAudioDSI);
		m_pAudioDSI = NULL;
	}

	//yoon
	if( m_pVideoTrackUUID )
	{
		m_iVideoTrackUUIDSize = -1;
		nexSAL_MemFree(m_pVideoTrackUUID);
		m_pVideoTrackUUID = NULL;
	}

	m_iVideoDecodeEnd		= FALSE;
	clearAudioEnvelop();

	if(m_ClipVideoRenderInfoVec.size())
	{
		CClipVideoRenderInfoVectorIterator i;
		for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
		{
			CClipVideoRenderInfo* pRenderInfo = (CClipVideoRenderInfo*)(*i);
			if(pRenderInfo)
			{
				SAFE_RELEASE(pRenderInfo);
			}
		}
	}
	m_ClipVideoRenderInfoVec.clear();

	if(m_ClipAudioRenderInfoVec.size())
	{
		ClipAudioRenderInfoVectorIterator i;
		for(i = m_ClipAudioRenderInfoVec.begin(); i != m_ClipAudioRenderInfoVec.end(); i++)
		{
			CClipAudioRenderInfo* pRenderInfo = (CClipAudioRenderInfo*)(*i);
			if(pRenderInfo)
			{
				if(pRenderInfo->m_pEnhancedAudioFilter)
				{
					nexSAL_MemFree(pRenderInfo->m_pEnhancedAudioFilter);
					pRenderInfo->m_pEnhancedAudioFilter = NULL;
				}
				if(pRenderInfo->m_pEqualizer)
				{
					nexSAL_MemFree(pRenderInfo->m_pEqualizer);
					pRenderInfo->m_pEqualizer = NULL;
				}

				if(pRenderInfo->m_puiEnvelopAudioTime)
					nexSAL_MemFree(pRenderInfo->m_puiEnvelopAudioTime);
				if(pRenderInfo->m_puiEnvelopAudioVolume)
					nexSAL_MemFree(pRenderInfo->m_puiEnvelopAudioVolume);
				SAFE_RELEASE(pRenderInfo);
			}
		}
	}
	m_ClipAudioRenderInfoVec.clear();

	if( m_pDrawInfos != NULL )
	{
		m_pDrawInfos->clearDrawInfo();
		SAFE_RELEASE(m_pDrawInfos);
	}

#ifdef FOR_IMAGE_LOADING_TASK
	if( m_pImageTask != NULL )
	{
		m_pImageTask->End(1000);
		SAFE_RELEASE(m_pImageTask);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay m_pImageTask End ", __LINE__, m_uiClipID);
	}
#endif

	if( m_ClipType == CLIPTYPE_IMAGE || m_pImageTrackInfo )
	{
		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			deregisteImageTrackInfo(pVideoRender);
			SAFE_RELEASE(pVideoRender);
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay delete m_pImageTrackInfo", __LINE__);
		SAFE_RELEASE(m_pImageTrackInfo);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[Clip.cpp %d] ~~~~~~~~~~~CClipItem", __LINE__);
}

void CClipItem::clearClipInfo()
{
	//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] clearClipInfo", __LINE__);
	
	m_uiVisualClipID		= INVALID_CLIP_ID;
	m_ClipType			= CLIPTYPE_NONE;
	m_uiTotalAudioTime	= 0;
	m_uiTotalVideoTime	= 0;
	m_uiTotalTime		= 0;
	m_uiStartTime		= 0;
	m_uiEndTime			= 0;

	m_uiStartTrimTime		= 0;
	m_uiEndTrimTime		= 0;

	m_iWidth			= 0;
	m_iHeight			= 0;
	m_iDisplayWidth		= 0;
	m_iDisplayHeight	= 0;

	m_iVideoFPS			= 0;
	m_fVideoFPS			= 0.0;
	m_iVideoH264Profile	= 0;
	m_iVideoH264Level	= 0;

	m_iVideoBitRate		= 0;
	m_iAudioBitRate		= 0;

	m_iAudioSampleRate	= 0;
	m_iAudioChannels		= 0;

	m_iSeekPointCount	= 0;

	m_isVideoExist		= 0;
	m_isAudioExist		= 0;

	m_eTitleStyle			= TITLE_STYLE_NONE;
	m_uiTitleStartTime		= 0;
	m_uiTitleEndTime		= 0;

	if( m_pEnhancedAudioFilter )
	{
		nexSAL_MemFree(m_pEnhancedAudioFilter);
		m_pEnhancedAudioFilter = NULL;
	}

	if( m_pEqualizer )
	{
		nexSAL_MemFree(m_pEqualizer);
		m_pEqualizer = NULL;
	}

	if( m_pTitle )
	{
		nexSAL_MemFree(m_pTitle);
		m_pTitle  = NULL;
	}

#if 1 // for Test
	m_pTitle = (char*)nexSAL_MemAlloc(100);
	strcpy(m_pTitle, "");
#endif

	if( m_pStartRect ) m_pStartRect->setRect(0, 0, 100000, 100000);
	if( m_pEndRect ) m_pEndRect->setRect(0, 0, 100000, 100000);
	if( m_pDstRect ) m_pDstRect->setRect(0, 0, 0, 0);
	if( m_pFaceRect ) m_pFaceRect->setRect(0, 0, 0, 0);


	memcpy(m_StartMatrix, default_matrix, sizeof(float)*MATRIX_MAX_COUNT);
	memcpy(m_EndMatrix, default_matrix, sizeof(float)*MATRIX_MAX_COUNT);

	if( m_strFilePath )
	{
		nexSAL_MemFree(m_strFilePath);
		m_strFilePath = NULL;
	}

	if( m_strThumbnailPath )
	{
		nexSAL_MemFree(m_strThumbnailPath);
		m_strThumbnailPath = NULL;
	}
	m_iThumb			= 0;
	m_nFadeInDuration	= 0;
	m_nFadeOutDuration	= 0;	
	
	m_bEffectEnable		= TRUE;
 	m_iEffectDuration		= 2000;
	m_iEffectOffset		= 100;
	m_iEffectOverlap		= 100;

	strcpy(m_strEffectClipID, THEME_RENDERER_DEFAULT_EFFECT);
	strcpy(m_strEffectTitleID, THEME_RENDERER_DEFAULT_TITLE_EFFECT);
	strcpy(m_strFilterID, "");

	m_iRotateState		= 0;

	m_iBrightness			= 0;
	m_iContrast				= 0;
	m_iSaturation			= 0;
    m_iHue                  = 0;
	m_iTintcolor			= 0;
	m_iLUT  				= 0;
	m_iCustomLUT_A			= 0;
	m_iCustomLUT_B			= 0;
	m_iCustomLUT_Power		= 0;
	
	m_iSpeedCtlFactor		= 100;
	m_bKeepPitch			= 1;
	m_iPitchIndex			= 0;
	m_iCompressorFactor		= 0;
	m_iMusicEffector		= 0;
	m_iProcessorStrength	= -1;
	m_iBassStrength			= -1;
	m_iSlowMotion			= 0;

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		char* pID = pEditor->getDefaultClipEffect();
		if( pID )
			strcpy(m_strEffectClipID, pID);

		strcpy(m_strEffectTitleID, "");
		SAFE_RELEASE(pEditor);
	}
	
	m_pSource			= NULL;
	m_pAudioTask		= NULL;
	m_pVideoTask		= NULL;
	m_pImageTask		= NULL;

#ifdef _ANDROID
	m_pPFrameTask		= NULL;
	m_pDirectExportTask	= NULL;
#endif
	m_pVideoThumbTask	= NULL;
	m_pVideoPreviewTask	= NULL;

	m_pImageTrackInfo	= NULL;
	m_iPanFactor[0]		= -111;
	m_iPanFactor[1]		= 111;

	m_iAudioOnOff		= 1;
	m_iAutoEnvelop		= 0;
	m_iClipVolume		= 100;

	m_iBackGroundVolume	= 100;
	
	m_iClipPCMLevelVec.clear();
	m_iClipSeekTabelVec.clear();

	m_pImageBuffer		= NULL;
	m_iImageBufferSize	= 0;

	m_uiCurrentClipIndex	= 0;
	m_uiTotalClipCount		= 0;

	m_isPlay				= FALSE;
	m_bRequireTrackUpdate = FALSE;

	if( m_AudioClipVec.size() > 0 )
	{
		for(int i = 0; i < (int)m_AudioClipVec.size(); i++)
		{
			m_AudioClipVec[i]->Release();
		}
		m_AudioClipVec.clear();
	}
	if( m_SubVideoClipVec.size() > 0 )
	{
		for( int i = 0; i < (int)m_SubVideoClipVec.size(); i++)
		{
			m_SubVideoClipVec[i]->Release();
		}
		m_SubVideoClipVec.clear();
	}

	if( m_pVideoDSI )
	{
		nexSAL_MemFree(m_pVideoDSI);
		m_pVideoDSI = NULL;
		m_iVideoDSISize = 0;
	}
	
	if( m_pAudioDSI )
	{
		nexSAL_MemFree(m_pAudioDSI);
		m_pAudioDSI = NULL;
		m_iAudioDSISize = 0;
	}

	m_bH264Interlaced		= FALSE;
	m_iVideoCodecType		= 0;
	m_iAudioCodecType		= 0;
	m_isStartIDRFrame		= FALSE;
	m_iContentOrientation		= 0;
	
	m_iAudioTrackCount		= 0;
	m_iVideoTrackCount		= 0;
	
	//yoon
	if( m_pVideoTrackUUID )
	{
	    nexSAL_MemFree(m_pVideoTrackUUID);
	    m_pVideoTrackUUID = NULL;
	    m_iVideoTrackUUIDSize = -1;
	}
	m_iVideoDecodeEnd = FALSE;
	m_iRenderItemCount = 0; //yoon
	m_iHDRType = 0; //yoon
	memset(&m_stHDRInfo,0,sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon

	clearAudioEnvelop();

	if(m_ClipVideoRenderInfoVec.size())
	{
		CClipVideoRenderInfoVectorIterator i;
		for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
		{
			CClipVideoRenderInfo* pRenderInfo = (CClipVideoRenderInfo*)(*i);
			if(pRenderInfo)
			{
				SAFE_RELEASE(pRenderInfo);
			}
		}
	}
	m_ClipVideoRenderInfoVec.clear();

	if(m_ClipAudioRenderInfoVec.size())
	{
		ClipAudioRenderInfoVectorIterator i;
		for(i = m_ClipAudioRenderInfoVec.begin(); i != m_ClipAudioRenderInfoVec.end(); i++)
		{
			CClipAudioRenderInfo* pRenderInfo = (CClipAudioRenderInfo*)(*i);
			if(pRenderInfo)
			{
				if(pRenderInfo->m_pEnhancedAudioFilter)
				{
					nexSAL_MemFree(pRenderInfo->m_pEnhancedAudioFilter);
					pRenderInfo->m_pEnhancedAudioFilter = NULL;
				}
				if(pRenderInfo->m_pEqualizer)
				{
					nexSAL_MemFree(pRenderInfo->m_pEqualizer);
					pRenderInfo->m_pEqualizer = NULL;
				}
				if(pRenderInfo->m_puiEnvelopAudioTime)
					nexSAL_MemFree(pRenderInfo->m_puiEnvelopAudioTime);
				if(pRenderInfo->m_puiEnvelopAudioVolume)
					nexSAL_MemFree(pRenderInfo->m_puiEnvelopAudioVolume);
				SAFE_RELEASE(pRenderInfo);
			}
		}
	}
	m_ClipAudioRenderInfoVec.clear();
	
	m_uiAudioEditBoxTime = 0;
	m_uiVideoEditBoxTime = 0;

	m_iFreezeDuration	= 0;
}

int CClipItem::getClipID()
{
	return m_uiClipID;
}

CLIP_TYPE CClipItem::getClipType()
{
	return m_ClipType;
}

int CClipItem::setClipType(CLIP_TYPE eType)
{
	m_ClipType = eType;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

const char* CClipItem::getClipPath()
{
	if( m_strFilePath == NULL )
		return "";
	return m_strFilePath;
}

int CClipItem::setClipPath(const char* pClipPath)
{
	if( pClipPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( m_strFilePath )
	{
		if( strcmp(m_strFilePath, pClipPath) != 0 && m_ClipType == CLIPTYPE_IMAGE )
			m_bRequireTrackUpdate = TRUE;
		nexSAL_MemFree(m_strFilePath);
		m_strFilePath = NULL;
	}

	m_strFilePath = (char*)nexSAL_MemAlloc(strlen(pClipPath)+1);
	strcpy(m_strFilePath, pClipPath);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CClipItem::getTotalAudioTime()
{
	return m_uiTotalAudioTime;
}

unsigned int CClipItem::getTotalVideoTime()
{
	return m_uiTotalVideoTime;
}

unsigned int CClipItem::getTotalTime()
{
	return m_uiTotalTime;
}

int CClipItem::setTotalTime(unsigned int uiTotalTime)
{
	m_uiTotalTime = uiTotalTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::isVideoExist()
{
	return m_isVideoExist;
}

int CClipItem::setVideoExist(NXBOOL isExist)
{
	m_isVideoExist = isExist;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::isAudioExist()
{
	return m_isAudioExist;
}

int CClipItem::setAudioExist(NXBOOL isExist)
{
	m_isAudioExist = isExist;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getWidth()
{
	return m_iWidth;
}

int CClipItem::setWidth(int iWidth)
{
	m_iWidth = iWidth;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getHeight()
{
	return m_iHeight;
}

int CClipItem::setHeight(int iHeight)
{
	m_iHeight = iHeight;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getDisplayWidth()
{
	return m_iDisplayWidth;
}

int CClipItem::setDisplayWidth(int iDisplayWidth)
{
	m_iDisplayWidth = iDisplayWidth;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getDisplayHeight()
{
	return m_iDisplayHeight;
}

int CClipItem::setDisplayHeight(int iDisplayHeight)
{
	m_iDisplayHeight = iDisplayHeight;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getVideoFPS()
{
	return m_iVideoFPS;
}

float CClipItem::getVideoFPSFloat()
{
	return m_fVideoFPS;
}

int CClipItem::getVideoH264Profile()
{
	return m_iVideoH264Profile;
}

int CClipItem::getVideoH264Level()
{
	return m_iVideoH264Level;
}

int CClipItem::getVideoOrientation()
{
	return m_iRotateState;
}

int CClipItem::getVideoH264Interlaced()
{
	return m_bH264Interlaced;
}

int CClipItem::setVideoH264Interlaced(NXBOOL isInterlaced)
{
	m_bH264Interlaced = isInterlaced;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getVideoBitRate()
{
	return m_iVideoBitRate;
}

int CClipItem::getAudioBitRate()
{
	return m_iAudioBitRate;
}

int CClipItem::getAudioSampleRate()
{
	return m_iAudioSampleRate;
}

int CClipItem::getAudioChannels()
{
	return m_iAudioChannels;
}

int CClipItem::getSeekPointCount()
{
	return m_iSeekPointCount;
}

int CClipItem::setSeekPointCount(int iSeekPointCount)
{
	m_iSeekPointCount = iSeekPointCount;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CClipItem::getStartTime()
{
	return m_uiStartTime;
}

int CClipItem::setStartTime(unsigned int uiStartTime)
{
	m_uiStartTime = uiStartTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CClipItem::getEndTime()
{
	return m_uiEndTime;
}

int CClipItem::setEndTime(unsigned int uiEndTime)
{
	m_uiEndTime = uiEndTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CClipItem::getStartTrimTime()
{
	return m_uiStartTrimTime;
}

int CClipItem::setStartTrimTime(unsigned int uiStartTime)
{
	m_uiStartTrimTime = uiStartTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CClipItem::getEndTrimTime()
{
	return m_uiEndTrimTime;
}

int CClipItem::setEndTrimTime(unsigned int uiEndTime)
{
	m_uiEndTrimTime = uiEndTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getFaceDetectProcessed(){

	return m_iFaceDetectProcessed;
}
IRectangle* CClipItem::getStartPosition()
{
	SAFE_ADDREF(m_pStartRect);
	return m_pStartRect;
}

IRectangle* CClipItem::getEndPosition()
{
	SAFE_ADDREF(m_pEndRect);
	return m_pEndRect;
}

IRectangle* CClipItem::getDstPosition()
{
	SAFE_ADDREF(m_pDstRect);
	return m_pDstRect;
}

IRectangle* CClipItem::getFacePosition()
{
	SAFE_ADDREF(m_pFaceRect);
	return m_pFaceRect;
}

float* CClipItem::getStartMatrix(int* pCount)
{
	if( pCount == NULL )
	{
		return NULL;
	}
	*pCount = MATRIX_MAX_COUNT;
	
	return m_StartMatrix;
}

int CClipItem::setStartMatrix(float* pMatrix, int iCount)
{
	if( pMatrix == NULL || iCount != MATRIX_MAX_COUNT )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] ClipID(%d) setStartMatrix failed(%p %d)", __LINE__, pMatrix, iCount);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	memcpy(m_StartMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

float* CClipItem::getEndMatrix(int* pCount)
{
	if( pCount == NULL )
	{
		return NULL;
	}
	*pCount = MATRIX_MAX_COUNT;
	
	return m_EndMatrix;
}

int CClipItem::setEndMatrix(float* pMatrix, int iCount)
{
	if( pMatrix == NULL || iCount != MATRIX_MAX_COUNT )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] ClipID(%d) setEndMatrix failed(%p %d)", __LINE__, pMatrix, iCount);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	memcpy(m_EndMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CClipItem::getTitleStartTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] ClipID(%d) getTitleStartTime(%d)", __LINE__, m_uiClipID, m_uiTitleStartTime);
	return m_uiTitleStartTime;
}

int CClipItem::setTitleStartTime(unsigned int uiStartTime)
{
	m_uiTitleStartTime = uiStartTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] ClipID(%d) setTitleStartTime(%d)", __LINE__, m_uiClipID, m_uiTitleStartTime);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CClipItem::getTitleEndTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] ClipID(%d) getTitleEndTime(%d)", __LINE__, m_uiClipID, m_uiTitleEndTime);
	return m_uiTitleEndTime;
}

int CClipItem::setTitleEndTime(unsigned int uiEndTime)
{
	m_uiTitleEndTime = uiEndTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] ClipID(%d) setTitleEndTime(%d)", __LINE__, m_uiClipID, m_uiTitleEndTime);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getTitleStyle()
{
	return (int)m_eTitleStyle;
}

int CClipItem::setTitleStyle(int iStyle)
{
	m_eTitleStyle = (TITLE_STYLE)iStyle;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

char* CClipItem::getTitle()
{
	if( m_pTitle && strlen(m_pTitle) > 0 )
		return m_pTitle;
	return NULL;
}

int CClipItem::setTitle(char* pTitle)
{
	if( pTitle == NULL ) return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	if( m_pTitle )
	{
		nexSAL_MemFree(m_pTitle);
		m_pTitle = NULL;
	}

	m_pTitle = (char*)nexSAL_MemAlloc(strlen(pTitle)+1);
	if( m_pTitle == NULL ) return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;

	strcpy(m_pTitle, pTitle);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

char* CClipItem::getEqualizer()
{
	if( m_pEqualizer && strlen(m_pEqualizer) > 0 )
		return m_pEqualizer;
	return NULL;
}

int CClipItem::setEqualizer(char* pEqualizer)
{
	if( pEqualizer == NULL ) return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	if( m_pEqualizer )
	{
		nexSAL_MemFree(m_pEqualizer);
		m_pEqualizer = NULL;
	}

	m_pEqualizer = (char*)nexSAL_MemAlloc(strlen(pEqualizer)+1);
	if( m_pEqualizer == NULL ) return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;

	strcpy(m_pEqualizer, pEqualizer);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

char* CClipItem::getEnhancedAudioFilter()
{
	if( m_pEnhancedAudioFilter && strlen(m_pEnhancedAudioFilter) > 0 )
		return m_pEnhancedAudioFilter;
	return NULL;
}

int CClipItem::setEnhancedAudioFilter(char* pEnhancedAudioFilter)
{
	if( pEnhancedAudioFilter == NULL ) return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	if( m_pEnhancedAudioFilter )
	{
		nexSAL_MemFree(m_pEnhancedAudioFilter);
		m_pEnhancedAudioFilter = NULL;
	}

	m_pEnhancedAudioFilter = (char*)nexSAL_MemAlloc(strlen(pEnhancedAudioFilter)+1);
	if( m_pEnhancedAudioFilter == NULL ) return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;

	strcpy(m_pEnhancedAudioFilter, pEnhancedAudioFilter);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

const char* CClipItem::getThumbnailPath()
{
	if( m_strThumbnailPath == NULL )
		return "";
		return m_strThumbnailPath;
}

int CClipItem::setThumbnailPath(const char* pThumbnailPath)
{
	if( pThumbnailPath == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( m_strThumbnailPath )
	{
		nexSAL_MemFree(m_strThumbnailPath);
		m_strThumbnailPath = NULL;
	}

	m_strThumbnailPath = (char*) nexSAL_MemAlloc(strlen(pThumbnailPath) + 1);
	strcpy(m_strThumbnailPath, pThumbnailPath);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getClipEffectEnable()
{
	return m_bEffectEnable;
}

int CClipItem::setClipEffectEnable(NXBOOL bEnable)
{
	m_bEffectEnable = bEnable;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getClipEffectDuration()
{
	return m_iEffectDuration;
}

int CClipItem::setClipEffectDuration(int iTime)
{
	m_iEffectDuration = iTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getClipEffectOffset()
{
	return m_iEffectOffset;
}

int CClipItem::setClipEffectOffset(int iPercent)
{
	m_iEffectOffset = iPercent;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getClipEffectOverlap()
{
	return m_iEffectOverlap;
}

int CClipItem::setClipEffectOverlap(int iPercent)
{
	m_iEffectOverlap = iPercent;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

const char* CClipItem::getClipEffectID()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) getClipEffectID(%s)", __LINE__, m_uiClipID, m_strEffectTitleID);
	return m_strEffectClipID;
}

int CClipItem::setClipEffectID(const char* pStrID)
{
	if( pStrID == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	strcpy(m_strEffectClipID, pStrID);

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setClipEffectID(%s)", __LINE__, m_uiClipID, m_strEffectClipID);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

const char* CClipItem::getTitleEffectID()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) getTitleEffectID(%s)", __LINE__, m_uiClipID, m_strEffectTitleID);
	return m_strEffectTitleID;
}

int CClipItem::setTitleEffectID(const char* pStrID)
{
	if( pStrID == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	strcpy(m_strEffectTitleID, pStrID);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

const char* CClipItem::getFilterID()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) getFilterID(%s)", __LINE__, m_uiClipID, m_strFilterID);
	return m_strFilterID;
	
}

int CClipItem::setFilterID(const char* pStrID)
{
	if( pStrID == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	strcpy(m_strFilterID, pStrID);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

//yoon
int CClipItem::setRenderItemIDs(const char* pStrIDs)
{
    m_iRenderItemCount = 0; //yoon
    
	if( pStrIDs == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	
    char *ptr = (char*)pStrIDs;
    char temp[80];
    int end = 0;
    int index = 0;
    while( 1 )
    {
        char *last;
        RENDER_ITEM *pItem = &m_RenerItems[index];
        
        last = strchr(ptr,',');
        strncpy(temp,ptr,last-ptr);
        temp[last-ptr] = 0x00;
        pItem->m_iStartTime = atoi(temp);
        ptr = last+1;
        
        last = strchr(ptr,'@');
        strncpy(temp,ptr,last-ptr);
        temp[last-ptr] = 0x00;
        pItem->m_iEndTime = atoi(temp);
        ptr = last+1;

        last = strchr(ptr,'|');
        if(last==NULL)
        {
            strcpy(pItem->m_strID,ptr);
            end = 1;
        }
        else
        {
            strncpy(pItem->m_strID,ptr,last-ptr);
            pItem->m_strID[last-ptr] = 0x00;
            ptr = last+1;
        }
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setRenderItemIDs (ST=%d, ET=%d, ID=%s) ", __LINE__, m_uiClipID, pItem->m_iStartTime,pItem->m_iEndTime,pItem->m_strID);
        index++;
        if(index == 4 || end || *ptr==0x00)
            break;
    }
	m_iRenderItemCount = index;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getRotateState()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) getRotateState(%d)", __LINE__, m_uiClipID, m_iRotateState);
	return m_iRotateState;
}

int CClipItem::setRotateState(int iRotate)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setRotateState(pre:%d cur:%d)", __LINE__, m_uiClipID, m_iRotateState, iRotate);
	m_iRotateState = iRotate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getBrightness()
{
	return m_iBrightness;
}

int CClipItem::setBrightness(int iBrightness)
{
	m_iBrightness = iBrightness;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getContrast()
{
	return m_iContrast;
}

int CClipItem::setContrast(int iContrast)
{
	m_iContrast = iContrast;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getSaturation()
{
	return m_iSaturation;
}

int CClipItem::setSaturation(int iSaturation)
{
	m_iSaturation = iSaturation;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getHue()
{
	return m_iHue;
}

int CClipItem::setHue(int iHue)
{
	m_iHue = iHue;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getTintcolor()
{
	return m_iTintcolor;
}

int CClipItem::setTintcolor(int iTintcolor)
{
	m_iTintcolor = iTintcolor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getLUT()
{
	return m_iLUT;
}

int CClipItem::setLUT(int LUT)
{
	m_iLUT = LUT;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getCustomLUTA(){

	return m_iCustomLUT_A;
}

int CClipItem::getCustomLUTB(){

	return m_iCustomLUT_B;
}

int CClipItem::getCustomLUTPower(){

	return m_iCustomLUT_Power;
}

int CClipItem::setCustomLUTA(int LUT)
{
	m_iCustomLUT_A = LUT;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::setCustomLUTB(int LUT)
{
	m_iCustomLUT_B = LUT;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::setCustomLUTPower(int LUT)
{
	m_iCustomLUT_Power = LUT;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getVignette()
{
	return m_iVignette;
}

int CClipItem::setVignette(int vignette)
{
	m_iVignette = vignette;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

//yoon
int CClipItem::getHdrMetaData(NEXCODECUTIL_SEI_HDR_INFO *out_hdr_meta){
	parseDSIofClip();
	if( out_hdr_meta != NULL ){
		memcpy(out_hdr_meta, &m_stHDRInfo , sizeof(NEXCODECUTIL_SEI_HDR_INFO));
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] getHdrMetaData ID(%d) Hdr type=%d\n", __LINE__, getClipID(), m_iHDRType);
	return m_iHDRType;
}

//yoon
int CClipItem::getVideoRenderMode()
{
    if( m_ClipType != CLIPTYPE_VIDEO )
    {
        return 0;
    }
    int size = 0;
    unsigned char *pszUUID = getVideoTrackUUID(&size);
  
  int flags = 0;
    
	if( size >0 && pszUUID != NULL )
	{
	    int rval = findString(pszUUID,size,(unsigned char *)"rdf:SphericalVideo",strlen("rdf:SphericalVideo"));
	    if( rval < 0 ){
	        nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] getVideoRenderMode ID(%d) 360video not found\n", __LINE__, getClipID());
	        return 0;
	    }
	    nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] getVideoRenderMode ID(%d) 360video set\n", __LINE__, getClipID());
	    //PszUUID
	    flags = RENDERMODE_360VIDEO;
	}
	else
	{
	    nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] getVideoRenderMode ID(%d) 360video fail!(%d)\n", __LINE__, getClipID(),size);
	}
	
  return flags;
}

int CClipItem::setSpeedCtlFactor(int iFactor)
{
	//if( iFactor < 13 || iFactor > 400 )
	if( iFactor < 3 || iFactor > 1600 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setSpeedCtlFactor failed(%d) ", __LINE__, m_uiClipID, iFactor);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setSpeedCtlFactor (%d) ", __LINE__, m_uiClipID, iFactor);
	m_iSpeedCtlFactor = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getSpeedCtlFactor()
{
	return m_iSpeedCtlFactor;
}

int CClipItem::getKeepPitch()
{
	return m_bKeepPitch;
}

int CClipItem::setKeepPitch(int iKeepPitch)
{
	m_bKeepPitch = iKeepPitch;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::setVoiceChangerFactor(int iFactor)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setVoiceChangerFactor In(%d) ", __LINE__, m_uiClipID, iFactor);
	if( iFactor < 0 || iFactor > 4 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setVoiceChangerFactor failed(%d) ", __LINE__, m_uiClipID, iFactor);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ClipID(%d) setSpeedCtlFactor (%d) ", __LINE__, m_uiClipID, iFactor);
	m_iVoiceChangerFactor = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}
int CClipItem::getVoiceChangerFactor()
{
	return m_iVoiceChangerFactor;
}
int CClipItem::getAudioOnOff()
{
	return m_iAudioOnOff;
}

int CClipItem::setAudioOnOff(int bOn)
{
	m_iAudioOnOff = bOn;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getAutoEnvelop()
{
	return m_iAutoEnvelop;
}

int CClipItem::setAutoEnvelop(int bOn)
{
	m_iAutoEnvelop = bOn;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getAudioVolume()
{
	return m_iClipVolume;
}

int CClipItem::setAudioVolume(int iVolume)
{
	m_iClipVolume = iVolume;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getBGMVolume()
{
	return m_iBackGroundVolume;
}

int CClipItem::setBGMVolume(int iVolume)
{
	m_iBackGroundVolume = iVolume;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int* CClipItem::getPanFactor()
{
	return m_iPanFactor;
}

int CClipItem::getPanLeftFactor()
{
	return m_iPanFactor[0];
}

int CClipItem::getPanRightFactor()
{
	return m_iPanFactor[1];
}

int CClipItem::setPanLeftFactor(int iPanFactor)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;

	if (iPanFactor >= -100 && iPanFactor <= 100)
		m_iPanFactor[0] = iPanFactor;
	else
		eRet = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	return eRet;
}

int CClipItem::setPanRightFactor(int iPanFactor)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;

	if (iPanFactor >= -100 && iPanFactor <= 100)
		m_iPanFactor[1] = iPanFactor;
	else
		eRet = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	return eRet;
}

int CClipItem::getMusicEffector()
{
	return m_iMusicEffector;
}

int CClipItem::setMusicEffector(int iFactor)
{
	m_iMusicEffector = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getProcessorStrength()
{
	return m_iProcessorStrength;
}

int CClipItem::setProcessorStrength(int iFactor)
{
	m_iProcessorStrength = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getBassStrength()
{
	return m_iBassStrength;
}

int CClipItem::setBassStrength(int iFactor)
{
	m_iBassStrength = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getSlowMotion()
{
	return m_iSlowMotion;
}

int CClipItem::setSlowMotion(int iFactor)
{
	m_iSlowMotion = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getPitchFactor()
{
	return m_iPitchIndex;
}

int CClipItem::setPitchFactor(int iFactor)
{
	m_iPitchIndex = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getCompressorFactor()
{
	return m_iCompressorFactor;
}

int CClipItem::setCompressorFactor(int iVolume)
{
	m_iCompressorFactor = iVolume;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getAudioPCMLevelCount()
{
	return (unsigned int)m_iClipPCMLevelVec.size();
}

int CClipItem::getAudioPCMLevelValue(int uiIndex)
{
	if( uiIndex < 0 || uiIndex >= m_iClipPCMLevelVec.size() )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	return m_iClipPCMLevelVec[uiIndex];
}

int CClipItem::getSeekTableCount()
{
	return (unsigned int)m_iClipSeekTabelVec.size();
}

int CClipItem::getSeekTableValue(int uiIndex)
{
	if( uiIndex < 0 || uiIndex >= m_iClipSeekTabelVec.size() )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	return m_iClipSeekTabelVec[uiIndex];
}

unsigned int CClipItem::getVisualClipID()
{
	if( m_ClipType != CLIPTYPE_AUDIO )
		return INVALID_CLIP_ID;
	return m_uiVisualClipID;
}

void CClipItem::setVisualClipID(unsigned int uiVisualClipID)
{
	m_uiVisualClipID = uiVisualClipID;
}

int CClipItem::getAudioClipCount()
{
	return (int)m_AudioClipVec.size();
}

IClipItem* CClipItem::getAudioClip(int iIndex)
{
	if( iIndex < 0 || iIndex > (int)m_AudioClipVec.size() )
		return NULL;
	m_AudioClipVec[iIndex]->AddRef();
	return m_AudioClipVec[iIndex];
}

CClipItem* CClipItem::getAudioClipUsingClipID(unsigned int uiClipID)
{
	for(int i = 0; i < (int)m_AudioClipVec.size(); i++)
	{
		if( m_AudioClipVec[i]->getClipID() == uiClipID )
		{
			m_AudioClipVec[i]->AddRef();
			return m_AudioClipVec[i];
		}
	}
	return NULL;
}

int CClipItem::getSubVideoClipCount()
{
	return (int)m_SubVideoClipVec.size();
}

IClipItem* CClipItem::getSubVideoClip(int iIndex)
{
	if( iIndex < 0 || iIndex > (int)m_SubVideoClipVec.size() )
		return NULL;
	m_SubVideoClipVec[iIndex]->AddRef();
	return m_SubVideoClipVec[iIndex];
}

CClipItem* CClipItem::getSubVideoClipUsingClipID(unsigned int uiClipID)
{
	for(int i = 0; i < (int)m_SubVideoClipVec.size(); i++)
	{
		if( m_SubVideoClipVec[i]->getClipID() == uiClipID )
		{
			m_SubVideoClipVec[i]->AddRef();
			return m_SubVideoClipVec[i];
		}
	}
	return NULL;
}

NXBOOL CClipItem::isPlayTime(unsigned int uiCurrentTime, NXBOOL bPrepare)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] isPlayTime(%d %d %d)", __LINE__, uiCurrentTime, m_uiStartTime, m_uiEndTime);

	unsigned int uiStart = m_uiStartTime;
	if( bPrepare )
	{
		if( m_ClipType == CLIPTYPE_VIDEO || m_ClipType == CLIPTYPE_VIDEO_LAYER )
		{
			unsigned int video_prepation_time = CNexCodecManager::getVideoPreparationTime();
			uiStart = uiStart > video_prepation_time? uiStart - video_prepation_time : 0;
		}
		// JIRA 2834.
		if( m_ClipType == CLIPTYPE_IMAGE )
		// if( m_ClipType == CLIPTYPE_IMAGE && uiStart > 1000 )
		{
			unsigned int video_prepation_time = CNexCodecManager::getImagePreparationTime();
			uiStart = uiStart > video_prepation_time ? uiStart - video_prepation_time : 0;
			//uiStart = uiStart > 4000 ? uiStart - 4000 : 0;
		}
	}

	if( (uiCurrentTime >= uiStart && uiCurrentTime < m_uiStartTime)  && uiCurrentTime < m_uiEndTime )
		return TRUE;

	CClipVideoRenderInfo* pRenderInfo = NULL;
	pRenderInfo = getActiveVideoRenderInfo(uiCurrentTime, FALSE);

	/* for parse/resume*/
	unsigned int uResumeTime = 0;
	unsigned int uiStartTime =  m_uiStartTime;
	unsigned int uiStartTrimTime = m_uiEndTime;
	unsigned int uiEndTrimTime = m_uiEndTime;
	int iSpeedCtlFactor = m_iSpeedCtlFactor;
	int iFreezeDuration = m_iFreezeDuration;

	if(pRenderInfo)
	{
		uiStartTime =  pRenderInfo->mStartTime;
		uiStartTrimTime = pRenderInfo->mStartTrimTime;
		uiEndTrimTime = pRenderInfo->mEndTrimTime;
		iSpeedCtlFactor = pRenderInfo->m_iSpeedCtlFactor;
		iFreezeDuration = pRenderInfo->mFreezeDuration;
	}

	if(uiCurrentTime > uiStartTime)
	{
		if( iSpeedCtlFactor == 2)
			uResumeTime = uiStartTime + (uiCurrentTime - uiStartTime) / 50;
		else if( iSpeedCtlFactor == 3)
			uResumeTime = uiStartTime + (uiCurrentTime - uiStartTime) / 32;
		else if( iSpeedCtlFactor == 6)
			uResumeTime = uiStartTime + (uiCurrentTime - uiStartTime) / 16;
		else if(iSpeedCtlFactor == 13)
			uResumeTime = uiStartTime + (uiCurrentTime - uiStartTime) / 8;
		else
			uResumeTime = uiStartTime + (uiCurrentTime - uiStartTime) * iSpeedCtlFactor / 100;
	}
	else
	{
		uResumeTime = uiCurrentTime;
	}

	if( m_pDrawInfos != NULL ) {
		if( m_pDrawInfos->isDrawTime(uResumeTime) ) {
			return TRUE;
		}
	}
	else {
		/* for parse/resume*/

		if( uiCurrentTime >= uiStartTime && uiCurrentTime < m_uiEndTime && m_uiTotalTime - (uiStartTrimTime + uiEndTrimTime) + uiStartTime + iFreezeDuration >= uResumeTime) {
			return TRUE;
		}
	}

#ifdef FOR_TEST_PREPARE_VIDEO

	if(CNexVideoEditor::m_iVideoLayerProject)
		return FALSE;

 	if( bPrepare==true && uiCurrentTime < m_uiEndTime && CNexVideoEditor::m_bPrepareVideoCodec && m_ClipType == CLIPTYPE_VIDEO )
	{
		if( m_isPlay )
		{
			if(  uiCurrentTime > m_uiEndTime && m_uiTotalTime - (uiStartTrimTime + uiEndTrimTime) + uiStartTime + iFreezeDuration > uResumeTime )
				return FALSE;
			return TRUE;
		}

		/*
		if( CNexCodecManager::getHardwareDecoderUseCount() <= 0 )
		{
			return FALSE;
		}
		*/
		
#ifdef _ANDROID
		if( CNexCodecManager::getHardwareDecodeMaxCount()  >= 2 &&
			CNexCodecManager::getHardwareDecoderUseCount() == 1 &&
			CNexCodecManager::getPrepareHardwareDecoderFlag() == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ClipID(%d) second clip forced start", __LINE__, m_uiClipID);
			CNexCodecManager::setPrepareHardwareDecoderFlag(1);
			return TRUE;
		}
#endif
	}
#endif		
	return FALSE;
}

NXBOOL CClipItem::isPlayTimeAtDirectExport(unsigned int uiCurrentTime, NXBOOL bPrepare)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] isPlayTime(%d %d %d)", __LINE__, uiCurrentTime, m_uiStartTime, m_uiEndTime);

	unsigned int uiStart = m_uiStartTime;
	if( bPrepare )
	{
		if( m_ClipType == CLIPTYPE_VIDEO || m_ClipType == CLIPTYPE_VIDEO_LAYER )
		{
			uiStart = uiStart > 300 ? uiStart - 300 : 0;
		}
	}

	if( uiCurrentTime >= uiStart  && uiCurrentTime < m_uiEndTime )
		return TRUE;
	return FALSE;
}

NXBOOL CClipItem::isPlayTimeAudio(unsigned int uiBaseTime, unsigned int uiCurrentTime, NXBOOL bCheckOnOff)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] isPlayTime(%d %d %d)", __LINE__, uiCurrentTime, m_uiStartTime, m_uiEndTime);
	if( bCheckOnOff && m_iAudioOnOff == 0 )
		return FALSE;

	// If need to check Start and End Trim Time?
	unsigned int uiStartTime = uiBaseTime + m_uiStartTime;
	// For KMSA-210 audio task early terminate problem
	unsigned int uiEndTime = uiBaseTime + m_uiEndTime;
	// unsigned int uiEndTime = uiBaseTime + m_uiEndTime - (m_uiStartTrimTime + m_uiEndTrimTime);

	// Fixed about audio latly starting issue.
	if( bCheckOnOff  )
		uiStartTime = uiStartTime < 1500 ? 0 : uiStartTime - 1500;
	if( uiCurrentTime >= uiStartTime  && uiCurrentTime < uiEndTime )
		return TRUE;
	return FALSE;
}

unsigned char* CClipItem::getImageData()
{
	return m_pImageBuffer;
}

int CClipItem::setImageData(unsigned char* pBuffer, int iSize)
{
	if( pBuffer == NULL || iSize <= 0 ) return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( m_pImageBuffer )
	{
		nexSAL_MemFree(m_pImageBuffer);
		m_pImageBuffer = NULL;
	}

	m_pImageBuffer = pBuffer;
	m_iImageBufferSize = iSize;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::setAudioClipEndTime(unsigned int uiTotalPlayTime)
{
	for( int i = 0; i < (int)m_AudioClipVec.size(); i++)
	{
		unsigned int uiAudioEndTime = m_uiStartTime + m_AudioClipVec[i]->getEndTime();
		if( uiAudioEndTime > uiTotalPlayTime )
		{
			uiAudioEndTime -= uiTotalPlayTime;
			m_AudioClipVec[i]->setEndTime(m_AudioClipVec[i]->getEndTime() - uiAudioEndTime);
		}
		m_AudioClipVec[i]->printClipTime();
	}	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CClipItem::updateTrackInfo(CVideoTrackInfo* pTrack)
{
	if( pTrack == NULL ) return FALSE;
	
	pTrack->updateTrackTime(m_uiStartTime, m_uiEndTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	RECT rcStart, rcEnd;
	rcStart.left		= m_pStartRect->getLeft();
	rcStart.top			= m_pStartRect->getTop();
	rcStart.right		= m_pStartRect->getRight();
	rcStart.bottom		= m_pStartRect->getBottom();

	rcEnd.left			= m_pEndRect->getLeft();
	rcEnd.top			= m_pEndRect->getTop();
	rcEnd.right			= m_pEndRect->getRight();
	rcEnd.bottom		= m_pEndRect->getBottom();	

	pTrack->updateImageTrackRect(rcStart, rcEnd);
	pTrack->setTrackIndexInfo(m_uiCurrentClipIndex, m_uiTotalClipCount);


	if( getClipEffectEnable() )
	{
		unsigned int uiEffectStartTime = getEndTime() - (getClipEffectDuration() * getClipEffectOffset() / 100);
		pTrack->setEffectInfo(	uiEffectStartTime, 
										getClipEffectDuration(), 
										getClipEffectOffset(), 
										getClipEffectOverlap(),
										getClipEffectID());
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setEffect(%s)\n", __LINE__, getClipEffectID());
	}
	else
	{
		pTrack->setEffectInfo(	0, 0, 0, 0, NULL);		
	}

	pTrack->setTitleInfo(m_pTitle, getTitleEffectID(), m_uiTitleStartTime, m_uiTitleEndTime);
	pTrack->setRotateState(m_iRotateState);
	pTrack->setBrightness(m_iBrightness);
	pTrack->setContrast(m_iContrast);
	pTrack->setSaturation(m_iSaturation);
    pTrack->setHue(m_iHue);
	pTrack->setTintcolor(m_iTintcolor);
	pTrack->setLUT(m_iLUT);
	pTrack->setCustomLUTA(m_iCustomLUT_A);
	pTrack->setCustomLUTB(m_iCustomLUT_B);
	pTrack->setCustomLUTPower(m_iCustomLUT_Power);
	pTrack->setVignette(m_iVignette);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] updateTrackInfo color(%d %d %d %d %d %d %d %d %d)", __LINE__, 
		m_iBrightness, m_iContrast, m_iContrast, m_iTintcolor, m_iLUT,m_iCustomLUT_A,m_iCustomLUT_B,m_iCustomLUT_Power, m_iVignette );

	pTrack->setDrawInfos(m_pDrawInfos);

	return TRUE;
}

CVideoTrackInfo* CClipItem::getCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, unsigned int uiClipID)
{
	CNexProjectManager* pProject = CNexProjectManager::getProjectManager();
	if( pProject )
	{
		CVideoTrackInfo* pTrack = pProject->getCachedVisualTrackInfo(eType, getClipID());
		SAFE_RELEASE(pProject);
		return pTrack;
	}
	return NULL;
}

void CClipItem::setCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pImageTrackInfo)
{
	/*
	if( m_pImageTrackInfo == NULL )
		return;
	*/
	
	CNexProjectManager*pProject = CNexProjectManager::getProjectManager();
	if( pProject )
	{
		pProject->setCachedVisualTrackInfo(eType, pImageTrackInfo);
		SAFE_RELEASE(pProject);
	}	
}

void CClipItem::clearCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pImageTrackInfo)
{
	CNexProjectManager*pProject = CNexProjectManager::getProjectManager();
	if( pProject )
	{
		pProject->clearCachedVisualTrackInfo(eType, pImageTrackInfo);
		SAFE_RELEASE(pProject);
	}	
}

NXBOOL CClipItem::registeImageTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo In(%p)", __LINE__, getClipID(), m_pImageTrackInfo);
	CClipVideoRenderInfoVectorIterator i;
	CClipVideoRenderInfo* pRenderInfo;

	deregisteImageTrackInfo(pVideoRender);

	if( m_bRequireTrackUpdate )
	{
		m_bRequireTrackUpdate = FALSE;
	}
	else
	{
		m_pImageTrackInfo = getCachedVisualTrackInfo(CACHE_TYPE_IMAGE, getClipID());
		if( m_pImageTrackInfo )
		{
			m_pImageTrackInfo->clearImageTrackDrawFlag();
			updateTrackInfo(m_pImageTrackInfo);

			m_pImageTrackInfo->clearClipRenderInfoVec();
			for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
			{
				pRenderInfo = ((CClipVideoRenderInfo*)(*i));
				m_pImageTrackInfo->setClipRenderInfoVec(pRenderInfo);
			}

			CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pImageTrackInfo);
			if( pMsg )
			{
				pVideoRender->SendCommand(pMsg);
				SAFE_RELEASE(pMsg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo successed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
				if( isPreView )
					SAFE_RELEASE(m_pImageTrackInfo);
				return TRUE;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo failed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
			SAFE_RELEASE(m_pImageTrackInfo);
			return FALSE;
		}
	}

	m_pImageTrackInfo = new CVideoTrackInfo;
	
	if( m_pImageTrackInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo create failed(%p)", __LINE__, getClipID(), m_pImageTrackInfo);
		return FALSE;
	}

	CClipVideoRenderInfo ImageInfo;
	//memset(&ImageInfo, 0x00, sizeof(CClipVideoRenderInfo));

	ImageInfo.mFaceDetectProcessed = 1;

	m_pImageTrackInfo->m_uiTrackID = m_uiClipID;

	ImageInfo.muiTrackID		= m_uiClipID;
	ImageInfo.mClipType			= m_ClipType;

	ImageInfo.mWidth			= m_iWidth;
	ImageInfo.mHeight			= m_iHeight;
	ImageInfo.mPitch			= m_iWidth;
	
	ImageInfo.mSrc.left			= 0;
	ImageInfo.mSrc.top			= 0;
	ImageInfo.mSrc.right		= m_iWidth;
	ImageInfo.mSrc.bottom		= m_iHeight;
	
	ImageInfo.mStart.left		= m_pStartRect->getLeft();
	ImageInfo.mStart.top		= m_pStartRect->getTop();
	ImageInfo.mStart.right		= m_pStartRect->getRight();
	ImageInfo.mStart.bottom		= m_pStartRect->getBottom();

	ImageInfo.mEnd.left			= m_pEndRect->getLeft();
	ImageInfo.mEnd.top			= m_pEndRect->getTop();
	ImageInfo.mEnd.right		= m_pEndRect->getRight();
	ImageInfo.mEnd.bottom		= m_pEndRect->getBottom();
	
	memcpy(ImageInfo.mStartMatrix, m_StartMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	memcpy(ImageInfo.mEndMatrix, m_EndMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	
	ImageInfo.mStartTime		= m_uiStartTime;
	ImageInfo.mEndTime			= m_uiEndTime;

	ImageInfo.mRotateState		= m_iRotateState;
	ImageInfo.mBrightness		= m_iBrightness;
	ImageInfo.mContrast			= m_iContrast;
	ImageInfo.mSaturation		= m_iSaturation;
    ImageInfo.mHue              = m_iHue;
	ImageInfo.mTintcolor		= m_iTintcolor;
	ImageInfo.mLUT				= m_iLUT;
	ImageInfo.mCustomLUT_A		= m_iCustomLUT_A;
	ImageInfo.mCustomLUT_B		= m_iCustomLUT_B;
	ImageInfo.mCustomLUT_Power	= m_iCustomLUT_Power;
	ImageInfo.mVignette 		= m_iVignette;

	CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
	if( pVideoEditor == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo getVideoEditor handle fail", __LINE__, getClipID());
		SAFE_RELEASE(m_pImageTrackInfo);
		return FALSE;
	}

	int iWidth			= 0;
	int iHeight			= 0;
	int iLoadedType			= 0;
	int iPitch				= 0;
	int iBitForPixel		= 0;
	int iImageDataSize	= 0;
	unsigned char* pImageBuffer = NULL;
	unsigned char* pY		= NULL;
	unsigned char* pU		= NULL;
	unsigned char* pV		= NULL;

	IMAGE_TYPE imageType = getImageType(m_strFilePath);

	// KM-2964
	if( imageType == IMAGE_JPEG && pVideoEditor->getPropertyBoolean("UseAndroidJPEG", FALSE) )
	{
        imageType = IMAGE_JPEG_ANDROID;
	}
#ifdef __APPLE__
    else if( imageType == IMAGE_JPEG )
    {
        imageType = IMAGE_JPEG_IOS;
    }
#endif

	switch(imageType)
	{
		case IMAGE_JPEG:
		{
			if( pVideoEditor->existProperty("JpegMaxWidthFactor") && pVideoEditor->existProperty("JpegMaxHeightFactor") && pVideoEditor->existProperty("JpegMaxSizeFactor") )
			{
				iWidth = pVideoEditor->getPropertyInt("JpegMaxWidthFactor", 0);
				iHeight = pVideoEditor->getPropertyInt("JpegMaxHeightFactor", 0);
				iPitch = pVideoEditor->getPropertyInt("JpegMaxSizeFactor", 0);
			}

			if( pVideoEditor->getJPEGImageHeader(m_strFilePath, &iWidth, &iHeight, &iPitch) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}
			
			ImageInfo.mWidth			= iWidth;
			ImageInfo.mHeight			= iHeight;
			ImageInfo.mPitch			= iPitch;

			pImageBuffer = new unsigned char[iPitch*iHeight*3];
			if( pImageBuffer == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			pY	= pImageBuffer;
			pU	= pY +(iPitch*iHeight * 3 / 2);
			pV	= pU +(iPitch*iHeight * 3 / 4);

			iWidth = 0;
			iHeight = 0;
			iPitch = 0;
			if( pVideoEditor->existProperty("JpegMaxWidthFactor") && pVideoEditor->existProperty("JpegMaxHeightFactor") && pVideoEditor->existProperty("JpegMaxSizeFactor") )
			{
				iWidth = pVideoEditor->getPropertyInt("JpegMaxWidthFactor", 0);
				iHeight = pVideoEditor->getPropertyInt("JpegMaxHeightFactor", 0);
				iPitch = pVideoEditor->getPropertyInt("JpegMaxSizeFactor", 0);
			}
			
			if( pVideoEditor->getJPEGImageData(m_strFilePath, &iWidth, &iHeight, &iPitch, pY, pU, pV) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				if( pImageBuffer != NULL )
				{
					delete [] pImageBuffer;
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			if( iWidth == 0 || iHeight == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				if( pImageBuffer != NULL )
				{
					delete [] pImageBuffer;
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			ImageInfo.mByteCount	= 0;
			ImageInfo.mImageY		= pY;
			ImageInfo.mImageU		= pU;
			ImageInfo.mImageV		= pV;
			ImageInfo.mIsNV12Jepg	= TRUE;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo(%d %d %p)", __LINE__, getClipID(), m_iWidth, m_iHeight, ImageInfo.mRGB);
			if( m_pImageTrackInfo->setClipRenderInfo(&ImageInfo) == FALSE )
			{
				// Render fail
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Set registeImageTrackInfo Failed", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				if( pImageBuffer != NULL )
				{
					delete [] pImageBuffer;
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			if( pImageBuffer != NULL )
			{
				delete [] pImageBuffer;
				pImageBuffer = NULL;
			}

			break;
		}

		case IMAGE_HEIF:
		{
#ifdef _ANDROID
			if (pVideoEditor->getDeviceAPILevel() < 28)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo not supported IMAGE_HEIF under android version 28", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}
#endif
		}
		case IMAGE_JPEG_ANDROID:
        case IMAGE_JPEG_IOS:
		case IMAGE_RGB:
		case IMAGE_PNG:
		case IMAGE_SVG:
		case IMAGE_WEBP:
		case IMAGE_GIF:
		case IMAGE_BMP:
		{
			void* pUserData;
			m_iThumb = 0;
			if( pVideoEditor->callbackGetImageUsingFile(m_strFilePath, m_iThumb, strlen(m_strFilePath), &iWidth, &iHeight, &iBitForPixel, &pImageBuffer, &iImageDataSize, &pUserData, &iLoadedType) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			if( iWidth == 0 || iHeight == 0 || iBitForPixel == 0 || pImageBuffer == NULL || iImageDataSize == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo image Processing fail", __LINE__, getClipID());
				pVideoEditor->callbackReleaseImage(&pUserData);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) Image date result(%d %d %p)", __LINE__, getClipID(), iWidth, iHeight, pImageBuffer);
			ImageInfo.mWidth			= iWidth;
			ImageInfo.mHeight			= iHeight;
			ImageInfo.mPitch			= iWidth;
			ImageInfo.mByteCount		= iBitForPixel / 8;
			ImageInfo.mRGB			= pImageBuffer;
			ImageInfo.mLoadedType		= 1;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo(%d %d %p)", __LINE__, getClipID(), m_iWidth, m_iHeight, ImageInfo.mRGB);
			if( m_pImageTrackInfo->setClipRenderInfo(&ImageInfo) == FALSE )
			{
				// Render fail
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Set registeImageTrackInfo Failed", __LINE__, getClipID());
				pVideoEditor->callbackReleaseImage(&pUserData); 
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			pVideoEditor->callbackReleaseImage(&pUserData);

			break;
		}
		default:
			SAFE_RELEASE(pVideoEditor);
			SAFE_RELEASE(m_pImageTrackInfo);
			return FALSE;
	};

	SAFE_RELEASE(pVideoEditor);
	
	updateTrackInfo(m_pImageTrackInfo);

	setCachedVisualTrackInfo(CACHE_TYPE_IMAGE, m_pImageTrackInfo);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipVideoRenderInfoVec.size (%d, %zu)\n", __LINE__,  getClipID(), m_ClipVideoRenderInfoVec.size());

	for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
	{
		pRenderInfo = ((CClipVideoRenderInfo*)(*i));
		m_pImageTrackInfo->setClipRenderInfoVec(pRenderInfo);
	}

 	CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pImageTrackInfo);
	if( pMsg )
	{
		pVideoRender->SendCommand(pMsg);
		pMsg->Release();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo successed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
		if( isPreView )
			SAFE_RELEASE(m_pImageTrackInfo);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo failed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
	SAFE_RELEASE(m_pImageTrackInfo);
	return FALSE;
}

NXBOOL CClipItem::registeThumbImageTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo In(%p)", __LINE__, getClipID(), m_pImageTrackInfo);
	CClipVideoRenderInfoVectorIterator i;
	CClipVideoRenderInfo* pRenderInfo;

	//deregisteImageTrackInfo(pVideoRender);
	
	if( m_bRequireTrackUpdate )
	{
		m_bRequireTrackUpdate = FALSE;
	}
	else
	{	
		m_pImageTrackInfo = getCachedVisualTrackInfo(CACHE_TYPE_IMAGE, getClipID());
		if( m_pImageTrackInfo )
		{
			if( m_pImageTrackInfo->m_iLoadedType == 1 ) 	// 1 - Original, 2 - Thumbnail
			{
				m_pImageTrackInfo->clearImageTrackDrawFlag();
				updateTrackInfo(m_pImageTrackInfo);

				m_pImageTrackInfo->clearClipRenderInfoVec();
				for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
				{
					pRenderInfo = ((CClipVideoRenderInfo*)(*i));
					m_pImageTrackInfo->setClipRenderInfoVec(pRenderInfo);
				}

				CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pImageTrackInfo);
				if( pMsg )
				{
					pVideoRender->SendCommand(pMsg);
					SAFE_RELEASE(pMsg);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo successed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
					if( isPreView )
						SAFE_RELEASE(m_pImageTrackInfo);
					return TRUE;
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo failed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}
			else
			{
				clearCachedVisualTrackInfo(CACHE_TYPE_IMAGE, m_pImageTrackInfo);
				CClipVideoRenderInfo ImageInfo;
		//		memset(&ImageInfo, 0x00, sizeof(CLIP_VIDEO_RENDER_INFO));
			
				ImageInfo.mFaceDetectProcessed = 1;
			
				m_pImageTrackInfo->m_uiTrackID = m_uiClipID;
			
				ImageInfo.muiTrackID		= m_uiClipID;
				ImageInfo.mClipType 		= m_ClipType;
			
				ImageInfo.mWidth			= m_iWidth;
				ImageInfo.mHeight			= m_iHeight;
				ImageInfo.mPitch			= m_iWidth;
				
				ImageInfo.mSrc.left 		= 0;
				ImageInfo.mSrc.top			= 0;
				ImageInfo.mSrc.right		= m_iWidth;
				ImageInfo.mSrc.bottom		= m_iHeight;
				
				ImageInfo.mStart.left		= m_pStartRect->getLeft();
				ImageInfo.mStart.top		= m_pStartRect->getTop();
				ImageInfo.mStart.right		= m_pStartRect->getRight();
				ImageInfo.mStart.bottom 	= m_pStartRect->getBottom();
			
				ImageInfo.mEnd.left 		= m_pEndRect->getLeft();
				ImageInfo.mEnd.top			= m_pEndRect->getTop();
				ImageInfo.mEnd.right		= m_pEndRect->getRight();
				ImageInfo.mEnd.bottom		= m_pEndRect->getBottom();
				
				memcpy(ImageInfo.mStartMatrix, m_StartMatrix, sizeof(float)*MATRIX_MAX_COUNT);
				memcpy(ImageInfo.mEndMatrix, m_EndMatrix, sizeof(float)*MATRIX_MAX_COUNT);
				
				ImageInfo.mStartTime		= m_uiStartTime;
				ImageInfo.mEndTime			= m_uiEndTime;
			
				ImageInfo.mRotateState		= m_iRotateState;
				ImageInfo.mBrightness		= m_iBrightness;
				ImageInfo.mContrast 		= m_iContrast;
				ImageInfo.mSaturation		= m_iSaturation;
				ImageInfo.mTintcolor		= m_iTintcolor;
				ImageInfo.mLUT				= m_iLUT;
				ImageInfo.mCustomLUT_A		= m_iCustomLUT_A;
				ImageInfo.mCustomLUT_B		= m_iCustomLUT_B;
				ImageInfo.mCustomLUT_Power	= m_iCustomLUT_Power;
				ImageInfo.mVignette 		= m_iVignette;
			
				CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
				if( pVideoEditor == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo getVideoEditor handle fail", __LINE__, getClipID());
					SAFE_RELEASE(m_pImageTrackInfo);
					return FALSE;
				}
			
				int iWidth			= 0;
				int iHeight 		= 0;
				int iLoadedType 		= 0;
				int iPitch				= 0;
				int iBitForPixel		= 0;
				int iImageDataSize	= 0;
				unsigned char* pImageBuffer = NULL;
				unsigned char* pY		= NULL;
				unsigned char* pU		= NULL;
				unsigned char* pV		= NULL;
				
				void* pUserData;
				m_iThumb = 0;
				if( pVideoEditor->callbackGetImageUsingFile(m_strFilePath, m_iThumb, strlen(m_strFilePath), &iWidth, &iHeight, &iBitForPixel, &pImageBuffer, &iImageDataSize, &pUserData, &iLoadedType) != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo image Processing fail", __LINE__, getClipID());
					pVideoEditor->callbackReleaseImage(&pUserData);
					SAFE_RELEASE(pVideoEditor);
					SAFE_RELEASE(m_pImageTrackInfo);
					return FALSE;
				}
	
				if( iWidth == 0 || iHeight == 0 || iBitForPixel == 0 || pImageBuffer == NULL || iImageDataSize == 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo image Processing fail", __LINE__, getClipID());
					pVideoEditor->callbackReleaseImage(&pUserData);
					SAFE_RELEASE(pVideoEditor);
					SAFE_RELEASE(m_pImageTrackInfo);
					return FALSE;
				}
	
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) Image date result(%d %d %p)", __LINE__, getClipID(), iWidth, iHeight, pImageBuffer);
				ImageInfo.mWidth			= iWidth;
				ImageInfo.mHeight			= iHeight;
				ImageInfo.mPitch			= iWidth;
				ImageInfo.mByteCount		= iBitForPixel / 8;
				ImageInfo.mRGB				= pImageBuffer;
				ImageInfo.mLoadedType		= iLoadedType;
	
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo(%d %d %p)", __LINE__, getClipID(), m_iWidth, m_iHeight, ImageInfo.mRGB);

				if( m_pImageTrackInfo->setClipRenderInfo(&ImageInfo) == FALSE )
				{
					// Render fail
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Set registeThumbImageTrackInfo Failed", __LINE__, getClipID());
					pVideoEditor->callbackReleaseImage(&pUserData); 
					SAFE_RELEASE(pVideoEditor);
					SAFE_RELEASE(m_pImageTrackInfo);
					return FALSE;
				}
	
				pVideoEditor->callbackReleaseImage(&pUserData);

				SAFE_RELEASE(pVideoEditor);

				m_pImageTrackInfo->clearImageTrackDrawFlag();
				updateTrackInfo(m_pImageTrackInfo);

				setCachedVisualTrackInfo(CACHE_TYPE_IMAGE, m_pImageTrackInfo);
			
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipVideoRenderInfoVec.size (%d, %zu)\n", __LINE__,  getClipID(), m_ClipVideoRenderInfoVec.size());
			
				for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
				{
					pRenderInfo = ((CClipVideoRenderInfo*)(*i));
					m_pImageTrackInfo->setClipRenderInfoVec(pRenderInfo);
				}

				pVideoRender->SendSimpleCommand(MESSAGE_DRAWIDLE);
				return TRUE;
			}
		}
	}

	m_pImageTrackInfo = new CVideoTrackInfo;
	if( m_pImageTrackInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo create failed(%p)", __LINE__, getClipID(), m_pImageTrackInfo);
		return FALSE;
	}

	CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
	if( pVideoEditor == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo getVideoEditor handle fail", __LINE__, getClipID());
		SAFE_RELEASE(m_pImageTrackInfo);
		return FALSE;
	}

	CClipVideoRenderInfo ImageInfo;
//	memset(&ImageInfo, 0x00, sizeof(CClipVideoRenderInfo));

	ImageInfo.mFaceDetectProcessed = 1;

	m_pImageTrackInfo->m_uiTrackID = m_uiClipID;

	ImageInfo.muiTrackID		= m_uiClipID;
	ImageInfo.mClipType			= m_ClipType;

	ImageInfo.mWidth			= m_iWidth;
	ImageInfo.mHeight			= m_iHeight;
	ImageInfo.mPitch			= m_iWidth;
	
	ImageInfo.mSrc.left			= 0;
	ImageInfo.mSrc.top			= 0;
	ImageInfo.mSrc.right		= m_iWidth;
	ImageInfo.mSrc.bottom		= m_iHeight;
	
	ImageInfo.mStart.left		= m_pStartRect->getLeft();
	ImageInfo.mStart.top		= m_pStartRect->getTop();
	ImageInfo.mStart.right		= m_pStartRect->getRight();
	ImageInfo.mStart.bottom		= m_pStartRect->getBottom();

	ImageInfo.mEnd.left			= m_pEndRect->getLeft();
	ImageInfo.mEnd.top			= m_pEndRect->getTop();
	ImageInfo.mEnd.right		= m_pEndRect->getRight();
	ImageInfo.mEnd.bottom		= m_pEndRect->getBottom();
	
	memcpy(ImageInfo.mStartMatrix, m_StartMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	memcpy(ImageInfo.mEndMatrix, m_EndMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	
	ImageInfo.mStartTime		= m_uiStartTime;
	ImageInfo.mEndTime			= m_uiEndTime;

	ImageInfo.mRotateState		= m_iRotateState;
	ImageInfo.mBrightness		= m_iBrightness;
	ImageInfo.mContrast			= m_iContrast;
	ImageInfo.mSaturation		= m_iSaturation;
	ImageInfo.mTintcolor		= m_iTintcolor;
	ImageInfo.mLUT				= m_iLUT;
	ImageInfo.mCustomLUT_A		= m_iCustomLUT_A;
	ImageInfo.mCustomLUT_B		= m_iCustomLUT_B;
	ImageInfo.mCustomLUT_Power	= m_iCustomLUT_Power;
	ImageInfo.mVignette 		= m_iVignette;

	int iWidth				= 0;
	int iHeight				= 0;
	int iLoadedType			= 0;
	int iPitch				= 0;
	int iBitForPixel		= 0;
	int iImageDataSize		= 0;
	unsigned char* pImageBuffer = NULL;
	unsigned char* pY		= NULL;
	unsigned char* pU		= NULL;
	unsigned char* pV		= NULL;

	IMAGE_TYPE imageType = getImageType(m_strFilePath);

	// KM-2964
	if( imageType == IMAGE_JPEG && pVideoEditor->getPropertyBoolean("UseAndroidJPEG", FALSE) )
	{
        imageType = IMAGE_JPEG_ANDROID;
	}
#ifdef __APPLE__
    else if( imageType == IMAGE_JPEG )
    {
        imageType = IMAGE_JPEG_IOS;
    }
#endif

	switch(imageType)
	{
		case IMAGE_JPEG:
		{
			if( pVideoEditor->existProperty("JpegMaxWidthFactor") && pVideoEditor->existProperty("JpegMaxHeightFactor") && pVideoEditor->existProperty("JpegMaxSizeFactor") )
			{
				iWidth = pVideoEditor->getPropertyInt("JpegMaxWidthFactor", 0);
				iHeight = pVideoEditor->getPropertyInt("JpegMaxHeightFactor", 0);
				iPitch = pVideoEditor->getPropertyInt("JpegMaxSizeFactor", 0);
			}

			if( pVideoEditor->getJPEGImageHeader(m_strFilePath, &iWidth, &iHeight, &iPitch) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}
			
			ImageInfo.mWidth			= iWidth;
			ImageInfo.mHeight			= iHeight;
			ImageInfo.mPitch			= iPitch;

			pImageBuffer = new unsigned char[iPitch*iHeight*3];
			if( pImageBuffer == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			pY	= pImageBuffer;
			pU	= pY +(iPitch*iHeight * 3 / 2);
			pV	= pU +(iPitch*iHeight * 3 / 4);

			iWidth = 0;
			iHeight = 0;
			iPitch = 0;
			if( pVideoEditor->existProperty("JpegMaxWidthFactor") && pVideoEditor->existProperty("JpegMaxHeightFactor") && pVideoEditor->existProperty("JpegMaxSizeFactor") )
			{
				iWidth = pVideoEditor->getPropertyInt("JpegMaxWidthFactor", 0);
				iHeight = pVideoEditor->getPropertyInt("JpegMaxHeightFactor", 0);
				iPitch = pVideoEditor->getPropertyInt("JpegMaxSizeFactor", 0);
			}
			
			if( pVideoEditor->getJPEGImageData(m_strFilePath, &iWidth, &iHeight, &iPitch, pY, pU, pV) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				if( pImageBuffer != NULL )
				{
					delete [] pImageBuffer;
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			if( iWidth == 0 || iHeight == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				if( pImageBuffer != NULL )
				{
					delete [] pImageBuffer;
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			ImageInfo.mByteCount	= 0;
			ImageInfo.mImageY		= pY;
			ImageInfo.mImageU		= pU;
			ImageInfo.mImageV		= pV;
			ImageInfo.mIsNV12Jepg	= TRUE;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo(%d %d %p)", __LINE__, getClipID(), m_iWidth, m_iHeight, ImageInfo.mRGB);
			if( m_pImageTrackInfo->setClipRenderInfo(&ImageInfo) == FALSE )
			{
				// Render fail
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Set registeThumbImageTrackInfo Failed", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				if( pImageBuffer != NULL )
				{
					delete [] pImageBuffer;
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			if( pImageBuffer != NULL )
			{
				delete [] pImageBuffer;
				pImageBuffer = NULL;
			}

			break;
		}

		case IMAGE_HEIF:
		{
#ifdef _ANDROID
			if (pVideoEditor->getDeviceAPILevel() < 28)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) registeImageTrackInfo not supported IMAGE_HEIF under android version 28", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}
#endif
		}
		case IMAGE_JPEG_ANDROID:
        case IMAGE_JPEG_IOS:
		case IMAGE_RGB:
		case IMAGE_PNG:
		case IMAGE_SVG:
		case IMAGE_WEBP:
		case IMAGE_GIF:
		case IMAGE_BMP:
		{
			void* pUserData;
			m_iThumb = 1;
			if( pVideoEditor->callbackGetImageUsingFile(m_strFilePath, m_iThumb, strlen(m_strFilePath), &iWidth, &iHeight, &iBitForPixel, &pImageBuffer, &iImageDataSize, &pUserData, &iLoadedType) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo image Processing fail", __LINE__, getClipID());
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			if( iWidth == 0 || iHeight == 0 || iBitForPixel == 0 || pImageBuffer == NULL || iImageDataSize == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo image Processing fail", __LINE__, getClipID());
				pVideoEditor->callbackReleaseImage(&pUserData);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) Image date result(%d %d %p)", __LINE__, getClipID(), iWidth, iHeight, pImageBuffer);
			ImageInfo.mWidth			= iWidth;
			ImageInfo.mHeight			= iHeight;
			ImageInfo.mPitch			= iWidth;
			ImageInfo.mByteCount		= iBitForPixel / 8;
			ImageInfo.mRGB				= pImageBuffer;
			ImageInfo.mLoadedType		= iLoadedType;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo(%d %d %p)", __LINE__, getClipID(), m_iWidth, m_iHeight, ImageInfo.mRGB);
			if( m_pImageTrackInfo->setClipRenderInfo(&ImageInfo) == FALSE )
			{
				// Render fail
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Set registeThumbImageTrackInfo Failed", __LINE__, getClipID());
				pVideoEditor->callbackReleaseImage(&pUserData); 
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pImageTrackInfo);
				return FALSE;
			}

			pVideoEditor->callbackReleaseImage(&pUserData);

			break;
		}
		default:
			SAFE_RELEASE(pVideoEditor);
			SAFE_RELEASE(m_pImageTrackInfo);
			return FALSE;
	};

	SAFE_RELEASE(pVideoEditor);
	updateTrackInfo(m_pImageTrackInfo);

	setCachedVisualTrackInfo(CACHE_TYPE_IMAGE, m_pImageTrackInfo);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipVideoRenderInfoVec.size (%d, %zu)\n", __LINE__,  getClipID(), m_ClipVideoRenderInfoVec.size());

	for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
	{
		pRenderInfo = ((CClipVideoRenderInfo*)(*i));
		m_pImageTrackInfo->setClipRenderInfoVec(pRenderInfo);
	}

 	CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pImageTrackInfo);
	if( pMsg )
	{
		pVideoRender->SendCommand(pMsg);
		pMsg->Release();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo successed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
		if( isPreView )
			SAFE_RELEASE(m_pImageTrackInfo);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] ID(%d) registeThumbImageTrackInfo failed(%p)\n", __LINE__, getClipID(), m_pImageTrackInfo);
	SAFE_RELEASE(m_pImageTrackInfo);
	return FALSE;
}

void CClipItem::deregisteImageTrackInfo(CNEXThreadBase* pVideoRender)
{
	if( m_pImageTrackInfo == NULL ) return;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] deregisteImageTrackInfo In(%p)", __LINE__, m_pImageTrackInfo);
	SAFE_RELEASE(m_pImageTrackInfo);
	
	CNxMsgDeleteTrack* pMsg = new CNxMsgDeleteTrack(getClipID());
	if( pMsg )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Send Msg Image Clip delete Track(%p)", __LINE__, m_pImageTrackInfo);
		pVideoRender->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
	}
}

NXBOOL CClipItem::setMissingFileInfo(void* pInfo)
{
	CClipVideoRenderInfo* pClipInfo = (CClipVideoRenderInfo*)pInfo;
	if( pClipInfo == NULL ) return FALSE;
	pClipInfo->muiTrackID			= m_uiClipID;
	pClipInfo->mClipType			= CLIPTYPE_IMAGE;

	pClipInfo->mWidth				= FILE_MISSING_IMAGE_WIDTH;
	pClipInfo->mHeight				= FILE_MISSING_IMAGE_HEIGHT;
	pClipInfo->mPitch				= FILE_MISSING_IMAGE_WIDTH;
	
	pClipInfo->mSrc.left			= 0;
	pClipInfo->mSrc.top				= 0;
	pClipInfo->mSrc.right			= FILE_MISSING_IMAGE_WIDTH;
	pClipInfo->mSrc.bottom			= FILE_MISSING_IMAGE_HEIGHT;
	
	// JIRA 2623
	pClipInfo->mStart.left			= 0;
	pClipInfo->mStart.top			= 0;
	pClipInfo->mStart.right			= 100000;
	pClipInfo->mStart.bottom		= 100000;

	pClipInfo->mEnd.left			= 0;
	pClipInfo->mEnd.top				= 0;
	pClipInfo->mEnd.right			= 100000;
	pClipInfo->mEnd.bottom			= 100000;
	
	pClipInfo->mStartTime			= m_uiStartTime;
	pClipInfo->mEndTime				= m_uiEndTime;

	pClipInfo->mRotateState			= 0;
	pClipInfo->mBrightness			= 0;
	pClipInfo->mContrast			= 0;
	pClipInfo->mSaturation			= 0;
    pClipInfo->mHue                 = 0;
	pClipInfo->mTintcolor			= 0;
	pClipInfo->mLUT					= 0;
	pClipInfo->mCustomLUT_A 		= 0;
	pClipInfo->mCustomLUT_B 		= 0;
	pClipInfo->mCustomLUT_Power		= 0;
    pClipInfo->mVignette			= 0;

	pClipInfo->mByteCount			= 0;
	pClipInfo->mRGB					= NULL;
	pClipInfo->mImageY				= NULL;
	pClipInfo->mImageU				= NULL;
	pClipInfo->mImageV				= NULL;

	pClipInfo->mImageNV12			= getFileMissingImageData();
	return TRUE;
}

NXBOOL CClipItem::registerMissingImageTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] registerMissingImageTrackInfo In(%p)", __LINE__, m_pImageTrackInfo);
	deregisteImageTrackInfo(pVideoRender);

	m_pImageTrackInfo = new CVideoTrackInfo;
	if( m_pImageTrackInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] registerMissingImageTrackInfo create failed(%p)", __LINE__, m_pImageTrackInfo);
		return FALSE;
	}

	updateTrackInfo(m_pImageTrackInfo);

	CClipVideoRenderInfo ImageInfo;
	//memset(&ImageInfo, 0x00, sizeof(CClipVideoRenderInfo));
	ImageInfo.mFaceDetectProcessed = 1;
	m_pImageTrackInfo->m_uiTrackID = m_uiClipID;

	setMissingFileInfo((void*)&ImageInfo);

	CNEXThread_VideoRenderTask* pRender = (CNEXThread_VideoRenderTask*)pVideoRender;
	if( pRender != NULL ) {
		float imageWidth = (float)ImageInfo.mWidth;
		float imageHeight = (float)ImageInfo.mHeight;
		float imageRatio = imageWidth / imageHeight;

        float surfaceWidth = 0;
        float surfaceHeight = 0;
        void* pthemeRenderer = NULL;
        {
            CNxMsgGetBaseRendererInfo* msg = new CNxMsgGetBaseRendererInfo();
            pRender->SendCommand(msg);
            msg->waitProcessDone(NEXSAL_INFINITE);

            surfaceWidth = (float)msg->m_Width;
            surfaceHeight = (float)msg->m_Height;
            pthemeRenderer = msg->m_pRenderer;

            SAFE_RELEASE(msg);
        }
        

#ifdef __APPLE__
        if (surfaceWidth == 0 || surfaceHeight == 0)
        {
            unsigned int width; unsigned int height;
            NXT_ThemeRenderer_GetRendererViewSize((NXT_HThemeRenderer)pthemeRenderer, &width, &height);
            
            surfaceWidth = (float)width;
            surfaceHeight = (float)height;
        }
#endif
        float surfaceRatio = surfaceWidth / surfaceHeight;
		float left, top, right, bottom;
		int ctrx = imageWidth/2;
		int ctry = imageHeight/2;

		if( imageRatio <= surfaceRatio )
		{
			float newWidth = imageHeight*surfaceRatio;
			left = ctrx - newWidth/2;
			right = left + newWidth;

			top = ctry - imageHeight/2;
			bottom = top + imageHeight;
		}
		else
		{
			float newHeight = imageWidth/surfaceRatio;
			top = ctry - newHeight/2;
			bottom = top + newHeight;

			left = ctrx - imageWidth/2;
			right = left + imageWidth;
		}

		ImageInfo.mStart.left			= left * 100000 / imageWidth;
		ImageInfo.mStart.top			= top * 100000 / imageHeight;
		ImageInfo.mStart.right			= right * 100000 / imageWidth;
		ImageInfo.mStart.bottom			= bottom * 100000 / imageHeight;

		ImageInfo.mEnd.left				= left * 100000 / imageWidth;
		ImageInfo.mEnd.top				= top * 100000 / imageHeight;
		ImageInfo.mEnd.right			= right * 100000 / imageWidth;
		ImageInfo.mEnd.bottom			= bottom * 100000 / imageHeight;
	}



	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] registerMissingImageTrackInfo(%d %d %p)", __LINE__, m_iWidth, m_iHeight, ImageInfo.mRGB);
	if( m_pImageTrackInfo->setClipRenderInfo(&ImageInfo) == FALSE )
	{
		// Render fail
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Set registerMissingImageTrackInfo Failed", __LINE__);
		SAFE_RELEASE(m_pImageTrackInfo);
		return FALSE;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] registerMissingImageTrackInfo Effect(%d)\n", __LINE__, getEndTime());

	CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pImageTrackInfo);
	if( pMsg )
	{
		pVideoRender->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] registerMissingImageTrackInfo successed(%p)\n", __LINE__, m_pImageTrackInfo);
		if( isPreView )
			SAFE_RELEASE(m_pImageTrackInfo);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] registerMissingImageTrackInfo failed(%p)\n", __LINE__, m_pImageTrackInfo);
	SAFE_RELEASE(m_pImageTrackInfo);
	return FALSE;
}

NXBOOL CClipItem::startPlayImage(CNEXThreadBase* pVideoRender)
{
	if( m_isPlay ) return TRUE;

	if( pVideoRender == NULL ) return  FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayImage", __LINE__);
	
#ifdef ANDROID
	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			NXBOOL bUseMissingFile = TRUE;
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor != NULL )
			{
				bUseMissingFile = pEditor->getPropertyBoolean("useMissingImage", TRUE);
				SAFE_RELEASE(pEditor);
			}

			if( bUseMissingFile ) {
				if( registerMissingImageTrackInfo(pVideoRender) )
				{
					m_isPlay = TRUE;
					return TRUE;
				}
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Image Clip did not exist(%s)", __LINE__, m_strFilePath);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);
	}
#endif

	CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
	if( pProjectMng )
	{
		CNxMsgPrepareClipLoading* pClipLoading = new CNxMsgPrepareClipLoading((int)m_uiClipID);
		if( pClipLoading )
		{
			pProjectMng->SendCommand(pClipLoading);
			SAFE_RELEASE(pClipLoading);
		}
		SAFE_RELEASE(pProjectMng);
	}

#ifdef FOR_IMAGE_LOADING_TASK
	if( m_pImageTask == NULL )
	{
		m_pImageTask = new CNEXThread_ImageTask;
		if( m_pImageTask )
		{
			m_pImageTask->setVideoRenderer((CNEXThread_VideoRenderTask*)pVideoRender);
			m_pImageTask->setClipItem(this);
			m_pImageTask->Begin();
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Image Task Create Failed(not enough memory)", __LINE__);			
		}
	}
	m_isPlay = TRUE;
	return TRUE;
#else
	if( registeImageTrackInfo(pVideoRender) )
	{
		m_isPlay = TRUE;
		return TRUE;
	}
	return FALSE;
#endif
}

NXBOOL CClipItem::startPlayVideo(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	if( m_isPlay ) return TRUE;

	if( pVideoRender == NULL ) return  FALSE;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) startPlayVideo In(%d) isPlay(%d)", __LINE__, m_uiClipID, uiTime, m_isPlay);

	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			NXBOOL bUseMissingFile = TRUE;
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor != NULL )
			{
				bUseMissingFile = pEditor->getPropertyBoolean("useMissingImage", TRUE);
				SAFE_RELEASE(pEditor);
			}

			if( bUseMissingFile ) {
				if( registerMissingImageTrackInfo(pVideoRender) )
				{
					m_isPlay = TRUE;
					return TRUE;
				}
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Video Clip did not exist(%s)", __LINE__, m_strFilePath);
			sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_uiClipID);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);
	}
	
	if( m_pSource == NULL)
	{
		int iRet = initSource();
		if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip Start Play failed(%d)", __LINE__, uiTime);
			sendNotifyEvent(MESSAGE_CLIP_PLAY, iRet, m_uiClipID);
			// Report about file initialize error
			return FALSE;
		}
		
		m_pSource->setClipID(getClipID());
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);
	}

    getVideoTrackUUID(NULL);

	unsigned int uiForAudioStartTime = uiTime;
	CCalcTime calcTime;
	CClipVideoRenderInfo* pRenderInfo;
	if((pRenderInfo = getActiveVideoRenderInfo(uiTime, FALSE)) == NULL)
		pRenderInfo = m_ClipVideoRenderInfoVec[0];
    
	if( uiTime > m_uiStartTime )
	{
		uiTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiTime);
		uiTime = uiTime + m_uiStartTime - m_uiStartTrimTime;
	}	
	
	unsigned int uiStartTime = uiTime > m_uiStartTime ? uiTime : m_uiStartTime;
	unsigned int uiResultTime = 0;
	unsigned int uiSeekResultTime = 0;

	if( (uiStartTime - m_uiStartTime) > 5000 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayVideo  Find near I Frame", __LINE__);
		
		int iRet = m_pSource->getFindNearIFramePos(uiStartTime - 5000, uiStartTime, &uiResultTime);

		// KM-1407
		unsigned int uiEndTime = pRenderInfo->mEndTime;
		uiEndTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiEndTime);

		// Case: for Mantis 9219
		if( uiResultTime > uiEndTime )
			iRet = 0;
		
		switch(iRet)
		{
			case 1:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayVideo  near I Frame finded(Target:%d Finded:%d)", __LINE__, uiStartTime, uiResultTime);
				m_pSource->seekToAudio(uiStartTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
				m_pSource->seekToVideo(uiResultTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayVideo near I Frame find failed(Target:%d Finded:%d)", __LINE__, uiStartTime, uiResultTime);
				m_pSource->seekTo(uiStartTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
				break;
		}
	}
	else
	{
		m_pSource->seekTo(uiStartTime, &uiResultTime, NXFF_RA_MODE_CUR_PREV);
	}

	CNexProjectManager* pProjectMng = CNexProjectManager::getProjectManager();
	if( pProjectMng )
	{
		CNxMsgPrepareClipLoading* pClipLoading = new CNxMsgPrepareClipLoading((int)m_uiClipID);
		if( pClipLoading )
		{
			pProjectMng->SendCommand(pClipLoading);
			SAFE_RELEASE(pClipLoading);
		}
		SAFE_RELEASE(pProjectMng);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Video Clip init Reader End Start Video(%d) Video/Audio(%p, %p)", __LINE__, uiTime, m_pVideoTask, m_pAudioTask);
	SAFE_RELEASE(m_pVideoTask);
	SAFE_RELEASE(m_pAudioTask);
	if( m_pSource->isVideoExist() && m_pVideoTask == NULL )
	{
		m_pVideoTask = new CNEXThread_VideoTask;
		if( m_pVideoTask )
		{
			m_pVideoTask->setSpeedFactor(m_iSpeedCtlFactor);
			m_pVideoTask->setVideoRenderer((CNEXThread_VideoRenderTask*)pVideoRender);
			m_pVideoTask->setClipItem(this);
			if( m_pVideoTask->setSource(m_pSource) )
			{
				m_pVideoTask->Begin();
			}
			else
			{
				SAFE_RELEASE(m_pVideoTask);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Audio Task Create Failed(not enough memory)", __LINE__);			
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Video Clip init Reader End And Start Audio Time(%d) OnOff(%d) track(%p)", __LINE__, uiTime, getAudioOnOff(), m_pAudioTask);
	// for mantis 9039 problem.
	if( getAudioOnOff() && m_pSource->isAudioExist() && uiForAudioStartTime < m_uiEndTime && m_pAudioTask == NULL ) 
	{
		m_pAudioTask = new CNEXThread_AudioTask;
		if( m_pAudioTask )
		{
			m_pAudioTask->setSpeedFactor(m_iSpeedCtlFactor);
			m_pAudioTask->setKeepPitch(m_bKeepPitch);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_pAudioTask->set m_iSpeedCtlFactor(%d) KeepPitch(%d) VoiceChangerFactor(%d) CompressorFactor(%d) PitchIndex(%d) MusicEffector(%d) ProcessorStrength(%d) iBassStrength(%d) EnhancedAudioFilter exist(%d) Equalizer exist(%d)", __LINE__, m_iSpeedCtlFactor, m_bKeepPitch, m_iVoiceChangerFactor, m_iCompressorFactor, m_iPitchIndex, m_iMusicEffector, m_iProcessorStrength, m_iBassStrength, m_pEnhancedAudioFilter?1:0, m_pEqualizer?1:0);
			m_pAudioTask->setVoiceChangerFactor(m_iVoiceChangerFactor);
			m_pAudioTask->setEnhancedAudioFilter(m_pEnhancedAudioFilter);
			m_pAudioTask->setEqualizer(m_pEqualizer);
			m_pAudioTask->setCompressorFactor(m_iCompressorFactor);
			m_pAudioTask->setPitchFactor(m_iPitchIndex);
			m_pAudioTask->setMusicEffectFactor(m_iMusicEffector, m_iProcessorStrength, m_iBassStrength);
			m_pAudioTask->setAudioRenderer((CNEXThread_AudioRenderTask*)pAudioRender);
			m_pAudioTask->setClipItem(this);
			m_pAudioTask->setClipBaseTime(0);

			if(uiStartTime > 300)
			{
				//NXINT64 uiCTS = 0;
				//if( m_pSource->getAudioFrame() != _SRC_GETFRAME_END )
				//{
				//	m_pSource->getAudioFrameCTS( &uiCTS );
				//}

				m_pAudioTask->setStartCTS(uiStartTime);
				m_pSource->seekToAudio(uiStartTime-300, &uiResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
			}
			
			if( m_pAudioTask->setSource(m_pSource) )
			{
				m_pAudioTask->Begin();
			}
			else
			{
				SAFE_RELEASE(m_pAudioTask);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Audio Task Create Failed(not enough memory)", __LINE__);
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Video Clip init Reader End Started Audio(%d)", __LINE__, uiTime);
	m_isPlay = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayVideo Out(%d)", __LINE__, uiTime);
	return TRUE;
}

NXBOOL CClipItem::startPlayVideo(unsigned int uiTime, CNEXThreadBase* pAudioRender, CNexExportWriter* pFileWriter, char* pUserData)
{
	if( m_isPlay ) return TRUE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) startPlayVideo In(%d) isPlay(%d)", __LINE__, m_uiClipID, uiTime, m_isPlay);
	
	if( pFileWriter == NULL ) return  FALSE;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) startPlayVideo In(%d) isPlay(%d)", __LINE__, m_uiClipID, uiTime, m_isPlay);

	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			sendNotifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_UNSUPPORT_FORMAT, m_uiClipID);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);		
	}
	
	if( m_pSource == NULL)
	{
		int iRet = initSource();
		if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip Start Play failed(%d)", __LINE__, uiTime);
			sendNotifyEvent(MESSAGE_CLIP_PLAY, iRet, m_uiClipID);
			// Report about file initialize error
			return FALSE;
		}
		
		m_pSource->setClipID(getClipID());
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);
	}
	
	unsigned int uiForAudioStartTime = uiTime;
	CCalcTime calcTime;
	CClipVideoRenderInfo* pRenderInfo = NULL;
	if((pRenderInfo = getActiveVideoRenderInfo(uiTime, FALSE)) == NULL)
		pRenderInfo = m_ClipVideoRenderInfoVec[0];

	if( uiTime > m_uiStartTime )
	{
		uiTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiTime);
		uiTime = uiTime + m_uiStartTime - m_uiStartTrimTime;
	}	
	
	unsigned int uiStartTime = uiTime > m_uiStartTime ? uiTime : m_uiStartTime;
	unsigned int uiResultTime = 0;
	unsigned int uiSeekResultTime = 0;

	if( (uiStartTime - m_uiStartTime) > 5000 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayVideo  Find near I Frame", __LINE__);
		
		int iRet = m_pSource->getFindNearIFramePos(uiStartTime - 5000, uiStartTime, &uiResultTime);

		// KM-1407
		unsigned int uiEndTime = pRenderInfo->mEndTime;
		uiEndTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiEndTime);

		// Case: for Mantis 9219
		if( uiResultTime > uiEndTime )
			iRet = 0;
		
		switch(iRet)
		{
			case 1:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayVideo  near I Frame finded(Target:%d Finded:%d)", __LINE__, uiStartTime, uiResultTime);
				m_pSource->seekToAudio(uiStartTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
				m_pSource->seekToVideo(uiResultTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] startPlayVideo near I Frame find failed(Target:%d Finded:%d)", __LINE__, uiStartTime, uiResultTime);
				m_pSource->seekTo(uiStartTime, &uiSeekResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
				break;
		}
	}
	else
	{
		m_pSource->seekTo(uiStartTime, &uiResultTime, NXFF_RA_MODE_CUR_PREV);
	}

#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip init Reader End Start Video(%d) Video/Audio(%p, %p %p)", 
		__LINE__, uiTime, m_pVideoTask, m_pAudioTask, m_pPFrameTask);
#endif

#ifdef __APPLE__
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip init Reader End Start Video(%d) Video/Audio(%p, %p)", 
		__LINE__, uiTime, m_pVideoTask, m_pAudioTask);
#endif

	SAFE_RELEASE(m_pVideoTask);
	SAFE_RELEASE(m_pAudioTask);
#ifdef _ANDROID
	SAFE_RELEASE(m_pDirectExportTask);
	SAFE_RELEASE(m_pPFrameTask);
#endif

#ifdef _ANDROID
	if( m_pSource->isVideoExist() )
	{
		if( isStartIDRFrame() )
		{
			if( m_pDirectExportTask == NULL )
			{
				m_pDirectExportTask = new CNEXThread_DirectExportTask;
				if( m_pDirectExportTask )
				{
					m_pDirectExportTask->setClipItem(this);
					m_pDirectExportTask->setSource(m_pSource);
					m_pDirectExportTask->setFileWriter(pFileWriter);
					m_pDirectExportTask->setDirectTaskMode(pAudioRender == NULL? FALSE : TRUE);
					m_pDirectExportTask->Begin();
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] DE Task Create Failed(not enough memory)", __LINE__);
			}
		}
		else
		{
			m_pPFrameTask = new CNEXThread_PFrameProcessTask;
			if( m_pPFrameTask )
			{
				m_pPFrameTask->setClipItem(this);
				m_pPFrameTask->setSource(m_pSource);
				m_pPFrameTask->setFileWriter(pFileWriter);
				m_pPFrameTask->setUserData(pUserData);

				m_pPFrameTask->setPFrameAndAudioMode(pAudioRender == NULL? TRUE : FALSE);
				m_pPFrameTask->Begin();
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Pframe Task Create Failed(not enough memory)", __LINE__);
			}
		}
	}
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip init Reader End And Start Audio Time(%d) OnOff(%d) track(%p)", __LINE__, uiTime, getAudioOnOff(), m_pAudioTask);
	// for mantis 9039 problem.
	// if( getAudioOnOff() && m_pFFReader->isAudioExist() && uiForAudioStartTime < m_uiEndTime - 500 ) && m_pAudioTask == NULL )
	if( pAudioRender != NULL && getAudioOnOff() && m_pSource->isAudioExist() )
	{
		m_pAudioTask = new CNEXThread_AudioTask;
		if( m_pAudioTask )
		{
			m_pAudioTask->setSpeedFactor(m_iSpeedCtlFactor);
			m_pAudioTask->setKeepPitch(m_bKeepPitch);
			m_pAudioTask->setVoiceChangerFactor(m_iVoiceChangerFactor);
			m_pAudioTask->setEnhancedAudioFilter(m_pEnhancedAudioFilter);
			m_pAudioTask->setEqualizer(m_pEqualizer);
			m_pAudioTask->setCompressorFactor(m_iCompressorFactor);
			m_pAudioTask->setPitchFactor(m_iPitchIndex);
			m_pAudioTask->setMusicEffectFactor(m_iMusicEffector, m_iProcessorStrength, m_iBassStrength);
			m_pAudioTask->setAudioRenderer((CNEXThread_AudioRenderTask*)pAudioRender);

			m_pAudioTask->setClipItem(this);
			m_pAudioTask->setClipBaseTime(0);

			if(uiStartTime > 300)
			{
				//NXINT64 uiCTS = 0;
				//if( m_pSource->getAudioFrame() != _SRC_GETFRAME_END )
				//{
				//	m_pSource->getAudioFrameCTS( &uiCTS );
				//}

				m_pAudioTask->setStartCTS(uiStartTime);
				m_pSource->seekToAudio(uiStartTime-300, &uiResultTime, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE);
			}
			
			if( m_pAudioTask->setSource(m_pSource) )
			{
				m_pAudioTask->Begin();
			}
			else
			{
				SAFE_RELEASE(m_pAudioTask);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Audio Task Create Failed(not enough memory)", __LINE__);
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip init Reader End Started Audio(%d)", __LINE__, uiTime);
	m_isPlay = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] startPlayVideo Out(%d)", __LINE__, uiTime);
	return TRUE;	
}

#define PLAY_VIDEO_DE_NONE					0
#define PLAY_VIDEO_DE_WRITE_END				1
#define PLAY_VIDEO_DE_WRITER_ERR			2
#define PLAY_VIDEO_DE_READER_INIT_ERR		3
#define PLAY_VIDEO_DE_TASK_START_ERR		4

int CClipItem::playVideoForDirectExport(CNexExportWriter* pFileWriter, char* pUserData)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) playVideoForDirectExport (%p)", __LINE__, m_uiClipID, pFileWriter);
	
	if( pFileWriter == NULL ) return  PLAY_VIDEO_DE_WRITER_ERR;
	
	if( m_pSource == NULL)
	{
		int iRet = initSource();
		if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip Start Play failed", __LINE__);
			return PLAY_VIDEO_DE_READER_INIT_ERR;
		}

		unsigned int uiResultTime = 0;
		m_pSource->setClipID(getClipID());
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);

		m_pSource->seekTo(m_uiStartTrimTime, &uiResultTime, NXFF_RA_MODE_CUR_PREV);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip init Reader End (%d %d)", __LINE__, m_uiStartTrimTime, uiResultTime);
	}
	
	// if( m_pFFReader->isVideoExist() && m_pFFReader->isAudioExist() )
	if( m_pSource->isVideoExist()  )
	{
		if( isStartIDRFrame() )
		{
			if( m_pDirectExportTask == NULL )
			{
				m_pDirectExportTask = new CNEXThread_DirectExportTask;
				if( m_pDirectExportTask )
				{
					m_pDirectExportTask->setClipItem(this);
					m_pDirectExportTask->setSource(m_pSource);
					m_pDirectExportTask->setFileWriter(pFileWriter);
					m_pDirectExportTask->Begin();
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Task Create Failed(not enough memory)", __LINE__);
					return PLAY_VIDEO_DE_TASK_START_ERR;
				}				
			}
			else
			{
				if( m_pDirectExportTask->isEndDirectExport() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] DE Task write End", __LINE__);
					m_pDirectExportTask->End(1000);
					SAFE_RELEASE(m_pDirectExportTask);					
					return PLAY_VIDEO_DE_WRITE_END;
				}
			}
		}
		else
		{
			if( m_pPFrameTask == NULL )
			{
				m_pPFrameTask = new CNEXThread_PFrameProcessTask;
				if( m_pPFrameTask )
				{
					m_pPFrameTask->setClipItem(this);
					m_pPFrameTask->setSource(m_pSource);
					m_pPFrameTask->setFileWriter(pFileWriter);
					m_pPFrameTask->setUserData(pUserData);
					m_pPFrameTask->setPFrameAndAudioMode(TRUE);
					m_pPFrameTask->Begin();
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Audio Task Create Failed(not enough memory)", __LINE__);		
					return PLAY_VIDEO_DE_TASK_START_ERR;
				}
			}
			else
			{
				if( m_pPFrameTask->isEndDirectExport() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] PFrame Task write End", __LINE__);
					m_pPFrameTask->End(1000);
					SAFE_RELEASE(m_pPFrameTask);					
					return PLAY_VIDEO_DE_WRITE_END;
				}
			}
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] playVideoForDirectExport Failed() : video is not exist!", __LINE__);
		return PLAY_VIDEO_DE_TASK_START_ERR;	
	}
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] playVideoForDirectExport Out", __LINE__);
	return PLAY_VIDEO_DE_NONE;	
}

NXBOOL CClipItem::startPlayAudio(unsigned int uiTime, CNEXThreadBase* pAudioRender, NXBOOL bBGM )
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] startPlayAudio In(%d) isPlay(%d)", __LINE__, uiTime, m_isPlay);
	
	unsigned int uiInternalTime = uiTime;
	unsigned int uiStartTime	= 0;
	unsigned int uiTotalTime	= m_uiTotalTime - (m_uiStartTrimTime + m_uiEndTrimTime);

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Time(%d %d %d) Index(%d)", __LINE__, uiTime, m_AudioClipVec[i]->m_uiStartTime, m_AudioClipVec[i]->m_uiEndTime, i);
	CNEXThread_AudioRenderTask* pAudio = (CNEXThread_AudioRenderTask*)pAudioRender;
	if( pAudio )
	{
		// Fixed about audio starting time was differnt problem.
		// For JIRA 2674
		unsigned int uiAudioRenderInternalTime  = pAudio->getInternalAudioTime();;
		uiInternalTime = uiAudioRenderInternalTime > 15 ? uiAudioRenderInternalTime : uiInternalTime;
	}

#ifdef _ANDROID
	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_uiClipID);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);
	}
	
#endif
	if( isPlayTimeAudio(0, uiInternalTime)  == FALSE )
	{
		if( m_pAudioTask != NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] startPlayAudio Stop Audio Play(%d %d %d %d %d %d) (%p %p)",
				__LINE__, uiTime, uiInternalTime, m_uiStartTime, m_uiEndTime, m_uiStartTrimTime, m_uiEndTrimTime, m_pAudioTask, m_pSource);
			if( m_pAudioTask->IsWorking() )
				m_pAudioTask->End(3000);
			SAFE_RELEASE(m_pAudioTask);
			SAFE_RELEASE(m_pSource);
			m_isPlay = FALSE;
		}
		return TRUE;
	}
	
	if( m_isPlay ) return TRUE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) startPlayAudio In(%d) isPlay(%d)", __LINE__, m_uiClipID, uiTime, m_isPlay);


	unsigned int uiResultTime = 0;
	unsigned int uiRepeatCount = 0;    
	unsigned int uiRemainTime = 0;
	unsigned int uiBaseTime = 0;    
	if( m_pSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Video Clip Start Play(%d)", __LINE__, m_uiClipID, uiTime);
		if( initSource() != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Video Clip Start Play failed(%d)", __LINE__, m_uiClipID, uiTime);
			// Report about file initialize error
			return FALSE;
		}
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);

		if( m_iSpeedCtlFactor != 100 )
		{
			if( uiTime > m_uiStartTime )
			{
				if(m_iSpeedCtlFactor == 3)
				{
					uiTotalTime = uiTotalTime * 32;
				}
				else if(m_iSpeedCtlFactor == 6)
				{
					uiTotalTime = uiTotalTime * 16;
				}
				else if(m_iSpeedCtlFactor == 13)
				{
					uiTotalTime = uiTotalTime * 8;
				}
				else
				{
					uiTotalTime = uiTotalTime * 100 / m_iSpeedCtlFactor;
				}
			}
		}

		if(m_ClipAudioRenderInfoVec.size() > 1)
		{
			uiTotalTime = 	m_uiEndTime - m_uiStartTime;
		}

		// for mantis 6045, 6774
		if( bBGM )
		{
			if( uiTotalTime > 0 )
			{
				unsigned int uiRepeatCount = uiTime / uiTotalTime;
				unsigned int uiRemainTime = uiTime % uiTotalTime;
				if( (uiTotalTime - uiRemainTime) < 100 )
				{
					m_pSource->setBaseTimeStamp(uiTotalTime * (uiRepeatCount + 1));
					uiStartTime = 0;
				}
				else
				{
					m_pSource->setBaseTimeStamp(uiTotalTime * uiRepeatCount );
					uiStartTime	=  uiRemainTime;
				}
				m_pSource->seekTo(uiStartTime, &uiResultTime, NXFF_RA_MODE_CUR_PREV, TRUE, FALSE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Audio Clip init Reader End And Start Audio SeekTime(%d) Remain(%d)OnOff(%d)", __LINE__, m_uiClipID, uiStartTime, uiRemainTime, getAudioOnOff());
			}
		}
		else
		{
			// patch for audio repeat problem
			if( uiTotalTime > 0 && uiTime > 0)
			{
				if(uiTime > m_uiStartTime + uiTotalTime)
				{
					uiRepeatCount = (uiTime - m_uiStartTime) / uiTotalTime;
					uiRemainTime = (uiTime - m_uiStartTime) % uiTotalTime;
					if( (uiTotalTime - uiRemainTime) < 20 )
					{
						uiBaseTime = uiTotalTime * (++uiRepeatCount);
					}
					else
					{
						uiBaseTime = uiTotalTime * uiRepeatCount;
					}
					uiBaseTime += m_uiStartTime;
					m_pSource->setBaseTimeStamp(uiBaseTime);
				}
			}

			if( uiTime > m_uiStartTime )
			{
				CClipAudioRenderInfo* pRenderInfo;
				CCalcTime calcTime;

				if((pRenderInfo = getActiveAudioRenderInfo(uiTime, FALSE)) == NULL)
					pRenderInfo = m_ClipAudioRenderInfoVec[0];

				if(uiTime > uiBaseTime)
					uiTime = uiTime - uiBaseTime;
				else
					uiTime = 0;

				uiTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiTime);
				uiTime = uiTime + m_uiStartTime;
				if(uiRepeatCount == 0)
					uiTime = uiTime - m_uiStartTrimTime;

				uiStartTime	=  uiTime;
			}
			else
			{
				uiStartTime	=  uiTime > m_uiStartTime ? uiTime : m_uiStartTime; 
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Audio Clip init Reader End And Start Audio SeekTime(%d) Remain(%d, %d)OnOff(%d)", __LINE__, m_uiClipID, uiStartTime, uiRepeatCount, uiRemainTime, getAudioOnOff());            
			m_pSource->seekToAudio(uiStartTime, &uiResultTime, NXFF_RA_MODE_CUR_PREV, uiRepeatCount, FALSE);
		}
	}
	
	SAFE_RELEASE(m_pAudioTask);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Audio Clip init Reader End And Start Audio Time(%d) OnOff(%d)", __LINE__, m_uiClipID, uiTime, getAudioOnOff());
	
	if( getAudioOnOff() && m_pSource->isAudioExist() && m_pAudioTask == NULL )
	{
		m_pAudioTask = new CNEXThread_AudioTask;
		if( m_pAudioTask )
		{
			m_pAudioTask->setSpeedFactor(m_iSpeedCtlFactor);
			m_pAudioTask->setKeepPitch(m_bKeepPitch);
			m_pAudioTask->setClipBaseTime(0);
			m_pAudioTask->setAudioRenderer((CNEXThread_AudioRenderTask*)pAudioRender);
			m_pAudioTask->setClipItem(this);
			m_pAudioTask->setVoiceChangerFactor(m_iVoiceChangerFactor);
			m_pAudioTask->setEnhancedAudioFilter(m_pEnhancedAudioFilter);
			m_pAudioTask->setEqualizer(m_pEqualizer);
			m_pAudioTask->setCompressorFactor(m_iCompressorFactor);
			m_pAudioTask->setPitchFactor(m_iPitchIndex);
			m_pAudioTask->setMusicEffectFactor(m_iMusicEffector, m_iProcessorStrength, m_iBassStrength);
			if( bBGM )
			{
				m_pAudioTask->setAutoEnvelop(TRUE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] set BGM status(%d)", __LINE__, bBGM);
			}
			
			if( m_iAutoEnvelop == 1 )
			{
				m_pAudioTask->setAutoEnvelop(TRUE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) set Auto envelop status(%d)", __LINE__, m_uiClipID, m_iAutoEnvelop);
			}

			if(uiStartTime > 300)
			{
				NXINT64 uiCTS = 0;
				m_pSource->getAudioFrameCTS( &uiCTS );
				m_pAudioTask->setStartCTS(uiCTS);
				m_pSource->seekToAudio(uiStartTime-300, &uiResultTime, NXFF_RA_MODE_CUR_PREV, uiRepeatCount, FALSE);
			}

			if( m_pAudioTask->setSource(m_pSource) )
			{
				m_pAudioTask->Begin();
			}
			else
			{
				SAFE_RELEASE(m_pAudioTask);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Audio Task Create Failed(not enough memory)", __LINE__, m_uiClipID);
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Video Clip init Reader End Started Audio(%d)", __LINE__, m_uiClipID, uiTime);
	m_isPlay = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) startPlayAudio Out(%d)", __LINE__, m_uiClipID, uiTime);
	return TRUE;
}

NXBOOL CClipItem::updatePlayTime(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	CAutoLock lock(m_Lock);

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] updatePlayTime(%d %d %d %d %p)", __LINE__, m_ClipType, m_uiStartTime, m_uiEndTime, uiTime, m_pImageTrackInfo);	
	if( isPlayTime(uiTime, TRUE) )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] updatePlayTime(%d %d %d %d %p)", __LINE__, m_ClipType, m_uiStartTime, m_uiEndTime, uiTime, m_pImageTrackInfo);
		if( m_ClipType == CLIPTYPE_IMAGE )
		{
			if( startPlayImage(pVideoRender) == FALSE )
			{
				sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_uiClipID);
			}
		}
		else
		{
			/*
			if( m_iSpeedCtlFactor != 100 )
			{
				uiTime = uiTime * m_iSpeedCtlFactor / 100;
			}
			*/
			startPlayVideo(uiTime, pVideoRender, pAudioRender);
			/*
			if( startPlayVideo(uiTime, pVideoRender, pAudioRender) == FALSE )
			{
				sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED, m_uiClipID);
			}
			*/
		}
	}
	else
	{
		if(uiTime > m_uiEndTime)
		{
			if( m_ClipType == CLIPTYPE_VIDEO || m_ClipType == CLIPTYPE_VIDEO_LAYER )
			{
				//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip Stop Play", __LINE__);
				if( m_pVideoTask )
				{
					unsigned int uiTick = nexSAL_GetTickCount();
					if(m_pVideoTask->IsWorking())
					{
						m_pVideoTask->End(3000);
					}
					SAFE_RELEASE(m_pVideoTask);
					uiTick = nexSAL_GetTickCount() - uiTick;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay Video End Time(%d)", __LINE__, m_uiClipID, uiTick);	
				}
				if( m_pAudioTask && m_pAudioTask->IsWorking() )
				{
					unsigned int uiTick = nexSAL_GetTickCount();
					m_pAudioTask->End(3000);
					SAFE_RELEASE(m_pAudioTask);
					uiTick = nexSAL_GetTickCount() - uiTick;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay Audio End Time(%d)", __LINE__, m_uiClipID, uiTick);	
				}
				SAFE_RELEASE(m_pSource);
				deregisteImageTrackInfo(pVideoRender);
				m_isPlay = FALSE;
			}
			
#ifdef FOR_IMAGE_LOADING_TASK
			if( m_pImageTask )
			{
				m_pImageTask->End(1000);
				SAFE_RELEASE(m_pImageTask);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay m_pImageTask End ", __LINE__, m_uiClipID);
			}
#endif

			if( m_ClipType == CLIPTYPE_IMAGE )
			{
				deregisteImageTrackInfo(pVideoRender);
				m_isPlay = FALSE;
			}
		}
	}

	for( int i = 0; i < (int)m_AudioClipVec.size(); i++)
	{
		unsigned int uiInternalTime = uiTime;
		
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Time(%d %d %d) Index(%d)", __LINE__, uiTime, m_AudioClipVec[i]->m_uiStartTime, m_AudioClipVec[i]->m_uiEndTime, i);
		CNEXThread_AudioRenderTask* pAudio = (CNEXThread_AudioRenderTask*)pAudioRender;
		if( pAudio )
		{
			// Fixed about audio starting time was differnt problem.
			// For JIRA 2674
			unsigned int uiAudioRenderInternalTime  = pAudio->getInternalAudioTime();;
			uiInternalTime = uiAudioRenderInternalTime > 15 ? uiAudioRenderInternalTime : uiInternalTime;
		}
		
		if( m_AudioClipVec[i]->isPlayTimeAudio(m_uiStartTime, uiInternalTime) )
		{
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Time(%d %d %d)", __LINE__, uiTime, m_AudioClipVec[i]->m_uiStartTime, m_AudioClipVec[i]->m_uiEndTime);
			unsigned int uiResultTime 		= 0;
			unsigned int uiClipStartPos		= 0;
			unsigned int uiClipPlayTime	= m_AudioClipVec[i]->getTotalTime() - (m_AudioClipVec[i]->getStartTrimTime() + m_AudioClipVec[i]->getEndTrimTime() );
			unsigned int uiClipBaseTime	= m_uiStartTime + m_AudioClipVec[i]->getStartTime();

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Time(%d %d %d)", __LINE__, uiTime, m_AudioClipVec[i]->m_uiStartTime, m_AudioClipVec[i]->m_uiEndTime);

			if( uiInternalTime > uiClipBaseTime )
			{
				uiClipStartPos		= (uiInternalTime - uiClipBaseTime)  % uiClipPlayTime;
				uiClipBaseTime	= uiInternalTime - uiClipStartPos;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] recalc Audio Clip Time(base %d start %d)", __LINE__, uiClipBaseTime, uiClipStartPos);
				// recalc Audio Clip Time(base 2924 start 0)
			}
			
			if( m_AudioClipVec[i]->m_pSource == NULL )
			{

				// RYU 20131025 : if the current time + 1000 is bigger than endtime, don't start audio task. 
				if(m_AudioClipVec[i]->isPlayTimeAudio(m_uiStartTime, uiInternalTime+1000) == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] current Time and end time is too close to start task!!", __LINE__, m_AudioClipVec[i]->getClipID());
					return FALSE;
				}
				
				int iRet = m_AudioClipVec[i]->initSource();
				if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Start failed", __LINE__);
					// sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED, m_AudioClipVec[i]->getClipID());
					sendNotifyEvent(MESSAGE_CLIP_PLAY, iRet, m_AudioClipVec[i]->getClipID());
					return FALSE;
				}
				m_AudioClipVec[i]->m_pSource->setTrimTime(m_AudioClipVec[i]->getStartTrimTime(), m_AudioClipVec[i]->getEndTrimTime());
				m_AudioClipVec[i]->m_pSource->setRepeatAudioClip(uiClipBaseTime, uiClipStartPos + m_AudioClipVec[i]->getStartTrimTime()) ;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] File reader already created(ID:%d)", __LINE__, m_AudioClipVec[i]->getClipID());
			}

			if( m_AudioClipVec[i]->m_pSource->isAudioExist() && m_AudioClipVec[i]->m_pAudioTask == NULL )
			{
				m_AudioClipVec[i]->m_pAudioTask = new CNEXThread_AudioTask;
				m_AudioClipVec[i]->m_pAudioTask->setAudioRenderer((CNEXThread_AudioRenderTask*)pAudioRender);
				m_AudioClipVec[i]->m_pAudioTask->setClipItem(m_AudioClipVec[i]);
				m_AudioClipVec[i]->m_pAudioTask->setClipBaseTime(m_uiStartTime);
				m_AudioClipVec[i]->m_pAudioTask->setVoiceChangerFactor(m_iVoiceChangerFactor);
				m_AudioClipVec[i]->m_pAudioTask->setEnhancedAudioFilter(m_pEnhancedAudioFilter);
				m_AudioClipVec[i]->m_pAudioTask->setEqualizer(m_pEqualizer);
				m_AudioClipVec[i]->m_pAudioTask->setCompressorFactor(m_iCompressorFactor);
				m_AudioClipVec[i]->m_pAudioTask->setPitchFactor(m_iPitchIndex);
				m_AudioClipVec[i]->m_pAudioTask->setKeepPitch(m_bKeepPitch);
				m_AudioClipVec[i]->m_pAudioTask->setMusicEffectFactor(m_iMusicEffector, m_iProcessorStrength, m_iBassStrength);
				if( m_AudioClipVec[i]->m_pAudioTask->setSource(m_AudioClipVec[i]->m_pSource) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Start Play(%d)", __LINE__, uiInternalTime);
					m_AudioClipVec[i]->m_pAudioTask->Begin();
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] prepare Audio start failed(%d) ", __LINE__, uiInternalTime);
					SAFE_RELEASE(m_AudioClipVec[i]->m_pAudioTask);
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] AD Task already created(ID:%d)", __LINE__, m_AudioClipVec[i]->getClipID());
			}
		}
		else
		{
			if( m_AudioClipVec[i]->m_pAudioTask && m_AudioClipVec[i]->m_pAudioTask->IsWorking() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Stop Play(%d)", __LINE__, uiInternalTime);
				m_AudioClipVec[i]->m_pAudioTask->End(3000);
				SAFE_RELEASE(m_AudioClipVec[i]->m_pAudioTask);
			}
			SAFE_RELEASE(m_AudioClipVec[i]->m_pSource);
		}
	}
	return TRUE;
}

NXBOOL CClipItem::updatePlayTime(unsigned int uiTime, CNEXThreadBase* pAudioRender, CNexExportWriter* pFileWriter, char* pUserData)
{
	CAutoLock lock(m_Lock);

	if( isPlayTimeAtDirectExport(uiTime, TRUE) )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] updatePlayTime(%d %d %d %d %p)", __LINE__, m_ClipType, m_uiStartTime, m_uiEndTime, uiTime, m_pImageTrackInfo);	
		if( m_ClipType == CLIPTYPE_IMAGE )
		{
			sendNotifyEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_UNSUPPORT_FORMAT, m_uiClipID);
			return FALSE;
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] updatePlayTime(%d %d %d %d %p)", __LINE__, m_ClipType, m_uiStartTime, m_uiEndTime, uiTime, m_pImageTrackInfo);	
			/*
			if( m_iSpeedCtlFactor != 100 )
			{
				uiTime = uiTime * m_iSpeedCtlFactor / 100;
			}
			*/
			startPlayVideo(uiTime, pAudioRender, pFileWriter, pUserData);
			/*
			if( startPlayVideo(uiTime, pVideoRender, pAudioRender) == FALSE )
			{
				sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED, m_uiClipID);
			}
			*/
		}
	}
	else
	{
		if( m_ClipType == CLIPTYPE_VIDEO || m_ClipType == CLIPTYPE_VIDEO_LAYER )
		{
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Video Clip Stop Play", __LINE__);
			if( m_pVideoTask )
			{
				unsigned int uiTick = nexSAL_GetTickCount();
				if(m_pVideoTask->IsWorking())
				{
					m_pVideoTask->End(3000);
				}
				SAFE_RELEASE(m_pVideoTask);
				uiTick = nexSAL_GetTickCount() - uiTick;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay Video End Time(%d)", __LINE__, m_uiClipID, uiTick);	
			}
			if( m_pAudioTask && m_pAudioTask->IsWorking() )
			{
				unsigned int uiTick = nexSAL_GetTickCount();
				m_pAudioTask->End(3000);
				SAFE_RELEASE(m_pAudioTask);
				uiTick = nexSAL_GetTickCount() - uiTick;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay Audio End Time(%d)", __LINE__, m_uiClipID, uiTick);	
			}
#ifdef _ANDROID
			if( m_pPFrameTask && m_pPFrameTask->IsWorking() )
			{
				unsigned int uiTick = nexSAL_GetTickCount();
				m_pPFrameTask->End(3000);
				SAFE_RELEASE(m_pPFrameTask);
				uiTick = nexSAL_GetTickCount() - uiTick;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay Video(PFrameTask) End Time(%d)", __LINE__, m_uiClipID, uiTick);	
			}
			
			if( m_pDirectExportTask && m_pDirectExportTask->IsWorking() )
			{
				unsigned int uiTick = nexSAL_GetTickCount();
				m_pDirectExportTask->End(3000);
				SAFE_RELEASE(m_pDirectExportTask);
				uiTick = nexSAL_GetTickCount() - uiTick;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay Video(DE) End Time(%d)", __LINE__, m_uiClipID, uiTick);
			}
#endif
			
			SAFE_RELEASE(m_pSource);
			m_isPlay = FALSE;
		}
	}

	for( int i = 0; i < (int)m_AudioClipVec.size(); i++)
	{
		unsigned int uiInternalTime = uiTime;
		
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Time(%d %d %d) Index(%d)", __LINE__, uiTime, m_AudioClipVec[i]->m_uiStartTime, m_AudioClipVec[i]->m_uiEndTime, i);
		CNEXThread_AudioRenderTask* pAudio = (CNEXThread_AudioRenderTask*)pAudioRender;
		if( pAudio )
		{
			// Fixed about audio starting time was differnt problem.
			// For JIRA 2674
			unsigned int uiAudioRenderInternalTime  = pAudio->getInternalAudioTime();;
			uiInternalTime = uiAudioRenderInternalTime > 15 ? uiAudioRenderInternalTime : uiInternalTime;
		}
		
		if( m_AudioClipVec[i]->isPlayTimeAudio(m_uiStartTime, uiInternalTime) )
		{
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Time(%d %d %d)", __LINE__, uiTime, m_AudioClipVec[i]->m_uiStartTime, m_AudioClipVec[i]->m_uiEndTime);
			unsigned int uiResultTime 		= 0;
			unsigned int uiClipStartPos		= 0;
			unsigned int uiClipPlayTime	= m_AudioClipVec[i]->getTotalTime() - (m_AudioClipVec[i]->getStartTrimTime() + m_AudioClipVec[i]->getEndTrimTime() );
			unsigned int uiClipBaseTime	= m_uiStartTime + m_AudioClipVec[i]->getStartTime();

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Time(%d %d %d)", __LINE__, uiTime, m_AudioClipVec[i]->m_uiStartTime, m_AudioClipVec[i]->m_uiEndTime);

			if( uiInternalTime > uiClipBaseTime )
			{
				uiClipStartPos		= (uiInternalTime - uiClipBaseTime)  % uiClipPlayTime;
				uiClipBaseTime	= uiInternalTime - uiClipStartPos;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] recalc Audio Clip Time(base %d start %d)", __LINE__, uiClipBaseTime, uiClipStartPos);
				// recalc Audio Clip Time(base 2924 start 0)
			}
			
			if( m_AudioClipVec[i]->m_pSource == NULL )
			{

				// RYU 20131025 : if the current time + 1000 is bigger than endtime, don't start audio task. 
				if(m_AudioClipVec[i]->isPlayTimeAudio(m_uiStartTime, uiInternalTime+1000) == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] current Time and end time is too close to start task!!", __LINE__, m_AudioClipVec[i]->getClipID());
					return FALSE;
				}
				
				int iRet = m_AudioClipVec[i]->initSource();
				if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Start failed", __LINE__);
					// sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED, m_AudioClipVec[i]->getClipID());
					sendNotifyEvent(MESSAGE_CLIP_PLAY, iRet, m_AudioClipVec[i]->getClipID());
					return FALSE;
				}
				m_AudioClipVec[i]->m_pSource->setTrimTime(m_AudioClipVec[i]->getStartTrimTime(), m_AudioClipVec[i]->getEndTrimTime());
				m_AudioClipVec[i]->m_pSource->setRepeatAudioClip(uiClipBaseTime, uiClipStartPos + m_AudioClipVec[i]->getStartTrimTime()) ;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] File reader already created(ID:%d)", __LINE__, m_AudioClipVec[i]->getClipID());
			}

			if( m_AudioClipVec[i]->m_pSource->isAudioExist() && m_AudioClipVec[i]->m_pAudioTask == NULL )
			{
				m_AudioClipVec[i]->m_pAudioTask = new CNEXThread_AudioTask;
				m_AudioClipVec[i]->m_pAudioTask->setAudioRenderer((CNEXThread_AudioRenderTask*)pAudioRender);
				m_AudioClipVec[i]->m_pAudioTask->setClipItem(m_AudioClipVec[i]);
				m_AudioClipVec[i]->m_pAudioTask->setClipBaseTime(m_uiStartTime);
				m_AudioClipVec[i]->m_pAudioTask->setVoiceChangerFactor(m_iVoiceChangerFactor);
				m_AudioClipVec[i]->m_pAudioTask->setEnhancedAudioFilter(m_pEnhancedAudioFilter);
				m_AudioClipVec[i]->m_pAudioTask->setEqualizer(m_pEqualizer);
				m_AudioClipVec[i]->m_pAudioTask->setCompressorFactor(m_iCompressorFactor);
				m_AudioClipVec[i]->m_pAudioTask->setPitchFactor(m_iPitchIndex);
				m_AudioClipVec[i]->m_pAudioTask->setKeepPitch(m_bKeepPitch);
				m_AudioClipVec[i]->m_pAudioTask->setMusicEffectFactor(m_iMusicEffector, m_iProcessorStrength, m_iBassStrength);
				if( m_AudioClipVec[i]->m_pAudioTask->setSource(m_AudioClipVec[i]->m_pSource) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Start Play(%d)", __LINE__, uiInternalTime);
					m_AudioClipVec[i]->m_pAudioTask->Begin();
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] prepare Audio start failed(%d) ", __LINE__, uiInternalTime);
					SAFE_RELEASE(m_AudioClipVec[i]->m_pAudioTask);
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] AD Task already created(ID:%d)", __LINE__, m_AudioClipVec[i]->getClipID());
			}
		}
		else
		{
			if( m_AudioClipVec[i]->m_pAudioTask && m_AudioClipVec[i]->m_pAudioTask->IsWorking() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Stop Play(%d)", __LINE__, uiInternalTime);
				m_AudioClipVec[i]->m_pAudioTask->End(3000);
				SAFE_RELEASE(m_AudioClipVec[i]->m_pAudioTask);
			}
			SAFE_RELEASE(m_AudioClipVec[i]->m_pSource);
		}
	}
	return TRUE;	
}

NXBOOL CClipItem::updatePlay4Export(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	CAutoLock lock(m_Lock);

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Clip.cpp %d] updatePlayTime(%d %d %d %d %p)", __LINE__, m_ClipType, m_uiStartTime, m_uiEndTime, uiTime, m_pImageTrackInfo);	
	if( isPlayTime(uiTime, TRUE) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] updatePlay4Export(%d %d %d %d %p)", __LINE__, m_uiClipID, m_uiStartTime, m_uiEndTime, uiTime, m_pImageTrackInfo);
		if( m_ClipType == CLIPTYPE_IMAGE )
		{
			if( startPlayImage(pVideoRender) == FALSE )
			{
				sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_uiClipID);
			}
		}
		else
		{
			/*
			if( m_iSpeedCtlFactor != 100 )
			{
				uiTime = uiTime * m_iSpeedCtlFactor / 100;
			}
			*/
			startPlayVideo(uiTime, pVideoRender, pAudioRender);
			/*
			if( startPlayVideo(uiTime, pVideoRender, pAudioRender) == FALSE )
			{
				sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED, m_uiClipID);
			}
			*/
		}
	}

	return TRUE;
}

NXBOOL CClipItem::stopPlay()
{
	CAutoLock lock(m_Lock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d](%d) stopPlay(%d %d %d %p) AudioTask(%p) VideoTask(%p)", 
		__LINE__, m_uiClipID, m_ClipType, m_uiStartTime, m_uiEndTime, m_pImageTrackInfo, m_pAudioTask, m_pVideoTask);	
	if( m_pVideoTask )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		if(m_pVideoTask->IsWorking())
		{
			m_pVideoTask->End(1000);
		}
		SAFE_RELEASE(m_pVideoTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay Video End Time(%d)", __LINE__, uiTick);	
	}

#ifdef _ANDROID
	if( m_pDirectExportTask && m_pDirectExportTask->IsWorking() )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		m_pDirectExportTask->End(1000);
		SAFE_RELEASE(m_pDirectExportTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay DirectExportTask End Time(%d)", __LINE__, uiTick);	
	} 
#endif
 
	if( m_pVideoThumbTask && m_pVideoThumbTask->IsWorking() )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		m_pVideoThumbTask->End(1000);
		SAFE_RELEASE(m_pVideoThumbTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay Video Thumb task End Time(%d)", __LINE__, uiTick);	
	}

	if( m_pVideoPreviewTask && m_pVideoPreviewTask->IsWorking() )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		m_pVideoPreviewTask->End(1000);
		SAFE_RELEASE(m_pVideoPreviewTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay Video Preview task End Time(%d)", __LINE__, uiTick);	
	}

#ifdef _ANDROID
	if( m_pPFrameTask && m_pPFrameTask->IsWorking() )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		m_pPFrameTask->End(1000);
		SAFE_RELEASE(m_pPFrameTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay Video(PFrameTask) End Time(%d)", __LINE__, uiTick);	
	}
#endif
	
	if( m_pAudioTask && m_pAudioTask->IsWorking() )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		m_pAudioTask->setPCMRenderWaitFlag(FALSE);
		m_pAudioTask->End(1000);
		SAFE_RELEASE(m_pAudioTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay Audio End Time(%d)", __LINE__, uiTick);	
	}

	SAFE_RELEASE(m_pSource);

	for( int i = 0; i < (int)m_AudioClipVec.size(); i++)
	{
		if( m_AudioClipVec[i]->m_pAudioTask && m_AudioClipVec[i]->m_pAudioTask->IsWorking() )
		{
			m_AudioClipVec[i]->m_pAudioTask->setPCMRenderWaitFlag(FALSE);
			m_AudioClipVec[i]->m_pAudioTask->End(1000);
			SAFE_RELEASE(m_AudioClipVec[i]->m_pAudioTask);
		}

		SAFE_RELEASE(m_AudioClipVec[i]->m_pSource);
	}

#ifdef FOR_IMAGE_LOADING_TASK
	if( m_pImageTask != NULL )
	{
		m_pImageTask->End(1000);
		SAFE_RELEASE(m_pImageTask);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) stopPlay m_pImageTask End ", __LINE__, m_uiClipID);
	}
#endif

	if( m_ClipType == CLIPTYPE_IMAGE || m_pImageTrackInfo )
	{
		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			deregisteImageTrackInfo(pVideoRender);
			SAFE_RELEASE(pVideoRender);
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] stopPlay delete m_pImageTrackInfo", __LINE__);
		SAFE_RELEASE(m_pImageTrackInfo);
	}
	
	m_isPlay = FALSE;
	return TRUE;
}

CNexSource* CClipItem::getSource()
{
	if( m_pSource == NULL )
	{
		int iRet = initSource();
		if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] getFileReader failed because reader init failed(%d)", __LINE__, iRet);
			return NULL;
		}
		
		m_pSource->setClipID(getClipID());
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);		
	}
	SAFE_ADDREF(m_pSource);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] getFileReader successed(%p)", __LINE__, m_pSource);
	return m_pSource;
}

NXBOOL CClipItem::pausePlayTime()
{
	CAutoLock lock(m_Lock);
	if(m_isPlay)
	{
		if( m_pVideoTask )
		{
			m_pVideoTask->SetSuspend();
			m_pVideoTask->Suspend();
		}

#if 1
		if( m_pAudioTask )
		{
			m_pAudioTask->SetSuspend();
			m_pAudioTask->Suspend();
		}
#endif

#ifdef FOR_IMAGE_LOADING_TASK
		if( m_pImageTask != NULL )
		{
			m_pImageTask->End(1000);
			SAFE_RELEASE(m_pImageTask);
		}
#endif
		if( m_ClipType == CLIPTYPE_IMAGE || m_pImageTrackInfo )
		{
			CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
			if( pVideoRender )
			{
				deregisteImageTrackInfo((CNEXThreadBase*)pVideoRender);
				SAFE_RELEASE(pVideoRender);
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] pausePlayTime delete m_pImageTrackInfo", __LINE__);
			SAFE_RELEASE(m_pImageTrackInfo);
			m_isPlay = FALSE;   
		}
	}
    return true;
}

NXBOOL CClipItem::resumePlayTime(CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	CAutoLock lock(m_Lock);
	if(m_isPlay)
	{
		if( m_pVideoTask )
		{
			m_pVideoTask->setLastEncOutCTS();
			m_pVideoTask->SetActivate();
			m_pVideoTask->Activate();
		}

#if 1
		if( m_pAudioTask )
		{
			m_pAudioTask->SetActivate();
			m_pAudioTask->Activate();
		}
#endif
	}
    return true;
}

void CClipItem::sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2)
{
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		pEditor->notifyEvent(uiEventType, uiParam1, uiParam2);
		SAFE_RELEASE(pEditor);
	}	
}

NXBOOL CClipItem::setPreviewImage(unsigned int uiCurrentTimeStamp, CNEXThreadBase* pVideoRender)
{
	unsigned int uiResultTime = 0;
	CAutoLock lock(m_Lock);

#ifdef __APPLE__
	if(strstr(m_strFilePath, "phasset-image://")) {
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Thie clip is phasset", __LINE__);
	} 
	else 
#endif
	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Image Clip did not exist(%s)", __LINE__, m_strFilePath);
			if( registerMissingImageTrackInfo(pVideoRender, TRUE) )
				return TRUE;
			sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_uiClipID);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);
	}

#ifdef FOR_IMAGE_LOADING_TASK
	if( m_pImageTask == NULL )
	{
		m_pImageTask = new CNEXThread_ImageTask;
		if( m_pImageTask != NULL )
		{
			m_pImageTask->setVideoRenderer((CNEXThread_VideoRenderTask*)pVideoRender);
			m_pImageTask->setClipItem(this);
			m_pImageTask->setSetTimeMode(TRUE);
			m_pImageTask->Begin();
			if( m_pImageTask->waitLoadEvent(1000) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Image Task Wait done", __LINE__, m_uiClipID);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Image Task Wait failed", __LINE__, m_uiClipID);
			}
			return TRUE;
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) Image Task Create Failed(not enough memory)", __LINE__, m_uiClipID);
		}
	}
	return FALSE;
#else
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] setPreviewImage(%d %d %d %p)", __LINE__, m_ClipType, m_uiStartTime, m_uiEndTime, m_pImageTrackInfo);
	return registeImageTrackInfo(pVideoRender, TRUE);
#endif
}

NXBOOL CClipItem::clearImageTask()
{
#ifdef FOR_IMAGE_LOADING_TASK
	if( m_pImageTask != NULL )
	{
		m_pImageTask->End(2000);
		SAFE_RELEASE(m_pImageTask);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) clearImageTask End ", __LINE__, m_uiClipID);
		return TRUE;
	}
#endif
	return TRUE;
}

NXBOOL CClipItem::setPreviewVideo(unsigned int uiCurrentTimeStamp, int iIDRFrame, CNEXThreadBase* pVideoRender)
{
	CNexSource* pSource = NULL;
	unsigned int uiResultTime = 0;
	CAutoLock lock(m_Lock);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] setPreviewVideo In(ID:%d %p time:%d)", __LINE__, getClipID(), m_pSource, uiCurrentTimeStamp);

	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Video Clip did not exist(%s)", __LINE__, m_strFilePath);
			if( registerMissingImageTrackInfo(pVideoRender, TRUE) )
				return TRUE;
			
			sendNotifyEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_uiClipID);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);
	}

	CVideoTrackInfo* pTrackInfo = getCachedVisualTrackInfo(CACHE_TYPE_VIDEO, getClipID());
	if( pTrackInfo == NULL )
	{
		pTrackInfo = new CVideoTrackInfo;
		if( pTrackInfo == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewVideo Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
			return FALSE;
		}
		pTrackInfo->setTrackID(getClipID());
		setCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pTrackInfo);
	}

	if( pTrackInfo->isReaderSeted() == FALSE )
	{
		if( m_pSource ==  NULL )
		{
			if( initSource() != NEXVIDEOEDITOR_ERROR_NONE )
			{
				// Report about file initialize error
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewVideo Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
				SAFE_RELEASE(pTrackInfo);
				return FALSE;
			}
		}
		pSource = m_pSource;
		m_pSource = NULL;

		pSource->setClipID(getClipID());
		pSource->setBaseTimeStamp(m_uiStartTime);
		pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);
		unsigned int uiSeekResult = 0;
		pSource->seekTo(m_uiStartTime, &uiSeekResult);

		if( pTrackInfo->setSourceHandle(this, pSource) == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewVideo Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pTrackInfo);
			SAFE_RELEASE(pTrackInfo);
			SAFE_RELEASE(pSource);
			return FALSE;
		}
		SAFE_RELEASE(pSource);
	}

	CClipVideoRenderInfoVectorIterator i;
	CClipVideoRenderInfo* pRenderInfo;
	pTrackInfo->clearClipRenderInfoVec();
	for(i = m_ClipVideoRenderInfoVec.begin(); i != m_ClipVideoRenderInfoVec.end(); i++)
	{
		pRenderInfo = ((CClipVideoRenderInfo*)(*i));
		pTrackInfo->setClipRenderInfoVec(pRenderInfo);
	}
	updateTrackInfo(pTrackInfo);
	
	/*
	if( m_iSpeedCtlFactor != 100 )
	{
		if(m_iSpeedCtlFactor == 13)
			uiCurrentTimeStamp = (uiCurrentTimeStamp - m_uiStartTime) * 25 / 100 / 2 + m_uiStartTime;
		else
			uiCurrentTimeStamp = (uiCurrentTimeStamp - m_uiStartTime) * m_iSpeedCtlFactor / 100 + m_uiStartTime;
	}
	*/
	
	if( pTrackInfo->setPreviewTime(this, iIDRFrame, uiCurrentTimeStamp) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewVideo Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
		SAFE_RELEASE(pTrackInfo);
		return FALSE;
	}	

	CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(pTrackInfo);
	if( pMsg )
	{
		pVideoRender->SendCommand(pMsg);
		pMsg->Release();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewVideo successed ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
		SAFE_RELEASE(pTrackInfo);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewVideo failed(%p)\n", __LINE__, pTrackInfo);
	SAFE_RELEASE(pTrackInfo);
	return FALSE;
}

NXBOOL CClipItem::unsetPreview()
{
	CAutoLock lock(m_Lock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] unsetPreview(FileReader : %p ImageTrack : %p)", __LINE__, m_pSource, m_pImageTrackInfo);

	if( m_pImageTask != NULL )
	{
		m_pImageTask->End(1000);
		SAFE_RELEASE(m_pImageTask);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) unsetPreview End ", __LINE__, m_uiClipID);
	}

	if( m_pImageTrackInfo  )
	{
		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			deregisteImageTrackInfo((CNEXThreadBase*)pVideoRender);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] unsetPreview(%p)", __LINE__, m_pImageTrackInfo);
			SAFE_RELEASE(pVideoRender);
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] unsetPreview delete m_pImageTrackInfo", __LINE__);
		SAFE_RELEASE(m_pImageTrackInfo);
	}

	m_isPlay = FALSE;
	SAFE_RELEASE(m_pSource);

	/* case, for mantis 9094		
	CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
	if( pVideoRender )
	{
		CNxMsgDeleteTrack* pMsg = new CNxMsgDeleteTrack(getClipID());
		if( pMsg )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Send Msg Video Clip delete Track ID(%d)", __LINE__, getClipID());
			pVideoRender->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
		}
		SAFE_RELEASE(pVideoRender);
	}
	*/
	return TRUE;
}

unsigned char* CClipItem::getPreviewThumbData(unsigned int uiTime)
{
	unsigned char* pData = NULL;
	if(m_pVideoPreviewTask)
	{
		pData = m_pVideoPreviewTask->getPreviewData(uiTime);
	}
	return pData;
}

NXBOOL CClipItem::setPreviewThumb(unsigned int uiCurrentTimeStamp, void* pThumbRender, void* pOutputSurface, CNEXThreadBase* pVideoRender)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;

	if(m_pVideoPreviewTask)
	{
		CVideoTrackInfo* pTrackInfo = getCachedVisualTrackInfo(CACHE_TYPE_VIDEO, getClipID());
		if( pTrackInfo == NULL )
		{
			pTrackInfo = new CVideoTrackInfo;
			if( pTrackInfo == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
				return FALSE;
			}
			pTrackInfo->setTrackID(getClipID());
			setCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pTrackInfo);
		}

		unsigned char* pBuffer = m_pVideoPreviewTask->getPreviewData(uiCurrentTimeStamp);

		if(pBuffer == NULL)
		{
			m_pVideoPreviewTask->setPreviewTime(uiCurrentTimeStamp, TRUE);
			pBuffer = m_pVideoPreviewTask->getPreviewData(uiCurrentTimeStamp, TRUE);
		}

		if(pBuffer == NULL)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail no data\n", __LINE__, getClipID());
			SAFE_RELEASE(pTrackInfo);
			return FALSE;	
		}
		if( pTrackInfo->setPreviewThumb(pBuffer, uiCurrentTimeStamp) == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
			SAFE_RELEASE(pTrackInfo);
			return FALSE;
		}	

		CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(pTrackInfo);
		if( pMsg )
		{
			pVideoRender->SendCommand(pMsg);
			pMsg->Release();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb successed ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
		}
		SAFE_RELEASE(pTrackInfo);
		m_pVideoPreviewTask->setPreviewTime(uiCurrentTimeStamp);
		return TRUE;	
	}
#ifdef FOR_CHECK_HW_RESOURCE_SIZE
	if(CNexCodecManager::getHardwareMemRemainSize() < m_iWidth * m_iHeight)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] video HW not enough resource %d, %d, %d", __LINE__, CNexCodecManager::getHardwareDecoderUseCount(), CNexCodecManager::getHardwareMemRemainSize(), m_iWidth * m_iHeight);
		return FALSE;
	}
#endif

	CVideoTrackInfo* pTrackInfo = getCachedVisualTrackInfo(CACHE_TYPE_VIDEO, getClipID());
	if( pTrackInfo == NULL )
	{
		pTrackInfo = new CVideoTrackInfo;
		if( pTrackInfo == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
			return FALSE;
		}
		pTrackInfo->setTrackID(getClipID());
		setCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pTrackInfo);
	}

	if( pTrackInfo->isReaderSeted() == FALSE )
	{
		CNexSource* pSource = NULL;

		if( m_pSource ==  NULL )
		{
			if( initSource() != NEXVIDEOEDITOR_ERROR_NONE )
			{
				// Report about file initialize error
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
				SAFE_RELEASE(pTrackInfo);
				return FALSE;
			}
		}
		pSource = m_pSource;
		m_pSource = NULL;

		pSource->setClipID(getClipID());
		pSource->setBaseTimeStamp(m_uiStartTime);
		pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);
		unsigned int uiSeekResult = 0;
		pSource->seekTo(m_uiStartTime, &uiSeekResult);

		if( pTrackInfo->setSourceHandle(this, pSource) == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
			clearCachedVisualTrackInfo(CACHE_TYPE_VIDEO, pTrackInfo);
			SAFE_RELEASE(pTrackInfo);
			SAFE_RELEASE(pSource);
			return FALSE;
		}
		SAFE_RELEASE(pSource);
	}

	m_pVideoPreviewTask = new CNEXThread_VideoPreviewTask;
	m_pVideoPreviewTask->setClipItem(this);
	m_pVideoPreviewTask->setPreviewTime(uiCurrentTimeStamp);
	m_pVideoPreviewTask->setPreviewRender(pThumbRender, pOutputSurface);

	if( m_pVideoPreviewTask->setSource(pTrackInfo->getSourceHandle()) == FALSE )
	{
		SAFE_RELEASE(m_pVideoThumbTask);
		SAFE_RELEASE(pTrackInfo);
		return FALSE;
	}
			
	m_pVideoPreviewTask->Begin();

	unsigned char* pBuffer = m_pVideoPreviewTask->getPreviewData(uiCurrentTimeStamp, TRUE);

	if(pBuffer == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail no data\n", __LINE__, getClipID());
		return FALSE;	
	}
	if( pTrackInfo->setPreviewThumb(pBuffer, uiCurrentTimeStamp) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb Fail ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
		SAFE_RELEASE(pTrackInfo);
		return FALSE;
	}	

	CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(pTrackInfo);
	if( pMsg )
	{
		pVideoRender->SendCommand(pMsg);
		pMsg->Release();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb successed ID(%d) RefCnt(%d)\n", __LINE__, getClipID(), pTrackInfo->GetRefCnt());
		SAFE_RELEASE(pTrackInfo);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Clip.cpp %d] setPreviewThumb failed(%p)\n", __LINE__, pTrackInfo);
	SAFE_RELEASE(pTrackInfo);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] setPreviewThumb task start", __LINE__);

	return NEXVIDEOEDITOR_ERROR_NONE;	
}

NXBOOL CClipItem::unsetPreviewThumb()
{
	if(m_pVideoPreviewTask)
	{
		m_pVideoPreviewTask->End(1000);
		SAFE_RELEASE(m_pVideoPreviewTask);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;	
}

NEXVIDEOEDITOR_ERROR CClipItem::checkVideoInfo()
{
	m_pSource->getVideoBitrate((unsigned int*)&m_iVideoBitRate);
	
	if( m_uiTotalTime < CNexVideoEditor::m_iSupportContentDuration )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] checkVideoInfo Not supported Duration(%d)", __LINE__, m_uiTotalTime);
		return NEXVIDEOEDITOR_ERROR_UNSUPPORT_MIN_DURATION;
	}

	if( m_pSource->getVideoResolution((NXUINT32*)&m_iWidth, (NXUINT32*)&m_iHeight) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		m_iWidth		= 0;
		m_iHeight	= 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] checkVideoInfo getVideoResolution failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}

	if( m_pSource->getDisplayVideoResolution((NXUINT32*)&m_iDisplayWidth, (NXUINT32*)&m_iDisplayHeight) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		m_iDisplayWidth	= 0;
		m_iDisplayHeight	= 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] checkVideoInfo getDisplayVideoResolution failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}

	if( (m_iWidth * m_iHeight ) > (CNexVideoEditor::m_iSupportedWidth * CNexVideoEditor::m_iSupportedHeight) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] checkVideoInfo not support video size(%d %d) Support Resolution(%d %d, %d %d)", 
			__LINE__, m_iWidth, m_iHeight, CNexVideoEditor::m_iSupportedWidth, CNexVideoEditor::m_iSupportedHeight, CNexVideoEditor::m_iSupportedMinWidth, CNexVideoEditor::m_iSupportedMinHeight);
		return NEXVIDEOEDITOR_ERROR_UNSUPPORT_MAX_RESOLUTION;
	}

	if( (m_iWidth * m_iHeight ) < (CNexVideoEditor::m_iSupportedMinWidth * CNexVideoEditor::m_iSupportedMinHeight))
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] checkVideoInfo not support video size(%d %d) Support Resolution(%d %d, %d %d)", 
			__LINE__, m_iWidth, m_iHeight, CNexVideoEditor::m_iSupportedWidth, CNexVideoEditor::m_iSupportedHeight, CNexVideoEditor::m_iSupportedMinWidth, CNexVideoEditor::m_iSupportedMinHeight);
		return NEXVIDEOEDITOR_ERROR_UNSUPPORT_MIN_RESOLUTION;
	}

	m_pSource->getH264ProfileLevelID((unsigned int*)&m_iVideoH264Profile);

#ifdef FOR_TEST_MP4V_VIDEO	
	if( m_iVideoH264Profile == 255 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo skip check profile and level for MP4V", __LINE__);
	}
	else if(m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264)
#endif		
	{
		if( m_iVideoH264Profile <= CNexVideoEditor::m_iSupportedMaxProfile )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo supported profile(%d) Supported(%d)", __LINE__, m_iVideoH264Profile, CNexVideoEditor::m_iSupportedMaxProfile);
		}
		else
		{
#ifdef _ANDROID
			// for ZIRA 1873
			if( isMSM8x26Device() && m_iVideoH264Profile <= eNEX_AVC_PROFILE_HIGH && ((m_iWidth * m_iHeight ) <=  (CONTENT_720P_WIDTH*CONTENT_720P_HEIGHT)) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo supported profile(%d) Supported(%d)", __LINE__, m_iVideoH264Profile, CNexVideoEditor::m_iSupportedMaxProfile);
			}
			else
#endif
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo Not supported profile(%d) Supported(%d)", __LINE__, m_iVideoH264Profile, CNexVideoEditor::m_iSupportedMaxProfile);
				return NEXVIDEOEDITOR_ERROR_UNSUPPORT_VIDEO_PROFILE;
			}
		}
		
		m_pSource->getH264Level((unsigned int*)&m_iVideoH264Level);
		if( m_iVideoH264Level <= CNexVideoEditor::m_iSupportedMaxLevel )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo supported Level(%d) Supported(%d)", __LINE__, m_iVideoH264Level, CNexVideoEditor::m_iSupportedMaxLevel);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo Not supported Level(%d) Supported(%d)", __LINE__, m_iVideoH264Level, CNexVideoEditor::m_iSupportedMaxLevel);
			return NEXVIDEOEDITOR_ERROR_UNSUPPORT_VIDEO_LEVEL;
		}
	}

	m_iVideoFPS = m_pSource->getVideoFrameRate();
	m_fVideoFPS = m_pSource->getVideoFrameRateFloat();
	if( m_iVideoFPS <= CNexVideoEditor::m_iSupportedMaxFPS )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo supported FPS(%d) Supported(%d)", __LINE__, m_iVideoFPS, CNexVideoEditor::m_iSupportedMaxFPS);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo Not supported FPS(%d) Supported(%d)", __LINE__, m_iVideoFPS, CNexVideoEditor::m_iSupportedMaxFPS);
		return NEXVIDEOEDITOR_ERROR_UNSUPPORT_VIDEO_FPS;
	}

	if (m_pSource->getVideoH264Interlaced(&m_bH264Interlaced) != NEXVIDEOEDITOR_ERROR_NONE)
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkVideoInfo Error Interlaced(%d)", __LINE__, m_bH264Interlaced);		

	m_iRotateState = m_pSource->getRotateState();
	m_iSeekPointCount = m_pSource->getIDRFrameCount();
	
	//yoon
	getVideoTrackUUID(NULL);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NEXVIDEOEDITOR_ERROR CClipItem::checkAudioInfo()
{
	m_pSource->getAudioBitrate((unsigned int*)&m_iAudioBitRate);
	m_pSource->getSamplingRate((unsigned int*)&m_iAudioSampleRate);
	m_pSource->getNumberOfChannel((unsigned int*)&m_iAudioChannels);
	
	if(	m_pSource->getAudioObjectType() == eNEX_CODEC_A_AAC ||m_pSource->getAudioObjectType() == eNEX_CODEC_A_AACPLUS)	
	{
		int nProfile = 0;
		int iResult= 0;
		unsigned char* pDSI = NULL;
		unsigned int uiSize  = 0;

		if( m_pSource->getAudioFrame() == _SRC_GETFRAME_OK )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[Clip.cpp %d] success to get first audio frame", __LINE__);
			m_pSource->getAudioFrameData( &pDSI, &uiSize );
			getAACProfile(pDSI, uiSize, &nProfile);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] fail to get first audio frame", __LINE__);
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[Clip.cpp %d] checkAudioInfo AAC profile(%d, %d, %d)", __LINE__, nProfile
				, CNexVideoEditor::m_bSupportAACSWCodec, CNexVideoEditor::m_iSupportAACProfile );

		switch( nProfile)
		{
		case 0: // main
			if(	CNexVideoEditor::m_bSupportAACSWCodec == FALSE  && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_MAIN) == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC main", __LINE__);
				return NEXVIDEOEDITOR_ERROR_UNSUPPORT_AUDIO_PROFILE;
			}
			break;
		case 1: // LC
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] support AAC LC", __LINE__);
			break;
		case 2: // SSR
			if(	CNexVideoEditor::m_bSupportAACSWCodec == FALSE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_SSR) == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC SSR", __LINE__);
				return NEXVIDEOEDITOR_ERROR_UNSUPPORT_AUDIO_PROFILE;
			}
			break;
		case 3: // LTP
			if(	CNexVideoEditor::m_bSupportAACSWCodec == FALSE && (CNexVideoEditor::m_iSupportAACProfile & SUPPORT_AAC_LTP) == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC LTP", __LINE__);
				return NEXVIDEOEDITOR_ERROR_UNSUPPORT_AUDIO_PROFILE;
			}
			break;
		default :
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] AAC Unknown ", __LINE__);
			break;
		}
	}		
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::parseClipFile(const char* pFile, NXBOOL bVideoThumbnail, NXBOOL bAudioPCMLevel)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	m_isParseClip = TRUE;	
	eRet = (NEXVIDEOEDITOR_ERROR)setClipPath(pFile);

	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile fail to setClipPath", __LINE__);
		m_isParseClip = FALSE;		
		return eRet;
	}
	
	if(m_isParseClipStop)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		return eRet;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile(%s) option(%d %d)", __LINE__, pFile, bVideoThumbnail, bAudioPCMLevel);

	eRet = (NEXVIDEOEDITOR_ERROR)initSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile.  initSource is failed", __LINE__);
		goto parseFile_Error;
	}

	if(m_isParseClipStop)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}

	// Check Audio AAC.
	if( m_isAudioExist )
	{
		eRet = checkAudioInfo();
		if( eRet != NEXVIDEOEDITOR_ERROR_NONE  )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC profile", __LINE__);
			goto parseFile_Error;
		}
	}

	if( m_isParseClipStop )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}

	if( m_isVideoExist )
	{
		eRet = checkVideoInfo();
		if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not video format(%d)", __LINE__, eRet);
			goto parseFile_Error;
		}		
	}

	if( m_isParseClipStop )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}

	if( bVideoThumbnail || bAudioPCMLevel )
	{
		unsigned int uiStartTick = nexSAL_GetTickCount();
		CThumbnail* thumbnail = new CThumbnail;

		if( m_strThumbnailPath && strlen(m_strThumbnailPath) > 0 && thumbnail )
		{
			if( thumbnail->init(m_strThumbnailPath, m_uiTotalTime, m_isVideoExist, m_isAudioExist) == FALSE )
			{
				delete thumbnail;
				eRet = NEXVIDEOEDITOR_ERROR_THUMBNAIL_INIT_FAIL;
				goto parseFile_Error;
			}
			
			if( m_isVideoExist && bVideoThumbnail )
			{
                if(m_isParseClipStop)
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
                    m_isParseClip = FALSE;    
                    SAFE_DELETE(thumbnail);    
                    goto parseFile_Error;
                }

				m_pVideoThumbTask = new CNEXThread_VideoThumbTask;
				m_pVideoThumbTask->setClipItem(this);
				m_pVideoThumbTask->setThumbnailHandle(thumbnail);

				if( m_pVideoThumbTask->setSource(m_pSource) == FALSE )
				{
					SAFE_DELETE(thumbnail);
					SAFE_RELEASE(m_pVideoThumbTask);
					eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
					goto parseFile_Error;
				}

				m_pVideoThumbTask->Begin();
				m_pVideoThumbTask->WaitTask();

				if( m_pVideoThumbTask->isGetThumbnailFailed() )
				{
					SAFE_RELEASE(m_pVideoThumbTask);
					thumbnail->deinit();
					SAFE_DELETE(thumbnail);
					eRet = NEXVIDEOEDITOR_ERROR_GENERAL;
					uiStartTick = nexSAL_GetTickCount() - uiStartTick;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Video thumbnail (%d)", __LINE__, uiStartTick);
					goto parseFile_Error;
				}

				SAFE_RELEASE(m_pVideoThumbTask);
				thumbnail->deinit();
			}
			
			uiStartTick = nexSAL_GetTickCount() - uiStartTick;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Video thumbnail (%d)", __LINE__, uiStartTick);
			uiStartTick = nexSAL_GetTickCount();

			unsigned int uiSeekResultCTS = 0;
			m_pSource->seekTo(0, &uiSeekResultCTS);
			if( m_isAudioExist && bAudioPCMLevel )
			{
                if(m_isParseClipStop)
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
                    m_isParseClip = FALSE;    
                    SAFE_DELETE(thumbnail);    
                    goto parseFile_Error;
                }

				m_pAudioTask = new CNEXThread_AudioTask;
				m_pAudioTask->setClipItem(this);
				m_pAudioTask->setThumbnailHandle(thumbnail);
				if( m_pAudioTask->setSource(m_pSource) == FALSE )
				{
					SAFE_DELETE(thumbnail);
					SAFE_RELEASE(m_pAudioTask);
					eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
					goto parseFile_Error;
				}
				m_pAudioTask->Begin();
				m_pAudioTask->WaitTask();
				SAFE_RELEASE(m_pAudioTask);
			}
			uiStartTick = nexSAL_GetTickCount() - uiStartTick;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Audio PCM (%d)", __LINE__, uiStartTick);
			thumbnail->deinit();
		}
		SAFE_DELETE(thumbnail);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info Dur(%d) A Exist(%d) V Exist(%d %d %d %d %d) Ret (%d)", __LINE__,
		m_uiTotalTime, m_isAudioExist, m_isVideoExist, m_iWidth, m_iHeight, m_iDisplayWidth, m_iDisplayHeight, eRet);
	
	deinitSource();
	m_isParseClip = FALSE;
    m_isParseClipStop = FALSE;
	return eRet;

parseFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]parseClipFile err(%d)", __LINE__, eRet);
	deinitSource();
	clearClipInfo();
	m_isParseClip = FALSE;
    m_isParseClipStop = FALSE;
	return eRet;
}

int CClipItem::parseClipFile(	const char* pFile, 
							NXBOOL bVideoThumbnail, 
							NXBOOL bAudioPCMLevel, 
							void* pThumbRender, 
							void* pOutputSurface, 
							int iWidth, 
							int iHeight, 
							int iStartTime, 
							int iEndTime, 
							int iCount,
							int iTimeTableSize,
							int* pTimeTable,
							int iFlag,
							int iTag,
							int iUseCount,
							int iSkipCount)
{
	unsigned int uiSeekResultCTS = 0;
	unsigned int uiSeekResultCTS2 = 0;
	unsigned int uiSeekResultCTS3 = 0;
    
	CNexSource*	pSource = NULL;
	CNexSource*	pSource2 = NULL;
	CNexSource*	pSource3 = NULL;    

	unsigned int uiTotal = 0;
	NXBOOL bMultiThread = FALSE;
    
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	m_isParseClip = TRUE;
	eRet = (NEXVIDEOEDITOR_ERROR)setClipPath(pFile);

	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile fail to setClipPath", __LINE__);
		m_isParseClip = FALSE;
		return eRet;
	}

	if(m_isParseClipStop)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		return eRet;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile(%s) option(%d %d)", __LINE__, pFile, bVideoThumbnail, bAudioPCMLevel);

	eRet = (NEXVIDEOEDITOR_ERROR)initSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile.  initSource is failed", __LINE__);
		goto parseFile_Error;
	}

	if(m_isParseClipStop)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	// Check Audio AAC.
	if( m_isAudioExist )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		eRet = checkAudioInfo();
		if( eRet !=  NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC profile", __LINE__);
			goto parseFile_Error;
		}
	}

	if(m_isParseClipStop)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	if( m_isVideoExist )
	{
		eRet = checkVideoInfo();
		if( eRet != NEXVIDEOEDITOR_ERROR_NONE  )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not video format(%d)", __LINE__, eRet);
			goto parseFile_Error;
		}
	}

	if(m_isParseClipStop)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	if( bVideoThumbnail && m_isVideoExist )
	{
#ifdef FOR_CHECK_HW_RESOURCE_SIZE
		if(CNexCodecManager::getHardwareMemRemainSize() < m_iWidth * m_iHeight)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] video HW not enough resource %d, %d, %d", __LINE__, CNexCodecManager::getHardwareDecoderUseCount(), CNexCodecManager::getHardwareMemRemainSize(), m_iWidth * m_iHeight);
			eRet = NEXVIDEOEDITOR_ERROR_HW_NOT_ENOUGH_MEMORY;
			goto parseFile_Error;			
		}
#endif
		
		CThumbnail* thumbnail = new CThumbnail;
		if(  thumbnail != NULL )
		{
			unsigned int uiStartTick = nexSAL_GetTickCount();
			
			if( thumbnail->init(m_strThumbnailPath, THUMBNAIL_MODE_VIDEO, pThumbRender, pOutputSurface, iWidth, iHeight, iCount, iFlag, iTag) == FALSE )
			{
				delete thumbnail;
				eRet = NEXVIDEOEDITOR_ERROR_THUMBNAIL_INIT_FAIL;
				goto parseFile_Error;
			}
			thumbnail->setStartEndTime(iStartTime, iEndTime);

			if( iTimeTableSize > 0 && pTimeTable != NULL )
			{
				thumbnail->setThumbTimeTable(iTimeTableSize, pTimeTable);
			}

            if(m_isParseClipStop)
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
                m_isParseClip = FALSE;    
                SAFE_DELETE(thumbnail);    
                goto parseFile_Error;
            }

			m_pVideoThumbTask = new CNEXThread_VideoThumbTask;
			m_pVideoThumbTask->setClipItem(this);
			m_pVideoThumbTask->setThumbnailHandle(thumbnail);

			if( m_pVideoThumbTask->setSource(m_pSource) == FALSE )
			{
				SAFE_DELETE(thumbnail);
				SAFE_RELEASE(m_pVideoThumbTask);
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				goto parseFile_Error;
			}
			
			m_pVideoThumbTask->Begin();
			m_pVideoThumbTask->WaitTask();

			if( m_pVideoThumbTask->isGetThumbnailFailed() )
			{
				SAFE_RELEASE(m_pVideoThumbTask);
				thumbnail->deinit();
				SAFE_DELETE(thumbnail);
				eRet = NEXVIDEOEDITOR_ERROR_GENERAL;
				uiStartTick = nexSAL_GetTickCount() - uiStartTick;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Video thumbnail (%d)", __LINE__, uiStartTick);
				goto parseFile_Error;				
			}
			
			SAFE_RELEASE(m_pVideoThumbTask);
			thumbnail->deinit();
			
			uiStartTick = nexSAL_GetTickCount() - uiStartTick;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Video thumbnail (%d)", __LINE__, uiStartTick);
		}
		SAFE_DELETE(thumbnail);
	}

	if(m_isParseClipStop)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	//m_pSource->seekTo(0, &uiSeekResultCTS);
	if( bAudioPCMLevel && m_isAudioExist )
	{
		CThumbnail* thumbnail = new CThumbnail;
		if( m_strThumbnailPath && strlen(m_strThumbnailPath) > 0 && thumbnail )
		{
			unsigned int uiStartTick = nexSAL_GetTickCount();

			if( thumbnail->init(m_strThumbnailPath, THUMBNAIL_MODE_AUDIO, (void*)NULL, (void*)NULL) == FALSE )
			{
				delete thumbnail;
				eRet = NEXVIDEOEDITOR_ERROR_THUMBNAIL_INIT_FAIL;
				goto parseFile_Error;
			}

            if(m_isParseClipStop)
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
                m_isParseClip = FALSE;    
                SAFE_DELETE(thumbnail);    
                goto parseFile_Error;
            }

			m_pSource->getTotalPlayTime(&uiTotal);
			if(iEndTime == 0)
			{
				iEndTime = uiTotal;
			}
			m_pSource->setTrimTime(iStartTime, uiTotal - iEndTime);
			m_pSource->seekTo(iStartTime, &uiSeekResultCTS);
            
			if(iEndTime - iStartTime > 15000 && iSkipCount < 50)
			{
				bMultiThread = TRUE;
			}

			if(bMultiThread)
			{
				CNexFileReader* pFileReader = new CNexFileReader;
				if( pFileReader == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexSource.cpp %d] createSource failed FileReader (%p)", __LINE__, pFileReader);
					goto parseFile_Error;
				}		
				pSource = (CNexSource*)pFileReader;

				eRet = (NEXVIDEOEDITOR_ERROR)pSource->createSource();	
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					eRet = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
					goto parseFile_Error;
				}
				pSource->setEncodeToTranscode(CNexProjectManager::getEncodeToTranscode());
				pSource->setTrimTime(iStartTime, uiTotal - iEndTime);
				eRet = (NEXVIDEOEDITOR_ERROR)pSource->openFile(m_strFilePath, (unsigned int)strlen(m_strFilePath));
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
					goto parseFile_Error;
				}
            
				pSource->seekTo((iEndTime-iStartTime)/4 + iStartTime, &uiSeekResultCTS);

				if(m_isParseClipStop)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
					m_isParseClip = FALSE;    
					SAFE_DELETE(thumbnail);    
					goto parseFile_Error;
				}

				pFileReader = new CNexFileReader;
				if( pFileReader == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexSource.cpp %d] createSource failed FileReader (%p)", __LINE__, pFileReader);
					goto parseFile_Error;
				}		
				pSource2 = (CNexSource*)pFileReader;                

				eRet = (NEXVIDEOEDITOR_ERROR)pSource2->createSource();	
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					eRet = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
					goto parseFile_Error;
				}
				pSource2->setEncodeToTranscode(CNexProjectManager::getEncodeToTranscode());
				pSource2->setTrimTime(iStartTime, uiTotal - iEndTime);
				eRet = (NEXVIDEOEDITOR_ERROR)pSource2->openFile(m_strFilePath, (unsigned int)strlen(m_strFilePath));
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
					goto parseFile_Error;
				}
            
				pSource2->seekTo((iEndTime-iStartTime)/4*2 + iStartTime, &uiSeekResultCTS2);

				if(m_isParseClipStop)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
					m_isParseClip = FALSE;    
					SAFE_DELETE(thumbnail);    
					goto parseFile_Error;
				}

				pFileReader = new CNexFileReader;
				if( pFileReader == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexSource.cpp %d] createSource failed FileReader (%p)", __LINE__, pFileReader);
					goto parseFile_Error;
				}		
				pSource3 = (CNexSource*)pFileReader;                

				eRet = (NEXVIDEOEDITOR_ERROR)pSource3->createSource();	
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					eRet = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
					goto parseFile_Error;
				}
				pSource3->setEncodeToTranscode(CNexProjectManager::getEncodeToTranscode());
				pSource3->setTrimTime(iStartTime, uiTotal - iEndTime);
				eRet = (NEXVIDEOEDITOR_ERROR)pSource3->openFile(m_strFilePath, (unsigned int)strlen(m_strFilePath));
				if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
				{
					eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
					goto parseFile_Error;
				}
            
				pSource3->seekTo((iEndTime-iStartTime)/4*3 + iStartTime, &uiSeekResultCTS3);

				if(m_isParseClipStop)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
					m_isParseClip = FALSE;    
					SAFE_DELETE(thumbnail);    
					goto parseFile_Error;
				}
			}

			m_pAudioTask = new CNEXThread_AudioTask;
			m_pAudioTask->setClipItem(this);
			m_pAudioTask->setThumbnailHandle(thumbnail);
			m_pAudioTask->setUseSkipCount(iUseCount, iSkipCount);
			
			if( m_pAudioTask->setSource(m_pSource) == FALSE )
			{
				SAFE_DELETE(thumbnail);
				SAFE_RELEASE(m_pAudioTask);
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				goto parseFile_Error;
			}

			if(bMultiThread)
			{
				m_pAudioTask->setThumbnailMultiThreadTime(uiSeekResultCTS);

				m_pAudioTask2 = new CNEXThread_AudioTask;
				m_pAudioTask2->setClipItem(this);
				m_pAudioTask2->setThumbnailHandle(thumbnail);
				m_pAudioTask2->setThumbnailMultiThreadTime(uiSeekResultCTS2);
				m_pAudioTask2->setUseSkipCount(iUseCount, iSkipCount);
				if( m_pAudioTask2->setSource(pSource) == FALSE )
				{
					SAFE_DELETE(thumbnail);
					SAFE_RELEASE(m_pAudioTask2);
					eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
					goto parseFile_Error;
				}

				m_pAudioTask3 = new CNEXThread_AudioTask;
				m_pAudioTask3->setClipItem(this);
				m_pAudioTask3->setThumbnailHandle(thumbnail);
				m_pAudioTask3->setThumbnailMultiThreadTime(uiSeekResultCTS3);
				m_pAudioTask3->setUseSkipCount(iUseCount, iSkipCount);
		
				if( m_pAudioTask3->setSource(pSource2) == FALSE )
				{
					SAFE_DELETE(thumbnail);
					SAFE_RELEASE(m_pAudioTask3);
					eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
					goto parseFile_Error;
				}

				m_pAudioTask4 = new CNEXThread_AudioTask;
				m_pAudioTask4->setClipItem(this);
				m_pAudioTask4->setThumbnailHandle(thumbnail);
				m_pAudioTask4->setUseSkipCount(iUseCount, iSkipCount);
		
				if( m_pAudioTask4->setSource(pSource3) == FALSE )
				{
					SAFE_DELETE(thumbnail);
					SAFE_RELEASE(m_pAudioTask4);
					eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
					goto parseFile_Error;
				}
			}          
            
			m_pAudioTask->Begin();

			if(bMultiThread)
			{
				m_pAudioTask2->Begin();
				m_pAudioTask3->Begin();            
				m_pAudioTask4->Begin();
//				m_pAudioTask->GetThumbWithMemory(pSource);            
				m_pAudioTask2->WaitTask();
				m_pAudioTask3->WaitTask();            
				m_pAudioTask4->WaitTask();                            
			}
			m_pAudioTask->WaitTask();
			SAFE_RELEASE(m_pAudioTask);
			SAFE_RELEASE(m_pAudioTask2);
			SAFE_RELEASE(m_pAudioTask3);
			SAFE_RELEASE(m_pAudioTask4);
			
			thumbnail->deinit();
			uiStartTick = nexSAL_GetTickCount() - uiStartTick;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Audio PCM (%d)", __LINE__, uiStartTick);
		}
		SAFE_DELETE(thumbnail);
	}	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info Dur(%d) A Exist(%d) V Exist(%d %d %d %d %d) Ret (%d)", __LINE__,
		m_uiTotalTime, m_isAudioExist, m_isVideoExist, m_iWidth, m_iHeight, m_iDisplayWidth, m_iDisplayHeight, eRet);
	
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pSource2);
	SAFE_RELEASE(pSource3);
    
	deinitSource();
	m_isParseClip = FALSE;
    m_isParseClipStop = FALSE;

	return eRet;

parseFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]parseClipFile err(%d)", __LINE__, eRet);
	SAFE_RELEASE(m_pAudioTask);
	SAFE_RELEASE(m_pAudioTask2);
	SAFE_RELEASE(m_pAudioTask3);
	SAFE_RELEASE(m_pAudioTask4);
    
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pSource2);
	SAFE_RELEASE(pSource3);    
	deinitSource();
	clearClipInfo();
	m_isParseClip = FALSE;
    m_isParseClipStop = FALSE;

	return eRet;
}

int CClipItem::parseClipFile(		const char* pFile, 
								NXBOOL bVideoThumbnail, 
								NXBOOL bAudioPCMLevel, 
								int* pTimeTable, 
								int iTimeTableCount, 
								unsigned int uStartTime, 
								unsigned int uEndTime, 
								void* pThumbRender, 
								void* pOutputSurface, 
								int iFlag)
{
	unsigned int uiSeekResultCTS = 0;
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	m_isParseClip = TRUE;	
	eRet = (NEXVIDEOEDITOR_ERROR)setClipPath(pFile);

	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile fail to setClipPath", __LINE__);
		m_isParseClip = FALSE;		
		return eRet;
	}

	if(m_isParseClipStop)
	{
		m_isParseClip = FALSE;		
		return eRet;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile(%s) option(%d %d)", __LINE__, pFile, bVideoThumbnail, bAudioPCMLevel);

	eRet = (NEXVIDEOEDITOR_ERROR)initSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile.  initSource is failed", __LINE__);
		goto parseFile_Error;
	}

	if(m_isParseClipStop)
	{
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	// Check Audio AAC.
	if( m_isAudioExist )
	{
		eRet = checkAudioInfo();
		if( eRet !=  NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC profile", __LINE__);
			goto parseFile_Error;
		}
	}

	if(m_isParseClipStop)
	{
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	if( m_isVideoExist )
	{
		eRet = checkVideoInfo();
		if( eRet != NEXVIDEOEDITOR_ERROR_NONE  )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not video format(%d)", __LINE__, eRet);
			goto parseFile_Error;
		}
	}

	if(m_isParseClipStop)
	{
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] bVideoThumbnail(%d), bAudioPCMLevel(%d)", __LINE__, bVideoThumbnail, bAudioPCMLevel);

	if( bVideoThumbnail && m_isVideoExist )
	{
#ifdef FOR_CHECK_HW_RESOURCE_SIZE
		if(CNexCodecManager::getHardwareMemRemainSize() < m_iWidth * m_iHeight)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] video HW not enough resource %d, %d, %d", __LINE__, CNexCodecManager::getHardwareDecoderUseCount(), CNexCodecManager::getHardwareMemRemainSize(), m_iWidth * m_iHeight);
			eRet = NEXVIDEOEDITOR_ERROR_HW_NOT_ENOUGH_MEMORY;
			goto parseFile_Error;			
		}
#endif

		CThumbnail* thumbnail = new CThumbnail;
		if( thumbnail )
		{
			unsigned int uiStartTick = nexSAL_GetTickCount();
			
			if( thumbnail->init(TRUE, THUMBNAIL_MODE_VIDEO, pThumbRender, pOutputSurface, pTimeTable, iTimeTableCount, iFlag) == FALSE )
			{
				delete thumbnail;
				eRet = NEXVIDEOEDITOR_ERROR_THUMBNAIL_INIT_FAIL;
				goto parseFile_Error;
			}

            if(m_isParseClipStop)
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
                m_isParseClip = FALSE;    
                SAFE_DELETE(thumbnail);    
                goto parseFile_Error;
            }

			m_pVideoThumbTask = new CNEXThread_VideoThumbTask;
			m_pVideoThumbTask->setClipItem(this);
			m_pVideoThumbTask->setThumbnailHandle(thumbnail);

			if( m_pVideoThumbTask->setSource(m_pSource) == FALSE )
			{
				SAFE_DELETE(thumbnail);
				SAFE_RELEASE(m_pVideoThumbTask);
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				goto parseFile_Error;
			}

			m_pVideoThumbTask->Begin();
			m_pVideoThumbTask->WaitTask();

			if( m_pVideoThumbTask->isGetThumbnailFailed() )
			{
				SAFE_RELEASE(m_pVideoThumbTask);
				thumbnail->deinit();
				SAFE_DELETE(thumbnail);
				eRet = NEXVIDEOEDITOR_ERROR_GENERAL;
				uiStartTick = nexSAL_GetTickCount() - uiStartTick;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Video thumbnail (%d)", __LINE__, uiStartTick);
				goto parseFile_Error;
			}

			SAFE_RELEASE(m_pVideoThumbTask);
			thumbnail->deinit();
			uiStartTick = nexSAL_GetTickCount() - uiStartTick;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Video thumbnail (%d)", __LINE__, uiStartTick);
		}
		SAFE_DELETE(thumbnail);
	}

	if(m_isParseClipStop)
	{
		m_isParseClip = FALSE;		
		goto parseFile_Error;
	}
	
	if( bAudioPCMLevel && m_isAudioExist )
	{
		CThumbnail* thumbnail = new CThumbnail;
		thumbnail->setStartEndTime(uStartTime, uEndTime);		
		
		m_pSource->seekTo(uStartTime, &uiSeekResultCTS);	
		
		if( thumbnail )
		{
			unsigned int uiStartTick = nexSAL_GetTickCount();

			if( thumbnail->init(TRUE, THUMBNAIL_MODE_AUDIO, (void*)NULL, (void*)NULL, NULL, 0, iFlag) == FALSE )
			{
				delete thumbnail;
				eRet = NEXVIDEOEDITOR_ERROR_THUMBNAIL_INIT_FAIL;
				goto parseFile_Error;
			}

            if(m_isParseClipStop)
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile stop(%d)", __LINE__, m_isParseClipStop);
                m_isParseClip = FALSE;    
                SAFE_DELETE(thumbnail);    
                goto parseFile_Error;
            }

			m_pAudioTask = new CNEXThread_AudioTask;
			m_pAudioTask->setClipItem(this);
			m_pAudioTask->setThumbnailHandle(thumbnail);
			
			if( m_pAudioTask->setSource(m_pSource) == FALSE )
			{
				SAFE_DELETE(thumbnail);
				SAFE_RELEASE(m_pAudioTask);
				eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				goto parseFile_Error;
			}
			m_pAudioTask->Begin();
			m_pAudioTask->WaitTask();
			SAFE_RELEASE(m_pAudioTask);
			
			thumbnail->deinit();
			uiStartTick = nexSAL_GetTickCount() - uiStartTick;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info process Time Audio PCM (%d)", __LINE__, uiStartTick);
			
		}
		SAFE_DELETE(thumbnail);
	}	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info Dur(%d) A Exist(%d) V Exist(%d %d %d %d %d) Ret (%d)", __LINE__,
		m_uiTotalTime, m_isAudioExist, m_isVideoExist, m_iWidth, m_iHeight, m_iDisplayWidth, m_iDisplayHeight, eRet);
	
	deinitSource();
	m_isParseClip = FALSE;
    m_isParseClipStop = FALSE;

	return eRet;

parseFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]parseClipFile err(%d)", __LINE__, eRet);
	deinitSource();
	clearClipInfo();
	m_isParseClip = FALSE;	
    m_isParseClipStop = FALSE;

	return eRet;
}

int CClipItem::parseClipFile(const char* pFile, int iFlag)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	m_isParseClip = TRUE;	
	eRet = (NEXVIDEOEDITOR_ERROR)setClipPath(pFile);

	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile fail to setClipPath", __LINE__);
		m_isParseClip = FALSE;		
		return eRet;
	}

	eRet = (NEXVIDEOEDITOR_ERROR)initSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFile.  initSource is failed", __LINE__);
		goto parseFile_Error;
	}

	// Check Audio AAC.
	if( m_isAudioExist )
	{
		eRet = checkAudioInfo();
		if( eRet !=  NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC profile", __LINE__);
			goto parseFile_Error;
		}
	}

	if( m_isVideoExist )
	{
		eRet = checkVideoInfo();
		if( eRet !=  NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not video format(%d)", __LINE__, eRet);
			goto parseFile_Error;
		}
	}

	if( (iFlag & GET_CLIPINFO_CHECK_IDR_MODE) ==  GET_CLIPINFO_CHECK_IDR_MODE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info Dur(%d) A Exist(%d) V Exist(%d %d %d) Ret (%d)", __LINE__,
				m_uiTotalTime, m_isAudioExist, m_isVideoExist, m_iWidth, m_iHeight, eRet);		
		m_isParseClip = FALSE;	
		return eRet;
	}

	if( (iFlag & GET_CLIPINFO_INCLUDE_SEEKTABLE) ==  GET_CLIPINFO_INCLUDE_SEEKTABLE )
	{
		int iSeekTableCount = 0;
		unsigned int* pSeekTable = NULL;
		
		if( m_pSource->getSeekTable(0, &iSeekTableCount, &pSeekTable) == NEXVIDEOEDITOR_ERROR_NONE )
		{
			m_iClipSeekTabelVec.clear();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info set Seek table(%d %p)", __LINE__, iSeekTableCount, pSeekTable);
			for(int i = 0; i<iSeekTableCount; i++)
			{
				m_iClipSeekTabelVec.insert(m_iClipSeekTabelVec.end(), pSeekTable[i]);
			}
			nexSAL_MemFree(pSeekTable);
			pSeekTable = NULL;
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info can not get seektable", __LINE__);
			m_iClipSeekTabelVec.clear();
		}
	}

	if( (iFlag & GET_CLIPINFO_CHECK_AUDIO_DEC) ==  GET_CLIPINFO_CHECK_AUDIO_DEC )
	{
		m_pAudioTask = new CNEXThread_AudioTask;
		m_pAudioTask->setClipItem(this);
		if( m_pAudioTask->setSource(m_pSource) == FALSE )
		{
			SAFE_RELEASE(m_pAudioTask);
			eRet = NEXVIDEOEDITOR_ERROR_GENERAL;
			goto parseFile_Error;
		}

		if( m_pAudioTask->checkDecoding() == FALSE )
		{
			SAFE_RELEASE(m_pAudioTask);
			eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip checkDecoding failed", __LINE__);
			goto parseFile_Error;				
		}
		SAFE_RELEASE(m_pAudioTask);
	}

	if( (iFlag & GET_CLIPINFO_CHECK_VIDEO_DEC) ==  GET_CLIPINFO_CHECK_VIDEO_DEC )
	{
		m_pVideoTask = new CNEXThread_VideoTask;
		m_pVideoTask->setClipItem(this);
		if( m_pVideoTask->setSource(m_pSource) == FALSE )
		{
			SAFE_RELEASE(m_pVideoTask);
			eRet = NEXVIDEOEDITOR_ERROR_GENERAL;
			goto parseFile_Error;
		}
		
		if( m_pVideoTask->checkDecoding() == FALSE )
		{
			SAFE_RELEASE(m_pVideoTask);
			eRet = NEXVIDEOEDITOR_ERROR_CODEC_INIT;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip checkDecoding failed", __LINE__);
			goto parseFile_Error;				
		}
		SAFE_RELEASE(m_pVideoTask);
	}	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info Dur(%d) A Exist(%d) V Exist(%d %d %d %d %d) Ret (%d)", __LINE__,
		m_uiTotalTime, m_isAudioExist, m_isVideoExist, m_iWidth, m_iHeight, m_iDisplayWidth, m_iDisplayHeight, eRet);
	
	deinitSource();
	m_isParseClip = FALSE;
	return eRet;

parseFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]parseClipFile err(%d)", __LINE__, eRet);
	deinitSource();
	clearClipInfo();
	m_isParseClip = FALSE;
	return eRet;
}

int CClipItem::parseClipFileWithoutVideo(const char* pFile)
{
	NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
	eRet = (NEXVIDEOEDITOR_ERROR)setClipPath(pFile);

	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFileWithoutVideo fail to setClipPath", __LINE__);
		return eRet;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseClipFileWithoutVideo(%s)", __LINE__, pFile);

	eRet = (NEXVIDEOEDITOR_ERROR)initSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d]  initSource is failed", __LINE__);
		goto parseFile_Error;
	}

	setClipType(CLIPTYPE_AUDIO);
	m_isVideoExist = 0;
	setWidth(0);
	setHeight(0);

	// Check Audio AAC.
	if( m_isAudioExist )
	{
		eRet = checkAudioInfo();
		if( eRet != NEXVIDEOEDITOR_ERROR_NONE  )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] not support AAC profile", __LINE__);
			goto parseFile_Error;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip Parse Info Dur(%d) A Exist(%d) Ret (%d)", __LINE__,
		m_uiTotalTime, m_isAudioExist,  eRet);
	
	deinitSource();
	return eRet;

parseFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]parseClipFileWithoutVideo err(%d)", __LINE__, eRet);
	deinitSource();
	clearClipInfo();
	return eRet;
}

int CClipItem::initSource()
{
	NXUINT32				uTotalPlayTime	= 0;
	NEXVIDEOEDITOR_ERROR	eRet			= NEXVIDEOEDITOR_ERROR_NONE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] initSource In", __LINE__);

	CNexSource* pSource = NULL;
	int iRet = NEXVIDEOEDITOR_ERROR_NONE;

#ifdef __APPLE__	
	int rval = findString((unsigned char *)m_strFilePath,(unsigned int)strlen(m_strFilePath),(unsigned char *)"ipod-library://", strlen("ipod-library://"));
	if(rval >= 0)
	{
		CNexIpodSource* pIpodSource = new CNexIpodSource;
		if( pIpodSource == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexSource.cpp %d] createSource failed OutSource (%p)", __LINE__, pIpodSource);
			return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		}		
		pSource = (CNexSource*)pIpodSource;
	}else
#endif
	if(m_bReverse)
	{
		CNexFileReverse* pFileReverse = new CNexFileReverse;
		if( pFileReverse == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexSource.cpp %d] createSource failed FileReverse (%p)", __LINE__, pFileReverse);
			return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		}		
		pSource = (CNexSource*)pFileReverse;                
	}
	else
	{
		CNexFileReader* pFileReader = new CNexFileReader;
		if( pFileReader == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexSource.cpp %d] createSource failed FileReader (%p)", __LINE__, pFileReader);
			return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		}		
		pSource = (CNexSource*)pFileReader;                
	}

/*      // for pcm dump
	else
	{
		CNexOutSource* pOutSource = new CNexOutSource;
		if( pOutSource == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexSource.cpp %d] createSource failed FileReader (%p)", __LINE__, pOutSource);
			return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		}		
		pSource = (CNexSource*)pOutSource;                
	}
*/

	eRet = (NEXVIDEOEDITOR_ERROR)pSource->createSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		eRet = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		goto parseFile_Error;
	}
	pSource->setEncodeToTranscode(CNexProjectManager::getEncodeToTranscode());
	eRet = (NEXVIDEOEDITOR_ERROR)pSource->openFile(m_strFilePath, (unsigned int)strlen(m_strFilePath));
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
		goto parseFile_Error;
	}

	m_uiTotalAudioTime	= pSource->getTotalPlayAudioTime();
	m_uiTotalVideoTime	= pSource->getTotalPlayVideoTime();

	pSource->getTotalPlayTime(&uTotalPlayTime);
	setTotalTime(uTotalPlayTime);

	m_isVideoExist = pSource->isVideoExist();
	m_isAudioExist = pSource->isAudioExist();

	pSource->getEditBoxTime(&m_uiAudioEditBoxTime, &m_uiVideoEditBoxTime);

	// for changing clip type after play(audio to video)
	if( getClipType() == CLIPTYPE_NONE )
	{
		setClipType(CLIPTYPE_AUDIO);
		// setAudioOnOff(TRUE);
		if( pSource->isVideoExist() )
		{
			NXUINT32 iWidth		= 0;
			NXUINT32 iHeight	= 0;
			if( pSource->getVideoResolution(&iWidth, &iHeight) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
				goto parseFile_Error;
			}
			setWidth(iWidth);
			setHeight(iHeight);

			setClipType(CLIPTYPE_VIDEO);

			m_pStartRect->setRect(0, 0, iWidth, iHeight);
			m_pEndRect->setRect(0, 0, iWidth, iHeight);

			// setAudioOnOff(TRUE);
			// setAudioOnOff(FALSE);
		}		
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] initSource Out(%p)", __LINE__, m_pSource);
	
	pSource->getH264ProfileLevelID((unsigned int*)&m_iVideoH264Profile);
	
	SAFE_RELEASE(m_pSource);
	m_pSource = pSource;
	return eRet;

parseFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] initSource Failed Out(%p)", __LINE__, m_pSource);
	SAFE_RELEASE(pSource);
	// Fixed that clip's information was disappeared porblem on deleted file.
	// clearClipInfo();
	return eRet;
}

int CClipItem::deinitSource()
{
	SAFE_RELEASE(m_pSource);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getAudioClipCountAtTime(unsigned int uiTime)
{
	int iAudioClipCount = 0;
	for(int i = 0; i < m_AudioClipVec.size(); i++)
	{
		if( m_AudioClipVec[i]->isPlayTimeAudio(m_uiStartTime, uiTime, FALSE) )
			iAudioClipCount++;
	}
	return iAudioClipCount;
}

unsigned int CClipItem::getClipIndex()
{
	return m_uiCurrentClipIndex;
}

void CClipItem::setClipIndexInfo(unsigned int uiClipIndex)
{
	m_uiCurrentClipIndex	= uiClipIndex;
}

unsigned int CClipItem::getClipTotalCount()
{
	return m_uiTotalClipCount;
}

void CClipItem::setClipTotalCount(unsigned int uiClipCount)
{
	m_uiTotalClipCount = uiClipCount;
}

int CClipItem::addAudioClip(IClipItem* pClip)
{
	/*
	unsigned int uiTime = pClip->getStartTime();
	int iSameTimeAudioClipCount = 0;

	for(uiTime; uiTime < pClip->getEndTime(); uiTime += 1000 )
	{
		if( getAudioClipCountAtTime(uiTime) >= 3 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio Clip Add failed because video can not add audio clip", __LINE__);
			return NEXVIDEOEDITOR_ERROR_ADD_CLIP_FAIL;
		}
	}
	*/

	CClipItem* pClipItem = (CClipItem*)pClip;
	if( pClipItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Dynamic cast failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	SAFE_ADDREF(pClipItem);
	pClipItem->setVisualClipID(m_uiClipID);
	m_AudioClipVec.insert(m_AudioClipVec.end(), pClipItem);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::deleteAudioClip(unsigned int uiClipID)
{
	CClipItemVectorIterator i;
	for(i = m_AudioClipVec.begin(); i != m_AudioClipVec.end(); i++)
	{
		if( uiClipID == ((CClipItem*)(*i))->getClipID() )
		{
			CClipItem* pItem = (CClipItem*)(*i);
			m_AudioClipVec.erase(i);
			pItem->Release();
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
}

int CClipItem::addSubVideoClip(IClipItem* pClip)
{
	CClipItem* pClipItem = (CClipItem*)pClip;
	if( pClipItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] Dynamic cast failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	SAFE_ADDREF(pClipItem);
	pClipItem->setVisualClipID(m_uiClipID);
	m_SubVideoClipVec.insert(m_SubVideoClipVec.end(), pClipItem);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::deleteSubVideoClip(unsigned int uiClipID)
{
	CClipItemVectorIterator i;
	for(i = m_SubVideoClipVec.begin(); i != m_SubVideoClipVec.end(); i++)
	{
		if( uiClipID == ((CClipItem*)(*i))->getClipID() )
		{
			CClipItem* pItem = (CClipItem*)(*i);
			m_SubVideoClipVec.erase(i);
			pItem->Release();
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
	}
	return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
}

// RYU 20130614.
int CClipItem::setAudioEnvelop(int nSize, unsigned int* puiTime, unsigned int* puiVolume)
{
	if( nSize <= 0 || puiTime == NULL || puiVolume == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) setAudioEnvelop invalid parameter(%d, %p, %p)", __LINE__, m_uiClipID, nSize, puiTime, puiVolume);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	clearAudioEnvelop();

	m_puiEnvelopAudioVolume = (unsigned int*)nexSAL_MemAlloc(sizeof(int)*nSize);
	m_puiEnvelopAudioTime = (unsigned int*)nexSAL_MemAlloc(sizeof(int)*nSize);

	if( m_puiEnvelopAudioVolume == NULL || m_puiEnvelopAudioTime == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) fail setAudioEnvelop Size(%d) ", __LINE__,m_uiClipID,  nSize);
		goto FAIL_TO_INITAUDIOENVELOP;
	}
	m_iEnvelopAudioSize = nSize;

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) setAudioEnvelop Size(%d)", __LINE__,m_uiClipID,  m_iEnvelopAudioSize);
	for( int i = 0 ; i < m_iEnvelopAudioSize ; i++)
	{
		m_puiEnvelopAudioVolume[i] = puiVolume[i];
		m_puiEnvelopAudioTime[i] = puiTime[i];

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) setAudioEnvelop [%d][Time:%d, VOL:%d]", __LINE__, m_uiClipID,  i, m_puiEnvelopAudioTime[i] , m_puiEnvelopAudioVolume[i]);
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) setAudioEnvelop END!", __LINE__,m_uiClipID);
	
	return NEXVIDEOEDITOR_ERROR_NONE;

FAIL_TO_INITAUDIOENVELOP:
	clearAudioEnvelop();
	return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
	
}
int CClipItem::clearAudioEnvelop()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) clearAudioEnvelop!", __LINE__, m_uiClipID);
	
	if( m_puiEnvelopAudioVolume)
	{
		nexSAL_MemFree(m_puiEnvelopAudioVolume);
	}
	m_puiEnvelopAudioVolume = NULL;
	

	if( m_puiEnvelopAudioTime)
	{
		nexSAL_MemFree(m_puiEnvelopAudioTime);
	}
	m_puiEnvelopAudioTime = NULL;
	m_iEnvelopAudioSize = 0;
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getAudioEnvelopSize()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) getAudioEnvelopSize (%d)!", __LINE__, m_uiClipID, m_iEnvelopAudioSize);
	return m_iEnvelopAudioSize;
}
int CClipItem::getAudioEnvelop(int nIndex, unsigned int* puiTime, unsigned int* puiVolume)
{
	if( nIndex < 0 )
		nIndex = 0;

	if( nIndex >= m_iEnvelopAudioSize)
		nIndex = m_iEnvelopAudioSize-1;

#if 0 // to use uiTime
	if( m_iSpeedCtlFactor != 100 )
	{
		if(m_iSpeedCtlFactor == 6)
			*puiTime = m_puiEnvelopAudioTime[nIndex] * 25 / 100 / 4;
		else if(m_iSpeedCtlFactor == 13)
			*puiTime = m_puiEnvelopAudioTime[nIndex] * 25 / 100 / 2;
		else
			*puiTime = m_puiEnvelopAudioTime[nIndex] * m_iSpeedCtlFactor / 100;			
	}
	else
#endif
	{
		*puiTime = m_puiEnvelopAudioTime[nIndex];
	}
	*puiVolume = m_puiEnvelopAudioVolume[nIndex];
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) getAudioEnvelop [%d][TS:%d, VOL:%d]", __LINE__, m_uiClipID, nIndex, *puiTime , *puiVolume);
	return NEXVIDEOEDITOR_ERROR_NONE;
}


int CClipItem::getAudioEnvelop(int* pnSize, unsigned int** ppuiTime, unsigned int** ppuiVolume)
{
	*pnSize = m_iEnvelopAudioSize;
	*ppuiTime = m_puiEnvelopAudioTime;
	*ppuiVolume = m_puiEnvelopAudioVolume;
	return NEXVIDEOEDITOR_ERROR_NONE;
}
int CClipItem::setAudioEnvelopOnOff(int bOnOff)
{
	m_bEnableEnvelopAudio = bOnOff;
	return NEXVIDEOEDITOR_ERROR_NONE;
}
int CClipItem::isAudioEnvelopEnabled()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]Clip(%d) isAudioEnvelopEnabled (%d, %d)!", __LINE__, m_uiClipID, m_iEnvelopAudioSize, m_bEnableEnvelopAudio);
	if(m_iEnvelopAudioSize == 0)
	{
		return FALSE;
	}
	return m_bEnableEnvelopAudio;
}

int CClipItem::setMotionTracked(int bMotionTracked)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip(%d) setMotionTracked (%d)!", __LINE__, m_uiClipID, bMotionTracked);
	m_isMotionTracked = bMotionTracked == 1 ? TRUE : FALSE;

	return 0;
}

NXBOOL CClipItem::getMotionTracked()
{
	return m_isMotionTracked;
}

int CClipItem::setReverse(int bReverse)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip(%d) setReverse (%d)!", __LINE__, m_uiClipID, bReverse);
	m_bReverse = bReverse;

	return 0;
}

int CClipItem::setFreezeDuration(int duration)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiClipID: %d m_iFreezeDuration: %d", __LINE__, m_uiClipID, duration);
	m_iFreezeDuration = duration;
	return 0;
}

int CClipItem::getFreezeDuration()
{
	return m_iFreezeDuration;
}

int CClipItem::setIframePlay(int iframePlay)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiClipID: %d iframePlay: %d", __LINE__, m_uiClipID, iframePlay);
	m_iIframePlay = iframePlay;
	return 0;
}

int CClipItem::getIframePlay()
{
	return m_iIframePlay;
}

void CClipItem::printClipInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] -------------- Clip Info ------------------", __LINE__);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiClipID: %d", __LINE__, m_uiClipID);
	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_strFilePath: %s", __LINE__, m_strFilePath);
	
	const char* pClipType = NULL;
	switch(m_ClipType)
	{
		case CLIPTYPE_IMAGE:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipType : Image", __LINE__);
			break;
		case CLIPTYPE_TEXT:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipType : Text", __LINE__);
			break;
		case CLIPTYPE_AUDIO:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipType : Audio", __LINE__);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiVisualClipID: %d", __LINE__, m_uiVisualClipID);
			break;
		case CLIPTYPE_VIDEO:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipType : Video", __LINE__);
			break;
		case CLIPTYPE_VIDEO_LAYER:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_ClipType : Video Layer", __LINE__);
			break;
		default:
			pClipType = "unKnown";
			break;
	};
	
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiTotalTime(%d) StartTime(%d) EndTime(%d)", __LINE__, m_uiTotalTime, m_uiStartTime, m_uiEndTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiStartTrimTime(%d) m_uiEndTrimTime(%d)", __LINE__, m_uiStartTrimTime, m_uiEndTrimTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iWidth(%d) m_iHeight(%d) m_iDisplayWidth(%d) m_iDisplayHeight(%d)", __LINE__, m_iWidth, m_iHeight, m_iDisplayWidth, m_iDisplayHeight);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_isVideoExist(%d) m_isAudioExist(%d)", __LINE__, m_isVideoExist, m_isAudioExist);

	if( m_pStartRect != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] StartRect(%d %d %d %d)", 
			__LINE__, m_pStartRect->getLeft(), m_pStartRect->getTop(), m_pStartRect->getRight(), m_pStartRect->getBottom());
	}

	if( m_pEndRect != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] EndRect(%d %d %d %d)", 
			__LINE__, m_pEndRect->getLeft(), m_pEndRect->getTop(), m_pEndRect->getRight(), m_pEndRect->getBottom());
	}	

	if( m_pDstRect != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] DstRect(%d %d %d %d)", 
			__LINE__, m_pDstRect->getLeft(), m_pDstRect->getTop(), m_pDstRect->getRight(), m_pDstRect->getBottom());
	}	
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_strEffectTitleID(%s)", __LINE__, m_strEffectTitleID);

	if( m_pTitle != NULL && strlen(m_pTitle) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_pTitle: %s", __LINE__, m_pTitle);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiTitle StartTime(%d) EndTime(%d)", __LINE__, m_uiTitleStartTime, m_uiTitleEndTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iAudioOnOff: %d Vol: %d AutoEnvelop(%d) BGMVol(%d), Pan(L:%d,R:%d)", __LINE__, m_iAudioOnOff, m_iClipVolume, m_iAutoEnvelop, m_iBackGroundVolume, m_iPanFactor[0], m_iPanFactor[1]);
	if( m_iEnvelopAudioSize > 0 && m_puiEnvelopAudioTime != NULL && m_puiEnvelopAudioVolume != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iEnvelopAudioSize(%d) m_bEnableEnvelopAudio(%d)", __LINE__, m_iEnvelopAudioSize, m_bEnableEnvelopAudio);
		for( int i = 0; i < m_iEnvelopAudioSize; i++)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] EnvelopAudio Idx[%d] [Time:%d Vol:%d] ", __LINE__, i, m_puiEnvelopAudioTime[i], m_puiEnvelopAudioVolume[i]);
		}
	}
	
	if( //m_strEffectClipID != NULL &&
	    strlen(m_strEffectClipID) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_strEffectClipID(%s)", __LINE__, m_strEffectClipID);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_bEffect Enable(%d) Dur(%d) Offset(%d) Overlap(%d)", __LINE__, m_bEffectEnable, m_iEffectDuration, m_iEffectOffset, m_iEffectOverlap);
	
	if( //m_strFilterID != NULL && 
	    strlen(m_strFilterID) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_strFilterID(%s)", __LINE__, m_strFilterID);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iRotateState(%d) m_iSpeedCtlFactor(%d)", __LINE__, m_iRotateState, m_iSpeedCtlFactor);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_bKeepPitch(%d)", __LINE__, m_bKeepPitch);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iVoiceChangerFactor(%d)", __LINE__, m_iVoiceChangerFactor);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_pEnhancedAudioFilter(%s)", __LINE__, m_pEnhancedAudioFilter?m_pEnhancedAudioFilter:"NULL");
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_pEqualizer(%s)", __LINE__, m_pEqualizer?m_pEqualizer:"NULL");
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iCompressorFactor(%d)", __LINE__, m_iCompressorFactor);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iPitchIndex(%d)", __LINE__, m_iPitchIndex);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iMusicEffector(%d) m_iProcessorStrength(%d) m_iBassStrength(%d)", __LINE__, m_iMusicEffector, m_iProcessorStrength, m_iBassStrength);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iBrightness(%d) m_iContrast(%d) m_iSaturation(%d) m_iTintcolor(%d) m_iLUT(%d)", __LINE__, m_iBrightness, m_iContrast, m_iSaturation, m_iTintcolor,m_iLUT);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iCustomLUT_A(%d) m_iCustomLUT_B(%d) m_iCustomLUT_Power(%d", __LINE__, m_iCustomLUT_A, m_iCustomLUT_B, m_iCustomLUT_Power);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_iFreezeDuration(%d)", __LINE__, m_iFreezeDuration);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiCurrentClipIndex(%d) m_uiTotalClipCount(%d)", __LINE__, m_uiCurrentClipIndex, m_uiTotalClipCount);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] -------------- Clip Info End -----------------", __LINE__);
}

int CClipItem::getCodecType(NXFF_MEDIA_TYPE eType)
{
	parseDSIofClip();
	int nRet;
	switch(eType)
	{
		case NXFF_MEDIA_TYPE_AUDIO:
			nRet = m_iAudioCodecType;
			break;
		case NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO:
			nRet = m_iVideoCodecType;
			break;
		default :
			nRet = eNEX_CODEC_UNKNOWN;
			break;
	};

	return nRet;
}

int CClipItem::parseDSIofClip()
{
	if(	(isVideoExist() && (m_pVideoDSI == NULL || m_iVideoDSISize <= 0 )) || 
		(isAudioExist() && (m_pAudioDSI == NULL || m_iAudioDSISize <= 0 )))
	{
		NEXVIDEOEDITOR_ERROR eRet = NEXVIDEOEDITOR_ERROR_NONE;
		eRet = (NEXVIDEOEDITOR_ERROR)initSource();
		if( eRet != NEXVIDEOEDITOR_ERROR_NONE || m_pSource == NULL)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseDSIofClip.  initSource is failed", __LINE__);
			return NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
		}
		
		if( m_pSource->isAudioExist() )
		{
			unsigned char*	pDSI		= NULL;
			unsigned int		iDSISize		= 0;
			
			int iDSIRet = m_pSource->getDSI(NXFF_MEDIA_TYPE_AUDIO, &pDSI, &iDSISize);
			if( iDSIRet == NEXVIDEOEDITOR_ERROR_NONE && pDSI != NULL && iDSISize > 0 )
			{
				m_iAudioDSISize = 0;
				m_pAudioDSI = (unsigned char*)nexSAL_MemAlloc(iDSISize);
				if( m_pAudioDSI != NULL )
				{
					memcpy(m_pAudioDSI, pDSI, iDSISize);
					m_iAudioDSISize = iDSISize;
				}
			}
            //yoon
            m_pSource->getNumberOfChannel((unsigned int*)&m_iAudioChannels);
			m_iAudioCodecType = m_pSource->getAudioObjectType();
		}

		if( m_pSource->isVideoExist() )
		{
			unsigned char*	pDSI		= NULL;
			unsigned int		iDSISize		= 0;
			
			int iDSIRet = m_pSource->getDSI(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pDSI, &iDSISize);
			if( iDSIRet == NEXVIDEOEDITOR_ERROR_NONE && pDSI != NULL && iDSISize > 0 )
			{
				m_iVideoDSISize = 0;
				m_pVideoDSI = (unsigned char*)nexSAL_MemAlloc(iDSISize);
				if( m_pVideoDSI != NULL )
				{
					memcpy(m_pVideoDSI, pDSI, iDSISize);
					m_iVideoDSISize = iDSISize;
				}
			}

			m_iVideoCodecType = m_pSource->getVideoObjectType();
			if( m_iVideoCodecType == eNEX_CODEC_V_HEVC )
			{
				if( m_iVideoDSISize > 0 && m_pVideoDSI != NULL )
				{
					NEXCODECUTIL_SPS_INFO sps;
					memset(&sps,0,sizeof(NEXCODECUTIL_SPS_INFO));
					if( NexCodecUtil_HEVC_GetSPSInfo( (NXCHAR*)m_pVideoDSI, m_iVideoDSISize, &sps, NexCodecUtil_CheckByteFormat( m_pVideoDSI, m_iVideoDSISize)) == 0 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseDSIofClip HdrType(%d)", __LINE__, sps.transfer_characteristics);	
						m_iHDRType = sps.transfer_characteristics;
					}	
				}
			}
		}		

		m_pSource->setClipID(getClipID());
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);

		unsigned int uiResultTime = 0;

		m_pSource->seekToVideoWithAbsoluteTime(m_uiStartTrimTime, &uiResultTime, NXFF_RA_MODE_CUR_PREV);
		if( m_uiStartTrimTime - uiResultTime < 5 )
		{
			if( m_pSource->getVideoFrame(FALSE, TRUE) == _SRC_GETFRAME_OK )
			{
				unsigned char* pFrame = NULL;
				unsigned int uiFrameSize = 0;
				m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
				int iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();
				if( (m_iVideoCodecType == eNEX_CODEC_V_HEVC) && (m_iHDRType != 0) )
				{
					NXINT32 len = uiFrameSize;
					NXUINT8 * pTarget = pFrame; 
					int ismalloc = 0;
					if( NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW == NexCodecUtil_CheckByteFormat((unsigned char*)pFrame, uiFrameSize) )
					{	
						ismalloc = 1;
						pTarget = (NXUINT8*)malloc(uiFrameSize+100);
						len = NexCodecUtil_ConvertFormat
							( 	pTarget
								, uiFrameSize+80
								, pFrame
								, uiFrameSize
								, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW
								, iFrameNALHeaderSize );
					}
					
					if( NexCodecUtil_HEVC_SEI_GetHDRInfo( pTarget, len, iFrameNALHeaderSize, NULL , &m_stHDRInfo ) == 0 )
					{
						if( m_stHDRInfo.MDCV_prsent_flag )
						{	
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] parseDSIofClip Hdr mastering display start ", __LINE__);
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] G(X:%f,Y:%f) , B(X:%f,Y:%f) , R(X:%f,Y:%f) , W(X:%f,Y:%f) Max Lum:%f, Min Lum:%f", __LINE__
								,m_stHDRInfo.stMDCVinfo.display_primaries[0][0]
								,m_stHDRInfo.stMDCVinfo.display_primaries[0][1]
								,m_stHDRInfo.stMDCVinfo.display_primaries[1][0]
								,m_stHDRInfo.stMDCVinfo.display_primaries[1][1]
								,m_stHDRInfo.stMDCVinfo.display_primaries[2][0]
								,m_stHDRInfo.stMDCVinfo.display_primaries[2][1]
								,m_stHDRInfo.stMDCVinfo.white_point[0]
								,m_stHDRInfo.stMDCVinfo.white_point[1]
								,m_stHDRInfo.stMDCVinfo.max_display_mastering_luminance
								,m_stHDRInfo.stMDCVinfo.min_display_mastering_luminance
							);
						}
						if( m_stHDRInfo.CLL_present_flag )
						{	
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VFrame.cpp %d] parseDSIofClip Hdr CLL ( max cll=%d, max pall=%d)", __LINE__
								,m_stHDRInfo.stCLLinfo.max_content_light_level
								,m_stHDRInfo.stCLLinfo.max_pic_average_light_level
							);
						}
					}	
					if( ismalloc )
						free(pTarget);
				}
				if( NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), TRUE) )
				{
					m_isStartIDRFrame = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] parseDSIofClip. start with IDR Frame", __LINE__);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] parseDSIofClip. start with Not IDR Frame", __LINE__);
					m_isStartIDRFrame = FALSE;
				}
			}
		}

		m_iContentOrientation = m_pSource->getReaderRotateState();

		m_iAudioTrackCount = m_pSource->getAudioTrackCount();
		m_iVideoTrackCount = m_pSource->getVideoTrackCount();;
		

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] parseDSIofClip. initSource and SeekResult (%d %d) startIDR(%d) (%d) (%d %d)",
			__LINE__, m_uiStartTime, uiResultTime, m_isStartIDRFrame, m_iContentOrientation, m_iAudioTrackCount, m_iVideoTrackCount);
		
		deinitSource();

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] getDSIofClip parse clip end", __LINE__);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::getDSIofClip(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize )
{
	if( ppDSI == NULL || pDSISize == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] getDSIofClip parameter failed(%d %p %d)", __LINE__, eType, ppDSI, pDSISize);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	parseDSIofClip();

	switch( eType )
	{
	case NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO :
		*ppDSI = m_pVideoDSI;
		*pDSISize = m_iVideoDSISize;
		break;
	case NXFF_MEDIA_TYPE_AUDIO :
		*ppDSI = m_pAudioDSI;
		*pDSISize = m_iAudioDSISize;
		break;
	default:
		*ppDSI = NULL;
		*pDSISize = 0;
		break;
	};
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CClipItem::compareAudioDSIofClip(CClipItem* pClip)
{
	if( pClip == NULL )
		return FALSE;
	
	parseDSIofClip();
	pClip->parseDSIofClip();

	unsigned char* pDSI = NULL;
	unsigned int iDSISize = 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareAudioDSIofClip (%d %d)", __LINE__, m_uiClipID, pClip->getClipID());

	int iRet = pClip->getDSIofClip(NXFF_MEDIA_TYPE_AUDIO, &pDSI, &iDSISize);

	if( iRet == NEXVIDEOEDITOR_ERROR_NONE && pDSI != NULL && iDSISize > 0 )
	{
		nexSAL_MemDump(m_pAudioDSI, m_iAudioDSISize);
		nexSAL_MemDump(pDSI, iDSISize);

		if( m_iAudioDSISize != iDSISize )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareAudioDSIofClip DSI length unmatch(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
			return FALSE;
		}

		if( memcmp(m_pAudioDSI, pDSI, iDSISize) == 0  )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareAudioDSIofClip DSI match(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
			return TRUE;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareAudioDSIofClip DSI unmatch(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI unmatch(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
	return FALSE;	
}

NXBOOL CClipItem::compareVideoDSIofClip(CClipItem* pClip)
{
	if( pClip == NULL )
		return FALSE;
	
	parseDSIofClip();
	pClip->parseDSIofClip();

	unsigned char* pDSI = NULL;
	unsigned int iDSISize = 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip (%d %d)", __LINE__, m_uiClipID, pClip->getClipID());

    //yoon for LG
    if( CNexVideoEditor::m_bForceDirectExport )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] compareVideoDSIofClip  ForceDirectExport is true", __LINE__);
        return TRUE;
    }

	int iFirstVideoCodecType = getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO);
	int iSecondVideoCodecType = pClip->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO);

	if(iFirstVideoCodecType != iSecondVideoCodecType)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] compareVideoDSIofClip Check codec OTI(0x%x, 0x%x)", __LINE__, iFirstVideoCodecType, iSecondVideoCodecType);
		return FALSE;
	}

#if 0
	if((iFirstVideoCodecType != eNEX_CODEC_V_H264 && iFirstVideoCodecType != eNEX_CODEC_V_HEVC) || (iSecondVideoCodecType != eNEX_CODEC_V_H264 && iSecondVideoCodecType != eNEX_CODEC_V_HEVC) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[ClipList.cpp %d] compareVideoDSIofClip Check codec OTI(0x%x, 0x%x)", __LINE__, getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO), pClip->getCodecType(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO));
		return FALSE;
	}
#endif

	if( m_iVideoH264Profile !=  pClip->getVideoH264Profile() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip codec unmatch(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
		return FALSE;
	}
		
	int iRet = pClip->getDSIofClip(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pDSI, &iDSISize);

	if( iRet == NEXVIDEOEDITOR_ERROR_NONE && pDSI != NULL && iDSISize > 0 )
	{
		nexSAL_MemDump(m_pVideoDSI, m_iVideoDSISize);
		nexSAL_MemDump(pDSI, iDSISize);
		if( iFirstVideoCodecType == eNEX_CODEC_V_MPEG4V ) 
		{
			if( NexCodecUtil_MPEG4V_IsDirectMixable(m_pVideoDSI, m_iVideoDSISize, pDSI, iDSISize) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI match(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
				return TRUE;
			}
		}
		else if(iFirstVideoCodecType == eNEX_CODEC_V_H264)
		{
			if( NexCodecUtil_AVC_IsDirectMixable(m_pVideoDSI, m_iVideoDSISize, pDSI, iDSISize) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI match(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
				return TRUE;
			}
		}
		else if(iFirstVideoCodecType == eNEX_CODEC_V_HEVC)
		{
			if( NexCodecUtil_HEVC_IsDirectMixable(m_pVideoDSI, m_iVideoDSISize, pDSI, iDSISize) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI match(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
				return TRUE;
			}
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI unmatch(%d %d)", __LINE__, m_uiClipID, pClip->getClipID());
	return FALSE;
}

int CClipItem::compareVideoDSIWithEncoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) compareVideoDSIWithEncoder", __LINE__, m_uiClipID);
	
	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			return FALSE;
		}
		nexSAL_FileClose(handleTest);		
	}
	
	if( m_pSource == NULL )
	{
		int iRet = initSource();
		if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] compareVideoDSIWithEncoder init source failed", __LINE__);
			return FALSE;
		}

		if( m_pSource->isVideoExist()  )
		{
			if( m_pVideoDSI == NULL )
			{
				unsigned char*	pDSI		= NULL;
				unsigned int		iDSISize		= 0;

				int iDSIRet = m_pSource->getDSI(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pDSI, &iDSISize);
				if( iDSIRet == NEXVIDEOEDITOR_ERROR_NONE && pDSI != NULL && iDSISize > 0 )
				{
					m_iVideoDSISize = 0;
					m_pVideoDSI = (unsigned char*)nexSAL_MemAlloc(iDSISize);
					if( m_pVideoDSI != NULL )
					{
						memcpy(m_pVideoDSI, pDSI, iDSISize);
						m_iVideoDSISize = iDSISize;
					}
				}

				m_iVideoCodecType = m_pSource->getVideoObjectType();
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] compareVideoDSIWithEncoder video not exist", __LINE__);
			return FALSE;
		}

		unsigned int uiResultTime = 0;
		m_pSource->setClipID(getClipID());
		m_pSource->setBaseTimeStamp(m_uiStartTime);
		m_pSource->setTrimTime(m_uiStartTrimTime, m_uiEndTrimTime);
		m_pSource->seekTo(0, &uiResultTime, NXFF_RA_MODE_CUR_PREV);
	}

#ifdef _ANDROID
	SAFE_RELEASE(m_pPFrameTask);
	if( m_pSource != NULL && m_pSource->isVideoExist() )
	{
		m_pPFrameTask = new CNEXThread_PFrameProcessTask;
		if( m_pPFrameTask )
		{
			m_pPFrameTask->setSource(m_pSource);
			m_pPFrameTask->setClipItem(this);
			m_pPFrameTask->setCheckEncoderDSIMode(TRUE);
			
			m_pPFrameTask->Begin();
			m_pPFrameTask->WaitTask();

			int iEncoderDSISize = m_pPFrameTask->getEncoderDSISize();
			unsigned char* pEncoderDSI = m_pPFrameTask->getEncoderDSI();
			if( pEncoderDSI == NULL ||  iEncoderDSISize <= 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] compareVideoDSIWithEncoder get encoder dsi failed(%p, %d)", __LINE__, pEncoderDSI, iEncoderDSISize );
				SAFE_RELEASE(m_pPFrameTask);
				return FALSE;
			}

			nexSAL_MemDump(m_pVideoDSI, m_iVideoDSISize);
			nexSAL_MemDump(pEncoderDSI, iEncoderDSISize);

            int isMixable = 0;
			if(m_iVideoCodecType == eNEX_CODEC_V_H264)
			{
			    isMixable = NexCodecUtil_AVC_IsDirectMixable(m_pVideoDSI, m_iVideoDSISize, pEncoderDSI, iEncoderDSISize);
				if( isMixable == NEXCODECUTIL_DIRECT_MIXABLE || isMixable == NEXCODECUTIL_DIRECT_MIXABLE_ONLY_DECODING )
				{
					SAFE_RELEASE(m_pPFrameTask);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI match with encoder", __LINE__, m_uiClipID);
					return TRUE;
				}
			}
			else if(m_iVideoCodecType == eNEX_CODEC_V_HEVC)
			{
			    isMixable = NexCodecUtil_HEVC_IsDirectMixable(m_pVideoDSI, m_iVideoDSISize, pEncoderDSI, iEncoderDSISize);
				if( isMixable )
				{
					SAFE_RELEASE(m_pPFrameTask);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI match with encoder", __LINE__, m_uiClipID);
					return TRUE;
				}
			}
			else if(m_iVideoCodecType == eNEX_CODEC_V_MPEG4V)
			{
			    isMixable = NexCodecUtil_MPEG4V_IsDirectMixable(m_pVideoDSI, m_iVideoDSISize, pEncoderDSI, iEncoderDSISize);
				if( isMixable )
				{
					SAFE_RELEASE(m_pPFrameTask);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] compareVideoDSIofClip DSI match with encoder", __LINE__, m_uiClipID);
					return TRUE;
				}
			}
			
			SAFE_RELEASE(m_pPFrameTask);
		}
	}
	deinitSource();
#endif
	return FALSE;
}

NXBOOL CClipItem::isStartIDRFrame()
{
	return m_isStartIDRFrame;
}

NXBOOL CClipItem::setStartIDRFrame(NXBOOL bFlag)
{
	m_isStartIDRFrame = bFlag;
	return TRUE;
}

int CClipItem::getContentOrientation()
{
	return m_iContentOrientation;
}

int CClipItem::getAudioTrackCount()
{
	return m_iAudioTrackCount;
}

int CClipItem::getVideoTrackCount()
{
	return m_iVideoTrackCount;
}

void CClipItem::printClipTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  ----------- Clip Time Info -----------", __LINE__);

	if( m_ClipType == CLIPTYPE_IMAGE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip ID : %d Image clip", __LINE__, m_uiClipID);
	}
	else if( m_ClipType == CLIPTYPE_VIDEO )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip ID : %d Video clip", __LINE__, m_uiClipID);
	}
	else if( m_ClipType == CLIPTYPE_VIDEO_LAYER )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip ID : %d Video layer clip", __LINE__, m_uiClipID);
	}
	else if( m_ClipType == CLIPTYPE_AUDIO )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip ID : %d Audio clip", __LINE__, m_uiClipID);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip ID : %d Type(%d)", __LINE__, m_uiClipID, m_ClipType);
	}
	
	if( m_ClipType == CLIPTYPE_IMAGE || m_ClipType == CLIPTYPE_VIDEO || m_ClipType == CLIPTYPE_VIDEO_LAYER )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio OnOff(%d) Volume(%d) BGM(%d) Pan(L:%d,R:%d)", __LINE__, m_iAudioOnOff, m_iClipVolume, m_iBackGroundVolume, m_iPanFactor[0], m_iPanFactor[1]);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Start End Total Time(%d %d %d)", __LINE__, m_uiStartTime, m_uiEndTime, m_uiTotalTime);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] StartTrim EndTrim Time (%d %d)", __LINE__, m_uiStartTrimTime, m_uiEndTrimTime);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] EffectInfo(%s Dur(%d) Offset(%d) Overlap(%d)), Title(%s, Stime(%u) Etime(%u)", 
														__LINE__, 
														m_strEffectClipID, 
														m_iEffectDuration,
														m_iEffectOffset,
														m_iEffectOverlap,
														//m_strEffectTitleID == NULL ? "NULL":
														m_strEffectTitleID,
														m_uiTitleStartTime,
														m_uiTitleEndTime
														);

														
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Clip ID : %d", __LINE__, m_uiClipID);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Parent Clip ID : %d", __LINE__, m_uiVisualClipID);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Audio OnOff(%d) Volume(%d) BGM(%d) Pan(L:%d,R:%d)", __LINE__, m_iAudioOnOff, m_iClipVolume, m_iBackGroundVolume, m_iPanFactor[0], m_iPanFactor[1]);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] Start End Total Time(%d %d %d)", __LINE__, m_uiStartTime, m_uiEndTime, m_uiTotalTime);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] StartTrim EndTrim Time (%d %d)", __LINE__, m_uiStartTrimTime, m_uiEndTrimTime);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ----------- Clip Time Info -----------", __LINE__);
}

int CClipItem::parseClipStop()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] parseClipStop %d, %p, %p, %p", __LINE__, m_isParseClip, m_pAudioTask, m_pVideoTask, m_pVideoThumbTask);
	m_isParseClipStop = TRUE;
	while(m_isParseClip)
	{
		if(m_pAudioTask == NULL && m_pAudioTask2 == NULL && m_pAudioTask3 == NULL && m_pAudioTask4 == NULL && m_pVideoTask == NULL && m_pVideoThumbTask == NULL)
			break;

		if(m_pAudioTask)
		{
			m_pAudioTask->setStopThumb();
			if(m_pAudioTask2)
			{
				m_pAudioTask2->setStopThumb();			
			}
			if(m_pAudioTask3)
			{
				m_pAudioTask3->setStopThumb();			
			}
			if(m_pAudioTask4)
			{
				m_pAudioTask4->setStopThumb();			
			}
			m_isParseClipStop	= FALSE;
			break;
		}

		if(m_pVideoThumbTask )
		{
			m_pVideoThumbTask->setStopThumb();
			m_isParseClipStop	= FALSE;
			break;
		}

		nexSAL_TaskSleep(5);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] parseClipStop End", __LINE__);

	return 0;
}

int CClipItem::checkResourceSize(unsigned int uiTime)
{
	if( uiTime < m_uiStartTime || uiTime > m_uiEndTime )
		return 0;

	if( m_ClipType == CLIPTYPE_VIDEO || m_ClipType == CLIPTYPE_VIDEO_LAYER )
	{

		CVideoTrackInfo* pTrackInfo = getCachedVisualTrackInfo(CACHE_TYPE_VIDEO, getClipID());
		if( pTrackInfo != NULL )
		{
			SAFE_RELEASE(pTrackInfo);
			return 0;
		}
		return m_iWidth * m_iHeight;
	}

	return 0;
}

int CClipItem::findIDRFrameTime(unsigned int uiTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] findIDRFrameTime In(%p %d)", __LINE__, m_pSource, uiTime);
	if( m_pSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] findIDRFrameTime failed(%p %d)", __LINE__, m_pSource, uiTime);
		return -1;
	}

	unsigned char* 	pFrame			= NULL;
	unsigned int 		uiFrameSize		= 0;
	unsigned int 		uiResultTime	= 0;
	int 				iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();

	// Fixed for some content with wrong stss box
	int time = uiTime == 0 ? 1 : uiTime;

	while( time > 0 )
	{
		m_pSource->seekToVideoWithAbsoluteTime(time, &uiResultTime, NXFF_RA_MODE_PREV); // NXFF_RA_MODE_CUR_PREV
		if( m_pSource->getVideoFrame(FALSE, TRUE) == _SRC_GETFRAME_OK )
		{
			m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
			if( NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), TRUE) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] findIDRFrameTime End(%d %d)", __LINE__, time, uiResultTime);
				return uiResultTime + 1;
			}
			else
			{
				time = uiResultTime - 10;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] findIDRFrameTime failed retry(%d %d)", __LINE__, time, uiResultTime);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] findIDRFrameTime failed because read frame failed", __LINE__);
			break;
		}
	}

	return -1;
}

//yoon
unsigned char * CClipItem::getVideoTrackUUID(int * piSize)
{
    unsigned char* puuid;
    int size = 0;
    if( m_iVideoTrackUUIDSize < 0 )
    {
        if( m_pSource == NULL )
        {
        	int iRet = initSource();
    		if( iRet != NEXVIDEOEDITOR_ERROR_NONE )
    		{
    			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] getVideoTrackUUID init reader failed", __LINE__);
    			if( piSize != NULL )
                    *piSize = 0;
    			return NULL;
    		}
    		m_pSource->getVideoTrackUUID(&puuid,&size);
    		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] getVideoTrackUUID size(%d)", __LINE__,size);    
    		m_iVideoTrackUUIDSize = size;
    		deinitSource();
        }
        else
        {
	        m_pSource->getVideoTrackUUID(&puuid,&size);
	        m_iVideoTrackUUIDSize = size;
        }
    
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[Clip.cpp %d] getVideoTrackUUID size(%d)", __LINE__,m_iVideoTrackUUIDSize);
        
        if( m_pVideoTrackUUID != NULL ){
            nexSAL_MemFree(m_pVideoTrackUUID);
        }
        
        if(puuid == NULL)
        {
            m_pVideoTrackUUID = NULL;
            m_iVideoTrackUUIDSize = 0;
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] getVideoTrackUUID NULL", __LINE__);
        }
        else    
        {
            m_pVideoTrackUUID = (unsigned char *)nexSAL_MemAlloc(m_iVideoTrackUUIDSize+1);
            if( m_pVideoTrackUUID == NULL )
            {
                m_iVideoTrackUUIDSize = -1;
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] getVideoTrackUUID memalloc fail!", __LINE__);   
            }
            else
            {
                memcpy(m_pVideoTrackUUID,puuid,m_iVideoTrackUUIDSize);
                m_pVideoTrackUUID[m_iVideoTrackUUIDSize] = 0;
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] getVideoTrackUUID uuid(%s)", __LINE__,m_pVideoTrackUUID);  
                //nexSAL_MemDump(m_pVideoTrackUUID,m_iVideoTrackUUIDSize);
            }
        }
    
    }

    if( piSize != NULL )
        *piSize = m_iVideoTrackUUIDSize;

    return m_pVideoTrackUUID;
}

int CClipItem::getEditBoxTIme(unsigned int* piAudioEditBoxTime, unsigned int* piVideoEditBoxTime)
{
	*piAudioEditBoxTime = m_uiAudioEditBoxTime;
	*piVideoEditBoxTime = m_uiVideoEditBoxTime;	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipItem::setVideoDecodingEnd(int iEnd)
{
	m_iVideoDecodeEnd = iEnd;
	return true;
}

int CClipItem::getVideoDecodingEnd()
{
	return m_iVideoDecodeEnd;
}

CNexDrawInfoVec* CClipItem::getDrawInfos()
{
	SAFE_ADDREF(m_pDrawInfos);
	return m_pDrawInfos;
}

void CClipItem::setDrawInfos(CNexDrawInfoVec* pInfos)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) setDrawInfo(%p %p)", __LINE__, m_uiClipID, m_pDrawInfos, pInfos);
	SAFE_RELEASE(m_pDrawInfos);
	m_pDrawInfos = pInfos;
	SAFE_ADDREF(m_pDrawInfos);
}

void CClipItem::updateDrawInfo(CNexDrawInfo* pInfo)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) updateDrawInfo(%p %p)", __LINE__, m_uiClipID, m_pDrawInfos, pInfo);

	m_pDrawInfos->updateDrawInfo(pInfo);
}

CNexDrawInfo* CClipItem::getDrawInfo(int id)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] ID(%d) getDrawInfo(%d)", __LINE__, id);

	return m_pDrawInfos->getDrawInfo(id);
}


int CClipItem::isIDRFrame(unsigned int uiTime)//yoon
{

	unsigned int uiResultTime = 0;
    int ret = 0;
 	if( m_pSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] isIDRFrame failed(%d)", __LINE__, uiTime);
		return 0;
	}

	m_pSource->seekToVideoWithAbsoluteTime(uiTime, &uiResultTime, NXFF_RA_MODE_CUR_PREV);
    if( uiTime - uiResultTime < 5 )
    {
    	if( m_pSource->getVideoFrame(FALSE, TRUE) == _SRC_GETFRAME_OK )
    	{
    		unsigned char* pFrame = NULL;
    		unsigned int uiFrameSize = 0;
    		m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
    		int iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();
    		if( NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), TRUE) )
    		{
    			ret = 1;
    		}
    		else
    		{
    			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Clip.cpp %d] isIDRFrame. cts(%u) with Not IDR Frame", __LINE__,uiTime);
    		}
    	}
    }
	return ret;
}

NXBOOL CClipItem::updateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face){

	CAutoLock lock(m_Lock);

	if(clipid == getClipID()){

		m_iFaceDetectProcessed = 1;
		m_uiFaceDetected = face_detected;

		IRectangle*	prect = NULL;
		prect = this->getStartPosition();
		if(prect){

			prect->setRect(start.left, start.top, start.right, start.bottom);
			SAFE_RELEASE(prect);
		}

		prect = this->getEndPosition();
		if(prect){

			prect->setRect(end.left, end.top, end.right, end.bottom);
			SAFE_RELEASE(prect);
		}

		prect = this->getFacePosition();
		if(prect){

			prect->setRect(face.left, face.top, face.right, face.bottom);
			SAFE_RELEASE(prect);
		}
	}

	for(CClipVideoRenderInfoVectorIterator itor = m_ClipVideoRenderInfoVec.begin(); itor != m_ClipVideoRenderInfoVec.end(); ++itor){

		CClipVideoRenderInfo* prenderinfo = *itor;
		if(prenderinfo->muiTrackID == clipid){

	    CNexProjectManager* pProject = CNexProjectManager::getProjectManager();
	    unsigned int time = 0;
	    if(pProject){
				time = pProject->getCurrentTimeStamp();
			}

			if((time <= prenderinfo->mStartTime) || (pProject->getState() != PLAY_STATE_RUN) || (prenderinfo->mStartTime <= 0))
			{

				prenderinfo->mFaceDetectProcessed = 1;
				prenderinfo->mStart = start;
				prenderinfo->mEnd = end;
				prenderinfo->mFace = face;
				prenderinfo->mFaceDetected = face_detected;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] uiCurrentTime:%d updateRenderInfo prenderinfo->mStartTime:%d prenderinfo->mFaceDetectProcessed:%d", prenderinfo->muiTrackID, time, prenderinfo->mStartTime, prenderinfo->mFaceDetectProcessed);
			}
			else{

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] uiCurrentTime:%d updateRenderInfo fail prenderinfo->mStartTime:%d prenderinfo->mFaceDetectProcessed:%d", prenderinfo->muiTrackID, time, prenderinfo->mStartTime, prenderinfo->mFaceDetectProcessed);
			}

			SAFE_RELEASE(pProject);

			return TRUE;
		}
	}

	return FALSE;
}

NXBOOL CClipItem::resetFaceDetectInfo(unsigned int clipid){

	CAutoLock lock(m_Lock);

	if(clipid == getClipID()){

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] resetFaceDetectInfo", clipid);
		m_iFaceDetectProcessed = 0;
	}

	for(CClipVideoRenderInfoVectorIterator itor = m_ClipVideoRenderInfoVec.begin(); itor != m_ClipVideoRenderInfoVec.end(); ++itor){

		CClipVideoRenderInfo* prenderinfo = *itor;
		if(prenderinfo->muiTrackID == clipid){

			prenderinfo->mFaceDetectProcessed = 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] resetFaceDetectInfo prenderinfo->mStartTime:%d prenderinfo->mFaceDetectProcessed:%d", prenderinfo->muiTrackID, prenderinfo->mStartTime, prenderinfo->mFaceDetectProcessed);

			return TRUE;
		}
	}

	return FALSE;
}

bool CClipItem::checkFaceDetectProcessedAtTime(unsigned int uiTime){

	CAutoLock lock(m_Lock);

	if( m_pDrawInfos != NULL )
	{
		if( m_ClipType != CLIPTYPE_IMAGE )
			return true;

		return m_pDrawInfos->checkFaceDetectDoneAtTime(uiTime);
	}

	for(CClipVideoRenderInfoVectorIterator itor = m_ClipVideoRenderInfoVec.begin(); itor != m_ClipVideoRenderInfoVec.end(); ++itor){

		CClipVideoRenderInfo* prenderinfo = *itor;
		if(prenderinfo->mClipType != CLIPTYPE_IMAGE)
			continue;

		if(uiTime < prenderinfo->mStartTime || uiTime > prenderinfo->mEndTime)
			continue;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] checkFaceDetectProcessedAtTime(%d) prenderinfo->mStartTime:%d prenderinfo->mFaceDetectProcessed:%d", prenderinfo->muiTrackID, uiTime, prenderinfo->mStartTime, prenderinfo->mFaceDetectProcessed);

		if(prenderinfo->mFaceDetectProcessed)
			return true;
		return false;
	}

	return true;
}

CClipAudioRenderInfo* CClipItem::getActiveAudioRenderInfo(unsigned int uiTime, NXBOOL bByReader)
{
	if(m_ClipAudioRenderInfoVec.size() == 1)
	{
		return m_ClipAudioRenderInfoVec[0];
	}

	if(bByReader)
	{	
		CCalcTime		calcTime;                   
		for(int i = 0; i < m_ClipAudioRenderInfoVec.size(); i++)
		{
			int iAudioUITime = (int)calcTime.applySpeed(m_ClipAudioRenderInfoVec[i]->mStartTime, m_ClipAudioRenderInfoVec[i]->mStartTrimTime, m_ClipAudioRenderInfoVec[i]->m_iSpeedCtlFactor, uiTime);

			if( m_ClipAudioRenderInfoVec[i]->mStartTime <= iAudioUITime && m_ClipAudioRenderInfoVec[i]->mEndTime >= iAudioUITime )
			{
				return m_ClipAudioRenderInfoVec[i];
			}
		}
	}
	else
	{
		for(int i = 0; i < m_ClipAudioRenderInfoVec.size(); i++)
		{
			if( m_ClipAudioRenderInfoVec[i]->mStartTime <= uiTime && m_ClipAudioRenderInfoVec[i]->mEndTime >= uiTime )
			{
				return m_ClipAudioRenderInfoVec[i];
			}
		}
	}		
	return NULL;
}

CClipVideoRenderInfo* CClipItem::getActiveVideoRenderInfo(unsigned int uiTime, NXBOOL bByReader, int * pIndex)
{
	if(m_ClipVideoRenderInfoVec.size() == 1)
	{
		return m_ClipVideoRenderInfoVec[0];
	}

	if(bByReader)
	{	
		CCalcTime		calcTime;
		int 			freezeTime = 0;
		for(int i = 0; i < m_ClipVideoRenderInfoVec.size(); i++)
		{
			int iVideoUITime = (int)calcTime.applySpeed(m_ClipVideoRenderInfoVec[i]->mStartTime, m_ClipVideoRenderInfoVec[i]->mStartTrimTime, m_ClipVideoRenderInfoVec[i]->m_iSpeedCtlFactor, uiTime);

			if( m_ClipVideoRenderInfoVec[i]->mFreezeDuration != 0 )
				iVideoUITime += m_ClipVideoRenderInfoVec[i]->mFreezeDuration;

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) getActiveVideoRenderInfo(%d %d %d %d %d) %d %d %d", __LINE__, m_ClipVideoRenderInfoVec[i]->muiTrackID, m_ClipVideoRenderInfoVec[i]->mStartTime, m_ClipVideoRenderInfoVec[i]->mEndTime, m_ClipVideoRenderInfoVec[i]->mStartTrimTime, m_ClipVideoRenderInfoVec[i]->mEndTrimTime, m_ClipVideoRenderInfoVec[i]->m_iSpeedCtlFactor, uiTime, iVideoUITime, m_ClipVideoRenderInfoVec[i]->mFreezeDuration);

			if( m_ClipVideoRenderInfoVec[i]->mStartTime <= iVideoUITime && m_ClipVideoRenderInfoVec[i]->mEndTime >= iVideoUITime )
			{
				if(pIndex != NULL)
					*pIndex = i;
				return m_ClipVideoRenderInfoVec[i];
			}
		}
	}
	else
	{
		for(int i = 0; i < m_ClipVideoRenderInfoVec.size(); i++)
		{
			if( m_ClipVideoRenderInfoVec[i]->mStartTime <= uiTime && m_ClipVideoRenderInfoVec[i]->mEndTime >= uiTime )
			{
				if(pIndex != NULL)
					*pIndex = i;                    
				return m_ClipVideoRenderInfoVec[i];
			}
		}
	}		
	return NULL;
}

//---------------------------------------------------------------------------
CClipInfo::CClipInfo()
{
	m_existVideo	= FALSE;
	m_existAudio	= FALSE;
	m_isH264Interlaced = FALSE;
	m_iAudioCodecType = 0;
	m_iVideoCodecType = 0;
	m_iWidth		= 0;
	m_iHeight	= 0;
	m_iDisplayWidth	= 0;
	m_iDisplayHeight	= 0;
	m_iFPS		= 0;
	m_iH264Profile	=0;
	m_iH264Level		= 0;
	m_iRotateState	= 0;

	m_iVideoBitRate	= 0;
	m_iAudioBitRate	= 0;

	m_iAudioSampleRate	= 0;
	m_iAudioChannels		= 0;	
	
	m_uiAudioDuration	= 0;
	m_uiVideoDuration	= 0;
	m_iSeekPointCount = 0;
	m_strThumbnailPath = NULL;
	m_VideoType = 0;
    m_strUUID = NULL;
    m_iUUIDSize = 0;
    m_iHDRType = 0; //yoon
	m_uiAudioEditBoxTime = 0;
	m_uiVideoEditBoxTime = 0;	
	m_iClipSeekTabelVec.clear();
}

CClipInfo::~CClipInfo()
{
	if( m_strThumbnailPath )
	{
		nexSAL_MemFree(m_strThumbnailPath);
		m_strThumbnailPath = NULL;
	}
	
	if( m_strUUID )
	{
	    nexSAL_MemFree(m_strUUID);
	    m_iUUIDSize = 0;
	}    
	m_iClipSeekTabelVec.clear();
}

int CClipInfo::existVideo()
{
	return m_existVideo;
}

int CClipInfo::setExistVideo(NXBOOL isExist)
{
	m_existVideo = isExist;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::existAudio()
{
	return m_existAudio;
}

int CClipInfo::setExistAudio(NXBOOL isExist)
{
	m_existAudio = isExist;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setAudioCodecType(int iAudioCodecType)
{
	m_iAudioCodecType = iAudioCodecType;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::getAudioCodecType()
{
	return m_iAudioCodecType;
}

int CClipInfo::setVideoCodecType(int iVideoCodecType)
{
	m_iVideoCodecType = iVideoCodecType;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::getVideoCodecType()
{
	return m_iVideoCodecType;
}

int CClipInfo::getWidth()
{
	return m_iWidth;
}

int CClipInfo::getHeight()
{
	return m_iHeight;
}

int CClipInfo::getDisplayWidth()
{
	return m_iDisplayWidth;
}

int CClipInfo::getDisplayHeight()
{
	return m_iDisplayHeight;
}

int CClipInfo::getVideoFPS()
{
	return m_iFPS;
}

int CClipInfo::setVideoFPS(int iFPS)
{
	m_iFPS = iFPS;
    return TRUE;
}

float CClipInfo::getVideoFPSFloat()
{
	return m_fFPS;
}

int CClipInfo::setVideoFPSFloat(float fFPS)
{
	m_fFPS = fFPS;
    return TRUE;
}

int CClipInfo::getVideoH264Interlaced()
{
	return m_isH264Interlaced;
}

int CClipInfo::setVideoH264Interlaced(NXBOOL isInterlaced)
{
	m_isH264Interlaced = isInterlaced;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::getVideoH264Profile()
{
	return m_iH264Profile;
}

int CClipInfo::setVideoH264Profile(int iProfile)
{
	m_iH264Profile = iProfile;
    return TRUE;
}

int CClipInfo::getVideoH264Level()
{
	return m_iH264Level;
}

int CClipInfo::getVideoOrientation()
{
	return m_iRotateState;
}

int CClipInfo::setVideoH264Level(int iLevel)
{
	m_iH264Level = iLevel;
    return TRUE;
}

int CClipInfo::setVideoOrientation(int iOrientation)
{
	m_iRotateState = iOrientation;
    return TRUE;
}

int CClipInfo::getVideoBitRate()
{
	return m_iVideoBitRate;
}

int CClipInfo::setVideoBitRate(int iVideoBitRate)
{
	m_iVideoBitRate = iVideoBitRate;
    return TRUE;
}

int CClipInfo::getAudioBitRate()
{
	return m_iAudioBitRate;
}

int CClipInfo::setAudioBitRate(int iAudioBitRate)
{
	m_iAudioBitRate = iAudioBitRate;
    return TRUE;
}

int CClipInfo::getAudioSampleRate()
{
	return m_iAudioSampleRate;
}

int CClipInfo::setAudioSampleRate(int iSampleRate)
{
	m_iAudioSampleRate = iSampleRate;
    return TRUE;
}

int CClipInfo::getAudioChannels()
{
	return m_iAudioChannels;
}

int CClipInfo::setAudioChannels(int iChannels)
{
	m_iAudioChannels = iChannels;
    return TRUE;
}

unsigned int CClipInfo::getClipAudioDuration()
{
	return m_uiAudioDuration;
}

unsigned int CClipInfo::getClipVideoDuration()
{
	return m_uiVideoDuration;
}

const char* CClipInfo::getThumbnailPath()
{
	if( m_strThumbnailPath == NULL )
		return "";
	
	return (const char*)m_strThumbnailPath;
}

int CClipInfo::getSeekPointCount()
{
	return m_iSeekPointCount;
}

int CClipInfo::getSeekTableCount()
{
	return m_iClipSeekTabelVec.size();
}

int CClipInfo::getSeekTableValue(int iIndex)
{
	return m_iClipSeekTabelVec[iIndex];
}

int CClipInfo::setThumbnailPath(const char* pThumbnailPath)
{
	if( pThumbnailPath == NULL )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	if( m_strThumbnailPath )
	{
		nexSAL_MemFree( m_strThumbnailPath );
		m_strThumbnailPath = NULL;
	}

	m_strThumbnailPath = (char*)nexSAL_MemAlloc(strlen(pThumbnailPath) + 1);
	strcpy(m_strThumbnailPath, pThumbnailPath);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setWidth(int iWidth)
{
	m_iWidth = iWidth;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setHeight(int iHeight)
{
	m_iHeight = iHeight;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setDisplayWidth(int iDisplayWidth)
{
	m_iDisplayWidth = iDisplayWidth;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setDisplayHeight(int iDisplayHeight)
{
	m_iDisplayHeight = iDisplayHeight;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setClipAudioDuration(unsigned int uiDuration)
{
	m_uiAudioDuration = uiDuration;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setClipVideoDuration(unsigned int uiDuration)
{
	m_uiVideoDuration = uiDuration;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setSeekPointCount(int iSeekPointCount)
{
	m_iSeekPointCount = iSeekPointCount;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::setSeekTable(int iSeekTableCount, int* pSeekTable)
{
	m_iClipSeekTabelVec.clear();
	if( iSeekTableCount <= 0 || pSeekTable == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	for( int i = 0; i < iSeekTableCount; i++)
	{
		m_iClipSeekTabelVec.insert(m_iClipSeekTabelVec.end(), pSeekTable[i]);
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CClipInfo::addSeekTableItem(int iValue)
{
	m_iClipSeekTabelVec.insert(m_iClipSeekTabelVec.end(), iValue);
	return NEXVIDEOEDITOR_ERROR_NONE;	
}

int CClipInfo::setVideoRenderType(int type)
{
	m_VideoType = type;
	return NEXVIDEOEDITOR_ERROR_NONE;
}    

int CClipInfo::getVideoRenderType()
{
	return m_VideoType;	
}

int CClipInfo::setVideoHDRType(int type) //yoon
{
	m_iHDRType = type;
	return NEXVIDEOEDITOR_ERROR_NONE;
}    

int CClipInfo::getVideoHDRType() //yoon
{
	return m_iHDRType;	
}

int CClipInfo::setVideoUUID(const char* pUUID , int size )
{
	if( pUUID == NULL || size == 0 )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	if( m_strUUID )
	{
		nexSAL_MemFree( m_strUUID );
		m_strUUID = NULL;
		m_iUUIDSize = 0;
	}
    
	m_strUUID = (char*)nexSAL_MemAlloc(size + 1);
	if( m_strUUID == NULL )
	{
	    return NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
	}
	m_iUUIDSize = size+1;
	strncpy(m_strUUID, pUUID,size);
	m_strUUID[size] = 0x00;
	return NEXVIDEOEDITOR_ERROR_NONE;
}    

const char * CClipInfo::getVideoUUID(int *size)
{
    *size = m_iUUIDSize;
	return m_strUUID;	
}

int CClipInfo::setEditBoxTime(unsigned int uiAudioEditBoxTime, unsigned int uiVideoEditBoxTime )
{
	m_uiAudioEditBoxTime = uiAudioEditBoxTime;
	m_uiVideoEditBoxTime = uiVideoEditBoxTime;	
	return NEXVIDEOEDITOR_ERROR_NONE;
}    

int CClipInfo::getEditBoxTIme(unsigned int* piAudioEditBoxTime, unsigned int* piVideoEditBoxTime)
{
	*piAudioEditBoxTime = m_uiAudioEditBoxTime;
	*piVideoEditBoxTime = m_uiVideoEditBoxTime;	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

void CClipInfo::printClipInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ------------- Clip Info --------------", __LINE__);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiAidoDuration : %d", __LINE__, m_uiAudioDuration);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiVideoDuration : %d", __LINE__, m_uiVideoDuration);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiAidoEditBoxTime : %d", __LINE__, m_uiAudioEditBoxTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_uiVideoEditBoxTIme : %d", __LINE__, m_uiVideoEditBoxTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_existVideo : %d", __LINE__, m_existVideo);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iVideoCodecType : %d", __LINE__, m_iVideoCodecType);	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iWidth : %d", __LINE__, m_iWidth);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iHeight : %d", __LINE__, m_iHeight);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iDisplayWidth : %d", __LINE__, m_iDisplayWidth);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iDisplayHeight : %d", __LINE__, m_iDisplayHeight);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iFPS : %d(%f)", __LINE__, m_iFPS, m_fFPS);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iH264Profile : %d", __LINE__, m_iH264Profile);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iH264Level : %d", __LINE__, m_iH264Level);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iOrientation : %d", __LINE__, m_iRotateState);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iSeekPointCount : %d", __LINE__, m_iSeekPointCount);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_existAudio : %d", __LINE__, m_existAudio);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] m_isH264Interlaced : %d", __LINE__, m_isH264Interlaced);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_iAudioCodecType : %d", __LINE__, m_iAudioCodecType);		
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d]  m_VideoType : %d (uuid size=%d)", __LINE__, m_VideoType,m_iUUIDSize);		
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ------------- Clip Info --------------", __LINE__);
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/
