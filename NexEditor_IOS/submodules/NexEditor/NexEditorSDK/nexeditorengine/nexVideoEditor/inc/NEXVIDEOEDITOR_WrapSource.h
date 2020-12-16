/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapFileReader.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/21	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_WRAPSOURCE_H__
#define __NEXVIDEOEDITOR_WRAPSOURCE_H__

//#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NxFFReaderAPI.h"

#include "NEXVIDEOEDITOR_Def.h"
#include "NexCodecUtil.h"

typedef enum 
{
	NEXVIDEOEDITOR_SOURCE_NONE                     = 0,
	NEXVIDEOEDITOR_SOURCE_READER                 = 1,
	NEXVIDEOEDITOR_SOURCE_IPODLIBRARY                     = 2,
	NEXVIDEOEDITOR_SOURCE_DUMP                     = 3,

} NEXVIDEOEDITORSOURCEType;

#define DEFAUKT_FILEEADER_MAX_HEAPSIZE			( 10 * 1024 * 1024 )
#define DEFAULT_FRAME_BUFF_SIZE_VIDEO			(3*1024*1024)		// 1MB  :  AVI기준
#define DEFAULT_FRAME_BUFF_SIZE_AUDIO			(384*1024)		// 300KB : AVI기준
#define DEFAULT_FRAME_BUFF_SIZE_TEXT				(1024)

#ifdef FOR_TEST_AVI_FORMAT拙
#define DEFAILT_DSI_MAX_SIZE						(1*1024*1024)		// 1MB  :  AVI기준
#else
#define DEFAILT_DSI_MAX_SIZE						(2*1024)
#endif

#define _GetFrameIntervalDouble(s,r) (double)(((double)(s))*1000./((double)(r)))//+.5)		// JDKIM 2006/12/11
#define _GetFrameInterval(s,r) (unsigned int)((s)*1000/(double)(r)+.5)

typedef enum _NEXVIDEOEDITORGetFrameReturn
{
	_SRC_GETFRAME_OK = 0,
	_SRC_GETFRAME_END = 0x00100001,	// edit by zenith at 070622 

	// PD				: 읽으려는 프레임이 덜 다운로드 된 경우
	_SRC_GETFRAME_NOFRAME,
	// Streaming / DVBH	: RTP 버퍼에 쌓이는 속도보다 소비하는 속도가 빨라 Underflow가 발생한 경우
	_SRC_GETFRAME_NEEDBUFFERING,
	// Audio 에서만...
	// 이후 GetFrameData 는 NULL
	_SRC_GETFRAME_OK_LOSTFRAME,
	_SRC_GETFRAME_END_STREAM,				// JDKIM 2010/10/28
	_SRC_GETFRAME_ERROR,					//k.lee 20110421
	_SRC_GETFRAME_FOR4BYTESALIGN = 0x7FFFFFFF

} NEXVIDEOEDITORGetFrameReturn;

typedef enum _NEXVIDEOEDITORSourceGetLostFrameStatusReturn
{
	_SRC_GETAUDIOLOSTFRAME_STATUS_NO = 0,
	_SRC_GETAUDIOLOSTFRAME_STATUS_LOST,
	_SRC_GETAUDIOLOSTFRAME_STATUS_TAIL

} NEXVIDEOEDITORSourceGetLostFrameStatusReturn;

typedef enum
{
	_RA_MODE_SEEK = 0,
	_RA_MODE_PREV,
	_RA_MODE_NEXT,

	_RA_MODE_FOR4BYTESALIGN = 0x7FFFFFFF

} NEXVIDEPEDITORRandomAccessMode;

class CNexSource : public CNxRef<INxRefObj>
{
public:
	virtual ~CNexSource()
	{
	}

	virtual int createSource() = 0;
	virtual int deleteSource() = 0;
	
	virtual unsigned int getClipID() = 0;
	virtual void setClipID(unsigned int uiClipID) = 0;
	virtual void setEncodeToTranscode(int iValue) = 0;

	virtual int openFile(char* pURL, unsigned int iUrlLength) = 0;
	virtual int closeFile() = 0;
	
	virtual int isEndOfChannel(NXFF_MEDIA_TYPE eType, NXBOOL* pisResult) = 0;
	virtual int getDSI(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize) = 0;
	virtual int getTS(NXFF_MEDIA_TYPE eType, unsigned int* piTS ) = 0;
	
	virtual int seekTo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE) = 0;
	virtual int seekToAudio(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE) = 0;
	virtual int seekToVideo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE) = 0;
	virtual int seekToVideoWithAbsoluteTime(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode) = 0;
	virtual int getFindIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime) = 0;
	virtual int getFindNearIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime) = 0;
	virtual int getSeekTable(unsigned int uiStartTime, int* pSeekTableCount, unsigned int** ppSeekTable) = 0;
	virtual int getSeekTable(unsigned int uiStartTime, unsigned int uiEndtime, int iCount, int* pSeekTableCount, unsigned int** ppSeekTable) = 0;

	virtual int getVideoFrame(NXBOOL isEnhanced = FALSE, NXBOOL bForceRead = FALSE) = 0;
	virtual int getVideoFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize) = 0;
	virtual int getVideoFrameDTS(NXUINT32* puDTS, NXBOOL bReadTime = FALSE) = 0;
	virtual int getVideoFramePTS(NXUINT32* puPTS, NXBOOL bReadTime = FALSE) = 0;

	virtual int getAudioFrame() = 0;
	virtual int getAudioFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize) = 0;
	virtual int getAudioFrameCTS(NXINT64* puCTS) = 0;

	virtual NXBOOL isAudioExist() = 0;
	virtual NXBOOL isVideoExist() = 0;
	virtual NXBOOL isTextExist() = 0;
	virtual unsigned int getIDRFrameCount() = 0;
	virtual unsigned int getFrameFormat() = 0;
	virtual NXUINT32 parseFrameFormat(NXUINT8 *pFrame, NXUINT32 uFrameSize) = 0;

	virtual int getVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight) = 0;
	virtual int getDisplayVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight) = 0;
	virtual int getAudioBitrate(NXUINT32* puAudioBitrate) = 0;
	virtual int getVideoBitrate(NXUINT32* puVideoBitrate) = 0;
	virtual int getTotalPlayTime(NXUINT32* puTotalPlayTime) = 0;
	virtual int getSamplingRate(NXUINT32* puSamplingRate) = 0;
	virtual int getNumberOfChannel(NXUINT32* puNumOfChannels) = 0;
	virtual int getSamplesPerChannel(NXUINT32* piNumOfSamplesPerChannel) = 0;
	virtual int getDSINALHeaderLength() = 0;
	virtual int getFrameNALHeaderLength() = 0;
	virtual int getH264ProfileLevelID(unsigned int* puH264ProfileLevelID) = 0;
	virtual int getH264Level(unsigned int* puH264Level ) = 0;
	virtual int getVideoH264Interlaced(NXBOOL* puH264Interlaced ) = 0;
	virtual int getWAVEFormat (unsigned int *pFormatTag, unsigned int* pBitsPerSample, unsigned int *pBlockAlign, unsigned int *pAvgBytesPerSec, unsigned int *pSamplesPerBlock, unsigned int *pEncodeOpt, unsigned int *pExtraSize, unsigned char **ppExtraData) = 0;

	virtual int setBaseTimeStamp(unsigned int uiBaseTime) = 0;
	virtual int setTrimTime(unsigned int uiStartTrimTime, unsigned int uiEndTrimTime) = 0;
	virtual NXBOOL setRepeatAudioClip(unsigned int uiBaseTime, unsigned int uiStartTime) = 0;

	virtual int getReaderRotateState() = 0;
	virtual int getRotateState() = 0;
	virtual unsigned int getVideoFrameRate() = 0;
	virtual float getVideoFrameRateFloat() = 0;
	virtual unsigned int getTotalPlayAudioTime() = 0;
	virtual unsigned int getTotalPlayVideoTime() = 0;

	virtual int getAudioTrackCount() = 0;
	virtual int getVideoTrackCount() = 0;
	virtual int getVideoTrackUUID( NXUINT8** ppretAllocUUID, NXINT32* pretUUIDSize ) = 0; //yoon
	virtual int getEditBoxTime(unsigned int* puiAudioTime, unsigned int* puiVideoTime) = 0;

	virtual int getAudioObjectType() = 0;
	virtual int getVideoObjectType() = 0;
	virtual void setAudioObjectType(unsigned int uiAudioObjectType) = 0;

	virtual void setNumOfChannels(unsigned int uiNumOfChannels) = 0;
	virtual void setSamplingRate(unsigned int uiSamplingRate) = 0;
	virtual void setNumOfSamplesPerChannel(unsigned int uiNumOfSamplesPerChannel) = 0;
	virtual NXBOOL getEnhancedExist() = 0;
	virtual unsigned int getVideoEditBox() = 0;
	virtual unsigned int getAudioEditBox() = 0;

public:
	int getAnnexBStartCode(unsigned int uiVideoObjectType, unsigned char* pFrame, unsigned int uFrameLen, unsigned int uNalSize)
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

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[Source.cpp %d] getAnnexBStartCode %d, %d", __LINE__, uOffset, iSize);

		
		int iCodeLen = 0;
		//NexCodecUtil_FindAnnexBStartCode(pFrame, 4, uFrameLen > 200?200:uFrameLen, &iCodeLen);
		NexCodecUtil_FindAnnexBStartCode(pFrame, 0, 8, &iCodeLen);

		return iCodeLen;
	}
	
};

#endif // __NEXVIDEOEDITOR_WRAPSOURCE_H__
