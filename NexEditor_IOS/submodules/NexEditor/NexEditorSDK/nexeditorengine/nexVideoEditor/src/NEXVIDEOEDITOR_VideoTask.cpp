/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoTask.cpp
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

#include "NEXVIDEOEDITOR_VideoTask.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "NEXVIDEOEDITOR_Util.h"

NXBOOL	 CNEXThread_VideoTask::m_iAvailableHWCodec = 0;

// #define VIDEO_RESULT_DUMP
// #define VIDEO_FRAME_DUMP

#define FAKE_PREVIEW_FPS 120

CNEXThread_VideoTask::CNEXThread_VideoTask( void ) : 
	m_perforDecode((char*)"VDTask Decode"),
	m_perforGetOutput((char*)"VDTask GetOutPut")
{
	m_eTaskPriority			= NEXSAL_PRIORITY_HIGH;
		
	m_pClipItem				= NULL;
	m_uiClipID				= INVALID_CLIP_ID;
	m_pSource			= NULL;
	m_pFileWriter			= NULL;
	m_pCodecWrap			= NULL;
	m_pCodecCache = NULL;
	m_pVideoRenderer		= NULL;
	m_pVideoTrack			= NULL;

	m_iWidth				= 0;
	m_iHeight				= 0;
	m_iPitch				= 0;
    m_iFPS                  = 0;
	m_isNextFrameRead		= TRUE;

	m_pSurfaceTexture		= NULL;

	m_uiEffectStartTime		= 0;

	m_iSpeedFactor			= 100;
	m_uiPreviousPTS			= -1;

	m_uiSystemTick			= 0;

	// comments related to m_uiDecFrameCountAfterDecInit will be removed.
	//m_uiDecFrameCountAfterDecInit	 = 0;

	m_FrameTimeChecker.clear();
	m_EncodeTimeChecker.clear();
	m_uiLastEncOutCTS = 0;
	m_pPreviousRenderInfo = NULL;
	m_pCurrentRenderInfo = NULL;	    
}

CNEXThread_VideoTask::~CNEXThread_VideoTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ClipID:%d ~~~~CNEXThread_VideoTask In", __LINE__, m_uiClipID);
	if(m_pCodecCache == NULL)
	{	
		deinitVideoDecoder();	
	}
	else 
	{
		m_pCodecCache->removeCodecTimeVec(m_uiClipID);
		if(m_pCodecCache->getSize() == 0)
		{
			clearCachedVisualCodecInfo(m_pCodecCache);
			deinitVideoDecoder();	
		}    
		else
		{
			if( m_pVideoTrack )
				m_pVideoTrack->setReUseSurfaceTexture(TRUE);
		}
	}
	deregistTrack();

	SAFE_RELEASE(m_pClipItem);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pFileWriter);
	SAFE_RELEASE(m_pCodecCache);
	SAFE_RELEASE(m_pCodecWrap);

	SAFE_RELEASE(m_pVideoTrack);
	SAFE_RELEASE(m_pVideoRenderer);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ClipID:%d ~~~~CNEXThread_VideoTask Out", __LINE__, m_uiClipID);
}

void CNEXThread_VideoTask::End( unsigned int uiTimeout )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) End In(%d)", __LINE__, m_uiClipID, m_bIsWorking);
	if( m_bIsWorking == FALSE ) return;

	m_bIsWorking = FALSE;

	if(m_bIsTaskPaused)
		nexSAL_SemaphoreRelease(m_hSema);
    
	if( nexSAL_TaskWait(m_hThread) != 0 )
	{
		// nexSAL_TaskTerminate(m_hThread);
	}

	nexSAL_TaskDelete(m_hThread);
	m_hThread = NEXSAL_INVALID_HANDLE;

	if(m_hSema != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_SemaphoreDelete(m_hSema);
		m_hSema = NEXSAL_INVALID_HANDLE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) End Out", __LINE__, m_uiClipID);
}

unsigned int CNEXThread_VideoTask::SetSuspend()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d] ClipID %d\n", Name(), __FUNCTION__, __LINE__, m_pClipItem->getClipID());

	if (m_hThread && !m_bThreadFuncEnd)
	{
		m_bIsTaskPaused = FALSE;
		m_bIsActive = FALSE;

		if (m_pVideoTrack && !m_pVideoTrack->getVideoDecodeEnd())
		{
			m_pVideoTrack->DequeueAllFrameOutBuffer();
		}
	}

	return TRUE;
}

unsigned int CNEXThread_VideoTask::Suspend()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d] ClipID %d\n", Name(), __FUNCTION__, __LINE__, m_pClipItem->getClipID());

	if (m_hThread && !m_bThreadFuncEnd)
	{
		if (m_isSemaState)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[%s %s %d] Already task waiting... return TRUE ClipID %d\n", Name(), __FUNCTION__, __LINE__, m_pClipItem->getClipID());
			return TRUE;
		}

		while (m_bIsTaskPaused == FALSE && m_bIsWorking == TRUE)
		{
			nexSAL_TaskSleep(10);
		}
	}

	return TRUE;
}

NXBOOL CNEXThread_VideoTask::setSpeedFactor(int iFactor)
{
	if( iFactor < 2 || iFactor > 400 )	//yoon
		return FALSE;
	m_iSpeedFactor = iFactor;
	return TRUE;
}

void CNEXThread_VideoTask::setLastEncOutCTS()
{
	NXBOOL				bSupportFrameTimeChecker = 0;
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		bSupportFrameTimeChecker = pEditor->m_bSupportFrameTimeChecker;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] bSupportFrameTimeChecker (%d)", __LINE__, bSupportFrameTimeChecker);		
		SAFE_RELEASE(pEditor);
	}

	if(bSupportFrameTimeChecker)
	{
		if(m_pVideoTrack && m_EncodeTimeChecker.size())
		{
			if(m_pVideoTrack->m_uiLastEncodedFrameCTS == 0)
			{
				m_uiLastEncOutCTS = 0;
			}
			else
			{
				m_EncodeTimeChecker.removeSmallTime(m_pVideoTrack->m_uiLastEncodedFrameCTS);
				m_uiLastEncOutCTS = m_EncodeTimeChecker.getFirstCTSInVec();
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] ID(%d) Video Task update  m_EncodeTimeChecker %d,  %d!\n", __LINE__, m_uiClipID, m_pVideoTrack->m_uiLastEncodedFrameCTS, m_uiLastEncOutCTS);
		}
	}
}

int CNEXThread_VideoTask::getVideoClipCount()
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d]  ID(%d) getVideoClipCount(%d)",  __LINE__, m_pClipItem->getClipID(), count);
			SAFE_RELEASE(pList);
		}
		SAFE_RELEASE(pProjectMng);
	}
	return count;
}

int CNEXThread_VideoTask::OnThreadMain( void )
{
	NEXVIDEOEDITOR_ERROR		eRet			= NEXVIDEOEDITOR_ERROR_NONE;
	unsigned int				uiResultTime	= 0;
	unsigned int				uiDTS			= 0;
	unsigned int				uiPTS			= 0;
	unsigned char*				pFrame			= NULL;
	unsigned int				uiFrameSize		= 0;
	unsigned int				uiDecoderErrRet	= 0;
	unsigned int				uiEnhancement	= NEXCAL_VDEC_FLAG_NONE;

	CFrameInfo*					pFrameInfo		= NULL;

	unsigned int				uiDuration		= 0;
	unsigned int				uiSeekResult	= 0;
	unsigned int				uiLastTime		= 0;
	NXBOOL						isIDRFrame		= TRUE;
	NXBOOL						bIDRFrame		= FALSE;

	unsigned int				uiVideoBaseTime		= 0;

	unsigned int				uiVideoStartTimeCur = 0;
	unsigned int				uiVideoEndTimeCur	= 0;
	unsigned int				uiVideoBaseTimeCur		= 0;
	unsigned int				uiVideoStartTrimCur		= 0;

	unsigned int				uiVideoStartTimePrev = 0;
	unsigned int				uiVideoEndTimePrev	= 0;
	unsigned int				uiVideoBaseTimePrev		= 0;
	unsigned int				uiVideoStartTrimPrev		= 0;
	unsigned int				uiSpeedFactorPrev = 0;
	unsigned int				uiIframeModePrev = 0;

	unsigned int                        uiUpdatedTime = 0;
	unsigned int                        uiFakeUpdatedTime = 0;
    
	int							iDecOutRealTime		= 0;
	unsigned int				uiDecOutCTS		= 0;
	unsigned int				uiLastDecOutCTS		= 0xffffffff;	
	unsigned int				uiLastEncOutCTS		= 0;
	unsigned int				uiDropPTS = 0;

	NXBOOL						bApplyFreezeFrame	= FALSE;
	int							iFreezeDuraion		= 0;

	NXBOOL						bEndContent		= FALSE;

	int 						iSeekTableCount			= 0;
	unsigned int* 				pSeekTable				= NULL;

	int 						iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();
	unsigned int				uiExpectanceTime		= 0;
	unsigned int				uiCheckDecTime			= 0;
	unsigned int				uiCheckGetOutputTime	= 0;

	unsigned int				uiCurrentTimeWhenStartTask	= CNexProjectManager::getCurrentTimeStamp();

	int							iTimeStampControlForMSMDevice = 0;
	int							iMultipleFactor			= 1;

	PLAY_STATE					curState					= CNexProjectManager::getCurrentState();//  == PLAY_STATE_RECORD
	if(curState == PLAY_STATE_PAUSE)
	{
		curState = PLAY_STATE_RECORD;
	}
	
	int							iSkipRenderNumber		= (curState == PLAY_STATE_RUN)?(m_iFPS * m_iSpeedFactor/100)/61:0;
	int							iSkipRenderNumberPrev	= 0;    
#ifdef FOR_PROJECT_LGE
	if( m_iFPS >= 240 ){
		iSkipRenderNumber = 0;
	}
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) video old FPs=%d. SpeedFactor=%d, skipRender=%d)", __LINE__, m_uiClipID, m_iFPS, m_iSpeedFactor ,iSkipRenderNumber);
	int							iSkipRenderCount		= iSkipRenderNumber; //yoon
	int	iOperatingFps = m_iFPS * m_iSpeedFactor/100;
	int	iOperatingFpsPrev = 0;

	int							iCurrProgress = 0;
	int							iPrevProgress = 0;

	unsigned int			uDropCountFromGLR = 0;
	unsigned int			uSkipFramePer = 0;
	unsigned int			uDecCountForSkip = 0;

	int					iFakePreview = 0;
	int					iFakeDecodingCount = 0;
	int					iFakeDecodedCount = 0;
	int					iFakeSeekIndex = 0;
	unsigned int			uiFakeStartDTS = 0;
	unsigned int			uiFakeStartPTS = 0;
	unsigned int			uiFakeLastPTS = 0;	
	unsigned int			uiFakeBaseDTS = 0;
	unsigned int			uiFakeBasePTS = 0;	
	float			fFakeInterval = 0;	
	unsigned int uiFakeSpeedFactor = 0;
	int					iVideoClipCount = 0;
	NXBOOL				bSupportFrameTimeChecker = 0;

	unsigned int                  uiLevelTick = nexSAL_GetTickCount();    
#ifdef FOR_PROJECT_LGE
	NXBOOL bLGInterPoly = 0;
#endif

	unsigned int			uiCachedPTS = 0;	
	NXBOOL                      bSetMyRender = 0;
	NXBOOL                      bReadyIPlay = FALSE;
	NXBOOL                      bCheckEditBox = FALSE;    
	unsigned int uiVideoEditBox = m_pSource->getVideoEditBox();

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		bSupportFrameTimeChecker = pEditor->m_bSupportFrameTimeChecker;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] bSupportFrameTimeChecker (%d)", __LINE__, bSupportFrameTimeChecker);		
		SAFE_RELEASE(pEditor);
	}
	
#ifdef VIDEO_RESULT_DUMP
	FILE* pFileVideoDump = fopen("/sdcard/H264DecOutput.yuv", "wb");
#endif

#ifdef VIDEO_FRAME_DUMP
	char path[256];
	sprintf(path, "/sdcard/H264DecInput_%d_%d.dump", m_uiClipID, nexSAL_GetTickCount());
	FILE* pFileVideoFrame = NULL;
	if( m_uiClipID == 1 )
	{
		pFileVideoFrame = fopen(path, "wb");
	}
#endif

	if( m_pClipItem )
	{
		uiVideoStartTimeCur = m_pClipItem->getStartTime();
		uiVideoEndTimeCur = m_pClipItem->getEndTime();

		uiVideoBaseTimeCur = uiVideoStartTimeCur;
		uiVideoStartTrimCur = m_pClipItem->getStartTrimTime();
	}

	iVideoClipCount = getVideoClipCount();
	
	m_pSource->getTotalPlayTime(&uiDuration);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] Video Task Start ClipID(%d) StartTime(%d) StartTrim(%d) Speed(%d) videoClipCount(%d)",
		__LINE__, m_uiClipID, uiVideoStartTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, iVideoClipCount);

	if( m_pFileWriter != NULL )
	{
		unsigned int uiSeekResult = 0;
		m_pSource->seekToVideoWithAbsoluteTime(uiVideoStartTrimCur, &uiSeekResult, NXFF_RA_MODE_CUR_PREV);
		m_isNextFrameRead = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ClipID(%d) Video Task run with direct export mode(%d %d)", __LINE__, m_uiClipID, uiVideoStartTrimCur, uiSeekResult);
	}
	else
	{
		SAFE_RELEASE(m_pCodecWrap);
		SAFE_RELEASE(m_pCodecCache);

		#if 0        
		if(m_pClipItem->getClipType() != CLIPTYPE_VIDEO_LAYER)
			m_pCodecCache = getCachedVisualCodecInfo();
		#endif
        
		if(m_pCodecCache)
		{
			m_pCodecCache->addCodecTimeVec(m_uiClipID, (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), NULL, 0);
			setCachedVisualCodecInfo(m_pCodecCache);

			while( m_bIsWorking )
			{
				m_pCodecWrap = m_pCodecCache->getCodecWrap();
				if( m_pCodecWrap )
				{
					break;
				}
				nexSAL_TaskSleep(10);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Wait available CodecWrap", __LINE__, m_uiClipID);
			}

			if( initCachedDecoder() == FALSE )
			{
				CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, CNexProjectManager::getCurrentState(), NEXVIDEOEDITOR_ERROR_CODEC_INIT, m_uiClipID, 1);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ClipID(%d) Video Task init Decoder failed ", __LINE__, m_uiClipID);
				m_bIsWorking = FALSE;
				return 181818;
			}

			while( m_bIsWorking )
			{
				if( m_pCodecCache->getMyDecoder(m_uiClipID))
				{
					break;
				}
				nexSAL_TaskSleep(10);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, m_uLogCnt++%30, "[VDTask.cpp %d] ID(%d) Wait CodecCache MyDecoder", __LINE__, m_uiClipID);
			}
			m_uLogCnt = 0;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Wait CodecCache MyDecoder finished", __LINE__, m_uiClipID);            
		}
		else
		{
			#if 0
			if(CNexVideoEditor::m_bNexEditorSDK && curState == PLAY_STATE_RUN)
			{
				if(m_pClipItem->getClipType() != CLIPTYPE_VIDEO_LAYER)
				{
					NXUINT8*		pBaseDSI		= NULL;
					NXUINT32		uiBaseDSISize	= 0;
					m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );
			
					m_pCodecCache = new CCodecCacheInfo;
					m_pCodecCache->addCodecTimeVec(m_uiClipID, (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), pBaseDSI, uiBaseDSISize);
					setCachedVisualCodecInfo(m_pCodecCache);
				}
			}
			#endif
            
			if( initVideoDecoder() == FALSE )
			{
				CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, CNexProjectManager::getCurrentState(), NEXVIDEOEDITOR_ERROR_CODEC_INIT, m_uiClipID, 1);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ClipID(%d) Video Task init Decoder failed ", __LINE__, m_uiClipID);
				m_bIsWorking = FALSE;
				return 181818;
			}
		}

		if( CNexVideoEditor::m_bNexEditorSDK && curState == PLAY_STATE_RUN )
		{
 			//int newFPS = m_iFPS * m_iSpeedFactor/100;
			//if(newFPS > FAKE_PREVIEW_FPS && m_iFPS > 60 && m_iWidth*m_iWidth >= 1920*1080)
			//if( newFPS > FAKE_PREVIEW_FPS && m_iFPS < 150 )
			//if(newFPS > FAKE_PREVIEW_FPS)
			{
				//iFakePreview = TRUE;
				if( m_pSource->getSeekTable(0, &iSeekTableCount, &pSeekTable) != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] getSeekTable failed", __LINE__);
					m_bIsWorking = FALSE;
				}

				int iNewSeekTableCount = 0;
				unsigned int* pNewSeekTable = (unsigned int*)nexSAL_MemAlloc(sizeof(unsigned int)*iSeekTableCount);
				if( pNewSeekTable == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] Fail to alloc seek table because not enough memory", __LINE__);
					m_bIsWorking = FALSE;
				}
				else
				{
					unsigned int uTime = 0;
					if( m_isNextFrameRead )
					{
						int iReaderRet = m_pSource->getVideoFrame();
						switch(iReaderRet)
						{
							case _SRC_GETFRAME_OK:
								break;
							case _SRC_GETFRAME_END:
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] getVideoFrame End\n", __LINE__);
								break;
							default:
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
								break;
						};
						m_isNextFrameRead = FALSE;
					}
				
					m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
					m_pSource->getVideoFramePTS( &uiPTS, TRUE);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] fake preview start time(%d, %d)", __LINE__, uiDTS, uiPTS);						
					
					for( int i = 0; i < iSeekTableCount; i++ )
					{
						uTime = (uiDTS > uiVideoEditBox) ? uiDTS - uiVideoEditBox : 0;
						if( pSeekTable[i] < uTime )
							continue;

						if(iNewSeekTableCount == 0)
						{
							pNewSeekTable[iNewSeekTableCount] = pSeekTable[i];
							iNewSeekTableCount++;
						}
						else
						{
							if(pNewSeekTable[iNewSeekTableCount - 1] + 500 < pSeekTable[i])
							{
								pNewSeekTable[iNewSeekTableCount] = pSeekTable[i];
								iNewSeekTableCount++;
							}
						}
					}

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] update seektable (%d -> %d)", __LINE__, iSeekTableCount, iNewSeekTableCount);
					nexSAL_MemFree(pSeekTable);
					pSeekTable = pNewSeekTable;
					iSeekTableCount = iNewSeekTableCount;
				}
			}
 		}
	}

	m_pSource->getVideoFramePTS( &uiPTS, TRUE );
	m_pCurrentRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[0];
	checkRenderInfo(uiPTS);

	{
		uiVideoStartTimeCur = m_pCurrentRenderInfo->mStartTime;
		uiVideoEndTimeCur = m_pCurrentRenderInfo->mEndTime;

		uiVideoBaseTimeCur = uiVideoStartTimeCur;
		uiVideoStartTrimCur = m_pCurrentRenderInfo->mStartTrimTime;
		m_iSpeedFactor = m_pCurrentRenderInfo->m_iSpeedCtlFactor;
		iSkipRenderNumber = (curState == PLAY_STATE_RUN)?(m_iFPS * m_iSpeedFactor/100)/61:0;
#ifdef FOR_PROJECT_LGE
		if( m_iFPS >= 240 ){
			iSkipRenderNumber = 0;
		}
#endif
		iSkipRenderCount		= iSkipRenderNumber;
		iOperatingFps = m_iFPS * m_iSpeedFactor/100;        

		int iVideoUITime = (int)m_CalcTime.applySpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiPTS);
       	if( iVideoUITime < 0)
			uiVideoBaseTime -= iVideoUITime;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ClipID(%d) uiVideoBaseTime %d %d %d %d %d %d ", __LINE__, m_uiClipID, iVideoUITime, uiVideoBaseTime, uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiPTS);

		uiVideoStartTimePrev = uiVideoStartTimeCur;
		uiVideoEndTimePrev = uiVideoEndTimeCur;                        
		uiVideoBaseTimePrev = uiVideoStartTimePrev;                        
		uiVideoStartTrimPrev = uiVideoStartTrimCur;                        
		uiSpeedFactorPrev = m_iSpeedFactor;
		iSkipRenderNumberPrev = iSkipRenderNumber;
		iOperatingFpsPrev = iOperatingFps;
	}
    
	if( m_pCodecWrap == NULL )
	{
		// GVE-438
		if( m_bIsWorking == FALSE ) {
			return 0;
		}	
		CNexProjectManager::sendEvent(MESSAGE_VDEC_INIT_FAIL, CNexProjectManager::getCurrentState(), NEXVIDEOEDITOR_ERROR_CODEC_INIT, m_uiClipID, 1);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ClipID(%d) Video Task init Decoder failed ", __LINE__, m_uiClipID);
		m_bIsWorking = FALSE;
		return 181818;
	}

	
	// comments related to m_uiDecFrameCountAfterDecInit will be removed.
	//m_uiDecFrameCountAfterDecInit = 0;
	
#ifdef _ANDROID
#ifdef FOR_TEST_QUALCOMM_CHIP
	// if( isQualcommChip() && CNexProjectManager::getCurrentState() == PLAY_STATE_RECORD )
	if( isQualcommChip() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Time Stamp control for QualComm devices", __LINE__, m_uiClipID);
		// do not use flow control KM-4162
		//iTimeStampControlForMSMDevice = 1;
	}
#endif	
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Task init end and start ", __LINE__, m_uiClipID);
	m_perforDecode.CheckModuleStart();
	m_perforGetOutput.CheckModuleStart();

	// initialize Vector of FrameCTS
	m_FrameTimeChecker.clear();
	m_EncodeTimeChecker.clear();

	while ( m_bIsWorking )
	{
		if(m_bIsActive)
		{
			if (m_bIsTaskResumed == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] ID(%d) VideoDecTask RESUMED!!!\n",  __LINE__, m_uiClipID);

				if(m_bIsTaskPaused)
				{
					bEndContent = FALSE;
					uiEnhancement = NEXCAL_VDEC_FLAG_NONE;
					m_pSource->seekToVideo(m_pClipItem->getStartTime(), &uiSeekResult);
					int iReaderRet = m_pSource->getVideoFrame();
					switch(iReaderRet)
					{
						case _SRC_GETFRAME_OK:
							break;
						case _SRC_GETFRAME_END:
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getVideoFrame End\n", __LINE__, m_uiClipID);
							bEndContent = TRUE;
							break;
						default:
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getVideoFrame error(%d) so End clip", __LINE__, m_uiClipID, iReaderRet);
							bEndContent = TRUE;
							break;
					};

					m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
					reinitVideoDecoder(pFrame, uiFrameSize);

					m_pPreviousRenderInfo = NULL;
					uiDropPTS = m_uiLastEncOutCTS;
					if(checkRenderInfo(m_uiLastEncOutCTS))
					{
						uiVideoStartTimeCur = m_pCurrentRenderInfo->mStartTime;
						uiVideoEndTimeCur = m_pCurrentRenderInfo->mEndTime;

						uiVideoBaseTimeCur = uiVideoStartTimeCur;
						uiVideoStartTrimCur = m_pCurrentRenderInfo->mStartTrimTime;
						m_iSpeedFactor = m_pCurrentRenderInfo->m_iSpeedCtlFactor;
						iSkipRenderNumber = (curState == PLAY_STATE_RUN)?(m_iFPS * m_iSpeedFactor/100)/61:0;
						iSkipRenderCount		= iSkipRenderNumber;
						iOperatingFps = m_iFPS * m_iSpeedFactor/100;

						uiVideoStartTimePrev = uiVideoStartTimeCur;
						uiVideoEndTimePrev = uiVideoEndTimeCur;
						uiVideoBaseTimePrev = uiVideoStartTimePrev;
						uiVideoStartTrimPrev = uiVideoStartTrimCur;
						uiSpeedFactorPrev = m_iSpeedFactor;
						iSkipRenderNumberPrev = iSkipRenderNumber;
						iOperatingFpsPrev = iOperatingFps;

						bApplyFreezeFrame = false;
						iFreezeDuraion = m_pCurrentRenderInfo->mFreezeDuration;
					}

#ifdef FOR_PROJECT_LGE
					if( m_iFPS >= 240 ){
						iSkipRenderNumber = 0;
						   iSkipRenderNumberPrev = 0;
					}
#endif
					uiUpdatedTime = 0;
					uiSeekResult = (int)m_CalcTime.restoreSpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, (m_uiLastEncOutCTS==0)?m_pCurrentRenderInfo->mStartTime:m_uiLastEncOutCTS);
					//uiSeekResult = uiSeekResult - m_pClipItem->getStartTime() - m_pClipItem->getStartTrimTime();
					//m_pSource->seekToVideo(uiSeekResult, &uiSeekResult, NXFF_RA_MODE_CUR_PREV);
					m_pSource->seekToVideoWithAbsoluteTime(uiSeekResult, &uiSeekResult, NXFF_RA_MODE_CUR_PREV);                    
					m_isNextFrameRead = TRUE;

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] ID(%d) VideoDecTask resume PTS %d!!!\n",  __LINE__, m_uiClipID, uiDropPTS);

				}
				m_bIsTaskResumed = TRUE;
				m_bIsTaskPaused = FALSE;
			}

			if( m_pFileWriter != NULL )
			{
				nexSAL_TaskSleep(0);
				unsigned int uiDuration = 0;
				unsigned int uiSize = 0;

				if( m_isNextFrameRead )
				{
					int iReaderRet = m_pSource->getVideoFrame();
					switch(iReaderRet)
					{
						case _SRC_GETFRAME_OK:
							break;
						case _SRC_GETFRAME_END:
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] getVideoFrame End\n", __LINE__);
							bEndContent = TRUE;
							break;
						default:
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
							bEndContent = TRUE;
							break;
					};

					processEnhancedFrame(&uiEnhancement);
					m_isNextFrameRead = FALSE;
				}

				int iNewDTS = 0;
				int iNewPTS = 0;

				if( bEndContent )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getVideoFrame EOS(%d %d)", __LINE__, m_uiClipID, m_pClipItem->getEndTime(), uiDTS);
					m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, m_pClipItem->getEndTime(), m_pClipItem->getEndTime(), NULL, 0, &uiDuration, &uiSize);
					break;
				}
				else
				{
					m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
					m_pSource->getVideoFramePTS( &uiPTS, TRUE);

					iNewDTS = (int)m_CalcTime.applySpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiDTS);
					iNewPTS = (int)m_CalcTime.applySpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiPTS);

					if( m_pClipItem->getEndTime() < (iNewDTS + 10) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getVideoFrame EOS(%d %d %d)",
							__LINE__, m_uiClipID, m_pClipItem->getEndTime(), uiDTS, iNewDTS);
						m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, m_pClipItem->getEndTime(), m_pClipItem->getEndTime(), NULL, 0, &uiDuration, &uiSize);
						break;
					}

					m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) read video frame time info(%p %d) (%d %d) (%d %d)",
						__LINE__, m_uiClipID, pFrame, uiFrameSize, uiDTS, uiPTS, iNewDTS, iNewPTS);
				}

				int iWriterRet = m_pFileWriter->setBaseVideoFrameWithoutEncode(m_uiClipID, iNewPTS, iNewDTS, pFrame, uiFrameSize, &uiDuration, &uiSize);
				if( iWriterRet == 0 )		// write sucessed
				{
					m_isNextFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VDTask.cpp %d] ID(%d) Video One frame Write sucess(%d, %d)", __LINE__, m_uiClipID, iNewDTS, iWriterRet);
				}
				else if( iWriterRet == 1 )	// write failed
				{
					m_isNextFrameRead = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video One frame Write failed(%d, %d)", __LINE__, m_uiClipID, iNewDTS, iWriterRet);
				}
				else if( iWriterRet == 2 )	// write wait
				{
					nexSAL_TaskSleep(5);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VDTask.cpp %d] ID(%d) Video One frame Write failed(%d, %d)", __LINE__, m_uiClipID, iNewDTS, iWriterRet);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video One frame Writer unknown(%d, %d)", __LINE__, m_uiClipID, iNewDTS, iWriterRet);
				}
				continue;
			}

			if( iOperatingFps	< 59 )
				nexSAL_TaskSleep(1); //yoon delete

			if( m_pVideoTrack == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] m_pVideoTrack is null", __LINE__);
				break;
			}

			if( m_isNextFrameRead )
			{
				int iReaderRet = m_pSource->getVideoFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] getVideoFrame End", __LINE__);
						bEndContent = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] getVideoFrame error(%d) so End clip", __LINE__, iReaderRet);
						if(CNexProjectManager::getCurrentState() == PLAY_STATE_RECORD)
							CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_pClipItem->getClipID(), 0);
						else
							CNexProjectManager::sendEvent(MESSAGE_CLIP_PLAY, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, m_pClipItem->getClipID(), 0);
						bEndContent = TRUE;
						break;
				};

				processEnhancedFrame(&uiEnhancement);
				m_isNextFrameRead = FALSE;

#ifdef VIDEO_FRAME_DUMP
				m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
				if( pFileVideoFrame  )
				{
					fwrite(&uiFrameSize, 4, 1, pFileVideoFrame);
					fwrite(pFrame, uiFrameSize , 1, pFileVideoFrame);
				}
#endif
			}

			if( bEndContent )
			{
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			}
			else
			{
				m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
				m_pSource->getVideoFramePTS( &uiPTS, TRUE );

				int iVideoUITime = (int)m_CalcTime.applySpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiPTS);
				NXBOOL bUpdateTime = FALSE;

				if(iVideoUITime > uiVideoEndTimeCur)
				{
					uiIframeModePrev = m_pCurrentRenderInfo->mIframePlay;
					if(checkRenderInfo(uiPTS))
					{

						uiVideoStartTimePrev = uiVideoStartTimeCur;
						uiVideoEndTimePrev = uiVideoEndTimeCur;
						uiVideoBaseTimePrev = uiVideoStartTimePrev;
						uiVideoStartTrimPrev = uiVideoStartTrimCur;
						uiSpeedFactorPrev = m_iSpeedFactor;
						iSkipRenderNumberPrev = iSkipRenderNumber;
						iOperatingFpsPrev = iOperatingFps;

						uiVideoStartTimeCur = m_pCurrentRenderInfo->mStartTime;
						uiVideoEndTimeCur = m_pCurrentRenderInfo->mEndTime;

						uiVideoBaseTimeCur = uiVideoStartTimeCur;
						uiVideoStartTrimCur = m_pCurrentRenderInfo->mStartTrimTime;
						m_iSpeedFactor = m_pCurrentRenderInfo->m_iSpeedCtlFactor;
						iSkipRenderNumber = (curState == PLAY_STATE_RUN)?(m_iFPS * m_iSpeedFactor/100)/61:0;
						iSkipRenderCount		= iSkipRenderNumber;
						if(curState == PLAY_STATE_RUN && m_pCurrentRenderInfo->mIframePlay && bReadyIPlay)
						{
							iSkipRenderNumber = 0;
							iSkipRenderCount = 0;
						}
                        
						iOperatingFps = m_iFPS * m_iSpeedFactor/100;
						uiUpdatedTime = uiPTS;
						bUpdateTime = TRUE;

						bApplyFreezeFrame = false;
						iFreezeDuraion = m_pCurrentRenderInfo->mFreezeDuration;

#ifdef FOR_PROJECT_LGE
						if( m_iFPS >= 240 ){
							iSkipRenderNumber = 0;
						}
#endif
						if(curState == PLAY_STATE_RUN && m_pCurrentRenderInfo->mIframePlay == FALSE && uiIframeModePrev == TRUE)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) set to not iframe mode (%d %d)", __LINE__, m_uiClipID, uiVideoStartTimeCur, uiPTS);						
							bReadyIPlay = FALSE;
							uiSeekResult = (int)m_CalcTime.restoreSpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiVideoStartTimeCur);
							m_pSource->seekToVideoWithAbsoluteTime(uiSeekResult, &uiSeekResult, NXFF_RA_MODE_CUR_PREV);
							m_isNextFrameRead = TRUE;
							uiUpdatedTime = uiCachedPTS + 1;
							continue;                            
						}
					}
				}

				if( m_pCodecWrap->isHardwareCodec() )
				{

					//updateSpeedFactor(uiPTS);

					if( CNexVideoEditor::m_bNexEditorSDK )
					{
						if( curState == PLAY_STATE_RECORD )
						{
#ifdef FOR_PROJECT_LGE
							bLGInterPoly = 1;
							uiDTS = m_CalcTime.applySpeed(uiVideoBaseTime + uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiDTS);
							uiPTS = m_CalcTime.applySpeed(uiVideoBaseTime + uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiPTS);
							if(bUpdateTime)
								uiUpdatedTime = uiPTS;
#else
							if(bSupportFrameTimeChecker && iVideoClipCount == 1)
							{
								if(m_iFPS < 45)
								{
									iMultipleFactor = 200;
									uiDTS = uiDTS/2;
									uiPTS = uiPTS/2;
								}
							}
#endif
						}
						else
						{
							if(iFakePreview)
							{
								//uiDTS = uiFakeLastPTS + (uiDTS - uiFakeStartDTS)* 100 / m_iSpeedFactor*iFakePreview;
								//uiPTS = uiFakeLastPTS + (uiPTS - uiFakeStartDTS)* 100 / m_iSpeedFactor*iFakePreview;
								uiDTS = uiFakeBaseDTS + 1 + (uiDTS - uiFakeStartDTS)/ fFakeInterval;
								uiPTS = uiFakeBaseDTS + 1 + (uiPTS - uiFakeStartDTS)/ fFakeInterval;
							}
							else
							{
								//uiDTS = m_CalcTime.applySpeed(m_iSpeedFactor, uiDTS);
								//uiPTS = m_CalcTime.applySpeed(m_iSpeedFactor, uiPTS);
								uiDTS = m_CalcTime.applySpeed(uiVideoBaseTime + uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiDTS);
								uiPTS = m_CalcTime.applySpeed(uiVideoBaseTime + uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiPTS);
							}
							if(bUpdateTime)
								uiUpdatedTime = uiPTS;
						}
					}
					else
					{
						if( iTimeStampControlForMSMDevice >= 1 )
						{
							if( iTimeStampControlForMSMDevice == 1 )
							{
								iTimeStampControlForMSMDevice = 2;
								iMultipleFactor = 1;
							}
							else
							{
								iMultipleFactor = 2;
								uiDTS = uiDTS * 2;
								uiPTS = uiPTS * 2;
							}
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) read video frame time info (%d %d)", __LINE__, m_uiClipID, uiDTS, uiPTS);
					}
				}
				else
				{
					if( m_iSpeedFactor != 100 )
					{
						//int iSpeedFactor = (m_iSpeedFactor==13? 25: m_iSpeedFactor);
						int iSpeedFactor = m_iSpeedFactor;
						if( iSpeedFactor == 3 || iSpeedFactor == 6 || iSpeedFactor == 13 )
						{
							iSpeedFactor = 25;
						}

						int tmpTime = (uiDTS < uiVideoBaseTime) ? 0 : (uiDTS - uiVideoBaseTime);
						uiDTS = tmpTime * 100 / iSpeedFactor ;

						tmpTime = (uiPTS < uiVideoBaseTime) ? 0 : (uiPTS - uiVideoBaseTime);
						uiPTS = tmpTime  * 100 / iSpeedFactor ;

						if(m_iSpeedFactor == 13)
						{
							uiDTS *= 2;
							uiPTS *= 2;
						}
						else if( m_iSpeedFactor == 6 )
						{
							uiDTS *= 4;
							uiPTS *= 4;
						}
						else if( m_iSpeedFactor == 3 )
						{
							uiDTS *= 8;
							uiPTS *= 8;
						}

						uiDTS +=  uiVideoBaseTime;
						uiPTS +=  uiVideoBaseTime;
					}
				}

				m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) read video frame time info (%d %d)", __LINE__, m_uiClipID, uiDTS, uiPTS);
			}

#if 1		// Not Use Codec Change.

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Frame Info(CTS : %d, %p, %d)", __LINE__, uiCTS, pFrame, uiFrameSize);
			if(	// CNexProjectManager::getCurrentState() == PLAY_STATE_RECORD &&
				m_pCodecWrap->isHardwareCodec() == FALSE &&
				CNexCodecManager::getHardwareMemRemainSize() > (m_iWidth * m_iHeight) )
			{
				bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
				if( bIDRFrame )
				{
					if( reinitVideoDecoder(pFrame, uiFrameSize) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ---------------------", __LINE__);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d)Codec Change Sucessed", __LINE__, m_uiClipID);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ---------------------", __LINE__);

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] read video frame time info (%d %d)", __LINE__, uiDTS, uiPTS);
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ---------------------", __LINE__);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d)Codec Change Failed", __LINE__, m_uiClipID);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ---------------------", __LINE__);
						break;
					}
				}
				else
				{
					if( CNexProjectManager::getCurrentState() == PLAY_STATE_RUN )
					{
						if( isEOSFlag(uiEnhancement) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Not need codec chnaging while preview state because track end", __LINE__, m_uiClipID);
							break;
						}

						m_isNextFrameRead = TRUE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) need codec changing while preview state", __LINE__, m_uiClipID);
						continue;
					}
				}
			}
#endif

			bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
			if( isIDRFrame || (m_pCurrentRenderInfo->mIframePlay && bReadyIPlay))
			{
				if( bIDRFrame == FALSE && bEndContent == FALSE)
				{
					m_isNextFrameRead = TRUE;
					continue;
				}

				isIDRFrame = FALSE;
			}

			// JIRA 2028 issue // KM-1926
			//NXBOOL bBFrame = NexCodecUtil_IsBFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), TRUE);
			NXBOOL bBFrame = 0;
			//int newFPS = m_iFPS * m_iSpeedFactor/100;
			if( iOperatingFps >= 119/*70*/ )
			{
				if( m_pSource->getVideoObjectType() == eNEX_CODEC_V_H264 )
				{
					bBFrame = !NexCodecUtil_AVC_IsAVCReferenceFrame(pFrame, uiFrameSize, m_pSource->getFrameFormat(), iFrameNALHeaderSize);
				}
				else
				{
				        bBFrame = NexCodecUtil_IsBFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), TRUE);
				}
			}
			// fixed KMSA-195 issue
			//if(iFakePreview == 0)
			{
				//if( bBFrame && bEndContent == FALSE && m_iFPS > PREVIEW_BFRAME_CHECK_FPS && CNexProjectManager::getCurrentState() == PLAY_STATE_RUN )
				if( bBFrame && bEndContent == FALSE && CNexProjectManager::getCurrentState() == PLAY_STATE_RUN )
				{
					iSkipRenderCount++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Not Reference B Frame(%d,%d,%d)", __LINE__, m_uiClipID, m_iFPS,uiPTS,iSkipRenderCount);
					m_isNextFrameRead = TRUE;
					continue;
				}
			}

			// nexSAL_MemDump(pFrame, uiFrameSize > 64 ? 64 : uiFrameSize);

			uiCheckDecTime = nexSAL_GetTickCount();
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) DecFrame(%p, %d) CTS(%d)", __LINE__, m_uiClipID, pFrame, uiFrameSize, uiCTS);
			m_perforDecode.CheckModuleUnitStart();

			if(m_pCodecCache)
			{
				if(isEOSFlag(uiEnhancement) && m_pCodecCache->getSize() > 1)
				{
					m_pCodecCache->setNextDecoder(m_uiClipID);
					uiEnhancement |= NEXCAL_VDEC_FLAG_OUTPUT_ONLY;
					m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) go to next clip to decoder %d, %d, %d", __LINE__, m_uiClipID, uiDTS, uiPTS, uiEnhancement);
				}

				if(bSetMyRender == 0 && m_pCodecCache->getMyRenderer(m_uiClipID) == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, m_uLogCnt%5, "[VDTask.cpp %d] ID(%d) Wait CodecCache MyRenderer", __LINE__, m_uiClipID);
					nexSAL_TaskSleep(10);
					uiEnhancement = NEXCAL_VDEC_FLAG_DECODE_ONLY;
				}
				else
				{
					if(bSetMyRender == 0)
						uiEnhancement = 0;
					bSetMyRender = 1;
				}
			}

			nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
											pFrame,
											uiFrameSize,
											NULL,
											uiDTS,
											uiPTS,
											uiEnhancement,
											&uiDecoderErrRet );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, m_uLogCnt++%30, "[VDTask.cpp %d] ID(%d) DecFrame End(%p, %d) DTS(%d) PTS(%d) uiRet(%d) IDR(%d) DecTime(%d)",
				__LINE__, m_uiClipID, pFrame, uiFrameSize, uiDTS, uiPTS, uiDecoderErrRet, bIDRFrame, nexSAL_GetTickCount() - uiCheckDecTime);

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
			{
				m_uiSystemTick = nexSAL_GetTickCount();

				uiDecOutCTS					= 0;
				if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Decode Frame End and next frame", __LINE__);
					// whenever decoding frames, add PTS on the FrameCTSVector

					if(isEOSFlag(uiEnhancement) == FALSE)
					{
						uiCachedPTS = uiPTS;
					}

					if(bSupportFrameTimeChecker)
						m_FrameTimeChecker.addFrameCTSVec(uiDTS, uiPTS, iMultipleFactor);

					m_isNextFrameRead = TRUE;
					m_perforDecode.CheckModuleUnitEnd();

					if(CNexVideoEditor::m_bNexEditorSDK && curState == PLAY_STATE_RUN && uiLevelTick + 500 <  nexSAL_GetTickCount() && isEOSFlag(uiEnhancement) == FALSE)
					{
						if(uDropCountFromGLR > 5)
						{
							CClipVideoRenderInfo* pRenderInfo = NULL;

							unsigned int uiTime = 0, uiTempTime;
							int i = 0;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) uDropCountFromGLR ( %d )", __LINE__, m_uiClipID, uDropCountFromGLR);

							switch(iFakePreview)
							{
								case 0:
									if((pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(CNexProjectManager::getCurrentTimeStamp(), FALSE)) == NULL)
										pRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[m_pClipItem->m_ClipVideoRenderInfoVec.size() - 1];

									uiTempTime = (int)m_CalcTime.restoreSpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, CNexProjectManager::getCurrentTimeStamp());

									if(pRenderInfo->m_iSpeedCtlFactor < m_pCurrentRenderInfo->m_iSpeedCtlFactor)
									{
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Fake Preveiw but speed changed, So try normal ( %d, %d )", __LINE__, m_uiClipID, m_pCurrentRenderInfo->m_iSpeedCtlFactor, pRenderInfo->m_iSpeedCtlFactor);
										break;
									}

									iSkipRenderNumber = 1;
									iFakePreview = 2;

									for( i = 0; i < iSeekTableCount; i++ )
									{
										if( pSeekTable[i] > uiTempTime )
										{
											break;
										}
									}
									iFakeSeekIndex = i;
									uiFakeSpeedFactor = pRenderInfo->m_iSpeedCtlFactor;

									// if(iFakeSeekIndex)
									{
										unsigned int uiTimeFromSeek = 0;
										m_pSource->getVideoFrameDTS( &uiTime, TRUE );
										uiFakeStartDTS = uiTime;
										m_pSource->getVideoFramePTS( &uiTime, TRUE );
										uiFakeStartPTS = uiTime;
										uiFakeLastPTS = CNexProjectManager::getCurrentTimeStamp();
										uiFakeBaseDTS = uiDTS;
										uiFakeBasePTS = uiPTS;

										uiTimeFromSeek = m_pClipItem->getEndTime();
										if(iFakeSeekIndex == iSeekTableCount)
										{
											iFakeDecodingCount = (m_pSource->getTotalPlayVideoTime() - m_pClipItem->getEndTrimTime() - uiTempTime)*m_iFPS/1000/iFakePreview;
										}
										else
										{
											int i = 0;
											iFakeDecodingCount = 0;
											for(i = 0; iFakeSeekIndex + i < iSeekTableCount - 1 && iFakeDecodingCount == 0;i++)
											{
												iFakeDecodingCount = (pSeekTable[iFakeSeekIndex+i] - uiTempTime)*m_iFPS/1000/iFakePreview;
											}

											if((pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(pSeekTable[iFakeSeekIndex+i], TRUE)) == NULL)
												pRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[m_pClipItem->m_ClipVideoRenderInfoVec.size() - 1];

											uiTimeFromSeek = (int)m_CalcTime.applySpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, pSeekTable[iFakeSeekIndex+i]);
											iFakeSeekIndex += i;
										}

										iFakeDecodingCount = (iFakeDecodingCount == 0)?1:iFakeDecodingCount;

										int tempVal1 = (uiTimeFromSeek - uiFakeLastPTS) == 0 ? 1 : uiTimeFromSeek - uiFakeLastPTS;
										fFakeInterval = (float)(1000.0/m_iFPS) / (float)(tempVal1 / iFakeDecodingCount);

										int tempVal2 = (uiTimeFromSeek - uiFakeLastPTS) == 0 ? 1 : uiTimeFromSeek - uiFakeLastPTS;
										iOperatingFps = (1000*iFakeDecodingCount/tempVal2);
										iOperatingFps = (iOperatingFps == 0)?1:iOperatingFps;

										//fFakeInterval = (float)(fFakeInterval - (uiTimeFromSeek - uiFakeLastPTS)) / iFakeDecodingCount;
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Fake Preveiw count ( %d, %d, %d ) %d, %d, %d, %d, %d, %f, %d", __LINE__, m_uiClipID, iFakeDecodingCount, uiTempTime, pSeekTable[iFakeSeekIndex], uiTimeFromSeek, uiFakeLastPTS, uiFakeBasePTS, uiFakeStartDTS, uiFakeStartPTS, fFakeInterval, uiFakeSpeedFactor);
									}
									break;
								case 2:
									iFakePreview = 4;
									iFakeDecodingCount = iFakeDecodedCount - 1;
									break;
							}
						}
						uiLevelTick = nexSAL_GetTickCount();
						uDropCountFromGLR = 0;

						if(iFakePreview > 0)
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Display Level ( %d )", __LINE__, m_uiClipID, iFakePreview);
					}

					if(iFakePreview > 0)
					{
						iFakeDecodedCount++;

						if(iFakeDecodingCount <= iFakeDecodedCount)
						{
							if(iSeekTableCount >= iFakeSeekIndex + 1)
							{
								CClipVideoRenderInfo* pRenderInfo = NULL;
								unsigned int uiTimeFromSeek = 0, uiTimeFromSeekNext;
								NXBOOL bKeepFake = TRUE;

								m_pSource->seekToVideoWithAbsoluteTime(pSeekTable[iFakeSeekIndex], &uiSeekResult, NXFF_RA_MODE_CUR_PREV);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Fake Preveiw seek result( %d e : %d, %d, %d )", __LINE__, m_uiClipID, pSeekTable[iFakeSeekIndex], uiSeekResult, uiDTS, uiPTS );

								uiTimeFromSeek = m_pClipItem->getEndTime();
								pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiSeekResult, TRUE);

								if(pRenderInfo)
								{
									uiTimeFromSeek = (int)m_CalcTime.applySpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, uiSeekResult);
									if(pRenderInfo->m_iSpeedCtlFactor < uiFakeSpeedFactor)
									{
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Fake Preveiw stop and go to normal( %d e : %d, %d )", __LINE__, m_uiClipID, uiTimeFromSeek, uiSeekResult, pRenderInfo->m_iSpeedCtlFactor );
										bKeepFake = FALSE;
										iFakePreview = 0;
										uiUpdatedTime = uiPTS;
										uiFakeUpdatedTime = uiPTS;
									}
								}

								if(bKeepFake)
								{
									unsigned int uiTime = 0;
									m_pSource->getVideoFrameDTS( &uiTime, TRUE );
									uiFakeStartDTS = uiTime;
									m_pSource->getVideoFramePTS( &uiTime, TRUE );
									uiFakeStartPTS = uiTime;
									uiFakeBaseDTS = uiDTS;
									uiFakeBasePTS = uiPTS;

									if(uiTimeFromSeek > uiFakeLastPTS + (uiPTS - uiFakeBasePTS))
									{
										uiFakeLastPTS = uiTimeFromSeek;
									}
									else
									{
										uiFakeLastPTS = uiFakeLastPTS + (uiPTS - uiFakeBasePTS) + 1;
									}

									uiTimeFromSeekNext = m_pClipItem->getEndTime();
									if(iFakeSeekIndex + 1 >= iSeekTableCount)
									{
										iFakeDecodingCount = (m_pSource->getTotalPlayVideoTime() - m_pClipItem->getEndTrimTime() - pSeekTable[iFakeSeekIndex])*m_iFPS/1000/iFakePreview;
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Fake Preveiw count ( count : %d, %d, %d )", __LINE__, m_uiClipID, iFakeDecodingCount, m_pSource->getTotalPlayVideoTime(), pSeekTable[iFakeSeekIndex] );
										iFakeSeekIndex++;
									}
									else
									{
										int i = 0;
										iFakeDecodingCount = 0;
										for(i = 1; iFakeSeekIndex + i < iSeekTableCount - 1 && iFakeDecodingCount == 0;i++)
										{
											iFakeDecodingCount = (pSeekTable[iFakeSeekIndex+i] - pSeekTable[iFakeSeekIndex])*m_iFPS/1000/iFakePreview;
										}

										pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(pSeekTable[iFakeSeekIndex+i], TRUE);
										if(pRenderInfo)
											uiTimeFromSeekNext = (int)m_CalcTime.applySpeed(pRenderInfo->mStartTime, pRenderInfo->mStartTrimTime, pRenderInfo->m_iSpeedCtlFactor, pSeekTable[iFakeSeekIndex + i]);

										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Fake count ( count : %d, %d, %d )", __LINE__, m_uiClipID, iFakeDecodingCount, pSeekTable[iFakeSeekIndex+i], pSeekTable[iFakeSeekIndex] );
											iFakeSeekIndex = iFakeSeekIndex + i;
									}
									iFakeDecodingCount = (iFakeDecodingCount == 0)?1:iFakeDecodingCount;
									int tempVal1 = (uiTimeFromSeekNext - uiTimeFromSeek) == 0 ? 1 : uiTimeFromSeekNext - uiTimeFromSeek;
									fFakeInterval = (float)(1000.0/m_iFPS) / (float)(tempVal1 / iFakeDecodingCount);

									int tempVal2 = (uiTimeFromSeekNext - uiFakeLastPTS) == 0 ? 1 : uiTimeFromSeekNext - uiFakeLastPTS;
									iOperatingFps = (1000*iFakeDecodingCount/tempVal2);
									iOperatingFps = (iOperatingFps == 0)?1:iOperatingFps;
									iFakeDecodedCount = 0;
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Fake Preveiw Updated ( %d, %d, %d ) %d, %d, %d, %d, %f", __LINE__, m_uiClipID, iFakeDecodingCount, uiTimeFromSeekNext, uiTimeFromSeek, uiFakeLastPTS, uiFakeBasePTS, uiFakeStartDTS, uiFakeStartPTS, fFakeInterval );
								}
							}
						}
					}
				}

				if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
				{
					unsigned char*	pY			= NULL;
					unsigned char*	pU			= NULL;
					unsigned char*	pV			= NULL;
					unsigned int 		uiCheckCTS 	= 0;

					pFrameInfo					= NULL;

					if( m_uiCheckVideoDecInit != 0 )
					{
						m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) VideoDecoder_First output( Time : %d )", __LINE__,
							m_uiClipID, m_uiCheckVideoDecInit );
						m_uiCheckVideoDecInit = 0;
					}

					if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
					{
						m_perforGetOutput.CheckModuleUnitStart();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed",__LINE__, m_uiClipID);

							/* delete for mantis 6764
							if( (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getOutFrame End(%d %d)",__LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
								break;
							}
							*/
							continue;
						}
						m_perforGetOutput.CheckModuleUnitEnd();

						unsigned int uiNewPTS = 0;
						// Video split ±â´É »ç¿ëÁß Decoder ¿?·ù·Î Ãß°¡ÇÔ.
						if(!bSupportFrameTimeChecker)
						{
							uiNewPTS = uiDecOutCTS;
						}
						else if(  m_FrameTimeChecker.getValidFrameCTSWithApplyMutiplePTS(uiDecOutCTS, &uiNewPTS, TRUE) == FALSE  )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
#if defined(_ANDROID)
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
							m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
							continue;
						}

						int iUsePrev = 0;
						if(uiUpdatedTime > uiNewPTS)
						{
							if(uiFakeUpdatedTime > uiNewPTS)
								iUsePrev = 1;
							else
								iUsePrev = 2;
						}

						if( CNexVideoEditor::m_bNexEditorSDK )
						{
							unsigned int uiRestoretime = 0;

							uiRestoretime = uiNewPTS;

							if( curState == PLAY_STATE_RUN
#ifdef FOR_PROJECT_LGE
								|| bLGInterPoly
#endif
								)
							{
								if(iFakePreview == FALSE && iUsePrev != 1)
								{
									//uiRestoretime = m_CalcTime.restoreSpeed(m_iSpeedFactor, uiNewPTS);
									if(iUsePrev)
										uiRestoretime = m_CalcTime.restoreSpeed(uiVideoBaseTime + uiVideoBaseTimePrev, uiVideoStartTrimPrev, uiSpeedFactorPrev, uiNewPTS);
									else
										uiRestoretime = m_CalcTime.restoreSpeed(uiVideoBaseTime + uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiNewPTS);
								}
							}

							if(uiVideoEditBox)
							{
								if(uiVideoEditBox > uiRestoretime)
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) drop before EditBox (%d, %d)", __LINE__, m_uiClipID, uiVideoEditBox, uiRestoretime);
#if defined(_ANDROID)
									m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
									m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
									continue;
								}
								uiRestoretime -= uiVideoEditBox;
							}


							int iVideoT = 0;
							if(iFakePreview == FALSE && iUsePrev != 1)
							{
								if(iUsePrev)
									iVideoT = (int)m_CalcTime.applySpeed(uiVideoBaseTimePrev, uiVideoStartTrimPrev, uiSpeedFactorPrev, uiRestoretime);
								else
									iVideoT = (int)m_CalcTime.applySpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiRestoretime);
							}
							else
							{
								iVideoT = uiFakeLastPTS + (uiRestoretime - uiFakeBasePTS);
							}

							nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, m_uLogCnt%30, "[VDTask.cpp %d] ID(%d) GetOutput Time (outT:%d restoreT:%d realT:%d, base:%d trim:%d, spc:%d))",
								__LINE__, m_uiClipID, uiNewPTS, uiRestoretime, iVideoT, uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor);

							if( iVideoT < 0 )
							{
#if defined(_ANDROID)
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
								m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
								continue;
							}

							iDecOutRealTime = iVideoT;
						}
						else
						{
							if(uiVideoEditBox)
							{
								if(uiVideoEditBox > uiNewPTS)
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) drop before EditBox (%d, %d)", __LINE__, m_uiClipID, uiVideoEditBox, uiNewPTS);
#if defined(_ANDROID)
									m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
									m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
									continue;
								}
								uiNewPTS -= uiVideoEditBox;                                
							}

							if(iUsePrev)
							{
								iDecOutRealTime = m_CalcTime.applySpeed(uiVideoBaseTimePrev, uiVideoStartTrimPrev, uiSpeedFactorPrev, uiNewPTS);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput Time (outT:%d realT:%d, base:%d trim:%d, spc:%d))",
									__LINE__, m_uiClipID, uiNewPTS, iDecOutRealTime, uiVideoBaseTimePrev, uiVideoStartTimePrev, uiSpeedFactorPrev);
							}
							else
							{
								iDecOutRealTime = m_CalcTime.applySpeed(uiVideoBaseTimeCur, uiVideoStartTrimCur, m_iSpeedFactor, uiNewPTS);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput Time (outT:%d realT:%d, base:%d trim:%d, spc:%d))",
									__LINE__, m_uiClipID, uiNewPTS, iDecOutRealTime, uiVideoBaseTimeCur, uiVideoStartTimeCur, m_iSpeedFactor);
							}
						}



						// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput *m_pY=0x%08x PTS(%d %d) real PTS(%d)", __LINE__, m_uiClipID, pY, uiDecOutCTS, uiNewPTS, iDecOutRealTime);

						if(uiDropPTS != 0 && uiDropPTS >= iDecOutRealTime)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) drop decoding frame after resume (%d, %d)", __LINE__, m_uiClipID, uiDropPTS, iDecOutRealTime);
	#if defined(_ANDROID)
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
	#elif defined(__APPLE__)
							m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
	#endif
							continue;
						}
						uiDropPTS = 0;

						if( iDecOutRealTime < 0 || m_pClipItem->getStartTime() > iDecOutRealTime || uiCurrentTimeWhenStartTask > (iDecOutRealTime + 5/*GVE-295 + DROP_RENDER_TIME_PREVIEW*/) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame drop before start(Start:%d End:%d real frame:%d cur: %d)",__LINE__,
								m_uiClipID, m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), iDecOutRealTime, uiCurrentTimeWhenStartTask);
	#if defined(_ANDROID)
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
	#elif defined(__APPLE__)
								m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
	#endif
							continue;
						}


						if( 	iDecOutRealTime > m_pClipItem->getEndTime() )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame drop after end(Start:%d End:%d real frame:%d cur: %d)",__LINE__,
								m_uiClipID, m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), iDecOutRealTime, uiCurrentTimeWhenStartTask);
	#if defined(_ANDROID)
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
	#elif defined(__APPLE__)
							m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
	#endif
							if(m_pCodecCache)
							{
								if(m_pCodecCache->getSize() > 1)
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) go to next clip to render %d, %d", __LINE__, m_uiClipID, m_pClipItem->getEndTime(), uiDecOutCTS);
									break;
								}
							}

							continue;
						}



						/* comments related to m_uiDecFrameCountAfterDecInit will be removed.

						if( m_uiDecFrameCountAfterDecInit > 0 )
						{
							callVideoFrameRenderCallback(pY, TRUE);
							m_uiDecFrameCountAfterDecInit--;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame First drop for first black screen (Start:%d End:%d real frame:%d cur: %d) %d",__LINE__,
								m_uiClipID, uiVideoStartTime, uiVideoEndTime, iDecOutRealTime, CNexProjectManager::getCurrentTimeStamp(), m_uiDecFrameCountAfterDecInit);
							continue;
						}
						*/

						if(curState == PLAY_STATE_RUN && uiLastDecOutCTS != 0xffffffff)
						{
							NXBOOL bDrop = FALSE;
							if(iUsePrev == 2 && iSkipRenderNumberPrev)
							{
								if(uiLastDecOutCTS + (1000/iOperatingFpsPrev*iSkipRenderNumberPrev) > iDecOutRealTime)
									bDrop = TRUE;
							}
							else if (iUsePrev == 1 && iSkipRenderNumberPrev)
							{
								if (uiLastDecOutCTS + (1000/iOperatingFps*iSkipRenderNumberPrev) > iDecOutRealTime)
									bDrop = TRUE;
							}
							else
							{
								if(uiLastDecOutCTS + (1000/iOperatingFps*iSkipRenderNumber) > iDecOutRealTime)
									bDrop = TRUE;
							}

							if(bDrop)
							{
#if defined(_ANDROID)
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
								m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame drop for iSkipRenderNumber(Start:%d End:%d real frame:%d iSkipRenderNumber:%d cur: %d)",__LINE__, m_uiClipID, m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), iDecOutRealTime, iSkipRenderNumber, CNexProjectManager::getCurrentTimeStamp());
								continue;
							}
						}

						while( m_bIsWorking && pFrameInfo == NULL)
						{
							if(m_bIsActive == FALSE)
								break;
							pFrameInfo = m_pVideoTrack->getFrameInBuffer();
							if( pFrameInfo == NULL )
							{
								nexSAL_TaskSleep(iOperatingFps<=60 ? 5 : 2);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VDTask.cpp %d] getFrame Input Buffer return null wait buffer", __LINE__);
								continue;
							}
						}

						if( pFrameInfo == NULL )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getFrame Input Buffer return null wait buffer", __LINE__, m_uiClipID);
							// Case : mantis 9240
							// callVideoFrameRenderCallback(pY, FALSE);

							if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiDecOutCTS) != 0 )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed",__LINE__, m_uiClipID);

								/* delete for mantis 6764
								if( (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getOutFrame End(%d %d)",__LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
									break;
								}
								*/
							}
							else
							{
#if defined(_ANDROID)
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
								m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
							}
							continue;
						}

						if( CNexVideoEditor::m_bNexEditorSDK && curState == PLAY_STATE_RUN /*&& iOperatingFps > 120*/)
						{
							int oFps = iUsePrev==2?iOperatingFpsPrev:iOperatingFps;

							if(/*oFps > 120 &&*/ pFrameInfo->m_uiDropRenderTime + iDecOutRealTime < CNexProjectManager::getCurrentTimeStamp() && iFreezeDuraion == 0 )
							{
								uDropCountFromGLR++;
#if defined(_ANDROID)
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#elif defined(__APPLE__)
								m_pCodecWrap->releaseDecodedFrame(pY, FALSE);
#endif
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame drop before GLR(Start:%d End:%d real frame:%d dropRenderTime:%d cur: %d)",__LINE__,
									m_uiClipID, m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), iDecOutRealTime, pFrameInfo->m_uiDropRenderTime, CNexProjectManager::getCurrentTimeStamp());

								continue;
							}
						}

						int applyFreezeTime = iDecOutRealTime;
						if( iFreezeDuraion != 0 )
						{
							if( applyFreezeTime + iFreezeDuraion < uiVideoEndTimeCur )
							{
								if( bApplyFreezeFrame == FALSE )
								{
									bApplyFreezeFrame = TRUE;
								}
								else
								{
									applyFreezeTime += iFreezeDuraion;
								}
							}
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame(%d -> %d) %d %d %d",__LINE__, m_uiClipID, iDecOutRealTime, applyFreezeTime, bApplyFreezeFrame, iFreezeDuraion, uiVideoEndTimeCur);
						}

						pFrameInfo->setVideoFrame(applyFreezeTime, pY, NULL, NULL);
						if( curState == PLAY_STATE_RECORD )
						{
							if(bSupportFrameTimeChecker)
							{
								m_EncodeTimeChecker.addFrameCTSVec(iDecOutRealTime, iDecOutRealTime);
								m_EncodeTimeChecker.removeSmallTime(m_pVideoTrack->m_uiLastEncodedFrameCTS);
								//nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VDTask.cpp %d] ID(%d) m_EncodeTimeChecker %d, %d",__LINE__, m_uiClipID, m_pVideoTrack->m_uiLastEncodedFrameCTS, m_EncodeTimeChecker.getFirstCTSInVec());
							}
						}

						if(curState == PLAY_STATE_RUN && m_pCurrentRenderInfo->mIframePlay && bReadyIPlay == FALSE)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] bReadyIPlay",__LINE__);
							iSkipRenderNumber = 0;
							bReadyIPlay = TRUE;
						}

						uiLastDecOutCTS = iDecOutRealTime;
						/*
						if( m_uiPreviousPTS == -1 )
						{
							pFrameInfo->setVideoFrame(iDecOutRealTime, pY, NULL, NULL);
							m_uiPreviousPTS = iDecOutRealTime;
						}
						else
						{
							pFrameInfo->setVideoFrame(m_uiPreviousPTS, pY, NULL, NULL);
							m_uiPreviousPTS = iDecOutRealTime;
						}
						*/
					}

					if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420 )
					{
						m_perforGetOutput.CheckModuleUnitStart();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, &pU, &pV, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] YUV nexCAL_VideoDecoderGetOutput failed",__LINE__);
							continue;
						}
						m_perforGetOutput.CheckModuleUnitEnd();

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput *m_pY=0x%08x CTS(%d)",
							__LINE__, m_uiClipID, *pFrameInfo->m_pY, uiDecOutCTS);
						// Video split ±â´É »ç¿ëÁß Decoder ¿?·ù·Î Ãß°¡ÇÔ.
						if( (int)uiDecOutCTS < 0 || uiDecOutCTS == 2  ||
							( isEOSFlag(uiEnhancement)  == FALSE && (!bSupportFrameTimeChecker || m_FrameTimeChecker.isValidFrameCTS(uiDecOutCTS, TRUE) == FALSE )) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
							continue;
						}

						if(	m_pClipItem->getStartTime() > uiDecOutCTS || uiDecOutCTS > m_pClipItem->getEndTime() )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame drop(Start:%d End:%d frame:%d cur: %d",__LINE__,
								m_uiClipID, m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), uiDecOutCTS, CNexProjectManager::getCurrentTimeStamp());
							continue;
						}

						pFrameInfo->setVideoFrame(uiDecOutCTS, pY, pU, pV);
					}

					if( m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NV12 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VDTask.cpp %d] ID(%d) GetOutput CTS(%d)", __LINE__, m_uiClipID, uiDecOutCTS);
						m_perforGetOutput.CheckModuleUnitStart();
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pFrameInfo->m_pY, NULL, NULL, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) YUV nexCAL_VideoDecoderGetOutput failed",__LINE__, m_uiClipID);
							continue;
						}
						m_perforGetOutput.CheckModuleUnitEnd();
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput *m_pY=0x%08x CTS(%d)",
							__LINE__, m_uiClipID, pFrameInfo->m_pY, uiDecOutCTS);

						// Video split ±â´É »ç¿ëÁß Decoder ¿?·ù·Î Ãß°¡ÇÔ.
						if( (int)uiDecOutCTS < 0 || uiDecOutCTS == 2  ||
							( isEOSFlag(uiEnhancement)  == FALSE && (!bSupportFrameTimeChecker || m_FrameTimeChecker.isValidFrameCTS(uiDecOutCTS, TRUE) == FALSE )) )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput invalid Time (uiCTS:%d Frame:%d)", __LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
							continue;
						}

						if(	m_pClipItem->getStartTime() > uiDecOutCTS || uiDecOutCTS > m_pClipItem->getEndTime() )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Video Frame drop(Start:%d End:%d frame:%d cur: %d",__LINE__,
								m_uiClipID, m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), uiDecOutCTS, CNexProjectManager::getCurrentTimeStamp());
							continue;
						}

	#ifdef VIDEO_RESULT_DUMP
						if( pFileVideoDump && uiDecOutCTS > 6000 )
						{
							fwrite(pFrameInfo->m_pY, m_iPitch*m_iHeight*3/2 , 1, pFileVideoDump);
						}
	#endif
						pFrameInfo->setVideoFrame(uiDecOutCTS, NULL, NULL, NULL);
					}

					uiCheckCTS = uiDecOutCTS;
					m_pVideoTrack->EnqueueFrameInBuffer();

					// ?ç»ý ¼Óµµ ´Ê´Â ¹®Á¦ ¼öÁ¤
					if( (uiDecOutCTS & 0x80000000 ) == 0x80000000 )
					{
						if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
						{
							m_isNextFrameRead = TRUE;
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%D) Skip current frame because invalid frame CTS(%d)", __LINE__, m_uiClipID, uiDecOutCTS);
						continue;
					}
				}
				else
				{
					// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) GetOutput did not exist", __LINE__, m_uiClipID);
					nexSAL_TaskSleep(1);
				}

				if( m_pCodecWrap->isHardwareCodec() && NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Received EOS from decoder", __LINE__, m_uiClipID);
					break;
				}

				if(m_pCodecCache)
				{
					if(isEOSFlag(uiEnhancement) && m_pCodecCache->getSize() > 1)
					{
						if(uiDecOutCTS >= uiCachedPTS)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) go to next clip to render %d, %d", __LINE__, m_uiClipID, uiCachedPTS, uiDecOutCTS);
							break;
						}
					}
				}
			}
			else
			{
				if( m_pCodecWrap->isHardwareCodec() && NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Received EOS from decoder", __LINE__, m_uiClipID);
					break;
				}

				NXBOOL bNeedMoreDecode = FALSE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Decoding Failed(%d)",__LINE__, m_uiClipID, uiDecoderErrRet);

				if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_NEXT_FRAME) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Decode Frame Failed and next frame", __LINE__, m_uiClipID);
					m_isNextFrameRead = TRUE;
					bNeedMoreDecode = TRUE;
				}

				if(NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_TIMEOUT))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Decode Frame Failed (Output timeout)", __LINE__, m_uiClipID);
					bNeedMoreDecode = TRUE;
				}

				if( bNeedMoreDecode == FALSE )
				{
					CNexProjectManager::sendEvent(MESSAGE_VDEC_DEC_FAIL, CNexProjectManager::getCurrentState(), NEXVIDEOEDITOR_ERROR_CODEC_DECODE, m_uiClipID, 1);
					break;
				}
				nexSAL_TaskSleep(1);
			}

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Check VideoEnd(%d %d %d)",__LINE__, uiCTS, uiDecOutCTS, (uiEnhancement & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM);
			if( (uiDTS <= uiDecOutCTS) && isEOSFlag(uiEnhancement) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) getOutFrame End(%d %d)",__LINE__, m_uiClipID, uiDTS, uiDecOutCTS);
				break;
			}
			m_uiOldTimeStamp = m_uiCurrentTime;
		}
		else
		{
			// nexSAL_TaskSleep(10);

			if(m_bIsTaskPaused == FALSE)
			{
				if (m_pVideoTrack && !m_pVideoTrack->getVideoDecodeEnd())
				{
					m_pVideoTrack->setPause();
					m_pVideoTrack->DequeueAllFrameOutBuffer();
				}

				m_pCodecWrap->deinitDecoder();
				m_bIsTaskPaused = TRUE;
				m_bIsTaskResumed = FALSE;

				if( curState == PLAY_STATE_RECORD )
				{
					if(bSupportFrameTimeChecker)
					{
						if(m_EncodeTimeChecker.size())
						{
							if(m_pVideoTrack->m_uiLastEncodedFrameCTS == 0)
							{
								m_uiLastEncOutCTS = 0;
							}
							else
							{
								m_EncodeTimeChecker.removeSmallTime(m_pVideoTrack->m_uiLastEncodedFrameCTS);
								m_uiLastEncOutCTS = m_EncodeTimeChecker.getFirstCTSInVec();
							}
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] ID(%d) Video Task pause  m_EncodeTimeChecker %d,  %d!\n", __LINE__, m_uiClipID, m_pVideoTrack->m_uiLastEncodedFrameCTS, m_uiLastEncOutCTS);
						}
					}
				}
			}

			if( m_bIsTaskPaused && m_hSema != NEXSAL_INVALID_HANDLE )
			{
				m_isSemaState = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] ID(%d) Video Task Semaphore Wait!\n", __LINE__, m_uiClipID);
				nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask %d] ID(%d) Video Task PauseMode Semaphore is NULL!!!\n", __LINE__, m_uiClipID);
			}
		}
	}

	if( pSeekTable )
	{
		nexSAL_MemFree(pSeekTable);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Task End while", __LINE__, m_uiClipID);

	// + mantis 7722
	if( m_pVideoTrack )
		m_pVideoTrack->setVideoDecodeEnd();
	// - mantis 7722

	m_pClipItem->setVideoDecodingEnd(TRUE);

	if(m_pCodecCache)
	{
		if(m_pCodecCache->getSize() > 1)
		{
			m_pCodecCache->setNextDecoder(m_uiClipID);
			m_pCodecCache->setNextRenderer(m_uiClipID);
		}
	}

	CNexDrawInfoVec* pDrawInfo = m_pClipItem->getDrawInfos();
	if( pDrawInfo != NULL )
	{
		while( m_bIsWorking && m_pVideoRenderer != NULL && pDrawInfo->isDrawTime(m_pVideoRenderer->getCurrentTime()) )
		{
			nexSAL_TaskSleep(10);
		}
		SAFE_RELEASE(pDrawInfo);
	}

	if( CNexProjectManager::getCurrentState() == PLAY_STATE_RECORD )
	{
		unsigned int iWaitTime = 2000;

		if(m_pCurrentRenderInfo->mIframePlay)
			iWaitTime = 0xffffffff;
		
		while ( m_bIsWorking && iWaitTime > 0 && m_pVideoRenderer != NULL && m_pVideoTrack != NULL && m_pClipItem != NULL )
		{
			if(m_bIsActive)
			{
				if (m_bIsTaskResumed == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] ID(%d) VideoDecTask RESUMED!!!\n",  __LINE__, m_uiClipID);
					m_bIsTaskResumed = TRUE;
					m_bIsTaskPaused = FALSE;
				}

				if( m_pVideoRenderer->getCurrentTime() < m_pClipItem->getEndTime() )
				{
					nexSAL_TaskSleep(10);
					iWaitTime--;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Wait Video Renderer",__LINE__, m_uiClipID);
					continue;
				}

				if( m_pVideoTrack->isTrackEnded() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Frame Draw End",__LINE__, m_uiClipID);
					break;
				}
				nexSAL_TaskSleep(20);
				iWaitTime -= 20;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Wait Video Frame Draw",__LINE__, m_uiClipID);
			}
			else
			{
				if(m_bIsTaskPaused == FALSE)
				{
					m_bIsTaskPaused = TRUE;
					m_bIsTaskResumed = FALSE;
				}

				if( m_bIsTaskPaused && m_hSema != NEXSAL_INVALID_HANDLE )
				{
					m_isSemaState = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] ID(%d) Video Task Semaphore Wait!\n", __LINE__, m_uiClipID);
					nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask %d] ID(%d) Video Task PauseMode Semaphore is NULL!!!\n", __LINE__, m_uiClipID);
				}
			}
		}
	}
	else
	{
		int iWaitTime = 2000;
		while ( m_bIsWorking && iWaitTime > 0 && m_pVideoTrack != NULL )
		{
			if(m_bIsActive)
			{
				if (m_bIsTaskResumed == FALSE)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] VideoDecTask RESUMED!!!\n",  __LINE__);
					m_bIsTaskResumed = TRUE;
					m_bIsTaskPaused = FALSE;
				}

				if( m_pVideoTrack->isTrackEnded() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Frame Draw End",__LINE__, m_uiClipID);
					break;
				}
				nexSAL_TaskSleep(20);
				iWaitTime -= 20;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Wait Video Frame Draw",__LINE__, m_uiClipID);
			}
			else
			{
				if(m_bIsTaskPaused == FALSE)
				{
					m_bIsTaskPaused = TRUE;
					m_bIsTaskResumed = FALSE;
				}

				if( m_bIsTaskPaused && m_hSema != NEXSAL_INVALID_HANDLE )
				{
					m_isSemaState = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask %d] Video Task Semaphore Wait!\n", __LINE__);
					nexSAL_SemaphoreWait(m_hSema, NEXSAL_INFINITE);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask %d] Video Task PauseMode Semaphore is NULL!!!\n", __LINE__);
				}
			}
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Task Wait Track ending", __LINE__, m_uiClipID);
	
	m_bThreadFuncEnd = TRUE;
	
    // call sequence was changed by eric.
    // 2016.7.21
	deregistTrack();
    
	if( m_pVideoTrack )
		m_pVideoTrack->setCodecWrap(NULL);

	if(m_pCodecCache == NULL)
	{	
		deinitVideoDecoder();	
	}
	else 
	{
		m_pCodecCache->removeCodecTimeVec(m_uiClipID);		
		if(m_pCodecCache->getSize() == 0)
		{
			clearCachedVisualCodecInfo(m_pCodecCache);
			deinitVideoDecoder();	
		}    
		else
		{
			if( m_pVideoTrack )
				m_pVideoTrack->setReUseSurfaceTexture(TRUE);
		}
	}

	SAFE_RELEASE(m_pVideoTrack);

#ifdef VIDEO_RESULT_DUMP
	if( pFileVideoDump )
	{
		fclose(pFileVideoDump);
		pFileVideoDump = NULL;
	}
#endif

#ifdef VIDEO_FRAME_DUMP
	if( pFileVideoFrame )
	{
		fclose(pFileVideoFrame);
		pFileVideoFrame = NULL;
	}
#endif
	m_pSurfaceTexture = NULL;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Task End and Release Codec", __LINE__, m_uiClipID);
	return 0;
}

NXBOOL CNEXThread_VideoTask::setClipItem(CClipItem* pClipItem)
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
	}
	return TRUE;
}

NXBOOL CNEXThread_VideoTask::setSource(CNexSource* pSource)
{
	if( pSource == NULL )
	{
		return FALSE;
	}
	deinitVideoDecoder();
	SAFE_RELEASE(m_pSource);
	SAFE_ADDREF(pSource);
	m_pSource = pSource;

	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;

	m_iWidth	= CNexVideoEditor::m_iSupportedWidth;
	m_iHeight	= CNexVideoEditor::m_iSupportedHeight;
	m_iPitch	= CNexVideoEditor::m_iSupportedWidth;
    
    if( m_pSource != NULL )
    {
        if( m_pSource->getVideoResolution(&uiWidth, &uiHeight) == NEXVIDEOEDITOR_ERROR_NONE )
        {
            m_iWidth		= (int)uiWidth;
            m_iPitch		= (int)uiWidth;
            m_iHeight       = (int)uiHeight;
        }
        
        m_iFPS = m_pSource->getVideoFrameRate();
    }

	return TRUE;
}

NXBOOL CNEXThread_VideoTask::setFileWriter(CNexExportWriter* pFileWriter)
{
	if( pFileWriter == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pFileWriter);
	m_pFileWriter = pFileWriter;
	SAFE_ADDREF(m_pFileWriter);

	return TRUE;
}

NXBOOL CNEXThread_VideoTask::setVideoRenderer(CNEXThread_VideoRenderTask* pVideoRenderer)
{
	if( pVideoRenderer == NULL ) return FALSE;

	SAFE_ADDREF(pVideoRenderer);
	SAFE_RELEASE(m_pVideoRenderer);
	m_pVideoRenderer = pVideoRenderer;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d]setVideoRenderer(%p)", __LINE__, m_pVideoRenderer);	
	return TRUE;
}

void CNEXThread_VideoTask::processEnhancedFrame(unsigned int* pEnhancement)
{
	if( m_pSource->getEnhancedExist())
	{
		unsigned int uiBaseTS		= 0;
		unsigned int uiEnhanceTS	= 0;
		m_pSource->getTS( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &uiBaseTS );
		m_pSource->getTS( NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, &uiEnhanceTS );
		if( uiBaseTS > uiEnhanceTS )
		{
			*pEnhancement |= NEXCAL_VDEC_FLAG_ENHANCEMENT;
			m_pSource->getVideoFrame(TRUE);
		}
	}
}

CCodecCacheInfo* CNEXThread_VideoTask::getCachedVisualCodecInfo()
{
	CNexProjectManager* pProject = CNexProjectManager::getProjectManager();
	CCodecCacheInfo* pCodecCache = NULL;
	NXBOOL bReUse = FALSE;

	if(m_pClipItem->getClipType() == CLIPTYPE_VIDEO_LAYER)
		return NULL;

	unsigned int uiDTS = 0, uiStartTime = 0; 
	m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
	uiStartTime = (int)m_CalcTime.restoreSpeed(m_pClipItem->getStartTime(), m_pClipItem->getStartTrimTime(), m_iSpeedFactor, m_pClipItem->getStartTime());

	if(uiStartTime > uiDTS + 300)
	{
		return NULL;
	}

	if( pProject )
	{
		int uSize = pProject->getCachedVisualCodecSize();

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] getCachedVisualCodecInfo() %d", __LINE__, uSize);

		for( int i = 0; i < uSize; i++ )
		{
			NXUINT8*		pBaseDSI		= NULL;
			NXUINT32		uiBaseDSISize	= 0;
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			pCodecCache = pProject->getCachedVisualCodecInfo(i);
			if((bReUse = pCodecCache->checkReUseCodec(m_uiClipID, (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), m_pClipItem->getStartTime(), m_pClipItem->getEndTime(), pBaseDSI, uiBaseDSISize)))
			{
				SAFE_ADDREF(pCodecCache);
				break;					
			}
		}
		SAFE_RELEASE(pProject);
	}
    
	if(bReUse == FALSE)
		pCodecCache = NULL;
	return pCodecCache;
}

void CNEXThread_VideoTask::setCachedVisualCodecInfo(CCodecCacheInfo* pCodecCache)
{
	CNexProjectManager*pProject = CNexProjectManager::getProjectManager();
	if( pProject )
	{
		pProject->setCachedVisualCodecInfo(pCodecCache);
		SAFE_RELEASE(pProject);
	}	
}

void CNEXThread_VideoTask::clearCachedVisualCodecInfo(CCodecCacheInfo* pCodecCache)
{
	CNexProjectManager*pProject = CNexProjectManager::getProjectManager();
	if( pProject )
	{
		pProject->clearCachedVisualCodecInfo(pCodecCache);
		SAFE_RELEASE(pProject);
	}	
}

NXBOOL CNEXThread_VideoTask::initCachedDecoder()
{
	CAutoLock m(CNexCodecManager::getVideoDecInitLock());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%D) initCachedDecoder() In", __LINE__, m_uiClipID);
	if( m_pSource == NULL )
		return FALSE;

	if( m_pSource->isVideoExist() == FALSE )
		return FALSE;

	if( m_bIsWorking == FALSE )
	{
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Task exit before video decoder init", __LINE__, m_uiClipID);
		return TRUE;
	}

	NXUINT8*		pFrame			= NULL;
	NXUINT32		uiFrameSize		= 0;
	NXUINT32		uiDTS			= 0;
	NXUINT32		uiPTS			= 0;
	NXBOOL			isValidVideo	= FALSE;
	NXBOOL			isIntraFrame	= FALSE;

	NXUINT32		uiRetryCnt		= 0;
	NXUINT8*		pBaseDSI		= NULL;
	NXUINT32		uiBaseDSISize	= 0;
	NXUINT8*		pNewConfig		= NULL;
	NXUINT32		uiNewConfigSize	= 0;
	NXUINT8*		pEnhancedDSI			= NULL;
	NXUINT32		uiEnhancedDSISize		= 0;
	NXUINT32		uiH264ProfileLevelID	= 0;

	unsigned int	eRet 					= 0;
	int 			iDSINALHeaderSize		= 0;
	int 			iFrameNALHeaderSize		= 0;

	unsigned int	iWidth					= 0;
	unsigned int	iHeight					= 0;
	void*			pSurfaceTexture			= NULL;
	
	IRectangle*	pRec						= NULL;
	
	float* pMatrix = NULL;
	int iMatrixSize = 0;

	int iHdrType = 0;
	NEXCODECUTIL_SEI_HDR_INFO stHdrInfo;

	unsigned char* puuid = NULL;
	int uuid_size = 0;
	int is360video = 0;

	memset( &stHdrInfo, 0, sizeof(NEXCODECUTIL_SEI_HDR_INFO));

	CClipVideoRenderInfo videoInfo;
	videoInfo.mFaceDetectProcessed = 1;

	CClipVideoRenderInfoVectorIterator i;
	CClipVideoRenderInfo* pRenderInfo;

	iDSINALHeaderSize = m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

	pSurfaceTexture = m_pCodecWrap->getSurfaceTexture();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Use surfaceTexture(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
    
RETRY_INIT_DECODER:
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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Need buffering Not normal condition(0x%x)", __LINE__, m_uiClipID, uiRet);
				continue;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Frame fail while decoder init(0x%x)", __LINE__, m_uiClipID, uiRet);
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) This Video frame is invalid", __LINE__, m_uiClipID);
			nexSAL_TaskSleep(20);
			m_isNextFrameRead = TRUE;
			continue;
		}
		// Codec ÃÊ±âÈ­½Ã IDR ÇÁ·¹?Ó?» Âü°í ÇÏÁö ¾Ê¾Æ IFrame?» Ã£µµ·Ï ÇÔ.

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Check I-frame CTS(%u) PTS(%u) DSINALSzie(%d) FrameNALSize(%d)", __LINE__, m_uiClipID, uiDTS, uiPTS,  iDSINALHeaderSize, iFrameNALHeaderSize);
		isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
		if( isIntraFrame )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) I-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
			break;
		}
		
		m_isNextFrameRead = TRUE;			
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%D) P-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
	}

	//yoon
	m_pSource->getVideoTrackUUID(&puuid,&uuid_size);
	is360video = CVideoTrackInfo::is360Video(puuid,uuid_size);
	
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iWidth	= iWidth;
			m_iHeight	= iHeight;
			m_iPitch	= iWidth;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_uiClipID, m_iWidth, m_iHeight, m_iPitch );
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iWidth	= iWidth;
			m_iHeight	= iHeight;
			m_iPitch	= iWidth;

//yoon
			iHdrType = CVideoTrackInfo::getHDRMetaData(m_pSource->getVideoObjectType(),	pBaseDSI , uiBaseDSISize
				, pFrame, uiFrameSize, iFrameNALHeaderSize
				,&stHdrInfo);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_uiClipID, m_iWidth, m_iHeight, m_iPitch );
			break;

		default:
			goto DECODER_INIT_ERROR;
	};

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	m_pVideoTrack = new CVideoTrackInfo;
	if( m_pVideoTrack == NULL )
	{
		goto DECODER_INIT_ERROR;
	}

	videoInfo.muiTrackID		= m_pClipItem->getClipID();
	videoInfo.mClipType			= m_pClipItem->getClipType();

	videoInfo.mStartTime		= m_pClipItem->getStartTime();
	videoInfo.mEndTime			= m_pClipItem->getEndTime();

	videoInfo.mWidth			= m_iWidth;
	videoInfo.mHeight			= m_iHeight;
	videoInfo.mPitch			= m_iPitch;
	videoInfo.mColorFormat		= m_pCodecWrap->getFormat();
	videoInfo.mIsPreview		= FALSE;
	videoInfo.mRotateState		= m_pClipItem->getRotateState();
	
	videoInfo.mBrightness		= m_pClipItem->getBrightness();
	videoInfo.mContrast			= m_pClipItem->getContrast();
	videoInfo.mSaturation		= m_pClipItem->getSaturation();
	videoInfo.mTintcolor		= m_pClipItem->getTintcolor();
	videoInfo.mLUT				= m_pClipItem->getLUT();
	videoInfo.mVignette 		= m_pClipItem->getVignette();
	
	videoInfo.mVideoRenderMode =  is360video; //m_pClipItem->getVideoRenderMode();
	videoInfo.mHDRType =  iHdrType;
	memcpy(&videoInfo.mHdrInfo, &stHdrInfo, sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon

	pRec =  m_pClipItem->getStartPosition();
	if( pRec != NULL )
	{
		videoInfo.mStart.left		= pRec->getLeft();
		videoInfo.mStart.top		= pRec->getTop();
		videoInfo.mStart.right		= pRec->getRight();
		videoInfo.mStart.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pRec =  m_pClipItem->getEndPosition();
	if( pRec != NULL )
	{
		videoInfo.mEnd.left			= pRec->getLeft();
		videoInfo.mEnd.top			= pRec->getTop();
		videoInfo.mEnd.right		= pRec->getRight();
		videoInfo.mEnd.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pMatrix = m_pClipItem->getStartMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mStartMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}

	pMatrix = m_pClipItem->getEndMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mEndMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}	
	
	if( m_pVideoTrack->setClipRenderInfo(&videoInfo) == FALSE )
	{
		goto DECODER_INIT_ERROR;
	}

	if( m_pCodecWrap->isHardwareCodec() &&
		pSurfaceTexture && 
		m_pVideoTrack->setSurfaceTexture(pSurfaceTexture) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Regist surfacetexture failed(%p)", __LINE__, pSurfaceTexture);
		goto DECODER_INIT_ERROR;
	}

	m_pVideoTrack->setTrackIndexInfo(m_pClipItem->getClipIndex(), m_pClipItem->getClipTotalCount());
	m_pVideoTrack->setCodecWrap(m_pCodecWrap);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] m_ClipVideoRenderInfoVec.size (%d, %zu)\n", __LINE__,  m_pClipItem->getClipID(),m_pClipItem->m_ClipVideoRenderInfoVec.size());
	m_pVideoTrack->clearClipRenderInfoVec();
	for(i = m_pClipItem->m_ClipVideoRenderInfoVec.begin(); i != m_pClipItem->m_ClipVideoRenderInfoVec.end(); i++)
	{
		pRenderInfo = ((CClipVideoRenderInfo*)(*i));
		m_pVideoTrack->setClipRenderInfoVec(pRenderInfo);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setEffect(%p, %d)\n", __LINE__,  m_pClipItem, m_pClipItem->getEndTime());
	if( m_pClipItem )
	{
		if( m_pClipItem->getClipEffectEnable() )
		{
			char* pEffect = (char*)m_pClipItem->getClipEffectID();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setEffect(%s)\n", __LINE__, pEffect);
			if( pEffect )
			{
				m_uiEffectStartTime = m_pClipItem->getEndTime() - (m_pClipItem->getClipEffectDuration() * m_pClipItem->getClipEffectOffset() / 100);
				m_pVideoTrack->setEffectInfo(	m_uiEffectStartTime, 
											m_pClipItem->getClipEffectDuration(), 
											m_pClipItem->getClipEffectOffset(), 
											m_pClipItem->getClipEffectOverlap(),
											pEffect);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setEffect(%p)\n", __LINE__, pEffect);
			}
		}

		// set title for Effect option, mantis 5524
		char* pTitleEffect =(char*)m_pClipItem->getTitleEffectID();
		m_pVideoTrack->setTitleInfo(	m_pClipItem->getTitle(),
									pTitleEffect,
									m_pClipItem->getTitleStartTime(),
									m_pClipItem->getTitleEndTime());

		CNexDrawInfoVec* pDrawInfos = m_pClipItem->getDrawInfos();
		m_pVideoTrack->setDrawInfos(pDrawInfos);
		SAFE_RELEASE(pDrawInfos);

		m_pVideoTrack->setMotionTracked(m_pClipItem->getMotionTracked());

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setTitleInfo(%s, %d %d)", 
			__LINE__, pTitleEffect, m_pClipItem->getTitleStartTime(), m_pClipItem->getTitleEndTime());
	}
	
	if( registTrack() == FALSE )
	{
		goto DECODER_INIT_ERROR;
	}
	m_isNextFrameRead = FALSE;

	return TRUE;

DECODER_INIT_ERROR:

	if( pSurfaceTexture )
	{
		m_pVideoRenderer->releaseSurfaceTexture(pSurfaceTexture);
		pSurfaceTexture = NULL;
	}

	m_isNextFrameRead = FALSE;
	deinitVideoDecoder();
	return FALSE;
}

NXBOOL CNEXThread_VideoTask::initVideoDecoder()
{
	CAutoLock m(CNexCodecManager::getVideoDecInitLock());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%D) initVideoDecoder() In", __LINE__, m_uiClipID);
	if( m_pSource == NULL )
		return FALSE;

	if( m_pSource->isVideoExist() == FALSE )
		return FALSE;

	SAFE_RELEASE(m_pCodecWrap);
	m_pCodecWrap = new CNexCodecWrap();

	NXBOOL bUseSoftCodec = FALSE;
	if( canUseSWDecoder() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) PlayTime(%d) with SW Decoder", __LINE__, m_uiClipID, m_pClipItem->getTotalTime() );
		bUseSoftCodec = TRUE;
	}
	
	if( bUseSoftCodec == FALSE && canUseSWDecoder() )
	{
		// for decoder max memory limitation while exporting about mantis 6363
		// for Mantis 8989 problem
		// for ZIRA 1675 issue.
		if(	m_iWidth * m_iHeight > CNexCodecManager::getHardwareMemRemainSize() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) have to use sw video codec because codec not available", __LINE__, m_uiClipID);
			bUseSoftCodec = TRUE;
		}
	}
    
	if( bUseSoftCodec == FALSE )
	{
        if( CNexVideoEditor::m_bNexEditorSDK &&
            (m_iWidth * m_iHeight >= 1920*1080) &&
            (m_iFPS > 30) &&
            CNexProjectManager::getCurrentState() == PLAY_STATE_RECORD )
        {
            // TODO asdf
            while( m_bIsWorking )
            {
            	// for KMSA-295
            	if( CNexCodecManager::canUseHardwareDecode(m_iWidth, m_iHeight) )
                // if( CNexCodecManager::getHardwareDecoderUseCount() == 0 )
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Now available hardware codec", __LINE__, m_uiClipID);
                    break;
                }
                nexSAL_TaskSleep(30);
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Wait available hardware codec", __LINE__, m_uiClipID);
            }
        }
        else
        {
            // TODO asdf
            while( m_bIsWorking )
            {
                if( CNexCodecManager::isHardwareDecodeAvailable(m_iWidth, m_iHeight) )
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Now available hardware codec", __LINE__, m_uiClipID);
                    break;
                }
                nexSAL_TaskSleep(30);
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Wait available hardware codec", __LINE__, m_uiClipID);
            }
        }
	}

	if( m_bIsWorking == FALSE )
	{
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Task exit before video decoder init", __LINE__, m_uiClipID);
		return TRUE;
	}

	unsigned int uiVideoType = m_pSource->getVideoObjectType();
	if(  bUseSoftCodec )
	{
		// Use Software Codec;
		if( uiVideoType  == eNEX_CODEC_V_MPEG4V )
		{
			uiVideoType = eNEX_CODEC_V_MPEG4V_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Used Mpeg4V Software Decoder(0x%x)", __LINE__, m_uiClipID, uiVideoType);
		}
		else if( uiVideoType  == eNEX_CODEC_V_H264 )
		{
			uiVideoType = eNEX_CODEC_V_H264_S;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Used AVC Software Decoder(0x%x)", __LINE__, m_uiClipID, uiVideoType);
		}
		else
		{
			SAFE_RELEASE(m_pCodecWrap);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Not Support Video Codec(0x%x)", __LINE__, m_uiClipID, uiVideoType);
			return FALSE;
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Used Hardware Decoder(0x%x)", __LINE__, m_uiClipID, uiVideoType);
	}

	NXBOOL bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
											NEXCAL_MODE_DECODER,
											uiVideoType,
											m_iWidth,
											m_iHeight,
											m_iWidth);

	if( bRet == FALSE )
	{
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec Failed(0x%x)", __LINE__, m_uiClipID, uiVideoType);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec end(%p)", __LINE__, m_uiClipID, uiVideoType, m_pCodecWrap->getCodecHandle());

	NXUINT8*		pFrame			= NULL;
	NXUINT32		uiFrameSize		= 0;
	NXUINT32		uiDTS			= 0;
	NXUINT32		uiPTS			= 0;
	NXBOOL			isValidVideo	= FALSE;
	NXBOOL			isIntraFrame	= FALSE;

	NXUINT32		uiRetryCnt		= 0;
	NXUINT8*		pBaseDSI		= NULL;
	NXUINT32		uiBaseDSISize	= 0;
	NXUINT8*		pNewConfig		= NULL;
	NXUINT32		uiNewConfigSize	= 0;
	NXUINT8*		pEnhancedDSI			= NULL;
	NXUINT32		uiEnhancedDSISize		= 0;
	NXUINT32		uiH264ProfileLevelID	= 0;

	unsigned int	eRet 					= 0;
	int 			iDSINALHeaderSize		= 0;
	int 			iFrameNALHeaderSize		= 0;

	unsigned int	iWidth					= 0;
	unsigned int	iHeight					= 0;
	void*			pSurfaceTexture			= NULL;
	
	IRectangle*	pRec						= NULL;
	
	float* pMatrix = NULL;
	int iMatrixSize = 0;

	int iHdrType = 0;
	NEXCODECUTIL_SEI_HDR_INFO stHdrInfo;

	unsigned char* puuid = NULL;
	int uuid_size = 0;
	int is360video = 0;

	memset( &stHdrInfo, 0, sizeof(NEXCODECUTIL_SEI_HDR_INFO));

	CClipVideoRenderInfo videoInfo;
	//memset(&videoInfo, 0x00, sizeof(CClipVideoRenderInfo));

	videoInfo.mFaceDetectProcessed = 1;

	CClipVideoRenderInfoVectorIterator i;
	CClipVideoRenderInfo* pRenderInfo;

	NXINT64		isUseIframeVDecInit			= 0;
	if( nexCAL_VideoDecoderGetProperty( m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	iDSINALHeaderSize = m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

	if( m_pVideoRenderer && m_pCodecWrap->isHardwareCodec() )
	{
#ifdef _ANDROID
		pSurfaceTexture = m_pVideoRenderer->getSurfaceTexture();
		if( pSurfaceTexture != NULL )
		{
			m_pCodecWrap->setSurfaceTexture(pSurfaceTexture);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Use surfaceTexture(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
		}
		// for JIRA 2971
		else
		{
			m_pCodecWrap->deinitDecoder();
			SAFE_RELEASE(m_pCodecWrap);
#ifdef FOR_TEST_PREPARE_VIDEO
			if( CNexVideoEditor::m_bPrepareVideoCodec )
			{
				CNexCodecManager::clearPrepareHardwareDecoderFlag();
			}	
#endif			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec dec surface failed", __LINE__, m_uiClipID);
			return FALSE;
		}
#endif
	}

	if( m_pCodecWrap->isHardwareCodec() == FALSE )
	{
		nexCAL_VideoDecoderSetProperty( m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_VIDEO_BUFFER_NV12);
	}

RETRY_INIT_DECODER:
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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Need buffering Not normal condition(0x%x)", __LINE__, m_uiClipID, uiRet);
				continue;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Frame fail while decoder init(0x%x)", __LINE__, m_uiClipID, uiRet);
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) This Video frame is invalid", __LINE__, m_uiClipID);
			nexSAL_TaskSleep(20);
			m_isNextFrameRead = TRUE;
			continue;
		}
		// Codec ÃÊ±âÈ­½Ã IDR ÇÁ·¹?Ó?» Âü°í ÇÏÁö ¾Ê¾Æ IFrame?» Ã£µµ·Ï ÇÔ.

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Check I-frame CTS(%u) PTS(%u) DSINALSzie(%d) FrameNALSize(%d)", __LINE__, m_uiClipID, uiDTS, uiPTS,  iDSINALHeaderSize, iFrameNALHeaderSize);
		isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
		if( isIntraFrame )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) I-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
			break;
		}
		
		m_isNextFrameRead = TRUE;			
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%D) P-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
	}

	//yoon
	m_pSource->getVideoTrackUUID(&puuid,&uuid_size);
	is360video = CVideoTrackInfo::is360Video(puuid,uuid_size);
	
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Decoder Inint", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Dec init(%p %p %d)", 
				__LINE__, m_uiClipID, m_pCodecWrap->getCodecHandle(), pBaseDSI, uiBaseDSISize);

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
				if ( eRet == NEXCAL_ERROR_CORRUPTED_FRAME && uiRetryCnt < 1 )
				{
					uiRetryCnt++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] ID(%d) Video Codec initialize Error. So retry[%d]\n", __LINE__, m_uiClipID, uiRetryCnt);
					goto RETRY_INIT_DECODER;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] ID(%d) Video Codec initialize Error", __LINE__, m_uiClipID);
					m_pCodecWrap->deinitDecoder();
					SAFE_RELEASE(m_pCodecWrap);
					goto DECODER_INIT_ERROR;
				}
			}

			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iWidth	= iWidth;
			m_iHeight	= iHeight;
			m_iPitch	= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Video Decoder Init", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Dec init %p %d",
				__LINE__, m_uiClipID, pBaseDSI, uiBaseDSISize);
//yoon
			iHdrType = CVideoTrackInfo::getHDRMetaData(m_pSource->getVideoObjectType(),	pBaseDSI , uiBaseDSISize
				, pFrame, uiFrameSize, iFrameNALHeaderSize
				,&stHdrInfo);


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

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);

			if( eRet != NEXCAL_ERROR_NONE )
			{
				if ( uiRetryCnt < 1 )
				{
					uiRetryCnt++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] ID(%D) Video Codec initialize Error. So retry[%d]\n", __LINE__, m_uiClipID, uiRetryCnt);

#if 0
#ifdef _ANDROID
					if(CNexVideoEditor::m_bNexEditorSDK && uiVideoType == eNEX_CODEC_V_H264)
					{
						SAFE_RELEASE(m_pCodecWrap);
						m_pCodecWrap = new CNexCodecWrap();
						uiVideoType = eNEX_CODEC_V_H264_MC_S;

						NXBOOL bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
																NEXCAL_MODE_DECODER,
																uiVideoType,
																m_iWidth,
																m_iHeight,
																m_iWidth);

						if( bRet == FALSE )
						{
							SAFE_RELEASE(m_pCodecWrap);
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec Failed(0x%x)", __LINE__, m_uiClipID, uiVideoType);
							return FALSE;
						}

						if( nexCAL_VideoDecoderGetProperty( m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
							isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;
					
						if( m_pVideoRenderer && m_pCodecWrap->isHardwareCodec() )
						{
#ifdef _ANDROID
							pSurfaceTexture = m_pVideoRenderer->getSurfaceTexture();
							if( pSurfaceTexture != NULL )
							{
								m_pCodecWrap->setSurfaceTexture(pSurfaceTexture);
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Use surfaceTexture(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
							}
							// for JIRA 2971
							else
							{
								m_pCodecWrap->deinitDecoder();
								SAFE_RELEASE(m_pCodecWrap);
#ifdef FOR_TEST_PREPARE_VIDEO
								if( CNexVideoEditor::m_bPrepareVideoCodec )
								{
									CNexCodecManager::clearPrepareHardwareDecoderFlag();
								}	
#endif			
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec dec surface failed", __LINE__, m_uiClipID);
								return FALSE;
							}
#endif
						}
					
						if( m_pCodecWrap->isHardwareCodec() == FALSE )
						{
							nexCAL_VideoDecoderSetProperty( m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE, NEXCAL_PROPERTY_VIDEO_BUFFER_NV12);
						}
					}
#endif				
#endif
					goto RETRY_INIT_DECODER;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] ID(%d) Video Codec initialize Error So End", __LINE__, m_uiClipID);
					// Error report
					if( pNewConfig != NULL)
					{
						nexSAL_MemFree(pNewConfig);
						pNewConfig = NULL;
					}

					m_pCodecWrap->deinitDecoder();
					SAFE_RELEASE(m_pCodecWrap);
					goto DECODER_INIT_ERROR;
				}
			}
			
			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_uiClipID, m_iWidth, m_iHeight, m_iPitch );
			break;

		default:
			goto DECODER_INIT_ERROR;
	};

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	if(m_pCodecCache)
	{
		m_pCodecCache->setCodecWrap(m_pCodecWrap);
	}

	m_pVideoTrack = new CVideoTrackInfo;
	if( m_pVideoTrack == NULL )
	{
		goto DECODER_INIT_ERROR;
	}

	videoInfo.muiTrackID		= m_pClipItem->getClipID();
	videoInfo.mClipType			= m_pClipItem->getClipType();

	videoInfo.mStartTime		= m_pClipItem->getStartTime();
	videoInfo.mEndTime			= m_pClipItem->getEndTime();

	videoInfo.mWidth			= m_iWidth;
	videoInfo.mHeight			= m_iHeight;
	videoInfo.mPitch			= m_iPitch;
	videoInfo.mColorFormat		= m_pCodecWrap->getFormat();
	videoInfo.mIsPreview		= FALSE;
	videoInfo.mRotateState		= m_pClipItem->getRotateState();
	
	videoInfo.mBrightness		= m_pClipItem->getBrightness();
	videoInfo.mContrast			= m_pClipItem->getContrast();
	videoInfo.mSaturation		= m_pClipItem->getSaturation();
    videoInfo.mHue      		= m_pClipItem->getHue();
	videoInfo.mTintcolor		= m_pClipItem->getTintcolor();
	videoInfo.mLUT				= m_pClipItem->getLUT();
	videoInfo.mVignette 		= m_pClipItem->getVignette();
	
	videoInfo.mVideoRenderMode =  is360video; //m_pClipItem->getVideoRenderMode();
	videoInfo.mHDRType =  iHdrType;
	memcpy(&videoInfo.mHdrInfo, &stHdrInfo, sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon

	pRec =  m_pClipItem->getStartPosition();
	if( pRec != NULL )
	{
		videoInfo.mStart.left		= pRec->getLeft();
		videoInfo.mStart.top		= pRec->getTop();
		videoInfo.mStart.right		= pRec->getRight();
		videoInfo.mStart.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pRec =  m_pClipItem->getEndPosition();
	if( pRec != NULL )
	{
		videoInfo.mEnd.left			= pRec->getLeft();
		videoInfo.mEnd.top			= pRec->getTop();
		videoInfo.mEnd.right		= pRec->getRight();
		videoInfo.mEnd.bottom		= pRec->getBottom();
		SAFE_RELEASE(pRec);
	}

	pMatrix = m_pClipItem->getStartMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mStartMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}

	pMatrix = m_pClipItem->getEndMatrix(&iMatrixSize);
	if( pMatrix != NULL && iMatrixSize > 0 )
	{
		memcpy(videoInfo.mEndMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
	}	
	
	if( m_pVideoTrack->setClipRenderInfo(&videoInfo) == FALSE )
	{
		goto DECODER_INIT_ERROR;
	}

	if( m_pCodecWrap->isHardwareCodec() &&
		pSurfaceTexture && 
		m_pVideoTrack->setSurfaceTexture(pSurfaceTexture) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Regist surfacetexture failed(%p)", __LINE__, pSurfaceTexture);
		goto DECODER_INIT_ERROR;
	}

	m_pVideoTrack->setTrackIndexInfo(m_pClipItem->getClipIndex(), m_pClipItem->getClipTotalCount());
	m_pVideoTrack->setCodecWrap(m_pCodecWrap);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] m_ClipVideoRenderInfoVec.size (%d, %zu)\n", __LINE__,  m_pClipItem->getClipID(),m_pClipItem->m_ClipVideoRenderInfoVec.size());
	m_pVideoTrack->clearClipRenderInfoVec();
	for(i = m_pClipItem->m_ClipVideoRenderInfoVec.begin(); i != m_pClipItem->m_ClipVideoRenderInfoVec.end(); i++)
	{
		pRenderInfo = ((CClipVideoRenderInfo*)(*i));
		m_pVideoTrack->setClipRenderInfoVec(pRenderInfo);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setEffect(%p, %d)\n", __LINE__,  m_pClipItem, m_pClipItem->getEndTime());
	if( m_pClipItem )
	{
		if( m_pClipItem->getClipEffectEnable() )
		{
			char* pEffect = (char*)m_pClipItem->getClipEffectID();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setEffect(%s)\n", __LINE__, pEffect);
			if( pEffect )
			{
				m_uiEffectStartTime = m_pClipItem->getEndTime() - (m_pClipItem->getClipEffectDuration() * m_pClipItem->getClipEffectOffset() / 100);
				m_pVideoTrack->setEffectInfo(	m_uiEffectStartTime, 
											m_pClipItem->getClipEffectDuration(), 
											m_pClipItem->getClipEffectOffset(), 
											m_pClipItem->getClipEffectOverlap(),
											pEffect);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setEffect(%p)\n", __LINE__, pEffect);
			}
		}

		// set title for Effect option, mantis 5524
		char* pTitleEffect =(char*)m_pClipItem->getTitleEffectID();
		m_pVideoTrack->setTitleInfo(	m_pClipItem->getTitle(),
									pTitleEffect,
									m_pClipItem->getTitleStartTime(),
									m_pClipItem->getTitleEndTime());

		CNexDrawInfoVec* pDrawInfos = m_pClipItem->getDrawInfos();
		m_pVideoTrack->setDrawInfos(pDrawInfos);
		SAFE_RELEASE(pDrawInfos);

		m_pVideoTrack->setMotionTracked(m_pClipItem->getMotionTracked());

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] setTitleInfo(%s, %d %d)", 
			__LINE__, pTitleEffect, m_pClipItem->getTitleStartTime(), m_pClipItem->getTitleEndTime());
	}
	
	if( registTrack() == FALSE )
	{
		goto DECODER_INIT_ERROR;
	}
	m_isNextFrameRead = FALSE;

	return TRUE;

DECODER_INIT_ERROR:

	if( pSurfaceTexture )
	{
		m_pVideoRenderer->releaseSurfaceTexture(pSurfaceTexture);
		pSurfaceTexture = NULL;
	}

	m_isNextFrameRead = FALSE;
	deinitVideoDecoder();
	return FALSE;
}

NXBOOL CNEXThread_VideoTask::reinitVideoDecoder(unsigned char* pIDRFrame, unsigned int uiSize)
{
	if( m_pCodecWrap == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) reinitVideoDecoder failed(%p)", __LINE__, m_uiClipID, m_pCodecWrap);
		return FALSE;
	}

	m_pCodecWrap->deinitDecoder();
	NXBOOL bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
											NEXCAL_MODE_DECODER,
											m_pSource->getVideoObjectType(),
											m_iWidth,
											m_iHeight,
											m_iWidth);

	if( bRet == FALSE )
	{
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec Failed(0x%x)", __LINE__, m_uiClipID, m_pSource->getVideoObjectType());
		return FALSE;
	}

	NXUINT32	uiDTS			= 0;
	NXBOOL		isValidVideo	= FALSE;
	NXBOOL		isIntraFrame	= FALSE;

	NXUINT8*	pBaseDSI		= NULL;
	NXUINT32	uiBaseDSISize	= 0;
	NXUINT8*	pNewConfig		= NULL;
	NXUINT32	uiNewConfigSize	= 0;
	NXUINT8*	pEnhancedDSI			= NULL;
	NXUINT32	uiEnhancedDSISize		= 0;
	NXUINT32	uiH264ProfileLevelID	= 0;

	NXINT64		isUseIframeVDecInit			= 0;
	unsigned int	eRet						= 0;

	int 			iDSINALHeaderSize			= 0;
	int 			iFrameNALHeaderSize		= 0;
 	
	unsigned int	iWidth						= 0;
	unsigned int	iHeight						= 0;

	void*		pSurfaceTexture				= NULL;
	int iHdrType = 0;
	NEXCODECUTIL_SEI_HDR_INFO stHdrInfo;
	memset( &stHdrInfo, 0, sizeof(NEXCODECUTIL_SEI_HDR_INFO));

	unsigned char* puuid = NULL;
	int uuid_size = 0;
	int is360video = 0;

	
	iDSINALHeaderSize = m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

	if( m_pVideoRenderer && m_pCodecWrap->isHardwareCodec() )
	{
		pSurfaceTexture = m_pVideoRenderer->getSurfaceTexture();
		if( pSurfaceTexture != NULL )
		{
			m_pCodecWrap->setSurfaceTexture(pSurfaceTexture);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Use surfaceTexture(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
		}
		// for JIRA 2971
		else
		{
			m_pCodecWrap->deinitDecoder();
			SAFE_RELEASE(m_pCodecWrap);
#ifdef FOR_TEST_PREPARE_VIDEO
			if( CNexVideoEditor::m_bPrepareVideoCodec )
			{
				CNexCodecManager::clearPrepareHardwareDecoderFlag();
			}
#endif
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec dec surface failed", __LINE__, m_uiClipID);
			return FALSE;
		}
	}

	//yoon
	m_pSource->getVideoTrackUUID(&puuid,&uuid_size);
	is360video = CVideoTrackInfo::is360Video(puuid, uuid_size);


	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );


			eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
												pBaseDSI,
												uiBaseDSISize,
												pIDRFrame,
												uiSize,
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
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] Video Codec re-initialize Error(%d)", __LINE__, eRet);
				goto DECODER_INIT_ERROR;
			}
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iWidth		= iWidth;
			m_iHeight		= iHeight;
			m_iPitch		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Dec init(%p %d %p %d)", __LINE__, pBaseDSI, uiBaseDSISize, pIDRFrame, uiSize);

//yoon			
			iHdrType = CVideoTrackInfo::getHDRMetaData(m_pSource->getVideoObjectType(),	pBaseDSI , uiBaseDSISize
				, pIDRFrame, uiSize, iFrameNALHeaderSize
				,&stHdrInfo);

			
			isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;
			nexCAL_VideoDecoderGetProperty( m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit);

			if( NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES == isUseIframeVDecInit )
			{
				eRet = m_pCodecWrap->initDecoder(	(NEX_CODEC_TYPE)m_pSource->getVideoObjectType(),
													pBaseDSI,
													uiBaseDSISize,
													pIDRFrame,
													uiSize,
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

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] Video Codec re-initialize Error(%d)", __LINE__, eRet);
				goto DECODER_INIT_ERROR;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_iWidth, m_iHeight, m_iPitch );
			break;

		default:
			SAFE_RELEASE(m_pCodecWrap);
			goto DECODER_INIT_ERROR;
	};

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ColorFormat(%d)", __LINE__, m_pCodecWrap->getFormat());

	if(	m_pCodecWrap->getFormat() == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY )
	{
		IRectangle* pRec		= NULL;
		CClipVideoRenderInfoVectorIterator i;
		CClipVideoRenderInfo* pRenderInfo;
		CClipVideoRenderInfo videoInfo;
		//memset(&videoInfo, 0x00, sizeof(CClipVideoRenderInfo));

		videoInfo.muiTrackID		= m_pClipItem->getClipID();
		videoInfo.mClipType			= m_pClipItem->getClipType();

		videoInfo.mStartTime		= m_pClipItem->getStartTime();
		videoInfo.mEndTime			= m_pClipItem->getEndTime();

		videoInfo.mWidth			= m_iWidth;
		videoInfo.mHeight			= m_iHeight;
		videoInfo.mPitch			= m_iPitch;
		videoInfo.mColorFormat		= m_pCodecWrap->getFormat();
		videoInfo.mIsPreview		= FALSE;
		videoInfo.mRotateState		= m_pClipItem->getRotateState();

		videoInfo.mBrightness		= m_pClipItem->getBrightness();
		videoInfo.mContrast			= m_pClipItem->getContrast();
		videoInfo.mSaturation		= m_pClipItem->getSaturation();
        videoInfo.mHue		        = m_pClipItem->getHue();
		videoInfo.mTintcolor		= m_pClipItem->getTintcolor();
		videoInfo.mLUT				= m_pClipItem->getLUT();
		videoInfo.mVignette 		= m_pClipItem->getVignette();
		videoInfo.mVideoRenderMode = is360video; //yoon
		videoInfo.mHDRType = iHdrType;
		memcpy(&videoInfo.mHdrInfo, &stHdrInfo, sizeof(NEXCODECUTIL_SEI_HDR_INFO)); //yoon

		pRec =  m_pClipItem->getStartPosition();
		if( pRec != NULL )
		{
			videoInfo.mStart.left		= pRec->getLeft();
			videoInfo.mStart.top		= pRec->getTop();
			videoInfo.mStart.right		= pRec->getRight();
			videoInfo.mStart.bottom		= pRec->getBottom();
			SAFE_RELEASE(pRec);
		}

		pRec =  m_pClipItem->getEndPosition();
		if( pRec != NULL )
		{
			videoInfo.mEnd.left			= pRec->getLeft();
			videoInfo.mEnd.top			= pRec->getTop();
			videoInfo.mEnd.right		= pRec->getRight();
			videoInfo.mEnd.bottom		= pRec->getBottom();
			SAFE_RELEASE(pRec);
		}
		
		float* pMatrix = NULL;
		int iMatrixSize = 0;

		pMatrix = m_pClipItem->getStartMatrix(&iMatrixSize);
		if( pMatrix != NULL && iMatrixSize > 0 )
		{
			memcpy(videoInfo.mStartMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
		}

		pMatrix = m_pClipItem->getEndMatrix(&iMatrixSize);
		if( pMatrix != NULL && iMatrixSize > 0 )
		{
			memcpy(videoInfo.mEndMatrix, pMatrix, sizeof(float)*MATRIX_MAX_COUNT);
		}			
		
		CFrameInfo* pFrame = m_pVideoTrack->getFrameOutBuffer();
		while( m_bIsWorking && pFrame )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Draw wait for remain frame while chdec change", __LINE__, m_uiClipID);
			pFrame = m_pVideoTrack->getFrameOutBuffer();
			nexSAL_TaskSleep(30);
		}

		if( m_bIsWorking == FALSE )
		{
			goto DECODER_INIT_ERROR;
		}

		m_pVideoTrack->setClipRenderInfo(&videoInfo);

		if( pSurfaceTexture )
			m_pVideoTrack->setSurfaceTexture(pSurfaceTexture);

		m_pVideoTrack->setCodecWrap(m_pCodecWrap);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Track Reinit Sucessed (%d)", __LINE__);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] Codec Reinit Sucessed (%d)", __LINE__);
	m_isNextFrameRead	= FALSE;
	return TRUE;

DECODER_INIT_ERROR:

	deinitVideoDecoder();

	if( pSurfaceTexture )
	{
		m_pVideoRenderer->releaseSurfaceTexture(pSurfaceTexture);
		pSurfaceTexture = NULL;
	}

	return FALSE;
}

NXBOOL CNEXThread_VideoTask::deinitVideoDecoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) deinitVideoDecoder In", __LINE__, m_uiClipID);
	NXBOOL bHardwareCodec = FALSE;
	if( m_pCodecWrap )
	{
		bHardwareCodec = m_pCodecWrap->isHardwareCodec();
		m_pCodecWrap->deinitDecoder();
	}
	SAFE_RELEASE(m_pCodecWrap);

#ifdef FOR_TEST_PREPARE_VIDEO
	if( CNexVideoEditor::m_bPrepareVideoCodec && bHardwareCodec )
	{
		CNexCodecManager::clearPrepareHardwareDecoderFlag();
	}
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) deinitVideoDecoder Out", __LINE__, m_uiClipID);
	return TRUE;
}

NXBOOL CNEXThread_VideoTask::registTrack()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] registTrack() In",__LINE__);

	if( m_pVideoRenderer && m_pVideoTrack )
	{
		CNxMsgAddTrack* pMsg = new CNxMsgAddTrack(m_pVideoTrack);
		if( pMsg )
		{
			m_pVideoRenderer->SendCommand(pMsg);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] registTrack() Out",__LINE__);	
			return TRUE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] registTrack() Out",__LINE__);	
	return FALSE;
}

NXBOOL CNEXThread_VideoTask::deregistTrack()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] deregistTrack() In",__LINE__);	
	if( m_pVideoRenderer && m_pVideoTrack )
	{
		CNxMsgDeleteTrack* pMsg = new CNxMsgDeleteTrack(m_pVideoTrack);
		if( pMsg )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] deregistTrack(%d %d) Out - start",__LINE__, m_pVideoTrack->m_uiTrackID, m_pVideoTrack->GetRefCnt());
			m_pVideoRenderer->SendCommand(pMsg);
			pMsg->waitProcessDone(1000);
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] deregistTrack(%d %d) Out - end",__LINE__, m_pVideoTrack->m_uiTrackID, m_pVideoTrack->GetRefCnt());
			return TRUE;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] deregistTrack() Out",__LINE__);	
	return FALSE;
}

NXBOOL CNEXThread_VideoTask::isEOSFlag(unsigned int uiFlag)
{
	if( (uiFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
		return TRUE;
	return FALSE;
}

NXBOOL CNEXThread_VideoTask::canUseSWDecoder()
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

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) can use resource(0x%x, H:%d M:%d) ret(%d)", 
		__LINE__, m_uiClipID, uiVideoType, CNexVideoEditor::m_bSuppoertSWH264Codec, CNexVideoEditor::m_bSuppoertSWMP4Codec, canUseSWDec);
	return canUseSWDec;
}

NXBOOL CNEXThread_VideoTask::checkDecoding()
{
	if( m_pSource == NULL )
		return FALSE;

	if( m_pSource->isVideoExist() == FALSE )
		return FALSE;

    SAFE_RELEASE(m_pCodecWrap);
    m_pCodecWrap = new CNexCodecWrap();

	NXBOOL bRet = m_pCodecWrap->getCodec(	NEXCAL_MEDIATYPE_VIDEO,
											NEXCAL_MODE_DECODER,
											m_pSource->getVideoObjectType(),
											m_iWidth,
											m_iHeight,
											m_iWidth);

	if( bRet == FALSE )
	{
		SAFE_RELEASE(m_pCodecWrap);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec Failed(0x%x)", __LINE__, m_uiClipID, m_pSource->getVideoObjectType());
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] ID(%d) Get Codec end(%p)", __LINE__, m_uiClipID, m_pCodecWrap->getCodecHandle());

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
	NXUINT32	uiH264ProfileLevelID		= 0;

	unsigned int	eRet 						= 0;
	int 			iDSINALHeaderSize			= 0;
	int 			iFrameNALHeaderSize		= 0;
 
	unsigned int	iWidth						= 0;
	unsigned int	iHeight						= 0;

	unsigned int	uiEnhancement	= NEXCAL_VDEC_FLAG_NONE;
	unsigned int	uiDecoderErrRet	= 0;
	unsigned int	uiDecOutCTS		= 0;
	unsigned int 	uiSeekResultCTS = 0;
	
	m_pSource->seekTo(0, &uiSeekResultCTS);

	NXINT64 isUseIframeVDecInit = 0;
	if( nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	iDSINALHeaderSize = m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

RETRY_INIT_DECODER:
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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] checkDecodingID(%d) Need buffering Not normal condition(0x%x)", __LINE__, m_uiClipID, uiRet);
				continue;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] checkDecoding ID(%d) Get Frame fail while decoder init(0x%x)", __LINE__, m_uiClipID, uiRet);
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] checkDecoding ID(%d) This Video frame is invalid", __LINE__, m_uiClipID);
			nexSAL_TaskSleep(20);
			m_isNextFrameRead = TRUE;
			continue;
		}
		// Codec ÃÊ±âÈ­½Ã IDR ÇÁ·¹?Ó?» Âü°í ÇÏÁö ¾Ê¾Æ IFrame?» Ã£µµ·Ï ÇÔ.

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d) Check I-frame CTS(%u) PTS(%u) NALSzie(%d)\n", __LINE__, m_uiClipID, uiDTS, uiPTS,  iDSINALHeaderSize);
		isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
		if( isIntraFrame )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d) I-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
			break;
		}
		
		m_isNextFrameRead = TRUE;			
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%D) P-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
	}

	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d) Dec init(%p %d)", __LINE__, m_uiClipID, pBaseDSI, uiBaseDSISize);

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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] ID(%d) Video Codec initialize Error(%d)", __LINE__, m_uiClipID, eRet);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d)  Codec init end", __LINE__, m_uiClipID);
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			m_pSource->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iWidth		= iWidth;
			m_iHeight		= iHeight;
			m_iPitch		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d) Dec init(%p %d)", __LINE__, m_uiClipID, pBaseDSI, uiBaseDSISize);


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

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp  %d] ID(%d) Video Codec initialize Error So End(%d)", __LINE__, m_uiClipID, eRet);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] ID(%d) VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_uiClipID, m_iWidth, m_iHeight, m_iPitch );
			break;

		default:
			SAFE_RELEASE(m_pCodecWrap);
			goto DECODER_INIT_ERROR;
	};

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d) ColorFormat(0x%x) HeaderSize(%d)", __LINE__, m_uiClipID, m_pCodecWrap->getFormat(), iDSINALHeaderSize);

	m_isNextFrameRead = FALSE;

	// decoding
	while (1) {
		nexSAL_TaskSleep(1);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding ID(%d) Frame(p:%p s:%d DTS:%d PTS:%d)", 
			__LINE__, m_uiClipID, pFrame, uiFrameSize, uiDTS, uiPTS);
		nexCAL_VideoDecoderDecode(	m_pCodecWrap->getCodecHandle(),
										pFrame,
										uiFrameSize,
										NULL,
										uiDTS,
										uiPTS,
										uiEnhancement, 
										&uiDecoderErrRet );

		uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
		if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_DECODING_SUCCESS) )
		{
			uiDecOutCTS = 0;
			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char*	pY = NULL;
				unsigned char*	pU = NULL;
				unsigned char*	pV = NULL;
				
				if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiDecOutCTS) != 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] checkDecoding ID(%d) nexCAL_VideoDecoderGetOutput failed(%d)",__LINE__, m_uiClipID, uiDecOutCTS);
					goto DECODER_INIT_ERROR;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding Video Output Buffer(%p)", __LINE__, pY);
				//callVideoFrameRenderCallback(pY, FALSE);
				break;
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VDTask.cpp %d] checkDecoding EOS detected before Output", __LINE__ );
				goto DECODER_INIT_ERROR;	
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] checkDecoding ID(%d) nexCAL_VideoDecoderGetOutput failed(%d), %d",__LINE__, m_uiClipID, uiDecOutCTS, uiDecoderErrRet);
			goto DECODER_INIT_ERROR;
			
		}
	}
	deinitVideoDecoder();
	return TRUE;

DECODER_INIT_ERROR:

	m_isNextFrameRead = FALSE;
	deinitVideoDecoder();
	return FALSE;	
	
}

int CNEXThread_VideoTask::checkRenderInfo(unsigned int uiTime)
{
	if(m_pClipItem == NULL)
		return FALSE;

	if(m_pClipItem->m_ClipVideoRenderInfoVec.size() == 1)
	{
		return FALSE;
	}
	else if(m_pClipItem->m_ClipVideoRenderInfoVec.size() > 1)
	{
		int index = -1;	
		CClipVideoRenderInfo* pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiTime, TRUE, &index);

		if(pRenderInfo != NULL && pRenderInfo != m_pCurrentRenderInfo)
		{
			m_pCurrentRenderInfo = pRenderInfo;
			m_pPreviousRenderInfo = (index==0)?NULL:m_pClipItem->m_ClipVideoRenderInfoVec[index - 1];              
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VDTask.cpp %d] checkRenderInfo ID(%d) %d, %d, %d, %d, %d %d",__LINE__, m_uiClipID, m_pCurrentRenderInfo->mStartTime, m_pCurrentRenderInfo->mEndTime, m_pCurrentRenderInfo->mStartTrimTime, m_pCurrentRenderInfo->mEndTrimTime, m_pCurrentRenderInfo->m_iSpeedCtlFactor, m_pCurrentRenderInfo->mFreezeDuration);
			return TRUE;
		}
	}
	
	return FALSE;
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/09/06	Draft.
-----------------------------------------------------------------------------*/
