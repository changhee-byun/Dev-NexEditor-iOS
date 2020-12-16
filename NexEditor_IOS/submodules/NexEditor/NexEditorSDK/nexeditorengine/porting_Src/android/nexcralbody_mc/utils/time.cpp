/******************************************************************************
* File Name        : time.cpp
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
#define LOCATION_TAG "Utils"

#define LOG_TAG "nexcral_mc"
#include "./time.h"

#include <time.h>
#include <sys/time.h>
#include <stdio.h>

// for differentiating between encoder and decoder in logs (U == undefined)
#define NEX_MC_TYPE ('U' - 'D')

namespace {
	struct timespec gTimeSpec = {0, 0};
	unsigned int gPrevTick = 0;
};

namespace Nex_MC {
namespace Utils {

namespace Time {
	unsigned int GetRealTime()
	{
		struct timespec ts;
		clock_gettime( CLOCK_REALTIME, &ts );
		return ts.tv_sec  * 1000 + ts.tv_nsec / 1000000;
	}
	
	unsigned int GetTickCount()
	{
		struct timespec ts;
		unsigned int currentVal;

		if ( ( 0 == gTimeSpec.tv_sec ) && ( 0 == gTimeSpec.tv_nsec ) )
		{
			clock_gettime( CLOCK_MONOTONIC, &gTimeSpec );
		}

		clock_gettime( CLOCK_MONOTONIC, &ts );
		currentVal = ( ts.tv_sec  - gTimeSpec.tv_sec )  * 1000
					+ ( ts.tv_nsec - gTimeSpec.tv_nsec ) / 1000000;

		if ( ( currentVal - gPrevTick ) > 0x80000000 )
		{
			currentVal = gPrevTick;
		}
		gPrevTick = currentVal;

		return currentVal;
	}

	void GetPrettyLocalTime(char *timeBuf, size_t timeBufSize, const char *format, bool include_ms)
	{
		struct timespec ts;
		clock_gettime( CLOCK_REALTIME, &ts );
#if defined(HAVE_LOCALTIME_R)
		struct tm tmBuf;
#endif
		struct tm* ptm;

#if defined(HAVE_LOCALTIME_R)
		ptm = localtime_r(&(ts.tv_sec), &tmBuf);
#else
		ptm = localtime(&(ts.tv_sec));
#endif
		size_t written = strftime(timeBuf, timeBufSize, format, ptm);

		if (include_ms)
		{
			snprintf(timeBuf + written, timeBufSize - written, ".%03ld", ts.tv_nsec / 1000000);
		}
	}
}; // namespace Time

}; // namespace Utils
}; // namespace Nex_MC
