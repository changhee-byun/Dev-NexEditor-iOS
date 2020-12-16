/******************************************************************************
* File Name   :	nexWrapCalBody_api.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2013/10/15	Draft.
-----------------------------------------------------------------------------*/


#ifndef __NEXWRAPCALBODY_API__
#define __NEXWRAPCALBODY_API__

#include "NexCAL.h"

#define NEXWRAP_CALBODY_API_VERSION 1

typedef struct NEXWRAPCALBody_
{
	void *func[13];
} NEXWRAPCALBody;

extern "C"
{

NEXWRAPCALBody *getNexCAL_Wrap_Body(
	  unsigned int uCodecObjectTypeIndication
	, unsigned int API_Version = NEXWRAP_CALBODY_API_VERSION
	, unsigned int nexCAL_API_MAJOR = NEXCAL_VERSION_MAJOR
	, unsigned int nexCAL_API_MINOR = NEXCAL_VERSION_MINOR
	, unsigned int nexCAL_API_PATCH_NUM = NEXCAL_VERSION_PATCH);
}

#endif // __NEXWRAPCALBODY_API__
