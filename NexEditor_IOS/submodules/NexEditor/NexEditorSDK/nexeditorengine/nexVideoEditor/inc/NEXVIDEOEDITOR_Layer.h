/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Clip.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2014/12/2	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_LAYER__H_
#define _NEXVIDEOEDITOR_LAYER__H_
//---------------------------------------------------------------------------

#include "NEXVIDEOEDITOR_Interface.h"
#include "NexSAL_Internal.h"
#include <vector>
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_Rect.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"

class CNEXThread_VideoTask;
class CNEXThread_AudioTask;
class CNEXThread_AudioRenderTask;
class CNEXThread_VideoRenderTask;
class CVideoTrackInfo;
class CNexProjectManager;

#define TRANSITION_ITEM_NAME_LEN	128

class CLayerItem;
typedef std::vector<CLayerItem*> CLayerItemVectorBase;
typedef CLayerItemVectorBase::iterator CLayerItemVectorIterator;

//---------------------------------------------------------------------------
class CLayerItem : public CNxRef<ILayerItem>
{
public:
	CLayerItem();
	CLayerItem(unsigned int uiClipID);
	virtual ~CLayerItem();

	virtual int getID();
	virtual int setID(unsigned int id);
	
	virtual CLIP_TYPE getType();
	virtual int setType(CLIP_TYPE eType);

	virtual unsigned int getTotalTime();
	virtual int setTotalTime(unsigned int uiPlayTime);	

	virtual unsigned int getStartTime();
	virtual int setStartTime(unsigned int uiStartTime);
	virtual unsigned int getEndTime();
	virtual int setEndTime(unsigned int uiEndTime);

	virtual unsigned int getStartTrimTime();
	virtual int setStartTrimTime(unsigned int uiStartTime);
	virtual unsigned int getEndTrimTime();
	virtual int setEndTrimTime(unsigned int uiEndTime);

	virtual int getWidth();
	virtual int setWidth(int iWidth);
	virtual int getHeight();
	virtual int setHeight(int iHeight);

	virtual int isVideoExist();
	virtual int setVideoExist(int iExist);
	
	virtual int getBrightness();
	virtual int setBrightness(int iBrightness);
	virtual int getContrast();
	virtual int setContrast(int iContrast);
	virtual int getSaturation();
	virtual int setSaturation(int iSaturation);
    virtual int getHue();
    virtual int setHue(int iHue);
	virtual int getTintcolor();
	virtual int setTintcolor(int iTintcolor);
	virtual int getLUT();
	virtual int setLUT(int iLUT);
	virtual int setVignette(int iVignette);
	virtual int getVignette();
	
	virtual int getRotateState();
	virtual int setRotateState(int iRotate);
	
	virtual int setSpeedCtlFactor(int iFactor);
	virtual int getSpeedCtlFactor();

	virtual const char* getDisplayOption();
	virtual int setDisplayOption(const char* pDisplayOption);
	
	virtual const char* getPath();
	virtual int setPath(const char* pClipPath);

	virtual void printLayerInfo();

	NXBOOL isPlayTime(unsigned int uiCurrentTime, NXBOOL bPrepare = FALSE);

	int initFileReader();
	int deinitFileReader();

	NXBOOL registeLayerTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView = FALSE);
	void deregisteLayerTrackInfo(CNEXThreadBase* pVideoRender);

	NXBOOL setPreview(unsigned int uiCurrentTimeStamp, CNEXThreadBase* pVideoRender);
	NXBOOL unsetPreview();

	NXBOOL updatePlayTime(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);
	NXBOOL startPlay(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);
	NXBOOL stopPlay()	;

private:
	void clearLayerInfo();

public:
	int 						m_iTemp;
	CNexFileReader*			m_pFFReader;
	CNEXThread_VideoTask*	m_pVideoTask;
	CNEXThread_AudioTask*	m_pAudioTask;

	CVideoTrackInfo*			m_pTrackInfo;

private:
	unsigned int			m_uiClipID;
 	CLIP_TYPE			m_ClipType;

	unsigned int			m_uiTotalTime;
	unsigned int			m_uiStartTime;
	unsigned int			m_uiEndTime;

	unsigned int			m_uiStartTrimTime;
	unsigned int			m_uiEndTrimTime;

	int					m_iWidth;
	int					m_iHeight;

	int					m_isVideoExist;

	int					m_iRotateState;
	int					m_iBrightness;
	int					m_iContrast;
	int					m_iSaturation;
    int                 m_iHue;
	int					m_iTintcolor;
	int 				m_iLUT;
    int 				m_iVignette;

	int					m_iSpeedCtlFactor;

	char					m_strEffectID[TRANSITION_ITEM_NAME_LEN];
	char					m_strFilterID[TRANSITION_ITEM_NAME_LEN];
 
	char*				m_strDisplayOption;
	char*				m_strFilePath;

	CNexLock			m_Lock;

	NXBOOL				m_isPlay;
	
};

//---------------------------------------------------------------------------
class CLayerItemVector :public CLayerItemVectorBase
{
public:
	CLayerItemVector()
	{
	}
	~CLayerItemVector()
	{
	}
};

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_CLIP__H_
