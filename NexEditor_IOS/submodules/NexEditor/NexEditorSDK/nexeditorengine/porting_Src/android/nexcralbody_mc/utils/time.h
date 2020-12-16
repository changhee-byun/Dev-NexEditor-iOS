/******************************************************************************
* File Name        : time.h
* Description      : time utilities for NexCRALBody_MC
*******************************************************************************
*
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
*
*	Nexstreaming Confidential Proprietary
*	Copyright (C) 2006~2013 Nexstreaming Corporation
*	All rights are reserved by Nexstreaming Corporation
*
******************************************************************************/

#ifndef _NEXCRAL_MC_TIME_H_
#define _NEXCRAL_MC_TIME_H_

#include <stdlib.h>

namespace Nex_MC {
namespace Utils {
namespace Time {
	unsigned int GetRealTime();
	unsigned int GetTickCount();
	void GetPrettyLocalTime(char *timeBuf, size_t timeBufSize, const char *format, bool include_ms);
}; // namespace Time
}; // namespace Utils
}; // namespace Nex_MC
#endif //#ifndef _NEXCRAL_MC_TIME_H_
