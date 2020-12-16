/*
 * File Name   : CNexAVAssetExportWriter.cpp
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

#include "CNexExportWriterStreamWriterAdapter.h"
#include "NEXVIDEOEDITOR_Clip.h"

#define __LOGTAG__  "CNexAVAssetExportWriter.cpp"

#define NAEW_RESULT_FAILED   1

#define MEDIAMASK_AUDIO         0x01
#define MEDIAMASK_VIDEO         0x02
#define MEDIAMASK_AUDIOVIDEO    (MEDIAMASK_AUDIO | MEDIAMASK_VIDEO)

/*
 * (30.0 * FRAMERATE_INTEGER_BASE) -> 3000
 * 3000 / FRAMERATE_INTEGER_BASE -> 30.0
 */
#define FRAMERATE_INTEGER_BASE    (100.0)

CNexExportWriterStreamWriterAdapter::CNexExportWriterStreamWriterAdapter(void)
{
    uiDuration = 0;
    bStarted = FALSE;
    uMediaMask = MEDIAMASK_AUDIOVIDEO;
    uiVideoFrameCount = 0;
    
    uiAudioCTS = 0;
    uiVideoCTS = 0;
    bAudioFinished = FALSE;
    bVideoFinished = FALSE;
    
    stVideoInfo.format = StreamWriterSampleFormatCVPixelBuffer;
    stVideoInfo.width = 0;
    stVideoInfo.height = 0;
    stVideoInfo.frameRate = EDITOR_DEFAULT_FRAME_RATE / FRAMERATE_INTEGER_BASE;
    stVideoInfo.rotationAngle = 0;
    
    stAudioInfo.format = StreamWriterSampleFormatRawAudioSampleBlock;
    stAudioInfo.samplingRate = EDITOR_DEFAULT_SAMPLERATE;
    stAudioInfo.channels = EDITOR_DEFAULT_CHANNELS;
    stAudioInfo.bitsPerChannel = EDITOR_DEFAULT_BITFORSAMPLE;
    stAudioInfo.bitrate = EDITOR_DEFAULT_AUDIO_BITRATE;

    frameIntervalPrecalculated = (EDITOR_DEFAULT_FRAME_TIME / EDITOR_DEFAULT_FRAME_RATE);
    streamWriterContext = NULL;
}

CNexExportWriterStreamWriterAdapter::~CNexExportWriterStreamWriterAdapter()
{
}

#pragma mark -
void CNexExportWriterStreamWriterAdapter::setStreamWriterContext(StreamWriterContext *context)
{
    this->streamWriterContext = context;
}


#pragma mark -
NXBOOL CNexExportWriterStreamWriterAdapter::setFilePath(char* pStrTargetPath)
{
    return TRUE;
}

NXBOOL CNexExportWriterStreamWriterAdapter::initFileWriter(long long llMaxFileSize, unsigned int uiDuration)
{
    this->uiDuration = uiDuration;
    return TRUE;
}

NXBOOL CNexExportWriterStreamWriterAdapter::startFileWriter()
{
    if (bStarted) return FALSE;

    // Reset progress status
    uMediaMask = MEDIAMASK_AUDIOVIDEO;
    uiVideoFrameCount = 0;
    
    uiAudioCTS = 0;
    uiVideoCTS = 0;
    bAudioFinished = FALSE;
    bVideoFinished = FALSE;

    if (streamWriterContext) {
        streamWriterContext->begin(streamWriterContext, &stVideoInfo, &stAudioInfo);
    }
    bStarted = TRUE;
    return TRUE;
}

NXBOOL CNexExportWriterStreamWriterAdapter::endFileWriter(unsigned int* pDuration, NXBOOL bCancel)
{
    if (streamWriterContext) {
        uint8_t finish = bCancel ? 0 : 1;
        streamWriterContext->finish(streamWriterContext, finish);
        streamWriterContext = NULL;
    }
    // TODO: update pDuration
    
    bStarted = FALSE;
    return TRUE;
}

NXBOOL CNexExportWriterStreamWriterAdapter::isStarted()
{
    return bStarted;
}

void CNexExportWriterStreamWriterAdapter::setAudioOnlyMode(NXBOOL bEnable)
{
    uMediaMask = MEDIAMASK_AUDIO;
}

NXBOOL CNexExportWriterStreamWriterAdapter::getVideoOnlyMode()
{
    return ((uMediaMask & MEDIAMASK_VIDEO) && ((uMediaMask & MEDIAMASK_AUDIO) == 0));
}

void CNexExportWriterStreamWriterAdapter::setVideoOnlyMode(NXBOOL bEnable)
{
    uMediaMask = MEDIAMASK_VIDEO;
}

NXBOOL CNexExportWriterStreamWriterAdapter::isVideoFrameWriteStarted()
{
    return uiVideoFrameCount > 0;
}

NXBOOL CNexExportWriterStreamWriterAdapter::setAudioCodecInfo(unsigned int uiType, unsigned int uiSampleRate, unsigned int uiNumOfChannel, unsigned int uiBitRate)
{
    stAudioInfo.samplingRate = (uint32_t) uiSampleRate;
    stAudioInfo.channels = uiNumOfChannel;
    stAudioInfo.bitsPerChannel = EDITOR_DEFAULT_BITFORSAMPLE;
    stAudioInfo.bitrate = uiBitRate;
    return TRUE;
}


NXBOOL CNexExportWriterStreamWriterAdapter::setVideoCodecInfo(unsigned int uiType, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, unsigned int uiVideoFrameRate, unsigned int uiBitRate, int iProfile, int iLevel)
{
    stVideoInfo.width = (uint32_t) iWidth;
    stVideoInfo.height = (uint32_t) iHeight;
    stVideoInfo.frameRate = (float) (uiVideoFrameRate / FRAMERATE_INTEGER_BASE);
    frameIntervalPrecalculated = (EDITOR_DEFAULT_FRAME_TIME / uiVideoFrameRate);
    return TRUE;
}

NXBOOL CNexExportWriterStreamWriterAdapter::setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize)
{
    if (isMediaTrackFinished(StreamWriterTrackTypeAudio)) {
        return TRUE;
    }
	
    if (uiDuration <= uiAudioCTS ) {
        // Allow one extra audio frame
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[%s %d] audio drop at %u >= %u", __LOGTAG__, __LINE__, uiCTS, uiDuration);
        markFinishedMediaTrack(StreamWriterTrackTypeAudio);
        return TRUE;
    }
    
    if (streamWriterContext) {
        StreamWriterRawAudioSampleBlock block = { pFrame, (size_t) uiFrameSize};
        streamWriterContext->appendSample(streamWriterContext, StreamWriterTrackTypeAudio, (int64_t) uiCTS, &block);
    }
    
    uiAudioCTS = (uint32_t) uiCTS;
    return TRUE;
}

NXBOOL CNexExportWriterStreamWriterAdapter::setBaseVideoFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet)
{
    if (isMediaTrackFinished(StreamWriterTrackTypeVideo)) {
        return TRUE;
    }
    if (uiDuration < uiCTS) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[%s %d] video drop at %u > %u", __LOGTAG__, __LINE__, uiCTS, uiDuration);
        markFinishedMediaTrack(StreamWriterTrackTypeVideo);
        return TRUE;
    }
    
    if (streamWriterContext) {
        streamWriterContext->appendSample(streamWriterContext, StreamWriterTrackTypeVideo, (int64_t) uiCTS, pFrame);
    }
    uiVideoFrameCount++;
    
    uiVideoCTS = uiCTS;
    bStarted = TRUE;
    return TRUE;
}

int CNexExportWriterStreamWriterAdapter::getEncodeWidth()
{
    return (int) stVideoInfo.width;
}
int CNexExportWriterStreamWriterAdapter::getEncodeHeight()
{
    return (int) stVideoInfo.height;
}

/// frame interval in unit: miliseconds x 100. For example, 33.33ms is denoted as 3333
int CNexExportWriterStreamWriterAdapter::getEncoderFrameInterval()
{
    return frameIntervalPrecalculated;
}

unsigned int CNexExportWriterStreamWriterAdapter::getEncodeInputFormat()
{
    return NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY;
}

void CNexExportWriterStreamWriterAdapter::setVideoRotate(int iRotate)
{
    stVideoInfo.rotationAngle = iRotate;
}

void CNexExportWriterStreamWriterAdapter::setAudioSamplingRate(int iSamplingRate)
{
    stAudioInfo.samplingRate = iSamplingRate;
}

unsigned int CNexExportWriterStreamWriterAdapter::getAudioTime()
{
    return (unsigned int) uiAudioCTS;
}
unsigned int CNexExportWriterStreamWriterAdapter::getVideoTime()
{
    return (unsigned int) uiVideoCTS;
}

int CNexExportWriterStreamWriterAdapter::getErrorStatus()
{
    return 0;
}

NXBOOL CNexExportWriterStreamWriterAdapter::isReadyForMoreVideo(void)
{
    NXBOOL result = TRUE;

    if (streamWriterContext) {
        result = streamWriterContext->isTrackReadyForMoreSamples(streamWriterContext, StreamWriterTrackTypeVideo) > 0 ? TRUE : FALSE;
    }
    return result;
}

NXBOOL CNexExportWriterStreamWriterAdapter::isReadyForMoreAudio(void)
{
    static const int audio_time_advance_limit_ms = 1000;

    // 1. NOT if audioTime > videoTime + advance_limit
    // 2. NOT if audioTime reached max duration
    // 3. AVAssetWriterInput can accept more
    NXBOOL result = uiAudioCTS < (uiVideoCTS + audio_time_advance_limit_ms);

    if (result) {
        result = !isMediaTrackFinished(StreamWriterTrackTypeAudio);
    }

    if (result) {
        if (streamWriterContext) {
            result = streamWriterContext->isTrackReadyForMoreSamples(streamWriterContext, StreamWriterTrackTypeAudio) > 0 ? TRUE : FALSE;
        }
    }
    return result;
}

void CNexExportWriterStreamWriterAdapter::setLastTrackEnded(NXBOOL bSet)
{
	// UNSUPPORTED
}

void CNexExportWriterStreamWriterAdapter::markFinishedMediaTrack(StreamWriterTrackType mediaType)
{
    NXBOOL needsMark = FALSE;
    
    if (isMediaTrackFinished(mediaType)) {
        return;
    }
    
    if (mediaType == StreamWriterTrackTypeAudio && !bAudioFinished) {
        bAudioFinished = TRUE;
        needsMark = TRUE;
    } else if (mediaType == StreamWriterTrackTypeVideo && !bVideoFinished) {
        bVideoFinished = TRUE;
        needsMark = TRUE;
    }
    
    if (needsMark && streamWriterContext) {
        streamWriterContext->markTrackFinished(streamWriterContext, mediaType);
    }
}

NXBOOL CNexExportWriterStreamWriterAdapter::isMediaTrackFinished(StreamWriterTrackType mediaType)
{
    return (bAudioFinished && mediaType == StreamWriterTrackTypeAudio) || (bVideoFinished && StreamWriterTrackTypeVideo);
}

#pragma mark - Unsupported Methods

NXBOOL CNexExportWriterStreamWriterAdapter::setAudioCodecInfo(unsigned int uiType, unsigned char* pDSI, unsigned int uiDSISize)
{
    // UNSUPPORTED
    return FALSE;
}
NXBOOL CNexExportWriterStreamWriterAdapter::setVideoCodecInfo(unsigned int uiType, unsigned char* pDSI, int iSize)
{
    // UNSUPPORTED
    return FALSE;
}
NXBOOL CNexExportWriterStreamWriterAdapter::setAudioFrameForVoice(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize)
{
    // UNSUPPORTED
    return FALSE;
}
int CNexExportWriterStreamWriterAdapter::setAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
    // UNSUPPORTED
    return NAEW_RESULT_FAILED;
}
int CNexExportWriterStreamWriterAdapter::writeAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
    // UNSUPPORTED
    return NAEW_RESULT_FAILED;
}
int CNexExportWriterStreamWriterAdapter::writeVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize)
{
    // UNSUPPORTED
    return NAEW_RESULT_FAILED;
}
NXBOOL CNexExportWriterStreamWriterAdapter::setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
    // UNSUPPORTED
    return FALSE;
}
NXBOOL CNexExportWriterStreamWriterAdapter::checkEncoderEOS(unsigned int uiTime)
{
    // UNSUPPORTED
    return FALSE;
}
NXBOOL CNexExportWriterStreamWriterAdapter::setBaseVideoFrame(unsigned int uiCTS, NXBOOL bEnd, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet)
{
    // UNSUPPORTED
    return FALSE;
}
int CNexExportWriterStreamWriterAdapter::setBaseVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize)
{
    // UNSUPPORTED
    return NAEW_RESULT_FAILED;
}
NXBOOL CNexExportWriterStreamWriterAdapter::setBaseVideoFrameTime(unsigned int uiCTS)
{
    // UNSUPPORTED
    return FALSE;
}
NXBOOL CNexExportWriterStreamWriterAdapter::setPause(NXBOOL bEncodecRelease)
{
    // UNSUPPORTED
    return FALSE;
}
NXBOOL CNexExportWriterStreamWriterAdapter::setResume()
{
    // UNSUPPORTED
    return FALSE;
}
NXBOOL CNexExportWriterStreamWriterAdapter::getPause()
{
    // UNSUPPORTED
    return FALSE;
}
CVideoWriteBuffer* CNexExportWriterStreamWriterAdapter::getBuffer()
{
    // UNSUPPORTED
    return NULL;
}
NXBOOL CNexExportWriterStreamWriterAdapter::releaseBuffer(CVideoWriteBuffer* pBuffer)
{
    // UNSUPPORTED
    return FALSE;
}
void* CNexExportWriterStreamWriterAdapter::getMediaCodecInputSurface()
{
    // UNSUPPORTED
    return NULL;
}
void CNexExportWriterStreamWriterAdapter::setMediaCodecTimeStamp(unsigned int uiTime)
{
    // UNSUPPORTED
}

void CNexExportWriterStreamWriterAdapter::setTotalDuration(NXINT64 qAudioDuration, NXINT64 qVideoDuration)
{
    // UNSUPPORTED
}
NXBOOL CNexExportWriterStreamWriterAdapter::isDirectExportWriteEndVideo(unsigned int uiTotalTime)
{
    // UNSUPPORTED
    return FALSE;
}
unsigned int CNexExportWriterStreamWriterAdapter::getDECurrentVideoClipID()
{
    // UNSUPPORTED
    return INVALID_CLIP_ID;
}
