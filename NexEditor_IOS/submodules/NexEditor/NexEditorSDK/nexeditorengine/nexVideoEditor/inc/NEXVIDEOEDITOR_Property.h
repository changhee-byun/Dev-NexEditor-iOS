/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Property.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
kyebeom song		2013/11/11	Draft.
-----------------------------------------------------------------------------*/

#ifndef _NEXVIDEOEDITOR_PROPERTY__H_
#define _NEXVIDEOEDITOR_PROPERTY__H_

#include "NexSAL_Internal.h"
#include <vector>

#define PROPERTY_VALUE_MAX_LEN 64

class CNexProperty
{
public:
	CNexProperty();
	CNexProperty(char* pName, char* pValue);
	CNexProperty(const char* pName, char* pValue);
	CNexProperty(const char* pName, const char* pValue);
	~CNexProperty();

	char* getProperty(char* pName);
	char* getProperty(const char* pName);
	NXBOOL setProperty(char* pValue);
	NXBOOL setProperty(const char* pValue);
	NXBOOL isName(char* pName);
	NXBOOL isName(const char* pName);

	char m_Name[PROPERTY_VALUE_MAX_LEN];
	char m_Value[PROPERTY_VALUE_MAX_LEN];
};

typedef std::vector<CNexProperty> CNexPropertyVec;
typedef CNexPropertyVec::iterator CNexPropertyVecIter;

#endif // _NEXVIDEOEDITOR_PROPERTY__H_

