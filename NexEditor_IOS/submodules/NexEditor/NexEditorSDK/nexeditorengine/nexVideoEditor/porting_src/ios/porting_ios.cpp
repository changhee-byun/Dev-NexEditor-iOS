/******************************************************************************
 * File Name   : porting_ios.cpp
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "string.h"
#import "porting_ios.h"
#import "SALBody_Task.h"
#import "SALBody_SyncObj.h"
#import "SALBody_File.h"
#import "SALBody_Mem.h"
#import "SALBody_Task.h"
#import "SALBody_Time.h"
#import "nexRALBody_Audio_API.h"
#import "NexCAL.h"
#import "NexMediaDef.h"
#import "NEXVIDEOEDITOR_Def.h"
#import "nexCalBody_VideoToolBox.h"
#import "nexCalBody_MPEG2.h"
#import "nexCalBody_AAC.h"
#import "nexCalBody_PCM.h"
#include "nexCALBody_ATX.h"
#include "nexCALBody_VTX.h"

#define IS_PCM_Codec(type) \
( (type == eNEX_CODEC_A_PCM_S16LE) || (type == eNEX_CODEC_A_PCM_S16BE) || (type == eNEX_CODEC_A_PCM_RAW) || (type == eNEX_CODEC_A_PCM_FL32LE) \
|| (type == eNEX_CODEC_A_PCM_FL32BE) || (type == eNEX_CODEC_A_PCM_FL64LE) || (type == eNEX_CODEC_A_PCM_FL64BE) || (type == eNEX_CODEC_A_PCM_IN24LE) \
|| (type == eNEX_CODEC_A_PCM_IN24BE) || (type == eNEX_CODEC_A_PCM_IN32LE) || (type == eNEX_CODEC_A_PCM_IN32BE) || (type == eNEX_CODEC_A_PCM_LPCMLE) \
|| (type == eNEX_CODEC_A_PCM_LPCMBE))

NEXCALHandle	g_hNexCAL = NULL;

typedef struct _CALQUERY_USERDATA
{
    char m_strModelName[255];
    int	m_iPlatformInfo;
}CALQUERY_USERDATA;

typedef struct _CAL_ONLOAD_INFO
{
    char m_strLibVideoOcFile[512];
    char m_strLibCALMcFile[512];
    char m_strLibCALSWAudioFile[512];
    char m_strLibCALSWPCMFile[512];
    char m_strLibCALSWH264File[512];
    char m_strLibCALSWMP4File[512];
    void* m_pVideo_OC_Handle;
    void* m_pCAL_MC_Handle;
    void* m_pCAL_nexaudio_Handle;
    void* m_pCAL_nexH264_Handle;
    void* m_pCAL_nexMP4_Handle;
    
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

AUDIO_RALBODY_FUNCTION_ST g_audioRendererHandle =
{
    /* audioRendererHandle.fnNexRALBody_Audio_getProperty               = */ nexRALBody_Audio_getProperty
    , /* audioRendererHandle.fnNexRALBody_Audio_setProperty             = */ nexRALBody_Audio_setProperty
    , /* audioRendererHandle.fnNexRALBody_Audio_init                    = */ nexRALBody_Audio_init
    , /* audioRendererHandle.fnNexRALBody_Audio_deinit                  = */ nexRALBody_Audio_deinit
    , /* audioRendererHandle.fnNexRALBody_Audio_getEmptyBuffer          = */ nexRALBody_Audio_getEmptyBuffer
    , /* audioRendererHandle.fnNexRALBody_Audio_consumeBuffer           = */ nexRALBody_Audio_consumeBuffer
    , /* audioRendererHandle.fnNexRALBody_Audio_setBufferMute           = */ nexRALBody_Audio_setBufferMute
    , /* audioRendererHandle.fnNexRALBody_Audio_getCurrentCTS           = */ nexRALBody_Audio_getCurrentCTS
    , /* audioRendererHandle.fnNexRALBody_Audio_clearBuffer             = */ nexRALBody_Audio_clearBuffer
    , /* audioRendererHandle.fnNexRALBody_Audio_pause                   = */ nexRALBody_Audio_pause
    , /* audioRendererHandle.fnNexRALBody_Audio_resume                  = */ nexRALBody_Audio_resume
    , /* audioRendererHandle.fnNexRALBody_Audio_flush                   = */ nexRALBody_Audio_flush
    , /* audioRendererHandle.fnNexRALBody_Audio_setTime                 = */ nexRALBody_Audio_setTime
    , /* audioRendererHandle.fnNexRALBody_Audio_setPlaybackRate         = */ nexRALBody_Audio_setPlaybackRate
    , /* audioRendererHandle.fnNexRALBody_Audio_SetSoundPath            = */ nexRALBody_Audio_SetSoundPath
    , /* audioRendererHandle.fnNexRALBody_Audio_create                  = */ nexRALBody_Audio_create
    , /* audioRendererHandle.fnNexRALBody_Audio_delete                  = */ nexRALBody_Audio_delete
    , /* audioRendererHandle.fnNexRALBody_Audio_getAudioSessionId       = */ nexRALBody_Audio_getAudioSessionId
    , /* audioRendererHandle.fnNexRALBody_Audio_prepareAudioTrack       = */ nexRALBody_Audio_prepareAudioTrack
    , /* audioRendererHandle.fnNexRALBody_Audio_SetVolume               = */ nexRALBody_Audio_SetVolume
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenInit               = */ NULL/*nexRALBody_Audio_MavenInit*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetVolume          = */ NULL/*nexRALBody_Audio_MavenSetVolume*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetOutput          = */ NULL/*nexRALBody_Audio_MavenSetOutput*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetParam           = */ nexRALBody_Audio_MavenSetParam
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSetAutoVolumeParam = */ nexRALBody_Audio_MavenSetAutoVolumeParam
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenSEQSetParam        = */ NULL/*nexRALBody_Audio_MavenSEQSetParam*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenVMS2SetParam       = */ NULL/*nexRALBody_Audio_MavenVMS2SetParam*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenVMS2FilterSetMode  = */ NULL/*nexRALBody_Audio_MavenVMS2FilterSetMode*/
    , /* audioRendererHandle.fnNexRALBody_Audio_MavenVMS2FilterSetParam = */ NULL/*nexRALBody_Audio_MavenVMS2FilterSetParam*/
};




void registerSAL(void)
{
    nexSALBODY_SyncObjectsInit();
    
    // Heap
    nexSAL_RegisterFunction( NEXSAL_REG_MEM_ALLOC, (void*)nexSALBody_MemAlloc, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_MEM_CALLOC, (void*)nexSALBody_MemCalloc, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_MEM_FREE, (void*)nexSALBody_MemFree, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_MEM_ALLOC2, (void*)nexSALBody_MemAlloc2, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_MEM_FREE2, (void*)nexSALBody_MemFree2, NULL );
    
    // File
    nexSAL_RegisterFunction( NEXSAL_REG_FILE_OPENA, (void*)nexSALBody_FileOpen, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_FILE_CLOSE, (void*)nexSALBody_FileClose, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_FILE_READ, (void*)nexSALBody_FileRead, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_FILE_WRITE, (void*)nexSALBody_FileWrite, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK, (void*)nexSALBody_FileSeek, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK64, (void*)nexSALBody_FileSeek64, NULL );		// JDKIM 2010/05/13
    nexSAL_RegisterFunction( NEXSAL_REG_FILE_SIZE, (void*)nexSALBody_FileSize, NULL);
    
    
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
    nexSAL_RegisterFunction( NEXSAL_REG_TASK_CURRENT, (void*)nexSALBody_TaskGetCurrentHandle, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_TASK_DELETE, (void*)nexSALBody_TaskDelete, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_TASK_GETPRIORITY, (void*)nexSALBody_TaskGetPriority, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_TASK_SETPRIORITY, (void*)nexSALBody_TaskSetPriority, NULL );
    //	nexSAL_RegisterFunction( NEXSAL_REG_TASK_TERMINATE, , NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_TASK_SLEEP, (void*)nexSALBody_TaskSleep, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_TASK_WAIT, (void*)nexSALBody_TaskWait, NULL );
    
    nexSAL_RegisterFunction( NEXSAL_REG_GETTICKCOUNT, (void*)nexSALBody_GetTickCount, NULL );
    nexSAL_RegisterFunction( NEXSAL_REG_GETMSECFROMEPOCH, (void*)nexSALBody_GetMSecFromEpoch, NULL );
    
}

void unregisterSAL(void)
{
//    nexSALBODY_SyncObjectsDeinit();
}


void loadRAL( char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel )
{
    
}

void unloadRAL()
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s unloadRAL In", __LINE__, __func__);
//    memset(&g_audioRendererHandle, 0x00, sizeof(AUDIO_RALBODY_FUNCTION_ST));
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s NEXRAL unloaded succesfully!\n", __LINE__, __func__);
}

void* getRALgetAudioRenderFuncs()
{
    return (void*)&g_audioRendererHandle;
}

int getCALOnload(	NXBOOL bRelease
                 , NEXCALMediaType eMediaType
                 , NEXCALMode eMode
                 , NEX_CODEC_TYPE uCodecObjectTypeIndication
                 , NXUINT32 a_uStaticProperties
                 , NXVOID* pUserData
                 , NXVOID* pClientID
                 , NXVOID* pFuncs
                 , NXINT32 nFuncSize
                 , NEXCALQueryHandler* ppQueryHandler
                 , NXVOID ** puUserDataForQueryHandler )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload pFuncs(0x%x) nFuncSize(%d)", __LINE__, __func__, pFuncs, nFuncSize);
    CAL_ONLOAD_INFO* hOnloadInfo = (CAL_ONLOAD_INFO*)pUserData;
    if( hOnloadInfo == NULL )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload UserData is null", __LINE__, __func__);
        return 1;
    }
    
    if( eMediaType == NEXCAL_MEDIATYPE_VIDEO )
    {
        if( eMode == NEXCAL_MODE_DECODER )
        {
            if( uCodecObjectTypeIndication == eNEX_CODEC_V_H264 || uCodecObjectTypeIndication == eNEX_CODEC_V_HEVC )
            {
                if( bRelease )
                    return 0;
                NEXCALVideoDecoder CAL_AVC =
                {
                    nexCALBody_Video_VTB_GetProperty,
                    nexCALBody_Video_VTB_SetProperty,
                    (NEXCALVideoInit)nexCALBody_Video_VTB_Init,
                    nexCALBody_Video_VTB_Close,
                    (NEXCALVideoDecode)nexCALBody_Video_VTB_Dec,
                    nexCALBody_Video_VTB_GetOutput,
                    nexCALBody_Video_VTB_Reset,
                    NULL
                };
            
                
                memcpy(pFuncs, &CAL_AVC, sizeof(NEXCALVideoDecoder));
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload %s dec sucessed", __LINE__, __func__, uCodecObjectTypeIndication == eNEX_CODEC_V_H264 ? "eNEX_CODEC_V_H264": uCodecObjectTypeIndication == eNEX_CODEC_V_HEVC ? "eNEX_CODEC_V_HEVC": "");
                return 0;
            }
            
            if( uCodecObjectTypeIndication == (NEX_CODEC_TYPE)eNEX_CODEC_V_MPEG2V )
            {
                if( bRelease )
                    return 0;
                NEXCALVideoDecoder CAL_MPEG2 =
                {
                    NULL,
                    NULL,
                    (NEXCALVideoInit)nexCALBody_Video_MPEG2_Init,
                    nexCALBody_Video_MPEG2_Close,
                    (NEXCALVideoDecode)nexCALBody_Video_MPEG2_Dec,
                    nexCALBody_Video_MPEG2_GetOutput,
                    nexCALBody_Video_MPEG2_Reset,
                    NULL
                };
                
                
                memcpy(pFuncs, &CAL_MPEG2, sizeof(NEXCALVideoDecoder));
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload eNEX_CODEC_V_MPEG2V dec sucessed", __LINE__, __func__);
                return 0;
            }
        }
        else if( eMode == NEXCAL_MODE_ENCODER )
        {
            if( uCodecObjectTypeIndication == (NEX_CODEC_TYPE)eNEX_CODEC_V_H264 )
            {
                if( bRelease )
                    return 0;
      
                NEXCALVideoEncoder CAL_AVC =
                {
                    nexCALBody_VideoEncoder_VTX_GetProperty,
                    nexCALBody_VideoEncoder_VTX_SetProperty,
                    nexCALBody_VideoEncoder_VTX_Init,
                    nexCALBody_VideoEncoder_VTX_Deinit,
                    nexCALBody_VideoEncoder_VTX_Encode,
                    nexCALBody_VideoEncoder_VTX_GetOutput,
                    nexCALBody_VideoEncoder_VTX_Skip
                };
                
                memcpy(pFuncs, &CAL_AVC, sizeof(NEXCALVideoEncoder));
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload eNEX_CODEC_V_H264 enc sucessed", __LINE__, __func__);
                return 0;
            }
            
        }
    }
    else if( eMediaType == NEXCAL_MEDIATYPE_AUDIO )
    {
        if( eMode == NEXCAL_MODE_DECODER )
        {
            if( IS_PCM_Codec(uCodecObjectTypeIndication))
            {
                if( bRelease )
                    return 0;
                NEXCALAudioDecoder CAL_PCM =
                {
                    nexCALBody_Audio_PCM_GetProperty,
                    nexCALBody_Audio_PCM_SetProperty,
                    (NEXCALAudioInit)nexCALBody_Audio_PCM_Init,
                    nexCALBody_Audio_PCM_Deinit,
                    (NEXCALAudioDecode)nexCALBody_Audio_PCM_Dec,
                    nexCALBody_Audio_PCM_Reset,
                    NULL
                };

                memcpy(pFuncs, &CAL_PCM, sizeof(NEXCALAudioDecoder));
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload eNEX_CODEC_A_PCM dec sucessed", __LINE__, __func__);
                return 0;
            }
            
            if( uCodecObjectTypeIndication == (NEX_CODEC_TYPE)eNEX_CODEC_A_AAC )
            {
                if( bRelease )
                    return 0;
                NEXCALAudioDecoder CAL_AAC =
                {
                    nexCALBody_AudioDecoder_ATX_GetProperty,
                    nexCALBody_AudioDecoder_ATX_SetProperty,
                    (NEXCALAudioInit)nexCALBody_AudioDecoder_ATX_Init,
                    nexCALBody_AudioDecoder_ATX_Deinit,
                    (NEXCALAudioDecode)nexCALBody_AudioDecoder_ATX_Dec,
                    nexCALBody_AudioDecoder_ATX_Reset,
                    NULL
                };
/* // sw decoder part
                {
                    nexCALBody_Audio_AAC_GetProperty,
                    nexCALBody_Audio_AAC_SetProperty,
                    (NEXCALAudioInit)nexCALBody_Audio_AAC_Init,
                    nexCALBody_Audio_AAC_Deinit,
                    (NEXCALAudioDecode)nexCALBody_Audio_AAC_Dec,
                    nexCALBody_Audio_AAC_Reset,
                    NULL
                };
*/
                memcpy(pFuncs, &CAL_AAC, sizeof(NEXCALAudioDecoder));
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload eNEX_CODEC_A_AAC dec sucessed", __LINE__, __func__);
                return 0;
            }
            else if( uCodecObjectTypeIndication == (NEX_CODEC_TYPE)eNEX_CODEC_A_MP3 )
            {
                if( bRelease )
                    return 0;
                NEXCALAudioDecoder CAL_MP3 =
                {
                    nexCALBody_AudioDecoder_ATX_GetProperty,
                    nexCALBody_AudioDecoder_ATX_SetProperty,
                    (NEXCALAudioInit)nexCALBody_AudioDecoder_ATX_Init,
                    nexCALBody_AudioDecoder_ATX_Deinit,
                    (NEXCALAudioDecode)nexCALBody_AudioDecoder_ATX_Dec,
                    nexCALBody_AudioDecoder_ATX_Reset,
                    NULL
                };
                /* // sw decoder part
                 {
                 nexCALBody_Audio_AAC_GetProperty,
                 nexCALBody_Audio_AAC_SetProperty,
                 (NEXCALAudioInit)nexCALBody_Audio_AAC_Init,
                 nexCALBody_Audio_AAC_Deinit,
                 (NEXCALAudioDecode)nexCALBody_Audio_AAC_Dec,
                 nexCALBody_Audio_AAC_Reset,
                 NULL
                 };
                 */
                memcpy(pFuncs, &CAL_MP3, sizeof(NEXCALAudioDecoder));
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload eNEX_CODEC_A_MP3 dec sucessed", __LINE__, __func__);
                return 0;
            }
        }
        if( eMode == NEXCAL_MODE_ENCODER )
        {
            if( uCodecObjectTypeIndication == (NEX_CODEC_TYPE)eNEX_CODEC_A_AAC )
            {
                if( bRelease )
                    return 0;
                NEXCALAudioEncoder CAL_AAC =
                {
                    nexCALBody_AudioEncoder_ATX_GetProperty,
                    nexCALBody_AudioEncoder_ATX_SetProperty,
                    nexCALBody_AudioEncoder_ATX_Init,
                    nexCALBody_AudioEncoder_ATX_Deinit,
                    nexCALBody_AudioEncoder_ATX_Encode,
                    nexCALBody_AudioEncoder_ATX_Reset
                };
                memcpy(pFuncs, &CAL_AAC, sizeof(NEXCALAudioEncoder));
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s getCALOnload eNEX_CODEC_A_AAC enc sucessed", __LINE__, __func__);
                return 0;
            }
        }
    }
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s not using Codec(0x%x 0x%x 0x%x)", __LINE__, __func__, eMediaType, eMode, uCodecObjectTypeIndication);
    return 1;
}

static int videoCodecTypesForDecoder[] = {
    eNEX_CODEC_V_H264, eNEX_CODEC_V_MPEG2V, eNEX_CODEC_V_HEVC
};
static int audioCodecTypesForDecoder[] = {
    eNEX_CODEC_A_AAC, eNEX_CODEC_A_MP3, eNEX_CODEC_A_PCM_S16LE, eNEX_CODEC_A_PCM_S16BE, eNEX_CODEC_A_PCM_RAW, eNEX_CODEC_A_PCM_FL32LE, eNEX_CODEC_A_PCM_FL32BE, eNEX_CODEC_A_PCM_FL64LE, eNEX_CODEC_A_PCM_FL64BE, eNEX_CODEC_A_PCM_IN24LE, eNEX_CODEC_A_PCM_IN24BE, eNEX_CODEC_A_PCM_IN32LE, eNEX_CODEC_A_PCM_IN32BE, eNEX_CODEC_A_PCM_LPCMLE, eNEX_CODEC_A_PCM_LPCMBE
};

static int videoCodecTypesForEncoder[] = {
    eNEX_CODEC_V_H264, eNEX_CODEC_V_MPEG2V
};
static int audioCodecTypesForEncoder[] = {
    eNEX_CODEC_A_AAC
};

int countSupportedDecoderCodecTypes( bool isVideo ) {
    if ( isVideo ) {
        return sizeof(videoCodecTypesForDecoder) / sizeof(int);
    } else {
        return sizeof(audioCodecTypesForDecoder) / sizeof(int);
    }
}

int countSupportedEncoderCodecTypes( bool isVideo ) {
    if ( isVideo ) {
        return sizeof(videoCodecTypesForEncoder) / sizeof(int);
    } else {
        return sizeof(audioCodecTypesForEncoder) / sizeof(int);
    }
}

int * getSupportedDecoderCodecTypes( bool isVideo ) {
    if ( isVideo ) {
        return videoCodecTypesForDecoder;
    } else {
        return audioCodecTypesForDecoder;
    }
}

int * getSupportedEncoderCodecTypes( bool isVideo ) {
    if ( isVideo ) {
        return videoCodecTypesForEncoder;
    } else {
        return audioCodecTypesForEncoder;
    }
}

void registerCAL( char* strLibPath, char* strModelName, int iAPILevel, int iLogLevel )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s registerCAL In(%s, %d %d)", __LINE__, __func__, strModelName, iAPILevel, iLogLevel);
    
    if( strLibPath == NULL )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s parameter error(0x%x %s %d)", __LINE__, __func__, strLibPath, strModelName, iAPILevel);
        return;
    }
    
    if( g_hNexCAL == NULL )
    {
        g_hNexCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
    }
    
    if( g_hNexCAL == NULL )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s nexCAL_GetHandle failed!\n", __LINE__, __func__);
        return;
    }
	
    unsigned int uHWCodecStaticProperties;
    NEXCAL_INIT_STATIC_PROPERTY(uHWCodecStaticProperties);
    NEXCAL_SET_STATIC_PROPERTY(uHWCodecStaticProperties, NEXCAL_STATIC_PROPERTY_HW_CODEC);

    NXUINT32 uSWCodecStaticProperties;
    NEXCAL_INIT_STATIC_PROPERTY(uSWCodecStaticProperties);
    NEXCAL_SET_STATIC_PROPERTY(uSWCodecStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC);

    NXUINT32 uPSWCodecStaticProperties;
    NEXCAL_INIT_STATIC_PROPERTY(uPSWCodecStaticProperties);
    NEXCAL_SET_STATIC_PROPERTY(uPSWCodecStaticProperties, NEXCAL_STATIC_PROPERTY_SW_CODEC);
    NEXCAL_SET_STATIC_PROPERTY(uPSWCodecStaticProperties, NEXCAL_STATIC_PROPERTY_PLATFORM_CODEC);

    bool isVideo = true;
    
    int *videoCodecs = getSupportedDecoderCodecTypes(isVideo);
    int loopid = 0; int loopCount = 0;
    
    for( loopid = 0, loopCount = countSupportedDecoderCodecTypes(isVideo); loopid < loopCount; loopid++ ) {
        nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, (NEX_CODEC_TYPE)(*(videoCodecs+loopid)), getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWCodecStaticProperties);
    }
    
    int *audioCodecs = getSupportedDecoderCodecTypes(!isVideo);

	for( loopid = 0, loopCount = countSupportedDecoderCodecTypes(!isVideo); loopid < loopCount; loopid++) {
        NXUINT32 a_uStaticProperties = uSWCodecStaticProperties;
        NEX_CODEC_TYPE codecType = (NEX_CODEC_TYPE)(*(audioCodecs+loopid));
        if (codecType == eNEX_CODEC_A_AAC || codecType == eNEX_CODEC_A_MP3) {
            a_uStaticProperties = uPSWCodecStaticProperties;
        }
        nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, codecType, getCALOnload, (NXVOID*)&g_hOnloadInfo, a_uStaticProperties);
    }
    
    videoCodecs = getSupportedEncoderCodecTypes(isVideo);
    
    for( loopid = 0, loopCount = countSupportedEncoderCodecTypes(isVideo); loopid < loopCount; loopid++) {
        nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_ENCODER, (NEX_CODEC_TYPE)(*(videoCodecs+loopid)), getCALOnload, (NXVOID*)&g_hOnloadInfo, uHWCodecStaticProperties);
    }
    
    audioCodecs = getSupportedEncoderCodecTypes(!isVideo);
    
    for( loopid = 0, loopCount = countSupportedEncoderCodecTypes(!isVideo); loopid < loopCount; loopid++) {
        nexCAL_RegisterCodec( g_hNexCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_ENCODER, (NEX_CODEC_TYPE)(*(audioCodecs+loopid)), getCALOnload, (NXVOID*)&g_hOnloadInfo, uPSWCodecStaticProperties);
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s NEXCAL initialized succesfully!", __LINE__, __func__);
}

void unregisterCAL()
{
    if( g_hNexCAL )
    {
        nexCAL_ReleaseHandle( g_hNexCAL );
        g_hNexCAL = NULL;
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s NEXCAL uninitialized succesfully!", __LINE__, __func__);
    }
    
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[porting_ios.cpp %d]%s dlclose and uninitialized succesfully!", __LINE__, __func__);

    if( g_hOnloadInfo.m_pCAL_nexaudio_Handle)
    {
        g_hOnloadInfo.m_pCAL_nexaudio_Handle = NULL;
    }
    
    if( g_hOnloadInfo.m_pCAL_nexH264_Handle)
    {
        g_hOnloadInfo.m_pCAL_nexH264_Handle = NULL;
    }
    
    if( g_hOnloadInfo.m_pCAL_nexMP4_Handle)
    {
        g_hOnloadInfo.m_pCAL_nexMP4_Handle = NULL;
    }
    
}
