/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapOutSource.h
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

#ifndef __NEXVIDEOEDITOR_WRAPIPODSOURCE_H__
#define __NEXVIDEOEDITOR_WRAPIPODSOURCE_H__

#include "NexSAL_Com.h"
#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_WrapSource.h"
#include "NEXVIDEOEDITOR_Def.h"
#ifdef __APPLE__
#include "IPodLibrarySource.hpp"
#endif

class CNexIpodSource : public CNexSource
{
public:
    CNexIpodSource();
	virtual ~CNexIpodSource();

	virtual int createSource();
	virtual int deleteSource();

	virtual unsigned int getClipID();
	virtual void setClipID(unsigned int uiClipID);
	virtual void setEncodeToTranscode(int iValue);

	virtual int openFile(char* pURL, unsigned int iUrlLength);
	virtual int closeFile();
	
	virtual int isEndOfChannel(NXFF_MEDIA_TYPE eType, NXBOOL* pisResult);
	virtual int getDSI(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize);
	virtual int getTS(NXFF_MEDIA_TYPE eType, unsigned int* piTS );
	
	virtual int seekTo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE);
	virtual int seekToAudio(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE);
	virtual int seekToVideo(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode = NXFF_RA_MODE_CUR_PREV, NXBOOL bRepeat = FALSE, NXBOOL bABSTime = TRUE);
	virtual int seekToVideoWithAbsoluteTime(unsigned int uiTime, unsigned int* pSeekResultCTS, int iSeekMode);
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

	virtual NXBOOL isAudioExist();
	virtual NXBOOL isVideoExist();
	virtual NXBOOL isTextExist();
	virtual unsigned int getIDRFrameCount();
	virtual unsigned int getFrameFormat();
	virtual NXUINT32 parseFrameFormat(NXUINT8 *pFrame, NXUINT32 uFrameSize);

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

public:
	unsigned int				m_uiClipID;
	NXBOOL					m_isCreated;

        #if defined(__APPLE__)
	IPodLibraryReader          *m_pIPodSource;
	#endif
	NEXSALMutexHandle		m_hIpodSourceMutex;

	char*					m_pFilePath;

	unsigned int				m_uiTotalSourceSize;
	unsigned int				m_uiTotalSourceTime;
	
	unsigned int				m_uiBaseTimeStamp;
	unsigned int				m_uiTotalPlayAudioTime;
	unsigned int				m_uiTotalPlayVideoTime;
	unsigned int				m_uiTotalPlayTime;
	
	unsigned int				m_uiStartTrimTime;
	unsigned int				m_uiEndTrimTime;

	NXBOOL					m_isAudioExist;
	NXBOOL					m_isVideoExist;
	NXBOOL					m_isTextExist;

	unsigned char*			m_pBuffVideo;
	unsigned char*			m_pBuffAudio;
	unsigned char*			m_pBuffText;

	unsigned int				m_uBuffVideoSize;
	unsigned int				m_uBuffAudioSize;
	unsigned int				m_uBuffTextSize;
	
	unsigned int				m_uiVideoObjectType;
	unsigned int				m_uiAudioObjectType;

	unsigned int				m_uiH264ProfileID;
	unsigned int				m_uiH264Level;
	unsigned int				m_uiVideoBitRate;
	unsigned int				m_uiDSINALSize;
	unsigned int				m_uiFrameNALSize;
	unsigned int				m_uiFrameFormat;
	unsigned int				m_isInterlaced;

	unsigned int				m_uiNumOfChannels;
	unsigned int				m_uiSamplingRate;
	unsigned int				m_uiBitsPerSample;	
	unsigned int				m_uiNumOfSamplesPerChannel;
	unsigned int				m_uiAudioBitRate;

	unsigned int                		m_uiTotalFrameCount;
	unsigned int				m_uiCountIframe;
	unsigned int				m_uiOrientation;
	unsigned int				m_uiFrameFPS;
	float						m_fFrameFPS;

	unsigned int				m_uiVideoLastReadDTS;
	unsigned int				m_uiVideoLastReadPTS;
	
	unsigned int				m_uiAudioLastReadCTS;

	unsigned int				m_uiLastAudioCTS;
	unsigned int				m_uiLastAudioCTSRead;
	unsigned int				m_uiAudioFrameInterval;
	double					m_dbAudioFrameInterval;
	unsigned int				m_uiMaxAudioFrameInterval;

	NXBOOL					m_isAudioEnd;
	NXBOOL					m_isVideoEnd;
	NXBOOL					m_isTextEnd;

	unsigned int				m_uiVideoEditBoxTime;
	unsigned int				m_uiAudioEditBoxTime;	

	int						m_iEncodeToTranscode;
};

#endif // __NEXVIDEOEDITOR_WRAPIPODSOURCE_H__
