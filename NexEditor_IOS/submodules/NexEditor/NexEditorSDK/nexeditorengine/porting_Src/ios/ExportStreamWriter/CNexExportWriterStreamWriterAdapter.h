/*
 * File Name   : CNexAVAssetExportWriter.h
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

#ifndef CNexAVAssetExportWriter_hpp
#define CNexAVAssetExportWriter_hpp

#include "NEXVIDEOEDITOR_ExportWriter.h"
#include "StreamWriterContext.h"

/// Implements CNexExportWriter interface by forwarding incoming audio/video samples to StreamWriterContext
class CNexExportWriterStreamWriterAdapter: public CNexExportWriter
{
public:
    CNexExportWriterStreamWriterAdapter(void);
    virtual ~CNexExportWriterStreamWriterAdapter(void);
    
#pragma mark - CNexExportWriter interface
    
    NXBOOL setFilePath(char* pStrTargetPath);
    NXBOOL initFileWriter(long long llMaxFileSize, unsigned int uiDuration = 0);
    NXBOOL startFileWriter();
    NXBOOL endFileWriter(unsigned int* pDuration, NXBOOL bCancel = FALSE);
    NXBOOL isStarted();
    
    void setAudioOnlyMode(NXBOOL bEnable);
    NXBOOL getVideoOnlyMode();
    void setVideoOnlyMode(NXBOOL bEnable);
    NXBOOL isVideoFrameWriteStarted();
    
    NXBOOL setAudioCodecInfo(unsigned int uiType, unsigned int uiSampleRate, unsigned int uiNumOfChannel, unsigned int uiBitRate);
    NXBOOL setAudioCodecInfo(unsigned int uiType, unsigned char* pDSI, unsigned int uiDSISize);
    NXBOOL setVideoCodecInfo(unsigned int uiType, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, unsigned int uiVideoFrameRate, unsigned int uiBitRate, int iProfile=0, int iLevel=0);
    NXBOOL setVideoCodecInfo(unsigned int uiType, unsigned char* pDSI, int iSize);
    NXBOOL setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize);
    NXBOOL setAudioFrameForVoice(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize);
    int setAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize);
    int writeAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize);
    int writeVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize);
    
    NXBOOL setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize); // without encode.
    
    NXBOOL checkEncoderEOS(unsigned int uiTime);
    
    NXBOOL setBaseVideoFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet = NULL);
    NXBOOL setBaseVideoFrame(unsigned int uiCTS, NXBOOL bEnd, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet);
    int setBaseVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize);
    NXBOOL setBaseVideoFrameTime(unsigned int uiCTS);
    
    NXBOOL setPause(NXBOOL bEncodecRelease);
    NXBOOL setResume();
    NXBOOL getPause();
    
    int getEncodeWidth();
    int getEncodeHeight();
    
    int getEncoderFrameInterval();
    unsigned int getEncodeInputFormat();
    
    void setVideoRotate(int iRotate);
    void setAudioSamplingRate(int iSamplingRate);
    
    CVideoWriteBuffer* getBuffer();
    NXBOOL releaseBuffer(CVideoWriteBuffer* pBuffer);
    
    void* getMediaCodecInputSurface();
    void setMediaCodecTimeStamp(unsigned int uiTime);
    
    void setTotalDuration(NXINT64 qAudioDuration, NXINT64 qVideoDuration);
    unsigned int getAudioTime();
    unsigned int getVideoTime();
    NXBOOL isDirectExportWriteEndVideo(unsigned int uiTotalTime);
    int getErrorStatus();
    
    unsigned int getDECurrentVideoClipID();

    NXBOOL isReadyForMoreVideo(void);
    NXBOOL isReadyForMoreAudio(void);
	void setLastTrackEnded(NXBOOL bSet);

#pragma mark -
    /// If set, all the stream writing requests will be forwarded to the context. Otherwise, nothing will happen while exporting.
    void setStreamWriterContext(StreamWriterContext *context);
    
private:
    uint32_t uiDuration;
    NXBOOL bStarted;
    uint8_t uMediaMask;
    uint32_t uiVideoFrameCount;
    uint32_t uiAudioCTS;
    uint32_t uiVideoCTS;
    uint32_t frameIntervalPrecalculated;
    NXBOOL bAudioFinished;
    NXBOOL bVideoFinished;
    
    StreamWriterVideoInfo stVideoInfo;
    StreamWriterAudioInfo stAudioInfo;

    StreamWriterContext *streamWriterContext;

    void markFinishedMediaTrack(StreamWriterTrackType mediaType);
    NXBOOL isMediaTrackFinished(StreamWriterTrackType mediaType);
    
};


#endif /* CNexAVAssetExportWriter_hpp */
