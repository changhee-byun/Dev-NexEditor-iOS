/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoTask.h
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

#ifndef __NEXVIDEOEDITOR_VIDEOTASK_H__
#define __NEXVIDEOEDITOR_VIDEOTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"
#include "NEXVIDEOEDITOR_CalcTime.h"

#include "NEXVIDEOEDITOR_CodecCacheInfo.h"
#ifdef _WIN32
#include "NEXVIDEOEDITOR_WaveOut_AudioRenderTask.h"
#include "NEXVIDEOEDITOR_DDraw_VideoRenderTask.h"
#endif

#ifdef _ANDROID
#include "NEXVIDEOEDITOR_Android_AudioRenderTask.h"
#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"
#endif
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"

#include "nexPerMon.h"
#include <vector>

#define VIDEO_TASK_NAME "NEXVIDEOEDITOR Video task"

class CNEXThread_VideoTask : public CNEXThreadBase
{
public:
    CNEXThread_VideoTask( void );
	virtual ~CNEXThread_VideoTask( void );

	virtual const char* Name()
	{	return VIDEO_TASK_NAME;
	}

	NXBOOL setClipItem(CClipItem* pClipItem);
	NXBOOL setSource(CNexSource* pSource);
	NXBOOL setFileWriter(CNexExportWriter* pFileWriter);
	NXBOOL setVideoRenderer(CNEXThread_VideoRenderTask* pVideoRenderer);
	
	virtual void End( unsigned int uiTimeout );

	virtual unsigned int SetSuspend();
	virtual unsigned int Suspend();

	NXBOOL setSpeedFactor(int iFactor);
	void setLastEncOutCTS();

	int getVideoClipCount();

	NXBOOL checkDecoding();

	int checkRenderInfo(unsigned int uiTime);
	
protected:                                 
    virtual int OnThreadMain( void );

private:
	void processEnhancedFrame(unsigned int* pEnhancement);

	CCodecCacheInfo* getCachedVisualCodecInfo();
	void setCachedVisualCodecInfo(CCodecCacheInfo* pCodecCache);
	void clearCachedVisualCodecInfo(CCodecCacheInfo* pCodecCache);
	NXBOOL initCachedDecoder();
	NXBOOL initVideoDecoder();
	NXBOOL reinitVideoDecoder(unsigned char* pIDRFrame, unsigned int uiSize);

	NXBOOL deinitVideoDecoder();

	NXBOOL registTrack();
	NXBOOL deregistTrack();

	NXBOOL isEOSFlag(unsigned int uiFlag);
	NXBOOL canUseSWDecoder();

	NXBOOL getFrameDataFromMediaBuffer(void* pBuffer);

public:
	int					m_iTemp;
	static int			m_iAvailableHWCodec;

private:
	unsigned int		m_uiClipID;
	CClipItem*			m_pClipItem;
	CNexSource*		m_pSource;
	CNexExportWriter*		m_pFileWriter;
	CNexCodecWrap*		m_pCodecWrap;
	CCodecCacheInfo*		m_pCodecCache;

	CNEXThread_VideoRenderTask*	m_pVideoRenderer;

	int					m_iWidth;
	int					m_iHeight;
	int					m_iPitch;
    int                 m_iFPS;

	NXBOOL				m_isNextFrameRead;

	void*				m_pSurfaceTexture;

	unsigned int		m_uiOldTimeStamp;

	CVideoTrackInfo*	m_pVideoTrack;

	unsigned int				m_uiCheckVideoDecInit;

	unsigned int				m_uiEffectStartTime;

	CNexPerformanceMonitor		m_perforDecode;
	CNexPerformanceMonitor		m_perforGetOutput;

	int							m_iSpeedFactor;
	
	CFrameTimeChecker			m_FrameTimeChecker;
	unsigned int				m_uiPreviousPTS;

	unsigned int				m_uiSystemTick;
	unsigned int				m_uiDecFrameCountAfterDecInit;

	CCalcTime					m_CalcTime;
	CFrameTimeChecker		m_EncodeTimeChecker;
	unsigned int m_uiLastEncOutCTS = 0;

	CClipVideoRenderInfo* m_pPreviousRenderInfo;
	CClipVideoRenderInfo* m_pCurrentRenderInfo;
};

#endif // __NEXVIDEOEDITOR_VIDEOTASK_H__
