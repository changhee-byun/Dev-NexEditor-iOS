/******************************************************************************
* File Name   :	nexPerMon.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006~2010 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation
******************************************************************************/

#ifndef NEXTREAMINT_NEXPERMON_H_INCLUDED
#define NEXTREAMINT_NEXPERMON_H_INCLUDED

#define MODULE_NAME_LENGTH	255

class CNexPerformanceMonitor
{
public:
	CNexPerformanceMonitor();
	CNexPerformanceMonitor(char* pModuleName);
	~CNexPerformanceMonitor();
	
	void CheckModuleStart();
	void CheckModuleLog();
	void CheckModuleUnitStart();
	void CheckModuleUnitEnd();
	
	char			m_strModule[MODULE_NAME_LENGTH];
	unsigned int	m_uiStart;
	unsigned int	m_uiUnitStart;
	unsigned int	m_uiNetWorkingTIme;
	unsigned int	m_uiCount;
};

#endif // NEXTREAMINT_NEXPERMON_H_INCLUDED

