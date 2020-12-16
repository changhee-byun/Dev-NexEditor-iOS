/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_LAYER.cpp
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
#include "NEXVIDEOEDITOR_Layer.h"
#include "NexSAL_Internal.h"
#include "NEXVIDEOEDITOR_Def.h"

#include "NEXVIDEOEDITOR_AudioTask.h"
#include "NEXVIDEOEDITOR_VideoTask.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_Thumbnail.h"
#include "NEXVIDEOEDITOR_FileMissing.h"
#include "NEXVIDEOEDITOR_Util.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"

#include "stdio.h"
#include "string.h"

//---------------------------------------------------------------------------
CLayerItem::CLayerItem()
{
	m_uiClipID			= INVALID_CLIP_ID;

	m_pFFReader			= NULL;
	m_pVideoTask		= NULL;
	m_pAudioTask		= NULL;

	m_pTrackInfo			= NULL;

	m_strDisplayOption	= NULL;
	m_strFilePath			= NULL;

	clearLayerInfo();
}

CLayerItem::CLayerItem(unsigned int uiClipID)
{
	m_uiClipID = uiClipID;

	m_pFFReader			= NULL;
	m_pVideoTask		= NULL;
	m_pAudioTask		= NULL;

	m_pTrackInfo			= NULL;

	m_strDisplayOption	= NULL;
	m_strFilePath			= NULL;

	clearLayerInfo();
}

CLayerItem::~CLayerItem()
{
	if( m_pFFReader )
	{
		m_pFFReader->deleteSource();
	}
 
	SAFE_RELEASE(m_pFFReader);
	SAFE_RELEASE(m_pVideoTask);
	SAFE_RELEASE(m_pAudioTask);
	SAFE_RELEASE(m_pTrackInfo);

	if( m_strDisplayOption )
	{
		nexSAL_MemFree(m_strDisplayOption);
		m_strDisplayOption = NULL;
	}
	
	if( m_strFilePath )
	{
		nexSAL_MemFree(m_strFilePath);
		m_strFilePath = NULL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ~~~~~~~~~~~CLayerItem", __LINE__);
}

void CLayerItem::clearLayerInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] clearClipInfo", __LINE__);
	
	m_ClipType			= CLIPTYPE_NONE;
	m_uiTotalTime		= 0;
	m_uiStartTime		= 0;
	m_uiEndTime			= 0;

	m_uiStartTrimTime		= 0;
	m_uiEndTrimTime		= 0;

	m_iWidth				= 0;
	m_iHeight			= 0;

	m_isVideoExist		= 0;

	m_iRotateState		= 0;

	m_iBrightness			= 0;
	m_iContrast			= 0;
	m_iSaturation			= 0;
    m_iHue              = 0;
	m_iTintcolor			= 0;
	
	m_iSpeedCtlFactor		= 100;
 
 	strcpy(m_strEffectID, "");
	strcpy(m_strFilterID, "");

	if( m_strDisplayOption )
	{
		nexSAL_MemFree(m_strDisplayOption);
		m_strDisplayOption = NULL;
	}
	
	if( m_strFilePath )
	{
		nexSAL_MemFree(m_strFilePath);
		m_strFilePath = NULL;
	}

	if( m_pFFReader )
	{
		m_pFFReader->deleteSource();
	}	

	SAFE_RELEASE(m_pFFReader);
	SAFE_RELEASE(m_pAudioTask);
	SAFE_RELEASE(m_pVideoTask);
	
	SAFE_RELEASE(m_pTrackInfo);

	m_isPlay				= FALSE;	
  }

int CLayerItem::getID()
{
	return m_uiClipID;
}

int CLayerItem::setID(unsigned int id)
{
	m_uiClipID = id;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

CLIP_TYPE CLayerItem::getType()
{
	return m_ClipType;
}

int CLayerItem::setType(CLIP_TYPE eType)
{
	m_ClipType = eType;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CLayerItem::getTotalTime()
{
	return m_uiTotalTime;
}

int CLayerItem::setTotalTime(unsigned int uiTotalTime)
{
	m_uiTotalTime = uiTotalTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CLayerItem::getStartTime()
{
	return m_uiStartTime;
}

int CLayerItem::setStartTime(unsigned int uiStartTime)
{
	m_uiStartTime = uiStartTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CLayerItem::getEndTime()
{
	return m_uiEndTime;
}

int CLayerItem::setEndTime(unsigned int uiEndTime)
{
	m_uiEndTime = uiEndTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CLayerItem::getStartTrimTime()
{
	return m_uiStartTrimTime;
}

int CLayerItem::setStartTrimTime(unsigned int uiStartTime)
{
	m_uiStartTrimTime = uiStartTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CLayerItem::getEndTrimTime()
{
	return m_uiEndTrimTime;
}

int CLayerItem::setEndTrimTime(unsigned int uiEndTime)
{
	m_uiEndTrimTime = uiEndTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getWidth()
{
	return m_iWidth;
}

int CLayerItem::setWidth(int iWidth)
{
	m_iWidth = iWidth;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getHeight()
{
	return m_iHeight;
}

int CLayerItem::setHeight(int iHeight)
{
	m_iHeight = iHeight;
	return NEXVIDEOEDITOR_ERROR_NONE;
}


int CLayerItem::isVideoExist()
{
	return m_isVideoExist;
}

int CLayerItem::setVideoExist(NXBOOL isExist)
{
	m_isVideoExist = isExist;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getBrightness()
{
	return m_iBrightness;
}

int CLayerItem::setBrightness(int iBrightness)
{
	m_iBrightness = iBrightness;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getContrast()
{
	return m_iContrast;
}

int CLayerItem::setContrast(int iContrast)
{
	m_iContrast = iContrast;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getSaturation()
{
	return m_iSaturation;
}

int CLayerItem::setSaturation(int iSaturation)
{
	m_iSaturation = iSaturation;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getHue(){

    return m_iHue;
}

int CLayerItem::setHue(int iHue){

    m_iHue = iHue;
    return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getTintcolor()
{
	return m_iTintcolor;
}

int CLayerItem::setTintcolor(int iTintcolor)
{
	m_iTintcolor = iTintcolor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getLUT()
{
	return m_iLUT;
}

int CLayerItem::setLUT(int iLUT)
{
	m_iLUT = iLUT;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getVignette()
{
	return m_iVignette;
}

int CLayerItem::setVignette(int iVignette)
{
	m_iVignette = iVignette;
	return NEXVIDEOEDITOR_ERROR_NONE;
}


int CLayerItem::getRotateState()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ClipID(%d) getRotateState(%d)", __LINE__, m_uiClipID, m_iRotateState);
	return m_iRotateState;
}

int CLayerItem::setRotateState(int iRotate)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ClipID(%d) setRotateState(pre:%d cur:%d)", __LINE__, m_uiClipID, m_iRotateState, iRotate);
	m_iRotateState = iRotate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::setSpeedCtlFactor(int iFactor)
{
	//if( iFactor < 13 || iFactor > 400 )
	if( iFactor < 3 || iFactor > 400 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ClipID(%d) setSpeedCtlFactor failed(%d) ", __LINE__, m_uiClipID, iFactor);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ClipID(%d) setSpeedCtlFactor (%d) ", __LINE__, m_uiClipID, iFactor);
	m_iSpeedCtlFactor = iFactor;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CLayerItem::getSpeedCtlFactor()
{
	return m_iSpeedCtlFactor;
}

const char* CLayerItem::getDisplayOption()
{
	if( m_strDisplayOption == NULL )
		return "";
	return m_strDisplayOption;
}

int CLayerItem::setDisplayOption(const char* pDisplayOption)
{
	if( pDisplayOption == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( m_strDisplayOption )
	{
		nexSAL_MemFree(m_strDisplayOption);
		m_strDisplayOption = NULL;
	}

	m_strDisplayOption = (char*)nexSAL_MemAlloc(strlen(pDisplayOption)+1);
	strcpy(m_strDisplayOption, pDisplayOption);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

const char* CLayerItem::getPath()
{
	if( m_strFilePath == NULL )
		return "";
	return m_strFilePath;
}

int CLayerItem::setPath(const char* pPath)
{
	if( pPath == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( m_strFilePath )
	{
		nexSAL_MemFree(m_strFilePath);
		m_strFilePath = NULL;
	}

	m_strFilePath = (char*)nexSAL_MemAlloc(strlen(pPath)+1);
	strcpy(m_strFilePath, pPath);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CLayerItem::isPlayTime(unsigned int uiCurrentTime, NXBOOL bPrepare)
{
	unsigned int uiStart = m_uiStartTime;
	if( bPrepare )
	{
		uiStart = uiStart > 4000 ? uiStart - 4000 : 0;
	}

	if( uiCurrentTime >= uiStart  && uiCurrentTime < m_uiEndTime )
		return TRUE;

	return FALSE;	
}

int CLayerItem::initFileReader()
{
	NXUINT32				uTotalPlayTime	= 0;
	NEXVIDEOEDITOR_ERROR	eRet			= NEXVIDEOEDITOR_ERROR_NONE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) initFileReader In", __LINE__, m_uiClipID);

	CNexFileReader* pFileReader = new CNexFileReader;
	if( pFileReader == NULL )
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;

	eRet = (NEXVIDEOEDITOR_ERROR)pFileReader->createSource();
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		eRet = NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
		goto parseFile_Error;
	}
	eRet = (NEXVIDEOEDITOR_ERROR)pFileReader->openFile(m_strFilePath, (unsigned int)strlen(m_strFilePath));
	if( eRet != NEXVIDEOEDITOR_ERROR_NONE )
	{
		// eRet = NEXVIDEOEDITOR_ERROR_FILEREADER_CREATE_FAIL;
		goto parseFile_Error;
	}

	// m_uiTotalAudioTime	= pFileReader->getTotalPlayAudioTime();
	// m_uiTotalVideoTime	= pFileReader->getTotalPlayVideoTime();

	pFileReader->getTotalPlayTime(&uTotalPlayTime);
	setTotalTime(uTotalPlayTime);

	m_isVideoExist = pFileReader->isVideoExist();
	// m_isAudioExist = pFileReader->isAudioExist();

	// for changing clip type after play(audio to video)
	/*
	if( getClipType() == CLIPTYPE_NONE )
	{
		setClipType(CLIPTYPE_AUDIO);
		// setAudioOnOff(TRUE);
		if( pFileReader->isVideoExist() )
		{
			NXUINT32 iWidth		= 0;
			NXUINT32 iHeight	= 0;
			if( pFileReader->getVideoResolution(&iWidth, &iHeight) != NEXVIDEOEDITOR_ERROR_NONE )
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

	*/

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) initFileReader Out(%p)", __LINE__, m_uiClipID, m_pFFReader);
	
	m_pFFReader = pFileReader;
	return eRet;

parseFile_Error:
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) initFileReader Failed Out(%p)", __LINE__, m_uiClipID, m_pFFReader);
	SAFE_RELEASE(pFileReader);
	// ������ ���� ũ�� ������ �������� ������ ������.
	// clearClipInfo();
	return eRet;
}

int CLayerItem::deinitFileReader()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) deinitFileReader Out(%p)", __LINE__, m_uiClipID, m_pFFReader);
	SAFE_RELEASE(m_pFFReader);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CLayerItem::registeLayerTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo In(%p)", __LINE__, m_uiClipID, m_pTrackInfo);
	deregisteLayerTrackInfo(pVideoRender);

	m_pTrackInfo = new CVideoTrackInfo;
	
	if( m_pTrackInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo create failed(%p)", __LINE__, m_uiClipID, m_pTrackInfo);
		return FALSE;
	}

	CClipVideoRenderInfo renderInfo;
	//memset(&renderInfo, 0x00, sizeof(CClipVideoRenderInfo));

	renderInfo.mFaceDetectProcessed = 1;

	m_pTrackInfo->m_uiTrackID = m_uiClipID;

	renderInfo.muiTrackID		= m_uiClipID;
	renderInfo.mClipType		= m_ClipType;

	renderInfo.mWidth			= m_iWidth;
	renderInfo.mHeight		= m_iHeight;
	renderInfo.mPitch			= m_iWidth;
	
	renderInfo.mStartTime		= m_uiStartTime;
	renderInfo.mEndTime		= m_uiEndTime;

	renderInfo.mRotateState	= m_iRotateState;
	renderInfo.mBrightness		= m_iBrightness;
	renderInfo.mContrast		= m_iContrast;
	renderInfo.mSaturation		= m_iSaturation;
    renderInfo.mHue		        = m_iHue;
	renderInfo.mTintcolor		= m_iTintcolor;
	renderInfo.mLUT 			= m_iLUT;
	renderInfo.mVignette        = m_iVignette;

	CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
	if( pVideoEditor == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo getVideoEditor handle fail", __LINE__, m_uiClipID);
		SAFE_RELEASE(m_pTrackInfo);
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
				SAFE_RELEASE(m_pTrackInfo);
				return FALSE;
			}
			
			renderInfo.mWidth			= iWidth;
			renderInfo.mHeight		= iHeight;
			renderInfo.mPitch			= iPitch;

			pImageBuffer = (unsigned char*)nexSAL_MemAlloc(iPitch*iHeight*3);
			if( pImageBuffer == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo image Processing fail", __LINE__, m_uiClipID);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pTrackInfo);
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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo image Processing fail", __LINE__, m_uiClipID);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pTrackInfo);
				if( pImageBuffer != NULL )
				{
					nexSAL_MemFree(pImageBuffer);
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			if( iWidth == 0 || iHeight == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo image Processing fail", __LINE__, m_uiClipID);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pTrackInfo);
				if( pImageBuffer != NULL )
				{
					nexSAL_MemFree(pImageBuffer);
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			renderInfo.mByteCount		= 0;
			renderInfo.mImageY		= pY;
			renderInfo.mImageU		= pU;
			renderInfo.mImageV		= pV;
			renderInfo.mIsNV12Jepg	= TRUE;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo(%d %d %p)", __LINE__, m_uiClipID, m_iWidth, m_iHeight, renderInfo.mRGB);
			if( m_pTrackInfo->setClipRenderInfo(&renderInfo) == FALSE )
			{
				// Render fail
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ID(%d) Set registeLayerTrackInfo Failed", __LINE__, m_uiClipID);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pTrackInfo);
				if( pImageBuffer != NULL )
				{
					nexSAL_MemFree(pImageBuffer);
					pImageBuffer = NULL;
				}
				return FALSE;
			}

			if( pImageBuffer != NULL )
			{
				nexSAL_MemFree(pImageBuffer);
				pImageBuffer = NULL;
			}

			break;
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
			if( pVideoEditor->callbackGetImageUsingFile(m_strFilePath, 1, strlen(m_strFilePath), &iWidth, &iHeight, &iBitForPixel, &pImageBuffer, &iImageDataSize, &pUserData, &iLoadedType) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo image Processing fail", __LINE__, m_uiClipID);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pTrackInfo);
				return FALSE;
			}

			if( iWidth == 0 || iHeight == 0 || iBitForPixel == 0 || pImageBuffer == NULL || iImageDataSize == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo image Processing fail", __LINE__, m_uiClipID);
				pVideoEditor->callbackReleaseImage(&pUserData);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pTrackInfo);
				return FALSE;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) Image date result(%d %d %p)", __LINE__, m_uiClipID, iWidth, iHeight, pImageBuffer);
			renderInfo.mWidth			= iWidth;
			renderInfo.mHeight		= iHeight;
			renderInfo.mPitch			= iWidth;
			renderInfo.mByteCount		= iBitForPixel / 8;
			renderInfo.mRGB			= pImageBuffer;
			renderInfo.mLoadedType			= 1;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo(%d %d %p)", __LINE__, m_uiClipID, m_iWidth, m_iHeight, renderInfo.mRGB);
			if( m_pTrackInfo->setClipRenderInfo(&renderInfo) == FALSE )
			{
				// Render fail
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ID(%d) Set registeLayerTrackInfo Failed", __LINE__, m_uiClipID);
				pVideoEditor->callbackReleaseImage(&pUserData);
				SAFE_RELEASE(pVideoEditor);
				SAFE_RELEASE(m_pTrackInfo);
				return FALSE;
			}

			pVideoEditor->callbackReleaseImage(&pUserData);
			break;
		}
		default:
			SAFE_RELEASE(pVideoEditor);
			SAFE_RELEASE(m_pTrackInfo);
			return FALSE;
	};
	
	SAFE_RELEASE(pVideoEditor);

 	CNxMsgAddLayer* pMsg = new CNxMsgAddLayer(m_pTrackInfo);
	if( pMsg )
	{
		pVideoRender->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo successed(%p)\n", __LINE__, m_uiClipID, m_pTrackInfo);
		if( isPreView )
			SAFE_RELEASE(m_pTrackInfo);
		return TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[Layer.cpp %d] ID(%d) registeLayerTrackInfo failed(%p)\n", __LINE__, m_uiClipID, m_pTrackInfo);
	SAFE_RELEASE(m_pTrackInfo);
	return FALSE;
}

void CLayerItem::deregisteLayerTrackInfo(CNEXThreadBase* pVideoRender)
{
	if( m_pTrackInfo == NULL ) return;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ID(%d) deregisteLayerTrackInfo In(%p)", __LINE__, m_uiClipID, m_pTrackInfo);
	SAFE_RELEASE(m_pTrackInfo);
	
	CNxMsgDeleteLayer* pMsg = new CNxMsgDeleteLayer(m_uiClipID);
	if( pMsg )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] Send deregisteLayerTrackInfo(%p)", __LINE__, m_pTrackInfo);
		pVideoRender->SendCommand(pMsg);
		SAFE_RELEASE(pMsg);
	}
}

NXBOOL CLayerItem::setPreview(unsigned int uiCurrentTimeStamp, CNEXThreadBase* pVideoRender)
{
	unsigned int uiResultTime = 0;
	CAutoLock lock(m_Lock);

	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] Image Clip did not exist(%s)", __LINE__, m_strFilePath);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] setPreviewImage(%d %d %d %p)", __LINE__, m_ClipType, m_uiStartTime, m_uiEndTime, m_pTrackInfo);
	return registeLayerTrackInfo(pVideoRender, TRUE);
}

NXBOOL CLayerItem::unsetPreview()
{
	CAutoLock lock(m_Lock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ID(%d) unsetPreview(FileReader : %p ImageTrack : %p)", __LINE__, m_uiClipID, m_pFFReader, m_pTrackInfo);
	if( m_pTrackInfo  )
	{
		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			deregisteLayerTrackInfo((CNEXThreadBase*)pVideoRender);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[Layer.cpp %d] ID(%d) unsetPreview(%p)", __LINE__, m_uiClipID, m_pTrackInfo);
			SAFE_RELEASE(pVideoRender);
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) unsetPreview delete m_pTrackInfo", __LINE__, m_uiClipID);
		SAFE_RELEASE(m_pTrackInfo);
	}
	m_isPlay = 0;
	SAFE_RELEASE(m_pFFReader);
	return TRUE;	
}

NXBOOL CLayerItem::updatePlayTime(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	CAutoLock lock(m_Lock);

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[Layer.cpp %d] ID(%d) updatePlayTime(%d %d %d %p)", __LINE__, m_uiClipID, m_uiStartTime, m_uiEndTime, uiTime, m_pTrackInfo);	
	if( isPlayTime(uiTime, TRUE) )
	{
		startPlay(uiTime, pVideoRender, pAudioRender);
	}
	else
	{
		stopPlay();
	}
	return TRUE;
}

NXBOOL CLayerItem::startPlay(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender)
{
	if( m_isPlay ) return TRUE;

	if( pVideoRender == NULL || pAudioRender == NULL ) return  FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) startPlayLayer", __LINE__, m_uiClipID);
	
	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 && m_strFilePath[0] != '@' )
	{
		NEXSALFileHandle handleTest = nexSAL_FileOpen(m_strFilePath, NEXSAL_FILE_READ);
		if( handleTest == NEXSAL_INVALID_HANDLE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Clip.cpp %d] ID(%d) startPlayLayer File missing(%s)", __LINE__, m_uiClipID, m_strFilePath);
			return FALSE;
		}
		nexSAL_FileClose(handleTest);
	}

	if( registeLayerTrackInfo(pVideoRender) )
	{
		m_isPlay = TRUE;
		return TRUE;
	}
	return FALSE;
}

NXBOOL CLayerItem::stopPlay()
{
	CAutoLock lock(m_Lock);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) stopPlay(%d %d %d %p) AudioTask(%p) VideoTask(%p)", 
		__LINE__, m_uiClipID, m_ClipType, m_uiStartTime, m_uiEndTime, m_pTrackInfo, m_pAudioTask, m_pVideoTask);
	
	if( m_pVideoTask && m_pVideoTask->IsWorking() )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		m_pVideoTask->End(1000);
		SAFE_RELEASE(m_pVideoTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) stopPlay Video End Time(%d)", __LINE__, m_uiClipID, uiTick);	

	}

	if( m_pAudioTask && m_pAudioTask->IsWorking() )
	{
		unsigned int uiTick = nexSAL_GetTickCount();
		m_pAudioTask->End(1000);
		SAFE_RELEASE(m_pAudioTask);
		uiTick = nexSAL_GetTickCount() - uiTick;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) stopPlay Audio End Time(%d)", __LINE__, m_uiClipID, uiTick);	
	}

	SAFE_RELEASE(m_pFFReader);

	if( m_pTrackInfo )
	{
		CNEXThread_VideoRenderTask* pVideoRender = CNexProjectManager::getVideoRenderer();
		if( pVideoRender )
		{
			deregisteLayerTrackInfo((CNEXThreadBase*)pVideoRender);
			SAFE_RELEASE(pVideoRender);
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ID(%d) stopPlay delete m_pTrackInfo", __LINE__, m_uiClipID);
		SAFE_RELEASE(m_pTrackInfo);
	}
	
	m_isPlay	= 0;
	return TRUE;
}

void CLayerItem::printLayerInfo()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] -------------- Layer Info ------------------", __LINE__);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_uiClipID: %d", __LINE__, m_uiClipID);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_ClipType: %d", __LINE__, m_ClipType);

	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_uiTotalTime(%d) StartTime(%d) EndTime(%d)", __LINE__, m_uiTotalTime, m_uiStartTime, m_uiEndTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_uiStartTrimTime(%d) m_uiEndTrimTime(%d)", __LINE__, m_uiStartTrimTime, m_uiEndTrimTime);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_iWidth(%d) m_iHeight(%d) ", __LINE__, m_iWidth, m_iHeight);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_isVideoExist(%d)", __LINE__, m_isVideoExist);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_iBrightness(%d) m_iContrast(%d) m_iSaturation(%d) m_iTintcolor(%d) m_iHue(%d)", __LINE__, m_iBrightness, m_iContrast, m_iSaturation, m_iTintcolor, m_iHue);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_iRotateState(%d) m_iSpeedCtlFactor(%d)", __LINE__, m_iRotateState, m_iSpeedCtlFactor);
	
	if( //m_strEffectID != NULL && 
		strlen(m_strEffectID) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_strEffectClipID(%s)", __LINE__, m_strEffectID);

	if( //m_strFilterID != NULL && 
		strlen(m_strFilterID) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_strFilterID(%s)", __LINE__, m_strFilterID);

	if( m_strDisplayOption != NULL && strlen(m_strDisplayOption) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_strDisplayOption: %s", __LINE__, m_strDisplayOption);
	
	if( m_strFilePath != NULL && strlen(m_strFilePath) > 0 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] m_strFilePath: %s", __LINE__, m_strFilePath);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[Layer.cpp %d] ------------- Layer Info --------------", __LINE__);
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2014/12/02	Draft.
-----------------------------------------------------------------------------*/
