/*
 * File Name   : StreamWriterContext.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#ifndef StreamWriterContext_h
#define StreamWriterContext_h

#include <stdint.h>

typedef enum  {
    StreamWriterSampleFormatCVPixelBuffer,          // CVPixelBufferRef
    StreamWriterSampleFormatRawAudioSampleBlock     // StreamWriterRawAudioSampleBlock
} StreamWriterSampleFormat;

typedef struct {
    StreamWriterSampleFormat format;
    uint32_t width;
    uint32_t height;
    float frameRate;
    uint32_t bitrate;
    uint32_t rotationAngle;   // 0, 90, or 270
} StreamWriterVideoInfo;

typedef struct {
    StreamWriterSampleFormat format;
    float samplingRate;
    uint8_t channels;
    uint8_t bitsPerChannel;
    uint32_t bitrate;
} StreamWriterAudioInfo;

typedef enum {
    StreamWriterTrackTypeAudio = 0x01,
    StreamWriterTrackTypeVideo = 0x02,
} StreamWriterTrackType;

typedef struct {
    StreamWriterSampleFormat video;
    StreamWriterSampleFormat audio;
} StreamWriterSampleFormats;

typedef struct {
    void *pSamples;
    size_t size;
} StreamWriterRawAudioSampleBlock;

typedef struct _StreamWriterContext {
    void (*begin)(struct _StreamWriterContext *ctx, StreamWriterVideoInfo *pVideoInfo, StreamWriterAudioInfo *pAudioInfo);
    void (*appendSample)(struct _StreamWriterContext *ctx, StreamWriterTrackType trackType, int64_t timeMs, void *sample);
    void (*markTrackFinished)(struct _StreamWriterContext *ctx, StreamWriterTrackType trackType);
    uint8_t (*isTrackReadyForMoreSamples)(struct _StreamWriterContext *ctx, StreamWriterTrackType trackType);
    /// 1 to finish, 0 to abort
    void (*finish)(struct _StreamWriterContext *ctx, uint8_t finish);
    void *priv;
} StreamWriterContext;

typedef StreamWriterContext *(*CreateStreamWriterContextFunction)(void);

#endif /* StreamWriterContext_h */
