/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_CodecManager.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2012/08/29	Draft.
-----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_CODECMANAGER_H__
#define __NEXVIDEOEDITOR_CODECMANAGER_H__

#include "NexSAL_Internal.h"
#include "NexCAL.h"
#include "NexMediaDef.h"
#include "nexLock.h"


#define NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE									(NEXCAL_PROPERTY_USER_START+0x700)
#		define NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE_NORMAL						0
#		define NEXCAL_PROPERTY_AVC_GETOUTPUT_MODE_THUMBNAIL					1


#ifndef postDisplayCallback_t

#ifdef FOR_TEST_MEDIACODEC_DEC
typedef void (*postDisplayCallback_t)(void *pBuffer, bool render, void* uUserData);
typedef void* (*getMediaCodecInputSurf)(void* uUserData);
typedef void (*setMediaCodecsetTimeStamp)(unsigned int uiPTS, void* uUserData);
typedef void (*resetMediaCodecVideoEncoder)(void* uUserData);


typedef unsigned int (*setCropToAchieveResolution)(unsigned char *dsi_in_out, unsigned int maxLength, unsigned int reqWidth, unsigned int reqHeight);


#else
typedef void (*postDisplayCallback_t)(void *calUD, void *mediaBuffer, bool rendered);
#endif

#endif

typedef struct _CALHandleUnit
{
	NEXCALCodecHandle	m_hCAL;
	NXBOOL				m_bUsed;
	NEXCALMode			m_eMode;
	int					m_iWidth;
	int					m_iHeight;
	int					m_iPitch;
}CALHandleUnit;

#define MAX_HARDWARECODEC_COUNT		16
	
class CNexCodecManager
{
public:
	CNexCodecManager( void );
	virtual ~CNexCodecManager( void );

	NXBOOL setHardwareResource(int iMemSize, int iHardwareDecodeCount, int iHardwareEncodeCount);

	static NEXCALCodecHandle getCodec(NEXCALMediaType eType, NEXCALMode eMode, unsigned int uiOTI, int iWidth = 0, int iHeight = 0, int iPitch = 0);
	static void releaseCodec(NEXCALCodecHandle hCodecHandle);

	static NXBOOL isHardwareCodec(NEXCALCodecHandle hCodecHandle);
	static NXBOOL canUseHardwareDecode();
	static NXBOOL canUseHardwareDecode(int iWidth, int iHeight);
	static NXBOOL canUseHardwareEncode();
	static NXBOOL canUseHardwareEncode(int iWidth, int iHeight);
	static NXBOOL isHardwareDecodeAvailable();
	static NXBOOL isHardwareDecodeAvailable(int iWidth, int iHeight);
	static NXBOOL isHardwareEncodeAvailable();
	static NXBOOL isHardwareEncodeAvailable(int iWidth, int iHeight);
	static int isVideoLayerAvailable();
	static int getHardwareDecodeMaxCount();
	static int getHardwareEncodeMaxCount();

	static int getHardwareMemRemainSize();
	static int getHardwareDecoderUseCount();

	static int isHardWareResourceAvailable(char *pModelIdentifier, int iCount, void* pInfo);

	static int getPrepareHardwareDecoderFlag();
	static void setPrepareHardwareDecoderFlag(int bSet);
	static void clearPrepareHardwareDecoderFlag();

	static NXBOOL canUse_SW_H264();
	static NXBOOL canUse_SW_MP4();
	static NXBOOL canUse_SW_AAC();
	static NXBOOL canUse_MC_SW_H264();
	static NXBOOL canUseMonoInfoFromCodec();

	static NXBOOL prepare_MC_AVC_Level(		NXBOOL bEncoder, 
												NXBOOL bHardware, 
												int* pBaselineLevel, 
												int* pBaselineSize, 
												int *pMainLevel, 
												int* pMainSize, 
												int* pHighLevel, 
												int* pHighSize);
	static CNexLock& getVideoDecInitLock();
	static unsigned int getVideoPreparationTime();
	static unsigned int getImagePreparationTime();
	static void setImagePreparationTime(int time);
	static CNexCodecManager*		m_pThis;
	
	NEXCALHandle				m_hCAL;
	
	int							m_iHardwareDecodeMaxCount;
	int							m_iHardwareDecodeUsedCount;
	
	int							m_iHardwareEncodeMaxCount;
	int							m_iHardwareEncodeUsedCount;

	int							m_iHardwareMemSize;
	int							m_iHardwareMemUsedSize;

	int							m_iPrepareHardwareDecoderFlag;

	CALHandleUnit				m_CodecEntry[MAX_HARDWARECODEC_COUNT];
	CNexLock					m_LockCodecEntry;

	CNexLock					m_LockVideoDecInit;	
private:
	static int getDeviceModel(char *pModelIdentifier);

	static unsigned int image_preparation_time;
};

#endif // __NEXVIDEOEDITOR_CODECMANAGER_H__
