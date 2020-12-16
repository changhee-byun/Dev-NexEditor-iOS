/*
 * File Name   : StreamWriter.m
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

#import "StreamWriter.h"

#pragma mark - StreamWriterBind
static void _ctx_begin(struct _StreamWriterContext *ctx, StreamWriterVideoInfo *pVideoInfo, StreamWriterAudioInfo *pAudioInfo)
{
    id<StreamWriter> streamWriter = (id<StreamWriter>) ctx->priv;
    
    [streamWriter beginWithVideoInfo:pVideoInfo audioInfo:pAudioInfo];
}

static void _ctx_appendSample(struct _StreamWriterContext *ctx, StreamWriterTrackType trackType, int64_t timeMs, void *sample)
{
    id<StreamWriter> streamWriter = (id<StreamWriter>) ctx->priv;

    [streamWriter appendSample:sample trackType:trackType time:timeMs];
}

static void _ctx_markTrackFinished(struct _StreamWriterContext *ctx, StreamWriterTrackType trackType)
{
    id<StreamWriter> streamWriter = (id<StreamWriter>) ctx->priv;
    
    [streamWriter markTrackFinished:trackType];
}

static uint8_t _ctx_isTrackReadyForMoreSamples(struct _StreamWriterContext *ctx, StreamWriterTrackType trackType)
{
    id<StreamWriter> streamWriter = (id<StreamWriter>) ctx->priv;
    
    return [streamWriter isTrackReadyForMoreSamples:trackType] ? 1 : 0;
}

static void _ctx_finish(struct _StreamWriterContext *ctx, uint8_t finish)
{
    id<StreamWriter> streamWriter = (id<StreamWriter>) ctx->priv;
    @autoreleasepool {
    [streamWriter finish:finish ? YES : NO];
    CFRelease(ctx->priv);
        ctx->priv = NULL;
    }
}

void StreamWriterBindToContext(id<StreamWriter> streamWriter, StreamWriterContext *context)
{
    // From now on, no need to strong hold streamWriter anywhere else. context->finish will release
    
    context->priv = (void *) CFBridgingRetain(streamWriter);
    context->begin = _ctx_begin;
    context->appendSample = _ctx_appendSample;
    context->markTrackFinished = _ctx_markTrackFinished;
    context->isTrackReadyForMoreSamples = _ctx_isTrackReadyForMoreSamples;
    context->finish = _ctx_finish;
}
