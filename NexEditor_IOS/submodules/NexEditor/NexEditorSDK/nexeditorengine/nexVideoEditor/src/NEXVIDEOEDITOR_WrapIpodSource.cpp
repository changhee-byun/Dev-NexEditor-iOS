 /******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapIpodSource.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
salabara		2016/08/21	Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_WrapIpodSource.h"
#include "NEXVIDEOEDITOR_Error.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"

#define IPODSOURCE_ONEFRAME_SIZE 4096
//---------------------------------------------------------------------------
CNexIpodSource::CNexIpodSource()
{
	m_uiClipID			= INVALID_CLIP_ID;
	m_isCreated			= FALSE;
	m_pIPodSource		= NULL;

	m_hIpodSourceMutex		= NEXSAL_INVALID_HANDLE;
	m_pFilePath			= NULL;

	m_uiTotalSourceSize			= 0;
	m_uiTotalSourceTime			= 0;

	m_isAudioExist				= FALSE;
	m_isVideoExist				= FALSE;
	m_isTextExist					= FALSE;

	m_pBuffVideo					= NULL;
	m_pBuffAudio					= NULL;
	m_pBuffText					= NULL;

	m_uBuffVideoSize				= 0;
	m_uBuffAudioSize				= 0;
	m_uBuffTextSize				= 0;

	m_uiBaseTimeStamp			= 0;
	m_uiTotalPlayAudioTime		= 0;
	m_uiTotalPlayVideoTime		= 0;
	m_uiTotalPlayTime				= 0;

	m_uiStartTrimTime				= 0;
	m_uiEndTrimTime				= 0;

	m_uiVideoObjectType			= 0;
	m_uiAudioObjectType			= 0;

	m_uiH264ProfileID				= 0;
	m_uiH264Level				= 0;
	m_isInterlaced				= FALSE;
	m_uiVideoBitRate				= 0;
	m_uiDSINALSize				= 0;
	m_uiFrameNALSize				= 0;

	m_uiFrameFormat				= NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;

	m_uiNumOfChannels			= 0;
	m_uiSamplingRate				= 0;
	m_uiBitsPerSample				= 0;
	m_uiNumOfSamplesPerChannel	= 0;
	m_uiAudioBitRate				= 0;

	m_uiTotalFrameCount         = 0;
	m_uiCountIframe				= 0;
	m_uiOrientation				= 0;
	m_uiFrameFPS				= 30;

	m_uiVideoLastReadDTS			= 0;
	m_uiVideoLastReadPTS			= 0;

	m_uiAudioLastReadCTS			= 0;

	m_uiLastAudioCTS				= 0;
	m_uiLastAudioCTSRead			= 0;
	m_uiAudioFrameInterval		= 0;
	m_dbAudioFrameInterval		= 0.0;
	m_uiMaxAudioFrameInterval		= 0;

	m_isAudioEnd					= FALSE;
	m_isVideoEnd					= FALSE;
	m_isTextEnd					= FALSE;

	m_uiVideoEditBoxTime = 0;
	m_uiAudioEditBoxTime = 0;

	m_iEncodeToTranscode	 = 0;
}

CNexIpodSource::~CNexIpodSource()
{
	deleteSource();

	if( m_pFilePath )
	{
		nexSAL_MemFree(m_pFilePath);
		m_pFilePath = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] ~~~~~~~~~~~~~~CNexIpodSource()", __LINE__ );
}

int CNexIpodSource::createSource()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] createIpodSource Start", __LINE__ );

	m_hIpodSourceMutex = nexSAL_MutexCreate();
	if( m_hIpodSourceMutex == NEXSAL_INVALID_HANDLE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] createIpodSource - Create Mutex Error", __LINE__ );
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED; 
	}

	#if 0
	m_pBuffVideo = (unsigned char*)nexSAL_MemAlloc( DEFAULT_FRAME_BUFF_SIZE_VIDEO );
	if ( m_pBuffVideo )
	{
		memset( m_pBuffVideo, 0x00, DEFAULT_FRAME_BUFF_SIZE_VIDEO);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] createIpodSource - Alloc Error", __LINE__ );
		goto Create_Error;
	}
	#endif

	m_pBuffAudio = (unsigned char*)nexSAL_MemAlloc( DEFAULT_FRAME_BUFF_SIZE_AUDIO );
	if ( m_pBuffAudio )
	{
		memset( m_pBuffAudio, 0x00, DEFAULT_FRAME_BUFF_SIZE_AUDIO);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] createIpodSource - Alloc Error", __LINE__ );
		goto Create_Error;
	}

	#if 0
	m_pBuffText = (unsigned char*)nexSAL_MemAlloc( DEFAULT_FRAME_BUFF_SIZE_TEXT );
	if ( m_pBuffText )
	{
		memset( m_pBuffText, 0x00, DEFAULT_FRAME_BUFF_SIZE_TEXT);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] createIpodSource - Alloc Error", __LINE__ );
		goto Create_Error;
	}
	#endif

	m_isCreated = TRUE;

	return NEXVIDEOEDITOR_ERROR_NONE;

Create_Error:
	if( m_hIpodSourceMutex != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MutexDelete( m_hIpodSourceMutex );
		m_hIpodSourceMutex = NEXSAL_INVALID_HANDLE;
	}

	if( m_pBuffVideo )
	{
		nexSAL_MemFree( m_pBuffVideo );
		m_pBuffVideo = NULL;
	}

	if( m_pBuffAudio )
	{
		nexSAL_MemFree( m_pBuffAudio );
		m_pBuffAudio = NULL;
	}

	if ( m_pBuffText )
	{
		nexSAL_MemFree( m_pBuffText );
		m_pBuffText = NULL;
	}

	return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED; 	
}

int CNexIpodSource::deleteSource()
{
	closeFile();

	if( m_pBuffVideo )
	{
		nexSAL_MemFree( m_pBuffVideo );
		m_pBuffVideo = NULL;
	}

	if( m_pBuffAudio )
	{
		nexSAL_MemFree( m_pBuffAudio );
		m_pBuffAudio = NULL;
	}

	if ( m_pBuffText )
	{
		nexSAL_MemFree( m_pBuffText );
		m_pBuffText = NULL;
	}

	if( m_hIpodSourceMutex != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MutexDelete( m_hIpodSourceMutex );
		m_hIpodSourceMutex = NEXSAL_INVALID_HANDLE;
	}
	
	m_isCreated = FALSE;
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CNexIpodSource::getClipID()
{
	return m_uiClipID;
}

void CNexIpodSource::setClipID(unsigned int uiClipID)
{
	m_uiClipID = uiClipID;
}

void CNexIpodSource::setEncodeToTranscode(int iValue)
{
	m_iEncodeToTranscode = iValue;
}

int CNexIpodSource::openFile(char* pURL, unsigned int uiUrlLength)
{
	NEX_FF_RETURN		eiRet					= eNEX_FF_RET_FAIL;
	unsigned int			uiSupportedFileType		= 0;
	unsigned int			uiRet					= 0;
	unsigned int			uiCodecType				= 0;
	unsigned int			uiSamplesPerChannel	= 0;
	unsigned int			uiSamplingrate			= 0;
	unsigned int			uiNumOfChannels		= 0;
	NEXVIDEOEDITOR_ERROR	eRet = NEXVIDEOEDITOR_ERROR_NONE;
	NxFFReaderFileAPI		readerFileAPIs;

	if( pURL == NULL || uiUrlLength <= 0 )
	{
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( m_pFilePath )
	{
		nexSAL_MemFree(m_pFilePath);
		m_pFilePath = NULL;
	}

	m_pFilePath = (char*)nexSAL_MemAlloc(uiUrlLength + 1);
	if( m_pFilePath == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] malloc file path failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	strcpy(m_pFilePath, pURL);
	m_pFilePath[uiUrlLength] = '\0';

	m_pIPodSource = new IPodLibraryReader;
	if(m_pIPodSource == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] IPod Source init fail", __LINE__);
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}
  
	m_pIPodSource->openURL(m_pFilePath);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] Clip open(%s)", __LINE__, pURL);

	m_isAudioExist = 1;
	m_isVideoExist = 0;
	m_isTextExist = 0;

	if( m_isAudioExist )
	{
		m_uiAudioObjectType = eNEX_CODEC_A_PCM_S16LE;
		m_uiTotalSourceSize = (int)m_pIPodSource->getTotalSize();
        
		m_uiNumOfChannels = m_pIPodSource->getNumOfChannels();
		m_uiSamplingRate = m_pIPodSource->getSampleRate();
		m_uiTotalPlayTime = m_pIPodSource->getDuration();
		m_uiTotalPlayAudioTime = m_pIPodSource->getDuration();
		m_uiAudioBitRate = m_pIPodSource->getBitrate();
		m_uiTotalFrameCount = m_pIPodSource->getTotalFrameCount();
        
		m_uiBitsPerSample = 16;
		m_uiNumOfSamplesPerChannel = 1024;
		m_uiTotalSourceTime = m_uiTotalSourceSize/m_uiNumOfChannels/(m_uiBitsPerSample>> 3)*1000/m_uiSamplingRate;
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] Audio meta Info(%d %d %d %d)", 
				__LINE__, 
				m_uiSamplingRate,
				m_uiNumOfChannels,
				m_uiNumOfSamplesPerChannel,
				m_uiAudioBitRate);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] Audio duration(%d), (%d)", __LINE__, m_uiTotalPlayAudioTime, m_uiTotalSourceTime);
		m_uiTotalPlayTime = m_uiTotalPlayTime > m_uiTotalPlayAudioTime ? m_uiTotalPlayTime : m_uiTotalPlayAudioTime;

		m_uiAudioFrameInterval		= 0;
		m_dbAudioFrameInterval		= 0.0;
		m_uiMaxAudioFrameInterval		= 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] Open File sucessed Dur(%d)", __LINE__, m_uiTotalPlayTime);
	return eRet;
}

int CNexIpodSource::closeFile()
{
	if( m_pIPodSource == NULL  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] closeFile End", __LINE__ );
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	if(m_pIPodSource)
	{
		m_pIPodSource->close();
		m_pIPodSource = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] closeFile End", __LINE__ );
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::isEndOfChannel(NXFF_MEDIA_TYPE eType, NXBOOL* pisResult )
{
	if( m_pIPodSource == NULL || pisResult == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	switch( eType )
	{
	case NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO :
		if( m_isVideoEnd || !m_isVideoExist )	 *pisResult = 1;
		else *pisResult	= 0;
		break;
	case NXFF_MEDIA_TYPE_AUDIO :
		if( m_isAudioEnd )  *pisResult = 1;
		else *pisResult	= 0;
		break;
	case NXFF_MEDIA_TYPE_TEXT :
		if( m_isTextEnd ) *pisResult = 1;
		else *pisResult	= 0;
		break;
	default:
		*pisResult = FALSE;
		break;
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getDSI(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize )
{
	if( m_pIPodSource == NULL || ppDSI == NULL || pDSISize == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*ppDSI = NULL;
	*pDSISize = 0;
	return 1;
}

int CNexIpodSource::seekTo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pIPodSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	unsigned int uiResultAudioCTS	= 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekTo(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);

	uiTime = uiTime <= 1 ? 0 : uiTime;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekTo(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( bRepeat == FALSE )
	{
		uiTime = (uiTime + m_uiStartTrimTime) > m_uiBaseTimeStamp ?  (uiTime + m_uiStartTrimTime) - m_uiBaseTimeStamp : 0;
	}

	if( bABSTime && uiTime > (m_uiTotalPlayTime - m_uiEndTrimTime) )
	{
		uiTime = m_uiTotalPlayTime > (m_uiEndTrimTime + 1000) ? m_uiTotalPlayTime - (m_uiEndTrimTime + 1000) : 0;
	}

	nexSAL_MutexLock( m_hIpodSourceMutex, NEXSAL_INFINITE );
	if( m_isAudioExist )
	{
		int iPos = m_uiTotalFrameCount * ((double)uiTime / (double)m_uiTotalPlayAudioTime);
        
		m_pIPodSource->seek(&iPos);
        
		uiResultAudioCTS = m_uiTotalPlayAudioTime * (double)iPos / (double)m_uiTotalFrameCount;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekTo(A) Target:%d Result(%d)", __LINE__, uiTime, uiResultAudioCTS);
		*pSeekResultCTS = uiResultAudioCTS;
	}
	nexSAL_MutexUnlock( m_hIpodSourceMutex );
	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekTo result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	m_isAudioEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;
	m_uiAudioLastReadCTS	= uiResultAudioCTS;
	m_uiLastAudioCTS		= m_uiAudioLastReadCTS;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekTo last read CTS and PTS(%u, %u) result(%d)",
		__LINE__, m_uiVideoLastReadDTS, m_uiVideoLastReadPTS, *pSeekResultCTS);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::seekToAudio(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pIPodSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultAudioCTS		= 0;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToAudio(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToAudio(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( m_isAudioExist == FALSE )
	{
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	}

	if( bRepeat == FALSE )
	{
		uiTime = (uiTime + m_uiStartTrimTime) > m_uiBaseTimeStamp ?  (uiTime + m_uiStartTrimTime) - m_uiBaseTimeStamp : 0;
	}
	
	if( bABSTime && uiTime > (m_uiTotalPlayTime - m_uiEndTrimTime) )
	{
		uiTime = m_uiTotalPlayTime > (m_uiEndTrimTime + 1000) ? m_uiTotalPlayTime - (m_uiEndTrimTime + 1000) : 0;
	}

	nexSAL_MutexLock( m_hIpodSourceMutex, NEXSAL_INFINITE );
	int iPos = 0;

    iPos = m_uiTotalFrameCount * ((double)uiTime / (double)m_uiTotalPlayAudioTime);
    
    m_pIPodSource->seek(&iPos);

    uiResultAudioCTS = m_uiTotalPlayAudioTime * (double)iPos / (double)m_uiTotalFrameCount;
    
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToAudio(A) Target:%d Result(%d)", __LINE__, uiTime, uiResultAudioCTS);
	*pSeekResultCTS = uiResultAudioCTS;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToAudio(A) Target:%d Result(%d)", __LINE__, uiTime, uiResultAudioCTS);
	*pSeekResultCTS = uiResultAudioCTS;
	nexSAL_MutexUnlock( m_hIpodSourceMutex );

	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToAudio result(%d)", __LINE__, *pSeekResultCTS);

	m_isAudioEnd = 0;
	
	m_uiAudioLastReadCTS	= uiResultAudioCTS;
	m_uiLastAudioCTS = m_uiAudioLastReadCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::seekToVideo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pIPodSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToVideo(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToVideo(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( m_isVideoExist == FALSE )
	{
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	}

	if( bRepeat == FALSE )
	{
		uiTime = uiTime + m_uiStartTrimTime - m_uiBaseTimeStamp;
	}
	
	if( bABSTime && uiTime > (m_uiTotalPlayTime - m_uiEndTrimTime) )
	{
		uiTime = m_uiTotalPlayTime - (m_uiEndTrimTime + 1000);
	}

	nexSAL_MutexLock( m_hIpodSourceMutex, NEXSAL_INFINITE );
	*pSeekResultCTS = uiResultVideoCTS;
	nexSAL_MutexUnlock( m_hIpodSourceMutex );
	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToVideo result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;	
}

int CNexIpodSource::seekToVideoWithAbsoluteTime(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode)
{
	if( m_pIPodSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	NEX_FF_RETURN eFFRet = eNEX_FF_RET_FAIL;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToVideoWithAbsoluteTime(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToVideoWithAbsoluteTime(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( m_isVideoExist == FALSE )
	{
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	}

	nexSAL_MutexLock( m_hIpodSourceMutex, NEXSAL_INFINITE );
	*pSeekResultCTS = uiResultVideoCTS;
	nexSAL_MutexUnlock( m_hIpodSourceMutex );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] seekToVideoWithAbsoluteTime result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;	
}

int CNexIpodSource::getFindNearIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime)
{
	*pResultTime = 0;
	return 0;
}

int CNexIpodSource::getSeekTable(unsigned int uiStartTime, int* pSeekTableCount, unsigned int** ppSeekTable)
{
	*pSeekTableCount = 0;
	*ppSeekTable = NULL;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getSeekTable(unsigned int uiStartTime, unsigned int uiEndTime, int iCount, int* pSeekTableCount, unsigned int** ppSeekTable)
{
	*pSeekTableCount = 0;
	*ppSeekTable = NULL;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getVideoFrame(NXBOOL isEnhanced, NXBOOL bForceRead)
{
	return _SRC_GETFRAME_END;
}

int CNexIpodSource::getVideoFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize )
{
	*ppFrame = NULL;
	*pFrameSize = 0;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getVideoFrameDTS(NXUINT32* puDTS, NXBOOL bReadTime)
{
	*puDTS = 0;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getVideoFramePTS(NXUINT32* puPTS, NXBOOL bReadTime)
{
	*puPTS = 0;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getAudioFrame()
{
	if( m_pIPodSource == NULL  )
		return _SRC_GETFRAME_ERROR;

	if( m_isAudioExist && m_isAudioEnd )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] Audio End(%d %d)", __LINE__, m_isAudioExist , m_isAudioEnd);
		return _SRC_GETFRAME_END;
	}

	int iRet = eNEX_FF_RET_SUCCESS, iPos = 0;
	nexSAL_MutexLock( m_hIpodSourceMutex, NEXSAL_INFINITE );
    
    m_pIPodSource->ftell(&iPos);
    
    m_uBuffAudioSize = IPODSOURCE_ONEFRAME_SIZE / (2 * m_uiNumOfChannels);
    
    // Ï?�´Î≤àÏß???�Ïû�?sample (in): wantRead (out):actuallyRead
    m_pIPodSource->read(&m_uBuffAudioSize, (unsigned short*)m_pBuffAudio);
    m_uBuffAudioSize *= 2 * m_uiNumOfChannels;	 
	if(m_uBuffAudioSize == 0)
	{
		iRet = _SRC_GETFRAME_END;		
		m_isAudioEnd = 1;
        m_pIPodSource->resetBufferSize();
	}
	else
	{
        m_uiAudioLastReadCTS = m_uiTotalPlayAudioTime * ((double)iPos / (double)m_uiTotalFrameCount);
	}
	nexSAL_MutexUnlock( m_hIpodSourceMutex );

	if( m_uiAudioLastReadCTS > (m_uiTotalPlayTime - m_uiEndTrimTime + 300))
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getAudioFrame End(Content play end %d %d, %d, %d)", __LINE__, m_uiAudioLastReadCTS, m_uiBaseTimeStamp, m_uiStartTrimTime,  m_uiTotalPlayTime - m_uiEndTrimTime);
		m_isAudioEnd = 1;
		iRet = _SRC_GETFRAME_END;
	}	
	
	return iRet; 
}

int CNexIpodSource::getAudioFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize )
{
	if( m_pIPodSource == NULL  )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	*ppFrame = (unsigned char*)m_pBuffAudio;
	*pFrameSize = m_uBuffAudioSize;
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getAudioFrameCTS(NXINT64* puCTS )
{
	if( m_pIPodSource == NULL  )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

    *puCTS = (m_uiAudioLastReadCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNexIpodSource::isAudioExist()
{
	return m_isAudioExist;
}

NXBOOL CNexIpodSource::isVideoExist()
{
	return m_isVideoExist;
}

NXBOOL CNexIpodSource::isTextExist()
{
	return m_isTextExist;
}

unsigned int CNexIpodSource::getIDRFrameCount()
{
	return m_uiCountIframe;
}

unsigned int CNexIpodSource::getFrameFormat()
{
	return m_uiFrameFormat;
}

NXUINT32 CNexIpodSource::parseFrameFormat(NXUINT8 *pFrame, NXUINT32 uFrameSize)
{
	NXUINT32	uFrameFormat=NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANY;

#ifdef DISABLE_MIXED_RAW_AND_ANNEXB_NAL
	uFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameSize);
#else
	if((uFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameSize)) == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW)
	{
		int iCodeLen = 0;
		NexCodecUtil_FindAnnexBStartCode(pFrame, 4, uFrameSize > 200?200:uFrameSize, &iCodeLen);
		//iCodeLen = getAnnexBStartCode(m_uiVideoObjectType, pFrame, uFrameSize, m_uiFrameNALSize);     		
	
		if(iCodeLen == 4)
		{
			uFrameFormat = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
		}
	}
#endif

	return uFrameFormat;
}

int CNexIpodSource::getVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight )
{
	if( m_pIPodSource == NULL || pusWidth == NULL || pusHeight == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*pusWidth = 0;
	*pusHeight = 0;	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getDisplayVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight )
{
	if( m_pIPodSource == NULL || pusWidth == NULL || pusHeight == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*pusWidth = 0;
	*pusHeight = 0;	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getAudioBitrate(NXUINT32* puAudioBitrate )
{
	if( m_pIPodSource == NULL || puAudioBitrate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puAudioBitrate = m_uiAudioBitRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getVideoBitrate(NXUINT32* puVideoBitrate )
{
	if( m_pIPodSource == NULL || puVideoBitrate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puVideoBitrate = m_uiVideoBitRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getTotalPlayTime(NXUINT32* puTotalPlayTime )
{
	if( m_pIPodSource == NULL || puTotalPlayTime == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puTotalPlayTime = m_uiTotalPlayTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getSamplingRate(NXUINT32* puSamplingRate )
{
	if( m_pIPodSource == NULL || puSamplingRate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puSamplingRate = m_uiSamplingRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getNumberOfChannel(NXUINT32* puNumOfChannels )
{
	if( m_pIPodSource == NULL || puNumOfChannels == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puNumOfChannels = m_uiNumOfChannels;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getSamplesPerChannel(NXUINT32* piNumOfSamplesPerChannel )
{
	if( m_pIPodSource == NULL || piNumOfSamplesPerChannel == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*piNumOfSamplesPerChannel = m_uiNumOfSamplesPerChannel;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getDSINALHeaderLength()
{
	return m_uiDSINALSize;
}

int CNexIpodSource::getFrameNALHeaderLength()
{
	return m_uiFrameNALSize;
}

int CNexIpodSource::getH264ProfileLevelID(unsigned int* puH264ProfileLevelID )
{
	if( m_pIPodSource == NULL || puH264ProfileLevelID == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264ProfileLevelID = m_uiH264ProfileID;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getH264Level(unsigned int* puH264Level )
{
	if( m_pIPodSource == NULL || puH264Level == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264Level = m_uiH264Level;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getVideoH264Interlaced(NXBOOL* puH264Interlaced )
{
	if( m_pIPodSource == NULL || puH264Interlaced == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264Interlaced = m_isInterlaced;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getWAVEFormat (unsigned int *pFormatTag, unsigned int* pBitsPerSample, unsigned int *pBlockAlign, unsigned int *pAvgBytesPerSec, unsigned int *pSamplesPerBlock, unsigned int *pEncodeOpt, unsigned int *pExtraSize, unsigned char **ppExtraData )
{
	if( m_pIPodSource == NULL || pFormatTag == NULL || pBitsPerSample == NULL || pBlockAlign == NULL || 
		pAvgBytesPerSec == NULL || pSamplesPerBlock == NULL || pEncodeOpt == NULL || pExtraSize == NULL || ppExtraData == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*pFormatTag = 0;
	*pBitsPerSample = m_uiBitsPerSample;
	*pBlockAlign = 0;
	*pAvgBytesPerSec = 0;
	*pSamplesPerBlock = 0;
	*pEncodeOpt = 0;
	*pExtraSize = 0;
	*ppExtraData = NULL;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::setBaseTimeStamp(unsigned int uiBaseTime)
{
	m_uiBaseTimeStamp = uiBaseTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::setTrimTime(unsigned int uiStartTrimTime, unsigned int uiEndTrimTime)
{
	m_uiStartTrimTime		= uiStartTrimTime;
	m_uiEndTrimTime		= uiEndTrimTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}
	
NXBOOL CNexIpodSource::setRepeatAudioClip(unsigned int uiBaseTime, unsigned int uiStartTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CNexIpodSource.cpp %d] setRepeatAudioClip(Base:%d StartTime %d)", __LINE__, uiBaseTime, uiStartTime);
	unsigned int uiSeekResultCTS	= 0;
	unsigned int uiTempBase		= m_uiBaseTimeStamp;

	m_uiBaseTimeStamp = uiBaseTime;
	if( seekToAudio(uiStartTime, &uiSeekResultCTS, NXFF_RA_MODE_PREV, TRUE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CNexIpodSource.cpp %d] do not repeat because clip did not seek", __LINE__);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] repeat Audio Clip baseTime(%d->%d) SeekR(%d)", __LINE__, uiTempBase, m_uiBaseTimeStamp, uiSeekResultCTS);
	m_isAudioEnd			= 0;
	return TRUE;
}

int CNexIpodSource::getReaderRotateState()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getRotateState(%d)", __LINE__, m_uiOrientation);
	return m_uiOrientation;
}

int CNexIpodSource::getRotateState()
{
	int iOrientation = m_uiOrientation;

	if( m_uiOrientation == 90 )
	{
		iOrientation = 270;
	}
	else if( m_uiOrientation == 270 )
	{
		iOrientation = 90;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getRotateState(%d %d)", __LINE__, m_uiOrientation, iOrientation);
	return iOrientation;
}

unsigned int CNexIpodSource::getVideoFrameRate()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getVideoFrameRate(%d)", __LINE__, m_uiFrameFPS);
	return m_uiFrameFPS;
}

float CNexIpodSource::getVideoFrameRateFloat()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getVideoFrameRate(%f)", __LINE__, m_fFrameFPS);
	return m_fFrameFPS;
}

unsigned int CNexIpodSource::getTotalPlayAudioTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getTotalPlayAudioTime(%d)", __LINE__, m_uiTotalPlayAudioTime);
	return m_uiTotalPlayAudioTime;
}

unsigned int CNexIpodSource::getTotalPlayVideoTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getTotalPlayVideoTime(%d)", __LINE__, m_uiTotalPlayVideoTime);
	return m_uiTotalPlayVideoTime;
}

int CNexIpodSource::getAudioTrackCount()
{
	int iCount = 1;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getAudioTrackCount(%d)", __LINE__, iCount);
	return iCount;
}

int CNexIpodSource::getVideoTrackCount()
{
	int iCount = 1;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getVideoTrackCount(%d)", __LINE__, iCount);
	return iCount;
}

//yoon
int  CNexIpodSource::getVideoTrackUUID( NXUINT8** ppretAllocUUID, NXINT32* pretUUIDSize )
{
	NXINT32 i32Size = 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d] getVideoTrackUUID() start", __LINE__ );	
	if( m_pIPodSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d]getVideoTrackUUID() FFReader no instans", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( !m_isVideoExist )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexIpodSource.cpp %d]getVideoTrackUUID() no video", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	*pretUUIDSize = 0;
	*ppretAllocUUID = NULL;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexIpodSource::getEditBoxTime(unsigned int* puiAudioTime, unsigned int* puiVideoTime)
{
	*puiAudioTime = 0;
	*puiVideoTime = 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexIpodSource.cpp %d] getEditBoxTime(%d, %d)", __LINE__, *puiAudioTime, *puiVideoTime);
	return 0;
}

int CNexIpodSource::getAudioObjectType()
{
	return m_uiAudioObjectType;
}

int CNexIpodSource::getVideoObjectType()
{
	return m_uiVideoObjectType;
}

void CNexIpodSource::setAudioObjectType(unsigned int uiAudioObjectType)
{
	m_uiAudioObjectType = uiAudioObjectType;
}

void CNexIpodSource::setNumOfChannels(unsigned int uiNumOfChannels)
{
	m_uiNumOfChannels = uiNumOfChannels;
}

void CNexIpodSource::setSamplingRate(unsigned int uiSamplingRate)
{
	m_uiSamplingRate = uiSamplingRate;
}

void CNexIpodSource::setNumOfSamplesPerChannel(unsigned int uiNumOfSamplesPerChannel)
{
	m_uiNumOfSamplesPerChannel = uiNumOfSamplesPerChannel;

	m_uiAudioFrameInterval		= _GetFrameInterval(m_uiNumOfSamplesPerChannel, m_uiSamplingRate);
	m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiNumOfSamplesPerChannel, m_uiSamplingRate);
	m_uiMaxAudioFrameInterval	= (m_uiAudioFrameInterval*3) >> 1;			

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CNexIpodSource.cpp %d] Interval(%d %f %d)",  __LINE__, 
		m_uiAudioFrameInterval, 
		m_dbAudioFrameInterval, 
		m_uiMaxAudioFrameInterval);
}

NXBOOL CNexIpodSource::getEnhancedExist()
{
	return 0;
}

unsigned int CNexIpodSource::getVideoEditBox()
{
	return 0;
}

unsigned int CNexIpodSource::getAudioEditBox()
{
	return 0;
}

int CNexIpodSource::getTS(NXFF_MEDIA_TYPE eType, unsigned int* piTS )
{
	*piTS = 0;
	return 0;    
}

int CNexIpodSource::getFindIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime)
{
	return 0;
}