/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoThumbTask.cpp
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

#include "NEXVIDEOEDITOR_VideoPreviewTask.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "NEXVIDEOEDITOR_Util.h"

#include "NexMediaDef.h"
#include "NexCodecUtil.h"

CNEXThread_VideoPreviewTask::CNEXThread_VideoPreviewTask( void )
{
	m_uiClipID				= INVALID_CLIP_ID;
	m_pClipItem				= NULL;
	m_pSource			= NULL;

	m_pCodecWrap			= NULL;

	m_iWidth					= 0;
	m_iHeight				= 0;
	m_iPitch					= 0;
	m_isNextFrameRead		= TRUE;

	m_bPreviewFailed		= FALSE;
	m_iPreviewStartTime		= 0;
	m_iPreviewEndTime		= 0;
	m_pSurfaceTexture		= NULL;

	m_uiCheckVideoDecInit		= 0;

	m_FrameTimeChecker.clear();
	m_bSemaWait			= FALSE;
	m_pPreviewThumb = new CPreviewThumb;

	m_uiStartTime = 0;
	m_uiEndTime = 0;
	m_uiCurrentTime = 0;
	m_uiLastDecTime = 0xffffffff;    
	m_bEndFlag = 0;   
	m_bStartFlag = 0;
	m_bDecToReverse = 0;   
	m_bNeedReset = 0;

	m_pPreviousRenderInfo = NULL;
	m_pCurrentRenderInfo = NULL; 	

	m_uiUpdatedTime = 0;
}

CNEXThread_VideoPreviewTask::~CNEXThread_VideoPreviewTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ClipID:%d ~~~~CNEXThread_VideoPreviewTask In", __LINE__, m_uiClipID);
	SAFE_RELEASE(m_pClipItem);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pCodecWrap);
	delete m_pPreviewThumb;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ClipID:%d ~~~~CNEXThread_VideoPreviewTask Out", __LINE__, m_uiClipID);
}

void CNEXThread_VideoPreviewTask::End( unsigned int uiTimeout )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) End In(%d)", __LINE__, m_uiClipID, m_bIsWorking);
	if( m_bIsWorking == FALSE ) return;

	m_bIsWorking = FALSE;

	if(m_bSemaWait)    
		nexSAL_SemaphoreRelease(m_hSema);

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
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) End Out", __LINE__, m_uiClipID);
}

void CNEXThread_VideoPreviewTask::WaitTask()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] WaitTask In", __LINE__, m_uiClipID);
	if( m_pPreviewThumb  )
	{
		nexSAL_TaskWait(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] WaitTask Wait end", __LINE__, m_uiClipID);
		nexSAL_TaskDelete(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] WaitTask delete end", __LINE__, m_uiClipID);
		m_hThread = NEXSAL_INVALID_HANDLE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] WaitTask Out", __LINE__, m_uiClipID);
}

int CNEXThread_VideoPreviewTask::OnThreadMain( void )
{
	NEXVIDEOEDITOR_ERROR		eRet			= NEXVIDEOEDITOR_ERROR_NONE;
	unsigned int				uiDTS			= 0;
	unsigned int				uiPTS			= 0;
	unsigned char*				pFrame			= NULL;
	unsigned int				uiFrameSize		= 0;
	unsigned int				uiDecoderErrRet	= 0;
	unsigned int				uiEnhancement	= NEXCAL_VDEC_FLAG_NONE;

	unsigned int				uiSeekResult	= 0;
	NXBOOL						isIDRFrame		= TRUE;
	NXBOOL						bIDRFrame		= FALSE;

	unsigned int				uiDecOutCTS		= 0;
	int				iDecRealTime		= 0;
    
	NXBOOL						bEndContent		= FALSE;

	int 						iFrameNALHeaderSize		= 0;
	NXBOOL						bSupportFrameTimeChecker = 0;

	int						iReadFrameRet	= 0;
	NXBOOL                              bSetWait = 0;
	CCalcTime		calcTime;
	CClipVideoRenderInfo* pRenderInfo = NULL;	

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		bSupportFrameTimeChecker = pEditor->m_bSupportFrameTimeChecker;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] bSupportFrameTimeChecker (%d)", __LINE__, bSupportFrameTimeChecker);		
		SAFE_RELEASE(pEditor);
	}

	if( m_pPreviewThumb == NULL || m_pSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ClipID(%d) PreviewThumb Task failed because reader or PreviewThumb invalid handle(%p %p)", 
			__LINE__, m_uiClipID, m_pPreviewThumb, m_pSource);
		m_bPreviewFailed = TRUE;
		return 181818;		
	}

	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ClipID(%d) PreviewThumb process start", __LINE__, m_uiClipID);

	if( m_pSource->seekToVideoWithAbsoluteTime(0, &uiSeekResult, NXFF_RA_MODE_CUR_PREV) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] seek failed for p frame preview", __LINE__);
		m_bIsWorking = FALSE;
	}

	if( initVideoDecoder() == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ClipID(%d) Video Task init Decoder failed ", __LINE__, m_uiClipID);
		m_bPreviewFailed = TRUE;
		return 181818;
	}

	m_pCurrentRenderInfo = m_pClipItem->getActiveVideoRenderInfo(m_iPreviewStartTime, FALSE);	
	int seekTime = calcTime.restoreSpeed(m_pCurrentRenderInfo->mStartTime, m_pCurrentRenderInfo->mStartTrimTime, m_pCurrentRenderInfo->m_iSpeedCtlFactor, m_iPreviewStartTime);

	if( m_pSource->seekToVideoWithAbsoluteTime(seekTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] seek failed for p frame preview", __LINE__);
		m_bIsWorking = FALSE;
	}

	if((m_pCurrentRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiSeekResult, TRUE)) == NULL)
		m_pCurrentRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[0];

	// comments related to m_uiDecFrameCountAfterDecInit will be removed.
	//m_uiDecFrameCountAfterDecInit = 0;

	NXBOOL bSoftwareCodec = !m_pCodecWrap->isHardwareCodec();
	
	if( m_pPreviewThumb->setPreviewThumbInfo(m_iWidth, m_iHeight, m_iPitch) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] setPreview info failed", __LINE__);
		m_bIsWorking = FALSE;
	}

	m_isNextFrameRead = TRUE;

	// initialize Vector of FrameCTS
	m_FrameTimeChecker.clear();

	while ( m_bIsWorking )
	{
		if(bSetWait || m_bNeedReset)
		{
			m_bEndFlag = bEndContent;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) wait task (0x%x)", __LINE__, m_uiClipID);		
			// sort
			m_pPreviewThumb->sortWithTime();
			m_bSemaWait = TRUE;
			nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
			if(bEndContent)
			{
				bEndContent = FALSE;
			}
			uiDecoderErrRet = 0;
			uiEnhancement	= NEXCAL_VDEC_FLAG_NONE;
			m_bSemaWait = FALSE;
			bSetWait = FALSE;
			m_bNeedReset = FALSE;
		}

		if( m_isNextFrameRead )
		{
			iReadFrameRet = m_pSource->getVideoFrame();
			if( iReadFrameRet != _SRC_GETFRAME_OK )
			{
				if( iReadFrameRet == _SRC_GETFRAME_END )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] End this contents", __LINE__);
					bEndContent = TRUE;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] contents read operation failed(0x%x)", __LINE__, iReadFrameRet);
					m_bPreviewFailed = TRUE;
					return FALSE;
				}
			}
			m_isNextFrameRead = FALSE;
		}
        
		if( bEndContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] get frame end", __LINE__);
		}
		else
		{
			m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
			m_pSource->getVideoFramePTS( &uiPTS, TRUE );
			m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );

			pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiPTS, TRUE);
			if(pRenderInfo != NULL && pRenderInfo != m_pCurrentRenderInfo)
			{
				m_pPreviousRenderInfo = m_pCurrentRenderInfo;
				m_pCurrentRenderInfo = pRenderInfo;
				m_uiUpdatedTime = uiPTS;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] changed renderInfo %d", __LINE__, uiPTS);
			}
		}

		if( !bEndContent && m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264 )
		{
          		NXBOOL bBFrame = 0;
			bBFrame = !NexCodecUtil_AVC_IsAVCReferenceFrame(pFrame, uiFrameSize, m_pSource->getFrameFormat(), iFrameNALHeaderSize);

			if(bBFrame)
			{
				m_isNextFrameRead = TRUE;
				continue;
			}
		}

		bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);

		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
									pFrame,
									uiFrameSize,

									NULL,
									uiDTS,
									uiPTS,
									uiEnhancement, 
									&uiDecoderErrRet );

		if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
			{
				m_isNextFrameRead = TRUE;
				if(bSupportFrameTimeChecker)
					m_FrameTimeChecker.addFrameCTSVec(uiDTS, uiPTS);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Decode PreviewThumb Frame(p:%p s:%d DTS:%d PTS:%d) IDR(%d)", 
					__LINE__, m_uiClipID, pFrame, uiFrameSize, uiDTS, uiPTS, bIDRFrame);
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char*	pY = NULL;
				unsigned char*	pU = NULL;
				unsigned char*	pV = NULL;
				NXBOOL                bUsePrevInfo = FALSE;

				uiDecOutCTS = 0;
				switch(m_pCodecWrap->getFormat())
				{
					case NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY:
					{
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed(%d)",__LINE__, m_uiClipID, uiDecOutCTS);
							break;
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) GetOutput *m_pY=0x%08x CTS(%d)", __LINE__, m_uiClipID, pY, uiDecOutCTS);
						if( bSupportFrameTimeChecker && m_FrameTimeChecker.isValidFrameCTS(uiDecOutCTS, TRUE) == FALSE)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, m_uiClipID, uiPTS, uiDecOutCTS);
#ifdef _ANDROID
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
							break;
						}

						if(bSupportFrameTimeChecker)
							m_FrameTimeChecker.removeSmallerFrameTime(uiDecOutCTS);

						if(m_uiUpdatedTime > uiDecOutCTS)
							bUsePrevInfo = TRUE;

						if( bEndContent && (!bSupportFrameTimeChecker || m_FrameTimeChecker.empty()) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Last Video Frame force display",__LINE__, m_uiClipID);
							if(bUsePrevInfo)
								iDecRealTime = (int)calcTime.applySpeed(m_pPreviousRenderInfo->mStartTime, m_pPreviousRenderInfo->mStartTrimTime, m_pPreviousRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);
							else
								iDecRealTime = (int)calcTime.applySpeed(m_pCurrentRenderInfo->mStartTime, m_pCurrentRenderInfo->mStartTrimTime, m_pCurrentRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);
						}
						else
						{
							if(bUsePrevInfo)
								iDecRealTime = (int)calcTime.applySpeed(m_pPreviousRenderInfo->mStartTime, m_pPreviousRenderInfo->mStartTrimTime, m_pPreviousRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);
							else
								iDecRealTime = (int)calcTime.applySpeed(m_pCurrentRenderInfo->mStartTime, m_pCurrentRenderInfo->mStartTrimTime, m_pCurrentRenderInfo->m_iSpeedCtlFactor, uiDecOutCTS);

							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) GetOutput Time (outTime:%d realTime:%d), %d", 
								__LINE__, m_uiClipID, uiDecOutCTS, iDecRealTime, bUsePrevInfo);

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
						}

						if( iDecRealTime < m_iPreviewStartTime )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] Decoded frame time is smaller than start time(%d %d)", __LINE__, iDecRealTime, m_iPreviewStartTime);
#ifdef _ANDROID
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
							break;
						}

						if( m_uiLastDecTime == 0xffffffff || m_uiLastDecTime + 50 <= iDecRealTime )
						{
							if( m_pPreviewThumb->addPreviewThumbInfo((void*)pY, iDecRealTime, m_bDecToReverse) == FALSE )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] Add Preview  Fail!", __LINE__);
							}
							m_uiLastDecTime = iDecRealTime;
						}
						else
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] Decoded frame skip(%d %d)", __LINE__, m_uiLastDecTime, iDecRealTime);
#ifdef _ANDROID
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
						}

						if( iDecRealTime > m_iPreviewEndTime )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] bSet Wait(%d %d)", __LINE__, iDecRealTime, m_iPreviewEndTime);
							bSetWait = TRUE;
							break;
						}
                        
						break;
					}
					default:
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] not support getOutFrame format(0x%x) for thumb", __LINE__, m_pCodecWrap->getFormat());
						break;
					}
				};
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] EOS detected", __LINE__);
				bSetWait = TRUE;
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] Video Dec failed(%d)", __LINE__, uiDecoderErrRet);
			if( isEOSFlag(uiEnhancement) || NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] Can't Prepare Preview",__LINE__);
			}
			break;
		}
		
		if( (!bSupportFrameTimeChecker || m_FrameTimeChecker.empty() == TRUE ) && isEOSFlag(uiEnhancement) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] EOS detected", __LINE__);
            
			bSetWait = TRUE;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Video Task End while", __LINE__, m_uiClipID);

	deinitVideoDecoder();
	m_pSurfaceTexture = NULL;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Video Task End and Release Codec", __LINE__, m_uiClipID);
	return 0;
}

NXBOOL CNEXThread_VideoPreviewTask::setClipItem(CClipItem* pClipItem)
{
	if( pClipItem == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pClipItem);
	SAFE_ADDREF(pClipItem);
	m_pClipItem	= pClipItem;
	if( m_pClipItem )
	{
		m_uiClipID = m_pClipItem->getClipID();
		m_uiClipTotalTime = m_pClipItem->getTotalTime();
	}

	m_uiStartTime		= m_pClipItem->getStartTime();
	m_uiEndTime		= m_pClipItem->getEndTime();
    
	return TRUE;
}

NXBOOL CNEXThread_VideoPreviewTask::setSource(CNexSource* pSource)
{
	SAFE_RELEASE(m_pSource);
	SAFE_ADDREF(pSource);
	m_pSource = pSource;


	m_iWidth	= CNexVideoEditor::m_iSupportedWidth;
	m_iHeight	= CNexVideoEditor::m_iSupportedHeight;
	m_iPitch	= CNexVideoEditor::m_iSupportedWidth;

	if( m_pSource != NULL )
	{
		unsigned int uiWidth = 0;
		unsigned int uiHeight = 0;

		if( m_pSource->getVideoResolution(&uiWidth, &uiHeight) == NEXVIDEOEDITOR_ERROR_NONE )
		{
			m_iWidth
			= (int)uiWidth;
			m_iPitch	= (int)uiWidth;
			m_iHeight	= (int)uiHeight;
		}
	}

	return TRUE;
}

NXBOOL CNEXThread_VideoPreviewTask::setPreviewTime(unsigned int uPreviewTime, NXBOOL bReset)
{
	unsigned int uiStartTime = 0, uiEndTime = 0, uiSeekResult = 0;	
	CCalcTime		calcTime;                   
	int seekTime = 0;
	CClipVideoRenderInfo* pRenderInfo = NULL;	
    
	if(bReset && m_iPreviewStartTime < uPreviewTime && m_iPreviewEndTime > uPreviewTime)
	{
		return TRUE;	
	}

	if(bReset)
	{
		m_bNeedReset = TRUE;	
		while(!m_bSemaWait)
			nexSAL_TaskSleep(5);
		//m_pPreviewThumb->removePreviewData(0, 0xffffffff);
	}
	
	if(m_iPreviewStartTime  == 0 && m_iPreviewEndTime == 0)
	{
		m_iPreviewStartTime = uPreviewTime > m_uiStartTime + 2000?uPreviewTime - 2000:m_uiStartTime;
		m_iPreviewEndTime = (m_iPreviewStartTime + 4000 > m_uiEndTime)?m_uiEndTime:m_iPreviewStartTime + 4000;

		if(m_iPreviewEndTime == m_uiEndTime)
		{
			m_iPreviewStartTime = (m_iPreviewEndTime - 4000 > m_uiStartTime)?m_iPreviewEndTime - 4000:m_uiStartTime;
		}
	}
	
	if(m_bSemaWait)
	{
		m_iPreviewStartTime = uPreviewTime > m_uiStartTime + 2000?uPreviewTime - 2000:m_uiStartTime;
		m_iPreviewEndTime = (m_iPreviewStartTime + 4000 > m_uiEndTime)?m_uiEndTime:m_iPreviewStartTime + 4000;

		m_pPreviewThumb->getStartEndTime(&uiStartTime, &uiEndTime);        

		if(m_uiCurrentTime < uPreviewTime) 
		{
			if(bReset || (m_bEndFlag == FALSE && uPreviewTime + 1500 > uiEndTime && m_iPreviewEndTime > uiEndTime))
			{
				if(m_iPreviewStartTime > m_uiStartTime)
					m_bStartFlag = FALSE;
				m_pPreviewThumb->removePreviewData(m_iPreviewStartTime, m_iPreviewEndTime);
				if(m_pPreviewThumb->getSize())
				{
					m_pPreviewThumb->getStartEndTime(&uiStartTime, &uiEndTime);        
					m_iPreviewStartTime = uiEndTime;
				}
				if(m_pPreviewThumb->getSize() == 0 || m_bDecToReverse)
				{
					m_uiLastDecTime = 0xffffffff;
					m_isNextFrameRead = TRUE;
					if((pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(m_iPreviewStartTime, FALSE)) == NULL)
						pRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[m_pClipItem->m_ClipVideoRenderInfoVec.size() -1];
					seekTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, m_iPreviewStartTime);
					m_pSource->seekToVideoWithAbsoluteTime(seekTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV);
					resetVideoDecoder();
					m_uiUpdatedTime = 0;
				}
				m_bDecToReverse = FALSE;
				nexSAL_SemaphoreRelease(m_hSema);
			}
		}
		else if(m_uiCurrentTime > uPreviewTime)
		{
			if(bReset || (m_bStartFlag == FALSE && m_uiCurrentTime < uiStartTime + 1500))
			{
				m_bDecToReverse = TRUE;
				if(m_iPreviewEndTime < m_uiEndTime)
					m_bEndFlag = FALSE;
				m_pPreviewThumb->removePreviewData(m_iPreviewStartTime, m_iPreviewEndTime);
				if(m_pPreviewThumb->getSize())
				{
					m_pPreviewThumb->getStartEndTime(&uiStartTime, &uiEndTime);        
					m_iPreviewEndTime = uiStartTime;
				}

				m_uiLastDecTime = 0xffffffff;
				m_isNextFrameRead = TRUE;
				if((pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(m_iPreviewStartTime, FALSE)) == NULL)
					pRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[0];
				seekTime = calcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, m_iPreviewStartTime);
				m_pSource->seekToVideoWithAbsoluteTime(seekTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV);
				resetVideoDecoder();
				m_uiUpdatedTime = 0;
				nexSAL_SemaphoreRelease(m_hSema);
			}
		}
	}

	if(m_iPreviewStartTime == m_uiStartTime)
		m_bStartFlag = TRUE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%D) buffertime(%d, %d) setPreviewTime(%d, %d, %d, %d)", __LINE__, m_uiClipID, uiStartTime, uiEndTime, m_uiCurrentTime, uPreviewTime, m_iPreviewStartTime, m_iPreviewEndTime);
	m_uiCurrentTime = uPreviewTime;

	return TRUE;
}

NXBOOL CNEXThread_VideoPreviewTask::setPreviewRender(void* pThumbRender, void* pOutputSurface)
{
	m_pPreviewThumb->init(pThumbRender, pOutputSurface);
	return TRUE;
}

NXBOOL CNEXThread_VideoPreviewTask::getPreviewThumbState()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%D) getPreviewThumbState(%d)", __LINE__, m_uiClipID, m_bSemaWait);
	return m_bSemaWait;
}

unsigned char* CNEXThread_VideoPreviewTask::getPreviewData(unsigned int uiTime, NXBOOL bWait)
{
	if(bWait)
	{
		while(!m_bSemaWait)
		{
			if(m_bPreviewFailed == TRUE)
				return NULL;
			nexSAL_TaskSleep(5);
		}
	}
	return m_pPreviewThumb->getPreviewData(uiTime);
}

NXBOOL CNEXThread_VideoPreviewTask::initVideoDecoder()
{
	CAutoLock m(CNexCodecManager::getVideoDecInitLock());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%D) initVideoDecoder() In", __LINE__, m_uiClipID);
	if( m_pSource == NULL )
		return FALSE;

	if( m_pSource->isVideoExist() == FALSE )
		return FALSE;

	SAFE_RELEASE(m_pCodecWrap);
	m_pCodecWrap = new CNexCodecWrap();

	if( m_pCodecWrap == NULL )
	{
		return FALSE;
	}

    int retryCount = 3;
	while( m_bIsWorking )
	{
		if( CNexCodecManager::isHardwareDecodeAvailable(m_iWidth, m_iHeight) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Now available hardware codec", __LINE__, m_uiClipID);
			break;
		}
		nexSAL_TaskSleep(30);
		retryCount--;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Wait available hardware codec", __LINE__, m_uiClipID);
		if(retryCount < 0)
		{
		    break;
		}
	}

	if( m_bIsWorking == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Task exit before video decoder init", __LINE__, m_uiClipID);
		return TRUE;
	}

	NXBOOL bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
											NEXCAL_MODE_DECODER,
											m_pSource->getVideoObjectType(),
											m_iWidth,
											m_iHeight,
											m_iWidth);

	if( bRet == FALSE )
	{
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Get Codec Failed(0x%x)", __LINE__, m_uiClipID, m_pSource->getVideoObjectType());
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Get Codec end(%p)", __LINE__, m_uiClipID, m_pSource->getVideoObjectType(), m_pCodecWrap->getCodecHandle());

	
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
	NXUINT32	uiH264ProfileLevelID	= 0;

	NXINT64			isUseIframeVDecInit	= 0;
	unsigned int	eRet 				= 0;
	int 			iDSINALHeaderSize	= 0;
	int 			iFrameNALHeaderSize	= 0;
 
	unsigned int	uiUserDataType		= 0;

	unsigned int	iWidth				= 0;
	unsigned int	iHeight				= 0;
	void*			pSurfaceTexture		= NULL;
	
	IRectangle*	pRec					= NULL;
	
	float* pMatrix = NULL;
	int iMatrixSize = 0;

	CClipVideoRenderInfo videoInfo;
	//memset(&videoInfo, 0x00, sizeof(CLIP_VIDEO_RENDER_INFO));

	if( nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	iDSINALHeaderSize = m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

#ifdef _ANDROID
	pSurfaceTexture = m_pPreviewThumb->getPreviewThumbSurfaceTexture();
	if( pSurfaceTexture == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Get Dec Surface Failed(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
		SAFE_RELEASE(m_pCodecWrap);
		return FALSE;
	}
	
	m_pSurfaceTexture = pSurfaceTexture;
	m_pCodecWrap->setSurfaceTexture(m_pSurfaceTexture);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Use surfaceTexture(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
#endif

	while(1)
	{
		if( m_isNextFrameRead )
		{
			unsigned int uiRet = m_pSource->getVideoFrame();
			m_isNextFrameRead = FALSE;
			if( uiRet == _SRC_GETFRAME_OK )
			{
			}
			else if( uiRet == _SRC_GETFRAME_NEEDBUFFERING )
			{
				nexSAL_TaskSleep(20);
				m_isNextFrameRead = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Need buffering Not normal condition(0x%x)", __LINE__, m_uiClipID, uiRet);
				continue;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) Get Frame fail while decoder init(0x%x)", __LINE__, m_uiClipID, uiRet);
				goto DECODER_INIT_ERROR;
			}			
		}

		if( m_pCodecWrap->isHardwareCodec() )
		{
			m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
			m_pSource->getVideoFramePTS( &uiPTS, TRUE );
		}
		else
		{
			m_pSource->getVideoFrameDTS( &uiDTS );
			m_pSource->getVideoFramePTS( &uiPTS);
		}
		m_pSource->getVideoFrameData(&pFrame, &uiFrameSize);
		
		isValidVideo = NexCodecUtil_IsValidVideo( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize );
		if( isValidVideo == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VPreviewTask.cpp %d] ID(%d) This Video frame is invalid", __LINE__, m_uiClipID);
			nexSAL_TaskSleep(20);
			m_isNextFrameRead = TRUE;
			continue;
		}
		// Codec �ʱ�ȭ�� IDR �������� ���� ���� �ʾ� IFrame�� ã���� ��.

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Check I-frame CTS(%u) PTS(%u) NALSzie(%d)\n", __LINE__, m_uiClipID, uiDTS, uiPTS,  iDSINALHeaderSize);
		isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
		if( isIntraFrame )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) I-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
			break;
		}
		
		m_isNextFrameRead = TRUE;			
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%D) P-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
	}

	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Dec init(%p %p %d)", __LINE__, m_uiClipID, m_pCodecWrap->getCodecHandle(), pBaseDSI, uiBaseDSISize);

			eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
												pBaseDSI,
												uiBaseDSISize,
												pFrame,
												uiFrameSize,
												NULL,
												NULL,
												0,
												&m_iWidth,
												&m_iHeight,
												&m_iPitch,
												m_pSource->getVideoFrameRate(),												
												NEXCAL_VDEC_MODE_NONE);

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp  %d] Video codec init failed(%d %d) and retry", __LINE__, uiRetryCnt, eRet);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iWidth		= iWidth;
			m_iHeight	= iHeight;
			m_iPitch		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Video Decoder Init", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Dec init %p %d",
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
												 &m_iWidth,
												 &m_iHeight,
												 &m_iPitch,
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
												 &m_iWidth,
												 &m_iHeight,
												 &m_iPitch,
												m_pSource->getVideoFrameRate(),												 
												 NEXCAL_VDEC_MODE_NONE);
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp  %d] ID(%d) Video Codec initialize Error So End", __LINE__, m_uiClipID);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_uiClipID, m_iWidth, m_iHeight, m_iPitch );
			break;

		default:
			goto DECODER_INIT_ERROR;
	};

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	m_pPreviewThumb->setCodecWrap(m_pCodecWrap);

	m_isNextFrameRead = FALSE;
	return TRUE;

DECODER_INIT_ERROR:

	m_isNextFrameRead = FALSE;
	deinitVideoDecoder();
	return FALSE;
}

NXBOOL CNEXThread_VideoPreviewTask::deinitVideoDecoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) deinitVideoDecoder In", __LINE__, m_uiClipID);

	if( m_pCodecWrap != NULL )
	{
		m_pCodecWrap->deinitDecoder();
		SAFE_RELEASE(m_pCodecWrap);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) deinitVideoDecoder Out", __LINE__, m_uiClipID);
	return TRUE;
}

void CNEXThread_VideoPreviewTask::resetVideoDecoder()
{
	if( m_pCodecWrap == NULL ) return;
	
#ifdef _ANDROID
	if( isNexus10Device() == FALSE )
	{
		nexCAL_VideoDecoderReset(m_pCodecWrap->getCodecHandle());
	}	
#endif
}

NXBOOL CNEXThread_VideoPreviewTask::isEOSFlag(unsigned int uiFlag)
{
	if( (uiFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
		return TRUE;
	return FALSE;
}

NXBOOL CNEXThread_VideoPreviewTask::canUseSWDecoder()
{
	NXBOOL canUseSWDec = FALSE;

	if( m_pSource == NULL ) 
		return canUseSWDec;

	unsigned int uiVideoType = m_pSource->getVideoObjectType();

	if( uiVideoType == eNEX_CODEC_V_H264 && CNexVideoEditor::m_bSuppoertSWH264Codec )
	{
		canUseSWDec = TRUE;
	}
	else if( uiVideoType == eNEX_CODEC_V_MPEG4V && CNexVideoEditor::m_bSuppoertSWMP4Codec )
	{
		canUseSWDec = TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VPreviewTask.cpp %d] ID(%d) can use resource(0x%x, H:%d M:%d) ret(%d)", 
		__LINE__, m_uiClipID, uiVideoType, CNexVideoEditor::m_bSuppoertSWH264Codec, CNexVideoEditor::m_bSuppoertSWMP4Codec, canUseSWDec);
	return canUseSWDec;
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2015/08/25	Draft.
-----------------------------------------------------------------------------*/
