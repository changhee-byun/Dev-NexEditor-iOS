/*
 *  SALBody_Mem.c
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 03.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#define LOG_TAG "NEXEDITOR"

#include "SALBody_Mem.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(USE_NDK)
#include <android/log.h>
#else
#include <utils/Log.h>
#endif


//namespace android {
static unsigned int g_mallocCnt = 0;
void* nexSALBody_MemAlloc( unsigned int uSize, char * pSrcFile, int iSrcLine  )		// JDKIM 2010/09/09
{
	if ( uSize == 0 )
	{
		printf("MemAlloc size is zero\n");
		return NULL;
	}
	
	g_mallocCnt++;
	return malloc( uSize );
}

void* nexSALBody_MemCalloc( unsigned int uNum, unsigned int uSize, char * pSrcFile, int iSrcLine )	// JDKIM 2010/09/09
{
	if ( uNum == 0 || uSize == 0 )
	{
		printf("MemCalloc size is zero\n");
		return NULL;
	}
	g_mallocCnt++;
	return calloc( uNum, uSize );
}

void nexSALBody_MemFree( void* pMem )
{
	if ( pMem == NULL )
	{
		printf("MemFree target is NULL\n");
		return;
	}
	g_mallocCnt--;
	free( pMem );
}

// JDKIM 2010/09/09
#define	_ADDR_ALIGN_(a, w)	((unsigned char*)(((long)(a)+(w)-1)&~((w)-1)))

void* nexSALBody_MemAlloc2( unsigned int uSize,  unsigned int uType, char * pSrcFile, int iSrcLine )
{
	void *mem;
	int *size;
	unsigned int	uAlign;
	unsigned char	*pAlign;
	
	if ( uSize == 0 )
	{
		printf("MemAlloc2 size is zero\n");
		return NULL;
	}

	switch ( uType & 0x0000FFFF )
	{
		case NEXSAL_MEM_ALIGN_NORMAL:
				uAlign=0;
				break;
		case NEXSAL_MEM_ALIGN_2:
				uAlign=2;
				break;
		case NEXSAL_MEM_ALIGN_4:
				uAlign=4;
				break;
		case NEXSAL_MEM_ALIGN_8:
				uAlign=8;
				break;
		case NEXSAL_MEM_ALIGN_16:
				uAlign=16;
				break;
		case NEXSAL_MEM_ALIGN_32:
				uAlign=32;
				break;
		case NEXSAL_MEM_ALIGN_64:
				uAlign=64;
				break;
		case NEXSAL_MEM_ALIGN_128:
				uAlign=128;
				break;
		default :
				uAlign=4;
				break;
	}

	mem = malloc(uSize + (sizeof(int)*2) + uAlign-1);

	if ( mem == NULL )
	{
//		printf("[NXSYS:MEM %d] Memory is not allocated. (%d)\r\n", __LINE__, (uiSize + (sizeof(int)*2) + uAlign-1));
//		nxSys_SetError(NXER_NOT_ENOUGH_MEMORY, _SYS_GetLastError());
		return NULL;
	}

	pAlign = _ADDR_ALIGN_((char *)mem+(sizeof(int)*2), uAlign);

	size = (int*)pAlign-1;
	*size = uSize;
	size = (int*)pAlign-2;
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
	*size = (NXINT32)(NXINT64)mem;
#else
	*size = (int)mem;
#endif	

	g_mallocCnt++;

	return (void*)pAlign;	
}

void nexSALBody_MemFree2( void* pMem )
{
	int size;

	if ( pMem == NULL )
	{
		return;
	}

	size = *((int*)pMem-1);
	
	free((void *)*((int*)pMem - 2));

	g_mallocCnt--;
}
// JDKIM : end

void nexSALBody_Printf()
{
	//LOGW("MEM cnt: %d, \n", g_mallocCnt);
	__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "MEM cnt: %d", g_mallocCnt);
}
//};
