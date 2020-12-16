/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ProjectManager.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/05	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_PROJECTMANAGER_H__
#define __NEXVIDEOEDITOR_PROJECTMANAGER_H__

#include "NEXVIDEOEDITOR_Interface.h"
#include "NEXVIDEOEDITOR_ClipList.h"
#include "NEXVIDEOEDITOR_MsgDef.h"
#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_PlayClipTask.h"
#include "NEXVIDEOEDITOR_VideoTrack.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_ThumbnailTask.h"
#include "NEXVIDEOEDITOR_TranscodingTask.h"
#include "NEXVIDEOEDITOR_SetTimeTask.h"
#include "NEXVIDEOEDITOR_FastPreviewTask.h"

#include "NEXVIDEOEDITOR_Android_AudioRenderTask.h"
#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"

#include "NEXVIDEOEDITOR_CodecCacheInfo.h"

#define PROJECTMANAGER_TASK_NAME "NEXVIDEOEDITOR Project Manager task"

typedef std::vector<CNEXThread_ThumbnailTask*> ThumbnailTaskVec;
typedef ThumbnailTaskVec::iterator ThumbnailTaskIterator;

class CNexVideoEditor;

class CNexProjectManager : public CNEXThreadBase
{
public:
	int m_iTemp;
	
    CNexProjectManager( void );
    CNexProjectManager( CNexVideoEditor* pVE, CNexExportWriter *pWriter = NULL );
	virtual ~CNexProjectManager( void );

	virtual const char* Name()
	{	return PROJECTMANAGER_TASK_NAME;
	}

	static CNexProjectManager* getProjectManager();
	static CNEXThread_VideoRenderTask* getVideoRenderer();
	static CNEXThread_AudioRenderTask* getAudioRenderer();
	static PLAY_STATE getCurrentState();
	static unsigned int getCurrentTimeStamp();
	CClipList* getClipList();
	CVideoEffectItemVec* getEffectItemVec();
	int getExportSamplingRate();
	int getPathHash( char * path );
	NXBOOL getFileName(char* pFilePath, char* pFileName);
	int getTotalAudioVolumeWhilePlay(int* pMasterVolume, int* pSlaveVolume);

	unsigned int setThemeRenderer(void* pRender);
	unsigned int setExportThemeRenderer(void* pRender);
	unsigned int getDuration();

	CVideoTrackInfo* getCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, unsigned int uiClipID);
	void setCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pTrack);
	void clearCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pTrack);
	void clearCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType);

	void clearCacheTranscodingDecSurf();
	void setCacheTranscodingDecSurf(void * pRender, void* pDecSurface);

	int getCachedVisualCodecSize();
	CCodecCacheInfo* getCachedVisualCodecInfo(unsigned int uIndex);
	void setCachedVisualCodecInfo(CCodecCacheInfo* pCodec);
	void clearCachedVisualCodecInfo(CCodecCacheInfo* pCodec);
	void clearCachedVisualCodecInfo();

	CNexFileReader* getCachedFileReaderInfo(unsigned int uiClipID);
	void setCachedFileReaderInfo(CNexFileReader* pReader);
	void clearCachedFileReaderInfo();

	static void sendEvent(unsigned int uiEventType, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0, unsigned int uiParam3 = 0, unsigned int uiParam4 = 0);

	NXBOOL getPlayEnd();
	unsigned int getAudioSessionID();
	static int getEncodeToTranscode();
    void closePreExecute(); //yoon

	CNEXThread_PlayClipTask* getPlayClipTask();
	
protected:
	// virtual int OnThreadMain( void );
	virtual int ProcessCommandMsg( CNxMsgInfo* pMsg );
	virtual void ProcessIdleState();
	virtual void ProcessPlayState();
	virtual void ProcessRecordState();
	virtual unsigned int getCurrentTime();

	NXBOOL changeState(PLAY_STATE eState, int iFlag);

	int closeProject();

	int addVisualClip(unsigned int uiNextToClipID, char* pFile, int iNewClipID);
	int addAudioClip(unsigned int uiStartTime, char* pFile, int iNewClipID);
	int deleteClip(int iClipID);

	int moveVisualClip(unsigned int uiNextToClipID, unsigned int uiMovedClipID);
	int moveAudioClip(unsigned int uiMoveTime, unsigned int uiMovedClipID);

private:
	CClipItem* createClip(int iNewClipID, char* pFile, NXBOOL bAudio = FALSE);

	NXBOOL						startAudioThumbTask(CNxMsgBackgroundGetInfo* pGetInfo);
	NXBOOL						endAudioThumbTask(CNxMsgBackgroundGetInfo* pGetInfo);
	NXBOOL						stopAudioThumbTask(CNxMsgBackgroundGetInfoStop* pGetInfo);
	

public:
	int							m_iTemp2;
	CClipList*					m_pClipList;
	
	static CNexProjectManager*	m_pProjectManager;

	CNEXThread_AudioRenderTask*	m_pAudioRender;
	CNEXThread_VideoRenderTask*	m_pVideoRender;
	CNEXThread_PlayClipTask*	m_pPlayClipTask;
	
	CNEXThread_ThumbnailTask*	m_pThumbnailTask;
	ThumbnailTaskVec		m_ThumbAudioVec;

	#ifdef _ANDROID
	CNEXThread_TranscodingTask*	m_pTranscodingTask;
	CNEXThread_FastPreviewTask*	m_pFastPreviewTask;
	#endif

	CNEXThread_SetTimeTask*		m_pSetTimeTask;

private:
	CNexVideoEditor*				m_pVideoEditor;

	CNexExportWriter*				m_pFileWriter;

	unsigned int					m_uiLastNotiTime;

	int							m_iRenderLeft;
	int							m_iRenderTop;
	int							m_iRenderRight;
	int							m_iRenderBottom;

	NXBOOL						m_bPlayEnd;
	NXBOOL						m_bMuteAudio;
	NXBOOL						m_bManualVolumeControl;
	
	CNexLock					m_CachedLock;
	VideoTrackVec				m_ImageTrackCache;
	VideoTrackVec				m_VideoTrackCache;
	vecCodecCacheInfo			m_VideoCodecCache;
	
	CNexLock					m_FileReaderCachedLock;

	CNexCodecManager			m_CodecManager;
	unsigned int					m_uiExportingTime;
	long							m_lMaxFileDuration;

	#ifdef _ANDROID
	void*						m_pTranscodingRenderer;
	void*						m_pTranscodingDecSurf;
	#endif

	int							m_iProjectVolume;
	int							m_iProjectVolumeFadeInTime;
	int							m_iProjectVolumeFadeOutTime;
	int							m_iMasterVolume;
	int							m_iSlaveVolume;

	unsigned int				m_uLastTickCount;
	unsigned int				m_uCurrentTickTime;
	unsigned int				m_uAudioStartTime;

	NXBOOL						m_RecordToIdle;
	int							m_iExportSamplingRate;
	int							m_iExportAudioBitRate;

	int							m_iEncodeToTranscode;

	CNexLock					m_LockClipList;
	NXBOOL						m_isGIFMode;
};


#endif // __NEXVIDEOEDITOR_PROJECTMANAGER_H__
