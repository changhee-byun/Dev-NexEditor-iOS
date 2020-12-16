/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Property.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/11/11	Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_Property.h"

CNexProperty::CNexProperty()
{
	strcpy(m_Name, "");
	strcpy(m_Value, "");
}

CNexProperty::CNexProperty(char* pName, char* pValue)
{
	strcpy(m_Name, "");
	strcpy(m_Value, "");
	
	if( pName == NULL || pValue == NULL )
	{
		return;
	}
	strcpy(m_Name, pName);
	strcpy(m_Value, pValue);
}

CNexProperty::CNexProperty(const char* pName, const char* pValue)
{
	strcpy(m_Name, "");
	strcpy(m_Value, "");
	
	if( pName == NULL || pValue == NULL )
	{
		return;
	}
	strcpy(m_Name, pName);
	strcpy(m_Value, pValue);
}

CNexProperty::CNexProperty(const char* pName, char* pValue)
{
	strcpy(m_Name, "");
	strcpy(m_Value, "");
	if( pName == NULL || pValue == NULL )
	{
		return;
	}
	strcpy(m_Name, pName);
	strcpy(m_Value, pValue);
}

CNexProperty::~CNexProperty()
{
}

char* CNexProperty::getProperty(char* pName)
{
	if( pName == NULL )
		return NULL;

	if( strcmp(pName, m_Name) == 0 )
		return m_Value;
	return NULL;
}

char* CNexProperty::getProperty(const char* pName)
{
	if( pName == NULL )
		return NULL;

	if( strcmp(pName, m_Name) == 0 )
		return m_Value;
	return NULL;
}

NXBOOL CNexProperty::setProperty(char* pValue)
{
	if( pValue == NULL ) return FALSE;
	strcpy(m_Value, pValue);
	return TRUE;
}

NXBOOL CNexProperty::setProperty(const char* pValue)
{
	if( pValue == NULL ) return FALSE;
	strcpy(m_Value, pValue);
	return TRUE;
}

NXBOOL CNexProperty::isName(char* pName)
{
	if( pName == NULL )
		return FALSE;
	if( strcmp(pName, m_Name) == 0 )
		return  TRUE;
	return FALSE;
}

NXBOOL CNexProperty::isName(const char* pName)
{
	if( pName == NULL )
		return FALSE;
	if( strcmp(pName, m_Name) == 0 )
		return  TRUE;
	return FALSE;
}


