/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_PlayClipTask.h
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

#ifndef __NEXVIDEOEDITOR_PLAYCLIPTASK_H__
#define __NEXVIDEOEDITOR_PLAYCLIPTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_ClipList.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"

#define PLAYCLIPTASK_TASK_NAME "NEXVIDEOEDITOR Play Clip Task"

class CNexProjectManager;
class CNEXThread_PlayClipTask : public CNEXThreadBase
{
public:
    CNEXThread_PlayClipTask( void );
    CNEXThread_PlayClipTask( CNexProjectManager* pPM );
	virtual ~CNEXThread_PlayClipTask( void );

	virtual const char* Name()
	{	return PLAYCLIPTASK_TASK_NAME;
	}

	NXBOOL setProjectManager(CNexProjectManager* pMng); 
	NXBOOL setRenderer(CNEXThreadBase* pVideo, CNEXThreadBase* pAudio); 
	NXBOOL setFileWriter(CNexExportWriter* pFileWriter);
	NXBOOL setClipList(CClipList* pCliplist);
	NXBOOL setDirectEncodeUserData(char* pUserData);
	NXBOOL setDirectExportOption(int option);

protected:
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	virtual void ProcessIdleState();
	virtual void ProcessPlayState();
	virtual void ProcessRecordState();
	
	NXBOOL playingClip(unsigned int uiCurrentTime);
	NXBOOL pauseClip();
	NXBOOL resumeClip();
	NXBOOL updatePlay4Export(unsigned int uCurrentTime);
	
private:
	CNexProjectManager*	m_pProjectMng;
	CClipList*			m_pCliplist;
	CNEXThreadBase*		m_pVideoRenderer;
	CNEXThreadBase*		m_pAudioRenderer;

	CNexExportWriter*		m_pFileWriter;

	char*				m_pUserData;
	int					m_iOption;
};

#endif // __NEXVIDEOEDITOR_PLAYCLIPTASK_H__
