/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_DirectExportTask.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
Case Song	2015/08/13	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_DirectExportTask.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_Util.h"

CNEXThread_DirectExportTask::CNEXThread_DirectExportTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] CNEXThread_DirectExportTask In", __LINE__);
	m_eTaskPriority			= NEXSAL_PRIORITY_HIGH;

	m_uiClipID				= INVALID_CLIP_ID;
	m_pClipItem				= NULL;
	m_pSource			= NULL;
	m_pFileWriter			= NULL;

	m_isNextFrameRead		= FALSE;

	m_uiStartTime			= 0;
	m_uiEndTime				= 0;
	m_uiBaseTime			= 0;
	m_uiStartTrimTime		= 0;
	m_iSpeedFactor			= 0;
	
	m_bDirectExportEnd		= FALSE;
	m_bVideoOnlyMode		= FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] CNEXThread_DirectExportTask Out", __LINE__);
}

CNEXThread_DirectExportTask::~CNEXThread_DirectExportTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ClipID:%d ~~~~CNEXThread_DirectExportTask In", __LINE__, m_uiClipID);
	SAFE_RELEASE(m_pClipItem);
	SAFE_RELEASE(m_pSource);
	SAFE_RELEASE(m_pFileWriter);

	m_isNextFrameRead		= FALSE;

	m_uiStartTime			= 0;
	m_uiEndTime				= 0;
	m_uiBaseTime			= 0;
	m_uiStartTrimTime			= 0;
	m_iSpeedFactor			= 0;
	
	m_bDirectExportEnd		= FALSE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ClipID:%d ~~~~CNEXThread_DirectExportTask Out", __LINE__, m_uiClipID);
}

void CNEXThread_DirectExportTask::End( unsigned int uiTimeout )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) End In(%d)", __LINE__, m_uiClipID, m_bIsWorking);
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
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) End Out", __LINE__, m_uiClipID);
}

NXBOOL CNEXThread_DirectExportTask::setClipItem(CClipItem* pClipItem)
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
		m_uiClipID			= m_pClipItem->getClipID();
		m_uiStartTime		= m_pClipItem->getStartTime();
		m_uiEndTime			= m_pClipItem->getEndTime();
		m_uiBaseTime		= m_pClipItem->getStartTime();
		m_uiStartTrimTime	= m_pClipItem->getStartTrimTime();
		m_iSpeedFactor		= m_pClipItem->getSpeedCtlFactor();
	}
	return TRUE;
}

NXBOOL CNEXThread_DirectExportTask::setSource(CNexSource* pSource)
{
	if( pSource == NULL )
	{
		return FALSE;
	}
	SAFE_RELEASE(m_pSource);
	SAFE_ADDREF(pSource);
	m_pSource = pSource;
	return TRUE;
}

NXBOOL CNEXThread_DirectExportTask::setFileWriter(CNexExportWriter* pFileWriter)
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

void CNEXThread_DirectExportTask::setDirectTaskMode(NXBOOL bVideoOnly)
{
	m_bVideoOnlyMode = bVideoOnly;
}

NXBOOL CNEXThread_DirectExportTask::isEndDirectExport()
{
	return m_bDirectExportEnd;
}

int CNEXThread_DirectExportTask::OnThreadMain( void )
{
	NEXVIDEOEDITOR_ERROR	eRet			= NEXVIDEOEDITOR_ERROR_NONE;
	
	NXBOOL					bAudioEnd		= FALSE;
	NXBOOL					bVideoEnd		= FALSE;

	NXBOOL					bAudioReadEnd	= FALSE;
	NXBOOL					bVideoReadEnd	= FALSE;

	unsigned char*			pFrame			= NULL;
	unsigned int			uiFrameSize		= 0;
	
	NXINT64					uiAudioDTS		= 0;
	
	unsigned int			uiVideoDTS		= 0;
	unsigned int			uiVideoPTS		= 0;

	int						iNewVideoDTS	= 0;
	int						iNewVideoPTS	= 0;

	unsigned int			uiWriteDuration	= 0;
	unsigned int			uiWriteSize		= 0;

	int 					iFrameNALHeaderSize	= m_pSource->getFrameNALHeaderLength();
	CClipVideoRenderInfo* pCurRenderInfo = NULL;			

	int						iSkipLevelForSlow = 0;
	int						iSkipCountForSlow = 0;
	NXBOOL					bReadError			= FALSE;

	NXBOOL	bIDRFrame = FALSE;
	NXBOOL	isFindIDRFrame = FALSE;

	int iVideoNalHeaderSize = m_pSource->getFrameNALHeaderLength();

	if( m_pClipItem == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) DE Task Start failed because clip is null(%p)", __LINE__, m_uiClipID, m_pClipItem);
		return 18181818;
	}

	if( m_pSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) DE Task Start failed because reader is null(%p)", __LINE__, m_uiClipID, m_pSource);
		return 18181818;
	}

	if( m_pFileWriter == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) DE Task Start failed because writer is null(%p)", __LINE__, m_uiClipID, m_pFileWriter);
		return 18181818;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) DE Task Start with info(basetime(%d) startTrim(%d) StartTime(%d) EndTime(%d) Speed(%d) )", 
		__LINE__, m_uiClipID, m_uiBaseTime, m_uiStartTrimTime, m_uiStartTime, m_uiEndTime, m_iSpeedFactor);

	unsigned int uiSeekResult = 0;
	m_pSource->seekTo(m_uiBaseTime, &uiSeekResult, NXFF_RA_MODE_CUR_PREV);

	if((pCurRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiSeekResult, TRUE)) == NULL)
		pCurRenderInfo = m_pClipItem->m_ClipVideoRenderInfoVec[0];

	if(m_pClipItem->getSlowMotion())
	{
		if(pCurRenderInfo->m_iSpeedCtlFactor == 3)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/32;
		else if(pCurRenderInfo->m_iSpeedCtlFactor == 6)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/16;
		else if(pCurRenderInfo->m_iSpeedCtlFactor == 13)
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/8;
		else
			iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30*pCurRenderInfo->m_iSpeedCtlFactor/100;
		iSkipCountForSlow = 0;
	}

	if( m_pSource->isAudioExist() ==  FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) Audio Track not exist", __LINE__);
		bAudioEnd = TRUE;
	}

	if( bAudioEnd == FALSE && m_bVideoOnlyMode ) {
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[DETask.cpp %d] ID(%d) Audio encode mode", __LINE__);
		bAudioEnd = TRUE;
	}
		
	while( m_bIsWorking )
	{
		if( bReadError )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) DE Task error exit with frame read failed", __LINE__, m_uiClipID);
			CNexProjectManager::sendEvent(MESSAGE_MAKE_DIRECT_EXPORT_DONE, NEXVIDEOEDITOR_ERROR_FILE_IO_FAILED, 0, 0);
			break;
		}
		// nexSAL_TaskSleep(1);
		if( bAudioEnd && bVideoEnd )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) DE Task process audio and video end(%d %d)", __LINE__, m_uiClipID, bAudioEnd, bVideoEnd);
			break;
		}

		if( bAudioEnd == FALSE )
		{
			if( iNewVideoPTS > (unsigned int)uiAudioDTS || bVideoEnd )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DETask.cpp %d] ID(%d) write AudioFrame start(Video:%d) (Audio:%lld) (%d)",
								__LINE__, m_uiClipID, iNewVideoPTS, uiAudioDTS, bVideoEnd);

				int iReaderRet = m_pSource->getAudioFrame();
				switch(iReaderRet)
				{
					case _SRC_GETFRAME_OK:
						break;
					case _SRC_GETFRAME_END:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) getAudioFrame End", __LINE__, m_uiClipID);
						bAudioReadEnd = TRUE;
						break;
					default:
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) getAudioFrame error(%d) so End clip", __LINE__, m_uiClipID, iReaderRet);
						bReadError = TRUE;
						continue;
				};

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DETask.cpp %d] ID(%d) Write AudioFrame getAudioFrame End(%d)", __LINE__, m_uiClipID);
				if( bAudioReadEnd )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) getAudioFrame EOS(%d)", __LINE__, m_uiClipID, m_uiEndTime);
					m_pFileWriter->writeAudioFrameWithoutEncode(m_uiClipID, m_uiEndTime, NULL, 0);
					bAudioEnd = TRUE;
					continue;
				}
				else
				{
					m_pSource->getAudioFrameCTS( &uiAudioDTS );

					if( m_uiStartTime > uiAudioDTS )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) Write AudioFrame Skip(%d %lld)", __LINE__, m_uiClipID, m_uiStartTime, uiAudioDTS);
						continue;
					}
					
					if( m_uiEndTime < ((unsigned int)uiAudioDTS + 10) )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) Write AudioFrame End(%d %d %lld)", __LINE__, m_uiClipID, m_uiEndTime, uiAudioDTS);
						m_pFileWriter->writeAudioFrameWithoutEncode(m_uiClipID, m_uiEndTime, NULL, 0);
						bAudioEnd = TRUE;
						continue;
					}

					m_pSource->getAudioFrameData( &pFrame, &uiFrameSize );
					m_pFileWriter->writeAudioFrameWithoutEncode(m_uiClipID, (unsigned int)uiAudioDTS, pFrame, uiFrameSize);
				}
				continue;
			}
		}

		if( bVideoEnd == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DETask.cpp %d] ID(%d) Write VideoFrame getVideoFrame start", __LINE__, m_uiClipID);
			int iReaderRet = m_pSource->getVideoFrame();
			switch(iReaderRet)
			{
				case _SRC_GETFRAME_OK:
					break;
				case _SRC_GETFRAME_END:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) getVideoFrame End", __LINE__, m_uiClipID);
					bVideoReadEnd = TRUE;
					break;
				default:
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) getVideoFrame error(%d) so End clip", __LINE__, m_uiClipID, iReaderRet);
					bReadError = TRUE;
					continue;
			};
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DETask.cpp %d] ID(%d) Write VideoFrame getVideoFrame End", __LINE__, m_uiClipID);

			if( bVideoReadEnd )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) getVideoFrame EOS(%d)", __LINE__, m_uiClipID, m_uiEndTime);
				m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, m_uiEndTime, m_uiEndTime, NULL, 0, &uiWriteDuration, &uiWriteSize);
				bVideoEnd = TRUE;
				continue;;
			}
			else
			{
				unsigned char* pTemp = NULL;
				CClipVideoRenderInfo* pRenderInfo = NULL;			
				m_pSource->getVideoFrameDTS( &uiVideoDTS, TRUE );
				m_pSource->getVideoFramePTS( &uiVideoPTS, TRUE);
				m_pSource->getVideoFrameData( &pFrame, &uiFrameSize );

				iNewVideoDTS = (int)m_CalcTime.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoDTS);
				iNewVideoPTS= (int)m_CalcTime.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);

				if(iNewVideoPTS > pCurRenderInfo->mEndTime)
				{
					pRenderInfo = m_pClipItem->getActiveVideoRenderInfo(uiVideoPTS, TRUE);
					if(pRenderInfo != NULL && pRenderInfo != pCurRenderInfo)
					{
						pCurRenderInfo = pRenderInfo;
						iNewVideoDTS = (int)m_CalcTime.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoDTS);
						iNewVideoPTS= (int)m_CalcTime.applySpeed(pCurRenderInfo->mStartTime, pCurRenderInfo->mStartTrimTime, pCurRenderInfo->m_iSpeedCtlFactor, uiVideoPTS);

						if(m_pClipItem->getSlowMotion())
						{
							if(pCurRenderInfo->m_iSpeedCtlFactor == 3)
								iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/32;
							else if(pCurRenderInfo->m_iSpeedCtlFactor == 6)
								iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/16;
							else if(pCurRenderInfo->m_iSpeedCtlFactor == 13)
								iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30/8;
							else
								iSkipLevelForSlow = (m_pSource->getVideoFrameRate() + 1)/30*pCurRenderInfo->m_iSpeedCtlFactor/100;
							isFindIDRFrame = TRUE;
						}
					}
				}

				if(m_pClipItem->getSlowMotion())
				{
					bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_pSource->getVideoObjectType(), pFrame, uiFrameSize, (void*)&iVideoNalHeaderSize, m_pSource->getFrameFormat(), TRUE);
					if( isFindIDRFrame )
					{
						if( bIDRFrame == FALSE )
						{
							continue;
						}
						isFindIDRFrame = FALSE;
					}

					if(bIDRFrame)
						iSkipCountForSlow = 0;
                
					if(m_pClipItem->getSlowMotion() && iSkipLevelForSlow > 1 && iSkipCountForSlow%iSkipLevelForSlow != 0)
					{
						iSkipCountForSlow++;
						continue;
					}
					iSkipCountForSlow++;
				}

				if( m_uiEndTime < (iNewVideoDTS + 10) )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) Write VideoFrame End(%d %d %d)", __LINE__, m_uiClipID, m_uiEndTime, iNewVideoDTS, uiVideoDTS);
					m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, m_uiEndTime, m_uiEndTime, NULL, 0, &uiWriteDuration, &uiWriteSize);
					bVideoEnd = TRUE;
					continue;
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[DETask.cpp %d] ID(%d) Write VideoFrame frame info(%p %d) (%d %d) (%d %d)",
					__LINE__, m_uiClipID, pFrame, uiFrameSize, uiVideoDTS, uiVideoPTS, iNewVideoDTS, iNewVideoPTS);

				if(m_pSource->getVideoObjectType()== eNEX_CODEC_V_H264 || m_pSource->getVideoObjectType() == eNEX_CODEC_V_HEVC)
				{
					if(iFrameNALHeaderSize < 4)	// consider 1 or 2 byte nalsize
					{
						pTemp = (unsigned char*)nexSAL_MemAlloc(uiFrameSize+32);

						for(int i=0; i<4-iFrameNALHeaderSize; i++)
						{
							pTemp[i] = 0;
						}

						memcpy(pTemp + (4- iFrameNALHeaderSize), pFrame, uiFrameSize);
						pFrame = pTemp;
						uiFrameSize = uiFrameSize + (4 - iFrameNALHeaderSize);
					}
				}
/*
				while( m_bIsWorking && m_bVideoOnlyMode && m_pFileWriter->getAudioTime() + 100 < iNewVideoPTS )
				{
					nexSAL_TaskSleep(10);
				}
*/
				m_pFileWriter->writeVideoFrameWithoutEncode(m_uiClipID, iNewVideoPTS, iNewVideoDTS, pFrame, uiFrameSize, &uiWriteDuration, &uiWriteSize);

				if( pTemp )
				{
					nexSAL_MemFree(pTemp);
					pTemp = NULL;
				}
			}			
		}
	}

	m_bDirectExportEnd = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[DETask.cpp %d] ID(%d) Video Task End and Release Codec", __LINE__, m_uiClipID);
	return 0;
}

