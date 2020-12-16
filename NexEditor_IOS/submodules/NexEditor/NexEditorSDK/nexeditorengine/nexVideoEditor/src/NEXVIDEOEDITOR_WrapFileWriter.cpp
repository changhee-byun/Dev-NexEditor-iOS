/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapFileWrite.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/10/25	Draft.
-----------------------------------------------------------------------------*/
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "string.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"

#define VERSION_INFO_WRITER_FORMAT_STRING "\
********************************************\n\
     NxMP4FWriter Version : %d.%d.%d.%d\n\
********************************************\n"

#define VERSION_INFO_WRITER_FORMAT_STRING2 "\
********************************************\n\
     NxMP4FWriter Version : %s\n\
********************************************\n"

// #define VIDEO_DUMP_TEST
#ifdef VIDEO_DUMP_TEST
#include "stdio.h"
FILE* g_Video_DumpFrameForMW = NULL;
FILE* g_Video_DumpFrame = NULL;

FILE* g_Video_Dump_YUV = NULL;
int gVideoFrameCount = 0;
#endif

// #define AUDIO_DUMP_PCM

#ifdef AUDIO_DUMP_PCM
#include "stdio.h"
FILE* g_Audio_Dump_PCM = NULL;
#endif

// #define AUDIO_DUMP_AAC

#ifdef AUDIO_DUMP_AAC
#include "stdio.h"
FILE* g_Audio_Dump_AAC = NULL;
#endif

// #define AUDIO_ENCODE_SKIP
// #define VIDEO_ENCODE_SKIP
#ifdef FOR_PROJECT_LGE
#define ISSEEKABLEFRAME_IDRONLY TRUE
#else
#define ISSEEKABLEFRAME_IDRONLY FALSE
#endif

#define PCM_BUFFER_SIZE			(2048 * 2)

#define VOICE_PCM_BUFFER_SIZE PCM_BUFFER_SIZE/2


CNexFileWriter::CNexFileWriter() :
	m_perfMonVideoEncode((char*)"Writer Encode"),
	m_perfMonVideoGetOutput((char*)"Writer Enc GetOutput"),
	m_perfMonVideoWrite((char*)"Writer VideoWrite "),
	m_perfMonVideoConverter((char*)"Writer Format Converter"),
	m_perfMonAudio((char*)"Writer AudioWrite")
{
	m_strTargetPath = NULL;
	m_isStarted					= FALSE;
	m_bAudioOnly				= FALSE;
	m_bVideoOnly				= FALSE;

	m_uiAudioCodecType			= 0;
	m_pAudioDSIInfo				= NULL;
	m_uiAudioDSISize			= 0;

	m_uiVideoCodecType			= 0;
	m_pBaseVideoDSIInfo			= NULL;
	m_uiBaseVideoDSISize		= 0;

	m_pEnhanceVideoDSIInfo		= NULL;
	m_uiEnhanceVideoDSISize		= 0;

	m_isTS						= 0;
	m_uiWidth					= 0;
	m_uiHeight					= 0;
	m_uiDisplayWidth				= 0;
	m_uiDisplayHeight				= 0;
	m_uiFrameRate				= EDITOR_DEFAULT_FRAME_RATE;
	m_uiAudioBitRate				= 0;
	m_uiBitRate					= 0;
	m_uiProfile					= 0;
	m_uiLevel					= 0;
	
	m_uiProfileLevelID			= 0;
	m_Rotate					= 0;
	m_uiDuration				= 0;
	m_qAudioTotalDuration		= 0;
	m_qVideoTotalDuration		= 0;
	
	m_pWriterHandle				= NULL;

	m_hCodecVideoEnc			= NULL;
	m_hCodecAudioEnc			= NULL;

	m_uiVideoFrameCount			= 0;
	m_uiVideoContinueCount		= 0;
	m_uiVideoStartGap			= 0;

	m_uiSamplingRate			= 0;
	
	m_uiVideoCTS				= 0;
	m_uiAudioCTS				= 0;
	m_qLastAudioCTS				= 0;
	m_qLastVideoCTS				= 0;
	m_pRawBuffer				= NULL;

	m_uiEncoderInputFormat		= NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;

	m_uiDSINalSize				= 0;

	m_MediaCodecInputSurf		= NULL;

	m_pVideoWriteTask			= NULL;

	m_uiMediaCodecUserData		= 0;
#ifdef FOR_TEST_MEDIACODEC_DEC	
	m_fnSetMediaCodecTimeStamp	= NULL;
	m_fnResetMediaCodecVideoEncoder	= NULL;
	m_fnSetCropToAchieveResolution	= NULL;
#endif

	m_bSkipMediaCodecTimeStamp	= FALSE;
	m_vecSkipMeidaCodecTimeStamp.clear();

	m_uiDECurrentAudioClipID	= INVALID_CLIP_ID;
	m_uiDECurrentVideoClipID	= INVALID_CLIP_ID;

	m_bPaused = FALSE;
	m_bVideoEncoderInitToDiscardDSI = FALSE;
	m_bLastTrackEnded = FALSE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d]  CNexFileWriter Create Done", __LINE__ );
	
}

CNexFileWriter::~CNexFileWriter()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] ~~~~CNexFileWriter Destroy In", __LINE__ );
	deinitFileWriter();
	if( m_hCodecVideoEnc )
	{
		nexCAL_VideoEncoderDeinit( m_hCodecVideoEnc );
		CNexCodecManager::releaseCodec( m_hCodecVideoEnc );	
		m_hCodecVideoEnc = NULL;
	}

	if( m_hCodecAudioEnc )
	{
		nexCAL_AudioEncoderDeinit( m_hCodecAudioEnc );
		CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
		m_hCodecAudioEnc = NULL;
	}
	
	if( m_pRawBuffer )
	{
		nexSAL_MemFree(m_pRawBuffer);
		m_pRawBuffer = NULL;
	}

#ifdef AUDIO_DUMP_PCM
	if( g_Audio_Dump_PCM )
	{
		fclose(g_Audio_Dump_PCM);
		g_Audio_Dump_PCM = NULL;
	}
#endif

#ifdef AUDIO_DUMP_AAC
	if( g_Audio_Dump_AAC )
	{
		fclose(g_Audio_Dump_AAC);
		g_Audio_Dump_AAC = NULL;
	}
#endif

	m_uiAudioCodecType		= 0;
	if( m_pAudioDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pAudioDSIInfo);
		m_pAudioDSIInfo = NULL;
	}
	m_uiAudioDSISize			= 0;

	m_uiVideoCodecType		= 0;
	if( m_pBaseVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pBaseVideoDSIInfo);
		m_pBaseVideoDSIInfo = NULL;
	}
	m_uiBaseVideoDSISize	= 0;

	if( m_pEnhanceVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pEnhanceVideoDSIInfo);
		m_pEnhanceVideoDSIInfo = NULL;
	}
	m_uiEnhanceVideoDSISize	= 0;
	
	if( m_pVideoWriteTask != NULL )
	{
		if( m_pVideoWriteTask->IsWorking() )
		{
			m_pVideoWriteTask->End(1000);
		}
		m_pVideoWriteTask->setFileWrite(NULL);
		SAFE_RELEASE(m_pVideoWriteTask);
	}

	if(m_strTargetPath)
	{
		nexSAL_MemFree(m_strTargetPath);
		m_strTargetPath = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] ~~~~CNexFileWriter Destroy Done", __LINE__ );
}

NXBOOL CNexFileWriter::setFilePath(char* pStrTargetPath)
{
	if( pStrTargetPath == NULL ) return FALSE;

	if(m_strTargetPath)
	{
		nexSAL_MemFree(m_strTargetPath);
		m_strTargetPath = NULL;
	}
	m_strTargetPath = (char*)nexSAL_MemAlloc(strlen(pStrTargetPath)+1);

	strcpy(m_strTargetPath, pStrTargetPath);
	return TRUE;
}

NXBOOL CNexFileWriter::setEncoderInputFormat(unsigned int uiFormat)
{
	m_uiEncoderInputFormat = uiFormat;
	return TRUE;
}

NXBOOL CNexFileWriter::initFileWriter(long long llMaxFileSize, unsigned int uiDuration)
{
#if 0
	nexSAL_DebugPrintf( VERSION_INFO_WRITER_FORMAT_STRING,
						NxFFWriterGetVersionNum(0), NxFFWriterGetVersionNum(1), 
						NxFFWriterGetVersionNum(2), NxFFWriterGetVersionNum(3) );
#else
	nexSAL_DebugPrintf( VERSION_INFO_WRITER_FORMAT_STRING2, NxFFWriter_GetVersionString());
#endif

	if( m_pWriterHandle )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] initFileWriter(%p)", __LINE__, m_pWriterHandle);
		NxFFWriterDestroy(m_pWriterHandle);	
		m_pWriterHandle = NULL;
	}

	m_bLastTrackEnded = FALSE;
	m_uiDuration = uiDuration;
	m_bPaused = FALSE;
	unsigned int uiMovboxSize = (3*1024*m_uiDuration/1000) + (2*1024*m_uiDuration/1000);
	uiMovboxSize = uiMovboxSize * 120 / 100;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] initFileWriter expect movbox size(%d) dur(%d)", __LINE__, uiMovboxSize, m_uiDuration);

	NXUINT64 nEpochTime = 0;
	nexSAL_GetMSecFromEpoch(&nEpochTime);

	if( m_pRawBuffer == NULL )
		m_pRawBuffer = (unsigned char*)nexSAL_MemAlloc(WRITER_RAW_FRAME_TEMP_SIZE);

	m_uiAudioCodecType		= 0;
	if( m_pAudioDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pAudioDSIInfo);
		m_pAudioDSIInfo = NULL;
	}
	m_uiAudioDSISize			= 0;

	m_uiVideoCodecType		= 0;
	if( m_pBaseVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pBaseVideoDSIInfo);
		m_pBaseVideoDSIInfo = NULL;
	}
	m_uiBaseVideoDSISize	= 0;

	if( m_pEnhanceVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pEnhanceVideoDSIInfo);
		m_pEnhanceVideoDSIInfo = NULL;
	}
	m_uiEnhanceVideoDSISize	= 0;

	m_pWriterHandle = NxFFWriterCreate();
	if( m_pWriterHandle == NULL ) return FALSE;

	m_pWriterHandle->AvailableRAMMemory		= 10 *1024*1024;	// 0: temp_file 이용, non-zero: 가용 메모리 사이즈
	m_pWriterHandle->MaxFileSize			= llMaxFileSize;	// k3g/3gpp/3g2 파일 생성 가용 메모리 사이즈, AvailableRAMMemory가 양수일 경우에만 사용됨.
	m_pWriterHandle->MaxRecordingTime		= 0;			// millisecond 단위, streaming Recording Mode에서만 지원함. 0일 경우 MaxFileSize와 AvailableMemory에 따라 저장됨.

	m_pWriterHandle->MediaFileFormatType	= eNEX_FF_MP4;
	m_pWriterHandle->MediaFileFormatSubType	= eNEX_FF_MP4_3GPP;

	m_pWriterHandle->StreamingRecord		= 1;				// 0: local recording, not zero: streaming recording
	m_pWriterHandle->bStreamingLargeFrame	= TRUE;
	m_pWriterHandle->MPEG4system			= 0;				// 0: not support BIFS, not zero: support BIFS
	m_pWriterHandle->TSflag					= 0;				// 0: baselayer recording, not zero: enhancement layer recording
	m_pWriterHandle->CreationTime			= (NXUINT32)(nEpochTime/1000);				// mvhd, mdhd (creation time)

	m_pWriterHandle->VOTI					= 0;				// VideoCodingType : 0 (None), (MPEG-4: 0x20), (H.263: 0xC0), (H.264: 0xC1)
	m_pWriterHandle->H263FrameRate			= 0;				// 0 (variable frame rate), 1 (29.97Hz), 2 (14.485Hz), 3 (7.243Hz), ....
	m_pWriterHandle->VideoBufferSizeDB[0]	= 3*1024*1024;
	m_pWriterHandle->VideoBufferSizeDB[1]	= 3*1024*1024;
	m_pWriterHandle->VideoWidth				= 0;
	m_pWriterHandle->VideoHeight			= 0;
	m_pWriterHandle->VideoDecoderSpecificInfoSize[0]	= 0;	// For Streaming Recording
	m_pWriterHandle->VideoDecoderSpecificInfoSize[1]	= 0;	// For Streaming Recording

	m_pWriterHandle->pVideoDecoderSpecificInfo[0] = NULL;
	m_pWriterHandle->pVideoDecoderSpecificInfo[1] = NULL;

	m_pWriterHandle->profile_level_id		= 0;				// Only for H.264
	m_pWriterHandle->parameter_sets_bytecnt	= 0;				// Obly for H.264
	m_pWriterHandle->parameter_sets			= NULL;			// Only for H.264

	m_pWriterHandle->AOTI					= 0;				// AudioCodingType : None(0x00), AAC(0x40), MP3(0x6B), AMR(0xD0), EVRC(0xD1), QCELP(0xD2), SMV(0xD3), AMRWB(0xD4), G711(0xDF), G723(0xDE), QCELP_ALT(0xE1)
	m_pWriterHandle->AudioBufferSizeDB		= 20*1024;
	m_pWriterHandle->AudioDecoderSpecificInfoSize		= 0;		// For Streaming Recording
	m_pWriterHandle->pAudioDecoderSpecificInfo	= NULL;
	m_pWriterHandle->G711ChannelNum			= 0;				// For G711 support

	m_pWriterHandle->TextCodingType			= 0;				// TextCodingType : 0 (None), 1 (T.140) 
	m_pWriterHandle->TextBufferSizeDB		= 0;
	m_pWriterHandle->TextDecoderSpecificInfoSize		= 0;
	m_pWriterHandle->pTextDecoderSpecificInfo	= NULL;

	m_pWriterHandle->bFreeBox					= 1;
	m_pWriterHandle->uFreeBoxMaxSize			= uiMovboxSize;

	m_pWriterHandle->BIFSBufferSizeDB			= 0;
	m_pWriterHandle->ODBufferSizeDB				= 0;

	m_pWriterHandle->MidiFileFlag				= 0;				// For Streaming Recording
	m_pWriterHandle->LinkDataFlag				= 0;				// For Streaming Recording
	m_pWriterHandle->LinkDataSize				= 0;				// For Streaming Recording
	m_pWriterHandle->LinkData					= NULL;				// For Streaming Recording

	m_pWriterHandle->MovieDuration				= 0;
	m_pWriterHandle->MovieSize					= 0;

	m_pWriterHandle->WriteBufferSize			= 0;
	m_pWriterHandle->pWriteBuffer				= NULL;

	m_uiVideoCTS								= 0;
	m_uiAudioCTS								= 0;

	m_uiVideoFrameCount							= 0;
	m_uiVideoStartGap							= 0;

	if( m_hCodecVideoEnc )
	{
		nexCAL_VideoEncoderDeinit( m_hCodecVideoEnc );
		CNexCodecManager::releaseCodec( m_hCodecVideoEnc );	
		m_hCodecVideoEnc = NULL;
	}
	
	if( m_hCodecAudioEnc )
	{
		nexCAL_AudioEncoderDeinit( m_hCodecAudioEnc );
		CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
		m_hCodecAudioEnc = NULL;
	}

#ifdef AUDIO_DUMP_PCM
	g_Audio_Dump_PCM = fopen("/sdcard/PCMIn.pcm", "wb");
#endif

#ifdef AUDIO_DUMP_AAC
	g_Audio_Dump_AAC = fopen("/sdcard/aacOut.aac", "wb");
#endif

	m_uiSystemTickCheck		= 0;
	m_uiVideoContinueCount	= 0;

	m_uiWidth				= 0;
	m_uiHeight				= 0;
	m_uiDisplayWidth			= 0;
	m_uiDisplayHeight			= 0;

	m_uiFrameRate			= EDITOR_DEFAULT_FRAME_RATE;
	m_uiAudioBitRate		= 0;
	m_uiBitRate				= 0;
	m_uiProfile				= 0;
	m_uiLevel				= 0;
	
	m_uiDECurrentAudioClipID = INVALID_CLIP_ID;
	m_uiDECurrentVideoClipID = INVALID_CLIP_ID;

	m_uiDSINalSize				= 0;

#ifdef _ANDROID
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor != NULL )
	{
		for(int i = 0; i < pEditor->getUDTACount(); i++ )
		{
			CNexUDTA* pdata = pEditor->getUDTA(i);
			if( pdata == NULL )
				continue;

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] set UDta(0x%x %s)", __LINE__, pdata->m_iType, pdata->m_pData);

			UdtaBox* pUdtabox;

			pUdtabox = (UdtaBox*)nexSAL_MemAlloc(sizeof(_UdtaBox));
			if( pUdtabox == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] FileWriter Init failed with Alloc UDta", __LINE__);
				continue;
			}

			NXUINT32 datasize = (NXUINT32)strlen(pdata->m_pData);
			pUdtabox->pData = (NXUINT8*)nexSAL_MemAlloc(datasize + 4);
			if( pUdtabox->pData == NULL )
			{
				nexSAL_MemFree(pUdtabox);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] FileWriter Init failed with Alloc pData of UDta", __LINE__);
				continue;
			}

			pUdtabox->pData[0] = 0x15;
			pUdtabox->pData[1] = 0xC7;
			strcpy((char*)pUdtabox->pData+2, pdata->m_pData);
			datasize += 2;
			pUdtabox->pData[datasize++] = 0x00;
			pUdtabox->pData[datasize++] = 0x2F;

			pUdtabox->uSize = 12 + datasize;
			pUdtabox->uType = pdata->m_iType;
			pUdtabox->uVersionFlag = 0;
			pUdtabox->uDataSize = datasize;		// data size

			int Ret = NxFFWriteUdta(m_pWriterHandle, pUdtabox);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxFFWriteUdta uSize(%d) UType(0x%x) uVersion(%d) uDataSize(%d) iRet (%d)",
							__LINE__,
							pUdtabox->uSize,
							pUdtabox->uType,
							pUdtabox->uVersionFlag,
							pUdtabox->uDataSize,
							Ret);
			nexSAL_MemDump(pUdtabox->pData, pUdtabox->uDataSize);

			if( pUdtabox->pData != NULL )
			{
				nexSAL_MemFree(pUdtabox->pData);
				pUdtabox->pData = NULL;
			}

			if( pUdtabox != NULL )
			{
				nexSAL_MemFree(pUdtabox);
				pUdtabox = NULL;
			}
		}

		NXINT32 uiCreationTime = pEditor->getPropertyInt("CreationTime", 0);
		if(uiCreationTime)
			m_pWriterHandle->CreationTime = (NXUINT32)uiCreationTime/1000;

		SAFE_RELEASE(pEditor);
	}
#endif
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] FileWriter Init  End", __LINE__);
	return TRUE;

Init_Error:
	
	deinitFileWriter();
	
	if( m_hCodecVideoEnc )
	{
		nexCAL_VideoEncoderDeinit( m_hCodecVideoEnc );
		CNexCodecManager::releaseCodec( m_hCodecVideoEnc );	
		m_hCodecVideoEnc = NULL;
	}
	
	if( m_hCodecAudioEnc )
	{
		nexCAL_AudioEncoderDeinit( m_hCodecAudioEnc );
		CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
		m_hCodecAudioEnc = NULL;
	}
	
	return FALSE;
}

NXBOOL CNexFileWriter::deinitFileWriter()
{
	if( m_pWriterHandle )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] deinitFileWriter(%p)", __LINE__, m_pWriterHandle);
		NxFFWriterDestroy(m_pWriterHandle);	
		m_pWriterHandle = NULL;
	}

	if( m_pRawBuffer )
	{
		nexSAL_MemFree(m_pRawBuffer);
		m_pRawBuffer = NULL;
	}

	return TRUE;
}

NXBOOL CNexFileWriter::startFileWriter()
{
	if( m_isStarted || m_pWriterHandle == NULL )
		return FALSE;

	if( m_bVideoOnly == TRUE && m_uiBaseVideoDSISize > 0 )
	{
	}
	else if( m_bAudioOnly == FALSE && (m_uiBaseVideoDSISize <= 0 || m_uiAudioDSISize <= 0) )
	{
#ifndef USE_WRITE_TASK
		if( CNexVideoEditor::m_iUseSurfaceMediaSource == 1 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Input media source mode(did not use writer task)", __LINE__ );
		}
		else
#endif
		{
        	if( CNexVideoEditor::m_iUseSurfaceMediaSource == 1 )
			{
        		m_uiEncoderInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
        	}

			if( m_pVideoWriteTask != NULL )
			{
				if( m_pVideoWriteTask->IsWorking() == FALSE )
				{
					m_pVideoWriteTask->setFileWrite(this);
					m_pVideoWriteTask->setEncoderInputFormat(m_uiEncoderInputFormat);
					m_pVideoWriteTask->Begin();
				}
			}
			else
			{
				m_pVideoWriteTask = new CNEXThread_VideoFrameWriteTask;
				if( m_pVideoWriteTask != NULL )
				{
					m_pVideoWriteTask->setFileWrite(this);
					m_pVideoWriteTask->setEncoderInputFormat(m_uiEncoderInputFormat);
					m_pVideoWriteTask->Begin();
				}
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Not input media source mode(use writer task)", __LINE__ );
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Audio or Video DSI was not ready", __LINE__ );
		return TRUE;
	}

	m_pWriterHandle->TSflag						= m_isTS;

	// for Video encoding crop dsi
	int iExportWidth = m_uiWidth;
	int iExportHeight = m_uiHeight;
	int iExportDisplayWidth = m_uiDisplayWidth;
	int iExportDisplayHeight = m_uiDisplayHeight;

	if( m_pBaseVideoDSIInfo != NULL &&  m_fnSetCropToAchieveResolution != NULL )
	{
		if( iExportHeight == 1088 )
		{
			iExportHeight = 1080;
			if (iExportDisplayHeight == 1088)
				iExportDisplayHeight = 1080;
		}
		else if(iExportHeight == 736 )
		{
			iExportHeight = 720;
			if (iExportDisplayHeight == 736)
				iExportDisplayHeight = 720;
		}
		else if(iExportHeight == 544 )
		{
			iExportHeight = 540;
			if (iExportDisplayHeight == 544)
				iExportDisplayHeight = 540;
		}
		else if(iExportHeight == 368 )
		{
			iExportHeight = 360;
			if (iExportDisplayHeight == 368)
				iExportDisplayHeight = 360;
		}

		nexSAL_MemDump(m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize);

		NEXCODECUTIL_SPS_INFO SPS_Info;
		NXUINT32 uByteFormat4DSI = NexCodecUtil_CheckByteFormat(m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize);
		NXINT32 nSPSRet = NexCodecUtil_AVC_GetSPSInfo((NXCHAR *)m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize, &SPS_Info, uByteFormat4DSI);

		if(m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V)
		{
			if (!nSPSRet)
			{
				if (SPS_Info.usWidth > iExportWidth || SPS_Info.usHeight > iExportHeight)
					m_uiBaseVideoDSISize = m_fnSetCropToAchieveResolution(m_pBaseVideoDSIInfo, 150, iExportWidth, iExportHeight);
			}
			else
			{
				m_uiBaseVideoDSISize = m_fnSetCropToAchieveResolution(m_pBaseVideoDSIInfo, 150, iExportWidth, iExportHeight);
			}
		}

		nexSAL_MemDump(m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Apply dsi crop(%d ->%d)", __LINE__, m_uiHeight, iExportHeight );
	}

	m_pWriterHandle->VideoWidth					= iExportWidth;
	m_pWriterHandle->VideoHeight				= iExportHeight;
	m_pWriterHandle->DisplayVideoWidth			= iExportDisplayWidth;
	m_pWriterHandle->DisplayVideoHeight			= iExportDisplayHeight;
	
	m_pWriterHandle->parameter_sets_bytecnt		= m_uiBaseVideoDSISize;		// Only for H.264
	m_pWriterHandle->parameter_sets				= m_pBaseVideoDSIInfo;		// Only for H.264
	m_pWriterHandle->RotationValue				= m_Rotate;

	m_pWriterHandle->profile_level_id			= m_uiProfileLevelID;		// Only for H.264

	// Base Video
	if( m_uiBaseVideoDSISize )
	{
		m_pWriterHandle->VideoDecoderSpecificInfoSize[0] = m_uiBaseVideoDSISize;
		m_pWriterHandle->pVideoDecoderSpecificInfo[0]	= m_pBaseVideoDSIInfo;
	}

	// Enhancement Video
	if( m_pEnhanceVideoDSIInfo != NULL && m_uiEnhanceVideoDSISize > 0 )
	{
		m_pWriterHandle->VideoDecoderSpecificInfoSize[1] = m_uiEnhanceVideoDSISize;
		m_pWriterHandle->pVideoDecoderSpecificInfo[1] = m_pEnhanceVideoDSIInfo;
	}
	m_pWriterHandle->VOTI	= m_uiVideoCodecType;

	if( m_bVideoOnly == TRUE )
	{
		m_pWriterHandle->AudioDecoderSpecificInfoSize = 0;
		m_pWriterHandle->AOTI = 0;
	}
	else
	{
		// Audio
		if( m_uiAudioDSISize )
		{
			m_pWriterHandle->AudioDecoderSpecificInfoSize = m_uiAudioDSISize;
			m_pWriterHandle->pAudioDecoderSpecificInfo = m_pAudioDSIInfo;
		}

		// m_pWriterHandle->TextDecoderSpecificInfoSize = pTargetConfig->m_uTextDSISize;
		// memcpy( m_pWriterHandle->TextDecoderSpecificInfo, pTargetConfig->m_pTextDSI, pTargetConfig->m_uTextDSISize );

		//
		
		m_pWriterHandle->AOTI	= m_uiAudioCodecType;

		// JDKIM : FileWriter에서 AAC+에 대한 Object Type 을 인식하지 못하므로 AAC로 변경하여 입력
		if ( m_pWriterHandle->AOTI == eNEX_CODEC_A_AACPLUS )
		{
			m_pWriterHandle->AOTI = eNEX_CODEC_A_AAC;
		}
	}

	// for writer version 3.6
#if 0
	m_pWriterHandle->bLargeFileSize = FALSE;

	if( m_uiDuration >= 0 )
	{
		long long llMaxSize = (long long)1024  * 1024 * 1024 * 7 / 2;
		long long llTotalSize = (long long)m_uiDuration / 1000 * (m_uiBitRate / 8 );
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] check size(%lld, %lld)", __LINE__, llMaxSize, llTotalSize );
		if( llTotalSize > llMaxSize )
		{
			// for writer version 3.6
			m_pWriterHandle->bLargeFileSize = TRUE;
		}
	}
#else
	m_pWriterHandle->bLargeFileSize = TRUE;
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "  Writer Init Info Start  -----------------------" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AvailableRAMMemory					: %d", m_pWriterHandle->AvailableRAMMemory );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "MaxFileSize						: %lld", m_pWriterHandle->MaxFileSize );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "MaxRecordingTime					: %d", m_pWriterHandle->MaxRecordingTime );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "NXFFW_MP4_FF						: %d", m_pWriterHandle->MediaFileFormatType );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "NXFFW_STANDARD_3GPP				: %d", m_pWriterHandle->MediaFileFormatSubType );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "StreamingRecord					: %d", m_pWriterHandle->StreamingRecord );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "bStreamingLargeFrame				: %d", m_pWriterHandle->bStreamingLargeFrame );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "MPEG4system						: %d", m_pWriterHandle->MPEG4system );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "TSflag								: %d", m_pWriterHandle->TSflag );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "CreationTime						: %d", m_pWriterHandle->CreationTime );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "\n" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoCodingType					: %d", m_pWriterHandle->VOTI );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "H263FrameRate						: %d", m_pWriterHandle->H263FrameRate );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoBufferSizeDB[0]				: %d", m_pWriterHandle->VideoBufferSizeDB[0] );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoBufferSizeDB[1]				: %d", m_pWriterHandle->VideoBufferSizeDB[1] );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoWidth							: %d", m_pWriterHandle->VideoWidth );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoHeight						: %d", m_pWriterHandle->VideoHeight );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoDecoderSpecificInfoSize[0]	: %d", m_pWriterHandle->VideoDecoderSpecificInfoSize[0] );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "VideoDecoderSpecificInfoSize[1]	: %d", m_pWriterHandle->VideoDecoderSpecificInfoSize[1] );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "profile_level_id					: %d", m_pWriterHandle->profile_level_id );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "parameter_sets_bytecnt				: %d", m_pWriterHandle->parameter_sets_bytecnt );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "parameter_sets						: 0x%08x", m_pWriterHandle->parameter_sets );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "RotationValue						:%d", m_pWriterHandle->RotationValue );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "\n" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioCodingType					: %d", m_pWriterHandle->AOTI );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioBufferSizeDB					: %d", m_pWriterHandle->AudioBufferSizeDB );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "AudioDecoderSpecificInfoSize		: %d", m_pWriterHandle->AudioDecoderSpecificInfoSize );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "G711ChannelNum						: %d", m_pWriterHandle->G711ChannelNum );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "\n" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "TextCodingType						: %d", m_pWriterHandle->TextCodingType );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "TextBufferSizeDB					: %d", m_pWriterHandle->TextBufferSizeDB );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "\n" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "bFreeBox							: %d", m_pWriterHandle->bFreeBox );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "uFreeBoxMaxSize					: %d", m_pWriterHandle->uFreeBoxMaxSize );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "\n" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "BIFSBufferSizeDB					: %d", m_pWriterHandle->BIFSBufferSizeDB );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "ODBufferSizeDB						: %d", m_pWriterHandle->ODBufferSizeDB );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "\n" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "MidiFileFlag						: %d", m_pWriterHandle->MidiFileFlag );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "LinkDataFlag						: %d", m_pWriterHandle->LinkDataFlag );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "LinkDataSize						: %d", m_pWriterHandle->LinkDataSize );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "LinkData							: 0x%08x", m_pWriterHandle->LinkData );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "\n" );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "MovieDuration						: %d", m_pWriterHandle->MovieDuration );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "MovieSize							: %d", m_pWriterHandle->MovieSize );
	// for writer version 3.6
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "bLargeFileSize						: %d", m_pWriterHandle->bLargeFileSize );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_uiDuration						: %d", m_uiDuration );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_uiAudioBitRate					: %d", m_uiAudioBitRate );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_uiBitRate						: %d", m_uiBitRate );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_uiProfile						: %d", m_uiProfile);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_uiLevel							: %d", m_uiLevel);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_szWriterODFPath					: %s", m_strTargetPath );
	
	/*
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_pCID                       : %d", pTargetConfig->m_pCID );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_uCIDLen                       : %d", pTargetConfig->m_uCIDLen );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_pRIU                       : %d", pTargetConfig->m_pRIU );
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "m_uRIULen                       : %d", pTargetConfig->m_uRIULen );
	*/
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "   Writer Init Info End   -----------------------" );

	if( m_pWriterHandle->pVideoDecoderSpecificInfo[0] != NULL && m_pWriterHandle->VideoDecoderSpecificInfoSize[0] > 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Video DSI Info", __LINE__ );
        if (nexSAL_TraceCondition(NEX_TRACE_CATEGORY_FLOW, 1)) {
		nexSAL_MemDump(m_pWriterHandle->pVideoDecoderSpecificInfo[0], m_pWriterHandle->VideoDecoderSpecificInfoSize[0] );
        }
	}
	if( m_pWriterHandle->pVideoDecoderSpecificInfo[1] != NULL && m_pWriterHandle->VideoDecoderSpecificInfoSize[1] > 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] enhance Video DSI Info", __LINE__ );
		nexSAL_MemDump(m_pWriterHandle->pVideoDecoderSpecificInfo[1], m_pWriterHandle->VideoDecoderSpecificInfoSize[1] );
	}
	
	int iRet = NxFFWriterInit( m_pWriterHandle, (unsigned char *)m_strTargetPath, NULL, NULL);
	if( iRet !=  0 )
	{
		switch ( iRet )
		{
			case 1 :
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] MP4 FILE OPEN ERROR", __LINE__ );
				break;
			case 2 :
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] MIDI FILE OPEN ERROR", __LINE__ );
				break;
			case 3 :
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] INVALID VIDEO CODING TYPE", __LINE__ );
				break;
			case 4 :
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NO-MEDIA SELECTION", __LINE__ );
				break;
			case 10 :
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] TOO SMALL MEMORY", __LINE__ );
				break;
			default :
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Writer INIT FAIL(errcode:%d)", __LINE__, iRet );
				break;
		};
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Writer INIT SUCCESS(%p)", __LINE__,  m_pWriterHandle);
	m_isStarted = TRUE;
    //yoon
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
        if( pEditor->getVideoTrackUUIDMode() != 0 )
        {
            int iUUIDSize = 0;
            unsigned char *pUUID = pEditor->getVideoTrackUUID(&iUUIDSize);
            
            if(iUUIDSize > 0 && pUUID != NULL )
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] getVideoTrackUUID setting", __LINE__);
                nexSAL_MemDump(pUUID,iUUIDSize);
                NXINT64 nDataSize = iUUIDSize;
		        NXINT8* pData = (NXINT8*)pUUID;
                NxFFWriterSetExtInfo(m_pWriterHandle,NXFFW_EXTINFO_SET_360_METADATA_DATA,nDataSize,(NXVOID*)pData);	
            }    
        }
		SAFE_RELEASE(pEditor);
	}
    
    
	m_perfMonVideoEncode.CheckModuleStart();
	m_perfMonVideoGetOutput.CheckModuleStart();
	m_perfMonVideoWrite.CheckModuleStart();
	m_perfMonVideoConverter.CheckModuleStart();
	m_perfMonAudio.CheckModuleStart();

#ifndef USE_WRITE_TASK
	if( CNexVideoEditor::m_iUseSurfaceMediaSource == 1 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Input media source mode(did not use writer task)", __LINE__ );
	}
	else
#endif
	{
    		if( CNexVideoEditor::m_iUseSurfaceMediaSource == 1 )
    		{
    			m_uiEncoderInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
    		}

		if( m_pVideoWriteTask != NULL )
		{
			if( m_pVideoWriteTask->IsWorking() == FALSE )
			{
				m_pVideoWriteTask->setFileWrite(this);
				m_pVideoWriteTask->setEncoderInputFormat(m_uiEncoderInputFormat);
				m_pVideoWriteTask->Begin();
			}
		}
		else
		{
			m_pVideoWriteTask = new CNEXThread_VideoFrameWriteTask;
			if( m_pVideoWriteTask != NULL )
			{
				m_pVideoWriteTask->setFileWrite(this);
				m_pVideoWriteTask->setEncoderInputFormat(m_uiEncoderInputFormat);
				m_pVideoWriteTask->Begin();
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Not input media source mode(use writer task)", __LINE__ );
	}

	m_uiSystemTickCheck		= 0;
	m_uiVideoContinueCount		= 0;
	m_llWriteAudioTotal		= 0;
	m_llWriteVideoTotal		= 0;

	m_uiVideoStartGap			= 0;

	m_uiDECurrentAudioClipID = INVALID_CLIP_ID;
	m_uiDECurrentVideoClipID = INVALID_CLIP_ID;
	return TRUE;
}

NXBOOL CNexFileWriter::endFileWriter(unsigned int* pDuration, NXBOOL bCancel)
{
	if( pDuration == NULL || m_pWriterHandle == NULL)
		return FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] endFileWriter started(%d) cancel(%d) TotalSize(%lld %lld)", __LINE__, m_isStarted, bCancel, m_llWriteAudioTotal, m_llWriteVideoTotal);

	if( m_isStarted == FALSE  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Video Encoder close start", __LINE__);
		if( m_hCodecVideoEnc )
		{
			nexCAL_VideoEncoderDeinit( m_hCodecVideoEnc );
			CNexCodecManager::releaseCodec( m_hCodecVideoEnc );	
			m_hCodecVideoEnc = NULL;
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Video Encoder close sucessed", __LINE__);
		
		if( m_hCodecAudioEnc )
		{
			nexCAL_AudioEncoderDeinit( m_hCodecAudioEnc );
			CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
			m_hCodecAudioEnc = NULL;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Audio Encoder close sucessed", __LINE__);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] WRITER CLOSE SUCCESS this Dur(%d), RefCount(%d)", __LINE__, *pDuration, GetRefCnt());
		// for mantis 6152
		if( bCancel )
		{
			nexSAL_FileRemove(m_strTargetPath);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Deleting done current export file", __LINE__);
		}

		m_uiAudioCodecType		= 0;
		if( m_pAudioDSIInfo != NULL )
		{
			nexSAL_MemFree(m_pAudioDSIInfo);
			m_pAudioDSIInfo = NULL;
		}
		m_uiAudioDSISize			= 0;

		m_uiVideoCodecType		= 0;
		if( m_pBaseVideoDSIInfo != NULL )
		{
			nexSAL_MemFree(m_pBaseVideoDSIInfo);
			m_pBaseVideoDSIInfo = NULL;
		}
		m_uiBaseVideoDSISize	= 0;

		if( m_pEnhanceVideoDSIInfo != NULL )
		{
			nexSAL_MemFree(m_pEnhanceVideoDSIInfo);
			m_pEnhanceVideoDSIInfo = NULL;
		}
		m_uiEnhanceVideoDSISize	= 0;

		m_Rotate					= 0;
		m_uiSamplingRate			= 0;

		if( m_pVideoWriteTask != NULL )
		{
			if( m_pVideoWriteTask->IsWorking() )
				m_pVideoWriteTask->End(1000);
			m_pVideoWriteTask->setFileWrite(NULL);
		}

		return TRUE;
	}

	m_isStarted = FALSE;
	m_bAudioOnly = FALSE;
	m_bVideoOnly = FALSE;

	NXBOOL	bEndWritingAudioFrame = FALSE;
	NXBOOL	bEndWritingVideoFrame = FALSE;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] endFileWriter In bCancel(%d)", __LINE__, bCancel);		

	if( m_pVideoWriteTask != NULL )
	{
		if( m_pVideoWriteTask->IsWorking() )
			m_pVideoWriteTask->End(1000);
		m_pVideoWriteTask->setFileWrite(NULL);
		SAFE_RELEASE(m_pVideoWriteTask);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] File Writer Task Ended", __LINE__ );		

	// write remained video audio frames.
	if( bCancel == FALSE)
	{
		unsigned int uiRet = 0;
		unsigned int uiAEncRet = 0;
		unsigned int uiVEncRet = 0;
		unsigned char* pBitStream = NULL;
		unsigned int uiBitStreamLen = 0;

		if( m_hCodecAudioEnc == NULL)
			bEndWritingAudioFrame = TRUE;

		if( m_hCodecVideoEnc == NULL)
			bEndWritingVideoFrame = TRUE;
		
		do
		{
			if( bEndWritingAudioFrame == FALSE )
			{
				pBitStream = NULL;
				uiBitStreamLen = 0;
				
				uiRet = nexCAL_AudioEncoderEncode(m_hCodecAudioEnc, NULL, 0 , &pBitStream, &uiBitStreamLen, &uiAEncRet);

				if( uiRet != 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encode Failed(%d %d)", __LINE__, uiRet, uiAEncRet);
					bEndWritingAudioFrame = TRUE;
					//return FALSE;
				}

				if( NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_ENCODING_SUCCESS))
				{
					if(NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_OUTPUT_EXIST))
					{
						if( writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiBitStreamLen - 7, pBitStream + 7, m_uiEncodedCTS, m_uiEncodedCTS, NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
						{
							return FALSE;
						}

#ifdef AUDIO_DUMP_AAC
						if( g_Audio_Dump_AAC )
							fwrite(pBitStream, 1, uiBitStreamLen, g_Audio_Dump_AAC);
#endif
						m_ullEncodedPCMSize +=4096;
						m_uiEncodedCTS =  (unsigned int)(m_ullEncodedPCMSize*1000/(m_uiSamplingRate*m_uiChannels*2));
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[WrapFileWriter.cpp %d]endFileWriter  Audio One Frame Writen Len(%d) E(%lld, %d)", __LINE__, uiBitStreamLen - 7, m_ullEncodedPCMSize, m_uiEncodedCTS);	
					}
					
					if(NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_EOS))
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Audio Encode is finished.(EOS)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
						bEndWritingAudioFrame = TRUE;
					}				
					
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encode is failed!", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
					bEndWritingAudioFrame = TRUE;
				}
			}


			if( bEndWritingVideoFrame == FALSE )
			{
				pBitStream = NULL;
				uiBitStreamLen = 0;
				
				uiRet = nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, NULL, NULL, NULL, m_uiVideoCTS, &uiVEncRet);

				if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
				{
					if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
					{
						unsigned int uiVEncPTS = 0;
						uiRet = nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, (int*)&uiBitStreamLen, &uiVEncPTS);
						if( uiRet != 0 )
						{
							m_uiVideoContinueCount++;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
							bEndWritingVideoFrame = TRUE;
						}

						if( m_bSkipMediaCodecTimeStamp )
						{
							if( m_vecSkipMeidaCodecTimeStamp.size() > 0 )
							{
								m_uiVideoCTS = m_vecSkipMeidaCodecTimeStamp[0];
								m_vecSkipMeidaCodecTimeStamp.erase(m_vecSkipMeidaCodecTimeStamp.begin());
							}
							else
							{
								pBitStream = NULL;
								uiBitStreamLen = 0;
								bEndWritingVideoFrame = TRUE;
							}
                            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Video enc No outputtime(%d)", __LINE__, m_uiVideoCTS);
						}
						else
						{
							m_uiVideoCTS = uiVEncPTS;
						}
					}
					else
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode(did not exist output frame)", __LINE__);
						bEndWritingVideoFrame = TRUE;
					}

					
					if(NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_EOS))
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode is finished.(EOS)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
						bEndWritingVideoFrame = TRUE;
					}
					
				}
				else
				{
					m_uiVideoContinueCount++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
					bEndWritingVideoFrame = TRUE;
				}

				if( pBitStream != NULL && uiBitStreamLen != 0)
				{
					unsigned char* pNewBuff = NULL;
					unsigned int uiNewSize = 0;
					if( m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V && NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == NexCodecUtil_CheckByteFormat( pBitStream, uiBitStreamLen) )
					{
						m_perfMonVideoConverter.CheckModuleUnitStart();
						// nexSAL_MemDump(pBitStream, 10);
						
						uiNewSize = NexCodecUtil_ConvertFormat( m_pRawBuffer, WRITER_RAW_FRAME_TEMP_SIZE, pBitStream, uiBitStreamLen, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, 4);
						m_perfMonVideoConverter.CheckModuleUnitEnd();
						if( uiNewSize == -1 )
						{	
							m_uiVideoContinueCount++;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert Annex To Raw Failed ErrorCnt(%d)", __LINE__, m_uiVideoContinueCount);
							bEndWritingVideoFrame = TRUE;
						}
						pNewBuff = m_pRawBuffer;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert AnnexB->Raw bitstream", __LINE__);
					}
					else
					{
						pNewBuff = pBitStream;
						uiNewSize = uiBitStreamLen;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Encode output is Raw bitstream", __LINE__);
					}

					if( m_uiDSINalSize == 0 )
					{
						nexSAL_MemDump(pNewBuff, 100);
						m_uiDSINalSize = NexCodecUtil_GuessNalHeaderLengthSize(pNewBuff, uiNewSize);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] get Frame Nal Size(%d)", __LINE__, m_uiDSINalSize);
					}

					NXBOOL bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoCodecType, pNewBuff, uiNewSize, (void*)&m_uiDSINalSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, ISSEEKABLEFRAME_IDRONLY);
					
					if( m_uiDuration < m_uiVideoCTS )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video frame writer skip(%p %d) Time : %d %d FrameCount : %d IDR(%d)",
						__LINE__, pBitStream, uiBitStreamLen, m_uiVideoCTS, m_uiDuration, m_uiVideoFrameCount, bIDRFrame);
					}
					else
					{
						m_perfMonVideoWrite.CheckModuleUnitStart();
						if( writeOneFrame(NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO, uiNewSize, pNewBuff, m_uiVideoCTS, m_uiVideoCTS, bIDRFrame ? NXFF_FRAME_TYPE_INDEXED : NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
						{
							m_uiVideoContinueCount++;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter writeframe failed(%p %d) (%d)", __LINE__, m_pRawBuffer, uiNewSize, m_uiVideoContinueCount);
							bEndWritingAudioFrame = TRUE;
						}
						m_perfMonVideoWrite.CheckModuleUnitEnd();
						m_uiVideoFrameCount++;
						m_uiVideoContinueCount = 0;
					}
				}
				
			}

		}while(bEndWritingAudioFrame == FALSE || bEndWritingVideoFrame == FALSE);
	
	}

	if (bEndWritingAudioFrame == TRUE && bEndWritingVideoFrame == TRUE && (m_qAudioTotalDuration!=0 || m_qVideoTotalDuration!=0))//Fix CHC-134. Add (m_qAudioTotalDuration!=0 || m_qVideoTotalDuration!=0)
	{
		NxFFWriterLastDuration sLastDuration;
		NXINT64 qAudioFrameDuration = m_qAudioTotalDuration ? m_qAudioTotalDuration-m_qLastAudioCTS : 0;
		NXINT64 qVideoFrameDuration = m_qVideoTotalDuration ? m_qVideoTotalDuration - m_qLastVideoCTS : m_uiDuration - m_qLastVideoCTS;

		sLastDuration.nAudioDuration = qAudioFrameDuration;
		sLastDuration.nVideoDuration = qVideoFrameDuration;
		sLastDuration.nTextDuration = 0;

		NxFFWriterSetExtInfo(m_pWriterHandle, NXFFW_EXTINFO_SET_LAST_FRAME_DURATION, 0, (NXVOID*)&sLastDuration);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] NXFFW_EXTINFO_SET_LAST_FRAME_DURATION (Total:%u A:%lld,%lld,%lld, V:%lld,%lld,%lld) ", __LINE__, m_uiDuration, m_qAudioTotalDuration, m_qLastAudioCTS, sLastDuration.nAudioDuration, m_qVideoTotalDuration, m_qLastVideoCTS, sLastDuration.nVideoDuration);
	}

	*pDuration = m_pWriterHandle->MovieDuration;
	*pDuration = m_uiAudioCTS;

	int iRet = NxFFWriterClose( m_pWriterHandle );
	// for Mantis 7862 problem.
	if (  bCancel && (iRet == 1 || iRet == 2 || iRet == 3) )
	{
		*pDuration = 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] return SUCCESS in these Error Case(%d)", __LINE__, iRet);	
	}
	else if ( iRet != 0 )
	{
		if(iRet == 1 || iRet == 2 || iRet == 3)
		{	
			*pDuration = 0;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] return SUCCESS in these Error Case(%d)", __LINE__, iRet);	
		}

		if( m_hCodecVideoEnc )
		{
			nexCAL_VideoEncoderDeinit( m_hCodecVideoEnc );
			CNexCodecManager::releaseCodec( m_hCodecVideoEnc );	
			m_hCodecVideoEnc = NULL;
			m_MediaCodecInputSurf = NULL;
		}
		
		if( m_hCodecAudioEnc )
		{
			nexCAL_AudioEncoderDeinit( m_hCodecAudioEnc );
			CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
			m_hCodecAudioEnc = NULL;
		}

		nexSAL_FileRemove(m_strTargetPath);

		// MONGTO 20130415 . for removing file. // sometimes file doesn't remove when call remove().
		NEXSALFileHandle handle = nexSAL_FileOpen(m_strTargetPath, NEXSAL_FILE_WRITE);
		if( handle != NEXSAL_INVALID_HANDLE)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] remove file", __LINE__, iRet );
			nexSAL_FileClose(handle);
			nexSAL_FileRemove(m_strTargetPath);
		}
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] WRITER CLOSE FAILED(errcode:%d)", __LINE__, iRet );
		return FALSE;
	}
	
	/*
	NEXSALFileHandle handle = nexSAL_FileOpen(m_strTargetPath, NEXSAL_FILE_READ);
	if( handle != NEXSAL_INVALID_HANDLE)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Record content file was ok(%p) Path:%s", __LINE__, handle, m_strTargetPath);
		nexSAL_FileClose(handle);
	}
	*/
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Video Encoder close start", __LINE__);
	if( m_hCodecVideoEnc )
	{
		nexCAL_VideoEncoderDeinit( m_hCodecVideoEnc );
		CNexCodecManager::releaseCodec( m_hCodecVideoEnc );	
		m_hCodecVideoEnc = NULL;
		m_MediaCodecInputSurf = NULL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Video Encoder close sucessed", __LINE__);
	
	if( m_hCodecAudioEnc )
	{
		nexCAL_AudioEncoderDeinit( m_hCodecAudioEnc );
		CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
		m_hCodecAudioEnc = NULL;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Audio Encoder close sucessed", __LINE__);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] WRITER CLOSE SUCCESS this Dur(%d), RefCount(%d)", __LINE__, *pDuration, GetRefCnt());
	// for mantis 6152
	if( bCancel )
	{
		nexSAL_FileRemove(m_strTargetPath);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Deleting done current export file", __LINE__);
	}

	m_uiAudioCodecType		= 0;
	if( m_pAudioDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pAudioDSIInfo);
		m_pAudioDSIInfo = NULL;
	}
	m_uiAudioDSISize			= 0;

	m_uiVideoCodecType		= 0;
	if( m_pBaseVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pBaseVideoDSIInfo);
		m_pBaseVideoDSIInfo = NULL;
	}
	m_uiBaseVideoDSISize	= 0;

	if( m_pEnhanceVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pEnhanceVideoDSIInfo);
		m_pEnhanceVideoDSIInfo = NULL;
	}

	m_uiEnhanceVideoDSISize	= 0;

	m_Rotate					= 0;
	m_uiSamplingRate			= 0;

	m_uiWidth				= 0;
	m_uiHeight				= 0;
	m_uiDisplayWidth			= 0;
	m_uiDisplayHeight			= 0;

	m_uiFrameRate			= EDITOR_DEFAULT_FRAME_RATE;
	m_uiAudioBitRate		= 0;
	m_uiBitRate				= 0;
	m_uiProfile				= 0;
	m_uiLevel				= 0;

#ifdef VIDEO_DUMP_TEST
	if( g_Video_DumpFrameForMW )
	{
		fclose(g_Video_DumpFrameForMW);
		g_Video_DumpFrameForMW = NULL;
	}

	if( g_Video_DumpFrame )
	{
		fclose(g_Video_DumpFrame);
		g_Video_DumpFrame = NULL;
	}	

	if( g_Video_Dump_YUV )
	{
		fclose(g_Video_Dump_YUV);
		g_Video_Dump_YUV = NULL;
	}
#endif

#ifdef AUDIO_DUMP_PCM
	if( g_Audio_Dump_PCM )
	{
		fclose(g_Audio_Dump_PCM);
		g_Audio_Dump_PCM = NULL;
	}
#endif

#ifdef AUDIO_DUMP_AAC
	if( g_Audio_Dump_AAC )
	{
		fclose(g_Audio_Dump_AAC);
		g_Audio_Dump_AAC = NULL;
	}
#endif
	m_fnSetCropToAchieveResolution = NULL;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] endFileWriter Out", __LINE__ );		
	return TRUE;
}

NXBOOL CNexFileWriter::isStarted()
{
	return m_isStarted;
}

void CNexFileWriter::setAudioOnlyMode(NXBOOL bEnable)
{
	m_bAudioOnly = bEnable;
}

NXBOOL CNexFileWriter::getVideoOnlyMode()
{
	return m_bVideoOnly;
}

void CNexFileWriter::setVideoOnlyMode(NXBOOL bEnable)
{
	m_bVideoOnly = bEnable;
}

NXBOOL CNexFileWriter::isVideoFrameWriteStarted()
{
	return m_uiVideoFrameCount > 0;
}

NXBOOL CNexFileWriter::setAudioCodecInfo(unsigned int uiType, unsigned int uiSampleRate, unsigned int uiNumOfChannel, unsigned int uiBitRate)
{
#ifdef AUDIO_ENCODE_SKIP	
	return TRUE;
#endif

	if( uiType != eNEX_CODEC_A_AAC )
	{
		m_uiAudioCodecType = eNEX_CODEC_UNKNOWN;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setAudioCodecInfo codec type failed(0x%x)", __LINE__, uiType );
		return FALSE;
	}
	m_uiAudioCodecType = eNEX_CODEC_A_AAC;
	m_uiAudioBitRate = uiBitRate;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, " [WrapFileWriter.cpp %d] setAudioCodecInfo codec type (0x%x) (0x%x)", __LINE__, m_uiAudioCodecType, uiType);
	
	m_hCodecAudioEnc = CNexCodecManager::getCodec( NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_ENCODER, uiType);
	if( m_hCodecAudioEnc ==  NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setAudioCodecType get codec failed(%p)", __LINE__, m_hCodecAudioEnc );
		return FALSE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, " [WrapFileWriter.cpp %d] setAudioCodecType get codec Sucessed(%p)", __LINE__, m_hCodecAudioEnc );
	unsigned char*	pConfig		= NULL;
	int				iConfigLen	= 0;

	unsigned int uiRet = nexCAL_AudioEncoderInit( 	m_hCodecAudioEnc, 
												(NEX_CODEC_TYPE)uiType, 
												&pConfig, 
												&iConfigLen, 
												uiSampleRate, 
												uiNumOfChannel, 
												(int)uiBitRate, 
												0);		// unsigned int uClientIdentifier
	m_uiSamplingRate = uiSampleRate;
	m_uiChannels = uiNumOfChannel;
	m_ullEncodedPCMSize = 0;
	m_uiEncodedCTS = 0;

	m_bFirstEncodedFrameSkip = 1;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Audio Encoder init SamplingRate : %d Channels :%d DSI : %d, %p", __LINE__, m_uiSamplingRate, m_uiChannels , iConfigLen, pConfig);

	if( uiRet != 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encoder init failed(%d)", __LINE__, uiRet );
		CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
		m_hCodecAudioEnc = NULL;
		return FALSE;
	}
    if (nexSAL_TraceCondition(NEX_TRACE_CATEGORY_FLOW, 1)) {
	nexSAL_MemDump(pConfig, iConfigLen);
    }
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] setAudioConfing Info(%p, %d)", __LINE__, pConfig, iConfigLen );
	if( setAudioInfo(pConfig, iConfigLen) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioInfo failed(%d)", __LINE__, uiRet );
		CNexCodecManager::releaseCodec( m_hCodecAudioEnc );
		m_hCodecAudioEnc = NULL;
		return FALSE;
	}

	return TRUE;
}
NXBOOL CNexFileWriter::setAudioCodecInfo(unsigned int uiType, unsigned char* pDSI, unsigned int uiDSISize)
{
	switch(uiType)
	{
		case eNEX_CODEC_A_AAC:
		case eNEX_CODEC_A_MP3:
			m_uiAudioCodecType = uiType;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setAudioCodecInfo codec type (0x%x) (0x%x)", __LINE__, m_uiAudioCodecType, uiType);
			break;
		default:
			m_uiAudioCodecType = eNEX_CODEC_UNKNOWN;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setAudioCodecInfo codec type failed(0x%x)", __LINE__, uiType );
			return FALSE;
	}

	m_ullEncodedPCMSize = 0;
	m_uiEncodedCTS = 0;

	m_bFirstEncodedFrameSkip = 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioCodecInfo AudioCodecType(%d)", __LINE__ , m_uiAudioCodecType);
	if( setAudioInfo(pDSI, uiDSISize) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioInfo failed(%d)", __LINE__ );
		return FALSE;
	}
	return TRUE;
}

NXBOOL CNexFileWriter::setVideoCodecInfo(unsigned int uiType, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, unsigned int uiVideoFrameRate, unsigned int uiBitRate, int iProfile, int iLevel)
{
#ifdef VIDEO_ENCODE_SKIP
	return TRUE;
#endif

	setVideoWidthHeight(iWidth, iHeight);

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setVideoCodecInfo(0x%x, %d %d, %d, %d)", __LINE__, uiType, iWidth, iHeight, iProfile, iLevel);

	switch(uiType)
	{
		case 0x50010301:
			m_uiVideoCodecType = eNEX_CODEC_V_H264;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, " [WrapFileWriter.cpp %d] setVideoCodecType codec type (0x%x) (0x%x)", __LINE__, m_uiVideoCodecType, uiType);
			break;
		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
		case eNEX_CODEC_V_MPEG4V:
			m_uiVideoCodecType = uiType;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, " [WrapFileWriter.cpp %d] setVideoCodecType codec type (0x%x) (0x%x)", __LINE__, m_uiVideoCodecType, uiType);
			break;
		default:
			m_uiVideoCodecType = eNEX_CODEC_UNKNOWN;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setVideoCodecType codec type failed(0x%x)", __LINE__, uiType );
			return FALSE;
	}
	
	m_hCodecVideoEnc = CNexCodecManager::getCodec( NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_ENCODER, uiType, iWidth, iHeight, iWidth);
	if( m_hCodecVideoEnc ==  NULL  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setVideoCodecType get codec failed(%p)", __LINE__, m_hCodecVideoEnc );
		return FALSE;
	}

	unsigned char*	pConfig		= NULL;
	int				iConfigLen	= 0;

	m_uiWidth		= iWidth;
	m_uiHeight		= iHeight;
	m_uiDisplayWidth	= iDisplayWidth?iDisplayWidth:iWidth;
	m_uiDisplayHeight	= iDisplayHeight?iDisplayHeight:iHeight;
	m_uiFrameRate	= uiVideoFrameRate;
	m_uiBitRate		= uiBitRate;
	m_uiProfile		= iProfile;
	m_uiLevel		= iLevel;

#ifdef FOR_TEST_ENCODER_PROFILE	
	if( CNexVideoEditor::m_bNexEditorSDK && (m_uiWidth*m_uiHeight) >= (SUPPORTED_ENCODE_WIDTH *SUPPORTED_ENCODE_HEIGHT_1080) )
	{
		nexCAL_SetClientIdentifier(m_hCodecVideoEnc, (void*)this);
		if( m_uiVideoCodecType == eNEX_CODEC_V_H264)
			nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH);
		else if( m_uiVideoCodecType == eNEX_CODEC_V_HEVC)        
			nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE, NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN);
		else if( m_uiVideoCodecType == eNEX_CODEC_V_MPEG4V)
			nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_PROFILE, 0x8);           
	}
#else
	if (iProfile == 0 || iLevel == 0)
	{
		iProfile = NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_BASELINE;
		iLevel = translatedToOMXLevel(NexCodecUtil_AVC_LevelLimits(m_uiWidth, m_uiHeight));
	}

	if (iProfile != 0)
	{
		nexCAL_SetClientIdentifier(m_hCodecVideoEnc, (void*)this);
		switch ( m_uiVideoCodecType )
		{
			case eNEX_CODEC_V_H264:
				nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE, iProfile);
				nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL, iLevel);
				break;

			case eNEX_CODEC_V_HEVC:
				nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE, iProfile);
				nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_HEVC_ENCODER_SET_LEVEL, iLevel);
				break;
				
			case eNEX_CODEC_V_MPEG4V:
				nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_PROFILE, iProfile);
				nexCAL_VideoEncoderSetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_MPEG4V_ENCODER_SET_LEVEL, iLevel);
				break;
		}
	}
#endif

	int iFrameRate = m_uiFrameRate/100;
#if defined( _ANDROID) && !defined(FOR_PROJECT_LGE)
	if(CNexVideoEditor::m_bNexEditorSDK && isQualcommChip() && m_uiVideoCodecType == eNEX_CODEC_V_H264)
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
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d]  getVideoClipCount(%d)",  __LINE__, count);
			}
			SAFE_RELEASE(pProjectMng);
		}

		if(count < 2 && m_uiWidth*m_uiHeight <= 1920*1088)
		{
			if(iFrameRate < 60)
			{
				iFrameRate = 60;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d]  encode flow control (%d -> %d)",  __LINE__, m_uiFrameRate/100, iFrameRate);
			}
		}
	}
#endif

	unsigned int uiRet = nexCAL_VideoEncoderInit(	m_hCodecVideoEnc, 
												(NEX_CODEC_TYPE)uiType, 
												&pConfig,
												&iConfigLen,
												10,	// int iQuality
												m_uiWidth, 
												m_uiHeight,
												m_uiHeight,												
												iFrameRate,
												1,	// unsignec int bCBR
												(int)uiBitRate,
												(void*)this);	// unsigned int uClientIdentifier

	if( uiRet != 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder init failed(%d)", __LINE__, uiRet );
		CNexCodecManager::releaseCodec( m_hCodecVideoEnc );
		m_hCodecVideoEnc = NULL;
		return FALSE;
	}

	m_bVideoEncoderInitToDiscardDSI = TRUE;

	NXINT64 uiInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
	uiRet = nexCAL_VideoEncoderGetProperty(m_hCodecVideoEnc, NEXCAL_PROPERTY_VIDEO_INPUT_BUFFER_TYPE, &uiInputFormat);
	if( uiRet != 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder getProperty failed so We use NV12 Format", __LINE__);
		m_uiEncoderInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Video Encoder getProperty(NV12:0x%x YUV420:0x%x Device:0x%x)", __LINE__,
			NEXCAL_PROPERTY_VIDEO_BUFFER_NV12, 
			NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420,
			uiInputFormat);

#if 0
		// for Galuxy and Note2 korea version
		if(	strcmp(CNexVideoEditor::getDeviceModel(), "SHV-E250S") == 0 ||
			strcmp(CNexVideoEditor::getDeviceModel(), "SHV-E210S") == 0 ||
			strcmp(CNexVideoEditor::getDeviceModel(), "SHV-E210K") == 0 ||
			strcmp(CNexVideoEditor::getDeviceModel(), "SHV-E210L") == 0 ||
			strcmp(CNexVideoEditor::getDeviceModel(), "SM-N900") == 0 ||
			strcmp(CNexVideoEditor::getDeviceModel(), "SM-N900S") == 0 ||
			strcmp(CNexVideoEditor::getDeviceModel(), "SM-N900K") == 0 ||
			strcmp(CNexVideoEditor::getDeviceModel(), "SM-N900L") == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Set NV21 for Galaxy internal serises", __LINE__);
			uiInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
		}
#else
		{
			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor )
			{
				char pValue[64];
				strcpy(pValue, "");
				if( pEditor->getProperty("setExportColorFormat", pValue) == NEXVIDEOEDITOR_ERROR_NONE )
				{
					if( strcmp(pValue, "NV12") == 0 )
						uiInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
					else if( strcmp(pValue, "NV21") == 0 )
						uiInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV21;

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Encoder color format update(0x%x)", __LINE__, uiInputFormat);
				}
				SAFE_RELEASE(pEditor);
			}
		}
#endif

		m_uiEncoderInputFormat = uiInputFormat;

		// for clovertrail(x86) encoding format
		// m_uiEncoderInputFormat = NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;
	}

#ifdef FOR_TEST_MEDIACODEC_DEC	

	nexCAL_VideoDecoderGetProperty( m_hCodecVideoEnc, NEXCAL_PROPERTY_GET_DECODER_INFO, &m_uiMediaCodecUserData);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Get MediaCodec UserData(%lld)", __LINE__, m_uiMediaCodecUserData);

	m_bSkipMediaCodecTimeStamp = TRUE;
	m_vecSkipMeidaCodecTimeStamp.clear();
	
	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		if( pEditor->getDeviceAPILevel() >= 18 ) // Android 4.3 higher
		{
			getMediaCodecInputSurf fInputSurf = (getMediaCodecInputSurf)pEditor->getMediaCodecInputSurf();
			if( fInputSurf )
			{
				m_MediaCodecInputSurf = fInputSurf((void*)m_uiMediaCodecUserData);
				if( m_MediaCodecInputSurf != NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Can use media Codec input Surface(%p)", __LINE__, m_MediaCodecInputSurf);
					CNexVideoEditor::m_iUseSurfaceMediaSource = 1;

					// for x86 Asus memo pad fhd
#ifdef _ANDROID
					if( isx86Device() )
					{
						m_bSkipMediaCodecTimeStamp = TRUE;
						m_fnSetMediaCodecTimeStamp = NULL;
					}
					else
#endif
					{
						m_fnSetMediaCodecTimeStamp = (setMediaCodecsetTimeStamp)pEditor->getMediaCodecSetTimeStampOnSurf();
					}
				}
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Media Codec input Surface(%p)", __LINE__, m_MediaCodecInputSurf);
		}
		m_fnSetCropToAchieveResolution = (setCropToAchieveResolution)pEditor->getMediaCodecSetCropAchieveResolution();
		m_fnResetMediaCodecVideoEncoder = (resetMediaCodecVideoEncoder)pEditor->getMediaCodecResetVideoEncoder();
		SAFE_RELEASE(pEditor);
	}

	if( pConfig == NULL || iConfigLen <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] setVideoConfing using encoded data(%p, %d)", __LINE__, pConfig, iConfigLen );
#ifdef FOR_TEST_MEDIACODEC_ENCODER_DSI
		if( m_MediaCodecInputSurf == NULL )
		{
			if( setVideoFrameForDSI() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] get DSI sucessed from video encoder", __LINE__ );
				nexSAL_MemDump(m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize);
				return TRUE;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] get DSI failed from video encoder", __LINE__ );
			return FALSE;
		}
#endif
		return TRUE;
	}
#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setVideoConfing Info(%p, %d)", __LINE__, pConfig, iConfigLen );
	nexSAL_MemDump(pConfig, iConfigLen);
	if( setBaseVideoInfo(pConfig, iConfigLen) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoInfo failed(%d)", __LINE__, uiRet );
		nexCAL_VideoEncoderDeinit(m_hCodecVideoEnc );
		CNexCodecManager::releaseCodec( m_hCodecVideoEnc );
		m_hCodecVideoEnc = NULL;
		return FALSE;
	}

	m_uiVideoContinueCount = 0;
	
	return TRUE;
}

NXBOOL CNexFileWriter::setVideoCodecInfo(unsigned int uiType, unsigned char* pDSI, int iSize)
{
	if( pDSI == NULL || iSize <= 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setVideoCodecInfo dis info failed(%p %d)", __LINE__, pDSI, iSize);
		return FALSE;
	}

	int iRet = 0;

	if( uiType == eNEX_CODEC_V_H264 )
	{
		NEXCODECUTIL_SPS_INFO SPS_Info;
		iRet = NexCodecUtil_AVC_GetSPSInfo((char *)pDSI, iSize, &SPS_Info, NexCodecUtil_CheckByteFormat( pDSI, iSize));           

		m_uiWidth	= SPS_Info.usWidth;
		m_uiHeight	= SPS_Info.usHeight;		
	}
	else if( uiType == eNEX_CODEC_V_HEVC )
	{
		NEXCODECUTIL_SPS_INFO SPS_Info;
		iRet = NexCodecUtil_HEVC_GetSPSInfo((char *)pDSI, iSize, &SPS_Info, NexCodecUtil_CheckByteFormat( pDSI, iSize));

		m_uiWidth = SPS_Info.usWidth;
		m_uiHeight = SPS_Info.usHeight;
	}
	else if( uiType == eNEX_CODEC_V_MPEG4V )
	{
		NEXCODECUTIL_MPEG4V_DSI_INFO DSI_Info;
		iRet = NexCodecUtil_MPEG4V_GetDSIInfo((NEX_CODEC_TYPE)uiType, (char *)pDSI, iSize, &DSI_Info);

		m_uiWidth = DSI_Info.usWidth;
		m_uiHeight = DSI_Info.usHeight;
	}
	
	if( iRet == 0)
	{
		setVideoWidthHeight(m_uiWidth, m_uiHeight);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] from SPS Width(%d) Height(%d)", __LINE__, m_uiWidth, m_uiHeight);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] get SPSInfo failed(%d)", __LINE__, iRet );
		return FALSE;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setVideoCodecInfo(0x%x, %d %d)", __LINE__, uiType, m_uiWidth, m_uiHeight);

	switch(uiType)
	{
		case eNEX_CODEC_V_H264:
		case eNEX_CODEC_V_HEVC:
		case eNEX_CODEC_V_MPEG4V:
			m_uiVideoCodecType = uiType;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setVideoCodecType codec type (0x%x) (0x%x)", __LINE__, m_uiVideoCodecType, uiType);
			break;
		default:
			m_uiVideoCodecType = eNEX_CODEC_UNKNOWN;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, " [WrapFileWriter.cpp %d] setVideoCodecType codec type failed(0x%x)", __LINE__, uiType );
			return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setVideoConfing Info(%p, %d)", __LINE__, pDSI, iSize );
	nexSAL_MemDump(pDSI, iSize);
	if( setBaseVideoInfo(pDSI, iSize) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoInfo failed", __LINE__ );
		return FALSE;
	}

	m_uiVideoContinueCount = 0;
	
	return TRUE;
}

NXBOOL CNexFileWriter::setAudioInfo(unsigned char* pDSI, unsigned int uiDSISize)
{
	if( pDSI == NULL || uiDSISize <= 0 )
		return FALSE;

	if( m_pAudioDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pAudioDSIInfo);
		m_pAudioDSIInfo = NULL;
	}

	m_pAudioDSIInfo = (unsigned char*)nexSAL_MemAlloc(uiDSISize);
	if( m_pAudioDSIInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio DSI Buffer alloc failed", __LINE__ );
		return FALSE;
	}

	memcpy(m_pAudioDSIInfo, pDSI, uiDSISize);
	m_uiAudioDSISize = uiDSISize;
	return TRUE;
}

static unsigned char g_msm8974_dsiInfo_1920_1088[] = {0x01, 0x00, 0x0E, 0x67, 0x42, 0x80, 0x28, 0xE4, 0x40, 0x3C, 0x01, 0x13, 0x40, 0x36, 0x85, 0x09, 0xA8, 0x01, 0x00, 0x04, 0x68, 0xCE, 0x38, 0x80};
static unsigned char g_msm8974_dsiInfo_1280_736[] = {0x01, 0x00, 0x0E, 0x67, 0x42, 0x80, 0x20, 0xE4, 0x40, 0x28, 0x02, 0xED, 0x00, 0xDA, 0x14, 0x26, 0xA0, 0x01, 0x00, 0x04, 0x68, 0xCE, 0x38, 0x80};
static unsigned char g_msm8974_dsiInfo_640_352[] = {0x01, 0x00, 0x0D, 0x67, 0x42, 0x80, 0x1E, 0xE4, 0x40, 0x50, 0x16, 0xD0, 0x0D, 0xA1, 0x42, 0x6A, 0x01, 0x00, 0x04, 0x68, 0xCE, 0x38, 0x80};
#include "NEXVIDEOEDITOR_SupportDevices.h"

NXBOOL CNexFileWriter::setBaseVideoInfo(unsigned char* pDSI, unsigned int uiDSISize)
{
	if( m_pBaseVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pBaseVideoDSIInfo);
		m_pBaseVideoDSIInfo = NULL;
	}
	m_uiBaseVideoDSISize = 0;
	
	if( pDSI == NULL || uiDSISize <= 0 )
	{
		// for LG G2(msm8974), we can not get the dsi information from omx encoder on this device.
#ifdef _ANDROID
		if( isLGEG2Device() )
		{
			unsigned int uiTempDSISzie = 0;
			unsigned char* pTempDSI = NULL;
			if( m_uiWidth	== 1920 && m_uiHeight == 1088 )
			{
				uiTempDSISzie = sizeof(g_msm8974_dsiInfo_1920_1088);
				pTempDSI = g_msm8974_dsiInfo_1920_1088;
				// memcpy(m_BaseVideoDSIInfo, g_msm8974_dsiInfo_1920_1088, m_uiBaseVideoDSISize);
			}
			else if( m_uiWidth	== 1280 && m_uiHeight == 736 )
			{
				uiTempDSISzie = sizeof(g_msm8974_dsiInfo_1280_736);
				pTempDSI = g_msm8974_dsiInfo_1280_736;
				// memcpy(m_BaseVideoDSIInfo, g_msm8974_dsiInfo_1280_736, m_uiBaseVideoDSISize);
			}
			else if( m_uiWidth	== 640 && m_uiHeight == 368 )
			{
				uiTempDSISzie = sizeof(g_msm8974_dsiInfo_640_352);
				pTempDSI = g_msm8974_dsiInfo_640_352;
				// memcpy(m_BaseVideoDSIInfo, g_msm8974_dsiInfo_640_352, m_uiBaseVideoDSISize);
			}
			else
			{
				return FALSE;
			}

			if( pTempDSI != NULL && uiTempDSISzie > 0 )
			{
				m_pBaseVideoDSIInfo = (unsigned char*)nexSAL_MemAlloc(uiTempDSISzie);
				if( m_pBaseVideoDSIInfo == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] setBaseVideoInfo DSI buffer alloc failed", __LINE__);
					return FALSE;
				}
				memcpy(m_pBaseVideoDSIInfo, pTempDSI, uiTempDSISzie);
				m_uiBaseVideoDSISize = uiTempDSISzie;
			}

			for( int i = 0; i < m_uiBaseVideoDSISize - 4; i++)
			{
				if( (m_pBaseVideoDSIInfo[i] & 0x1F) == 0x07 )
				{
					unsigned int uiProfile = m_pBaseVideoDSIInfo[i + 1];
					uiProfile = (uiProfile << 8) | m_pBaseVideoDSIInfo[i + 2];
					uiProfile = (uiProfile << 8) | m_pBaseVideoDSIInfo[i + 3];
					uiProfile = (uiProfile << 8) | 0xFF;
					setVideoProfileLevel(uiProfile);
					break;
				}
			}			
			return TRUE;
		}
#endif
		return FALSE;
	}

	if(m_uiVideoCodecType == eNEX_CODEC_V_H264 || m_uiVideoCodecType == eNEX_CODEC_V_HEVC)
	{
		if( NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == NexCodecUtil_CheckByteFormat( pDSI, uiDSISize) )
		{
			unsigned char*	pTmpDSI		= NULL;
			int			iTmpDSISize 	= 0;
			int			iDSISize			= 0;
		
			pTmpDSI = (unsigned char*)nexSAL_MemAlloc(uiDSISize + 100);
			if( pTmpDSI == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] setBaseVideoInfo DSI Tmp buffer alloc failed", __LINE__);
				return FALSE;
			}
		
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Input AnnexB DSI Info", __LINE__);

			nexSAL_MemDump(pDSI, uiDSISize);
		
			int iNalSize = 0;
			if( pDSI[0] == 0x00 && pDSI[1] == 0x00 && pDSI[2] == 0x01 )
			{
				iNalSize = 3;
			}
			else if( pDSI[0] == 0x00 && pDSI[1] == 0x00 && pDSI[2] == 0x00 && pDSI[3] == 0x01 )
			{
				iNalSize = 4;
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Check NAL size error", __LINE__);
				nexSAL_MemFree(pTmpDSI);
				return FALSE;
			}		
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Check NAL size(%d)", __LINE__, iNalSize);

			iTmpDSISize = NexCodecUtil_ConvertFormat( pTmpDSI, uiDSISize+100, pDSI, uiDSISize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, iNalSize);
			if( iTmpDSISize <= 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert frame failed", __LINE__);
				nexSAL_MemFree(pTmpDSI);
				return FALSE;
			}

			nexSAL_MemDump(pTmpDSI, iTmpDSISize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Check NAL size(%d)", __LINE__, iNalSize);
		
			m_pBaseVideoDSIInfo = (unsigned char*)nexSAL_MemAlloc(iTmpDSISize + 100);
			if( m_pBaseVideoDSIInfo == NULL )
			{
				nexSAL_MemFree(pTmpDSI);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] setBaseVideoInfo DSI buffer alloc failed", __LINE__);
				return FALSE;
			}

			int			iSize		= 0;
			int 			iIndex		= 0;
			unsigned char nalType		= 0;

			if(m_uiVideoCodecType == eNEX_CODEC_V_H264)
			{
				while(iIndex < iTmpDSISize - iNalSize)
				{
					iSize = 0;
					if( iNalSize == 3 )
					{
						iSize |= ((unsigned int)pTmpDSI[iIndex]<<16) & 0x00ff0000;
						iSize |= ((unsigned int)pTmpDSI[iIndex+1]<<8 ) & 0x0000ff00;
						iSize |= ((unsigned int)pTmpDSI[iIndex+2]    ) & 0x000000ff;
					}
					else
					{
						iSize |= ((unsigned int)pTmpDSI[iIndex]<<24) & 0xff000000;
						iSize |= ((unsigned int)pTmpDSI[iIndex+1]<<16) & 0x00ff0000;
						iSize |= ((unsigned int)pTmpDSI[iIndex+2]<<8 ) & 0x0000ff00;
						iSize |= ((unsigned int)pTmpDSI[iIndex+3]    ) & 0x000000ff;
					}

					iIndex += iNalSize;
					nalType = pTmpDSI[iIndex];
					if( (nalType & 0x1F )== 0x07 )
					{
						m_pBaseVideoDSIInfo[iDSISize++] = 0x01;
						m_pBaseVideoDSIInfo[iDSISize++] = 0x00;	
					
						m_pBaseVideoDSIInfo[iDSISize++] = iSize;
						memcpy(&m_pBaseVideoDSIInfo[iDSISize], &(pTmpDSI[iIndex]), iSize);

						unsigned int uiProfile = m_pBaseVideoDSIInfo[iDSISize + 1];
						uiProfile = (uiProfile << 8) | m_pBaseVideoDSIInfo[iDSISize + 2];
						uiProfile = (uiProfile << 8) | m_pBaseVideoDSIInfo[iDSISize + 3];
						uiProfile = (uiProfile << 8) | 0xFF;

						iDSISize += iSize;
						iIndex += iSize;
						setVideoProfileLevel(uiProfile);
					}
					else if ( (nalType & 0x1F) == 0x08 )
					{
						m_pBaseVideoDSIInfo[iDSISize++] = 0x01;
						m_pBaseVideoDSIInfo[iDSISize++] = 0x00;	
				
						m_pBaseVideoDSIInfo[iDSISize++] = iSize;
						memcpy(&m_pBaseVideoDSIInfo[iDSISize], &(pTmpDSI[iIndex]), iSize);
						iDSISize += iSize;
						iIndex += iSize;
					}
					else
					{
						iIndex += iSize;		
					}
				}
				m_uiBaseVideoDSISize = iDSISize;     
			}
			else if(m_uiVideoCodecType == eNEX_CODEC_V_HEVC)
			{
				unsigned int numOfArrays = 0;
				unsigned int numVPS = 0;
				unsigned int numSPS = 0;
	                	unsigned int numPPS = 0;

				iDSISize = 22;
	      	  		while(iIndex < iTmpDSISize - iNalSize)
       	 		{
	        			iSize = 0;
       	 			if( iNalSize == 3 )
        				{
        					iSize |= ((unsigned int)pTmpDSI[iIndex]<<16) & 0x00ff0000;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+1]<<8 ) & 0x0000ff00;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+2]    ) & 0x000000ff;
	        			}
	        			else
	        			{
       	 				iSize |= ((unsigned int)pTmpDSI[iIndex]<<24) & 0xff000000;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+1]<<16) & 0x00ff0000;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+2]<<8 ) & 0x0000ff00;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+3]    ) & 0x000000ff;
	        			}
       	 			iIndex += iNalSize;
        				nalType = (pTmpDSI[iIndex] >> 1) & 0x3F;
        				if( nalType == 32 )		// VPS
	        				numVPS++;
       	 			if( nalType == 33 )		// SPS
        					numSPS++;
        				if( nalType == 34 )		// PPS
        					numPPS++;

	        			iIndex += iSize;
       	 		}
        			if(numVPS)
        				numOfArrays++;
	        		if(numSPS)
       	 			numOfArrays++;
        			if(numPPS)
        				numOfArrays++;

	        		iIndex = 0;
       	 		m_pBaseVideoDSIInfo[iDSISize++] = (unsigned char)numOfArrays;
        
	        		while(iIndex < iTmpDSISize - iNalSize)
       	 		{
        				unsigned int i;
        				iSize = 0;
        				if( iNalSize == 3 )
	        			{
       	 				iSize |= ((unsigned int)pTmpDSI[iIndex]<<16) & 0x00ff0000;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+1]<<8 ) & 0x0000ff00;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+2]    ) & 0x000000ff;
        				}
	        			else
       	 			{
        					iSize |= ((unsigned int)pTmpDSI[iIndex]<<24) & 0xff000000;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+1]<<16) & 0x00ff0000;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+2]<<8 ) & 0x0000ff00;
        					iSize |= ((unsigned int)pTmpDSI[iIndex+3]    ) & 0x000000ff;
	        			}

       	 			iIndex += iNalSize;
        				nalType = (pTmpDSI[iIndex] >> 1) & 0x3F;
        				if( nalType == 32 )		// VPS
        				{
	        				m_pBaseVideoDSIInfo[iDSISize++] = (unsigned char) nalType;
	
       	 				m_pBaseVideoDSIInfo[iDSISize++] = numVPS >> 8;
        					m_pBaseVideoDSIInfo[iDSISize++] = numVPS & 0xFf;
	
       	 				for(i = 0; i < numVPS; i++)
        					{
        						m_pBaseVideoDSIInfo[iDSISize++] = iSize >> 8;
        						m_pBaseVideoDSIInfo[iDSISize++] = iSize & 0xFF;

        						memcpy(&m_pBaseVideoDSIInfo[iDSISize], &(pTmpDSI[iIndex]), iSize);

        						iDSISize += iSize;
	        					iIndex += iSize;
       	 				}
        				}
	        			else if( nalType == 33 )	// SPS
       	 			{
        					m_pBaseVideoDSIInfo[iDSISize++] = (unsigned char) nalType;

        					m_pBaseVideoDSIInfo[iDSISize++] = numSPS >> 8;
        					m_pBaseVideoDSIInfo[iDSISize++] = numSPS & 0xFF;

        					for(i = 0; i < numSPS; i++)
	        				{
       	 					m_pBaseVideoDSIInfo[iDSISize++] = iSize >> 8;
        						m_pBaseVideoDSIInfo[iDSISize++] = iSize & 0xFF;

        						memcpy(&m_pBaseVideoDSIInfo[iDSISize], &(pTmpDSI[iIndex]), iSize);

        						iDSISize += iSize;
        						iIndex += iSize;
	        				}
       	 			}
        				else if ( nalType == 34 )	// PPS
        				{
	        				m_pBaseVideoDSIInfo[iDSISize++] = (unsigned char) nalType;

       	 				m_pBaseVideoDSIInfo[iDSISize++] = numPPS >> 8;
        					m_pBaseVideoDSIInfo[iDSISize++] = numPPS & 0xFF;
	
       	 				for(i = 0; i < numPPS; i++)
        					{
        						m_pBaseVideoDSIInfo[iDSISize++] = iSize >> 8;
        						m_pBaseVideoDSIInfo[iDSISize++] = iSize & 0xFF;

        						memcpy(&m_pBaseVideoDSIInfo[iDSISize], &(pTmpDSI[iIndex]), iSize);

        						iDSISize += iSize;
	        					iIndex += iSize;
       	 				}
        				}
        				else
        				{
	        				iIndex += iSize;
       	 			}
        			}
        			m_uiBaseVideoDSISize = iDSISize;

	        		NEXCODECUTIL_SPS_INFO SPS_Info;
        		
       	 		int iRet = (NEXVIDEOEDITOR_ERROR)NexCodecUtil_HEVC_GetSPSInfo((char *)m_pBaseVideoDSIInfo+22, m_uiBaseVideoDSISize-22, &SPS_Info, NexCodecUtil_CheckByteFormat( m_pBaseVideoDSIInfo+22, m_uiBaseVideoDSISize-22));
        			if( iRet == 0)
        			{
	        			setVideoProfileLevel((NXUINT32)SPS_Info.eProfile);
       	 		}		

				{
					unsigned char *tmp;
					int i = 0;
					iDSISize = 0;
					m_pBaseVideoDSIInfo[iDSISize++] = 0x01;													// configurationVersion = 1
					m_pBaseVideoDSIInfo[iDSISize]   = 0;
					m_pBaseVideoDSIInfo[iDSISize]   |= (SPS_Info.general_profile_space & 0x03) << 6;		// general_profile_space
					m_pBaseVideoDSIInfo[iDSISize]   |= (SPS_Info.general_tier_flag & 0x01) << 5;			// general_tier_flag
					m_pBaseVideoDSIInfo[iDSISize++] |= (SPS_Info.general_tier_flag & 0x1f);			// general_profile_idc

					for(i = 0; i < 32; i += 8)
					{
						m_pBaseVideoDSIInfo[iDSISize]  = 0;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 0] & 0x01) << 7;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 1] & 0x01) << 6;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 2] & 0x01) << 5;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 3] & 0x01) << 4;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 4] & 0x01) << 3;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 5] & 0x01) << 2;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 6] & 0x01) << 1;
						m_pBaseVideoDSIInfo[iDSISize] |= (SPS_Info.general_profile_compatibility_flag[i + 7] & 0x01) << 0;
						iDSISize++;
					}

					/* general_constraint_indicator_flags */
					tmp = &pTmpDSI[iIndex];
					tmp += 2 + /* nal_unit_header */
						   1 + /* sps_video_parameter_set_id(4b) + sps_max_sub_layers_minus1(3) + sps_temporal_id_nesting_flag(1b) */
						   1 + /* general_profile_space(2b) + general_tier_flag(1b) + general_profile_idc(5b) */
						   4;  /* general_profile_compatibility_flag(1b) * 32 */
					for(i = 0; i < 6; i++)
					{
						unsigned char val = *tmp++;
	
						if( (*(tmp - 2) == 0x00) && (*(tmp - 3) == 0x00) && (val == 0x03) )
							val = *tmp++;
						m_pBaseVideoDSIInfo[iDSISize++] = val;
					}

					m_pBaseVideoDSIInfo[iDSISize++] = SPS_Info.general_level_idc;		// general_level_idc

					m_pBaseVideoDSIInfo[iDSISize] = 0xf0;									// reserved = '1111'b;


					// min_spatial_segmentation_idc
					if( (SPS_Info.vui_parameters_present_flag) && (SPS_Info.bitstream_restriction_flag) )
					{
						m_pBaseVideoDSIInfo[iDSISize++] |= (SPS_Info.min_spatial_segmentation_idc >> 8) & 0x0f;
						m_pBaseVideoDSIInfo[iDSISize++]  = (SPS_Info.min_spatial_segmentation_idc & 0xff);
					}
					else
					{
						m_pBaseVideoDSIInfo[iDSISize++] |= 0;
						m_pBaseVideoDSIInfo[iDSISize++]  = 0;
					}

					m_pBaseVideoDSIInfo[iDSISize]    = 0xfc;									// reserved = '111111'b
					m_pBaseVideoDSIInfo[iDSISize++] |= 0x00;									// parallelismType : 0 = unknown, 3 = 'entropy coding synchronizaton'
					m_pBaseVideoDSIInfo[iDSISize]    = 0xfc;									// reserved = '111111'b
					m_pBaseVideoDSIInfo[iDSISize++] |= (SPS_Info.chroma_format_idc & 0x03);			// chromaFormat;

					m_pBaseVideoDSIInfo[iDSISize]    = 0xf8;									// reserved = '11111'b
					m_pBaseVideoDSIInfo[iDSISize++] |= (SPS_Info.bit_depth_luma_minus8 & 0x03);		// bitDepthLumaMinus8

					m_pBaseVideoDSIInfo[iDSISize]    = 0xf8;									// reserved = '11111'b
					m_pBaseVideoDSIInfo[iDSISize++] |= (SPS_Info.bit_depth_chroma_minus8 & 0x03);	// bitDepthChromaMinus8
					m_pBaseVideoDSIInfo[iDSISize++]  = 0;										// avgFrameRate constantFrameRate 
					m_pBaseVideoDSIInfo[iDSISize++]  = 0;

					m_pBaseVideoDSIInfo[iDSISize]    = 0;													// constantFrameRate = may or may not be of constant frame rate
					m_pBaseVideoDSIInfo[iDSISize]   |= ((SPS_Info.sps_max_sub_layers_minus1 + 1) & 0x07) << 3;	// numTemporalLayers
					m_pBaseVideoDSIInfo[iDSISize]   |= ((SPS_Info.sps_temporal_id_nesting_flag) & 0x01) << 2;	// temporalIdNested
					m_pBaseVideoDSIInfo[iDSISize++] |= ((iNalSize - 1) & 0x03);				// lengthSizeMinusOne
				}                
			}

			if( pTmpDSI != NULL )
			{
				nexSAL_MemFree(pTmpDSI);
				pTmpDSI = NULL;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Out Raw DSI Info", __LINE__);
			nexSAL_MemDump(m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize);
			return TRUE;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Input Raw DSI Info", __LINE__);
	
	int 				tempBufferLen 	= uiDSISize;
	unsigned char* 	pTemDSI		= pDSI;
    if (nexSAL_TraceCondition(NEX_TRACE_CATEGORY_FLOW, 1)) {
	nexSAL_MemDump(pTemDSI, tempBufferLen);
    }
	const NXINT32 nConfigRecorderLen = 5; // [shoh][2012.04.06] Configuration recorder length is 5 bytes and master 3bits of the 6th byte should be 0.
	if(m_uiVideoCodecType == eNEX_CODEC_V_H264)
	{
		if((NexCodecUtil_AVC_IsConfigFromRecordType(pTemDSI, tempBufferLen) == TRUE) && (tempBufferLen > nConfigRecorderLen))
		{
			pTemDSI += nConfigRecorderLen;
			tempBufferLen -= nConfigRecorderLen;
		}
	}
    if (nexSAL_TraceCondition(NEX_TRACE_CATEGORY_FLOW, 1)) {
	nexSAL_MemDump(pTemDSI, tempBufferLen);
    }
	m_pBaseVideoDSIInfo = (unsigned char*)nexSAL_MemAlloc(tempBufferLen + 100);
	if( m_pBaseVideoDSIInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] setBaseVideoInfo DSI buffer alloc failed", __LINE__);
		return FALSE;
	}

	memcpy(m_pBaseVideoDSIInfo, pTemDSI, tempBufferLen);
	m_uiBaseVideoDSISize = tempBufferLen;

	if(m_uiVideoCodecType == eNEX_CODEC_V_H264)
	{
		for( int i = 0; i < m_uiBaseVideoDSISize - 4; i++)
		{
			if( (m_pBaseVideoDSIInfo[i] & 0x1F) == 0x07 )
			{
				unsigned int uiProfile = m_pBaseVideoDSIInfo[i + 1];
				uiProfile = (uiProfile << 8) | m_pBaseVideoDSIInfo[i + 2];
				uiProfile = (uiProfile << 8) | m_pBaseVideoDSIInfo[i + 3];
				uiProfile = (uiProfile << 8) | 0xFF;
				setVideoProfileLevel(uiProfile);
				break;
			}
		}	
	}
	else if(m_uiVideoCodecType == eNEX_CODEC_V_HEVC)
	{
		NEXCODECUTIL_SPS_INFO SPS_Info;
		
		int iRet = (NEXVIDEOEDITOR_ERROR)NexCodecUtil_HEVC_GetSPSInfo((char *)m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize, &SPS_Info, NexCodecUtil_CheckByteFormat( m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize));
		if( iRet == 0)
		{
			setVideoProfileLevel((NXUINT32)SPS_Info.eProfile);
		}		
	}	
	else if(m_uiVideoCodecType == eNEX_CODEC_V_MPEG4V)
	{
		NEXCODECUTIL_MPEG4V_DSI_INFO DSI_Info;
		int iRet = NexCodecUtil_MPEG4V_GetDSIInfo((NEX_CODEC_TYPE)m_uiVideoCodecType, (char *)m_pBaseVideoDSIInfo, m_uiBaseVideoDSISize, &DSI_Info);

		if( iRet == 0)
		{
			setVideoProfileLevel((NXUINT32)DSI_Info.uProfile);
		}
	}

#ifdef VIDEO_DUMP_TEST
	char strDumpFile[256];
	sprintf(strDumpFile, "%sForMW.framedmp", m_strTargetPath);
	if( g_Video_DumpFrameForMW == NULL )
		g_Video_DumpFrameForMW = fopen(strDumpFile, "wb");	

	sprintf(strDumpFile, "%sForMKV.framedmp", m_strTargetPath);
	if( g_Video_DumpFrame == NULL )
		g_Video_DumpFrame = fopen(strDumpFile, "wb");	
	

	sprintf(strDumpFile, "%s.yuvdmp", m_strTargetPath);
	// if( g_Video_Dump_YUV == NULL )
	// 	g_Video_Dump_YUV = fopen(strDumpFile, "wb");
	// gVideoFrameCount = 0;


	if( g_Video_DumpFrameForMW )
	{
 		fwrite(&m_uiBaseVideoDSISize, 1, sizeof(unsigned int), g_Video_DumpFrameForMW);
		fwrite(m_pBaseVideoDSIInfo, 1, m_uiBaseVideoDSISize, g_Video_DumpFrameForMW);
	}

	if( g_Video_DumpFrame )
	{
		int iStart = 0x01000000;
		fwrite(&iStart, 1, 4, g_Video_DumpFrame);
		fwrite(m_pBaseVideoDSIInfo+3, 1, 9, g_Video_DumpFrame);
		fwrite(&iStart, 1, 4, g_Video_DumpFrame);
		fwrite(m_pBaseVideoDSIInfo+15, 1, 4, g_Video_DumpFrame);
	}
#endif	
	return TRUE;
}

NXBOOL CNexFileWriter::setEnhanceVideoInfo(unsigned char* pDSI, unsigned int uiDSISize)
{
	if( pDSI == NULL || uiDSISize <= 0 )
		return FALSE;

	if( m_pEnhanceVideoDSIInfo != NULL )
	{
		nexSAL_MemFree(m_pEnhanceVideoDSIInfo);
		m_pEnhanceVideoDSIInfo = NULL;
	}

	m_pEnhanceVideoDSIInfo = (unsigned char*)nexSAL_MemAlloc(uiDSISize);
	if( m_pEnhanceVideoDSIInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video enhance DSI Buffer alloc failed", __LINE__ );
		return FALSE;
	}

	memcpy(m_pEnhanceVideoDSIInfo, pDSI, uiDSISize);
	m_uiEnhanceVideoDSISize = uiDSISize;
	m_isTS					= TRUE;
	return TRUE;
}

NXBOOL CNexFileWriter::setVideoWidthHeight(unsigned int uiWidth, unsigned int uiHeight)
{
	m_uiWidth	= uiWidth;
	m_uiHeight	= uiHeight;
	return TRUE;
}

NXBOOL CNexFileWriter::setVideoProfileLevel(unsigned int uiProfile)
{
	m_uiProfileLevelID = uiProfile;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] setVideoProfileLevel(0x%x)", __LINE__, uiProfile);
	return TRUE;
}

NXBOOL CNexFileWriter::writeOneFrame(NXFFW_MEDIA_TYPE MediaType, unsigned int samplesize, unsigned char *samplebuf, unsigned int CTS, unsigned int DTS, NXFF_FRAME_TYPE FrameType)
{
	CAutoLock m(m_Lock);
	
	long  lWriterRet = 0;

	if( MediaType == NXFFW_MEDIA_TYPE_AUDIO )
	{
		m_llWriteAudioTotal += samplesize;
	}
	else
	{
		m_llWriteVideoTotal += samplesize;

		if( m_uiVideoFrameCount == 0 && m_uiVideoStartGap == 0 && CTS != 0 && DTS != 0 )
		{
			m_uiVideoStartGap = CTS;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Start time stamp re-order(%d %d %d %d)", 
				__LINE__, m_uiVideoFrameCount, m_uiVideoStartGap, CTS, DTS );
		}

		// for JIRA 3034
		if( m_uiVideoFrameCount != 0 && m_uiVideoStartGap != 0 && CTS == 0 && DTS == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Encoded frame was ignored because abnormal CTS(% %d %d %d %d)", 
				__LINE__, m_uiVideoFrameCount, m_uiVideoStartGap, CTS, DTS );
			return TRUE;
		}

		/*
		if( CTS != 0 && DTS != 0 )
		{
			CTS -= m_uiVideoStartGap;
			DTS -= m_uiVideoStartGap;
		}
		*/
	}

	if( m_uiDuration <= CTS &&  MediaType == NXFFW_MEDIA_TYPE_AUDIO)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Write frame skip (%d %d)", __LINE__, m_uiDuration, CTS);
		return TRUE;
	}

	lWriterRet = NxFFWriteFrame(m_pWriterHandle, MediaType, samplesize, samplebuf, CTS, DTS, FrameType);
	if( lWriterRet != 0 )
	{
		switch ( lWriterRet )
		{
			case 10:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] _TRGT_ADDFRAME_FAIL_MEMFULL(%d)", __LINE__, lWriterRet );
				break;
			case 11:
				m_uiVideoContinueCount = 8;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] _TRGT_ADDFRAME_FAIL_FILESIZE(%d)", __LINE__, lWriterRet );
				break;
			case 13:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] _TRGT_ADDFRAME_FAIL_TIME FULL(%d)", __LINE__, lWriterRet );
				break;
			default:
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] _TRGT_ADDFRAME_FAIL(%d)", __LINE__, lWriterRet );
				break;
		};
		return FALSE;
	}

	if (MediaType == NXFFW_MEDIA_TYPE_AUDIO)
		m_qLastAudioCTS = DTS;
	else if (MediaType == NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO)
		m_qLastVideoCTS = DTS;

	return TRUE;
}

//RYU 20130729 USE NATIVE CODEC.
NXBOOL CNexFileWriter::setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize)
{
#ifdef AUDIO_ENCODE_SKIP

    m_uiAudioCTS = uiCTS;
	return TRUE;
#endif

	if( m_isStarted == FALSE || m_pWriterHandle == NULL) return FALSE;
	if( pDuration == NULL || pSize == NULL ) return FALSE;
	if( m_hCodecAudioEnc == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrame AAC Encoder Handle failed(%p)", __LINE__, m_hCodecAudioEnc );
		return FALSE;
	}

	*pDuration	= 0;
	*pSize		= 0;

	unsigned char*	pBitStream		= NULL;
	unsigned int		uiBitStreamLen	= 0;
	unsigned int		uiAEncRet		= 0;
	unsigned int		uiRet			= 0;

#ifdef AUDIO_DUMP_PCM
	if( g_Audio_Dump_PCM )
		fwrite(pFrame, 1, uiFrameSize, g_Audio_Dump_PCM);
#endif

	do
	{
		uiRet = nexCAL_AudioEncoderEncode(m_hCodecAudioEnc, pFrame, uiFrameSize , &pBitStream, &uiBitStreamLen, &uiAEncRet);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] nexCAL_AudioEncoderEncode Ret (%d, %d %d) CTS(%d)", __LINE__, uiRet, uiAEncRet, uiBitStreamLen, uiCTS);
		if( uiRet != 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encode Failed(%d %d)", __LINE__, uiRet, uiAEncRet);
			return FALSE;
		}

		if( NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_ENCODING_SUCCESS))
		{
			if(NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_OUTPUT_EXIST))
			{
				//m_uiAudioCTS					= uiCTS;

				if(m_bPaused == FALSE)
				{
					if( !m_bAudioOnly && m_uiAudioCTS > m_uiVideoCTS )
					{
						int iAVGap = m_uiAudioCTS - m_uiVideoCTS;
						if( iAVGap > 10)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Wait Video(A:%d V:%d %d)", __LINE__, m_uiAudioCTS, m_uiVideoCTS, iAVGap);
							nexSAL_TaskSleep(iAVGap);
						}
					}
				}

				if( m_bFirstEncodedFrameSkip > 0 )
				{
					m_bFirstEncodedFrameSkip--;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1 , "[WrapFileWriter.cpp %d] First encoded audio frame skip", __LINE__);
				}
				else
				{
					if( writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiBitStreamLen - 7, pBitStream + 7, m_uiEncodedCTS, m_uiEncodedCTS, NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
					{
						return FALSE;
					}

#ifdef AUDIO_DUMP_AAC
					if( g_Audio_Dump_AAC )
					{
						fwrite(pBitStream, 1, uiBitStreamLen, g_Audio_Dump_AAC);
					}
#endif

					*pDuration	= m_pWriterHandle->MovieDuration;		
					*pSize		= m_pWriterHandle->MovieSize;	

					m_ullEncodedPCMSize +=AUDIO_ENCODE_MAX_SIZE;
					m_uiAudioCTS = m_uiEncodedCTS;
					m_uiEncodedCTS =  (unsigned int)(m_ullEncodedPCMSize*1000/(m_uiSamplingRate*m_uiChannels*2));
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1 , "[WrapFileWriter.cpp %d] Audio One Frame Writen Len(%d) time(%d %d) E(%lld, %d)", __LINE__, 
						uiBitStreamLen - 7, *pDuration, uiCTS, m_ullEncodedPCMSize, m_uiEncodedCTS);	
				}
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encode Failed(%d %d)", __LINE__, uiRet, uiAEncRet);
		}
	}while(NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_NEXT_INPUT)== FALSE);

	return TRUE;
}


NXBOOL CNexFileWriter::setAudioFrameForVoice(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize)
{
#ifdef AUDIO_ENCODE_SKIP
	return TRUE;
#endif

	if( m_isStarted == FALSE || m_pWriterHandle == NULL) return FALSE;
	if( pDuration == NULL || pSize == NULL ) return FALSE;
	if( m_hCodecAudioEnc == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrame AAC Encoder Handle failed(%p)", __LINE__, m_hCodecAudioEnc );
		return FALSE;
	}

	*pDuration	= 0;
	*pSize		= 0;

	unsigned char*	pBitStream		= NULL;
	unsigned int		uiBitStreamLen	= 0;
	unsigned int		uiAEncRet		= 0;
	unsigned int		uiRet			= 0;

	if( (uiCTS - m_uiAudioCTS) > 26 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encode Time to late(%d)", __LINE__, uiCTS - m_uiAudioCTS);


#ifdef AUDIO_DUMP_PCM
	if( g_Audio_Dump_PCM )
		fwrite(pFrame, 1, uiFrameSize, g_Audio_Dump_PCM);
#endif

	do
	{
		uiRet = nexCAL_AudioEncoderEncode(m_hCodecAudioEnc, pFrame, uiFrameSize , &pBitStream, &uiBitStreamLen, &uiAEncRet);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 3, "[WrapFileWriter.cpp %d][%d] nexCAL_AudioEncoderEncode Ret (%d, %d)", __LINE__, uiRet, uiAEncRet);

		if( uiRet != 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encode Failed(%d %d)", __LINE__, uiRet, uiAEncRet);
			return FALSE;
		}

		if( NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_ENCODING_SUCCESS))
		{
			if(NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_OUTPUT_EXIST))
			{
				m_uiAudioCTS					= uiCTS;	
				if( m_uiAudioCTS > m_uiVideoCTS )
				{
					int iAVGap = m_uiAudioCTS - m_uiVideoCTS;
					if( iAVGap > 200 )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Wait Video(A:%d V:%d %d)", __LINE__, m_uiAudioCTS, m_uiVideoCTS, iAVGap);
						nexSAL_TaskSleep(iAVGap);
					}
				}

				if( writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiBitStreamLen - 7, pBitStream + 7, m_uiEncodedCTS, m_uiEncodedCTS, NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
				{
					return FALSE;
				}

#ifdef AUDIO_DUMP_AAC
				if( g_Audio_Dump_AAC )
					fwrite(pBitStream, 1, uiBitStreamLen, g_Audio_Dump_AAC);
#endif

				*pDuration	= m_pWriterHandle->MovieDuration;		
				*pSize		= m_pWriterHandle->MovieSize;	

				m_ullEncodedPCMSize += 2048;
				m_uiEncodedCTS =  (unsigned int)(m_ullEncodedPCMSize*1000/(m_uiSamplingRate*m_uiChannels*2));
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 4, "[WrapFileWriter.cpp %d] Audio One Frame Writen Len(%d) time(%d %d) E(%lld, %d)", __LINE__, uiBitStreamLen - 7, *pDuration, uiCTS, m_ullEncodedPCMSize, m_uiEncodedCTS);	
				
			}
		}


	}while(NEXCAL_CHECK_AENC_RET(uiAEncRet, NEXCAL_AENC_NEXT_INPUT)== FALSE);
	return TRUE;
}

NXBOOL CNexFileWriter::setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
#ifdef AUDIO_ENCODE_SKIP
    m_uiAudioCTS = uiCTS;
	return TRUE;
#endif

	if( m_isStarted == FALSE || m_pWriterHandle == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrame Wait for writer start", __LINE__ );
		return FALSE;
	}
	/*
	if( m_hCodecAudioEnc == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrame AAC Encoder Handle failed(%p)", __LINE__, m_hCodecAudioEnc );
		return FALSE;
	}
	*/
	NXBOOL bRet = FALSE;

	if( (uiCTS - m_uiAudioCTS) > 26 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Audio Encode Time to late(%d)", __LINE__, uiCTS - m_uiAudioCTS);

	// for mantis 10783
	if( m_uiAudioCodecType == eNEX_CODEC_A_AAC )
	{
		bRet = writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiFrameSize-7, pFrame+7, uiCTS, uiCTS, NXFF_FRAME_TYPE_UNINDEXED);
	}
	else
	{
		bRet = writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiFrameSize, pFrame, uiCTS, uiCTS, NXFF_FRAME_TYPE_UNINDEXED);
	}
	
	if( bRet == FALSE)
	{
		return FALSE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Write AudioFrame (Ret:%d) (%p, %d, %d)", __LINE__, bRet, pFrame, uiFrameSize, uiCTS);
	m_uiAudioCTS = uiCTS;
	return bRet;
}

/* return 
	0: sucess
	1: failed
	2: wait
	3: drop
*/	
int CNexFileWriter::setAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 1, "[WrapFileWriter.cpp %d] setAudioFrameWithoutEncode in(ID: %d curID : %d lastCTS : %d cts : %d)", 
		__LINE__, uiID, m_uiDECurrentAudioClipID, m_uiAudioCTS, uiCTS);
	
	CAutoLock m(m_lockAudioDE);	
	
	if( m_isStarted == FALSE || m_pWriterHandle == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrameWithoutEncode failed(start: %d handle : %d)", __LINE__, m_isStarted, m_pWriterHandle);
		return 1;
	}

	if( m_uiDECurrentAudioClipID == INVALID_CLIP_ID )
	{
		if( m_uiAudioCTS == 0 && uiCTS > 500 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrameWithoutEncode Wait(ID: %d cts : %d)", __LINE__, uiID, uiCTS);
			return 2;
		}

		if( (m_uiAudioCTS + 500) < uiCTS  )
		{
			if( pFrame == NULL && uiFrameSize == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrameWithoutEncode(ID: %d cts : %d v:%d a:%d)", 
					__LINE__, uiID, uiCTS, m_uiVideoCTS, m_uiAudioCTS);
				m_uiDECurrentAudioClipID = uiID;
				return 2;
			}				
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrameWithoutEncode Wait(ID: %d cts : %d)", __LINE__, uiID, uiCTS);
			return 2;
		}
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrameWithoutEncode Start(ID: %d cts : %d)", __LINE__, uiID, uiCTS);
		m_uiDECurrentAudioClipID = uiID;
	}
	else if( m_uiDECurrentAudioClipID == uiID )
	{
		if( pFrame == NULL && uiFrameSize == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrameWithoutEncode End(ID: %d cts : %d a : %d v : %d)", 
				__LINE__, uiID, uiCTS, m_uiAudioCTS, m_uiVideoCTS);

			if( (m_uiAudioCTS + 50) < uiCTS )
			{
				m_uiAudioCTS = m_uiVideoCTS;
				return 2;
			}

			if( (uiCTS + 500) >  m_uiDuration )
			{
				m_uiAudioCTS = m_uiDuration;
			}
			m_uiDECurrentAudioClipID = INVALID_CLIP_ID;
			return 3;
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setAudioFrameWithoutEncode Wait(ID: %d cts : %d) %d", __LINE__, uiID, uiCTS, m_uiDECurrentAudioClipID);
		return 2;
	}
	
	if( uiCTS > m_uiVideoCTS && (uiCTS + 250) <  m_uiDuration )
	{
		return 2;
	}

	NXBOOL bRet = FALSE;
	// for mantis 10783
	if( m_uiAudioCodecType == eNEX_CODEC_A_AAC )
	{
		bRet = writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiFrameSize-7, pFrame+7, uiCTS, uiCTS, NXFF_FRAME_TYPE_UNINDEXED);
	}
	else
	{
		bRet = writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiFrameSize, pFrame, uiCTS, uiCTS, NXFF_FRAME_TYPE_UNINDEXED);
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] ID(%d) Write AudioFrame (Ret:%d) (%p, %d, %d)", __LINE__, uiID, bRet, pFrame, uiFrameSize, uiCTS);
	m_uiAudioCTS = uiCTS;
	return 0;	
}

int CNexFileWriter::writeAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 1, "[WrapFileWriter.cpp %d] ID(%d) writeAudioFrameWithoutEncode in(lastCTS : %d cts : %d)", 
		__LINE__, uiID, m_uiAudioCTS, uiCTS);
	
	CAutoLock m(m_lockAudioDE);	

	if( m_isStarted == FALSE || m_pWriterHandle == NULL)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] ID(%d) writeAudioFrameWithoutEncode failed(start: %d handle : %d)", __LINE__, uiID, m_isStarted, m_pWriterHandle);
		return 1;
	}

	if( pFrame == NULL && uiFrameSize == 0 )
	{
		m_uiAudioCTS = uiCTS;
		return 0;
	}
	
	if( m_uiAudioCodecType == eNEX_CODEC_A_AAC )
	{
		writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiFrameSize-7, pFrame+7, uiCTS, uiCTS, NXFF_FRAME_TYPE_UNINDEXED);
	}
	else
	{
		writeOneFrame(NXFFW_MEDIA_TYPE_AUDIO, uiFrameSize, pFrame, uiCTS, uiCTS, NXFF_FRAME_TYPE_UNINDEXED);
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] ID(%d) writeAudioFrameWithoutEncode (%p, %d, %d)", __LINE__, uiID, pFrame, uiFrameSize, uiCTS);
	m_uiAudioCTS = uiCTS;
	return 0;	
}

NXBOOL CNexFileWriter::setVideoFrameForDSI()
{
	if( m_hCodecVideoEnc == NULL ) return FALSE;

	unsigned int uiMemSize = m_uiWidth*m_uiHeight*3/2;
	unsigned int uiTime = 0;
	unsigned char* pTemp = (unsigned char*)nexSAL_MemAlloc(uiMemSize);

	if( pTemp == NULL ) return FALSE;

	memset(pTemp, 0x00, uiMemSize);

	int				iCount			= 20; // retryCount
	unsigned int		uiVEncRet		= 0;
	unsigned char*	pBitStream		= NULL;
	int 				uiBitStreamLen	= 0;
	unsigned int		uiVEncPTS		= 0;

	while( iCount >= 0 )
	{
		iCount--;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc one frame for DSI time(%d)", __LINE__, uiTime);
		unsigned int uiRet = nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, pTemp, 0, 0, uiTime, &uiVEncRet);
		uiTime += 33;

		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
		{
			if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
			{
				uiRet = nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
				if( uiRet != 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput for DSI(%d %d)", __LINE__, uiRet, uiVEncRet);
					continue;
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput for DSI(%d %d)", __LINE__, uiRet, uiVEncRet);

				if( m_uiBaseVideoDSISize <= 0 && m_isStarted == FALSE )
				{
					setBaseVideoInfo(pBitStream, uiBitStreamLen);

					nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, NULL, 0, 0, uiTime, &uiVEncRet);
					if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
					{
						if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
						{
							nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
						}
					}
					
					nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, NULL, 0, 0, uiTime, &uiVEncRet);
					if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
					{
						if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
						{
							nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
						}
					}
					
					m_fnResetMediaCodecVideoEncoder((void*)m_uiMediaCodecUserData);
					// video encode need reset
					nexSAL_MemFree(pTemp);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode for DSI End", __LINE__);
					return TRUE;
				}			
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode for DSI(did not exist output frame)", __LINE__);
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode for DSI Failed(%d %d) ", __LINE__, uiRet, uiVEncRet);
			break;
		}		
	}
	nexSAL_MemFree(pTemp);
	return FALSE;
}

/* return 
	0: sucess
	1: failed
	2: wait
	3: drop
*/	
int CNexFileWriter::setVideoFrameForDSIwithInputSurf()
{
	if( m_hCodecVideoEnc == NULL ) return 3;

	unsigned int 		uiTime			= 0;

	unsigned char*	pBitStream		= NULL;
	int 				uiBitStreamLen	= 0;
	unsigned int		uiVEncRet		= 0;

	unsigned int		uiVEncPTS		= 0;

	unsigned char		pTempBuffer[50];

	unsigned int uiRet = nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, pTempBuffer, 0, 0, uiTime, &uiVEncRet);
	uiTime += 33;

	if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
	{
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
		{
			uiRet = nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
			if( uiRet != 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput for DSI Failed(%d %d)", __LINE__, uiRet, uiVEncRet);
				return 1;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput for DSI sucessed pFrame(%p) Size(%d) CTS(%d)", 
				__LINE__, pBitStream, uiBitStreamLen, uiVEncPTS);

			if( m_uiBaseVideoDSISize <= 0 && m_isStarted == FALSE )
			{
				setBaseVideoInfo(pBitStream, uiBitStreamLen);
				if( startFileWriter() == FALSE )
				{
					CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_START_FAIL);
					return 2;
				}

				nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, NULL, 0, 0, uiTime, &uiVEncRet);
				m_fnResetMediaCodecVideoEncoder((void*)m_uiMediaCodecUserData);
				return 0;
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode for DSI (did not exist output frame)", __LINE__);
		return 1;
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode for DSI Failed(%d %d)", __LINE__, uiRet, uiVEncRet);
	return 2;
}

NXBOOL CNexFileWriter::checkEncoderEOS(unsigned int uiTime)
{
	NXINT32 uiRet = 0;
	NXUINT8* pBitStream = NULL;
	unsigned int uiVEncRet = 0;
	unsigned int uiBitStreamLen = 0;
	unsigned int uiVEncPTS = 0;

	int iTryTime = 0;

	while( iTryTime < 100 )
	{

		uiRet = nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, NULL, NULL, NULL, m_uiVideoCTS, &uiVEncRet);

		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
		{
			if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
			{
				unsigned int uiVEncPTS = 0;
				uiRet = nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, (int*)&uiBitStreamLen, &uiVEncPTS);
				if( uiRet != 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
				}
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode(did not exist output frame)", __LINE__);
			}

			if(NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_EOS))
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode is finished.(EOS)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
				return TRUE;
			}
		}
		else
		{
			m_uiVideoContinueCount++;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
		}
		nexSAL_TaskSleep(10);
		iTryTime += 10;
	}
	return FALSE;
}

NXBOOL CNexFileWriter::setBaseVideoFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet)
{
#ifdef VIDEO_ENCODE_SKIP
    m_uiVideoCTS = uiCTS;
    m_isStarted = TRUE;
	return TRUE;
#endif

	if( m_pWriterHandle == NULL) return FALSE;
	if( pDuration == NULL || pSize == NULL ) return FALSE;
	if( m_hCodecVideoEnc == NULL ) return FALSE;
	// for mantis 10225
	if( m_uiBaseVideoDSISize > 0 && m_isStarted == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Writer start failed(%d %p %d)", __LINE__, uiCTS, pFrame, uiFrameSize);
		return FALSE;
	}

	if( m_uiVideoContinueCount > 0 )
	{
		if( m_uiVideoContinueCount == 8 )
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_NOT_ENOUGHT_DISK_SPACE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		else
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_VIDEO_ENC_FAIL);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		nexSAL_TaskSleep(100);
		return FALSE;
	}

	
	*pDuration	= 0;
	*pSize		= 0;

	unsigned char*	pBitStream		= NULL;
	int 				uiBitStreamLen	= 0;
	unsigned int		uiVEncRet		= 0;

	unsigned int		uiVEncPTS		= 0;

	/*
	if( (uiCTS - m_uiVideoCTS) > 34 )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode Time to late(%d)", __LINE__, uiCTS - m_uiVideoCTS);
	*/

	uiVEncRet = 0;
	int iRetryTime = 1000;
	NXBOOL bIDRFrame = FALSE;
	while(iRetryTime > 0 && NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_NEXT_INPUT) == FALSE )
	{
		m_perfMonVideoEncode.CheckModuleUnitStart();
		unsigned int uiRet = nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, pFrame, 0, 0, uiCTS, &uiVEncRet);
		m_perfMonVideoEncode.CheckModuleUnitEnd();
		
		iRetryTime -= 50;
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
		{
			if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
			{
				m_perfMonVideoGetOutput.CheckModuleUnitStart();
				uiRet = nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
				m_perfMonVideoGetOutput.CheckModuleUnitEnd();
				if( uiRet != 0 )
				{
					m_uiVideoContinueCount++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
					continue;
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput sucessed pFrame(%p) Size(%d) CTS(%d)",
					__LINE__, pBitStream, uiBitStreamLen, uiVEncPTS);

				if( m_uiBaseVideoDSISize <= 0 && m_isStarted == FALSE )
				{
					setBaseVideoInfo(pBitStream, uiBitStreamLen);
					if( startFileWriter() == FALSE )
					{
						CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_START_FAIL);
					}
					return TRUE;
				}			

				if(uiBitStreamLen == 0)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video enc No Output size(%d, %d)", __LINE__, uiBitStreamLen, m_uiVideoCTS);
					return FALSE;
				}

				if(pEncodeRet)
					*pEncodeRet = TRUE;
				m_uiVideoCTS = uiVEncPTS;

				unsigned char* pNewBuff = NULL;
				unsigned int uiNewSize = 0;
				if( m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V && NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == NexCodecUtil_CheckByteFormat( pBitStream, uiBitStreamLen) )
				{
					m_perfMonVideoConverter.CheckModuleUnitStart();
					// nexSAL_MemDump(pBitStream, 10);
					uiNewSize = NexCodecUtil_ConvertFormat( m_pRawBuffer, WRITER_RAW_FRAME_TEMP_SIZE, pBitStream, uiBitStreamLen, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, 4);
					m_perfMonVideoConverter.CheckModuleUnitEnd();
					if( uiNewSize == -1 )
					{	
						m_uiVideoContinueCount++;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert Annex To Raw Failed ErrorCnt(%d)", __LINE__, m_uiVideoContinueCount);
						continue;
					}
					pNewBuff = m_pRawBuffer;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert AnnexB->Raw bitstream", __LINE__);
				}
				else
				{
					pNewBuff = pBitStream;
					uiNewSize = uiBitStreamLen;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Encode output is Raw bitstream", __LINE__);
				}	

				if( m_uiDSINalSize == 0 )
				{
                    if (nexSAL_TraceCondition(NEX_TRACE_CATEGORY_FLOW, 1)) {
					nexSAL_MemDump(pNewBuff, 100);
                    }
					m_uiDSINalSize = NexCodecUtil_GuessNalHeaderLengthSize(pNewBuff, uiNewSize);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] get Frame Nal Size(%d)", __LINE__, m_uiDSINalSize);
				}	

				bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoCodecType, pNewBuff, uiNewSize, (void*)&m_uiDSINalSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, ISSEEKABLEFRAME_IDRONLY);
                
                if( m_uiDuration < m_uiVideoCTS )
                {
                    *pDuration    = m_pWriterHandle->MovieDuration;
                    *pSize        = m_pWriterHandle->MovieSize;
                    
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] Video frame writer skip(0x%x %d) Time : %d %d FrameCount : %d IDR(%d)",
                                    __LINE__, pBitStream, uiBitStreamLen, m_uiVideoCTS, m_uiDuration, m_uiVideoFrameCount, bIDRFrame);
                    return TRUE;
                }
                
				m_perfMonVideoWrite.CheckModuleUnitStart();
				if( writeOneFrame(NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO, uiNewSize, pNewBuff, m_uiVideoCTS, m_uiVideoCTS, bIDRFrame ? NXFF_FRAME_TYPE_INDEXED : NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
				{
					// JIRA 3186
					if( m_uiVideoContinueCount != 8 )
						m_uiVideoContinueCount++;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter writeframe failed(%p %d) (%d)", __LINE__, m_pRawBuffer, uiNewSize, m_uiVideoContinueCount);
					continue;
				}
				m_perfMonVideoWrite.CheckModuleUnitEnd();
				m_uiVideoFrameCount++;
				m_uiVideoContinueCount = 0;

				// nexSAL_MemDump(m_pRawBuffer, 10);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Video Encode one frame(%p %d) Time : %d FrameCount : %d IDR(%d)", 
					__LINE__, pBitStream, uiBitStreamLen, m_uiVideoCTS, m_uiVideoFrameCount, bIDRFrame);			

			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] Video Encode(did not exist output frame)", __LINE__);
			}
		}
		else
		{
			m_uiVideoContinueCount++;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
			return FALSE;
		}
	}

#ifdef VIDEO_DUMP_TEST
	if( g_Video_DumpFrameForMW )
	{
		fwrite(&uiNewSize, 1, sizeof(unsigned int), g_Video_DumpFrameForMW);
		fwrite(&uiCTS, 1, sizeof(unsigned int), g_Video_DumpFrameForMW);
		fwrite(m_pRawBuffer, 1, uiNewSize, g_Video_DumpFrameForMW);
	}

	if( g_Video_DumpFrame )
	{
		fwrite(pBitStream, 1, uiBitStreamLen, g_Video_DumpFrame);
	}

	if( g_Video_Dump_YUV )
	{
		gVideoFrameCount++;
		fwrite(pFrame, 1, uiFrameSize, g_Video_Dump_YUV);

		if( gVideoFrameCount == 10 )
		{
			fclose(g_Video_Dump_YUV);
			g_Video_Dump_YUV = NULL;
		}
	}
#endif

	*pDuration	= m_pWriterHandle->MovieDuration;		
	*pSize		= m_pWriterHandle->MovieSize;	
	return TRUE;
}

NXBOOL CNexFileWriter::setBaseVideoFrame(unsigned int uiCTS, NXBOOL bEnd, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet)
{
	unsigned int uiTempTime = nexSAL_GetTickCount();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrame in(cts : %d)", __LINE__, uiCTS);
	
	if( m_pWriterHandle == NULL) return FALSE;
	if( pDuration == NULL || pSize == NULL ) return FALSE;
	if( m_hCodecVideoEnc == NULL ) return FALSE;
	if( pEncodeRet == NULL ) return FALSE;
	// for mantis 10225
	if( m_uiBaseVideoDSISize > 0 && m_isStarted == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Writer start failed(%d %d)", __LINE__, uiCTS, bEnd);
		return FALSE;
	}

	if( m_uiVideoContinueCount > 0 )
	{
		if( m_uiVideoContinueCount == 8 )
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_NOT_ENOUGHT_DISK_SPACE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		else
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_VIDEO_ENC_FAIL);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		nexSAL_TaskSleep(100);
		return FALSE;
	}
	
	*pDuration	= 0;
	*pSize		= 0;

	unsigned char*	pBitStream		= NULL;
	int 				uiBitStreamLen	= 0;
	unsigned int		uiVEncRet		= 0;

	unsigned int		uiVEncPTS		= 0;

	unsigned char		pTempBuffer[50];

	if( m_bSkipMediaCodecTimeStamp )
	{
		m_vecSkipMeidaCodecTimeStamp.insert(m_vecSkipMeidaCodecTimeStamp.end(), uiCTS);
	}

ENCODER_RETRY:

	m_perfMonVideoEncode.CheckModuleUnitStart();
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrame call encoder(cts : %d)", __LINE__, uiCTS);
	unsigned int uiRet = nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, bEnd ? NULL : pTempBuffer, 0, 0, uiCTS, &uiVEncRet);

 	m_perfMonVideoEncode.CheckModuleUnitEnd();
	if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
	{
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
		{
			m_perfMonVideoGetOutput.CheckModuleUnitStart();
			uiRet = nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
			m_perfMonVideoGetOutput.CheckModuleUnitEnd();
			if( uiRet != 0 )
			{
				m_uiVideoContinueCount++;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
				return FALSE;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput sucessed pFrame(%p) Size(%d) CTS(%d)", 
				__LINE__, pBitStream, uiBitStreamLen, uiVEncPTS);

			if( m_uiBaseVideoDSISize <= 0 && m_isStarted == FALSE )
			{
				setBaseVideoInfo(pBitStream, uiBitStreamLen);
				if( startFileWriter() == FALSE )
				{
					CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_WRITER_START_FAIL);
				}
				m_bVideoEncoderInitToDiscardDSI = FALSE;
				return TRUE;
			}

			if(uiBitStreamLen == 0)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video enc No Output size(%d, %d)", __LINE__, uiBitStreamLen, m_uiVideoCTS);
				return FALSE;
			}

			if(m_bVideoEncoderInitToDiscardDSI)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video enc first output drop after resume(%d, %d)", __LINE__, m_uiVideoCTS, uiVEncPTS);
				m_bVideoEncoderInitToDiscardDSI = FALSE;
				goto ENCODER_RETRY;
				return FALSE;
			}

			*pEncodeRet = TRUE;
			// for x86 Asus memo pad fhd
			if( m_bSkipMediaCodecTimeStamp )
			{
				if( m_vecSkipMeidaCodecTimeStamp.size() > 0 )
				{
					m_uiVideoCTS = m_vecSkipMeidaCodecTimeStamp[0];
					m_vecSkipMeidaCodecTimeStamp.erase(m_vecSkipMeidaCodecTimeStamp.begin());
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video enc No Output Time(%d)", __LINE__, m_uiVideoCTS);
					m_uiVideoContinueCount++;
					return FALSE;
				}

				//(KM-5308:Chrome book) : It can cause a problem when setting high profile to encoder.
				if (NULL == m_fnSetMediaCodecTimeStamp)
					uiVEncPTS = m_uiVideoCTS;
			}
			else
			{
				m_uiVideoCTS = uiVEncPTS;
			}
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode(did not exist output frame)", __LINE__);
			return TRUE;
		}
	}
	else
	{
		m_uiVideoContinueCount++;
#ifdef _ANDROID
		if( isSamsungMSM8974Device() || isXperiaM4AquaOrGalaxyA7() )
		{
			if( m_uiVideoContinueCount > 0 )
			{
				CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_VIDEO_ENC_FAIL);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
				nexSAL_TaskSleep(100);
				return FALSE;
			}
		}
#endif
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
		return FALSE;
	}

	unsigned char* pNewBuff = NULL;
	unsigned int uiNewSize = 0;
	if( m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V && NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == NexCodecUtil_CheckByteFormat( pBitStream, uiBitStreamLen) )
	{
		m_perfMonVideoConverter.CheckModuleUnitStart();
		uiNewSize = NexCodecUtil_ConvertFormat( m_pRawBuffer, WRITER_RAW_FRAME_TEMP_SIZE, pBitStream, uiBitStreamLen, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, 4);
		m_perfMonVideoConverter.CheckModuleUnitEnd();
		if( uiNewSize == -1 )
		{	
			m_uiVideoContinueCount++;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert Annex To Raw Failed ErrorCnt(%d)", __LINE__, m_uiVideoContinueCount);
			return TRUE;
		}
		pNewBuff = m_pRawBuffer;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert AnnexB->Raw bitstream", __LINE__);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d]  frame convert to raw(%d -> %d)", __LINE__, uiBitStreamLen, uiNewSize);
	}
	else
	{
		pNewBuff = pBitStream;
		uiNewSize = uiBitStreamLen;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Encode output is Raw bitstream", __LINE__);
	}

	// nexSAL_MemDump(pBitStream, uiBitStreamLen > 64 ? 64 : uiBitStreamLen);
	// nexSAL_MemDump(m_pRawBuffer, uiNewSize > 64 ? 64 : uiNewSize);

	if( m_uiDSINalSize == 0 )
	{
		nexSAL_MemDump(pNewBuff, 100);
		m_uiDSINalSize = NexCodecUtil_GuessNalHeaderLengthSize(pNewBuff, uiNewSize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] get Frame Nal Size(%d)", __LINE__, m_uiDSINalSize);
	}

	NXBOOL bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoCodecType, pNewBuff, uiNewSize, (void*)&m_uiDSINalSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, ISSEEKABLEFRAME_IDRONLY);
	
    if( m_uiDuration < m_uiVideoCTS )
    {
        *pDuration	= m_pWriterHandle->MovieDuration;
        *pSize		= m_pWriterHandle->MovieSize;
        
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video frame writer skip(%p %d) Time : %d %d FrameCount : %d IDR(%d)",
                        __LINE__, pBitStream, uiBitStreamLen, m_uiVideoCTS, m_uiDuration, m_uiVideoFrameCount, bIDRFrame);
        return TRUE;
    }

    m_perfMonVideoWrite.CheckModuleUnitStart();
	if( writeOneFrame(NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO, uiNewSize, pNewBuff, uiVEncPTS, m_uiVideoCTS, bIDRFrame ? NXFF_FRAME_TYPE_INDEXED : NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
	{
		// JIRA 3186
		if( m_uiVideoContinueCount != 8 )
			m_uiVideoContinueCount++;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter writeframe failed(%p %d) (%d), %d", __LINE__, m_pRawBuffer, uiNewSize, m_uiVideoContinueCount, bIDRFrame);
		return TRUE;
	}
	m_perfMonVideoWrite.CheckModuleUnitEnd();
	m_uiVideoFrameCount++;

	m_uiVideoContinueCount = 0;

	*pDuration	= m_pWriterHandle->MovieDuration;		
	*pSize		= m_pWriterHandle->MovieSize;	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode one frame(%p %d) Time : %d,%d FrameCount : %d IDR(%d)", 
		__LINE__, pBitStream, uiBitStreamLen, m_uiVideoCTS, uiVEncPTS, m_uiVideoFrameCount, bIDRFrame);
	return TRUE;
}

NXBOOL CNexFileWriter::setBaseVideoFrame(unsigned int uiCTS, void* pBuffer, unsigned int* pDuration, unsigned int* pSize)
{
	unsigned int uiTempTime = nexSAL_GetTickCount();
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrame in(cts : %d)", __LINE__, uiCTS);
	
	if( m_isStarted == FALSE || m_pWriterHandle == NULL) return FALSE;
	if( pDuration == NULL || pSize == NULL ) return FALSE;
	if( m_hCodecVideoEnc == NULL ) return FALSE;

	if( m_uiVideoContinueCount > 0 )
	{
		if( m_uiVideoContinueCount == 8 )
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_NOT_ENOUGHT_DISK_SPACE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		else
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_VIDEO_ENC_FAIL);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		nexSAL_TaskSleep(100);
		return FALSE;
	}
	
	*pDuration	= 0;
	*pSize		= 0;

	unsigned char*	pBitStream		= NULL;
	int 				uiBitStreamLen	= 0;
	unsigned int		uiVEncRet		= 0;

	unsigned int		uiVEncPTS		= 0;

	unsigned char		pTempBuffer[50];

	m_perfMonVideoEncode.CheckModuleUnitStart();
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrame call encoder(cts : %d)", __LINE__, uiCTS);
	unsigned int uiRet = nexCAL_VideoEncoderEncode(m_hCodecVideoEnc, 0, (unsigned char*)0xff01, (unsigned char*)pBuffer, uiCTS, &uiVEncRet);

 	m_perfMonVideoEncode.CheckModuleUnitEnd();
	if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_ENCODING_SUCCESS) )
	{
		if( NEXCAL_CHECK_VENC_RET(uiVEncRet, NEXCAL_VENC_OUTPUT_EXIST) )
		{
			m_perfMonVideoGetOutput.CheckModuleUnitStart();
			uiRet = nexCAL_VideoEncoderGetOutput(m_hCodecVideoEnc, &pBitStream, &uiBitStreamLen, &uiVEncPTS);
			m_perfMonVideoGetOutput.CheckModuleUnitEnd();
			if( uiRet != 0 )
			{
				m_uiVideoContinueCount++;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
				return FALSE;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Enc getoutput sucessed CTS(%d)", __LINE__, uiVEncPTS);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode(did not exist output frame)", __LINE__);
			return TRUE;
		}
	}
	else
	{
		m_uiVideoContinueCount++;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encode Failed(%d %d) ErrorCnt(%d)", __LINE__, uiRet, uiVEncRet, m_uiVideoContinueCount);
		return FALSE;
	}

	unsigned char* pNewBuff = NULL;
	unsigned int uiNewSize = 0;
	if( m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V && NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == NexCodecUtil_CheckByteFormat( pBitStream, uiBitStreamLen) )
	{
		m_perfMonVideoConverter.CheckModuleUnitStart();
		// nexSAL_MemDump(pBitStream, 10);
		uiNewSize = NexCodecUtil_ConvertFormat( m_pRawBuffer, WRITER_RAW_FRAME_TEMP_SIZE, pBitStream, uiBitStreamLen, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, 4);
		m_perfMonVideoConverter.CheckModuleUnitEnd();
		if( uiNewSize == -1 )
		{	
			m_uiVideoContinueCount++;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert Annex To Raw Failed ErrorCnt(%d)", __LINE__, m_uiVideoContinueCount);
			return FALSE;
		}
		pNewBuff = m_pRawBuffer;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert AnnexB->Raw bitstream", __LINE__);
	}
	else
	{
		pNewBuff = pBitStream;
		uiNewSize = uiBitStreamLen;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Encode output is Raw bitstream", __LINE__);
	}

	if( m_uiDSINalSize == 0 )
	{
		nexSAL_MemDump(pNewBuff, 100);
		m_uiDSINalSize = NexCodecUtil_GuessNalHeaderLengthSize(pNewBuff, uiNewSize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] get Frame Nal Size(%d)", __LINE__, m_uiDSINalSize);
	}

	NXBOOL bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoCodecType, pNewBuff, uiNewSize, (void*)&m_uiDSINalSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, ISSEEKABLEFRAME_IDRONLY);
	
	m_perfMonVideoWrite.CheckModuleUnitStart();
	if( writeOneFrame(NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO, uiNewSize, pNewBuff, uiVEncPTS, uiVEncPTS, bIDRFrame ? NXFF_FRAME_TYPE_INDEXED : NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
	{
		// JIRA 3186
		if( m_uiVideoContinueCount != 8 )
			m_uiVideoContinueCount++;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter writeframe failed(%p %d) (%d)", __LINE__, m_pRawBuffer, uiNewSize, m_uiVideoContinueCount);
		return FALSE;
	}
	m_perfMonVideoWrite.CheckModuleUnitEnd();
	m_uiVideoFrameCount++;
	m_uiVideoContinueCount = 0;

	*pDuration	= m_pWriterHandle->MovieDuration;		
	*pSize		= m_pWriterHandle->MovieSize;	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 1, "[WrapFileWriter.cpp %d] Video Encode one frame(%p %d) Time : %d FrameCount : %d IDR(%d)", 
		__LINE__, pBitStream, uiBitStreamLen, uiVEncPTS, m_uiVideoFrameCount, bIDRFrame);
	
	m_uiVideoCTS = uiVEncPTS;	
	return TRUE;
}

/* return 
	0: sucess
	1: failed
	2: wait
	3: drop
*/	
int CNexFileWriter::setBaseVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrameWithoutEncode in(ID: %d curID : %d lastCTS(%d %d)cts : %u dts : %u)",
		__LINE__, uiID, m_uiDECurrentVideoClipID, m_uiAudioCTS, m_uiVideoCTS, uiCTS, uiDTS);

	CAutoLock m(m_lockVideoDE);
	
	if( m_isStarted == FALSE || m_pWriterHandle == NULL) return 1;
	if( pDuration == NULL || pSize == NULL ) return 1;

	if( m_uiDECurrentVideoClipID == INVALID_CLIP_ID )
	{
		if( m_uiVideoCTS == 0 &&  uiDTS > 500 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrameWithoutEncode Wait(ID: %d dts : %u)", __LINE__, uiID, uiDTS);
			return 2;
		}

		if( (m_uiVideoCTS + 500) < uiDTS  )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrameWithoutEncode Wait(ID: %d dts : %u)", __LINE__, uiID, uiDTS);
			return 2;
		}
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrameWithoutEncode Start(ID: %d dts : %u)", __LINE__, uiID, uiDTS);
		m_uiDECurrentVideoClipID = uiID;
	}
	else if( m_uiDECurrentVideoClipID == uiID )
	{
		if( pBuffer == NULL && iBufferSize == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrameWithoutEncode End(ID: %d dts : %u)", __LINE__, uiID, uiDTS);
			m_uiVideoCTS = uiDTS;
			m_uiDECurrentVideoClipID = INVALID_CLIP_ID;
			return 3;
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setBaseVideoFrameWithoutEncode Wait(ID: %d dts : %u)", __LINE__, uiID, uiDTS);
		return 2;
	}

	if( m_bVideoOnly )
	{
	}
	else
	{
		if( uiDTS > m_uiAudioCTS + 500 )
			nexSAL_TaskSleep(1);
	}

	if( m_uiVideoContinueCount > 0 )
	{
		if( m_uiVideoContinueCount == 8 )
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_NOT_ENOUGHT_DISK_SPACE);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		else
		{
			CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_VIDEO_ENC_FAIL);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		}
		nexSAL_TaskSleep(100);
		return 1;
	}

	if( m_uiDSINalSize == 0 )
	{
		nexSAL_MemDump((unsigned char*)pBuffer, 100);
		m_uiDSINalSize = NexCodecUtil_GuessNalHeaderLengthSize((unsigned char*)pBuffer, iBufferSize);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] get Frame Nal Size(%d)", __LINE__, m_uiDSINalSize);
	}
	
	*pDuration	= 0;
	*pSize		= 0;

	unsigned char* pNewBuff = NULL;
	unsigned int uiNewSize = 0;
	if( m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V && NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == NexCodecUtil_CheckByteFormat((unsigned char*)pBuffer, iBufferSize) )
	{
		m_perfMonVideoConverter.CheckModuleUnitStart();
		uiNewSize = NexCodecUtil_ConvertFormat( m_pRawBuffer, WRITER_RAW_FRAME_TEMP_SIZE, (unsigned char*)pBuffer, iBufferSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, m_uiDSINalSize);
		m_perfMonVideoConverter.CheckModuleUnitEnd();
		if( uiNewSize == -1 )
		{	
			m_uiVideoContinueCount++;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert Annex To Raw Failed ErrorCnt(%d)", __LINE__, m_uiVideoContinueCount);
			return 1;
		}
		pNewBuff = m_pRawBuffer;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert AnnexB->Raw bitstream", __LINE__);
	}
	else
	{
		pNewBuff =  (unsigned char*)pBuffer;
		uiNewSize = iBufferSize;
	}

	if( m_uiVideoFrameCount == 0 )
	{
 		NXUINT32 uNewFrameSize = 0;
		NXUINT32 uConfigStreamOffset = 0;
		NXUINT8 *pConfigStream = NULL;

		if(m_uiVideoCodecType == eNEX_CODEC_V_H264)
		{
			pConfigStream = NexCodecUtil_AVC_NAL_GetConfigStream(pNewBuff, uiNewSize, (NXINT32)m_uiDSINalSize, (NXINT32*)&uNewFrameSize);
			if (pConfigStream != NULL)
			{
				if (uiNewSize != uNewFrameSize)
				{
 					uConfigStreamOffset = (NXUINT32)(pConfigStream - pNewBuff);
					pNewBuff = pConfigStream + uNewFrameSize;
					uiNewSize -= uConfigStreamOffset + uNewFrameSize;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter Frame info(%p %d) (%p %d)", 
					__LINE__, pBuffer, iBufferSize, pNewBuff, uiNewSize);
			}
		}
		else
		{
			pConfigStream = NexCodecUtil_HEVC_NAL_GetConfigStream(pNewBuff, uiNewSize, (NXINT32)m_uiDSINalSize, (NXINT32*)&uNewFrameSize);
			if (pConfigStream != NULL)
			{
				if (uiNewSize != uNewFrameSize)
				{
 					uConfigStreamOffset = (NXUINT32)(pConfigStream - pNewBuff);
					pNewBuff = pConfigStream + uNewFrameSize;
					uiNewSize -= uConfigStreamOffset + uNewFrameSize;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter Frame info(%p %d) (%p %d)", 
					__LINE__, pBuffer, iBufferSize, pNewBuff, uiNewSize);
			}
		}
	}

	NXBOOL bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoCodecType, pNewBuff, uiNewSize, (void*)&m_uiDSINalSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, ISSEEKABLEFRAME_IDRONLY);
	
	m_perfMonVideoWrite.CheckModuleUnitStart();

	if( uiDTS != 0 && m_uiVideoCTS == uiDTS )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter abnormal video time stamp(%d %d)", __LINE__, m_uiVideoCTS, uiDTS);
		uiDTS = m_uiVideoCTS + 1;
	}

	if( writeOneFrame(NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO, uiNewSize, pNewBuff, uiCTS, uiDTS, bIDRFrame ? NXFF_FRAME_TYPE_INDEXED : NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
	{
		// JIRA 3186
		if( m_uiVideoContinueCount != 8 )
			m_uiVideoContinueCount++;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter writeframe failed(%p %d) (%d) (%d)", __LINE__, m_pRawBuffer, uiNewSize, m_uiVideoContinueCount, bIDRFrame);
		return 1;
	}
	m_perfMonVideoWrite.CheckModuleUnitEnd();
	m_uiVideoFrameCount++;
	m_uiVideoContinueCount = 0;

	*pDuration	= m_pWriterHandle->MovieDuration;		
	*pSize		= m_pWriterHandle->MovieSize;	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] ID(%d) setBaseVideoFrameWithoutEncode Out Time : %u,%u FrameCount : %d IDR(%d)", 
		__LINE__, uiID, uiDTS, uiCTS, m_uiVideoFrameCount, bIDRFrame);
	
	m_uiVideoCTS = uiDTS;
	return 0;
}

int CNexFileWriter::writeVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[WrapFileWriter.cpp %d] writeVideoFrameWithoutEncode in(ID: %d curID : %d lastCTS(%d %d)cts : %u dts : %u)", 
		__LINE__, uiID, m_uiDECurrentVideoClipID, m_uiAudioCTS, m_uiVideoCTS, uiCTS, uiDTS);

	CAutoLock m(m_lockVideoDE);
	
	if( m_isStarted == FALSE || m_pWriterHandle == NULL) return 1;
	if( pDuration == NULL || pSize == NULL ) return 1;

	if( pBuffer == NULL && iBufferSize == 0 )
	{
		m_uiVideoCTS = uiDTS;
		return 0;
	}	

	if(m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V)
	{
		if( m_uiDSINalSize == 0 )
		{
			nexSAL_MemDump((unsigned char*)pBuffer, 100);
			m_uiDSINalSize = NexCodecUtil_GuessNalHeaderLengthSize((unsigned char*)pBuffer, iBufferSize);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[WrapFileWriter.cpp %d] get Frame Nal Size(%d)", __LINE__, m_uiDSINalSize);
		}
	}
	// for KMSA 362 issue about ff writer write failed.
	if( m_uiVideoContinueCount == 8 )
	{
		CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, NEXVIDEOEDITOR_ERROR_EXPORT_NOT_ENOUGHT_DISK_SPACE);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Video Encoder failed so send errerEvent(%d)", __LINE__, m_uiVideoContinueCount);
		return 1;
	}

	*pDuration	= 0;
	*pSize		= 0;

	unsigned char*	pNewBuff		= NULL;
	unsigned int	uiNewSize		= 0;
	unsigned int	uiFrameFormat	= 0;

	if(m_uiVideoCodecType != eNEX_CODEC_V_MPEG4V)
	{
#ifdef DISABLE_MIXED_RAW_AND_ANNEXB_NAL
		uiFrameFormat = NexCodecUtil_CheckByteFormat((unsigned char*)pBuffer, iBufferSize);
#else
		if((uiFrameFormat = NexCodecUtil_CheckByteFormat((unsigned char*)pBuffer, iBufferSize)) == NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW)
		{
			int iCodeLen = 0;
			unsigned int uNalSize = 0;
			NexCodecUtil_FindAnnexBStartCode((unsigned char*)pBuffer, 4, iBufferSize > 200?200:iBufferSize, &iCodeLen);

			if(iCodeLen == 4)
			{
				uiFrameFormat = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
				m_uiDSINalSize = iCodeLen;
			}
		}
		else
		{
			uiFrameFormat = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
		}
#endif
	}
    
	if( NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == uiFrameFormat )
	{
		if(m_uiDSINalSize == 0)
		{
			unsigned char *pTmpBuffer = (unsigned char*)pBuffer;
			if(pTmpBuffer[0] == 0 && pTmpBuffer[1] == 0 && pTmpBuffer[2] == 0 && pTmpBuffer[3] == 1)
				m_uiDSINalSize = 4;
			if(pTmpBuffer[0] == 0 && pTmpBuffer[1] == 0 && pTmpBuffer[2] == 1)
				m_uiDSINalSize = 3;
		}

		m_perfMonVideoConverter.CheckModuleUnitStart();
		uiNewSize = NexCodecUtil_ConvertFormat( m_pRawBuffer, WRITER_RAW_FRAME_TEMP_SIZE, (unsigned char*)pBuffer, iBufferSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB, m_uiDSINalSize);
		m_perfMonVideoConverter.CheckModuleUnitEnd();
		if( uiNewSize == -1 )
		{	
			m_uiVideoContinueCount++;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert Annex To Raw Failed ErrorCnt(%d)", __LINE__, m_uiVideoContinueCount);
			return 1;
		}
		pNewBuff = m_pRawBuffer;
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] Convert AnnexB->Raw bitstream", __LINE__);
	}
	else
	{
		pNewBuff =  (unsigned char*)pBuffer;
		uiNewSize = iBufferSize;
	}

	if( m_uiVideoFrameCount == 0 )
	{
 		NXUINT32 uNewFrameSize = 0;
		NXUINT32 uConfigStreamOffset = 0;
		NXUINT8 *pConfigStream = NULL;

		if(m_uiVideoCodecType == eNEX_CODEC_V_H264)
		{
			pConfigStream = NexCodecUtil_AVC_NAL_GetConfigStream(pNewBuff, uiNewSize, (NXINT32)m_uiDSINalSize, (NXINT32*)&uNewFrameSize);
			if (pConfigStream != NULL)
			{
				if (uiNewSize != uNewFrameSize)
				{
 					uConfigStreamOffset = (NXUINT32)(pConfigStream - pNewBuff);
					pNewBuff = pConfigStream + uNewFrameSize;
					uiNewSize -= uConfigStreamOffset + uNewFrameSize;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter Frame info(%p %d) (%p %d)", 
					__LINE__, pBuffer, iBufferSize, pNewBuff, uiNewSize);
			}
		}
		else if(m_uiVideoCodecType == eNEX_CODEC_V_HEVC)
		{
			pConfigStream = NexCodecUtil_HEVC_NAL_GetConfigStream(pNewBuff, uiNewSize, (NXINT32)m_uiDSINalSize, (NXINT32*)&uNewFrameSize);
			if (pConfigStream != NULL)
			{
				if (uiNewSize != uNewFrameSize)
				{
 					uConfigStreamOffset = (NXUINT32)(pConfigStream - pNewBuff);
					pNewBuff = pConfigStream + uNewFrameSize;
					uiNewSize -= uConfigStreamOffset + uNewFrameSize;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter Frame info(%p %d) (%p %d)", 
					__LINE__, pBuffer, iBufferSize, pNewBuff, uiNewSize);
			}
		}
	}

	NXBOOL bIDRFrame = NexCodecUtil_IsSeekableFrame( (NEX_CODEC_TYPE)m_uiVideoCodecType, pNewBuff, uiNewSize, (void*)&m_uiDSINalSize, NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW, ISSEEKABLEFRAME_IDRONLY);
	
	m_perfMonVideoWrite.CheckModuleUnitStart();

	if( 0 != uiDTS && m_uiVideoCTS == uiDTS )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter abnormal video time stamp(last:%u dts:%u cts:%u)", __LINE__, m_uiVideoCTS, uiDTS, uiCTS);
		if (uiCTS == uiDTS)
			uiCTS++;
		uiDTS = m_uiVideoCTS + 1;
	}

	if( writeOneFrame(NXFFW_MEDIA_TYPE_BASE_LAYER_VIDEO, uiNewSize, pNewBuff, uiCTS, uiDTS, bIDRFrame ? NXFF_FRAME_TYPE_INDEXED : NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
	{
		// JIRA 3186
		if( m_uiVideoContinueCount != 8 )
			m_uiVideoContinueCount++;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] NxMP4FWriter writeframe failed(%p %d) (%d) (%d)", __LINE__, m_pRawBuffer, uiNewSize, m_uiVideoContinueCount, bIDRFrame);
		return 1;
	}
	m_perfMonVideoWrite.CheckModuleUnitEnd();
	m_uiVideoFrameCount++;
	m_uiVideoContinueCount = 0;

	*pDuration	= m_pWriterHandle->MovieDuration;		
	*pSize		= m_pWriterHandle->MovieSize;	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] ID(%d) writeVideoFrameWithoutEncode Out Time(cts:%u dts:%u) FrameCount : %d IDR(%d)", 
		__LINE__, uiID, uiCTS, uiDTS, m_uiVideoFrameCount, bIDRFrame);
	
	m_uiVideoCTS = uiDTS;
	return 0;
}


NXBOOL CNexFileWriter::setEnhanceVideoFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize)
{
	if( m_isStarted == FALSE || m_pWriterHandle == NULL) return FALSE;
	if( pDuration == NULL || pSize == NULL ) return FALSE;

	*pDuration	= 0;
	*pSize		= 0;

	if( writeOneFrame(NXFFW_MEDIA_TYPE_ENHANCED_LAYER_VIDEO, uiFrameSize, pFrame, uiCTS, uiCTS, NXFF_FRAME_TYPE_UNINDEXED) == FALSE )
	{
		return FALSE;
	}
	*pDuration	= m_pWriterHandle->MovieDuration;		
	*pSize		= m_pWriterHandle->MovieSize;	
	return TRUE;
}

NXBOOL CNexFileWriter::setBaseVideoFrameTime(unsigned int uiCTS)
{
	m_uiVideoCTS = uiCTS;
	return TRUE;
}

NXBOOL CNexFileWriter::setPause(NXBOOL bEncodecRelease)
{
	if(bEncodecRelease)
	{
		if( m_hCodecVideoEnc )
		{
			nexCAL_VideoEncoderDeinit( m_hCodecVideoEnc );
			CNexCodecManager::releaseCodec( m_hCodecVideoEnc );	
			m_hCodecVideoEnc = NULL;

			m_MediaCodecInputSurf = NULL;
			m_uiMediaCodecUserData	= 0;
			m_fnSetMediaCodecTimeStamp = NULL;
			m_fnResetMediaCodecVideoEncoder = NULL;
			m_fnSetCropToAchieveResolution = NULL;
		}
	}
	m_bPaused = TRUE;
	return TRUE;
}

NXBOOL CNexFileWriter::setResume()
{
	m_bPaused = FALSE;
	if(m_hCodecVideoEnc == NULL)
	{
		setVideoCodecInfo(m_uiVideoCodecType, m_uiWidth, m_uiHeight, m_uiDisplayWidth, m_uiDisplayHeight, m_uiFrameRate, m_uiBitRate, m_uiProfile, m_uiLevel);
	}
    return TRUE;
}

NXBOOL CNexFileWriter::getPause()
{
	return m_bPaused;
}

unsigned int CNexFileWriter::getBaseVideoFrameTime()
{
	return m_uiVideoCTS;
}

int CNexFileWriter::getEncodeWidth()
{
	return m_uiWidth;
}

int CNexFileWriter::getEncodeHeight()
{
	return m_uiHeight;
}

int CNexFileWriter::getEncoderFrameInterval()
{
	int frameInterval = EDITOR_DEFAULT_FRAME_TIME / m_uiFrameRate;
	return frameInterval;
}

unsigned int CNexFileWriter::getEncodeInputFormat()
{
	return m_uiEncoderInputFormat;
}

void CNexFileWriter::setVideoRotate(int iRotate)
{
	m_Rotate = iRotate;
}

void CNexFileWriter::setAudioSamplingRate(int iSamplingRate)
{
	m_uiSamplingRate = iSamplingRate;
}

CVideoWriteBuffer* CNexFileWriter::getBuffer()
{
	if( m_pVideoWriteTask != NULL && m_pVideoWriteTask->IsWorking() )
	{
		return m_pVideoWriteTask->getBuffer();
	}
	return NULL;
}

NXBOOL CNexFileWriter::releaseBuffer(CVideoWriteBuffer* pBuffer)
{
	if( m_pVideoWriteTask != NULL && m_pVideoWriteTask->IsWorking() )
	{
		return m_pVideoWriteTask->releaseBuffer(pBuffer);
	}
	return FALSE;
}

NXBOOL CNexFileWriter::waitEmptyBuffer(int iTime)
{
	if( m_pVideoWriteTask != NULL && m_pVideoWriteTask->IsWorking() )
	{
		return m_pVideoWriteTask->waitEmptyBuffer(iTime);
	}
	return FALSE;
}

void* CNexFileWriter::getMediaCodecInputSurface()
{
	return (void*)m_MediaCodecInputSurf;
}

void CNexFileWriter::setMediaCodecTimeStamp(unsigned int uiTime)
{
#ifdef FOR_TEST_MEDIACODEC_DEC	
	if( m_fnSetMediaCodecTimeStamp )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setMediaCodec TimeStamp(%d)", __LINE__, uiTime);
		m_fnSetMediaCodecTimeStamp(uiTime, (void*)m_uiMediaCodecUserData);
	}
#endif	
}

void CNexFileWriter::setTotalDuration(NXINT64 qAudioDuration, NXINT64 qVideoDuration)
{
	m_qAudioTotalDuration = qAudioDuration;
	m_qVideoTotalDuration = qVideoDuration;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] setTotalDuration(A:%lld, V:%lld)", __LINE__, qAudioDuration, qVideoDuration);
}

unsigned int CNexFileWriter::getAudioTime()
{
	return m_uiAudioCTS;
}

unsigned int CNexFileWriter::getVideoTime()
{
	return m_uiVideoCTS;
}

NXBOOL CNexFileWriter::isDirectExportWriteEndAudio(unsigned int uiTotalTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] isDirectExportWriteEndAudio(%d %d %d)", 
		__LINE__, uiTotalTime, m_uiAudioCTS, m_uiDECurrentAudioClipID);
	if( (uiTotalTime - m_uiAudioCTS) < 500 && m_uiDECurrentAudioClipID == INVALID_CLIP_ID )
	{
		return TRUE;
	}
	return FALSE;
}
NXBOOL CNexFileWriter::isDirectExportWriteEndVideo(unsigned int uiTotalTime)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[WrapFileWriter.cpp %d] isDirectExportWriteEndVideo(%d %d %d)", 
		__LINE__, uiTotalTime, m_uiVideoCTS, m_uiDECurrentVideoClipID);
	if( (uiTotalTime - m_uiVideoCTS) < 500 && m_uiDECurrentVideoClipID == INVALID_CLIP_ID )
	{
		return TRUE;
	}
	return FALSE;
}

int CNexFileWriter::getErrorStatus()
{
	if( m_uiVideoContinueCount > 0 )
	{
		if( m_uiVideoContinueCount == 8 )
		{
			return 2;
		}
		else
		{
			return 1;
		}
	}
	return 0;
}

unsigned int CNexFileWriter::getDECurrentVideoClipID()
{
	return m_uiDECurrentVideoClipID;
}

NXBOOL CNexFileWriter::isReadyForMoreAudio(void)
{
	unsigned int uiVideoTime = getVideoTime();
	unsigned int uiAudioTime = getAudioTime();    
	if(uiAudioTime > uiVideoTime && m_bLastTrackEnded == FALSE)
	{
		nexSAL_TaskSleep(uiAudioTime - uiVideoTime);
		//nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[ARTask.cpp %d] wait video Enc time %d, %d", __LINE__, uiAudioTime, uiVideoTime);
		return FALSE;
	}

	return TRUE;
}

void CNexFileWriter::setLastTrackEnded(NXBOOL bSet)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[WrapFileWriter.cpp %d] setLastTrackEnded %d", __LINE__, bSet);
	m_bLastTrackEnded = bSet;
}

int CNexFileWriter::getAnnexBStartCode(unsigned int uiVideoObjectType, unsigned char* pFrame, unsigned int uFrameLen, unsigned int uNalSize)
{
	unsigned char* pData = NULL;
	int iSize = 0, uOffset = 0;
	if(uiVideoObjectType == eNEX_CODEC_V_H264)
		pData = (unsigned char *)NexCodecUtil_AVC_NAL_GetConfigStream(pFrame, uFrameLen, (int)uNalSize, &iSize);
	else if(uiVideoObjectType == eNEX_CODEC_V_HEVC)
		pData = (unsigned char *)NexCodecUtil_HEVC_NAL_GetConfigStream(pFrame, uFrameLen, (int)uNalSize, &iSize);
	else
		return 0;

	if( pData != NULL && iSize > 0 )
	{
		uOffset = (NXUINT32)(pData - pFrame);
		pFrame = pData + iSize;
		uFrameLen -= uOffset + iSize;
	}		
	int iCodeLen = 0;
	NexCodecUtil_FindAnnexBStartCode(pFrame, 0, 8, &iCodeLen);

	return iCodeLen;
}

int CNexFileWriter::translatedToOMXLevel(int AvcLevel)
{
	switch (AvcLevel)
	{
	case 10: /* AVCLevel1 */
		return 0x1;
	case 20: /* AVCLevel2 */
		return 0x20;
	case 21: /* AVCLevel21 */
		return 0x40;
	case 30: /* AVCLevel3 */
		return 0x100;
	case 31: /* AVCLevel31 */
		return 0x200;
	case 32: /* AVCLevel32 */
		return 0x400;
	case 40: /* AVCLevel4 */
		return 0x800;
	case 41: /* AVCLevel41 */
		return 0x1000;
	case 42: /* AVCLevel42 */
		return 0x2000;
	case 50: /* AVCLevel5 */
		return 0x4000;
	case 51: /* AVCLevel51 */
		return 0x8000;
	case 52: /* AVCLevel52 */
	case 62: // It is not defined on Android
		return 0x10000;
	default:
		return 0;
	}
}

