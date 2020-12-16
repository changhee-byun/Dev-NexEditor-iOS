/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_CalcTime.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
case		2015/07/20	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_CALCTIME_H__
#define __NEXVIDEOEDITOR_CALCTIME_H__

class CCalcTime
{
public:
	CCalcTime();
	~CCalcTime();

	unsigned int applySpeed(int iSpeedFactor, unsigned int uiTime);
	unsigned int applySpeed(unsigned int uiBaseTime, unsigned int uiStartTrim, int iSpeedFactor, unsigned int uiTime);
	unsigned int restoreSpeed(int iSpeedFactor, unsigned int uiTime);
	unsigned int restoreSpeed(unsigned int uiStartTrim, int iSpeedFactor, unsigned int uiTime);
	unsigned int restoreSpeed(unsigned int uiBaseTime, unsigned int uiStartTrim, int iSpeedFactor, unsigned int uiTime);
};

#endif // __NEXVIDEOEDITOR_CALCTIME_H__
