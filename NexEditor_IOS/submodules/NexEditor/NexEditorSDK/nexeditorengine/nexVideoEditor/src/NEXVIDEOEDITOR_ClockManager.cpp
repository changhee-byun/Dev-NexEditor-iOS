/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ClockManager.cpp
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
#include "NEXVIDEOEDITOR_ClockManager.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"

CNEXThread_ClockManager::CNEXThread_ClockManager( void )
{
	m_pProjectM		= NULL;
	m_uiStartTime	= 0;

	m_pUpdateCurrentTimeMsg = new CNxMsgUpdateCurrentTimeStamp;
}

CNEXThread_ClockManager::CNEXThread_ClockManager( CNexProjectManager* pPM )
{
	CNEXThreadBase::CNEXThreadBase();
	m_pProjectM = NULL;
	if( pPM )
	{
		m_pProjectM = pPM;
		m_pProjectM->AddRef();
	}
	m_uiStartTime	= 0;

	m_eTaskPriority		= NEXSAL_PRIORITY_HIGHEST;
	m_uTaskStackSize	= 16*1024;
	m_eTaskOption		= NEXSAL_TASK_NO_OPTION;

	m_pUpdateCurrentTimeMsg = new CNxMsgUpdateCurrentTimeStamp;
}

CNEXThread_ClockManager::~CNEXThread_ClockManager( void )
{
	SAFE_RELEASE(m_pProjectM);
	SAFE_RELEASE(m_pUpdateCurrentTimeMsg);
}

NXBOOL CNEXThread_ClockManager::Begin( void )
{
	m_hThread = nexSAL_TaskCreate(	(char*)Name(), 
									(NEXSALTaskFunc)ThreadProc, 
									(void*)this, 
									m_eTaskPriority, 
									m_uTaskStackSize, 
									m_eTaskOption);

	if( m_hThread == NEXSAL_INVALID_HANDLE )
		return FALSE;
	return TRUE;
}

NXBOOL CNEXThread_ClockManager::Begin(unsigned int uiStartTime)
{
	m_uiCurrentTime = uiStartTime;
	m_hThread = nexSAL_TaskCreate(	(char*)Name(), 
									(NEXSALTaskFunc)ThreadProc, 
									(void*)this, 
									m_eTaskPriority, 
									m_uTaskStackSize, 
									m_eTaskOption);

	if( m_hThread == NEXSAL_INVALID_HANDLE )
		return FALSE;
	return TRUE;
}

void CNEXThread_ClockManager::End( unsigned int uiTimeout )
{
	if( m_bIsWorking == FALSE ) return;

	m_bIsWorking = FALSE;
    if( nexSAL_TaskWait(m_hThread) != 0 )
	{
		nexSAL_TaskTerminate(m_hThread);
	}
	nexSAL_TaskDelete(m_hThread);
	m_hThread = NEXSAL_INVALID_HANDLE;

	if( m_hSema != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_SemaphoreDelete(m_hSema);
		m_hSema = NEXSAL_INVALID_HANDLE;
	}
}

int CNEXThread_ClockManager::OnThreadMain( void )
{
	m_uiStartTime	= nexSAL_GetTickCount();
    m_bIsWorking	= TRUE;
	while ( m_bIsWorking )
	{
		m_uiCurrentTime = m_uiCurrentTime + (nexSAL_GetTickCount() - m_uiStartTime);
		m_pUpdateCurrentTimeMsg->m_currentTimeStamp = m_uiCurrentTime;
		// if( m_pProjectM ) m_pProjectM->SendCommand(m_pUpdateCurrentTimeMsg);
		m_uiStartTime = nexSAL_GetTickCount();
		nexSAL_TaskSleep(CLOCKMANAGER_CLOCK_TIME);
	}
    return 0;
}

