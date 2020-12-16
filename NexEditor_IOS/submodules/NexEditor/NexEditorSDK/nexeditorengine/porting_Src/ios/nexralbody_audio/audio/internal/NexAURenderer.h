//
//  NexAURenderer.h
//  nexCalBody
//
//  Created by Matthew Feinberg on 9/7/10.
//  Copyright 2010 Nextreaming. All rights reserved.
//
//  Overview:  Provides audio render services for iOS using
//             the Audio Unit interface for low latency.

#import <Foundation/Foundation.h>
#import <AudioUnit/AudioUnit.h>
#import "NexCTSRingBuffer.h"

//#define RAL_LOG_GET_CTS
//#define RAL_LOG_CTS_CALCS
//#define RAL_LOG_CTS_DISCONTINUITY
//#define RAL_LIMIT_CTS_INTERP_MS 40

@interface NexAURenderer : NSObject {
	AudioUnit io_unit_instance;				// This is the Audio Unit instance we're using for output
	
	unsigned int workBufferSize;
	
	NSMutableData *workBuffer;				// This is the buffer returned by getEmptyBuffer() and 
											// consumed by consumeBuffer().
	
	unsigned int workBufCTS;				// The CTS for the start of the working buffer; this is
											// used to persist the CTS value from the call to
											// getEmptyBuffer() until the call to consumeBuffer() when
											// we actually need to use it.
	
	BOOL isPaused;							// TRUE if audio playback is currently paused
    BOOL pauseReq;                          // TRUE if a pause state is requested
    BOOL appInactvie;                        // TRUE if the app is inactive
	
	unsigned int pauseCTS;					// While paused (either indefinitely, or temporarily for seek), this
											// is the CTS value at which the pause was issued, and isused as the
											// initial CTS when resuming after a pause.  When not paused, this is
											// undefined.
	
    BOOL bufferGottenNotConsumed;           // True if we're between get buffer and consume buffer calls
    
    float gain;                             // Current gain (0.0 ... 1.0)
    int gain16;                             // Gain factor scaled to -32768 ... +32767
    int gain8;                              // Gain factor scaled to -128 ... +127
    int bitsPerSample;
    
#ifdef RAL_LOG_CTS_DISCONTINUITY
    BOOL didCallGetCTS;
    unsigned int prevReturnedCTS;
#endif
    
	// These are public because the AudioUnit callback must access them; besides the
	// AudioUnit callback and the NexAURenderer internal implementation, nothing else
	// should access these.
	@public
	NexCTSRingBuffer *ringBuffer;			// This ring buffer is used to make the AudioUnit pull model
											// compatible with the Nextreaming push model of audio
											// output.  When consumeBuffer() is called, the working buffer
											// is fed into the ring buffer (a blocking operaiton, if there
											// is no space), and the AudioUnit callback function reads
											// audio data from the ring buffer for playback.
	
	unsigned int lastCallbackCTS;			// CTS of start of play of last AudioUnit callback
	uint64_t lastCallbackSysTime;			// System time of start of play of last AudioUnit callback
	BOOL callbackHasExecuted;				// YES if the AudioUnit callback has executed at least once

    int isStarted;
    int TotalSize;
}

@property (nonatomic,assign) float gain;

// The functions here are equivalent to the functions provided by
// a normal RalBody implementation, and the normal RalBody functions
// simply call through to these in cases where we used audio units.

- (NexAURenderer*)initWithOTI: (unsigned int) uCodecObjectTypeIndication
				 samplingRate: (unsigned int) uSamplingRate
				 channelCount: (unsigned int) uNumChannels
				bitsPerSample: (unsigned int) uBitsPerSample
			samplesPerChannel: (unsigned int) uSamplesPerChannel; 


- (unsigned int)getEmptyBuffer: (void**) ppEmptyBuffer 
					   maxSize: (int*)nMaxBufferSize;

- (unsigned int) consumeBuffer: (void*)pBuffer 
					 bufferLen: (unsigned int)nBufferLen
						forCTS: (unsigned int)uCTS
				 decodeSuccess: (unsigned int)isDecodeSuccess;

- (unsigned int) setBufferMute: (void*)pBuffer
				 writenPCMSize: (int*) pWritenPCMSize;

- (unsigned int) getCurrentCTS: (unsigned int *)puCTS;

- (unsigned int) clearBuffer;

- (BOOL) isPaused;

- (unsigned int) pause;

- (unsigned int) resume;

- (unsigned int) flush;

- (unsigned int) setCTS: (unsigned int) uCTS;

- (unsigned int) setPlaybackRate: (int)rate;

@end
