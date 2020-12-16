/******************************************************************************
* File Name   :	nexPerMon.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006~2010 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation
******************************************************************************/

#include "nexPerMon.h"
#include <stdio.h>
#include <stdlib.h>

#include "NexSAL_Internal.h"
#include <sys/time.h>
#include <string.h>

unsigned int GetTickCount()
{
	struct timeval tm;

	gettimeofday( &tm, NULL );
	return (unsigned int)( (tm.tv_sec*1000) + (tm.tv_usec/1000) );
}

CNexPerformanceMonitor::CNexPerformanceMonitor()
{
	m_uiStart			= 0;
	m_uiUnitStart			= 0;
	m_uiNetWorkingTIme	= 0;
	m_uiCount			= 0;
	strcpy(m_strModule, "Unknown Module");
}

CNexPerformanceMonitor::CNexPerformanceMonitor(char* pModuleName)
{
	m_uiStart			= 0;
	m_uiUnitStart			= 0;
	m_uiNetWorkingTIme	= 0;
	m_uiCount			= 0;
	if( pModuleName )
	{
		strcpy(m_strModule, pModuleName);
	}
	else
	{
		strcpy(m_strModule, "Unknown Module");
	}
}

CNexPerformanceMonitor::~CNexPerformanceMonitor()
{
	CheckModuleLog();
}

void CNexPerformanceMonitor::CheckModuleStart()
{
	m_uiStart			= GetTickCount();
	m_uiNetWorkingTIme	= 0;
	m_uiCount			= 0;
}

void CNexPerformanceMonitor::CheckModuleUnitStart()
{
	m_uiUnitStart = GetTickCount();
}

void CNexPerformanceMonitor::CheckModuleUnitEnd()
{
	m_uiNetWorkingTIme += GetTickCount() - m_uiUnitStart;
	m_uiCount++;
	if( m_uiCount >= 100 )
	{
		CheckModuleLog();
		CheckModuleStart();
	}
}

void CNexPerformanceMonitor::CheckModuleLog()
{
	unsigned int uiElapse = GetTickCount() - m_uiStart;
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[nexPerMon.cpp %d] %s Elapsed=%d,Count=%d,avg=%f,fps=%f ",
							__LINE__, m_strModule, uiElapse, m_uiCount, (float)m_uiNetWorkingTIme/m_uiCount, (float)m_uiCount/uiElapse*1000);
}

