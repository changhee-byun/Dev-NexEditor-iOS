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
cskb		2011/07/25	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_CLIP__H_
#define _NEXVIDEOEDITOR_CLIP__H_
//---------------------------------------------------------------------------

#include "NEXVIDEOEDITOR_Interface.h"
#include "NexSAL_Internal.h"
#include <vector>
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_BaseRect.h"
#include "NEXVIDEOEDITOR_Rect.h"
#include "NEXVIDEOEDITOR_WrapFileReverse.h"
#include "NEXVIDEOEDITOR_WrapFileReader.h"
#include "NEXVIDEOEDITOR_WrapFileWriter.h"
#include "NEXVIDEOEDITOR_WrapSource.h"
#include "NEXVIDEOEDITOR_WrapOutSource.h"
#include "NEXVIDEOEDITOR_WrapIpodSource.h"

#include "NEXVIDEOEDITOR_DrawInfoVec.h"
#include "NexCodecUtil.h"

#define INVALID_CLIP_ID		0xFFFFFFFF

class CNEXThread_VideoTask;
class CNEXThread_AudioTask;
class CNEXThread_AudioRenderTask;
class CNEXThread_VideoRenderTask;
class CNEXThread_PFrameProcessTask;
class CNEXThread_DirectExportTask;
class CNEXThread_VideoPreviewTask;
class CNEXThread_VideoThumbTask;
class CVideoTrackInfo;
class CNexProjectManager;
class CNEXThread_ImageTask;

#define TRANSITION_ITEM_NAME_LEN	128
#define TRANSITION_ITEM_ICON_NAME_LEN	32
#define MATRIX_MAX_COUNT					9

static float default_matrix[] = {1.0, 0, 0, 0, 1.0, 0, 0, 0, 1.0};

typedef struct _TRANSITION_ITEM
{
	char	m_strID[TRANSITION_ITEM_NAME_LEN];
	char	m_strName[TRANSITION_ITEM_NAME_LEN];
	char	m_strIcon[TRANSITION_ITEM_ICON_NAME_LEN];
	int m_iEffectOffset;
	int m_iEffectOverlap;
	int m_iEffectMaximumDuration;
	int m_iEffectMinimumDuration;
	int m_iEffectDefaultDuration;
	int m_iEffectIsGlobal;
}TRANSITION_ITEM;

class CClipVideoRenderInfo : public CNxRef<INxRefObj>
{
public:
	CClipVideoRenderInfo() {
		muiTrackID = 0;
		mClipType = CLIPTYPE_NONE;

		mStartTime = 0;
		mEndTime = 0;
		
		mStartTrimTime = 0;
		mEndTrimTime = 0;
		
		mRotateState = 0;
		mBrightness = 0;
		mContrast = 0;
		mSaturation = 0;
        mHue = 0;
		mTintcolor = 0;
		mLUT = 0;
		mCustomLUT_A = 0;
		mCustomLUT_B = 0;
		mCustomLUT_Power = 0;
		mVignette = 0;

		mFaceDetectProcessed = 0;
		mFaceDetected = 0;
		mSrc = {0, 0, 0, 0};
		mStart = {0, 0, 0, 0};
		mEnd = {0, 0, 0, 0};
		mFace = {0, 0, 0, 0};

		memset(mStartMatrix, 0x00, sizeof(mStartMatrix));
		memset(mEndMatrix, 0x00, sizeof(mEndMatrix));

		mByteCount = 0;
		mRGB = NULL;

		mImageY = NULL;
		mImageU = NULL;
		mImageV = NULL;

		mImageNV12 = NULL;

		mWidth = 0;
		mHeight = 0;
		mPitch = 0;

		mIsPreview = FALSE;
		mColorFormat = 0L;
		mIsNV12Jepg = FALSE;

		mVideoRenderMode = 0;
		m_iSpeedCtlFactor = 0;

		mHDRType = 0;
		memset(&mHdrInfo, 0x00, sizeof(NEXCODECUTIL_SEI_HDR_INFO));

		mFreezeDuration = 0;
	};
	virtual ~CClipVideoRenderInfo() {};

	unsigned int muiTrackID;

	CLIP_TYPE mClipType;

	unsigned int mStartTime;
	unsigned int mEndTime;

	unsigned int mStartTrimTime;
	unsigned int mEndTrimTime;

	int mRotateState;
	int mBrightness;
	int mContrast;
	int mSaturation;
    int mHue;
	int mTintcolor;
	int mLUT;
	int mCustomLUT_A;
	int mCustomLUT_B;
	int mCustomLUT_Power;
	int mVignette;
	
	// for Image
	int mFaceDetectProcessed;
	unsigned int mFaceDetected;
 	RECT mSrc;
	RECT mStart;
	RECT mEnd;
	RECT mFace;

	float	mStartMatrix[MATRIX_MAX_COUNT];
	float	mEndMatrix[MATRIX_MAX_COUNT];
	
	unsigned int mByteCount;
	unsigned char* mRGB;

	unsigned char* mImageY;
	unsigned char* mImageU;
	unsigned char* mImageV;

	unsigned char* mImageNV12;

	int mLoadedType;

	// for Video
	int mWidth;
	int mHeight;
	int mPitch;

	NXBOOL mIsPreview;
	NXINT64 mColorFormat;
	NXBOOL mIsNV12Jepg;

	int mVideoRenderMode; //yoon
	int m_iSpeedCtlFactor;

	int mHDRType;//yoon
	NEXCODECUTIL_SEI_HDR_INFO mHdrInfo; //yoon

	int mFreezeDuration;
	NXBOOL mIframePlay;
};

class CClipAudioRenderInfo : public CNxRef<INxRefObj>
{
public:
	CClipAudioRenderInfo() {
		muiTrackID = 0;

		mClipType = CLIPTYPE_NONE;

		mStartTime = 0;
		mEndTime = 0;
		mStartTrimTime = 0;
		mEndTrimTime = 0;

		m_iAudioOnOff = 0;
		m_iClipVolume = 0;

		m_iVoiceChangerFactor = 0;

		m_iCompressorFactor = 0;
		m_iPitchIndex = 0;
		m_iMusicEffector = 0;
		m_iProcessorStrength = 0;
		m_iBassStrength = 0;
		m_iPanFactor[0] = 0;
		m_iPanFactor[1] = 0;

		m_uiBGMEndTIme = 0;
		m_iBackGroundVolume = 0;
		m_pEnhancedAudioFilter = NULL;
		m_pEqualizer = NULL;

		m_iEnvelopAudioSize = 0;
		m_puiEnvelopAudioTime = 0;
		m_puiEnvelopAudioVolume = 0;
		m_bEnableEnvelopAudio = FALSE;

		m_iSpeedCtlFactor = 0;
		m_bKeepPitch	= TRUE;
	}

	virtual ~CClipAudioRenderInfo() {};

	unsigned int muiTrackID;

	CLIP_TYPE mClipType;

	unsigned int mStartTime;
	unsigned int mEndTime;

	unsigned int mStartTrimTime;
	unsigned int mEndTrimTime;

	int					m_iAudioOnOff;
	int					m_iClipVolume;

	int					m_iVoiceChangerFactor;

	int					m_iCompressorFactor;
	int					m_iPitchIndex;
	int					m_iMusicEffector;
	int					m_iProcessorStrength;
	int					m_iBassStrength;
	int					m_iPanFactor[2];

	unsigned int			m_uiBGMEndTIme;
	int					m_iBackGroundVolume;

	char*				m_pEnhancedAudioFilter;
	char*				m_pEqualizer;

	int					m_iEnvelopAudioSize;
	unsigned int*			m_puiEnvelopAudioTime;
	unsigned int*			m_puiEnvelopAudioVolume;
	NXBOOL				m_bEnableEnvelopAudio;

	int					m_iSpeedCtlFactor;
	NXBOOL				m_bKeepPitch;
};

typedef std::vector<TRANSITION_ITEM> TransitionItemVec;
typedef TransitionItemVec::iterator TransitionItemIterator;

class CClipItem;
typedef std::vector<CClipItem*> CClipItemVectorBase;
typedef CClipItemVectorBase::iterator CClipItemVectorIterator;

typedef std::vector<CClipVideoRenderInfo*> CClipVideoRenderInfoVec;
typedef CClipVideoRenderInfoVec::iterator CClipVideoRenderInfoVectorIterator;

typedef std::vector<CClipAudioRenderInfo*> CClipAudioRenderInfoVec;
typedef CClipAudioRenderInfoVec::iterator ClipAudioRenderInfoVectorIterator;

typedef enum _CACHE_VISUAL_CLIPTYPE{
	CACHE_TYPE_IMAGE,
	CACHE_TYPE_VIDEO
}CACHE_VISUAL_CLIPTYPE;

//yoon
typedef struct _RENDERITEM_ENTRY
{
    char m_strID[TRANSITION_ITEM_NAME_LEN];
    int m_iStartTime;
    int m_iEndTime;
}RENDER_ITEM;

#define MAX_RENDER_ITEM 4

//---------------------------------------------------------------------------
class CClipItem : public CNxRef<IClipItem>
{
public:
	CClipItem();
	CClipItem(unsigned int uiClipID);
	virtual ~CClipItem();

	virtual int getClipID();
	
	virtual CLIP_TYPE getClipType();
	virtual int setClipType(CLIP_TYPE eType);

	virtual const char* getClipPath();
	virtual int setClipPath(const char* pClipPath);
//	int setFilePath(char* pFile);

	virtual unsigned int getTotalAudioTime();
	virtual unsigned int getTotalVideoTime();
	virtual unsigned int getTotalTime();
	virtual int setTotalTime(unsigned int uiPlayTime);	

	virtual int isVideoExist();
	virtual int setVideoExist(int iExist);
	virtual int isAudioExist();
	virtual int setAudioExist(int iExist);

	virtual int getWidth();
	virtual int setWidth(int iWidth);
	virtual int getHeight();
	virtual int setHeight(int iHeight);

	virtual int getDisplayWidth();
	virtual int setDisplayWidth(int iDisplayWidth);
	virtual int getDisplayHeight();
	virtual int setDisplayHeight(int iDisplayHeight);

	virtual int getVideoH264Interlaced();
	virtual int setVideoH264Interlaced(NXBOOL isInterlaced);

	virtual int getVideoFPS();
	virtual float getVideoFPSFloat();
	virtual int getVideoH264Profile();
	virtual int getVideoH264Level();
	virtual int getVideoOrientation();

	virtual int getVideoBitRate();
	virtual int getAudioBitRate();

	virtual int getAudioSampleRate();
	virtual int getAudioChannels();	

	virtual int getSeekPointCount();
	virtual int setSeekPointCount(int iSeekPointCount);

	virtual unsigned int getStartTime();
	virtual int setStartTime(unsigned int uiStartTime);
	virtual unsigned int getEndTime();
	virtual int setEndTime(unsigned int uiEndTime);

	virtual unsigned int getStartTrimTime();
	virtual int setStartTrimTime(unsigned int uiStartTime);
	virtual unsigned int getEndTrimTime();
	virtual int setEndTrimTime(unsigned int uiEndTime);

	virtual void printClipInfo();

	// Dimension Clip Property
	virtual int getFaceDetectProcessed();
	virtual IRectangle* getStartPosition();
	virtual IRectangle* getEndPosition();
	virtual IRectangle* getDstPosition();
	virtual IRectangle* getFacePosition();

	virtual float* getStartMatrix(int* pCount);
	virtual int setStartMatrix(float* pMatrix, int iCount);

	virtual float* getEndMatrix(int * pCount);
	virtual int setEndMatrix(float* pMatrix, int iCount);
	
	virtual unsigned int getTitleStartTime();
	virtual int setTitleStartTime(unsigned int uiStartTime);
	virtual unsigned int getTitleEndTime();
	virtual int setTitleEndTime(unsigned int uiEndTime);
	virtual int getTitleStyle();
	virtual int setTitleStyle(int iStyle);
	virtual char* getTitle();
	virtual int setTitle(char* pTitle);

	// ThumbNail Property
	virtual const char* getThumbnailPath();
	virtual int setThumbnailPath(const char* pThumbnailPath);

	virtual int getClipEffectEnable();
	virtual int setClipEffectEnable(NXBOOL bEnable);
	
	virtual int getClipEffectDuration();
	virtual int setClipEffectDuration(int iTime);

	virtual int getClipEffectOffset();
	virtual int setClipEffectOffset(int iEffectOffset);

	virtual int getClipEffectOverlap();
	virtual int setClipEffectOverlap(int iEffectOverlap);

	virtual const char* getClipEffectID();
	virtual int setClipEffectID(const char* pStrID);

	virtual const char* getTitleEffectID();
	virtual int setTitleEffectID(const char* pStrID);

	virtual const char* getFilterID();
	virtual int setFilterID(const char* pStrID);
    virtual int setRenderItemIDs(const char* pStrIDs); //yoon

	virtual int getRotateState();
	virtual int setRotateState(int iRotate);

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
	virtual int setLUT(int LUT);
	virtual int getCustomLUTA();
	virtual int setCustomLUTA(int LUT);
	virtual int getCustomLUTB();
	virtual int setCustomLUTB(int LUT);
	virtual int getCustomLUTPower();
	virtual int setCustomLUTPower(int power);
	virtual int setVignette(int iVignette);
	virtual int getVignette();
	virtual int getVideoRenderMode(); //yoon
	virtual int getHdrMetaData(NEXCODECUTIL_SEI_HDR_INFO *out_hdr_meta); //yoon
	virtual int setSpeedCtlFactor(int iFactor);
	virtual int getSpeedCtlFactor();
	virtual int setVoiceChangerFactor(int iFactor);
	virtual int getVoiceChangerFactor();

	// Audio Clip Property;
	// volume range -50 ~ 50;
	virtual int getAudioOnOff();
	virtual int setAudioOnOff(int bOn);
	virtual int getAutoEnvelop();
	virtual int setAutoEnvelop(int bOn);
	virtual int getAudioVolume();
	virtual int setAudioVolume(int iVolume);
	virtual int getBGMVolume();
	virtual int setBGMVolume(int iVolume);
	virtual int* getPanFactor();
	virtual int getPanLeftFactor();
	virtual int getPanRightFactor();
	virtual int setPanLeftFactor(int iPanFactor);
	virtual int setPanRightFactor(int iPanFactor);

	virtual int getMusicEffector();
	virtual int setMusicEffector(int bOn);
	virtual int getProcessorStrength();
	virtual int setProcessorStrength(int bOn);
	virtual int getBassStrength();
	virtual int setBassStrength(int bOn);
	virtual int getSlowMotion();
	virtual int setSlowMotion(int iFactor);
	
	virtual int getPitchFactor();
	virtual int setPitchFactor(int bOn);
	virtual int getKeepPitch();
	virtual int setKeepPitch(int bOn);
	virtual int getCompressorFactor();
	virtual int setCompressorFactor(int bOn);

	virtual char* getEnhancedAudioFilter();
	virtual int setEnhancedAudioFilter(char* pEnhancedAudioFilter);

	virtual char* getEqualizer();
	virtual int setEqualizer(char* pEqualizer);

	virtual int getAudioPCMLevelCount();
	virtual int getAudioPCMLevelValue(int uiIndex);

	virtual int getSeekTableCount();
	virtual int getSeekTableValue(int uiIndex);

	virtual unsigned int getVisualClipID();
	virtual void setVisualClipID(unsigned int uiVisualClipID);
	
	virtual int getAudioClipCount();
	virtual IClipItem* getAudioClip(int iIndex);

	virtual int addAudioClip(IClipItem* pClip);
	virtual int deleteAudioClip(unsigned int uiClipID);

	CClipItem* getAudioClipUsingClipID(unsigned int uiClipID);

	virtual int getSubVideoClipCount();
	virtual IClipItem* getSubVideoClip(int iIndex);

	int addSubVideoClip(IClipItem* pClip);
	int deleteSubVideoClip(unsigned int uiClipID);
	
	CClipItem* getSubVideoClipUsingClipID(unsigned int uiClipID);
		
	void clearClipInfo();
	NXBOOL isPlayTime(unsigned int uiCurrentTime, NXBOOL bPrepare = FALSE);
	NXBOOL isPlayTimeAtDirectExport(unsigned int uiCurrentTime, NXBOOL bPrepare);
	NXBOOL isPlayTimeAudio(unsigned int uiBaseTime, unsigned int uiCurrentTime, NXBOOL bCheckOnOff = TRUE);

	NXBOOL setPreviewImage(unsigned int uiCurrentTimeStamp, CNEXThreadBase* pVideoRender);
	NXBOOL clearImageTask();
	NXBOOL setPreviewVideo(unsigned int uiCurrentTimeStamp, int iIDRFrame, CNEXThreadBase* pVideoRender);
	NXBOOL unsetPreview();

	unsigned char* getPreviewThumbData(unsigned int uiTime);
	NXBOOL setPreviewThumb(unsigned int uiCurrentTimeStamp, void* pThumbRender, void* pOutputSurface, CNEXThreadBase* pVideoRender);
	NXBOOL unsetPreviewThumb();

	NXBOOL startPlayImage(CNEXThreadBase* pVideoRender);
	NXBOOL startPlayVideo(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);
	NXBOOL startPlayVideo(unsigned int uiTime, CNEXThreadBase* pAudioRender, CNexExportWriter* pFileWriter, char* pUserData);
	NXBOOL startPlayAudio(unsigned int uiTime, CNEXThreadBase* pAudioRender, NXBOOL bBGM = FALSE);
	
	int playVideoForDirectExport(CNexExportWriter* pFileWriter, char* pUserData);
	
	NXBOOL updatePlayTime(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);
	NXBOOL updatePlayTime(unsigned int uiTime, CNEXThreadBase* pAudioRender, CNexExportWriter* pFileWriter, char* pUserData);
	NXBOOL updatePlay4Export(unsigned int uiTime, CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);
	
	NXBOOL stopPlay();

	NXBOOL pausePlayTime();
	NXBOOL resumePlayTime(CNEXThreadBase* pVideoRender, CNEXThreadBase* pAudioRender);

	CNexSource* getSource();

	void sendNotifyEvent(unsigned int uiEventType, unsigned int uiParam1, unsigned int uiParam2);

	int parseClipFile(const char* pFile, NXBOOL bVideoThumbnail, NXBOOL bAudioPCMLevel);
	int parseClipFile(const char* pFile, int iFlag);
	int parseClipFile(	const char* pFile, 
						NXBOOL bVideoThumbnail, 
						NXBOOL bAudioPCMLevel, 
						void* pThumbRender, 
						void* pOutputSurface, 
						int iWidth = 0, 
						int iHeight= 0, 
						int iStartTime = 0, 
						int iEndTime= 0, 
						int iCount = 0,
						int iTimeTableSize = 0,
						int* pTimeTable = NULL,
						int iFlag = 0,
						int iTag = 0,
						int iUseCount = 0,
						int iSkipCount = 0);
	int parseClipFile(	const char* pFile, 
						NXBOOL bVideoThumbnail, 
						NXBOOL bAudioPCMLevel, 
						int* pTimeTable, 
						int iTimeTableCount,			
						unsigned int uStartTime, 
						unsigned int uEndTime, 
						void* pThumbRender, 
						void* pOutputSurface,
						int iFlag);

	int parseClipFileWithoutVideo(const char* pFile);
	int initSource();
	int deinitSource();

	unsigned char* getImageData();
	int setImageData(unsigned char* pBuffer, int iSize);
	int setAudioClipEndTime(unsigned int uiTotalPlayTime);
	int getAudioClipCountAtTime(unsigned int uiTime);

	unsigned int getClipIndex();
	void setClipIndexInfo(unsigned int uiClipIndex);
	
	unsigned int getClipTotalCount();
	void setClipTotalCount(unsigned int uiClipCount);
	
	void printClipTime();


	// RYU 20130614.
	int setAudioEnvelop(int nSize, unsigned int* puiTime = NULL, unsigned int* puiVolume = NULL);
	int clearAudioEnvelop();
	int getAudioEnvelopSize();
	int getAudioEnvelop(int nIndex, unsigned int* puiTime, unsigned int* puiVolume);
	int getAudioEnvelop(int* pnSize, unsigned int** ppuiTime, unsigned int** ppuiVolume);
	int setAudioEnvelopOnOff(int bOnOff);
	int isAudioEnvelopEnabled();

	int setMotionTracked(int bMotionTracked);
	NXBOOL getMotionTracked();

	virtual int setFreezeDuration(int duration);
	int getFreezeDuration();
	int setReverse(int bReverse);
	virtual int setIframePlay(int iframePlay);
	int getIframePlay();

	int getCodecType(NXFF_MEDIA_TYPE eType);	
	int parseDSIofClip();
	int getDSIofClip(NXFF_MEDIA_TYPE eType, NXUINT8** ppDSI, NXUINT32* pDSISize );
	NXBOOL compareAudioDSIofClip(CClipItem* pClip);
	NXBOOL compareVideoDSIofClip(CClipItem* pClip);
	int compareVideoDSIWithEncoder();
	NXBOOL isStartIDRFrame();
	NXBOOL setStartIDRFrame(NXBOOL bFlag);
	int getContentOrientation();

	int getAudioTrackCount();
	int getVideoTrackCount();

	int	parseClipStop();
	int checkResourceSize(unsigned int uiTime);
	
	int findIDRFrameTime(unsigned int uiTime);
	unsigned char * getVideoTrackUUID(int * piSize); //yoon
	int getEditBoxTIme(unsigned int* piAudioEditBoxTime, unsigned int* piVideoEditBoxTime);

	int setVideoDecodingEnd(int iEnd);
	int getVideoDecodingEnd();
	NXBOOL updateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face);
	NXBOOL resetFaceDetectInfo(unsigned int clipid);
	bool checkFaceDetectProcessedAtTime(unsigned int uiTime);

	CNexDrawInfoVec* getDrawInfos();
	void setDrawInfos(CNexDrawInfoVec* pInfos);
	void updateDrawInfo(CNexDrawInfo* pInfo);
	CNexDrawInfo* getDrawInfo(int id);


	int isIDRFrame(unsigned int uiTimes);//yoon

	CClipAudioRenderInfo* getActiveAudioRenderInfo(unsigned int uiTime, NXBOOL bByReader);
	CClipVideoRenderInfo* getActiveVideoRenderInfo(unsigned int uiTime, NXBOOL bByReader, int * pIndex = NULL);

	NXBOOL registeImageTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView = FALSE);
	NXBOOL registeThumbImageTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView = FALSE);
	void deregisteImageTrackInfo(CNEXThreadBase* pVideoRender);
	CVideoTrackInfo* getCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, unsigned int uiClipID);

private:
	static unsigned int getUniqueClipID();
	static unsigned int getUniqueClipID(unsigned int uiClipID);
	NXBOOL updateTrackInfo(CVideoTrackInfo* pTrack);
	

	void setCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pImageTrackInfo);
	void clearCachedVisualTrackInfo(CACHE_VISUAL_CLIPTYPE eType, CVideoTrackInfo* pImageTrackInfo);

	NXBOOL registerMissingImageTrackInfo(CNEXThreadBase* pVideoRender, NXBOOL isPreView = FALSE);
	NXBOOL setMissingFileInfo(void* pInfo);

	NEXVIDEOEDITOR_ERROR checkVideoInfo();
	NEXVIDEOEDITOR_ERROR checkAudioInfo();
	
public:
	int 						m_iTemp;
	CNexSource*				m_pSource;
	CNEXThread_VideoTask*	m_pVideoTask;
	CNEXThread_AudioTask*	m_pAudioTask;
	CNEXThread_AudioTask*	m_pAudioTask2;
	CNEXThread_AudioTask*	m_pAudioTask3;
	CNEXThread_AudioTask*	m_pAudioTask4;
	CNEXThread_ImageTask*	m_pImageTask;
	
	#ifdef _ANDROID
	CNEXThread_PFrameProcessTask* m_pPFrameTask;
	CNEXThread_DirectExportTask* m_pDirectExportTask;
	#endif
	CNEXThread_VideoThumbTask*		m_pVideoThumbTask;
	CNEXThread_VideoPreviewTask*	m_pVideoPreviewTask;
	CClipItemVectorBase				m_AudioClipVec;
	CClipItemVectorBase				m_SubVideoClipVec;

	CVideoTrackInfo*			m_pImageTrackInfo;
	CClipVideoRenderInfoVec		m_ClipVideoRenderInfoVec;
	CClipAudioRenderInfoVec		m_ClipAudioRenderInfoVec;

private:
	unsigned int			m_uiClipID;
	unsigned int			m_uiVisualClipID;
 	CLIP_TYPE				m_ClipType;

	unsigned int			m_uiTotalAudioTime;
	unsigned int			m_uiTotalVideoTime;
	unsigned int			m_uiTotalTime;
	unsigned int			m_uiStartTime;
	unsigned int			m_uiEndTime;

	unsigned int			m_uiStartTrimTime;
	unsigned int			m_uiEndTrimTime;

	int					m_iWidth;
	int					m_iHeight;
	int					m_iDisplayWidth;
	int					m_iDisplayHeight;

	int					m_iVideoFPS;
	float				m_fVideoFPS;
	int					m_iVideoH264Profile;
	int					m_iVideoH264Level;

	int					m_iVideoBitRate;
	int					m_iAudioBitRate;

	int					m_iAudioSampleRate;
	int					m_iAudioChannels;

	int					m_iSeekPointCount;

	int					m_isVideoExist;
	int					m_isAudioExist;

	TITLE_STYLE			m_eTitleStyle;
	char*				m_pTitle;
	unsigned int			m_uiTitleStartTime;
	unsigned int			m_uiTitleEndTime;

	int  				m_iFaceDetectProcessed;
	CRectangle*			m_pStartRect;
	CRectangle*			m_pEndRect;
	CRectangle*			m_pDstRect;
	unsigned int 		m_uiFaceDetected;
	CRectangle*			m_pFaceRect;

	float					m_StartMatrix[MATRIX_MAX_COUNT];
	float					m_EndMatrix[MATRIX_MAX_COUNT];

	char*				m_strFilePath;
	char*				m_strThumbnailPath;

	int					m_iThumb; //0 - original, 1 - thumbnail
	
	int					m_iAudioOnOff;
	int					m_iAutoEnvelop;
	int					m_iClipVolume;
	int					m_iBackGroundVolume;
	int					m_iPanFactor[2];
	int					m_iMusicEffector;
	int					m_iProcessorStrength;
	int					m_iBassStrength;
	int					m_iSlowMotion;

	int					m_iPitchIndex;
	int					m_iCompressorFactor;

	char*				m_pEnhancedAudioFilter;
	char*				m_pEqualizer;

	int					m_iEnvelopAudioSize;
	unsigned int*			m_puiEnvelopAudioTime;
	unsigned int*			m_puiEnvelopAudioVolume;
	NXBOOL				m_bEnableEnvelopAudio;

	std::vector<int>		m_iClipPCMLevelVec;
	std::vector<int>		m_iClipSeekTabelVec;

	int					m_nFadeInDuration;
	int					m_nFadeOutDuration;
	
	NXBOOL				m_bEffectEnable;
	int					m_iEffectDuration;	// ms
	int					m_iEffectOffset;		// 0 ~ 100 %
	int					m_iEffectOverlap;	// 0 ~ 100 %

	char				m_strEffectClipID[TRANSITION_ITEM_NAME_LEN];
	char				m_strEffectTitleID[TRANSITION_ITEM_NAME_LEN];
	char				m_strFilterID[TRANSITION_ITEM_NAME_LEN];
	RENDER_ITEM 		m_RenerItems[MAX_RENDER_ITEM];  //yoon
	int 				m_iRenderItemCount; //yoon

	int					m_iRotateState;
	int					m_iBrightness;
	int					m_iContrast;
	int					m_iSaturation;
	int					m_iHue;
	int					m_iTintcolor;
	int 				m_iLUT;
	int 				m_iCustomLUT_A;
	int 				m_iCustomLUT_B;
	int 				m_iCustomLUT_Power;
	int 				m_iVignette;

	int					m_iSpeedCtlFactor;
	int					m_iVoiceChangerFactor;
	NXBOOL				m_bKeepPitch;

	unsigned char*		m_pImageBuffer;
	int					m_iImageBufferSize;

	NXBOOL				m_isPlay;

	unsigned int			m_uiTotalClipCount;
	unsigned int			m_uiCurrentClipIndex;

	static unsigned int		m_uiUniqueIDCounter;
	CNexLock			m_Lock;

	NXBOOL				m_bRequireTrackUpdate;

	NXBOOL				m_bH264Interlaced;

	int					m_iVideoCodecType;
	unsigned char*		m_pVideoDSI;
	int					m_iVideoDSISize;

	int					m_iAudioCodecType;
	unsigned char*		m_pAudioDSI;
	int					m_iAudioDSISize;
	NXBOOL				m_isStartIDRFrame;
	int					m_iContentOrientation;

	int					m_iAudioTrackCount;
	int					m_iVideoTrackCount;

	NXBOOL				m_isParseClip;	
	NXBOOL				m_isParseClipStop;	
	//yoon
	unsigned char*		m_pVideoTrackUUID;
	int					m_iVideoTrackUUIDSize;	

	int					m_iVideoDecodeEnd;

	int					m_iHDRType; //yoon
	NEXCODECUTIL_SEI_HDR_INFO m_stHDRInfo; //yoon
	
	unsigned int		m_uiAudioEditBoxTime;
	unsigned int		m_uiVideoEditBoxTime;

	NXBOOL				m_isMotionTracked;

	CNexDrawInfoVec*	m_pDrawInfos;
	int					m_iFreezeDuration;
	NXBOOL					m_bReverse;
	int				m_iIframePlay;
};

//---------------------------------------------------------------------------
class CClipItemVector :public CClipItemVectorBase
{
public:
	CClipItemVector()
	{
	}
	~CClipItemVector()
	{
	}
};

//---------------------------------------------------------------------------
class CClipInfo :public CNxRef< IClipInfo >
{
public:
	CClipInfo();
	virtual ~CClipInfo();

	int existVideo();
	int existAudio();
	int getWidth();
	int getHeight();
	int getDisplayWidth();
	int getDisplayHeight();

	int getVideoFPS();
	float getVideoFPSFloat();

	int getVideoH264Profile();
	int getVideoH264Level();
	virtual int getVideoOrientation();

	int getVideoBitRate();
	int getAudioBitRate();

	int getAudioSampleRate();
	int getAudioChannels();	

	unsigned int getClipAudioDuration();
	unsigned int getClipVideoDuration();
	const char* getThumbnailPath();

	int getSeekPointCount();

	int getSeekTableCount();
	int getSeekTableValue(int iIndex);

	int setExistVideo(NXBOOL isExist);
	int setExistAudio(NXBOOL isExist);
	int setAudioCodecType(int iAudioCodecType);
	int getAudioCodecType();	
	int setVideoCodecType(int iVideoCodecType);	
	int getVideoCodecType();

	int setWidth(int iWidth);
	int setHeight(int iHeight);
	int setDisplayWidth(int iDisplayWidth);
	int setDisplayHeight(int iDisplayHeight);
	int setVideoFPS(int iFPS);
	int setVideoFPSFloat(float fFPS);
	int setVideoH264Profile(int iProfile);
	int setVideoH264Level(int iLevel);
	int setVideoOrientation(int iOrientation);

	int setVideoBitRate(int iVideoBitRate);
	int setAudioBitRate(int iAudioBitRate);

	int getVideoH264Interlaced();
	int setVideoH264Interlaced(NXBOOL isInterlaced);

	int setAudioSampleRate(int iSampleRate);
	int setAudioChannels(int iChannels);	
	
	int setClipAudioDuration(unsigned int uiDuration);
	int setClipVideoDuration(unsigned int uiDuration);
	int setThumbnailPath(const char* pPath);
	int setSeekPointCount(int iSeekPointCount);
	int setSeekTable(int iSeekTableCount, int* pSeekTable);
	int addSeekTableItem(int iValue);

	int setVideoRenderType(int type);
	int getVideoRenderType();
	int setVideoUUID(const char* pUUID , int size );
	int setVideoHDRType(int type); //yoon
	int getVideoHDRType(); //yoon
	const char* getVideoUUID(int * size );
	int setEditBoxTime(unsigned int uiAudioEditBoxTime, unsigned int uiVideoEditBoxTime );
	int getEditBoxTIme(unsigned int* piAudioEditBoxTime, unsigned int* piVideoEditBoxTime);

	void printClipInfo();

private:
	NXBOOL			m_existVideo;
	NXBOOL			m_existAudio;
	NXBOOL			m_isH264Interlaced;
	unsigned int		m_iAudioCodecType;
	unsigned int		m_iVideoCodecType;
	int				m_iWidth;
	int				m_iHeight;
	int				m_iDisplayWidth;
	int				m_iDisplayHeight;
	int				m_iFPS;
	float			m_fFPS;
	int				m_iH264Profile;
	int				m_iH264Level;
	int				m_iRotateState;

	int				m_iVideoBitRate;
	int				m_iAudioBitRate;

	int				m_iAudioSampleRate;
	int				m_iAudioChannels;
	
	unsigned int		m_uiAudioDuration;
	unsigned int		m_uiVideoDuration;
	int				m_iSeekPointCount;
	char*			m_strThumbnailPath;
	int				m_VideoType;
	int				m_iUUIDSize;
	char *			m_strUUID;
	int					m_iHDRType; //yoon
	unsigned int		m_uiAudioEditBoxTime;
	unsigned int		m_uiVideoEditBoxTime;
	std::vector<int>	m_iClipSeekTabelVec;
};

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_CLIP__H_
