/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_FastPreviewTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2015/07/22	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_FASTPREVIEWTASK_H__
#define __NEXVIDEOEDITOR_FASTPREVIEWTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_ClipList.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_CalcTime.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"
#include "NexThemeRenderer.h"
#ifdef _ANDROID
#include "NexThemeRenderer_SurfaceTexture.h"
#endif
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"
#include <vector>

#define FASTPREVIEW_TASK_NAME "NEXVIDEOEDITOR Fast Preview Task"

class CNexProjectManager;
class CNEXThread_VideoRenderTask;
class CNEXThread_FastPreviewTask: public CNEXThreadBase
{
public:
    CNEXThread_FastPreviewTask( void );
	virtual ~CNEXThread_FastPreviewTask( void );

	virtual const char* Name()
	{	return FASTPREVIEW_TASK_NAME;
	}

	NXBOOL setProjectManager(CNexProjectManager* pMng); 
	NXBOOL setRenderer(CNEXThread_VideoRenderTask* pVideo);
	NXBOOL setClipList(CClipList* pCliplist);
	
private:
	NEXVIDEOEDITOR_ERROR initVideoDecoder();
	NEXVIDEOEDITOR_ERROR deinitVideoDecoder();
    NXBOOL registTrack();
	NXBOOL deregistTrack();

protected:
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	NXBOOL FastPreview(int iDisplayWidth, int iDisplayHeight);
	NXBOOL FastPreviewTime(unsigned int uiTime);

private:
	CNexProjectManager*	m_pProjectMng;
	CClipList*			m_pCliplist;
	CNexSource*		m_pSource;

	CNEXThread_VideoRenderTask*		m_pVideoRenderer;

	NXBOOL				m_isInitEnd;

	CNexCodecWrap*		m_pCodecWrap;
	void*				m_pDecSurface;
	unsigned int		m_uiCheckVideoDecInit;

	NXINT32				m_iSrcWidth;
	NXINT32				m_iSrcHeight;
	NXINT32				m_iSrcPitch;

    NXUINT32            m_uiStartTime;
	NXUINT32            m_uiEndTime;
    
	NXINT32				m_iVideoBitrate;
	NXINT32				m_iVideoFramerate;
	NXINT32				m_iVideoNalHeaderSize;

	NXBOOL				m_isNextVideoFrameRead;
	NXBOOL				m_isSupportFrameTimeChecker;

	unsigned int		m_uiVideoBaseTime;
	unsigned int		m_uiVideoStartTrim;

	int					m_iDecOutRealTime;
	unsigned int		m_uiDecOutCTS;

	int 				m_iFrameNALHeaderSize;
	int					m_iFPS;

// add in VideoTask
    NXUINT32			m_uiClipID;

	int					m_iSpeedFactor;
    
    unsigned int		m_uiEffectStartTime;	

	CCalcTime			m_CalcTime;

    CNexPerformanceMonitor	m_perforDecode;
	CNexPerformanceMonitor	m_perforGetOutput;

    CFrameTimeChecker		m_FrameTimeChecker;
    
    CVideoTrackInfo*		m_pVideoTrack;

    unsigned int            m_uiFrameStartTime;
    unsigned int            m_uiFrameEndTime;

	unsigned int			m_uiEndDecTime;
	NXBOOL					m_isIDRFrame;
	NXBOOL					m_isDecodeMore;

	typedef std::vector<unsigned int>	m_vecFrameTime;
};

#endif // __NEXVIDEOEDITOR_FASTPREVIEWTASK_H__
