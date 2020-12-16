/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_PFrameProcessTask.h
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#ifndef __NEXVIDEOEDITOR_PFRAMEPROCESSTASK_H__
#define __NEXVIDEOEDITOR_PFRAMEPROCESSTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"
#include "NEXVIDEOEDITOR_CalcTime.h"
#include "NexThemeRenderer.h"
#ifdef _ANDROID
#include "NexThemeRenderer_SurfaceTexture.h"
#endif
#include "NexCAL.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NxVScaler.h"
#include <vector>
#include "NxResamplerAPI.h"
#include "NEXVIDEOEDITOR_HighlightDiffChecker.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"

#define PFRAMEPROCESS_TASK_NAME "NEXVIDEOEDITOR PFrameProcess Task"

class CNexProjectManager;
class CNEXThread_PFrameProcessTask : public CNEXThreadBase
{
public:
	CNEXThread_PFrameProcessTask( void );
	virtual ~CNEXThread_PFrameProcessTask( void );

	virtual const char* Name()
	{
		return PFRAMEPROCESS_TASK_NAME;
	}

	virtual void End( unsigned int uiTimeout );
	virtual void WaitTask();
	
	NXBOOL setSource(CNexSource* pSource);
	NXBOOL setFileWriter(CNexExportWriter* pFileWriter);
	NXBOOL setClipItem(CClipItem* pClip);
	NXBOOL setUserData(char* pUserData);

	void setPFrameAndAudioMode(NXBOOL bFlag);
	void setCheckEncoderDSIMode(NXBOOL bCheck);
	int getEncoderDSISize();
	unsigned char* getEncoderDSI();

	NXBOOL isEndDirectExport();

private :
	NEXVIDEOEDITOR_ERROR initVideoDecoder();
	NEXVIDEOEDITOR_ERROR deinitVideoDecoder();

	NEXVIDEOEDITOR_ERROR initVideoEncoder();
	NEXVIDEOEDITOR_ERROR deinitVideoEncoder();

	NEXVIDEOEDITOR_ERROR initRenderer();
	NEXVIDEOEDITOR_ERROR deinitRenderer();

	NXBOOL convertFrame(unsigned char* pSrc, unsigned int uiSize, unsigned char** ppDst, unsigned int* pDstSize);
	NXBOOL checkDSI_ClipAndEncoder(unsigned char* pDSI, int iSize);

	NEXVIDEOEDITOR_ERROR writeOneFrameWithEncode(NXBOOL bEnd, unsigned int uiCTS);
	NEXVIDEOEDITOR_ERROR writeOneFrameWithEncode2(NXBOOL bEnd, unsigned int uiCTS);

protected:
	virtual int OnThreadMain( void );
	NEXVIDEOEDITOR_ERROR processPFrameProcess();
	NEXVIDEOEDITOR_ERROR processPFrameAndAudio();
	NEXVIDEOEDITOR_ERROR prodessEncoderDSI();

private:
	CNexSource*		m_pSource;
	CNexExportWriter*		m_pFileWriter;
	CClipItem*			m_pClipItem;
	
	CNexCodecWrap*		m_pCodecWrap;
	NXBOOL				m_bFirstOutputDrop;

	unsigned int		m_uiCheckVideoDecInit;

	NXINT32				m_iSrcWidth;
	NXINT32				m_iSrcHeight;
	NXINT32				m_iSrcPitch;
	
	NXINT32				m_iVideoBitrate;
	NXINT32				m_iVideoFramerate;
	NXINT32				m_iVideoNalHeaderSize;

	NXBOOL				m_isNextVideoFrameRead;

	NXUINT32			m_uiClipID;

	unsigned int		m_uiStartTime;
	unsigned int		m_uiEndTime;
	unsigned int		m_uiBaseTime;
	unsigned int		m_uiStartTrimTime;
	int					m_iSpeedFactor;
	
	NXUINT32			m_uiDstTime;
	
	NEXCALCodecHandle	m_hVideoEnc;
	void*				m_MediaCodecInputSurf;
	NXINT64				m_uiMediaCodecUserData;
	setMediaCodecsetTimeStamp	m_fnSetMediaCodecTimeStamp;
	resetMediaCodecVideoEncoder	m_fnResetMediaCodecVideoEncoder;

	NXBOOL				m_bOutputDSI;
	int					m_iEncodeFrameDSINALSize;
	NXBOOL				m_bPFrameEncodeDone;
	unsigned int		m_uiNextIDRTime;
	
	unsigned char*		m_pRawBuffer;
	
	NXT_HThemeRenderer	m_hRenderer;
	void*				m_pDecSurface;
	void*				m_pOutputSurface;
	
	NXBOOL				m_bUserCancel;

	char*				m_pUserData;

	CCalcTime			m_TimeCalc;

	NXBOOL				m_bCheckEncoderDSI;
	unsigned char*		m_pEncoderDSI;
	int					m_iEncoderDSISize;

	NXBOOL				m_bPFrameAndAudio;
	NXBOOL				m_bDirectExportEnd;

	std::vector<NXUINT32>	m_vDTS;
};

#endif // __NEXVIDEOEDITOR_PFRAMEPROCESSTASK_H__

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2012/09/19	Draft.
-----------------------------------------------------------------------------*/
