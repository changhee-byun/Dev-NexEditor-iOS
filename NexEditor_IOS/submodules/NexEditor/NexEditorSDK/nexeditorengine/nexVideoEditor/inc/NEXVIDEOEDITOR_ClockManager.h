/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ClockManager.h
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

#ifndef __NEXVIDEOEDITOR_CLOCKMANAGER_H__
#define __NEXVIDEOEDITOR_CLOCKMANAGER_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NxFFReaderAPI.h"

#define CLOCKMANAGER_TASK_NAME "NEXVIDEOEDITOR Clock Manager task"

#define CLOCKMANAGER_CLOCK_TIME	10

class CNexProjectManager;

class CNEXThread_ClockManager : public CNEXThreadBase
{
public:
    CNEXThread_ClockManager( void );
    CNEXThread_ClockManager( CNexProjectManager* pPM );
	virtual ~CNEXThread_ClockManager( void );

	virtual const char* Name()
	{	return CLOCKMANAGER_TASK_NAME;
	}

	int	addTask(CNEXThreadBase* pThread);

	NXBOOL Begin();
	NXBOOL Begin(unsigned int uiStartTime);
	void End( unsigned int uiTimeout );
protected:                                 
	int OnThreadMain( void );

private:
	CNexProjectManager*				m_pProjectM;
	unsigned int					m_uiStartTime;
	CNxMsgUpdateCurrentTimeStamp*	m_pUpdateCurrentTimeMsg;
};

#endif // __NEXVIDEOEDITOR_CLOCKMANAGER_H__
