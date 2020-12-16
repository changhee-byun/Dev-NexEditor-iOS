#define LOG_TAG "nexCalBody"
//#include <utils/Log.h>

#include "nexCalBody.h"
#include "NexCAL.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <android/log.h>
#include <time.h>
#include <sys/time.h>


#define _AAC_SUPPORT_
#define _AMR_SUPPORT_
#define _AMRWB_SUPPORT_

unsigned char *g_pY;
unsigned char *g_pU;
unsigned char *g_pV;

// int g_nCALLogLevel = 0;
int g_nCALLogLevel = -1;

FILE* g_pDumpAudioFrame;
FILE* g_pDumpVideoFrame;

//namespace android {

#define	_ADDR_ALIGN_(a, w)	((unsigned char*)(((long)(a)+(w)-1)&~((w)-1)))
#define 	MAX_DEBUG_STRING_LENGTH 512

/*extern "C"*/ void nexCAL_DebugPrintf(const char *pszFormat, ...)
{
	if(g_nCALLogLevel >= 0)
	{
		va_list va;
		char szBuf[MAX_DEBUG_STRING_LENGTH];
		char *pDebug = szBuf;
		
		szBuf[MAX_DEBUG_STRING_LENGTH-1] = '\0';
		
		va_start( va, pszFormat );
		vsnprintf( pDebug, MAX_DEBUG_STRING_LENGTH-1, pszFormat, va );
		va_end( va );
		
	    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", pDebug);
	}
}

/*extern "C"*/ void nexCAL_DebugLogPrintf(const char *pszFormat, ...)
{
	va_list va;
	char szBuf[MAX_DEBUG_STRING_LENGTH];
	char *pDebug = szBuf;
	
	szBuf[MAX_DEBUG_STRING_LENGTH-1] = '\0';
	
	va_start( va, pszFormat );
	vsnprintf( pDebug, MAX_DEBUG_STRING_LENGTH-1, pszFormat, va );
	va_end( va );
	
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", pDebug);
}

/*extern "C"*/ void nexCAL_TraceCat( int iCategory, int iLevel, const char* pszFormat, ... )
{

	if(g_nCALLogLevel >= 0 && iLevel <= g_nCALLogLevel)
	{
		va_list va;
		char szBuf[MAX_DEBUG_STRING_LENGTH];
		char *pDebug = szBuf;
		
		szBuf[MAX_DEBUG_STRING_LENGTH-1] = '\0';
		
		va_start( va, pszFormat );
		vsnprintf( pDebug, MAX_DEBUG_STRING_LENGTH-1, pszFormat, va );
		va_end( va );
		

	    __android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", pDebug);
	}
}

/*extern "C"*/ void * nexCAL_MemAlloc(unsigned int uiSize)
{
	if(uiSize > 0)
	{
		return malloc(uiSize);
	}
	else
	{
		return NULL;
	}
}

/*extern "C"*/ void* nexCAL_MemCalloc( unsigned int uNum, unsigned int uSize )
{
	if ( uNum == 0 || uSize == 0 )
	{
		printf("MemCalloc size is zero\n");
		return NULL;
	}

	return calloc( uNum, uSize );
}

/*extern "C"*/ void nexCAL_MemFree(void *pMem)
{
	if(pMem)
	{
		free(pMem);
	}
}


/*extern "C"*/ void nexCAL_TaskSleep(unsigned int uiSleep)
{
	usleep(uiSleep * 1000);
}

unsigned int nexCAL_GetTickCount()
{
	static unsigned int uPreTick = 0;
	unsigned int uRet;
	struct timeval tm;

	gettimeofday( &tm, NULL );
	uRet = ( (tm.tv_sec*1000) + (tm.tv_usec/1000) );
	if ( uPreTick > uRet && (uPreTick-uRet) < 100000 )
		return uPreTick;

	uPreTick = uRet;
	return uRet;
}

#define NEXCAL_MEMDUMP_MAXBYTES_PER_LINE 16

void nexCAL_MemDump(void *pSrc, unsigned int nSize)
{
	int i, j, nLineCnt;
	unsigned char* pAddress = (unsigned char*)pSrc;
	unsigned char uc;
	unsigned char oneline[NEXCAL_MEMDUMP_MAXBYTES_PER_LINE*3+2];
	unsigned char *pBuffer = (unsigned char*)pSrc;

	nexCAL_DebugPrintf( "[nexSAL] Memory Dump - Addr : 0x%08X, Size : %d\n", pBuffer, nSize );
	nexCAL_DebugPrintf( "---------------------------------------------------------\n" );

	for ( i = 0, j = 0, nLineCnt = 0 ; i < nSize; i++ )
	{
		if ( ( i % NEXCAL_MEMDUMP_MAXBYTES_PER_LINE ) == 0 )
		{
			if ( nLineCnt != 0 )
			{
				oneline[j] = '\0';
				nexCAL_DebugPrintf( "0x%08X  %s\n", pAddress, oneline );
			}

			// address
			pAddress = (unsigned char*)pSrc + i;
			j =0;

			nLineCnt++;
		}
		uc = (*(pBuffer+i)) >> 4;
		if ( uc < 10 )
			oneline[j] = uc+'0';
		else
			oneline[j] = uc-10+'A';

		j++;

		uc = (*(pBuffer+i)) & 0x0F;
		if ( uc < 10 )
			oneline[j] = uc+'0';
		else
			oneline[j] = uc-10+'A';

		j++; oneline[j++] = ' ';

	}

	oneline[j] = '\0';
	nexCAL_DebugPrintf( "0x%08X  %s\n", pAddress, oneline );
	nexCAL_DebugPrintf( "---------------------------------------------------------\n" );
}


extern "C" void *nex_malloc(int size)
{
	return malloc(size);
}
 
extern "C" void *nex_calloc(int count, int size)
{
	return calloc(count, size);
}
 
extern "C" void nex_free(void *pMem)
{
	free(pMem);
}

unsigned int nexCALBody_Video_GetProperty( unsigned int uProperty, unsigned int *puValue, unsigned int uUserData )
{
	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_CODEC_IMPLEMENT:
		*puValue = NEXCAL_PROPERTY_CODEC_IMPLEMENT_SW;
		break;
	case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
		*puValue = NEXCAL_PROPERTY_ANSWERIS_YES;
		break;
	case NEXCAL_PROPERTY_VIDEO_OUTPUT_BUFFER_TYPE :
		*puValue = NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
		break;
	case NEXCAL_PROPERTY_BYTESTREAM_FORMAT:
		*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANY;
		break;
	case NEXCAL_PROPERTY_INITPS_FORMAT :
		*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;
		//*puValue = NEXCAL_PROPERTY_AVC_BYTESTREAMFORMAT_ANY;
		break;
	case NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM:
		*puValue = NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_OK;
		break;
	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
		*puValue = 66;//NEXAVCProfile_BASELINE;
		break;		
	case NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE :
		*puValue = NEXCAL_PROPERTY_MPEG4V_ACCEPTABLE_STARTCODE_VOP;
		break; 
	case NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT :
		*puValue = NEXCAL_PROPERTY_USE_IFRAME_WHEN_VDEC_INIT_YES;//;NEXCAL_PROPERTY_INIT_DECODER_PARTIAL
		break;

	case NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT:
		*puValue = NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_YES;//NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_NO;
		break;
	/*
	case NEXCAL_PROPERTY_DECODER_ERROR:
		*puValue = NEXCAL_PROPERTY_DECODER_ERROR_NONE;//NEXCAL_PROPERTY_DECODER_ERROR_RETURN;//NEXCAL_PROPERTY_MPEG4_SIMPLE_PROFILE_LEVEL3_SUPPORT_NO;
		break;
	*/	
	default:
		*puValue = 0;
		break;
	}

	//nexSAL_DebugPrintf("Video_GetProperty(%d:%d)\n", uProperty, *puValue);
	return 0;
}

unsigned int nexCALBody_Video_SetProperty( unsigned int uProperty, unsigned int uValue, unsigned int uUserData )
{
	return 0;
}

unsigned int nexCALBody_Audio_GetProperty( unsigned int uProperty, unsigned int *puValue, unsigned int uUserData )
{
	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_CODEC_IMPLEMENT:
		*puValue = NEXCAL_PROPERTY_CODEC_IMPLEMENT_HW;
		break;
	case NEXCAL_PROPERTY_NEED_EXCLUSIVE_RENDERER :
		*puValue = NEXCAL_PROPERTY_ANSWERIS_YES;
		break;
	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME :
		//*puValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_YES;
		*puValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_MUTE_FRAME_NO;
		break;
	case NEXCAL_PROPERTY_AUDIO_OUTPUT_BUFFER_TYPE :
		*puValue = NEXCAL_PROPERTY_AUDIO_BUFFER_PCM;
		break;
	case NEXCAL_PROPERTY_BYTESTREAM_FORMAT :
		*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;
		break;
	// JDKIM 2011/02/07	
	case NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER :
		*puValue = NEXCAL_PROPERTY_SUPPORT_TO_AAC_ADTSHEADER_YES;
		break;	
	// JDKIM : end	
	case NEXCAL_PROPERTY_AAC_SUPPORT_MAIN:
		*puValue = 1;
		break;
	case NEXCAL_PROPERTY_AAC_SUPPORT_LTP:
		*puValue = 1;
		break;
	case NEXCAL_PROPERTY_AAC_SUPPORT_SBR:
		*puValue = 1;
		break;
	case NEXCAL_PROPERTY_AAC_SUPPORT_PS:
		*puValue = 1;
		break;
	default:
		*puValue = 0;
		break;
	}
	return 0;
}

unsigned int nexCALBody_Audio_SetProperty( unsigned int uProperty, unsigned int uValue, unsigned int uUserData )
{
	return 0;
}

unsigned int nexCALBody_Text_GetProperty( unsigned int uProperty, unsigned int *puValue )
{
	*puValue = 0;
	return 0;
}


unsigned int nexCALBody_Dump_AudioInit(const  char* strDumpPath, unsigned int uiLen, unsigned char* pData)
{
	nexCAL_DebugPrintf( "nexCALBody_Dump_AudioInit (%s)\n", strDumpPath);
	g_pDumpAudioFrame = fopen(strDumpPath, "wb");
	if( g_pDumpAudioFrame)
	{
		fwrite( &uiLen, sizeof(unsigned int), 1,g_pDumpAudioFrame );
		fwrite( pData, uiLen, 1, g_pDumpAudioFrame);
		fclose( g_pDumpAudioFrame);
	}

	return 0;
}
unsigned int nexCALBody_Dump_VideoInit(const  char* strDumpPath, unsigned int uiLen, unsigned char* pData)
{
	nexCAL_DebugPrintf( "nexCALBody_Dump_VideoInit (%s)\n", strDumpPath);
	g_pDumpVideoFrame = fopen(strDumpPath, "wb");
	if( g_pDumpVideoFrame)
	{
		fwrite( &uiLen, sizeof(unsigned int), 1,g_pDumpVideoFrame );
		fwrite( pData, uiLen, 1, g_pDumpVideoFrame);
		fclose( g_pDumpVideoFrame);
	}

	return 0;
}
unsigned int nexCALBody_Dump_Audio(const  char* strDumpPath, unsigned int uiLen, unsigned int uiDTS, unsigned char* pData)
{
	g_pDumpAudioFrame = fopen(strDumpPath, "ab");

	if( g_pDumpAudioFrame)
	{
		fwrite( &uiDTS, sizeof(unsigned int), 1, g_pDumpAudioFrame);
		fwrite( &uiLen, sizeof(unsigned int), 1,g_pDumpAudioFrame );
		fwrite( pData, uiLen, 1, g_pDumpAudioFrame);
		fclose( g_pDumpAudioFrame);
	}

	return 0;
}
unsigned int nexCALBody_Dump_Video(const  char* strDumpPath, unsigned int uiLen, unsigned int uiDTS, unsigned char* pData)
{
	g_pDumpAudioFrame = fopen(strDumpPath, "ab");

	if( g_pDumpAudioFrame)
	{
		//fwrite( &uiDTS, sizeof(unsigned int), 1,g_pDumpAudioFrame );
		fwrite( &uiLen, sizeof(unsigned int), 1,g_pDumpAudioFrame );
		fwrite( pData, uiLen, 1, g_pDumpAudioFrame);
		fclose( g_pDumpAudioFrame);
	}

	return 0;
}





