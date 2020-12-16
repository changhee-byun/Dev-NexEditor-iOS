/******************************************************************************
 * File Name   :	NEXVIDEOEDITOR_UDTA.cpp
 * Description :
 *******************************************************************************

 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation

 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 cskb		2015/10/21	Draft.
 -----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_UDTA.h"

CNexUDTA::CNexUDTA()
{
	m_iType = 0;
}

CNexUDTA::~CNexUDTA()
{
}

NXBOOL CNexUDTA::setUDTA(int iType, const char* pUDTA)
{
	if( pUDTA != NULL )
	{
		if( strlen(pUDTA) > 127 )
		{
			return FALSE;
		}
		strcpy(m_pData, pUDTA);
	}

	m_iType = iType;
	return TRUE;
}

