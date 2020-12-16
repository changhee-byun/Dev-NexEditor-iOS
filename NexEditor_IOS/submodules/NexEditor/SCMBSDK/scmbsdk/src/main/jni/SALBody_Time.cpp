/*
 *  SALBody_Time.c
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "SALBody_Time.h"
//#include <stdio.h>
//#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
//namespace android {

unsigned int nexSALBody_GetTickCount()
{
	// JDKIM 2010/10/13 : Sometime GetTickCount value is smaller than previous get value.
/*
//	return GetTickCount();
	struct timeval tm;
	
	//struct tms ttmmss;
	//UnsignedWide mt;
	//ui = TickCount();
	gettimeofday( &tm, NULL );
	//printf("tm.tv_sec=%d, tm.tv_usec=%d\n", tm.tv_sec, tm.tv_usec);
	//return tm.tv_usec;
	return ( (tm.tv_sec*1000) + (tm.tv_usec/1000) );
	//		ti = clock();
	
	//times(&ttmmss);
	//Microseconds(&mt);
	//return (ui);
	//	return ti;
	//	return (  ti / ( CLOCKS_PER_SEC / 1000 ) );
	//clock_gettime( CLOCK_REAL_TIME, &tp );
	//	return tp.tv_nsec / 1000000;
*/
	static unsigned int uPreTick = 0;
	unsigned int uRet;
	struct timeval tm;

	gettimeofday( &tm, NULL );
	uRet = ( (tm.tv_sec*1000) + (tm.tv_usec/1000) );
	if ( uPreTick > uRet && (uPreTick-uRet) < 100000 )
		return uPreTick;

	uPreTick = uRet;
	return uRet;
	// JDKIM : end
}

NXVOID nexSALBody_GetMSecFromEpoch(NXUINT64 *a_puqMSec)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if (a_puqMSec)
	{
		*a_puqMSec = (NXUINT64)(tv.tv_sec) * 1000 +  (NXUINT64)(tv.tv_usec) / 1000;
	}
}
//};
