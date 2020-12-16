/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_GL_VideoRenderTask.h
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

#ifndef __NEXVIDEOEDITOR_GL_VIDEORENDERTASK_H__
#define __NEXVIDEOEDITOR_GL_VIDEORENDERTASK_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_VideoFrame.h"
#include "NEXVIDEOEDITOR_VideoTrack.h"
#include "yuv420_to_rgb16_ex.h"
#include "NexThemeRenderer.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_EffectItemVec.h"
#include "NEXVIDEOEDITOR_PlaybackCacheWorker.h"
#include "NEXVIDEOEDITOR_DrawInfoVec.h"

#include "nexPerMon.h"

#define VIDEO_RENDER_TASK_NAME "NEXVIDEOEDITOR GL Video Renader task"

class CNexProjectManager;
class WrapMediaSource;

typedef struct _SURFACEITEM
{
	void* 	m_pSurface;
	int		m_iUsed;
	void*	m_pRenderer;
}SURFACEITEM;

#define MAX_SURFACEITEM_COUNT	10

#include <map>


class CNEXThread_VideoRenderTask : public CNEXThreadBase
{

    typedef std::map<unsigned int, NXT_HThemeRenderer> UIDRenderMapper_t;
    UIDRenderMapper_t m_uidRenderMapper;
    void recordUIDRenderPair(unsigned int uid, NXT_HThemeRenderer renderer);
    void removeUIDRenderPair(unsigned int uid);
    NXT_HThemeRenderer getRendererForUID(unsigned int uid, bool remove);
public:
	CNEXThread_VideoRenderTask( void );
	virtual ~CNEXThread_VideoRenderTask( void );

	virtual const char* Name()
	{	return VIDEO_RENDER_TASK_NAME;
	}

	int	setThemeRender(void* pRender);
	int	setExportThemeRender(void* pRender);
    int setMapper(std::map<int, int>* pmapper);

	NXBOOL isLostNativeWindow();
	NXBOOL prepareSurface(void* pSurface, float fScaleFactor);

	void useSufaceTextureCache(NXBOOL bUse);
	void* getSurfaceTexture();
	void releaseSurfaceTexture(void* pSurface);
	void clearSurfaceTexture();

	void destroySurfaceTexture(void*pRender, void* pSurface);

	void setSeparateEffect(NXBOOL bSeparateEffect);
    void setVisualCountAtTime(int iVisualClip, int iVisualLayer);
	void setIsGLOperationAllowed(bool allowed);

    unsigned int getLastTime()
	{
		return m_uiLastTime;
	}

protected:
	void validCheckTrack(unsigned int uiTime);

	NXBOOL existDrawItem(unsigned int uiTime);
	int waitDrawItem(unsigned int uiTime);
	
    virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
    virtual void ProcessIdleState();
    virtual void ProcessPlayState();
    virtual void ProcessRecordState();
    virtual void ProcessPauseState();

private:
	void capture();
	void drawClear();
	int drawIdle(unsigned int uiTime, int iDisplay = 1);
	void drawEffect(unsigned int uiTime);
	void drawPlay(unsigned int uiTime, int iDisplay = 1);
	unsigned int drawPlayLayer(unsigned int uiTime);
	void drawExport(unsigned int uiTime);
	unsigned int drawExportLayer(unsigned int uiTime);

	void fastOptionPreview(char* pOption, int iDisplay = 1);
	
	int setRendererPosition(int iLeft, int iTop, int iRight, int iBottom);
	int writeVideoYUVFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize);

	int m_VideoProcessOk;
	void checkVideoProcessOk();
	void checkVideoProcessOk(unsigned int uiTime);
	void sendVideoStarted();
	// mantis 6038 : send the error message when initialize failed
	void sendInterruptExportEvent(NEXVIDEOEDITOR_ERROR error = NEXVIDEOEDITOR_ERROR_EXPORT_VIDEO_RENDER_INIT_FAIL);

	void clearGLRender();
	void clearUploadTexture(int iTextureID, int iWidth, int iHeight, NXBOOL bAquireContext = FALSE);
	int applyEffect(CVideoTrackInfo* pTrackInfo, unsigned int uiTime);

	NXT_HThemeRenderer getCurrentVideoRenderer();
	void updateEffectItemVec();
	
	void dropFirstFrameOnTrack();

    NXBOOL prepareSurface_internal(void* pSurface, float fScaleFactor);
    void releaseSurfaceTexture_internal(void* pSurface);
    void* createSurfaceTexture(void* pRender);
    void setSeparateEffect_internal(NXBOOL bSeparateEffect);
    void setVisualCountAtTime_internal(int iVisualClip, int iVisualLayer);
    void setIsGLOperationAllowed_internal(bool allowed);
    void destroySurfaceTexture_internal(void*pRender, void* pSurface);
    void* getSurfaceTexture_internal();

private:
	int						m_iTmp;
	VideoTrackVec			m_VideoTrack;
	VideoTrackVec			m_VideoLayer;

	NXBOOL					m_bRenderPause;
	NXBOOL					m_bLostNativeWindow;

    int						m_iVideoStarted;
	int						m_iFirstVideoFrameUploaded;

	RECT					m_RD;
	RECT					m_RS;
	
	unsigned int			m_uiLastTime;
	NXT_HThemeRenderer		m_hThemeRenderer;
	NXT_HThemeRenderer		m_hExportThemeRenderer;

	NXBOOL					m_isDrawEnd;

	CNexExportWriter*			m_pFileWriter;

	unsigned int			m_uiEncoderFrameSize;
	unsigned char*			m_pEncoderFrameBuffer;
	
	CNexPerformanceMonitor	m_perfNativeRender;
	CNexPerformanceMonitor	m_perfGlDraw;
	CNexPerformanceMonitor	m_perfGetPixel;

	NXBOOL					m_isAfterTransition;

	CNexProjectManager*		m_pProjectMng;

	int						m_iPreinitEnd;
	NXBOOL					m_bClearRenderer;
	NXBOOL					m_bUpdateCurrentTime;

	int						m_iEncodeWidth;
	int						m_iEncodeHeight;
	NXBOOL					m_bInterruptExport;

	unsigned int				m_uiRenderCallTime;
	unsigned int				m_uiVideoEncOutputFrameCount;
	unsigned int				m_uiVideoExportFrameCount;
	unsigned int				m_uiVideoExportTotalCount;
	unsigned int				m_uiVideoExportInterval;

	NXBOOL					m_bUseSurfaceCache;
	SURFACEITEM				m_SurfaceVec[MAX_SURFACEITEM_COUNT];

	void*					m_pMediaCodecInputSurf;

	CNxMsgSetProjectEffect*			m_pProjectEffect;

	CVideoEffectItemVec*			m_pEffectItemVec;
	NXBOOL						m_bLastClipRendered;
	NXBOOL						m_bSeparateEffect;
	unsigned int				m_uiLastDrawTick;
	unsigned int				m_uiLastSetTime;

	VideoTrackVec				m_LayerItemVec;
    
    int                         m_iVisualClipCountAtStartTime;
    int                         m_iVisualLayerCountAtStartTime;

    NXBOOL						m_bNeedRenderFinish;
	NXBOOL						m_bSkipPrecacheEffect;
	NXBOOL 						m_bSkipPrecacheEffectOnPrepare;
	NXBOOL						m_bAvailableCapture;

	int							m_iSurfaceWidth;
	int							m_iSurfaceHeight;

	CNexDrawInfoVec*			m_pDrawInfos;
	unsigned int							m_iPreviewFPS;
	unsigned int							m_iPreviewTime;

	unsigned int							m_bWaitForVisual;

    CNEXThread_PlaybackCacheWorker* m_pPlaybackCacheWorker;
    
	NXBOOL  m_isGLOperationAllowed;

};

#endif // __NEXVIDEOEDITOR_GL_VIDEORENDERTASK_H__
