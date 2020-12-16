/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoFrameWriteTask.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/12/29	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_VIDEOFRAMEWRITETASK_H__
#define __NEXVIDEOEDITOR_VIDEOFRAMEWRITETASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"

#include "nexPerMon.h"
#include "nexLock.h"

#define VIDEO_FRAME_WRITE_TASK_NAME "NEXVIDEOEDITOR Video Frame Write task"

class CNexExportWriter;
class WrapMediaSource;

typedef enum _STREAM_INPUT_BUFFER_FORMAT
{
	STREAM_INPUT_BUFFER_FORMAT_NONE = 0,
	STREAM_INPUT_BUFFER_FORMAT_NV12,
	STREAM_INPUT_BUFFER_FORMAT_Y2UVA,
	STREAM_INPUT_BUFFER_FORMAT_UVY2A,
	STREAM_INPUT_BUFFER_FORMAT_MEDIABUFFER,
    STREAM_INPUT_BUFFER_FORMAT_OPAQUE = 8
}STREAM_INPUT_BUFFER_FORMAT;

class CVideoWriteBuffer
{
public:
	CVideoWriteBuffer();
	~CVideoWriteBuffer();
	void initBuffer(int iWidth, int iHeight);
	void resetBuffer();

	STREAM_INPUT_BUFFER_FORMAT	m_eBufferFormat;
	unsigned char*	m_pY2UVA;
	unsigned char*	m_pTempBuffer;
	int				m_iWidth;
	int				m_iHeight;
	unsigned int		m_uiBufferSize;
	unsigned int		m_uiTime;
};

#define VIDEO_WRITER_BUFFER_COUNT	4
typedef std::vector<CVideoWriteBuffer*>	CVideoWriteBufferVec;


class CNEXThread_VideoFrameWriteTask : public CNEXThreadBase
{
public:
    CNEXThread_VideoFrameWriteTask( void );
	virtual ~CNEXThread_VideoFrameWriteTask( void );

	virtual const char* Name()
	{	return VIDEO_FRAME_WRITE_TASK_NAME;
	}

	virtual void End( unsigned int uiTimeout );
	virtual void WaitTask();

	NXBOOL setFileWrite(CNexExportWriter* pFileWriter);
	NXBOOL setEncoderInputFormat(unsigned int uiFormat);

	CVideoWriteBuffer* getBuffer();
	NXBOOL releaseBuffer(CVideoWriteBuffer* pBuffer);
	NXBOOL releaseOutBuffer(CVideoWriteBuffer* pBuffer);
	NXBOOL waitEmptyBuffer(int iTime);

	void dumpY2UVAFrameBuffer(CVideoWriteBuffer* pFrameBuffer);
	
protected:                                 
	virtual int OnThreadMain( void );
	NXBOOL processY2UVABuffer(CVideoWriteBuffer* pFrameBuffer);
	NXBOOL processMediaBuffer(CVideoWriteBuffer* pFrameBuffer);
    NXBOOL processBufferDefault(CVideoWriteBuffer* pFrameBuffer);


public:
	int						m_iTemp;
	CVideoWriteBuffer			m_pBuffer[VIDEO_WRITER_BUFFER_COUNT];

	CVideoWriteBufferVec		m_vecInBuffer;
	CVideoWriteBufferVec		m_vecOutBuffer;

private:
	CNexExportWriter*			m_pFileWriter;
	CNexLock				m_Lock;

	CNexPerformanceMonitor	m_perfVideoY2UV2YUV;
	CNexPerformanceMonitor	m_perfVideoRead;

	unsigned int				m_uiEncoderInputFormat;
}; // __NEXVIDEOEDITOR_VIDEOFRAMEWRITETASK_H__

#endif // __NEXVIDEOEDITOR_VIDEOTASK_H__
