/******************************************************************************
 * File Name   : IPodLibrarySource.mm
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#include "IPodLibrarySource.hpp"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

/////////////////////////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////////////////////////

#define NFAF_BUFFER_SAMPLES                                 1024

/////////////////////////////////////////////////////////////////////////////////
// Types
/////////////////////////////////////////////////////////////////////////////////

typedef struct
{
    UInt32  ckID;
    UInt32  ckSize;
    UInt32  wavID;
    
    UInt32  ckIDFmt;
    UInt32  ckSizeFmt;
    UInt16  wFormatTag;
    UInt16  nChannels;
    UInt32  nSamplesPerSec;
    UInt32  nAvgBytesPerSec;
    UInt16  nBlockAlign;
    UInt16  wBitsPerSample;
    
    UInt32  ckIDData;
    UInt32  ckSizeData;
    
} WAVHEADER, *WAVHEADERRef;

/////////////////////////////////////////////////////////////////////////////////
// Global variables
/////////////////////////////////////////////////////////////////////////////////
// TEST DUMP
static WAVHEADER g_wavHeader = { 'FFIR', 0, 'EVAW', ' tmf', 16, 0x0001, 2, 44100, 44100 * 2 * 2, 4, 16, 'atad', 0 };
static NSURL *outURL = nil;
static NSOutputStream *outFile = nil;
static UInt32 totalSamples = 0;

const static char* _FormatError(OSStatus error)
{
    /*
     Result codes defined for Audio File Services
     */
    switch (error) {
        case kAudioFileUnspecifiedError:
            return "kAudioFileUnspecifiedError";
        case kAudioFileUnsupportedFileTypeError:
            return "kAudioFileUnsupportedFileTypeError";
        case kAudioFileUnsupportedDataFormatError:
            return "kAudioFileUnsupportedDataFormatError";
        case kAudioFileUnsupportedPropertyError:
            return "kAudioFileUnsupportedPropertyError";
        case kAudioFileBadPropertySizeError:
            return "kAudioFileBadPropertySizeError";
        case kAudioFilePermissionsError:
            return "kAudioFilePermissionsError";
        case kAudioFileNotOptimizedError:
            return "kAudioFileNotOptimizedError";
        case kAudioFileInvalidChunkError:
            return "kAudioFileInvalidChunkError";
        case kAudioFileDoesNotAllow64BitDataSizeError:
            return "kAudioFileDoesNotAllow64BitDataSizeError";
        case kAudioFileInvalidPacketOffsetError:
            return "kAudioFileInvalidPacketOffsetError";
        case kAudioFileInvalidFileError:
            return "kAudioFileInvalidFileError";
        case kAudioFileOperationNotSupportedError:
            return "kAudioFileOperationNotSupportedError";
        case kAudioFileNotOpenError:
            return "kAudioFileNotOpenError";
        case kAudioFileEndOfFileError:
            return "kAudioFileEndOfFileError";
        case kAudioFilePositionError:
            return "kAudioFilePositionError";
        case kAudioFileFileNotFoundError:
            return "kAudioFileFileNotFoundError";
        default:
            return "";
    }
}

IPodLibraryReader::IPodLibraryReader()
{
    audioFile = NULL;
    sampleL = (SndSample*)malloc( NFAF_BUFFER_SAMPLES * sizeof(SndSample) * 2 );
    sampleR = sampleL + NFAF_BUFFER_SAMPLES;
    ablBuffer = (AudioBufferList*)malloc( sizeof(AudioBufferList) * 2 );
    
    numSamples = 0;
    fileChannels = outputChannels = 0;
    fileSampleRate = outputSampleRate = 0;
    srRatio = 1;
}

IPodLibraryReader::~IPodLibraryReader()
{
    close();
    if( ablBuffer ) free( ablBuffer );
    if( sampleL ) free( sampleL );
}

bool IPodLibraryReader::openURL(char* fileURL)
{
    close();
    
    filePath = fileURL;
    OSStatus result = ExtAudioFileOpenURL( (CFURLRef)[NSURL URLWithString:[NSString stringWithFormat:@"%s", fileURL]], &audioFile );
    
    if( result != noErr || !audioFile )
    {
        NSLog( @"ExtAudioFileOpenURL fail" );
        return NO;
    }
    
    AudioStreamBasicDescription audioFormat;
    UInt32 uiSize = sizeof(audioFormat);
    
    result = ExtAudioFileGetProperty( audioFile, kExtAudioFileProperty_FileDataFormat, &uiSize, &audioFormat );
    
    if( result != noErr )
    {
        NSLog( @"ExtAudioFileGetProperty(kExtAudioFileProperty_FileDataFormat) fail" );
        ExtAudioFileDispose( audioFile );
        audioFile = NULL;
        return NO;
    }
    
    UInt64 frameCount = 0;
    uiSize = sizeof(frameCount);
    result = ExtAudioFileGetProperty( audioFile, kExtAudioFileProperty_FileLengthFrames, &uiSize, &frameCount );
    
    if( result != noErr )
    {
        NSLog( @"ExtAudioFileGetProperty(kExtAudioFileProperty_FileLengthFrames) fail" );
        ExtAudioFileDispose( audioFile );
        audioFile = NULL;
        return NO;
    }
    
    fileChannels = NFAF_DEF_CHANNEL;
    fileSampleRate = NFAF_DEF_SAMPLERATE;

//    frameCount = (UInt64)( frameCount * NFAF_DEF_SAMPLERATE / audioFormat.mSampleRate );
    
    audioFormat.mFormatID = kAudioFormatLinearPCM;
    audioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger| kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsNonInterleaved;
    audioFormat.mSampleRate = NFAF_DEF_SAMPLERATE;
    audioFormat.mBitsPerChannel = sizeof(SndSample) * 8;
    audioFormat.mFramesPerPacket = 1;
    audioFormat.mBytesPerFrame = sizeof(SndSample);
    audioFormat.mBytesPerPacket = sizeof(SndSample);
    audioFormat.mChannelsPerFrame = NFAF_DEF_CHANNEL;
    
    uiSize = sizeof(audioFormat);
    result = ExtAudioFileSetProperty( audioFile, kExtAudioFileProperty_ClientDataFormat, uiSize, &audioFormat );
    
    if( result != noErr )
    {
        NSLog( @"ExtAudioFileSetProperty(kExtAudioFileProperty_ClientDataFormat) fail" );
        ExtAudioFileDispose( audioFile );
        audioFile = NULL;
        return NO;
    }
    
    numSamples = (UInt32)frameCount;
    outputChannels = 2;
    outputSampleRate = NFAF_DEF_SAMPLERATE;
    srRatio = fileSampleRate / outputSampleRate;
    
    ablBuffer->mNumberBuffers = fileChannels;

    ablBuffer->mBuffers[ 0 ].mNumberChannels = 1;
    ablBuffer->mBuffers[ 0 ].mData = sampleL;
    ablBuffer->mBuffers[ 0 ].mDataByteSize = NFAF_BUFFER_SAMPLES * sizeof(SndSample);
    
    if( ablBuffer->mNumberBuffers > 1 )
    {
        ablBuffer->mBuffers[ 1 ].mNumberChannels = 1;
        ablBuffer->mBuffers[ 1 ].mData = sampleR;
        ablBuffer->mBuffers[ 1 ].mDataByteSize = NFAF_BUFFER_SAMPLES * sizeof(SndSample);
    }
    
    return YES;
}

void IPodLibraryReader::close()
{
    if( audioFile )
    {
        ExtAudioFileDispose( audioFile );
        audioFile = NULL;
    }
}

bool IPodLibraryReader::read(unsigned int* samples, SndSample* pcm)
{
    
    if( !audioFile || !samples || !*samples )
        return NO;
    
    UInt32 wantRead = *samples;
    SndSample *pOut = pcm;
    
    while( wantRead )
    {
        UInt32 read = wantRead > NFAF_BUFFER_SAMPLES ? NFAF_BUFFER_SAMPLES : wantRead;
        
        OSStatus ret = ExtAudioFileRead( audioFile, &read, ablBuffer );
        
        if( ret != noErr )
        {
            NSLog( @"NFAudioFileRead fail" );
            return NO;
        }
        
        if( read )
        {
            SndSample *inL = sampleL;
            
            
            if( ablBuffer->mNumberBuffers == 1 )
            {
                // force stereo
                for( UInt32 i = 0; i < read; i++ )
                {
                    SndSample val = *inL++;
                    *pOut++ = val;
                    *pOut++ = val;
                }
            } else
            {
                SndSample *inR = sampleR;
                
                for( UInt32 i = 0; i < read; i++ )
                {
                    *pOut++ = *inL++;
                    *pOut++ = *inR++;
                }
            }
            
            wantRead -= read;
        } else
        {
            // eof
            *samples = 0;
            break;
        }
    }
    
    return YES;

}

bool IPodLibraryReader::seek(int* samplePosition)
{
    OSStatus ret;
    
    if( *samplePosition > numSamples )
        *samplePosition = numSamples;
    
    // offset must be sample position in original file(with file sample rate)
    int offset = (int)( *samplePosition * srRatio );
    
    ret = ExtAudioFileSeek( audioFile, offset );
    
    if( ret != noErr )
    {
        NSLog( @"ExtAudioFileSeek fail" );
        return NO;
    }
    
    return YES;
}

bool IPodLibraryReader::ftell(int *samplePosition)
{
    OSStatus ret;
        
    if( *samplePosition > numSamples )
        *samplePosition = numSamples;
    
    // offset must be sample position in original file(with file sample rate)
    SInt64 offset = (int)( *samplePosition * srRatio );

    if(audioFile == NULL)
    {
        return NO;
    }
    
    ret = ExtAudioFileTell( audioFile, &offset );
    if( ret != noErr )
    {
        NSLog( @"ExtAudioFileTell fail" );
        return NO;
    }
    
    *samplePosition = (int)offset;
    
    return YES;
}

bool IPodLibraryReader::dumpOpen()
{
    if(outURL == nil)
    {
        outURL = [[NSURL alloc] initFileURLWithPath:[NSTemporaryDirectory() stringByAppendingPathComponent:@"/testwav_in_nexeditor.wav"]];
        
        if( !outURL )
        {
            NSLog( @"alloc url fail" );
            return NO;
        }
        
        [[NSFileManager defaultManager] removeItemAtPath:outURL.path error:nil];
        
        if( ![[NSFileManager defaultManager] createFileAtPath:outURL.path contents:nil attributes:nil] )
        {
            NSLog( @"WAV file creation fail, createFileAtPath" );
            [outURL release];
            outURL = nil;
            return NO;
        }
        
        outFile = [[NSOutputStream alloc] initWithURL:outURL append:NO];
        
        if( !outFile )
        {
            NSLog( @"create output stream fail" );
            [outURL release];
            outURL = nil;
            return NO;
        }
        
        [outFile open];
        
        // write dummy space for header
        dump(0, (uint8_t*)&g_wavHeader, sizeof(WAVHEADER));
    }
    
    return YES;
}

bool IPodLibraryReader::dumpClose()
{
    [outFile close];
    if( totalSamples )
    {
        updateWAVHeaderToFile();
    }
    
    return YES;
}

void IPodLibraryReader::resetBufferSize()
{
    AudioBuffer *buffer;
    UInt32 i;
    
    for( i = 0; i < ablBuffer->mNumberBuffers; i++ )
    {
        buffer = &( ablBuffer->mBuffers[ i ] );
        buffer->mDataByteSize = NFAF_BUFFER_SAMPLES * sizeof(SndSample);
    }
}

void IPodLibraryReader::updateWAVHeaderToFile()
{
    WAVHEADER header;
    
    memcpy( &header, &g_wavHeader, sizeof(WAVHEADER) );
    header.ckSize = 4 + 24 + 8 + 2 * 2 * totalSamples;
    header.ckSizeData = 2 * 2 * totalSamples;
    
    NSFileHandle *outFile = [NSFileHandle fileHandleForUpdatingURL:outURL error:nil];
    
    if( !outFile )
    {
        NSLog( @"NFPCMWriter:updateWAVHeaderToFile:fileHandleForUpdatingURL fail" );
        return ;
    }
    
    NSData *data = [[NSData alloc] initWithBytesNoCopy:&header length:sizeof(WAVHEADER) freeWhenDone:NO];
    
    if( !data )
    {
        NSLog( @"NFPCMWriter:updateWAVHeaderToFile:init nsdata fail" );
        return ;
    }
    
    [outFile writeData:data];
    [data release];
    
    [outFile closeFile];
}

bool IPodLibraryReader::dump(int samples, uint8_t* pcm, int length)
{
    if(samples != 0)
        totalSamples += samples;
    
    while( length > 0 )
    {
        if( [outFile hasSpaceAvailable] )
        {
            int wrote;
            
            if( ( wrote = (int)[outFile write:(uint8_t*)pcm maxLength:length] ) == -1 )
            {
                NSLog( @"writeToStream fail" );
                return NO;
            }
            
            length -= wrote;
            pcm += wrote;
        }
    }
    
    return YES;
}

int IPodLibraryReader::getNumOfChannels()
{
    return fileChannels;
}

double IPodLibraryReader::getSampleRate()
{
    return fileSampleRate;
}

long long IPodLibraryReader::getTotalSize()
{
    AVURLAsset *songAsset = [AVURLAsset URLAssetWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"%s", filePath]] options:nil];
    AVAssetExportSession *exportSession = [[AVAssetExportSession alloc] initWithAsset: songAsset presetName: AVAssetExportPresetAppleM4A];
    CMTime half = CMTimeMultiplyByFloat64(exportSession.asset.duration, 1);
    exportSession.timeRange = CMTimeRangeMake(kCMTimeZero, half);
    long long size = exportSession.estimatedOutputFileLength;
    [exportSession release];
    return size;
}

float IPodLibraryReader::getDuration()
{
    AVURLAsset* audioAsset = [AVURLAsset URLAssetWithURL:[NSURL URLWithString:[NSString stringWithFormat:@"%s", filePath]] options:nil];
    CMTime audioDuration = audioAsset.duration;
    float audioDurationMilliSeconds = CMTimeGetSeconds(audioDuration) * 1000;
    
    return audioDurationMilliSeconds;
}

int IPodLibraryReader::getBitrate()
{
    OSStatus status;
    AudioFileID audioFileID = NULL;
    
    UInt32 size = sizeof(audioFileID);
    status = ExtAudioFileGetProperty(audioFile, kExtAudioFileProperty_AudioFile, &size, &audioFileID);
    if( status != noErr )
    {
        NSLog( @"ExtAudioFileGetProperty(kExtAudioFileProperty_AudioFile) Error" );
        return 0;
    }
    
    UInt32 bitrate = 0;
    size = sizeof(bitrate);
    status = AudioFileGetProperty(audioFileID, kAudioFilePropertyBitRate, &size, &bitrate);
    if ( status != noErr ) {
        NSLog(@"AudioFileGetProperty(kAudioFilePropertyBitRate) Error(:%s)", _FormatError(status));
        return 0;
    }
    
    return bitrate;
}

unsigned int IPodLibraryReader::getTotalFrameCount()
{
    return numSamples;
}
