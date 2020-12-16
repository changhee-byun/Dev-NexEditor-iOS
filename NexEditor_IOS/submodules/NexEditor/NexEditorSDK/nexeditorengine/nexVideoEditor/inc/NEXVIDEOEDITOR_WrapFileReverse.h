/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapFileReverse.h
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

#ifndef __NEXVIDEOEDITOR_WRAPFILEREVERSE_H__
#define __NEXVIDEOEDITOR_WRAPFILEREVERSE_H__

#include "NexSAL_Com.h"
#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NxFFReaderAPI.h"
#include "NexChunkParser.h"
#include "NEXVIDEOEDITOR_WrapSource.h"

#include "NEXVIDEOEDITOR_Def.h"
#include "nexPerMon.h"
#include "aes.h"

class CNexFileReverse : public CNexSource
{
public:
    CNexFileReverse();
	virtual ~CNexFileReverse();

	virtual int createSource();
	virtual int deleteSource();

	virtual unsigned int getClipID();
	virtual void setClipID(unsigned int uiClipID);
	virtual void setEncodeToTranscode(int iValue);

	int isReadyToOpen(NXBOOL *pResult);
	virtual int openFile(char* pURL, unsigned int iUrlLength);
	virtual int closeFile();
	
	virtual int isEndOfChannel(NXFF_MEDIA_TYPE eType, NXBOOL* pisResult);
	virtual int getDSI(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize);
	virtual int getTS(NXFF_MEDIA_TYPE eType, unsigned int* piTS);
	
	int getVideoDSIfromFirstFrame();

	int moveStartPos();
	virtual int seekTo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE);
	virtual int seekToAudio(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE);
	virtual int seekToVideo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE);
	virtual int seekToVideoWithAbsoluteTime(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode);
	int seekToVideoReverse(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode);
	
	virtual int getFindIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime);
	virtual int getFindNearIFramePos(unsigned int uiStartTime, unsigned int uiTargetTime, unsigned int* pResultTime);
	virtual int getSeekTable(unsigned int uiStartTime, int* pSeekTableCount, unsigned int** ppSeekTable);

	virtual int getSeekTable(unsigned int uiStartTime, unsigned int uiEndtime, int iCount, int* pSeekTableCount, unsigned int** ppSeekTable);

	virtual int getVideoFrame(NXBOOL isEnhanced = FALSE, NXBOOL bForceRead = FALSE);
	virtual int getVideoFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize);
	virtual int getVideoFrameDTS(NXUINT32* puDTS, NXBOOL bReadTime = FALSE);
	virtual int getVideoFramePTS(NXUINT32* puPTS, NXBOOL bReadTime = FALSE);

	virtual int getAudioFrame();
	virtual int getAudioFrameData(NXUINT8** ppFrame, NXUINT32* pFrameSize);
	virtual int getAudioFrameCTS(NXINT64* puCTS);

	NXBOOL isSeekable();
	NXBOOL isSeekable(unsigned int uiPosition);
	virtual NXBOOL isAudioExist();
	virtual NXBOOL isVideoExist();
	virtual NXBOOL isTextExist();
	int isIntraRA (NXBOOL *pIsIntraRA );
	virtual unsigned int getIDRFrameCount();
	virtual unsigned int getFrameFormat();
	virtual NXUINT32 parseFrameFormat(NXUINT8 *pFrame, NXUINT32 uFrameSize);
	int randomAccess(NXUINT32 uTargetCTS, NXUINT32* puResultCTS, NEXVIDEPEDITORRandomAccessMode eRAMode, NXBOOL bVideo, NXBOOL bAudio, NXBOOL bText);
	virtual int getVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight);
	virtual int getDisplayVideoResolution(NXUINT32* pusWidth, NXUINT32* pusHeight);
	virtual int getAudioBitrate(NXUINT32* puAudioBitrate);
	virtual int getVideoBitrate(NXUINT32* puVideoBitrate);
	virtual int getTotalPlayTime(NXUINT32* puTotalPlayTime);
	virtual int getSamplingRate(NXUINT32* puSamplingRate);
	virtual int getNumberOfChannel(NXUINT32* puNumOfChannels);
	virtual int getSamplesPerChannel(NXUINT32* piNumOfSamplesPerChannel);
	virtual int getDSINALHeaderLength();
	virtual int getFrameNALHeaderLength();
	virtual int getH264ProfileLevelID(unsigned int* puH264ProfileLevelID);
	virtual int getH264Level(unsigned int* puH264Level );
	virtual int getVideoH264Interlaced(NXBOOL* puH264Interlaced );
	
	virtual int getWAVEFormat (unsigned int *pFormatTag, unsigned int* pBitsPerSample, unsigned int *pBlockAlign, unsigned int *pAvgBytesPerSec, unsigned int *pSamplesPerBlock, unsigned int *pEncodeOpt, unsigned int *pExtraSize, unsigned char **ppExtraData);
	int getBitmapInfoHeader(unsigned char **pExtraData, unsigned int *pExtraDataLen);
	virtual int setBaseTimeStamp(unsigned int uiBaseTime);

	virtual int setTrimTime(unsigned int uiStartTrimTime, unsigned int uiEndTrimTime);

	virtual NXBOOL setRepeatAudioClip(unsigned int uiBaseTime, unsigned int uiStartTime);

	virtual int getReaderRotateState();
	virtual int getRotateState();
	virtual unsigned int getVideoFrameRate();
	virtual float getVideoFrameRateFloat();

	virtual unsigned int getTotalPlayAudioTime();
	virtual unsigned int getTotalPlayVideoTime();

	virtual int getAudioTrackCount();
	virtual int getVideoTrackCount();
	virtual int getVideoTrackUUID( NXUINT8** ppretAllocUUID, NXINT32* pretUUIDSize ); //yoon
	virtual int getEditBoxTime(unsigned int* puiAudioTime, unsigned int* puiVideoTime);

	virtual int getAudioObjectType();
	virtual int getVideoObjectType();
	virtual void setAudioObjectType(unsigned int uiAudioObjectType);
	virtual void setNumOfChannels(unsigned int uiNumOfChannels);
	virtual void setSamplingRate(unsigned int uiSamplingRate);
	virtual void setNumOfSamplesPerChannel(unsigned int uiNumOfSamplesPerChannel);
	virtual NXBOOL getEnhancedExist();
	virtual unsigned int getVideoEditBox();
	virtual unsigned int getAudioEditBox();
	
protected:
	int getAudioLostFrameStatus();
	int checkAudioLostFrame(NXUINT32 uAudioCTSRead);
	
	NXBOOL isAudioChunkType( NXUINT32 a_uCodecType, NXUINT32 a_uFourCC, NXUINT32 a_uFFType);
	NXBOOL isVideoChunkType(NXUINT32 a_uCodecType, NXUINT32 a_uFourCC, NXUINT32 a_uFFType);

	int fixAudioFrameCTS(NXUINT32 uAudioCTSRead, NXUINT32* puCTSRet);
	NEX_FF_RETURN getFrameFromAudioChunk(NxFFReaderWBuffer *a_pWBuff);
	NEX_FF_RETURN getFrameFromVideoChunk(NxFFReaderWBuffer *a_pWBuff, int iChunkParserGetOption = VIDEOCHUNKPARSER_GETFRAME_OPTION_NORMAL);
	NEX_FF_RETURN nxFFReaderDecrypt(NxFFReaderWBuffer *pFRWBuf, NXFF_MEDIA_TYPE iType);
	NEX_FF_RETURN nxFFReaderFrame(NXFF_MEDIA_TYPE iType);

	int dropVideoFrame(unsigned int uiTargetCTS, unsigned int* pCurrentDTS);

	unsigned int changeEndian( unsigned char* pData );
	int parseHex(char *pstr, int start, int end);
	int getParsedEncInfo(char *pIn, NXUINT8* pOut, NXINT32 iMaxOutLen);
	
/*
int LP_Start( NEXPLAYERSource* pThis )
int LP_Stop( NEXPLAYERSource* pThis )
int LP_Pause( NEXPLAYERSource* pThis )
int LP_Resume( NEXPLAYERSource* pThis )
*/

public:
	unsigned int				m_uiTemp;
	unsigned int				m_uiClipID;
	NXBOOL					m_isCreated;

	NxFFReader*				m_pFFReader;
	NxFFReaderWBuffer		m_BufVideo;
	NxFFReaderWBuffer		m_BufAudio;
	NxFFReaderWBuffer		m_BufText;
	unsigned char*			m_pBuffVideo;
	unsigned char*			m_pBuffAudio;
	unsigned char*			m_pBuffText;

	NEXSALMutexHandle		m_hReaderMutex;

	char*					m_pFilePath;
	
	unsigned int				m_uiBaseTimeStamp;
	unsigned int				m_uiTotalPlayAudioTime;
	unsigned int				m_uiTotalPlayVideoTime;
	unsigned int				m_uiTotalPlayTime;

	unsigned int				m_uiStartTrimTime;
	unsigned int				m_uiEndTrimTime;

	unsigned int				m_uiFileType;

	NXBOOL					m_isAudioExist;
	NXBOOL					m_isVideoExist;
	NXBOOL					m_isTextExist;
	NXBOOL					m_isEnhancedExist;

	unsigned int				m_uiVideoObjectType;
	unsigned int				m_uiVideoFourcc;
	unsigned int				m_uiAudioObjectType;
	unsigned int				m_uiAudioFourcc;

	unsigned int				m_uiH264ProfileID;
	unsigned int				m_uiH264Level;
	unsigned int				m_uiVideoBitRate;
	unsigned int				m_uiDSINALSize;
	unsigned int				m_uiFrameNALSize;
	unsigned int				m_uiFrameFormat;
	NXBOOL					m_isH264Interlaced;

	NXBOOL					m_isAudioChunkType;
	AUDIOCHUNK_HANDLE		m_hAudioChunkInfo;
	NXUINT8*				m_pAudioChunkFrame;
	NXUINT32				m_uiAudioChunkFrameLen;
	NXUINT32				m_uiAudioChunkFrameDTS;
	NXUINT32				m_uiAudioChunkFramePTS;

	NXBOOL					m_isVideoChunkType;
	VIDEOCHUNK_HANDLE		m_hVideoChunkInfo;
	NXUINT8*				m_pVideoChunkFrame;
	NXUINT32				m_uiVideoChunkFrameLen;
	NXUINT32				m_uiVideoChunkFrameDTS;
	NXUINT32				m_uiVideoChunkFramePTS;

	unsigned int				m_uiNumOfChannels;
	unsigned int				m_uiSamplingRate;
	unsigned int				m_uiNumOfSamplesPerChannel;
	unsigned int				m_uiAudioBitRate;

	unsigned int				m_uiCountIframe;
	unsigned int				m_uiframeCount;
	unsigned int				m_uiOrientation;
	unsigned int				m_uiFrameFPS;
	float						m_fFrameFPS;
	unsigned int				m_uiVideoFrameRate;
	int						m_iLostFrameCount;
	unsigned int				m_uiDisplayWidth;
	unsigned int				m_uiDisplayHeight;

	unsigned int				m_uiVideoLastReadDTS;
	unsigned int				m_uiVideoLastReadPTS;
	
	unsigned int				m_uiCorrectionTime;
	unsigned int				m_uiVideoTmpDTS;
	unsigned int				m_uiVideoTmpPTS;
	
	unsigned int				m_uiAudioLastReadCTS;

	unsigned int				m_uiLastAudioCTS;
	unsigned int				m_uiLastAudioCTSRead;
	unsigned int				m_uiAudioFrameInterval;
	double					m_dbAudioFrameInterval;
	unsigned int				m_uiMaxAudioFrameInterval;
	NXBOOL					m_isLostFrameCheck;

	NXBOOL					m_isAudioEnd;
	NXBOOL					m_isVideoEnd;
	NXBOOL					m_isTextEnd;

	void*					m_pBitmapInfoEx;

	NXBOOL					m_isEditBox;
	unsigned int				m_uiVideoEditBoxTime;
	unsigned int				m_uiAudioEditBoxTime;
	NXBOOL					m_isCTSBox;
	unsigned int				m_uiVideoCTSBoxTime;

#ifdef FOR_TEST_AVI_FORMAT
	unsigned char*			m_H264DSIBuffer;
#else
	unsigned char				m_H264DSIBuffer[DEFAILT_DSI_MAX_SIZE];
#endif
	int						m_H264DSIBufferSize;
	int						m_iEncodeToTranscode;

	NXBOOL					m_bUsePrevFrame;

	CNexPerformanceMonitor			m_perforReadFrame;
	NXUINT8					m_EncInfo[NXFF_MEDIA_TYPE_BASE_LAYER_VIDEO+1][ENC_INFO_SIZE];

	NXBOOL				m_uiReverseDuration;	

	NXUINT8				*m_pIntermediateFrame;
	NXUINT32			m_uIntermediateFrameSize;
};

typedef std::vector<CNexFileReverse*>	FileReverseVec;
typedef FileReverseVec::iterator			FileReverseVecIter;

#endif // __NEXVIDEOEDITOR_WRAPFILEREADER_H__
