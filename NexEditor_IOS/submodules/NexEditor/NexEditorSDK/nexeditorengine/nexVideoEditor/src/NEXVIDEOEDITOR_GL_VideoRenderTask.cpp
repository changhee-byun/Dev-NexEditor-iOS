/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_GL_VideoRenderTask.h
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

// #define CHECK_RENDER_FRAME

#ifdef CHECK_RENDER_FRAME
int g_bCheckFrame = 1;
// #include "GLES\gl.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include "NEXVIDEOEDITOR_GL_VideoRenderTask.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_VideoEditor.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "nexSeperatorByte_.h"
#ifdef ANDROID
#include "NexThemeRenderer_SurfaceTexture.h"
#include <EGL/egl.h>
#endif
#ifdef __APPLE__
#include "NexThemeRenderer_Platform_iOS-CAPI.h"
#endif
//#include "system/window.h"

//#include <system/window.h>
#include "nxtRenderItem.h"

#define EGL_PIXEL_FORMAT_YV12   0x32315659 // YCrCb 4:2:0 Planar

#define EXPORT_TIME_INTERVAL	3400
// #define EXPORT_TIME_INTERVAL	4000


// #define EXPORT_DIRECT_TEST

// #define DUMP_VIDEOFRAME

// #define NO_OVERLAPPED_TRANSITION_CODE

#ifdef DUMP_VIDEOFRAME
#include "nexYYUV2YUV420.h"
FILE*			g_pFrameFile		= NULL;
unsigned char*	g_pFrameBuffers	= NULL;
unsigned int	g_uiFrameCount		= 0;
#endif


CNEXThread_VideoRenderTask::CNEXThread_VideoRenderTask( void ) :
	m_perfNativeRender((char*)"VRTask perf(nativeRender)"), 
	m_perfGlDraw((char*)"VRTask perf(gl draw)"), 
	m_perfGetPixel((char*)"VRTask perf(getPixel)")
{
	m_iVideoStarted			= 0;
	m_VideoProcessOk 		= 0;
	m_iFirstVideoFrameUploaded	= 0;
	m_uiLastTime				= -1;
	m_hThemeRenderer		= NULL;
	m_hExportThemeRenderer	= NULL;
	m_pFileWriter			= NULL;
	m_VideoTrack.clear();
	m_VideoLayer.clear();

	m_isDrawEnd			= TRUE;

	m_uiEncoderFrameSize	= SUPPORTED_ENCODE_WIDTH * SUPPORTED_ENCODE_HEIGHT * 3 / 2;
	// m_pEncoderFrameBuffer = (unsigned char*)nexSAL_MemAlloc(m_uiEncoderFrameSize );
	m_pEncoderFrameBuffer = NULL;

	m_pProjectMng		= CNexProjectManager::getProjectManager();
	m_isAfterTransition		= FALSE;

	m_iPreinitEnd			= 0;

	// capture black image when first capture after first time excute,  mantis 7208
	m_bClearRenderer		= FALSE;
	m_bUpdateCurrentTime	= FALSE;
	m_uiRenderCallTime	= 0;

	m_iEncodeWidth		= 0;
	m_iEncodeHeight		= 0;
	m_bInterruptExport	= FALSE;

	m_bRenderPause		= FALSE;
#ifdef _ANDROID
	m_bLostNativeWindow	= TRUE;
#elif defined __APPLE__
	m_bLostNativeWindow = FALSE;
#endif
	m_bUseSurfaceCache	= TRUE;
	memset(m_SurfaceVec, 0x00, sizeof(SURFACEITEM)*MAX_SURFACEITEM_COUNT);

	m_pMediaCodecInputSurf = NULL;

	// ZIRA 1906
	m_uiVideoExportTotalCount	= 0;

	m_pProjectEffect			= NULL;
	m_pEffectItemVec			= NULL;
	m_bLastClipRendered		= FALSE;
	m_bSeparateEffect			= FALSE;

	m_uiVideoExportInterval	= 100000 / EDITOR_DEFAULT_FRAME_RATE;

	m_LayerItemVec.clear();
    
    m_iVisualClipCountAtStartTime   = -1;
    m_iVisualLayerCountAtStartTime  = -1;

    m_bNeedRenderFinish = FALSE;
	m_bSkipPrecacheEffect = FALSE;
    m_bSkipPrecacheEffectOnPrepare = TRUE;

	m_iPreviewFPS = 0;
	
	m_isGLOperationAllowed = TRUE;
	
	CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
	if( pVideoEditor != NULL )
	{
		m_bNeedRenderFinish = pVideoEditor->getPropertyBoolean("WaitGLRenderFinish", FALSE);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] WaitGLRenderFinish(%d)", __LINE__, m_bNeedRenderFinish);

		m_bSkipPrecacheEffect = pVideoEditor->getPropertyBoolean("skipPrefetchEffect", FALSE);
        m_bSkipPrecacheEffectOnPrepare = pVideoEditor->getPropertyBoolean("skipPrefetchEffectOnPrepare", TRUE);

		m_iPreviewFPS = pVideoEditor->getPropertyInt("PreviewFPS", 0);
		m_iPreviewTime = 0;

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] skipPrefetchEffect(%d)", __LINE__, m_bSkipPrecacheEffect);
		SAFE_RELEASE(pVideoEditor);
	}

	m_iSurfaceWidth = 0;
	m_iSurfaceHeight = 0;

	m_pDrawInfos = NULL;
	m_bWaitForVisual = 0;

    m_pPlaybackCacheWorker = new CNEXThread_PlaybackCacheWorker();
	m_bAvailableCapture = FALSE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] CNEXThread_VideoRenderTask", __LINE__);
}

CNEXThread_VideoRenderTask::~CNEXThread_VideoRenderTask( void )
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] ~~~~CNEXThread_VideoRenderTask In(%p)", __LINE__, this);

    if(m_pPlaybackCacheWorker){

        m_pPlaybackCacheWorker->stopCaching();
        m_pPlaybackCacheWorker->End(1000);
        SAFE_RELEASE(m_pPlaybackCacheWorker);
    }
	for(int i = 0; i < (int)m_VideoTrack.size(); i++)
	{
		m_VideoTrack[i]->Release();
	}
	m_VideoTrack.clear();

	for( int i = 0; i <(int)m_VideoLayer.size(); i++)
	{
		m_VideoLayer[i]->Release();
	}
	m_VideoLayer.clear();

	clearSurfaceTexture();

	if( m_hThemeRenderer )
	{
		NXT_ThemeRenderer_ClearTransitionEffect(m_hThemeRenderer);
		NXT_ThemeRenderer_ClearClipEffect(m_hThemeRenderer);
		
		m_hThemeRenderer = NULL;
	}

	if( m_hExportThemeRenderer )
	{
		NXT_ThemeRenderer_ClearTransitionEffect(m_hExportThemeRenderer);
		NXT_ThemeRenderer_ClearClipEffect(m_hExportThemeRenderer);
		m_hExportThemeRenderer = NULL;
	}
	
	SAFE_RELEASE(m_pFileWriter);
	m_perfNativeRender.CheckModuleStart();
	m_perfGlDraw.CheckModuleLog();
	m_perfGetPixel.CheckModuleLog();

	if( m_pEncoderFrameBuffer )
	{
		nexSAL_MemFree(m_pEncoderFrameBuffer);
		m_pEncoderFrameBuffer = NULL;
	}

	SAFE_RELEASE(m_pProjectMng);

#ifdef DUMP_VIDEOFRAME
	if( g_pFrameFile )
	{
		fclose(g_pFrameFile);
		g_pFrameFile = NULL;
	}

	if( g_pFrameBuffers )
	{
		nexSAL_MemFree( g_pFrameBuffers );
		g_pFrameBuffers = NULL;
	}
	g_uiFrameCount = 0;
#endif

	SAFE_RELEASE(m_pDrawInfos);
	SAFE_RELEASE(m_pProjectEffect);
	SAFE_RELEASE(m_pEffectItemVec);
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] ~~~~CNEXThread_VideoRenderTask Out", __LINE__);
}

NXT_HThemeRenderer CNEXThread_VideoRenderTask::getCurrentVideoRenderer()
{
	if( m_pFileWriter != NULL  && (m_eThreadState == PLAY_STATE_RECORD || m_eThreadState == PLAY_STATE_PAUSE) )
		return m_hExportThemeRenderer;
	return m_hThemeRenderer;
}

void CNEXThread_VideoRenderTask::updateEffectItemVec()
{
	if( m_bSeparateEffect && m_pProjectMng )
	{
		SAFE_RELEASE(m_pEffectItemVec);
		m_pEffectItemVec = m_pProjectMng->getEffectItemVec();
		
		if( m_pEffectItemVec != NULL ){

            m_pEffectItemVec->lock();
			m_pEffectItemVec->clearEffectFlag();
            m_pEffectItemVec->unlock();
        }
	}
}

// ZIRA 1905
void CNEXThread_VideoRenderTask::dropFirstFrameOnTrack()
{
	for( int i = 0;  i < m_VideoTrack.size(); i++)
	{
		m_VideoTrack[i]->setFirstVideoFrameDrop((void*)getCurrentVideoRenderer());
	}
}

int CNEXThread_VideoRenderTask::ProcessCommandMsg( CNxMsgInfo* pMsg )
{
	switch(pMsg->m_nMsgType)
	{
        case MESSAGE_CHECKISLOSTNATIVEWINDOW:
        {
            CNxMsgCheckIsLostNativeWindow* msg = (CNxMsgCheckIsLostNativeWindow*)pMsg;
            msg->m_Result = m_bLostNativeWindow;
            msg->setProcessDone();
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_SETGLOPERATIONALLOWED:
        {
            CNxMsgSetGLOperationAllowed* msg = (CNxMsgSetGLOperationAllowed*)pMsg;
            setIsGLOperationAllowed_internal(msg->m_Flag);
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        } 
        case MESSAGE_SETVISUALCOUNTATTIME:
        {
            CNxMsgSetVisualCountAtTime* msg = (CNxMsgSetVisualCountAtTime*)pMsg;
            setVisualCountAtTime_internal(msg->m_iVisualClip, msg->m_iVisualLayer);
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_SETSEPARATEEFFECT:
        {
            CNxMsgSetSaparateEffect* msg = (CNxMsgSetSaparateEffect*)pMsg;
            setSeparateEffect_internal(msg->m_Flag);
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_DESTROYSURFACETEXTURE:
        {
            CNxMsgDestroySurfaceTexture* msg = (CNxMsgDestroySurfaceTexture*)pMsg;
            destroySurfaceTexture_internal(msg->m_pRender, msg->m_pSurfaceTexture);
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_RELEASESURFACETEXTURE:
        {
            CNxMsgReleaseSurfaceTexture* msg = (CNxMsgReleaseSurfaceTexture*)pMsg;
            releaseSurfaceTexture_internal(msg->m_pSurfaceTexture);
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_GETSURFACETEXTURE:
        {
            CNxMsgGetSurfaceTexture* msg = (CNxMsgGetSurfaceTexture*)pMsg;
            msg->m_pSurface = getSurfaceTexture_internal();
            msg->setProcessDone();
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_PREPARESURFACE:
        {
            CNxMsgPrepareSurfaceRenderer* msg = (CNxMsgPrepareSurfaceRenderer*)pMsg;
            msg->m_Result = prepareSurface_internal(msg->m_pSurface, msg->m_fScaleFactor);
            msg->setProcessDone();
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_SET_MAPPER:
        {
            CNxMsgSetMapper* msg = (CNxMsgSetMapper*)pMsg;
            if(m_hThemeRenderer)
                NXT_ThemeRenderer_SetMapper(m_hThemeRenderer, (void*)msg->m_pMapper);
            if(m_hExportThemeRenderer)
                NXT_ThemeRenderer_SetMapper(m_hExportThemeRenderer, (void*)msg->m_pMapper);
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_SET_THEMERENDER:
        {
            CNxMsgSetThemeRender* msg = (CNxMsgSetThemeRender*)pMsg;
            
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setThemeRender(%p)", __LINE__, msg->m_pRenderer);
            if( msg->m_pRenderer )
            {
                if(msg->m_Mode == 0){

                    if(m_hThemeRenderer)
                    {
                        NXT_ThemeRenderer_ClearTransitionEffect(m_hThemeRenderer);
                        NXT_ThemeRenderer_ClearClipEffect(m_hThemeRenderer);
                        
                        m_hThemeRenderer = NULL;
                    }

                    m_hThemeRenderer = (NXT_HThemeRenderer)msg->m_pRenderer;
                }
                else{

                    if(m_hExportThemeRenderer)
                    {
                        NXT_ThemeRenderer_ClearTransitionEffect(m_hExportThemeRenderer);
                        NXT_ThemeRenderer_ClearClipEffect(m_hExportThemeRenderer);
                        
                        m_hExportThemeRenderer = NULL;
                    }

                    m_hExportThemeRenderer = (NXT_HThemeRenderer)msg->m_pRenderer;                    
                }
            }
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_GET_BASERENDERER_INFO:
        {
            CNxMsgGetBaseRendererInfo* msg = (CNxMsgGetBaseRendererInfo*)pMsg;
            msg->m_Width = m_iSurfaceWidth;
            msg->m_Height = m_iSurfaceHeight;
            msg->m_pRenderer = m_hThemeRenderer;
            msg->setProcessDone();
            SAFE_RELEASE(msg);
            return MESSAGE_PROCESS_OK;
        }
		case MESSAGE_STATE_CHANGE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE", __LINE__);
            if(m_pPlaybackCacheWorker){

                m_pPlaybackCacheWorker->stopCaching();
                m_pPlaybackCacheWorker->End(1000);
            }
			m_iVideoStarted 	= 0;
			m_VideoProcessOk	= 0;
			m_iFirstVideoFrameUploaded = 0;
			// m_uiLastTime		= -1;

			CNxMsgChangeState* pStateChangeMsg = (CNxMsgChangeState*)pMsg;
			m_bLastClipRendered = FALSE;
			if( pStateChangeMsg->m_currentState == PLAY_STATE_IDLE )
			{
				m_uiLastTime = 0;
				m_uiVideoExportTotalCount = 0;
				for( int i = 0; i < m_VideoTrack.size(); i++)
				{
					// for preview stop delay problem
					if( m_VideoTrack[i]->isVideoClip() )
					{
						while( m_VideoTrack[i]->getFrameOutBuffer() != NULL )
						{
							m_VideoTrack[i]->DequeueFrameOutBuffer();
						}
						m_VideoTrack[i]->setVideoRenderEnd();
					}

					if( m_pDrawInfos != NULL ) {

                        NXT_HThemeRenderer renderer = getRendererForUID(m_VideoTrack[i]->m_uiTrackID, true);
                        if(renderer){

                            NXT_ThemeRenderer_AquireContext(renderer);
                            m_VideoTrack[i]->resetDrawInfos((void*)renderer);
                            NXT_ThemeRenderer_ReleaseTextureForTrackID(renderer, m_VideoTrack[i]->m_uiTrackID, 0);
                            NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                        }
					}

					m_VideoTrack[i]->Release();
				}
				m_VideoTrack.clear();

				if( m_pDrawInfos != NULL ) {
                    NXT_HThemeRenderer renderer = (NXT_HThemeRenderer)getCurrentVideoRenderer();
                    NXT_ThemeRenderer_AquireContext(renderer);
					m_pDrawInfos->resetDrawInfos((void*)renderer);
                    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
				}

				for( int i = 0; i < m_VideoLayer.size(); i++)
				{
					// for preview stop delay problem
					if (m_VideoLayer[i]->isVideoLayer())
					{
						while( m_VideoLayer[i]->getFrameOutBuffer() != NULL )
						{
							m_VideoLayer[i]->DequeueFrameOutBuffer();
						}
						m_VideoLayer[i]->setVideoRenderEnd();
					}

                    NXT_HThemeRenderer renderer = getRendererForUID(m_VideoLayer[i]->m_uiTrackID, true);
                    if(renderer){

                        NXT_ThemeRenderer_AquireContext(renderer);
                        NXT_ThemeRenderer_ReleaseTextureForTrackID(renderer, m_VideoLayer[i]->m_uiTrackID, 0);
                        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                    }
					m_VideoLayer[i]->Release();
				}
				m_VideoLayer.clear();				
				
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE clear track", __LINE__);

				m_perfGlDraw.CheckModuleLog();
				m_perfGetPixel.CheckModuleLog();
				m_perfNativeRender.CheckModuleLog();

			 	m_perfGlDraw.m_uiCount = 0;
				m_perfGetPixel.m_uiCount = 0;
				m_perfNativeRender.m_uiCount = 0;

				m_iEncodeWidth	= 0;
				m_iEncodeHeight	= 0;

				m_bInterruptExport = FALSE;

				SAFE_RELEASE(m_pEffectItemVec);

				// NXT_ThemeRenderer_ClearThemesAndEffects(m_hThemeRenderer, 0x00000001);
			}

			if( pStateChangeMsg->m_currentState == PLAY_STATE_RUN )
			{
				CNexVideoEditor* pVideoEditor = CNexVideoEditor::getVideoEditor();
				if( pVideoEditor != NULL )
				{
					m_iPreviewFPS = pVideoEditor->getPropertyInt("PreviewFPS", 0);
					m_iPreviewTime = 0;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] PreviewFPS(%d)", __LINE__, m_iPreviewFPS);
					SAFE_RELEASE(pVideoEditor);
				}
                NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
				NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, -1);
                NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
                
#ifdef _ANDROID
#ifdef FOR_TEST_THEME_CACHE
				if( m_pProjectMng )
				{
					NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
					NXT_Theme_ResetTextureManager(m_hThemeRenderer);
					
#if 0
					CClipList* pClipList = m_pProjectMng->getClipList();

					if( pClipList )
					{
						pClipList->lockClipList();
						for( int i = 0; i < pClipList->getClipCount(); i++)
						{
							IClipItem* pClip = pClipList->getClip(i);

							const char* pEffect = pClip->getClipEffectID();
							const char* pTitleEffect = pClip->getTitleEffectID();

							if( pEffect != NULL)
							{
								NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pEffect);
							}

							if( pTitleEffect != NULL)
							{
								NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pTitleEffect);
							}

							SAFE_RELEASE(pClip);
						}
						pClipList->unlockClipList();
						SAFE_RELEASE(pClipList);
					}
#else
					updateEffectItemVec();

					if (m_bSkipPrecacheEffect == FALSE && NXT_ThemeRenderer_CheckUploadOk(m_hThemeRenderer))
					{
						int async_flag = 0;
						unsigned int target_time = CNexProjectManager::getCurrentTimeStamp();
                        unsigned int target_end_time = target_time + 5000;
						int max_replaceable = DEFAULT_MAX_TEXTURE_CACHE_SIZE;

						if(m_pEffectItemVec){

							m_pEffectItemVec->lock();

							for (int i = 0; i < m_pEffectItemVec->getCount(); i++)
							{
								CVideoEffectItem *m_pEffectItem = m_pEffectItemVec->getEffetItem(i);

								if(m_pEffectItem->m_eEffectType == EFFECT_TYPE_TRANSITION){

									if(m_pEffectItem->m_uiEffectEndTime < target_time)
										continue;
                                    else if(m_pEffectItem->m_uiEffectStartTime > target_end_time)
                                        continue;
								}
								else if(m_pEffectItem->m_eEffectType == EFFECT_TYPE_TITLE){

									if(m_pEffectItem->m_uiTitleEndTime < target_time)
										continue;
                                    else if(m_pEffectItem->m_uiTitleStartTime > target_end_time)
                                        continue;
								}
								else
									continue;

								const char *pEffect = m_pEffectItem->getClipEffectID();
								const char *pTitleEffect = m_pEffectItem->getTitleEffectID();

								if (pEffect != NULL)
								{
									if(NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pEffect, &async_flag, &max_replaceable))
										break;
								}

								if (pTitleEffect != NULL)
								{
									if(NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pTitleEffect, &async_flag, &max_replaceable))
										break;
								}
							}

							m_pEffectItemVec->unlock();
						}

						if( m_pDrawInfos != NULL )
						{
							for(int i = 0; i < m_pDrawInfos->sizeDrawInfos(); ++i){

								CNexDrawInfo* pinfo = m_pDrawInfos->mDrawVec[i];
								if(pinfo && pinfo->mEffectID)
									if(NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pinfo->mEffectID, &async_flag, &max_replaceable))
										break;
							}
						}
                        if(m_pPlaybackCacheWorker){

                            m_pPlaybackCacheWorker->resetCacheTime(m_hThemeRenderer, m_pEffectItemVec, target_time);
                            m_pPlaybackCacheWorker->Begin();
                        }
					}					
#endif
					NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
				}
#endif
#endif
			}

			if( pStateChangeMsg->m_currentState == PLAY_STATE_RUN || pStateChangeMsg->m_currentState == PLAY_STATE_RECORD )
			{

				m_uiLastDrawTick = nexSAL_GetTickCount();
				updateEffectItemVec();

				if( m_pDrawInfos != NULL ) {
                    NXT_HThemeRenderer renderer = (NXT_HThemeRenderer)getCurrentVideoRenderer();
                    NXT_ThemeRenderer_AquireContext(renderer);
					m_pDrawInfos->resetDrawInfos(renderer);
                    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
				}				
			}

#ifdef _ANDROID
			if( pStateChangeMsg->m_currentState == PLAY_STATE_PAUSE )
			{
				int i = 1;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE PAUSE", __LINE__);

				if( m_pFileWriter )
				{
					unsigned int uiDuration	= 0;
					unsigned int uiSize		= 0;
					NXBOOL bEncodeEnd		= FALSE;
					unsigned int uLastEncodedFrameCTS = 0;

					NXBOOL bUseSurfaceCache = m_bUseSurfaceCache;
					m_bUseSurfaceCache = FALSE;
					for( int i = 0; i < m_VideoTrack.size(); i++)
					{
						if(!m_VideoTrack[i]->getVideoDecodeEnd())
						{
							// for preview stop delay problem
							if( m_VideoTrack[i]->isVideoClip() )
							{
								while( m_VideoTrack[i]->getFrameOutBuffer() != NULL )
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE clear track %d", __LINE__, i);
									m_VideoTrack[i]->DequeueFrameOutBuffer(NULL, NULL);
								}
								if(m_VideoTrack[i]->getSurfaceTexture())
									releaseSurfaceTexture_internal(m_VideoTrack[i]->getSurfaceTexture());
							}
							else	// for image
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE clear Image %d", __LINE__, i);						
								m_VideoTrack[i]->clearImageTrackDrawFlag();
							}
						}
					}

					for( int i = 0; i < m_VideoLayer.size(); i++)
					{
						if(!m_VideoLayer[i]->getVideoDecodeEnd())
						{
							// for preview stop delay problem
							if( m_VideoLayer[i]->isVideoClip() )
							{
								while( m_VideoLayer[i]->getFrameOutBuffer() != NULL )
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE clear layer %d", __LINE__, i);
									m_VideoLayer[i]->DequeueFrameOutBuffer(NULL, NULL);
								}
							}
							if(m_VideoLayer[i]->getSurfaceTexture())						
								releaseSurfaceTexture_internal(m_VideoLayer[i]->getSurfaceTexture());						
						}
					}
					m_bUseSurfaceCache = bUseSurfaceCache;

					//clearSurfaceTexture();
					int iRetry = nexSAL_GetTickCount();
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] remained buffer Count(%d %d), %d", __LINE__, m_uiVideoEncOutputFrameCount, m_uiVideoExportFrameCount, m_uiVideoEncOutputFrameCount);
					while( (nexSAL_GetTickCount() -  iRetry) < 100 && m_uiVideoEncOutputFrameCount != m_uiVideoExportFrameCount )
					{
						bEncodeEnd = FALSE;
						if(m_pFileWriter->setBaseVideoFrame(m_uiLastTime + i*33, FALSE, &uiDuration, &uiSize, &bEncodeEnd))
						{
							if( bEncodeEnd )
							{
								for( int i = 0;  i < m_VideoTrack.size(); i++)
								{
									if(m_VideoTrack[i]->m_FrameTimeChecker4Pause.size())
									{
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Track m_uiLastEncodedFrameCTS(%d, %zu)%d", __LINE__, m_VideoTrack[i]->m_FrameTimeChecker4Pause.getFirstCTSInVec(), m_VideoTrack[i]->m_FrameTimeChecker4Pause.size(), m_pFileWriter->getVideoTime());
										m_VideoTrack[i]->m_FrameTimeChecker4Pause.removeSmallTime( m_pFileWriter->getVideoTime());
										m_VideoTrack[i]->m_uiLastEncodedFrameCTS = m_VideoTrack[i]->m_FrameTimeChecker4Pause.getFirstCTSInVec();										
									}
								}						

								for( int i = 0;  i < m_VideoLayer.size(); i++)
								{
									if(m_VideoLayer[i]->m_FrameTimeChecker4Pause.size())
									{
										nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Layer m_uiLastEncodedFrameCTS(%d, %zu)%d", __LINE__, m_VideoLayer[i]->m_FrameTimeChecker4Pause.getFirstCTSInVec(), m_VideoLayer[i]->m_FrameTimeChecker4Pause.size(), m_pFileWriter->getVideoTime());
										m_VideoLayer[i]->m_FrameTimeChecker4Pause.removeSmallTime( m_pFileWriter->getVideoTime());			
										m_VideoLayer[i]->m_uiLastEncodedFrameCTS = m_VideoLayer[i]->m_FrameTimeChecker4Pause.getFirstCTSInVec();					
									}
								}								
								m_uiVideoEncOutputFrameCount++;
							}
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] remained buffer Count(%d %d)", __LINE__, m_uiVideoEncOutputFrameCount, m_uiVideoExportFrameCount);
							i++;
						}
						else
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setBaseVideoFrame failed & remained buffer Count(%d %d)", __LINE__, m_uiVideoEncOutputFrameCount, m_uiVideoExportFrameCount);
							break;
						}
					}
				}

				#if 0
				while(1)
				{
					unsigned int time;
					time = ((m_uiVideoExportTotalCount-1) * EXPORT_TIME_INTERVAL) / 100;

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE PLAY_STATE_PAUSE m_uiCurrentTime %d, %d, %d", __LINE__, m_uiCurrentTime, m_uiLastTime, m_uiVideoExportTotalCount);
					if(m_uiLastTime < time)
						m_uiVideoExportTotalCount--;
					else
						break;
				}
				#endif

				m_uiVideoExportTotalCount -= (m_uiVideoExportFrameCount - m_uiVideoEncOutputFrameCount);
				m_uiVideoExportFrameCount = m_uiVideoEncOutputFrameCount;
				m_uiCurrentTime = (m_uiVideoExportTotalCount * m_uiVideoExportInterval) / 100;
				m_perfGlDraw.CheckModuleLog();
				m_perfGetPixel.CheckModuleLog();
				m_perfNativeRender.CheckModuleLog();

			 	m_perfGlDraw.m_uiCount = 0;
				m_perfGetPixel.m_uiCount = 0;
				m_perfNativeRender.m_uiCount = 0;

				m_bInterruptExport = FALSE;

				m_pFileWriter->checkEncoderEOS(m_uiLastTime + i*33);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE PLAY_STATE_PAUSE NXT_ThemeRenderer_EndExport", __LINE__);
				NXT_ThemeRenderer_EndExport( m_hExportThemeRenderer );
				m_pMediaCodecInputSurf = NULL;
				m_pFileWriter->setPause(TRUE);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE PLAY_STATE_PAUSE %d", __LINE__, m_uiCurrentTime);
				
				//m_uiVideoExportTotalCount = m_uiVideoEncOutputFrameCount + 1;
				//m_uiCurrentTime = (m_uiVideoExportTotalCount * EXPORT_TIME_INTERVAL) / 100;
			}

			if( pStateChangeMsg->m_currentState == PLAY_STATE_RESUME )
			{
				for( int i = 0; i < m_VideoTrack.size(); i++)
				{
					if(!m_VideoTrack[i]->getVideoDecodeEnd())
					{
						// for preview stop delay problem
						if( m_VideoTrack[i]->isVideoClip() )
						{
							m_VideoTrack[i]->m_FrameTimeChecker4Pause.clear();
							m_VideoTrack[i]->setResume();
						}
					}
				}

				for( int i = 0; i < m_VideoLayer.size(); i++)
				{
					if(!m_VideoLayer[i]->getVideoDecodeEnd())
					{
						// for preview stop delay problem
						if( m_VideoLayer[i]->isVideoClip() )
						{
							m_VideoLayer[i]->m_FrameTimeChecker4Pause.clear();
							m_VideoLayer[i]->setResume();	
						}
					}
				}
                
				m_iVideoStarted = 0;
				m_VideoProcessOk = 0;
				//clearSurfaceTexture();										
				m_pFileWriter->setResume();

				m_uiLastDrawTick = nexSAL_GetTickCount();

				//NXT_ThemeRenderer_EndExport( m_hExportThemeRenderer );
				if( m_pMediaCodecInputSurf == NULL )
				{
					m_pMediaCodecInputSurf = m_pFileWriter->getMediaCodecInputSurface();
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] mediacodec input Surf handle(%p) Width(%d) Height(%d)", __LINE__, m_pMediaCodecInputSurf, m_iEncodeWidth, m_iEncodeHeight);
				}

				if( m_pMediaCodecInputSurf )
				{
					NXT_Error eRet = NXT_ThemeRenderer_BeginExport( m_hExportThemeRenderer, m_iEncodeWidth, m_iEncodeHeight,  (ANativeWindow*)m_pMediaCodecInputSurf );
					if( eRet != NXT_Error_None )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] BeginExpot failed(0x%x)", __LINE__, eRet);
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] BeginExport with mediacodec's input Surf", __LINE__);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MediaCodec Input surface was null", __LINE__);
				}
				
			}
#endif
			
			if( m_pProjectEffect && m_pProjectMng)
			{
				NXT_ThemeRenderer_SetWatermarkEffect(m_hThemeRenderer, m_pProjectEffect->m_pEffectID, m_pProjectMng->getDuration());
				NXT_ThemeRenderer_SetWatermarkEffect(m_hExportThemeRenderer, m_pProjectEffect->m_pEffectID, m_pProjectMng->getDuration());
			}
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_STATE_CHANGE End", __LINE__);
			break;
		}
		case MESSAGE_UPDATE_CURRENTIME:
		{
			if( m_eThreadState == PLAY_STATE_IDLE )
			{
				m_bUpdateCurrentTime = TRUE;
			}
			break;
		}		
		case MESSAGE_VIDEO_RENDERING_INFO:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_VIDEO_RENDERING_INFO", __LINE__);
			CNxMsgVideoRenderInfo* pVideoInitMsg = (CNxMsgVideoRenderInfo*)pMsg;
			// WaveOut ReInit;

			m_RD.left	= pVideoInitMsg->m_iLeft;
			m_RD.top	= pVideoInitMsg->m_iTop;
			m_RD.right	= pVideoInitMsg->m_iRight;
			m_RD.bottom	= pVideoInitMsg->m_iBottom;

			SAFE_RELEASE(pVideoInitMsg);

			if( m_pProjectMng == NULL )
				m_pProjectMng = CNexProjectManager::getProjectManager();

			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_ADD_TRACK:
		{

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ADD_TRACK(tracksize:%zu)", __LINE__, m_VideoTrack.size());
			CNxMsgAddTrack* pVideoTrack = (CNxMsgAddTrack*)pMsg;
			CVideoTrackInfo* pTrack = (CVideoTrackInfo*)pVideoTrack->m_pTrack;

#ifdef FOR_VIDEO_LAYER_FEATURE
			if( pTrack && pTrack->isVideoLayer() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ADD_TRACKvideoLayer(videoLayer size:%zu)", __LINE__, m_VideoLayer.size());

				for(VideoTrackVecIter i = m_VideoLayer.begin(); i != m_VideoLayer.end(); i++)
				{
					CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
					if( pTrackItem == NULL ) continue;
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ADD_TRACKvideoLayer Check layer ID(Exist:%d Add:%d)", __LINE__, pTrackItem->m_uiTrackID, pTrack->m_uiTrackID);
					if( pTrackItem->m_uiTrackID == pTrack->m_uiTrackID )
					{
						m_VideoLayer.erase(i);
						SAFE_RELEASE(pTrackItem);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ADD_TRACKvideoLayer delete existed video Layer(Layer size:%zu)", __LINE__, m_VideoLayer.size());
						break;
					}
				}
				
				SAFE_ADDREF(pTrack);

				int iTextureID = NXT_ThemeRenderer_CreateTextureForVideoLayer((NXT_HThemeRenderer)getCurrentVideoRenderer(), pTrack->m_uiTrackID);
                recordUIDRenderPair(pTrack->m_uiTrackID, (NXT_HThemeRenderer)getCurrentVideoRenderer());
				pTrack->setVideoLayerTextureID(iTextureID);
				if(getState() == PLAY_STATE_RECORD)			
					pTrack->setVideoDropRenderTime(m_uiVideoExportInterval / 100 + 1);
				pTrack->printTrackInfo();
				m_VideoLayer.insert(m_VideoLayer.end(), pTrack);

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ADD_TRACKvideoLayer Video MESSAGE_ADD_VIDEO_LAYER(%d) Ended(%d, Track Size: %zu TextureID : %d)", 
					__LINE__, pTrack->m_uiTrackID, pTrack->GetRefCnt(), m_VideoLayer.size(), iTextureID);
				SAFE_RELEASE(pVideoTrack);

				return MESSAGE_PROCESS_OK;			
			}
#endif	

			for(VideoTrackVecIter i = m_VideoTrack.begin(); i != m_VideoTrack.end(); i++)
			{
				CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
				if( pTrackItem == NULL ) continue;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Check Track ID(Exist:%d Add:%d)", __LINE__, pTrackItem->m_uiTrackID, pTrack->m_uiTrackID);
				if( pTrackItem->m_uiTrackID == pTrack->m_uiTrackID )
				{
					m_VideoTrack.erase(i);
					SAFE_RELEASE(pTrackItem);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] delete exist video track(tracksize:%zu)", __LINE__, m_VideoTrack.size());
					break;
				}
			}
			
			// Add Mixer Track
			SAFE_ADDREF(pTrack);
			pTrack->printTrackInfo();

			if( m_pDrawInfos != NULL && pTrack != NULL )
			{
				pTrack->m_iTrackTextureID = NXT_ThemeRenderer_CreateTextureForTrackID((NXT_HThemeRenderer)getCurrentVideoRenderer(), pTrack->m_uiTrackID);
                recordUIDRenderPair(pTrack->m_uiTrackID, (NXT_HThemeRenderer)getCurrentVideoRenderer());
			}

			VideoTrackVecIter i;
			int iTrackIdx = 0;
			
			for( i = m_VideoTrack.begin(); i != m_VideoTrack.end(); i++, iTrackIdx++)
			{
				if( m_VideoTrack[iTrackIdx]->m_uiStartTime < pTrack->m_uiStartTime )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Find video track index(%d) Time(%d %d)", __LINE__, iTrackIdx, m_VideoTrack[iTrackIdx]->m_uiStartTime, pTrack->m_uiStartTime);
					continue;
				}
				if( iTrackIdx == 0 )
				{
					m_VideoTrack[0]->clearImageTrackDrawFlag();
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Found video track index(%d) Time(%d %d)", __LINE__, iTrackIdx, m_VideoTrack[iTrackIdx]->m_uiStartTime, pTrack->m_uiStartTime);
				break;
			}

			if(getState() == PLAY_STATE_RECORD)			
				pTrack->setVideoDropRenderTime(m_uiVideoExportInterval / 100 + 1);
            
			m_VideoTrack.insert(i, pTrack);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_ADD_TRACK Track( Ref Count : %d)", __LINE__, pTrack->GetRefCnt());

			/*  // remark this code because display title of dummy or pre clips when starting at transition position.
			if( 	m_iVideoStarted == 0 && m_hThemeRenderer )
			{
				NXT_ThemeRenderer_SetEffect( m_hThemeRenderer, (NXT_HEffect)pTrack->m_pTitleEffect, pTrack->m_uiCurrentClipIndex, pTrack->m_uiTotalClipCount);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_ADD_TRACK Track set Title Effect", __LINE__, pTrack->GetRefCnt());
			}
			*/
			SAFE_RELEASE(pVideoTrack);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_ADD_TRACK Track Ended(Track Size: %zu)", __LINE__, m_VideoTrack.size());
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_DELETE_TRACK:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK(%zu)", __LINE__, m_VideoTrack.size());
			CNxMsgDeleteTrack* pDeleteTrackMsg = (CNxMsgDeleteTrack*)pMsg;
			if( pDeleteTrackMsg == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK error (%p)", __LINE__, pDeleteTrackMsg);
				break;
			}

			unsigned int uiTrackID = pDeleteTrackMsg->m_uiTrackID;
			CVideoTrackInfo* pTrack = (CVideoTrackInfo*)pDeleteTrackMsg->m_pTrack;
			if( pTrack != NULL )
			{
				uiTrackID = pTrack->m_uiTrackID;
			}
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK(ID:%d)", __LINE__, uiTrackID);
			
#ifdef FOR_VIDEO_LAYER_FEATURE

			if(pTrack == NULL){

				//It is special case...for really deleted video layer...
                NXT_HThemeRenderer renderer = getRendererForUID(uiTrackID, true);
                if(renderer){

                    NXT_ThemeRenderer_AquireContext(renderer);
                    NXT_ThemeRenderer_ReleaseTextureForVideoLayer(renderer, uiTrackID, 1);
                    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                }
			}

			if( pTrack && pTrack->isVideoLayer() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DELETE_VIDEO_LAYER(ID:%d)", __LINE__, uiTrackID);
                NXT_HThemeRenderer renderer = getRendererForUID(pTrack->m_uiTrackID, true);
                if(renderer){

                    NXT_ThemeRenderer_AquireContext(renderer);
                    NXT_ThemeRenderer_ReleaseTextureForVideoLayer(renderer, pTrack->m_uiTrackID, 0);
                    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                    // pTrack->setVideoLayerTextureID(0);
                }
				
				for(VideoTrackVecIter i = m_VideoLayer.begin(); i != m_VideoLayer.end(); i++)
				{
					CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
					if( pTrackItem == NULL ) continue;
					if( pTrackItem->m_uiTrackID == uiTrackID )
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DELETE_VIDEO_LAYER Ref:%d", __LINE__, pTrackItem->GetRefCnt());
						m_VideoLayer.erase(i);

						if( getState() == PLAY_STATE_RUN && pTrackItem->isVideoLayer() && m_iVideoStarted == 0 )
						{
							m_iVisualLayerCountAtStartTime--;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Send Video Start event becuse first clip delete without display %d", __LINE__, m_iVisualLayerCountAtStartTime);
							//sendVideoStarted();
						}
						
						SAFE_RELEASE(pTrackItem);
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DELETE_VIDEO_LAYER", __LINE__);
						break;
					}
				}
				
				pDeleteTrackMsg->setProcessDone();
				SAFE_RELEASE(pDeleteTrackMsg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DELETE_VIDEO_LAYER(%zu)", __LINE__, m_VideoLayer.size());
				return MESSAGE_PROCESS_OK;
			}
#endif			
			
			for(VideoTrackVecIter i = m_VideoTrack.begin(); i != m_VideoTrack.end(); i++)
			{
				CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
				if( pTrackItem == NULL ) continue;
				if( pTrackItem->m_uiTrackID == uiTrackID )
				{
#ifdef NO_OVERLAPPED_TRANSITION_CODE
					if( pTrackItem->m_uiEndTime >= m_uiCurrentTime && pTrackItem->isVideoClip() )
					{
						pTrackItem->DequeueAllFrameOutBuffer();
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK ignored because track time is not ended ID(%d) EndTime(%d) Current(%d)", __LINE__, pTrackItem->m_uiTrackID, pTrackItem->m_uiEndTime, m_uiCurrentTime);
						continue;
					}
#endif

					if( m_pDrawInfos != NULL )
					{
                        NXT_HThemeRenderer renderer = getRendererForUID(pTrack->m_uiTrackID, true);
                        if(renderer){

                            NXT_ThemeRenderer_AquireContext(renderer);
                            pTrackItem->resetDrawInfos((void*)renderer);
                            NXT_ThemeRenderer_ReleaseTextureForTrackID(renderer, pTrackItem->m_uiTrackID, 0);
                            NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                        }
						
					}
					m_VideoTrack.erase(i);
					// mantis 5813 
					// ZIRA 1658
					if( m_bSeparateEffect == FALSE )
						pTrackItem->resetEffect(getCurrentVideoRenderer());

					// kbsong 20130808 : change for mantis 7722
					if( getState() == PLAY_STATE_RUN && pTrackItem->isVideoClip() && m_iVideoStarted == 0 )
					{
						m_iVisualClipCountAtStartTime--;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Send Video Start event becuse first clip delete without display %d", __LINE__, m_iVisualClipCountAtStartTime);
						sendVideoStarted();
					}

#ifdef __APPLE__                    
                    // added by eric.
                    // 2016.7.21
                    pTrackItem->DequeueAllFrameOutBuffer();
#endif

					CClipList* pClipList = m_pProjectMng->getClipList();
					if( getState() == PLAY_STATE_RECORD && pTrackItem->isLastTrack(pClipList->getClipTotalTime()) )
					{
						m_bLastClipRendered = TRUE;
						m_pFileWriter->setLastTrackEnded(TRUE);
					}
					SAFE_RELEASE(pClipList);
					SAFE_RELEASE(pTrackItem);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK(%zu)", __LINE__, m_VideoTrack.size());
					break;
				}
			}
			
			pDeleteTrackMsg->setProcessDone();
			SAFE_RELEASE(pDeleteTrackMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK(%zu)", __LINE__, m_VideoTrack.size());
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_DELETE_TRACK_ALL:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK_ALL size(%zu)", __LINE__, m_VideoTrack.size());
			
			for(VideoTrackVecIter i = m_VideoTrack.begin(); i != m_VideoTrack.end(); i++)
			{
				CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DELETE_ALL_VIDEO_TRACK Ref:%d", __LINE__, pTrackItem->GetRefCnt());

				if( pTrackItem != NULL && m_pDrawInfos != NULL ) {

                    NXT_HThemeRenderer renderer = getRendererForUID(pTrackItem->m_uiTrackID, true);
                    if(renderer) {

                        NXT_ThemeRenderer_AquireContext(renderer);
                        pTrackItem->resetDrawInfos((void*)renderer);
                        NXT_ThemeRenderer_ReleaseTextureForTrackID(renderer, pTrackItem->m_uiTrackID, 0);
                        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                    }
				}

				SAFE_RELEASE(pTrackItem);
			}
			m_VideoTrack.clear();

			if( m_pDrawInfos != NULL ) {

                NXT_HThemeRenderer renderer = (NXT_HThemeRenderer)getCurrentVideoRenderer();
                NXT_ThemeRenderer_AquireContext(renderer);
                m_pDrawInfos->resetDrawInfos((void*)renderer);
                NXT_ThemeRenderer_ReleaseContext(renderer, 0);
			}

#ifdef FOR_VIDEO_LAYER_FEATURE
			for (VideoTrackVecIter i = m_VideoLayer.begin(); i != m_VideoLayer.end(); i++)
			{
				CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;		
				if( pTrackItem != NULL )
				{
                    NXT_HThemeRenderer renderer = getRendererForUID(pTrackItem->m_uiTrackID, true);
                    if(renderer) {
                        NXT_ThemeRenderer_AquireContext(renderer);
                        NXT_ThemeRenderer_ReleaseTextureForVideoLayer(renderer, pTrackItem->m_uiTrackID, 1);
                        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                    }
					pTrackItem->setVideoLayerTextureID(0);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DELETE_ALL_VIDEO_LAYER Ref:%d", __LINE__, pTrackItem->GetRefCnt());
					SAFE_RELEASE(pTrackItem);
				}
			}
			m_VideoLayer.clear();
            // NXT_ThemeRenderer_AquireContext((NXT_HThemeRenderer)getCurrentVideoRenderer());
			// NXT_ThemeRenderer_ReleaseTextureForVideoLayer((NXT_HThemeRenderer)getCurrentVideoRenderer(), 0xFFFFFFFF, 0);
            // NXT_ThemeRenderer_ReleaseContext((NXT_HThemeRenderer)getCurrentVideoRenderer(), 0);
#endif
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_TRACK_ALL done size(%zu)", __LINE__, m_VideoTrack.size());
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;			
		}
		case MESSAGE_VIDEO_RENDER_POSITION:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_VIDEO_RENDER_POSITION", __LINE__);
			CNxMsgVideoRenderPos* pPos = (CNxMsgVideoRenderPos*)pMsg;

			m_RD.left		= pPos->m_iLeft;
			m_RD.top		= pPos->m_iTop;
			m_RD.right		= pPos->m_iRight;
			m_RD.bottom		= pPos->m_iBottom;

			SAFE_RELEASE(pPos);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_FILEWRITER:
		{
			if( m_pFileWriter )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_FILEWRITER Writer RefCnt(%d)", __LINE__, m_pFileWriter->GetRefCnt());
			}
			
			CNxMsgSetFileWriter* pSetFileWriter = (CNxMsgSetFileWriter*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_FILEWRITER(%p)", __LINE__, pSetFileWriter);
			
			if( pSetFileWriter )
			{
				if( CNexVideoEditor::m_iUseSurfaceMediaSource == 1)
				{
					clearSurfaceTexture();
					NXBOOL bInitCheck = TRUE;
					if( pSetFileWriter->m_pWriter )
					{
						m_uiVideoExportTotalCount = 0;
						m_uiCurrentTime = 0;
						SAFE_RELEASE(m_pFileWriter);
						m_pFileWriter = (CNexExportWriter*)pSetFileWriter->m_pWriter;

						m_iEncodeWidth		= m_pFileWriter->getEncodeWidth();
						m_iEncodeHeight		= m_pFileWriter->getEncodeHeight();

						m_iSurfaceWidth = m_iEncodeWidth;
						m_iSurfaceHeight = m_iEncodeHeight;

						m_uiVideoExportInterval = m_pFileWriter->getEncoderFrameInterval();

						if( m_pMediaCodecInputSurf == NULL )
						{
							m_pMediaCodecInputSurf = m_pFileWriter->getMediaCodecInputSurface();
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] mediacodec input Surf handle(%p) Width(%d) Height(%d)", __LINE__, m_pMediaCodecInputSurf, m_iEncodeWidth, m_iEncodeHeight);
						}

						NXT_RendererOutputType typeRenderOutput;
						if( m_pMediaCodecInputSurf )
						{
#ifdef _ANDROID
							NXT_Error eRet = NXT_ThemeRenderer_BeginExport( m_hExportThemeRenderer, m_iEncodeWidth, m_iEncodeHeight,  (ANativeWindow*)m_pMediaCodecInputSurf );
							if( eRet != NXT_Error_None )
							{
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] BeginExpot failed(0x%x)", __LINE__, eRet);
								bInitCheck = FALSE;
							}
#endif
							typeRenderOutput = NXT_RendererOutputType_RGBA_8888;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] BeginExport with mediacodec's input Surf", __LINE__);
						}
						else
						{
							bInitCheck = FALSE;
							typeRenderOutput = NXT_RendererOutputType_Y2CrA_8888;
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MediaCodec Input surface was null", __LINE__);
						}

#ifdef FOR_TEST_MEDIACODEC_ENCODER_DSI
						if( bInitCheck )
						{
							int iCount = 20;
								
							while( iCount > 0 )
							{
								iCount--;
								NXT_ThemeRenderer_AquireContext(m_hExportThemeRenderer);
								NXT_ThemeRenderer_SwapBuffers(m_hExportThemeRenderer);
								NXT_ThemeRenderer_ReleaseContext(m_hExportThemeRenderer, 0);
								int iRet = m_pFileWriter->setVideoFrameForDSIwithInputSurf();
								if( iRet == 0 )
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] get DSI from MediaCodec end", __LINE__);
									break;
								}
								else if( iRet == 1 )
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] get DSI from MediaCodec process(%d)", __LINE__, iCount);
									continue;
								}
								else
								{
									nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] get DSI from MediaCodec failed", __LINE__);
									bInitCheck = FALSE;
									break;
								}
							}
							if( iCount == 0  )
							{
								bInitCheck == FALSE;
							}
							
						}

						if( bInitCheck == FALSE )
						{
							pSetFileWriter->m_nResult = 1;
						}
#endif						
						
						m_uiVideoExportFrameCount		= 0;
						m_uiVideoEncOutputFrameCount	= 0;
						m_uiVideoExportTotalCount		= 0;
 					}
					else
					{
						unsigned int uiDuration	= 0;
						unsigned int uiSize		= 0;
						NXBOOL bEncodeEnd		= FALSE;
						
						int bPaused = 0;              
						
						if( m_pFileWriter )
						{
							unsigned int uiDuration	= 0;
							unsigned int uiSize		= 0;
							NXBOOL bEncodeEnd		= FALSE;
							
							// Kyebeom Song 20130807 : for Mantis 6521
							clearSurfaceTexture();
							int iRetry = nexSAL_GetTickCount();
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] remained buffer Count(%d %d)", __LINE__, m_uiVideoEncOutputFrameCount, m_uiVideoExportFrameCount);
							while( (nexSAL_GetTickCount() -  iRetry) < 1000 && m_uiVideoEncOutputFrameCount != m_uiVideoExportFrameCount )
							{
								bEncodeEnd = FALSE;
								m_uiLastTime += 33;
								m_pFileWriter->setBaseVideoFrame(m_uiLastTime, FALSE, &uiDuration, &uiSize, &bEncodeEnd);
								if( bEncodeEnd )
									m_uiVideoEncOutputFrameCount++;
								nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] remained buffer Count(%d %d)", __LINE__, m_uiVideoEncOutputFrameCount, m_uiVideoExportFrameCount);
									
								// NXT_ThemeRenderer_ReleaseContext(m_hExportThemeRenderer, 0);
								nexSAL_TaskSleep(30);
							}
							bPaused = m_pFileWriter->getPause();
							SAFE_RELEASE(m_pFileWriter);
						}

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] clear native window of theme renderer", __LINE__);
#ifdef _ANDROID
						if(bPaused == 0)
							NXT_ThemeRenderer_EndExport( m_hExportThemeRenderer );
#endif
						m_pMediaCodecInputSurf = NULL;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] clear native window of theme renderer", __LINE__);
					}
					pSetFileWriter->setProcessDone();
				}				
				else
				{
					SAFE_RELEASE(m_pFileWriter);					
					m_pFileWriter = (CNexExportWriter*)pSetFileWriter->m_pWriter;
					if( m_pFileWriter )
					{
#ifdef __APPLE__
                        // MJ: NESI-152, NESI-159 ???????반영
                        m_uiCurrentTime = 0;
#endif
						// ZIRA 1906
						m_uiVideoExportTotalCount = 0;
						m_iEncodeWidth		= m_pFileWriter->getEncodeWidth();
						m_iEncodeHeight		= m_pFileWriter->getEncodeHeight();

						m_iSurfaceWidth = m_iEncodeWidth;
						m_iSurfaceHeight = m_iEncodeHeight;

						m_uiVideoExportInterval = m_pFileWriter->getEncoderFrameInterval();
						m_bInterruptExport	= FALSE;
					};
					pSetFileWriter->setProcessDone();
				}
				
				SAFE_ADDREF(m_pFileWriter);
				SAFE_RELEASE(pSetFileWriter);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_FILEWRITER END!!(%p) (%d %d %d)", __LINE__, m_pFileWriter, m_iEncodeWidth, m_iEncodeHeight, m_uiVideoExportInterval);
				
				return MESSAGE_PROCESS_OK;
			}
			break;
		}

		case MESSAGE_SET_TIME:
		{
			CNxMsgSetTime* pSetTime = (CNxMsgSetTime*)pMsg;
			if( m_eThreadState != PLAY_STATE_IDLE || pSetTime == NULL || m_isDrawEnd == FALSE)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Ignore SetTime Event", __LINE__);
				CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
				pSetTime->setProcessDone();
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK; 
			}
			
			m_uiCurrentTime = m_iPreviewTime = pSetTime->m_uiTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] SetTime Event(%d)", __LINE__, m_uiCurrentTime);
			for(int i = 0; i < m_VideoTrack.size(); i++)
			{
				if( m_VideoTrack[i] != NULL )
				{
					m_VideoTrack[i]->resetEffect(getCurrentVideoRenderer());
				}
			}

			if( m_bSeparateEffect )
			{
				SAFE_RELEASE(m_pEffectItemVec);
				CClipList* pCliplist = (CClipList*)pSetTime->getClipList();
				if( pCliplist != NULL )
				{
					m_pEffectItemVec = pCliplist->getEffectItemVec();
					SAFE_RELEASE(pCliplist);
				}
				
				if( m_pEffectItemVec != NULL ){

                    m_pEffectItemVec->lock();
					m_pEffectItemVec->clearEffectFlag();				
                    m_pEffectItemVec->unlock();
                }
			}			

			// return 0 ignore
			// return 1 set time done sucessed
			// return 2 set time done failed			

			int iRet = drawIdle(pSetTime->m_uiTime, pSetTime->m_iDisplay);
			if( iRet == 0 )
			{
				CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
			}
			else if( iRet == 1 )
			{
				// for mantis 5702
				CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_NONE, 0, pSetTime->m_uiTime, m_uiLastSetTime);
//				if( m_pProjectMng == NULL )
//					m_pProjectMng = CNexProjectManager::getProjectManager();
//				m_pProjectMng->SendSimpleCommand(MESSAGE_LOAD_ORIGINAL_IMAGE);
			}
			else
			{
				CNexProjectManager::sendEvent(MESSAGE_SET_TIME_DONE, NEXVIDEOEDITOR_ERROR_INVALID_INFO, 1);
			}

//			CClipList* pCliplist = (CClipList*)pSetTime->getClipList();
//			if( pCliplist != NULL )
//			{
//				pCliplist->clearImageTask();
//				SAFE_RELEASE(pCliplist);
//			}
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_TIME(%d)", __LINE__, pSetTime->GetRefCnt());
			pSetTime->setProcessDone();
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_DRAWIDLE:
		{
//			int iRet = drawIdle(0, 1);
			CFrameInfo* 				pFrame			= NULL;
			if( m_pProjectMng == NULL )
				m_pProjectMng = CNexProjectManager::getProjectManager();
			unsigned int uiTime = m_pProjectMng->getCurrentTimeStamp();
			NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
//			m_pDrawInfos->applyDrawInfo((void*)m_hThemeRenderer, uiTime);
	
			for( int i = 0;  i < m_VideoTrack.size(); i++)
			{
				// ZIRA 1905
				// m_VideoTrack[i]->setFirstVideoFrameDrop((void*)m_hThemeRenderer);
	
				if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
				{
					if( m_VideoTrack[i]->m_uiEndTime <= uiTime )
					{
						if( m_VideoTrack[i]->isVideoClip() )
						{
							m_VideoTrack[i]->DequeueAllFrameOutBuffer();
						}
					}
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) drawIdle not track time(idx:%d) ", __LINE__, m_VideoTrack[i]->m_uiTrackID, i); 			
					continue;
				}
	
				pFrame = m_VideoTrack[i]->getFrameOutBuffer();
				if( pFrame == NULL )
				{
					m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, uiTime);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) drawIdle no Frame(idx:%d) ", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);							
					continue;
				}
	
				m_uiLastSetTime = pFrame->m_uiTime + 1;
	
				pFrame->drawVideo((void*)m_hThemeRenderer, uiTime, m_VideoTrack[i]->m_iTrackTextureID, FALSE, TRUE);
//				m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, uiTime);
				m_VideoTrack[i]->DequeueFrameOutBuffer();
			}
	
			m_uiLastTime = uiTime;
			m_uiCurrentTime = uiTime;
			NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);
			NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
			NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 1);
			SAFE_RELEASE(pMsg);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DRAWIDLE()", __LINE__ );
			return MESSAGE_PROCESS_OK; 
		}
		case MESSAGE_CAPTURE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_CAPTURE", __LINE__, m_uiCurrentTime);
			capture();
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK; 
		}		
		case MESSAGE_VIDEO_RENDER_CLEAR:
		{
			m_bClearRenderer = TRUE;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_VIDEO_RENDER_CLEAR", __LINE__, m_uiCurrentTime);
			drawClear();
			break;
		}
		case MESSAGE_CLEAR_SCREEN:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_CLEAR_SCREEN", __LINE__);

			CNxMsgClearScreen* pClearScreen = (CNxMsgClearScreen*)pMsg;
			if( pClearScreen )
			{
				NXT_ThemeRenderer_ClearScreen(getCurrentVideoRenderer(), pClearScreen->m_iTag);
			}
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_FAST_OPTION_PREVIEW:
		{
			CNxMsgFastOptionPreview* pFastOptionPreview = (CNxMsgFastOptionPreview*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_OPTION_PREVIEW %d", __LINE__, pFastOptionPreview);
			if( pFastOptionPreview )
			{
				fastOptionPreview(pFastOptionPreview->m_pOption, pFastOptionPreview->m_iDisplay);
			}
			SAFE_RELEASE(pFastOptionPreview);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_CLEAR_TEXTURE:
		{
			clearUploadTexture(1, 64,64, TRUE);
			clearUploadTexture(2, 64,64, TRUE);
			SAFE_RELEASE(pMsg);
			m_bAvailableCapture = FALSE;
			return MESSAGE_PROCESS_OK;
		}
#ifdef _ANDROID
		case MESSAGE_CLEAR_SURFACETEXTURE:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] SurfaceTexture clear message receive", __LINE__);
			clearSurfaceTexture();
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_PAUSE_RENDERER:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Renderer pause message receive", __LINE__);
			m_bRenderPause = TRUE;
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
			break;
		}
		case MESSAGE_RESUME_RENDERER:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Renderer resume message receive", __LINE__);
			m_bRenderPause = FALSE;
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
			break;
		}
#endif
		case MESSAGE_SET_PROJECT_EFFECT:
		{
			SAFE_RELEASE(m_pProjectEffect);
			m_pProjectEffect = (CNxMsgSetProjectEffect*)pMsg;
			SAFE_ADDREF(m_pProjectEffect);
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}		

		case MESSAGE_ADD_LAYER_ITEM:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_ADD_LAYER_ITEM(tracksize:%zu)", __LINE__, m_LayerItemVec.size());
			CNxMsgAddLayer* pLayerTrack = (CNxMsgAddLayer*)pMsg;
			if( pLayerTrack == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_ADD_LAYER_ITEM error (%p)", __LINE__, pLayerTrack);
				break;
			}
			
			CVideoTrackInfo* pTrack = (CVideoTrackInfo*)pLayerTrack->m_pTrack;
			for(VideoTrackVecIter i = m_LayerItemVec.begin(); i != m_LayerItemVec.end(); i++)
			{
				CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
				if( pTrackItem == NULL ) continue;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Check Layer ID(Exist:%d Add:%d)", __LINE__, pTrackItem->m_uiTrackID, pTrack->m_uiTrackID);
				if( pTrackItem->m_uiTrackID == pTrack->m_uiTrackID )
				{
					m_LayerItemVec.erase(i);
					SAFE_RELEASE(pTrackItem);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] delete exist video track(tracksize:%zu)", __LINE__, m_LayerItemVec.size());
					break;
				}
			}
			
			// Add Mixer Track
			SAFE_ADDREF(pTrack);
			pTrack->printTrackInfo();
			if(getState() == PLAY_STATE_RECORD)						
				pTrack->setVideoDropRenderTime(m_uiVideoExportInterval / 100 + 1);
			m_LayerItemVec.insert(m_LayerItemVec.end(), pTrack);

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_ADD_LAYER_ITEM( Ref Count : %d)", __LINE__, pTrack->GetRefCnt());

			SAFE_RELEASE(pLayerTrack);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_ADD_LAYER_ITEM Ended(Track Size: %zu)", __LINE__, m_LayerItemVec.size());
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_DELETE_LAYER_ITEM:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_LAYER_ITEM(%zu)", __LINE__, m_LayerItemVec.size());
			
			CNxMsgDeleteLayer* pLayerTrack = (CNxMsgDeleteLayer*)pMsg;
			if( pLayerTrack == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_LAYER_ITEM error (%p)", __LINE__, pLayerTrack);
				break;
			}

			unsigned int uiTrackID = pLayerTrack->m_uiTrackID;
			CVideoTrackInfo* pTrack = (CVideoTrackInfo*)pLayerTrack->m_pTrack;
			if( pTrack )
				uiTrackID = pTrack->m_uiTrackID;
			
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_LAYER_ITEM(ID:%d)", __LINE__, uiTrackID);
			for(VideoTrackVecIter i = m_LayerItemVec.begin(); i != m_LayerItemVec.end(); i++)
			{
				CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
				if( pTrackItem == NULL ) continue;
				
				if( pTrackItem->m_uiTrackID == uiTrackID )
				{
					m_LayerItemVec.erase(i);
					SAFE_RELEASE(pTrackItem);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_LAYER_ITEM(%zu)", __LINE__, m_LayerItemVec.size());
					break;
				}
			}
			
			pLayerTrack->setProcessDone();
			SAFE_RELEASE(pLayerTrack);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_DELETE_LAYER_ITEM(%zu)", __LINE__, m_LayerItemVec.size());
			return MESSAGE_PROCESS_OK;
		}
		
		case MESSAGE_LOAD_THEMEANDEFFECT:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_LOAD_THEMEANDEFFECT", __LINE__);
			CNxMsgLoadThemeAndEffect* pLoadtheme = (CNxMsgLoadThemeAndEffect*)pMsg;
			if( pLoadtheme != NULL )
			{
				if(pLoadtheme->m_iFlag == 0x00000001)// Preview
				{
					NXT_ThemeRenderer_LoadThemesAndEffects(m_hThemeRenderer, pLoadtheme->m_pResource, pLoadtheme->m_iFlag);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Load theme and effect on preview theme renderer", __LINE__);
				}
				else
				{
					NXT_ThemeRenderer_LoadThemesAndEffects(m_hExportThemeRenderer, pLoadtheme->m_pResource, pLoadtheme->m_iFlag);

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d]Load theme and effect on export theme renderer", __LINE__);
				}
			}
			m_bAvailableCapture = FALSE;
			SAFE_RELEASE(pLoadtheme);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_CLEAR_RENDERITEMS:
		{
			CNxMsgClearRenderItems* pClearRenderItems = (CNxMsgClearRenderItems*)pMsg;
			if(pClearRenderItems->m_iFlag == 0x00000001)// Preview
			{
				NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
				NXT_ThemeRenderer_ClearRenderItems(m_hThemeRenderer);
				NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
			}
			else
			{
				NXT_ThemeRenderer_AquireContext(m_hExportThemeRenderer);
				NXT_ThemeRenderer_ClearRenderItems(m_hExportThemeRenderer);
				NXT_ThemeRenderer_ReleaseContext(m_hExportThemeRenderer, 0);
			}
			SAFE_RELEASE(pClearRenderItems);
			return MESSAGE_PROCESS_OK;
		}

		case MESSAGE_LOAD_RENDERITEM:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video MESSAGE_LOAD_RENDERITEM", __LINE__);
			CNxMsgLoadRenderItem* pLoadRenderItem = (CNxMsgLoadRenderItem*)pMsg;
			if( pLoadRenderItem != NULL )
			{
				if(pLoadRenderItem->m_iFlag == 0x00000001)// Preview
				{
					NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
					NXT_ThemeRenderer_GetRenderItem(m_hThemeRenderer, pLoadRenderItem->m_pID, NULL, pLoadRenderItem->m_pResource, 0, LoadThemeFileCallback, NULL);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Load RenderItem on preview theme renderer", __LINE__);
					NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
				}
				else
				{
					NXT_ThemeRenderer_AquireContext(m_hExportThemeRenderer);
					NXT_ThemeRenderer_GetRenderItem(m_hExportThemeRenderer, pLoadRenderItem->m_pID, NULL, pLoadRenderItem->m_pResource, 0, LoadThemeFileCallback, NULL);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d]Load RenderItem on export theme renderer", __LINE__);
					NXT_ThemeRenderer_ReleaseContext(m_hExportThemeRenderer, 0);
				}
			}
			SAFE_RELEASE(pLoadRenderItem);
			return MESSAGE_PROCESS_OK;
		}
            
#ifdef _ANDROID            
        case MESSAGE_FAST_PREVIEW_RENDER_INIT:
        {
            CNxMsgFastPreview* pFastPreviewInit = (CNxMsgFastPreview*)pMsg;

            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_START In(%d %d)", __LINE__,
                            pFastPreviewInit->m_iWidth, pFastPreviewInit->m_iHeight);
            if( pFastPreviewInit != NULL )
            {
                NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
                NXT_ThemeRenderer_DeinitFastPreview(m_hThemeRenderer);
                NXT_ThemeRenderer_InitFastPreview(m_hThemeRenderer, pFastPreviewInit->m_iWidth, pFastPreviewInit->m_iHeight, pFastPreviewInit->m_iMaxCount, NXT_RendererOutputType_Y2CrA_8888);
                NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
            }
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_START Out", __LINE__);
            SAFE_RELEASE(pMsg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_FAST_PREVIEW_RENDER_DEINIT:
        {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_STOP In", __LINE__);
            
            NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
            NXT_ThemeRenderer_DeinitFastPreview(m_hThemeRenderer);
            NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_STOP Out", __LINE__);
            SAFE_RELEASE(pMsg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_FAST_PREVIEW_ADD_FRAME:
        {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_ADD_FRAME In", __LINE__);
            
			CNxMsgFastPreviewTime* pPreviewMsg = (CNxMsgFastPreviewTime*)pMsg;

            if( m_hThemeRenderer == NULL )
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_ADD_FRAME Failed because renderer not init", __LINE__);
				pPreviewMsg->setProcessDone();
                SAFE_RELEASE(pPreviewMsg);
                return MESSAGE_PROCESS_OK;
            }
            
            CFrameInfo* pFrame = NULL;
            if( m_pProjectEffect && m_pProjectMng)
            {
                NXT_ThemeRenderer_SetWatermarkEffect(m_hThemeRenderer, m_pProjectEffect->m_pEffectID, m_pProjectMng->getDuration());
                NXT_ThemeRenderer_SetWatermarkEffect(m_hExportThemeRenderer, m_pProjectEffect->m_pEffectID, m_pProjectMng->getDuration());
            }
            
            NXBOOL	bEffectEnd		= FALSE;
            NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);

            unsigned int uiTime = 0;
            int iTrackSize = m_VideoTrack.size();
            if( iTrackSize > 0 )
            {
                for( int i = 0; i < iTrackSize; i++)
                {
                    pFrame = m_VideoTrack[i]->getFrameOutBuffer();
                    if( pFrame == NULL )
                    {
                        continue;
                    }
                    uiTime = pFrame->m_uiTime;
                    // m_VideoTrack[i]->applyTitleEffect(m_hThemeRenderer, uiTime);
                    pFrame->drawVideo((void*)m_hThemeRenderer, uiTime, 200);
                    m_VideoTrack[i]->DequeueFrameOutBuffer();
                }
            }

            NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);

			NXT_ThemeRenderer_GetFastPreviewBufferTime(m_hThemeRenderer, &(pPreviewMsg->m_uiStartTime), &(pPreviewMsg->m_uiEndTime));
			pPreviewMsg->setProcessDone();
			
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_ADD_FRAME Out", __LINE__);
            SAFE_RELEASE(pPreviewMsg);
            return MESSAGE_PROCESS_OK;
        }
        case MESSAGE_FAST_PREVIEW_TIME:
        {
            CNxMsgFastPreview* pFastMsg = (CNxMsgFastPreview*)pMsg;

            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_TIME In(%d)", __LINE__, pFastMsg->m_uiTime);

            if( m_hThemeRenderer == NULL )
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_TIME Failed because renderer not init", __LINE__);
                SAFE_RELEASE(pMsg);
                return MESSAGE_PROCESS_OK;
            }

            NXBOOL	bEffectEnd		= FALSE;
            NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);

            NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, pFastMsg->m_uiTime);
            NXT_ThemeRenderer_GLDrawForFastPreview(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0, pFastMsg->m_uiTime);
            NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 1);
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_FAST_PREVIEW_TIME Out", __LINE__);

            pFastMsg->m_nMsgType = MESSAGE_FAST_PREVIEW_TIME_DONE;
            pFastMsg->m_nResult = NEXVIDEOEDITOR_ERROR_NONE;
            m_pProjectMng->SendCommand(pFastMsg);
            SAFE_RELEASE(pMsg);
            return MESSAGE_PROCESS_OK;
        }
#endif
		case MESSAGE_RELEASE_RENDERER_CACHE:
		{
			CNxMsgReleaseRendererCache* rrc = (CNxMsgReleaseRendererCache*)pMsg;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_RELEASE_RENDERER_CACHE In", __LINE__);

			if( rrc != NULL )
			{
				for( int i = 0; i < rrc->getIDCounts(); i++ )
				{
//#ifdef _ANDROID // NESI-484
                    NXT_HThemeRenderer renderer = getRendererForUID(rrc->getID(i), true);
                    if(renderer){

                        NXT_ThemeRenderer_AquireContext(renderer);
                        NXT_ThemeRenderer_ReleaseTextureForVideoLayer(renderer, rrc->getID(i), 1);
                        NXT_ThemeRenderer_ReleaseContext(renderer, 0);
                    }
//#endif
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_RELEASE_RENDERER_CACHE %d", __LINE__, rrc->getID(i));
				}
			}

			SAFE_RELEASE(pMsg);
            // NXT_ThemeRenderer_AquireContext(renderer);
            // NXT_ThemeRenderer_ReleaseTextureForVideoLayer(renderer, 0xFFFFFFFF, 0);
            // NXT_ThemeRenderer_ReleaseContext(renderer, 0);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_RELEASE_RENDERER_CACHE Out", __LINE__);
			m_bAvailableCapture = FALSE;
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_DUMMY:
		{
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_DUMMY", __LINE__);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_DRAWINFO_LIST:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_DRAWINFO_LIST", __LINE__);

			// for NESA1298 crash in prepareSurface
			CNxMsgSetDrawInfoList* pInfos = (CNxMsgSetDrawInfoList*)pMsg;
			if( pInfos != NULL )
			{
				if( m_pDrawInfos != NULL )
				{
                    NXT_HThemeRenderer renderer = (NXT_HThemeRenderer)getCurrentVideoRenderer();
                    NXT_ThemeRenderer_AquireContext(renderer);
                    m_pDrawInfos->resetDrawInfos((void*)renderer);
                    NXT_ThemeRenderer_ReleaseContext(renderer, 0);
				}
				SAFE_RELEASE(m_pDrawInfos);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_DRAWINFO_LIST(%p %p)", __LINE__, m_pDrawInfos, pInfos->masterList);
				m_pDrawInfos = (CNexDrawInfoVec*)pInfos->masterList;
				SAFE_ADDREF(m_pDrawInfos);
				SAFE_RELEASE(pInfos);
				if( m_pDrawInfos != NULL )
					m_pDrawInfos->updateDrawInfo();
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_DRAWINFO_LIST Out(%p)", __LINE__, m_pDrawInfos);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_SET_DRAWINFO:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_DRAWINFO", __LINE__);

			CNxMsgSetDrawInfo* pInfos = (CNxMsgSetDrawInfo*)pMsg;
			if( pInfos != NULL )
			{
				CNexDrawInfo* pDrawInfo = (CNexDrawInfo*)pInfos->drawInfo;
				if( pDrawInfo != NULL ) {
					pDrawInfo->printDrawInfo();
				}
				m_pDrawInfos->updateDrawInfo(pDrawInfo);

				if (m_isGLOperationAllowed)
				{
					if( NXT_Error_None != NXT_ThemeRenderer_AquireContext(m_hThemeRenderer) ){
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_SET_DRAWINFO(Fail to aquireContext)", __LINE__);
						SAFE_RELEASE(pMsg);
						return MESSAGE_PROCESS_OK;
					}
					// m_pDrawInfos->resetDrawInfos((void*)m_hThemeRenderer);
					m_pDrawInfos->applyDrawInfo((void*)m_hThemeRenderer, m_uiCurrentTime);
					for( int i = 0;  i < m_VideoTrack.size(); i++)
					{
						if( m_VideoTrack[i]->isTrackTime(m_uiCurrentTime) )
						{
							m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, m_uiCurrentTime);
						}
					}
					NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, m_uiCurrentTime);
					NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
					NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 1);
				}
			}

			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
		}
		case MESSAGE_ENCODE_PROJECT_JPEG:
		{
#ifdef __APPLE__
			SAFE_RELEASE(pMsg);
			return MESSAGE_PROCESS_OK;
#else
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG In", __LINE__);

			CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
			if( pEditor == NULL )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG with editor handle error", __LINE__);
				SAFE_RELEASE(pMsg);
				return MESSAGE_PROCESS_OK;
			}

			CNxMsgEncodeProjectJpeg* pJepg = (CNxMsgEncodeProjectJpeg*)pMsg;
			if( pJepg == NULL )
			{
				ANativeWindow_release((ANativeWindow*)pJepg->m_pNativeWindow);
				pEditor->callbackCapture(0, 0, 0, 0);
				SAFE_RELEASE(pEditor);
				SAFE_RELEASE(pMsg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG with jpeg command error", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			if( m_bAvailableCapture == FALSE )
			{
				ANativeWindow_release((ANativeWindow*)pJepg->m_pNativeWindow);
				pEditor->callbackCapture(0, 0, 0, 0);
				SAFE_RELEASE(pEditor);
				SAFE_RELEASE(pJepg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG with not available capture", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			if( m_hThemeRenderer == NULL )
			{
				ANativeWindow_release((ANativeWindow*)pJepg->m_pNativeWindow);
				pEditor->callbackCapture(0, 0, 0, 0);
				SAFE_RELEASE(pEditor);
				SAFE_RELEASE(pJepg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG with render handle error", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			if( pJepg->m_pNativeWindow == NULL )
			{
				ANativeWindow_release((ANativeWindow*)pJepg->m_pNativeWindow);
				pEditor->callbackCapture(0, 0, 0, 0);
				SAFE_RELEASE(pEditor);
				SAFE_RELEASE(pJepg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG with native window error", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			int iWidth = ANativeWindow_getWidth((ANativeWindow*)pJepg->m_pNativeWindow);
			int iHeight = ANativeWindow_getHeight((ANativeWindow*)pJepg->m_pNativeWindow);

			if( NXT_ThemeRenderer_SetNativeWindow( m_hThemeRenderer, (ANativeWindow*)pJepg->m_pNativeWindow, (int)((float)iWidth * 1.0f), (int)((float)iHeight * 1.0f)) != NXT_Error_None )
			{
				ANativeWindow_release((ANativeWindow*)pJepg->m_pNativeWindow);
				pEditor->callbackCapture(0, 0, 0, 0);
				SAFE_RELEASE(pEditor);
				SAFE_RELEASE(pJepg);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG with set native window error", __LINE__);
				return MESSAGE_PROCESS_OK;
			}

			ANativeWindow_release((ANativeWindow*)pJepg->m_pNativeWindow);

			m_bLostNativeWindow = TRUE;
			SAFE_RELEASE(pEditor);

			if( pJepg->m_pInfo )
			{
				CNexDrawInfo* info = (CNexDrawInfo*)pJepg->m_pInfo;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG mID:%d mTrackID:%d mSubEffectID:%d", __LINE__, info->mID, info->mTrackID, info->mSubEffectID);
				NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
				NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, m_uiCurrentTime);
				NXT_ThemeRenderer_GLDrawSrc(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, info->mSubEffectID);
				NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 1);
				SAFE_RELEASE(pJepg);
				return MESSAGE_PROCESS_OK;
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG Out", __LINE__);
			
			NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
			NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, m_uiCurrentTime);
			NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
			NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 1);

			SAFE_RELEASE(pJepg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_ENCODE_PROJECT_JPEG Out", __LINE__);
			return MESSAGE_PROCESS_OK;
#endif
		}
		case MESSAGE_WAIT_COMMAND:
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_WAIT_COMMAND", __LINE__);

			CNxMsgWaitCmd* pWait = (CNxMsgWaitCmd*)pMsg;
			if( pWait != NULL )
			{
				pWait->setProcessDone();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_WAIT_COMMAND Event done", __LINE__);
			}
			SAFE_RELEASE(pMsg);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] MESSAGE_WAIT_COMMAND Out", __LINE__);
			return MESSAGE_PROCESS_OK;
		}
		default:
		{
			break;
		}
	};
	return CNEXThreadBase::ProcessCommandMsg(pMsg);
}

void CNEXThread_VideoRenderTask::validCheckTrack(unsigned int uiTime)
{
#ifdef NO_OVERLAPPED_TRANSITION_CODE
	VideoTrackVecIter i = m_VideoTrack.begin();
	while( i != m_VideoTrack.end() )
	{
		CVideoTrackInfo* pTrackItem = (CVideoTrackInfo*)*i;
		if( pTrackItem == NULL ) {
			i++;
			continue;
		}

		if( pTrackItem->m_uiEndTime < uiTime && pTrackItem->GetRefCnt() == 1 )
		{
			m_VideoTrack.erase(i);
			// mantis 5813
			// ZIRA 1658

			// kbsong 20130808 : change for mantis 7722
			if( getState() == PLAY_STATE_RUN && pTrackItem->isVideoClip() && m_iVideoStarted == 0 )
			{
				m_iVisualClipCountAtStartTime--;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Send Video Start event becuse first clip delete without display %d", __LINE__, m_iVisualClipCountAtStartTime);
				sendVideoStarted();
			}

#ifdef __APPLE__
			// added by eric.
			// 2016.7.21
			pTrackItem->DequeueAllFrameOutBuffer();
#endif

#if 1 // case drawInfo
            NXT_HThemeRenderer renderer = getRendererForUID(pTrackItem->m_uiTrackID, true);
            if(renderer) {

                NXT_ThemeRenderer_AquireContext(renderer);
                pTrackItem->resetDrawInfos((void*)renderer)
                NXT_ThemeRenderer_ReleaseTextureForTrackID(renderer, pTrackItem->m_uiTrackID, 0);
                NXT_ThemeRenderer_ReleaseContext(renderer, 0);

            }
#endif

			CClipList* pClipList = m_pProjectMng->getClipList();
			if( getState() == PLAY_STATE_RECORD && pTrackItem->isLastTrack(pClipList->getClipTotalTime()) )
			{
				m_bLastClipRendered = TRUE;
				m_pFileWriter->setLastTrackEnded(TRUE);
			}
			SAFE_RELEASE(pClipList);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] validCheckTrack remove Video ID(%d) TrackEnd(%d) Cur(%d) reaminTrackSize(%zu)", __LINE__, pTrackItem->m_uiTrackID, pTrackItem->m_uiEndTime, m_uiCurrentTime, m_VideoTrack.size());
			SAFE_RELEASE(pTrackItem);
			i = m_VideoTrack.begin();
			continue;
		}
		i++;
	}
#endif
}


NXBOOL	CNEXThread_VideoRenderTask::existDrawItem(unsigned int uiTime)
{
	if( m_VideoTrack.size() == 0 )
		return FALSE;

	dropFirstFrameOnTrack();

	if( m_iVisualClipCountAtStartTime > 0 && m_iVisualClipCountAtStartTime > m_VideoTrack.size() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video regist(%d %zu)", __LINE__, m_iVisualClipCountAtStartTime, m_VideoTrack.size());
		nexSAL_TaskSleep(20);
		return FALSE;
	}
	if( m_iVisualClipCountAtStartTime > 0 )
	{
		for(int i = 0; i < m_VideoTrack.size(); i++)
		{
			if( m_VideoTrack[i]->isTrackTime(uiTime) && m_VideoTrack[i]->isTrackStarted() == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video Start(%d %zu)", __LINE__, m_iVisualClipCountAtStartTime, m_VideoTrack.size());
				nexSAL_TaskSleep(20);
				return FALSE;
			}
		}
	}
	m_iVisualClipCountAtStartTime = -1;

	if( m_iVisualLayerCountAtStartTime > 0 && m_iVisualLayerCountAtStartTime > m_VideoLayer.size() )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video layer regist(%d %zu)", __LINE__, m_iVisualLayerCountAtStartTime, m_VideoLayer.size());
		nexSAL_TaskSleep(20);
		return FALSE;
	}

	if( m_iVisualLayerCountAtStartTime > 0 )
	{
		for(int i = 0; i < m_VideoLayer.size(); i++)
		{
			if( m_VideoLayer[i]->isTrackTime(uiTime) && m_VideoLayer[i]->isTrackStarted() == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video layer Start(%d %zu)", __LINE__, m_iVisualLayerCountAtStartTime, m_VideoLayer.size());
				nexSAL_TaskSleep(20);
				return FALSE;
			}
		}
	}
	m_iVisualLayerCountAtStartTime = -1;

	CClipList* pClipList = m_pProjectMng->getClipList();
	unsigned int uCount = 0, uIndex = pClipList->getVisualIndex();
	unsigned int* pIDs = pClipList->getVisualClipIDsAtTime(uiTime, &uCount);
	CNEXThread_AudioRenderTask* pAudioRender = CNexProjectManager::getAudioRenderer();
    
	if(uCount)    
	{
		int iCheck = 0;          
		for(int i=0; i<uCount; i++)
		{
			for(int j=0; j<m_VideoTrack.size(); j++)		
			{
				if(pIDs[i] == m_VideoTrack[j]->m_uiTrackID)
				{
					iCheck++;
					break;
				}
			}
		}

		if(iCheck < uCount)
		{
			if(m_bWaitForVisual == FALSE)
			{
				pAudioRender->setPauseForVisual();
				m_bWaitForVisual = TRUE;
			}
			nexSAL_TaskSleep(10);
			pClipList->setVisualIndex(uIndex);            
			SAFE_RELEASE(pAudioRender);
			SAFE_RELEASE(pClipList);	            
			return FALSE;
		}
	}

	if(m_bWaitForVisual)
	{
		pAudioRender->setResumeForVisual();
		m_bWaitForVisual = FALSE;
	}
	SAFE_RELEASE(pAudioRender);
	SAFE_RELEASE(pClipList);

	int iStatus = 1;
	int iTrackSize = m_VideoTrack.size() > 2 ? 2 : m_VideoTrack.size();

	if( m_pDrawInfos != NULL )
	{
		iStatus = 0;
		for( int i = 0; i < m_VideoTrack.size(); i++ )
		{
			// 0 : draw Time
			// 1 : no available frame
			// 2 : fast frame
			// 3 : before track time
			// 4 : after track time
			// 5 : after track time of last clip
			// 6 : track not started
			// 7 : track decode end
			int iRet = m_VideoTrack[i]->existDrawFrame(uiTime, FALSE);
			if( iRet != 0 ) {
				if( iRet == 3 || iRet == 4 ) {
					iStatus = 0;
				}
				else {
					iStatus = iRet;
					break;					
				}
			}
		}
	}
	else
	{
		if( iTrackSize ==  1 )
		{
			iStatus = m_VideoTrack[0]->existDrawFrame(uiTime, FALSE);
		}
		else
		{
			iStatus = m_VideoTrack[0]->existDrawFrame(uiTime, FALSE);
			int iSecondRet = m_VideoTrack[1]->existDrawFrame(uiTime, FALSE);


			if( m_VideoTrack[0]->isVideoClip() && iStatus == 6 )
			{
			}
			else if( iSecondRet == 3 || iSecondRet == 6)
			{
			}
			// add this code for simple template export performance issue 20161129
			else if( iStatus == 4 && m_VideoTrack[0]->isImageClip() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] First track ended and Send delete track", __LINE__);

				CNxMsgDeleteTrack* pMsg = new CNxMsgDeleteTrack(m_VideoTrack[0]->m_uiTrackID);
				if( pMsg )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] Send Msg Image Clip delete Track", __LINE__);
					SendCommand(pMsg);
					SAFE_RELEASE(pMsg);
				}
			}
			else
			{
				iStatus = iSecondRet;
			}
		}
	}


#ifdef FOR_VIDEO_LAYER_FEATURE
	for( int i = 0;  i < m_VideoLayer.size(); i++){
		m_VideoLayer[i]->existDrawFrame(uiTime, FALSE);     
	}
#endif

	if(iStatus == 0 && m_iPreviewFPS == 0)
	{
		int bCheckDrawForImage = 0;
		if(iTrackSize == 2 && m_VideoTrack[1]->isImageClip() && m_VideoTrack[1]->isTrackTime(uiTime) && m_iVideoStarted)
			bCheckDrawForImage = 1;
		else if(m_VideoTrack[0]->isImageClip() && m_VideoTrack[0]->isTrackTime(uiTime) && m_iVideoStarted)
			bCheckDrawForImage = 1;

		if(bCheckDrawForImage)
		{
			CClipList* pClipList = m_pProjectMng->getClipList();

			if(m_uiLastTime != 0 && uiTime < m_uiLastTime + 31 && uiTime + 31 < pClipList->getClipTotalTime())
				iStatus = 1;

			SAFE_RELEASE(pClipList);
		}
	}

	// 0 : draw Time
	// 1 : no available frame
	// 2 : fast frame
	// 3 : before track time
	// 4 : after track time
	// 5 : after track time of last clip
	// 6 : track not started
	// 7 : track decode end
	switch(iStatus)
	{
		case 0:
			return TRUE;
		case 2:
			if( m_iVideoStarted == 0 )
			 	return TRUE;
			break;
		default :
			break;
	}
	return FALSE;
}

void CNEXThread_VideoRenderTask::ProcessIdleState()
{
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] ProcessPreviewState(%d) In", __LINE__, m_isDrawEnd);
	m_iVideoStarted = 0;
	m_VideoProcessOk = 0;
	nexSAL_TaskSleep(5);
	return;
}

void CNEXThread_VideoRenderTask::ProcessPlayState()
{
	m_uiCurrentTime = CNexProjectManager:: getCurrentTimeStamp();
	if( m_uiLastTime == m_uiCurrentTime )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] m_uiLastTime == m_uiCurrentTime (%d)", __LINE__, m_uiCurrentTime);
		nexSAL_TaskSleep(5);
		return;
	}

	validCheckTrack(m_uiCurrentTime);

	while( m_bIsWorking && GetMsgSize() == 0 )
	{
		m_uiCurrentTime = CNexProjectManager:: getCurrentTimeStamp();
		if( m_uiLastTime == m_uiCurrentTime )
		{
			nexSAL_TaskSleep(5);
			continue;
		}
		
		unsigned int uiTime = (m_iPreviewFPS)?m_iPreviewTime:m_uiCurrentTime;
		NXBOOL bExistDrawItem = existDrawItem(uiTime);

		if( m_iVideoStarted == FALSE && bExistDrawItem == FALSE )
		{
			nexSAL_TaskSleep(10);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] wait draw items started", __LINE__);
			return;
		}
		else if( m_iVideoStarted && bExistDrawItem == FALSE )
		{
			if(m_iPreviewFPS == 0)
			{
#ifdef FOR_VIDEO_LAYER_FEATURE
				if(m_VideoLayer.size() == 0)
				{
					nexSAL_TaskSleep(1);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 1, "[VRTask.cpp %d] wait draw item while playing", __LINE__);
					return;
				}
				else
				{
					unsigned int uRet = 1;
					if(m_iVideoStarted)
						uRet = drawPlayLayer(m_uiCurrentTime);
					
					if(uRet)
						nexSAL_TaskSleep(1);
					return;
				}
#else
				nexSAL_TaskSleep(1);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] wait draw item while playing", __LINE__);
				return;
#endif
			}
		}

		if(m_iPreviewFPS == 0)
		{
			drawPlay(m_uiCurrentTime);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] ProcessPlayState(%d %d)", __LINE__, m_uiCurrentTime, m_iPreviewTime);

			if(m_uiCurrentTime >= m_iPreviewTime)
			{
				if(bExistDrawItem == FALSE)
				{
					drawEffect(m_uiCurrentTime);
				}
				else
				{
					drawPlay(m_uiCurrentTime);
				}
				m_iPreviewTime = m_uiCurrentTime + 1000 / m_iPreviewFPS;
			}
			else
			{
				int iSleep = (m_iPreviewTime - m_uiCurrentTime)/2 + 1;
				nexSAL_TaskSleep(iSleep);
			}
		}
	}
}

// 0 : draw Time
// 1 : draw skip without increment time.
// 2 : draw skip without increment time because track not started 
// 3 : draw skip without increment time because track ended
// 4 : draw skip with increament time
// 5 : draw skip without increment time because track not registered.
// 6 : Force draw with increment time because project end;
int	CNEXThread_VideoRenderTask::waitDrawItem(unsigned int uiTime)
{
	CFrameInfo* pFrame = NULL;
	if( m_VideoTrack.size() == 0 )
	{
        if( m_bLastClipRendered )
		{
			// 6 : Force draw with increment time because project end;
			return 6;
		}
		// 5 : draw skip without increment time because track not registered.
		return 5;
	}

    if( m_iVisualClipCountAtStartTime > 0 && m_iVisualClipCountAtStartTime > m_VideoTrack.size() )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video regist(%d %zu)", __LINE__, m_iVisualClipCountAtStartTime, m_VideoTrack.size());
        nexSAL_TaskSleep(20);
        return 2;
    }

    if( m_iVisualClipCountAtStartTime > 0 )
    {
        for(int i = 0; i < m_VideoTrack.size(); i++)
        {
            if( m_VideoTrack[i]->isTrackTime(uiTime) && m_VideoTrack[i]->isTrackStarted() == FALSE )
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video Start(%d %zu)", __LINE__, m_iVisualClipCountAtStartTime, m_VideoTrack.size());
                nexSAL_TaskSleep(20);
                return 2;
            }
        }
    }
    m_iVisualClipCountAtStartTime = -1;
    
    if( m_iVisualLayerCountAtStartTime > 0 && m_iVisualLayerCountAtStartTime > m_VideoLayer.size() )
    {
        nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video layer regist(%d %zu)", __LINE__, m_iVisualLayerCountAtStartTime, m_VideoLayer.size());
        nexSAL_TaskSleep(20);
        return 2;
    }
    
    if( m_iVisualLayerCountAtStartTime > 0 )
    {
        for(int i = 0; i < m_VideoLayer.size(); i++)
        {
            if( m_VideoLayer[i]->isTrackTime(uiTime) && m_VideoLayer[i]->isTrackStarted() == FALSE )
            {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] wait video layer Start(%d %zu)", __LINE__, m_iVisualLayerCountAtStartTime, m_VideoLayer.size());
                nexSAL_TaskSleep(20);
                return 2;
            }
        }
    }
    m_iVisualLayerCountAtStartTime = -1;

	CClipList* pClipList = m_pProjectMng->getClipList();
	unsigned int uCount = 0, uIndex = pClipList->getVisualIndex();    
	unsigned int* pIDs = pClipList->getVisualClipIDsAtTime(uiTime, &uCount);
	CNEXThread_AudioRenderTask* pAudioRender = CNexProjectManager::getAudioRenderer();

	if(uCount)    
	{
		int iCheck = 0;
		for(int i=0; i<uCount; i++)
		{
			for(int j=0; j<m_VideoTrack.size(); j++)
			{
				if(pIDs[i] == m_VideoTrack[j]->m_uiTrackID)
				{
					iCheck++;
					break;
				}
			}
		}

		if(iCheck < uCount)
		{
			if(m_bWaitForVisual == FALSE)
			{
				pAudioRender->setPauseForVisual();
				m_bWaitForVisual = TRUE;
			}
			nexSAL_TaskSleep(10);
			pClipList->setVisualIndex(uIndex);            
			SAFE_RELEASE(pAudioRender);
			SAFE_RELEASE(pClipList);
			return 1;
		}
	}

	if(m_bWaitForVisual)
	{
		pAudioRender->setResumeForVisual();
		m_bWaitForVisual = FALSE;
	}
	SAFE_RELEASE(pAudioRender);
	SAFE_RELEASE(pClipList);

	dropFirstFrameOnTrack();

	int iTrackSize = 0;
	int iDraw = 1;
	int iStatus = 1;
	if( m_pDrawInfos != NULL )
	{
		iStatus = 0;
		for( int i = 0; i < m_VideoTrack.size(); i++ )
		{
			// 0 : draw Time
			// 1 : no available frame
			// 2 : fast frame
			// 3 : before track time
			// 4 : after track time
			// 5 : after track time of last clip
			// 6 : track not started
			// 7 : track decode end

			int iRet = m_VideoTrack[i]->existDrawFrame(uiTime, TRUE);
			if( iRet == 3 || iRet == 4 || iRet == 5 ) {

			}
			else if( iRet != 0 ) {
				iStatus = iRet;
				break;
			}
		}
	}
	else
	{
		iTrackSize = m_VideoTrack.size() > 2 ? 2 : m_VideoTrack.size();
		if( iTrackSize ==  1 )
		{
			iStatus = m_VideoTrack[0]->existDrawFrame(uiTime, TRUE);
			if( iStatus == 7 )
			{
				iStatus = 2;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] ID(%d) Track dec end and forced render", __LINE__, m_VideoTrack[0]->m_uiTrackID);
			}
		}
		else
		{
			iStatus = m_VideoTrack[0]->existDrawFrame(uiTime, TRUE);
			int iSecondRet = m_VideoTrack[1]->existDrawFrame(uiTime, TRUE);

			if( iStatus == 7 && iSecondRet == 3 )
			{
				iStatus = 3;
			}
			else if( iSecondRet == 3 || iSecondRet == 6 )
			{
			}
			// add this code for simple template export performance issue 20161129
			else if( iStatus == 4 && m_VideoTrack[0]->isImageClip() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] First track ended and Send delete track", __LINE__);

				CNxMsgDeleteTrack* pMsg = new CNxMsgDeleteTrack(m_VideoTrack[0]->m_uiTrackID);
				if( pMsg )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] Send Msg Image Clip delete Track", __LINE__);
					SendCommand(pMsg);
					SAFE_RELEASE(pMsg);
				}
			}
			else
			{
				iStatus = iSecondRet;
			}
		}

		if(m_VideoTrack.size() > 2)
		{
			int iTemp = 0;
			for(int i=2; i<m_VideoTrack.size(); i++)
			{
				if(m_VideoTrack[i]->isTrackTime(uiTime))
				{
					return 1;
				}
			}
		}

		for( int i = 0; i < m_VideoLayer.size(); i++ )
		{
			if( m_VideoLayer[i]->isTrackTime(uiTime) && m_VideoLayer[i]->isTrackStarted() == FALSE )
			{
				return 2;
			}
			if( m_VideoLayer[i]->isTrackTime(uiTime))
			{
				m_VideoLayer[i]->existDrawFrame(uiTime, TRUE);
			}
		}

		int bNeedSleep = 0;

		if( iTrackSize ==  1 )
		{
			if(m_VideoTrack[0]->isImageClip())
				bNeedSleep = 1;
		}
		else
		{
			if(m_VideoTrack[0]->isImageClip() && m_VideoTrack[1]->isImageClip())
				bNeedSleep = 1;
		}

		if(bNeedSleep)
		{
			if(m_VideoLayer.size() != 0 )
			{
				nexSAL_TaskSleep(5);
			}
		}
	}


	// 0 : draw Time
	// 1 : no available frame
	// 2 : fast frame
	// 3 : before track time
	// 4 : after track time
	// 5 : after track time of last clip
	// 6 : track not started
	// 7 : track decode end
	switch(iStatus)
	{
		case 0:
		case 2:
			// 0 : draw Time
			iDraw = 0;
			break;
		case 1:
		case 4:
			// 1 : draw skip without increment time.
			iDraw = 1;
			break;
		case 6:
			// 2 : draw skip without increment time because track not started 
			iDraw = 2;
			break;
		case 7:
			// 3 : draw skip without increment time because track ended
			iDraw = 3;
			break;
		case 3:
 			// 4 : draw skip with increament time
			iDraw = 4;
			break;
		case 5:
			// 6 : Force draw with increment time because project end
 			iDraw = 6;
			break;
	}
	return iDraw;	
}

void CNEXThread_VideoRenderTask::ProcessRecordState()
{
#ifdef EXPORT_DIRECT_TEST
	if( m_VideoTrack.size() == 1 && m_VideoTrack[0]->isVideoClip() )
	{
		if( m_VideoTrack[0]->m_pTitle == NULL || m_VideoTrack[0]->m_pTitleEffect == NULL )
		{
			if ( m_pFileWriter->waitEmptyBuffer() == FALSE )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] File Writer Task is working", __LINE__);
				return;
			}

			int iWaitTime = 30;
			while(iWaitTime > 0 )
			{
				CFrameInfo* pFrame = m_VideoTrack[0]->getFrameOutBuffer();
				if( pFrame ) 
				{
					m_uiCurrentTime += 33;
					m_uiLastTime = m_uiCurrentTime;

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] Video Frame Direct Write FrameTime(%d) CurrentTime(%d)", 
						__LINE__, pFrame->m_uiTime, m_uiLastTime);
					writeVideoYUVFrame(m_uiLastTime, pFrame->m_pY, m_uiEncoderFrameSize);
					m_VideoTrack[0]->DequeueFrameOutBuffer();
					if( m_iVideoStarted == 0)
						sendVideoStarted();
					return;
				}
				nexSAL_TaskSleep(10);
				iWaitTime -= 10;
			}
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] get Video Frame Write failed while recording", __LINE__);
			return;
		}
	}
#endif

	if( m_iVideoStarted == FALSE )
	{
		if( m_VideoTrack.size() == 0 || m_VideoTrack[0]->isTrackStarted() == FALSE )
		{
			nexSAL_TaskSleep(20);
			return;
		}

		if( m_pDrawInfos != NULL ) 
		{
			CClipList* pClipList = m_pProjectMng->getClipList();
			if( pClipList != NULL )
			{
				int allClipCount = pClipList->getAllVisualClipCountAtTime(0);
				int trackSize = m_VideoTrack.size();
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] Prepare track(%d %d)", __LINE__, allClipCount, trackSize);
				if( allClipCount > m_VideoTrack.size() )
				{
					nexSAL_TaskSleep(20);
					SAFE_RELEASE(pClipList);
					return;
				}
				SAFE_RELEASE(pClipList);
			}
		}
	}

	validCheckTrack(m_uiCurrentTime);

	while(m_bIsWorking && GetMsgSize() == 0)
	{
		// 0 : draw Time
		// 1 : draw skip without increment time.
		// 2 : draw skip without increment time because track not started 
		// 3 : draw skip without increment time because track ended
		// 4 : draw skip with increament time
		// 5 : draw skip without increment time because track not registered.
		// 6 : Force draw with increment time because project end;
		int iRet = 0;
		if ( m_pFileWriter != NULL && !m_pFileWriter->isReadyForMoreVideo()) {
			// Flow control: file writer needs to handle audio stream before another video sample
			iRet = 1;
        }
		if (iRet == 0) {
			iRet = waitDrawItem(m_uiCurrentTime );
		}
        
		if( iRet == 0 )
		{
		}
	 	else if( iRet == 1 )
		{
			if( nexSAL_GetTickCount() - m_uiLastDrawTick > 5000 )
			{
				m_uiLastDrawTick = nexSAL_GetTickCount();
				m_uiVideoExportTotalCount++;
				m_uiCurrentTime = (m_uiVideoExportTotalCount * m_uiVideoExportInterval) / 100;
			}
			// 1 : draw skip without increment time.

			CNEXThread_PlayClipTask* pPlayClipTask = m_pProjectMng->getPlayClipTask();
			if(pPlayClipTask)
			{
				CNxMsgUpdateClip4Export* pMsg = new CNxMsgUpdateClip4Export(m_uiCurrentTime);
				if(pMsg)
				{
					pPlayClipTask->SendCommand(pMsg);
					SAFE_RELEASE(pMsg);
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Send CNxMsgUpdateClip4Export", __LINE__);
				}
			}
			SAFE_RELEASE(pPlayClipTask);
			nexSAL_TaskSleep(10);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] wait draw item while playing(cur:%d)", __LINE__, m_uiCurrentTime);
			return;
		}
		else if( iRet == 2 )
		{
			if( nexSAL_GetTickCount() - m_uiLastDrawTick > 10000 )
			{
				m_uiLastDrawTick = nexSAL_GetTickCount();
				m_uiVideoExportTotalCount++;
				m_uiCurrentTime = (m_uiVideoExportTotalCount * m_uiVideoExportInterval) / 100;
			}		
			nexSAL_TaskSleep(30);
			// 2 : draw skip without increment time because track not started 
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Current track not started(cur:%d)", __LINE__, m_uiCurrentTime);
			return;
		}
		else if( iRet == 3 )
		{
			if( nexSAL_GetTickCount() - m_uiLastDrawTick > 5000 )
			{
				m_uiLastDrawTick = nexSAL_GetTickCount();
				m_uiVideoExportTotalCount++;
				m_uiCurrentTime = (m_uiVideoExportTotalCount * m_uiVideoExportInterval) / 100;
			}		
			nexSAL_TaskSleep(30);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Current track ended(cur:%d)", __LINE__, m_uiCurrentTime);
			return;
		}
		else if( iRet == 4 )
		{
			int i =0, iRet=0;
			for(i=0; i<m_VideoTrack.size();i++)
			{
				CClipVideoRenderInfo* pRenderInfo = m_VideoTrack[i]->getActiveRenderInfo(m_uiCurrentTime);

				if(pRenderInfo)
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] fast skip draw item while playing FORCE RENDER(cur:%d)", __LINE__, m_uiCurrentTime);
					goto FORCE_RENDER;
				}
			}

			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] fast skip draw item while playing(cur:%d)", __LINE__, m_uiCurrentTime);
			continue;
		}
		else if( iRet == 5 )
		{
			if( nexSAL_GetTickCount() - m_uiLastDrawTick > 10000 )
			{
				m_uiLastDrawTick = nexSAL_GetTickCount();
				m_uiVideoExportTotalCount++;
				m_uiCurrentTime = (m_uiVideoExportTotalCount * m_uiVideoExportInterval) / 100;
			}		
			// 5 : draw skip without increment time because track not registered.
			nexSAL_TaskSleep(30);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Draw skip because track not registered(cur:%d)", __LINE__, m_uiCurrentTime);
			return;
		}
		else if( iRet == 6 )
		{
			// 6 : Force draw with increment time because project end;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Force draw because play end(cur:%d)", __LINE__, m_uiCurrentTime);
		}
		
FORCE_RENDER:
		drawExport(m_uiCurrentTime);

		m_uiVideoExportTotalCount++;
		m_uiCurrentTime = (m_uiVideoExportTotalCount * m_uiVideoExportInterval) / 100;
		m_uiLastDrawTick = nexSAL_GetTickCount();
		// m_uiCurrentTime += 33;
	}
}

void CNEXThread_VideoRenderTask::ProcessPauseState()
{
	nexSAL_TaskSleep(50);
}

void CNEXThread_VideoRenderTask::sendVideoStarted()
{
	if( m_iVideoStarted == 1 ) return;
		
	if( m_pProjectMng == NULL )
	{
		m_pProjectMng = CNexProjectManager::getProjectManager();
	}
	m_pProjectMng->SendSimpleCommand(MESSAGE_VIDEO_STARTED);
	m_iVideoStarted = 1;
}

void CNEXThread_VideoRenderTask::sendInterruptExportEvent(NEXVIDEOEDITOR_ERROR error)
{
	if( m_bInterruptExport ) 
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Alreasy send interrupt export event", __LINE__);
		return;
	}
	m_bInterruptExport = TRUE;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Send interrupt export event", __LINE__);
	CNexProjectManager::sendEvent(MESSAGE_ENCODE_PROJECT_DONE, error);
	return;
}

void CNEXThread_VideoRenderTask::checkVideoProcessOk(){

	if( m_VideoProcessOk == 1 ) return;
		
	if( m_pProjectMng == NULL )
	{
		m_pProjectMng = CNexProjectManager::getProjectManager();
	}
	CClipList* pClipList = m_pProjectMng->getClipList();
	for(int i = 0; i < 100; ++i){

		if(pClipList->checkFaceDetectProcessedAtTime(0))
			break;
		nexSAL_TaskSleep(10);
	}
	SAFE_RELEASE(pClipList);
	m_VideoProcessOk = 1;
}

void CNEXThread_VideoRenderTask::checkVideoProcessOk(unsigned int uiTime){
		
	if( m_pProjectMng == NULL )
	{
		m_pProjectMng = CNexProjectManager::getProjectManager();
	}
	CClipList* pClipList = m_pProjectMng->getClipList();
	for(int i = 0; i < 100; ++i){

		if(pClipList->checkFaceDetectProcessedAtTime(uiTime))
			break;
		nexSAL_TaskSleep(10);
	}
	SAFE_RELEASE(pClipList);
}

int	CNEXThread_VideoRenderTask::setRendererPosition(int iLeft, int iTop, int iRight, int iBottom)
{
	m_RD.left		= iLeft;
	m_RD.top		= iTop;
	m_RD.right		= iRight;
	m_RD.bottom		= iBottom;

	// ?????? ?̹?? ?ٽ??׸?
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_VideoRenderTask::writeVideoYUVFrame(unsigned int uiCTS, unsigned char* pFrame, unsigned int uiFrameSize)
{
	unsigned int uiDuration	= 0;
	unsigned int uiSize		= 0;
	if( m_pFileWriter->setBaseVideoFrame(uiCTS, pFrame, uiFrameSize, &uiDuration, &uiSize) == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] Video Frame Write failed", __LINE__);
	}

	return NEXVIDEOEDITOR_ERROR_NONE;
}

// ù ȿ?? ???? ??ߴ????????ӽ÷?ó??? ???
void CNEXThread_VideoRenderTask::clearGLRender()
{
	unsigned int uiTick = nexSAL_GetTickCount();

	unsigned char* pTempBuff = (unsigned char*)nexSAL_MemAlloc(SUPPORTED_ENCODE_WIDTH * SUPPORTED_ENCODE_HEIGHT * 3 / 2);

	if( pTempBuff == NULL ) return;

	unsigned char* pY = pTempBuff;
	unsigned char* pUV = pTempBuff + (SUPPORTED_ENCODE_WIDTH * SUPPORTED_ENCODE_HEIGHT);

	memset(pY, 0x00, SUPPORTED_ENCODE_WIDTH * SUPPORTED_ENCODE_HEIGHT);
	memset(pUV, 0x80, SUPPORTED_ENCODE_WIDTH * SUPPORTED_ENCODE_HEIGHT/2);
	
	NXT_ThemeRenderer_ClearTransitionEffect(m_hThemeRenderer);
	NXT_ThemeRenderer_ClearClipEffect(m_hThemeRenderer);

	NXT_ThemeRenderer_UploadTexture(	m_hThemeRenderer,
										0,
										NXT_TextureID_Video_1,
										SUPPORTED_ENCODE_WIDTH,
										SUPPORTED_ENCODE_HEIGHT,
										SUPPORTED_ENCODE_WIDTH,
										NXT_PixelFormat_NV12,
										pY,
										pUV,
										NULL,
										0, 
										0,
										0,
										0,
										NULL);

	NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);

	nexSAL_MemFree(pTempBuff);

	uiTick = nexSAL_GetTickCount() - uiTick;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Clear GL Render Time(%d)", __LINE__, uiTick);
}

void CNEXThread_VideoRenderTask::clearUploadTexture(int iTextureID, int iWidth, int iHeight, NXBOOL bAquireContext)
{
	if( m_hThemeRenderer == NULL ) return;
	
	unsigned char* pTextureBuffer = (unsigned char*)nexSAL_MemAlloc(iWidth*iHeight*3/2);
	if( pTextureBuffer )
	{
		memset(pTextureBuffer, 0x00, iWidth*iHeight);
		memset(pTextureBuffer+(iWidth*iHeight), 0x80, iWidth*iHeight/2);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] clearUploadTexture(%d w:%d h:%d)", __LINE__, iTextureID == 1 ? NXT_TextureID_Video_1 : NXT_TextureID_Video_2, iWidth, iHeight);

		if( bAquireContext )
		{
			NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
			NXT_ThemeRenderer_UploadTexture(	m_hThemeRenderer,
												0,
												iTextureID == 1 ? NXT_TextureID_Video_1 : NXT_TextureID_Video_2,
												iWidth,
												iHeight,
												iWidth,
												NXT_PixelFormat_NV12,
												pTextureBuffer,
												pTextureBuffer+(iWidth*iHeight),
												NULL, 
												0, 
												0,
												0,
												0, 
												NULL);

			NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
		}
		else
		{
			NXT_ThemeRenderer_UploadTexture(	m_hThemeRenderer,
												0,
												iTextureID == 1 ? NXT_TextureID_Video_1 : NXT_TextureID_Video_2,
												iWidth,
												iHeight,
												iWidth,
												NXT_PixelFormat_NV12,
												pTextureBuffer,
												pTextureBuffer+(iWidth*iHeight),
												NULL,
												0, 
												0,
												0,
												0,
												NULL);
		}
		nexSAL_MemFree(pTextureBuffer);
		
	}
}

int CNEXThread_VideoRenderTask::applyEffect(CVideoTrackInfo* pTrackInfo, unsigned int uiTime)
{
	int iRet = pTrackInfo->applyClipEffect(getCurrentVideoRenderer(), uiTime);
	pTrackInfo->applyTitleEffect(getCurrentVideoRenderer(), uiTime);
	return iRet;
}

int	CNEXThread_VideoRenderTask::setThemeRender(void* pRender)
{
    if(pRender == NULL){

        nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setThemeRender failed because (pRender is null)(%p)", __LINE__, pRender);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
    }
    CNxMsgSetThemeRender* msg = new CNxMsgSetThemeRender(pRender, 0);
    SendCommand(msg);
    SAFE_RELEASE(msg);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

int CNEXThread_VideoRenderTask::setMapper(std::map<int, int>* pmapper) {

    CNxMsgSetMapper* msg = new CNxMsgSetMapper(pmapper);
    SendCommand(msg);
    SAFE_RELEASE(msg);
    return NEXVIDEOEDITOR_ERROR_NONE;
}

int	CNEXThread_VideoRenderTask::setExportThemeRender(void* pRender)
{
	if( pRender == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setExportThemeRender failed because (pRender is null)(%p)", __LINE__, pRender);
		return NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED;
	}
	
	CNxMsgSetThemeRender* msg = new CNxMsgSetThemeRender(pRender, 1);
    SendCommand(msg);
    SAFE_RELEASE(msg);
	return NEXVIDEOEDITOR_ERROR_NONE;
}

NXBOOL CNEXThread_VideoRenderTask::isLostNativeWindow()
{
#if defined(_ANDROID)
    CNxMsgCheckIsLostNativeWindow* msg = new CNxMsgCheckIsLostNativeWindow();
    SendCommand(msg);
    msg->waitProcessDone(NEXSAL_INFINITE);
    NXBOOL ret = msg->m_Result;
    SAFE_RELEASE(msg);
	return ret;
#endif
	return FALSE;
}

NXBOOL CNEXThread_VideoRenderTask::prepareSurface(void* pSurface, float fScaleFactor){

    CNxMsgPrepareSurfaceRenderer* msg = new CNxMsgPrepareSurfaceRenderer(pSurface, fScaleFactor);
    SendCommand(msg);
    msg->waitProcessDone(NEXSAL_INFINITE);
    NXBOOL ret = msg->m_Result;
    SAFE_RELEASE(msg);
    return ret;
}

NXBOOL CNEXThread_VideoRenderTask::prepareSurface_internal(void* pSurface, float fScaleFactor)
{
#if defined(_ANDROID)

    if(m_pPlaybackCacheWorker){

        m_pPlaybackCacheWorker->stopCaching();
        m_pPlaybackCacheWorker->End(1000);
    }

	m_bAvailableCapture = FALSE;
	if( m_hThemeRenderer )
	{
		int iWidth =0;
		int iHeight =0;
		int iFormat =0;
	
		if( pSurface != NULL)
		{
			iWidth = ANativeWindow_getWidth((ANativeWindow*)pSurface);
			iHeight = ANativeWindow_getHeight((ANativeWindow*)pSurface);
			iFormat = ANativeWindow_getFormat((ANativeWindow*)pSurface);				

			m_iSurfaceWidth = iWidth;
			m_iSurfaceHeight = iHeight;

			if( iWidth == 0 || iHeight == 0 )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Prepare Surface skip because surface information was wrong(W:%d H:%d, F:%d)", __LINE__, iWidth, iHeight, iFormat);
				return TRUE;
			}
		}
		else
		{
			EGLContext ctx = eglGetCurrentContext();
			EGLDisplay disp = eglGetCurrentDisplay();
			EGLSurface read = eglGetCurrentSurface(EGL_READ);
			EGLSurface draw = eglGetCurrentSurface(EGL_DRAW);

			NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
			NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_JUST_CLEAR, 0);
			NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);

			if(ctx != EGL_NO_CONTEXT){

				eglMakeCurrent(disp, draw, read, ctx);
			}

			m_iSurfaceWidth = 0;
			m_iSurfaceHeight = 0;
		}
		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Prepare Surface(%p, %p) (W:%d H:%d, F:%d)", __LINE__, m_hThemeRenderer, pSurface, iWidth, iHeight, iFormat);
		if( NXT_ThemeRenderer_SetNativeWindow( m_hThemeRenderer, (ANativeWindow*)pSurface, (int)((float)iWidth * fScaleFactor), (int)((float)iHeight * fScaleFactor)) != NXT_Error_None )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Prepare Surface failed(%p, %p)", __LINE__, m_hThemeRenderer, pSurface);
			return FALSE;
		}
				
		m_bLostNativeWindow = TRUE;
		if( pSurface != NULL )
		{
			m_bLostNativeWindow = FALSE;
		}

#ifdef FOR_TEST_THEME_CACHE
		if( m_pProjectMng && !m_bLostNativeWindow)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %s %d]", __func__, __LINE__);
			NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
			NXT_Theme_ResetTextureManager(m_hThemeRenderer);
			

			updateEffectItemVec();

			if (m_bSkipPrecacheEffectOnPrepare == FALSE)
			{
				int async_flag = 0;
				int max_replaceable = DEFAULT_MAX_TEXTURE_CACHE_SIZE;
                unsigned int target_time = CNexProjectManager::getCurrentTimeStamp();
                unsigned int target_end_time = target_time + 5000;

				if (m_pEffectItemVec){

					m_pEffectItemVec->lock();

					for (int i = 0; i < m_pEffectItemVec->getCount(); i++)
					{
						CVideoEffectItem *m_pEffectItem = m_pEffectItemVec->getEffetItem(i);

                        if(m_pEffectItem->m_eEffectType == EFFECT_TYPE_TRANSITION){

                            if(m_pEffectItem->m_uiEffectEndTime < target_time)
                                continue;
                            else if(m_pEffectItem->m_uiEffectStartTime > target_end_time)
                                continue;
                        }
                        else if(m_pEffectItem->m_eEffectType == EFFECT_TYPE_TITLE){

                            if(m_pEffectItem->m_uiTitleEndTime < target_time)
                                continue;
                            else if(m_pEffectItem->m_uiTitleStartTime > target_end_time)
                                continue;
                        }
                        else
                            continue;

						const char *pEffect = m_pEffectItem->getClipEffectID();
						const char *pTitleEffect = m_pEffectItem->getTitleEffectID();

						if (pEffect != NULL)
						{
							if(NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pEffect, &async_flag, &max_replaceable))
								break;
						}

						if (pTitleEffect != NULL)
						{
							if(NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pTitleEffect, &async_flag, &max_replaceable))
								break;
						}
					}
					m_pEffectItemVec->unlock();
				}

				if( m_pDrawInfos != NULL )
				{

					for (int i = 0; i < m_pDrawInfos->sizeDrawInfos(); ++i)
					{

						CNexDrawInfo *pinfo = m_pDrawInfos->mDrawVec[i];
						if (pinfo && pinfo->mEffectID)
							if(NXT_ThemeRenderer_PrecacheEffect(m_hThemeRenderer, pinfo->mEffectID, &async_flag, &max_replaceable))
								break;
					}
				}
                if(m_pPlaybackCacheWorker){

                    m_pPlaybackCacheWorker->resetCacheTime(m_hThemeRenderer, m_pEffectItemVec, target_time);
                    m_pPlaybackCacheWorker->Begin();
                }
			}

			NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
		}
#endif
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Prepare Surface End(%p, %p)", __LINE__, m_hThemeRenderer, pSurface);
#endif//_ANDROID
	return TRUE;
}

void CNEXThread_VideoRenderTask::useSufaceTextureCache(NXBOOL bUse)
{
	m_bUseSurfaceCache = bUse;
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] useSufaceTextureCache mode(%d)", __LINE__, m_bUseSurfaceCache);
}

void* CNEXThread_VideoRenderTask::getSurfaceTexture(){

    CNxMsgGetSurfaceTexture* msg = new CNxMsgGetSurfaceTexture();
    SendCommand(msg);
    msg->waitProcessDone(NEXSAL_INFINITE);
    void* ret = msg->m_pSurface;
    SAFE_RELEASE(msg);
    return ret;
}

void* CNEXThread_VideoRenderTask::getSurfaceTexture_internal()
{
#ifdef _ANDROID
	NXT_HThemeRenderer hRender = getCurrentVideoRenderer();
	if( hRender == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] getSurfaceTexture failed because theme renderer is null", __LINE__);
		return NULL;
	}
	ANativeWindow* surface;

	for(int i = 0; i < MAX_SURFACEITEM_COUNT; i++)
	{
		if( m_SurfaceVec[i].m_iUsed == 0 && m_SurfaceVec[i].m_pSurface != NULL )
		{
			m_SurfaceVec[i].m_iUsed = 1;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] getSurfaceTexture successed with cache(%p)", __LINE__, m_SurfaceVec[i].m_pSurface);
			return m_SurfaceVec[i].m_pSurface;
		}
	}

	if( NXT_ThemeRenderer_CreateSurfaceTexture( hRender, &surface ) != NXT_Error_None )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] getSurfaceTexture failed because surface did not create(%p)", __LINE__, surface);
		return NULL;
	}

	/*
	if( m_bUseSurfaceCache == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] getSurfaceTexture successed without cache(%p %p)", __LINE__, hRender, surface);
		return (void*)surface;
	}
	*/

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] getSurfaceTexture successed (%p %p)", __LINE__, hRender, surface);
	for(int i = 0; i < MAX_SURFACEITEM_COUNT; i++)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] SurfaceTexture mng set Cache(Index:%d Used:%d Surface:%p, render:%p, Surface:%p)", __LINE__, 
			i,  m_SurfaceVec[i].m_iUsed, m_SurfaceVec[i].m_pSurface, hRender, surface);
		if( m_SurfaceVec[i].m_iUsed == 0 && m_SurfaceVec[i].m_pSurface == NULL )
		{
			m_SurfaceVec[i].m_iUsed = 1;
			m_SurfaceVec[i].m_pSurface = (void*)surface;
			m_SurfaceVec[i].m_pRenderer = (void*)hRender;
			break;
		}
	}
	return (void*)surface;
#endif
	return NULL;
}

void CNEXThread_VideoRenderTask::releaseSurfaceTexture(void* pSurface){

    CNxMsgReleaseSurfaceTexture* msg = new CNxMsgReleaseSurfaceTexture(pSurface);
    SendCommand(msg);
    SAFE_RELEASE(msg);
}

void CNEXThread_VideoRenderTask::releaseSurfaceTexture_internal(void* pSurface)
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] releaseSurfaceTexture In(%p)", __LINE__, pSurface);
	if( pSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] releaseSurfaceTexture  failed because surface is null", __LINE__);
		return;
	}

	for(int i = 0; i < MAX_SURFACEITEM_COUNT; i++)
	{
		if( m_SurfaceVec[i].m_iUsed == 1 && m_SurfaceVec[i].m_pSurface == pSurface )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] releaseSurfaceTexture SurfaceTexture successed(%p)", __LINE__, pSurface);

			if( m_bUseSurfaceCache == FALSE )
			{
				if( NXT_ThemeRenderer_DestroySurfaceTexture((NXT_HThemeRenderer)m_SurfaceVec[i].m_pRenderer, (ANativeWindow*)pSurface) == NXT_Error_None )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] releaseSurfaceTexture with Destroy SurfaceTexture(%p %p)", __LINE__, m_SurfaceVec[i].m_pRenderer, pSurface);
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] releaseSurfaceTexture Destroy SurfaceTexture failed(%p %p)", __LINE__, m_SurfaceVec[i].m_pRenderer, pSurface);
				}
				
				m_SurfaceVec[i].m_pRenderer = NULL;
				m_SurfaceVec[i].m_pSurface = NULL;
			}
			m_SurfaceVec[i].m_iUsed = 0;
			return;
		}
	}


	NXT_HThemeRenderer hRender = getCurrentVideoRenderer();
	if( NXT_ThemeRenderer_DestroySurfaceTexture( hRender, (ANativeWindow*) pSurface ) != NXT_Error_None )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Destroy SurfaceTexture mng failed(%p %p)", __LINE__, hRender, pSurface);
		return;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Destroy SurfaceTexture mng successed(%p)", __LINE__, pSurface);
#endif
}

void CNEXThread_VideoRenderTask::clearSurfaceTexture()
{
#ifdef _ANDROID
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] SurfaceTexture clear() In", __LINE__);
	for(int i = 0; i < MAX_SURFACEITEM_COUNT; i++)
	{
		if( m_SurfaceVec[i].m_pRenderer && m_SurfaceVec[i].m_pSurface  )
		{
			NXT_ThemeRenderer_DestroySurfaceTexture( (NXT_HThemeRenderer)m_SurfaceVec[i].m_pRenderer, (ANativeWindow*)m_SurfaceVec[i].m_pSurface );
			m_SurfaceVec[i].m_iUsed		= 0;
			m_SurfaceVec[i].m_pRenderer	= NULL;
			m_SurfaceVec[i].m_pSurface	= NULL;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] SurfaceTexture clear()", __LINE__);
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] SurfaceTexture clear() Out", __LINE__);
#endif
}

void* CNEXThread_VideoRenderTask::createSurfaceTexture(void* pRender)
{
#ifdef _ANDROID
	if( pRender == NULL  )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] createSurfaceTexture failed because wrong parameter(%p) ", __LINE__, pRender);
		return NULL;
	}

	ANativeWindow* surface = NULL;
	NXT_Error err = NXT_ThemeRenderer_CreateSurfaceTexture( (NXT_HThemeRenderer)pRender, &surface);
	if(  err != NXT_Error_None )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] createSurfaceTexture failed(%d)", __LINE__, err);
		return NULL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] createSurfaceTexture success(%p)", __LINE__, surface);
	return (void*)surface;
#endif
	return NULL;
}

void CNEXThread_VideoRenderTask::destroySurfaceTexture(void*pRender, void* pSurface)
{
    CNxMsgDestroySurfaceTexture* msg = new CNxMsgDestroySurfaceTexture(pRender, pSurface);
    SendCommand(msg);
    SAFE_RELEASE(msg);
}

void CNEXThread_VideoRenderTask::destroySurfaceTexture_internal(void*pRender, void* pSurface)
{
#ifdef _ANDROID
	if( pRender == NULL || pSurface == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] destroySurfaceTexture failed because wrong parameter(%p %p) ", __LINE__, pRender, pSurface);
		return;
	}
	NXT_ThemeRenderer_DestroySurfaceTexture( (NXT_HThemeRenderer)pRender, (ANativeWindow*)pSurface );
#endif
}

void CNEXThread_VideoRenderTask::setSeparateEffect(NXBOOL bSeparateEffect)
{
    CNxMsgSetSaparateEffect* msg = new CNxMsgSetSaparateEffect(bSeparateEffect);
    SendCommand(msg);
    SAFE_RELEASE(msg);
}

void CNEXThread_VideoRenderTask::setSeparateEffect_internal(NXBOOL bSeparateEffect)
{
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setSeparateEffect(%d) ", __LINE__, bSeparateEffect);
    m_bSeparateEffect = bSeparateEffect;
}

void CNEXThread_VideoRenderTask::setVisualCountAtTime(int iVisualClip, int iVisualLayer)
{
    CNxMsgSetVisualCountAtTime* msg = new CNxMsgSetVisualCountAtTime(iVisualClip, iVisualLayer);
    SendCommand(msg);
    SAFE_RELEASE(msg);
}

void CNEXThread_VideoRenderTask::setVisualCountAtTime_internal(int iVisualClip, int iVisualLayer)
{
    m_iVisualClipCountAtStartTime   = iVisualClip;
    m_iVisualLayerCountAtStartTime  = iVisualLayer;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setVisualCountAtTime(%d %d)", __LINE__, iVisualClip, iVisualLayer);
}

void CNEXThread_VideoRenderTask::capture()
{
	NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
	int		iWidth		= 0;
	int		iHeight		= 0;
	int		iSize		= 0;
	char*	pData		= NULL;
	NXBOOL	bSucessed	= TRUE;

	if( NXT_ThemeRenderer_GetPixels(m_hThemeRenderer, &iWidth, &iHeight, &iSize, (unsigned char**)&pData, 1) != NXT_Error_None )
	{
		bSucessed = FALSE;
	}
	NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);

	CNexVideoEditor* pEditor = CNexVideoEditor::getVideoEditor();
	if( pEditor )
	{
		if( bSucessed )
		{
#if 0
			char strFile[256];
#if defined(_ANDROID)
			sprintf(strFile, "/sdcard/captureImage_%p_%d_%d_%d.dmp", pData, iWidth, iHeight, iSize);
#elif defined(__APPLE__)
			sprintf(strFile, "/Users/rooney/captureImage_%d_%d_%d.dmp", iWidth, iHeight, iSize);
#endif
			FILE* pFile = fopen(strFile, "wb");
			if( pFile )
			{
				fwrite(pData,sizeof(unsigned char),iSize,pFile);
				fclose(pFile);
			}
#endif
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] capture sucessed(%d %d %d %p)",__LINE__, iWidth, iHeight, iSize, pData);
			pEditor->callbackCapture(iWidth, iHeight, iSize, pData);
		}
		else
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] capture failed because getpixels failed",__LINE__);
			pEditor->callbackCapture(0, 0, 0, 0);
		}
		SAFE_RELEASE(pEditor);
		return;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] capture failed because video editor is null",__LINE__);
}

void CNEXThread_VideoRenderTask::drawClear()
{

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawClear() In", __LINE__);
	
	if( m_bClearRenderer && m_hThemeRenderer )
	{
		NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
		clearGLRender();
		NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 1);

		m_bClearRenderer = FALSE;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawClear() Process End", __LINE__);
	}
	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawClear() Out", __LINE__);
}

// return 0 ignore
// return 1 set time done sucessed
// return 2 set time done failed
int CNEXThread_VideoRenderTask::drawIdle(unsigned int uiTime, int iDisplay)
{
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VRTask.cpp %d] drawIdle(%d %d) TrackSize(%zu)", __LINE__, uiTime, iDisplay, m_VideoTrack.size());


	CFrameInfo* pFrame	= NULL;

	if( m_hThemeRenderer == NULL )
	{
		return 2;
	}
	
	if( m_bLostNativeWindow )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Lost native window after preparewindow", __LINE__);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp [%s %d] m_VideoTrack.size()=%zu", __func__, __LINE__, m_VideoTrack.size());
		for( int i = 0;  i < m_VideoTrack.size(); i++)
		{
			if (m_VideoTrack[i] && m_VideoTrack[i]->isTrackTime(uiTime))
			{
				nexSAL_TraceCat(9, 0, "[VRTask.cpp [%s %d] m_VideoTrack(%d)", __func__, __LINE__, i);
				pFrame = m_VideoTrack[i]->getFrameOutBuffer();
				if( pFrame == NULL ) continue;
				m_VideoTrack[i]->DequeueFrameOutBuffer();
			}
		}
#ifdef FOR_VIDEO_LAYER_FEATURE
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp [%s %d] m_VideoLayer.size()=%zu", __func__, __LINE__, m_VideoLayer.size());
		for( int i = 0;  i < m_VideoLayer.size(); i++)
		{
			if (m_VideoLayer[i] && m_VideoLayer[i]->isTrackTime(uiTime))
			{
				nexSAL_TraceCat(9, 0, "[VRTask.cpp [%s %d] m_VideoLayer(%d)", __func__, __LINE__, i);
				pFrame = m_VideoLayer[i]->getFrameOutBuffer();
				if( pFrame == NULL ) continue;
				m_VideoLayer[i]->DequeueFrameOutBuffer();
			}
		}
#endif

		return 2;
	}
	if ( m_isGLOperationAllowed == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] isOperationAllowed is false", __LINE__);
		return 2;
	}

	pFrame	= NULL;
	// For call render calback after did not request
	if( ((int)uiTime) == -1 )
	{
		NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
		NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, iDisplay);	
		return 0;
	}
	
	if( ((int)uiTime) == -2 )
	{
		for( int i = 0; i < m_VideoTrack.size(); i++)
		{
            NXT_HThemeRenderer renderer = getRendererForUID(m_VideoTrack[i]->m_uiTrackID, true);
            if(renderer){

                NXT_ThemeRenderer_AquireContext(renderer);
                NXT_ThemeRenderer_ReleaseTextureForTrackID(renderer, m_VideoTrack[i]->m_uiTrackID, 0);
                NXT_ThemeRenderer_ReleaseContext(renderer, 0);
            }
			m_VideoTrack[i]->Release();
		}
		m_VideoTrack.clear();
#ifdef FOR_VIDEO_LAYER_FEATURE //Jeff
		for (int i = 0; i < m_VideoLayer.size(); i++)
		{
            NXT_HThemeRenderer renderer = getRendererForUID(m_VideoLayer[i]->m_uiTrackID, true);
            if(renderer){

                NXT_ThemeRenderer_AquireContext(renderer);
                NXT_ThemeRenderer_ReleaseTextureForTrackID(renderer, m_VideoLayer[i]->m_uiTrackID, 0);
                NXT_ThemeRenderer_ReleaseContext(renderer, 0);
            }
			m_VideoLayer[i]->Release();
		}
		m_VideoLayer.clear();
#endif		
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] clear Track and SetTime done Fail (Idle State)", __LINE__);
		return 2;
	}

	if( m_pProjectEffect && m_pProjectMng)
	{
		NXT_ThemeRenderer_SetWatermarkEffect(m_hThemeRenderer, m_pProjectEffect->m_pEffectID, m_pProjectMng->getDuration());
		NXT_ThemeRenderer_SetWatermarkEffect(m_hExportThemeRenderer, m_pProjectEffect->m_pEffectID, m_pProjectMng->getDuration());
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] SetWatermarkEffect(%s)", __LINE__, m_pProjectEffect->m_pEffectID);
	}

	if( m_pDrawInfos != NULL )
	{
		NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
		m_pDrawInfos->applyDrawInfo((void*)m_hThemeRenderer, uiTime);

		for( int i = 0;  i < m_VideoTrack.size(); i++)
		{
			// ZIRA 1905
			// m_VideoTrack[i]->setFirstVideoFrameDrop((void*)m_hThemeRenderer);

			if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
			{
				if( m_VideoTrack[i]->m_uiEndTime <= uiTime )
				{
					if( m_VideoTrack[i]->isVideoClip() )
					{
						m_VideoTrack[i]->DequeueAllFrameOutBuffer();
					}
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) drawIdle not track time(idx:%d) ", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);				
				continue;
			}

			pFrame = m_VideoTrack[i]->getFrameOutBuffer();
			if( pFrame == NULL )
			{
				m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, uiTime);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) drawIdle no Frame(idx:%d) ", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);							
				continue;
			}

			m_uiLastSetTime = pFrame->m_uiTime + 1;

			pFrame->drawVideo((void*)m_hThemeRenderer, uiTime, m_VideoTrack[i]->m_iTrackTextureID, FALSE, TRUE);
			m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, uiTime);
			m_VideoTrack[i]->DequeueFrameOutBuffer();
		}

		m_uiLastTime = uiTime;
		m_uiCurrentTime = uiTime;
		NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);
		NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, iDisplay);

		m_bAvailableCapture = TRUE;

//
//		for( int i = 0; i < m_VideoTrack.size(); i++)
//		{
//			// for mantis 10664
//			if( m_VideoTrack[i]->GetRefCnt() == 1 )
//			{
//				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TRACKID(%d) wait some time before destroy track", __LINE__, m_VideoTrack[i]->m_uiTrackID);
//				nexSAL_TaskSleep(50);
//			}
//			m_VideoTrack[i]->Release();
//		}
//		m_VideoTrack.clear();
//
//#ifdef FOR_VIDEO_LAYER_FEATURE
//		for (int i = 0; i < m_VideoLayer.size(); i++)
//		{
//			if (m_VideoLayer[i])
//			{
//				if (m_VideoLayer[i]->GetRefCnt() == 1)
//				{
//					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TRACKID(%d) wait some time before destroy track", __LINE__, m_VideoLayer[i]->m_uiTrackID);
//					nexSAL_TaskSleep(50);
//				}
//				m_VideoLayer[i]->Release();
//			}
//		}
//		m_VideoLayer.clear();
//#endif
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setTime done(Display:%d %d %d) ", __LINE__, iDisplay, uiTime, m_uiLastSetTime);
		return 1;
	}
	
	int iTrackSize = m_VideoTrack.size();
	if( iTrackSize > 2 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] VideoRenderer has many tracks(%d)", __LINE__, iTrackSize);
		iTrackSize = 2;
	}

	NXBOOL	bEffectEnd		= FALSE;
	NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
	if( m_bSeparateEffect && m_pEffectItemVec != NULL )
	{
        m_pEffectItemVec->lock();
		int iRet = m_pEffectItemVec->applyEffectItem(m_hThemeRenderer, uiTime);
        m_pEffectItemVec->unlock();
		if( iRet >= 100 )
			bEffectEnd = TRUE;
	}
	
	if( iTrackSize > 0 )
	{
		for( int i = 0; i < iTrackSize; i++)
		{
			if( m_bSeparateEffect )
			{
				if(m_pEffectItemVec){

                    m_pEffectItemVec->lock();
					m_pEffectItemVec->applyTitleItem(m_hThemeRenderer, uiTime);
                    m_pEffectItemVec->unlock();
                }
				else
					m_VideoTrack[i]->applyTitleEffect(m_hThemeRenderer, uiTime);
			}
			else
			{
				m_VideoTrack[i]->applyClipEffect(m_hThemeRenderer, uiTime);
				m_VideoTrack[i]->applyTitleEffect(m_hThemeRenderer, uiTime);

				if( i == 0 && m_VideoTrack[i]->isEffectApplyEnd() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Transition effect ended", __LINE__);
					bEffectEnd = TRUE;
					continue;
				}
			}
		
			pFrame = m_VideoTrack[i]->getFrameOutBuffer();
			// kbsong 20130807 : mantix 7756
			if( pFrame == NULL ) 
			{
				NXT_TextureID eID = i == 0 ? NXT_TextureID_Video_1 : NXT_TextureID_Video_2;
				m_VideoTrack[i]->applyClipOptions(m_hThemeRenderer, eID);
				continue;
			}

			if( bEffectEnd ) 
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Effect End and draw current frame idx(%d)", __LINE__, i);
				pFrame->drawVideo((void*)m_hThemeRenderer, 0, uiTime, TRUE, FALSE, 1000);
			}
			else
			{
				pFrame->drawVideo((void*)m_hThemeRenderer, i, uiTime, TRUE, FALSE, 1000);
			}
			
			m_uiLastSetTime = pFrame->m_uiTime + 1;
			
			m_VideoTrack[i]->DequeueFrameOutBuffer();
		}
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] m_VideoTrack.size() %zu", __LINE__, m_VideoTrack.size());
		if( m_bClearRenderer ||(m_pProjectMng && m_pProjectMng->getDuration() <= 0) )
		{
			clearGLRender();
			m_uiLastTime = uiTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] clear Video Track(init flag(%d) is true)", __LINE__, m_bClearRenderer);
			m_bClearRenderer = FALSE;
		}
		
		m_uiLastTime = uiTime;
		NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);	
		NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, iDisplay);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setTime done(Display:%d)", __LINE__, iDisplay);

		return 1;
	}

#ifdef FOR_VIDEO_LAYER_FEATURE
	int iDrawRet = 0;
	nexSAL_TraceCat(9, 0, "NexThemeRenderer.c [%s %d] m_VideoLayer.size()=%zu", __func__, __LINE__, m_VideoLayer.size());
	for( int i = 0;  i < m_VideoLayer.size(); i++)
	{
		if (m_VideoLayer[i] && m_VideoLayer[i]->isTrackTime(uiTime))
		{
			nexSAL_TraceCat(9, 0, "NexThemeRenderer.c [%s %d] m_VideoLayer.play(%d)", __func__, __LINE__, i);
			pFrame = m_VideoLayer[i]->getFrameOutBuffer();
			if( pFrame == NULL ) continue;

			iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime, TRUE, FALSE, 200);
			//Jeff
			m_VideoLayer[i]->DequeueFrameOutBuffer();
		}
	}
#endif	

	m_uiLastTime = uiTime;	
	NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);
	NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
	NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, iDisplay);

	m_bAvailableCapture = TRUE;

	for( int i = 0; i < m_VideoTrack.size(); i++)
	{
		// for mantis 10664
		if( m_VideoTrack[i]->GetRefCnt() == 1 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TRACKID(%d) wait some time before destroy track", __LINE__, m_VideoTrack[i]->m_uiTrackID);
			nexSAL_TaskSleep(50);
		}
		m_VideoTrack[i]->Release();
	}
	m_VideoTrack.clear();
	
#ifdef FOR_VIDEO_LAYER_FEATURE
	for (int i = 0; i < m_VideoLayer.size(); i++)
	{
		if (m_VideoLayer[i])
		{
			if (m_VideoLayer[i]->GetRefCnt() == 1)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TRACKID(%d) wait some time before destroy track", __LINE__, m_VideoLayer[i]->m_uiTrackID);
				nexSAL_TaskSleep(50);
			}
            NXT_HThemeRenderer renderer = getRendererForUID(m_VideoLayer[i]->m_uiTrackID, true);
            if(renderer){

                NXT_ThemeRenderer_AquireContext(renderer);
                NXT_ThemeRenderer_ReleaseTextureForVideoLayer(renderer, m_VideoLayer[i]->m_uiTrackID, 0);
                NXT_ThemeRenderer_ReleaseContext(renderer, 0);
            }
			m_VideoLayer[i]->Release();
		}
	}
	m_VideoLayer.clear();
#endif	
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] setTime done(Display:%d %d %d) ", __LINE__, iDisplay, uiTime, m_uiLastSetTime);
	return 1;
}

void CNEXThread_VideoRenderTask::drawEffect(unsigned int uiTime)
{
	if( m_bRenderPause )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video Renderer Paused while playing", __LINE__);
		return;
	}
	
	unsigned int checkDrawPlay = nexSAL_GetTickCount();
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawPlay Time(%d)", __LINE__, uiTime);
	
	if( m_bLostNativeWindow )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Lost native window after preparewindow", __LINE__);
		return;
	}
	if ( m_isGLOperationAllowed == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] isOperationAllowed is false", __LINE__);
		return;
	}

	int			iTrackSize		= 0;
	NXBOOL		bEffectEnd		= FALSE;
	CFrameInfo*	pFrame = NULL;

 	if( m_perfGlDraw.m_uiCount == 0 )
		m_perfGlDraw.CheckModuleStart();

	checkVideoProcessOk();

	iTrackSize = m_VideoTrack.size();
	if( iTrackSize > 2 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] VideoRenderer has many tracks(%d)", __LINE__, iTrackSize);
		iTrackSize = 2;
	}

	if(NXT_Error_None != NXT_ThemeRenderer_AquireContext(m_hThemeRenderer)){

		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Fail to aquireContext", __LINE__);
		return;
	}

	if( m_pDrawInfos != NULL )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawInfows(%d) drawEffect", __LINE__, uiTime);

		m_pDrawInfos->applyDrawInfo((void*)m_hThemeRenderer, uiTime);

		for( int i = 0;  i < m_VideoTrack.size(); i++)
		{
			// ZIRA 1905
			// m_VideoTrack[i]->setFirstVideoFrameDrop((void*)m_hThemeRenderer);

			if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
			{
				if( m_VideoTrack[i]->m_uiEndTime <= uiTime )
				{
					if( m_VideoTrack[i]->isVideoClip() )
					{
						m_VideoTrack[i]->DequeueAllFrameOutBuffer();
					}
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) drawEffect not track time(idx:%d) ", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);
				continue;
			}

			m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, uiTime);
		}

		// for JIRA 2916 issue.
		if( m_iFirstVideoFrameUploaded )
		{
			NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);
			NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		}
		NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, m_iFirstVideoFrameUploaded);
		m_perfGlDraw.CheckModuleUnitEnd();
		return;
	}

	if( m_bSeparateEffect && m_pEffectItemVec != NULL )
	{
        m_pEffectItemVec->lock();
		int iRet = m_pEffectItemVec->applyEffectItem(m_hThemeRenderer, uiTime);
        m_pEffectItemVec->unlock();
		 if( iRet >= 100 )
		{
			bEffectEnd = TRUE;
		}
		 
		if( iTrackSize == 0 && iRet > 0)
		{
			sendVideoStarted();
		}		
	}	


	for( int i = 0;  i < iTrackSize; i++)
	{
		if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
		{
			if( i == 0  && m_VideoTrack[i]->m_uiEndTime <=  uiTime )
			{
				if( m_VideoTrack[i] ->isVideoClip() )
				{
					m_VideoTrack[i]->DequeueFrameOutBuffer();
				}
				bEffectEnd = TRUE;
			}			
			continue;
		}

		if( m_bSeparateEffect )
		{
			if(m_pEffectItemVec){
                m_pEffectItemVec->lock();
				m_pEffectItemVec->applyTitleItem(m_hThemeRenderer, uiTime);
                m_pEffectItemVec->unlock();
            }
			else
				m_VideoTrack[i]->applyTitleEffect(m_hThemeRenderer, uiTime);
		}
		else
		{
			m_VideoTrack[i]->applyClipEffect(m_hThemeRenderer, uiTime);
			m_VideoTrack[i]->applyTitleEffect(m_hThemeRenderer, uiTime);

			if( i == 0 && m_VideoTrack[i]->isEffectApplyEnd() )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Transition effect ended", __LINE__);
				bEffectEnd = TRUE;
				continue;
			}
		}

		if( bEffectEnd ) 
		{
			m_VideoTrack[i]->applyPosition((void*)m_hThemeRenderer, uiTime, 0);
		}
		else
		{
			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Effect Time Idx(%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);
			m_VideoTrack[i]->applyPosition((void*)m_hThemeRenderer, uiTime, i);
		}
	}
	
	m_uiLastTime = uiTime;
	m_perfGlDraw.CheckModuleUnitStart();
	
	// for JIRA 2916 issue.
	if( m_iFirstVideoFrameUploaded )
	{
		unsigned int checkRenderTime = nexSAL_GetTickCount();
		NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);
		NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] GL Draw Time(%d) DrawPlay Time(%d)", __LINE__, nexSAL_GetTickCount() - checkRenderTime, nexSAL_GetTickCount() - checkDrawPlay);	
	}
	NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, m_iFirstVideoFrameUploaded);	

	m_perfGlDraw.CheckModuleUnitEnd();
}

void CNEXThread_VideoRenderTask::drawPlay(unsigned int uiTime, int iDisplay)
{
	if( m_bRenderPause )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video Renderer Paused while playing", __LINE__);
		return;
	}
	
	unsigned int checkDrawPlay = nexSAL_GetTickCount();
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawPlay Time(%d)", __LINE__, uiTime);
	
	if( m_bLostNativeWindow )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Lost native window after preparewindow", __LINE__);
		return;
	}
	if ( m_isGLOperationAllowed == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] isOperationAllowed is false", __LINE__);
		return;
	}

	int			iTrackSize		= 0;
	int			iDrawRet		= 0;
	NXBOOL		bEffectEnd		= FALSE;
	CFrameInfo*	pFrame = NULL;

 	if( m_perfGlDraw.m_uiCount == 0 )
		m_perfGlDraw.CheckModuleStart();
	if( m_perfGetPixel.m_uiCount == 0 )
		m_perfGetPixel.CheckModuleStart();
	if( m_perfNativeRender.m_uiCount == 0 )
		m_perfNativeRender.CheckModuleStart();

	m_perfNativeRender.CheckModuleUnitStart();

	checkVideoProcessOk();

	if(NXT_Error_None != NXT_ThemeRenderer_AquireContext(m_hThemeRenderer)){

		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Fail to aquireContext", __LINE__);
		return;
	}

	if( m_pDrawInfos != NULL )
	{
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawInfows(%d) drawPlay", __LINE__, uiTime);

		m_pDrawInfos->applyDrawInfo((void*)m_hThemeRenderer, uiTime);

		for( int i = 0;  i < m_VideoTrack.size(); i++)
		{
			// ZIRA 1905
			// m_VideoTrack[i]->setFirstVideoFrameDrop((void*)m_hThemeRenderer);

			if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
			{
				if( m_VideoTrack[i]->m_uiEndTime <= uiTime )
				{
					if( m_VideoTrack[i]->isVideoClip() )
					{
						m_VideoTrack[i]->DequeueAllFrameOutBuffer();
					}
				}
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Track was not track time(%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, uiTime);
				continue;
			}

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Track is time", __LINE__, m_VideoTrack[i]->m_uiTrackID);

			pFrame = m_VideoTrack[i]->getFrameOutBuffer();
			if( pFrame == NULL )
			{
				if( m_VideoTrack[i]->isTrackStarted() == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Track was not started", __LINE__, m_VideoTrack[i]->m_uiTrackID);
				}

				// + mantis 7722
				if( i == 0 && m_iVideoStarted == 0 && m_VideoTrack[i]->isTrackStarted() == FALSE && m_VideoTrack[i]->getVideoDecodeEnd() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Forced sending video start because Video track end without display", __LINE__);
					sendVideoStarted();
				}

				if( m_VideoTrack[i]->getVideoDecodeEnd() )
				{
					m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, uiTime);
				}
				else 
				{
					// - mantis 7722
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) getFramebuffer of Video Track is null wait VideoFrame(%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);
					continue;				
				}
			}
			else 
			{
				iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, uiTime, m_VideoTrack[i]->m_iTrackTextureID, FALSE, FALSE);
				if( m_VideoTrack[i]->getMotionTracked() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Motion Tracked Video Time adjusted(%d->%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, uiTime, pFrame->m_uiTime);				
					uiTime = pFrame->m_uiTime;
				}

				m_VideoTrack[i]->applyDrawInfos((void*)m_hThemeRenderer, uiTime);

				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Track(%d) display flag(%d) Time(F:%d, C:%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, iDrawRet, pFrame->m_uiTime, uiTime);
				if( iDrawRet == 1 ) // draw drop
				{
					m_VideoTrack[i]->DequeueFrameOutBuffer();
				}
				else if( iDrawRet == 2 ) // draw wait
				{
					// sendVideoStarted();
				}
				else
				{
					m_iFirstVideoFrameUploaded = 1;
					// sendVideoStarted();
					m_VideoTrack[i]->DequeueFrameOutBuffer();
				}
			}
		}
	}
	else
	{
		iTrackSize = m_VideoTrack.size();
		if( iTrackSize > 2 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] VideoRenderer has many tracks(%d)", __LINE__, iTrackSize);
			iTrackSize = 2;
		}

		if( m_bSeparateEffect && m_pEffectItemVec != NULL )
		{
            m_pEffectItemVec->lock();
			int iRet = m_pEffectItemVec->applyEffectItem(m_hThemeRenderer, uiTime);
            m_pEffectItemVec->unlock();
			 if( iRet >= 100 )
			{
				bEffectEnd = TRUE;
			}
			 
			if( iTrackSize == 0 && iRet > 0)
			{
				sendVideoStarted();
			}		
		}	

#ifdef FOR_VIDEO_LAYER_FEATURE
		for( int i = 0;  i < m_VideoLayer.size(); i++){
			//nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] m_VideoLayer[%d]->m_iDrop is %d;", __LINE__, i, m_VideoLayer[i]->m_iDrop);
			if (NULL == m_VideoLayer[i])
				continue;
			/*if (m_VideoLayer[i]->isTrackTime(uiTime) == FALSE)
			{
				if (m_VideoLayer[i]->m_uiEndTime <= uiTime)
				{
					m_VideoLayer[i]->DequeueFrameOutBuffer();
				}
				continue;
			}
			else*/{
				while ((pFrame = m_VideoLayer[i]->getFrameOutBuffer())) {
					if( m_iVideoStarted == 0 || m_VideoLayer[i]->m_bFirstDisplay4Layer == 0)
						iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime, TRUE);
					else
						iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime);
					m_VideoLayer[i]->m_bFirstDisplay4Layer = 1;
					if (iDrawRet == 1) // draw drop
					{
						m_VideoLayer[i]->m_iDrop++;
						m_VideoLayer[i]->DequeueFrameOutBuffer();
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] m_VideoLayer[%d]->m_iDrop is %d;", __LINE__, i, m_VideoLayer[i]->m_iDrop);
					}
					else if (iDrawRet == 2) // draw wait
					{
						break;
					}
					else
					{
						m_VideoLayer[i]->m_iDrop = 0;
						m_VideoLayer[i]->DequeueFrameOutBuffer();
						break;
					}
				}
			}
		}	
#endif
		
		for( int i = 0;  i < iTrackSize; i++)
		{
			// ZIRA 1905
			// m_VideoTrack[i]->setFirstVideoFrameDrop((void*)m_hThemeRenderer);
			
			if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
			{
				if( i == 0  && m_VideoTrack[i]->m_uiEndTime <=  uiTime )
				{
					if( m_VideoTrack[i]->isVideoClip() )
					{
						m_VideoTrack[i]->DequeueFrameOutBuffer();
					}
					bEffectEnd = TRUE;
				}			
				continue;
			}

			if( m_bSeparateEffect )
			{
				if(m_pEffectItemVec){

                    m_pEffectItemVec->lock();
					m_pEffectItemVec->applyTitleItem(m_hThemeRenderer, uiTime);
                    m_pEffectItemVec->unlock();
                }
				else
					m_VideoTrack[i]->applyTitleEffect(m_hThemeRenderer, uiTime);
			}
			else
			{
				m_VideoTrack[i]->applyClipEffect(m_hThemeRenderer, uiTime);
				m_VideoTrack[i]->applyTitleEffect(m_hThemeRenderer, uiTime);

				if( i == 0 && m_VideoTrack[i]->isEffectApplyEnd() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Transition effect ended", __LINE__);
					bEffectEnd = TRUE;
					continue;
				}
			}
			
			pFrame = m_VideoTrack[i]->getFrameOutBuffer();
			if( pFrame == NULL )
			{
				NXT_TextureID eID = (i == 0 ? NXT_TextureID_Video_1 : NXT_TextureID_Video_2);
				m_VideoTrack[i]->applyClipOptions(m_hThemeRenderer, eID);
				
				if( i == 1 && m_VideoTrack[i]->isTrackStarted() == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Track was not started", __LINE__, m_VideoTrack[i]->m_uiTrackID);
					clearUploadTexture(2, 64,64);
				}
				// + mantis 7722
				if( i == 0 && m_iVideoStarted == 0 && m_VideoTrack[i]->isTrackStarted() == FALSE && m_VideoTrack[i]->getVideoDecodeEnd() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Forced sending video start because Video track end without display", __LINE__);
					sendVideoStarted();
				}
				// - mantis 7722
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] getFramebuffer of Video Track is null wait VideoFrame(%d)", __LINE__, i);
				continue;
			}

			if( bEffectEnd ) 
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Effect End Time(idx(%d))", __LINE__, i);
				iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, 0, uiTime);
				bEffectEnd = FALSE;
			}
			else
			{
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Effect Time Idx(%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);
				if( m_iVideoStarted == 0 )
					iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, i, uiTime, TRUE);
				else
					iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, i, uiTime);
			}

			 // nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Track(%d) display flag(%d) Time(F:%d, C:%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, iDrawRet, pFrame->m_uiTime, uiTime);
			if( iDrawRet == 1 ) // draw drop
			{
				m_VideoTrack[i]->DequeueFrameOutBuffer();
			}
			else if( iDrawRet == 2 ) // draw wait
			{
				sendVideoStarted();
			}
			else
			{
				m_iFirstVideoFrameUploaded = 1;
				sendVideoStarted();
				m_VideoTrack[i]->DequeueFrameOutBuffer();
			}
		}
	}
	
	m_uiLastTime = uiTime;
	m_perfGlDraw.CheckModuleUnitStart();
	
	// for JIRA 2916 issue.
	if( m_iFirstVideoFrameUploaded && iDisplay)
	{
		unsigned int checkRenderTime = nexSAL_GetTickCount();
		NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);
		NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] GL Draw Time(%d) DrawPlay Time(%d)", __LINE__, nexSAL_GetTickCount() - checkRenderTime, nexSAL_GetTickCount() - checkDrawPlay);	
	}
	NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, m_iFirstVideoFrameUploaded);	

	if( m_pDrawInfos != NULL )
		sendVideoStarted();

	m_perfGlDraw.CheckModuleUnitEnd();
	m_perfNativeRender.CheckModuleUnitEnd();
}

unsigned int CNEXThread_VideoRenderTask::drawPlayLayer(unsigned int uiTime)
{
	unsigned int uRet = 1;
	int drawCount = 0;
	
	if( m_bRenderPause )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video Renderer Paused while playing", __LINE__);
		return uRet;
	}
	
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawPlay Time(%d)", __LINE__, uiTime);
	
	if( m_bLostNativeWindow )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Lost native window after preparewindow", __LINE__);
		return uRet;
	}
	if ( m_isGLOperationAllowed == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] isOperationAllowed is false", __LINE__);
		return uRet;
	}
    
	int			iTrackSize		= 0;
	int			iDrawRet		= 0;
	NXBOOL		bEffectEnd		= FALSE;
	CFrameInfo*	pFrame = NULL;

	if(NXT_Error_None != NXT_ThemeRenderer_AquireContext(m_hThemeRenderer)){

		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Fail to aquireContext", __LINE__);
		return uRet;
	}
	
	for( int i = 0;  i < m_VideoLayer.size(); i++){
		if (NULL == m_VideoLayer[i])
			continue;
		/*if (m_VideoLayer[i]->isTrackTime(uiTime) == FALSE)
		{
			if (m_VideoLayer[i]->m_uiEndTime <= uiTime)
			{
				m_VideoLayer[i]->DequeueFrameOutBuffer();
			}
			continue;
		}
		else*/{
			while ((pFrame = m_VideoLayer[i]->getFrameOutBuffer())) {
				if( m_iVideoStarted == 0 || m_VideoLayer[i]->m_bFirstDisplay4Layer == 0)
					iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime, TRUE);
				else
					iDrawRet = pFrame->drawVideo((void*)m_hThemeRenderer, m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime);
				m_VideoLayer[i]->m_bFirstDisplay4Layer = 1;
				if (iDrawRet == 1) // draw drop
				{
					m_VideoLayer[i]->m_iDrop++;
					m_VideoLayer[i]->DequeueFrameOutBuffer();
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] m_VideoLayer[%d]->m_iDrop is %d;", __LINE__, i, m_VideoLayer[i]->m_iDrop);
					uRet = 0;
				}
				else if (iDrawRet == 2) // draw wait
				{
					break;
				}
				else
				{
					m_VideoLayer[i]->m_iDrop = 0;
					m_VideoLayer[i]->DequeueFrameOutBuffer();
					drawCount++;
					break;
				}
			}
		}
	}		
	//m_uiLastTime = uiTime;

	if(drawCount == 0)
	{
		m_perfNativeRender.CheckModuleUnitEnd();
		NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 0);
		return uRet;
	}
	
	// for JIRA 2916 issue.
	if( m_iFirstVideoFrameUploaded )
	{
		unsigned int checkRenderTime = nexSAL_GetTickCount();
		NXT_ThemeRenderer_SetCTS(m_hThemeRenderer, uiTime);
		NXT_ThemeRenderer_GLDraw(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, 0);
		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] GL Draw Time(%d) DrawPlay Time(%d)", __LINE__, nexSAL_GetTickCount() - checkRenderTime, nexSAL_GetTickCount() - checkDrawPlay);	
	}
	NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, m_iFirstVideoFrameUploaded);		

	return uRet;
}

void CNEXThread_VideoRenderTask::drawExport(unsigned int uiTime)
{
	if( m_bRenderPause )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video Renderer Paused while exporting", __LINE__);
		return;
	}

	int 				iDrawRet		= 0;
	NXBOOL 			iTextureSwap	= FALSE;
	NXBOOL			bEffectEnd		= FALSE;
	CFrameInfo*		pFrame 			= NULL;
	int 				iTrackSize		= 0;
	
 	if( m_perfGlDraw.m_uiCount == 0 )
		m_perfGlDraw.CheckModuleStart();
	if( m_perfGetPixel.m_uiCount == 0 )
		m_perfGetPixel.CheckModuleStart();
	if( m_perfNativeRender.m_uiCount == 0 )
		m_perfNativeRender.CheckModuleStart();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VRTask.cpp %d] drawExport(Time:(%d), State:%d, TrackSize:%d)(%d)",
		__LINE__, uiTime, m_eThreadState, m_VideoTrack.size(), m_iVideoStarted);

	if( m_pFileWriter == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawExport error(m_pFileWriter:%p ",
			__LINE__, m_pFileWriter);
		return;
	}
	
	m_perfNativeRender.CheckModuleUnitStart();

	iTrackSize = m_VideoTrack.size();
	if( m_pDrawInfos == NULL && iTrackSize > 2 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] VideoRenderer has many tracks(%d)", __LINE__, iTrackSize);
		iTrackSize = 2;
	}
	
	while (getCurrentVideoRenderer() == m_hThemeRenderer){
		nexSAL_TaskSleep(100);
	}

	checkVideoProcessOk(uiTime);

	unsigned int uiFrameTime = 0;

	NXT_ThemeRenderer_AquireContext(getCurrentVideoRenderer());

	if( m_pDrawInfos != NULL )
	{
		m_pDrawInfos->applyDrawInfo((void*)getCurrentVideoRenderer(), uiTime);

		for( int i = 0;  i < m_VideoTrack.size(); i++)
		{
			if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
			{
				if( m_VideoTrack[i]->m_uiEndTime <= uiTime )
				{
					if( m_VideoTrack[i]->isVideoClip() )
					{
						m_VideoTrack[i]->DequeueAllFrameOutBuffer();
					}
				}
				// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Track was not track time(%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, uiTime);
				continue;
			}

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Track is time", __LINE__, m_VideoTrack[i]->m_uiTrackID);

			pFrame = m_VideoTrack[i]->getFrameOutBuffer();
			if( pFrame == NULL )
			{
				if( m_VideoTrack[i]->isTrackStarted() == FALSE )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) Track was not started", __LINE__, m_VideoTrack[i]->m_uiTrackID);
				}
				// + mantis 7722
				if( i == 0 && m_iVideoStarted == 0 && m_VideoTrack[i]->isTrackStarted() == FALSE && m_VideoTrack[i]->getVideoDecodeEnd() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Forced sending video start because Video track end without display", __LINE__);
					sendVideoStarted();
				}
				// - mantis 7722
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] TID(%d) getFramebuffer of Video Track is null wait VideoFrame(%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, i);
				continue;
			}

			iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), uiTime, m_VideoTrack[i]->m_iTrackTextureID, TRUE, FALSE);
			m_VideoTrack[i]->applyDrawInfos((void*)getCurrentVideoRenderer(), uiTime);

			// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Track(%d) display flag(%d) Time(F:%d, C:%d)", __LINE__, m_VideoTrack[i]->m_uiTrackID, iDrawRet, pFrame->m_uiTime, uiTime);
			if( iDrawRet == 1 ) // draw drop
			{
				m_VideoTrack[i]->DequeueFrameOutBuffer();
			}
			else if( iDrawRet == 2 ) // draw wait
			{
				sendVideoStarted();
			}
			else
			{
				m_iFirstVideoFrameUploaded = 1;
				sendVideoStarted();
				m_VideoTrack[i]->DequeueFrameOutBuffer();
			}
		}
	}
	else
	{
		if( m_bSeparateEffect && m_pEffectItemVec != NULL )
		{
            m_pEffectItemVec->lock();
			int iRet = m_pEffectItemVec->applyEffectItem(getCurrentVideoRenderer(), uiTime);
            m_pEffectItemVec->unlock();
			if( iRet >= 100 )
			{
				bEffectEnd = TRUE;
			}
			
			if( iTrackSize == 0 && iRet > 0)
			{
				sendVideoStarted();
			}		
		}	

		for( int i = 0;  i < iTrackSize; i++)
		{
			// m_VideoTrack[i]->setFirstVideoFrameDrop((void*)getCurrentVideoRenderer());

			if( m_VideoTrack[i]->isTrackTime(uiTime) == FALSE )
			{
				if( i == 0  && m_VideoTrack[i]->m_uiEndTime <=  uiTime )
				{
					if( m_VideoTrack[i] ->isVideoClip() )
					{
						m_VideoTrack[i]->DequeueFrameOutBuffer();
					}
					bEffectEnd = TRUE;
				}
				continue;
			}		

			if( m_bSeparateEffect )
			{
				if(m_pEffectItemVec){

                    m_pEffectItemVec->lock();
					m_pEffectItemVec->applyTitleItem(getCurrentVideoRenderer(), uiTime);
                    m_pEffectItemVec->unlock();
                }
				else
					m_VideoTrack[i]->applyTitleEffect(getCurrentVideoRenderer(), uiTime);
			}
			else
			{
				m_VideoTrack[i]->applyClipEffect(getCurrentVideoRenderer(), uiTime);
				m_VideoTrack[i]->applyTitleEffect(getCurrentVideoRenderer(), uiTime);

				if( i == 0 && m_VideoTrack[i]->isEffectApplyEnd() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Transition effect ended", __LINE__);
					bEffectEnd = TRUE;
					continue;
				}
			}

			pFrame = m_VideoTrack[i]->getFrameOutBuffer();
			if( pFrame == NULL )
			{
				// + mantis 7722
				if( i == 0 && m_iVideoStarted == 0 && m_VideoTrack[i]->isTrackStarted() == FALSE && m_VideoTrack[i]->getVideoDecodeEnd() )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Forced sending video start because Video track end without display", __LINE__);
					sendVideoStarted();
				}
				// - mantis 7722
				continue;
			}

			if( m_iVideoStarted == 0 )
			{
				iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), i, uiTime, TRUE, TRUE);
				sendVideoStarted();
			}
			else
			{
				if( bEffectEnd ) 
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[VRTask.cpp %d] Effect End Time(idx(%d))", __LINE__, i);
					iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), 0, uiTime, FALSE, TRUE);
					bEffectEnd = FALSE;
				}
				else
				{
					iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), i, uiTime, FALSE, TRUE);
				}
			}
			
			if( iDrawRet == 2 ) // draw wait
			{
			}
			else
			{
				m_VideoTrack[i]->DequeueFrameOutBuffer(NULL, &uiFrameTime);
#ifndef USE_WRITE_TASK
				m_VideoTrack[i]->m_FrameTimeChecker4Pause.addFrameCTSVec(uiFrameTime, uiTime);
#endif
			}
		}

#ifdef FOR_VIDEO_LAYER_FEATURE
		for (int i = 0; i < m_VideoLayer.size(); i++){
			if (NULL == m_VideoLayer[i])
				continue;
			/*if (m_VideoLayer[i]->isTrackTime(uiTime) == FALSE)
			{
				if (m_VideoLayer[i]->m_uiEndTime <= uiTime)
				{
					m_VideoLayer[i]->DequeueFrameOutBuffer();
				}
				continue;
			}
			else*/{
				while ((pFrame = m_VideoLayer[i]->getFrameOutBuffer())) {
					if( m_iVideoStarted == 0 || m_VideoLayer[i]->m_bFirstDisplay4Layer == 0)
						iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime, TRUE, TRUE);
					else
						iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime, FALSE, TRUE);
					m_VideoLayer[i]->m_bFirstDisplay4Layer = 1;
					if (iDrawRet == 1) // draw drop
					{
						m_VideoLayer[i]->m_iDrop++;
						m_VideoLayer[i]->DequeueFrameOutBuffer();
					}
					else if (iDrawRet == 2) // draw wait
					{
						break;
					}
					else
					{
						unsigned int uiLayerTime = 0;
						m_VideoLayer[i]->m_iDrop = 0;
						m_VideoLayer[i]->DequeueFrameOutBuffer(NULL, &uiLayerTime);
#ifndef USE_WRITE_TASK
						m_VideoLayer[i]->m_FrameTimeChecker4Pause.addFrameCTSVec(uiLayerTime, uiTime);
#endif
						break;
					}
				}
			}
		}
#endif
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Encode one frame(%d %d %d)", __LINE__, uiFrameTime, m_uiLastTime, uiTime);
	if( uiFrameTime > m_uiLastTime )
	{
		m_uiLastTime = uiFrameTime;
	}
	else
	{
		m_uiLastTime = uiTime;
	}

	m_uiLastTime = uiTime;

	if( CNexVideoEditor::m_iUseSurfaceMediaSource == 1 )
	{
		unsigned int uiDuration	= 0;
		unsigned int uiSize		= 0;
		NXBOOL bEncodeEnd		= FALSE;

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport ", __LINE__);
			
		NXT_ThemeRenderer_SetCTS(getCurrentVideoRenderer(), uiTime);
		NXT_ThemeRenderer_GLDraw(getCurrentVideoRenderer(), NXT_RendererOutputType_RGBA_8888, 0);

#ifdef _ANDROID
		if( m_bNeedRenderFinish )
			NXT_ThemeRenderer_GLWaitToFinishRendering(getCurrentVideoRenderer());
#endif

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport ", __LINE__);

		m_pFileWriter->setMediaCodecTimeStamp(m_uiLastTime);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport Begin Swap(%d)", __LINE__, m_uiLastTime);
		m_perfGlDraw.CheckModuleUnitStart();
		NXT_ThemeRenderer_SwapBuffers(getCurrentVideoRenderer());
		m_perfGlDraw.CheckModuleUnitEnd();
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport end Swap", __LINE__);
		NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 0);

#ifndef USE_WRITE_TASK
		m_pFileWriter->setBaseVideoFrame(m_uiLastTime, FALSE, &uiDuration, &uiSize, &bEncodeEnd);
		m_uiVideoExportFrameCount++;
		if( bEncodeEnd )
		{
			for( int i = 0;  i < iTrackSize; i++)
			{
				if(m_VideoTrack[i]->m_FrameTimeChecker4Pause.size())
				{
					m_VideoTrack[i]->m_FrameTimeChecker4Pause.removeSmallTime( m_pFileWriter->getVideoTime());			
					m_VideoTrack[i]->m_uiLastEncodedFrameCTS = m_VideoTrack[i]->m_FrameTimeChecker4Pause.getFirstCTSInVec();					
				}
			}

			for( int i = 0;  i < m_VideoLayer.size(); i++)
			{
				if(m_VideoLayer[i]->m_FrameTimeChecker4Pause.size())
				{
					m_VideoLayer[i]->m_FrameTimeChecker4Pause.removeSmallTime( m_pFileWriter->getVideoTime());			
					m_VideoLayer[i]->m_uiLastEncodedFrameCTS = m_VideoLayer[i]->m_FrameTimeChecker4Pause.getFirstCTSInVec();					
				}
			}
			
			m_uiVideoEncOutputFrameCount++;
		}
#else
	CVideoWriteBuffer* pBuffer = m_pFileWriter->getBuffer();
        pBuffer->m_eBufferFormat= STREAM_INPUT_BUFFER_FORMAT_MEDIABUFFER;
        pBuffer->m_uiTime		= m_uiLastTime;
	m_pFileWriter->releaseBuffer(pBuffer);
#endif        
		m_perfNativeRender.CheckModuleUnitEnd();
		return;
	}

	m_perfGlDraw.CheckModuleUnitStart();
	NXT_ThemeRenderer_SetCTS(getCurrentVideoRenderer(), uiTime);
#if defined(ANDROID)
	NXT_ThemeRenderer_GLDraw(getCurrentVideoRenderer(), NXT_RendererOutputType_Y2CrA_8888, 0);
#elif defined(__APPLE__)
	NXT_ThemeRenderer_GLDraw(getCurrentVideoRenderer(), NXT_RendererOutputType_RGBA_8888_yflip, 0);
#endif

#ifdef CHECK_RENDER_FRAME
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] CHECK_RENDER_FRAME(%d)", __LINE__, g_bCheckFrame);
	if( g_bCheckFrame )
	{
		glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
	    glEnable(GL_SCISSOR_TEST);
	    glScissor(0.75, 0.75, 1, 1);
	    glClearColor(1.0, 0, 0, 1.0);
	    glClear(GL_COLOR_BUFFER_BIT);
	}
	else
	{
		glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_FALSE);
	    glEnable(GL_SCISSOR_TEST);
	    glScissor(0.75, 0.75, 1, 1);
	    glClearColor(0, 1.0, 0, 1.0);
	    glClear(GL_COLOR_BUFFER_BIT);
	}
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	g_bCheckFrame = !g_bCheckFrame;
#endif
	
	NXT_ThemeRenderer_GLWaitToFinishRendering(getCurrentVideoRenderer());
	m_perfGlDraw.CheckModuleUnitEnd();

	int 				iWidth	= 0;;
	int 				iHeight	= 0;
	int 				iSize	= 0;

#if defined(__APPLE__)
    {
     //   nexSAL_TaskSleep(50);
        
        void *pPlatformPixelBuff = NULL;//(unsigned char*)nexSAL_MemAlloc(sizeof(void*));
        unsigned int uiDuration, uiSize;
        iSize = sizeof(void*);
        NXT_HThemeRenderer currentRenderer = getCurrentVideoRenderer();
        NXT_ThemeRenderer_GetPixelBuffer( currentRenderer, &pPlatformPixelBuff, &iSize );
        
        if ( pPlatformPixelBuff ) {
            if( m_pFileWriter->setBaseVideoFrame(m_uiLastTime, (unsigned char*)pPlatformPixelBuff, iSize, &uiDuration, &uiSize, NULL) == FALSE ) {
                nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Video Frame Write failed", __func__, __LINE__);
            }
        } else {
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Failed allocating export output render buffer", __func__, __LINE__);
            sendInterruptExportEvent(NEXVIDEOEDITOR_ERROR_NOT_ENOUGH_MEMORY);
        }
        NXT_ThemeRenderer_ReturnPixelBufferForReuse(getCurrentVideoRenderer(), pPlatformPixelBuff);
        
        //nexSAL_MemFree( pPlatformPixelBuff);
        NXT_ThemeRenderer_ReleaseContext(currentRenderer, 0);
        return;
    }
#endif

	CVideoWriteBuffer* pBuffer = m_pFileWriter->getBuffer();
	if( pBuffer )
	{
		m_perfGetPixel.CheckModuleUnitStart();

		if( NXT_ThemeRenderer_GetPixels2( getCurrentVideoRenderer(), &iWidth, &iHeight, &iSize, pBuffer->m_pY2UVA ) == NXT_Error_None )
		{
			if( m_iEncodeWidth != iWidth || m_iEncodeHeight != iHeight )
			{
				m_perfGetPixel.CheckModuleUnitEnd();
				pBuffer->resetBuffer();
				m_pFileWriter->releaseBuffer(pBuffer);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] NXT_ThemeRenderer_GetPixels wrong Size(%d %d)", __LINE__, iWidth, iHeight);
				NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 1);
				m_perfNativeRender.CheckModuleUnitEnd();
				sendInterruptExportEvent();
				return;
			}
			pBuffer->m_iWidth			= iWidth;
			pBuffer->m_iHeight		= iHeight;
			pBuffer->m_eBufferFormat	= STREAM_INPUT_BUFFER_FORMAT_Y2UVA;
			pBuffer->m_uiBufferSize	= iSize;
			pBuffer->m_uiTime		= m_uiLastTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] nativeVideoRender getPixels(W(%d) H(%d) Size(%p %d)) CTS(%d)", 
				__LINE__, iWidth, iHeight, pBuffer->m_pY2UVA, iSize, m_uiLastTime);
		}
		else
		{
			pBuffer->resetBuffer();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] NXT_ThemeRenderer_GetPixels is failed", __LINE__);
		}
		m_perfGetPixel.CheckModuleUnitEnd();
		m_pFileWriter->releaseBuffer(pBuffer);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] get FileWriter Buffer is null", __LINE__);
	}
	NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 1);
	m_perfNativeRender.CheckModuleUnitEnd();
}

unsigned int CNEXThread_VideoRenderTask::drawExportLayer(unsigned int uiTime)
{
	unsigned int uRet = 1;
	int drawCount = 0;
	if( m_bRenderPause )
	{
		nexSAL_TaskSleep(100);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Video Renderer Paused while exporting", __LINE__);
		return uRet;
	}

	int 				iDrawRet		= 0;
	NXBOOL 			iTextureSwap	= FALSE;
	NXBOOL			bEffectEnd		= FALSE;
	CFrameInfo*		pFrame 			= NULL;
	int 				iTrackSize		= 0;
	
 	if( m_perfGlDraw.m_uiCount == 0 )
		m_perfGlDraw.CheckModuleStart();
	if( m_perfGetPixel.m_uiCount == 0 )
		m_perfGetPixel.CheckModuleStart();
	if( m_perfNativeRender.m_uiCount == 0 )
		m_perfNativeRender.CheckModuleStart();

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawExport(Time:(%d), State:%d, TrackSize:%d)(%d)", 
		__LINE__, uiTime, m_eThreadState, m_VideoTrack.size(), m_iVideoStarted);

	if( m_pFileWriter == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] drawExport error(m_pFileWriter:%p)",
			__LINE__, m_pFileWriter);
		return uRet;
	}
	
	m_perfNativeRender.CheckModuleUnitStart();

	while (getCurrentVideoRenderer() == m_hThemeRenderer){
		nexSAL_TaskSleep(100);
	}
	NXT_ThemeRenderer_AquireContext(getCurrentVideoRenderer());
	
	unsigned int uiFrameTime = 0;
	
#ifdef FOR_VIDEO_LAYER_FEATURE
	for (int i = 0; i < m_VideoLayer.size(); i++){
		if (NULL == m_VideoLayer[i])
			continue;
		/*if (m_VideoLayer[i]->isTrackTime(uiTime) == FALSE)
		{
			if (m_VideoLayer[i]->m_uiEndTime <= uiTime)
			{
				m_VideoLayer[i]->DequeueFrameOutBuffer();
			}
			continue;
		}
		else*/{
			while ((pFrame = m_VideoLayer[i]->getFrameOutBuffer())) {
				if( m_iVideoStarted == 0 || m_VideoLayer[i]->m_bFirstDisplay4Layer == 0)
					iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime, TRUE, TRUE);
				else
					iDrawRet = pFrame->drawVideo((void*)getCurrentVideoRenderer(), m_VideoLayer[i]->m_iVideoLayerTextureID, uiTime, FALSE, TRUE);
				m_VideoLayer[i]->m_bFirstDisplay4Layer = 1;
				if (iDrawRet == 1) // draw drop
				{
					m_VideoLayer[i]->m_iDrop++;
					m_VideoLayer[i]->DequeueFrameOutBuffer();
					uRet = 0;
				}
				else if (iDrawRet == 2) // draw wait
				{
					break;
				}
				else
				{
					m_VideoLayer[i]->m_iDrop = 0;
					m_VideoLayer[i]->DequeueFrameOutBuffer();
					drawCount++;
					break;
				}
			}
		}
	}
#endif	

	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Encode one frame(%d %d %d)", __LINE__, uiFrameTime, m_uiLastTime, uiTime);
	if( uiFrameTime > m_uiLastTime )
	{
		m_uiLastTime = uiFrameTime;
	}
	else
	{
		m_uiLastTime = uiTime;
	}

	m_uiLastTime = uiTime;

	if(drawCount == 0)
	{
		m_perfNativeRender.CheckModuleUnitEnd();
		NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 0);
		return uRet;
	}
	
	if( CNexVideoEditor::m_iUseSurfaceMediaSource == 1 )
	{
		unsigned int uiDuration	= 0;
		unsigned int uiSize		= 0;
		NXBOOL bEncodeEnd		= FALSE;

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport ", __LINE__);
			
		m_perfGlDraw.CheckModuleUnitStart();
		NXT_ThemeRenderer_SetCTS(getCurrentVideoRenderer(), uiTime);
		NXT_ThemeRenderer_GLDraw(getCurrentVideoRenderer(), NXT_RendererOutputType_RGBA_8888, 0);

		if( m_bNeedRenderFinish )
			NXT_ThemeRenderer_GLWaitToFinishRendering(getCurrentVideoRenderer());

		m_perfGlDraw.CheckModuleUnitEnd();

		// nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport ", __LINE__);

		m_pFileWriter->setMediaCodecTimeStamp(m_uiLastTime);

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport Begin Swap(%d)", __LINE__, m_uiLastTime);
		NXT_ThemeRenderer_SwapBuffers(getCurrentVideoRenderer());
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] DrawExport end Swap", __LINE__);
		NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 0);

#ifndef USE_WRITE_TASK
			m_pFileWriter->setBaseVideoFrame(m_uiLastTime, FALSE, &uiDuration, &uiSize, &bEncodeEnd);
			m_uiVideoExportFrameCount++;
			if( bEncodeEnd )
				m_uiVideoEncOutputFrameCount++;
#else
		CVideoWriteBuffer* pBuffer = m_pFileWriter->getBuffer();
	        pBuffer->m_eBufferFormat= STREAM_INPUT_BUFFER_FORMAT_MEDIABUFFER;
	        pBuffer->m_uiTime		= m_uiLastTime;
		m_pFileWriter->releaseBuffer(pBuffer);
#endif		
		m_perfNativeRender.CheckModuleUnitEnd();
		return uRet;
	}

	m_perfGlDraw.CheckModuleUnitStart();
	NXT_ThemeRenderer_SetCTS(getCurrentVideoRenderer(), uiTime);
#if defined(_ANDROID)
	NXT_ThemeRenderer_GLDraw(getCurrentVideoRenderer(), NXT_RendererOutputType_Y2CrA_8888, 0);
#elif defined(__APPLE__) // NESI-488
    NXT_ThemeRenderer_GLDraw(getCurrentVideoRenderer(), NXT_RendererOutputType_RGBA_8888_yflip, 0);
#endif

#ifdef CHECK_RENDER_FRAME
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] CHECK_RENDER_FRAME(%d)", __LINE__, g_bCheckFrame);
	if( g_bCheckFrame )
	{
		glColorMask(GL_TRUE,GL_FALSE,GL_FALSE,GL_FALSE);
	    glEnable(GL_SCISSOR_TEST);
	    glScissor(0.75, 0.75, 1, 1);
	    glClearColor(1.0, 0, 0, 1.0);
	    glClear(GL_COLOR_BUFFER_BIT);
	}
	else
	{
		glColorMask(GL_FALSE,GL_TRUE,GL_FALSE,GL_FALSE);
	    glEnable(GL_SCISSOR_TEST);
	    glScissor(0.75, 0.75, 1, 1);
	    glClearColor(0, 1.0, 0, 1.0);
	    glClear(GL_COLOR_BUFFER_BIT);
	}
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	g_bCheckFrame = !g_bCheckFrame;
#endif
	
	NXT_ThemeRenderer_GLWaitToFinishRendering(getCurrentVideoRenderer());
	m_perfGlDraw.CheckModuleUnitEnd();

	int 				iWidth	= 0;;
	int 				iHeight	= 0;
	int 				iSize	= 0;

#if defined(__APPLE__) // NESI-488
    {
        void *pPlatformPixelBuff = NULL;
        unsigned int uiDuration, uiSize;
        iSize = sizeof(void*);
        NXT_ThemeRenderer_GetPixelBuffer( getCurrentVideoRenderer(), &pPlatformPixelBuff, &iSize );
        
        if( m_pFileWriter->setBaseVideoFrame(m_uiLastTime, (unsigned char*)pPlatformPixelBuff, iSize, &uiDuration, &uiSize, NULL) == FALSE )
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[VRTask.cpp %d] Video Frame Write failed", __LINE__);
		
		NXT_ThemeRenderer_ReturnPixelBufferForReuse(getCurrentVideoRenderer(), pPlatformPixelBuff);
        NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 0);
        return uRet;
    }
#endif
	CVideoWriteBuffer* pBuffer = m_pFileWriter->getBuffer();
	if( pBuffer )
	{
		m_perfGetPixel.CheckModuleUnitStart();
		if( NXT_ThemeRenderer_GetPixels2(getCurrentVideoRenderer(), &iWidth, &iHeight, &iSize, pBuffer->m_pY2UVA) == NXT_Error_None )
		{
			if( m_iEncodeWidth != iWidth || m_iEncodeHeight != iHeight )
			{
				m_perfGetPixel.CheckModuleUnitEnd();
				pBuffer->resetBuffer();
				m_pFileWriter->releaseBuffer(pBuffer);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] NXT_ThemeRenderer_GetPixels wrong Size(%d %d)", __LINE__, iWidth, iHeight);
				NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 1);
				m_perfNativeRender.CheckModuleUnitEnd();
				sendInterruptExportEvent();
				return uRet;
			}
			pBuffer->m_iWidth			= iWidth;
			pBuffer->m_iHeight		= iHeight;
			pBuffer->m_eBufferFormat	= STREAM_INPUT_BUFFER_FORMAT_Y2UVA;
			pBuffer->m_uiBufferSize	= iSize;
			pBuffer->m_uiTime		= m_uiLastTime;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] nativeVideoRender getPixels(W(%d) H(%d) Size(%p %d)) CTS(%d)", 
				__LINE__, iWidth, iHeight, pBuffer->m_pY2UVA, iSize, m_uiLastTime);
		}
		else
		{
			pBuffer->resetBuffer();
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] NXT_ThemeRenderer_GetPixels is failed", __LINE__);
		}
		m_perfGetPixel.CheckModuleUnitEnd();
		m_pFileWriter->releaseBuffer(pBuffer);
	}
	else
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] get FileWriter Buffer is null", __LINE__);
	}
	NXT_ThemeRenderer_ReleaseContext(getCurrentVideoRenderer(), 1);
	return uRet;
}

void CNEXThread_VideoRenderTask::fastOptionPreview(char* pOption, int iDisplay)
{
	if( pOption == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] fastOptionPreview was nothing because param was null", __LINE__);
		CNexProjectManager::sendEvent(MESSAGE_FAST_OPTION_PREVIEW_DONE, (unsigned int)NXT_Error_MissingParam);
		return;
	}
	
	if ( m_isGLOperationAllowed == FALSE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] isOperationAllowed is false", __LINE__);
		CNexProjectManager::sendEvent(MESSAGE_FAST_OPTION_PREVIEW_DONE, (unsigned int)NXT_Error_InvalidState);
		return;
	}
	if(NXT_Error_None != NXT_ThemeRenderer_AquireContext(m_hThemeRenderer)){

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[VRTask.cpp %d] Fail to aquireContext", __LINE__);
		CNexProjectManager::sendEvent(MESSAGE_FAST_OPTION_PREVIEW_DONE, (unsigned int)NXT_Error_NoContext);
		return;
	}
	NXT_Error eRet = NXT_ThemeRenderer_fastOptionPreview(m_hThemeRenderer, NXT_RendererOutputType_RGBA_8888, pOption);
	NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, iDisplay);

	CNexProjectManager::sendEvent(MESSAGE_FAST_OPTION_PREVIEW_DONE, (unsigned int)eRet);

}

void CNEXThread_VideoRenderTask::setIsGLOperationAllowed(bool allowed)
{
    CNxMsgSetGLOperationAllowed* msg = new CNxMsgSetGLOperationAllowed(allowed);
    SendCommand(msg);
    SAFE_RELEASE(msg);
}

void CNEXThread_VideoRenderTask::setIsGLOperationAllowed_internal(bool allowed)
{
	m_isGLOperationAllowed = allowed;
}

void CNEXThread_VideoRenderTask::recordUIDRenderPair(unsigned int uid, NXT_HThemeRenderer renderer){

    UIDRenderMapper_t::iterator itor = m_uidRenderMapper.find(uid);
    NXT_HThemeRenderer render = NULL;
    if(itor != m_uidRenderMapper.end()){

        itor->second = renderer;
        return;
    }

    m_uidRenderMapper.insert(make_pair(uid, renderer));
}

void CNEXThread_VideoRenderTask::removeUIDRenderPair(unsigned int uid){

    UIDRenderMapper_t::iterator itor = m_uidRenderMapper.find(uid);
    NXT_HThemeRenderer render = NULL;
    if(itor != m_uidRenderMapper.end()){

        render = itor->second;
        m_uidRenderMapper.erase(itor);
    }
}

NXT_HThemeRenderer CNEXThread_VideoRenderTask::getRendererForUID(unsigned int uid, bool remove){

    UIDRenderMapper_t::iterator itor = m_uidRenderMapper.find(uid);
    NXT_HThemeRenderer render = NULL;
    if(itor != m_uidRenderMapper.end()){

        render = itor->second;
        if(remove){

            m_uidRenderMapper.erase(itor);
        }
    }

    return render;
}

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/
