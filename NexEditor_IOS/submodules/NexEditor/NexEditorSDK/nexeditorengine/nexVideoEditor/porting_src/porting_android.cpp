#include "porting_android .h"

#include "SALBody_Debug.h"
#include "SALBody_File.h"
#include "SALBody_Mem.h"
#include "SALBody_Sock.h"
#include "SALBody_SyncObj.h"
#include "SALBody_Task.h"
#include "SALBody_Time.h"

#include "NexCAL.h"
#include "NexMediaDef.h"
#include "nexCalBody.h"

#ifdef BUILD_JPEG_DECODER
#include "nexCalBody_JPEG.h"
#endif

#include "nexRALBody_Audio_API.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// #include "nexCalBody_AACEnc_Interface.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_SupportDevices.h"
#include "NexALQueryHandler.h"
// #include "deviceInfo.h"

#ifdef BUILD_NDK
#include "dlfcn.h"
#endif

// #define ENABLE_VIDEO_OC_DEC_LOG
// #define ENABLE_VIDEO_OC_ENC_LOG

// #define ENABLE_AUDIO_MC_DEC_LOG
// #define ENABLE_AUDIO_MC_ENC_LOG

// #define ENABLE_VIDEO_MC_DEC_LOG
// #define ENABLE_VIDEO_MC_ENC_LOG

#define NEXCAL_OC_DEC_API_VERSION 7
#define NEXCAL_OC_ENC_API_VERSION 3
#define NEXCAL_MC_API_VERSION 1
#define NEXCAL_SW_API_VERSION 1

#ifdef SW_AACENC
#define SW_AACEnc 1
#endif

#define IS_PCM_Codec(type) \
			( (type == eNEX_CODEC_A_PCM_S16LE) || (type == eNEX_CODEC_A_PCM_S16BE) || (type == eNEX_CODEC_A_PCM_RAW) || (type == eNEX_CODEC_A_PCM_FL32LE) \
			 || (type == eNEX_CODEC_A_PCM_FL32BE) || (type == eNEX_CODEC_A_PCM_FL64LE) || (type == eNEX_CODEC_A_PCM_FL64BE) || (type == eNEX_CODEC_A_PCM_IN24LE) \
			 || (type == eNEX_CODEC_A_PCM_IN24BE) || (type == eNEX_CODEC_A_PCM_IN32LE) || (type == eNEX_CODEC_A_PCM_IN32BE) || (type == eNEX_CODEC_A_PCM_LPCMLE) \
			 || (type == eNEX_CODEC_A_PCM_LPCMBE))
			 
NEXCALHandle	g_hNexCAL = NULL;

const char *g_strCodec_oc_gb					= "libnexcal_oc_gb.so";
const char *g_strCodec_oc_ics					= "libnexcal_oc_ics.so";
const char *g_strCodec_oc_jb					= "libnexcal_oc_jb.so";
const char *g_strCodec_oc_jb_msm8974		= "libnexcal_oc_jb_msm8974.so";
const char *g_strCodec_oc_jb_fih_msm8064	= "libnexcal_oc_jb_fih_msm8064.so";
const char *g_strCodec_oc_jb_mt6598			= "libnexcal_oc_jb_mt6589.so";


#ifdef FOR_PROJECT_SP
const char* g_strCodec_mc_jb					= "libnexcralbody_mc_jb_sp.so";
const char* g_strCodec_nex_audio				= "libwrapnexaudiocalbody_sp.so";
const char* g_strCodec_nex_pcm				= "libwrapnexpcmcalbody_sp.so";
const char* g_strCodec_nex_H264				= "libwrapnexh264calbody_sp.so";
const char* g_strCodec_nex_MP4				= "libwrapnexmp4calbody_sp.so";
const char* g_strCodec_nex_flac				= "libwrapnexflaccalbody_sp.so";
const char* g_strCodec_nex_aacEnc			= "libnexcalbody_aacenc_sp.so";
#else
const char* g_strCodec_mc_jb					= "libnexcralbody_mc_jb.so";
const char* g_strCodec_nex_audio				= "libwrapnexaudiocalbody.so";
const char* g_strCodec_nex_pcm				= "libwrapnexpcmcalbody.so";
const char* g_strCodec_nex_H264				= "libwrapnexh264calbody.so";
const char* g_strCodec_nex_MP4				= "libwrapnexmp4calbody.so";
const char* g_strCodec_nex_flac				= "libwrapnexflaccalbody.so";
const char* g_strCodec_nex_aacEnc			= "libnexcalbody_aacenc.so";
#endif


typedef struct _CAL_ONLOAD_INFO
{
 	char m_strLibVideoOcFile[512];
	char m_strLibCALMcFile[512];
	char m_strLibCALSWAudioFile[512];
	char m_strLibCALSWPCMFile[512];	
	char m_strLibCALSWFLACFile[512];	    
	char m_strLibCALSWH264File[512];
	char m_strLibCALSWMP4File[512];
	char m_strLibCALSWAACEncFile[512];
    
	void* m_pVideo_OC_Handle;
	void* m_pCAL_MC_Handle;
	void* m_pCAL_nexaudio_Handle;
	void* m_pCAL_nexPCM_Handle;
	void* m_pCAL_nexFLAC_Handle;    
	void* m_pCAL_nexH264_Handle;
	void* m_pCAL_nexMP4_Handle;
	void* m_pCAL_nexAACEnc_Handle;

	void* m_pCAL_MC_Handle_CallbackFunc;
	void* m_pCAL_MC_Handle_getInputSurf;
	void* m_pCAL_MC_Handle_setTimeStampOnSurf;
	void* m_pCAL_MC_Handle_resetVideoEncoder;
	void* m_pCAL_MC_Handle_setCropAchieveResolution;

	int	m_iLogLevel;

	int					m_iAPILevel;
	NEXCALGetProperty 	m_CAL_VideoGetProperty;
	
	CALQUERY_USERDATA m_QueryUserData;	
}CAL_ONLOAD_INFO;

CAL_ONLOAD_INFO g_hOnloadInfo;

#ifdef FOR_PROJECT_SP
const char *g_strRAL_audio						= "libnexralbody_audio_sp.so";
const char *g_strRAL_audio_kk					= "libnexralbody_audio_kk_sp.so";
#else
const char *g_strRAL_audio						= "libnexralbody_audio.so";
const char *g_strRAL_audio_kk					= "libnexralbody_audio_kk.so";
#endif

typedef struct _RAL_LOAD_INFO
{
	char m_strLibRALBodyAudio[512];
	void *m_pAUDIO_RALBODY_FUNCTION_ST_handle;
	void *m_pRalAudio_dl_handle;

	int m_iLogLevel;
} RAL_LOAD_INFO;

RAL_LOAD_INFO g_hRalLoadInfo;

 typedef void *(*RalbodyAudio_GetHandle)();
 typedef void (*RalbodyAudio_CloseHandle)(void *);

typedef struct NEXCALCodec2_
{
	void *func[13];
} NEXCALCodec2;

 typedef void *(*GetHandle)(	  
 	unsigned int uCodecObjectTypeIndication
	, unsigned int API_Version
	, unsigned int nexCAL_API_MAJOR
	, unsigned int nexCAL_API_MINOR
	, unsigned int nexCAL_API_PATCH_NUM);

 typedef void (*fnNexCAL_setDebugLevel)(
	  int types
	, int mex_level);


static int initCALRALHandle = 1;
void initCalRalHandle()
{
	if( initCALRALHandle == 1 ) {
		memset(&g_hOnloadInfo, 0x00, sizeof(CAL_ONLOAD_INFO));
		memset(&g_hRalLoadInfo, 0x00, sizeof(RAL_LOAD_INFO));
		initCALRALHandle = 0;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] initCalRalHandle !!!", __LINE__);
	}
}

#define MAX_DEBUG_STRING_LENGTH 512

FILE* 	g_LogFile = NULL;
int		g_iLogFileLine = 0;

static void _FileDebugPrintf( char* pszFormat, ... )
{
	if( g_LogFile == NULL )
		return;

	if( g_iLogFileLine > 10000 )
	{
		fseek(g_LogFile, 0, SEEK_SET);
		g_iLogFileLine = 0;
	}
	
	va_list va;
	char szBuf[MAX_DEBUG_STRING_LENGTH];
	char *pDebug = szBuf;
	
	szBuf[MAX_DEBUG_STRING_LENGTH-1] = '\0';
	
	va_start( va, pszFormat );
	vsnprintf( pDebug, MAX_DEBUG_STRING_LENGTH-1, pszFormat, va );
	va_end( va );

	fprintf(g_LogFile, "%d : %s%s", g_iLogFileLine, pDebug, "\n");
	g_iLogFileLine++;
}

static void _FileDebugOutputString( char* pszOutput )
{
	if( g_LogFile == NULL )
		return;

	if( g_iLogFileLine > 10000 )
	{
		fseek(g_LogFile, 0, SEEK_SET);
		g_iLogFileLine = 0;
	}

	fprintf(g_LogFile, "%d : %s%s", g_iLogFileLine, pszOutput, "\n" );
	g_iLogFileLine++;
} 

void registerSALLogforFile(const char * pFilePath)
{
	if( pFilePath == NULL )
		return;
	
	if( g_LogFile != NULL )
	{
		fclose(g_LogFile);
		g_iLogFileLine = 0;
	}

	g_LogFile = fopen(pFilePath, "w+");
	if( g_LogFile == NULL )
	{
		return;
	}

	// fseek(g_LogFile, 0, SEEK_SET);
	nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, (void*)_FileDebugPrintf, NULL );
	nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, (void*)_FileDebugOutputString, NULL );
}

void unregisterSALLogforFile()
{
	if( g_LogFile != NULL )
	{
		fclose(g_LogFile);
		g_iLogFileLine = 0;
	}

	nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, (void*)nexSALBody_DebugPrintf, NULL );
	nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, (void*)nexSALBody_DebugOutputString, NULL );
}



static void _NullDebugPrintf( char* pszFormat, ... )
{
}

static void _NullDebugOutputString( char* pszOutput )
{
}

//yoon
#include <jni.h>
static JavaVM *s_vm = NULL;

void registerJavaVM( void * java_vm )
{
    s_vm = (JavaVM*)java_vm;
}

void registerSAL( int nLogLevel )			// JDKIM 2010/11/11
{
	{
		nexSALBODY_SyncObjectsInit();   

		// JDKIM 2010/11/11
		if(nLogLevel >= 1)
		{
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, (void*)nexSALBody_DebugPrintf, NULL );
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, (void*)nexSALBody_DebugOutputString, NULL );
		}
		else
		{
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, (void*)_NullDebugPrintf, NULL );
			nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, (void*)_NullDebugOutputString, NULL );
		}

		// JDKIM : end
	 
		// Heap
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_ALLOC, (void*)nexSALBody_MemAlloc, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_CALLOC, (void*)nexSALBody_MemCalloc, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_FREE, (void*)nexSALBody_MemFree, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_ALLOC2, (void*)nexSALBody_MemAlloc2, NULL );			// JDKIM 2010/09/09
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_FREE2, (void*)nexSALBody_MemFree2, NULL );				// JDKIM 2010/09/09
	
		// File
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_OPENA, (void*)nexSALBody_FileOpen, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_CLOSE, (void*)nexSALBody_FileClose, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_READ, (void*)nexSALBody_FileRead, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_WRITE, (void*)nexSALBody_FileWrite, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK, (void*)nexSALBody_FileSeek, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK64, (void*)nexSALBody_FileSeek64, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SIZE, (void*)nexSALBody_FileSize, NULL);
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_REMOVEA, (void*)nexSALBody_FileRemove, NULL);

	
		// Sync Objects
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_CREATE, (void*)nexSALBody_EventCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_DELETE, (void*)nexSALBody_EventDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_SET, (void*)nexSALBody_EventSet, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_WAIT, (void*)nexSALBody_EventWait, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_CLEAR, (void*)nexSALBody_EventClear, NULL );
	
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_CREATE, (void*)nexSALBody_MutexCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_DELETE, (void*)nexSALBody_MutexDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_LOCK, (void*)nexSALBody_MutexLock, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_UNLOCK, (void*)nexSALBody_MutexUnlock, NULL );
	
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_CREATE, (void*)nexSALBody_SemaphoreCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_DELETE, (void*)nexSALBody_SemaphoreDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_RELEASE, (void*)nexSALBody_SemaphoreRelease, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_WAIT, (void*)nexSALBody_SemaphoreWait, NULL );
	
		// Task
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_CREATE, (void*)nexSALBody_TaskCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_DELETE, (void*)nexSALBody_TaskDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_SLEEP, (void*)nexSALBody_TaskSleep, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_WAIT, (void*)nexSALBody_TaskWait, NULL );
	
		
		nexSAL_RegisterFunction( NEXSAL_REG_GETTICKCOUNT, (void*)nexSALBody_GetTickCount, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_GETMSECFROMEPOCH, (void*)nexSALBody_GetMSecFromEpoch, NULL );
	
		// JDKIM 2010/11/01
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_FLOW, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_AUDIO, 0);// nLogLevel);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_VIDEO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_AUDIO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_VIDEO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_E_SYS, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_P_SYS, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_TEXT, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_DLOAD, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_INFO, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_WARNING, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_ERR, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_F_READER, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_F_WRITER, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_PVPD, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_PROTOCOL, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_CRAL, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_SOURCE, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_TARGET, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_DIVXDRM, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_RFC, 0);
		nexSAL_TraceSetCondition(NEX_TRACE_CATEGORY_NONE, 0);
		// JDKIM : end
	}
	 
//	pEngine->m_iSALRef++;
}

void unregisterSAL()
{
	//if(pEngine)
	{
		nexSALBODY_SyncObjectsDeinit();
		//pEngine->m_iSALRef--;
	}
}

NXINT32 nexCustom_NEXCALGetProperty(NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData)
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] Custom_NEX(%d %d) ", __LINE__, g_hOnloadInfo.m_iAPILevel, isSamsungMSM8974Device());
	if( ( g_hOnloadInfo.m_iAPILevel < 18 && isSamsungMSM8974Device() ) || isHuaweiHi3635Device()|| isHuaweiHi3630Device() )
	{
		switch(a_uProperty)
		{
			case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
				if( g_hOnloadInfo.m_CAL_VideoGetProperty != NULL )
				{
					g_hOnloadInfo.m_CAL_VideoGetProperty(a_uProperty, a_pqValue, a_pUserData);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
					if( *a_pqValue < 100 )
					{
 						*a_pqValue = 100;
						return 0;
					}
					return 0;
				}
				*a_pqValue = 100;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;
 				
			case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
				*a_pqValue = 51;
 				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;				
 			case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
				*a_pqValue = 51;
 				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;				
			case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
				*a_pqValue = 51;
 				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;				
			case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
				*a_pqValue = 51;
 				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;
			/*
			case NEXCAL_PROPERTY_AVC_GET_SUPPORT_LEVEL_HIGH10:
				*a_pqValue = 0;
 				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;				
			case NEXCAL_PROPERTY_AVC_GET_SUPPORT_LEVEL_HIGH422:
				*a_pqValue = 51;
 				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;				
			case NEXCAL_PROPERTY_AVC_GET_SUPPORT_LEVEL_HIGH444:
				*a_pqValue = 51;
 				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] GetProperty(%lld) ", __LINE__, *a_pqValue);
				return 0;
			*/
 		};
	}

	if( g_hOnloadInfo.m_CAL_VideoGetProperty != NULL )
	{
		return g_hOnloadInfo.m_CAL_VideoGetProperty(a_uProperty, a_pqValue, a_pUserData);
	}
	return 0;
}

int getCALOnload(	NXBOOL bRelease
					, NEXCALMediaType eMediaType
					, NEXCALMode eMode
					, NEX_CODEC_TYPE uCodecObjectTypeIndication
					, NXUINT32 uStaticProperties
					, NXVOID* pUserData
					, NXVOID* pClientID
					, NXVOID* pFuncs
					, NXINT32 nFuncSize
					, NEXCALQueryHandler* ppQueryHandler
					, NXVOID ** puUserDataForQueryHandler )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] getCALOnload Release(%d) eMediaType(0x%x) eMode(0x%x) uCodecObjectTypeIndication(0x%x)", __LINE__, bRelease, eMediaType, eMode, uCodecObjectTypeIndication);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] getCALOnload pFuncs(0x%x) nFuncSize(%d)", __LINE__, pFuncs, nFuncSize);
	CAL_ONLOAD_INFO* hOnloadInfo = (CAL_ONLOAD_INFO*)pUserData;
	if( hOnloadInfo == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] getCALOnload UserData is null", __LINE__);
		return 1;
	}

	if( eMediaType == NEXCAL_MEDIATYPE_VIDEO )
	{
		if( eMode == NEXCAL_MODE_DECODER )
		{	//eNEX_CODEC_V_H264_S
			if(  uCodecObjectTypeIndication == eNEX_CODEC_V_H264 && NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC)  && !NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC))
			{
				if(bRelease)
					return 0;

				if( hOnloadInfo->m_pCAL_nexH264_Handle == NULL && strlen(hOnloadInfo->m_strLibCALSWH264File) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALSWH264File);
					hOnloadInfo->m_pCAL_nexH264_Handle = dlopen(hOnloadInfo->m_strLibCALSWH264File, RTLD_NOW);
					if( hOnloadInfo->m_pCAL_nexH264_Handle == NULL )
					{
						hOnloadInfo->m_pCAL_nexH264_Handle = dlopen(g_strCodec_nex_H264, RTLD_NOW);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] codec file not found so try again system", __LINE__);
					}
				}
			
				if( hOnloadInfo->m_pCAL_nexH264_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_pCAL_nexH264_Handle, dlerror());
					return 1;
				}
		
				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_nexH264_Handle, "getNexCAL_Wrap_Body");
				if( getHandle )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2*)getHandle(	eNEX_CODEC_V_H264, 
																	NEXCAL_SW_API_VERSION, 
																	NEXCAL_VERSION_MAJOR, 
																	NEXCAL_VERSION_MINOR, 
																	NEXCAL_VERSION_PATCH);
					if( pGot )
					{
						NEXCALVideoDecoder* pCodecFunc = (NEXCALVideoDecoder*)pFuncs;

						pCodecFunc->GetProperty = (NEXCALGetProperty)pGot->func[11];
						pCodecFunc->SetProperty = (NEXCALSetProperty)pGot->func[6];
						pCodecFunc->Init = (NEXCALVideoInit)pGot->func[2];
						pCodecFunc->Deinit = (NEXCALVideoDeinit)pGot->func[3];
						pCodecFunc->Decode = (NEXCALVideoDecode)pGot->func[12];
						pCodecFunc->GetOutput = (NEXCALVideoGetOutput)pGot->func[8];
						pCodecFunc->Reset = (NEXCALVideoReset)pGot->func[5];
						return 0;
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed", __LINE__ );
				dlclose(hOnloadInfo->m_pCAL_nexH264_Handle);
				hOnloadInfo->m_pCAL_nexH264_Handle = NULL;	
				return 1;
			}

			//eNEX_CODEC_V_MPEG4V_S
			if( uCodecObjectTypeIndication == eNEX_CODEC_V_MPEG4V && NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC)  && !NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC))
			{
				if(bRelease)
					return 0;

				if( hOnloadInfo->m_pCAL_nexMP4_Handle == NULL && strlen(hOnloadInfo->m_strLibCALSWMP4File) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALSWMP4File);
					hOnloadInfo->m_pCAL_nexMP4_Handle = dlopen(hOnloadInfo->m_strLibCALSWMP4File, RTLD_NOW);
					if( hOnloadInfo->m_pCAL_nexMP4_Handle == NULL )
					{
						hOnloadInfo->m_pCAL_nexMP4_Handle = dlopen(g_strCodec_nex_MP4, RTLD_NOW);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] codec file not found so try again system", __LINE__);
					}
				}
			
				if( hOnloadInfo->m_pCAL_nexMP4_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_pCAL_nexMP4_Handle, dlerror());
					return 1;
				}
		
				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_nexMP4_Handle, "getNexCAL_Wrap_Body");
				if( getHandle )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2*)getHandle(	eNEX_CODEC_V_MPEG4V, 
																	NEXCAL_SW_API_VERSION, 
																	NEXCAL_VERSION_MAJOR, 
																	NEXCAL_VERSION_MINOR, 
																	NEXCAL_VERSION_PATCH);
					if( pGot )
					{
						NEXCALVideoDecoder* pCodecFunc = (NEXCALVideoDecoder*)pFuncs;

						pCodecFunc->GetProperty = (NEXCALGetProperty)pGot->func[11];
						pCodecFunc->SetProperty = (NEXCALSetProperty)pGot->func[6];
						pCodecFunc->Init = (NEXCALVideoInit)pGot->func[2];
						pCodecFunc->Deinit = (NEXCALVideoDeinit)pGot->func[3];
						pCodecFunc->Decode = (NEXCALVideoDecode)pGot->func[12];
						pCodecFunc->GetOutput = (NEXCALVideoGetOutput)pGot->func[8];
						pCodecFunc->Reset = (NEXCALVideoReset)pGot->func[5];
						return 0;
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed", __LINE__ );
				dlclose(hOnloadInfo->m_pCAL_nexMP4_Handle);
				hOnloadInfo->m_pCAL_nexMP4_Handle = NULL;	
				return 1;
			}
			
			if( uCodecObjectTypeIndication == eNEX_CODEC_V_H264 || uCodecObjectTypeIndication == eNEX_CODEC_V_MPEG4V || uCodecObjectTypeIndication == eNEX_CODEC_V_H264_MC_S || uCodecObjectTypeIndication == eNEX_CODEC_V_HEVC)
			{
				if( bRelease )
					return 0;
				
#ifdef FOR_TEST_MEDIACODEC_DEC	

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d]getCALOnLoad(0x%x): H264 and MPEG4V", __LINE__, uCodecObjectTypeIndication);
				
				if( hOnloadInfo->m_pCAL_MC_Handle == NULL && strlen(hOnloadInfo->m_strLibCALMcFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile);
					hOnloadInfo->m_pCAL_MC_Handle = dlopen(hOnloadInfo->m_strLibCALMcFile, RTLD_NOW);
				}

				if( hOnloadInfo->m_pCAL_MC_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile, dlerror());
					return 1;
				}
                
                //yoon
                typedef void (*SetJavaVMforCodec)(JavaVM* javaVM);
                SetJavaVMforCodec setvm = (SetJavaVMforCodec)dlsym(hOnloadInfo->m_pCAL_MC_Handle, "SetJavaVMforCodec");
                if(setvm != NULL)
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] SetJavaVMforCodec(%p) ", __LINE__, s_vm);
                    setvm(s_vm);
                }
                else
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] SetJavaVMforCodec found fail! ", __LINE__);
                }
                    
				if(uCodecObjectTypeIndication == eNEX_CODEC_V_H264 && NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC))
				{
					uCodecObjectTypeIndication = (NEX_CODEC_TYPE)eNEX_CODEC_V_H264_MC_S;
				}

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_MC_Handle, "getNexCAL_MC_Decoder");

				if( getHandle != NULL)
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2*)getHandle(uCodecObjectTypeIndication, 
																NEXCAL_MC_API_VERSION,
																NEXCAL_VERSION_MAJOR, 
																NEXCAL_VERSION_MINOR, 
																NEXCAL_VERSION_PATCH);
					if( pGot != NULL)
					{
						NEXCALCodec2 vdec2;
						memcpy(&vdec2, pGot, sizeof(NEXCALCodec2));

						//------------------------------------------------------------------------------------------
						// descrambler
						//------------------------------------------------------------------------------------------
						size_t curIdx = 13-1;

						for (int i = 0; i < 10; ++i)
						{
							void *tmpFunc = vdec2.func[curIdx];

							size_t shufKey = ((size_t)tmpFunc) >> 2;
							size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

							//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

							if (toIdx != curIdx)
							{
								if (curIdx < toIdx)
								{
									curIdx += 13;
								}
								for (size_t j = curIdx; j > toIdx; --j)
								{
									vdec2.func[j%13] = vdec2.func[(j-1)%13];
								}
								vdec2.func[toIdx%13] = tmpFunc;
							}
							curIdx = (toIdx + 13-1) % 13;
						}

						if (13-1 != curIdx)
						{
							size_t offset = 1 + curIdx;
							void *tmpFunc[2] = { NULL, vdec2.func[curIdx] };
							for (int i = 0; i < 13; ++i)
							{
								curIdx = (curIdx + 13-offset) % 13;
								tmpFunc[i%2] = vdec2.func[curIdx];
								vdec2.func[curIdx] = tmpFunc[(i+1)%2];
							}
						}

						NEXCALVideoDecoder* pCodecFunc = (NEXCALVideoDecoder*)pFuncs;

#ifdef FOR_USE_CUSTOM_CAL_VIDEO_GETPROPERTY
						g_hOnloadInfo.m_CAL_VideoGetProperty	= (NEXCALGetProperty)vdec2.func[4];
						pCodecFunc->GetProperty = nexCustom_NEXCALGetProperty;
#else
						pCodecFunc->GetProperty = (NEXCALGetProperty)vdec2.func[4];
#endif
						pCodecFunc->SetProperty = (NEXCALSetProperty)vdec2.func[5];
						pCodecFunc->Init = (NEXCALVideoInit)vdec2.func[6];
						pCodecFunc->Deinit = (NEXCALVideoDeinit)vdec2.func[7];
						pCodecFunc->Decode = (NEXCALVideoDecode)vdec2.func[8];
						pCodecFunc->GetOutput = (NEXCALVideoGetOutput)vdec2.func[9];
						pCodecFunc->Reset = (NEXCALVideoReset)vdec2.func[10];
						pCodecFunc->GetInfo = (NEXCALVideoGetInfo)vdec2.func[11];

						// for Extra Function 
						NEXCALCodec2 *pExtraFunc = (NEXCALCodec2 *)vdec2.func[12];
						if( pExtraFunc )
						{
							NEXCALCodec2 hExtraFunc;
							memcpy(&hExtraFunc, pExtraFunc, sizeof(NEXCALCodec2));

							//------------------------------------------------------------------------------------------
							// descrambler
							//------------------------------------------------------------------------------------------
							size_t curIdx = 13-1;

							for (int i = 0; i < 10; ++i)
							{
								void *tmpFunc = hExtraFunc.func[curIdx];

								size_t shufKey = ((size_t)tmpFunc) >> 2;
								size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

								//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

								if (toIdx != curIdx)
								{
									if (curIdx < toIdx)
									{
										curIdx += 13;
									}
									for (size_t j = curIdx; j > toIdx; --j)
									{
										hExtraFunc.func[j%13] = hExtraFunc.func[(j-1)%13];
									}
									hExtraFunc.func[toIdx%13] = tmpFunc;
								}
								curIdx = (toIdx + 13-1) % 13;
							}

							if (13-1 != curIdx)
							{
								size_t offset = 1 + curIdx;
								void *tmpFunc[2] = { NULL, hExtraFunc.func[curIdx] };
								for (int i = 0; i < 13; ++i)
								{
									curIdx = (curIdx + 13-offset) % 13;
									tmpFunc[i%2] = hExtraFunc.func[curIdx];
									hExtraFunc.func[curIdx] = tmpFunc[(i+1)%2];
								}
							}
							
							if( hOnloadInfo->m_pCAL_MC_Handle_CallbackFunc != hExtraFunc.func[4] )
								hOnloadInfo->m_pCAL_MC_Handle_CallbackFunc = hExtraFunc.func[4];
						}

						fnNexCAL_setDebugLevel mcDebugLevel = (fnNexCAL_setDebugLevel)vdec2.func[2];
						if( mcDebugLevel != NULL )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[porting_android.cpp %d] Decoder nexCAL_setDebugLevel(%d)", __LINE__, hOnloadInfo->m_iLogLevel);
							mcDebugLevel(0xFFFFFFFF, hOnloadInfo->m_iLogLevel);
#ifdef ENABLE_VIDEO_MC_DEC_LOG					
							// mcDebugLevel(-1, 0);
							// mcDebugLevel(NTHFROMLSBMASK(1), 4);
							// mcDebugLevel(NTHFROMLSBMASK(2), 6);
							// mcDebugLevel(NTHFROMLSBMASK(0), 3);
							// mcDebugLevel(NTHFROMLSBMASK( 0 ), 3);
							mcDebugLevel(0xffffffff, 1000);
							// mcDebugLevel(0xffffffff, 1);
#endif
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] succeeded to get Native Video Decoder!!!", __LINE__);
						return 0;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function table failed", __LINE__ );
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function handle failed", __LINE__ );
				return 1;
#else
				if( hOnloadInfo->m_pVideo_OC_Handle == NULL && strlen(hOnloadInfo->m_strLibVideoOcFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibVideoOcFile);
					hOnloadInfo->m_pVideo_OC_Handle = dlopen(hOnloadInfo->m_strLibVideoOcFile, RTLD_NOW);
				}

				if( hOnloadInfo->m_pVideo_OC_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_strLibVideoOcFile, dlerror());
					return 1;
				}

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pVideo_OC_Handle, "getNexCAL_OC_VideoDecoder");
				if( getHandle != NULL )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2*)getHandle(	uCodecObjectTypeIndication, 
																	NEXCAL_OC_DEC_API_VERSION, 
																	NEXCAL_VERSION_MAJOR, 
																	NEXCAL_VERSION_MINOR, 
																	NEXCAL_VERSION_PATCH);

					if( pGot != NULL )
					{
						NEXCALCodec2 vdec2;
						memcpy(&vdec2, pGot, sizeof(NEXCALCodec2));

						size_t curIdx = 13-1;

						for (int i = 0; i < 10; ++i)
						{
							void *tmpFunc = vdec2.func[curIdx];

							size_t shufKey = ((size_t)tmpFunc) >> 2;
							size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

							//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

							if (toIdx != curIdx)
							{
								if (curIdx < toIdx)
								{
									curIdx += 13;
								}
								for (size_t j = curIdx; j > toIdx; --j)
								{
									vdec2.func[j%13] = vdec2.func[(j-1)%13];
								}
								vdec2.func[toIdx%13] = tmpFunc;
							}
							curIdx = (toIdx + 13-1) % 13;
						}

						if (13-1 != curIdx)
						{
							size_t offset = 1 + curIdx;
							void *tmpFunc[2] = { NULL, vdec2.func[curIdx] };
							for (int i = 0; i < 13; ++i)
							{
								curIdx = (curIdx + 13-offset) % 13;
								tmpFunc[i%2] = vdec2.func[curIdx];
								vdec2.func[curIdx] = tmpFunc[(i+1)%2];
							}
						}

						NEXCALVideoDecoder* pCodecFunc = (NEXCALVideoDecoder*)pFuncs;

						pCodecFunc->GetProperty = (NEXCALGetProperty)vdec2.func[4];
						pCodecFunc->SetProperty = (NEXCALSetProperty)vdec2.func[5];
						pCodecFunc->Init = (NEXCALVideoInit)vdec2.func[6];
						pCodecFunc->Deinit = (NEXCALVideoDeinit)vdec2.func[7];
						pCodecFunc->Decode = (NEXCALVideoDecode)vdec2.func[8];
						pCodecFunc->GetOutput = (NEXCALVideoGetOutput)vdec2.func[9];
						pCodecFunc->Reset = (NEXCALVideoReset)vdec2.func[10];
						pCodecFunc->GetInfo = (NEXCALVideoGetInfo)vdec2.func[11];

						fnNexCAL_setDebugLevel ocVideoSetDebugLevel = (fnNexCAL_setDebugLevel)vdec2.func[2];
						if( ocVideoSetDebugLevel != NULL )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[porting_android.cpp %d] Decoder nexCAL_setDebugLevel(%d)", __LINE__, hOnloadInfo->m_iLogLevel);
							ocVideoSetDebugLevel(0xFFFFFFFF, hOnloadInfo->m_iLogLevel);
#ifdef ENABLE_VIDEO_OC_DEC_LOG
							ocVideoSetDebugLevel(0xFFFFFFFF, 1000);
							// ocVideoSetDebugLevel(5, 2);
#endif
						}

						/*
						if( ppQueryHandler )
						{
							*ppQueryHandler = CalQueryHandlerForHW;
							*puUserDataForQueryHandler = (unsigned int)&g_hOnloadInfo.m_QueryUserData;
						}
						*/

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] succeeded to get Native Video Decoder!!!", __LINE__);
						return 0;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function table failed", __LINE__ );
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function handle failed", __LINE__ );
				return 1;
#endif
			}
		}
		else if( eMode == NEXCAL_MODE_ENCODER )
		{
			if( uCodecObjectTypeIndication == eNEX_CODEC_V_H264 || uCodecObjectTypeIndication == 0x50010301 || uCodecObjectTypeIndication == eNEX_CODEC_V_HEVC || uCodecObjectTypeIndication == eNEX_CODEC_V_MPEG4V)
			{
				if( bRelease )
					return 0;
				
#ifdef FOR_TEST_MEDIACODEC_DEC	
				if( hOnloadInfo->m_pCAL_MC_Handle == NULL && strlen(hOnloadInfo->m_strLibCALMcFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile);
					hOnloadInfo->m_pVideo_OC_Handle = dlopen(hOnloadInfo->m_strLibCALMcFile, RTLD_NOW);
				}
				
				if( hOnloadInfo->m_pCAL_MC_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile, dlerror());
					return 1;
				}
				
				if(uCodecObjectTypeIndication == eNEX_CODEC_V_H264 && NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC))
				{
					uCodecObjectTypeIndication = (NEX_CODEC_TYPE)0x50010301;
				}
				
				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_MC_Handle, "getNexCAL_MC_Encoder");
				if( getHandle != NULL )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2 *)getHandle(		uCodecObjectTypeIndication, 
																		NEXCAL_MC_API_VERSION, 
																		NEXCAL_VERSION_MAJOR, 
																		NEXCAL_VERSION_MINOR, 
																		NEXCAL_VERSION_PATCH);

					if( pGot != NULL )
					{
					
						NEXCALCodec2 venc2;
						memcpy(&venc2, pGot, sizeof(NEXCALCodec2));

						size_t curIdx = 13-1;

						for (int i = 0; i < 10; ++i)
						{
							void *tmpFunc = venc2.func[curIdx];

							size_t shufKey = ((size_t)tmpFunc) >> 2;
							size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

							//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

							if (toIdx != curIdx)
							{
								if (curIdx < toIdx)
								{
									curIdx += 13;
								}
								for (size_t j = curIdx; j > toIdx; --j)
								{
									venc2.func[j%13] = venc2.func[(j-1)%13];
								}
								venc2.func[toIdx%13] = tmpFunc;
							}
							curIdx = (toIdx + 13-1) % 13;
						}

						if (13-1 != curIdx)
						{
							size_t offset = 1 + curIdx;
							void *tmpFunc[2] = { NULL, venc2.func[curIdx] };
							for (int i = 0; i < 13; ++i)
							{
								curIdx = (curIdx + 13-offset) % 13;
								tmpFunc[i%2] = venc2.func[curIdx];
								venc2.func[curIdx] = tmpFunc[(i+1)%2];
							}
						}

						NEXCALVideoEncoder* pCodecFunc = (NEXCALVideoEncoder*)pFuncs;

						pCodecFunc->GetProperty = (NEXCALGetProperty)venc2.func[4];
						pCodecFunc->SetProperty = (NEXCALSetProperty)venc2.func[5];
						pCodecFunc->Init = (NEXCALVideoEncoderInit)venc2.func[6];
						pCodecFunc->Deinit = (NEXCALVideoEncoderDeinit)venc2.func[7];
						pCodecFunc->Encode = (NEXCALVideoEncoderEncode)venc2.func[8];
						pCodecFunc->GetOutput = (NEXCALVideoEncoderGetOutput)venc2.func[9];
						pCodecFunc->Skip = (NEXCALVideoEncoderSkip)venc2.func[10];

						// for Extra Function 
						NEXCALCodec2 *pExtraFunc = (NEXCALCodec2 *)venc2.func[12];
						if( pExtraFunc )
						{
							NEXCALCodec2 hExtraFunc;
							memcpy(&hExtraFunc, pExtraFunc, sizeof(NEXCALCodec2));

							//------------------------------------------------------------------------------------------
							// descrambler
							//------------------------------------------------------------------------------------------
							size_t curIdx = 13-1;

							for (int i = 0; i < 10; ++i)
							{
								void *tmpFunc = hExtraFunc.func[curIdx];

								size_t shufKey = ((size_t)tmpFunc) >> 2;
								size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

								//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

								if (toIdx != curIdx)
								{
									if (curIdx < toIdx)
									{
										curIdx += 13;
									}
									for (size_t j = curIdx; j > toIdx; --j)
									{
										hExtraFunc.func[j%13] = hExtraFunc.func[(j-1)%13];
									}
									hExtraFunc.func[toIdx%13] = tmpFunc;
								}
								curIdx = (toIdx + 13-1) % 13;
							}

							if (13-1 != curIdx)
							{
								size_t offset = 1 + curIdx;
								void *tmpFunc[2] = { NULL, hExtraFunc.func[curIdx] };
								for (int i = 0; i < 13; ++i)
								{
									curIdx = (curIdx + 13-offset) % 13;
									tmpFunc[i%2] = hExtraFunc.func[curIdx];
									hExtraFunc.func[curIdx] = tmpFunc[(i+1)%2];
								}
							}
							
							if( hOnloadInfo->m_pCAL_MC_Handle_getInputSurf != hExtraFunc.func[5] )
								hOnloadInfo->m_pCAL_MC_Handle_getInputSurf = hExtraFunc.func[5];
							if( hOnloadInfo->m_pCAL_MC_Handle_setTimeStampOnSurf != hExtraFunc.func[6] )
								hOnloadInfo->m_pCAL_MC_Handle_setTimeStampOnSurf = hExtraFunc.func[6];
							if( hOnloadInfo->m_pCAL_MC_Handle_resetVideoEncoder != hExtraFunc.func[7] )
								hOnloadInfo->m_pCAL_MC_Handle_resetVideoEncoder = hExtraFunc.func[7];
							
							if( hOnloadInfo->m_pCAL_MC_Handle_setCropAchieveResolution != hExtraFunc.func[11] )
								hOnloadInfo->m_pCAL_MC_Handle_setCropAchieveResolution = hExtraFunc.func[11];
						}

						fnNexCAL_setDebugLevel ocVideoSetDebugLevel = (fnNexCAL_setDebugLevel)venc2.func[2];
						if( ocVideoSetDebugLevel != NULL )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[porting_android.cpp %d] Encoder nexCAL_setDebugLevel(%d)", __LINE__, hOnloadInfo->m_iLogLevel);
							ocVideoSetDebugLevel(0xFFFFFFFF, hOnloadInfo->m_iLogLevel);
							// ocVideoSetDebugLevel(NTHFROMLSBMASK(4), 4);
#ifdef ENABLE_VIDEO_MC_ENC_LOG
							ocVideoSetDebugLevel(0xFFFFFFFF, 1000);
#endif
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] succeeded to get Native Video Encoder!!!", __LINE__);
						return 0;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function table failed", __LINE__ );
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function handle failed", __LINE__ );
				return 1;
#else
				if( hOnloadInfo->m_pVideo_OC_Handle == NULL && strlen(hOnloadInfo->m_strLibVideoOcFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibVideoOcFile);
					hOnloadInfo->m_pVideo_OC_Handle = dlopen(hOnloadInfo->m_strLibVideoOcFile, RTLD_NOW);
				}
				
				if( hOnloadInfo->m_pVideo_OC_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_strLibVideoOcFile, dlerror());
					return 1;
				}
				
				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pVideo_OC_Handle, "getNexCAL_OC_VideoEncoder");
				if( getHandle != NULL )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2 *)getHandle(		eNEX_CODEC_V_H264, 
																		NEXCAL_OC_ENC_API_VERSION, 
																		NEXCAL_VERSION_MAJOR, 
																		NEXCAL_VERSION_MINOR, 
																		NEXCAL_VERSION_PATCH);

					if( pGot != NULL )
					{
					
						NEXCALCodec2 venc2;
						memcpy(&venc2, pGot, sizeof(NEXCALCodec2));

						size_t curIdx = 13-1;

						for (int i = 0; i < 10; ++i)
						{
							void *tmpFunc = venc2.func[curIdx];

							size_t shufKey = ((size_t)tmpFunc) >> 2;
							size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

							//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

							if (toIdx != curIdx)
							{
								if (curIdx < toIdx)
								{
									curIdx += 13;
								}
								for (size_t j = curIdx; j > toIdx; --j)
								{
									venc2.func[j%13] = venc2.func[(j-1)%13];
								}
								venc2.func[toIdx%13] = tmpFunc;
							}
							curIdx = (toIdx + 13-1) % 13;
						}

						if (13-1 != curIdx)
						{
							size_t offset = 1 + curIdx;
							void *tmpFunc[2] = { NULL, venc2.func[curIdx] };
							for (int i = 0; i < 13; ++i)
							{
								curIdx = (curIdx + 13-offset) % 13;
								tmpFunc[i%2] = venc2.func[curIdx];
								venc2.func[curIdx] = tmpFunc[(i+1)%2];
							}
						}

						NEXCALVideoEncoder* pCodecFunc = (NEXCALVideoEncoder*)pFuncs;

						pCodecFunc->GetProperty = (NEXCALGetProperty)venc2.func[4];
						pCodecFunc->SetProperty = (NEXCALSetProperty)venc2.func[5];
						pCodecFunc->Init = (NEXCALVideoEncoderInit)venc2.func[6];
						pCodecFunc->Deinit = (NEXCALVideoEncoderDeinit)venc2.func[7];
						pCodecFunc->Encode = (NEXCALVideoEncoderEncode)venc2.func[8];
						pCodecFunc->GetOutput = (NEXCALVideoEncoderGetOutput)venc2.func[9];
						pCodecFunc->Skip = (NEXCALVideoEncoderSkip)venc2.func[10];


						fnNexCAL_setDebugLevel ocVideoSetDebugLevel = (fnNexCAL_setDebugLevel)venc2.func[2];
						if( ocVideoSetDebugLevel != NULL )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[porting_android.cpp %d] Encoder nexCAL_setDebugLevel(%d)", __LINE__, hOnloadInfo->m_iLogLevel);
							ocVideoSetDebugLevel(0xFFFFFFFF, hOnloadInfo->m_iLogLevel);
							// ocVideoSetDebugLevel(NTHFROMLSBMASK(4), 4);
#ifdef ENABLE_VIDEO_OC_ENC_LOG
							ocVideoSetDebugLevel(0xFFFFFFFF, 1000);
#endif
						}
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] succeeded to get Native Video Encoder!!!", __LINE__);
						return 0;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function table failed", __LINE__ );
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function handle failed", __LINE__ );
				return 1;
#endif				
			}
		}
	}

	if( eMediaType == NEXCAL_MEDIATYPE_AUDIO )
	{
		if( eMode == NEXCAL_MODE_DECODER )
		{	// eNEX_CODEC_A_AAC_S
			if( uCodecObjectTypeIndication == eNEX_CODEC_A_AAC && NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC) && !NEXCAL_CHECK_STATIC_PROPERTY(uStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC))
			{
				if( bRelease )
					return 0;
				
				if( hOnloadInfo->m_pCAL_nexaudio_Handle == NULL && strlen(hOnloadInfo->m_strLibCALSWAudioFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALSWAudioFile);
					hOnloadInfo->m_pCAL_nexaudio_Handle = dlopen(hOnloadInfo->m_strLibCALSWAudioFile, RTLD_NOW);
					if( hOnloadInfo->m_pCAL_nexaudio_Handle == NULL )
					{
						hOnloadInfo->m_pCAL_nexaudio_Handle = dlopen(g_strCodec_nex_audio, RTLD_NOW);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] codec file not found so try again system", __LINE__);
					}
				}

				if( hOnloadInfo->m_pCAL_nexaudio_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_pCAL_nexaudio_Handle, dlerror());
					return 1;
				}

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_nexaudio_Handle, "getNexCAL_Wrap_Body");
				if( getHandle )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2 *)getHandle(		eNEX_CODEC_A_AAC, 
																		NEXCAL_SW_API_VERSION, 
																		NEXCAL_VERSION_MAJOR, 
																		NEXCAL_VERSION_MINOR, 
																		NEXCAL_VERSION_PATCH);
					if( pGot )
					{
						NEXCALAudioDecoder*	pAudioDec = (NEXCALAudioDecoder*)pFuncs;
						pAudioDec->GetProperty = (NEXCALGetProperty)pGot->func[11];
						pAudioDec->SetProperty = (NEXCALSetProperty)pGot->func[6];
						pAudioDec->Init = (NEXCALAudioInit)pGot->func[2];
						pAudioDec->Deinit = (NEXCALAudioDeinit)pGot->func[3];
						pAudioDec->Decode = (NEXCALAudioDecode)pGot->func[12];
						pAudioDec->Reset = (NEXCALAudioReset)pGot->func[5];
						pAudioDec->GetInfo = (NEXCALAudioGetInfo)pGot->func[9];
						return 0;
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed", __LINE__ );
				dlclose(hOnloadInfo->m_pCAL_nexaudio_Handle);
				hOnloadInfo->m_pCAL_nexaudio_Handle = NULL;	
				return 1;
			}			

			if( IS_PCM_Codec(uCodecObjectTypeIndication))
			{
				if( bRelease )
					return 0;
				
				if( hOnloadInfo->m_pCAL_nexPCM_Handle == NULL && strlen(hOnloadInfo->m_strLibCALSWPCMFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALSWPCMFile);
					hOnloadInfo->m_pCAL_nexPCM_Handle = dlopen(hOnloadInfo->m_strLibCALSWPCMFile, RTLD_NOW);
					if( hOnloadInfo->m_pCAL_nexPCM_Handle == NULL )
					{
						hOnloadInfo->m_pCAL_nexPCM_Handle = dlopen(g_strCodec_nex_pcm, RTLD_NOW);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] codec file not found so try again system", __LINE__);
					}
				}

				if( hOnloadInfo->m_pCAL_nexPCM_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_pCAL_nexPCM_Handle, dlerror());
					return 1;
				}

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_nexPCM_Handle, "getNexCAL_Wrap_Body");
				if( getHandle )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2 *)getHandle(		uCodecObjectTypeIndication, 
																		NEXCAL_SW_API_VERSION, 
																		NEXCAL_VERSION_MAJOR, 
																		NEXCAL_VERSION_MINOR, 
																		NEXCAL_VERSION_PATCH);
					if( pGot )
					{
						NEXCALAudioDecoder*	pAudioDec = (NEXCALAudioDecoder*)pFuncs;
						pAudioDec->GetProperty = (NEXCALGetProperty)pGot->func[11];
						pAudioDec->SetProperty = (NEXCALSetProperty)pGot->func[6];
						pAudioDec->Init = (NEXCALAudioInit)pGot->func[2];
						pAudioDec->Deinit = (NEXCALAudioDeinit)pGot->func[3];
						pAudioDec->Decode = (NEXCALAudioDecode)pGot->func[12];
						pAudioDec->Reset = (NEXCALAudioReset)pGot->func[5];
						pAudioDec->GetInfo = (NEXCALAudioGetInfo)pGot->func[9];
						return 0;
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed", __LINE__ );
				dlclose(hOnloadInfo->m_pCAL_nexPCM_Handle);
				hOnloadInfo->m_pCAL_nexPCM_Handle = NULL;	
				return 1;
			}	

			if( uCodecObjectTypeIndication == eNEX_CODEC_A_FLAC)
			{
				if( bRelease )
					return 0;
				
				if( hOnloadInfo->m_pCAL_nexFLAC_Handle == NULL && strlen(hOnloadInfo->m_strLibCALSWFLACFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALSWFLACFile);
					hOnloadInfo->m_pCAL_nexFLAC_Handle = dlopen(hOnloadInfo->m_strLibCALSWFLACFile, RTLD_NOW);
					if( hOnloadInfo->m_pCAL_nexFLAC_Handle == NULL )
					{
						hOnloadInfo->m_pCAL_nexFLAC_Handle = dlopen(g_strCodec_nex_flac, RTLD_NOW);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] codec file not found so try again system", __LINE__);
					}
				}

				if( hOnloadInfo->m_pCAL_nexFLAC_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%d)", __LINE__, hOnloadInfo->m_pCAL_nexFLAC_Handle, dlerror());
					return 1;
				}

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_nexFLAC_Handle, "getNexCAL_Wrap_Body");
				if( getHandle )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2 *)getHandle(		uCodecObjectTypeIndication, 
																		NEXCAL_SW_API_VERSION, 
																		NEXCAL_VERSION_MAJOR, 
																		NEXCAL_VERSION_MINOR, 
																		NEXCAL_VERSION_PATCH);
					if( pGot )
					{
						NEXCALAudioDecoder*	pAudioDec = (NEXCALAudioDecoder*)pFuncs;
						pAudioDec->GetProperty = (NEXCALGetProperty)pGot->func[11];
						pAudioDec->SetProperty = (NEXCALSetProperty)pGot->func[6];
						pAudioDec->Init = (NEXCALAudioInit)pGot->func[2];
						pAudioDec->Deinit = (NEXCALAudioDeinit)pGot->func[3];
						pAudioDec->Decode = (NEXCALAudioDecode)pGot->func[12];
						pAudioDec->Reset = (NEXCALAudioReset)pGot->func[5];
						pAudioDec->GetInfo = (NEXCALAudioGetInfo)pGot->func[9];
						return 0;
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed", __LINE__ );
				dlclose(hOnloadInfo->m_pCAL_nexFLAC_Handle);
				hOnloadInfo->m_pCAL_nexFLAC_Handle = NULL;	
				return 1;
			}	
			
			if( uCodecObjectTypeIndication == eNEX_CODEC_A_AAC || uCodecObjectTypeIndication == eNEX_CODEC_A_AACPLUS || uCodecObjectTypeIndication == eNEX_CODEC_A_MP3 || uCodecObjectTypeIndication == eNEX_CODEC_A_AMR || uCodecObjectTypeIndication == eNEX_CODEC_A_AMRWB)
			{

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d]getCALOnLoad(0x%x) AAC or MP3", __LINE__, uCodecObjectTypeIndication);
				
				if( bRelease )
					return 0;

				if( hOnloadInfo->m_pCAL_MC_Handle == NULL && strlen(hOnloadInfo->m_strLibCALMcFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile);
					hOnloadInfo->m_pCAL_MC_Handle = dlopen(hOnloadInfo->m_strLibCALMcFile, RTLD_NOW);
				}

				if( hOnloadInfo->m_pCAL_MC_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile, dlerror());
					return 1;
				}

                //yoon
                typedef void (*SetJavaVMforCodec)(JavaVM* javaVM);
                SetJavaVMforCodec setvm = (SetJavaVMforCodec)dlsym(hOnloadInfo->m_pCAL_MC_Handle, "SetJavaVMforCodec");
                if(setvm != NULL)
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] SetJavaVMforCodec(%p) ", __LINE__, s_vm);
                    setvm(s_vm);
                }
                else
                {
                    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] SetJavaVMforCodec found fail! ", __LINE__);
                }

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_MC_Handle, "getNexCAL_MC_Decoder");
				if( getHandle != NULL)
				{
					NEXCALCodec2* pGot = (NEXCALCodec2*)getHandle(	uCodecObjectTypeIndication, 
																	NEXCAL_MC_API_VERSION,
																	NEXCAL_VERSION_MAJOR, 
																	NEXCAL_VERSION_MINOR, 
																	NEXCAL_VERSION_PATCH);

					if( pGot != NULL)
					{
						NEXCALCodec2 	adec2;
						memcpy(&adec2, pGot, sizeof(NEXCALCodec2));

						//------------------------------------------------------------------------------------------
						// descrambler
						//------------------------------------------------------------------------------------------
						size_t curIdx = 13-1;

						for (int i = 0; i < 10; ++i)
						{
							void *tmpFunc = adec2.func[curIdx];

							size_t shufKey = ((size_t)tmpFunc) >> 2;
							size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

							//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

							if (toIdx != curIdx)
							{
								if (curIdx < toIdx)
								{
									curIdx += 13;
								}
								for (size_t j = curIdx; j > toIdx; --j)
								{
									adec2.func[j%13] = adec2.func[(j-1)%13];
								}
								adec2.func[toIdx%13] = tmpFunc;
							}
							curIdx = (toIdx + 13-1) % 13;
						}

						if (13-1 != curIdx)
						{
							size_t offset = 1 + curIdx;
							void *tmpFunc[2] = { NULL, adec2.func[curIdx] };
							for (int i = 0; i < 13; ++i)
							{
								curIdx = (curIdx + 13-offset) % 13;
								tmpFunc[i%2] = adec2.func[curIdx];
								adec2.func[curIdx] = tmpFunc[(i+1)%2];
							}
						}

						NEXCALAudioDecoder*	pAudioDec = (NEXCALAudioDecoder*)pFuncs;
						pAudioDec->GetProperty = (NEXCALGetProperty)adec2.func[4];
						pAudioDec->SetProperty = (NEXCALSetProperty)adec2.func[5];
						pAudioDec->Init = (NEXCALAudioInit)adec2.func[6];
						pAudioDec->Deinit = (NEXCALAudioDeinit)adec2.func[7];
						pAudioDec->Decode = (NEXCALAudioDecode)adec2.func[8];
						pAudioDec->Reset = (NEXCALAudioReset)adec2.func[9];
						pAudioDec->GetInfo = (NEXCALAudioGetInfo)adec2.func[10];


						fnNexCAL_setDebugLevel mcAudioSetDebugLevel = (fnNexCAL_setDebugLevel)adec2.func[2];
						if( mcAudioSetDebugLevel != NULL )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[porting_android.cpp %d] Decoder nexCAL_setDebugLevel(%d)", __LINE__, hOnloadInfo->m_iLogLevel);
							mcAudioSetDebugLevel(0xFFFFFFFF, hOnloadInfo->m_iLogLevel);
							//mcAudioSetDebugLevel(0xFFFFFFFF, 0);

#ifdef ENABLE_AUDIO_MC_DEC_LOG					
							mcAudioSetDebugLevel(0xffffffff, 1000);
#endif
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] succeeded to get Native Audio Decoder!!!", __LINE__);
						return 0;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function table failed", __LINE__ );
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function handle failed", __LINE__ );
			}
		}
		else if( eMode == NEXCAL_MODE_ENCODER )
		{
			if( uCodecObjectTypeIndication == eNEX_CODEC_A_AAC )
			{
#ifdef SW_AACEnc
				if( bRelease )
					return 0;
				
				if( hOnloadInfo->m_pCAL_nexAACEnc_Handle == NULL && strlen(hOnloadInfo->m_strLibCALSWAACEncFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALSWAACEncFile);
					hOnloadInfo->m_pCAL_nexAACEnc_Handle = dlopen(hOnloadInfo->m_strLibCALSWAACEncFile, RTLD_NOW);
					if( hOnloadInfo->m_pCAL_nexAACEnc_Handle == NULL )
					{
						hOnloadInfo->m_pCAL_nexAACEnc_Handle = dlopen(g_strCodec_nex_aacEnc, RTLD_NOW);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] codec file not found so try again system", __LINE__);
					}
				}

				if( hOnloadInfo->m_pCAL_nexAACEnc_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%d)", __LINE__, hOnloadInfo->m_pCAL_nexAACEnc_Handle, dlerror());
					return 1;
				}

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_nexAACEnc_Handle, "getNexCAL_Wrap_Body");
				if( getHandle )
				{
					NEXCALCodec2 *pGot = (NEXCALCodec2 *)getHandle(		uCodecObjectTypeIndication, 
																		NEXCAL_SW_API_VERSION, 
																		NEXCAL_VERSION_MAJOR, 
																		NEXCAL_VERSION_MINOR, 
																		NEXCAL_VERSION_PATCH);
					if( pGot )
					{
						NEXCALAudioEncoder*	pAudioEnc = (NEXCALAudioEncoder*)pFuncs;
						pAudioEnc->GetProperty = (NEXCALGetProperty)pGot->func[11];
						pAudioEnc->SetProperty = (NEXCALSetProperty)pGot->func[6];
						pAudioEnc->Init = (NEXCALAudioEncoderInit)pGot->func[2];
						pAudioEnc->Deinit = (NEXCALAudioEncoderDeinit)pGot->func[3];
						pAudioEnc->Encode = (NEXCALAudioEncoderEncode)pGot->func[12];
						pAudioEnc->Reset = (NEXCALAudioEncoderReset)pGot->func[5];
						return 0;
					}
				}

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed", __LINE__ );
				dlclose(hOnloadInfo->m_pCAL_nexAACEnc_Handle);
				hOnloadInfo->m_pCAL_nexAACEnc_Handle = NULL;	
				return 1;
#else
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d]getCALOnLoad(0x%x): AAC Native Encoder", __LINE__, uCodecObjectTypeIndication);
				
				if( bRelease )
					return 0;

				if( hOnloadInfo->m_pCAL_MC_Handle == NULL && strlen(hOnloadInfo->m_strLibCALMcFile) > 0 )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile);
					hOnloadInfo->m_pCAL_MC_Handle = dlopen(hOnloadInfo->m_strLibCALMcFile, RTLD_NOW);
				}

				if( hOnloadInfo->m_pCAL_MC_Handle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s) failed(%s)", __LINE__, hOnloadInfo->m_strLibCALMcFile, dlerror());
					return 1;
				}

				GetHandle getHandle = (GetHandle)dlsym(hOnloadInfo->m_pCAL_MC_Handle, "getNexCAL_MC_Encoder");

				if( getHandle != NULL)
				{
					NEXCALCodec2* pGot = (NEXCALCodec2*)getHandle(uCodecObjectTypeIndication, 
																NEXCAL_MC_API_VERSION,
																NEXCAL_VERSION_MAJOR, 
																NEXCAL_VERSION_MINOR, 
																NEXCAL_VERSION_PATCH);
					if( pGot != NULL)
					{
						NEXCALCodec2 	aenc2;
						memcpy(&aenc2, pGot, sizeof(NEXCALCodec2));

						//------------------------------------------------------------------------------------------
						// descrambler
						//------------------------------------------------------------------------------------------
						size_t curIdx = 13-1;

						for (int i = 0; i < 10; ++i)
						{
							void *tmpFunc = aenc2.func[curIdx];

							size_t shufKey = ((size_t)tmpFunc) >> 2;
							size_t toIdx = ((curIdx + 3*13) - (((shufKey % 3) + 1) * ((shufKey * shufKey % 13) + 1))) % 13;

							//nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "i(%d) cur(%zu) shufK(%zu) to(%zu)", i, curIdx, shufKey, toIdx);

							if (toIdx != curIdx)
							{
								if (curIdx < toIdx)
								{
									curIdx += 13;
								}
								for (size_t j = curIdx; j > toIdx; --j)
								{
									aenc2.func[j%13] = aenc2.func[(j-1)%13];
								}
								aenc2.func[toIdx%13] = tmpFunc;
							}
							curIdx = (toIdx + 13-1) % 13;
						}

						if (13-1 != curIdx)
						{
							size_t offset = 1 + curIdx;
							void *tmpFunc[2] = { NULL, aenc2.func[curIdx] };
							for (int i = 0; i < 13; ++i)
							{
								curIdx = (curIdx + 13-offset) % 13;
								tmpFunc[i%2] = aenc2.func[curIdx];
								aenc2.func[curIdx] = tmpFunc[(i+1)%2];
							}
						}

						NEXCALAudioEncoder* pAudioEnc = (NEXCALAudioEncoder*)pFuncs;
						pAudioEnc->GetProperty = (NEXCALGetProperty)aenc2.func[4];
						pAudioEnc->SetProperty = (NEXCALSetProperty)aenc2.func[5];
						pAudioEnc->Init = (NEXCALAudioEncoderInit)aenc2.func[6];
						pAudioEnc->Deinit = (NEXCALAudioEncoderDeinit)aenc2.func[7];
						pAudioEnc->Encode = (NEXCALAudioEncoderEncode)aenc2.func[8];
						pAudioEnc->Reset = (NEXCALAudioEncoderReset)aenc2.func[9];


						fnNexCAL_setDebugLevel mcAudioSetDebugLevel = (fnNexCAL_setDebugLevel)aenc2.func[2];
						if( mcAudioSetDebugLevel != NULL )
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[porting_android.cpp %d] Decoder nexCAL_setDebugLevel(%d)", __LINE__, hOnloadInfo->m_iLogLevel);
							mcAudioSetDebugLevel(0xFFFFFFFF, hOnloadInfo->m_iLogLevel);
							//mcAudioSetDebugLevel(0xFFFFFFFF, 0);
#ifdef ENABLE_AUDIO_MC_ENC_LOG					
							mcAudioSetDebugLevel(0xFFFFFFFF, 1000);
#endif
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] succeeded to get Native Audio Encode!!!", __LINE__);
						return 0;
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function table failed", __LINE__ );
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] get codec function handle failed", __LINE__ );
#endif
			}
		}
	}

#ifdef BUILD_JPEG_DECODER
	if( eMediaType == NEXCAL_MEDIATYPE_IMAGE )
	{
		if( eMode == NEXCAL_MODE_DECODER )
		{
			if( uCodecObjectTypeIndication == eNEX_CODEC_V_JPEG )
			{
				if( bRelease )
					return 0;

				NEXCALImageDecoder CAL_JPEG =	{	nexCALBody_Image_JPEG_GetProperty, 
													nexCALBody_Image_JPEG_SetProperty, 
													nexCALBody_Image_JPEG_getHeader, 
													nexCALBody_Image_JPEG_Decoder};

				memcpy(pFuncs, &CAL_JPEG, sizeof(NEXCALImageDecoder));
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] getCALOnload eNEX_CODEC_V_JPEG dec sucessed", __LINE__);
				return 0;
			}
		}
	}
#endif
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] not using Codec(0x%x 0x%x 0x%x)", __LINE__, eMediaType, eMode, uCodecObjectTypeIndication);
	return 1;
}

void loadRAL( char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] loadRAL In(%s, %d %d)", __LINE__, strModelName, iAPILevel, iLogLevel);

	if( strLibPath == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] parameter error(0x%x %s %d)", __LINE__, strLibPath, strModelName, iAPILevel);
		return;
	}

#ifdef BUILD_NDK
	// memset(&g_hRalLoadInfo, 0x00, sizeof(RAL_LOAD_INFO));
	g_hRalLoadInfo.m_iLogLevel = iLogLevel;

	switch(iAPILevel)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] Not support android version(%d)", __LINE__, iAPILevel);
			return;
		}
		case 9: // GB
		case 10: // GB - MR1
		case 11: // HC
		case 12: // HC - MR1
		case 13: // HC - MR2
		case 14: // ICS
		case 15: // ICS - MR1
		case 16: // JELLY_BEAN
		case 17: // JELLY_BEAN - MR1
		case 18: // JELLY_BEAN - MR2
		case 19: // KITKAT
		default:
		{
			sprintf(g_hRalLoadInfo.m_strLibRALBodyAudio, "%s%s", strLibPath, g_strRAL_audio);
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] support android version(%d)", __LINE__, iAPILevel);
			break;
		}
		/*
		case 19: // KITKAT
		default:
		{
			sprintf(g_hRalLoadInfo.m_strLibRALBodyAudio, "%s%s", strLibPath, g_strRAL_audio_kk);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] support android version(%d)", __LINE__, iAPILevel);
			break;
		}
		*/
	}



#if 0
	char m_strLibTest[512];
	sprintf(m_strLibTest, "%s%s", strLibPath, "libwrapnexhevccalbody.so");

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen Test(%s)", __LINE__, m_strLibTest);

	void *dl_test_handle = dlopen(m_strLibTest, RTLD_NOW);
	if( dl_test_handle != NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen Test sucessed(0x%x %s)", __LINE__, dl_test_handle, m_strLibTest);
		dlclose(dl_test_handle);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen Test failed(%s)", __LINE__, dlerror());
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen Test failed(0x%x %s)", __LINE__, dl_test_handle, m_strLibTest);
	}
#endif


	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen(%s)", __LINE__, g_hRalLoadInfo.m_strLibRALBodyAudio);
	if( g_hRalLoadInfo.m_pRalAudio_dl_handle == NULL )
	{
		g_hRalLoadInfo.m_pRalAudio_dl_handle = dlopen(g_hRalLoadInfo.m_strLibRALBodyAudio, RTLD_NOW);
		if( g_hRalLoadInfo.m_pRalAudio_dl_handle == NULL )
		{
			const char *dlerrmsg = dlerror();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlopen failed: \"%s\"", __LINE__, dlerrmsg);
			return;
		}
	}


    //yoon
    typedef void (*SetJavaVMforRender)(JavaVM* javaVM);
    SetJavaVMforRender setvm = (SetJavaVMforRender)dlsym(g_hRalLoadInfo.m_pRalAudio_dl_handle, "SetJavaVMforRender");
    if(setvm != NULL)
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] SetJavaVMforRender(%p) ", __LINE__, s_vm);
        setvm(s_vm);
    }
    else
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] SetJavaVMforRender found fail! ", __LINE__);
    }    
    
	RalbodyAudio_GetHandle getHandle = (RalbodyAudio_GetHandle)dlsym(g_hRalLoadInfo.m_pRalAudio_dl_handle, "GetHandle");
	if( getHandle == NULL )
	{
		const char *dlerrmsg = dlerror();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed: \"%s\"", __LINE__, dlerrmsg);
		dlclose(g_hRalLoadInfo.m_pRalAudio_dl_handle);
		g_hRalLoadInfo.m_pRalAudio_dl_handle = NULL;
		return;
	}

	g_hRalLoadInfo.m_pAUDIO_RALBODY_FUNCTION_ST_handle = getHandle();
	if( g_hRalLoadInfo.m_pAUDIO_RALBODY_FUNCTION_ST_handle == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] Error: AudioRenderer's GetHandle returned NULL", __LINE__);
		dlclose(g_hRalLoadInfo.m_pRalAudio_dl_handle);
		g_hRalLoadInfo.m_pRalAudio_dl_handle = NULL;
		return;
	}
	// g_gRALAudioHandle = (AUDIO_RALBODY_FUNCTION_ST *)g_hRalLoadInfo.m_pAUDIO_RALBODY_FUNCTION_ST_handle;
#else

#error "Do not support platform build"

#endif

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] NEXRAL loaded succesfully!\n", __LINE__ );
	return;	
}

void unloadRAL()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] unloadRAL In", __LINE__);
	if( g_hRalLoadInfo.m_pRalAudio_dl_handle )
	{
		if( g_hRalLoadInfo.m_pAUDIO_RALBODY_FUNCTION_ST_handle )
		{
			RalbodyAudio_CloseHandle closeHandle = (RalbodyAudio_CloseHandle)dlsym(g_hRalLoadInfo.m_pRalAudio_dl_handle, "CloseHandle");
			if( closeHandle )
			{
				closeHandle(g_hRalLoadInfo.m_pAUDIO_RALBODY_FUNCTION_ST_handle);
			}
			else
			{
				const char *dlerrmsg = dlerror();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlsym failed: \"%s\"", __LINE__, dlerrmsg);
			}
			g_hRalLoadInfo.m_pAUDIO_RALBODY_FUNCTION_ST_handle = NULL;
		}

		dlclose(g_hRalLoadInfo.m_pRalAudio_dl_handle);
		g_hRalLoadInfo.m_pRalAudio_dl_handle = NULL;
	}
	// memset(&g_hRalLoadInfo, 0x00, sizeof(RAL_LOAD_INFO));
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] NEXRAL unloaded succesfully!\n", __LINE__ );
}

void registerCAL( char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] registerCAL In(%s, %d %d)", __LINE__, strModelName, iAPILevel, iLogLevel);

	if( strLibPath == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] parameter error(0x%x %s %d)", __LINE__, strLibPath, strModelName, iAPILevel);
		return;
	}
	
	if( g_hNexCAL == NULL )
	{
		g_hNexCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
	}

	if( g_hNexCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] nexCAL_GetHandle failed!\n", __LINE__ );
		return;
	}

#ifdef BUILD_NDK
	// memset(&g_hOnloadInfo, 0x00, sizeof(CAL_ONLOAD_INFO));
	g_hOnloadInfo.m_iLogLevel = iLogLevel;
	g_hOnloadInfo.m_iAPILevel = iAPILevel;

	strcpy(g_hOnloadInfo.m_QueryUserData.m_strModelName, strModelName);

	switch(iAPILevel)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9: // GB
		case 10: // GB - MR1
		case 11:
		case 12:
		case 13:
		case 14: // ICS
		case 15: // ICS - MR1
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] Not support android version(%d)", __LINE__, iAPILevel);
			break;
		}
		case 16: // JELLY_BEAN
		case 17: // JELLY_BEAN
		default:
		{
			sprintf(g_hOnloadInfo.m_strLibCALMcFile, "%s%s", strLibPath, g_strCodec_mc_jb);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] USE Native Media Codec!", __LINE__);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] support android version(%d)", __LINE__, iAPILevel);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] using gerneral mc codecs(%s)", __LINE__, g_hOnloadInfo.m_strLibCALMcFile);
			break;
		}
	};

#else

#error "Do not support platform build"

#endif

	sprintf(g_hOnloadInfo.m_strLibCALSWAudioFile, "%s%s", strLibPath, g_strCodec_nex_audio);
	sprintf(g_hOnloadInfo.m_strLibCALSWPCMFile, "%s%s", strLibPath, g_strCodec_nex_pcm);
	sprintf(g_hOnloadInfo.m_strLibCALSWFLACFile, "%s%s", strLibPath, g_strCodec_nex_flac);    
	sprintf(g_hOnloadInfo.m_strLibCALSWH264File, "%s%s", strLibPath, g_strCodec_nex_H264);
	sprintf(g_hOnloadInfo.m_strLibCALSWMP4File, "%s%s", strLibPath, g_strCodec_nex_MP4);
	sprintf(g_hOnloadInfo.m_strLibCALSWAACEncFile, "%s%s", strLibPath, g_strCodec_nex_aacEnc);

//	unsigned int uHWDecoderStaticProperties = NEXCAL_STATIC_PROPERTY_HW | NEXCAL_STATIC_PROPERTY_PATENT_OFF |
//                                                                                     NEXCAL_STATIC_PROPERTY_PERFORMACE_HIGH | NEXCAL_STATIC_PROPERTY_POWERCONSUMPTION_LOW;
	unsigned int uHWDecoderStaticProperties;
	NEXCAL_INIT_STATIC_PROPERTY(uHWDecoderStaticProperties);
	NEXCAL_SET_STATIC_PROPERTY(uHWDecoderStaticProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC); // hw codec 사용 설정

	NXUINT32 uSWDecoderStaticProperties;
	NEXCAL_INIT_STATIC_PROPERTY(uSWDecoderStaticProperties);
	NEXCAL_SET_STATIC_PROPERTY(uSWDecoderStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC); // sw codec 사용 설정

	NXUINT32 uPSWDecoderStaticProperties;
	NEXCAL_INIT_STATIC_PROPERTY(uPSWDecoderStaticProperties);
	NEXCAL_SET_STATIC_PROPERTY(uPSWDecoderStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC); // sw codec 사용 설정
	NEXCAL_SET_STATIC_PROPERTY(uPSWDecoderStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC); // platform sw codec 사용 설정
	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_ENCODER, eNEX_CODEC_A_AAC, getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_AAC, getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_AACPLUS, getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_MP3, getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_S16LE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_S16BE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_RAW, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_FL32LE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_FL32BE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_FL64LE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_FL64BE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_IN24LE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_IN24BE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_IN32LE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_IN32BE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_LPCMLE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_PCM_LPCMBE, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_AMR, getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWDecoderStaticProperties);
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_AMRWB, getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_FLAC, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	
	// nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_AAC/*eNEX_CODEC_A_AAC_S*/, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);

 	// nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_MP3/*eNEX_CODEC_A_MP3_S*/, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	
	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_ENCODER, eNEX_CODEC_V_H264 /*(NEX_CODEC_TYPE)0x50010301*/, getCALOnload, (NXVOID*)&g_hOnloadInfo,uPSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_ENCODER, eNEX_CODEC_V_H264, getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_H264, getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_H264/*(NEX_CODEC_TYPE)eNEX_CODEC_V_H264_S*/, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	
	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_H264/*(NEX_CODEC_TYPE)eNEX_CODEC_V_H264_MC_S*/, getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_ENCODER, eNEX_CODEC_V_HEVC, getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_HEVC, getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_ENCODER, eNEX_CODEC_V_MPEG4V, getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_MPEG4V, getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWDecoderStaticProperties);	
 	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_MPEG4V/*(NEX_CODEC_TYPE)eNEX_CODEC_V_MPEG4V_S*/, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	

#ifdef BUILD_JPEG_DECODER
	nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_IMAGE, NEXCAL_MODE_DECODER, eNEX_CODEC_V_JPEG, getCALOnload, (NXVOID*)&g_hOnloadInfo, uSWDecoderStaticProperties);	
#endif
 	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] NEXCAL initialized succesfully!", __LINE__ );
}


void unregisterCAL(int bSDK)
{
	if( g_hNexCAL )
	{
		nexCAL_ReleaseHandle( g_hNexCAL );
		g_hNexCAL = NULL;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] NEXCAL uninitialized succesfully!", __LINE__ );
	}

	if( bSDK )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlclose skip and uninitialized succesfully!", __LINE__ );
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_android.cpp %d] dlclose and uninitialized succesfully!", __LINE__ );
		if( g_hOnloadInfo.m_pVideo_OC_Handle )
		{
			dlclose(g_hOnloadInfo.m_pVideo_OC_Handle);
			g_hOnloadInfo.m_pVideo_OC_Handle = NULL;
		}

		if( g_hOnloadInfo.m_pCAL_MC_Handle)
		{
			dlclose(g_hOnloadInfo.m_pCAL_MC_Handle);
			g_hOnloadInfo.m_pCAL_MC_Handle = NULL;
		}	

		if( g_hOnloadInfo.m_pCAL_nexaudio_Handle)
		{
			dlclose(g_hOnloadInfo.m_pCAL_nexaudio_Handle);
			g_hOnloadInfo.m_pCAL_nexaudio_Handle = NULL;
		}

		if( g_hOnloadInfo.m_pCAL_nexPCM_Handle)
		{
			dlclose(g_hOnloadInfo.m_pCAL_nexPCM_Handle);
			g_hOnloadInfo.m_pCAL_nexPCM_Handle = NULL;
		}
        
		if( g_hOnloadInfo.m_pCAL_nexFLAC_Handle)
		{
			dlclose(g_hOnloadInfo.m_pCAL_nexFLAC_Handle);
			g_hOnloadInfo.m_pCAL_nexFLAC_Handle = NULL;
		}

		if( g_hOnloadInfo.m_pCAL_nexH264_Handle)
		{
			dlclose(g_hOnloadInfo.m_pCAL_nexH264_Handle);
			g_hOnloadInfo.m_pCAL_nexH264_Handle = NULL;
		}

		if( g_hOnloadInfo.m_pCAL_nexMP4_Handle)
		{
			dlclose(g_hOnloadInfo.m_pCAL_nexMP4_Handle);
			g_hOnloadInfo.m_pCAL_nexMP4_Handle = NULL;
		}
	}
}

void* getCALCallbackFunc()
{
	return g_hOnloadInfo.m_pCAL_MC_Handle_CallbackFunc;
}

void* getCALInputSurf()
{
	return g_hOnloadInfo.m_pCAL_MC_Handle_getInputSurf;
}

void* getCALsetTimeStampOnSurf()
{
	return g_hOnloadInfo.m_pCAL_MC_Handle_setTimeStampOnSurf;
}

void* getCALresetVideoEncoder()
{
	return g_hOnloadInfo.m_pCAL_MC_Handle_resetVideoEncoder;
}

void* getCALsetCropAchieveResolution()
{
	return g_hOnloadInfo.m_pCAL_MC_Handle_setCropAchieveResolution;
}

void* getRALgetAudioRenderFuncs()
{
	return (void*)g_hRalLoadInfo.m_pAUDIO_RALBODY_FUNCTION_ST_handle;
}


