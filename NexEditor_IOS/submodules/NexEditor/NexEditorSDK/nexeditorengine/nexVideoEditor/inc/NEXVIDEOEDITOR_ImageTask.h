/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ImageTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_IMAGETASK_H__
#define __NEXVIDEOEDITOR_IMAGETASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"

#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"

#define IMAGE_TASK_NAME "NEXVIDEOEDITOR Image task"

class CNEXThread_ImageTask : public CNEXThreadBase
{
public:
    CNEXThread_ImageTask( void );
	virtual ~CNEXThread_ImageTask( void );

	virtual const char* Name()
	{	return IMAGE_TASK_NAME;
	}

	virtual void End( unsigned int uiTimeout );

	NXBOOL setClipItem(CClipItem* pClipItem);
	NXBOOL setVideoRenderer(CNEXThread_VideoRenderTask* pVideoRenderer);
	NXBOOL waitLoadEvent(int timeOut);
	void setSetTimeMode(NXBOOL isSetTime);

protected:                                 
	virtual int OnThreadMain( void );

	CClipItem*					m_pClipItem;
	CNEXThread_VideoRenderTask*	m_pVideoRenderer;
	CNexEvent					m_eventLoad;
	NXBOOL						m_isSetTime;
};

#endif // __NEXVIDEOEDITOR_IMAGETASK_H__
