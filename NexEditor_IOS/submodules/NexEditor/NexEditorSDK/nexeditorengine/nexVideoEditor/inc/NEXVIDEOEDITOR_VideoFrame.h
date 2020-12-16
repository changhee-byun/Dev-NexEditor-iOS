/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoFrame.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/12	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_VIDEOFRAME_H__
#define __NEXVIDEOEDITOR_VIDEOFRAME_H__

#include "NexSAL_Internal.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "yuv420_to_rgb16_ex.h"
#include "NexThemeRenderer.h"
#include "nexLock.h"

#include "nexPerMon.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"

#define IGNORE_RENDER_TIME				100

#define DROP_RENDER_TIME_PREVIEW		100
#define WAIT_RENDER_TIME_PREVIEW		20

#define DROP_RENDER_TIME_EXPORT		100
#define WAIT_RENDER_TIME_EXPORT		20

#define RENDER_FORCE_DISPLAY_FOR_EFFECT	50

// #define VIDEO_FRAME_DUMP

class CFrameInfo : public CClipVideoRenderInfo
{
public:
	CFrameInfo();
	~CFrameInfo();
	NXBOOL initClipRenderInfo(CClipVideoRenderInfo* pClipRenderInfo);
	NXBOOL clearClipRenderInfoVec();
	NXBOOL setClipRenderInfoVec(CClipVideoRenderInfo* pClipRenderInfo);
	
	NXBOOL setSurfaceTexture(void* pSurface);
	
	void deinitFrameInfo();
	NXBOOL setVideoFrame(unsigned int uiTime, unsigned char* pY, unsigned char* pU, unsigned char* pV);
	NXBOOL initVideoSurface(void* pDDraw, int iBitCount);
	void applyPosition(void* pSurface, unsigned int uiTime, int iIndex = -1);
	float calcPos(float fPos1, float fPos2, float fPer);
	NXBOOL drawVideo(void* pSurface, RECT* pRC, int iBitCount);
	int drawVideo(void* pSurface, int iIndex, unsigned int uiTime, NXBOOL bForceDisplay = FALSE, NXBOOL bExport = FALSE, int iTimeOut = -1);
    int drawVideo(void* pSurface, unsigned int uiTime, int iTimeOut);

	int drawVideo(void* pSurface, unsigned int uiTime, int textureID, NXBOOL bExport, NXBOOL bForced, int iTimeOut = -1);

	int checkDrawTime(unsigned int uiTime, NXBOOL bExport = FALSE);
	NXBOOL isImageClip();
	void clearTextureID();
	void updateFrameTime(unsigned int uiStartTime, unsigned uiEndTime); 
	void updateImageFrameRect(RECT rcStartRect, RECT rcEndRect);

	void dropFirstVideoTexture(void* pSurface);
	void setVideoLayerTextureID(int iTextureID);
	void setVideoDropRenderTime(unsigned int uTime);

	void setCodecWrap(CNexCodecWrap* pCodecWrap);
	#ifdef _ANDROID
	void callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender);
	#endif

	#ifdef __APPLE__
	void callCodecWrap_renderDecodedFrame(unsigned char* pBuff);
	void callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender);
	#endif
	
private:
	unsigned int getHighestPowerOfTwo( unsigned int v );
	bool uploadTexture(void* pSurface, int iTimeOut, CClipVideoRenderInfo* pRenderInfo);
	CClipVideoRenderInfo* getActiveRenderInfo(unsigned int uiTime);

public:
	unsigned int		m_uiTime;

	unsigned int		m_uiGraphicBufferPitch;
	unsigned int		m_uiGraphicBufferAlignHeight;

	unsigned char*	m_pY;
	unsigned char*	m_pU;
	unsigned char*	m_pV;

	unsigned char*	m_pMediaBuff;
	NXBOOL			m_bRenderMediaBuff;

#ifdef __APPLE__
    unsigned char*  m_pImageBuff;
#endif
	NXBOOL			m_isUsed;

	int				m_iBitCount;
	int				m_iTextureID;

	int				m_iImageFrameUploadFlag;
	int 			m_iFaceDetectFlag;
    
	void*			m_pSurfaceTexture;

	CNexLock		m_FrameLock;
	
	CNexPerformanceMonitor	m_performanceMonitor;
    CNexPerformanceMonitor	m_perfWaitFrame;

#ifdef VIDEO_FRAME_DUMP
	FILE*			m_pVIDEOFRAMEInFile;
	int				m_iDumpFrameCount;
	unsigned char*	m_pVideoFrameBuffer;
#endif
	int				m_iFeatureVersion;

	int				m_iVideoLayerTextureID;

	CNexCodecWrap*		m_pCodecWrap;
	unsigned int		m_uiDropRenderTime;
	CClipVideoRenderInfoVec		m_ClipVideoRenderInfoVec;
	
	NXBOOL	m_bPreviewThumb;

	unsigned int		m_uLogCnt;
};

#endif // __NEXVIDEOEDITOR_VIDEOFRAME_H__
