/******************************************************************************
* File Name   :    NEXVIDEOEDITOR_PlaybackCacheWorker.h
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

#ifndef __NEXVIDEOEDITOR_PLAYBACKCACHEWORKER_H__
#define __NEXVIDEOEDITOR_PLAYBACKCACHEWORKER_H__

#include "NEXVIDEOEDITOR_ThreadBase.h"
#include "NEXVIDEOEDITOR_EffectItemVec.h"
#include "NexThemeRenderer.h"

#define PLAYBACK_CACHE_WORKER_NAME "NEXVIDEOEDITOR PlaybackCache Worker"

class CNEXThread_PlaybackCacheWorker : public CNEXThreadBase
{
    CVideoEffectItemVec*            m_pEffectItemVec;
    NXT_HThemeRenderer              m_hThemeRenderer;
    unsigned int                    previous_cache_time_;
    unsigned int                    cache_worker_on_;
public:
    CNEXThread_PlaybackCacheWorker( void );
    virtual ~CNEXThread_PlaybackCacheWorker( void );

    virtual const char* Name(){

        return PLAYBACK_CACHE_WORKER_NAME;
    }

    virtual void End( unsigned int uiTimeout );

    void resetCacheTime(NXT_HThemeRenderer renderer, CVideoEffectItemVec* peffectitemvec, unsigned int time);

    void stopCaching();
protected:                                 
    virtual int OnThreadMain( void );
};

#endif // __NEXVIDEOEDITOR_PLAYBACKCACHEWORKER_H__
