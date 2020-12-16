/******************************************************************************
 * File Name   :	NEXVIDEOEDITOR_CodecWrap.h
 * Description :
 *******************************************************************************

 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation

 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 cskb		2015/10/13	Draft.
 -------------------------------------------------------------------------------
 eric		2016/03/25	renderDecodedFrame & releaseDecodedFrame were added.
                        For supporting multiple platform especially iOS.
 -----------------------------------------------------------------------------*/

#ifndef __NEXVIDEOEDITOR_CODECWRAP_H__
#define __NEXVIDEOEDITOR_CODECWRAP_H__

#include "NexCAL.h"
#include "nexIRef.h"
#include "nexDef.h"
#include "NEXVIDEOEDITOR_Def.h"

#include "NEXVIDEOEDITOR_CodecManager.h"

#define NEXCAL_PROPERTY_VIDEO_FPS						(NEXCAL_PROPERTY_USER_START + 0x100)
#define NEXCAL_PROPERTY_VIDEO_ENCODER_OPERATING_RATE	(NEXCAL_PROPERTY_USER_START + 0x101)
#define NEXCAL_PROPERTY_VIDEO_DECODER_MODE				(NEXCAL_PROPERTY_USER_START + 0x102)
#define NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2			(NEXCAL_PROPERTY_USER_START + 0x103)

class CNexCodecWrap : public CNxRef<INxRefObj>
{
public:
	CNexCodecWrap( void );
	virtual ~CNexCodecWrap( void );

	NXBOOL isInitDecoder();
	int getWidth();
	int getHeight();
	int getPitch();
	NXINT64 getFormat();

	void* getSurfaceTexture();
	NXBOOL setSurfaceTexture(void* pSurf);
	NXBOOL getCodec(NEXCALMediaType eType, NEXCALMode eMode, unsigned int uiOTI, int iWidth = 0, int iHeight = 0, int iPitch = 0);
	NXBOOL setCodec(NEXCALCodecHandle hCodecVideo);

	NXINT32 initDecoder(	NEX_CODEC_TYPE a_eCodecType,
							NXUINT8 *a_pConfig,
							NXINT32 a_nLen,
							NXUINT8 *a_pConfigEnhance,
							NXINT32 a_nEnhLen,
							NXVOID *a_pInitInfo,
							NXVOID *a_pExtraInfo,
							NXINT32 a_nNALHeaderSize,
							NXINT32 *a_pnWidth,
							NXINT32 *a_pnHeight,
							NXINT32 *a_pnPitch,
							NXINT32 a_nFps,							
							NXUINT32 a_uMode);
	NXBOOL deinitDecoder();

	NEXCALCodecHandle getCodecHandle();
	NXBOOL isHardwareCodec();
	#ifdef _ANDROID
	void postFrameRenderCallback(unsigned char* pBuff, NXBOOL bRender);
	#endif

	void renderDecodedFrame(unsigned char* pBuff);
	void releaseDecodedFrame(unsigned char* pBuff, NXBOOL isAfterRender);	

private:
	NXBOOL				m_bInitDecoder;
	NEXCALCodecHandle	m_hCodecVideo;
	NXINT64				m_uiCodecUserData;
	NXINT64				m_ColorFormat;

	int					m_iWidth;
	int					m_iHeight;
	int					m_iPitch;

	void*						m_pSurfaceTexture;
	postDisplayCallback_t		m_pPostDisplayCallback;
	void*						m_pPostDisplayCallbackUserData;
};

#endif // __NEXVIDEOEDITOR_CODECWRAP_H__
