/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_FrameYUVInfo.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
salabara		2019/03/22	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_FRAMEYUVINFO_H__
#define __NEXVIDEOEDITOR_FRAMEYUVINFO_H__

#include "NexSAL_Internal.h"
#include <vector>
#include "NEXVIDEOEDITOR_Def.h"
#include "nexIRef.h"

class CYUVInfo
{
public:
	CYUVInfo();
	CYUVInfo(unsigned int uiTime, unsigned char* pBuffer);
	CYUVInfo(unsigned int uiTime);
	virtual ~CYUVInfo(){};

	unsigned int getTime();
	unsigned char* getBuffer();

	 bool operator() (int i,int j) {return (i<j);}

private:
	unsigned int m_uiTime;
	unsigned char* m_pBuffer;
};

typedef std::vector<CYUVInfo> vecYUVInfo;
typedef vecYUVInfo::iterator vecYUVInfoItr;

class CFrameYUVInfo : public vecYUVInfo
{
public:
	CFrameYUVInfo( void );
	virtual ~CFrameYUVInfo( void );

	unsigned int getSize();
	NXBOOL addTime(unsigned int uiTime);	
	NXBOOL addYUVData(unsigned int uiTime, unsigned char* pData);

	NXBOOL isExistTime(unsigned int uiTime);
	unsigned int getFirstTime();	
	unsigned int getLastTime();	
	unsigned char* getFirstData();
	unsigned char* getLastData();
	
	NXBOOL removeAll();
	NXBOOL removeFirst();	
	NXBOOL removeLast();
	
	void sortWithTime();
	void printTime();

};

#endif // __NEXVIDEOEDITOR_FRAMEYUVINFO_H__
