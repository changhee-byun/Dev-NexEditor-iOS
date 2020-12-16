/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Thumbnail.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2011/09/06	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_PREVIEWTHUMB_H__
#define __NEXVIDEOEDITOR_PREVIEWTHUMB_H__

#include "NexSAL_Internal.h"
#include <vector>
#include "NxVScaler.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NexTheme.h"
#include "NexThemeRenderer.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"

typedef struct _PREVIEWTHUMBINFO
{
	unsigned int m_uiTime;
	unsigned char* m_pBuffer;
}PREVIEWTHUMBINFO;

typedef std::vector<PREVIEWTHUMBINFO> PreviewThumbVec;
typedef PreviewThumbVec::iterator PreviewThumbVecItr;

class CPreviewInfo
{
public:
	CPreviewInfo();
	CPreviewInfo(unsigned int uiTime, unsigned char* pBuffer);

	unsigned int getTime();
	unsigned char* getBuffer();

	 bool operator() (int i,int j) {return (i<j);}

private:
	unsigned int m_uiTime;
	unsigned char* m_pBuffer;
};

typedef std::vector<CPreviewInfo> vecPreviewInfo;
typedef vecPreviewInfo::iterator vecPreviewInfoItr;

class CPreviewThumb : public vecPreviewInfo
{
public:
	CPreviewThumb( void );
	virtual ~CPreviewThumb( void );

	NXBOOL init(void* pThumbRender, void* pOutputSurface);
	NXBOOL deinit();
	void* getPreviewThumbSurfaceTexture();
	unsigned int getSize();
	NXBOOL addPreviewData(unsigned int uiTime, unsigned char* pData);
	unsigned char* getPreviewData(unsigned int uiTime);
	NXBOOL getStartEndTime(unsigned int* puiStartTime, unsigned int* puiEndTime);
	NXBOOL removePreviewData(unsigned int uiStartTime, unsigned int uiEndTime);
	void sortWithTime();

	NXBOOL setPreviewThumbInfo(int iWidth, int iHeight, int iPitch);
	NXBOOL addPreviewThumbInfo(void* pMediaBuffer, unsigned int uiCTS, NXBOOL bReverse);

	void setCodecWrap(CNexCodecWrap* pCodec);
	#ifdef __APPLE__
	void callCodecWrap_renderDecodedFrame(unsigned char* pBuff);
	void callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender);
	#endif

	#ifdef _ANDROID
	void callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender);
	#endif


private:
	int				m_iSrcWidth;
	int				m_iSrcHeight;
	int				m_iSrcPitch;

	NXT_HThemeRenderer	m_hThumbRenderer;	
	void*				m_pThumbDecSurface;
	void* 				m_pOutputSurface;
	CNexCodecWrap*				m_pCodecWrap;
};

#endif // __NEXVIDEOEDITOR_PREVIEWTHUMB_H__
