/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoPreviewTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2015/08/25	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_VIDEOPREVIEWTASK_H__
#define __NEXVIDEOEDITOR_VIDEOPREVIEWTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_PreviewThumb.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"

#include "NEXVIDEOEDITOR_Android_AudioRenderTask.h"
#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"

#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"

#include <vector>

#define VIDEO_PREVIEW_TASK_NAME "NEXVIDEOEDITOR Video Preview task"

class CNEXThread_VideoPreviewTask : public CNEXThreadBase
{
public:
	CNEXThread_VideoPreviewTask( void );
	virtual ~CNEXThread_VideoPreviewTask( void );

	virtual const char* Name()
	{	return VIDEO_PREVIEW_TASK_NAME;
	}

	NXBOOL setClipItem(CClipItem* pClipItem);
	NXBOOL setSource(CNexSource* pSource);
	
	NXBOOL setPreviewTime(unsigned int uPreviewTime, NXBOOL bReset = FALSE);
	NXBOOL setPreviewRender(void* pThumbRender, void* pOutputSurface);

	NXBOOL getPreviewThumbState();
	unsigned char* getPreviewData(unsigned int uiTime, NXBOOL bWait = FALSE);

	virtual void End( unsigned int uiTimeout );
	virtual void WaitTask();

protected:                                 
	virtual int OnThreadMain( void );

private:
	NXBOOL initVideoDecoder();
	NXBOOL deinitVideoDecoder();
	void resetVideoDecoder();

	NXBOOL isEOSFlag(unsigned int uiFlag);
	NXBOOL canUseSWDecoder();

public:
	int					m_iTemp;
	static int			m_iAvailableHWCodec;

private:
	unsigned int		m_uiClipID;
	unsigned int		m_uiClipTotalTime;
	CClipItem*			m_pClipItem;
	CNexSource*		m_pSource;

	CNexCodecWrap*		m_pCodecWrap;

	int					m_iWidth;
	int					m_iHeight;
	int					m_iPitch;

	NXBOOL				m_isNextFrameRead;

	NXBOOL				m_bPreviewFailed;
	CPreviewThumb*			m_pPreviewThumb;
	int					m_iPreviewStartTime;
	int					m_iPreviewEndTime;
	void*				m_pSurfaceTexture;

	unsigned int		m_uiCheckVideoDecInit;
	
	CFrameTimeChecker	m_FrameTimeChecker;
	NXBOOL				m_bSemaWait;

	unsigned int			m_uiStartTime;
	unsigned int			m_uiEndTime;
	
	unsigned int			m_uiCurrentTime;
	unsigned int			m_uiLastDecTime;

	NXBOOL				m_bEndFlag;
	NXBOOL				m_bStartFlag;
	NXBOOL				m_bDecToReverse;
	NXBOOL				m_bNeedReset;	

	CClipVideoRenderInfo* m_pPreviousRenderInfo;
	CClipVideoRenderInfo* m_pCurrentRenderInfo;	

	unsigned int			m_uiUpdatedTime;	
};

#endif // __NEXVIDEOEDITOR_VIDEOPREVIEWTASK_H__
