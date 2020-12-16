 /******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapSource.cpp
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

#include "NEXVIDEOEDITOR_WrapOutSource.h"
#include "NEXVIDEOEDITOR_Error.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"

#define OUTSOURCE_HEADER_SIZE 20
#define OUTSOURCE_ONEFRAME_SIZE 4096
//---------------------------------------------------------------------------
CNexOutSource::CNexOutSource()
{
	m_uiClipID			= INVALID_CLIP_ID;
	m_isCreated			= FALSE;
	m_pOutSource		= NULL;

	m_hOutSourceMutex		= NEXSAL_INVALID_HANDLE;
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

CNexOutSource::~CNexOutSource()
{
	deleteSource();

	if( m_pFilePath )
	{
		nexSAL_MemFree(m_pFilePath);
		m_pFilePath = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] ~~~~~~~~~~~~~~CNexOutSource()", __LINE__ );
}

int CNexOutSource::createSource()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] createOutSource Start", __LINE__ );

	m_hOutSourceMutex = nexSAL_MutexCreate();
	if( m_hOutSourceMutex == NEXSAL_INVALID_HANDLE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] createOutSource - Create Mutex Error", __LINE__ );
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
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] createOutSource - Alloc Error", __LINE__ );
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
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] createOutSource - Alloc Error", __LINE__ );
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
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] createOutSource - Alloc Error", __LINE__ );
		goto Create_Error;
	}
	#endif

	m_isCreated = TRUE;

	return NEXVIDEOEDITOR_ERROR_NONE;

Create_Error:
	if( m_hOutSourceMutex != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MutexDelete( m_hOutSourceMutex );
		m_hOutSourceMutex = NEXSAL_INVALID_HANDLE;
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

int CNexOutSource::deleteSource()
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

	if( m_hOutSourceMutex != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MutexDelete( m_hOutSourceMutex );
		m_hOutSourceMutex = NEXSAL_INVALID_HANDLE;
	}
	
	m_isCreated = FALSE;
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CNexOutSource::getClipID()
{
	return m_uiClipID;
}

void CNexOutSource::setClipID(unsigned int uiClipID)
{
	m_uiClipID = uiClipID;
}

void CNexOutSource::setEncodeToTranscode(int iValue)
{
	m_iEncodeToTranscode = iValue;
}

int CNexOutSource::openFile(char* pURL, unsigned int uiUrlLength)
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
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] malloc file path failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}
	strcpy(m_pFilePath, pURL);
	m_pFilePath[uiUrlLength] = '\0';

	m_pOutSource = nexSAL_FileOpen(m_pFilePath, NEXSAL_FILE_READ);

	if( m_pOutSource == NEXSAL_INVALID_HANDLE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] file does not exist(%s)", __LINE__, m_pFilePath);
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}
  
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] Clip open(%s)", __LINE__, pURL);

	m_isAudioExist = 1;
	m_isVideoExist = 0;
	m_isTextExist = 0;

	if( m_isAudioExist )
	{
		m_uiTotalSourceSize = nexSAL_FileSize(m_pOutSource);
		if(m_uiTotalSourceSize > OUTSOURCE_ONEFRAME_SIZE + OUTSOURCE_HEADER_SIZE)
			m_uiTotalSourceSize -= OUTSOURCE_HEADER_SIZE;
		else
			return NEXVIDEOEDITOR_ERROR_UNSUPPORT_FORMAT;

		nexSAL_FileRead(m_pOutSource, &m_uiAudioObjectType, sizeof(m_uiAudioObjectType));
		m_uiAudioObjectType = eNEX_CODEC_A_PCM_S16LE;

		nexSAL_FileRead(m_pOutSource, &m_uiSamplingRate, sizeof(m_uiSamplingRate));
		nexSAL_FileRead(m_pOutSource, &m_uiNumOfChannels, sizeof(m_uiNumOfChannels));
		nexSAL_FileRead(m_pOutSource, &m_uiAudioBitRate, sizeof(m_uiAudioBitRate));
		nexSAL_FileRead(m_pOutSource, &m_uiTotalPlayAudioTime, sizeof(m_uiTotalPlayAudioTime));
		m_uiBitsPerSample = 16;
		m_uiNumOfSamplesPerChannel = 1024;
		m_uiTotalSourceTime = m_uiTotalSourceSize/m_uiNumOfChannels/(m_uiBitsPerSample>> 3)*1000/m_uiSamplingRate;
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] Audio meta Info(%d %d %d %d)", 
				__LINE__, 
				m_uiSamplingRate,
				m_uiNumOfChannels,
				m_uiNumOfSamplesPerChannel,
				m_uiAudioBitRate);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] Audio duration(%d), (%d)", __LINE__, m_uiTotalPlayAudioTime, m_uiTotalSourceTime);
		m_uiTotalPlayTime = m_uiTotalPlayTime > m_uiTotalPlayAudioTime ? m_uiTotalPlayTime : m_uiTotalPlayAudioTime;

		m_uiAudioFrameInterval		= 0;
		m_dbAudioFrameInterval		= 0.0;
		m_uiMaxAudioFrameInterval		= 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] Open File sucessed Dur(%d)", __LINE__, m_uiTotalPlayTime);
	return eRet;
}

int CNexOutSource::closeFile()
{
	if( m_pOutSource == NULL  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] closeFile End", __LINE__ );
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	if(m_pOutSource)
	{
		nexSAL_FileClose(m_pOutSource);
		m_pOutSource = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] closeFile End", __LINE__ );
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::isEndOfChannel(NXFF_MEDIA_TYPE eType, NXBOOL* pisResult )
{
	if( m_pOutSource == NULL || pisResult == NULL )
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

int CNexOutSource::getDSI(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize )
{
	if( m_pOutSource == NULL || ppDSI == NULL || pDSISize == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*ppDSI = NULL;
	*pDSISize = 0;
    return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::seekTo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pOutSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	unsigned int uiResultAudioCTS	= 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekTo(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);

	uiTime = uiTime <= 1 ? 0 : uiTime;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekTo(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( bRepeat == FALSE )
	{
		uiTime = (uiTime + m_uiStartTrimTime) > m_uiBaseTimeStamp ?  (uiTime + m_uiStartTrimTime) - m_uiBaseTimeStamp : 0;
	}

	if( bABSTime && uiTime > (m_uiTotalPlayTime - m_uiEndTrimTime) )
	{
		uiTime = m_uiTotalPlayTime > (m_uiEndTrimTime + 1000) ? m_uiTotalPlayTime - (m_uiEndTrimTime + 1000) : 0;
	}

	nexSAL_MutexLock( m_hOutSourceMutex, NEXSAL_INFINITE );
	if( m_isAudioExist )
	{
		unsigned int uiSize = 0;
		uiSize = uiTime * m_uiSamplingRate/(1000/m_uiNumOfChannels/(m_uiBitsPerSample>> 3));

		if(uiSize > m_uiTotalSourceSize - OUTSOURCE_ONEFRAME_SIZE)
			uiSize = m_uiTotalSourceSize - OUTSOURCE_ONEFRAME_SIZE - OUTSOURCE_HEADER_SIZE;

		nexSAL_FileSeek(m_pOutSource, uiSize, NEXSAL_SEEK_BEGIN);
		uiResultAudioCTS = uiSize/m_uiNumOfChannels/(m_uiBitsPerSample>> 3)*1000/m_uiSamplingRate;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekTo(A) Target:%d Result(%d)", __LINE__, uiTime, uiResultAudioCTS);
		*pSeekResultCTS = uiResultAudioCTS;
	}
	nexSAL_MutexUnlock( m_hOutSourceMutex );
	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekTo result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	m_isAudioEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;
	m_uiAudioLastReadCTS	= uiResultAudioCTS;
	m_uiLastAudioCTS		= m_uiAudioLastReadCTS;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekTo last read CTS and PTS(%u, %u) result(%d)",
		__LINE__, m_uiVideoLastReadDTS, m_uiVideoLastReadPTS, *pSeekResultCTS);

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::seekToAudio(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pOutSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultAudioCTS		= 0;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToAudio(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToAudio(Seek:%d Total(%d) BaseTime(%d))", 
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

	nexSAL_MutexLock( m_hOutSourceMutex, NEXSAL_INFINITE );
	unsigned int uiSize = 0;
	uiSize = uiTime * m_uiSamplingRate/(1000/m_uiNumOfChannels/(m_uiBitsPerSample>> 3));

	if(uiSize > m_uiTotalSourceSize - OUTSOURCE_ONEFRAME_SIZE)
		uiSize = m_uiTotalSourceSize - OUTSOURCE_ONEFRAME_SIZE - OUTSOURCE_HEADER_SIZE;

	uiSize = nexSAL_FileSeek(m_pOutSource, uiSize + OUTSOURCE_HEADER_SIZE, NEXSAL_SEEK_BEGIN);
	uiResultAudioCTS = uiSize/m_uiNumOfChannels/(m_uiBitsPerSample>> 3)*1000/m_uiSamplingRate;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToAudio(A) Target:%d Result(%d)", __LINE__, uiTime, uiResultAudioCTS);
	*pSeekResultCTS = uiResultAudioCTS;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToAudio(A) Target:%d Result(%d)", __LINE__, uiTime, uiResultAudioCTS);
	*pSeekResultCTS = uiResultAudioCTS;
	nexSAL_MutexUnlock( m_hOutSourceMutex );

	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToAudio result(%d)", __LINE__, *pSeekResultCTS);

	m_isAudioEnd = 0;
	
	m_uiAudioLastReadCTS	= uiResultAudioCTS;
	m_uiLastAudioCTS = m_uiAudioLastReadCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;}

int CNexOutSource::seekToVideo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pOutSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToVideo(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToVideo(Seek:%d Total(%d) BaseTime(%d))", 
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

	nexSAL_MutexLock( m_hOutSourceMutex, NEXSAL_INFINITE );
	*pSeekResultCTS = uiResultVideoCTS;
	nexSAL_MutexUnlock( m_hOutSourceMutex );
	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToVideo result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;	
}

int CNexOutSource::seekToVideoWithAbsoluteTime(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode)
{
	if( m_pOutSource == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	NEX_FF_RETURN eFFRet = eNEX_FF_RET_FAIL;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToVideoWithAbsoluteTime(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToVideoWithAbsoluteTime(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( m_isVideoExist == FALSE )
	{
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	}

	nexSAL_MutexLock( m_hOutSourceMutex, NEXSAL_INFINITE );
	*pSeekResultCTS = uiResultVideoCTS;
	nexSAL_MutexUnlock( m_hOutSourceMutex );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] seekToVideoWithAbsoluteTime result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;	
}

int CNexOutSource::getFindNearIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime)
{
	*pResultTime = 0;
	return 0;
}

int CNexOutSource::getSeekTable(unsigned int uiStartTime, int* pSeekTableCount, unsigned int** ppSeekTable)
{
	*pSeekTableCount = 0;
	*ppSeekTable = NULL;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getSeekTable(unsigned int uiStartTime, unsigned int uiEndTime, int iCount, int* pSeekTableCount, unsigned int** ppSeekTable)
{
	*pSeekTableCount = 0;
	*ppSeekTable = NULL;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getVideoFrame(NXBOOL isEnhanced, NXBOOL bForceRead)
{
	return _SRC_GETFRAME_END;
}

int CNexOutSource::getVideoFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize )
{
	*ppFrame = NULL;
	*pFrameSize = 0;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getVideoFrameDTS(NXUINT32* puDTS, NXBOOL bReadTime)
{
	*puDTS = 0;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getVideoFramePTS(NXUINT32* puPTS, NXBOOL bReadTime)
{
	*puPTS = 0;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getAudioFrame()
{
	if( m_pOutSource == NULL  )
		return _SRC_GETFRAME_ERROR;

	if( m_isAudioExist && m_isAudioEnd )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] Audio End(%d %d)", __LINE__, m_isAudioExist , m_isAudioEnd);
		return _SRC_GETFRAME_END;
	}

	int iRet = eNEX_FF_RET_SUCCESS, iPos = 0;
	nexSAL_MutexLock( m_hOutSourceMutex, NEXSAL_INFINITE );
	iPos = nexSAL_FileSeek(m_pOutSource, 0, NEXSAL_SEEK_CUR);
	m_uBuffAudioSize = nexSAL_FileRead(m_pOutSource, m_pBuffAudio, OUTSOURCE_ONEFRAME_SIZE);
	 
	if(m_uBuffAudioSize == 0)
	{
		iRet = _SRC_GETFRAME_END;		
		m_isAudioEnd = 1;
	}
	else
	{
		iPos -= OUTSOURCE_HEADER_SIZE;
		m_uiAudioLastReadCTS = iPos/m_uiNumOfChannels/(m_uiBitsPerSample>> 3)*1000/m_uiSamplingRate;
	}
	nexSAL_MutexUnlock( m_hOutSourceMutex );

	if( m_uiAudioLastReadCTS > (m_uiTotalPlayTime - m_uiEndTrimTime + 300))
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getAudioFrame End(Content play end %d %d, %d, %d)", __LINE__, m_uiAudioLastReadCTS, m_uiBaseTimeStamp, m_uiStartTrimTime,  m_uiTotalPlayTime - m_uiEndTrimTime);
		m_isAudioEnd = 1;
		iRet = _SRC_GETFRAME_END;
	}	
	
	return iRet; 
}

int CNexOutSource::getAudioFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize )
{
	if( m_pOutSource == NULL  )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	*ppFrame = m_pBuffAudio;
	*pFrameSize = m_uBuffAudioSize;
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getAudioFrameCTS(NXINT64* puCTS )
{
	if( m_pOutSource == NULL  )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	*puCTS = m_uiAudioLastReadCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNexOutSource::isAudioExist()
{
	return m_isAudioExist;
}

NXBOOL CNexOutSource::isVideoExist()
{
	return m_isVideoExist;
}

NXBOOL CNexOutSource::isTextExist()
{
	return m_isTextExist;
}

unsigned int CNexOutSource::getIDRFrameCount()
{
	return m_uiCountIframe;
}

unsigned int CNexOutSource::getFrameFormat()
{
	return m_uiFrameFormat;
}

NXUINT32 CNexOutSource::parseFrameFormat(NXUINT8 *pFrame, NXUINT32 uFrameSize)
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

int CNexOutSource::getVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight )
{
	if( m_pOutSource == NULL || pusWidth == NULL || pusHeight == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*pusWidth = 0;
	*pusHeight = 0;	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getDisplayVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight )
{
	if( m_pOutSource == NULL || pusWidth == NULL || pusHeight == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*pusWidth = 0;
	*pusHeight = 0;	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getAudioBitrate(NXUINT32* puAudioBitrate )
{
	if( m_pOutSource == NULL || puAudioBitrate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puAudioBitrate = m_uiAudioBitRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getVideoBitrate(NXUINT32* puVideoBitrate )
{
	if( m_pOutSource == NULL || puVideoBitrate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puVideoBitrate = m_uiVideoBitRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getTotalPlayTime(NXUINT32* puTotalPlayTime )
{
	if( m_pOutSource == NULL || puTotalPlayTime == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puTotalPlayTime = m_uiTotalPlayTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getSamplingRate(NXUINT32* puSamplingRate )
{
	if( m_pOutSource == NULL || puSamplingRate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puSamplingRate = m_uiSamplingRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getNumberOfChannel(NXUINT32* puNumOfChannels )
{
	if( m_pOutSource == NULL || puNumOfChannels == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puNumOfChannels = m_uiNumOfChannels;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getSamplesPerChannel(NXUINT32* piNumOfSamplesPerChannel )
{
	if( m_pOutSource == NULL || piNumOfSamplesPerChannel == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*piNumOfSamplesPerChannel = m_uiNumOfSamplesPerChannel;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getDSINALHeaderLength()
{
	return m_uiDSINALSize;
}

int CNexOutSource::getFrameNALHeaderLength()
{
	return m_uiFrameNALSize;
}

int CNexOutSource::getH264ProfileLevelID(unsigned int* puH264ProfileLevelID )
{
	if( m_pOutSource == NULL || puH264ProfileLevelID == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264ProfileLevelID = m_uiH264ProfileID;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getH264Level(unsigned int* puH264Level )
{
	if( m_pOutSource == NULL || puH264Level == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264Level = m_uiH264Level;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getVideoH264Interlaced(NXBOOL* puH264Interlaced )
{
	if( m_pOutSource == NULL || puH264Interlaced == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264Interlaced = m_isInterlaced;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getWAVEFormat (unsigned int *pFormatTag, unsigned int* pBitsPerSample, unsigned int *pBlockAlign, unsigned int *pAvgBytesPerSec, unsigned int *pSamplesPerBlock, unsigned int *pEncodeOpt, unsigned int *pExtraSize, unsigned char **ppExtraData )
{
	if( m_pOutSource == NULL || pFormatTag == NULL || pBitsPerSample == NULL || pBlockAlign == NULL || 
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

int CNexOutSource::setBaseTimeStamp(unsigned int uiBaseTime)
{
	m_uiBaseTimeStamp = uiBaseTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::setTrimTime(unsigned int uiStartTrimTime, unsigned int uiEndTrimTime)
{
	m_uiStartTrimTime		= uiStartTrimTime;
	m_uiEndTrimTime		= uiEndTrimTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}
	
NXBOOL CNexOutSource::setRepeatAudioClip(unsigned int uiBaseTime, unsigned int uiStartTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CNexOutSource.cpp %d] setRepeatAudioClip(Base:%d StartTime %d)", __LINE__, uiBaseTime, uiStartTime);
	unsigned int uiSeekResultCTS	= 0;
	unsigned int uiTempBase		= m_uiBaseTimeStamp;

	m_uiBaseTimeStamp = uiBaseTime;
	if( seekToAudio(uiStartTime, &uiSeekResultCTS, NXFF_RA_MODE_PREV, TRUE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CNexOutSource.cpp %d] do not repeat because clip did not seek", __LINE__);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] repeat Audio Clip baseTime(%d->%d) SeekR(%d)", __LINE__, uiTempBase, m_uiBaseTimeStamp, uiSeekResultCTS);
	m_isAudioEnd			= 0;
	return TRUE;
}

int CNexOutSource::getReaderRotateState()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getRotateState(%d)", __LINE__, m_uiOrientation);
	return m_uiOrientation;
}

int CNexOutSource::getRotateState()
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
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getRotateState(%d %d)", __LINE__, m_uiOrientation, iOrientation);
	return iOrientation;
}

unsigned int CNexOutSource::getVideoFrameRate()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getVideoFrameRate(%d)", __LINE__, m_uiFrameFPS);
	return m_uiFrameFPS;
}

float CNexOutSource::getVideoFrameRateFloat()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getVideoFrameRate(%f)", __LINE__, m_fFrameFPS);
	return m_fFrameFPS;
}
unsigned int CNexOutSource::getTotalPlayAudioTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getTotalPlayAudioTime(%d)", __LINE__, m_uiTotalPlayAudioTime);
	return m_uiTotalPlayAudioTime;
}

unsigned int CNexOutSource::getTotalPlayVideoTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getTotalPlayVideoTime(%d)", __LINE__, m_uiTotalPlayVideoTime);
	return m_uiTotalPlayVideoTime;
}

int CNexOutSource::getAudioTrackCount()
{
	int iCount = 1;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getAudioTrackCount(%d)", __LINE__, iCount);
	return iCount;
}

int CNexOutSource::getVideoTrackCount()
{
	int iCount = 1;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getVideoTrackCount(%d)", __LINE__, iCount);
	return iCount;
}

//yoon
int  CNexOutSource::getVideoTrackUUID( NXUINT8** ppretAllocUUID, NXINT32* pretUUIDSize )
{
	NXINT32 i32Size = 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d] getVideoTrackUUID() start", __LINE__ );	
	if( m_pOutSource == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d]getVideoTrackUUID() FFReader no instans", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( !m_isVideoExist )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[CNexOutSource.cpp %d]getVideoTrackUUID() no video", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	*pretUUIDSize = 0;
	*ppretAllocUUID = NULL;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexOutSource::getEditBoxTime(unsigned int* puiAudioTime, unsigned int* puiVideoTime)
{
	*puiAudioTime = 0;
	*puiVideoTime = 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CNexOutSource.cpp %d] getEditBoxTime(%d, %d)", __LINE__, *puiAudioTime, *puiVideoTime);
	return 0;
}

int CNexOutSource::getAudioObjectType()
{
	return m_uiAudioObjectType;
}

int CNexOutSource::getVideoObjectType()
{
	return m_uiVideoObjectType;
}

void CNexOutSource::setAudioObjectType(unsigned int uiAudioObjectType)
{
	m_uiAudioObjectType = uiAudioObjectType;
}

void CNexOutSource::setNumOfChannels(unsigned int uiNumOfChannels)
{
	m_uiNumOfChannels = uiNumOfChannels;
}

void CNexOutSource::setSamplingRate(unsigned int uiSamplingRate)
{
	m_uiSamplingRate = uiSamplingRate;
}

void CNexOutSource::setNumOfSamplesPerChannel(unsigned int uiNumOfSamplesPerChannel)
{
	m_uiNumOfSamplesPerChannel = uiNumOfSamplesPerChannel;

	m_uiAudioFrameInterval		= _GetFrameInterval(m_uiNumOfSamplesPerChannel, m_uiSamplingRate);
	m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiNumOfSamplesPerChannel, m_uiSamplingRate);
	m_uiMaxAudioFrameInterval	= (m_uiAudioFrameInterval*3) >> 1;			

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CNexOutSource.cpp %d] Interval(%d %f %d)",  __LINE__, 
		m_uiAudioFrameInterval, 
		m_dbAudioFrameInterval, 
		m_uiMaxAudioFrameInterval);
}

NXBOOL CNexOutSource::getEnhancedExist()
{
	return 0;
}

unsigned int CNexOutSource::getVideoEditBox()
{
	return 0;
}

unsigned int CNexOutSource::getAudioEditBox()
{
	return 0;
}

int CNexOutSource::getTS(NXFF_MEDIA_TYPE eType, unsigned int* piTS )
{
	*piTS = 0;
	return 0;    
}

int CNexOutSource::getFindIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime)
{
	return 0;
}