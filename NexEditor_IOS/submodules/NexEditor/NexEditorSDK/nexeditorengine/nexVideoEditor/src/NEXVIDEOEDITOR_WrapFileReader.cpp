/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapFileReader.cpp
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

#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_Error.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"

#define VERSION_INFO_READER_FORMAT_STRING "\
********************************************\n\
     NxMP4FReader Version : %s\n\
********************************************\n"

#define IS_PCM_Codec(type) \
			( (type == eNEX_CODEC_A_PCM_S16LE) || (type == eNEX_CODEC_A_PCM_S16BE) || (type == eNEX_CODEC_A_PCM_RAW) || (type == eNEX_CODEC_A_PCM_FL32LE) \
			 || (type == eNEX_CODEC_A_PCM_FL32BE) || (type == eNEX_CODEC_A_PCM_FL64LE) || (type == eNEX_CODEC_A_PCM_FL64BE) || (type == eNEX_CODEC_A_PCM_IN24LE) \
			 || (type == eNEX_CODEC_A_PCM_IN24BE) || (type == eNEX_CODEC_A_PCM_IN32LE) || (type == eNEX_CODEC_A_PCM_IN32BE) || (type == eNEX_CODEC_A_PCM_LPCMLE) \
			 || (type == eNEX_CODEC_A_PCM_LPCMBE))

#define IS_VARIABLE_OUTPUT_PCM(type) \
			( (type == eNEX_CODEC_A_VORBIS) || (type == eNEX_CODEC_A_FLAC) || (type == eNEX_CODEC_A_WMA) )

//---------------------------------------------------------------------------
unsigned int CNexFileReader::changeEndian( unsigned char* pData )
{
	unsigned int uRet = 0;

	uRet |= ((unsigned int)pData[0]<<24) & 0xff000000;
	uRet |= ((unsigned int)pData[1]<<16) & 0x00ff0000;
	uRet |= ((unsigned int)pData[2]<<8 ) & 0x0000ff00;
	uRet |= ((unsigned int)pData[3]    ) & 0x000000ff;

	return uRet;
}
//---------------------------------------------------------------------------
// For enc information decrypt.
int CNexFileReader::parseHex(char *pstr, int start, int end)
{
	int result = 0;

	if (pstr==NULL) {
		return result;
	}

	int len = strlen(pstr);
	for( int i=start; i<end && i<len; i++ ) {
		char c = *(pstr+i);
		result *= 16;
		if( c >= '0' && c <= '9' ) result += (c-'0');
		else if( c >='a' && c <= 'f' ) result += (c-'a'+0xa);
		else if( c >='A' && c <= 'F' ) result += (c-'A'+0xa);

	}
	return result;
}

int CNexFileReader::getParsedEncInfo(char *pIn, NXUINT8* pOut, NXINT32 iMaxOutLen) {
	if (pIn==NULL) {
		return 0;
	}

	int len = strlen(pIn);
	int v = parseHex(pIn,0,2) ^ 32;
	int newLen = (len-2)/2;

	memset(pOut, 0x00, iMaxOutLen);
	newLen = newLen > iMaxOutLen ? iMaxOutLen : newLen;
	
	for( int i=0; i<newLen; i++ ) {
		NXUINT8 b = (NXUINT8) parseHex(pIn,2+i*2,4+i*2);
		b = (NXUINT8) ((((b>>4)&0x0F) | ((b<<4)&0xF0))^v);
		*(pOut+i) = b;
	}

	return newLen;
}

//---------------------------------------------------------------------------
CNexFileReader::CNexFileReader()
{
	m_uiClipID			= INVALID_CLIP_ID;
	m_isCreated			= FALSE;
	m_pFFReader			= NULL;

	memset(&m_BufVideo, 0x00, sizeof(NxFFReaderWBuffer));
	memset(&m_BufAudio, 0x00, sizeof(NxFFReaderWBuffer));
	memset(&m_BufText, 0x00, sizeof(NxFFReaderWBuffer));

	m_pBuffVideo		= NULL;
	m_pBuffAudio		= NULL;
	m_pBuffText			= NULL;

	m_hReaderMutex		= NEXSAL_INVALID_HANDLE;
	m_pFilePath			= NULL;

	m_isAudioExist				= FALSE;
	m_isVideoExist				= FALSE;
	m_isTextExist					= FALSE;
	m_isEnhancedExist				= FALSE;

	m_uiBaseTimeStamp			= 0;
	m_uiTotalPlayAudioTime		= 0;
	m_uiTotalPlayVideoTime		= 0;
	m_uiTotalPlayTime				= 0;

	m_uiStartTrimTime				= 0;
	m_uiEndTrimTime				= 0;

	m_uiFileType					= 0;

	m_uiVideoObjectType			= 0;
	m_uiVideoFourcc				= 0;
	m_uiAudioObjectType			= 0;
	m_uiAudioFourcc				= 0;

	m_uiH264ProfileID				= 0;
	m_uiH264Level				= 0;
	m_uiVideoBitRate				= 0;
	m_uiDSINALSize				= 0;
	m_uiFrameNALSize				= 0;
	m_isH264Interlaced			= FALSE;

	m_uiFrameFormat				= NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;

	m_isAudioChunkType			= FALSE;
	m_hAudioChunkInfo			= NULL;
	m_pAudioChunkFrame			= NULL;
	m_uiAudioChunkFrameLen		= 0;
	m_uiAudioChunkFrameDTS		= 0;
	m_uiAudioChunkFramePTS		= 0;

	m_isVideoChunkType			= FALSE;
	m_hVideoChunkInfo			= NULL;
	m_pVideoChunkFrame			= NULL;
	m_uiVideoChunkFrameLen		= 0;
	m_uiVideoChunkFrameDTS		= 0;
	m_uiVideoChunkFramePTS		= 0;	

	m_uiNumOfChannels			= 0;
	m_uiSamplingRate				= 0;
	m_uiNumOfSamplesPerChannel	= 0;
	m_uiAudioBitRate				= 0;

	m_uiCountIframe				= 0;
	m_uiframeCount				= 0;
	m_uiOrientation				= 0;
	m_uiFrameFPS				= 30;
	m_uiVideoFrameRate			= 0;
	m_iLostFrameCount			= 0;
	m_uiDisplayWidth				= 0;
	m_uiDisplayHeight				= 0;

	m_uiVideoLastReadDTS			= 0;
	m_uiVideoLastReadPTS			= 0;

	m_uiCorrectionTime			= 0;

	m_uiAudioLastReadCTS			= 0;

	m_uiLastAudioCTS				= 0;
	m_uiLastAudioCTSRead			= 0;
	m_uiAudioFrameInterval		= 0;
	m_dbAudioFrameInterval		= 0.0;
	m_uiMaxAudioFrameInterval		= 0;
	m_isLostFrameCheck			= FALSE;

	m_isAudioEnd					= FALSE;
	m_isVideoEnd					= FALSE;
	m_isTextEnd					= FALSE;

#ifdef FOR_TEST_AVI_FORMAT
	m_H264DSIBuffer				= (unsigned char*)nexSAL_MemAlloc(DEFAILT_DSI_MAX_SIZE);
#endif
	memset(m_H264DSIBuffer, 0x00, DEFAILT_DSI_MAX_SIZE);
	m_H264DSIBufferSize			= 0;

	m_pBitmapInfoEx				= NULL;

    m_uiVideoTmpDTS             = 0;
    m_uiVideoTmpPTS             = 0;

	m_isEditBox = 0;
	m_uiVideoEditBoxTime = 0;
	m_uiAudioEditBoxTime = 0;
	m_isCTSBox = 0;
	m_uiVideoCTSBoxTime = 0;

	m_iEncodeToTranscode	 = 0;
	m_bUsePrevFrame = 0;

	m_uIntermediateFrameSize = 1024*512;
	m_pIntermediateFrame = (NXUINT8 *)nexSAL_MemAlloc(m_uIntermediateFrameSize + 100);
}

CNexFileReader::~CNexFileReader()
{
	deleteSource();

	if( m_pFilePath )
	{
		nexSAL_MemFree(m_pFilePath);
		m_pFilePath = NULL;
	}

#ifdef FOR_TEST_AVI_FORMAT
	if( m_H264DSIBuffer != NULL )
	{
		nexSAL_MemFree(m_H264DSIBuffer);
		m_H264DSIBuffer = NULL;
	}
#endif	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] ~~~~~~~~~~~~~~CNexFileReader()", __LINE__ );
}

int CNexFileReader::createSource()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] createFileReader Start", __LINE__ );

	deleteSource();

	nexSAL_DebugPrintf(VERSION_INFO_READER_FORMAT_STRING, NxFFR_GetVersionString());

	m_hReaderMutex = nexSAL_MutexCreate();
	if( m_hReaderMutex == NEXSAL_INVALID_HANDLE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create - Create Mutex Error", __LINE__ );
		goto Create_Error;
	}

	m_pBuffVideo = (unsigned char*)nexSAL_MemAlloc( DEFAULT_FRAME_BUFF_SIZE_VIDEO );
	if ( m_pBuffVideo )
	{
		memset( m_pBuffVideo, 0x00, DEFAULT_FRAME_BUFF_SIZE_VIDEO);
		m_BufVideo.bfrstart = m_BufVideo.bfr = m_pBuffVideo;
		m_BufVideo.nMaxSize = DEFAULT_FRAME_BUFF_SIZE_VIDEO;
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create - Alloc Error", __LINE__ );
		goto Create_Error;
	}

	// Get encryption information from frame buufer
	m_BufVideo.uSubSampleMaxCount = 20;
	m_BufVideo.pstSubSamples = (NxFFREADER_SUBSAMPLE*)nexSAL_MemAlloc(m_BufVideo.uSubSampleMaxCount * sizeof(NxFFREADER_SUBSAMPLE));
	if ( m_BufVideo.pstSubSamples ) {
		memset(m_BufVideo.pstSubSamples, 0, sizeof(NxFFREADER_SUBSAMPLE));
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create - Alloc Error", __LINE__ );
		goto Create_Error;
	}

	m_pBuffAudio = (unsigned char*)nexSAL_MemAlloc( DEFAULT_FRAME_BUFF_SIZE_AUDIO );
	if ( m_pBuffAudio )
	{
		memset( m_pBuffAudio, 0x00, DEFAULT_FRAME_BUFF_SIZE_AUDIO);
		m_BufAudio.bfrstart = m_BufAudio.bfr = m_pBuffAudio;
		m_BufAudio.nMaxSize = DEFAULT_FRAME_BUFF_SIZE_AUDIO;
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create - Alloc Error", __LINE__ );
		goto Create_Error;
	}

	// Get encryption information from frame buufer
	m_BufAudio.uSubSampleMaxCount = 20;
	m_BufAudio.pstSubSamples = (NxFFREADER_SUBSAMPLE*)nexSAL_MemAlloc(m_BufAudio.uSubSampleMaxCount * sizeof(NxFFREADER_SUBSAMPLE));
	if ( m_BufAudio.pstSubSamples ) {
		memset(m_BufAudio.pstSubSamples, 0, sizeof(NxFFREADER_SUBSAMPLE));
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create - Alloc Error", __LINE__ );
		goto Create_Error;
	}

	m_pBuffText = (unsigned char*)nexSAL_MemAlloc( DEFAULT_FRAME_BUFF_SIZE_TEXT );
	if ( m_pBuffText )
	{
		memset( m_pBuffText, 0x00, DEFAULT_FRAME_BUFF_SIZE_TEXT);
		m_BufText.bfrstart = m_BufText.bfr = m_pBuffText;
		m_BufText.nMaxSize = DEFAULT_FRAME_BUFF_SIZE_TEXT;
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create - Alloc Error", __LINE__ );
		goto Create_Error;
	}

	m_pFFReader = NxFFR_Create(DEFAUKT_FILEEADER_MAX_HEAPSIZE, 0, 0, NULL);
	if( m_pFFReader == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create - Alloc Error", __LINE__ );
		goto Create_Error;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Create End", __LINE__ );

	m_isCreated = TRUE;

	return NEXVIDEOEDITOR_ERROR_NONE;

Create_Error:
	// for mantis 8895
	if( m_hReaderMutex != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MutexDelete( m_hReaderMutex );
		m_hReaderMutex = NEXSAL_INVALID_HANDLE;
	}

	SAFE_FREE(m_BufVideo.pstSubSamples);
	if( m_pBuffVideo )
	{
		nexSAL_MemFree( m_pBuffVideo );
		m_pBuffVideo = NULL;
	}

	SAFE_FREE(m_BufAudio.pstSubSamples);
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

	if( m_pFFReader )
	{
		NxFFR_Destroy( m_pFFReader );
		m_pFFReader = NULL;
	}
	
	return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED; 
}

int CNexFileReader::deleteSource()
{
	closeFile();
	/*
	if ( m_hReaderTask )
	{
		m_StateReaderTask = RTState_END;
		nexSAL_TaskWait( m_hReaderTask );
		nexSAL_TaskDelete( m_hReaderTask );
		m_hReaderTask = NEXSAL_INVALID_HANDLE;
	}
	*/
	if( m_hReaderMutex != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_MutexDelete( m_hReaderMutex );
		m_hReaderMutex = NEXSAL_INVALID_HANDLE;
	}

	if( m_pFFReader )
	{
		NxFFR_Destroy( m_pFFReader );
		m_pFFReader = NULL;
	}
    SAFE_FREE(m_BufVideo.pstSubSamples);
	if( m_pBuffVideo )
	{
		nexSAL_MemFree( m_pBuffVideo );
		m_pBuffVideo = NULL;
	}
    SAFE_FREE(m_BufAudio.pstSubSamples);
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

	if( m_hAudioChunkInfo )
	{
		AudioChunkParser_Destroy(m_hAudioChunkInfo);
		m_hAudioChunkInfo = NULL;
	}

	if( m_hVideoChunkInfo )
	{
		VideoChunkParser_Destroy(m_hVideoChunkInfo);
		m_hVideoChunkInfo = NULL;
	}

	m_isCreated = FALSE;

	SAFE_FREE(m_pIntermediateFrame);
	m_uIntermediateFrameSize = 0;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CNexFileReader::getClipID()
{
	return m_uiClipID;
}

void CNexFileReader::setClipID(unsigned int uiClipID)
{
	m_uiClipID = uiClipID;
}

void CNexFileReader::setEncodeToTranscode(int iValue)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] setEncodeToTranscode %d", __LINE__, iValue);
	m_iEncodeToTranscode = 1;// iValue;
#if defined(_ANDROID)
	if(CNexVideoEditor::m_bNexEditorSDK)
		m_iEncodeToTranscode = iValue;
#endif
}

int CNexFileReader::isReadyToOpen(NXBOOL *pResult )
{
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::openFile(char* pURL, unsigned int uiUrlLength)
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

	if( m_pFFReader == NULL || pURL == NULL || uiUrlLength <= 0 )
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
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] malloc file path failed", __LINE__);
		return NEXVIDEOEDITOR_ERROR_MEMALLOC_FAILED;
	}

	strcpy(m_pFilePath, pURL);
	m_pFilePath[uiUrlLength] = '\0';
  
	m_pFFReader->nFlagOptions = NXFF_READER_OPTION_SUPPORT_RA;
	m_pFFReader->nFlagOptions |= NXFF_READER_OPTION_PARSE_ALL_FRAGMENT;
	m_pFFReader->nFlagOptions |= NXFF_READER_OPTION_SUPPORT_RA_WITHOUT_MFRA;
	m_pFFReader->nFlagOptions |= NXFF_READER_OPTION_SUPPORT_ADTSAAC_HEADER;
	m_pFFReader->nFlagOptions |= NXFF_READER_OPTION_MAKE_SEEKBLOCK;

	m_pFFReader->nPDStartFlag = NXFF_READER_FLAG_PD_NONE;
	
	readerFileAPIs.m_Open	= (__NxFFReaderOpen)nexSAL_FileOpen;
	readerFileAPIs.m_Close	= (__NxFFReaderClose)nexSAL_FileClose;
	readerFileAPIs.m_Seek	= (__NxFFReaderSeek)nexSAL_FileSeek;
	readerFileAPIs.m_Seek64	= (__NxFFReaderSeek64)nexSAL_FileSeek64;
	readerFileAPIs.m_Read	= (__NxFFReaderRead)nexSAL_FileRead;
	readerFileAPIs.m_Write	= NULL;
	readerFileAPIs.m_Size	= (__NxFFReaderSize)nexSAL_FileSize;

	NxFFR_RegisteFileAPI(m_pFFReader,  &readerFileAPIs);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Clip open(%s)", __LINE__, pURL);

	uiSupportedFileType 	=	FLAG_NXFF_READER_SUPPORT_MP4
							| FLAG_NXFF_READER_SUPPORT_MP3
#ifdef FOR_TEST_AVI_FORMAT
							| FLAG_NXFF_READER_SUPPORT_AVI
#endif
							| FLAG_NXFF_READER_SUPPORT_WAVE
							| FLAG_NXFF_READER_SUPPORT_SPEECH	// for AMR
							//| FLAG_NXFF_READER_SUPPORT_ADIFAAC // RYU 20130523 Editor������ �������� �ʴ� �������� �Ǵ�(because Editor can't get audio frames in the middle of this format.)
							| FLAG_NXFF_READER_SUPPORT_ADTSAAC;
#if defined(_ANDROID)
	if(CNexVideoEditor::m_bNexEditorSDK)
		uiSupportedFileType |= FLAG_NXFF_READER_SUPPORT_FLAC;
#endif

	NxFFR_SetSupportFF(m_pFFReader, uiSupportedFileType);	

	// set flag to get encryption info
	eiRet = NxFFR_SetExtInfo(m_pFFReader, NXFF_EXTINFO_SET_USE_MEDIACRYPTO, 1, NULL);

	eiRet = NxFFR_Init( m_pFFReader, (unsigned char *)pURL, uiUrlLength);
	if( eiRet != eNEX_FF_RET_SUCCESS )
	{
		switch( eiRet )
		{
		case eNEX_FF_RET_FILE_OPEN_ERROR:
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] _SRC_FFInfo_Open: FOPEN fail\n", __LINE__);
			//FIRE_ONERROR(hPlayer, NEXPLAYER_ERROR_FILE_OPERATION, NULL, NULL, NULL);
			break;

		case eNEX_FF_RET_INVALID_SYNTAX:
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] _SRC_FFInfo_Open: Invalid Syntax\n", __LINE__);
			//FIRE_ONERROR(hPlayer, NEXPLAYER_ERROR_FILE_INVALID_SYNTAX, NULL, NULL, NULL);
			break;

		case eNEX_FF_RET_MEMORY_ALLOC_ERROR:
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] _SRC_FFInfo_Open: MALLOC fail\n", __LINE__);
			//FIRE_ONERROR(hPlayer, NEXPLAYER_ERROR_MEMORY_OPERATION, NULL, NULL, NULL);
			break;

		case eNEX_FF_RET_UNSUPPORT_FORMAT:
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] _SRC_FFInfo_Open: Unsupported Contents\n", __LINE__);
			//FIRE_ONERROR(hPlayer, NEXPLAYER_ERROR_NOT_SUPPORT_MEDIA, NULL, NULL, NULL);
			break;

		default:
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] NxFFR_Init failed (eFFRet: %d)\n", __LINE__, eiRet);
			//FIRE_ONERROR(hPlayer, NEXPLAYER_ERROR_UNKNOWN, NULL, NULL, NULL);
			break;
		}

		NxFFR_Close( m_pFFReader );
		NxFFR_UnRegisteFileAPI( m_pFFReader );
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}
	
	if(m_pFFReader->DRMType == NXFF_ISOBMFF_DRM) {
		//1. NxFFR_GetExtInfo, NXFF_EXTINFO_GET_MEDIACRYPTO_PSSH, NXFF_EXTINFO_GET_MEDIACRYPTO_TENC
		NXINT64 nSize64 = 0;
		NXUINT32* pPParam = NULL;
		NXUINT32	nInitDataCount = 0;
		nxff_ISOBMFF_DRM_KID_T *pTENC;
		NxFFREADER_PSSHS*	pPSSHList = NULL;
		NXUINT8	pEncInfo[ENC_INFO_SIZE];
		NXBOOL bMatched=FALSE;

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] DRM Type is NXFF_ISOBMFF_DRM\n", __LINE__);
		
		eiRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_MEDIACRYPTO_PSSH, 0, &nSize64, (NXVOID**)&pPSSHList);
		nInitDataCount = (NXUINT32)nSize64;

		// 2. NXFF_EXTINFO_GET_MEDIACRYPTO_PSSH
		if (pPSSHList->m_dwSystemIdSize != ENC_INFO_SIZE || CNexVideoEditor::m_pThisEditor->getEncInfoCnt()<2) {
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] Is not a valid encryption info1! \n", __LINE__);
			NxFFR_Close( m_pFFReader );
			NxFFR_UnRegisteFileAPI( m_pFFReader );
			return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
		}

		bMatched = FALSE;
		for (int i=CNexVideoEditor::m_pThisEditor->getEncInfoCnt()-1 ; i>=0 ; i--) {
			getParsedEncInfo(CNexVideoEditor::m_pThisEditor->getEncInfo(i), pEncInfo, ENC_INFO_SIZE);
			if ( !memcmp(pPSSHList->m_pSystemId, pEncInfo, pPSSHList->m_dwSystemIdSize) ) {
				bMatched = TRUE;
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] System ID verification is success!\n", __LINE__);
				break;
			}
		}
		if ( bMatched == FALSE ) {
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] Is not a valid encryption info2!\n", __LINE__);
			NxFFR_Close( m_pFFReader );
			NxFFR_UnRegisteFileAPI( m_pFFReader );
			return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
		}
		
		// 3. Audio NXFF_EXTINFO_GET_MEDIACRYPTO_TENC
		bMatched = FALSE;
		eiRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_MEDIACRYPTO_TENC, (NXINT64)NXFF_MEDIA_TYPE_AUDIO, &nSize64, (NXVOID**)&pPParam);
		pTENC = (nxff_ISOBMFF_DRM_KID_T*)pPParam;
		for ( int i=0; i<CNexVideoEditor::m_pThisEditor->getEncInfoCnt() ; i+=2) {
			getParsedEncInfo(CNexVideoEditor::m_pThisEditor->getEncInfo(i), pEncInfo, ENC_INFO_SIZE);
			if (!memcmp(pTENC->pKID, pEncInfo, ENC_INFO_SIZE))
			{
				getParsedEncInfo(CNexVideoEditor::m_pThisEditor->getEncInfo(i+1), pEncInfo, ENC_INFO_SIZE);
				memcpy(m_EncInfo[NXFF_MEDIA_TYPE_AUDIO], pEncInfo, ENC_INFO_SIZE);
				bMatched = true;
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] Audio KID is success! \n", __LINE__);
				break;
			}
		}

		// 4. Video NXFF_EXTINFO_GET_MEDIACRYPTO_TENC
		eiRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_MEDIACRYPTO_TENC, (NXINT64)NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &nSize64, (NXVOID**)&pPParam);
		pTENC = (nxff_ISOBMFF_DRM_KID_T*)pPParam;
		for ( int i=0; i<CNexVideoEditor::m_pThisEditor->getEncInfoCnt() ; i+=2) {
			getParsedEncInfo(CNexVideoEditor::m_pThisEditor->getEncInfo(i), pEncInfo, ENC_INFO_SIZE);
			if (!memcmp(pTENC->pKID, pEncInfo, ENC_INFO_SIZE))
			{
				getParsedEncInfo(CNexVideoEditor::m_pThisEditor->getEncInfo(i+1), pEncInfo, ENC_INFO_SIZE);
				memcpy(m_EncInfo[NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO], pEncInfo, ENC_INFO_SIZE);
				bMatched = true;
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] Video KID is success! \n", __LINE__);
				break;
			}
		}

		if (!bMatched)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] Is not a valid KID\n", __LINE__);
			NxFFR_Close( m_pFFReader );
			NxFFR_UnRegisteFileAPI( m_pFFReader );
			return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
		}
	}

#ifdef FOR_TEST_AUDIO_CBR_NO_SEEKTAB
	NEX_FF_RETURN eRetVal = eNEX_FF_RET_PADDING;
	NXINT64 nBitRateMode = 0;

	eRetVal = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_AUDIO_BIT_RATE_MODE, 0, &nBitRateMode, NULL);
	if( eNEX_FF_RET_SUCCESS == eRetVal )
	{
		if( 1 == nBitRateMode ) //0:CBR mode, 1:VBR mode
		{
			//If call the below property, NxFFReader internally creates a seek table.
			NxFFR_SetExtInfo(m_pFFReader, NXFF_EXTINFO_SET_AUDIODURATIONCHECK, TRUE, NULL);
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] NxFFR_Init failed (eFFRet: %d)\n", __LINE__, eiRet);
		}
	}
#else
	NxFFR_SetExtInfo(m_pFFReader, NXFF_EXTINFO_SET_AUDIODURATIONCHECK, TRUE, NULL);
#endif

	m_isTextExist = 0;
	
	m_isAudioExist = m_pFFReader->bAudioExist;
	m_isVideoExist = m_pFFReader->bVideoExist;
	m_isTextExist = m_pFFReader->bTextExist;

	m_uiFileType = m_pFFReader->MediaFileFormatType;
	
	m_isEnhancedExist = m_pFFReader->nTSFlag;

	if( m_isVideoExist )
	{
		pNxFFRDisplayImageSize    pDisplayImageSize = NULL;
		pDisplayImageSize = (pNxFFRDisplayImageSize) nexSAL_MemAlloc(sizeof(nxFFRDisplayImageSize));
		memset(pDisplayImageSize, 0, sizeof(nxFFRDisplayImageSize));

		eiRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_DISPLAYIMAGESIZE, 0, NULL, (NXVOID**)pDisplayImageSize);
		if( eiRet != eNEX_FF_RET_SUCCESS )
		{
			pDisplayImageSize = NULL;
			nexSAL_MemFree(pDisplayImageSize);
			NxFFR_Close( m_pFFReader );
			NxFFR_UnRegisteFileAPI( m_pFFReader );
			return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
		}

		m_uiDisplayWidth	= pDisplayImageSize->unDisplayImageWidth;
		m_uiDisplayHeight	= pDisplayImageSize->unDisplayImageHeight;
		//NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] NXFF_EXTINFO_GET_DISPLAYIMAGESIZE (%u, %u)\n", __LINE__, m_uiDisplayWidth, m_uiDisplayHeight);
		nexSAL_MemFree(pDisplayImageSize);
		pDisplayImageSize = NULL;

		NXINT64 nTime64 = 0, nTime64_Audio = 0;
		S_ELST *pVideoEditbox = NULL, *pAudioEditbox = NULL;
		
		if( m_isAudioExist )
		{
			eiRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_EDTSBOX, NXFF_MEDIA_TYPE_AUDIO, NULL, (NXVOID**)&pAudioEditbox);
			if( eiRet == eNEX_FF_RET_SUCCESS)
			{
				if (pAudioEditbox)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio EditBox nEntry_count(%d)", __LINE__, pAudioEditbox->nEntry_count);
					for(int i=0; i<pAudioEditbox->nEntry_count; i++)
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio EditBox (%d : %lld, %lld, %d)", __LINE__, i, pAudioEditbox->pSegmentDurationTable[i], pAudioEditbox->pMediaTimeTable[i], (int)(pAudioEditbox->pMediaRateTable[i]));

					if (pAudioEditbox->nEntry_count > 0)
					{
						if (pAudioEditbox->pMediaTimeTable[0] < 0 && pAudioEditbox->nEntry_count > 1)
							nTime64_Audio = pAudioEditbox->pMediaTimeTable[1]; // use the first edts mediatime temporally
						else if (pAudioEditbox->pMediaTimeTable[0] >= 0)
							nTime64_Audio = pAudioEditbox->pMediaTimeTable[0]; // use the first edts mediatime temporally
					}
				}
				m_uiAudioEditBoxTime = (unsigned int)nTime64_Audio;
				if(m_uiAudioEditBoxTime)
				{
					m_isEditBox = 1;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio EditBox Time(%d)", __LINE__, m_uiAudioEditBoxTime);				
				}
			}
		}

		eiRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_EDTSBOX, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, NULL, (NXVOID**)&pVideoEditbox);
		if( eiRet == eNEX_FF_RET_SUCCESS)
		{
			if (pVideoEditbox)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video EditBox nEntry_count(%d)", __LINE__, pVideoEditbox->nEntry_count);
				for(int i=0; i<pVideoEditbox->nEntry_count; i++)
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video EditBox (%d : %lld, %lld, %d)", __LINE__, i, pVideoEditbox->pSegmentDurationTable[i], pVideoEditbox->pMediaTimeTable[i], (int)(pVideoEditbox->pMediaRateTable[i]));

				if (pVideoEditbox->nEntry_count > 0)
				{
					if (pVideoEditbox->pMediaTimeTable[0] < 0 && pVideoEditbox->nEntry_count > 1)
						nTime64 = pVideoEditbox->pMediaTimeTable[1]; // use the first edts mediatime temporally
					else if (pVideoEditbox->pMediaTimeTable[0] >= 0)
						nTime64 = pVideoEditbox->pMediaTimeTable[0]; // use the first edts mediatime temporally
				}
			}

			m_uiVideoEditBoxTime = (unsigned int)nTime64;
			if(m_uiVideoEditBoxTime)
			{
				m_isEditBox = 1;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video EditBox Time(%d)", __LINE__, m_uiVideoEditBoxTime);
			}
		}

		if(nTime64 < 100 && nTime64_Audio < 100)
		{
			m_iEncodeToTranscode = 0;
		}

		eiRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_CHECK_EXIST_PTS, 0, &nTime64, NULL);
		if( eiRet == eNEX_FF_RET_SUCCESS )
		{
			if( 1 == nTime64 )
			{
				NXUINT32 uTime32 = 0;
				eiRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, NXFF_RA_MODE_CUR_PREV, 0, &uTime32);
				if (eiRet != eNEX_FF_RET_SUCCESS)
				{
					NxFFR_Close( m_pFFReader );
					NxFFR_UnRegisteFileAPI( m_pFFReader );
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Err(%d)", __LINE__, eiRet);
					return NEXVIDEOEDITOR_ERROR_UNKNOWN;
				}

				if(uTime32)
				{
					m_isCTSBox = 1;
					m_uiVideoCTSBoxTime = uTime32;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video CTSBox Time(%d, %d)", __LINE__, m_uiVideoCTSBoxTime, uTime32);
			}
		}		

		if(m_iEncodeToTranscode == 0)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Does not support EditBox and CTS mode in Transcode (%d, %d)", __LINE__, m_uiVideoEditBoxTime, m_uiVideoCTSBoxTime);
			m_isEditBox = 0;
        }

		m_uiVideoFourcc = m_pFFReader->hVideoTrackInfo[0].n4CC;	//For WMV fourcc
		switch( m_pFFReader->hVideoTrackInfo[0].nOTI )
		{
			case eNEX_CODEC_V_MPEG4V:
				if(	(m_pFFReader->MediaFileFormatType == eNEX_FF_AVI) 
					|| (m_pFFReader->MediaFileFormatType == eNEX_FF_MATROSKA) 
					|| (m_pFFReader->MediaFileFormatType == eNEX_FF_RMFF) 
					|| (m_pFFReader->MediaFileFormatType == eNEX_FF_ASF) 
					|| (m_pFFReader->MediaFileFormatType == eNEX_FF_MPEG_TS) )
					m_uiVideoObjectType = 0;
				else
					m_uiVideoObjectType = eNEX_CODEC_V_MPEG4V;
				break;
			case eNEX_CODEC_V_H264:
				m_uiVideoObjectType = eNEX_CODEC_V_H264;
				break;
			case eNEX_CODEC_V_HEVC:
				m_uiVideoObjectType = eNEX_CODEC_V_HEVC;
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] Not Supported Video codec(0x%x)",__LINE__, m_pFFReader->hVideoTrackInfo[0].nOTI);
				m_uiVideoObjectType=0;
				break;
		};			
		// add MPEG4V

#ifdef FOR_TEST_MP4V_VIDEO
		if( m_uiVideoObjectType == eNEX_CODEC_V_H264  ||  m_uiVideoObjectType == eNEX_CODEC_V_MPEG4V || m_uiVideoObjectType == eNEX_CODEC_V_HEVC )
#else
		if( m_uiVideoObjectType == eNEX_CODEC_V_H264  || ( CNexVideoEditor::m_bSuppoertSWMP4Codec && m_uiVideoObjectType == eNEX_CODEC_V_MPEG4V ) || m_uiVideoObjectType == eNEX_CODEC_V_HEVC )
#endif			
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Support Video Codec Type(0x%x)", __LINE__, m_uiVideoObjectType);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not Support Video Codec Type(0x%x)", __LINE__, m_uiVideoObjectType);
			NxFFR_Close( m_pFFReader );
			NxFFR_UnRegisteFileAPI( m_pFFReader );
			return NEXVIDEOEDITOR_ERROR_UNSUPPORT_VIDEO_CODEC;
		}

		NxFFR_GetMediaDuration(m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &m_uiTotalPlayVideoTime);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video duration(%d), (%d, %d)", __LINE__, m_uiTotalPlayVideoTime, m_uiVideoEditBoxTime, m_uiVideoCTSBoxTime);
		
		if(m_isEditBox)
			m_uiTotalPlayVideoTime = m_uiTotalPlayVideoTime - m_uiVideoEditBoxTime + m_uiVideoCTSBoxTime;
		m_uiTotalPlayTime = m_uiTotalPlayVideoTime;

		if( m_uiVideoObjectType == eNEX_CODEC_V_H264 )
		{
			unsigned char*		pDSI		= NULL;
			unsigned int			iDSISize		= 0;

			NXUINT32			uErrorCode	= 0;
			NXUINT8*			pFrame		= NULL;
			NXUINT32			uFrameLen	= 0;
			int					iReadCount = 0;
			
			if( getDSI(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pDSI, &iDSISize) == NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] H264 NAL HeaderSize(%d)", __LINE__, m_uiDSINALSize);
			}

			while(getVideoFrame() == _SRC_GETFRAME_OK)
			//if( getVideoFrame() == _SRC_GETFRAME_OK )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Calc H264 NAL Get Frame OK", __LINE__);
				iReadCount++;				
				if( getVideoFrameData( &pFrame, &uFrameLen ) == NEXVIDEOEDITOR_ERROR_NONE )
				{
					NXBOOL			isIntraFrame	= FALSE;
					int 			iFrameNALHeaderSize		= 0;

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Calc H264 NAL Get Frame Data OK", __LINE__);
					m_uiFrameNALSize = NexCodecUtil_GuessNalHeaderLengthSize(pFrame, uFrameLen);
					iFrameNALHeaderSize = m_uiFrameNALSize;

					isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoObjectType, pFrame, uFrameLen, (void*)&iFrameNALHeaderSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, FALSE);
					if(!isIntraFrame)
					{
						isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoObjectType, pFrame, uFrameLen, (void*)&iFrameNALHeaderSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, FALSE);					
					}

					if(isIntraFrame)
					{
#ifdef DISABLE_MIXED_RAW_AND_ANNEXB_NAL
                        m_uiFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameLen);
#else
                        // fixed NESA-843
                        if((m_uiFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameLen)) == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW)
                        {
                            int iCodeLen = 0;
                            NexCodecUtil_FindAnnexBStartCode(pFrame, 4, uFrameLen > 200?200:uFrameLen, &iCodeLen);
                            if(iCodeLen == 4)
                            {
                                m_uiFrameFormat = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
                                m_uiFrameNALSize = iCodeLen;
                            }
                        }
#endif
						m_uiFrameNALSize = m_uiFrameNALSize == 0 ? 4 : m_uiFrameNALSize;
						break;
					}
				}

				if(iReadCount == 30)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not found IDR frame(0x%x)", __LINE__, m_uiVideoObjectType);
					NxFFR_Close( m_pFFReader );
					NxFFR_UnRegisteFileAPI( m_pFFReader );
					return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				}
			}

			unsigned int uiSeekResultCTS = 0;
			seekTo(0, &uiSeekResultCTS, NXFF_RA_MODE_PREV, FALSE);

			const char* pByteFormat = "UNKNOWN";
			pByteFormat = m_uiFrameFormat == 1 ? "RAW" : m_uiFrameFormat == 2 ? "ANNEX-B" : pByteFormat;
			
			m_uiDSINALSize = m_uiDSINALSize == 0 ? m_uiFrameNALSize : m_uiDSINALSize;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Calc H264 NAL Header size(DSI:%d, Frame:%d) %s", __LINE__, 
				m_uiDSINALSize, 
				m_uiFrameNALSize,
				pByteFormat );
			
			NEXCODECUTIL_SPS_INFO SPS_Info;
#ifdef FOR_TEST_AVI_FORMAT
			int iRet = (NEXVIDEOEDITOR_ERROR)NexCodecUtil_AVC_GetSPSInfo((char *)pDSI, iDSISize, &SPS_Info, m_uiFrameFormat);
#else
			int iRet = (NEXVIDEOEDITOR_ERROR)NexCodecUtil_AVC_GetSPSInfo((char *)pDSI, iDSISize, &SPS_Info, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW);
#endif
			if( iRet == 0)
			{
#if 0
				NEXCODECUTIL_AVC_SLICE_HEADER_INFO stAVCSHInfo;
				memset(&stAVCSHInfo, 0x00, sizeof(stAVCSHInfo));
				if( NEXVIDEOEDITOR_ERROR_NONE == (NEXVIDEOEDITOR_ERROR)NexCodecUtil_AVC_GetSliceHeaderInfo((char*)pFrame, uFrameLen, &SPS_Info, m_uiFrameNALSize, m_uiFrameFormat, &stAVCSHInfo))
					m_isH264Interlaced = stAVCSHInfo.field_pic_flag;
#else
				m_isH264Interlaced = SPS_Info.frame_mbs_only_flag?FALSE:TRUE;
#endif
				m_uiH264ProfileID = (NXUINT32)SPS_Info.eProfile;
				m_uiH264Level	= (NXUINT32)SPS_Info.uLevel;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] from PSs Width Height(%d %d)", __LINE__, SPS_Info.usWidth, SPS_Info.usHeight);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] from PSs uH264ProfileLevelID(%d) Level(%d) Interlaced(%d)", __LINE__, m_uiH264ProfileID, m_uiH264Level, m_isH264Interlaced);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] get SPSInfo failed(%d)", __LINE__, iRet );
			}
		}

		if( m_uiVideoObjectType == eNEX_CODEC_V_HEVC )
		{
			unsigned char*		pDSI		= NULL;
			unsigned int			iDSISize		= 0;

			NXUINT32			uErrorCode	= 0;
			NXUINT8*			pFrame		= NULL;
			NXUINT32			uFrameLen	= 0;
			int					iReadCount = 0;
			
			if( getDSI(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pDSI, &iDSISize) == NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] H264 NAL HeaderSize(%d)", __LINE__, m_uiDSINALSize);
			}

			while(getVideoFrame() == _SRC_GETFRAME_OK)
			//if( getVideoFrame() == _SRC_GETFRAME_OK )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Calc H264 NAL Get Frame OK", __LINE__);
				iReadCount++;				
				if( getVideoFrameData( &pFrame, &uFrameLen ) == NEXVIDEOEDITOR_ERROR_NONE )
				{
					NXBOOL			isIntraFrame	= FALSE;
					int 			iFrameNALHeaderSize		= 0;

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Calc H264 NAL Get Frame Data OK", __LINE__);
					m_uiFrameNALSize = NexCodecUtil_GuessNalHeaderLengthSize(pFrame, uFrameLen);
					iFrameNALHeaderSize = m_uiFrameNALSize;

					isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoObjectType, pFrame, uFrameLen, (void*)&iFrameNALHeaderSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, FALSE);
					if(!isIntraFrame)
					{
						isIntraFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoObjectType, pFrame, uFrameLen, (void*)&iFrameNALHeaderSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, FALSE);					
					}

					if(isIntraFrame)
					{
#ifdef DISABLE_MIXED_RAW_AND_ANNEXB_NAL
                        m_uiFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameLen);
#else
                        // fixed NESA-843
                        if((m_uiFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameLen)) == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW)
                        {
                            int iCodeLen = 0;
                            NexCodecUtil_FindAnnexBStartCode(pFrame, 4, uFrameLen > 200?200:uFrameLen, &iCodeLen);
                            if(iCodeLen == 4)
                            {
                                m_uiFrameFormat = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
                                m_uiFrameNALSize = iCodeLen;
                            }
                        }
#endif
						m_uiFrameNALSize = m_uiFrameNALSize == 0 ? 4 : m_uiFrameNALSize;
						break;
					}
				}

				if(iReadCount == 30)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not found IDR frame(0x%x)", __LINE__, m_uiVideoObjectType);
					NxFFR_Close( m_pFFReader );
					NxFFR_UnRegisteFileAPI( m_pFFReader );
					return NEXVIDEOEDITOR_ERROR_CODEC_INIT;
				}
			}

			unsigned int uiSeekResultCTS = 0;
			seekTo(0, &uiSeekResultCTS, NXFF_RA_MODE_PREV, FALSE);

			const char* pByteFormat = "UNKNOWN";
			pByteFormat = m_uiFrameFormat == 1 ? "RAW" : m_uiFrameFormat == 2 ? "ANNEX-B" : pByteFormat;
			
			m_uiDSINALSize = m_uiDSINALSize == 0 ? m_uiFrameNALSize : m_uiDSINALSize;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Calc H264 NAL Header size(DSI:%d, Frame:%d) %s", __LINE__, 
				m_uiDSINALSize, 
				m_uiFrameNALSize,
				pByteFormat );
			
			NEXCODECUTIL_SPS_INFO SPS_Info;
#ifdef FOR_TEST_AVI_FORMAT
			int iRet = (NEXVIDEOEDITOR_ERROR)NexCodecUtil_HEVC_GetSPSInfo((char *)pDSI, iDSISize, &SPS_Info, m_uiFrameFormat);
#else
			int iRet = (NEXVIDEOEDITOR_ERROR)NexCodecUtil_HEVC_GetSPSInfo((char *)pDSI, iDSISize, &SPS_Info, NexCodecUtil_CheckByteFormat( pDSI, iDSISize));
#endif

			if( iRet == 0)
			{
				m_uiH264ProfileID = (NXUINT32)SPS_Info.eProfile;
				m_uiH264Level	= (NXUINT32)SPS_Info.uLevel;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] from PSs Width Height(%d %d)", __LINE__, SPS_Info.usWidth, SPS_Info.usHeight);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] from PSs uH264ProfileLevelID(%d) Level(%d)", __LINE__, m_uiH264ProfileID, m_uiH264Level);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] get SPSInfo failed(%d)", __LINE__, iRet );
			}
		}
        
		if( m_uiVideoObjectType == eNEX_CODEC_V_MPEG4V )
		{
			m_uiH264ProfileID = 255;
			m_uiH264Level	= 0;

			unsigned char*		pDSI		= NULL;
			unsigned int			iDSISize		= 0;

			if( getDSI(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pDSI, &iDSISize) != NEXVIDEOEDITOR_ERROR_NONE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] MPEG4V get DSI failed", __LINE__);
			}

			NEXCODECUTIL_MPEG4V_DSI_INFO DSI_Info;
			int iRet = NexCodecUtil_MPEG4V_GetDSIInfo((NEX_CODEC_TYPE)m_uiVideoObjectType, (char *)pDSI, iDSISize, &DSI_Info);

			if( iRet == 0)
			{
				m_uiH264ProfileID = (NXUINT32)DSI_Info.uProfile;
				m_uiH264Level	= (NXUINT32)DSI_Info.uLevel;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] from PSs uMPEG4VProfileLevelID(%d) Level(%d)", __LINE__, m_uiH264ProfileID, m_uiH264Level);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] get MPEG4V DSIInfo failed(%d)", __LINE__, iRet );
				m_uiH264ProfileID = 255;
				m_uiH264Level	= 0;
			}            
		}			
		m_uiVideoBitRate = NxFFR_GetBitrate(m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video meta Info(%d %d %d)", 
			__LINE__, 
			m_uiH264ProfileID,
			m_uiH264Level,
			m_uiVideoBitRate);

		m_isVideoChunkType = isVideoChunkType(	m_pFFReader->hVideoTrackInfo[0].nOTI, 
												m_pFFReader->hVideoTrackInfo[0].n4CC,
												m_pFFReader->MediaFileFormatType);

		if( m_isVideoChunkType )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] Using Video Chunk Parser(%d)", __LINE__, m_isVideoChunkType );


			VIDEOCHUNK_EXTINFO VCPExt;
			memset(&VCPExt, 0, sizeof(VIDEOCHUNK_EXTINFO));

			VCPExt.m_eCodecType			= (NEX_CODEC_TYPE)m_uiVideoObjectType;
			VCPExt.m_uFourCC			= m_pFFReader->hVideoTrackInfo[0].n4CC;
			VCPExt.m_eFFType				= (NEX_FILEFORMAT_TYPE)m_pFFReader->MediaFileFormatType;
			VCPExt.m_uNalHeaderLen		= m_uiDSINALSize;
			
			VCPExt.m_pDSI				= m_H264DSIBuffer;
			VCPExt.m_uDSILen			= m_H264DSIBufferSize;
			
			VCPExt.m_pExtraDSI			= NULL;
			VCPExt.m_uExtraDSILen		= 0;
			VCPExt.m_pExtraData			= NULL;
			VCPExt.m_bEncrypted			= FALSE;
			VCPExt.m_uTimescale			= 0;

			m_hVideoChunkInfo = VideoChunkParser_CreateInstance(&VCPExt, NULL);

			m_pVideoChunkFrame			= NULL;
			m_uiVideoChunkFrameLen		= 0;
			m_uiVideoChunkFrameDTS		= 0;
			m_uiVideoChunkFramePTS		= 0;	
			
		}

		NXUINT64 uVideoOrientation = 0;
		NXINT64 iIDRCount = 0;
		double dVideoFPS = 0;

		m_uiVideoFrameRate = (unsigned int)(m_pFFReader->fVideoFramerates + 0.5);

		// I-frame�� ������ üũ
		m_uiCountIframe = 0;
		NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_VIDEO_SYNC_SAMPLE_COUNT, 0, &iIDRCount, NULL);
		NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_VIDEO_ORIENTATION, 0, &uVideoOrientation, NULL);			
		NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_VIDEO_FPS, 0, &dVideoFPS, NULL);			
		m_uiOrientation = (unsigned int)uVideoOrientation;
		m_fFrameFPS = (float)dVideoFPS;
		m_uiFrameFPS = (unsigned int)dVideoFPS;
		m_uiCountIframe = (unsigned int)iIDRCount;
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] I frame[Count = %d]. Orient:%d FPS:%d", __LINE__, m_uiCountIframe, m_uiOrientation, m_uiFrameFPS);
	}
	
	if( m_isAudioExist )
	{
		//Covert Audio Codec Type//////////////////////////////////////////////////////////////////////////////////////////////
		m_uiAudioFourcc = m_pFFReader->hAudioTrackInfo[0].nOTI;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] reader Audio codec(0x%x), EditBox %d",__LINE__, m_pFFReader->hAudioTrackInfo[0].nOTI, m_uiAudioEditBoxTime);
		switch( m_pFFReader->hAudioTrackInfo[0].nOTI)
		{
			case eNEX_CODEC_A_AAC:
			case eNEX_CODEC_A_AACPLUS:
			case eNEX_CODEC_A_AACPLUSV2:
				m_uiAudioObjectType = eNEX_CODEC_A_AAC;
				break;
			case eNEX_CODEC_A_MP3:
				m_uiAudioObjectType = eNEX_CODEC_A_MP3;
				break;
			case eNEX_CODEC_A_PCM_S16LE:
			case eNEX_CODEC_A_PCM_S16BE:		
			case eNEX_CODEC_A_PCM_RAW:
			case eNEX_CODEC_A_PCM_FL32LE:
			case eNEX_CODEC_A_PCM_FL32BE:
			case eNEX_CODEC_A_PCM_FL64LE:
			case eNEX_CODEC_A_PCM_FL64BE:
			case eNEX_CODEC_A_PCM_IN24LE:
			case eNEX_CODEC_A_PCM_IN24BE:
			case eNEX_CODEC_A_PCM_IN32LE:
			case eNEX_CODEC_A_PCM_IN32BE:		
			case eNEX_CODEC_A_PCM_LPCMLE:
			case eNEX_CODEC_A_PCM_LPCMBE:
			case eNEX_CODEC_A_AMR:
			case eNEX_CODEC_A_AMRWB:
			case eNEX_CODEC_A_FLAC:
				m_uiAudioObjectType = m_pFFReader->hAudioTrackInfo[0].nOTI;
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] Not Supported Audio codec(0x%x)",__LINE__, m_pFFReader->hAudioTrackInfo[0].nOTI);
				m_uiAudioObjectType=0;
				break;
		};
		
		if( m_uiAudioObjectType != 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Support Audio Codec Type(0x%x)", __LINE__, m_uiAudioObjectType);

			m_uiAudioBitRate = NxFFR_GetBitrate(m_pFFReader, NXFF_MEDIA_TYPE_AUDIO);
			
			NEX_WAVEFORMATEX* pWaveFormat = NxFFR_GetWAVEFormatEX(m_pFFReader, 0);
			if( pWaveFormat )
			{
				m_uiSamplingRate			= pWaveFormat->nSamplePerSecond;
				m_uiNumOfChannels			= pWaveFormat->nChannel;
				m_uiNumOfSamplesPerChannel	= pWaveFormat->nSampleNumber;

				if( m_uiAudioObjectType == eNEX_CODEC_A_AAC )
				{
					NXUINT32 samplerate = 0;
					NXUINT32 channels = 0;
					NXINT32 bSBR = 0;

					NexCodecUtil_AAC_ParseDSI( m_pFFReader->hAudioTrackInfo[0].pDSI, m_pFFReader->hAudioTrackInfo[0].nDSI, &samplerate, &channels, &bSBR);

					if( bSBR )
					{
						m_uiSamplingRate = samplerate * 2;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio samplerate(%d)", __LINE__, m_uiSamplingRate);
					}
					else
					{
						NEX_FF_RETURN eFileReaderRet = NxFFR_ReadFrame( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, 1, &m_BufAudio );
						if( eFileReaderRet == eNEX_FF_RET_SUCCESS )
						{
							NXUINT8* pFrame = m_BufAudio.bfrstart+9;
							NXUINT32 FrameSize = m_BufAudio.bytecnt-9;

							NXINT32 iCodecType = NexCodecUtil_GetAACType( m_pFFReader->hAudioTrackInfo[0].nOTI, samplerate, channels, pFrame, FrameSize);
							if( iCodecType == eNEX_CODEC_A_AACPLUS || iCodecType == eNEX_CODEC_A_AACPLUSV2 )
							{
								m_uiSamplingRate = samplerate * 2;
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio samplerate(%d)", __LINE__, m_uiSamplingRate);
							}
							else
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio samplerate(%d)", __LINE__, m_uiSamplingRate);
							}
						}
						unsigned int uiSeekResultCTS = 0;
						seekTo(0, &uiSeekResultCTS, NXFF_RA_MODE_PREV, FALSE);
					}
				}

				#if 0
				if( m_uiNumOfChannels > SUPPORTED_AUDIO_MAX_CHANNEL )
				{
					NxFFR_Close( m_pFFReader );
					NxFFR_UnRegisteFileAPI( m_pFFReader );
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not supported Audio channels(%d)", __LINE__, pWaveFormat->nChannel);
					return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
				}
				#endif

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio meta Info(%d %d %d %d)", 
					__LINE__, 
					m_uiSamplingRate,
					m_uiNumOfChannels,
					m_uiNumOfSamplesPerChannel,
					m_uiAudioBitRate);

				/*
				if( pWaveFormat->nSamplePerSecond > SUPPORTED_AUDIO_MAX_SAMPLINGRATE )
				{
					NxFFR_Close( m_pFFReader );
					NxFFR_UnRegisteFileAPI( m_pFFReader );
					return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not supported Audio Sampling rate(%d)", __LINE__, pWaveFormat->nSamplePerSecond);
				}
				*/
			}

			m_isAudioChunkType = isAudioChunkType(	m_pFFReader->hAudioTrackInfo[0].nOTI, 
													m_pFFReader->hAudioTrackInfo[0].n4CC,
													m_pFFReader->MediaFileFormatType);
			
			if( m_isAudioChunkType == TRUE )
			{
				m_hAudioChunkInfo = AudioChunkParser_CreateInstance(	(NEX_CODEC_TYPE)m_uiAudioObjectType,
																		m_pFFReader->hAudioTrackInfo[0].n4CC,
																		(NEX_FILEFORMAT_TYPE)m_pFFReader->MediaFileFormatType,
																		(void*)pWaveFormat,
																		0,
																		NULL);

				m_pAudioChunkFrame			= NULL;
				m_uiAudioChunkFrameLen		= 0;
				m_uiAudioChunkFrameDTS		= 0;
				m_uiAudioChunkFramePTS		= 0;
			}
			else
			{
				if (IS_VARIABLE_OUTPUT_PCM(m_uiAudioObjectType) == FALSE)
					m_isLostFrameCheck = TRUE;
			}

			NEX_FF_RETURN ret = NxFFR_GetMediaDuration(m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, &m_uiTotalPlayAudioTime);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio duration(%d) EditBox(%d) ret(%d)", __LINE__, m_uiTotalPlayAudioTime, m_uiAudioEditBoxTime, ret);
			if(m_isEditBox)
				m_uiTotalPlayAudioTime -= m_uiAudioEditBoxTime;
			m_uiTotalPlayTime = m_uiTotalPlayTime > m_uiTotalPlayAudioTime ? m_uiTotalPlayTime : m_uiTotalPlayAudioTime;

			m_uiAudioFrameInterval		= 0;
			m_dbAudioFrameInterval		= 0.0;
			m_uiMaxAudioFrameInterval		= 0;			
		}
		else
		{
			m_isAudioExist = FALSE;
			if( m_isVideoExist )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Support Video only (NotSupportAudioCodecType(0x%x)", __LINE__, m_uiAudioObjectType);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not Support Audio Codec Type(0x%x)", __LINE__, m_uiAudioObjectType);
				NxFFR_Close( m_pFFReader );
				NxFFR_UnRegisteFileAPI( m_pFFReader );
				return NEXVIDEOEDITOR_ERROR_UNSUPPORT_AUDIO_CODEC;
			}
		}
	}

	m_uiCorrectionTime = 10;
	if( m_uiFrameFPS > 0 )
	{
		int iFrameGap = 1000 / m_uiFrameFPS;
		m_uiCorrectionTime = m_uiCorrectionTime < iFrameGap ? m_uiCorrectionTime : m_uiCorrectionTime - 5;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Open File sucessed Dur(%d)", __LINE__, m_uiTotalPlayTime);
	return eRet;
}

int CNexFileReader::closeFile()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Close Start", __LINE__ );

	if( m_pFFReader == NULL  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Close End", __LINE__ );
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	if( m_pFFReader )
	{
		switch(m_pFFReader->DRMType)
		{
		case NXFF_WINDOWS_MEDIA_DRM:
			break;
		case NXFF_OMA_PDCFV2_DRM:
			break;
		case NXFF_OMA_BCAST_APDCF_DRM:
			break;
		case NXFF_DIVX_DRM:
			break;
		case NXFF_NO_DRM:
		default:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] ---- No DRM ----",__LINE__);
			break;
		}

		NxFFR_Close( m_pFFReader );
		NxFFR_UnRegisteFileAPI(m_pFFReader);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_Close End", __LINE__ );
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::isEndOfChannel(NXFF_MEDIA_TYPE eType, NXBOOL* pisResult )
{
	if( m_pFFReader == NULL || pisResult == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	switch( eType )
	{
	case NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO :
		if( m_isVideoEnd || !m_isVideoExist )	 *pisResult = 1;
		else *pisResult	= 0;
		break;

	case NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO :
		if( !m_isVideoExist )	 *pisResult = 1;
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

int CNexFileReader::getTS(NXFF_MEDIA_TYPE eType, unsigned int* piTS )
{
	if( m_pFFReader == NULL  )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	NEXVIDEOEDITOR_ERROR enRet = NEXVIDEOEDITOR_ERROR_NONE;
	unsigned int pCTS = 0;
	switch( eType )
	{
	case NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO :
		NxFFR_MediaTimeStamp( m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, piTS, &pCTS );
		break;
	case NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO :
		NxFFR_MediaTimeStamp( m_pFFReader, NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, piTS, &pCTS );
		break;
	case NXFF_MEDIA_TYPE_AUDIO :
		NxFFR_MediaTimeStamp( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, piTS, &pCTS );
		break;
	case NXFF_MEDIA_TYPE_TEXT :
		NxFFR_MediaTimeStamp( m_pFFReader, NXFF_MEDIA_TYPE_TEXT, piTS, &pCTS );
		break;
	default :
		*piTS = 0;
		enRet = NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	return enRet;
}

int CNexFileReader::getDSI(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize )
{
	if( m_pFFReader == NULL || ppDSI == NULL || pDSISize == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	switch( eType )
	{
	case NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO :

		if( m_H264DSIBufferSize > 0 )
		{
			*ppDSI = (NXUINT8*)m_H264DSIBuffer;
			*pDSISize = (NXUINT32)m_H264DSIBufferSize;
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
		*ppDSI = m_pFFReader->hVideoTrackInfo[0].pDSI;
		*pDSISize = m_pFFReader->hVideoTrackInfo[0].nDSI;
		break;
	case NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO :
		*ppDSI = m_pFFReader->hVideoTrackInfo[1].pDSI;
		*pDSISize = m_pFFReader->hVideoTrackInfo[1].nDSI;
		break;
	case NXFF_MEDIA_TYPE_AUDIO :
		*ppDSI = m_pFFReader->hAudioTrackInfo[0].pDSI;
		*pDSISize = m_pFFReader->hAudioTrackInfo[0].nDSI;
		//k.lee 20101203 [
		if( m_uiAudioFourcc == 5633 && m_uiFileType == eNEX_FF_ASF )
		{
			/****************************************************************************************************
			* 	Field name(4byte)          *   Description                                                      *
			*****************************************************************************************************
			*	Output Frame Size          *   Number of bytes per frame generated by the decoder.              * 
			*       No Of Samples              *   Number of samples per frame generated by the decoder.            *
			*	Input Sampling Frequency   *   Sampling frequency of the encoded samples.                       *
			*	No Of Output Channels      *   1 = mono, 2 = stereo.                                            *
			*	Profile                    *   1 = AAC-LC.                                                      *
			*       Output Sampling Frequency  *   Sample frequency of the decoded samples.                         *
			*       External Object Type       *   5 = AAC+ (AAC-LC + SBR), 6 = Enhanced AAC+ (AAC-LC + SBR + PS).  *
			*	Down Sample Mode           *   0 = Normal mode.                                                 *
			****************************************************************************************************/

			*ppDSI = NULL;
			*pDSISize = 0;
		}
		//k.lee ]
		break;
	case NXFF_MEDIA_TYPE_TEXT:
		*ppDSI		= NULL;
		*pDSISize	= 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not supported text media type in getDSI", __LINE__);
		break;
	default:
		*ppDSI		= NULL;
		*pDSISize	= 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Not supported unknow media type in getDSI", __LINE__);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( eType == NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO && m_uiVideoObjectType == eNEX_CODEC_V_H264 )
	{
		if( *pDSISize > DEFAILT_DSI_MAX_SIZE )
			return NEXVIDEOEDITOR_ERROR_NONE;

		int 				tempBufferLen 	= *pDSISize;
		unsigned char* 	pTemDSI		= (unsigned char*)*ppDSI;

		memset(m_H264DSIBuffer, 0x00, DEFAILT_DSI_MAX_SIZE);

		if( pTemDSI == NULL || tempBufferLen <= 0 )
		{
			getVideoDSIfromFirstFrame();
			
			*ppDSI		= (NXUINT8*)m_H264DSIBuffer;
			*pDSISize	= m_H264DSIBufferSize;

			m_uiDSINALSize = NexCodecUtil_AVC_ParseH264NalHeaderLengthSize(m_H264DSIBuffer, m_H264DSIBufferSize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] h264 dsi info(%p, %d NalHeaderSize(%d))", __LINE__,  m_H264DSIBuffer, m_H264DSIBufferSize, m_uiDSINALSize);
			nexSAL_MemDump(m_H264DSIBuffer, m_H264DSIBufferSize);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
		
		nexSAL_MemDump(pTemDSI, tempBufferLen);

		m_uiDSINALSize = NexCodecUtil_AVC_ParseH264NalHeaderLengthSize(pTemDSI, tempBufferLen);
		const NXINT32 nConfigRecorderLen = 5; // [shoh][2012.04.06] Configuration recorder length is 5 bytes and master 3bits of the 6th byte should be 0.
		if((NexCodecUtil_AVC_IsConfigFromRecordType(pTemDSI, tempBufferLen) == TRUE) && (tempBufferLen > nConfigRecorderLen))
		{
			pTemDSI += nConfigRecorderLen;
			tempBufferLen -= nConfigRecorderLen;
		}

		m_H264DSIBufferSize = tempBufferLen;
		memcpy(m_H264DSIBuffer, pTemDSI, m_H264DSIBufferSize);
		*ppDSI		= (NXUINT8*)m_H264DSIBuffer;
		*pDSISize	= m_H264DSIBufferSize;
		
		nexSAL_MemDump(pTemDSI, tempBufferLen);
	}

	if( eType == NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO && m_uiVideoObjectType == eNEX_CODEC_V_HEVC )
	{
		if( *pDSISize > DEFAILT_DSI_MAX_SIZE )
			return NEXVIDEOEDITOR_ERROR_NONE;

		int 				tempBufferLen 	= *pDSISize;
		unsigned char* 	pTemDSI		= (unsigned char*)*ppDSI;

		memset(m_H264DSIBuffer, 0x00, DEFAILT_DSI_MAX_SIZE);

		if( pTemDSI == NULL || tempBufferLen <= 0 )
		{
			getVideoDSIfromFirstFrame();
			
			*ppDSI		= (NXUINT8*)m_H264DSIBuffer;
			*pDSISize	= m_H264DSIBufferSize;

			m_uiDSINALSize = NexCodecUtil_HEVC_ParseNalHeaderLengthSize(m_H264DSIBuffer, m_H264DSIBufferSize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] h264 dsi info(%p, %d NalHeaderSize(%d))", __LINE__,  m_H264DSIBuffer, m_H264DSIBufferSize, m_uiDSINALSize);
			nexSAL_MemDump(m_H264DSIBuffer, m_H264DSIBufferSize);
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
		
		nexSAL_MemDump(pTemDSI, tempBufferLen);

		m_uiDSINALSize = NexCodecUtil_HEVC_ParseNalHeaderLengthSize(pTemDSI, tempBufferLen);

#if 0
		const NXINT32 nConfigRecorderLen = 5; // [shoh][2012.04.06] Configuration recorder length is 5 bytes and master 3bits of the 6th byte should be 0.
		NXINT32 PsStartPos;
		if((NexCodecUtil_HEVC_IsConfigFromRecordType(pTemDSI, tempBufferLen, &PsStartPos) == TRUE) && (tempBufferLen > nConfigRecorderLen))
		{
			pTemDSI += nConfigRecorderLen;
			tempBufferLen -= nConfigRecorderLen;
		}
#endif

		m_H264DSIBufferSize = tempBufferLen;
		memcpy(m_H264DSIBuffer, pTemDSI, m_H264DSIBufferSize);
		*ppDSI		= (NXUINT8*)m_H264DSIBuffer;
		*pDSISize	= m_H264DSIBufferSize;
		
		nexSAL_MemDump(pTemDSI, tempBufferLen);
	}    
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getVideoDSIfromFirstFrame()
{
	if( m_pFFReader == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	memset(m_H264DSIBuffer, 0x00, DEFAILT_DSI_MAX_SIZE);
	m_H264DSIBufferSize = 0;

	unsigned int uiSeekResultCTS = 0;
	seekTo(0, &uiSeekResultCTS, NXFF_RA_MODE_PREV, FALSE);

	if( getVideoFrame() == _SRC_GETFRAME_OK )
	{
		unsigned char*	pFrame		= NULL;
		unsigned int		uiFrameLen	= 0;
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] get frame OK", __LINE__);
		if( getVideoFrameData( &pFrame, &uiFrameLen ) == NEXVIDEOEDITOR_ERROR_NONE )
		{
			// nexSAL_MemDump(pFrame, 100);
			m_uiFrameFormat = NexCodecUtil_CheckByteFormat( pFrame, uiFrameLen);
			m_uiFrameNALSize = NexCodecUtil_GuessNalHeaderLengthSize(pFrame, uiFrameLen);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] frma NAL Size(%d)", __LINE__, m_uiFrameNALSize);

			if( m_uiFrameFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB )
			{
				unsigned char*	pData		= NULL;
				int				iDataSize	= 0;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp  %d] video frame type = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB", __LINE__ );

				if(m_uiVideoObjectType == eNEX_CODEC_V_H264)
					pData = (unsigned char *)NexCodecUtil_AVC_ANNEXB_GetConfigStream(pFrame, uiFrameLen, &iDataSize);

				if(m_uiVideoObjectType == eNEX_CODEC_V_HEVC)
					pData = (unsigned char *)NexCodecUtil_HEVC_ANNEXB_GetConfigStream(pFrame, uiFrameLen, &iDataSize);

				if( pData != NULL && iDataSize > 0 )
				{
					iDataSize -= (int)(pFrame - pData);

					if(m_uiVideoObjectType == eNEX_CODEC_V_H264)
						m_H264DSIBufferSize = NexCodecUtil_AVC_ConvertFormatPSs( m_H264DSIBuffer, DEFAILT_DSI_MAX_SIZE, pData, iDataSize, m_uiFrameFormat );

					if(m_uiVideoObjectType == eNEX_CODEC_V_HEVC)
						m_H264DSIBufferSize = NexCodecUtil_HEVC_ConvertFormatPSs( m_H264DSIBuffer, DEFAILT_DSI_MAX_SIZE, pData, iDataSize, m_uiFrameFormat );

					if( iDataSize < (int)m_H264DSIBufferSize || m_H264DSIBufferSize == (unsigned int)-1)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp  %d] NexCodecUtil_AVC_ConvertFormatPSs failed", __LINE__ );               
						memset(m_H264DSIBuffer, 0x00, DEFAILT_DSI_MAX_SIZE);
						m_H264DSIBufferSize = 0;
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp  %d] SPS/PPS info from first frame", __LINE__ );
						nexSAL_MemDump(m_H264DSIBuffer, m_H264DSIBufferSize);
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp  %d] no SPS/PPS info", __LINE__ );
					nexSAL_MemDump(pFrame, uiFrameLen);
				}
			}
			else
			{
				unsigned char*	pData		= NULL;
				int				iDataSize	= 0;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp  %d] video frame type = NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW", __LINE__ );

				m_uiFrameNALSize = NexCodecUtil_GuessNalHeaderLengthSize(pFrame, uiFrameLen);
				if(m_uiVideoObjectType == eNEX_CODEC_V_H264)
					pData = (unsigned char *)NexCodecUtil_AVC_NAL_GetConfigStream(pFrame, uiFrameLen, (int)m_uiFrameNALSize, &iDataSize);

				if(m_uiVideoObjectType == eNEX_CODEC_V_HEVC)
					pData = (unsigned char *)NexCodecUtil_HEVC_NAL_GetConfigStream(pFrame, uiFrameLen, (int)m_uiFrameNALSize, &iDataSize);
                
				if( pData != NULL && iDataSize > 0 )
				{
					memcpy(m_H264DSIBuffer, pData, iDataSize);
					m_H264DSIBufferSize = iDataSize;
					
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp  %d] SPS/PPS info from first frame", __LINE__ );
					nexSAL_MemDump(m_H264DSIBuffer, m_H264DSIBufferSize);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp  %d] no SPS/PPS info", __LINE__ );
					nexSAL_MemDump(pFrame, uiFrameLen);

					memset(m_H264DSIBuffer, 0x00, DEFAILT_DSI_MAX_SIZE);
					m_H264DSIBufferSize = 0;
				}
			}
		}
	}
	seekTo(0, &uiSeekResultCTS, NXFF_RA_MODE_PREV, FALSE);		

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::moveStartPos()
{
	if( m_pFFReader == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	unsigned int uiResultAudioCTS	= 0;
	unsigned int uiNxFFRaMode		= NXFF_RA_MODE_SEEK;

	NEX_FF_RETURN eFFRet = eNEX_FF_RET_FAIL;

	nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );

	if( m_isVideoExist )
	{
		eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, NXFF_RA_MODE_SEEK, m_uiStartTrimTime, &uiResultVideoCTS);
		if( eFFRet != eNEX_FF_RET_SUCCESS )
		{
			nexSAL_MutexUnlock( m_hReaderMutex );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u, Err(%d)", __LINE__, m_uiStartTrimTime, eFFRet);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}

		if( m_isEnhancedExist )
		{
			unsigned int uiTempTime;
			eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, NXFF_RA_MODE_SEEK, uiResultVideoCTS, &uiTempTime);
			if( eFFRet != eNEX_FF_RET_SUCCESS )
			{
				nexSAL_MutexUnlock( m_hReaderMutex );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(EV) Target:%u, Err(%d)", __LINE__, uiResultVideoCTS, eFFRet);
				return NEXVIDEOEDITOR_ERROR_UNKNOWN;
			}			
		}

		if( m_isAudioExist )
		{
			eFFRet = NxFFR_RASeek( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, NXFF_RA_MODE_SEEK, uiResultVideoCTS, &uiResultAudioCTS);
			if( eFFRet != eNEX_FF_RET_SUCCESS)
			{
				nexSAL_MutexUnlock( m_hReaderMutex );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%u, Err(%d)", __LINE__, uiResultVideoCTS, eFFRet);
				return NEXVIDEOEDITOR_ERROR_UNKNOWN;
			}
		}
	}
	else if( m_isAudioExist )
	{
		eFFRet = NxFFR_RASeek( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, NXFF_RA_MODE_SEEK, m_uiStartTrimTime, &uiResultAudioCTS);
		if( eFFRet != eNEX_FF_RET_SUCCESS)
		{
			nexSAL_MutexUnlock( m_hReaderMutex );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%u, Err(%d)", __LINE__, m_uiStartTrimTime, eFFRet);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}
	}
	nexSAL_MutexUnlock( m_hReaderMutex );
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::seekTo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pFFReader == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	unsigned int uiResultAudioCTS	= 0;
	NEX_FF_RETURN eFFRet = eNEX_FF_RET_FAIL;
	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekTo(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);

	uiTime = uiTime <= 1 ? 0 : uiTime;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekTo(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( bRepeat == FALSE )
	{
		uiTime = (uiTime + m_uiStartTrimTime) > m_uiBaseTimeStamp ?  (uiTime + m_uiStartTrimTime) - m_uiBaseTimeStamp : 0;
	}

	if( bABSTime && uiTime > (m_uiTotalPlayTime - m_uiEndTrimTime) )
	{
		uiTime = m_uiTotalPlayTime > (m_uiEndTrimTime + 1000) ? m_uiTotalPlayTime - (m_uiEndTrimTime + 1000) : 0;
	}


	nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );

	if( m_isVideoExist )
	{
		unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;
		eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, iSeekMode, uiTime + uiVideoEditBoxTime, &uiResultVideoCTS);
		if (eFFRet != eNEX_FF_RET_SUCCESS)
		{
			nexSAL_MutexUnlock( m_hReaderMutex );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u, Err(%d)", __LINE__, uiTime, eFFRet);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u Result:(%u)", __LINE__, uiTime, uiResultVideoCTS);
 		if( m_isEnhancedExist )
		{
			unsigned int uiTempTime;
			eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, iSeekMode, uiResultVideoCTS, &uiTempTime);
			if (eFFRet != eNEX_FF_RET_SUCCESS)
			{
				nexSAL_MutexUnlock( m_hReaderMutex );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(EV) Target:%u, Err(%d)", __LINE__, uiResultVideoCTS, eFFRet);
				return NEXVIDEOEDITOR_ERROR_UNKNOWN;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) EditBox:%d, Target:%u Result:%u", __LINE__, uiVideoEditBoxTime, uiTime, uiResultVideoCTS);
		}
		*pSeekResultCTS = uiResultVideoCTS > uiVideoEditBoxTime?uiResultVideoCTS-uiVideoEditBoxTime:0;
	}

	if( m_isAudioExist )
	{
		unsigned int uiAudioEditBoxTime = m_isEditBox ? m_uiAudioEditBoxTime : 0;
		m_bUsePrevFrame = FALSE;
		if(m_isAudioChunkType && uiTime != 0)
		{
			int count = 0;
			while(count < 5)
			{
				int seekTime = uiTime+uiAudioEditBoxTime - count*500;
				if(seekTime < 0) 
					seekTime = 0;
				//nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek 002(%d)", __LINE__, uiTime );
				eFFRet = NxFFR_RASeek( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, iSeekMode, seekTime, &uiResultAudioCTS);
		
				if( eFFRet != eNEX_FF_RET_SUCCESS)
				{
					nexSAL_MutexUnlock( m_hReaderMutex );
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%d, Err(%d)", __LINE__, uiTime, eFFRet);
					return NEXVIDEOEDITOR_ERROR_UNKNOWN;
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) EditBox:%d, Target:%d Result(%d)", __LINE__, uiAudioEditBoxTime, seekTime, uiResultAudioCTS);
				if(uiResultAudioCTS < uiTime + uiAudioEditBoxTime + 30 || uiResultAudioCTS == 0)
				{
					if( m_isAudioChunkType && m_hAudioChunkInfo)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio reset chunk parser", __LINE__);
						AudioChunkParser_Reset(m_hAudioChunkInfo);

						m_pAudioChunkFrame		= NULL;
						m_uiAudioChunkFrameLen	= 0;
						m_uiAudioChunkFrameDTS	= 0;
						m_uiAudioChunkFramePTS	= 0;
					}					
					break;
				}
				count++;
			}

			nexSAL_MutexUnlock( m_hReaderMutex );
			while(uiResultAudioCTS < uiTime+uiAudioEditBoxTime)
			{
				NXINT64 uCTS;
				unsigned int uiReaderRet = 0;
				uiReaderRet = getAudioFrame();
				if( uiReaderRet != _SRC_GETFRAME_OK )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%d, Err(%d)", __LINE__, uiTime, eFFRet);
					return NEXVIDEOEDITOR_ERROR_UNKNOWN;
				}

				if(m_uiAudioChunkFrameDTS + 30 > uiTime + uiAudioEditBoxTime)
				{
					m_bUsePrevFrame = TRUE;
					uiResultAudioCTS = m_uiAudioChunkFrameDTS;
					break;
				}
			}
			nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );
		}
		else
		{
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek 002(%d)", __LINE__, uiTime );
			eFFRet = NxFFR_RASeek( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, iSeekMode, uiTime+uiAudioEditBoxTime, &uiResultAudioCTS);
	
			if( eFFRet != eNEX_FF_RET_SUCCESS)
			{
				nexSAL_MutexUnlock( m_hReaderMutex );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%d, Err(%d)", __LINE__, uiTime, eFFRet);
				return NEXVIDEOEDITOR_ERROR_UNKNOWN;
			}

			if( m_isAudioChunkType && m_hAudioChunkInfo)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio reset chunk parser", __LINE__);
				AudioChunkParser_Reset(m_hAudioChunkInfo);

				m_pAudioChunkFrame		= NULL;
				m_uiAudioChunkFrameLen	= 0;
				m_uiAudioChunkFrameDTS	= 0;
				m_uiAudioChunkFramePTS	= 0;
			}
		}

		if(m_uiAudioObjectType == eNEX_CODEC_A_FLAC)
		{
			if(uiTime > uiResultAudioCTS + 300)
			{
				NXINT64	uiCTS = 0;
				nexSAL_MutexUnlock( m_hReaderMutex );
				while(getAudioFrame() == _SRC_GETFRAME_OK)
				{
					getAudioFrameCTS(&uiCTS);
					if(uiTime < uiCTS + 300)
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) ReqTime:%d, OriResult:%d, AfterResult(%d)", __LINE__, uiTime, uiResultAudioCTS, uiCTS);
						uiResultAudioCTS = uiCTS;
						break;
					}
				}
				nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );
			}
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) EditBox:%d, Target:%d Result(%d)", __LINE__, uiAudioEditBoxTime, uiTime, uiResultAudioCTS);
		*pSeekResultCTS = uiResultAudioCTS > uiAudioEditBoxTime?uiResultAudioCTS-uiAudioEditBoxTime:0;
	}
	nexSAL_MutexUnlock( m_hReaderMutex );
	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	m_isAudioEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;
	m_uiAudioLastReadCTS	= uiResultAudioCTS;
	m_uiLastAudioCTS		= m_uiAudioLastReadCTS;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek last read CTS and PTS(%u, %u) result(%d)",
		__LINE__, m_uiVideoLastReadDTS, m_uiVideoLastReadPTS, *pSeekResultCTS);

	/*
	if( m_uiBaseTimeStamp == 0 && uiResultVideoCTS < m_uiStartTrimTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Update StartTrimTime lastCTS(%d) m_uiStartTrimTime(%d -> %d)", 
			__LINE__, m_uiVideoLastReadCTS, m_uiStartTrimTime, m_uiVideoLastReadCTS);
		m_uiStartTrimTime = uiResultVideoCTS;
	}
	*/
	if( m_isVideoChunkType && m_hVideoChunkInfo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video reset chunk parser", __LINE__);
		VideoChunkParser_Reset(m_hVideoChunkInfo);

		m_pVideoChunkFrame		= NULL;
		m_uiVideoChunkFrameLen	= 0;
		m_uiVideoChunkFrameDTS	= 0;
		m_uiVideoChunkFramePTS	= 0;
	}

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::seekToAudio(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pFFReader == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultAudioCTS		= 0;
	unsigned int uiOriTime = uiTime;
	NEX_FF_RETURN eFFRet = eNEX_FF_RET_FAIL;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekToAudio(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekToAudio(Seek:%d Total(%d) BaseTime(%d))", 
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

	nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );

	m_isAudioEnd = 0;
	unsigned int uiAudioEditBoxTime = m_isEditBox ? m_uiAudioEditBoxTime : 0;
	m_bUsePrevFrame = FALSE;
	if(m_isAudioChunkType && uiTime != 0)
	{
		int count = 0;
		while(count < 5)
		{
			int seekTime = uiTime+uiAudioEditBoxTime - count*500;
			if(seekTime < 0) 
				seekTime = 0;
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek 002(%d)", __LINE__, uiTime );
			eFFRet = NxFFR_RASeek( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, iSeekMode, seekTime, &uiResultAudioCTS);
	
			if( eFFRet != eNEX_FF_RET_SUCCESS)
			{
				nexSAL_MutexUnlock( m_hReaderMutex );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%d, Err(%d)", __LINE__, uiTime, eFFRet);
				return NEXVIDEOEDITOR_ERROR_UNKNOWN;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) EditBox:%d, Target:%d Result(%d)", __LINE__, uiAudioEditBoxTime, seekTime, uiResultAudioCTS);
			if(uiResultAudioCTS < uiTime + uiAudioEditBoxTime + 30 || uiResultAudioCTS == 0)
			{
				if( m_isAudioChunkType && m_hAudioChunkInfo)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio reset chunk parser", __LINE__);
					AudioChunkParser_Reset(m_hAudioChunkInfo);

					m_pAudioChunkFrame		= NULL;
					m_uiAudioChunkFrameLen	= 0;
					m_uiAudioChunkFrameDTS	= 0;
					m_uiAudioChunkFramePTS	= 0;
				}					
				break;
			}
			count++;
		}

		nexSAL_MutexUnlock( m_hReaderMutex );
		while(uiResultAudioCTS < uiTime+uiAudioEditBoxTime)
		{
			NXINT64 uCTS;
			unsigned int uiReaderRet = 0;
			uiReaderRet = getAudioFrame();
			if( uiReaderRet != _SRC_GETFRAME_OK )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%d, Err(%d)", __LINE__, uiTime, eFFRet);
				return NEXVIDEOEDITOR_ERROR_UNKNOWN;
			}

			if(m_uiAudioChunkFrameDTS + 30 > uiTime+uiAudioEditBoxTime)
			{
				m_bUsePrevFrame = TRUE;
				uiResultAudioCTS = m_uiAudioChunkFrameDTS;
				break;
			}
		}
		nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );
	}
	else
	{
		if (uiTime < 20)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek Audio (%d -> 0)", __LINE__, uiTime);
			uiTime = 0;
		}
		//nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek 002(%d)", __LINE__, uiTime );
		eFFRet = NxFFR_RASeek( m_pFFReader, NXFF_MEDIA_TYPE_AUDIO, iSeekMode, uiTime+uiAudioEditBoxTime, &uiResultAudioCTS);

		if( eFFRet != eNEX_FF_RET_SUCCESS)
		{
			nexSAL_MutexUnlock( m_hReaderMutex );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Target:%d, Err(%d)", __LINE__, uiTime, eFFRet);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}

		if( m_isAudioChunkType && m_hAudioChunkInfo)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio reset chunk parser", __LINE__);
			AudioChunkParser_Reset(m_hAudioChunkInfo);

			m_pAudioChunkFrame		= NULL;
			m_uiAudioChunkFrameLen	= 0;
			m_uiAudioChunkFrameDTS	= 0;
			m_uiAudioChunkFramePTS	= 0;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(A) Edit Box:%d Target:%d Result(%d)", __LINE__, uiAudioEditBoxTime, uiTime, uiResultAudioCTS);
	*pSeekResultCTS = uiResultAudioCTS > uiAudioEditBoxTime?uiResultAudioCTS-uiAudioEditBoxTime:0;

	nexSAL_MutexUnlock( m_hReaderMutex );
	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek result(%d)", __LINE__, *pSeekResultCTS);

	m_uiAudioLastReadCTS	= uiResultAudioCTS;
	m_uiLastAudioCTS = m_uiAudioLastReadCTS;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::seekToVideo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode, NXBOOL bRepeat, NXBOOL bABSTime)
{
	if( m_pFFReader == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	NEX_FF_RETURN eFFRet = eNEX_FF_RET_FAIL;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekTo(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekTo(Seek:%d Total(%d) BaseTime(%d))", 
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
		uiTime = m_uiTotalPlayTime > (m_uiEndTrimTime + 1000) ? m_uiTotalPlayTime - (m_uiEndTrimTime + 1000) : 0;
	}

	nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );

	unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;
	eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, iSeekMode, uiTime + uiVideoEditBoxTime, &uiResultVideoCTS);
	if (eFFRet != eNEX_FF_RET_SUCCESS)
	{
		nexSAL_MutexUnlock( m_hReaderMutex );
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u, Err(%d)", __LINE__, uiTime, eFFRet);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u Result:%u", __LINE__, uiTime, uiResultVideoCTS);
	if( m_isEnhancedExist )
	{
		unsigned int uiTempTime = 0;
		eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, iSeekMode, uiResultVideoCTS, &uiTempTime);
		if (eFFRet != eNEX_FF_RET_SUCCESS)
		{
			nexSAL_MutexUnlock( m_hReaderMutex );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(EV) EditBox:%d, Target:%u, Err(%d)", __LINE__, uiVideoEditBoxTime, uiResultVideoCTS, eFFRet);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u Result:%u", __LINE__, uiTime, uiResultVideoCTS);
	}

	*pSeekResultCTS = uiResultVideoCTS > uiVideoEditBoxTime?uiResultVideoCTS-uiVideoEditBoxTime:0;

	nexSAL_MutexUnlock( m_hReaderMutex );
	*pSeekResultCTS = (*pSeekResultCTS + m_uiBaseTimeStamp) - m_uiStartTrimTime;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;

	if( m_isVideoChunkType && m_hVideoChunkInfo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video reset chunk parser", __LINE__);
		VideoChunkParser_Reset(m_hVideoChunkInfo);

		m_pVideoChunkFrame		= NULL;
		m_uiVideoChunkFrameLen	= 0;
		m_uiVideoChunkFrameDTS	= 0;
		m_uiVideoChunkFramePTS	= 0;
	}
	return NEXVIDEOEDITOR_ERROR_NONE;	
}

int CNexFileReader::seekToVideoWithAbsoluteTime(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode)
{
	if( m_pFFReader == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	if( pSeekResultCTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	unsigned int uiResultVideoCTS	= 0;
	NEX_FF_RETURN eFFRet = eNEX_FF_RET_FAIL;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekTo(Seek:%d StartTrim(%d) EndTrim(%d))", 
		__LINE__, uiTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] seekTo(Seek:%d Total(%d) BaseTime(%d))", 
		__LINE__, uiTime, m_uiTotalPlayTime, m_uiBaseTimeStamp);

	if( m_isVideoExist == FALSE )
	{
		return NEXVIDEOEDITOR_ERROR_NO_ACTION;
	}

	nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );

	unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;
	eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, iSeekMode, uiTime+uiVideoEditBoxTime, &uiResultVideoCTS);
	if (eFFRet != eNEX_FF_RET_SUCCESS)
	{
		nexSAL_MutexUnlock( m_hReaderMutex );
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u, Err(%d)", __LINE__, uiTime, eFFRet);
		return NEXVIDEOEDITOR_ERROR_UNKNOWN;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) Target:%u Result:%u", __LINE__, uiTime, uiResultVideoCTS);
	if( m_isEnhancedExist )
	{
		unsigned int uiTempTime = 0;
		eFFRet = NxFFR_RASeek(m_pFFReader, NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, iSeekMode, uiResultVideoCTS, &uiTempTime);
		if (eFFRet != eNEX_FF_RET_SUCCESS)
		{
			nexSAL_MutexUnlock( m_hReaderMutex );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(EV) Target:%u, Err(%d)", __LINE__, uiResultVideoCTS, eFFRet);
			return NEXVIDEOEDITOR_ERROR_UNKNOWN;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek(V) EditBox:%d, Target:%u Result:%u", __LINE__, uiVideoEditBoxTime, uiTime, uiResultVideoCTS);
	}
	*pSeekResultCTS = uiResultVideoCTS > uiVideoEditBoxTime?uiResultVideoCTS-uiVideoEditBoxTime:0;

	nexSAL_MutexUnlock( m_hReaderMutex );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFReaderRASeek result(%d)", __LINE__, *pSeekResultCTS);

	m_isVideoEnd = 0;
	
	m_uiVideoLastReadDTS	= uiResultVideoCTS;
	m_uiVideoLastReadPTS	= uiResultVideoCTS;

	if( m_isVideoChunkType && m_hVideoChunkInfo)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video reset chunk parser", __LINE__);
		VideoChunkParser_Reset(m_hVideoChunkInfo);

		m_pVideoChunkFrame		= NULL;
		m_uiVideoChunkFrameLen	= 0;
		m_uiVideoChunkFrameDTS	= 0;
		m_uiVideoChunkFramePTS	= 0;
	}
	return NEXVIDEOEDITOR_ERROR_NONE;	
}

int CNexFileReader::getFindIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime)
{
	if( pResultTime == NULL )
		return 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos(Start:%d Target:%d Base:%d STrim:%d)", 
		__LINE__, uiStartTime, uiTargetTime, m_uiBaseTimeStamp, m_uiStartTrimTime);	
	nxFFRIFrameIdx* iframeIndex = NULL;

	unsigned int uiRealTarget = (uiTargetTime - m_uiBaseTimeStamp + m_uiStartTrimTime);
	NXINT64 startTime = (NXINT64)(uiStartTime - m_uiBaseTimeStamp + m_uiStartTrimTime);
	unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFR_GetExtInfo(Start:%lld)", __LINE__, startTime);	

	NEX_FF_RETURN eRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_IFRAME_INDEX, startTime, NULL, (void**)&iframeIndex);
	if( eRet != eNEX_FF_RET_SUCCESS )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos is Failed(Ret:%d)", __LINE__, eRet);	
		return 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos is Sucessed(Ret:%d)", __LINE__, eRet);	
	if( iframeIndex == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos is failed because parameter is null", __LINE__);	
		return 0;
	}

	if( iframeIndex->nCnt >= 1 && uiRealTarget + uiVideoEditBoxTime  < iframeIndex->pFinfo[0].nTimeStamp )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos did not find IDR frames", __LINE__);	
		return 2;
	}

	if( iframeIndex->nCnt >= 1 && uiRealTarget + uiVideoEditBoxTime  > iframeIndex->pFinfo[iframeIndex->nCnt-1].nTimeStamp )
	{
		int iGap = uiRealTarget + uiVideoEditBoxTime  - iframeIndex->pFinfo[iframeIndex->nCnt-1].nTimeStamp;
		if( iGap > 2000 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos did not find IDR frames", __LINE__);	
			return 3;
		}
	}	

	int i = 0;
	// for JIRA 2588
#if 0	
	for( i = 0; i < iframeIndex->nCnt; i++)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Check Time(Tar : %d IDR : %d)", __LINE__, uiRealTarget , iframeIndex->pFinfo[i].nTimeStamp);		
		if( uiRealTarget  < iframeIndex->pFinfo[i].nTimeStamp )
		{
			if( ( uiRealTarget  + 500 ) >= iframeIndex->pFinfo[i].nTimeStamp )
			{
				//���� I Frame�� �����̸� ���� IFrame�� ��.
				*pResultTime = iframeIndex->pFinfo[i].nTimeStamp + 10 + m_uiBaseTimeStamp - m_uiStartTrimTime;;
			}
			else
			{
				*pResultTime = iframeIndex->pFinfo[i].nTimeStamp - 10 + m_uiBaseTimeStamp - m_uiStartTrimTime;;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Find idr frame(Target:%d Result:%d)", __LINE__, uiRealTarget , *pResultTime);	
			return 1;
		}
	}
#else
	for( i = iframeIndex->nCnt-1; i >= 0; i--)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Check Time(Tar : %d IDR : %d)", __LINE__, uiRealTarget , iframeIndex->pFinfo[i].nTimeStamp);		
		if( uiRealTarget + m_uiVideoEditBoxTime == iframeIndex->pFinfo[i].nTimeStamp )
		{
			*pResultTime = iframeIndex->pFinfo[i].nTimeStamp - 10 + m_uiBaseTimeStamp - m_uiStartTrimTime - m_uiVideoEditBoxTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Find idr frame(Target:%d Result:%d)", __LINE__, uiRealTarget , *pResultTime);	
			return 1;
		}
		else if( uiRealTarget + m_uiVideoEditBoxTime > iframeIndex->pFinfo[i].nTimeStamp )
		{
			*pResultTime = iframeIndex->pFinfo[i].nTimeStamp + m_uiBaseTimeStamp - m_uiStartTrimTime - m_uiVideoEditBoxTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Find idr frame(Target:%d Result:%d)", __LINE__, uiRealTarget , *pResultTime);	
			return 1;
		}
	}
#endif	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos did not find IDR frames", __LINE__);	
	return 3;
}

#define ABS_INT(a) (a<0?-a:a)

int CNexFileReader::getFindNearIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime)
{
	if( pResultTime == NULL )
		return 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos(Start:%d Target:%d Base:%d STrim:%d)", 
		__LINE__, uiStartTime, uiTargetTime, m_uiBaseTimeStamp, m_uiStartTrimTime);	
	nxFFRIFrameIdx* iframeIndex = NULL;

	unsigned int uiRealTarget = (uiTargetTime - m_uiBaseTimeStamp + m_uiStartTrimTime);
	NXINT64 startTime = (NXINT64)(uiStartTime - m_uiBaseTimeStamp + m_uiStartTrimTime);
	unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] NxFFR_GetExtInfo(Start:%lld)", __LINE__, startTime);	

	NEX_FF_RETURN eRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_IFRAME_INDEX, startTime, NULL, (void**)&iframeIndex);
	if( eRet != eNEX_FF_RET_SUCCESS )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos is Failed(Ret:%d)", __LINE__, eRet);	
		return 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos is Sucessed(Ret:%d)", __LINE__, eRet);	
	if( iframeIndex == NULL || iframeIndex->nCnt <= 0)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos is failed because parameter is null", __LINE__);	
		return 0;
	}

	// for mantis 8718
	// int iPreCheckGap = ABS_INT(uiRealTarget  - iframeIndex->pFinfo[0].nTimeStamp);
	if( iframeIndex->nCnt == 1 )
	{
		if( uiRealTarget + uiVideoEditBoxTime > iframeIndex->pFinfo[0].nTimeStamp )
		{
			int iPreCheckGap = ABS_INT(uiRealTarget + uiVideoEditBoxTime - iframeIndex->pFinfo[0].nTimeStamp);
			if( iPreCheckGap < 2000 )
			{
				*pResultTime = (iframeIndex->pFinfo[0].nTimeStamp + 10) + m_uiBaseTimeStamp - m_uiStartTrimTime - uiVideoEditBoxTime;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Find idr frame(Target:%d Result:%d)", __LINE__, uiRealTarget , *pResultTime);	
				return 1;
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos did not find IDR frames", __LINE__);	
		return 2;
	}
	else
	{
		for( int i = 0; i < iframeIndex->nCnt -1; i++)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Check Time(Tar : %d IDR : %d %d)",
				__LINE__, uiRealTarget , iframeIndex->pFinfo[i].nTimeStamp, iframeIndex->pFinfo[i+1].nTimeStamp);
			if( (uiRealTarget + uiVideoEditBoxTime  > iframeIndex->pFinfo[i].nTimeStamp) && (iframeIndex->pFinfo[i].nTimeStamp < (m_uiTotalPlayTime - m_uiEndTrimTime)) )
			{
				if( uiRealTarget + uiVideoEditBoxTime > iframeIndex->pFinfo[i+1].nTimeStamp )
				{
					continue;
				}
				
				int uiPreGap = uiRealTarget + uiVideoEditBoxTime - iframeIndex->pFinfo[i].nTimeStamp;
				int uiNextGap = iframeIndex->pFinfo[i+1].nTimeStamp - uiRealTarget - uiVideoEditBoxTime;

				if( uiPreGap < uiNextGap )
				{
					*pResultTime = iframeIndex->pFinfo[i].nTimeStamp + 10 + m_uiBaseTimeStamp - m_uiStartTrimTime - uiVideoEditBoxTime;
				}
				else
				{
					*pResultTime = iframeIndex->pFinfo[i+1].nTimeStamp - 10 + m_uiBaseTimeStamp - m_uiStartTrimTime - uiVideoEditBoxTime;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos Find idr frame(Target:%d Result:%d)", __LINE__, uiRealTarget , *pResultTime);	
				return 1;
			}
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getFindIFramePos did not find IDR frames", __LINE__);	
	return 2;
}

int CNexFileReader::getSeekTable(unsigned int uiStartTime, int* pSeekTableCount, unsigned int** ppSeekTable)
{
	unsigned int uiStartTick = nexSAL_GetTickCount();
	if( pSeekTableCount == NULL || ppSeekTable == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable Arg failed", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( uiStartTime > m_uiTotalPlayTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable Arg failed", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( m_pFFReader == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable reader handle failed", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	int iRet = NEXVIDEOEDITOR_ERROR_NONE;
	nxFFRIFrameIdx* iframeIndex = NULL;
	unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;
	int iSeekTableCount = m_uiCountIframe?m_uiCountIframe:1; //set 1 if m_uiCountIframe is 0 - NESA-894:stss box(Sample Table box) is not exist.
	int iSeekTableIndex = 0;
	int iSkipForEditbox = 0;    
	unsigned int* pSeekTable = (unsigned int*)nexSAL_MemAlloc(sizeof(unsigned int)*iSeekTableCount);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable TotalCount(%d)", __LINE__, m_uiCountIframe);	
	
	if( pSeekTable == NULL )
	{
		iRet = NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
		goto ERR_GETSEEKTABLE;
	}

	while( iSeekTableIndex + iSkipForEditbox < iSeekTableCount)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable Find start(%d)", __LINE__, uiStartTime);	
		NEX_FF_RETURN eRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_IFRAME_INDEX, (NXINT64)uiStartTime, NULL, (void**)&iframeIndex);
		if( eRet != eNEX_FF_RET_SUCCESS ||iframeIndex ==  NULL )
		{
			//NESA-894:stss box(Sample Table box) is not exist.
			if (m_uiCountIframe == 0 && iSeekTableIndex == 0)
			{
				pSeekTable[iSeekTableIndex] = 1; // for pre seek option
				uiStartTime = pSeekTable[iSeekTableIndex];
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[WrapFileReader.cpp %d] getSeekTable (inx:%d, Time:%d)", __LINE__, iSeekTableIndex, uiStartTime );
				iSeekTableIndex++;
				continue;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable is Failed(Ret:%d)", __LINE__, eRet);	
			break;
		}

		int i = 0;
		for( ; i < iframeIndex->nCnt && iSeekTableIndex > 1; i++)
		{
			unsigned int uTime = iframeIndex->pFinfo[i].nTimeStamp + 1 - uiVideoEditBoxTime;
			if( pSeekTable[iSeekTableIndex-1] == iframeIndex->pFinfo[i].nTimeStamp + 1 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getSeekTable wrongCTS(%d %d)", __LINE__, pSeekTable[iSeekTableIndex-1], iframeIndex->pFinfo[i].nTimeStamp);
				continue;
			}
			break;
		}

		for( ; i < iframeIndex->nCnt && iSeekTableIndex + iSkipForEditbox < iSeekTableCount; i++)
		{
			if(iframeIndex->pFinfo[i].nTimeStamp >= uiVideoEditBoxTime)
			{
				pSeekTable[iSeekTableIndex] =iframeIndex->pFinfo[i].nTimeStamp + 1 - uiVideoEditBoxTime; // for pre seek option
				uiStartTime = pSeekTable[iSeekTableIndex];
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[WrapFileReader.cpp %d] getSeekTable (inx:%d, Time:%d File:%d)", __LINE__, iSeekTableIndex, uiStartTime, iframeIndex->pFinfo[i].nTimeStamp);
				iSeekTableIndex++;
			}
			else
			{
				iSkipForEditbox++;
			}
		}
		uiStartTime++;
	}

	uiStartTick = nexSAL_GetTickCount() - uiStartTick;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getSeekTable End( TotalCount:%d) Elapse(%d)", __LINE__, iSeekTableIndex, uiStartTick);	

	if( iSeekTableIndex == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable is Failed(Count:%d)", __LINE__, iSeekTableIndex);	
		goto ERR_GETSEEKTABLE;
	}

	*pSeekTableCount = iSeekTableIndex;
	*ppSeekTable = pSeekTable;

	return NEXVIDEOEDITOR_ERROR_NONE;

ERR_GETSEEKTABLE:

	if( pSeekTable )
	{
		nexSAL_MemFree(pSeekTable);
		pSeekTable = NULL;
	}
	
	*pSeekTableCount = 0;
	*ppSeekTable = NULL;
	return iRet;
}

int CNexFileReader::getSeekTable(unsigned int uiStartTime, unsigned int uiEndTime, int iCount, int* pSeekTableCount, unsigned int** ppSeekTable)
{
	unsigned int uiStartTick = nexSAL_GetTickCount();
	if( pSeekTableCount == NULL || ppSeekTable == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable Arg failed", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( uiStartTime > m_uiTotalPlayTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable Arg failed", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	uiEndTime = uiEndTime == 0 ? m_uiTotalPlayTime : uiEndTime;

	if( uiStartTime > uiEndTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable Arg failed", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	if( m_pFFReader == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable reader handle failed", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

	int iRet = NEXVIDEOEDITOR_ERROR_NONE;
	nxFFRIFrameIdx* iframeIndex = NULL;
	unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;

	unsigned int uiElapsedTime = uiEndTime - uiStartTime;

	int iSeekTableIndex = 0;
	int iSkipForEditbox = 0;
	int iSeekTableCount = iCount > m_uiCountIframe ? m_uiCountIframe : iCount;

	int iSeekGap = uiElapsedTime / iSeekTableCount;
	iSeekGap = iSeekGap == 0 ? 1 : iSeekGap;

	unsigned int* pSeekTable = (unsigned int*)nexSAL_MemAlloc(sizeof(unsigned int)*iSeekTableCount);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable req count(%d) TotalCount(%d) iGap(%d)", __LINE__, iCount, m_uiCountIframe, iSeekGap);
	
	if( pSeekTable == NULL )
	{
		iRet = NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY;
		goto ERR_GETSEEKTABLE;
	}

	while( iSeekTableIndex + iSkipForEditbox < iSeekTableCount)
	{
		NEX_FF_RETURN eRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_IFRAME_INDEX, (NXINT64)uiStartTime, NULL, (void**)&iframeIndex);
		if( eRet != eNEX_FF_RET_SUCCESS ||iframeIndex ==  NULL )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable is Failed(Ret:%d)", __LINE__, eRet);	
			break;
		}

		if( iframeIndex->nCnt <= 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] NxFFR_GetExtInfo failed", __LINE__);	
			break;
		}

		if( iframeIndex->pFinfo[0].nTimeStamp >= uiVideoEditBoxTime )
		{
			if( iframeIndex->pFinfo[0].nTimeStamp - uiVideoEditBoxTime > uiEndTime )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getSeekTable end(%d %d)", __LINE__);
				break;
			}
		}
		
		if(iframeIndex->pFinfo[0].nTimeStamp >= uiVideoEditBoxTime)
		{
			pSeekTable[iSeekTableIndex] = iframeIndex->pFinfo[0].nTimeStamp + 1 - uiVideoEditBoxTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable info(Start:%d time:%d index:%d)", __LINE__, pSeekTable[iSeekTableIndex], uiStartTime, iSeekTableIndex);	
			iSeekTableIndex++;
		}
		else
		{
			iSkipForEditbox++;
		}

		uiStartTime += iSeekGap;
	}

	uiStartTick = nexSAL_GetTickCount() - uiStartTick;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getSeekTable End( TotalCount:%d) Elapse(%d)", __LINE__, iSeekTableIndex, uiStartTick);	

	if( iSeekTableIndex == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getSeekTable is Failed(Count:%d)", __LINE__, iSeekTableIndex);	
		goto ERR_GETSEEKTABLE;
	}

	*pSeekTableCount = iSeekTableIndex;
	*ppSeekTable = pSeekTable;

	return NEXVIDEOEDITOR_ERROR_NONE;

ERR_GETSEEKTABLE:

	if( pSeekTable )
	{
		nexSAL_MemFree(pSeekTable);
		pSeekTable = NULL;
	}
	
	*pSeekTableCount = 0;
	*ppSeekTable = NULL;
	return iRet;
}

int CNexFileReader::getVideoFrame(NXBOOL isEnhanced, NXBOOL bForceRead)
{
	if( m_pFFReader == NULL )
		return _SRC_GETFRAME_ERROR;

	if( m_isVideoExist && m_isVideoEnd )
	{
		return _SRC_GETFRAME_END;
	}

	NEX_FF_RETURN		eFileReaderRet	= eNEX_FF_RET_SUCCESS;
	NXFF_MEDIA_TYPE		eType			= NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO;
	int					iRet 			= 0;

	if( isEnhanced )
	{
		eType = NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO;
	}

	nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );

	if( m_isVideoChunkType )
	{
		NxFFReaderWBuffer* pWBuff = &m_BufVideo;
		if( bForceRead )
		{
			eFileReaderRet = getFrameFromVideoChunk(pWBuff, VIDEOCHUNKPARSER_GETFRAME_OPTION_FORCEGET);
		}
		else
		{
			eFileReaderRet = getFrameFromVideoChunk(pWBuff);
		}
	}
	else
	{
		//Nx_robin__110527 Next Buffer Pointer Set
		eFileReaderRet = nxFFReaderFrame(eType);//_Wrap_FFReaderFrame( pThis, pWBuffNext, iType );
	}

	switch( (int)eFileReaderRet )
	{
	case eNEX_FF_RET_SUCCESS:
		iRet = _SRC_GETFRAME_OK;
		break;

	case eNEX_FF_RET_EOF:
		//K.Lee add 20090109 [ ä�κ� end üũ
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Video End", __LINE__);
		m_isVideoEnd = 1;
		iRet = _SRC_GETFRAME_END;
		break;

	case eNEX_FF_RET_NOT_YET:
		iRet = _SRC_GETFRAME_NOFRAME;
		break;

	case _SRC_GETFRAME_OK_LOSTFRAME:
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 1, "[WrapFileReader.cpp %d] There are lost frames", __LINE__);
		break;

	default:
		iRet = _SRC_GETFRAME_ERROR;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] Read Error(%d)", __LINE__, eFileReaderRet );
		break;
	};
	nexSAL_MutexUnlock( m_hReaderMutex );

	// CTS�� ���� ���Ѵ�.
	if( m_isVideoChunkType )
	{
		m_uiVideoLastReadDTS = m_uiVideoChunkFrameDTS;
		m_uiVideoLastReadPTS = m_uiVideoChunkFramePTS;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] Video getVideoFrame CTS(%u) PTS(%u)", __LINE__, m_uiVideoLastReadDTS, m_uiVideoLastReadPTS );
	}
	else
	{
		m_uiVideoLastReadDTS =m_BufVideo.nDTS;
		m_uiVideoLastReadPTS =m_BufVideo.nCTS;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] Video getVideoFrame CTS(%u) PTS(%u)", __LINE__, m_uiVideoLastReadDTS, m_uiVideoLastReadPTS );
	}

	unsigned int uiVideoEditBoxTime = m_isEditBox ? m_uiVideoEditBoxTime : 0;
	if( m_uiVideoLastReadDTS > (m_uiTotalPlayTime - m_uiEndTrimTime) + 200 + uiVideoEditBoxTime )
	{
		m_isVideoEnd = 1;
		iRet = _SRC_GETFRAME_END;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] Video GetFrameEnd( Last:%d Total:%d EndTrim:%d)", 
			__LINE__, m_uiVideoLastReadDTS, m_uiTotalPlayTime, m_uiEndTrimTime );
	}

	return iRet; 
}

int CNexFileReader::getVideoFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize )
{
	if( m_pFFReader == NULL  )
	{
		*ppFrame = NULL;
		*pFrameSize = 0;
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	}

	NxFFReaderWBuffer*		pBuffer	= &m_BufVideo;

	if( pBuffer )
	{
		if( m_isVideoChunkType )
		{
			*ppFrame = m_pVideoChunkFrame;
			*pFrameSize = m_uiVideoChunkFrameLen;
		}
		else
		{
			*ppFrame = pBuffer->bfrstart+9;
			*pFrameSize = pBuffer->bytecnt-9;
		}

		if ( m_uiVideoObjectType == eNEX_CODEC_V_H264 )
		{
			NXUINT32 uFrameFormat = NexCodecUtil_CheckByteFormat(*ppFrame, *pFrameSize);
			if ( uFrameFormat == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW )
			{
				NXINT32	nNewDataLen=0;
				if ( m_uiFrameNALSize == 0 )
				{
					m_uiFrameNALSize = NexCodecUtil_GuessNalHeaderLengthSize(*ppFrame, *pFrameSize);
				}
				if (m_uIntermediateFrameSize < *pFrameSize)
				{
					if (m_pIntermediateFrame && m_uIntermediateFrameSize > 0)
					{
						SAFE_FREE(m_pIntermediateFrame);
						m_pIntermediateFrame = (NXUINT8*)0;
						m_uIntermediateFrameSize = 0;
					}

					m_pIntermediateFrame = (NXUINT8 *)nexSAL_MemAlloc(*pFrameSize + 100);
					if (!m_pIntermediateFrame)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] mem alloc failed\n", _FUNLINE_);
						return NEXCAL_ERROR_FAIL;
					}
					m_uIntermediateFrameSize = *pFrameSize;
				}

				nNewDataLen = NexCodecUtil_ConvertFormat(m_pIntermediateFrame, *pFrameSize + 100, *ppFrame, *pFrameSize, uFrameFormat, m_uiFrameNALSize);
				if (nNewDataLen == NEXCAL_ERROR_FAIL)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] NexCodecUtil_ConvertFormat failed\n", _FUNLINE_);
					*ppFrame = NULL;
					*pFrameSize = 0;
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_FLOW, 1, "[%s %d] format:%d, Size:%d->%d, Data[0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x -> 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x]\n", _FUNLINE_,
						uFrameFormat,
						*pFrameSize, nNewDataLen,
						*(*ppFrame+0), *(*ppFrame+1), *(*ppFrame+2), *(*ppFrame+3), *(*ppFrame+4), *(*ppFrame+5), *(m_pIntermediateFrame+0), *(m_pIntermediateFrame+1), *(m_pIntermediateFrame+2), *(m_pIntermediateFrame+3), *(m_pIntermediateFrame+4), *(m_pIntermediateFrame+5));
					*ppFrame = m_pIntermediateFrame;
					*pFrameSize = nNewDataLen;
				}
			}
		}
	}
	else
	{
		*ppFrame = NULL;
		*pFrameSize = 0;
		return NEXVIDEOEDITOR_ERROR_INVALID_INFO;
	}

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getVideoFrameDTS(NXUINT32* puDTS, NXBOOL bReadTime)
{
	if( puDTS == NULL ) return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	int iTmpDTS = (int)((m_uiVideoLastReadDTS + m_uiBaseTimeStamp) - m_uiStartTrimTime);
	if( iTmpDTS < 0 )
	{
		iTmpDTS = m_uiVideoTmpDTS;
		m_uiVideoTmpDTS += m_uiCorrectionTime;
	}
	else
	{
		iTmpDTS = iTmpDTS > m_uiVideoTmpDTS ? iTmpDTS : m_uiVideoTmpDTS;
		m_uiVideoTmpDTS += m_uiCorrectionTime;
	}

	*puDTS = bReadTime ? m_uiVideoLastReadDTS : iTmpDTS;
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[WrapFileReader.cpp %d] Get Frame Time Video(flag:%d DTS:%d, Read:%d)", __LINE__, bReadTime, iTmpDTS, m_uiVideoLastReadDTS);	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getVideoFramePTS(NXUINT32* puPTS, NXBOOL bReadTime)
{
	if( puPTS == NULL ) return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	int iTmpPTS = (int)((m_uiVideoLastReadPTS + m_uiBaseTimeStamp) - m_uiStartTrimTime);
	if( iTmpPTS < 0 )
	{
		iTmpPTS = m_uiVideoTmpPTS;
		m_uiVideoTmpPTS += m_uiCorrectionTime;
	}
	else
	{
		iTmpPTS = iTmpPTS > m_uiVideoTmpPTS ? iTmpPTS : m_uiVideoTmpPTS;
		m_uiVideoTmpPTS += m_uiCorrectionTime;
	}

	*puPTS = bReadTime ? m_uiVideoLastReadPTS : iTmpPTS;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[WrapFileReader.cpp %d] Get Frame Time Video(flag:%d PTS:%d read:%d)", __LINE__, bReadTime, iTmpPTS, m_uiVideoLastReadPTS);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getAudioFrame()
{
	if( m_pFFReader == NULL  )
		return _SRC_GETFRAME_ERROR;

	if( m_isAudioExist && m_isAudioEnd )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio End(%d %d)", __LINE__, m_isAudioExist , m_isAudioEnd);
		return _SRC_GETFRAME_END;
	}

	NEX_FF_RETURN		eFileReaderRet	= eNEX_FF_RET_SUCCESS;
	NxFFReaderWBuffer*	pWBuff			= &m_BufAudio;
	int 					iRet				= 0;

	if(m_bUsePrevFrame)
	{
		m_bUsePrevFrame = FALSE;
		return _SRC_GETFRAME_OK;
	}
	nexSAL_MutexLock( m_hReaderMutex, NEXSAL_INFINITE );

	switch( getAudioLostFrameStatus() )
	{
	case _SRC_GETAUDIOLOSTFRAME_STATUS_LOST :
		nexSAL_MutexUnlock( m_hReaderMutex );
		return _SRC_GETFRAME_OK_LOSTFRAME;
		
	case _SRC_GETAUDIOLOSTFRAME_STATUS_TAIL :
		nexSAL_MutexUnlock( m_hReaderMutex );
		return _SRC_GETFRAME_OK;
		
	case _SRC_GETAUDIOLOSTFRAME_STATUS_NO :     
		if( m_isAudioChunkType )
		{
			eFileReaderRet = getFrameFromAudioChunk( pWBuff );
		}
		else
		{
			//Nx_robin__110527 Next Buffer Pointer Set
			eFileReaderRet = nxFFReaderFrame(NXFF_MEDIA_TYPE_AUDIO);
			if( eFileReaderRet == eNEX_FF_RET_SUCCESS )
			{
				//Nx_robin__110527 ���������� ���� Frame�� ������ ���? ���۸� ��ü�ϰ�, Index ���� �����Ѵ�.
				int iCheckLostRet = checkAudioLostFrame( changeEndian( pWBuff->bfrstart+5 ) );
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio GetFrame() : Buf[%p], CTS[%u] LostCheck(%d)", __LINE__, pWBuff->bfr, changeEndian( pWBuff->bfrstart+5), iCheckLostRet);
				if( iCheckLostRet == _SRC_GETFRAME_OK_LOSTFRAME )
				{
					m_uiAudioLastReadCTS = changeEndian( pWBuff->bfrstart+5 );
					nexSAL_MutexUnlock( m_hReaderMutex );
					return _SRC_GETFRAME_OK_LOSTFRAME;
				}
			}
		}
		break;
	};

	switch( eFileReaderRet )
	{
	case eNEX_FF_RET_SUCCESS:
		iRet = _SRC_GETFRAME_OK;
		break;

	case eNEX_FF_RET_EOS:
	case eNEX_FF_RET_EOF:
		m_isAudioEnd = 1;
		iRet = _SRC_GETFRAME_END;
		//K.Lee add 20090109 [ ä�κ� end üũ
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio End", __LINE__);
		break;

	case eNEX_FF_RET_NOT_YET:
		iRet = _SRC_GETFRAME_NOFRAME;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Audio frame not yet", __LINE__);

		break;
	default:
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] Read Error(%d)", __LINE__, eFileReaderRet );
		iRet = _SRC_GETFRAME_ERROR;
		break;
	}

	nexSAL_MutexUnlock( m_hReaderMutex );

	if( m_isAudioChunkType )
	{
		m_uiAudioLastReadCTS = m_uiAudioChunkFrameDTS;	
	}
	else
	{
		m_uiAudioLastReadCTS = changeEndian( m_BufAudio.bfrstart+5 );
	}

	/*
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getAudioFrame TimeInfo(cur:%d, Total:%d, StartTrim:%d, EndTrim:%d)", __LINE__, 
		m_uiAudioLastReadCTS,  m_uiTotalPlayTime, m_uiStartTrimTime, m_uiEndTrimTime);
	
	// Lost Frame
	// fixAudioFrameCTS( m_uiAudioLastReadCTS, &m_uiAudioLastReadCTS );

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getAudioFrame TimeInfo(cur:%d, Total:%d, StartTrim:%d, EndTrim:%d)", __LINE__, 
		m_uiAudioLastReadCTS,  m_uiTotalPlayTime, m_uiStartTrimTime, m_uiEndTrimTime);
	*/


	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getAudioFrame TimeInfo(cur:%d, Total:%d, StartTrim:%d, EndTrim:%d)", __LINE__, 
	//	m_uiAudioLastReadCTS,  m_uiTotalPlayTime, m_uiStartTrimTime, m_uiEndTrimTime);
	unsigned int uiAudioEditBoxTime = m_isEditBox ? m_uiAudioEditBoxTime : 0;
	if( m_uiAudioLastReadCTS > (m_uiTotalPlayTime - m_uiEndTrimTime + 20) + uiAudioEditBoxTime )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getAudioFrame End(Content play end %d %d, %d, %d)", __LINE__, m_uiAudioLastReadCTS, m_uiBaseTimeStamp, m_uiStartTrimTime,  m_uiTotalPlayTime - m_uiEndTrimTime);
		m_isAudioEnd = 1;
		iRet = _SRC_GETFRAME_END;
	}	
	
	return iRet; 
}

int CNexFileReader::fixAudioFrameCTS( NXUINT32 uAudioCTSRead, NXUINT32* puCTSRet )
{
	if( m_iLostFrameCount )
		*puCTSRet = uAudioCTSRead - (NXUINT32)( (double)m_iLostFrameCount * m_dbAudioFrameInterval );
	else
		*puCTSRet = m_uiLastAudioCTS;

	return 0;
}

int CNexFileReader::getAudioFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize )
{
	if( m_pFFReader == NULL  )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	NxFFReaderWBuffer* pBuffer = &m_BufAudio;

	if( pBuffer )
	{
		if( m_isAudioChunkType )
		{
			*ppFrame = m_pAudioChunkFrame;
			*pFrameSize = m_uiAudioChunkFrameLen;
		}
		else
		{
			*ppFrame = pBuffer->bfrstart+9;
			*pFrameSize = pBuffer->bytecnt-9;
		}
	}
	else
	{
		*ppFrame = NULL;
		*pFrameSize = 0;
		return NEXVIDEOEDITOR_ERROR_INVALID_INFO;
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getAudioFrameCTS(NXINT64* puCTS )
{
	if( puCTS == NULL ) return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	unsigned int uiAudioEditBoxTime = m_isEditBox ? m_uiAudioEditBoxTime : 0;
	*puCTS = (NXINT64)(m_uiAudioLastReadCTS + m_uiBaseTimeStamp) - (NXINT64)m_uiStartTrimTime - (NXINT64)uiAudioEditBoxTime;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[WrapFileReader.cpp %d] Get Frame Time Audio(DTS:%lld)", __LINE__, *puCTS);
	
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNexFileReader::isSeekable()
{
	if( m_pFFReader == NULL  )
		return FALSE;

	if( NxFFR_SeekableCheck(m_pFFReader) == eNEX_FF_RET_SUCCESS )
	{
		//[shoh][20101001]If I-frame count is more than 2, then seekable.
		if( m_uiCountIframe >= 2 )
			return TRUE;
		else if( m_isVideoExist == FALSE ) //If Video doesn't exist, then seekable.
			return TRUE;
		else
			return FALSE;
	}
	//k.lee add 20110425 [ adif seek ����
	else
	{
		if( m_uiFileType == eNEX_FF_ADIFAAC || m_uiFileType == eNEX_FF_QCELP )
		{
			return TRUE;
		}
		else
		{
			return FALSE;	
		}
	}
	//k.lee add ]

	return FALSE;
}

NXBOOL CNexFileReader::isSeekable(unsigned int uiPosition)
{
	if( m_pFFReader == NULL  )
		return FALSE;

	if( uiPosition == 0 )
		return TRUE;

	if( NxFFR_SeekableCheck(m_pFFReader) != eNEX_FF_RET_SUCCESS )
		return FALSE;

	//[shoh][20101001]If I-frame count is more than 2, then seekable.
	if( m_uiCountIframe >= 2 )
		return TRUE;
	else if( m_isVideoExist == FALSE ) //If Video doesn't exist, then seekable.
		return TRUE;

	return FALSE;
}


NXBOOL CNexFileReader::isAudioExist()
{
	return m_isAudioExist;
}

NXBOOL CNexFileReader::isVideoExist()
{
	return m_isVideoExist;
}

NXBOOL CNexFileReader::isTextExist()
{
	return m_isTextExist;
}

int CNexFileReader::isIntraRA (NXBOOL *pIsIntraRA )
{
	if( m_pFFReader == NULL || pIsIntraRA == NULL )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;

	*pIsIntraRA = FALSE;
	if( m_pFFReader->nSeekRestriction & NXFF_RA_RESTRICTION_LOOK_IFRAME )
		*pIsIntraRA = TRUE;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

unsigned int CNexFileReader::getIDRFrameCount()
{
	return m_uiCountIframe;
}

NXUINT32 CNexFileReader::parseFrameFormat(NXUINT8 *pFrame, NXUINT32 uFrameSize)
{
	NXUINT32	uFrameFormat=NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANY;

#ifdef DISABLE_MIXED_RAW_AND_ANNEXB_NAL
	uFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameSize);
#else
	//For NESA-843, NESA-1159, CHC-177
	if((uFrameFormat = NexCodecUtil_CheckByteFormat(pFrame, uFrameSize)) == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW)
	{
		int iCodeLen = 0;
		NexCodecUtil_FindAnnexBStartCode(pFrame, 4, uFrameSize > 200?200:uFrameSize, &iCodeLen);

		if(iCodeLen == 4)
		{
			uFrameFormat = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
		}
	}
#endif

	return uFrameFormat;
}

unsigned int CNexFileReader::getFrameFormat()
{
	return m_uiFrameFormat;
}

int CNexFileReader::randomAccess(NXUINT32 uTargetCTS, NXUINT32* puResultCTS, NEXVIDEPEDITORRandomAccessMode eRAMode, NXBOOL bVideo, NXBOOL bAudio, NXBOOL bText )
{
	if( m_pFFReader == NULL  )
		return NEXVIDEOEDITOR_ERROR_FILEREADER_FAILED;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight )
{
	if( m_pFFReader == NULL || pusWidth == NULL || pusHeight == NULL)
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	/*
	** RTSP Streaming/DVBH/CMMB����� SDP�� Width/Height ������ �ŷ��� �� ��� DSI�� ���� Parsing�ؼ� Resolution�� ���?
	*/
	if ( m_uiVideoObjectType == eNEX_CODEC_V_MPEG4V || m_uiVideoObjectType == eNEX_CODEC_V_H264 || m_uiVideoObjectType == eNEX_CODEC_V_HEVC )
	{
		NXUINT8 *pBaseDSI;
		NXUINT32 uBaseDSISize;      
		NXUINT32 uRet;

		getDSI( NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO, &pBaseDSI, &uBaseDSISize );
		if( pBaseDSI != NULL )
		{
			if( m_uiVideoObjectType == eNEX_CODEC_V_MPEG4V )
			{
				NEXCODECUTIL_MPEG4V_DSI_INFO DSI_Info;
				uRet = NexCodecUtil_MPEG4V_GetDSIInfo((NEX_CODEC_TYPE)m_uiVideoObjectType, (char *)pBaseDSI, uBaseDSISize, &DSI_Info);

				if( !uRet )
				{
					if ( *pusWidth != DSI_Info.usWidth || *pusHeight != DSI_Info.usHeight )
					{
						*pusWidth = DSI_Info.usWidth;
						*pusHeight = DSI_Info.usHeight;
					}
				}
			}
			else if( m_uiVideoObjectType == eNEX_CODEC_V_H264 )
			{
				NEXCODECUTIL_SPS_INFO SPS_Info;
				// uRet = nexCALTools_AVC_GetSPSInfo((char *)pBaseDSI, uBaseDSISize, &SPS_Info, getFrameFormat());
#ifdef FOR_TEST_AVI_FORMAT
				uRet = NexCodecUtil_AVC_GetSPSInfo((char *)pBaseDSI, uBaseDSISize, &SPS_Info, m_uiFrameFormat);
#else
				uRet = NexCodecUtil_AVC_GetSPSInfo((char *)pBaseDSI, uBaseDSISize, &SPS_Info, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW);
#endif
				if ( !uRet )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] spsppsInfo(%d %d, %d %d)", __LINE__, *pusWidth, *pusHeight, SPS_Info.usWidth, SPS_Info.usHeight);
					if ( *pusWidth != SPS_Info.usWidth || *pusHeight != SPS_Info.usHeight )
					{
						*pusWidth = SPS_Info.usWidth;
						*pusHeight = SPS_Info.usHeight;
					}
				}
				else
				{
					// nexSAL_MemDump(pBaseDSI, uBaseDSISize);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] get width height failed(%d)", __LINE__, uRet);
				}
			}
			else if( m_uiVideoObjectType == eNEX_CODEC_V_HEVC )
			{
				NEXCODECUTIL_SPS_INFO SPS_Info;
				// uRet = nexCALTools_AVC_GetSPSInfo((char *)pBaseDSI, uBaseDSISize, &SPS_Info, getFrameFormat());
#ifdef FOR_TEST_AVI_FORMAT
				uRet = NexCodecUtil_HEVC_GetSPSInfo((char *)pBaseDSI, uBaseDSISize, &SPS_Info, m_uiFrameFormat);
#else
				uRet = NexCodecUtil_HEVC_GetSPSInfo((char *)pBaseDSI, uBaseDSISize, &SPS_Info, NexCodecUtil_CheckByteFormat( pBaseDSI, uBaseDSISize));
#endif
				if ( !uRet )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] spsppsInfo(%d %d, %d %d)", __LINE__, *pusWidth, *pusHeight, SPS_Info.usWidth, SPS_Info.usHeight);
					if ( *pusWidth != SPS_Info.usWidth || *pusHeight != SPS_Info.usHeight )
					{
						*pusWidth = SPS_Info.usWidth;
						*pusHeight = SPS_Info.usHeight;
					}
				}
				else
				{
					// nexSAL_MemDump(pBaseDSI, uBaseDSISize);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] get width height failed(%d)", __LINE__, uRet);
				}
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getVideoResolution(W:%d H:%d)", __LINE__, *pusWidth,  *pusHeight);
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getVideoResolution error", __LINE__);
	return NEXVIDEOEDITOR_ERROR_GENERAL;
}

int CNexFileReader::getDisplayVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight)
{
	if (pusWidth && pusHeight)
	{
		*pusWidth = m_uiDisplayWidth;
		*pusHeight = m_uiDisplayHeight;
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getDisplayVideoResolution error", __LINE__);
	return NEXVIDEOEDITOR_ERROR_GENERAL;
}

int CNexFileReader::getAudioBitrate(NXUINT32* puAudioBitrate )
{
	if( m_pFFReader == NULL || puAudioBitrate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puAudioBitrate = m_uiAudioBitRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getVideoBitrate(NXUINT32* puVideoBitrate )
{
	if( m_pFFReader == NULL || puVideoBitrate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puVideoBitrate = m_uiVideoBitRate;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getTotalPlayTime(NXUINT32* puTotalPlayTime )
{
	if( m_pFFReader == NULL || puTotalPlayTime == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puTotalPlayTime = m_uiTotalPlayTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

/*
int CNexFileReader::getAudioSampleRate()
{
	return m_uiSamplingRate;
}

int CNexFileReader::getAudioChannels()
{
	return m_uiNumOfChannels;
}

int CNexFileReader::getAudioSampleForChannel()
{
	return m_uiNumOfSamplesPerChannel;
}
*/

int CNexFileReader::getSamplingRate(NXUINT32* puSamplingRate )
{
	if( m_pFFReader == NULL || puSamplingRate == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	NEXVIDEOEDITOR_ERROR enRet = NEXVIDEOEDITOR_ERROR_NONE;

	/*
	if( _SRC_Common_GetSamplingRate( pThis, puSamplingRate ) == 0 )
		return NEXVIDEOEDITOR_ERROR_NONE;
	*/

	switch( m_uiAudioObjectType )
	{
	case eNEX_CODEC_A_AAC :
	case eNEX_CODEC_A_MP3 :
		if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
		{
			if( m_pFFReader->hAudioTrackInfo[0].pDSI )
			{
				*puSamplingRate = (m_pFFReader->hAudioTrackInfo[0].pDSI[0]<<8) | (m_pFFReader->hAudioTrackInfo[0].pDSI[1]);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] LP_GetSamplingRate pAudioDSIs is NULL", __LINE__ );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] LP_GetSamplingRate pAudioDSIs:%p size:%d",
					__LINE__, m_pFFReader->hAudioTrackInfo[0].pDSI, m_pFFReader->hAudioTrackInfo[0].nDSI  );
				*puSamplingRate = 0;
			}
		}
		else if( m_uiAudioObjectType == eNEX_CODEC_A_AAC )
		{
			/*
			NXUINT32 samplerate = 0;
			NXUINT32 channels = 0;
			NXINT32 bSBR = 0;			

			NexCodecUtil_AAC_ParseDSI( m_pFFReader->hAudioTrackInfo[0].pDSI, &samplerate, &channels, &bSBR);
			 */

			*puSamplingRate = m_uiSamplingRate;
			// *puSamplingRate = bSBR ? samplerate * 2 : samplerate;

		}
		else
		{
			enRet = NEXVIDEOEDITOR_ERROR_INVALID_INFO;
		}
		break;

	case eNEX_CODEC_A_FLAC:
	case eNEX_CODEC_A_PCM_S16LE:
	case eNEX_CODEC_A_PCM_S16BE:		
	case eNEX_CODEC_A_PCM_RAW:
	case eNEX_CODEC_A_PCM_FL32LE:
	case eNEX_CODEC_A_PCM_FL32BE:
	case eNEX_CODEC_A_PCM_FL64LE:
	case eNEX_CODEC_A_PCM_FL64BE:
	case eNEX_CODEC_A_PCM_IN24LE:
	case eNEX_CODEC_A_PCM_IN24BE:
	case eNEX_CODEC_A_PCM_IN32LE:
	case eNEX_CODEC_A_PCM_IN32BE:		
	case eNEX_CODEC_A_PCM_LPCMLE:
	case eNEX_CODEC_A_PCM_LPCMBE:
		*puSamplingRate = m_uiSamplingRate;		
		break;
		
	case eNEX_CODEC_A_AMR:
		if(m_uiSamplingRate)
			*puSamplingRate = m_uiSamplingRate;
		else
			*puSamplingRate = 8000;
		break;
	case eNEX_CODEC_A_AMRWB:
		if(m_uiSamplingRate)
			*puSamplingRate = m_uiSamplingRate;
		else
			*puSamplingRate = 16000;
		break;
		
	default:
		enRet = NEXVIDEOEDITOR_ERROR_INVALID_INFO;
		break;
	}

	return enRet;
}

int CNexFileReader::getNumberOfChannel(NXUINT32* puNumOfChannels )
{
	if( m_pFFReader == NULL || puNumOfChannels == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	//Nx_robin__110523 Video Only�� ���?����ó��
	if ( !m_isAudioExist )
	{
		*puNumOfChannels = 0;
		return NEXVIDEOEDITOR_ERROR_NONE;
	}

	/*
	if( _SRC_Common_GetNumberOfChannel( pThis, puNumOfChannels ) == 0 )
		return NEXVIDEOEDITOR_ERROR_NONE;
	*/

	if( m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
	{
		if(m_pFFReader->hAudioTrackInfo[0].pDSI)
			*puNumOfChannels = m_pFFReader->hAudioTrackInfo[0].pDSI[2];
		else
			*puNumOfChannels = 0;
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	else if( m_uiAudioObjectType == eNEX_CODEC_A_AAC )
	{
		NXUINT32 samplerate = 0;
		NXUINT32 channels = 0;
		NXINT32 bSBR = 0;			
		
		NexCodecUtil_AAC_ParseDSI( m_pFFReader->hAudioTrackInfo[0].pDSI, m_pFFReader->hAudioTrackInfo[0].nDSI, &samplerate, &channels, &bSBR);
		*puNumOfChannels = channels;
	}	
	else if(IS_PCM_Codec(m_uiAudioObjectType) || m_uiAudioObjectType == eNEX_CODEC_A_FLAC)
	{
		*puNumOfChannels = m_uiNumOfChannels;	
	}
	else if(m_uiAudioObjectType == eNEX_CODEC_A_AMR || m_uiAudioObjectType == eNEX_CODEC_A_AMRWB)
	{
		if(m_uiNumOfChannels)
			*puNumOfChannels = m_uiNumOfChannels;
		else
			*puNumOfChannels = 1;
	}

	return NEXVIDEOEDITOR_ERROR_INVALID_INFO;
}

int CNexFileReader::getSamplesPerChannel(NXUINT32* piNumOfSamplesPerChannel )
{
	if( m_pFFReader == NULL || piNumOfSamplesPerChannel == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	/*
	if( _SRC_Common_GetSamplesPerChannel( pThis, piNumOfSamplesPerChannel ) == 0 )
		return NEXVIDEOEDITOR_ERROR_NONE;
	*/

	if(  m_uiAudioObjectType == eNEX_CODEC_A_MP3 )
	{
		if(m_pFFReader->hAudioTrackInfo[0].pDSI)
			*piNumOfSamplesPerChannel = ( m_pFFReader->hAudioTrackInfo[0].pDSI[3]<<8 ) | ( m_pFFReader->hAudioTrackInfo[0].pDSI[4] );
		else
			*piNumOfSamplesPerChannel = 0;
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	else if(  m_uiAudioObjectType == eNEX_CODEC_A_AAC )
	{
		// set default value.
		*piNumOfSamplesPerChannel = 1024;
		return NEXVIDEOEDITOR_ERROR_NONE;
	}
	else if(IS_PCM_Codec(m_uiAudioObjectType) || m_uiAudioObjectType == eNEX_CODEC_A_FLAC)
	{
		*piNumOfSamplesPerChannel = m_uiNumOfSamplesPerChannel;	
	}
	else if(m_uiAudioObjectType == eNEX_CODEC_A_AMR || m_uiAudioObjectType == eNEX_CODEC_A_AMRWB)
	{
		*piNumOfSamplesPerChannel = m_uiNumOfSamplesPerChannel;	
	}


	return NEXVIDEOEDITOR_ERROR_INVALID_INFO;
}

int CNexFileReader::getDSINALHeaderLength()
{
	return m_uiDSINALSize;
}

int CNexFileReader::getFrameNALHeaderLength()
{
	return m_uiFrameNALSize;
}

int CNexFileReader::getH264ProfileLevelID(unsigned int* puH264ProfileLevelID )
{
	if( m_pFFReader == NULL || puH264ProfileLevelID == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264ProfileLevelID = m_uiH264ProfileID;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getH264Level(unsigned int* puH264Level )
{
	if( m_pFFReader == NULL || puH264Level == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264Level = m_uiH264Level;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getVideoH264Interlaced(NXBOOL* puH264Interlaced )
{
	if( m_pFFReader == NULL || puH264Interlaced == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	*puH264Interlaced = m_isH264Interlaced;

	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getWAVEFormat (unsigned int *pFormatTag, unsigned int* pBitsPerSample, unsigned int *pBlockAlign, unsigned int *pAvgBytesPerSec, unsigned int *pSamplesPerBlock, unsigned int *pEncodeOpt, unsigned int *pExtraSize, unsigned char **ppExtraData )
{
	if( m_pFFReader == NULL || pFormatTag == NULL || pBitsPerSample == NULL || pBlockAlign == NULL || 
		pAvgBytesPerSec == NULL || pSamplesPerBlock == NULL || pEncodeOpt == NULL || pExtraSize == NULL || ppExtraData == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	//Nx_robin__110523 Video Only�� ���?����ó��
	if ( !m_isAudioExist ) return NEXVIDEOEDITOR_ERROR_INVALID_INFO;

	NEX_WAVEFORMATEX*pWaveFormat = NxFFR_GetWAVEFormatEX(m_pFFReader, 0);

	if(pWaveFormat)
	{
		*pFormatTag = pWaveFormat->nCodecID;
		*pBitsPerSample = pWaveFormat->nBitPerSample;
		*pBlockAlign = pWaveFormat->nBlockAlignment;
		*pAvgBytesPerSec = pWaveFormat->nAvrByte;
		*pExtraSize = pWaveFormat->nExtraDataLen;
		*ppExtraData = pWaveFormat->pExtraData;

		if((pWaveFormat->nCodecID == 352) && (pWaveFormat->nExtraDataLen >= 4))
		{
			*pSamplesPerBlock = (unsigned int)(*((NXUINT16*)pWaveFormat->pExtraData));
			*pEncodeOpt = (unsigned int)(*((NXUINT16*)(pWaveFormat->pExtraData+2)));
		}
		else if((pWaveFormat->nCodecID == 353) && (pWaveFormat->nExtraDataLen >= 6))
		{
			*pSamplesPerBlock = (unsigned int)(*((NXUINT32*)pWaveFormat->pExtraData));
			*pEncodeOpt = (unsigned int)(*((NXUINT16*)(pWaveFormat->pExtraData+4)));
		}
		/*
		else if ((pWaveFormat->nCodecID == 354 && (pWaveFormat->nExtraDataLen >= 18))
		{
+        s->decode_flags     = AV_RL16(edata_ptr+14);
+        s->dwChannelMask    = AV_RL32(edata_ptr+2);
+        s->sample_bit_depth = AV_RL16(edata_ptr);            
		}
		*/
		else
		{
			*pSamplesPerBlock = 0;
			*pEncodeOpt = 0;
		}
	}
	else
	{
		*pFormatTag = 0;
		*pBitsPerSample = 0;
		*pBlockAlign = 0;
		*pAvgBytesPerSec = 0;
		*pSamplesPerBlock = 0;
		*pEncodeOpt = 0;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] LP_GetWAVEFormat End : FormatTag = %d, BitPerSample =%d, BlockAlignment = %d, AvrByte = %d, SamplesPerBlock = %d, EncodeOpt = %d", __LINE__,
					*pFormatTag, *pBitsPerSample, *pBlockAlign, *pAvgBytesPerSec, *pSamplesPerBlock, *pEncodeOpt);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getBitmapInfoHeader(unsigned char **pExtraData, unsigned int *pExtraDataLen)
{
	if( m_pFFReader == NULL || pExtraData == NULL || pExtraDataLen == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	NEX_BITMAPINFOHEADER* pBitmapInfo = NxFFR_GetBITMAPINFOHEADER(m_pFFReader, 0);
	if( pBitmapInfo )
	{
		m_pBitmapInfoEx = (void*)pBitmapInfo;
	}
	else
	{
		m_pBitmapInfoEx = NULL;
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getAudioLostFrameStatus()
{
	int iRet = _SRC_GETAUDIOLOSTFRAME_STATUS_NO;

	if( m_iLostFrameCount > 0 )
	{
		if( m_iLostFrameCount == 1 )
			iRet = _SRC_GETAUDIOLOSTFRAME_STATUS_TAIL;
		else
			iRet = _SRC_GETAUDIOLOSTFRAME_STATUS_LOST;

		m_iLostFrameCount--;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d]!!! Lost Frame Remained count : %d", __LINE__, m_iLostFrameCount );
	}

	return iRet;
}

int CNexFileReader::checkAudioLostFrame(NXUINT32 uAudioCTSRead )
{
	int			iRet		= 0;
	NXUINT32	uCTS	= uAudioCTSRead;	

	if( m_isLostFrameCheck )
	{
		m_iLostFrameCount = 0;

		if( uAudioCTSRead == m_uiLastAudioCTSRead )
		{
			uCTS = m_uiLastAudioCTS + m_uiAudioFrameInterval;
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "!!! SAMERTP CTS %d", uCTS );
		}
		else if( ( uCTS > m_uiLastAudioCTS ) && ( uCTS - m_uiLastAudioCTS ) > (m_uiMaxAudioFrameInterval+1) )  // JDKIM 2006/11/21
		{
			m_iLostFrameCount = (NXUINT32)( (double)(uCTS - m_uiLastAudioCTS) / m_dbAudioFrameInterval + .1 ) - 1;
			m_iLostFrameCount = (m_iLostFrameCount==1)? 0 : m_iLostFrameCount; //[shoh][2013.08.08] Ignore one lost frame.

			if(m_iLostFrameCount > 0)			// JDKIM 2010/09/07 
			{
				iRet = _SRC_GETFRAME_OK_LOSTFRAME;

				//nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "!!! Lost Frame Detected. count : %d", pSource->m_uiLostFrameCount );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] Lost Frame Detected(Audio Packet Loss)!!! Previous CTS[%u], Current CTS[%u], LostFrame[%u], FrameInterval[%u] MaxFrameInterval[%d] dbInterval[%f]",
								__LINE__,
								m_uiLastAudioCTS,
								uCTS,
								m_iLostFrameCount,
								m_uiAudioFrameInterval,
								m_uiMaxAudioFrameInterval,
								m_dbAudioFrameInterval);
			}
			// JDKIM 2010/09/07
			else
			{
				iRet = _SRC_GETFRAME_OK;
			}
			// JDKIM : end
		}
		// JDKIM 2006/11/21
		else if( uCTS < m_uiLastAudioCTS )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] CTS value is abnormal. [%d, %d]", __LINE__, uCTS, m_uiLastAudioCTS);
			uCTS = m_uiLastAudioCTS;
		}
		// JDKIM : end
	}

	m_uiLastAudioCTS = uCTS;
	m_uiLastAudioCTSRead = uAudioCTSRead;

	return iRet;
}


NXBOOL CNexFileReader::isAudioChunkType( NXUINT32 a_uCodecType, NXUINT32 a_uFourCC, NXUINT32 a_uFFType)
{
	NXUINT32 uFourCC = a_uFourCC;
	NXBOOL bChunkType = FALSE;
	
	if( (a_uFFType == eNEX_FF_AVI) || (a_uFFType == eNEX_FF_MATROSKA) || (a_uFFType == eNEX_FF_RMFF)
	   || (a_uFFType == eNEX_FF_ASF) || (a_uFFType == eNEX_FF_OGG) || (a_uFFType == eNEX_FF_WAVE)
	   || (a_uFFType == eNEX_FF_FLV) || (a_uFFType == eNEX_FF_MP4) )
	{
		switch(a_uCodecType)
		{
			case eNEX_CODEC_A_DTS :
			case eNEX_CODEC_A_AC3 :
			case eNEX_CODEC_A_MP3 :
			case eNEX_CODEC_A_MP2 :
			case eNEX_CODEC_A_PCM_S16LE :
			case eNEX_CODEC_A_PCM_S16BE :				
			case eNEX_CODEC_A_PCM_RAW:
			case eNEX_CODEC_A_PCM_FL32LE:
			case eNEX_CODEC_A_PCM_FL32BE:
			case eNEX_CODEC_A_PCM_FL64LE:
			case eNEX_CODEC_A_PCM_FL64BE:
			case eNEX_CODEC_A_PCM_IN24LE:
			case eNEX_CODEC_A_PCM_IN24BE:
			case eNEX_CODEC_A_PCM_IN32LE:
			case eNEX_CODEC_A_PCM_IN32BE:				
			case eNEX_CODEC_A_ADPCM_MS :
			case eNEX_CODEC_A_ADPCM_IMA_WAV :
			case eNEX_CODEC_A_PCM_LPCMLE:
			case eNEX_CODEC_A_PCM_LPCMBE:
				bChunkType = TRUE;
				break;

			default:
				bChunkType = FALSE;
				break;
		}
	}
	else if(a_uFFType == eNEX_FF_MPEG_TS)
	{
		switch(a_uCodecType)
		{
			case eNEX_CODEC_A_AAC :
			case eNEX_CODEC_A_AACPLUS :
			case eNEX_CODEC_A_AACPLUSV2 :
			case eNEX_CODEC_A_MP2:
			case eNEX_CODEC_A_MP3 :
					bChunkType = TRUE;
				break;

			default:
				bChunkType = FALSE;
				break;
		}
	}

	if (a_uCodecType == eNEX_CODEC_A_AMR || a_uCodecType == eNEX_CODEC_A_AMRWB 
		|| a_uCodecType == eNEX_CODEC_A_QCELP || a_uCodecType == eNEX_CODEC_A_EVRC)
	{
		bChunkType = TRUE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] Audio Chunk type(%d)", __LINE__, bChunkType);
	return bChunkType;
}

NXBOOL CNexFileReader::isVideoChunkType(NXUINT32 a_uCodecType, NXUINT32 a_uFourCC, NXUINT32 a_uFFType)
{
#if 0    // to use NxFFReader instread of ChunkParser
	NXUINT32 uFourCC = a_uFourCC;
#if 0	
	switch(a_uCodecType)
	{
		case NXFF_CODEC_V_H264:
		case NXFF_CODEC_V_MPEG2V:
		case NXFF_CODEC_V_MPEG4:
			if((a_uFFType == NXFF_MPEG_TS_FF) || (a_uFFType == NXFF_MPEG_PS_FF))
			{
				return TRUE;
			}

		default:
			return FALSE;
			
	}
#else
	switch(a_uCodecType)
	{
		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
			if((a_uFFType == eNEX_FF_MPEG_TS) || (a_uFFType == eNEX_FF_MPEG_PS))// || (a_uFFType == eNEX_FF_MP4))
			{
				return TRUE;
			}

			if(a_uFFType == eNEX_FF_MP4)
			{
				if(m_isCTSBox == 0)
				return TRUE;
			}

		case eNEX_CODEC_V_MPEG2V:
		case eNEX_CODEC_V_MPEG4V:
			if((a_uFFType == eNEX_FF_MPEG_TS) || (a_uFFType == eNEX_FF_MPEG_PS))
			{
				return TRUE;
			}

		default:
			return FALSE;
	};
#endif
#endif
	return FALSE;
}

NEX_FF_RETURN CNexFileReader::getFrameFromAudioChunk(NxFFReaderWBuffer *a_pWBuff)
{
	NXUINT8*	pFrameData		= NULL;
	NXUINT32	uFrameSize		= 0;
	NXUINT32	uFrameCTS		= 0;
	NXUINT32	uFramePTS		= 0;
	NXUINT8*	pChunkData		= NULL;
	NXUINT32	uChunkLen		= 0;
	NXUINT32	uChunkCTS		= 0;
	NXUINT32	uNextIdx		= 0;
	NXINT32		eChunkRet		= 0;


	NxFFReaderWBuffer *pWBuffNext;
	
	if( m_hAudioChunkInfo == NULL )
	{
		return eNEX_FF_RET_FAIL;
	}

	while(1)
	{
		eChunkRet = AudioChunkParser_GetFrame32(m_hAudioChunkInfo, AUDIOCHUNKPARSER_GETFRAME_OPTION_NORMAL, &pFrameData, &uFrameSize, &uFrameCTS, &uFramePTS);

		if( eChunkRet < 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp%4d][_LP_GetFrameFromAudioChunk] GetFrame failed! (nRet: %d)", __LINE__, eChunkRet);
		}
		else
		{
			if(pFrameData != NULL && uFrameSize > 0)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp%4d][_LP_GetFrameFromAudioChunk] GetFrame OK! (Frame: %p, Len: %4d, CTS: %5d, PTS: %5d)", 
																__LINE__, pFrameData, uFrameSize, uFrameCTS, uFramePTS);
				m_pAudioChunkFrame		= pFrameData;
				m_uiAudioChunkFrameLen	= uFrameSize;
				m_uiAudioChunkFrameDTS	= uFrameCTS;
				m_uiAudioChunkFramePTS	= uFramePTS;
				return eNEX_FF_RET_SUCCESS;
			}
		}
		//Nx_robin__110527 Next Buffer Pointer Set

		pWBuffNext = &m_BufAudio;


		NEX_FF_RETURN eFFRet = nxFFReaderFrame((NXFF_MEDIA_TYPE)NXFF_MEDIA_TYPE_AUDIO);
		if ( eFFRet == eNEX_FF_RET_SUCCESS )
		{
			//Nx_robin__110527 ���������� ���� Frame�� ������ ���? ���۸� ��ü�ϰ�, Index ���� �����Ѵ�.
			a_pWBuff = pWBuffNext;
		
			pChunkData = (NXUINT8*)(a_pWBuff->bfrstart + 9);
			uChunkLen = (NXUINT32)(a_pWBuff->bytecnt - 9);
			uChunkCTS = (NXUINT32)changeEndian(a_pWBuff->bfrstart+1);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp %d] Audio GetFrame() : Buf[%p | %7u], CTS[%7u] ", __LINE__, a_pWBuff->bfr, uChunkLen, uChunkCTS);
		}
		else 
		{
			if( eFFRet == eNEX_FF_RET_EOF )
			{
				eChunkRet = AudioChunkParser_GetFrame32(m_hAudioChunkInfo, AUDIOCHUNKPARSER_GETFRAME_OPTION_FORCEGET, &pFrameData, &uFrameSize, &uFrameCTS, &uFramePTS);
				if(eChunkRet < 0 || (pFrameData == NULL && uFrameSize == 0))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp%4d][_LP_GetFrameFromAudioChunk] Audio End Frame!", __LINE__);					
					return eNEX_FF_RET_EOF;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp%4d][_LP_GetFrameFromAudioChunk] EOF but Audio Frame Remained! (Frame: %p, Len: %4d, CTS: %5d, PTS: %5d)", 
																__LINE__, pFrameData, uFrameSize, uFrameCTS, uFramePTS);
				m_pAudioChunkFrame		= pFrameData;
				m_uiAudioChunkFrameLen	= uFrameSize;
				m_uiAudioChunkFrameDTS	= uFrameCTS;
				m_uiAudioChunkFramePTS	= uFramePTS;
				return eNEX_FF_RET_SUCCESS;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp%4d][_LP_GetFrameFromAudioChunk] m_pGetFrame Failed(%d)!", __LINE__, eFFRet);
				return eNEX_FF_RET_FILE_API_ERROR;
			}
		}

		if( pChunkData != NULL && (uChunkLen > 0) )
		{
			eChunkRet = AudioChunkParser_PutFrame32(m_hAudioChunkInfo, pChunkData, uChunkLen, uChunkCTS, uChunkCTS);
			if (eChunkRet < 0)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp%4d][_LP_GetFrameFromAudioChunk] AudioChunkParser_PutFrame Failed(%d)", __LINE__, eChunkRet);
				return eNEX_FF_RET_FILE_API_ERROR;
			}			
		}
	}
}

NEX_FF_RETURN CNexFileReader::getFrameFromVideoChunk(NxFFReaderWBuffer *a_pWBuff, int iChunkParserGetOption)
{
	NXUINT8*						pFrameData		= NULL;
	NXUINT32						uFrameSize		= 0;
	NXINT32							iChunkRet		= 0;
	NXUINT8*						pChunkData		= NULL;
	NXUINT32						uChunkLen		= 0;
	NXUINT32						uChunkDTS		= 0;
	NXUINT32						uChunkPTS		= 0;
	NXUINT32						uNextIdx		= 0;
	NXBOOL							bIframe			= FALSE;
	NxFFReaderWBuffer*				pWBuffNext		= NULL;
	NEX_FF_RETURN					eFrameRet		= eNEX_FF_RET_SUCCESS;

	// Add this flag for codec fail problem at clip end position
	NXBOOL							bEndOfStream		= FALSE;

	if( m_hVideoChunkInfo == NULL )
	{
		return eNEX_FF_RET_FAIL;
	}

	while(1)
	{
		if( bEndOfStream )
		{
			iChunkParserGetOption = VIDEOCHUNKPARSER_GETFRAME_OPTION_FORCEGET;
		}
		iChunkRet = VideoChunkParser_GetFrame32(	m_hVideoChunkInfo, 
													iChunkParserGetOption, 
													&pFrameData, 
													&uFrameSize, 
													&uChunkDTS, 
													&uChunkPTS, 
													&bIframe);

		if( iChunkRet < 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d][_LP_GetFrameFromVideoChunk] GetFrame failed! (nRet: %d)", __LINE__, iChunkRet);
		}
		else
		{
			if(pFrameData != NULL && uFrameSize > 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp %d][_LP_GetFrameFromVideoChunk] GetFrame OK! (Frame: %p, CTS: %7u, PTS: %7u, Len: %4d)", 
															__LINE__, pFrameData, uChunkDTS, uChunkPTS, uFrameSize);

				m_pVideoChunkFrame		= pFrameData;
				m_uiVideoChunkFrameLen	= uFrameSize;
				m_uiVideoChunkFrameDTS	= uChunkDTS;
				m_uiVideoChunkFramePTS	= uChunkPTS;
				// nexSAL_MemDump(m_pVideoChunkFrame, 1000);
				return eNEX_FF_RET_SUCCESS;
			}
		}

		if( bEndOfStream )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d][_LP_GetFrameFromVideoChunk] return Video End Frame !", __LINE__);
			return eNEX_FF_RET_EOF;
		}

		//Nx_robin__110527 Next Buffer Pointer Set
		pWBuffNext = &m_BufVideo;

		eFrameRet = nxFFReaderFrame(NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO);
		if ( eFrameRet == eNEX_FF_RET_SUCCESS )
		{
			//Nx_robin__110527 ���������� ���� Frame�� ������ ���? ���۸� ��ü�ϰ�, Index ���� �����Ѵ�.
			a_pWBuff = pWBuffNext;
		
			pChunkData = (NXUINT8*)(a_pWBuff->bfrstart + 9);
			uChunkLen = (NXUINT32)(a_pWBuff->bytecnt - 9);
			uChunkDTS = (NXUINT32)a_pWBuff->nDTS;
			uChunkPTS = (NXUINT32)a_pWBuff->nCTS;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp %d] Video GetFrame() : Buf[%p | %7u], DTS[%7u] PTS[%7u]", __LINE__, a_pWBuff->bfr, uChunkLen, uChunkDTS, uChunkPTS);
			// nexSAL_MemDump(pChunkData, 1000);
		}
		else
		{
			if( eFrameRet == eNEX_FF_RET_EOF )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d][_LP_GetFrameFromVideoChunk] Video End Frame!", __LINE__);					
				bEndOfStream = TRUE;
				continue;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d][_LP_GetFrameFromVideoChunk] m_pGetFrame Failed(%d)!", __LINE__, eFrameRet);
				return eNEX_FF_RET_FILE_API_ERROR;
			}
		}

		if( pChunkData != NULL && (uChunkLen > 0) )
		{
			iChunkRet = VideoChunkParser_PutFrame32(m_hVideoChunkInfo, pChunkData, uChunkLen, uChunkDTS, uChunkPTS);
			if (iChunkRet < 0)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d][_LP_GetFrameFromVideoChunk] VideoChunkParser_PutFrame Failed(%d)", __LINE__, iChunkRet);
				return eNEX_FF_RET_FILE_API_ERROR;
			}			
		}
	}
}

NEX_FF_RETURN CNexFileReader::nxFFReaderDecrypt(NxFFReaderWBuffer *pFRWBuf, NXFF_MEDIA_TYPE iType) {
	NXUINT32	uEncIndex = 0;
	NXINT32 	nIndex = 0;
	NXINT32		iAdtsHeaderSize=0;
	if ( iType == NXFF_MEDIA_TYPE_AUDIO
		&& (m_pFFReader->hAudioTrackInfo[0].nOTI >= eNEX_CODEC_A_AAC && m_pFFReader->hAudioTrackInfo[0].nOTI <= eNEX_CODEC_A_ELDAAC ) ) {
		iAdtsHeaderSize = NexCodecUtil_GetSizeOfADTSHeader(pFRWBuf->bfrstart+9);
	}

	NXUINT8 	*pEncPtr =	pFRWBuf->bfrstart +9+iAdtsHeaderSize;
	NXUINT32	uSize = pFRWBuf->bytecnt-9-iAdtsHeaderSize;
	struct		AES_ctx ctx;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp %d] uSubSampleMaxCount: %u, uSubSampleCount: %u, iType=%d, iAdtsHeaderSize=%d\n", __LINE__, pFRWBuf->uSubSampleMaxCount, pFRWBuf->uSubSampleCount, iType, iAdtsHeaderSize);

	AES_init_ctx_iv(&ctx, m_EncInfo[iType], pFRWBuf->pIV);
	if ( pFRWBuf->uSubSampleCount == 0 ) {
		AES_CTR_xcrypt_buffer2(&ctx, pEncPtr,  pFRWBuf->bytecnt-9-7);
	} else {
		for (int i = 0; i < pFRWBuf->uSubSampleCount; i ++) {
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 1, "[WrapFileReader.cpp %d] EncPtr : %p, ClearDataSize : %d, EncryptedDataSize : %d \n", __LINE__, pEncPtr, pFRWBuf->pstSubSamples[i].uBytesOfClearData, pFRWBuf->pstSubSamples[i].uBytesOfEncryptedData);
			pEncPtr += pFRWBuf->pstSubSamples[i].uBytesOfClearData;
			if ( pFRWBuf->pstSubSamples[i].uBytesOfEncryptedData > 0 ) {
				AES_CTR_xcrypt_buffer2(&ctx, pEncPtr,  pFRWBuf->pstSubSamples[i].uBytesOfEncryptedData);
			}
			pEncPtr += pFRWBuf->pstSubSamples[i].uBytesOfEncryptedData;
		}
	}

	return eNEX_FF_RET_SUCCESS;
}

NEX_FF_RETURN CNexFileReader::nxFFReaderFrame(NXFF_MEDIA_TYPE iType)
{
	NEX_FF_RETURN			eFileReaderRet	= eNEX_FF_RET_SUCCESS;
	NXBOOL					bContinue		= FALSE;
	NxFFReaderWBuffer*		pFRWBuf		= NULL;
	unsigned char**			pBuf			= NULL;

	switch ( (int)iType )
	{
		case NXFF_MEDIA_TYPE_AUDIO:
			pFRWBuf		= &m_BufAudio;
			pBuf			= &m_pBuffAudio;
			break;

		case NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO:
		case NXFF_MEDIA_TYPE_ENHANCED_LAYER_VIDEO:
			pFRWBuf		= &m_BufVideo;
			pBuf			= &m_pBuffVideo;
			break;

		case NXFF_MEDIA_TYPE_TEXT:
			pFRWBuf		= &m_BufText;
			pBuf			= &m_pBuffText;
			break;
		default:
			return eNEX_FF_RET_FILE_API_ERROR;
	}

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp%4d] nxFFReaderFrame iType(x%x) pFRWBuf(%p)", __LINE__, iType, pFRWBuf);
	do
	{
		bContinue = FALSE;

		//uTick1 = nexSAL_GetTickCount();
		eFileReaderRet = NxFFR_ReadFrame( m_pFFReader, iType, 1, pFRWBuf );
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp%4d] NxFFR_ReadFrame Ret(%d)", __LINE__, nFileReaderRet);
		// uDiff = nexSAL_GetTickCount() - uTick1;

		switch ( eFileReaderRet )
		{
			case eNEX_FF_RET_SUCCESS:
				//if ( uDiff > 5 )
				//	nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "[WrapFileReader.cpp %d] NxFFReadFrame() elapsed (%u)ms!", __LINE__, uDiff);

				if ( pFRWBuf->nEncrypted && (iType <= NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO) )
				{
					eFileReaderRet=nxFFReaderDecrypt(pFRWBuf, iType);
				}
				break;

			case eNEX_FF_RET_READFRAME_SIZE_TOO_BIG:
				if( *pBuf )
				{
					nexSAL_MemFree(*pBuf);
					*pBuf = NULL;
				}
				*pBuf = (NXUINT8*)nexSAL_MemAlloc(pFRWBuf->bytecnt);

				if ( *pBuf )
				{
					pFRWBuf->bfrstart = pFRWBuf->bfr = *pBuf;
					pFRWBuf->nMaxSize = pFRWBuf->bytecnt;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] Channel[%d] big frame [%d]", __LINE__, iType, pFRWBuf->bytecnt);
					bContinue = TRUE;
				}
				else
				{
					pFRWBuf->bfrstart = pFRWBuf->bfr = *pBuf;
					pFRWBuf->nMaxSize = pFRWBuf->bytecnt = 0;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] MemAlloc fail - Channel[%d] big frame[%d]", __LINE__, iType, pFRWBuf->bytecnt);
					return eNEX_FF_RET_FILE_API_ERROR;
				}
				break;
			
			case eNEX_FF_RET_EOS:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] [%d] Channel End", __LINE__, iType);
				break;

			case eNEX_FF_RET_NOT_YET:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] [%d] Channel No Frame", __LINE__, iType);
				break;

            case eNEX_FF_RET_EOF:
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] [%d] End of file", __LINE__, iType);
                break;
                
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] [%d] Read Error(%d)", __LINE__, iType, eFileReaderRet );
				break;
		}
	}while(bContinue);

	return eFileReaderRet;
}

int CNexFileReader::dropVideoFrame(unsigned int uiTargetCTS, unsigned int* pCurrentDTS)
{
	if( pCurrentDTS == NULL )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	if( uiTargetCTS == 0 || uiTargetCTS > m_uiTotalPlayTime )
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;

	NEXVIDEOEDITORGetFrameReturn	eGetFrameRet	= _SRC_GETFRAME_OK;
	unsigned int						uiDTS			= 0;
	unsigned int						uiPTS			= 0;
	unsigned int						uiCount			= 100;

	while(uiCount > 0)
	{
		eGetFrameRet = (NEXVIDEOEDITORGetFrameReturn)getVideoFrame();
		if( eGetFrameRet == _SRC_GETFRAME_END )
		{
			*pCurrentDTS = 0;
			return -1;
		}

		getVideoFrameDTS(&uiDTS);
		getVideoFramePTS(&uiPTS);
		if( uiTargetCTS - 50 < uiDTS )
		{
			*pCurrentDTS = uiDTS;
			return NEXVIDEOEDITOR_ERROR_NONE;
		}
		uiCount--;
	}
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::setBaseTimeStamp(unsigned int uiBaseTime)
{
	m_uiBaseTimeStamp = uiBaseTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::setTrimTime(unsigned int uiStartTrimTime, unsigned int uiEndTrimTime)
{
	m_uiStartTrimTime		= uiStartTrimTime;
	m_uiEndTrimTime		= uiEndTrimTime;
	return NEXVIDEOEDITOR_ERROR_NONE;
}
	
NXBOOL CNexFileReader::setRepeatAudioClip(unsigned int uiBaseTime, unsigned int uiStartTime)
{
	/*  // for Audio Video seperate function
	if( isVideoExist () )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] do not repeat because clip have video track", __LINE__);
		return FALSE;
	}
	*/

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] setRepeatAudioClip(Base:%d StartTime %d)", __LINE__, uiBaseTime, uiStartTime);
	unsigned int uiSeekResultCTS	= 0;
	unsigned int uiTempBase		= m_uiBaseTimeStamp;

	m_uiBaseTimeStamp = uiBaseTime;
	if( seekToAudio(uiStartTime, &uiSeekResultCTS, NXFF_RA_MODE_PREV, TRUE, FALSE) != NEXVIDEOEDITOR_ERROR_NONE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileReader.cpp %d] do not repeat because clip did not seek", __LINE__);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] repeat Audio Clip baseTime(%d->%d) SeekR(%d)", __LINE__, uiTempBase, m_uiBaseTimeStamp, uiSeekResultCTS);
	// m_uiAudioLastReadCTS = uiSeekResultCTS;
	// m_uiLastAudioCTS		= uiSeekResultCTS;	// comment out for harmony 352.
	m_iLostFrameCount	= 0;
	m_isAudioEnd			= 0;
	return TRUE;
}

int CNexFileReader::getReaderRotateState()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getRotateState(%d)", __LINE__, m_uiOrientation);
	return m_uiOrientation;
}

int CNexFileReader::getRotateState()
{
	// For rotate state problem.
	/*
	int iRotate = 0;
	switch(m_uiOrientation)
	{
		case 1:
			iRotate = 90;
			break;
		case 2:
			iRotate = 180;
			break;
		case 3:
			iRotate = 270;
			break;
	}
	*/

	int iOrientation = m_uiOrientation;

	
	if( m_uiOrientation == 90 )
	{
		iOrientation = 270;
	}
	else if( m_uiOrientation == 270 )
	{
		iOrientation = 90;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getRotateState(%d %d)", __LINE__, m_uiOrientation, iOrientation);
	return iOrientation;
}

unsigned int CNexFileReader::getVideoFrameRate()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getVideoFrameRate(%d)", __LINE__, m_uiFrameFPS);
	return m_uiFrameFPS;
}

float CNexFileReader::getVideoFrameRateFloat()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getVideoFrameRate(%f)", __LINE__, m_fFrameFPS);
	return m_fFrameFPS;
}

unsigned int CNexFileReader::getTotalPlayAudioTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getTotalPlayAudioTime(%d)", __LINE__, m_uiTotalPlayAudioTime);
	return m_uiTotalPlayAudioTime;
}

unsigned int CNexFileReader::getTotalPlayVideoTime()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getTotalPlayVideoTime(%d)", __LINE__, m_uiTotalPlayVideoTime);
	return m_uiTotalPlayVideoTime;
}

int CNexFileReader::getAudioTrackCount()
{
	int iCount = 0;
	if( m_pFFReader != NULL )
	{
		iCount = m_pFFReader->nAudioTrackCnt;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getAudioTrackCount(%d)", __LINE__, iCount);
	return iCount;
}

int CNexFileReader::getVideoTrackCount()
{
	int iCount = 0;
	if( m_pFFReader != NULL )
	{
		iCount = m_pFFReader->nVideoTrackCnt;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getVideoTrackCount(%d)", __LINE__, iCount);
	return iCount;
}

//yoon
int  CNexFileReader::getVideoTrackUUID( NXUINT8** ppretAllocUUID, NXINT32* pretUUIDSize )
{
    NXINT32 i32Size = 0;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d] getVideoTrackUUID() start", __LINE__ );	
	if( m_pFFReader == NULL )
	{
	    nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d]getVideoTrackUUID() FFReader no instans", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

    if( !m_isVideoExist )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d]getVideoTrackUUID() no video", __LINE__);	
        return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
    }
	
    NXINT64 iUUIDsize = 0;
    NXUINT8 *pUuuid;
    *pretUUIDSize = 0;
	NEX_FF_RETURN eRet = NxFFR_GetExtInfo(m_pFFReader, NXFF_EXTINFO_GET_360_METADATA_DATA, 0, &iUUIDsize, (NXVOID**)&pUuuid);
	if( eRet != eNEX_FF_RET_SUCCESS )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d]getVideoTrackUUID() 360 metadatasize is Failed(Ret:%d)", __LINE__, eRet);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}

    *pretUUIDSize = i32Size = (NXINT32)iUUIDsize;
    
    if( iUUIDsize == 0 )
    {
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d]getVideoTrackUUID() 360 metadata size is zero", __LINE__);	
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_SOURCE, 0, "[WrapFileReader.cpp %d]getVideoTrackUUID() 360 metadata size(%d)", __LINE__,i32Size);	
    
    *ppretAllocUUID = pUuuid;
    //nexSAL_MemDump(pUuuid,i32Size);

    return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNexFileReader::getEditBoxTime(unsigned int* puiAudioTime, unsigned int* puiVideoTime)
{
	if( m_pFFReader != NULL )
	{
		*puiAudioTime = m_uiAudioEditBoxTime;
		*puiVideoTime = (m_uiVideoEditBoxTime - m_uiVideoCTSBoxTime)>0?m_uiVideoEditBoxTime - m_uiVideoCTSBoxTime:0;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getEditBoxTime(%d, %d)", __LINE__, *puiAudioTime, *puiVideoTime);
	return 0;
}

int CNexFileReader::getAudioObjectType()
{
	return m_uiAudioObjectType;
}

int CNexFileReader::getVideoObjectType()
{
	return m_uiVideoObjectType;
}

void CNexFileReader::setAudioObjectType(unsigned int uiAudioObjectType)
{
	m_uiAudioObjectType = uiAudioObjectType;
}

void CNexFileReader::setNumOfChannels(unsigned int uiNumOfChannels)
{
	m_uiNumOfChannels = uiNumOfChannels;
}

void CNexFileReader::setSamplingRate(unsigned int uiSamplingRate)
{
	m_uiSamplingRate = uiSamplingRate;
}

void CNexFileReader::setNumOfSamplesPerChannel(unsigned int uiNumOfSamplesPerChannel)
{
	m_uiNumOfSamplesPerChannel = uiNumOfSamplesPerChannel;

	m_uiAudioFrameInterval		= _GetFrameInterval(m_uiNumOfSamplesPerChannel, m_uiSamplingRate);
	m_dbAudioFrameInterval		= _GetFrameIntervalDouble(m_uiNumOfSamplesPerChannel, m_uiSamplingRate);
	m_uiMaxAudioFrameInterval	= (m_uiAudioFrameInterval*3) >> 1;			

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] Interval(%d %f %d)",  __LINE__,
		m_uiAudioFrameInterval, 
		m_dbAudioFrameInterval, 
		m_uiMaxAudioFrameInterval);
}

NXBOOL CNexFileReader::getEnhancedExist()
{
	return m_isEnhancedExist;
}

unsigned int CNexFileReader::getVideoEditBox()
{
	unsigned int uVTime = 0;
	if( m_pFFReader != NULL )
	{
		if(m_isEditBox)
			uVTime = m_uiVideoEditBoxTime;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getVideoEditBox(%d)", __LINE__, uVTime);
	return uVTime;
}

unsigned int CNexFileReader::getAudioEditBox()
{
	unsigned int uATime = 0;
	if( m_pFFReader != NULL )
	{
		if(m_isEditBox)
			uATime = m_uiAudioEditBoxTime;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileReader.cpp %d] getAudioEditBox(%d)", __LINE__, uATime);
	return uATime;
}


/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/21	Draft.
-----------------------------------------------------------------------------*/
