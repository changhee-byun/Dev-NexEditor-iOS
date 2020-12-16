//
//  NexAURenderer.m
//  nexCalBody
//
//  Created by Matthew Feinberg on 9/7/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "NexAURenderer.h"
#import	<mach/mach_time.h>
#import	<UIKit/UIKit.h>
//#import "NexOTIs.h"
#import "NexMediaDef.h"
#import "NexEditorLog.h"

#define LOG_TAG @"NexAURenderer"

static OSStatus auRender (
						  void                        *inRefCon,
						  AudioUnitRenderActionFlags  *ioActionFlags,
						  const AudioTimeStamp        *inTimeStamp,
						  UInt32                      inBusNumber,
						  UInt32                      inNumberFrames,
						  AudioBufferList             *ioData )
{
	NexAURenderer *renderer = (NexAURenderer*)inRefCon;
	NSUInteger bufsize;
	NSUInteger cts = 0;
    BOOL readSuccess = NO;
    BOOL needSetTimeout = NO;
    
    if( !renderer->callbackHasExecuted ) {
        needSetTimeout = YES;
    }
	
	for( int i=0; i<ioData->mNumberBuffers; i++ ) {
		bufsize = ioData->mBuffers[i].mDataByteSize;
        if( needSetTimeout && bufsize > 0 ) {
            //[renderer->ringBuffer setTimeoutForCapacity:bufsize];  // Matthew:20120607: Experimental fix for Mantis 2313 that wasn't needed; leaving the code here in case we need to apply this for a different issue.
            needSetTimeout = NO;
        }

        if(!renderer->isStarted)
        {
            memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
        }
        else
        if (!readSuccess) {
			[renderer->ringBuffer read: ioData->mBuffers[i].mData
								length: &bufsize
								   cts: &cts];
		} else {
			[renderer->ringBuffer read: ioData->mBuffers[i].mData 
								length: &bufsize];
		}
        if( bufsize ) {
            readSuccess = YES;
            ioData->mBuffers[i].mDataByteSize = (UInt32) bufsize;
        } else {
            // timeout reading ring buffer; setting silence for ioData->mBuffers[i].mDataByteSize bytes
            memset(ioData->mBuffers[i].mData, 0, ioData->mBuffers[i].mDataByteSize);
        }
	}
    
    if( readSuccess ) {
        @synchronized (renderer) {
            renderer->lastCallbackCTS = (unsigned int) cts;
            renderer->lastCallbackSysTime = mach_absolute_time();
            renderer->callbackHasExecuted = YES;
    #ifdef RAL_LOG_CTS_CALCS
            NexLogD(LOG_TAG, @"[%u] lastCallbackCTS set to %u\n", __LINE__, cts);
    #endif
        }
    } else {
        *ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;
    }
	
	return 0;
}

@interface NexAURenderer ()  // Private methods
#if defined(RAL_LOG_GET_CTS) || defined(RAL_LOG_CTS_DISCONTINUITY)
- (unsigned int) getCurrentCTS_internal:(unsigned int *)puCTS;
#endif
- (void)appWillResignActive;
- (void)appDidBecomeActive;
- (void)updatePauseState;
@end

@implementation NexAURenderer

- (NexAURenderer *) initWithOTI:(unsigned int)uCodecObjectTypeIndication 
				   samplingRate:(unsigned int)uSamplingRate 
				   channelCount:(unsigned int)uNumChannels 
				  bitsPerSample:(unsigned int)uBitsPerSample 
			  samplesPerChannel:(unsigned int)uSamplesPerChannel 
{
    self = [super init];
	if( self ) {

        bitsPerSample = uBitsPerSample;
        gain = 1.0;
        gain16 = 32767;
        gain8 = 127;
        isStarted = 0;
        TotalSize = 0;
        // rooney
		workBufferSize = uSamplesPerChannel*uNumChannels*uBitsPerSample/8;
//        workBufferSize = 384 * 1024;
        
		switch (uCodecObjectTypeIndication) {
			//case NEXOTI_AAC :
			//case NEXOTI_MPEG2AAC :
            case eNEX_CODEC_A_AAC:
            case eNEX_CODEC_A_AACPLUS:
            case eNEX_CODEC_A_ELDAAC:
//				if( workBufferSize < 16384 )
//					workBufferSize = 16384;
                
/*if this code makes problem,
 use this code in Audio decoder task, line 1424
 #ifdef __APPLE__
 {
 int tempWrittenPCMSize = hPlayer->m_nexPlayerSource.m_uSamplingRate * hPlayer->m_nexPlayerSource.m_uNumOfChannels * 2 * uFrameInterval/1000;
 if(tempWrittenPCMSize < iWrittenPCMSize)
 {
 iWrittenPCMSize = tempWrittenPCMSize;
 }
 }
 #endif
 */
				break;
            //case NEXOTI_WMA:
            case eNEX_CODEC_A_WMA:
            case eNEX_CODEC_A_WMA1:
            case eNEX_CODEC_A_WMA2:
            case eNEX_CODEC_A_WMA3:
            case eNEX_CODEC_A_WMA_LOSSLESS:
            case eNEX_CODEC_A_WMA_SPEECH:
            case eNEX_CODEC_A_WMASPDIF:
                workBufferSize = 8192;
                break;
			default:
				break;
		}

		NexLogD(LOG_TAG, @"[%i (0x%x)] init; OTI=%u uBitsPerSample=%u, uSamplingRate=%u, uNumChannels=%u, uSamplesPerChannel=%u; workBufferSize=%u",
			  __LINE__, (unsigned int)self, uCodecObjectTypeIndication, uBitsPerSample, uSamplingRate, uNumChannels, uSamplesPerChannel, workBufferSize );
		
        {
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(appWillResignActive)
                                                         name:UIApplicationWillResignActiveNotification
                                                       object:nil];
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(appDidBecomeActive)
                                                         name:UIApplicationDidBecomeActiveNotification
                                                       object:nil];
            NexLogD(LOG_TAG, @"[%i (0x%x)] Registered for active notifications", __LINE__, (unsigned int)self);
        }	
        
		// TODO: Check size of ring buffer.  Is this a good size?
		ringBuffer = [[NexCTSRingBuffer alloc] initWithCapacity: workBufferSize*4 // !!!:Matthew:111010: Fix for Mantis0462
														bitrate: uNumChannels*uBitsPerSample*uSamplingRate];
		
		workBuffer = [[NSMutableData alloc] initWithLength:workBufferSize*4];
		
		AudioComponentDescription au_description = {
			.componentType			= kAudioUnitType_Output,
			.componentSubType		= kAudioUnitSubType_RemoteIO,
			.componentManufacturer	= kAudioUnitManufacturer_Apple };
		
		AudioComponent foundComponent = AudioComponentFindNext ( NULL, &au_description );
		
		AudioComponentInstanceNew ( foundComponent, &io_unit_instance );
		
		UInt32 doSetProperty       = 1;
		AudioUnitElement outputBus = 0;
		
		AudioUnitSetProperty (
							  io_unit_instance,
							  kAudioOutputUnitProperty_EnableIO,
							  kAudioUnitScope_Output,
							  outputBus,
							  &doSetProperty,
							  sizeof (doSetProperty)
							  );
		
		AudioStreamBasicDescription audioStreamFormat = {
			.mSampleRate       = uSamplingRate,
			.mFormatID         = kAudioFormatLinearPCM,
			.mFormatFlags      = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked,
			.mBytesPerPacket   = (uBitsPerSample*uNumChannels)/8,
			.mFramesPerPacket  = 1,
			.mBytesPerFrame    = uNumChannels*uBitsPerSample/8,
			.mChannelsPerFrame = uNumChannels,
			.mBitsPerChannel   = uBitsPerSample };
		
		OSStatus result = AudioUnitSetProperty (
							  io_unit_instance,
							  kAudioUnitProperty_StreamFormat,
							  kAudioUnitScope_Input,
							  outputBus,
							  &audioStreamFormat,
							  sizeof (audioStreamFormat)
							  );
		
		NexLogD(LOG_TAG, @"AudioUnitSetProperty(ASBD): %i", (int)result);
		
		AURenderCallbackStruct renderCallbackStruct = {
			.inputProc			= auRender,
			.inputProcRefCon	= self
		};
		
		result = AudioUnitSetProperty (
							  io_unit_instance,
							  kAudioUnitProperty_SetRenderCallback,               // 5
							  kAudioUnitScope_Input,                              // 6
							  outputBus,                                          // 7
							  &renderCallbackStruct,                              // 8
							  sizeof (renderCallbackStruct)
							  );

        NexLogD(LOG_TAG, @"AudioUnitSetProperty(renderCallback): %i", (int)result);
        result = AudioUnitInitialize (io_unit_instance);
        NexLogD(LOG_TAG, @"AudioUnitInitialize: %i", (int)result);
        result = AudioOutputUnitStart(io_unit_instance);
        NexLogD(LOG_TAG, @"AudioOutputUnitStart: %i", (int)result);
	}
	return self;
}

- (void)appDidBecomeActive {
	NexLogD(LOG_TAG, @"[%i (0x%x)] appDidBecomeActive", __LINE__, (unsigned int)self );
    appInactvie = NO;
    [self updatePauseState];
}

- (void)appWillResignActive {
	NexLogD(LOG_TAG, @"%s AppWillResignActive", __PRETTY_FUNCTION__);
	
    appInactvie = YES;
    [self updatePauseState];
}

- (unsigned int) setPlaybackRate:(int)rate {
	NexLogD(LOG_TAG, @"[%i (0x%x)] setPlaybackRate:%i", __LINE__, (unsigned int)self, rate );
	return 0;
}

- (unsigned int) setCTS:(unsigned int)uCTS {
	// Our assumption is that we are in a paused state when setCTS is called.
	// We also assume that the ring buffer has been flushed.  This works fine
	// with the way the player currently calls the renderer.  
	//
	// If we're not in a paused state it doesn't make sense to call setCTS 
	// because new audio data will be continuously being processed, and the CTS 
	// associated with the audio data will override the CTS here.
	//
	// Basically, this is to handle the seeks.  The chain of events for a seek
	// is:
	//
	//  1. Pause the audio renderer
	//	2. (perform the seek operation)
	//  3. Flush the audio renderer buffer [audioRenderer flush]
	//  4. Set the current CTS [audioRenderer setCTS]
	//  5. Resume the audio renderer
	//  6. Send the seek complete event
	//  7. Begin sending audio data
	//
	// Steps 6 and 7 are asynchronous with respect to each other, so it's
	// possible that CTS may be requested before audio data is sent.  That
	// is the case where the value from setCTS is important to prevent
	// UI skipping.
	NexLogD(LOG_TAG, @"[%i (0x%x)] setCTS:%u", __LINE__, (unsigned int)self, uCTS );
	workBufCTS = uCTS; // This may not actually be necessary
	pauseCTS = uCTS;			// This is good enough because it gets copied into lastCallbackCTS when we do a resume
	return 0;
}

- (unsigned int) flush {
	NexLogD(LOG_TAG, @"[%i (0x%x)] flush", __LINE__, (unsigned int)self );
	[ringBuffer flush];
	return 0;
}

- (unsigned int) pause {
    pauseReq = YES;
    [self updatePauseState];
	return 0;
}

- (unsigned int) resume {
    pauseReq = NO;
    [self updatePauseState];
	return 0;
}

- (BOOL) isPaused {
    return pauseReq;
}

- (void)updatePauseState {
    NexLogD(LOG_TAG, @"[%i (0x%x)] updatePauseState; pauseReq=%d appInactvie=%d isPaused=%d (%d)", __LINE__,
          (unsigned int)self, pauseReq, appInactvie, isPaused, pauseCTS );
    if( pauseReq || appInactvie ) {
        if( !isPaused ) {
            [self getCurrentCTS:&pauseCTS];
            NexLogD(LOG_TAG, @"[%i (0x%x)] paused at %d", __LINE__, (unsigned int)self, pauseCTS );
            AudioOutputUnitStop(io_unit_instance);
            isPaused = YES;
            if( appInactvie )
                [ringBuffer flush];     // Prevent discontinuity due to possibly discarded audio data
        }
    } else {
        if( isPaused ) {
            NexLogD(LOG_TAG, @"[%i (0x%x)] resume from %d", __LINE__, (unsigned int)self, pauseCTS );
            @synchronized (self) {
                lastCallbackCTS = pauseCTS;
                lastCallbackSysTime = mach_absolute_time();
            }
            AudioOutputUnitStart(io_unit_instance);
            isPaused = NO;
        }
        
    }
}

- (unsigned int) clearBuffer {
	NexLogD(LOG_TAG, @"[%i (0x%x)] clearBuffer", __LINE__, (unsigned int)self );
	memset([workBuffer mutableBytes], 0, [workBuffer length]);
    //[ringBuffer flush];
	// TODO:  Also flush pending data in ring buffer?
	return 0;
}

- (unsigned int) getCurrentCTS:(unsigned int *)puCTS {
    
#if defined(RAL_LOG_GET_CTS) || defined(RAL_LOG_CTS_DISCONTINUITY)
    unsigned int result = [self getCurrentCTS_internal:puCTS];
#ifdef RAL_LOG_GET_CTS
    NexLogD(LOG_TAG, @"getCurrentCTS: *puCTS=%u\n", *puCTS);
#endif //#ifdef RAL_LOG_GET_CTS
#ifdef RAL_LOG_CTS_DISCONTINUITY
    if( !didCallGetCTS ) {
        didCallGetCTS = YES;
        NexLogD(LOG_TAG, @"getCurrentCTS: [TRACKING] First call for instance; *puCTS=%u\n", *puCTS);
    } else if ( *puCTS < prevReturnedCTS ) {
        NexLogD(LOG_TAG, @"getCurrentCTS: [TRACKING] CTS going backwards; was %u; new *puCTS=%u\n", prevReturnedCTS, *puCTS);
    } else if ( *puCTS - prevReturnedCTS > 500 ) {
        NexLogD(LOG_TAG, @"getCurrentCTS: [TRACKING] CTS jumping forwards more than 500ms; was %u; new *puCTS=%u\n", prevReturnedCTS, *puCTS);
    }
    prevReturnedCTS = *puCTS;
#endif //#ifdef RAL_LOG_CTS_DISCONTINUITY
    return result;
}
- (unsigned int) getCurrentCTS_internal:(unsigned int *)puCTS {
#endif //#if defined(RAL_LOG_GET_CTS) || defined(RAL_LOG_CTS_DISCONTINUITY)
	
#if 1
	if( isPaused )
    {
#ifdef RAL_LOG_CTS_CALCS
        NexLogD(LOG_TAG, @"[%u] isPaused set; using pauseCTS (%u)\n", __LINE__, pauseCTS);
#endif //#ifdef RAL_LOG_CTS_CALCS
        *puCTS = pauseCTS;
        return 0;
    }

	static double ticks_to_ns_factor = 0;
	if( ticks_to_ns_factor == 0 ) {
		mach_timebase_info_data_t tbi = {0};
		mach_timebase_info(&tbi);
		ticks_to_ns_factor = (double)tbi.numer / tbi.denom;
		NexLogD(LOG_TAG, @"getCurrentCTS: mach_timebase_info gives:  %i/%i; ticks_to_ns_factor=%f",
			  tbi.numer, tbi.denom, ticks_to_ns_factor);
	}
	
	unsigned int cts = 0;
	uint64_t ticks = 0;
	BOOL cbstart = NO;
	
	@synchronized (self) {
		cts = lastCallbackCTS;
		ticks = lastCallbackSysTime;
		cbstart = callbackHasExecuted;
	}
    
    if( !cbstart )  {
#ifdef RAL_LOG_CTS_CALCS
        NexLogD(LOG_TAG, @"[%u] callback not yet run; using pauseCTS (%u)\n", __LINE__, pauseCTS);
#endif //#ifdef RAL_LOG_CTS_CALCS
    	*puCTS= pauseCTS;
        return 0;
    }
    
	unsigned int elapsed_ms = 0;
	if( cbstart ) {
		elapsed_ms = (unsigned int)(((double)(mach_absolute_time() - ticks)*ticks_to_ns_factor)/1000000.0);
#ifdef RAL_LIMIT_CTS_INTERP_MS
        if( elapsed_ms > RAL_LIMIT_CTS_INTERP_MS )
            elapsed_ms = RAL_LIMIT_CTS_INTERP_MS;
#endif //#ifdef RAL_LIMIT_CTS_INTERP_MS
	}
	
#ifdef RAL_LOG_CTS_CALCS
    NexLogD(LOG_TAG, @"[%u] cts=%u, elapsed=%u; calculated cts=%u\n", __LINE__, cts, elapsed_ms, cts+elapsed_ms);
#endif //#ifdef RAL_LOG_CTS_CALCS
	
	*puCTS = cts+elapsed_ms;
#else //#if 1
	@synchronized (self) {
#ifdef RAL_LOG_CTS_CALCS
        NexLogD(LOG_TAG, @"[%u] just using lastCallbackCTS=%u\n", __LINE__, lastCallbackCTS);
#endif //#ifdef RAL_LOG_CTS_CALCS
        *puCTS = lastCallbackCTS;
	}
#endif //#if 1
	return 0;
}

- (unsigned int) getEmptyBuffer:(void **)ppEmptyBuffer 
						maxSize:(int *)nMaxBufferSize
{
	*ppEmptyBuffer = [workBuffer mutableBytes];
	*nMaxBufferSize = workBufferSize;//[workBuffer length];
    if( bufferGottenNotConsumed ) {
        NexLogD(LOG_TAG, @"[%u] Duplicate call to getEmptyBuffer(0x%x, %d) without consuming buffer.\n", __LINE__, (unsigned int)(*ppEmptyBuffer), *nMaxBufferSize);
    }
    bufferGottenNotConsumed = YES;
#ifdef RAL_LOG_CTS_CALCS
    NexLogD(LOG_TAG, @"[%u] getEmptyBuffer(0x%x, %d)\n", __LINE__, (unsigned int)(*ppEmptyBuffer), *nMaxBufferSize);
#endif
   // NexLogD(LOG_TAG, @"[NexAURenderer.m %d] getEmptyBuffer(0x%x, %d, %u)", __LINE__, *ppEmptyBuffer, *nMaxBufferSize, uCTS);
	return 0;
}

- (unsigned int) consumeBuffer:(void *)pBuffer 
					 bufferLen:(unsigned int)nBufferLen
						 forCTS:(unsigned int)uCTS
				 decodeSuccess:(unsigned int)isDecodeSuccess
{
	workBufCTS = uCTS;
    NexLogD(LOG_TAG, @"consumeBuffer; workBufCTS=%u", workBufCTS);
    NexLogD(LOG_TAG, @"[NexAURenderer.m %d] consumeBuffer(0x%x, %d)", __LINE__, pBuffer, nBufferLen);
    if( !bufferGottenNotConsumed ) {
        NexLogE(LOG_TAG, @"[%u] Duplicate call to consumeBuffer; ignoring. workBufCTS=%u\n", __LINE__, workBufCTS);
        return 0;
    }
    bufferGottenNotConsumed = NO;

    if( pauseReq ) {
        NexLogE(LOG_TAG, @"[%u] (WARNING) Attempting to consume buffer while paused. workBufCTS=%u\n", __LINE__, workBufCTS);
//        return 0;
    }
/*    if( isPaused ) {
        return 0;
    }*/

#ifdef RAL_LOG_CTS_CALCS
    NexLogD(LOG_TAG, @"[%u] consumeBuffer(0x%x, %d) workBufCTS=%u\n", __LINE__, (unsigned int)pBuffer, nBufferLen, workBufCTS);
#endif
    
    // rooney
//    if( !isDecodeSuccess ) {
//        NexLogD(LOG_TAG, @"[%u] Decode not successful; outputting silence.\n", __LINE__ );
//        memset(pBuffer,0,nBufferLen);
//    }
    if( 1/*gain != 1.0*/ ) {
        if( bitsPerSample == 8 ) {
            signed char *b = (signed char*)pBuffer;
            for( int i=0; i<nBufferLen; i++ ) {
                b[i] = (gain8 * (int)b[i])/32767;
            }
        } else if( bitsPerSample == 16 ) {
            signed short *b = (signed short*)pBuffer;
            for( int i=0; i<nBufferLen/2; i++ ) {
                b[i] = (gain16 * (int)b[i])/32767;
            }
        }
    }
    [ringBuffer write:pBuffer length:nBufferLen cts:workBufCTS];
    
    TotalSize += nBufferLen;
    
    //if(!isStarted && TotalSize > 4096*8)
    {
        isStarted = 1;
    }
    
	return 0;
}

- (unsigned int) setBufferMute: (void*)pBuffer
				 writenPCMSize: (int*) pWritenPCMSize
{
	return 0;
}

- (float)gain {
    return gain;
}

- (void)setGain:(float)newGain {
    if( newGain < 0.0 )
        gain = 0.0;
    else if( newGain > 1.0 )
        gain = 1.0;
    else
        gain = newGain;
    
    gain16 = (int)(32767.0 * gain);
    gain8 = (int)(127.0 * gain);
    
    if( bitsPerSample != 8 && bitsPerSample != 16 && gain != 1.0 ) {
        NexLogE(LOG_TAG, @"[%i] gain adjustment not supported for this bitrate (%i)", __LINE__, bitsPerSample );
    }
}

- (void) dealloc {
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	NexLogD(LOG_TAG, @"[%i (0x%x)] dealloc", __LINE__, (unsigned int)self );
	AudioOutputUnitStop(io_unit_instance);
	AudioComponentInstanceDispose(io_unit_instance);
	[workBuffer release];
	[ringBuffer release];
	[super dealloc];
}

@end












