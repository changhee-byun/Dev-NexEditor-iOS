/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_IRef.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/05/13	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_MSGINFO__H_
#define _NEXVIDEOEDITOR_MSGINFO__H_
//---------------------------------------------------------------------------

#include "nexIRef.h"
#include "nexLock.h"
#include "nexEvent.h"
#include "NEXVIDEOEDITOR_MsgDef.h"
#include "NEXVIDEOEDITOR_Types.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NEXVIDEOEDITOR_BaseRect.h"
#include <stdio.h>
#include <vector>
#include <map>

//---------------------------------------------------------------------------

#define MESSAGE_PROCESS_OK		0
#define MESSAGE_PROCESS_PASS	1

/**	\class CNxMsgInfo
	\brief CNxMsgInfo class.
*/
class CNxMsgInfo : public CNxRef<INxRefObj>
{
public:

	/**	\fn		CNxMsgInfo()
		\brief	메세지 기저 클래스 생성자
		\param	
		\return 
	*/
	CNxMsgInfo()
	{
		m_nMsgType = 0;
		m_nResult = 0;
	}

	CNxMsgInfo(int iMsgType)
	{
		m_nMsgType = iMsgType;
	}

	/**	\fn		~CNxMsgInfo()
		\brief	메세지 기저 클래스 소멸자
		\param	
		\return 
	*/
	virtual ~CNxMsgInfo()
	{}
	int m_nMsgType;
	int m_nResult;
};

class CNxMsgChangeState : public CNxMsgInfo
{
public:
    CNxMsgChangeState()
	{
		m_nMsgType = MESSAGE_STATE_CHANGE;
		m_iFlag = 0;
	}
    CNxMsgChangeState(PLAY_STATE currentState)
	{
		m_nMsgType = MESSAGE_STATE_CHANGE;
		m_currentState = currentState;
		m_iFlag = 0;		
	}

    CNxMsgChangeState(PLAY_STATE currentState, int iFlag)
	{
		m_nMsgType = MESSAGE_STATE_CHANGE;
		m_currentState = currentState;
		m_iFlag = iFlag;		
	}
    virtual ~CNxMsgChangeState()
    {}

	PLAY_STATE m_currentState;
	int m_iFlag;
};

class CNxMsgUpdateClipInfo : public CNxMsgInfo
{
public:
	CNxMsgUpdateClipInfo()
	{
		m_nMsgType = MESSAGE_UPDATE_CLIP;
		m_pUpdatedClip = NULL;
	}
	CNxMsgUpdateClipInfo(INxRefObj* pClip)
	{	
		m_nMsgType = MESSAGE_UPDATE_CLIP;
		
		SAFE_ADDREF(pClip);
		m_pUpdatedClip = pClip;
	}
	virtual ~CNxMsgUpdateClipInfo()
	{
		SAFE_ADDREF(m_pUpdatedClip);
	}

	INxRefObj*			m_pUpdatedClip;
};

class CNxMsgUpdateCurrentTimeStamp : public CNxMsgInfo
{
public:
    CNxMsgUpdateCurrentTimeStamp()
	{
		m_nMsgType			= MESSAGE_UPDATE_CURRENTIME;
		m_currentTimeStamp	= 0;
	}
    CNxMsgUpdateCurrentTimeStamp(unsigned int uiCurrentTime)
	{
		m_nMsgType			= MESSAGE_UPDATE_CURRENTIME;
		m_currentTimeStamp	= uiCurrentTime;
	}
    virtual ~CNxMsgUpdateCurrentTimeStamp()
    {}

	unsigned int m_currentTimeStamp;
};

class CNxMsgCloseProject : public CNxMsgInfo
{
public:
    CNxMsgCloseProject()
	{
		m_nMsgType			= MESSAGE_CLOSE_PROJECT;
	}
    virtual ~CNxMsgCloseProject()
    {}
};

class CNxMsgEncodeProject : public CNxMsgInfo
{
public:
    CNxMsgEncodeProject()
	{
		m_nMsgType			= MESSAGE_ENCODE_PROJECT;
		m_strPath = NULL;
		m_iFlag = 0;
	}
    CNxMsgEncodeProject(char* pstrSavePath, int iWidth, int iHeight, int iBitrate, long long llMaxFileSize, long lMaxFileDuration, int iFPS, int iSamplingRate, int iAudioBitrate, int iProfile, int iLevel, int iVideoCodecType, int iFlag)
	{
		m_nMsgType			= MESSAGE_ENCODE_PROJECT;
		m_strPath = (char*)nexSAL_MemAlloc(strlen(pstrSavePath)+1);

		strcpy(m_strPath, pstrSavePath);

		m_iWidth			= iWidth;
		m_iHeight			= iHeight;
		
		m_iBitRate			= iBitrate;
		m_llMaxFileSize		= llMaxFileSize;
		m_lMaxFileDuration	= lMaxFileDuration;
		m_iFPS				= iFPS == 0 ? EDITOR_DEFAULT_FRAME_RATE : iFPS;
		m_iSamplingRate		= iSamplingRate == 0 ? EDITOR_DEFAULT_SAMPLERATE : iSamplingRate;
		m_iAudioBitRate		= iAudioBitrate == 0 ? EDITOR_DEFAULT_AUDIO_BITRATE : iAudioBitrate;
		m_iProfile			= iProfile;
		m_iLevel			= iLevel;
		m_iVideoCodecType			= iVideoCodecType==0 ? EDITOR_DEFAULT_CODEC_TYPE : iVideoCodecType;
		m_iFlag				= iFlag;
	}
    virtual ~CNxMsgEncodeProject()
    {
		if(m_strPath)
		{
			nexSAL_MemFree(m_strPath);
			m_strPath = NULL;
		}
    }

	char* m_strPath;
	int m_iWidth;
	int m_iHeight;
	int m_iBitRate;
	long long m_llMaxFileSize;
	long m_lMaxFileDuration;
	int m_iFPS;
	int m_iSamplingRate;
	int m_iAudioBitRate;
	int m_iProfile;
	int m_iLevel;
	int m_iVideoCodecType;
	int m_iFlag;
};

class CNxMsgEncodeProjectJpeg : public CNxMsgInfo
{
public:
	CNxMsgEncodeProjectJpeg()
	{
		m_nMsgType			= MESSAGE_ENCODE_PROJECT_JPEG;
		m_strPath			= NULL;
		m_iWidth			= 0;
		m_iHeight			= 0;
		m_iQuality			= 0;
		m_iFlag				= 0;
		m_pNativeWindow		= NULL;
		m_pInfo				= NULL;
	}

	CNxMsgEncodeProjectJpeg(void* nativeWindow, char* pstrSavePath, int iWidth, int iHeight, int iQuality, int iFlag)
	{
		m_nMsgType			= MESSAGE_ENCODE_PROJECT_JPEG;

		if( pstrSavePath != NULL )
		{
			m_strPath = new char[strlen(pstrSavePath) + 1];
			strcpy(m_strPath, pstrSavePath);
		}

		m_pNativeWindow		= nativeWindow;
		m_iWidth			= iWidth;
		m_iHeight			= iHeight;
		m_iQuality			= iQuality;
		m_iFlag				= iFlag;
		m_pInfo				= NULL;
	}

	virtual ~CNxMsgEncodeProjectJpeg()
	{
		SAFE_DELETE(m_strPath);
		SAFE_RELEASE(m_pInfo);
	}

	void setDrawInfo(INxRefObj* pDrawInfo) {
		SAFE_RELEASE(m_pInfo);
		m_pInfo = pDrawInfo;
		SAFE_ADDREF(m_pInfo);
	}

	char* m_strPath;
	int m_iWidth;
	int m_iHeight;
	int m_iQuality;
	int m_iFlag;
	void* m_pNativeWindow;
	INxRefObj*	m_pInfo;
};

class CNxMsgAddClip : public CNxMsgInfo
{
public:
	CNxMsgAddClip()
	{
		m_nMsgType			= MESSAGE_ADD_AUDIO_CLIP;
		m_uiNextToClipID		= 0;
		m_uiStartTime		= 0;
		m_pClipPath			= NULL;
	}
	CNxMsgAddClip(int isVisual, unsigned int uiNextToClipID, unsigned int uiStartTime, char* pClipPath, int iNewClipID)
	{
		m_nMsgType			= MESSAGE_ADD_AUDIO_CLIP;
		if( isVisual )
			m_nMsgType		= MESSAGE_ADD_VISUAL_CLIP;
		m_uiNextToClipID		= uiNextToClipID;
		m_uiStartTime		= uiStartTime;
		m_iNewClipID			= iNewClipID;

		if( pClipPath )
		{
			m_pClipPath = new char[strlen(pClipPath)+1];
			if( m_pClipPath )
			{
				strcpy(m_pClipPath, pClipPath);
			}
		}
	}
	virtual ~CNxMsgAddClip()
	{
		if( m_pClipPath )
		{
			delete [] m_pClipPath;
			m_pClipPath = NULL;
		}

	}

	unsigned int	m_uiNextToClipID;
	unsigned int	m_uiStartTime;
	char*		m_pClipPath;
	int			m_iNewClipID;
};

class CNxMsgMoveClip : public CNxMsgInfo
{
public:
    CNxMsgMoveClip()
	{
		m_nMsgType			= MESSAGE_MOVE_CLIP;
	}
    CNxMsgMoveClip(unsigned int uiNextToID, unsigned int uiMoveTime, unsigned int uiMoveClipID, int isEventSend)
	{
		m_nMsgType			= MESSAGE_MOVE_CLIP;
		m_uiNextToID			= uiNextToID;
		m_uiMoveTime		= uiMoveTime;
		m_uiMoveClipID		= uiMoveClipID;
		m_isEventSend		= isEventSend;
	}

    virtual ~CNxMsgMoveClip()
    {}

	unsigned int	m_uiNextToID;
	unsigned int	m_uiMoveTime;
	unsigned int	m_uiMoveClipID;
	int			m_isEventSend;
};

class CNxMsgDeleteClip : public CNxMsgInfo
{
public:
    CNxMsgDeleteClip()
	{
		m_nMsgType			= MESSAGE_DELETE_CLIP;
	}
    CNxMsgDeleteClip(int iClipID)
	{
		m_nMsgType			= MESSAGE_DELETE_CLIP;
		m_iClipID			= iClipID;
	}
    virtual ~CNxMsgDeleteClip()
    {}

	int	m_iClipID;
};

class CNxMsgSetTime : public CNxMsgInfo
{
public:
	CNxMsgSetTime()
	{
		m_nMsgType			= MESSAGE_SET_TIME;
		m_pClipList			= NULL;
        m_pEventCmd = NULL;
	}
	
	CNxMsgSetTime(unsigned int uiTime, int iDisplay = 1, int iIDRFrame = 1, void* pThumbRender = NULL, void* pOutputSurface = NULL)
	{
		m_nMsgType			= MESSAGE_SET_TIME;
		m_uiTime			= uiTime;
		m_iDisplay			= iDisplay;
		m_iIDRFrame			= iIDRFrame;
		m_pClipList			= NULL;
		m_pThumbRender = pThumbRender;
		m_pOutputSurface = pOutputSurface;
        m_pEventCmd = NULL;
	}
	
	virtual ~CNxMsgSetTime()
	{
		SAFE_RELEASE(m_pClipList);
        if (m_pEventCmd != NULL) {
            delete m_pEventCmd;
            m_pEventCmd = NULL;
        }
	}

	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
        if (m_pEventCmd != NULL)
            return m_pEventCmd->WaitEvent(uiTimeMS);
        else
            return FALSE;
	}

	void setProcessDone()
	{
        if (m_pEventCmd != NULL)
            m_pEventCmd->Set();
	}

	void setClipList(INxRefObj* pClipList)
	{
		SAFE_RELEASE(m_pClipList);
		m_pClipList = pClipList;
		SAFE_ADDREF(m_pClipList);
	}	

    NXBOOL createNexEvent()
    {
        if (m_pEventCmd == NULL) {
            m_pEventCmd = new CNexEvent();
        }
        if (m_pEventCmd->isValid()) {
            return TRUE;
        } else {
            return FALSE;
        }
    }

    INxRefObj* getClipList()
	{
		SAFE_ADDREF(m_pClipList);
		return m_pClipList;
	}

	unsigned int	m_uiTime;
	int			m_iDisplay;
	int			m_iIDRFrame;
	INxRefObj*	m_pClipList;
    CNexEvent*  m_pEventCmd;
	void* m_pThumbRender;
	void* m_pOutputSurface;
};

class CNxMsgDSIInfo : public CNxMsgInfo
{
public:

    CNxMsgDSIInfo()
	{}

    virtual ~CNxMsgDSIInfo()
    {}
};

class CNxMsgVideoRenderInfo : public CNxMsgInfo
{
public:

    CNxMsgVideoRenderInfo()
	{
		m_nMsgType			= MESSAGE_VIDEO_RENDERING_INFO;
	}

    virtual ~CNxMsgVideoRenderInfo()
    {}
    int m_iLeft;
    int m_iTop;
    int m_iRight;
    int m_iBottom;
};

class CNxMsgAudioRenderInfo : public CNxMsgInfo
{
public:

    CNxMsgAudioRenderInfo()
	{
		m_nMsgType			= MESSAGE_AUDIO_RENDERING_INFO;
	}

    virtual ~CNxMsgAudioRenderInfo()
    {}

	// for JIRA 2059
	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}
	
	void setProcessDone()
	{
		m_eventCmd.Set();
	}	
	
	unsigned int	m_uiCurrentTimeStamp;
	unsigned int	m_uiTotalTime;
	unsigned int	m_uiSamplingRate;
	unsigned int	m_uiChannels;
	unsigned int	m_uiBitsForSample;
	unsigned int	m_uiSampleForChannel;
	int				m_iMuteAudio;
	int				m_iManualVolumeControl;
	int				m_iPreview;

	int				m_iProjectVolume;
	int				m_iFadeInTime;
	int				m_iFadeOutTime;
	// for JIRA 2059
	CNexEvent		m_eventCmd;	
};

class CNxMsgAddTrack : public CNxMsgInfo
{
public:

    CNxMsgAddTrack()
	{
		m_nMsgType			= MESSAGE_ADD_TRACK;
		m_pTrack			= NULL;
	}

	CNxMsgAddTrack(INxRefObj* pTrack)
	{
		m_nMsgType		= MESSAGE_ADD_TRACK;

		SAFE_ADDREF(pTrack);
		m_pTrack		= pTrack;
	}

	virtual ~CNxMsgAddTrack()
    {
		SAFE_RELEASE(m_pTrack)
	}
	INxRefObj*	m_pTrack;
};

class CNxMsgDeleteTrack : public CNxMsgInfo
{
public:
	CNxMsgDeleteTrack()
	{
		m_nMsgType			= MESSAGE_DELETE_TRACK;
		m_pTrack			= NULL;
		m_uiTrackID			= 0;
	}

	CNxMsgDeleteTrack(unsigned int uiTrackID)
	{
		m_nMsgType		= MESSAGE_DELETE_TRACK;
		m_pTrack		= NULL;
		m_uiTrackID		= uiTrackID;
	}	

	CNxMsgDeleteTrack(INxRefObj* pTrack)
	{
		m_nMsgType		= MESSAGE_DELETE_TRACK;
		SAFE_ADDREF(pTrack);
		m_pTrack		= pTrack;
		m_uiTrackID		= 0;
	}

	virtual ~CNxMsgDeleteTrack()
	{
		SAFE_RELEASE(m_pTrack)
	}

	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}
	
	void setProcessDone()
	{
		m_eventCmd.Set();
	}
	
	INxRefObj*			m_pTrack;
	unsigned int			m_uiTrackID;

	CNexEvent	m_eventCmd;
	
};

class CNxMsgVideoRenderPos : public CNxMsgInfo
{
public:

    CNxMsgVideoRenderPos()
	{
		m_nMsgType			= MESSAGE_VIDEO_RENDER_POSITION;
	}

    virtual ~CNxMsgVideoRenderPos()
    {}
    int m_iLeft;
    int m_iTop;
    int m_iRight;
    int m_iBottom;
};

class CNxMsgTranscodingInfo : public CNxMsgInfo
{
public:
	CNxMsgTranscodingInfo()
	{
		m_strSrcClipPath = NULL;
		m_strDstClipPath = NULL;

		m_iVideo				= 0;
		m_iAudio				= 0;
		m_iWidth				= 0;
		m_iHeight			= 0;
		m_iDisplayWidth		= 0;
		m_iDisplayHeight		= 0;
		m_iBitrate			= 0;
		m_llMaxFileSize		= 0;
		m_pFrameRenderer	= 0;
		m_pOutputSurface	= 0;
		m_pDecSurface		= 0;
		m_iUserTag			= 0;
		m_iFPS				= EDITOR_DEFAULT_FRAME_RATE;
		m_iFlag				= 0;
		m_iSpeedFactor		= 0;
		m_pUserData			= NULL;
	}

	~CNxMsgTranscodingInfo()
	{
		if(m_strSrcClipPath)
		{
			nexSAL_MemFree(m_strSrcClipPath);
			m_strSrcClipPath = NULL;
		}
		if(m_strDstClipPath)
		{
			nexSAL_MemFree(m_strDstClipPath);
			m_strDstClipPath = NULL;
		}

		clearUserData();
	}

	void setUserData(const char* pUserData)
	{
		clearUserData();
		if( pUserData )
		{
			m_pUserData = new char[strlen(pUserData) + 1];
			strcpy(m_pUserData, pUserData);
		}
	}

	void clearUserData()
	{
		if( m_pUserData != NULL )
		{
			delete [] m_pUserData;
			m_pUserData = NULL;
		}	
	}

	char*			m_strSrcClipPath;
	char*			m_strDstClipPath;
	int				m_iVideo;
	int				m_iAudio;
	int				m_iWidth;
	int				m_iHeight;
	int				m_iDisplayWidth;
	int				m_iDisplayHeight;
	int				m_iBitrate;
	long long		m_llMaxFileSize;
	void*			m_pFrameRenderer;
	void*			m_pOutputSurface;
	void*			m_pDecSurface;
	int				m_iUserTag;
	int				m_iFPS;
	int				m_iFlag;
	int				m_iSpeedFactor;
	char*			m_pUserData;
};


class CNxMsgSetEffectInfo : public CNxMsgInfo
{
public:

	CNxMsgSetEffectInfo()
	{
		m_nMsgType = MESSAGE_SET_EFFECT_INFO;
		m_uiEffectStartTime	= 0;
		m_uiEffectDuration		= 0;
		m_iEffectOffect		= 0;
		m_iEffectOverlap		= 0;
		m_iStartEffect			= 0;	
		m_hEffect			= NULL;
	}
	virtual ~CNxMsgSetEffectInfo()
	{}
	unsigned int	m_uiEffectStartTime;
	unsigned int	m_uiEffectDuration;
	int 			m_iEffectOffect;
	int 			m_iEffectOverlap;
	int 			m_iStartEffect;	
	void*		m_hEffect;
	
};

class CNxMsgSendEvent : public CNxMsgInfo
{
public:

	CNxMsgSendEvent()
	{
		m_nMsgType		= MESSAGE_SEND_EVENT;
		m_uiEventType	= 0;
		m_uiParam1		= 0;
		m_uiParam2		= 0;
		m_uiParam3		= 0;
		m_uiParam4		= 0;
	}
	virtual ~CNxMsgSendEvent()
	{}
	unsigned int	m_uiEventType;
	unsigned int	m_uiParam1;
	unsigned int	m_uiParam2;
	unsigned int	m_uiParam3;
	unsigned int	m_uiParam4;
};

class CNxMsgSetFileWriter : public CNxMsgInfo
{
public:
	CNxMsgSetFileWriter() : m_eventCmd(FALSE, FALSE)
	{
		m_nMsgType		= MESSAGE_SET_FILEWRITER;
		m_pWriter		= NULL;
	}	

	CNxMsgSetFileWriter(INxRefObj*	pWriter) : m_eventCmd(FALSE, FALSE)
	{
		m_nMsgType		= MESSAGE_SET_FILEWRITER;
		m_pWriter		= NULL;
		if( pWriter )
		{
			m_pWriter	= pWriter;
			SAFE_ADDREF(m_pWriter);
		}
	}
	virtual ~CNxMsgSetFileWriter()
	{
		SAFE_RELEASE(m_pWriter);
	}

	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}
	
	void setProcessDone()
	{
		m_eventCmd.Set();
	}
	
	INxRefObj*	m_pWriter;
	CNexEvent	m_eventCmd;
	
};

class CNxMsgBackgroundMusic: public CNxMsgInfo
{
public:
	CNxMsgBackgroundMusic(const char* pFilePath, int iNewClipID)
	{
		m_nMsgType		= MESSAGE_BACKGROUND_MUSIC;
		m_iNewClipID		= iNewClipID;

		if( pFilePath == NULL )
		{
			m_pFilePath = NULL;
		}
		else
		{
			m_pFilePath = new char[strlen(pFilePath) + 1];
			strcpy(m_pFilePath, pFilePath);
		}
	}	

	virtual ~CNxMsgBackgroundMusic()
	{
		if( m_pFilePath )
		{
			delete [] m_pFilePath;
			m_pFilePath = NULL;
		}
	}
	char* m_pFilePath;
	int		m_iNewClipID;
};

class CNxMsgBackgroundMusicVolume: public CNxMsgInfo
{
public:
	CNxMsgBackgroundMusicVolume(int iVolume, int iFadeInTime, int iFadeOutTime)
	{
		m_nMsgType		= MESSAGE_BACKGROUND_MUSIC_VOLUME;
		m_iVolume		= iVolume;
		m_iFadeInTime	= iFadeInTime;
		m_iFadeOutTime	= iFadeOutTime;
	}	

	virtual ~CNxMsgBackgroundMusicVolume()
	{
	}
	int		m_iVolume;
	int		m_iFadeInTime;
	int		m_iFadeOutTime;
};

class CNxMsgProjectVolumeFade: public CNxMsgInfo
{
public:
	CNxMsgProjectVolumeFade(int iFadeInTime, int iFadeOutTime)
	{
		m_nMsgType		= MESSAGE_SET_PROJECT_VOLUME_FADE;
		m_iFadeInTime	= iFadeInTime;
		m_iFadeOutTime	= iFadeOutTime;
	}	

	virtual ~CNxMsgProjectVolumeFade()
	{
	}
	int		m_iFadeInTime;
	int		m_iFadeOutTime;
};

class CNxMsgBackgroundGetInfo: public CNxMsgInfo
{
public:
	CNxMsgBackgroundGetInfo(	char* pFileName, 
									char* pThumbFilePath, 
									int iVideoThumbnail, 
									int iAudioPCMLevel, 
									void* pThumbRender, 
									void* pOutputSurface,
									int iUserTag)
	{
		m_nMsgType		= MESSAGE_GETCLIPINFO_BACKGROUND;

		m_iWidth			= 0;
		m_iHeight		= 0;
		m_iStartTime		= 0;
		m_iEndTime		= 0;
		m_iCount			= 0;		
		m_iFlag			= 0;
		m_pTimeTable	= NULL;
		m_iTimeTableCount	= 0;
		m_iUseCount		= 0;
		m_iSkipCount		= 0;

		m_strFileName =  new char[strlen(pFileName)+1];
		if( m_strFileName )
		{
			strcpy(m_strFileName, pFileName);
		}

		m_strThumbFileName =  new char[strlen(pThumbFilePath)+1];
		if( m_strThumbFileName )
		{
			strcpy(m_strThumbFileName, pThumbFilePath);
		}

		m_iVideoThumbnail	= iVideoThumbnail;
		m_iAudioPCMLevel		= iAudioPCMLevel;
		m_pThumbRenderer	= pThumbRender;
		m_pOutputSurface 		= pOutputSurface;
		m_iUserTag			= iUserTag;
	}	

	CNxMsgBackgroundGetInfo(	char* pFileName, 
									char* pThumbFilePath, 
									int iVideoThumbnail, 
									int iAudioPCMLevel, 
									int iStartTime,
									int iEndTime,
									int iUseCount,
									int iSkipCount,
									void* pThumbRender, 
									void* pOutputSurface,
									int iUserTag)
	{
		m_nMsgType		= MESSAGE_GETCLIPINFO_BACKGROUND;

		m_iWidth			= 0;
		m_iHeight		= 0;
		m_iStartTime		= iStartTime;
		m_iEndTime		= iEndTime;
		m_iCount			= 0;		
		m_iFlag			= 0;
		m_pTimeTable	= NULL;
		m_iTimeTableCount	= 0;
		m_iUseCount		= iUseCount;
		m_iSkipCount		= iSkipCount;

		m_strFileName =  new char[strlen(pFileName)+1];
		if( m_strFileName )
		{
			strcpy(m_strFileName, pFileName);
		}

		m_strThumbFileName =  new char[strlen(pThumbFilePath)+1];
		if( m_strThumbFileName )
		{
			strcpy(m_strThumbFileName, pThumbFilePath);
		}

		m_iVideoThumbnail	= iVideoThumbnail;
		m_iAudioPCMLevel		= iAudioPCMLevel;
		m_pThumbRenderer	= pThumbRender;
		m_pOutputSurface 		= pOutputSurface;
		m_iUserTag			= iUserTag;
	}	

	CNxMsgBackgroundGetInfo(	char* pFileName, 
									char* pThumbFilePath, 
									int iVideoThumbnail, 
									int iAudioPCMLevel, 
									void* pThumbRender, 
									void* pOutputSurface,
									int iWidth,
									int iHeight,
									int iStartTime,
									int iEndTime,
									int iCount,
									int iFlag,
									int iUserTag)
	{
		m_nMsgType		= MESSAGE_GETCLIPINFO_BACKGROUND;

		m_iWidth			= iWidth;
		m_iHeight		= iHeight;
		m_iStartTime		= iStartTime;
		m_iEndTime		= iEndTime;
		m_iCount			= iCount;		
		m_iFlag			= iFlag;
		m_pTimeTable	= NULL;
		m_iTimeTableCount	= 0;
		m_iUseCount		= 0;
		m_iSkipCount		= 0;

		m_strFileName =  new char[strlen(pFileName)+1];
		if( m_strFileName )
		{
			strcpy(m_strFileName, pFileName);
		}

		m_strThumbFileName =  new char[strlen(pThumbFilePath)+1];
		if( m_strThumbFileName )
		{
			strcpy(m_strThumbFileName, pThumbFilePath);
		}
		
		m_iVideoThumbnail	= iVideoThumbnail;
		m_iAudioPCMLevel		= iAudioPCMLevel;
		m_pThumbRenderer	= pThumbRender;
		m_pOutputSurface 	= pOutputSurface;
		m_iUserTag			= iUserTag;
	}	

	CNxMsgBackgroundGetInfo(		char* pFileName, 
									char* pThumbFilePath, 
									int iVideoThumbnail, 
									int iAudioPCMLevel, 
									int iWidth,
									int iHeight,
									int* pTimeTable,
									int iTimeTableCount,
									void* pThumbRender, 
									void* pOutputSurface,
									int iFlag,
									int iUserTag)
	{
		m_nMsgType			= MESSAGE_GETCLIPINFO_BACKGROUND;

		m_pTimeTable		= NULL;
		m_iTimeTableCount	= 0;
		m_iStartTime		= 0;
		m_iEndTime			= 0;		
		m_iUseCount		= 0;
		m_iSkipCount		= 0;

		m_strFileName =  new char[strlen(pFileName)+1];
		if( m_strFileName )
		{
			strcpy(m_strFileName, pFileName);
		}

		if( pThumbFilePath != NULL )
		{
			m_strThumbFileName =  new char[strlen(pThumbFilePath)+1];
			if( m_strThumbFileName )
			{
				strcpy(m_strThumbFileName, pThumbFilePath);
			}
		}

		m_iVideoThumbnail	= iVideoThumbnail;
		m_iAudioPCMLevel	= iAudioPCMLevel;
		m_iWidth			= iWidth;
		m_iHeight			= iHeight;
		m_iCount 			= 0;
		m_pThumbRenderer	= pThumbRender;
		m_pOutputSurface 	= pOutputSurface;
		m_iFlag				= iFlag;
		m_iUserTag			= iUserTag;

		m_pTimeTable = new int[iTimeTableCount];
		if( m_pTimeTable )
		{
			memcpy(m_pTimeTable, pTimeTable, iTimeTableCount*sizeof(int));
		}
		m_iTimeTableCount = iTimeTableCount;
			
	}		

	CNxMsgBackgroundGetInfo(	char* pFileName, 
								int iVideoThumbnail, 
								int iAudioPCMLevel, 
								int iStartTime,
								int iEndTIme,
								void* pThumbRender, 
								void* pOutputSurface,
								int iFlag,
								int iUserTag)
	{
		m_nMsgType		= MESSAGE_GETTHUMB_RAWDATA;

		m_pTimeTable	= NULL;
		m_iTimeTableCount	= 0;
		m_iUseCount		= 0;
		m_iSkipCount		= 0;

		m_strFileName =  new char[strlen(pFileName)+1];
		if( m_strFileName )
		{
			strcpy(m_strFileName, pFileName);
		}

		m_iVideoThumbnail	= iVideoThumbnail;
		m_iAudioPCMLevel	= iAudioPCMLevel;
		m_iStartTime		= iStartTime;
		m_iEndTime			= iEndTIme;		
		m_pThumbRenderer	= pThumbRender;
		m_pOutputSurface 	= pOutputSurface;
		m_iFlag				= iFlag;
		m_iUserTag			= iUserTag;

		m_iWidth = 0;
		m_iHeight = 0;
		m_iCount = 0;
		m_strThumbFileName = NULL;
	}	

	CNxMsgBackgroundGetInfo(	char* pFileName, 
								int iVideoThumbnail, 
								int iAudioPCMLevel, 
								int* pTimeTable,
								int iTimeTableCount,
								void* pThumbRender, 
								void* pOutputSurface,
								int iFlag,
								int iUserTag)
	{
		m_nMsgType			= MESSAGE_GETTHUMB_RAWDATA;

		m_pTimeTable		= NULL;
		m_iTimeTableCount	= 0;
		m_iStartTime		= 0;
		m_iEndTime			= 0;		
		m_iUseCount		= 0;
		m_iSkipCount		= 0;

		m_strFileName =  new char[strlen(pFileName)+1];
		if( m_strFileName )
		{
			strcpy(m_strFileName, pFileName);
		}

		m_iVideoThumbnail	= iVideoThumbnail;
		m_iAudioPCMLevel	= iAudioPCMLevel;
		m_pThumbRenderer	= pThumbRender;
		m_pOutputSurface 	= pOutputSurface;
		m_iFlag				= iFlag;
		m_iUserTag			= iUserTag;

		m_pTimeTable = new int[iTimeTableCount];
		if( m_pTimeTable )
		{
			memcpy(m_pTimeTable, pTimeTable, iTimeTableCount*sizeof(int));
		}
		m_iTimeTableCount = iTimeTableCount;
			
		m_iWidth = 0;
		m_iHeight = 0;
		m_iCount = 0;
		m_strThumbFileName = NULL;
	}	

	virtual ~CNxMsgBackgroundGetInfo()
	{
		if( m_strFileName )
			delete [] m_strFileName;

		if(m_strThumbFileName)
			delete [] m_strThumbFileName;

		if(m_pTimeTable)
			delete [] m_pTimeTable;
	}
	
	char*	m_strFileName;
	char* 	m_strThumbFileName;
	int		m_iVideoThumbnail;
	int		m_iAudioPCMLevel;
	void*	m_pThumbRenderer;
	void*	m_pOutputSurface;
	int		m_iWidth;
	int		m_iHeight;
	int		m_iStartTime;
	int		m_iEndTime;
	int		m_iCount;
	int		m_iFlag;
	int		m_iUserTag;
	int*		m_pTimeTable;
	int		m_iTimeTableCount;
	int		m_iUseCount;
	int		m_iSkipCount;
	
};

class CNxMsgBackgroundGetInfoStop: public CNxMsgInfo
{
public:
	CNxMsgBackgroundGetInfoStop(int iUserTag)
	{
		m_nMsgType		= MESSAGE_GETCLIPINFO_BACKGROUND_STOP;
		m_iUserTag = iUserTag;
	}	

	virtual ~CNxMsgBackgroundGetInfoStop()
	{
	}
	int		m_iUserTag;
};

class CNxMsgClearScreen: public CNxMsgInfo
{
public:
	CNxMsgClearScreen(int iTag)
	{
		m_nMsgType		= MESSAGE_CLEAR_SCREEN;
		m_iTag			= iTag;
	}	

	virtual ~CNxMsgClearScreen()
	{
	}
	int		m_iTag;
};

class CNxMsgFastOptionPreview: public CNxMsgInfo
{
public:
	CNxMsgFastOptionPreview(const char* pOption, int iDisplay = 1)
	{
		m_pOption		= NULL;
		m_nMsgType		= MESSAGE_FAST_OPTION_PREVIEW;
		m_iDisplay		= iDisplay;
		
		if( pOption )
		{
			m_pOption = new char[strlen(pOption) + 1];
			if( m_pOption )
			{
				strcpy(m_pOption, pOption);
			}
		}
	}	

	virtual ~CNxMsgFastOptionPreview()
	{
		if( m_pOption )
		{
			delete [] m_pOption;
			m_pOption = NULL;
		}
	}
	char*	m_pOption;
	int	m_iDisplay;
};

class CNxMsgPrepareSurface: public CNxMsgInfo
{
public:
	CNxMsgPrepareSurface(void* pSurface)
	{
		m_pSurface		= pSurface;
		m_nMsgType		= MESSAGE_PREPARE_SURFACE;
	}	

	virtual ~CNxMsgPrepareSurface()
	{
	}
	void* m_pSurface;
};

class CNxMsgPrepareClipLoading: public CNxMsgInfo
{
public:
	CNxMsgPrepareClipLoading(int iClipID)
	{
		m_iClipID		= iClipID;
		m_nMsgType		= MESSAGE_PREPARE_CLIP_LOADING;
	}	

	virtual ~CNxMsgPrepareClipLoading()
	{
	}

	int m_iClipID;
};

class CNxMsgSetProjectEffect: public CNxMsgInfo
{
public:
	CNxMsgSetProjectEffect(char* pEffect)
	{
		m_nMsgType		= MESSAGE_SET_PROJECT_EFFECT;
		m_pEffectID		= NULL;
		
		if( pEffect != NULL )
		{
			m_pEffectID = new char[strlen(pEffect)+1];
			if( m_pEffectID != NULL )
			{
				strcpy(m_pEffectID, pEffect);
			}
		}
	}	

	virtual ~CNxMsgSetProjectEffect()
	{
		if( m_pEffectID )
		{
			delete [] m_pEffectID;
			m_pEffectID = NULL;
		}
	}

	char* m_pEffectID;
};

class CNxMsgAddLayerItem: public CNxMsgInfo
{
public:
	CNxMsgAddLayerItem(INxRefObj* pItem)
	{
		m_nMsgType		= MESSAGE_SET_ADD_LAYER_ITEM;
		SAFE_ADDREF(pItem);
		m_pItem = pItem;
	}	

	virtual ~CNxMsgAddLayerItem()
	{
		SAFE_RELEASE(m_pItem);
	}

	INxRefObj* m_pItem;
};

class CNxMsgDeleteLayerItem: public CNxMsgInfo
{
public:
	CNxMsgDeleteLayerItem(int iLayerID)
	{
		m_nMsgType		= MESSAGE_SET_DELETE_LAYER_ITEM;
		m_iLayerID = iLayerID;
	}	

	virtual ~CNxMsgDeleteLayerItem()
	{
	}

	int m_iLayerID;
};

class CNxMsgAddLayer : public CNxMsgInfo
{
public:

    CNxMsgAddLayer()
	{
		m_nMsgType			= MESSAGE_ADD_LAYER_ITEM;
		m_pTrack			= NULL;
	}

	CNxMsgAddLayer(INxRefObj* pTrack)
	{
		m_nMsgType		= MESSAGE_ADD_LAYER_ITEM;

		SAFE_ADDREF(pTrack);
		m_pTrack		= pTrack;
	}

	virtual ~CNxMsgAddLayer()
    {
		SAFE_RELEASE(m_pTrack)
	}
	INxRefObj*	m_pTrack;
};

class CNxMsgDeleteLayer : public CNxMsgInfo
{
public:
	CNxMsgDeleteLayer()
	{
		m_nMsgType			= MESSAGE_DELETE_LAYER_ITEM;
		m_pTrack			= NULL;
		m_uiTrackID			= 0;
	}

	CNxMsgDeleteLayer(unsigned int uiTrackID)
	{
		m_nMsgType		= MESSAGE_DELETE_LAYER_ITEM;
		m_pTrack		= NULL;
		m_uiTrackID		= uiTrackID;
	}	

	CNxMsgDeleteLayer(INxRefObj* pTrack)
	{
		m_nMsgType		= MESSAGE_DELETE_LAYER_ITEM;
		SAFE_ADDREF(pTrack);
		m_pTrack		= pTrack;
		m_uiTrackID		= 0;
	}

	virtual ~CNxMsgDeleteLayer()
	{
		SAFE_RELEASE(m_pTrack)
	}

	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}
	
	void setProcessDone()
	{
		m_eventCmd.Set();
	}
	
	INxRefObj*			m_pTrack;
	unsigned int			m_uiTrackID;

	CNexEvent	m_eventCmd;
	
};

class CNxMsgLoadThemeAndEffect: public CNxMsgInfo
{
public:
	CNxMsgLoadThemeAndEffect( const char* pResurce, int iFlag)
	{
		m_nMsgType		= MESSAGE_LOAD_THEMEANDEFFECT;

		m_pResource =  new char[strlen(pResurce)+1];
		if( m_pResource )
		{
			strcpy(m_pResource, pResurce);
		}
		m_iFlag			= iFlag;
	}	

	virtual ~CNxMsgLoadThemeAndEffect()
	{
		if( m_pResource )
			delete [] m_pResource;
	}
	
	char*	m_pResource;
	int		m_iFlag;
};

class CNxMsgClearRenderItems: public CNxMsgInfo
{
public:
	CNxMsgClearRenderItems(int iFlag)
	{
		m_nMsgType		= MESSAGE_CLEAR_RENDERITEMS;
		m_iFlag			= iFlag;
	}	

	virtual ~CNxMsgClearRenderItems()
	{
	}
	
	int		m_iFlag;
};

class CNxMsgLoadRenderItem: public CNxMsgInfo
{
public:
	CNxMsgLoadRenderItem(const char* pID, const char* pResurce, int iFlag)
	{
		m_nMsgType		= MESSAGE_LOAD_RENDERITEM;

		m_pID = new char[strlen(pID) + 1];
		if(m_pID)
		{
			strcpy(m_pID, pID);
		}

		m_pResource =  new char[strlen(pResurce)+1];
		if( m_pResource )
		{
			strcpy(m_pResource, pResurce);
		}

		m_iFlag			= iFlag;
	}	

	virtual ~CNxMsgLoadRenderItem()
	{
		if( m_pResource )
			delete [] m_pResource;

		if( m_pID )
			delete [] m_pID;
	}
	
	char*	m_pID;
	char*	m_pResource;
	int		m_iFlag;
};

class CNxMsgHighLightIndex: public CNxMsgInfo
{
public:
	CNxMsgHighLightIndex()
	{
		m_nMsgType		= MESSAGE_MAKE_HIGHLIGHT_START;
		m_strSrcClipPath = NULL;
		m_strDstClipPath = NULL;

		m_iVideo				= 0;
		m_iAudio				= 0;
		m_iWidth				= 0;
		m_iHeight			= 0;
		m_iBitrate			= 0;
		m_llMaxFileSize		= 0;
		m_pFrameRenderer	= 0;
		m_pOutputSurface		= 0;
		m_pDecSurface		= 0;
		m_iUserTag			= 0;
		m_pUserData			= NULL;
		m_iIndexMode		= 0;
		m_iRequestInterval 	= 0;
		m_iRequestCount 		= 0;
		m_iOutputMode 		= 0;
		m_iDecodeMode		= 0;
	}

	~CNxMsgHighLightIndex()
	{
		if(m_strSrcClipPath)
		{
			nexSAL_MemFree(m_strSrcClipPath);
			m_strSrcClipPath = NULL;
		}
		if(m_strDstClipPath)
		{
			nexSAL_MemFree(m_strDstClipPath);
			m_strDstClipPath = NULL;
		}
	
		clearUserData();
	}

	void setUserData(const char* pUserData)
	{
		clearUserData();
		if( pUserData )
		{
			m_pUserData = new char[strlen(pUserData) + 1];
			strcpy(m_pUserData, pUserData);
		}
	}

	void clearUserData()
	{
		if( m_pUserData != NULL )
		{
			delete [] m_pUserData;
			m_pUserData = NULL;
		}	
	}

	char* m_strSrcClipPath;
	char* m_strDstClipPath;
	int		m_iVideo;
	int		m_iAudio;
	int 		m_iWidth;
	int		m_iHeight;
	int 		m_iBitrate;
	long long	m_llMaxFileSize;
	void*	m_pFrameRenderer;
	void*	m_pOutputSurface;
	void*	m_pDecSurface;
	int 		m_iUserTag;
	char*	m_pUserData;
	int		m_iIndexMode;
	int		m_iRequestInterval;
	int		m_iRequestCount;
	int		m_iOutputMode;
	int		m_iDecodeMode;
};

class CNxMsgCheckDirectExport : public CNxMsgInfo
{
public:
	CNxMsgCheckDirectExport()
	{
		m_nMsgType			= MESSAGE_MAKE_CHECK_DIRECT_EXPORT;
		m_iFlag				= 0;
	}

	virtual ~CNxMsgCheckDirectExport()
	{
	}

	int m_iFlag;
};


class CNxMsgDirectExport : public CNxMsgInfo
{
public:
	CNxMsgDirectExport()
	{
		m_nMsgType			= MESSAGE_MAKE_DIRECT_EXPORT_START;
		m_strPath = NULL;

		m_llMaxFileSize		= 0;
		m_lMaxFileDuration	= 0;
		m_pUserData = NULL;
		m_iFlag				= 0;
	}
	CNxMsgDirectExport(const char* pstrSavePath, long long llMaxFileSize, long lMaxFileDuration, const char* pUserData, int iFlag)
	{
		m_nMsgType			= MESSAGE_MAKE_DIRECT_EXPORT_START;
		m_strPath = (char*)nexSAL_MemAlloc(strlen(pstrSavePath)+1);
		
		strcpy(m_strPath, pstrSavePath);

		m_llMaxFileSize		= llMaxFileSize;
		m_lMaxFileDuration	= lMaxFileDuration;

		m_pUserData = NULL;
		if( pUserData != NULL )
		{
			m_pUserData = new char[strlen(pUserData)+1];
			if( m_pUserData != NULL )
			{
				strcpy(m_pUserData, pUserData);
			}
		}
		m_iFlag = iFlag;
	}
	virtual ~CNxMsgDirectExport()
	{
		if( m_pUserData != NULL )
		{
			delete [] m_pUserData;
			m_pUserData = NULL;
		}
		if(m_strPath)
		{
			nexSAL_MemFree(m_strPath);
			m_strPath = NULL;
		}
	}

	char* m_strPath;
	long long m_llMaxFileSize;
	long m_lMaxFileDuration;

	char* m_pUserData;
	int m_iFlag;
};

class CNxMsgAsyncUpdateCliplist : public CNxMsgInfo
{
public:
	CNxMsgAsyncUpdateCliplist()
	{
		m_nMsgType			= MESSAGE_ASYNC_UPDATE_CLIPLIST;
		m_pClipList			= NULL;
		m_iOption			= 0;
	}

	CNxMsgAsyncUpdateCliplist(INxRefObj* pClipList, int iOption)
	{
		m_nMsgType			= MESSAGE_ASYNC_UPDATE_CLIPLIST;

		m_pClipList = pClipList;
		SAFE_ADDREF(m_pClipList);
		m_iOption			= iOption;
	}

	virtual ~CNxMsgAsyncUpdateCliplist()
	{
		SAFE_RELEASE(m_pClipList);
	}

	INxRefObj*			m_pClipList;
	int					m_iOption;
};

class CNxMsgAsyncUpdateEffectlist : public CNxMsgInfo
{
public:
	CNxMsgAsyncUpdateEffectlist()
	{
		m_nMsgType			= MESSAGE_ASYNC_UPDATE_EFFECTLIST;
		m_pClipList			= NULL;
		m_iOption			= 0;
	}

	CNxMsgAsyncUpdateEffectlist(INxRefObj* pClipList, int iOption)
	{
		m_nMsgType			= MESSAGE_ASYNC_UPDATE_EFFECTLIST;

		m_pClipList = pClipList;
		SAFE_ADDREF(m_pClipList);
		m_iOption			= iOption;
	}

	virtual ~CNxMsgAsyncUpdateEffectlist()
	{
		SAFE_RELEASE(m_pClipList);
	}

	INxRefObj*			m_pClipList;
	int					m_iOption;
};

class CNxMsgFastPreview : public CNxMsgInfo
{
public:
	CNxMsgFastPreview()
	{
		m_uiStartTime	= 0;
		m_uiEndTime		= 0;
		m_uiTime		= 0;
        
        m_iWidth        = 0;
        m_iHeight       = 0;
        m_iMaxCount     = 0;
	}

	unsigned int 	m_uiStartTime;
	unsigned int 	m_uiEndTime;
	unsigned int 	m_uiTime;
    
    int             m_iWidth;
    int             m_iHeight;
    int             m_iMaxCount;
};

class CNxMsgFastPreviewTime : public CNxMsgInfo
{
public:
	CNxMsgFastPreviewTime()
	{
		m_nMsgType		= MESSAGE_FAST_PREVIEW_ADD_FRAME;
		m_uiStartTime	= 0;
		m_uiEndTime		= 0;
		m_uiTime		= 0;
	}

	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}

	void setProcessDone()
	{
		m_eventCmd.Set();
	}

	unsigned int 	m_uiStartTime;
	unsigned int 	m_uiEndTime;
	unsigned int 	m_uiTime;

	CNexEvent	m_eventCmd;
};

class CNxMsgWaitCmd : public CNxMsgInfo
{
public:
	CNxMsgWaitCmd()
	{
		m_nMsgType			= MESSAGE_WAIT_COMMAND;
	}
	virtual ~CNxMsgWaitCmd()
	{}

	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}	

	void setProcessDone()
	{
		m_eventCmd.Set();
	}		
	CNexEvent	m_eventCmd;	
};

class CNxMsgReverseInfo: public CNxMsgInfo
{
public:
	CNxMsgReverseInfo()
	{
		m_nMsgType		= MESSAGE_MAKE_REVERSE_START;
		m_strSrcClipPath = NULL;
		m_strDstClipPath = NULL;
		m_strTempClipPath = NULL;
		
		m_iVideo				= 0;
		m_iAudio				= 0;
		m_iWidth				= 0;
		m_iHeight			= 0;
		m_iBitrate			= 0;
		m_llMaxFileSize		= 0;
		m_pFrameRenderer	= 0;
		m_pOutputSurface		= 0;
		m_pDecSurface		= 0;
		m_iUserTag			= 0;
		m_pUserData			= NULL;
		m_iFlag				= 0;
		m_uStartTime			= 0;
		m_uEndTime			= 0;
		m_iDecodeMode		= 0;
		m_uFrameMemoryCount = 0;
	}

	~CNxMsgReverseInfo()
	{
		if(m_strSrcClipPath)
		{
			nexSAL_MemFree(m_strSrcClipPath);
			m_strSrcClipPath = NULL;
		}
		if(m_strDstClipPath)
		{
			nexSAL_MemFree(m_strDstClipPath);
			m_strDstClipPath = NULL;
		}
		if(m_strTempClipPath)
		{
			nexSAL_MemFree(m_strTempClipPath);
			m_strTempClipPath = NULL;
		}
	
		clearUserData();
	}

	void setUserData(const char* pUserData)
	{
		clearUserData();
		if( pUserData )
		{
			m_pUserData = new char[strlen(pUserData) + 1];
			strcpy(m_pUserData, pUserData);
		}
	}

	void clearUserData()
	{
		if( m_pUserData != NULL )
		{
			delete [] m_pUserData;
			m_pUserData = NULL;
		}	
	}

	char* m_strSrcClipPath;
	char* m_strDstClipPath;
	char* m_strTempClipPath;
	
	int		m_iVideo;
	int		m_iAudio;
	int 		m_iWidth;
	int		m_iHeight;
	int 		m_iBitrate;
	long long	m_llMaxFileSize;
	void*	m_pFrameRenderer;
	void*	m_pOutputSurface;
	void*	m_pDecSurface;
	int 		m_iUserTag;
	int m_iFlag;	
	char*	m_pUserData;

	unsigned int	m_uStartTime;
	unsigned int	m_uEndTime;
	int		m_iDecodeMode;
	unsigned int	m_uFrameMemoryCount;
};

class CNxMsgSetVolumeWhilePlay : public CNxMsgInfo
{
public:
	CNxMsgSetVolumeWhilePlay()
	{
		m_nMsgType			= MESSAGE_SET_VOLUME_WHILE_PLAYING;
	}
	virtual ~CNxMsgSetVolumeWhilePlay()
	{}

	int m_iMasterVolume;
	int m_iSlaveVolume;
};

class CNxMsgReleaseRendererCache : public CNxMsgInfo
{
public:
	CNxMsgReleaseRendererCache()
	{
		m_nMsgType			= MESSAGE_RELEASE_RENDERER_CACHE;
		m_uiIDs.clear();
	}
	virtual ~CNxMsgReleaseRendererCache()
	{
		m_uiIDs.clear();
	}

	void addID(unsigned int uiID)
	{
		m_uiIDs.insert(m_uiIDs.end(), uiID);
	}

	int getIDCounts()
	{
		return (int)m_uiIDs.size();
	}

	unsigned int getID(int iIndex)
	{
		return m_uiIDs[iIndex];
	}

	std::vector<unsigned int> m_uiIDs;

};

class CNxMsgProjectVolume: public CNxMsgInfo
{
public:
	CNxMsgProjectVolume(int iProjectVolume)
	{
		m_nMsgType		= MESSAGE_SET_PROJECT_VOLUME;
		m_iProjectVolume	= iProjectVolume;
	}	

	virtual ~CNxMsgProjectVolume()
	{}

	int		m_iProjectVolume;
};

class CNxMsgSetManualVolumeControl : public CNxMsgInfo
{
public:
	CNxMsgSetManualVolumeControl(NXBOOL bManualVolumeControl)
	{
		m_nMsgType					= MESSAGE_SET_MANUAL_VOLUME_CONTROL;
		m_bManualVolumeControl	= bManualVolumeControl;
	}
	virtual ~CNxMsgSetManualVolumeControl()
	{}

	NXBOOL m_bManualVolumeControl;
};


class CNxMsgSetDrawInfoList : public CNxMsgInfo
{
public:
	CNxMsgSetDrawInfoList(INxRefObj* master, INxRefObj* sub)
	{
		m_nMsgType					= MESSAGE_SET_DRAWINFO_LIST;
		masterList = master;
		SAFE_ADDREF(masterList);
		subList = sub;
		SAFE_ADDREF(subList);
	}
	virtual ~CNxMsgSetDrawInfoList()
	{
		SAFE_RELEASE(masterList);
		SAFE_RELEASE(subList)
	}

	INxRefObj* masterList;
	INxRefObj* subList;
};

class CNxMsgSetDrawInfo : public CNxMsgInfo
{
public:
	CNxMsgSetDrawInfo(INxRefObj* drawinfo)
	{
		m_nMsgType					= MESSAGE_SET_DRAWINFO;
		drawInfo = drawinfo;
		SAFE_ADDREF(drawInfo);
	}
	virtual ~CNxMsgSetDrawInfo()
	{
		SAFE_RELEASE(drawInfo);
	}

	INxRefObj* drawInfo;
};

class CNxMsgSetTaskSleep : public CNxMsgInfo
{
public:
	CNxMsgSetTaskSleep(int sleep)
	{
		m_nMsgType					= MESSAGE_TASK_SLEEP;
		m_Sleep	= sleep;
	}
	virtual ~CNxMsgSetTaskSleep()
	{}

	int m_Sleep;
};

class CNxMsgUpdateRenderInfo : public CNxMsgInfo
{

public:
	CNxMsgUpdateRenderInfo(unsigned int clipid, unsigned int face_detected, RECT& start, RECT& end, RECT& face){

		m_nMsgType					= MESSAGE_UPDATE_RENDERINFO;

		m_uiClipID = clipid;
		m_uiFaceDetected = face_detected;
		m_StartRect = start;
		m_EndRect = end;
		m_FaceRect = face;
	}

	virtual ~CNxMsgUpdateRenderInfo(){

	}

	unsigned int m_uiClipID;
	unsigned int m_uiFaceDetected;
	RECT m_StartRect;
	RECT m_EndRect;
	RECT m_FaceRect;
};

class CNxMsgResetFaceDetectInfo : public CNxMsgInfo
{

public:
	CNxMsgResetFaceDetectInfo(unsigned int clipid){

		m_nMsgType					= MESSAGE_RESET_FACEDETECTINFO;
		m_uiClipID = clipid;		
	}

	virtual ~CNxMsgResetFaceDetectInfo(){

	}

	unsigned int m_uiClipID;
};

class CNxMsgSetGIFMode : public CNxMsgInfo
{
public:
	CNxMsgSetGIFMode()
	{
		m_nMsgType			= MESSAGE_SET_GIF_MODE;
		m_mode = 0;
	}

	virtual ~CNxMsgSetGIFMode()
	{
	}

	NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}

	void setProcessDone()
	{
		m_eventCmd.Set();
	}

	CNexEvent	m_eventCmd;
	int			m_mode;
};

class CNxMsgUpdateClip4Export : public CNxMsgInfo
{
public:
	CNxMsgUpdateClip4Export(unsigned int uCurrentTime)
	{
		m_nMsgType			= MESSAGE_UPDATE_CLIP_FOR_EXPORT;
		m_uCurrentTime = uCurrentTime;
	}

	int			m_uCurrentTime;
};

class CNxMsgGetBaseRendererInfo : public CNxMsgInfo
{
public:
	CNxMsgGetBaseRendererInfo()
	{
		m_nMsgType			= MESSAGE_GET_BASERENDERER_INFO;
        m_pRenderer = NULL;
        m_Width = 0;
        m_Height = 0;
	}

	virtual ~CNxMsgGetBaseRendererInfo(){

	}

    NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}

	void setProcessDone()
	{
		m_eventCmd.Set();
	}

	CNexEvent	m_eventCmd;
    void*       m_pRenderer;
    int         m_Width;
    int         m_Height;
};

class CNxMsgSetThemeRender : public CNxMsgInfo
{
public:
	CNxMsgSetThemeRender(void* pRenderer, int mode)
	{
		m_nMsgType			= MESSAGE_SET_THEMERENDER;
        m_pRenderer = pRenderer;
        m_Mode = mode;
	}

	virtual ~CNxMsgSetThemeRender(){
		
	}

    void*       m_pRenderer;
    int         m_Mode;
};

class CNxMsgSetMapper : public CNxMsgInfo
{
public:
	CNxMsgSetMapper(std::map<int,int>* pmapper)
	{
		m_nMsgType			= MESSAGE_SET_MAPPER;
        m_pMapper = pmapper;
	}

	virtual ~CNxMsgSetMapper(){

        SAFE_DELETE(m_pMapper);		
	}

    std::map<int,int>*       m_pMapper;
};

class CNxMsgPrepareSurfaceRenderer : public CNxMsgInfo
{
public:
	CNxMsgPrepareSurfaceRenderer(void* pSurface, float fScaleFactor)
	{
		m_nMsgType			= MESSAGE_PREPARESURFACE;
        m_pSurface = pSurface;
        m_fScaleFactor = fScaleFactor;
        m_Result = FALSE;
	}

	virtual ~CNxMsgPrepareSurfaceRenderer(){
		
	}

    NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}

	void setProcessDone()
	{
		m_eventCmd.Set();
	}

	CNexEvent	m_eventCmd;
    void*       m_pSurface;
    float       m_fScaleFactor;
    NXBOOL      m_Result;
};

class CNxMsgGetSurfaceTexture : public CNxMsgInfo
{
public:
	CNxMsgGetSurfaceTexture()
	{
		m_nMsgType			= MESSAGE_GETSURFACETEXTURE;
        m_pSurface = NULL;
	}

	virtual ~CNxMsgGetSurfaceTexture(){
		
	}

    NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}

	void setProcessDone()
	{
		m_eventCmd.Set();
	}

	CNexEvent	m_eventCmd;
    void*       m_pSurface;
};

class CNxMsgReleaseSurfaceTexture : public CNxMsgInfo
{
public:
	CNxMsgReleaseSurfaceTexture(void* pSurfaceTexture)
	{
		m_nMsgType			= MESSAGE_RELEASESURFACETEXTURE;
        m_pSurfaceTexture = pSurfaceTexture;
	}

	virtual ~CNxMsgReleaseSurfaceTexture(){
		
	}

    void*       m_pSurfaceTexture;
};



class CNxMsgDestroySurfaceTexture : public CNxMsgInfo
{
public:
	CNxMsgDestroySurfaceTexture(void* pRender, void* pSurfaceTexture)
	{
		m_nMsgType			= MESSAGE_DESTROYSURFACETEXTURE;
        m_pSurfaceTexture = pSurfaceTexture;
        m_pRender = pRender;
	}

	virtual ~CNxMsgDestroySurfaceTexture(){
		
	}

    void*       m_pRender;
    void*       m_pSurfaceTexture;
};

class CNxMsgSetSaparateEffect : public CNxMsgInfo
{
public:
	CNxMsgSetSaparateEffect(NXBOOL flag)
	{
		m_nMsgType			= MESSAGE_SETSEPARATEEFFECT;
        m_Flag = flag;
	}

	virtual ~CNxMsgSetSaparateEffect(){
		
	}

    NXBOOL m_Flag;
};

class CNxMsgSetVisualCountAtTime : public CNxMsgInfo
{
public:
	CNxMsgSetVisualCountAtTime(int iVisualClip, int iVisualLayer)
	{
		m_nMsgType			= MESSAGE_SETVISUALCOUNTATTIME;
        m_iVisualClip = iVisualClip;
    	m_iVisualLayer = iVisualLayer;
	}

	virtual ~CNxMsgSetVisualCountAtTime(){
		
	}

    int m_iVisualClip;
    int m_iVisualLayer;
};

class CNxMsgSetGLOperationAllowed : public CNxMsgInfo
{
public:
	CNxMsgSetGLOperationAllowed(NXBOOL flag)
	{
		m_nMsgType			= MESSAGE_SETGLOPERATIONALLOWED;
        m_Flag = flag;
	}

	virtual ~CNxMsgSetGLOperationAllowed(){
		
	}

    NXBOOL m_Flag;
};

class CNxMsgCheckIsLostNativeWindow : public CNxMsgInfo
{
public:
	CNxMsgCheckIsLostNativeWindow()
	{
		m_nMsgType			= MESSAGE_CHECKISLOSTNATIVEWINDOW;
        m_Result = FALSE;
	}

	virtual ~CNxMsgCheckIsLostNativeWindow(){
		
	}

    NXBOOL waitProcessDone(unsigned int uiTimeMS)
	{
		return m_eventCmd.WaitEvent(uiTimeMS);
	}

	void setProcessDone()
	{
		m_eventCmd.Set();
	}

	CNexEvent	m_eventCmd;
    NXBOOL      m_Result;
};

//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_MSGINFO__H_



