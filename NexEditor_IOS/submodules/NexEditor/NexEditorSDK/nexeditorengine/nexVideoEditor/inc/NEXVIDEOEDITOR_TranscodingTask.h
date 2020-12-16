/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_TranscodingTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2012/09/19	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_TRANSCODINGTASK_H__
#define __NEXVIDEOEDITOR_TRANSCODINGTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"
#include "NexThemeRenderer.h"
#ifdef _ANDROID
#include "NexThemeRenderer_SurfaceTexture.h"
#endif
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NxVScaler.h"
#include <vector>
#include "NxResamplerAPI.h"
#include "NEXVIDEOEDITOR_HighlightDiffChecker.h"
#include "NEXVIDEOEDITOR_ReverseTask.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"
#include "NexSound.h"
#include "NEXVIDEOEDITOR_FrameYUVInfo.h"

#define TRANSCODING_TASK_NAME "NEXVIDEOEDITOR Transcoding task"

typedef std::vector<unsigned int> vecNexReverseYUVInfo;
typedef vecNexReverseYUVInfo::iterator vecNexReverseYUVInfoItr;

class CNEXThread_ReverseTask;
class CNexProjectManager;
class CNEXThread_TranscodingTask : public CNEXThreadBase
{
public:
	CNEXThread_TranscodingTask( void );
	CNEXThread_TranscodingTask( CNexProjectManager* pPM );
	virtual ~CNEXThread_TranscodingTask( void );

	virtual const char* Name()
	{
		return TRANSCODING_TASK_NAME;
	}

	int setInfo(	const char* pSrcFile,
					const char* pDstFile,
					int iWidth,
					int iHeight,
					int iDisplayWidth,
					int iDisplayHeight,
					int iBitrate,
					long long llMaxFileSize,
					int iFPS,
					int iFlag,
					int iSpeedFactor,
					void* pTranscodingRenderer, 
					void* pOutputSurface, 
					void* pDecSurface, 
					char* pUserData);

	int setReverseInfo(const char* pTempFile, unsigned int uStartTime, unsigned int uEndTime);
	
	int setVideoResolution(int iDstWidth, int iDstHeight);
	int setMaxFileSize(long long llMaxFileSize);
	int getVideoFinished();
	unsigned int getVideoEncodingTime();
	unsigned int getVideoBaseReverseTime();

	virtual void cancelTranscoding( );	

private :
	NXBOOL existTranscodingFile(const char* pTranscodingPath);
	void* getTranscodingSurfaceTexture();

	NXBOOL writeVideoFrame(void* pMediaBuffer, unsigned int uiCTS, unsigned int bWriteEnd = FALSE);
	NXBOOL writeVideoFrame(unsigned int uiCTS, unsigned char* pNV12Buffer);
	NXBOOL writeAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize);

	NXBOOL saveVideoYUV(void* pMediaBuffer, unsigned int uiCTS);
	NXBOOL saveVideoYUV(unsigned char* pData, unsigned int uSize, unsigned int uiCTS);

	NXBOOL writeVideoYUV(unsigned int uiCTS, unsigned char* pNV12Buffer, unsigned int uSize, unsigned int bWriteEnd = FALSE);

	NXBOOL processVideoFrameForHighLight(void* pMediaBuffer, unsigned int uiCTS);

	NEXVIDEOEDITOR_ERROR getWriterError();

	NXBOOL setSeekTableInfo(int iSeekTableCount, unsigned int* uiSeekTable);
	NXBOOL setTranscodingYUV(unsigned char* pY, unsigned char* pU, unsigned char* pV);

	void callVideoFrameRenderCallback(unsigned char* pBuff, NXBOOL bRender);
	void processEnhancedFrame(unsigned int* pEnhancement);
	NXBOOL isEOSFlag(unsigned int uiFlag);

	int	initFileReader();
	int initVideoDecoder();
	int reinitVideoDecoder(unsigned char* pIDRFrame, unsigned int uiSize);
	int	initWriter(unsigned int uDuration);
	int initRenderer();

	int	deinitFileReader();
	int	deinitVideoDecoder();
	int	deinitWriter();
	int deinitRenderer();

	NXBOOL initAudioDecoder(unsigned int uiStartTS);
	NXBOOL deinitAudioDecoder();
	
	int completeTranscoding();

	void sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3);

protected:
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	void processTransCoding(CNxMsgTranscodingInfo* pTranscodingInfo);
	void processHighLightIndex(CNxMsgHighLightIndex* pHighLightIndex);
	void processHighLight(CNxMsgHighLightIndex* pHighLightIndex);
	void processReverse(CNxMsgReverseInfo* pReverseInfo);
	void processReverseMemory(CNxMsgReverseInfo* pReverseInfo);	
	void processReverseDirect(CNxMsgReverseInfo* pReverseInfo);
	
	int decodingPCM( NXBOOL bFrameEnd, unsigned char* pFrame, unsigned int uSize, unsigned int uDTS);
	int CheckVideoSettingChange();
	NXBOOL initResampler();
	NXBOOL deinitResampler();
private:
	CNexProjectManager*	m_pProjectMng;
	
public:
	int					m_iTmp;

	NXBOOL				m_isNextVideoFrameRead;
	NXBOOL				m_isNextAudioFrameRead;

	NXUINT32			m_uiTotalPlayTime;
	NXINT32				m_isVideo;
	NXINT32				m_isAudio;

	unsigned char*		m_pDSI;
	unsigned int		m_uiDSISize;

	CNexCodecWrap*		m_pCodecWrap;
	unsigned int		m_uiCheckVideoDecInit;

	NEXCALCodecHandle	m_hCodecAudio;
	NXUINT32			m_uiSampleRate;
	NXUINT32			m_uiChannels;
	NXUINT32			m_uiBitsPerSample;
	NXUINT32			m_uiSamplePerChannel;
	long long			m_llMaxFileSize;

	unsigned int		m_uiAudioDecodeBufSize;
	unsigned char*		m_pAudioDecodeBuf;

	unsigned char*		m_pAudioUpChannelBuf;

	NXBOOL				m_isResampling;
	LP_INEXSOUND		m_hNexResamplerHandle;

	unsigned int		m_uiAudioReSampleBufSize;
	unsigned char*		m_pAudioReSampleBuf;
	
	NXINT32				m_iSrcWidth;
	NXINT32				m_iSrcHeight;
	NXINT32				m_iSrcPitch;

	NXINT32				m_iDstWidth;
	NXINT32				m_iDstHeight;
	NXINT32				m_iDstPitch;
	NXINT32				m_iDstDisplayWidth;
	NXINT32				m_iDstDisplayHeight;
	NXINT32				m_iBitrate;
	NXINT32				m_iEncodeProfile;
	NXINT32				m_iEncodeLevel;

	NXINT32				m_iSettingWidth;
	NXINT32				m_iSettingHeight;
	NXINT32				m_iSettingPitch;

	char*				m_strSrcFilePath;
	char*				m_strDstFilePath;
	char*				m_strTempFilePath;

	int					m_iFPS;
	int					m_iFlag;
	int					m_iSpeedFactor;
	char*				m_pUserData;
	NEXSALFileHandle	m_pFile;

	unsigned char* 		m_pTempBuffer;
	unsigned int		m_uiEncodedVideoCount;
	unsigned int		m_uiReqEncodeVideoCount;

	NXT_HThemeRenderer	m_hTranscodingRenderer;
	void*				m_pTranscodingDecSurface;
	void*				m_pOutputSurface;

	void*				m_pMediaSurface;

	CNexFileReader*		m_pFileReader;
	CNexExportWriter*		m_pFileWriter;

	unsigned int		m_uiEncoderInputFormat;

	CFrameTimeChecker	m_FrameTimeChecker;

	unsigned char*			m_pSoftwareTempBuffer;

	NXBOOL					m_bUserCancel;

	CNexPerformanceMonitor 	m_perfVideoY2UV2YUV;
	CNexPerformanceMonitor	m_perfGLDraw;
	CNexPerformanceMonitor	m_perfGetPixels;
	CNexPerformanceMonitor	m_perfSwapbuffer;
	CNexPerformanceMonitor	m_perfDrawAll;

	int 						m_iHighLightOutputMode;			// for highlight
	unsigned char* 			m_pHighLightBackBuffer;
	CHighlightDiffChecker	m_HighlightDiffChecker;

	FILE*					m_pYUVFRAME;

	int						m_iSrcOrientation;

	unsigned int			m_uStartTime;
	unsigned int			m_uEndTime;
	vecNexReverseYUVInfo	m_vecNexReverseYUVInfo;
	NEXSALFileHandle		m_pYuvFile;
	unsigned int			m_uYuvTotalSize;

	CNEXThread_ReverseTask*	m_pReverseAudioTask;
	unsigned int			m_uReverseMode;

	unsigned int			m_bVideoFinished;
	unsigned int			m_uEncodingTime;
	unsigned int			m_uBaseReverseTime;

	NXBOOL					m_bNeedRenderFinish;

	int					m_iDecodeMode;
	NXBOOL				m_bSupportFrameTimeChecker;

	int					m_iOutputFormat;

	CFrameYUVInfo*			m_pFrameYUVInfo;

};

#endif // __NEXVIDEOEDITOR_TRANSCODINGTASK_H__
