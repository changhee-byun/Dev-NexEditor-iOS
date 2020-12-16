/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_SetTimeTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/07/26	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_SETTIMETASK_H__
#define __NEXVIDEOEDITOR_SETTIMETASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"

#define SETTIME_TASK_NAME "NEXVIDEOEDITOR SetTime Task"

class CNexProjectManager;
class CNEXThread_SetTimeTask : public CNEXThreadBase
{
public:
	CNEXThread_SetTimeTask( void );
	virtual ~CNEXThread_SetTimeTask( void );

	virtual const char* Name()
	{	return SETTIME_TASK_NAME;
	}

	NXBOOL setProjectManager(CNexProjectManager* pMng); 
	NXBOOL setRenderer(CNEXThreadBase* pVideo); 

protected:
	virtual int OnThreadMain( void );
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	
private:
	CNexProjectManager*	m_pProjectMng;
	CNEXThreadBase*		m_pVideoRenderer;

	NXBOOL				m_bThumbMode;
};

#endif // __NEXVIDEOEDITOR_SETTIMETASK_H__