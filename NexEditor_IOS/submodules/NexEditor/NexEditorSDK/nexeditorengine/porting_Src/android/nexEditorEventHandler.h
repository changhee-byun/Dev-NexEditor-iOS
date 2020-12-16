/******************************************************************************
* File Name   :	nexEditorEventHandler.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/11/07	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_EVENTERHANDLER__H_
#define _NEXVIDEOEDITOR_EVENTERHANDLER__H_
//---------------------------------------------------------------------------

#include "NEXVIDEOEDITOR_Interface.h"

//---------------------------------------------------------------------------
class CNexVideoEditoerEventHandler : public CNxRef<INexVideoEditorEventHandler>
{
public:
	CNexVideoEditoerEventHandler();
	virtual ~CNexVideoEditoerEventHandler();
	
	virtual int notifyEvent(unsigned int uiEventType, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0, unsigned int uiParam3 = 0, unsigned int uiParam4 = 0);
	virtual int notifyError(unsigned int uiEventType, unsigned int uiResult, unsigned int uiParam1 = 0, unsigned int uiParam2 = 0);
	
	virtual int callbackCapture(int iWidth, int iHeight, int iSize, char* pBuffer);

	virtual void callbackGetThemeImage(char* pImageFilePath, int asyncmode, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData);
	virtual int callbackCheckImageWorkDone();
	virtual void callbackGetImageUsingFile(char* pImageFilePath, int iThumb, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData, int* pLoadedType);
	virtual void callbackGetImageUsingText(char* pText, unsigned int uiTextLen, int* pWidth, int* pHeight, int* pBitForPixel, unsigned char** ppImageData, int* pImageDataSize, void** ppUserData);
	virtual void callbackReleaseImage(void** ppUserData);
	virtual void callbackGetImageHeaderUsingFile(char* pImageFilePath, unsigned int uiFileLen, int* pWidth, int* pHeight, int* pBitForPixel);
	virtual void callbackGetThemeFile(char* pImageFilePath, unsigned int uiFileLen, int* pLength, char** ppFileData);	

	virtual void* callbackGetAudioTrack(int iSampleRate, int iChannels);
	virtual void callbackReleaseAudioTrack();

	virtual void* callbackGetAudioManager();
	virtual void callbackReleaseAudioManager();

	virtual int callbackCustomLayer(	int iParam1, int iParam2, int iParam3, int iParam4, int iParam5, 
									int iParam6, int iParam7, int iParam8, int iParam9, int iParam10, 
									int iParam11, int iParam12, int iParam13, int iParam14, int iParam15, int iParam16, int iParam17, int iParam18);

	virtual int getLUTWithID(int lut_resource_id, int export_flag);
	virtual int getVignetteTexID(int export_flag);
	virtual int getAssetResourceKey(const char * input_resourcpath, char *output_key);
	int setEventObject(jobject eventObj);

	virtual int callbackThumb(int iMode, int iTag, int iTime, int iWidth, int iHeight, int iCount, int iTotal, int iSize, char* pBuffer);	
	virtual int callbackHighLightIndex(int iCount, int* pBuffer);

private:
	//JavaVM*		m_vm;
	jobject		m_eventObj;
	jmethodID	m_callbackCapture;

	jmethodID	m_callbackGetThemeImage;
	jmethodID	m_callbackCheckImageWorkDone;
	jmethodID	m_callbackGetImageUsingFileID;
	jmethodID	m_callbackGetImageUsingTextID;
	jmethodID	m_callbackReleaseImageID;
	jmethodID	m_callbackGetThemeFile;

	jmethodID	m_callbackGetAudioTrackID;
	jmethodID	m_callbackReleaseAudioTrackID;
	
	jobject		m_objAudioManager;
	jmethodID	m_callbackGetAudioManagerID;
	
	jmethodID	m_callbackPrepareCustomLayerID;
	
	jmethodID	m_notifyEventID;
	jmethodID	m_notifyErrorID;

	jmethodID	m_callbackThumb;	
	jmethodID	m_callbackHighLightIndex;		
	jmethodID	m_getLUTWithID;
	jmethodID	m_getVignetteTexID;
	jmethodID   m_getAssetResourceKey;
};

#endif // _NEXVIDEOEDITOR_EVENTERHANDLER__H_
