/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_DirectExportTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
Case Song	2015/08/13	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_DIRECTEXPORTTASK_H__
#define __NEXVIDEOEDITOR_DIRECTEXPORTTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_CalcTime.h"

#include "nexPerMon.h"

#define DIRECTEXPORT_TASK_NAME "NEXVIDEOEDITOR DirectExport task"

class CNEXThread_DirectExportTask : public CNEXThreadBase
{
public:
	CNEXThread_DirectExportTask( void );
	virtual ~CNEXThread_DirectExportTask( void );

	virtual const char* Name()
	{	return DIRECTEXPORT_TASK_NAME;
	}

	virtual void End( unsigned int uiTimeout );
	
	NXBOOL setClipItem(CClipItem* pClipItem);
	NXBOOL setSource(CNexSource* pSource);
	NXBOOL setFileWriter(CNexExportWriter* pFileWriter);
	void setDirectTaskMode(NXBOOL bVideoOnly);
 
	NXBOOL isEndDirectExport();

protected:                                 
    virtual int OnThreadMain( void );

private:
	NXBOOL isEOSFlag(unsigned int uiFlag);

private:
	unsigned int		m_uiClipID;
	CClipItem*			m_pClipItem;
	CNexSource*		m_pSource;
	CNexExportWriter*		m_pFileWriter;

	NXBOOL				m_isNextFrameRead;

	unsigned int		m_uiStartTime;
	unsigned int		m_uiEndTime;
	unsigned int		m_uiBaseTime;
	unsigned int		m_uiStartTrimTime;
	int					m_iSpeedFactor;
	
	CCalcTime			m_CalcTime;

	NXBOOL				m_bDirectExportEnd;
	NXBOOL				m_bVideoOnlyMode;
};

#endif // __NEXVIDEOEDITOR_DIRECTEXPORTTASK_H__
