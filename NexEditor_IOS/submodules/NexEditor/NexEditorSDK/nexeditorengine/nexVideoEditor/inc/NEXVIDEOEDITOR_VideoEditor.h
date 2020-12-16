/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_VideoEditor.h
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

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/05	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_VIDEOEDITOR_H__
#define __NEXVIDEOEDITOR_VIDEOEDITOR_H__

#include "NEXVIDEOEDITOR_Interface.h"
#include "NEXVIDEOEDITOR_ProjectManager.h"
#include "NEXVIDEOEDITOR_MsgDef.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_Property.h"
#include "NEXVIDEOEDITOR_UDTA.h"
#include "NexSAL_Com.h"
#include "NexCAL.h"

#include "nextreaming_configuration_reader.h"

#include "NexTheme.h"
#include "NexThemeRenderer.h"
#ifdef _ANDROID
#include <android/native_window.h>
#include "NexThemeRenderer_SurfaceTexture.h"
#endif

#define NEXEDITOR_MAJOR_VERSION	2
#define NEXEDITOR_MINOR_VERSION	12
#define NEXEDITOR_PATCH_VERSION	41
//#define NEXEDITOR_BUILD_VERSION 0


class CNexVideoEditor : public CNxRef<INexVideoEditor>
{
	NXT_HThemeRenderer getRenderer(int export_flag);
public:
	CNexVideoEditor( char* strLibPath, char* strModelName, int iAPILevel, int* pProperties, void* pVM, void* wstl );
	virtual ~CNexVideoEditor( void );

	static CNexVideoEditor* getVideoEditor();
	static char* getDeviceModel();
	static int getDeviceAPILevel();
	static int getConfigLogLevel();

	virtual int getProperty(const char* pName, char* pValue);
	virtual int setProperty(const char* pName, const char* pValue);

	virtual void commandMarker(int iTag);
	
	virtual int prepareSurface(void* pSurface);
	virtual void prepareAudio();

	virtual int setVideoPosition(int iLeft, int iTop, int iRight, int iBottom);

	virtual int createProject(CNexExportWriter *pWriter = NULL);
	virtual int closeProject();
	virtual int clearProject(int iOption = 0);
	virtual int setProjectEffect(char* pEffectID);
	virtual int setProjectVolumeFade(int iFadeInTime, int iFadeOutTime);
	virtual int setProjectVolume(int iProjectVolume);
	virtual int setProjectManualVolumeControl(int iManualVolumeControl);

	virtual int encodeProject(char* pEncodeFilePath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iMaxFileDuration, int iFPS, int iProjectWidth, int iProjectHeight, int iSamplingRate, int iAudioBitrate, int iProfile, int iLevel, int iVideoCodecType, int iFlag);

	virtual int encodeProjectJpeg(void* nativeWindow, char* pEncodeFilePath, int iWidth, int iHeight, int iQuality, int iFlag);

	virtual int addVisualClip(unsigned int uiNextToClipID, char* pFileint, int iNewClipID);
	virtual int addAudioClip(unsigned int uiStartTime, char* pFile, int iNewClipID);
	virtual int moveVisualClip(unsigned int uiNextToClipID, int imovedClipID, int isEventSend = 1);
	virtual int moveAudioClip(unsigned int uiStartTime, int imovedClipID, int isEventSend = 1);

	virtual int deleteClip(int iClipID);
	virtual int updateClipInfo();
	virtual int updateClipInfo(IClipItem* pClip);

	virtual int updateLoadList();

	virtual int setBackgroundMusic(const char* pFile, int iNewClipID);
	virtual int setBackgroundMusicVolume(int iVolume, int iFadeInTime, int iFadeOutTime);
	
	virtual unsigned int getDuration();
	virtual int setTime(unsigned int uiTime, int iDisplay = 1, int iIDRFrame = 1);
	virtual unsigned char* setTimeThumbData(unsigned int uiTime);
	virtual int captureCurrentFrame();
	
	virtual int startPlay(int iMuteAudio);
	virtual int pausePlay();
	virtual int resumePlay();
	virtual int stopPlay(int iFlag);

	virtual int pauseExport();
	virtual int resumeExport();

	virtual IClipList* getClipList();

	virtual IClipList* createClipList();
	virtual int updateClipList(IClipList* pCliplist, int iOption);

	virtual IEffectItem* createEffectItem();
	virtual int updateEffectList(IClipList* pCliplist, int iOption);

	virtual int loadTheme(const char* pThemeResource, int iFlags);
	virtual char* getLoadThemeError(int iFlags);

	virtual int clearRenderItems(int iFlags);
	virtual int loadRenderItem(const char* pID, const char* pRenderItemResource, int isFile);
	virtual char* getLoadRenderItemError(int iFrags);

	virtual int setEventHandler(INexVideoEditorEventHandler* pEventHandler);
	virtual IClipInfo* getClipInfo(char* pFilePath, char* pThumbFilePath, int isVideoThumbnail, int isAudioPCMLevel, int isBackground, int iUserTag);

	virtual IClipInfo* getClipInfo_Sync(char* pFilePath, int iFlag, int iUserTag, int* pRet);	
	virtual int getClipVideoThumb(char* pFilePath, char* pThumbFilePath, int iWidth, int iHeight, int iStartTime, int iEndTime, int iCount, int iFlag, int iUserTag);
	virtual int getClipVideoThumbWithTimeTable(char* pFilePath, char* pThumbFilePath, int iWidth, int iHeight, int iSize, int* pTimeTable, int iFlag, int iUserTag);
	virtual int getClipAudioThumb(char* pFilePath, char* pThumbFilePath, int iUserTag);
	virtual int getClipAudioThumbPartial(char* pFilePath, char* pThumbFilePath, int iStartTime, int iEndTime, int iUseCount, int iSkipCount, int iUserTag);

	virtual int getClipStopThumb(int iUserTag);
	
	virtual int getClipVideoThumbRaw(char* pFilePath, int* pTimeTable, int iTimeTableCount, int iFlag, int iUserTag);
	virtual int getClipAudioThumbRaw(char* pFilePath, int iStartTime, int iEndTime, int iFlag, int iUserTag);

	virtual void clearScreen(int iTag);
	virtual int fastOptionPreview(const char* pStringOption, int iDisplay = 1);	

	virtual int clearTrackCache();
	virtual int getChipsetType();

	virtual int getVersionNumber(int iVersion);

	virtual int startVoiceRecorder(char* pFilePath, int iSampleRate, int iChannels, int iBitForSample);
	virtual int processVoiceRecorder(int iPCMSize, unsigned char* pPCMBuffer);
	virtual IClipInfo* endVoiceRecorder();

	virtual int getCurrentRenderImage(unsigned int uiTime);

	virtual int transcodingStart(const char* pSrcClipPath, const char* pDstClipPath, int iWidth, int iHeight, int iDisplayWidth, int iDisplayHeight, int iBitrate, long long llMaxFileSize, int iFPS, int iFlag, int iSpeedFactor, const char* pUser);
	virtual int transcodingStop();

	virtual int highlightStart(const char* pSrcClipPath, int iIndexMode, int iRequestInterval, int iRequestCount, int iOutputMode, const char* pDstClipPath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iDecodeMode);
	virtual int highlightStop();

	virtual int checkDirectExport(int option);
	virtual int directExport(const char* pEncodeFilePath, long long llMaxFileSize, int iMaxFileDuration, const char* pUserData, int iFlag);

	virtual int checkPFrameDirectExportSync(const char* pSrc);

	virtual int reverseStart(const char* pSrcClipPath, const char* pDstClipPath, const char* pTempClipPath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, int iStartTime, int iEndTime, int iDecodeMode);
	virtual int reverseStop();

	virtual int fastPreviewStart(unsigned int uiStartTime, unsigned int uiEndTime, int displayWidth, int displayHeight);
	virtual int fastPreviewStop();
	virtual int fastPreviewTime(unsigned int uiTime);	

	virtual int checkIDRStart(const char* pSrcClipPath);
	virtual int checkIDREnd();
	virtual int checkIDRTime(unsigned int uiTime);

	virtual int setVolumeWhilePlay(int iMasterVolume, int iSlaveVolume);
	virtual int getVolumeWhilePlay(int* pMasterVolume, int* pSlaveVolume);

	virtual int addUDTA(int iType, const char* pData);
	virtual int clearUDTA();

	virtual int setVideoTrackUUID(int mode, unsigned char *byteUUID);
	virtual int set360VideoTrackPosition(int iXAngle,int iYAngle, int iflags);
	virtual int releaseLUTTexture(int lut_resource_id);//yoon
    virtual int getThemeProperty(int nPropertyName, void** ppRetValue);
    virtual int setLayerWithRect(int x, int y, int width, int height, int scale);

	virtual int setTaskSleep(int sleep);

	virtual int* createCubeLUT(const char* stream);
	virtual int* createLGLUT(const char* stream);

	virtual int setBrightness(int value);
	virtual int setContrast(int value);
	virtual int setSaturation(int value);
	virtual int setVignette(int value);
	virtual int setVignetteRange(int value);
	virtual int setSharpness(int value);

	virtual int getBrightness();
	virtual int getContrast();
	virtual int getSaturation();
	virtual int getVignette();
	virtual int getVignetteRange();
	virtual int getSharpness();

	//Jeff
	virtual int setBaseFilterRenderItem(const char* uid);
	virtual int getTexNameForClipID(int export_flag, int clipid);
	virtual int setTexNameForLUT(int export_flag, int clipid, float x, float y);
	virtual int getTexNameForMask(int export_flag);
    virtual int getTexNameForBlend(int export_flag);
	virtual int getTexNameForWhite(int export_flag);
	virtual int cleanupMaskWithWhite(int export_flag);
    virtual int swapBlendMain(int export_flag);
	virtual int setRenderToMask(int export_flag);
	virtual int setRenderToDefault(int export_flag);
	virtual int getLUTWithID(int lut_resource_id, int export_flag);
	virtual int getVignetteTexID(int export_flag);
	virtual int createRenderItem(int export_flag, const char* effect_id);
	virtual int releaseRenderItem(int export_flag, int effect_id);
    virtual int drawRenderItemOverlay(int effect_id, int tex_id_for_second, int export_flag, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mask_enabled);
	virtual int createNexEDL(int export_flag, const char* effect_id);
	virtual int releaseNexEDL(int export_flag, int effect_id);
	virtual int drawNexEDLOverlay(int effect_id, int export_flag, char* effect_options, int current_time, int start_time, int end_time, float* pmatrix, float left, float top, float right, float bottom, float alpha_val, int mode);

	virtual unsigned int getAudioSessionID();

	virtual int setPreviewScaleFactor(float iPreviewScaleFactor);//Jeff
	virtual float getPreviewScaleFactor();//Jeff
	virtual int pushLoadedBitmap(const char* path, int* pixels, int width, int height, int loadedtype);
	virtual int removeBitmap(const char* path);

	virtual IDrawInfoList* createDrawInfoList();
	virtual IDrawInfo* createDrawInfo();
	virtual int setDrawInfoList(IDrawInfoList* master, IDrawInfoList* sub);
	virtual int setDrawInfo(IDrawInfo* drawinfo);
	
	virtual int setDeviceLightLevel(int lightLevel);
	virtual int setDeviceGamma(float gamma);
	virtual int setForceRTT(int val);
	virtual int setThumbnailRoutine(int value);
	virtual int getThumbnailRoutine();

	virtual int setGIFMode(int mode);

	//yoon
#ifdef FOR_LAYER_FEATURE
	virtual ILayerItem* createLayerItem();
	virtual int addLayerItem(ILayerItem* pLayer);
	virtual int deleteLayerItem(int iLayerID);
	virtual int clearLayerItem();
#endif
	virtual void setIsGLOperationAllowed(bool allowed);

	int callbackGetEffectImagePath(const char* input, char* output);

	char* getDefaultClipEffect();
	char* getDefaultTitleEffect();
	int getEffectProperty(char* pEffectID, unsigned int uiProperty);

	int getJPEGImageHeader(char* pFile, int* pWidth, int* pHeight, int* pPitch);
	int getJPEGImageData(char* pFile, int* pWidth, int* pHeight, int* pPitch, unsigned char* pY, unsigned char* pU, unsigned char* pV);
	
	void* getExportRenderer();
	void* getExportOutputSurface();	

	void* getMediaCodecCallback();
	void* getMediaCodecInputSurf();
	void* getMediaCodecSetTimeStampOnSurf();
	void* getMediaCodecResetVideoEncoder();
	void* getMediaCodecSetCropAchieveResolution();
	void* getAudioRenderFuncs();

	NXBOOL existProperty(const char* pName);
	int getPropertyInt(const char* pName, int iDefaultValue = 0);
	NXBOOL getPropertyBoolean(const char* pName, NXBOOL bDefaultValue = FALSE);

	int notifyEvent(unsigned int uiEventType, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0, unsigned int uiParam3 = 0, unsigned int uiParam4 = 0);
	int notifyError(unsigned int uiEventType, unsigned int uiResult, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0);

	int callbackCapture(int iWidth, int iHeight, int iSize, char* pBuffer);

	int callbackGetThemeFile(char* pImageFilePath, unsigned int uiFileLen, int* pLength, char** ppImageData);
	int callbackGetThemeImage(char* pImageFilePath, int asyncmode, unsigned int uiFileLen, int* pWidth, int* pHeight4, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData);
	int callbackCheckImageWorkDone();
	int callbackGetImageUsingFile(char* pImageFilePath, int iThumb, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData, int* pLoadedType);
	int callbackGetImageUsingText(char* pText, unsigned int uiTextLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppTextImageData, int* pTextImageDataSize, void** ppUserData);
	int callbackReleaseImage(void** ppUserData);
	int callbackGetImageHeaderUsingFile(char* pImageFilePath, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel);


	void* callbackGetAudioTrack(int iSampleRate, int iChannels);
	void callbackReleaseAudioTrack();
 	void* callbackGetAudioManager();
	void callbackReleaseAudioManager();
	
	int callbackCustomLayer(	int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
									int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
									int iParam11, int iParam12, int iParam13, int iParam14, int iParam15, int iParam16, int iParam17, int iParam18);
	
	int callbackThumb(int iMode, int iTag, int iTime, int iWidth, int iHeight, int iCount, int iTotal, int iSize, char* pBuffer);
	int callbackHighLightIndex(int iCount, int* pBuffer);
	int getUDTACount();
	CNexUDTA* getUDTA(int iIndex);
    unsigned char * getVideoTrackUUID(int * pSize); //yoon
    int getVideoTrackUUIDMode(); //yoon
	int get360VideoTrackPositionRadian(int iAxis); //yoon
	int updateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face);
	int resetFaceDetectInfo(unsigned int clipid);
	int setEncInfo(const int infoCnt, const NXCHAR* pInfo[]);
	int getEncInfoCnt() {return m_iEncInfoCnt;};
	NXCHAR *getEncInfo(const int idx) { return m_pEncInfo[idx];};
public:
	static CNexVideoEditor*			m_pThisEditor;
	static int						m_iSupportedWidth;
	static int						m_iSupportedHeight;
	static int						m_iSupportedMaxMemSize;
	static int						m_iSupportedMinWidth;
	static int						m_iSupportedMinHeight;
	
	static int						m_iSupportedMaxProfile;
	static int						m_iSupportedMaxLevel;
	static int						m_iSupportedMaxFPS;
	static int						m_iInputMaxFPS;
	static NXBOOL					m_bNexEditorSDK;

	static int						m_iMC_HW_H264BaselineMaxLevel;
	static int						m_iMC_HW_H264MainMaxLevel;
	static int						m_iMC_HW_H264HighMaxLevel;

	static int						m_iMC_HW_H264BaselineMaxSize;
	static int						m_iMC_HW_H264MainMaxSize;
	static int						m_iMC_HW_H264HighMaxSize;	

	static int						m_iMC_SW_H264BaselineMaxLevel;
	static int						m_iMC_SW_H264MainMaxLevel;
	static int						m_iMC_SW_H264HighMaxLevel;

	static int						m_iMC_SW_H264BaselineMaxSize;
	static int						m_iMC_SW_H264MainMaxSize;
	static int						m_iMC_SW_H264HighMaxSize;

	static int						m_iNX_SW_H264BaselineMaxLevel;
	static int						m_iNX_SW_H264MainMaxLevel;
	static int						m_iNX_SW_H264HighMaxLevel;

	static int						m_iNX_SW_H264BaselineMaxSize;
	static int						m_iNX_SW_H264MainMaxSize;
	static int						m_iNX_SW_H264HighMaxSize;
	
	static int						m_iSupportDecoderMaxCount;
	static int						m_iSupportEncoderMaxCount;
	static int						m_iUseSurfaceMediaSource;
	static int						m_isRGBADevice;
	static int						m_iSupportAACProfile;
	static NXBOOL					m_bSupportAACSWCodec;
	static NXBOOL					m_bSuppoertSWH264Codec;
	static NXBOOL					m_bSuppoertSWMP4Codec;
	static NXBOOL					m_bPrepareVideoCodec;
	static NXBOOL					m_bSupportSWMCH264;
	static NXBOOL					m_bTrustAudioMonoInfoFromCodec;
  static NXBOOL                     m_bForceDirectExport; //yoon for LG
	static int						m_iDeviceMaxLightLevel; //Hdr
	static float					m_fDeviceMaxGamma; //Hdr
	static NXBOOL					m_bHDR2SDR; //Hdr
	static int						m_iSupportContentDuration;


	unsigned int					m_uiAudioEncUserData;
	char							m_strDeviceModel[MODEL_NAME_MAX_LENGTH];
	int							m_iDeviceAPILevel;
	int							m_iAudioMultiChannelOut;
	int							m_bSupportFrameTimeChecker;
	static int 						m_iVideoLayerProject;
	static NXBOOL					m_bSupportPeakMeter;
	static NXBOOL					m_bVideoDecoderSW;
private:
	INexVideoEditorEventHandler*	m_pVideoEditorEventHandler;

	CNexProjectManager*			m_pProjectManager;

	char*						m_pVoiceRecordPath;
	unsigned char*				m_pPCMTempBuffer;
	unsigned int					m_uiPCMTempBufferSize;

	unsigned long	long				m_ullTotalSampleCount;
	unsigned int					m_uiCurrentTime;
	CNexExportWriter*				m_pFileWriter; // MONGTO 20130320  FOR VoiceRecorder 

	unsigned int					m_uiSampleRate;
	unsigned int					m_uiChannels;
	unsigned int					m_uiBitPerSample;

	NXT_HThemeRenderer_Context 	m_hContext;
#ifdef __APPLE__
	NXT_HThemeRenderer_Context 	m_hExportContext;
#endif
	NXT_HThemeRenderer			m_hRenderer;
	NXT_HThemeRenderer			m_hExportRenderer;
	void*						m_hOutputSurface;

	CNexPropertyVec				m_PropertyVec;

	NexConf 						m_Config;

	ANativeWindow*				m_pNativeSurface;

	void* 						m_pMediaCodecInputSurf;

	CClipItem*					m_pIDRCheckItem;
#ifdef _ANDROID	
	vecUDTA						m_vecUDTA;
#endif
	
	int 						iVideoTrackUUIDMode;//yoon
	//yoon	
	int 						m_i360VideoTrackPositionXAngle;
	int 						m_i360VideoTrackPositionYAngle;
	int 						m_i360VideoViewMode;

	float 						m_fPreviewScaleFactor;
	int							m_iThumbnailRoutine; // 0 : thumbnail to original, 1 : thumbnail, 2 : original

	int 						m_iBrightness;
	int 						m_iContrast;
	int 						m_iSaturation;
	int 						m_iVignette;
	int 						m_iVignetteRange;
	int 						m_iSharpness;

	NXCHAR						**m_pEncInfo;
	int							m_iEncInfoCnt;
};

int LoadThemeFileCallback(char** ppOutputData, int* pLength, char* path, void* cbdata);

#endif // __NEXVIDEOEDITOR_VIDEOEDITOR_H__

/*-----------------------------------------------------------------------------
Revision History 
Author		Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/08/05	Draft.
-----------------------------------------------------------------------------*/
