/*
 *  SALBody_Mem.h
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _SALBODY_MEM_HEADER_
#define _SALBODY_MEM_HEADER_

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif
	
	void* nexSALBody_MemAlloc( unsigned int uSize, char * pSrcFile=NULL, int iSrcLine=0  );						// JDKIM 2010/09/09
	void* nexSALBody_MemCalloc( unsigned int uNum, unsigned int uSize, char * pSrcFile=NULL, int iSrcLine =0 );		// JDKIM 2010/09/09	
	void nexSALBody_MemFree( void* pMem );
	void* nexSALBody_MemAlloc2( unsigned int uSize,  unsigned int uType, char * pSrcFile=NULL, int iSrcLine=0  );		// JDKIM 2010/09/09
	void nexSALBody_MemFree2( void* pMem );														// JDKIM 2010/09/09
	
	void nexSALBody_Printf();

#ifdef __cplusplus
}
#endif
//};

#endif
