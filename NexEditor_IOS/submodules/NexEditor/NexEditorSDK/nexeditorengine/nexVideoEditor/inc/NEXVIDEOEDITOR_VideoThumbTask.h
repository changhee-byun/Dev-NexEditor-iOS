/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoThumbTask.h
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

#ifndef __NEXVIDEOEDITOR_VIDEOTHUMBTASK_H__
#define __NEXVIDEOEDITOR_VIDEOTHUMBTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapSource.h"
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_Thumbnail.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"

#include "NEXVIDEOEDITOR_Android_AudioRenderTask.h"
#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"

#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"

#include <vector>

#define VIDEO_THUMB_TASK_NAME "NEXVIDEOEDITOR Video Thumb task"

class CNEXThread_VideoThumbTask : public CNEXThreadBase
{
public:
	CNEXThread_VideoThumbTask( void );
	virtual ~CNEXThread_VideoThumbTask( void );

	virtual const char* Name()
	{	return VIDEO_THUMB_TASK_NAME;
	}

	NXBOOL setClipItem(CClipItem* pClipItem);
	NXBOOL setSource(CNexSource* pSource);
	
	NXBOOL setThumbnailHandle(CThumbnail* pThumbnail);
	
	virtual void End( unsigned int uiTimeout );
	virtual void WaitTask();

	NXBOOL setStopThumb();

	NXBOOL isGetThumbnailFailed();
	
protected:                                 
	virtual int OnThreadMain( void );

private:
	NXBOOL initVideoDecoder();
	NXBOOL deinitVideoDecoder();

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

	int					m_isStopThumb;
	CNexCodecWrap*		m_pCodecWrap;

	int					m_iWidth;
	int					m_iHeight;
	int					m_iPitch;

	NXBOOL				m_isNextFrameRead;

	NXBOOL				m_bThumbnailFailed;
	CThumbnail*			m_pThumbnail;
	int					m_iThumbnailStartTime;
	int					m_iThumbnailEndTime;
	void*				m_pSurfaceTexture;

	unsigned int		m_uiCheckVideoDecInit;
	
	CFrameTimeChecker	m_FrameTimeChecker;
	CFrameTimeChecker	m_SeekTable_FrameTimeChecker;
	NXBOOL				m_isH264Interlaced;

	NXBOOL				m_bGetFirstFrame;
	NXBOOL				m_bGetLastFrame;		
};

#endif // __NEXVIDEOEDITOR_VIDEOTHUMBTASK_H__
