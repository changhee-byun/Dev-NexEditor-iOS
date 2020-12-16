/******************************************************************************
 * File Name   : nexRALBody_Audio_API.c
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <AudioToolBox/AudioSession.h>
#import "nexRALBody_Audio_API.h"
#import "nexRalBody_Audio.h"
#import "NexAURenderer.h"
#import "NexSAL_Internal.h"
#import "NexEditorLog.h"
#import "NexRAL.h"

//#define DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK

#ifdef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK
static char g_DummyBuffer[1024*4*2];
struct DummpyRenderer {
    int nBufferSize;
    int nSleepMS;
    int nCTS;
} g_DummyRenderer;
#endif

#define LOG_TAG @"nexRalBody_Audio"
/* FIXME: Internal/Temporary definitions for NexRALBody Callback returns
 * Replace values when those are available from nexRAL.h
 * - simon
 */
#define NEXRALBODY_RETURN_UNKNOWN_ERROR 1
#define NEXRALBODY_RETURN_SUCCEED 0

/*-------------------------------------------------------------------------------------------------------
 Audio Renderer implements
 --------------------------------------------------------------------------------------------------------*/
#define NEXRALBODY_AUDIO_GAIN_DEFAULT   (1.0f)

@interface NexRALBodyAudio()
@property (nonatomic, retain) NexAURenderer *auRenderer;
@property (atomic) NSUInteger localCounter;
@property (nonatomic) NSInteger renderBufferSize;
@end

unsigned int nexRALBody_Audio_getProperty( unsigned int uProperty, unsigned int *puValue, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "getProperty+ property(0x%x) userData(0x%x)\n", uProperty, uUserData);
    switch(uProperty)
    {
        case NEXRAL_PROPERTY_AUDIO_RENDERER_MODE:
            *puValue = NEXRAL_PROPERTY_AUDIO_RENDERER_MODE_NORMAL;
            break;
            
        default:
            *puValue = 0;
            break;
    }
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 2, "getProperty- property(0x%x) value(0x%x) userData(0x%x)\n", uProperty, *puValue, uUserData);
    return 0;
}

unsigned int nexRALBody_Audio_setProperty( unsigned int uProperty, unsigned int uValue, void* uUserData)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "setProperty+ property(0x%x) value(0x%x) userData(0x%x)\n", uProperty, uValue, uUserData);
    return 0;
}

unsigned int nexRALBody_Audio_init ( unsigned int uCodecType
                                    , unsigned int uSamplingRate
                                    , unsigned int uNumOfChannels
                                    , unsigned int uBitsPerSample
                                    , unsigned int uNumOfSamplesPerChannel
                                    , void** puUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "init+ codecType(0x%x) samplingRate(%u) numOfChannels(%u) bitsPerSample(%u) numSamplesPerCh(%u)\n", uCodecType, uSamplingRate, uNumOfChannels, uBitsPerSample, uNumOfSamplesPerChannel);
    
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK
    
    NexRALBodyAudio *ralBodyAudio = [[NexRALBodyAudio alloc] init];
    // rooney test
    ralBodyAudio.volume = 1.0f;
    
    if ( ralBodyAudio != nil ) {
        NexAURenderer *auRenderer = [[NexAURenderer alloc]	initWithOTI:uCodecType
                                                          samplingRate:uSamplingRate
                                                          channelCount:uNumOfChannels
                                                         bitsPerSample:uBitsPerSample
                                                     samplesPerChannel:uNumOfSamplesPerChannel];
        
        ralBodyAudio.auRenderer = auRenderer;
        ralBodyAudio.renderBufferSize = uNumOfSamplesPerChannel * (uBitsPerSample/8) * uNumOfSamplesPerChannel;
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[RALBodyAudio %u] Starting with volume set to %2.3f", __LINE__, ralBodyAudio.volume);
        
        
        /* Find me with this value */
        *puUserData = ralBodyAudio;
        [auRenderer release];
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[RALBodyAudio %u] userData:%p", __LINE__, *puUserData);
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBodyAudio %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
        return NEXRALBODY_RETURN_UNKNOWN_ERROR;
    }
    
#else
    g_DummyRenderer.nSleepMS = uNumOfSamplesPerChannel * 1000 / uSamplingRate;
    g_DummyRenderer.nBufferSize = (uNumOfSamplesPerChannel * (uBitsPerSample/8) * uNumOfSamplesPerChannel);
    g_DummyRenderer.nCTS = 0;
    
    *puUserData = (void*)(uNumOfSamplesPerChannel * (uBitsPerSample/8) * uNumOfSamplesPerChannel);
    
#endif
    
    return NEXRALBODY_RETURN_SUCCEED;
}

unsigned int nexRALBody_Audio_deinit( void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "deinit+ ud(0x%x)\n", uUserData);

#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK

    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        ralBodyAudio.auRenderer = nil;
        [ralBodyAudio release];
        ralBodyAudio = nil;
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with unknown uRALUserData. RALBody freed? (ignoring)", __LINE__, __func__);
        return NEXRALBODY_RETURN_UNKNOWN_ERROR;
    }
    
#endif
    
    return NEXRALBODY_RETURN_SUCCEED;
}

unsigned int nexRALBody_Audio_getEmptyBuffer( void** ppEmptyBuffer, int* nMaxBufferSize, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "getEmptyBuffer+ ud(0x%x)\n", uUserData);

    unsigned int ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;

#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK

    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        if([renderer isPaused] == YES) //GetEmpty Buffer, Consume buffer  12.10.26. Ian Lee.
        {
            [renderer resume];
        }
        ret = [renderer getEmptyBuffer: ppEmptyBuffer
                               maxSize: nMaxBufferSize ];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
    
#else
    
    usleep( ( g_DummyRenderer.nSleepMS -1 ) * 1000 );
    
    int nBufferSize = (int)uUserData;
    *ppEmptyBuffer = &g_DummyBuffer[0];
    *nMaxBufferSize = nBufferSize;
    
    ret = NEXRALBODY_RETURN_SUCCEED;
    
#endif
    
    return ret;
}

unsigned int nexRALBody_Audio_consumeBuffer( void* pBuffer, int nBufferLen, int uCTS, int isDecodeSuccess, int bEndFrame, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "consumeBuffer+ buffer(%p) bufferLen(%d) decSuccess(%s) endFrame(%s) ud(0x%x)\n", pBuffer, nBufferLen, 0 != isDecodeSuccess ? "true" : "false", 0 != bEndFrame ? "true" : "false", uUserData);

    unsigned int ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;

#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK

    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer consumeBuffer: pBuffer
                            bufferLen: nBufferLen
                               forCTS: uCTS
                        decodeSuccess: isDecodeSuccess];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
#else
    g_DummyRenderer.nCTS = uCTS;
    ret = NEXRALBODY_RETURN_SUCCEED;
    
#endif
    
    return ret;
}

unsigned int nexRALBody_Audio_setBufferMute( void* pBuffer, bool bSetPCMSize, int* piWrittenPCMSize, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "setBufferMute+ buffer(%p) ud(0x%x)\n", pBuffer, uUserData);
    
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( pBuffer != NULL) {
        if(!bSetPCMSize) {
            *piWrittenPCMSize = (NXINT32) ralBodyAudio.renderBufferSize;
        }
        memset(pBuffer, 0, *piWrittenPCMSize); // Matthew:20120607: Fix for Mantis 2313
        
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] ERROR: %s(pBuffer == NULL)", __LINE__, __func__);
    }
#endif
    return NEXRALBODY_RETURN_SUCCEED;
}

unsigned int nexRALBody_Audio_getCurrentCTS( unsigned int* puCTS, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 3, "getCurrentCTS+ ud(0x%x)\n", uUserData);
    
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;

#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK

    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer getCurrentCTS: puCTS];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
#else
    *puCTS = g_DummyRenderer.nCTS;
    ret = NEXRALBODY_RETURN_SUCCEED;
#endif
    
    return ret;
}

unsigned int nexRALBody_Audio_clearBuffer( void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "clearBuffer+ ud(0x%x)\n", uUserData);
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer clearBuffer];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
#endif
    return ret;
}

unsigned int nexRALBody_Audio_pause( void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "pause+ ud(0x%x)\n", uUserData);
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer pause];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
#endif
    return ret;
}

unsigned int nexRALBody_Audio_resume( void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "resume+ ud(0x%x)\n", uUserData);
    
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer resume];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
#endif
    return ret;
    
}

unsigned int nexRALBody_Audio_flush( unsigned int uCTS, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "flush+ cts(%u) ud(0x%x)\n", uCTS, uUserData);
    
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK

    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer flush];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
#endif
    return ret;
}

unsigned int nexRALBody_Audio_setTime( unsigned int uCTS, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "setTime+ cts(%u) ud(0x%x)\n", uCTS, uUserData);
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK

    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer setCTS: uCTS];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called for unknown player(userData: %p) (ignoring)", __LINE__, __func__, uUserData);
    }
#else
    ret = NEXRALBODY_RETURN_SUCCEED;
#endif
    return ret;
}

unsigned int nexRALBody_Audio_setPlaybackRate( int iRate, void* uUserData)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "setPlaybackRate+ rate(%d) ud(0x%x)\n", iRate, uUserData);
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
#ifndef DUMMY_RENDERER_FOR_CHECKING_RESOURCE_LEAK
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) uUserData;
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer setPlaybackRate: iRate];
    } else {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
    }
#endif
    return ret;
    
}

unsigned int nexRALBody_Audio_SetSoundPath( int iPath, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetSoundPath+ path(%d) ud(0x%x)\n", iPath, uUserData);

    return 0;
}


unsigned int nexRALBody_Audio_create(int nLogLevel, unsigned int bUseAudioEffect, void * /*jobject*/ audioManager,  FNRALCALLBACK fnCallback, void *pUserData)
{

    return 0;
}


unsigned int nexRALBody_Audio_delete(void* uUserData)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "delete+\n");

    return 0;
}


unsigned int nexRALBody_Audio_getAudioSessionId(unsigned int* puSessionId, void* uUserData)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "GetAudioSessionId+ ud(0x%x)\n", uUserData);
    
    return 0;
}


unsigned int nexRALBody_Audio_prepareAudioTrack(void* pAudioTrack, void* uUserData)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_WARNING, 0, "prepareAudioTrack+- not doing anything...\n");
    // TODO:
    return 0;
}


unsigned int nexRALBody_Audio_SetVolume(float fGain, void* uUserData)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetVolume+ gain(%f) ud(0x%x)\n", fGain, uUserData);
    return 0;
}


unsigned int nexRALBody_Audio_MavenSetParam( unsigned int uiMavenMode, unsigned int uiMavenStrength, unsigned int uiBassStrength, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetParam+ mode(%u) strength(%u) bassStrength(%u) ud(0x%x)\n", uiMavenMode, uiMavenStrength, uiBassStrength, uUserData);

    return 0;
}


unsigned int nexRALBody_Audio_MavenSetAutoVolumeParam( unsigned int uiEnable, unsigned int uiStrength, unsigned int uiReleaseTime, void* uUserData )
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_P_AUDIO, 1, "SetAutoVolumeParam+\n");
    
    return 0;
}

