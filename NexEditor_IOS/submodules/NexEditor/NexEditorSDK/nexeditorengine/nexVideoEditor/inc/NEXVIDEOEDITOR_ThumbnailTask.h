/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ThumbnailTask.h
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

#ifndef __NEXVIDEOEDITOR_THUMBNAILTASK_H__
#define __NEXVIDEOEDITOR_THUMBNAILTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_Thumbnail.h"
#include <vector>

#define THUMBNAIL_TASK_NAME "NEXVIDEOEDITOR Thumbnail task"

class CNexProjectManager;
class CNEXThread_ThumbnailTask : public CNEXThreadBase
{
public:
	CNEXThread_ThumbnailTask( void );
	CNEXThread_ThumbnailTask( CNexProjectManager* pPM );
	virtual ~CNEXThread_ThumbnailTask( void );

	virtual const char* Name()
	{	return THUMBNAIL_TASK_NAME;
	}

	NXBOOL setProjectManager(CNexProjectManager* pMng); 
	void setUserTag(int iUserTag);
	int getUserTag();
	void setStopThumb();	

protected:
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	virtual void ProcessIdleState();
	virtual void ProcessPlayState();
	virtual void ProcessRecordState();
	
	NXBOOL playingClip(unsigned int uiCurrentTime);
	
private:
	CNexProjectManager*	m_pProjectMng;
	int					m_iUserTag;
	CClipItem* 			m_pItem;
	int					m_isParseClip;
	int					m_isStopParse;

};

#endif // __NEXVIDEOEDITOR_THUMBNAILTASK_H__