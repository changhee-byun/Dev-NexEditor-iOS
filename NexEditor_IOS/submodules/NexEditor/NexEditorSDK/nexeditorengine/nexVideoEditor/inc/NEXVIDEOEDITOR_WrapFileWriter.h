/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_WrapFileWriter.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/09/06	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_WRAPFILEWRITER_H__
#define __NEXVIDEOEDITOR_WRAPFILEWRITER_H__

#include "nexIRef.h"
#include "nexDef.h"
#include "NexCAL.h"
#include "NexSAL_Internal.h"
#include "nexLock.h"
#include "nexPerMon.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_VideoFrameWriteTask.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_ExportWriter.h"

#ifdef _ANDROID
#include <android/native_window.h>
#endif

#include "NxFFWriterAPI.h"

#define NEXCAL_PROPERTY_SURFACE_MEDIA_SOURCE  0xf99ff99f

#define AUDIO_ENCODE_MAX_SIZE	4096

//#define USE_WRITE_TASK

class CNexFileWriter : public CNexExportWriter
{
public:
    CNexFileWriter( void );
	virtual ~CNexFileWriter( void );

	NXBOOL setFilePath(char* pStrTargetPath);
	NXBOOL setEncoderInputFormat(unsigned int uiFormat);

	NXBOOL initFileWriter(long long llMaxFileSize, unsigned int uiDuration = 0);
	NXBOOL deinitFileWriter();

	NXBOOL startFileWriter();
	NXBOOL endFileWriter(unsigned int* pDuration, NXBOOL bCancel = FALSE);

	NXBOOL isStarted();

	void setAudioOnlyMode(NXBOOL bEnable);
	NXBOOL getVideoOnlyMode();
	void setVideoOnlyMode(NXBOOL bEnable);

	NXBOOL isVideoFrameWriteStarted();

	NXBOOL setAudioCodecInfo(unsigned int uiType, unsigned int uiSampleRate, unsigned int uiNumOfChannel, unsigned int uiBitRate);
	NXBOOL setAudioCodecInfo(unsigned int uiType, unsigned char* pDSI, unsigned int uiDSISize);
	NXBOOL setVideoCodecInfo(unsigned int uiType, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, unsigned int uiVideoFrameRate, unsigned int uiBitRate, int iProfile=0, int iLevel=0);
	NXBOOL setVideoCodecInfo(unsigned int uiType, unsigned char* pDSI, int iSize);

	NXBOOL setAudioInfo(unsigned char* pDSI, unsigned int uiDSISize);
	NXBOOL setBaseVideoInfo(unsigned char* pDSI, unsigned int uiDSISize);
	NXBOOL setEnhanceVideoInfo(unsigned char* pDSI, unsigned int uiDSISize);
	NXBOOL setVideoWidthHeight(unsigned int uiWidth, unsigned int uiHeight);
	NXBOOL setVideoProfileLevel(unsigned int uiProfile);

	NXBOOL writeOneFrame(NXFFW_MEDIA_TYPE MediaType, unsigned int samplesize, unsigned char *samplebuf, unsigned int CTS, unsigned int DTS, NXFF_FRAME_TYPE FrameType);
	NXBOOL setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize);
	NXBOOL setAudioFrameForVoice(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize);
	int setAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize);
	int writeAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize);
	// for transcoding.
	NXBOOL setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize); // without encode.
	
	NXBOOL setVideoFrameForDSI();
	int setVideoFrameForDSIwithInputSurf();
	NXBOOL checkEncoderEOS(unsigned int uiTime);
	NXBOOL setBaseVideoFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet = NULL);
	NXBOOL setBaseVideoFrame(unsigned int uiCTS, NXBOOL bEnd, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet);
	NXBOOL setBaseVideoFrame(unsigned int uiCTS, void* pBuffer, unsigned int* pDuration, unsigned int* pSize);
	int setBaseVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize);
	int writeVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize);
	
	NXBOOL setBaseVideoFrameTime(unsigned int uiCTS);
	NXBOOL setEnhanceVideoFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize);

	unsigned int getBaseVideoFrameTime();	
	NXBOOL setPause(NXBOOL bEncodecRelease);
	NXBOOL setResume();
	NXBOOL getPause();

	int getEncodeWidth();
	int getEncodeHeight();

	int getEncoderFrameInterval();

	unsigned int getEncodeInputFormat();

	void setVideoRotate(int iRotate);
	void setAudioSamplingRate(int iSamplingRate);

	CVideoWriteBuffer* getBuffer();
	NXBOOL releaseBuffer(CVideoWriteBuffer* pBuffer);
	NXBOOL waitEmptyBuffer(int iTime);

	void* getMediaCodecInputSurface();
	void setMediaCodecTimeStamp(unsigned int uiTime);

	void setTotalDuration(NXINT64 qAudioDuration, NXINT64 qVideoDuration);

	unsigned int getAudioTime();
	unsigned int getVideoTime();
	NXBOOL isDirectExportWriteEndAudio(unsigned int uiTotalTime);
	NXBOOL isDirectExportWriteEndVideo(unsigned int uiTotalTime);
	int getErrorStatus();

	unsigned int getDECurrentVideoClipID();
	NXBOOL isReadyForMoreAudio(void);
	void setLastTrackEnded(NXBOOL bSet);
	int getAnnexBStartCode(unsigned int uiVideoObjectType, unsigned char* pFrame, unsigned int uFrameLen, unsigned int uNalSize);

	int translatedToOMXLevel(int AvcLevel);

private:
	char*				m_strTargetPath;
	NXBOOL				m_isStarted;
	NXBOOL				m_bAudioOnly;
	NXBOOL				m_bVideoOnly;

	NxFFWriter*			m_pWriterHandle;

	unsigned int			m_uiAudioCodecType;
	unsigned char*			m_pAudioDSIInfo;
	unsigned int			m_uiAudioDSISize;

	unsigned int			m_uiVideoCodecType;
	unsigned char*			m_pBaseVideoDSIInfo;
	unsigned int			m_uiBaseVideoDSISize;

	unsigned char*			m_pEnhanceVideoDSIInfo;
	unsigned int			m_uiEnhanceVideoDSISize;

	NXBOOL					m_isTS;
	unsigned int			m_uiWidth;
	unsigned int			m_uiHeight;
	unsigned int			m_uiDisplayWidth;
	unsigned int			m_uiDisplayHeight;
	unsigned int			m_uiFrameRate;
	unsigned int			m_uiAudioBitRate;
	unsigned int			m_uiBitRate;
	unsigned int			m_uiProfile;
	unsigned int			m_uiLevel;
	unsigned int			m_uiProfileLevelID;
	int						m_Rotate;
	unsigned int			m_uiDuration;
	NXINT64					m_qAudioTotalDuration;
	NXINT64					m_qVideoTotalDuration;

	NEXCALCodecHandle		m_hCodecVideoEnc;
	NEXCALCodecHandle		m_hCodecAudioEnc;

	unsigned int			m_uiVideoFrameCount;
	unsigned int			m_uiVideoContinueCount;
	unsigned int			m_uiVideoStartGap;
	unsigned char*			m_pRawBuffer;

	unsigned int			m_uiVideoCTS;
	unsigned int			m_uiAudioCTS;
	NXINT64				m_qLastAudioCTS;
	NXINT64				m_qLastVideoCTS;
	CNexLock				m_Lock;

	CNEXThread_VideoFrameWriteTask*	m_pVideoWriteTask;

	CNexPerformanceMonitor	m_perfMonVideoEncode;
	CNexPerformanceMonitor	m_perfMonVideoGetOutput;
	CNexPerformanceMonitor	m_perfMonVideoWrite;
	CNexPerformanceMonitor	m_perfMonVideoConverter;
	CNexPerformanceMonitor	m_perfMonAudio;

	unsigned int				m_uiAudioObjectType;
	unsigned int				m_uiSamplingRate;
	unsigned int				m_uiChannels;

	unsigned long long			m_ullEncodedPCMSize;
	unsigned int				m_uiEncodedCTS;

	unsigned int				m_uiEncoderInputFormat;

	unsigned int				m_uiDSINalSize;

	void*						m_MediaCodecInputSurf;
	NXINT64						m_uiMediaCodecUserData;
#ifdef FOR_TEST_MEDIACODEC_DEC	
	setMediaCodecsetTimeStamp	m_fnSetMediaCodecTimeStamp;
	resetMediaCodecVideoEncoder	m_fnResetMediaCodecVideoEncoder;
	setCropToAchieveResolution	m_fnSetCropToAchieveResolution;
	NXBOOL						m_bFirstEncodedFrameSkip;
#endif
	unsigned int				m_uiSystemTickCheck;
	long long					m_llWriteAudioTotal;
	long long					m_llWriteVideoTotal;

	NXBOOL						m_bSkipMediaCodecTimeStamp;
	std::vector<unsigned int>	m_vecSkipMeidaCodecTimeStamp;

	unsigned int				m_uiDECurrentAudioClipID;
	unsigned int				m_uiDECurrentVideoClipID;
	CNexLock					m_lockAudioDE;	
	CNexLock					m_lockVideoDE;

	NXBOOL					m_bPaused;
	NXBOOL					m_bVideoEncoderInitToDiscardDSI;
	NXBOOL					m_bLastTrackEnded;
};

#endif // __NEXVIDEOEDITOR_WRAPFILEWRITER_H__
