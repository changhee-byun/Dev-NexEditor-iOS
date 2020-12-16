//
//  nexRalBody_Audio.m
//  nexRalBody
//
//  Created by Matthew Feinberg on 4/8/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "nexRalBody_Audio.h"
#include "NexAURenderer.h"
#import <AudioToolBox/AudioSession.h>
#import "NexEditorLog.h"

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

@implementation NexRALBodyAudio
- (id) initWithHPlayer:(void *)hPlayer
{
    self = [super initWithHPlayer:hPlayer];
    if ( self ) {
        self.volume = NEXRALBODY_AUDIO_GAIN_DEFAULT;
	}
    return self;
}
- (void) setVolume:(float)volume
{
    if (_volume != volume) {
        NexLogD(LOG_TAG, @"[%d] setVolume:(%2.3f) -> (%2.3f)", __LINE__, _volume, volume);

        _volume = volume;
        if ( self.auRenderer != nil ) {
            NexLogE(LOG_TAG, @"[%d] NexPlayerSWP_Audio_SetVolume (%4.2f)", __LINE__, _volume);
            [self.auRenderer setGain:_volume];
        }
    } /* else, _volume will be applied when auRenderer property is set */
}

- (void) dealloc
{
    self.auRenderer = nil;
    if ([[self class] numberOfInstances] == 0) {
        /* FIXME: What does this do? What's the proper way to do it without warning and calling deprecated API? [[AVAudioSession sharedInstance] setActive:NO error:nil] ?
         */
        AudioSessionSetActive (false);
    }
    
    [super dealloc];
}

- (void) setAuRenderer:(NexAURenderer *)auRenderer
{
    if ( _auRenderer != auRenderer ) {
        if ( _auRenderer != nil ) {
            [_auRenderer release];
        }
        _auRenderer = [auRenderer retain];
        [_auRenderer setGain:_volume];
    }
}

+ (NSMutableDictionary *) map
{
    /* Override to have the own userData:RALBody map of this class */
    /* NSUInteger(userData) : NexRALBodyAudio */
    static NSMutableDictionary *map = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        map = [[NSMutableDictionary alloc] init];
    });
    return map;
}

@end

#pragma mark - 
#pragma mark Audio renderer implementation

#if 0

NXINT32 nexRALBody_Audio_GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *uUserData )
{
	*puValue = 0;
	return 0;
}

NXINT32 nexRALBody_Audio_SetProperty( NXUINT32 uProperty, NXINT64 uValue, NXVOID *uUserData )
{
	return 0;
}

NXINT32 nexRALBody_Audio_Init(	NEX_CODEC_TYPE uCodecObjectTypeIndication
								   , NXUINT32 uSamplingRate
								   , NXUINT32 uNumOfChannels
								   , NXUINT32 uBitPerSample
								   , NXUINT32 uNumOfSamplesPerChannel
								   , NXVOID **puRALUserData )
{
	NexLogD(LOG_TAG, @"(INIT)nexRALBody_Audio_Init; codec=%u (normal rendering)", uCodecObjectTypeIndication);
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyForPlayer:(NXVOID *) *puRALUserData];
	
    if ( ralBodyAudio != nil ) {
        NexAURenderer *auRenderer = [[NexAURenderer alloc]	initWithOTI:uCodecObjectTypeIndication
                                                         samplingRate:uSamplingRate
                                                         channelCount:uNumOfChannels
                                                        bitsPerSample:uBitPerSample 
                                                    samplesPerChannel:uNumOfSamplesPerChannel];
    
        ralBodyAudio.auRenderer = auRenderer;
		ralBodyAudio.renderBufferSize = uNumOfSamplesPerChannel * (uBitPerSample/8) * uNumOfChannels;
        NexLogD(LOG_TAG, @"[RALBodyAudio %u] Starting with volume set to %2.3f", __LINE__, ralBodyAudio.volume);
    
    
        /* Find me with this value */
        *puRALUserData = ralBodyAudio.userData;
        [auRenderer release];
		NexLogD(LOG_TAG, @"[RALBodyAudio %u] userData:%p", __LINE__, *puRALUserData);
    } else {
        NexLogE(LOG_TAG, @"[RALBodyAudio %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
        return NEXRALBODY_RETURN_UNKNOWN_ERROR;
    }
	return NEXRALBODY_RETURN_SUCCEED;
}

NXINT32 nexRALBody_Audio_Deinit( NXVOID *uRALUserData )
{
	NexLogD(LOG_TAG, @"[RALBodyAudio_Deinit %u] userData:%p", __LINE__, uRALUserData);
	
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        [ralBodyAudio.auRenderer release];
        ralBodyAudio.auRenderer = nil;
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with unknown uRALUserData. RALBody freed? (ignoring)", __LINE__, __func__);
        return NEXRALBODY_RETURN_UNKNOWN_ERROR;
	}	
	return NEXRALBODY_RETURN_SUCCEED;
}

NXINT32 nexRALBody_Audio_GetEmptyBuffer( NXVOID** ppEmptyBuffer, NXINT32* nMaxBufferSize, NXVOID * uRALUserData )
{
    unsigned int ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        if([renderer isPaused] == YES) //GetEmpty Buffer, Consume buffer  12.10.26. Ian Lee.
        {
            [renderer resume];
        }
        ret = [renderer getEmptyBuffer: ppEmptyBuffer
                                      maxSize: nMaxBufferSize ];
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}	

	return ret;
}

NXINT32 nexRALBody_Audio_ConsumeBuffer( NXVOID* pBuffer, NXINT32 nBufferLen, NXUINT32 uCTS, NXINT32 isDecodeSuccess, NXBOOL bEndFrame, NXVOID *uRALUserData )
{
    unsigned int ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer consumeBsuffer: pBuffer
							bufferLen: nBufferLen
							   forCTS: uCTS
						 decodeSuccess: isDecodeSuccess];
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}
	return ret;
}

NXINT32 nexRALBody_Audio_SetBufferMute( NXVOID *pBuffer, NXBOOL bSetPCMSize, NXINT32* piWrittenPCMSize, NXVOID *uRALUserData )
{
	NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
	if ( pBuffer != NULL) {
		if(!bSetPCMSize) {
			*piWrittenPCMSize = (NXINT32) ralBodyAudio.renderBufferSize;
		}
		memset(pBuffer, 0, *piWrittenPCMSize); // Matthew:20120607: Fix for Mantis 2313
		
	} else {
		NexLogE(LOG_TAG, @"[RALBody %u] ERROR: %s(pBuffer == NULL)", __LINE__, __func__);
	}
	return NEXRALBODY_RETURN_SUCCEED;
}

NXINT32 nexRALBody_Audio_GetCurrentCTS( NXUINT32 *puCTS, NXVOID *uRALUserData )
{
	NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
	NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
	if ( ralBodyAudio != nil ) {
		NexAURenderer *renderer = ralBodyAudio.auRenderer;
		ret = [renderer getCurrentCTS: puCTS];
	} else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}
	return ret;
}

NXINT32 nexRALBody_Audio_ClearBuffer( NXVOID *uRALUserData )
{
	NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
	NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
	if ( ralBodyAudio != nil ) {
		NexAURenderer *renderer = ralBodyAudio.auRenderer;
		ret = [renderer clearBuffer];
	} else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}
	return ret;
}

NXINT32 nexRALBody_Audio_Pause( NXVOID *uRALUserData )
{
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer pause];
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}	
	return ret;
}
	
NXINT32 nexRALBody_Audio_Resume( NXVOID *uRALUserData )
{
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer resume];
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}	
	return ret;
}

NXINT32 nexRALBody_Audio_Flush( NXUINT32 uCTS, NXVOID *uRALUserData )
{
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer flush];
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}
	return ret;
}

NXINT32 nexRALBody_Audio_SetTime( NXUINT32 uCTS, NXVOID *uRALUserData )
{
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer setCTS: uCTS];
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called for unknown player(userData: %p) (ignoring)", __LINE__, __func__, uRALUserData);
	}	
	return ret;
}

NXINT32 nexRALBody_Audio_PlaybackRate( NXINT32 iRate, NXVOID *uRALUserData )
{
    NXINT32 ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyWithUserData:uRALUserData];
    if ( ralBodyAudio != nil ) {
        NexAURenderer *renderer = ralBodyAudio.auRenderer;
        ret = [renderer setPlaybackRate: iRate];
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null uRALUserData (ignoring)", __LINE__, __func__);
	}	
    return ret;
}
	
#pragma mark - API
/* Legacy/Single-instance compatibility API
 * compaforwards gain parameter to AURenderer object
 */
int NexPlayerSWP_Audio_SetVolume( float gain ) {
    unsigned int ret = NEXRALBODY_RETURN_UNKNOWN_ERROR;
    NexRALBodyAudio *ralBodyAudio = ( NexRALBodyAudio *) [NexRALBodyAudio ralBodyForDefaultPlayer];
    if ( ralBodyAudio != nil ) {
        NexLogD(LOG_TAG, @"[nexRalBody.m %d] NexPlayerSWP_Audio_SetVolume (%4.2f)", __LINE__, gain);
        ralBodyAudio.volume = gain;
        ret = NEXRALBODY_RETURN_SUCCEED;
    } else {
		NexLogE(LOG_TAG, @"[RALBody %u] WARNING: %s called with null g_MostRecentAURenderer (ignoring)", __LINE__, __func__);
	}
	return ret;
}

#endif // #if 0
