/******************************************************************************
* File Name   :    NEXVIDEOEDITOR_CNEXThread_PlaybackCacheWorker.h
* Description :    
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author    Date        Description of Changes
-------------------------------------------------------------------------------
jeff        2018/08/10    Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_PlaybackCacheWorker.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_Util.h"
#include <string>
#include <vector>

CNEXThread_PlaybackCacheWorker::CNEXThread_PlaybackCacheWorker( void ){

    previous_cache_time_ = 0;
    m_pEffectItemVec = NULL;
    m_hThemeRenderer = NULL;
}

CNEXThread_PlaybackCacheWorker::~CNEXThread_PlaybackCacheWorker( void ){

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlaybackCacheWorker.cpp %d] ~CNEXThread_PlaybackCacheWorker", __LINE__);    
}

void CNEXThread_PlaybackCacheWorker::resetCacheTime(NXT_HThemeRenderer renderer, CVideoEffectItemVec* peffectitemvec, unsigned int time){

    SAFE_RELEASE(m_pEffectItemVec);
    m_pEffectItemVec = peffectitemvec;
    SAFE_ADDREF(m_pEffectItemVec);
    m_hThemeRenderer = renderer;
    previous_cache_time_ = time;
    cache_worker_on_ = 1;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlaybackCacheWorker.cpp %d] Reset(%d)", __LINE__, previous_cache_time_);
}

void CNEXThread_PlaybackCacheWorker::stopCaching(){

    cache_worker_on_ = 0;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlaybackCacheWorker.cpp %d] Request To Stop", __LINE__);    
}

static void premultAlpha(unsigned int *pixels, unsigned int numPixels) {
    
#ifdef __APPLE__ // NESI-380 pixels provided from iOS SDK is already premultified alpha
    return;
#else
    if( !pixels )
        return;
    
    unsigned char *p = (unsigned char*)pixels;
    unsigned char *end = p + (numPixels*4);
    
    for( ; p<end; p+=4 ) {
        unsigned char a = *(p+3);
        *(p+0) = *(p+0) * a / 255;
        *(p+1) = *(p+1) * a / 255;
        *(p+2) = *(p+2) * a / 255;

    }
#endif
}


int CNEXThread_PlaybackCacheWorker::OnThreadMain( void ){

    nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[PlaybackCacheWorker.cpp %d] PlaybackCacheWorker Begin", __LINE__);

    while(cache_worker_on_){

        unsigned int current_time = CNexProjectManager::getCurrentTimeStamp();

        if(previous_cache_time_ >= current_time){

            nexSAL_TaskSleep(100);
            continue;
        }

        unsigned int target_start_time = previous_cache_time_ + 3000;
        unsigned int target_end_time = target_start_time + 3000;
        previous_cache_time_ = target_start_time;

        nexSAL_TraceCat(0, 0, "[PlaybackCacheWorker.cpp %d] start:%d end:%d BEGIN", __LINE__, target_start_time, target_end_time);

        typedef std::vector<NXT_PrecacheResourceInfo> Resourcelist_t;
        typedef std::vector<std::string> Effectlist_t;

        Resourcelist_t load_resource_list;
        Effectlist_t effect_list;

        if(m_pEffectItemVec){

            m_pEffectItemVec->lock();
            for (int i = 0; i < m_pEffectItemVec->getCount(); i++)
            {
                CVideoEffectItem *peffectitem = m_pEffectItemVec->getEffetItem(i);

                if(peffectitem->m_eEffectType == EFFECT_TYPE_TRANSITION){

                    if(peffectitem->m_uiEffectEndTime < target_start_time)
                        continue;
                    if(peffectitem->m_uiEffectStartTime > target_end_time)
                        continue;
                }
                else if(peffectitem->m_eEffectType == EFFECT_TYPE_TITLE){

                    if(peffectitem->m_uiTitleEndTime < target_start_time)
                        continue;
                    if(peffectitem->m_uiTitleStartTime > target_end_time)
                        continue;
                }
                else
                    continue;

                const char *effect_name = peffectitem->getClipEffectID();
                const char *titleeffect_name = peffectitem->getTitleEffectID();
                if(effect_name)
                    effect_list.push_back(std::string(effect_name));

                if(titleeffect_name)
                    effect_list.push_back(std::string(titleeffect_name));
            }
            m_pEffectItemVec->unlock();

            if(effect_list.size() > 0){

                NXT_ThemeRenderer_AquireContext(m_hThemeRenderer);
            }

            for(Effectlist_t::iterator itor = effect_list.begin(); itor != effect_list.end(); ++itor){

                NXT_ThemeRenderer_GetPrecacheEffectResource(m_hThemeRenderer, (*itor).c_str(), &load_resource_list);
            }

            if(effect_list.size() > 0){

                NXT_ThemeRenderer_ReleaseContext(m_hThemeRenderer, 0);
            }

            typedef std::map<std::string, NXT_PrecacheResourceInfo> mapper_t;

            mapper_t mapper;

            for(Resourcelist_t::iterator itor = load_resource_list.begin(); itor != load_resource_list.end(); ++itor){

                NXT_PrecacheResourceInfo& info = *itor;
                mapper_t::iterator itor_for_find = mapper.find(info.name);
                if(itor_for_find == mapper.end()){

                    mapper.insert(make_pair(std::string(info.name), info));
                }
                else{

                    if(info.name)
                        free(info.name);
                    if(info.uid)
                        free(info.uid);
                    info.name = NULL;
                    info.uid = NULL;
                }
            }

            for(mapper_t::iterator itor = mapper.begin(); itor != mapper.end(); ++itor){

                NXT_PrecacheResourceInfo& info = itor->second;
                NXT_ThemeRenderer_DoPlayCacheWork(m_hThemeRenderer, &info, target_start_time, target_end_time);
                if(info.name)
                    free(info.name);
                if(info.uid)
                    free(info.uid);
                info.name = NULL;
                info.uid = NULL;
            }

            NXT_ThemeRenderer_ClearCachedBitmap(m_hThemeRenderer, current_time > 3000?current_time - 3000:0, current_time);
        }

        nexSAL_TraceCat(0, 0, "[PlaybackCacheWorker.cpp %d] start:%d end:%d END", __LINE__, target_start_time, target_end_time);
    }

    return TRUE;
}

void CNEXThread_PlaybackCacheWorker::End( unsigned int uiTimeout ){

    if( m_bIsWorking == FALSE ) return;
    if( m_hThread == NEXSAL_INVALID_HANDLE ) return;

    if( nexSAL_TaskWait(m_hThread) != 0 )
    {
        // nexSAL_TaskTerminate(m_hThread);
    }

    if( m_hSema != NEXSAL_INVALID_HANDLE )
    {
        nexSAL_SemaphoreDelete(m_hSema);
        m_hSema = NEXSAL_INVALID_HANDLE;
    }
    
    nexSAL_TaskDelete(m_hThread);
    m_hThread = NEXSAL_INVALID_HANDLE;
    m_bIsWorking = FALSE;
	NXT_ThemeRenderer_ClearCachedBitmap(m_hThemeRenderer, 0xFFFFFFFF, 0xFFFFFFFF);
    nexSAL_TraceCat(0, 0, "[PlaybackCacheWorker.cpp %d] END", __LINE__);
}

