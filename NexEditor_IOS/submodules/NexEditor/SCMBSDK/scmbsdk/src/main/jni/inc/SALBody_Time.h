/*
 *  SALBody_Time.h
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _SALBODY_TIME_HEADER_
#define _SALBODY_TIME_HEADER_

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif
	
	unsigned int nexSALBody_GetTickCount();
	NXVOID nexSALBody_GetMSecFromEpoch(NXUINT64 *a_puqMSec);

#ifdef __cplusplus
}
#endif
//};

#endif
