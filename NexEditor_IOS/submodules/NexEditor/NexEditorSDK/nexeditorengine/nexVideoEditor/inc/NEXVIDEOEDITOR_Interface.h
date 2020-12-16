/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Interface.h
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


//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_INTERFACE__H_
#define _NEXVIDEOEDITOR_INTERFACE__H_
//---------------------------------------------------------------------------

#include "nexDef.h"
#include "nexIRef.h"
#include "NEXVIDEOEDITOR_Types.h"
#include "NEXVIDEOEDITOR_EventTypes.h"
#include "NEXVIDEOEDITOR_Error.h"
#include "NEXVIDEOEDITOR_BaseRect.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_ExportWriter.h"

#define NEXEDITOR_INERFACE_MAJOR_VERSION	1
#define NEXEDITOR_INERFACE_MINOR_VERSION	1

//---------------------------------------------------------------------------
class IRectangle :public INxRefObj
{
public:
	virtual ~IRectangle(){};
	virtual int getLeft() = 0;
	virtual int getTop() = 0;
	virtual int getBottom() = 0;
	virtual int getRight() = 0;
	virtual int setRect(int iLeft = 0, int iTop = 0, int iRight = 0, int iBottom = 0) = 0;
};

//---------------------------------------------------------------------------
class IClipItem : public INxRefObj 
{
public:
	virtual ~IClipItem(){};
	// Gerneral Clip Property
	virtual int getClipID() = 0;

	virtual CLIP_TYPE getClipType() = 0;
	virtual int setClipType(CLIP_TYPE type) = 0;
	
	virtual unsigned int getTotalAudioTime() = 0;
	virtual unsigned int getTotalVideoTime() = 0;
	virtual unsigned int getTotalTime() = 0;
	virtual int setTotalTime(unsigned int uiTotalTime) = 0;

	virtual const char* getClipPath() = 0;
	virtual int setClipPath(const char* pClipPath) = 0;
	
	virtual int isVideoExist() = 0;
	virtual int setVideoExist(int iExist) = 0;
	virtual int isAudioExist() = 0;
	virtual int setAudioExist(int iExist) = 0;

	virtual int getWidth() = 0;
	virtual int setWidth(int iWidth) = 0;
	virtual int getHeight() = 0;
	virtual int setHeight(int iHeight) = 0;

	virtual int getVideoFPS() = 0;
	virtual float getVideoFPSFloat() = 0;	
	virtual int getVideoH264Profile() = 0;
	virtual int getVideoH264Level() = 0;
	virtual int getVideoOrientation() = 0;

	virtual int getVideoBitRate() = 0;
	virtual int getAudioBitRate() = 0;

	virtual int getAudioSampleRate() = 0;
	virtual int getAudioChannels() = 0;	

	virtual int getSeekPointCount() = 0;
	virtual int setSeekPointCount(int iSeekPointCount) = 0;

	virtual unsigned int getStartTime() = 0;
	virtual int setStartTime(unsigned int uiStartTime) = 0;
	virtual unsigned int getEndTime() = 0;
	virtual int setEndTime(unsigned int uiEndTime) = 0;
	
	virtual unsigned int getStartTrimTime() = 0;
	virtual int setStartTrimTime(unsigned int uiStartTime) = 0;
	virtual unsigned int getEndTrimTime() = 0;
	virtual int setEndTrimTime(unsigned int uiEndTime) = 0;

	virtual void printClipInfo() = 0;

	// Clip Dimension Property
	virtual int getFaceDetectProcessed() = 0;
	virtual IRectangle* getStartPosition() = 0;
	virtual IRectangle* getEndPosition() = 0;
	virtual IRectangle* getDstPosition() = 0;
	virtual IRectangle* getFacePosition() = 0;

	virtual float* getStartMatrix(int* pCount) = 0;
	virtual int setStartMatrix(float* pMatrix, int iCount) = 0;

	virtual float* getEndMatrix(int* pCount) = 0;
	virtual int setEndMatrix(float* pMatrix, int iCount) = 0;

	virtual unsigned int getTitleStartTime() = 0;
	virtual int setTitleStartTime(unsigned int uiStartTime) = 0;
	virtual unsigned int getTitleEndTime() = 0;
	virtual int setTitleEndTime(unsigned int uiEndTime) = 0;
	virtual int getTitleStyle() = 0;
	virtual int setTitleStyle(int iStyle) = 0;
	virtual char* getTitle() = 0;
	virtual int setTitle(char* pTitle) = 0;

	// ThumbNail Property
	virtual const char* getThumbnailPath() = 0;
	virtual int setThumbnailPath(const char* pThumbnailPath) = 0;

	virtual int getClipEffectDuration() = 0;
	virtual int setClipEffectDuration(int iTime) = 0;

	virtual int getClipEffectOffset() = 0;
	virtual int setClipEffectOffset(int iEffectOffset) = 0;
	virtual int getClipEffectOverlap() = 0;
	virtual int setClipEffectOverlap(int iEffectOverlap) =0;

	virtual const char* getClipEffectID() = 0;
	virtual int setClipEffectID(const char* pStrID) = 0;

	virtual const char* getTitleEffectID() = 0;
	virtual int setTitleEffectID(const char* pStrID) = 0;

	virtual const char* getFilterID() = 0;
	virtual int setFilterID(const char* pStrID) = 0;

	virtual int getRotateState() = 0;
	virtual int setRotateState(int iRotate) = 0;

	virtual int getBrightness() = 0;
	virtual int setBrightness(int iBrightness) = 0;
	virtual int getContrast() = 0;
	virtual int setContrast(int iContrast) = 0;
	virtual int getSaturation() = 0;
	virtual int setSaturation(int iTintcolor) = 0;
    virtual int getHue() = 0;
	virtual int setHue(int iTintcolor) = 0;
	virtual int getTintcolor() = 0;
	virtual int setTintcolor(int iTintcolor) = 0;
	virtual int getLUT() = 0;
	virtual int setLUT(int iLUT) = 0;
	virtual int getCustomLUTA() = 0;
	virtual int setCustomLUTA(int LUT) = 0;
	virtual int getCustomLUTB() = 0;
	virtual int setCustomLUTB(int LUT) = 0;
	virtual int getCustomLUTPower() = 0;
	virtual int setCustomLUTPower(int power) = 0;
	virtual int setVignette(int iVignette) = 0;
	virtual int getVignette() = 0;
	virtual int getVideoRenderMode() = 0; //yoon
	virtual int getHdrMetaData(NEXCODECUTIL_SEI_HDR_INFO *out_hdr_meta) = 0; //yoon
	virtual int setSpeedCtlFactor(int iFactor) = 0;
	virtual int getSpeedCtlFactor() = 0;
	virtual int setVoiceChangerFactor(int iFactor) = 0;
	virtual int getVoiceChangerFactor() = 0;
	// Audio Clip Property;
	// volume range -50 ~ 50;
	virtual int getAudioOnOff() = 0;
	virtual int setAudioOnOff(int bOn) = 0;

	virtual int getAutoEnvelop() = 0;
	virtual int setAutoEnvelop(int bOn) = 0;

	virtual int getAudioVolume() = 0;
	virtual int setAudioVolume(int iVolume) = 0;

	virtual int getBGMVolume() = 0;
	virtual int setBGMVolume(int iVolume) = 0;
	virtual int* getPanFactor() = 0;
	virtual int getPanLeftFactor() = 0;
	virtual int getPanRightFactor() = 0;
	virtual int setPanLeftFactor(int iPanFactor) = 0;
	virtual int setPanRightFactor(int iPanFactor) = 0;
	virtual int getMusicEffector() = 0;
	virtual int setMusicEffector(int bOn) = 0;
	virtual int getProcessorStrength() = 0;
	virtual int setProcessorStrength(int bOn) = 0;
	virtual int getBassStrength() = 0;
	virtual int setBassStrength(int bOn) = 0;
	virtual int getSlowMotion() = 0;
	virtual int setSlowMotion(int bOn) = 0;

	virtual int getPitchFactor() = 0;
	virtual int setPitchFactor(int bOn) = 0;

	virtual int getKeepPitch() = 0;
	virtual int setKeepPitch(int bOn) = 0;

	virtual int getCompressorFactor() = 0;
	virtual int setCompressorFactor(int bOn) = 0;

	virtual char* getEnhancedAudioFilter() = 0;
	virtual int setEnhancedAudioFilter(char* pEnhancedAudioFilter) = 0;

	virtual char* getEqualizer() = 0;
	virtual int setEqualizer(char* pEqualizer) = 0;

	virtual int getAudioPCMLevelCount() = 0;
	virtual int getAudioPCMLevelValue(int uiIndex) = 0;

	virtual int getAudioClipCount() = 0;
	virtual IClipItem* getAudioClip(int iIndex) = 0;

	virtual unsigned int getVisualClipID() = 0;
	virtual void setVisualClipID(unsigned int uiVisualClipID) = 0;

	virtual int addAudioClip(IClipItem* pClip) = 0;
	virtual int deleteAudioClip(unsigned int uiClipID) = 0;

	virtual int setAudioEnvelop(int nSize, unsigned int* puiTime = NULL, unsigned int* puiVolume = NULL)= 0;
	virtual int clearAudioEnvelop()= 0;
	virtual int getAudioEnvelopSize()= 0;
	virtual int getAudioEnvelop(int nIndex, unsigned int* puiTime, unsigned int* puiVolume)= 0;
	virtual int getAudioEnvelop(int* pnSize, unsigned int** ppuiTime, unsigned int** ppuiVolume)= 0;
	virtual int setAudioEnvelopOnOff(int bOnOff)= 0;
	virtual int isAudioEnvelopEnabled()= 0;
	virtual int setMotionTracked(int bMotionTracked)=0;
	virtual int setReverse(int bReverse)=0;

	virtual int setFreezeDuration(int duration) = 0;
	virtual int setIframePlay(int iframePlay) = 0;
	
};

class ILayerItem : public INxRefObj 
{
public:
	virtual ~ILayerItem(){};
	// Gerneral Clip Property
	virtual int getID() = 0;
	virtual int setID(unsigned int id) = 0;

	virtual CLIP_TYPE getType() = 0;
	virtual int setType(CLIP_TYPE type) = 0;
	
	virtual unsigned int getTotalTime() = 0;
	virtual int setTotalTime(unsigned int uiTotalTime) = 0;

	virtual unsigned int getStartTime() = 0;
	virtual int setStartTime(unsigned int uiStartTime) = 0;
	virtual unsigned int getEndTime() = 0;
	virtual int setEndTime(unsigned int uiEndTime) = 0;

	virtual unsigned int getStartTrimTime() = 0;
	virtual int setStartTrimTime(unsigned int uiStartTime) = 0;
	virtual unsigned int getEndTrimTime() = 0;
	virtual int setEndTrimTime(unsigned int uiEndTime) = 0;

	virtual int getWidth() = 0;
	virtual int setWidth(int iWidth) = 0;
	virtual int getHeight() = 0;
	virtual int setHeight(int iHeight) = 0;
	
	virtual int isVideoExist() = 0;
	virtual int setVideoExist(int iExist) = 0;

	virtual int getBrightness() = 0;
	virtual int setBrightness(int iBrightness) = 0;
	virtual int getContrast() = 0;
	virtual int setContrast(int iContrast) = 0;
	virtual int getSaturation() = 0;
	virtual int setSaturation(int iTintcolor) = 0;
    virtual int getHue() = 0;
	virtual int setHue(int iHue) = 0;
	virtual int getTintcolor() = 0;
	virtual int setTintcolor(int iTintcolor) = 0;	
	virtual int getLUT() = 0;
	virtual int setLUT(int iLUT) = 0;
	virtual int setVignette(int iVignette) = 0;
	virtual int getVignette() = 0;
	virtual int getSpeedCtlFactor() = 0;
	virtual int setSpeedCtlFactor(int iFactor) = 0;

	virtual int getRotateState() = 0;
	virtual int setRotateState(int iRotate) = 0;

	virtual const char* getDisplayOption() = 0;
	virtual int setDisplayOption(const char* pDisplayOption) = 0;
	
	virtual const char* getPath() = 0;
	virtual int setPath(const char* pPath) = 0;

	virtual void printLayerInfo() = 0;
};

//---------------------------------------------------------------------------
class IEffectItem :public INxRefObj
{
public:
	virtual ~IEffectItem(){};
	virtual int setEffectInfo(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int uiEffectTime, unsigned int uiDuration, int iOffset, int iOverlap, const char* pEffectID, const char* pTitle, int iCurrentIdx, int iTotalCount) = 0;
	virtual int setTitleEffectInfo(unsigned int uiStartTime, unsigned int uiEndTime, unsigned int uiTitleStartTime, unsigned int uiTitleEndTime, const char* pTitleID, const char* pTitle, int iCurrentIdx, int iTotalCount) = 0;

};

//---------------------------------------------------------------------------
class IClipList :public INxRefObj
{
public:
	virtual ~IClipList(){};
	virtual int getClipCount() = 0;
	virtual IClipItem* getClip(int nIndex) = 0;
	virtual IClipItem* getClipUsingID(unsigned int uiID) = 0;
	virtual IClipItem* createClipUsingID(unsigned int uiID, int isAudio = 0) = 0;
	virtual IClipItem* createEmptyClipUsingID(unsigned int uiID) = 0;

	virtual ILayerItem* createEmptyLayerUsingID(unsigned int uiID) = 0;

	virtual int addClipItem(IClipItem* pItem) = 0;
	
	virtual int addLayerItem(ILayerItem* pItem) = 0;
	virtual int deleteLayerItem(unsigned int uiID) = 0;
	virtual int clearLayerItem() = 0;

	virtual int addEffectItem(IEffectItem* pItem) = 0;

	virtual void lockClipList() = 0;
	virtual void unlockClipList() = 0;
	
	virtual int clearClipList() = 0;

	virtual int checkDirectExport(int option) = 0;
};

//---------------------------------------------------------------------------
class IDrawInfo : public INxRefObj
{
public:
	virtual ~IDrawInfo(){};

	virtual void setDrawInfo(unsigned int id,  unsigned int trackid, unsigned int subeffectid, unsigned int isTransition, unsigned int starttime, unsigned int endtime, int rotate, int user_rotate, int translate_x, int translate_y, int lut, int custom_lut_a, int custom_lut_b, int custom_lut_power) = 0;
	virtual void setEffect(const char* effect) = 0;
	virtual void setTitle(const char* title) = 0;
	virtual void setStartRect(int left, int top, int right, int bottom) = 0;
	virtual void setEndRect(int left, int top, int right, int bottom) = 0;
	virtual void setFaceRect(int left, int top, int right, int bottom) = 0;
	virtual void setColor(int brightness, int saturation, int contrast, int tintcolor, int hue) = 0;
};

class IDrawInfoList : public INxRefObj
{
public:
	virtual ~IDrawInfoList(){};

	virtual IDrawInfo* createDrawInfo() = 0;
	virtual void addDrawInfo(IDrawInfo* info) = 0;
	virtual void clearDrawInfo() = 0;
};


//---------------------------------------------------------------------------
class INexVideoEditorEventHandler : public INxRefObj
{
public:
	virtual ~INexVideoEditorEventHandler(){};
	virtual int notifyEvent(unsigned int uiEventType, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0, unsigned int uiParam3 = 0, unsigned int uiParam4 = 0) = 0;
	virtual int notifyError(unsigned int uiEventType, unsigned int uiResult, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0) = 0;
	virtual int callbackCapture(int iWidth, int iHeight, int iSize, char* pBuffer) = 0;
	
	virtual void callbackGetThemeImage(char* pImageFilePath, int asyncmode, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData)	 = 0;
	virtual int callbackCheckImageWorkDone() = 0;
	virtual void callbackGetImageUsingFile(char* pImageFilePath, int iThumb, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData, int* pLoadedType) = 0;
	virtual void callbackGetImageUsingText(char* pText, unsigned int uiTextLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppTextImageData, int* pTextImageDataSize, void** ppUserData) = 0;
	virtual void callbackReleaseImage(void** ppUserData) = 0;	
	virtual void callbackGetImageHeaderUsingFile(char* pImageFilePath, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel) = 0;

	virtual void* callbackGetAudioTrack(int iSampleRate, int iChannels) = 0;
	virtual void callbackReleaseAudioTrack() = 0;

	virtual void* callbackGetAudioManager() = 0;
	virtual void callbackReleaseAudioManager() = 0;

	virtual int callbackCustomLayer(	int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
									int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
									int iParam11, int iParam12, int iParam13, int iParam14, int iParam15, int iParam16, int iParam17, int iParam18) = 0;
    virtual int callbackUserRequest(int,void*) = 0;

	virtual int callbackThumb(int iMode, int iTag, int iTime, int iWidth, int iHeight, int iCount, int iTotal, int iSize, char* pBuffer) = 0;
	virtual int callbackHighLightIndex(int iCount, int* pBuffer) = 0;
	virtual void callbackGetThemeFile(char* pImageFilePath, unsigned int uiFileLen, int* pLength, char** ppImageData)	 = 0;

	virtual int getLUTWithID(int lut_resource_id, int export_flag) = 0;
	virtual int getVignetteTexID(int export_flag) = 0;
	virtual int getAssetResourceKey(const char * input_resourcpath, char *output_key) = 0;

};

//---------------------------------------------------------------------------
class IClipInfo :public INxRefObj
{
public:
	virtual ~IClipInfo(){};
	virtual int existVideo() = 0;
	virtual int existAudio() = 0;
	virtual int getAudioCodecType() = 0;
	virtual int getVideoCodecType() = 0;
	virtual int getWidth() = 0;
	virtual int getHeight() = 0;
	virtual int getDisplayWidth() = 0;
	virtual int getDisplayHeight() = 0;

	virtual int getVideoFPS() = 0;
	virtual float getVideoFPSFloat() = 0;
	virtual int getVideoH264Profile() = 0;
	virtual int getVideoH264Level() = 0;
	virtual int getVideoH264Interlaced() = 0;
	virtual int getVideoOrientation() = 0;

	virtual int getVideoBitRate() = 0;
	virtual int getAudioBitRate() = 0;

	virtual int getAudioSampleRate() = 0;
	virtual int getAudioChannels() = 0;
	
	virtual unsigned int getClipAudioDuration() = 0;
	virtual unsigned int getClipVideoDuration() = 0;
	virtual const char* getThumbnailPath() = 0;

	virtual int getSeekPointCount() = 0;

	virtual int getSeekTableCount() = 0;
	virtual int getSeekTableValue(int uiIndex) = 0;
	
	virtual int getVideoRenderType() = 0;
	virtual int getVideoHDRType() = 0;
	virtual const char* getVideoUUID(int * size ) = 0;
	virtual int getEditBoxTIme(unsigned int* piAudioEditBoxTime, unsigned int* piVideoEditBoxTime) = 0;

};

//---------------------------------------------------------------------------
class INexVideoEditor :public INxRefObj
{
public:
	virtual ~INexVideoEditor(){};

	virtual int getProperty(const char* pName, char* pValue) = 0;
	virtual int setProperty(const char* pName, const char* pValue) = 0;
	
	virtual void commandMarker(int iTag) = 0;
	
	virtual int prepareSurface(void* pSurface) = 0;
	virtual void prepareAudio() = 0;

	virtual int setVideoPosition(int iLeft, int iTop, int iRight, int iBottom) = 0;
	
	virtual int createProject(CNexExportWriter *pWriter = NULL) = 0;
	virtual int closeProject() = 0;
	virtual int clearProject(int iOption = 0) = 0;
	virtual int setProjectEffect(char* pEffectID) = 0;
	virtual int setProjectVolumeFade(int iFadeInTime, int iFadeOutTime) = 0;
	virtual int setProjectVolume(int iProjectVolume) = 0;
	virtual int setProjectManualVolumeControl(int iManualVolumeControl) = 0;

	virtual int encodeProject(char* pEncodeFilePath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iMaxFileDuration, int iFPS, int iProjectWidth, int iProjectHeight, int iSamplingRate, int iAudioBitrate, int iProfile, int iLevel, int iVideoCodecType, int iFlag) = 0;

	virtual int encodeProjectJpeg(void* nativeWindow, char* pEncodeFilePath, int iWidth, int iHeight, int iQuality, int iFlag) = 0;

	virtual int addVisualClip(unsigned int uiNextToClipID, char* pFile, int iNewClipID) = 0;
	virtual int addAudioClip(unsigned int uiStartTime, char* pFile, int iNewClipID) = 0;
	virtual int moveVisualClip(unsigned int uiNextToClipID, int imovedClipID, int isEventSend = 1) = 0;
	virtual int moveAudioClip(unsigned int uiStartTime, int imovedClipID, int isEventSend = 1) = 0;
	virtual int deleteClip(int iClipID) = 0;
	virtual int updateClipInfo() = 0;
	virtual int updateClipInfo(IClipItem* pClip) = 0;

	virtual int updateLoadList() = 0;

	virtual int setBackgroundMusic(const char* pFile, int iNewClipID) = 0;
	virtual int setBackgroundMusicVolume(int iVolume, int iFadeInTime, int iFadeOutTime) = 0;

	virtual unsigned int getDuration() = 0;
	virtual int setTime(unsigned int uiTime, int iDisplay = 1, int iIDRFrame = 1) = 0;
	virtual unsigned char* setTimeThumbData(unsigned int uiTime) = 0;
	virtual int captureCurrentFrame() = 0;

	virtual int startPlay(int iMuteAudio) = 0;
	virtual int pausePlay() = 0;
	virtual int resumePlay() = 0;
	virtual int stopPlay(int iFlag) = 0;

	virtual int pauseExport() = 0;
	virtual int resumeExport() = 0;

	virtual IClipList* getClipList() = 0;

	virtual IClipList* createClipList() = 0;
	virtual int updateClipList(IClipList* pCliplist, int iOption) = 0;

	virtual IEffectItem* createEffectItem() = 0;
	virtual int updateEffectList(IClipList* pCliplist, int iOption) = 0;

	virtual int loadTheme(const char* pThemeResource, int isFile) = 0;
	virtual char* getLoadThemeError(int iFlags) = 0;

	virtual int clearRenderItems(int iFlags) = 0;
	virtual int loadRenderItem(const char* pID, const char* pRenderItemResource, int isFile) = 0;
	virtual char* getLoadRenderItemError(int iFrags) = 0;

	virtual int setEventHandler(INexVideoEditorEventHandler* pEventHandler) = 0;

	virtual IClipInfo* getClipInfo(char* pFilePath, char* pThumbFilePath, int isVideoThumbnail, int isAudioPCMLevel, int isBackground, int iUserTag) = 0;

	virtual IClipInfo* getClipInfo_Sync(char* pFilePath, int iFlag, int iUserTag, int* pRet) = 0;
	virtual int getClipVideoThumb(char* pFilePath, char* pThumbFilePath, int iWidth, int iHeight, int iStartTime, int iEndTime, int iCount, int iFlag, int iUserTag) = 0;
	virtual int getClipVideoThumbWithTimeTable(char* pFilePath, char* pThumbFilePath, int iWidth, int iHeight, int iSize, int* pTimeTable, int iFlag, int iUserTag) = 0;
	virtual int getClipAudioThumb(char* pFilePath, char* pThumbFilePath, int iUserTag) = 0;
	virtual int getClipAudioThumbPartial(char* pFilePath, char* pThumbFilePath, int iStartTime, int iEndTime, int iUseCount, int iSkipCount, int iUserTag) = 0;

	virtual int getClipStopThumb(int iUserTag) = 0;

	virtual int getClipVideoThumbRaw(char* pFilePath, int* pTimeTable, int iTimeTableCount, int iFlag, int iUserTag) = 0;
	virtual int getClipAudioThumbRaw(char* pFilePath, int iStartTime, int iEndTime, int iFlag, int iUserTag) = 0;

	virtual void clearScreen(int iTag) = 0;
	virtual int fastOptionPreview(const char* pStringOption, int iDisplay = 1) = 0;

	virtual int clearTrackCache() = 0;
	virtual int getChipsetType() = 0;

	virtual int getVersionNumber(int iVersion) = 0;
	
	virtual int startVoiceRecorder(char* pFilePath, int iSampleRate, int iChannels, int iBitForSample) = 0;
	virtual int processVoiceRecorder(int iPCMSize, unsigned char* pPCMBuffer) = 0;
	virtual IClipInfo* endVoiceRecorder() = 0;

	virtual int getCurrentRenderImage(unsigned int uiTime) = 0;

	virtual int transcodingStart(const char* pSrcClipPath, const char* pDstClipPath, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, int iBitrate, long long llMaxFileSize, int iFPS, int iFlag, int iSpeedFactor, const char* pUser) = 0;
	virtual int transcodingStop() = 0;

#ifdef FOR_LAYER_FEATURE
	virtual ILayerItem* createLayerItem() = 0;
	virtual int addLayerItem(ILayerItem* pLayer) = 0;
	virtual int deleteLayerItem(int iLayerID) = 0;
	virtual int clearLayerItem() = 0;
#endif

	virtual int highlightStart(const char* pSrcClipPath, int iIndexMode, int iRequestInterval, int iRequestCount, int iOutputMode, const char* pDstClipPath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iDecodeMode) = 0;
	virtual int highlightStop() = 0;

	virtual int checkDirectExport(int option) = 0;
	virtual int directExport(const char* pEncodeFilePath, long long llMaxFileSize, int iMaxFileDuration, const char* pUserData, int iFlag) = 0;

	virtual int checkPFrameDirectExportSync(const char* pSrc) = 0;

	virtual int reverseStart(const char* pSrcClipPath, const char* pDstClipPath, const char* pTempClipPath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iStartTime, int iEndTime, int iDecodeMode) = 0;
	virtual int reverseStop() = 0;

	virtual int fastPreviewStart(unsigned int uiStartTime, unsigned int uiEndTime, int displayWidth, int displayHeight) = 0;
	virtual int fastPreviewStop() = 0;
	virtual int fastPreviewTime(unsigned int uiTime) = 0;

	virtual int checkIDRStart(const char* pSrcClipPath) = 0;
	virtual int checkIDREnd() = 0;
	virtual int checkIDRTime(unsigned int uiTime) = 0;

	virtual int setVolumeWhilePlay(int iMasterVolume, int iSlaveVolume) = 0;
	virtual int getVolumeWhilePlay(int* pMasterVolume, int* pSlaveVolume) = 0;
	
	virtual int addUDTA(int iType, const char* pData) = 0;
	virtual int clearUDTA() = 0;
	virtual int setVideoTrackUUID(int mode, unsigned char *byteUUID) = 0;
	virtual int set360VideoTrackPosition(int iXAngle,int iYAngle, int iflags)=0;
	virtual int releaseLUTTexture(int lut_resource_id) = 0;//yoon
    virtual int getThemeProperty(int nPropertyName, void** ppRetValue) = 0;
	virtual int setLayerWithRect(int x, int y, int width, int height, int scale) = 0;

	//Jeff
	virtual int setBaseFilterRenderItem(const char* uid) = 0;
	virtual int getTexNameForClipID(int export_flag, int clipid) = 0;
	virtual int setTexNameForLUT(int export_flag, int clipid, float x, float y) = 0;
	virtual int getTexNameForMask(int export_flag) = 0;
    virtual int getTexNameForBlend(int export_flag) = 0;
	virtual int getTexNameForWhite(int export_flag) = 0;
    virtual int swapBlendMain(int export_flag) = 0;
	virtual int setRenderToMask(int export_flag) = 0;
	virtual int setRenderToDefault(int export_flag) = 0;
	virtual int cleanupMaskWithWhite(int export_flag) = 0;
	virtual int createRenderItem(int export_flag, const char* effect_id) = 0;
	virtual int releaseRenderItem(int export_flag, int effect_id) = 0;
    virtual int drawRenderItemOverlay(int effect_id, int tex_id_for_second, int export_flag, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mask_enabled) = 0;
	virtual int createNexEDL(int export_flag, const char* effect_id) = 0;
	virtual int releaseNexEDL(int export_flag, int effect_id) = 0;
	virtual int drawNexEDLOverlay(int effect_id, int export_flag, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mode) = 0;
	virtual unsigned int getAudioSessionID() = 0;

	virtual int setPreviewScaleFactor(float fPreviewScaleFactor) = 0;//Jeff
	virtual float getPreviewScaleFactor() = 0;//Jeff
	virtual int pushLoadedBitmap(const char* path, int* pixels, int width, int height, int loadedtype) = 0;//Jeff
	virtual int removeBitmap(const char* path) = 0;//Jeff

	virtual IDrawInfoList* createDrawInfoList() = 0;
	virtual IDrawInfo* createDrawInfo() = 0;
	virtual int setDrawInfoList(IDrawInfoList* master, IDrawInfoList* sub) = 0;
	virtual int setDrawInfo(IDrawInfo* drawinfo) = 0;
	virtual int setTaskSleep(int sleep) = 0;
	virtual int updateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face) = 0;
	virtual int resetFaceDetectInfo(unsigned int clipid) = 0;
	virtual int* createCubeLUT(const char* stream) = 0;
	virtual int* createLGLUT(const char* stream) = 0;
	virtual int setBrightness(int value) = 0;
	virtual int setContrast(int value) = 0;
	virtual int setSaturation(int value) = 0;
	virtual int setVignette(int value) = 0;
	virtual int setVignetteRange(int value) = 0;
	virtual int setSharpness(int value) = 0;

	virtual int getBrightness() = 0;
	virtual int getContrast() = 0;
	virtual int getSaturation() = 0;
	virtual int getVignette() = 0;
	virtual int getVignetteRange() = 0;
	virtual int getSharpness() = 0;

	virtual int setDeviceLightLevel(int lightLevel) = 0;
	virtual int setDeviceGamma(float gamma) = 0;
	virtual int setForceRTT(int val) = 0;
	virtual int setThumbnailRoutine(int value) = 0;
	virtual int getThumbnailRoutine() = 0;

	virtual int setGIFMode(int mode) = 0;
    
	virtual void setIsGLOperationAllowed(bool allowed) = 0;
	virtual int setEncInfo(const int infoCnt, const NXCHAR* pInfo[]) = 0;
};

//---------------------------------------------------------------------------
INexVideoEditor* CreateNexVideoEditor(int iMajor, int iMinor, char* strLibPath, char* strModelName, int iAPILevel, int iUserData, int* pProperties, void* pVM, void* wstl);
INexVideoEditor* CreateNexThumb(int iMajor, int iMinor, char* strLibPath, char* strModelName, int iAPILevel, int* pProperties, void* pVM, void* wstl);
INexVideoEditor* getNexVideoEditor();

int				InitUserData();
int 				getSystemProperty(const char* pStrName, char* pValue);

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_INTERFACE__H_

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/07/25	Draft.
-----------------------------------------------------------------------------*/
