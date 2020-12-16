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

#ifndef __NEXVIDEOEDITOR_THUMBNAIL_H__
#define __NEXVIDEOEDITOR_THUMBNAIL_H__

#include "NexSAL_Internal.h"
#include <vector>
#include "NxVScaler.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NexTheme.h"
#include "NexThemeRenderer.h"
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"

// #define YUV_TEST_DUMP

typedef enum _THUMBNAIL_MODE
{
	THUMBNAIL_MODE_NONE = 0,
	THUMBNAIL_MODE_VIDEO,	
	THUMBNAIL_MODE_AUDIO	
}THUMBNAIL_MODE;


typedef struct _THUMBNAILINFO
{
	unsigned int m_uiTime;
	unsigned int m_iFilePos;
}THUMBNAILINFO;

typedef struct _THUMBNAIL
{
	int m_iDuration;
	int m_isVideo;
	int m_isAudio;
	int m_iVideoWidth;
	int m_iVideoHeight;
	int m_iThumbnailWidth;
	int m_iThumbnailHeight;
	int m_iSeekTablePos;
	int m_iSeekTableCount;
	int m_iPCMLevelPos;
	int m_iPCMLevelCount;
	int m_iLargeThumbWidth;
	int m_iLargeThumbHeight;
	int m_iLargeThumbPos;
	int m_iLargeThumbSize;
	int m_iThumbnailInfoPos;
	int m_iThumbnailInfoCount;
}THUMBNAIL;

typedef std::vector<THUMBNAILINFO> ThumbnailVec;

class CThumbnail
{
public:
	CThumbnail( void );
	virtual ~CThumbnail( void );

	NXBOOL existThumbnailFile(const char* pThumbnailPath);
	
	NXBOOL init(NXBOOL bRawData, THUMBNAIL_MODE eMode, void* pThumbRender, void* pOutputSurface, int* pTimeTable, int iTimeTableCount, int iFlag);
	NXBOOL init(const char* pThumbnailPath, int iDuration, int isVideo, int isAudio);
	NXBOOL init(const char* pThumbnailPath, THUMBNAIL_MODE eMode, void* pThumbRender, void* pOutputSurface, int iDstWidth = 0, int iDstHeight = 0, int iMaxCount = 0, int iMaxFlag = 0, int iTag = 0);
	NXBOOL deinit();
	void* getThumbSurfaceTexture();
	NXBOOL setThumbnailInfo(int iWidth, int iHeight, int iPitch, NXBOOL bSoftwareCodec);

//	NXBOOL addThumbInfo(unsigned int uiCTS, unsigned char* pNV12Buffer);
//	NXBOOL addThumbInfo(unsigned int uiCTS, unsigned char* pY, unsigned char* pU, unsigned char* pV);
	NXBOOL addThumbInfo(void* pMediaBuffer, unsigned int uiCTS);
	NXBOOL setSeekTableInfo(int iSeekTableCount, unsigned int* uiSeekTable);
	NXBOOL setPCMTableInfo(int iPCMTableCount, unsigned char* uiPCMTable);
//	NXBOOL setLargeThumbnail(unsigned char* pY, unsigned char* pU, unsigned char* pV);

	unsigned int getThumbnailcount();
	int getMaxThumbnailCount();
	int getThumbnailFlag();

	void getStartEndTime(unsigned int *puStartTime, unsigned int *puEndTime);
	void setStartEndTime(unsigned int uStartTime, unsigned int uEndTime);
	NXBOOL getRequestRawData();
	void setAudioPCMSize(unsigned int uSize);
	NXBOOL addAudioPCM(unsigned int uSize, unsigned char* pData);

	int* getTimeTable(int* piTimeTableCount);

	int getThumbTimeTableCount();
	int getThumbTimeTable(int iIndex);
	NXBOOL setThumbTimeTable(int iSize, int* pTable);

	NXBOOL needSeekNextTimeTableItem(unsigned int uiDecOutTime, unsigned int uiReaderTime, int iNextTime);

	void setCodecWrap(CNexCodecWrap* pCodec);
	#ifdef __APPLE__
	void callCodecWrap_renderDecodedFrame(unsigned char* pBuff);
	void callCodecWrap_releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender);
	#endif

	#ifdef _ANDROID
	void callCodecWrapPostCallback(unsigned char* pBuff, NXBOOL bRender);
	#endif


private:

	THUMBNAIL_MODE	m_eThumbMode;
	int				m_iDuration;
	int				m_isVideo;
	int				m_isAudio;
	
	int				m_iMaxThumbnailCount;
	int				m_iThumbnailFlag;
	int				m_iThumbnailTag;
	
	int				m_iSrcWidth;
	int				m_iSrcHeight;
	int				m_iSrcPitch;

	int				m_iHexWidth;
	int				m_iHexHeight;
	int				m_iHexPitch;
	
	int				m_iDstWidth;
	int				m_iDstHeight;
	int				m_iDstPitch;
	int				m_iBitsForPixel;
	int				m_iBufferSize;

	NXBOOL			m_bScaleMode;
	
	char*			m_strPath;

	NEXSALFileHandle	m_pFile;
	NXBOOL			m_bSkipFirstFrame;

	unsigned char*	m_pHexY;
	unsigned char*	m_pHexU;
	unsigned char*	m_pHexV;

	unsigned char*	m_pTempY;
	unsigned char*	m_pTempU;
	unsigned char*	m_pTempV;

	unsigned char*	m_pRGBBuffer;

	unsigned int		m_uiCurrentFilePos;
	unsigned int		m_uiThumbnailCount;
	THUMBNAILINFO*	m_pThumbnailInfo;

	unsigned int*		m_pSeekTable;
	int				m_uSeekTableCount;

	unsigned char*	m_pPCMTable;
	int				m_uPCMTableCount;

	unsigned char*	m_pLargeThumbnail;
	int				m_iLargeThumbnailSize;
	int				m_iLargeThumbnailWidth;
	int				m_iLargeThumbnailHeight;
	
	NxVScalerStruct*	m_pScaler;

	NXT_HThemeRenderer	m_hThumbRenderer;	
	void*				m_pThumbDecSurface;
	void* 				m_pOutputSurface;

#ifdef YUV_TEST_DUMP
	FILE*			m_pDumpYUV;
#endif

	CNexCodecWrap*				m_pCodecWrap;

	unsigned int m_uStartTime;
	unsigned int m_uEndTime;
	unsigned int m_bRequestedRawData;

	unsigned char* m_pRAWData;
	unsigned int m_uRAWSize;

	unsigned char* m_pPCMData;
	unsigned int m_uPCMSize;	

	int*	m_pTimeTable;
	int	m_iTimeTableCount;

	int	m_iRotateState;

	std::vector<int> 			m_vecThumbTime;
};

int makeBigendian( unsigned int uiData );

#endif // __NEXVIDEOEDITOR_THUMBNAIL_H__
