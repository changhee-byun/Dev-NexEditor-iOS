/*
 * File Name   : NEXVIDEOEDITOR_ExportWriter.h
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

#ifndef CNexExportWriter_h
#define CNexExportWriter_h

#include "nexIRef.h"
#include "nexDef.h"
#include "NexTypeDef.h"

class CVideoWriteBuffer;

class CNexExportWriter : public CNxRef<INxRefObj>
{
public:
    virtual ~CNexExportWriter(){}
    
    virtual NXBOOL setFilePath(char* pStrTargetPath) = 0;
    virtual NXBOOL initFileWriter(long long llMaxFileSize, unsigned int uiDuration = 0) = 0;
    virtual NXBOOL startFileWriter() = 0;
    virtual NXBOOL endFileWriter(unsigned int* pDuration, NXBOOL bCancel = FALSE) = 0;
    virtual NXBOOL isStarted() = 0;
    
    virtual void setAudioOnlyMode(NXBOOL bEnable) = 0;
    virtual NXBOOL getVideoOnlyMode() = 0;
    virtual void setVideoOnlyMode(NXBOOL bEnable) = 0;
    virtual NXBOOL isVideoFrameWriteStarted() = 0;
    
    virtual NXBOOL setAudioCodecInfo(unsigned int uiType, unsigned int uiSampleRate, unsigned int uiNumOfChannel, unsigned int uiBitRate) = 0;
    virtual NXBOOL setAudioCodecInfo(unsigned int uiType, unsigned char* pDSI, unsigned int uiDSISize) = 0;
    virtual NXBOOL setVideoCodecInfo(unsigned int uiType, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, unsigned int uiVideoFrameRate, unsigned int uiBitRate, int iProfile=0, int iLevel=0) = 0;
    virtual NXBOOL setVideoCodecInfo(unsigned int uiType, unsigned char* pDSI, int iSize) = 0;
    virtual NXBOOL setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize) = 0;
    virtual NXBOOL setAudioFrameForVoice(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize) = 0;
    virtual int setAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize) = 0;
    virtual int writeAudioFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize) = 0;
    virtual int writeVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize) = 0;
    
    virtual NXBOOL setAudioFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize) = 0; // without encode.
    
    virtual NXBOOL checkEncoderEOS(unsigned int uiTime) = 0;
    
    virtual NXBOOL setBaseVideoFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet = NULL) = 0;
    virtual NXBOOL setBaseVideoFrame(unsigned int uiCTS, NXBOOL bEnd, unsigned int* pDuration, unsigned int* pSize, NXBOOL* pEncodeRet) = 0;
    virtual int setBaseVideoFrameWithoutEncode(unsigned int uiID, unsigned int uiCTS, unsigned int uiDTS, void* pBuffer, int iBufferSize, unsigned int* pDuration, unsigned int* pSize) = 0;
    virtual NXBOOL setBaseVideoFrameTime(unsigned int uiCTS) = 0;
    
    virtual NXBOOL setPause(NXBOOL bEncodecRelease) = 0;
    virtual NXBOOL setResume() = 0;
    virtual NXBOOL getPause() = 0;
    
    virtual int getEncodeWidth() = 0;
    virtual int getEncodeHeight() = 0;
    
    virtual int getEncoderFrameInterval() = 0;
    virtual unsigned int getEncodeInputFormat() = 0;
    
    virtual void setVideoRotate(int iRotate) = 0;
    virtual void setAudioSamplingRate(int iSamplingRate) = 0;
    
    virtual CVideoWriteBuffer* getBuffer() = 0;
    virtual NXBOOL releaseBuffer(CVideoWriteBuffer* pBuffer) = 0;
    
    virtual void* getMediaCodecInputSurface() = 0;
    virtual void setMediaCodecTimeStamp(unsigned int uiTime) = 0;
    
    virtual void setTotalDuration(NXINT64 qAudioDuration, NXINT64 qVideoDuration) = 0;
    virtual unsigned int getAudioTime() = 0;
    virtual unsigned int getVideoTime() = 0;
    virtual NXBOOL isDirectExportWriteEndVideo(unsigned int uiTotalTime) = 0;
    virtual int getErrorStatus() = 0;
    
    virtual unsigned int getDECurrentVideoClipID() = 0;
    
    virtual NXBOOL isReadyForMoreVideo(void);
    virtual NXBOOL isReadyForMoreAudio(void);
	virtual void setLastTrackEnded(NXBOOL bSet) = 0;
};

#endif /* CNexExportWriter_h */
