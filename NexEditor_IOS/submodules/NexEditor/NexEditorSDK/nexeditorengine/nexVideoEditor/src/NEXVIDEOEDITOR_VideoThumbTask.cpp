/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoThumbTask.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2015/08/25	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_VideoThumbTask.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "NEXVIDEOEDITOR_Util.h"

#include "NexMediaDef.h"
#include "NexCodecUtil.h"

CNEXThread_VideoThumbTask::CNEXThread_VideoThumbTask( void )
{
	m_uiClipID				= INVALID_CLIP_ID;
	m_pClipItem				= NULL;
	m_pSource			= NULL;

	m_pCodecWrap			= NULL;

	m_iWidth					= 0;
	m_iHeight				= 0;
	m_iPitch					= 0;
	m_isNextFrameRead		= TRUE;

	m_bThumbnailFailed		= FALSE;
	m_pThumbnail			= NULL;
	m_iThumbnailStartTime		= 0;
	m_iThumbnailEndTime		= 0;
	m_pSurfaceTexture		= NULL;

	m_uiCheckVideoDecInit		= 0;

	m_FrameTimeChecker.clear();
	m_SeekTable_FrameTimeChecker.clear();

	m_isStopThumb			= 0;
	m_isH264Interlaced		= FALSE;

	m_bGetFirstFrame = 0;
	m_bGetLastFrame = 0; 		
}

CNEXThread_VideoThumbTask::~CNEXThread_VideoThumbTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ClipID:%d ~~~~CNEXThread_VideoThumbTask In", __LINE__, m_uiClipID);
	SAFE_RELEASE(m_pClipItem);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pCodecWrap);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ClipID:%d ~~~~CNEXThread_VideoThumbTask Out", __LINE__, m_uiClipID);
}

void CNEXThread_VideoThumbTask::End( unsigned int uiTimeout )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) End In(%d)", __LINE__, m_uiClipID, m_bIsWorking);
	if( m_bIsWorking == FALSE ) return;

	m_bIsWorking = FALSE;
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
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) End Out", __LINE__, m_uiClipID);
}

void CNEXThread_VideoThumbTask::WaitTask()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] WaitTask In", __LINE__, m_uiClipID);
	if( m_pThumbnail  )
	{
		nexSAL_TaskWait(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] WaitTask Wait end", __LINE__, m_uiClipID);
		nexSAL_TaskDelete(m_hThread);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] WaitTask delete end", __LINE__, m_uiClipID);
		m_hThread = NEXSAL_INVALID_HANDLE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] WaitTask Out", __LINE__, m_uiClipID);
}

int CNEXThread_VideoThumbTask::OnThreadMain( void )
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

	NXBOOL						bEndContent		= FALSE;

	int 						iSeekTableCount			= 0;
	unsigned int* 				pSeekTable				= NULL;
	
	int							iThumbnailGap			= 1;
	int							iThumbnailIndex			= 0;
	int							iThumbnailProcess		= 0;
	unsigned int				uiThumbnailGapTime		= 0;
	unsigned int				uiThumbnailLastTime		= 0;

	int 						iFrameNALHeaderSize		= 0;
	unsigned int				uiExpectanceTime		= 0;
	unsigned int				uiCheckDecTime			= 0;
	unsigned int				uiCheckGetOutputTime	= 0;

	int							iCurrProgress			= 0;
	int							iPrevProgress			= 0;

	NXBOOL						bThumbnailSeekMode		= TRUE;

	int							iThumbnailTimeTableIndex= 0;
	int							iThumbnailTimeTableTime	= 0;
	NXBOOL						bSupportFrameTimeChecker = TRUE;

	NXBOOL						bSeekFromSeekTable		= TRUE;
	NXBOOL						bSkipFrame				= FALSE;

	int							iThumbnailInterval		= 0;
	int							iThumbnailDecCount		= 0;
	unsigned int				iThumbnailIntervalLast	= 0;
	int							iThumbnailIntervalGap	= 33;

	NXBOOL                              bGetLastFrame = 0;
	unsigned int				uiPreviousDTS			= 0;
	unsigned int				uiPreviousPTS			= 0;

/*
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		bSupportFrameTimeChecker = pEditor->m_bSupportFrameTimeChecker;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] bSupportFrameTimeChecker (%d)", __LINE__, bSupportFrameTimeChecker);		
		SAFE_RELEASE(pEditor);
	}
*/
	if( m_pThumbnail == NULL || m_pSource == NULL )
	{
		m_bThumbnailFailed = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ClipID(%d) Thumb Task failed because reader or thumbnail invalid handle(%p %p)", 
			__LINE__, m_uiClipID, m_pThumbnail, m_pSource);
		return 181818;		
	}

	unsigned int				uiVideoEditBoxTime			= m_pSource->getVideoEditBox();    

	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ClipID(%d) Thumbnail process start", __LINE__, m_uiClipID);

	if( initVideoDecoder() == FALSE )
	{
		m_bThumbnailFailed = TRUE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ClipID(%d) Video Task init Decoder failed ", __LINE__, m_uiClipID);
		return 181818;
	}

	// comments related to m_uiDecFrameCountAfterDecInit will be removed.
	//m_uiDecFrameCountAfterDecInit = 0;

	unsigned int uiSeekTableTotalTime = m_uiClipTotalTime;
	if( m_iThumbnailEndTime != 0 && m_iThumbnailStartTime != 0 )
	{
		uiSeekTableTotalTime = m_iThumbnailEndTime - m_iThumbnailStartTime;
	}

	if( uiSeekTableTotalTime > (10 * 60 * 1000) && m_pThumbnail->getMaxThumbnailCount() <= 50 && m_pThumbnail->getThumbTimeTableCount() <= 0)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ClipID(%d) minimize seektable mode ", __LINE__, m_uiClipID);

		if( m_pSource->getSeekTable(m_iThumbnailStartTime, m_iThumbnailEndTime, m_pThumbnail->getMaxThumbnailCount(), &iSeekTableCount, &pSeekTable) != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] getSeekTable failed", __LINE__);
			m_bIsWorking = FALSE;
			m_bThumbnailFailed = TRUE;
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ClipID(%d) normal seektable mode ", __LINE__, m_uiClipID);
		if( m_pSource->getSeekTable(0, &iSeekTableCount, &pSeekTable) != NEXVIDEOEDITOR_ERROR_NONE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] getSeekTable failed", __LINE__);
			m_bIsWorking = FALSE;
			m_bThumbnailFailed = TRUE;
		}
	}

	if( m_pThumbnail->setSeekTableInfo(iSeekTableCount, pSeekTable ) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] setSeekTable failed ", __LINE__);
		m_bIsWorking = FALSE;
		m_bThumbnailFailed = TRUE;
	}

	NXBOOL bSoftwareCodec = !m_pCodecWrap->isHardwareCodec();
	
	if( m_pThumbnail->setThumbnailInfo(m_iWidth, m_iHeight, m_iPitch, bSoftwareCodec) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] setThumbnail info failed", __LINE__);
		m_bIsWorking = FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] setThumbnail time info(%d %d %d)", __LINE__, m_iThumbnailStartTime, m_iThumbnailEndTime, m_uiClipTotalTime);

	if( m_pThumbnail->getThumbTimeTableCount() > 0 )
	{
		iThumbnailTimeTableTime = m_pThumbnail->getThumbTimeTable(iThumbnailTimeTableIndex);

		int iNewSeekTableCount = 0;
		unsigned int* pNewSeekTable = (unsigned int*)nexSAL_MemAlloc(sizeof(unsigned int)*m_pThumbnail->getThumbTimeTableCount());
		if( pNewSeekTable == NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] setThumbnail info failed because not enough memory", __LINE__);
			m_bIsWorking = FALSE;
			m_bThumbnailFailed = TRUE;
		}
		else
		{
			for( int i = 0; i < iSeekTableCount; i++ )
			{
				if( iThumbnailTimeTableTime == pSeekTable[i] )
				{
					pNewSeekTable[iNewSeekTableCount] = pSeekTable[i];
					iNewSeekTableCount++;

					iThumbnailTimeTableIndex++;
					iThumbnailTimeTableTime = m_pThumbnail->getThumbTimeTable(iThumbnailTimeTableIndex);
				}
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] update seektable for thumbnail(%d -> %d)", __LINE__, iSeekTableCount, iNewSeekTableCount);
			nexSAL_MemFree(pSeekTable);
			pSeekTable = pNewSeekTable;
			iSeekTableCount = iNewSeekTableCount;
		}
	}
	else if( (m_iThumbnailStartTime != 0 || m_iThumbnailEndTime != 0) && m_iThumbnailStartTime < m_iThumbnailEndTime )
	{
		m_iThumbnailEndTime = m_iThumbnailEndTime >  m_uiClipTotalTime ? m_uiClipTotalTime : m_iThumbnailEndTime;

		iThumbnailInterval = m_pThumbnail->getThumbnailFlag();
		iThumbnailInterval = (iThumbnailInterval & GET_THUMBNAIL_INTERVAL_MASK) >> 8;

		if( iThumbnailInterval == 1 )
		{
			bThumbnailSeekMode = FALSE;

			if( m_pSource->seekTo(m_iThumbnailStartTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] seek failed for p frame thumbnail", __LINE__);
				m_bIsWorking = FALSE;
			}

			if( m_pThumbnail->getMaxThumbnailCount() > 0 )
			{
				iThumbnailIntervalGap = (m_iThumbnailEndTime - m_iThumbnailStartTime) / m_pThumbnail->getMaxThumbnailCount();
				iThumbnailIntervalGap = iThumbnailIntervalGap > 30 ? iThumbnailIntervalGap - 30 : iThumbnailIntervalGap;
			}

			iThumbnailDecCount		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] Thumbnail interval mode(%d)", __LINE__, iThumbnailInterval);

		}
		else if( iThumbnailInterval > 0 )
		{
			bThumbnailSeekMode = FALSE;

			if( m_pSource->seekTo(m_iThumbnailStartTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] seek failed for p frame thumbnail", __LINE__);
				m_bIsWorking = FALSE;
			}

			iThumbnailDecCount		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] Thumbnail interval mode(%d)", __LINE__, iThumbnailInterval);
		}
		else
		{
			int iNewSeekTableCount = 0;
			unsigned int* pNewSeekTable = (unsigned int*)nexSAL_MemAlloc(sizeof(unsigned int)*iSeekTableCount);
			if( pNewSeekTable == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] setThumbnail info failed because not enough memory", __LINE__);
				m_bIsWorking = FALSE;
				m_bThumbnailFailed = TRUE;
			}
			else
			{
				for( int i = 0; i < iSeekTableCount; i++ )
				{
					if( pSeekTable[i] < m_iThumbnailStartTime )
						continue;

					if( pSeekTable[i] > m_iThumbnailEndTime )
						break;

					pNewSeekTable[iNewSeekTableCount] = pSeekTable[i];
					iNewSeekTableCount++;
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] update seektable for thumbnail(%d -> %d), (%d, %d)", __LINE__, iSeekTableCount, iNewSeekTableCount, pNewSeekTable[0], pNewSeekTable[iNewSeekTableCount-1]);
				nexSAL_MemFree(pSeekTable);
				pSeekTable = pNewSeekTable;
				iSeekTableCount = iNewSeekTableCount;

				if(pSeekTable[0] != m_iThumbnailStartTime +1)
					m_bGetFirstFrame = (m_pThumbnail->getThumbnailFlag() & GET_THUMBRAW_FIRST_FRAME)?1:0;
				if(pSeekTable[iSeekTableCount-1] != m_iThumbnailEndTime + 1)
					m_bGetLastFrame = (m_pThumbnail->getThumbnailFlag() & GET_THUMBRAW_LAST_FRAME)?1:0;

				if(m_bGetFirstFrame)
				{
					if( m_pSource->seekTo(m_iThumbnailStartTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] seek failed for p frame thumbnail", __LINE__);
						m_bIsWorking = FALSE;
					}
				}
			}
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] setThumbnail info failed", __LINE__);
		m_bIsWorking = FALSE;
		m_bThumbnailFailed = TRUE;
	}

	if( m_pThumbnail->getRequestRawData() )
	{
		if( pSeekTable != NULL )
			nexSAL_MemFree(pSeekTable);
		
		pSeekTable = (unsigned int*)m_pThumbnail->getTimeTable(&iSeekTableCount);

		if( m_pSource->seekTo(m_iThumbnailStartTime, &uiSeekResult) != NEXVIDEOEDITOR_ERROR_NONE )			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] Video Task Begin Thumb Mode seek failed", __LINE__);
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] getRequestRawData %d", __LINE__, iSeekTableCount);			
	}
	
	if( iSeekTableCount+m_bGetFirstFrame+m_bGetLastFrame <= m_pThumbnail->getMaxThumbnailCount())
	{
		iThumbnailGap = 100;
	}
	else
	{
		iThumbnailGap = (iSeekTableCount+m_bGetFirstFrame+m_bGetLastFrame) * 100 / m_pThumbnail->getMaxThumbnailCount();
	}
	
	m_isNextFrameRead = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] Video Thumbnail Task Begin Mode(%d) ThumbnailGap(%d)", __LINE__, bThumbnailSeekMode, iThumbnailGap);

	// initialize Vector of FrameCTS
	m_FrameTimeChecker.clear();
	m_SeekTable_FrameTimeChecker.clear();
#ifdef __APPLE__        
    int nRetryCntAfterEOSSet = 0;
#endif

	if (m_isH264Interlaced && pSeekTable )
	{
		unsigned int uTempCTS;
		for (int i = 0; i < iSeekTableCount; i++)
		{
			uTempCTS = (pSeekTable[i] == 1?0:pSeekTable[i]);
			m_SeekTable_FrameTimeChecker.addFrameCTSVec(uTempCTS, uTempCTS);
		}
	}

	while ( m_bIsWorking )
	{
		nexSAL_TaskSleep(1);
		if(m_isStopThumb)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Video Task thumb stop flag set ", __LINE__, m_uiClipID);			
			break;
		}
		
		if( m_pThumbnail->getThumbnailcount() >= m_pThumbnail->getMaxThumbnailCount() )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d Max : %d)", 
				__LINE__, m_pThumbnail->getThumbnailcount(), m_pThumbnail->getMaxThumbnailCount());
			break;
		}
#if 0
		if( iThumbnailIndex >= iSeekTableCount )
		{
			if( m_pCodecWrap->isHardwareCodec() )
			{
				bEndContent = TRUE;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d Max : %d)", 
					__LINE__, m_pThumbnail->getThumbnailcount(), m_pThumbnail->getMaxThumbnailCount());
				break;
			}
		}

		if( bEndContent == FALSE && m_isNextFrameRead )
		{
			if( iThumbnailIndex < iSeekTableCount && bSeekFromSeekTable)
			{
				if( m_pSource->seekTo(pSeekTable[iThumbnailIndex], &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
				{
					iThumbnailProcess++;
					iThumbnailIndex = (iThumbnailProcess * iThumbnailGap) / 100;
					continue;
				}

				if (m_isH264Interlaced) bSeekFromSeekTable = FALSE;
			}
			else
				bSeekFromSeekTable = TRUE;
			
			int iReaderRet = m_pSource->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] getVideoFrame End or error(%d)", __LINE__, iReaderRet);
					bEndContent = TRUE;
					break;
			};
			m_isNextFrameRead = FALSE;
		}

		if( bEndContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d Max : %d)", 
				__LINE__, m_pThumbnail->getThumbnailcount(), m_pThumbnail->getMaxThumbnailCount());
		}
		else
		{
			m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
			m_pSource->getVideoFramePTS( &uiPTS, TRUE );
			m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );
		}
#else
		if( bThumbnailSeekMode && !m_bGetFirstFrame && bGetLastFrame == FALSE)
		{

			if((iThumbnailIndex >= iSeekTableCount || m_pThumbnail->getMaxThumbnailCount()-m_pThumbnail->getThumbnailcount() == 1) && m_bGetLastFrame)
			{
				bGetLastFrame = TRUE;
				if( m_pSource->seekTo(m_iThumbnailEndTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] seek failed(%d)",
									__LINE__, m_iThumbnailEndTime);
					break;
				}

				if( m_isH264Interlaced ) bSeekFromSeekTable = FALSE;
			}

			if(bGetLastFrame == FALSE)
			{
				if( iThumbnailIndex >= iSeekTableCount )
				{
					if( m_pCodecWrap->isHardwareCodec() )
					{
						bEndContent = TRUE;
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d Max : %d)",
										__LINE__, m_pThumbnail->getThumbnailcount(), m_pThumbnail->getMaxThumbnailCount());
						break;
					}
				}

				if( bEndContent == FALSE && m_isNextFrameRead )
				{
					if( iThumbnailIndex < iSeekTableCount && bSeekFromSeekTable )
					{
						if( m_pSource->seekTo(pSeekTable[iThumbnailIndex], &uiSeekResult, NXFF_RA_MODE_CUR_PREV, FALSE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
						{
							iThumbnailProcess++;
							iThumbnailIndex = (iThumbnailProcess * iThumbnailGap) / 100;
							continue;
						}

						if( m_isH264Interlaced ) bSeekFromSeekTable = FALSE;
					}
					else
					{
						bSeekFromSeekTable = TRUE;
					}
				}
			}
		}
		else
		{


		}

		if( bEndContent == FALSE && m_isNextFrameRead )
		{
			int iReaderRet = m_pSource->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] getVideoFrame End or error(%d)", __LINE__, iReaderRet);
					bEndContent = TRUE;
					break;
			};
			m_isNextFrameRead = FALSE;
		}

		if( bEndContent )
		{
			uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d Max : %d)",
							__LINE__, m_pThumbnail->getThumbnailcount(), m_pThumbnail->getMaxThumbnailCount());
		}
		else
		{
			m_pSource->getVideoFrameDTS( &uiDTS, TRUE );
			m_pSource->getVideoFramePTS( &uiPTS, TRUE );
			m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );

			if(m_bGetLastFrame && m_iThumbnailEndTime < uiPTS)
			{
				bEndContent = TRUE;
				uiEnhancement = NEXCAL_VDEC_FLAG_END_OF_STREAM;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d Max : %d)",
							__LINE__, m_pThumbnail->getThumbnailcount(), m_pThumbnail->getMaxThumbnailCount());
			}
		}
#endif

		bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Decode Thumbnail idx(%d) Frame(p:%p s:%d DTS:%d PTS:%d) IDR(%d)", 
			__LINE__, m_uiClipID, iThumbnailIndex, pFrame, uiFrameSize, uiDTS, uiPTS, bIDRFrame);
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
				if( bThumbnailSeekMode && !m_bGetFirstFrame && TRUE == bSeekFromSeekTable )
				{
					iThumbnailProcess++;
					iThumbnailIndex = (iThumbnailProcess* iThumbnailGap) / 100;
				}
				m_isNextFrameRead = TRUE;

				if (m_isH264Interlaced)
				{
					bSkipFrame = FALSE;
					if( m_SeekTable_FrameTimeChecker.isValidFrameCTS(uiPTS, TRUE) == FALSE && !bGetLastFrame)
						bSkipFrame = TRUE;
				}

				if(bSupportFrameTimeChecker && FALSE == bSkipFrame)
				{
					if(m_bGetFirstFrame)
					{
						if(m_iThumbnailStartTime <= uiPTS)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] m_bGetFirstFrame %d, DTS(%u) PTS(%u)", __LINE__, m_iThumbnailStartTime, uiDTS, uiPTS);
							m_FrameTimeChecker.addFrameCTSVec(uiDTS, uiPTS);

							m_bGetFirstFrame = FALSE;
							iThumbnailProcess++;
							iThumbnailIndex = (iThumbnailProcess* iThumbnailGap) / 100;
						}
					}
					else if(bGetLastFrame && m_bGetLastFrame)
					{
						if(bEndContent)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] m_bGetLastFrame %d, DTS(%u) PTS(%u)", __LINE__, m_iThumbnailStartTime, uiPreviousDTS, uiPreviousPTS);
							m_FrameTimeChecker.addFrameCTSVec(uiPreviousDTS, uiPreviousPTS);
							m_bGetLastFrame = FALSE;
							bGetLastFrame = FALSE;
						}
					}
					else
					{
						if(m_bGetLastFrame)
						{
							if(iThumbnailProcess < m_pThumbnail->getMaxThumbnailCount())
								m_FrameTimeChecker.addFrameCTSVec(uiDTS, uiPTS);
						}
						else
						{
							m_FrameTimeChecker.addFrameCTSVec(uiDTS, uiPTS);
						}
					}
					uiPreviousDTS = uiDTS;
					uiPreviousPTS = uiPTS;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Skip Frame DTS(%u) PTS(%u)", __LINE__, uiDTS, uiPTS);
				}
			}
			
			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_OUTPUT_EXIST) )
			{
				unsigned char*	pY = NULL;
				unsigned char*	pU = NULL;
				unsigned char*	pV = NULL;
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Thumbnail Count(%d) Size(%d %d %d)", __LINE__,
					m_pThumbnail->getThumbnailcount(), m_iWidth, m_iHeight, m_iPitch);
				
				uiDecOutCTS = 0;
				switch(m_pCodecWrap->getFormat())
				{
					case NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY:
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] getOutFrame Thumbnail Frame media buffer", __LINE__);
						if( nexCAL_VideoDecoderGetOutput(m_pCodecWrap->getCodecHandle(), &pY, NULL, NULL, &uiDecOutCTS) != 0 )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) nexCAL_VideoDecoderGetOutput failed(%d)",__LINE__, m_uiClipID, uiDecOutCTS);
							break;
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) GetOutput *m_pY=0x%08x CTS(%d)", __LINE__, m_uiClipID, pY, uiDecOutCTS);
						if( bSupportFrameTimeChecker && m_FrameTimeChecker.isValidFrameCTS(uiDecOutCTS, TRUE) == FALSE)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) GetOutput invalid Time (DTS:%d Frame:%d)", __LINE__, m_uiClipID, uiPTS, uiDecOutCTS);
#ifdef _ANDROID
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
							break;
						}

						if( uiVideoEditBoxTime > uiDecOutCTS )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Skip for Editbox(%d %d)", __LINE__, uiDecOutCTS, uiVideoEditBoxTime);
#ifdef _ANDROID
							m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
							break;

						}

						uiDecOutCTS -= uiVideoEditBoxTime;

						if( bThumbnailSeekMode )
						{
							// KM-3035 move into addThumbInfo
							//callVideoFrameRenderCallback(pY, TRUE);
							if( m_pThumbnail->addThumbInfo(pY, uiDecOutCTS) == FALSE )
							{
								m_bThumbnailFailed = TRUE;
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Add Thumbnail  Fail!", __LINE__);
							}
						}
						else
						{
							if( uiDecOutCTS < m_iThumbnailStartTime )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Decoded frame time is smaller than start time(%d %d)", __LINE__, uiDecOutCTS, m_iThumbnailStartTime);
#ifdef _ANDROID
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
								break;
							}

							if( uiDecOutCTS > m_iThumbnailEndTime )
							{
								m_bIsWorking = FALSE;
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Decoded frame time is greater than end time, will end thumbnail(%d %d)", __LINE__, uiDecOutCTS, m_iThumbnailEndTime);
#ifdef _ANDROID
								m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
								break;
							}

							if( iThumbnailInterval == 1 )
							{
								unsigned int checkTime = m_iThumbnailStartTime + m_pThumbnail->getThumbnailcount() * iThumbnailIntervalGap;
								// if( m_pThumbnail->getThumbnailcount() == 0 || iThumbnailIntervalLast + iThumbnailIntervalGap <= uiDecOutCTS )
								if( checkTime <= uiDecOutCTS )
								{
									if( m_pThumbnail->addThumbInfo(pY, uiDecOutCTS) == FALSE )
									{
										m_bThumbnailFailed = TRUE;
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Add Thumbnail  Fail!", __LINE__);
									}
									iThumbnailIntervalLast = uiDecOutCTS;
								}
								else
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Decoded frame skip(%d %d %d)", __LINE__, iThumbnailDecCount, iThumbnailInterval, uiDecOutCTS);
#ifdef _ANDROID
									m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
#endif
								}
							}
							else
							{
								if( (iThumbnailDecCount % iThumbnailInterval) == 0 )
								{
									if( m_pThumbnail->addThumbInfo(pY, uiDecOutCTS) == FALSE )
									{
										m_bThumbnailFailed = TRUE;
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Add Thumbnail  Fail!", __LINE__);
									}
								}
								else
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Decoded frame skip(%d %d %d)", __LINE__, iThumbnailDecCount, iThumbnailInterval, uiDecOutCTS);
	#ifdef _ANDROID
									m_pCodecWrap->postFrameRenderCallback(pY, FALSE);
	#endif
								}
								iThumbnailDecCount++;

							}
						}
						break;
					}
					default:
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] not support getOutFrame format(0x%x) for thumb", __LINE__, m_pCodecWrap->getFormat());
						break;
					}
				};
				
				if( m_pThumbnail->getRequestRawData() && m_pThumbnail->getThumbnailcount() >= m_pThumbnail->getMaxThumbnailCount())
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ready Raw Thumb Data", __LINE__);					
					break;
				}

				if( (m_pThumbnail->getThumbnailFlag() & GET_THUMBRAW_Y_ONLY ) == GET_THUMBRAW_Y_ONLY )
				{
					int max = iSeekTableCount > m_pThumbnail->getMaxThumbnailCount() ? m_pThumbnail->getMaxThumbnailCount() : iSeekTableCount;
					iCurrProgress = m_pThumbnail->getThumbnailcount() * 100 / max;
					if( iCurrProgress > iPrevProgress)
					{
						CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
						if( pEditor )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Highlight Thumbnail PROGRESS(%d %d)", __LINE__, iCurrProgress, max);
							pEditor->notifyEvent(MESSAGE_HIGHLIGHT_THUMBNAIL_PROGRESS, iCurrProgress, max);
							SAFE_RELEASE(pEditor);
						}
						iPrevProgress = iCurrProgress;	
					}
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] getOutFrame Thumbnail Frame is not exist", __LINE__);
			}

			if( NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				if( m_pThumbnail->getThumbnailcount() <= 0 )
				{
					m_bThumbnailFailed = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Can't thumbnail",__LINE__);
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d Max : %d)", 
					__LINE__, m_pThumbnail->getThumbnailcount(), m_pThumbnail->getMaxThumbnailCount());
				break;
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Video Dec failed(%d)", __LINE__, uiDecoderErrRet);
			if( isEOSFlag(uiEnhancement) || NEXCAL_CHECK_VDEC_RET(uiDecoderErrRet, NEXCAL_VDEC_EOS) )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d)", __LINE__, m_pThumbnail->getThumbnailcount());
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] thumbnail getOutFrame End(%d %d)",__LINE__, uiDTS, uiDecOutCTS);
				if( m_pThumbnail->getThumbnailcount() <= 0 )
				{
					m_bThumbnailFailed = TRUE;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] Can't thumbnail",__LINE__);
				}
				break;
			}
			m_bThumbnailFailed = TRUE;
			break;
		}
		
		if(bGetLastFrame && m_bGetLastFrame)
		{
			if(bEndContent)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] m_bGetLastFrame %d, DTS(%u) PTS(%u)", __LINE__, m_iThumbnailEndTime, uiPreviousDTS, uiPreviousPTS);
				m_FrameTimeChecker.addFrameCTSVec(uiPreviousDTS, uiPreviousPTS);
				m_bGetLastFrame = FALSE;
				bGetLastFrame = FALSE;
			}
		}
		
		if( (!bSupportFrameTimeChecker || m_FrameTimeChecker.empty() == TRUE ) && isEOSFlag(uiEnhancement) )
		{
#ifdef __APPLE__
			if ( nRetryCntAfterEOSSet < 3 ) 
			{
				nexSAL_TaskSleep(33);
				nRetryCntAfterEOSSet++;
			}
			else 
#endif
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] get Thumbnail end(Count : %d)", __LINE__, m_pThumbnail->getThumbnailcount());
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] thumbnail getOutFrame End(%d %d)",__LINE__, uiDTS, uiDecOutCTS);
				break;
			}
		}
	}

	if( pSeekTable && m_pThumbnail->getRequestRawData() == FALSE )
	{
		nexSAL_MemFree(pSeekTable);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Video Task End while", __LINE__, m_uiClipID);

	deinitVideoDecoder();
	m_pSurfaceTexture = NULL;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Video Task End and Release Codec", __LINE__, m_uiClipID);
	return 0;
}

NXBOOL CNEXThread_VideoThumbTask::setClipItem(CClipItem* pClipItem)
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
	return TRUE;
}

NXBOOL CNEXThread_VideoThumbTask::setSource(CNexSource* pSource)
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

NXBOOL CNEXThread_VideoThumbTask::setThumbnailHandle(CThumbnail* pThumbnail)
{
	m_pThumbnail = pThumbnail;

	if( m_pThumbnail != NULL )
	{
		unsigned int uiStartTime = 0;
		unsigned int uiEndTime = 0;
		m_pThumbnail->getStartEndTime(&uiStartTime, &uiEndTime);
		m_iThumbnailStartTime = uiStartTime;
		m_iThumbnailEndTime = uiEndTime;
	}
	return TRUE;
}

NXBOOL CNEXThread_VideoThumbTask::setStopThumb()
{
	m_isStopThumb = TRUE;
	return TRUE;
}

NXBOOL CNEXThread_VideoThumbTask::isGetThumbnailFailed()
{
	return m_bThumbnailFailed;
}

NXBOOL CNEXThread_VideoThumbTask::initVideoDecoder()
{
	CAutoLock m(CNexCodecManager::getVideoDecInitLock());

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%D) initVideoDecoder() In", __LINE__, m_uiClipID);
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Now available hardware codec", __LINE__, m_uiClipID);
			break;
		}
		nexSAL_TaskSleep(30);
		retryCount--;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Wait available hardware codec", __LINE__, m_uiClipID);
		if(retryCount < 0)
		{
		    break;
		}
	}

	if( m_bIsWorking == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Task exit before video decoder init", __LINE__, m_uiClipID);
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
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Get Codec Failed(0x%x)", __LINE__, m_uiClipID, m_pSource->getVideoObjectType());
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Get Codec end(%p)", __LINE__, m_uiClipID, m_pSource->getVideoObjectType(), m_pCodecWrap->getCodecHandle());

	
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
	//memset(&videoInfo, 0x00, sizeof(CClipVideoRenderInfo));

	if( nexCAL_VideoDecoderGetProperty(m_pCodecWrap->getCodecHandle(), NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT, &isUseIframeVDecInit) != NEXCAL_ERROR_NONE )
		isUseIframeVDecInit = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_NO;

	iDSINALHeaderSize = m_pSource->getDSINALHeaderLength();
	iFrameNALHeaderSize = m_pSource->getFrameNALHeaderLength();

#ifdef _ANDROID
	pSurfaceTexture = m_pThumbnail->getThumbSurfaceTexture();
	if( pSurfaceTexture == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Get Dec Surface Failed(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
		SAFE_RELEASE(m_pCodecWrap);
		return FALSE;
	}
	
	m_pSurfaceTexture = pSurfaceTexture;
	m_pCodecWrap->setSurfaceTexture(m_pSurfaceTexture);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Use surfaceTexture(%p)", __LINE__, m_uiClipID, pSurfaceTexture);
#endif

	while(1)
	{
		if( m_isNextFrameRead )
		{
			unsigned int uiRet = m_pSource->getVideoFrame(FALSE, TRUE);
			m_isNextFrameRead = FALSE;
			if( uiRet == _SRC_GETFRAME_OK )
			{
			}
			else if( uiRet == _SRC_GETFRAME_NEEDBUFFERING )
			{
				nexSAL_TaskSleep(20);
				m_isNextFrameRead = TRUE;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Need buffering Not normal condition(0x%x)", __LINE__, m_uiClipID, uiRet);
				continue;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) Get Frame fail while decoder init(0x%x)", __LINE__, m_uiClipID, uiRet);
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
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VThumbTask.cpp %d] ID(%d) This Video frame is invalid", __LINE__, m_uiClipID);
			nexSAL_TaskSleep(20);
			m_isNextFrameRead = TRUE;
			continue;
		}
		// Codec 초기화시 IDR 프레임을 참고 하지 않아 IFrame을 찾도록 함.

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Check I-frame CTS(%u) PTS(%u) NALSzie(%d)\n", __LINE__, m_uiClipID, uiDTS, uiPTS,  iDSINALHeaderSize);
		isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iFrameNALHeaderSize, m_pSource->getFrameFormat(), FALSE);
		if( isIntraFrame )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) I-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
			break;
		}
		
		m_isNextFrameRead = TRUE;			
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%D) P-frame searched...CTS(%u) PTS(%u)", __LINE__, m_uiClipID, uiDTS, uiPTS);
	}

	m_uiCheckVideoDecInit = nexSAL_GetTickCount();
	switch(m_pSource->getVideoObjectType())
	{
		case eNEX_CODEC_V_MPEG4V:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Dec init(%p %p %d)", __LINE__, m_uiClipID, m_pCodecWrap->getCodecHandle(), pBaseDSI, uiBaseDSISize);

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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp  %d] Video codec init failed(%d %d) and retry", __LINE__, uiRetryCnt, eRet);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}
			break;

		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			m_pSource->getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uiBaseDSISize );

			NEXCODECUTIL_SPS_INFO SPSInfo;
			NEXCODECUTIL_AVC_SLICE_HEADER_INFO stAVCSHInfo;
			if( NEXVIDEOEDITOR_ERROR_NONE == (NEXVIDEOEDITOR_ERROR)NexCodecUtil_AVC_GetSPSInfo((char *)pBaseDSI, uiBaseDSISize, &SPSInfo, m_pSource->getFrameFormat()) )
			{
				if( NEXVIDEOEDITOR_ERROR_NONE == (NEXVIDEOEDITOR_ERROR)NexCodecUtil_AVC_GetSliceHeaderInfo((char*)pFrame, uiFrameSize, &SPSInfo, m_pSource->getFrameNALHeaderLength(), m_pSource->getFrameFormat(), &stAVCSHInfo))
				{
					m_isH264Interlaced = stAVCSHInfo.field_pic_flag;
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[VThumbTask.cpp %d] Fail get GetSliceHeaderInfo", __LINE__);
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] H264 Interlaced(%d)", __LINE__, m_isH264Interlaced);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[VThumbTask.cpp %d] Fail get SPS_Info", __LINE__);
			}

			m_pSource->getH264ProfileLevelID(&uiH264ProfileLevelID );
			m_pSource->getVideoResolution(&iWidth, &iHeight);
			m_iWidth		= iWidth;
			m_iHeight	= iHeight;
			m_iPitch		= 0;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Video Decoder Init", __LINE__, m_uiClipID);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Dec init %p %d",
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

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) Dec init End(%d)", __LINE__, m_uiClipID, eRet);

			if( pNewConfig != NULL)
			{
				nexSAL_MemFree(pNewConfig);
				pNewConfig = NULL;
			}

			if( eRet != NEXCAL_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp  %d] ID(%d) Video Codec initialize Error So End", __LINE__, m_uiClipID);
				m_pCodecWrap->deinitDecoder();
				SAFE_RELEASE(m_pCodecWrap);
				goto DECODER_INIT_ERROR;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) VideoDecoder_Initialized() : W[%d], H[%d], P[%d]\n", __LINE__, m_uiClipID, m_iWidth, m_iHeight, m_iPitch );
			break;

		default:
			goto DECODER_INIT_ERROR;
	};

	m_uiCheckVideoDecInit = nexSAL_GetTickCount() - m_uiCheckVideoDecInit;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) VideoDecoder_Initialized( Time : %d )", __LINE__, m_uiClipID, m_uiCheckVideoDecInit );
	m_uiCheckVideoDecInit = nexSAL_GetTickCount();

	m_pThumbnail->setCodecWrap(m_pCodecWrap);

	m_isNextFrameRead = FALSE;
	return TRUE;

DECODER_INIT_ERROR:

	m_isNextFrameRead = FALSE;
	deinitVideoDecoder();
	return FALSE;
}

NXBOOL CNEXThread_VideoThumbTask::deinitVideoDecoder()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) deinitVideoDecoder In", __LINE__, m_uiClipID);

	if( m_pCodecWrap != NULL )
	{
		m_pCodecWrap->deinitDecoder();
		SAFE_RELEASE(m_pCodecWrap);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) deinitVideoDecoder Out", __LINE__, m_uiClipID);
	return TRUE;
}

NXBOOL CNEXThread_VideoThumbTask::isEOSFlag(unsigned int uiFlag)
{
	if( (uiFlag & NEXCAL_VDEC_FLAG_END_OF_STREAM) == NEXCAL_VDEC_FLAG_END_OF_STREAM )
		return TRUE;
	return FALSE;
}

NXBOOL CNEXThread_VideoThumbTask::canUseSWDecoder()
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

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VThumbTask.cpp %d] ID(%d) can use resource(0x%x, H:%d M:%d) ret(%d)", 
		__LINE__, m_uiClipID, uiVideoType, CNexVideoEditor::m_bSuppoertSWH264Codec, CNexVideoEditor::m_bSuppoertSWMP4Codec, canUseSWDec);
	return canUseSWDec;
}
