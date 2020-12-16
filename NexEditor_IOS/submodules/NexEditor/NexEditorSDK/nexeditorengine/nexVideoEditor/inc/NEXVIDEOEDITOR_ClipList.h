/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_ClipList.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/07/25	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_CLIPLIST__H_
#define _NEXVIDEOEDITOR_CLIPLIST__H_
//---------------------------------------------------------------------------

#include "NEXVIDEOEDITOR_Clip.h"
#include "NEXVIDEOEDITOR_Layer.h"
#include "NEXVIDEOEDITOR_EffectItemVec.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"

#include "NEXVIDEOEDITOR_DrawInfoVec.h"

#include <map>


//---------------------------------------------------------------------------
class CClipList : public CNxRef<INxRefObj>
{
public:
	CClipList();
	virtual ~CClipList();

	virtual int getClipCount();
	virtual IClipItem* getClip(int nIndex);
	virtual IClipItem* getClipUsingID(unsigned int uiID);
	virtual IClipItem* createClipUsingID(unsigned int uiID, int isAudio = 0);
	virtual IClipItem* createEmptyClipUsingID(unsigned int uiID);

	virtual ILayerItem* createEmptyLayerUsingID(unsigned int uiID);

	virtual int addClipItem(IClipItem* pItem);
	
	virtual int addLayerItem(ILayerItem* pItem);
	virtual int deleteLayerItem(unsigned int uiID);
	virtual int clearLayerItem();
	virtual int addEffectItem(IEffectItem* pItem);

	virtual void lockClipList();
	virtual void unlockClipList();

	virtual int clearClipList();

	virtual int checkDirectExport(int option);
    void cancelcheckDirectExport();//yoon
	int checkAudioDirectExport();
	NXBOOL checkDirectExportForEncoder();

	int clearIDRFlagForDirectExport();
	int setIDRFlagForDirectExport();
	void setSeparateEffect(NXBOOL bSeparate);

	CVideoEffectItemVec* getEffectItemVec();

	CClipItem* getVisualClipUsingID(unsigned int uiClipID);
	CClipItem* getAudioClipUsingID(unsigned int uiClipID);
	CClipItem* getVisualClipUsingTime(unsigned int uiTime);
	CClipItem* getVisualClipUsingTimeAppliedEffect(unsigned int uiTime);
	int getBGMVolumeOfVisualClipAtCurrentTime(unsigned int uiTime);
		
	NXBOOL checkAudioClipCount(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int* pEndTime);

	int addVisualClip(unsigned int uiNextToClipID, CClipItem* pItem);
	int addAudioClip(unsigned int uiStartTime, CClipItem* pItem);
	int deleteClipItem(unsigned int uiClipID);

	int setBackGroundMusic(CClipItem* pItem);
	int setBackGroundMusicVolume(int iVolume, int iFadeInTime, int iFadeOutTime);

	int moveVisualClip(unsigned int uiNextToClipID, unsigned int uimovedClipID);
	int moveAudioClip(unsigned int uiMoveTime, unsigned int uimovedClipID);

	void resetClipReader();

	unsigned int getClipTotalTime();

	NXBOOL updatePlay(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);
	NXBOOL updatePlay(unsigned int uiTime, CNEXThreadBase* pAudioRender, CNexExportWriter* pFileWriter, char* pUserData, int option);
	NXBOOL updatePlay4Export(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);
	
	NXBOOL stopPlay();
	NXBOOL pausePlay();
	NXBOOL resumePlay(CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);

	NXBOOL setPreviewTime(unsigned int uiTime, int iIDRFrame, CNEXThreadBase* pVideoRander);
	NXBOOL clearImageTask();
	unsigned char* getPreviewThumbData(unsigned int uiTime);
	NXBOOL setPreviewThumb(unsigned int uiTime, void* pThumbRender, void* pOutputSurface, CNEXThreadBase* pVideoRander);
	NXBOOL unsetPreviewThumb();

	NXBOOL isAudioClipTime(unsigned int uiClipID, unsigned int uiTime);

	void updateClipInfo(IClipItem* pItem);
	
	void reCalcTime();
	void reCalcTimeForLoadList(NXBOOL bOnlyTotalTime = FALSE, std::map<int, int>* pmapper = nullptr);

	void printClipInfo();
	
	int getFirstClipCodecType(NXFF_MEDIA_TYPE eType);	
	int getFirstClipDSIofClip(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize );
	int getFirstClipRotateInfo(int* pRotate);
	
	CClipItem* getVideoClipItem(unsigned int uiStartTime, unsigned int uiEndTime);
	unsigned int getMaxHWUseSize(unsigned int uiTime);
	unsigned int getPreviewHWSize(unsigned int uiTime);
    
    int getVisualClipCountAtTime(unsigned int uiTime);
    int getAllVisualClipCountAtTime(unsigned int uiTime);
    int getVisualLayerCountAtTime(unsigned int uiTime);
    unsigned int* getVisualClipIDsAtTime(unsigned int uiTime, unsigned int* pCount);
    unsigned int getVisualIndex();
    void setVisualIndex(unsigned int uiIndex);

    int getVideoClipCount();

	int setVideoDecodingEnd(int iEnd);

	int checkVideoItems(CClipItem* pFirst, CClipItem* pSecond);
	int checkAudioItems(CClipItem* pFirst, CClipItem* pSecond);
	CClipVideoRenderInfo* getVideoRenderInfoByItem(CClipItem* pItem);
	CClipAudioRenderInfo* getAudioRenderInfoByItem(CClipItem* pItem);

	void setDrawInfos(IDrawInfoList* pDrawInfos);
	void updateDrawInfo(IDrawInfo* pDrawInfo);
	CNexDrawInfo* getDrawInfo(int id);

	void updateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face);
	void resetFaceDetectInfo(unsigned int clipid);
	bool checkFaceDetectProcessedAtTime(unsigned int uiTime);
public:
	int						m_iTemp;
	CClipItemVector			m_ClipItemVec;
	
	CClipItemVector			m_AudioClipItemVec;
	CLayerItemVector		m_LayerItemVec;
	
	CVideoEffectItemVec*	m_pEffectItemVec;
private:
	unsigned int			m_uiTotalTime;
	CNexLock				m_ClipListLock;

	CClipItem*				m_pBackGround;
	
	int						m_iBGMFadeInTime;
	int						m_iBGMFadeOutTime;

	NXBOOL					m_bSeparateEffect;

	NXBOOL					m_isCheckDirectExport;

	int						m_iDirectExportIndex;
	
	NXBOOL m_isCheckingDirectExport; //yoon
	NXBOOL m_isCancelDirectExport; //yoon

	unsigned int m_pVisualIDs[50];
	unsigned int m_uVisualIndex;	
};

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_CLIPLIST__H_
