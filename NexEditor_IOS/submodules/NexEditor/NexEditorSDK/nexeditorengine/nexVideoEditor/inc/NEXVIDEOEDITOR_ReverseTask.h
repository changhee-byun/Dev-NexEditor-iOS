/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ReverseTask.h
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

#ifndef __NEXVIDEOEDITOR_REVERSETASK_H__
#define __NEXVIDEOEDITOR_REVERSETASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NxVScaler.h"
#include <vector>
#include "NxResamplerAPI.h"
#include "NEXVIDEOEDITOR_TranscodingTask.h"
#include "NexSound.h"

#define REVERSE_TASK_NAME "NEXVIDEOEDITOR Reverse task"

class CNEXThread_TranscodingTask;
class CNEXThread_ReverseTask : public CNEXThreadBase
{
public:
	CNEXThread_ReverseTask( void );
	CNEXThread_ReverseTask( CNEXThread_TranscodingTask* pPM );
	virtual ~CNEXThread_ReverseTask( void );

	virtual const char* Name()
	{
		return REVERSE_TASK_NAME;
	}

	int setReverseInfo(const char* pTempFile, unsigned int uStartTime, unsigned int uEndTime);
	NXBOOL setFileReader(CNexFileReader* pFileReader);
	NXBOOL setFileWriter(CNexExportWriter* pFileWriter);
	
	int getErrorCheck();
	int getAudioFinished();
	int getAudioEncodingTime();
	
	virtual void cancelReverseAudio( );	

private :
	NXBOOL writeAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize);

	NEXVIDEOEDITOR_ERROR getWriterError();

	int	initFileReader();
	int	deinitFileReader();

	NXBOOL initAudioDecoder(unsigned int uiStartTS);
	NXBOOL deinitAudioDecoder();
	
	void sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2, unsigned int uiParam3);

protected:
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	void processReverseAudioFrame();
	int decodingPCM( NXBOOL bFrameEnd, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int uiCTS);
	NXBOOL initResampler();
	NXBOOL deinitResampler();

private:
	CNEXThread_TranscodingTask* m_pTranscodingTask;
	
public:
	NXBOOL				m_bInitDecoder;

	NXBOOL				m_isNextVideoFrameRead;
	NXBOOL				m_isNextAudioFrameRead;

	NXUINT32			m_uiTotalPlayTime;
	NXINT32				m_isVideo;
	NXINT32				m_isAudio;

	unsigned char*		m_pDSI;
	unsigned int			m_uiDSISize;

	NEXCALCodecHandle		m_hCodecAudio;
	NXUINT32			m_uiSampleRate;
	NXUINT32			m_uiChannels;
	NXUINT32			m_uiBitsPerSample;
	NXUINT32			m_uiSamplePerChannel;
	long long				m_llMaxFileSize;

	unsigned int			m_uiAudioDecodeBufSize;
	unsigned char*			m_pAudioDecodeBuf;

	NXBOOL				m_isResampling;
	LP_INEXSOUND		m_hNexResamplerHandle;

	unsigned int			m_uiAudioReSampleBufSize;
	unsigned char*			m_pAudioReSampleBuf;

	NXINT32				m_iBitrate;

	int					m_iFlag;
	char*				m_pUserData;
	NEXSALFileHandle		m_pFile;

	CNexFileReader*		m_pFileReader;
	CNexExportWriter*		m_pFileWriter;

	NXBOOL					m_bUserCancel;

	unsigned char*			m_pPCMBuffer;
	unsigned int				m_uPCMBufferSize;
	int						m_iErrorType;

	unsigned int				m_uStartTime;
	unsigned int				m_uEndTime;
	NXBOOL					m_bAudioFinished;
	unsigned int				m_uEncodingTime;
	unsigned int				m_uEncodingCount;
	
	CNexPerformanceMonitor 	m_perfVideoY2UV2YUV;
	CNexPerformanceMonitor	m_perfGLDraw;
	CNexPerformanceMonitor	m_perfGetPixels;
	CNexPerformanceMonitor	m_perfSwapbuffer;
	CNexPerformanceMonitor	m_perfDrawAll;
};

#endif // __NEXVIDEOEDITOR_TRANSCODINGTASK_H__
