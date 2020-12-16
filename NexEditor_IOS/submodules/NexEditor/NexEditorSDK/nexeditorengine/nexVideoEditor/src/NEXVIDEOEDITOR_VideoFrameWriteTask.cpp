/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoFrameWriteTask.cpp
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
#include "NEXVIDEOEDITOR_VideoFrameWriteTask.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "nexSeperatorByte_.h"
#include "NEXVIDEOEDITOR_Def.h"

#include "nexYYUV2YUV420.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"

// #define DUMP_VIDEO_FRAME

FILE* g_pVIDEOFRAMEInFile = NULL;

CVideoWriteBuffer::CVideoWriteBuffer()
{
	m_eBufferFormat	= STREAM_INPUT_BUFFER_FORMAT_NONE;
	m_pY2UVA		= NULL;
	m_pTempBuffer	= NULL;
	m_iWidth			= 0;
	m_iHeight		= 0;
	m_uiBufferSize	= 0;
	m_uiTime		= 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VideoFrameWriteTask.cpp %d] CVideoWriteBuffer", __LINE__);
}

CVideoWriteBuffer::~CVideoWriteBuffer()
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VideoFrameWriteTask.cpp %d] ~~CVideoWriteBuffer(%p)", __LINE__, m_pTempBuffer);
	if( m_pTempBuffer )
	{
		nexSAL_MemFree(m_pTempBuffer);
		m_pTempBuffer = NULL;
	}

	if( m_pY2UVA )
	{
		nexSAL_MemFree(m_pY2UVA);
		m_pY2UVA = NULL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VideoFrameWriteTask.cpp %d] ~~CVideoWriteBuffer Out", __LINE__);
}

void CVideoWriteBuffer::initBuffer(int iWidth, int iHeight)
{
	m_eBufferFormat	= STREAM_INPUT_BUFFER_FORMAT_NONE;
	m_pY2UVA		= (unsigned char*)nexSAL_MemAlloc(iWidth * iHeight*4);
	m_pTempBuffer	= (unsigned char*)nexSAL_MemAlloc(iWidth * iHeight*3/2);
	m_iWidth			= 0;
	m_iHeight		= 0;
	m_uiBufferSize	= 0;
	m_uiTime		= 0;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VideoFrameWriteTask.cpp %d] initBuffer(%d %d %p)", __LINE__, iWidth, iHeight, m_pTempBuffer);
}
	
void CVideoWriteBuffer::resetBuffer()
{
	m_eBufferFormat	= STREAM_INPUT_BUFFER_FORMAT_NONE;
	m_iWidth			= 0;
	m_iHeight		= 0;
	m_uiBufferSize	= 0;
	m_uiTime		= 0;
}

CNEXThread_VideoFrameWriteTask::CNEXThread_VideoFrameWriteTask( void ) : 
	m_perfVideoY2UV2YUV((char*)"VFWriterTask Y2UVToNV12"),
	m_perfVideoRead((char*)"VFWriterTask ReadFrameFromMedia")
		
{
	m_pFileWriter				= NULL;
	m_uiEncoderInputFormat	= NEXCAL_PROPERTY_VIDEO_BUFFER_NV12;

	m_vecInBuffer.clear();
	m_vecOutBuffer.clear();

	for( int i = 0; i<VIDEO_WRITER_BUFFER_COUNT; i++)
	{
		m_pBuffer[i].initBuffer(SUPPORTED_ENCODE_WIDTH, SUPPORTED_ENCODE_HEIGHT);
		m_vecInBuffer.insert(m_vecInBuffer.end(), &m_pBuffer[i]);
	}
}

CNEXThread_VideoFrameWriteTask::~CNEXThread_VideoFrameWriteTask( void )
{
	if( IsWorking() )
	{
		End( 100 );
	}
	SAFE_RELEASE(m_pFileWriter);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VideoFrameWriteTask.cpp %d] ~~~~CNEXThread_VideoFrameWriteTask", __LINE__);
}

void CNEXThread_VideoFrameWriteTask::End( unsigned int uiTimeout )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VideoFrameWriteTask.cpp %d] CNEXThread_VideoFrameWriteTask::End In", __LINE__);
	if( m_bIsWorking == FALSE ) return;
	if( m_hThread == NEXSAL_INVALID_HANDLE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VideoFrameWriteTask.cpp %d] End Thread handle is null", __LINE__);	
		return;
	}

	m_bIsWorking = FALSE;
	if( nexSAL_TaskWait(m_hThread) != 0 )
	{
		// nexSAL_TaskTerminate(m_hThread);
	}
	nexSAL_TaskDelete(m_hThread);
	m_hThread = NEXSAL_INVALID_HANDLE;

	if( m_hSema != NEXSAL_INVALID_HANDLE )
	{
		nexSAL_SemaphoreDelete(m_hSema);
		m_hSema = NEXSAL_INVALID_HANDLE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VideoFrameWriteTask.cpp %d] CNEXThread_VideoFrameWriteTask::End Out", __LINE__);
}

void CNEXThread_VideoFrameWriteTask::WaitTask()
{
	nexSAL_TaskWait(m_hThread);
	nexSAL_TaskDelete(m_hThread);
	m_hThread = NEXSAL_INVALID_HANDLE;
}

NXBOOL CNEXThread_VideoFrameWriteTask::setFileWrite(CNexExportWriter* pFileWriter)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VideoFrameWriteTask.cpp %d] setFileWrite(Handle : %p)", __LINE__, pFileWriter);
	CAutoLock m(m_Lock);	
	SAFE_RELEASE(m_pFileWriter);
	m_pFileWriter = pFileWriter;
	SAFE_ADDREF(m_pFileWriter);
	return TRUE;
}

NXBOOL CNEXThread_VideoFrameWriteTask::setEncoderInputFormat(unsigned int uiFormat)
{
	m_uiEncoderInputFormat = uiFormat;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VideoFrameWriteTask.cpp %d] setEncoderInputFormat(Format : 0x%x)", __LINE__, m_uiEncoderInputFormat);
	return TRUE;
}

CVideoWriteBuffer* CNEXThread_VideoFrameWriteTask::getBuffer()
{
	if( !m_vecInBuffer.empty() )
	{
		return m_vecInBuffer[0];
	}

	int uiWaitTime = 500;
	while(uiWaitTime > 0)
	{
		nexSAL_TaskSleep(5);
		if( !m_vecInBuffer.empty() )
		{
			return m_vecInBuffer[0];
		}
		uiWaitTime -= 5;
	}
	return NULL;
#if 0	
	int uiWaitTime = 500;
	while(uiWaitTime > 0)
	{
		m_Lock.Lock();
		if( m_pBuffer.m_isUsed == FALSE )
		{
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VideoFrameWriteTask.cpp %d] Write buffer Seached(Wait Time %d)", __LINE__, 300 - uiWaitTime);
			// m_Lock.Unlock();
			return &m_pBuffer;
		}
		m_Lock.Unlock();
		nexSAL_TaskSleep(1);
		uiWaitTime -= 1;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VideoFrameWriteTask.cpp %d] Not available Write buffer(%d)", __LINE__, m_pBuffer.m_isUsed);
	return NULL;
#endif	
}

NXBOOL CNEXThread_VideoFrameWriteTask::releaseBuffer(CVideoWriteBuffer* pBuffer)
{
	CAutoLock m(m_Lock);
	m_vecOutBuffer.insert(m_vecOutBuffer.end(), m_vecInBuffer[0]);
	m_vecInBuffer.erase(m_vecInBuffer.begin());
	return TRUE;
#if 0	
	// CAutoLock m(m_Lock);
	// pBuffer->m_isUsed = TRUE;
	m_Lock.Unlock();
	return TRUE;
#endif	
}

NXBOOL CNEXThread_VideoFrameWriteTask::releaseOutBuffer(CVideoWriteBuffer* pBuffer)
{
	CAutoLock m(m_Lock);
	if( pBuffer == NULL )
		return FALSE;

	pBuffer->resetBuffer();
	m_vecInBuffer.insert(m_vecInBuffer.end(), m_vecOutBuffer[0]);
	m_vecOutBuffer.erase(m_vecOutBuffer.begin());
	return TRUE;
#if 0	
	// CAutoLock m(m_Lock);
	// pBuffer->m_isUsed = TRUE;
	m_Lock.Unlock();
	return TRUE;
#endif	
}

NXBOOL CNEXThread_VideoFrameWriteTask::waitEmptyBuffer(int iTime)
{
	while(iTime > 0)
	{
		if( m_vecOutBuffer.size() <= 0 )
		{
			return TRUE;
		}
		nexSAL_TaskSleep(10);
		iTime -= 10;
	}
	return FALSE;
#if 0	
	int uiWaitTime = 1000;
	while(uiWaitTime > 0)
	{
		m_Lock.Lock();
		if( m_pBuffer.m_isUsed == FALSE )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VideoFrameWriteTask.cpp %d] Empty Buffer", __LINE__);
			m_Lock.Unlock();
			return TRUE;
		}
		m_Lock.Unlock();
		nexSAL_TaskSleep(5);
		uiWaitTime -= 5;
	}
	return FALSE;
#endif	
}

int CNEXThread_VideoFrameWriteTask::OnThreadMain( void )
{
	unsigned int uiDuration	= 0;
	unsigned int uiSize		= 0;
	m_bIsWorking 			= TRUE;

       unsigned int uiTime = 0;
       NXBOOL bEncodeEnd = 0;

	CVideoWriteBuffer*		pVideoFrameBuffer = NULL;

#ifdef DUMP_VIDEO_FRAME
	g_pVIDEOFRAMEInFile = fopen("/sdcard/H264InFrame.yuv", "wb");
#endif			

	m_perfVideoY2UV2YUV.CheckModuleStart();
	m_perfVideoRead.CheckModuleStart();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VideoFrameWriteTask.cpp %d] Frame Write Task Start(%d)", __LINE__, m_bIsWorking);
	while ( m_bIsWorking && m_pFileWriter )
	{
#ifdef USE_WRITE_TASK	
            if(m_uiEncoderInputFormat == NEXCAL_PROPERTY_VIDEO_BUFFER_NOT_MEMORY)
            {
                if( m_vecOutBuffer.empty() )
                {
                    nexSAL_TaskSleep(10);
                    continue;
                }

		pVideoFrameBuffer = m_vecOutBuffer[0];
                m_pFileWriter->setBaseVideoFrame(pVideoFrameBuffer->m_uiTime, FALSE, &uiDuration, &uiSize, &bEncodeEnd);
		releaseOutBuffer(pVideoFrameBuffer);
                continue;
            }
#endif
    
		if( m_vecOutBuffer.empty() )
		{
			nexSAL_TaskSleep(10);
			continue;
		}

		pVideoFrameBuffer = m_vecOutBuffer[0];

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VideoFrameWriteTask.cpp %d] Exist Video Frame(%d)", __LINE__, m_bIsWorking);
		if( pVideoFrameBuffer->m_eBufferFormat == STREAM_INPUT_BUFFER_FORMAT_Y2UVA )
		{
#ifdef DUMP_VIDEO_FRAME
			dumpY2UVAFrameBuffer(pVideoFrameBuffer);
#endif
			processY2UVABuffer(pVideoFrameBuffer);
		}
		else if( pVideoFrameBuffer->m_eBufferFormat == STREAM_INPUT_BUFFER_FORMAT_MEDIABUFFER )
		{
			processMediaBuffer(pVideoFrameBuffer);
		}
        else if ( pVideoFrameBuffer->m_eBufferFormat == STREAM_INPUT_BUFFER_FORMAT_OPAQUE )
        {
            processBufferDefault(pVideoFrameBuffer);
        }
		
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VideoFrameWriteTask.cpp %d] Write One Video Frame End Time(%d)", __LINE__, pVideoFrameBuffer->m_uiTime);
		releaseOutBuffer(pVideoFrameBuffer);
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VideoFrameWriteTask.cpp %d] Write Video Frame Task End m_pFileWriter(%p)", __LINE__, m_pFileWriter);

	if( g_pVIDEOFRAMEInFile )
	{
		fclose(g_pVIDEOFRAMEInFile);
		g_pVIDEOFRAMEInFile = NULL;
	}
	
	return 0;
}

NXBOOL CNEXThread_VideoFrameWriteTask::processBufferDefault(CVideoWriteBuffer* pFrameBuffer)
{
    unsigned int uiDuration = 0;
    unsigned int uiSize = 0;
    unsigned int uiBufferSize = sizeof(void*);
    
    if( m_pFileWriter->setBaseVideoFrame(pFrameBuffer->m_uiTime, pFrameBuffer->m_pY2UVA, uiBufferSize, &uiDuration, &uiSize, NULL) == FALSE )
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VideoFrameWriteTask.cpp %d] Video Frame Write failed", __LINE__);
    
    return TRUE;
}

NXBOOL CNEXThread_VideoFrameWriteTask::processY2UVABuffer(CVideoWriteBuffer* pFrameBuffer)
{
	unsigned int uiDuration = 0;
	unsigned int uiSize = 0;

	unsigned int uiBufferSize = pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight * 3 / 2;
	unsigned char* pY = pFrameBuffer->m_pTempBuffer;
	unsigned char* pUV = pFrameBuffer->m_pTempBuffer + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight);
	unsigned char* pU = pFrameBuffer->m_pTempBuffer + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight);
	unsigned char* pV = pFrameBuffer->m_pTempBuffer + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight/4));
	
	m_perfVideoY2UV2YUV.CheckModuleUnitStart();
	switch(m_uiEncoderInputFormat)
	{
		case NEXCAL_PROPERTY_VIDEO_BUFFER_NV12:
#if defined(__ARM_ARCH_7__)
			seperatorByte( pFrameBuffer->m_iWidth, pFrameBuffer->m_iHeight,  pFrameBuffer->m_pY2UVA, pY, pUV);
#else
			nexYYUVtoY2UV(pFrameBuffer->m_iWidth, pFrameBuffer->m_iHeight,  pFrameBuffer->m_pY2UVA, pY, pUV);
#endif
			break;
		case NEXCAL_PROPERTY_VIDEO_BUFFER_NV21:
	nexYYUVtoY2VU(pFrameBuffer->m_iWidth, pFrameBuffer->m_iHeight,  pFrameBuffer->m_pY2UVA, pY, pUV);
			break;
		case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YUV420:
	nexYYUVAtoYUV420(pFrameBuffer->m_iWidth, pFrameBuffer->m_iHeight, pFrameBuffer->m_pY2UVA, pY, pU, pV);
			break;
		case NEXCAL_PROPERTY_VIDEO_BUFFER_SEPARATED_YVU420:
	nexYYUVAtoYUV420(pFrameBuffer->m_iWidth, pFrameBuffer->m_iHeight, pFrameBuffer->m_pY2UVA, pY, pV, pU);
			break;
		default:
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VideoFrameWriteTask.cpp %d] Not support encode format", __LINE__);	
			break;
	};
	m_perfVideoY2UV2YUV.CheckModuleUnitEnd();
	
	if( m_pFileWriter->setBaseVideoFrame(pFrameBuffer->m_uiTime, pFrameBuffer->m_pTempBuffer, uiBufferSize, &uiDuration, &uiSize) == FALSE )
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VideoFrameWriteTask.cpp %d] Video Frame Write failed", __LINE__);
	return TRUE;
}

NXBOOL CNEXThread_VideoFrameWriteTask::processMediaBuffer(CVideoWriteBuffer* pFrameBuffer)
{
	return TRUE;
}

void CNEXThread_VideoFrameWriteTask::dumpY2UVAFrameBuffer(CVideoWriteBuffer* pFrameBuffer)
{
	if( g_pVIDEOFRAMEInFile == NULL || pFrameBuffer == NULL ) return;

	unsigned int uiBufferSize = pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight * 3 / 2;
	unsigned char* pY = pFrameBuffer->m_pTempBuffer;
	unsigned char* pUV = pFrameBuffer->m_pTempBuffer + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight);
	unsigned char* pU = pFrameBuffer->m_pTempBuffer + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight);
	unsigned char* pV = pFrameBuffer->m_pTempBuffer + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight + (pFrameBuffer->m_iWidth * pFrameBuffer->m_iHeight/4));
	
	nexYYUVAtoYUV420(pFrameBuffer->m_iWidth, pFrameBuffer->m_iHeight, pFrameBuffer->m_pY2UVA, pY, pU, pV);
			
	fwrite(pFrameBuffer->m_pTempBuffer, sizeof(char), uiBufferSize, g_pVIDEOFRAMEInFile);
			}

