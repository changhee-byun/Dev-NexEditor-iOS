/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoTrack.h
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

#ifndef __NEXVIDEOEDITOR_VIDEOTRACK_H__
#define __NEXVIDEOEDITOR_VIDEOTRACK_H__

#include "NexSAL_Internal.h"
#include "NEXVIDEOEDITOR_VideoFrame.h"
#include "NEXVIDEOEDITOR_Clip.h"
#include "yuv420_to_rgb16_ex.h"
#include "NexThemeRenderer.h"
#include <queue>

#include "NexCAL.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NexMediaDef.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"
#include "NEXVIDEOEDITOR_FrameTimeChecker.h"

typedef std::vector<CFrameInfo*>	VideoFrameVec;

#define VIDEO_FRAME_MAX_COUNT 4

class CVideoTrackInfo : public CNxRef<INxRefObj>
{
public:
	CVideoTrackInfo();
	virtual ~CVideoTrackInfo();
	static int is360Video( unsigned char * inUUID, int inUUIDSize ); 
	static int getHDRMetaData(unsigned int inCodecType,	NXUINT8*	inDSI , NXUINT32 inDSISize 
				, NXUINT8* in1stFrame, NXUINT32 in1stFrameSize, int inNalSize 
				,NEXCODECUTIL_SEI_HDR_INFO *out);
	
	void setTrackID(unsigned int uiTrackID);

	NXBOOL setClipRenderInfo(CClipVideoRenderInfo* pClipRenderInfo);
	NXBOOL clearClipRenderInfoVec();
	NXBOOL setClipRenderInfoVec(CClipVideoRenderInfo* pClipRenderInfo);

	NXBOOL setSurfaceTexture(void* pSurface);
	void* getSurfaceTexture();

	NXBOOL isImageClip();
	NXBOOL isVideoClip();
	NXBOOL isVideoLayer();

	NXBOOL initVideoSurface(void* pDDraw, int iBitCount);
	NXBOOL setTrackIndexInfo( unsigned int uiCurrentIndex, unsigned int uiTotalClipCount);
	NXBOOL isLastTrack(unsigned int uClipTotalTime);

	NXBOOL setEffectInfo( unsigned int uiEffectStartTime, unsigned int uiEffectDuration, int iEffectOffset, int iEffectOverlap, const char* pEffect);
	NXBOOL setTitleInfo(char* pStrTitle, const char* pEffect, unsigned int uiStartTime, unsigned int uiEndTime);
	NXBOOL setFilterInfo(void* pEffect);
	NXBOOL setRotateState(int iRotate);
	NXBOOL setBrightness(int iBrightness);
	NXBOOL setContrast(int iContrast);
	NXBOOL setSaturation(int iSaturation);
    NXBOOL setHue(int iHue);
	NXBOOL setTintcolor(int iTintcolor);
	NXBOOL setLUT(int iLUT);
	NXBOOL setCustomLUTA(int iLUT);
	NXBOOL setCustomLUTB(int iLUT);
	NXBOOL setCustomLUTPower(int iLUT);
	NXBOOL setVignette(int iVignette);
	NXBOOL getVignette(int iVignette);
	NXBOOL setVideoRenerMode(int iRenderMode); //yoon

	int applyClipOptions(NXT_HThemeRenderer hThemeRenderer, NXT_TextureID eTextureID);
	int applyClipEffect(NXT_HThemeRenderer hThemeRenderer, unsigned int uiCurrentTime);
	NXBOOL checkClipEffect() { return m_bTitleEffectStart; };
	int applyTitleEffect(NXT_HThemeRenderer hThemeRenderer, unsigned int uiCurrentTime);
	void resetEffect(NXT_HThemeRenderer hThemeRenderer);
	char* getAppliedTitleEffect();

	int existDrawFrame(unsigned int uiCurrentTime, NXBOOL bWait);
	NXBOOL isTrackTime(unsigned int uiTime);
	NXBOOL isTrackStarted();
	NXBOOL isTrackEnded();
	NXBOOL isEffectApplyEnd();
	NXBOOL isFaceDetectProcessed(unsigned int uiTime);
	
	CFrameInfo* getFrameInBuffer();
	void EnqueueFrameInBuffer();
	void ReorderFrameInBuffer();
	CFrameInfo* getFrameOutBuffer(NXBOOL bCheckTime = FALSE);
	void DequeueFrameOutBuffer(CFrameInfo* pFrameInfo = NULL, unsigned int* pTime = NULL);
	void DequeueAllFrameOutBuffer(CFrameInfo* pFrameInfo = NULL, unsigned int* pTime = NULL);
	void clearImageTrackDrawFlag();
	
	void updateTrackTime(unsigned int uiStartTime, unsigned uiEndTime, unsigned int uiStartTrimTime, unsigned int uiEndTrimTime);
	void updateImageTrackRect(RECT rcStartRect, RECT rcEndRect);

	void setMotionTracked(NXBOOL tracked);
	NXBOOL getMotionTracked();

	void setReUseSurfaceTexture(NXBOOL bReUse);

	void printTrackInfo();
	void printMatrixInfo(float* pMatrix);

	NXBOOL isReaderSeted();
	NXBOOL setSourceHandle(CClipItem* pClip, CNexSource* pSource);
	CNexSource* getSourceHandle();
	
	NXBOOL waitFinishTrackFrame(int iTime);
	NXBOOL setPreviewTime(CClipItem* pClip, int iIDRFrame, unsigned int uiTime);
	NXBOOL setPreviewThumb(unsigned char* pY, unsigned int uiTime);
	
	void resetVideoDecoder();

	char* getEffectID();
	int	setEffectID(const char* pEffect);
	char* getTitleEffectID();
	int 	setTitleEffectID(const char* pTitleEffect);

	void setCodecWrap(CNexCodecWrap* pCodec);
#ifdef _ANDROID
	void callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender);
#endif

#ifdef __APPLE__
	void callCodecWrap_renderDecodedFrame(unsigned char* pBuff);
	void callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender);
#endif

	// + mantis 7722
	void setVideoDecodeEnd(NXBOOL bEnd = TRUE);
	NXBOOL getVideoDecodeEnd();
	// - mantis 7722

	// RYU : for mantis 9527
	void setVideoRenderEnd(NXBOOL bEnd = TRUE);

	void setFirstVideoFrameDrop(void* pSurface);
	void setVideoLayerTextureID(int iTextureID);
	void setVideoDropRenderTime(unsigned int uTime);

	unsigned int getDropVideoCount();
	NXBOOL setPause();
	NXBOOL setResume();
	CClipVideoRenderInfo* getActiveRenderInfo(unsigned int uiTime);
	void applyPosition(void* pSurface, unsigned int uiTime, int iIndex = -1);
	float calcPos(float fPos1, float fPos2, float fPer);

	void setDrawInfos(CNexDrawInfoVec* pInfos);
	void applyDrawInfos(void* pRender, unsigned int uiTime);
	void resetDrawInfos(void* pRender);

	NXBOOL isEOSFlag(unsigned int uiFlag);

	unsigned int		m_uiTrackID;

	unsigned int		m_uiCodecUserData;
	NXBOOL				m_bTrackStart;

	int					m_iWidth;
	int					m_iHeight;
	int					m_iPitch;
	CFrameInfo			m_FrameInfo[VIDEO_FRAME_MAX_COUNT];
	int					m_iFrameInfoCount;

	CNexLock			m_FrameQueueLock;

	VideoFrameVec		m_FrameInVec;
	VideoFrameVec		m_FrameOutVec;

	CLIP_TYPE			m_ClipType;

	unsigned int		m_uiStartTime;
	unsigned int		m_uiEndTime;

	unsigned int		m_uiEffectStartTime;
	unsigned int		m_uiEffectDuration;
	int 				m_iEffectOffect;
	int 				m_iEffectOverlap;
	NXBOOL				m_bClipEffectStart;	
	char*				m_pEffectID;

	NXBOOL				m_bEffectApplyEnd;
	
	char*				m_pTitle;
	char*				m_pTitleEffectID;
	NXBOOL				m_bTitleEffectStart;
	unsigned int		m_uiTitleStartTime;
	unsigned int		m_uiTitleEndTime;

	void*				m_pFilterEffect;

	float				m_StartMatrix[MATRIX_MAX_COUNT];
	float				m_EndMatrix[MATRIX_MAX_COUNT];

	int					m_iRotateState;

	int					m_iBrightness;
	int					m_iContrast;
	int					m_iSaturation;
    int                 m_iHue;
	int					m_iTintcolor;
	int   				m_iLUT;
	int   				m_iCustomLUT_A;
	int   				m_iCustomLUT_B;
	int   				m_iCustomLUT_Power;
	int 				m_iVignette;
	
	unsigned int		m_uiTotalClipCount;
	unsigned int		m_uiCurrentClipIndex;

	CNexSource*		m_pSource;
	NXBOOL				m_isNextFrameRead;
	CNexCodecWrap*		m_pCodecWrap;

	CFrameTimeChecker	m_FrameTimeChecker;

	CNexLock			m_CallbackLock;

	void*				m_pSurfaceTexture;

	NXBOOL				m_bDecodingEnd;
	NXBOOL				m_bRenderingEnd;
	NXBOOL				m_bFirstFrameFlag;
	int					m_iFirstVideoFrameCounter;
	int					m_iVideoLayerTextureID;
	int					m_iDrop;
	int					m_iSkipVideoStartGap;

	unsigned int		m_iDropCount;
	unsigned int		m_iDropStartTime;
	unsigned int		m_iDropLastCount;

	CFrameTimeChecker	m_FrameTimeChecker4Pause;
	unsigned int		m_uiLastEncodedFrameCTS;
	int					m_iVideoRenderMode; // 0 - common, 1- 360 video yoon
	int					m_iHDRType; //yoon
	NEXCODECUTIL_SEI_HDR_INFO	m_stHdrInfo; //yoon
	CClipVideoRenderInfoVec 	m_ClipVideoRenderInfoVec;
	NXBOOL						m_bFirstDisplay4Layer;

	CNexDrawInfoVec*			m_pDrawInfos;
	int							m_iTrackTextureID;
	int 						m_iFeatureVersion;
	NXBOOL						m_isMotionTracked;
	unsigned int				m_uLogCnt;
	int							m_iLoadedType;	// 1 - Original, 2 - Thumbnail
	NXBOOL						m_bReUseSurfaceTexture;

	NXBOOL				m_bTrackPaused;	
};

typedef std::vector<CVideoTrackInfo*>	VideoTrackVec;
typedef VideoTrackVec::iterator			VideoTrackVecIter;

#endif // __NEXVIDEOEDITOR_VIDEOTRACK_H__
