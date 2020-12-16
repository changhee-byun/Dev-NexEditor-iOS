/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_CodecManager.cpp
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
#include "NEXVIDEOEDITOR_CodecManager.h"
#include "NEXVIDEOEDITOR_Def.h"
#include "NexCodecUtil.h"
#include "NEXVIDEOEDITOR_CodecWrap.h"
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PHONE_MODEL_IDENTIFIER "iPhone9,1" /*IPHONE_7*/
#define DEFAULT_OTHER_MODEL_IDENTIFIER "iPad6,3" /*IPAD_PRO_9*/

typedef struct VideoMomoryInfo
{
	NXINT32 iWidth;
	NXINT32 iHeight;
	NXINT32 ifps;
} VideoMomoryInfo;

static const struct {
	char	sModelName[16];
	int		iWidth;
	int		iHeight;
	int		iSupportedCodecCount;
	int		iSupportedMemorySize;
} SupportedCodecMemoryTable[] = {
	/*IPHONE_5S*/		{"iPhone6,1",	1920, 1080,  7,  8294400},
	/*IPHONE_5S*/		{"iPhone6,2",	1920, 1080,  7,  8294400},
	/*IPHONE_6P*/		{"iPhone7,1",	1920, 1080,  8,  8294400},
	/*IPHONE_6*/		{"iPhone7,2",	1920, 1080, 10, 14515200},
	/*IPHONE_6S*/		{"iPhone8,1",	3840, 2160, 10, 16588800},
	/*IPHONE_6SP*/		{"iPhone8,2",	3840, 2160, 10, 16588800},
	/*IPHONE_SE*/		{"iPhone8,4",	1920, 1080, 10, 16588800},
	/*IPHONE_7*/		{"iPhone9,1",	3840, 2160,	5,	16588800},
	/*IPHONE_7P*/		{"iPhone9,2",	3840, 2160,	10, 24883200},
	/*IPHONE_7*/		{"iPhone9,3",	3840, 2160,	5,	16588800},
	/*IPHONE_7P*/		{"iPhone9,4",	3840, 2160,	10, 24883200},
	/*IPHONE_8*/		{"iPhone10,1",	3840, 2160,	3,	16588800},
	/*IPHONE_8P*/		{"iPhone10,2",	3840, 2160,	9,	24883200},
	/*IPHONE_X*/		{"iPhone10,3",	3840, 2160,	9,	16588800},
	/*IPHONE_8*/		{"iPhone10,4",	3840, 2160,	3,	16588800},
	/*IPHONE_8P*/		{"iPhone10,5",	3840, 2160,	9,	24883200},
	/*IPHONE_X*/		{"iPhone10,6",	3840, 2160,	9,	16588800},
	/*IPHONE_XS*/		{"iPhone11,2",	3840, 2160,	10,	24883200},
	/*IPHONE_XSMAX*/	{"iPhone11,4",	3840, 2160,	10,	24883200},
	/*IPHONE_XSMAX*/	{"iPhone11,6",	3840, 2160,	10,	24883200},
	/*IPHONE_XR*/		{"iPhone11,8",	3840, 2160,	10,	16588800},

	/*IPOD_TOUCH_6G*/	{"iPod7,1",		1920, 1080,	10,	16588800},

	/*IPAD_AIR*/		{"iPad4,1",		1920, 1080,	7,	 8294400},
	/*IPAD_AIR*/		{"iPad4,2",		1920, 1080,	7,	 8294400},
	/*IPAD_AIR*/		{"iPad4,3",		1920, 1080,	7,	 8294400},
	/*IPAD_MINI_2*/		{"iPad4,4",		1920, 1080,	10,	 8294400},
	/*IPAD_MINI_2*/		{"iPad4,5",		1920, 1080,	10,	 8294400},
	/*IPAD_MINI_2*/		{"iPad4,6",		1920, 1080,	10,	 8294400},
	/*IPAD_MINI_3*/		{"iPad4,7",		1920, 1080,	8,	 8294400},
	/*IPAD_MINI_3*/		{"iPad4,9",		1920, 1080,	8,	 8294400},
	/*IPAD_MINI_3*/		{"iPad4,8",		1920, 1080,	8,	 8294400},
	/*IPAD_MINI_4*/		{"iPad5,1",		1920, 1080,	10,	12441600},
	/*IPAD_MINI_4*/		{"iPad5,2",		1920, 1080,	10,	12441600},
	/*IPAD_AIR_2*/		{"iPad5,3",		1920, 1080,	10,	12441600},
	/*IPAD_AIR_2*/		{"iPad5,4",		1920, 1080,	10,	12441600},
	/*IPAD_PRO_9*/		{"iPad6,3",		1920, 1080,	10,	24883200},
	/*IPAD_PRO_9*/		{"iPad6,4",		1920, 1080,	10,	24883200},
	/*IPAD_PRO_12*/		{"iPad6,7",		1920, 1080, 10,	24883200},
	/*IPAD_PRO_12*/		{"iPad6,8",		1920, 1080, 10,	24883200},
	/*IPAD_5_GEN*/		{"iPad6,11",	3840, 2160, 10,	16588800},
	/*IPAD_5_GEN*/		{"iPad6,12",	3840, 2160, 10,	16588800},
	/*IPAD_PRO_12_2GEN*/{"iPad7,1",		3840, 2160,	10,	24883200},
	/*IPAD_PRO_12_2GEN*/{"iPad7,2",		3840, 2160,	10,	24883200},
	/*IPAD_PRO_10*/		{"iPad7,3",		3840, 2160,	10,	24883200},
	/*IPAD_PRO_10*/		{"iPad7,4",		3840, 2160,	10,	24883200},
	/*IPAD_6_GEN*/		{"iPad7,5",		3840, 2160, 10,	16588800},
	/*IPAD_6_GEN*/		{"iPad7,6",		3840, 2160, 10,	16588800},
	/*IPAD_PRO_11*/		{"iPad8,1",		3840, 2160,	10,	33177600},
	/*IPAD_PRO_11*/		{"iPad8,2",		3840, 2160,	10,	33177600},
	/*IPAD_PRO_11*/		{"iPad8,3",		3840, 2160,	10,	33177600},
	/*IPAD_PRO_11*/		{"iPad8,4",		3840, 2160,	10,	33177600},
	/*IPAD_PRO_12_3GEN*/{"iPad8,5",		3840, 2160,	10,	33177600},
	/*IPAD_PRO_12_3GEN*/{"iPad8,6",		3840, 2160,	10,	33177600},
	/*IPAD_PRO_12_3GEN*/{"iPad8,7",		3840, 2160,	10,	33177600},
	/*IPAD_PRO_12_3GEN*/{"iPad8,8",		3840, 2160,	10,	33177600},

	// The following devices are not supported.
	/*IPHONE*/			{"iPhone1,1",	   0,    0,  0,        0},
	/*IPHONE_3G*/		{"iPhone1,2",	   0,    0,  0,        0},
	/*IPHONE_3GS*/		{"iPhone2,1",	   0,    0,  0,        0},
	/*IPHONE_4*/		{"iPhone3,1",	   0,    0,  0,        0},
	/*IPHONE_4*/		{"iPhone3,2",	   0,    0,  0,        0},
	/*IPHONE_4*/		{"iPhone3,3",	   0,    0,  0,        0},
	/*IPHONE_4S*/		{"iPhone4,1",	   0,    0,  0,        0},
	/*IPHONE_5*/		{"iPhone5,1",	   0,    0,  0,        0},
	/*IPHONE_5*/		{"iPhone5,2",	   0,    0,  0,        0},
	/*IPHONE_5C*/		{"iPhone5,3",	   0,    0,  0,        0},
	/*IPHONE_5C*/		{"iPhone5,4",	   0,    0,  0,        0},

	/*IPOD_TOUCH*/		{"iPod1,1",		   0,    0,  0,        0},
	/*IPOD_TOUCH_2G*/	{"iPod2,1",		   0,    0,  0,        0},
	/*IPOD_TOUCH_3G*/	{"iPod3,1",		   0,    0,  0,        0},
	/*IPOD_TOUCH_4G*/	{"iPod4,1",		   0,    0,  0,        0},
	/*IPOD_TOUCH_5G*/	{"iPod5,1",		   0,    0,  0,        0},

	/*IPAD*/			{"iPad1,1",		   0,    0,  0,        0},
	/*IPAD2*/			{"iPad2,1",		   0,    0,  0,        0},
	/*IPAD2*/			{"iPad2,2",		   0,    0,  0,        0},
	/*IPAD2*/			{"iPad2,3",		   0,    0,  0,        0},
	/*IPAD2*/			{"iPad2,4",		   0,    0,  0,        0},
	/*IPAD_MINI*/		{"iPad2,5",		   0,    0,  0,        0},
	/*IPAD_MINI*/		{"iPad2,6",		   0,    0,  0,        0},
	/*IPAD_MINI*/		{"iPad2,7",		   0,    0,  0,        0},
	/*IPAD3*/			{"iPad3,1",		   0,    0,  0,        0},
	/*IPAD3*/			{"iPad3,2",		   0,    0,  0,        0},
	/*IPAD3*/			{"iPad3,3",		   0,    0,  0,        0},
	/*IPAD4*/			{"iPad3,4",		   0,    0,  0,        0},
	/*IPAD4*/			{"iPad3,5",		   0,    0,  0,        0},
	/*IPAD4*/			{"iPad3,6",		   0,    0,  0,        0}
};
	

CNexCodecManager*	CNexCodecManager::m_pThis = NULL;

CNexCodecManager::CNexCodecManager( void )
{
	m_pThis					= this;

	m_hCAL					= NULL;

	m_iHardwareDecodeMaxCount	= 0;
	m_iHardwareDecodeUsedCount	= 0;
	
	m_iHardwareEncodeMaxCount	= 0;
	m_iHardwareEncodeUsedCount	= 0;

	m_iHardwareMemSize			= 0;
	m_iHardwareMemUsedSize		= 0;
	
	m_iPrepareHardwareDecoderFlag	= 0;
	
	memset(m_CodecEntry, 0x00, sizeof(CALHandleUnit)*MAX_HARDWARECODEC_COUNT);

	m_hCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
}

CNexCodecManager::~CNexCodecManager( void )
{
	m_pThis = NULL;

	if( m_hCAL )
	{
		nexCAL_ReleaseHandle(m_hCAL);
		m_hCAL = NULL;
	}
}

NXBOOL CNexCodecManager::setHardwareResource(int iMemSize, int iHardwareDecodeCount, int iHardwareEncodeCount)
{
	m_iHardwareDecodeMaxCount	= iHardwareDecodeCount;
	m_iHardwareDecodeUsedCount	= 0;
	
	m_iHardwareEncodeMaxCount	= iHardwareEncodeCount;
	m_iHardwareEncodeUsedCount	= 0;

	m_iHardwareMemSize			= iMemSize;
	m_iHardwareMemUsedSize		= 0;

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] setHardwareResource(decoderMax=%d encoderMax=%d size=%d)",
		__LINE__, m_iHardwareDecodeMaxCount, m_iHardwareEncodeMaxCount, m_iHardwareMemSize);
	return TRUE;
}

NEXCALCodecHandle CNexCodecManager::getCodec(NEXCALMediaType eType, NEXCALMode eMode, unsigned int uiOTI, int iWidth, int iHeight, int iPitch)
{
	
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return NULL;
	}
	
	NEXCALMethod uCalMethod;
	
	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return NULL;
	}

	if( uiOTI == eNEX_CODEC_V_H264 || uiOTI == eNEX_CODEC_V_MPEG4V || uiOTI == eNEX_CODEC_V_H264_MC_S || uiOTI == 0x50010301 || uiOTI == eNEX_CODEC_V_HEVC)
	{
#ifdef FOR_CHECK_MEMSIZE_TEST		
		if( m_pThis->m_iHardwareMemSize < m_pThis->m_iHardwareMemUsedSize + (iHeight * iPitch ) )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getCodec(not available hw codec mem Max:%d Cur:%d)", 
				__LINE__, m_pThis->m_iHardwareMemSize, m_pThis->m_iHardwareMemUsedSize + (iHeight * iPitch ));
			return NULL;
		}
#endif
		if( eMode == NEXCAL_MODE_DECODER && m_pThis->m_iHardwareDecodeMaxCount <= m_pThis->m_iHardwareDecodeUsedCount )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getCodec(not available hw codec Max:%d Cur:%d)", 
				__LINE__, m_pThis->m_iHardwareDecodeMaxCount, m_pThis->m_iHardwareDecodeUsedCount );
			return NULL;
		}
		else if( eMode == NEXCAL_MODE_ENCODER && m_pThis->m_iHardwareEncodeMaxCount <= m_pThis->m_iHardwareEncodeUsedCount )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getCodec(not available hw codec Max:%d Cur:%d)", 
				__LINE__, m_pThis->m_iHardwareEncodeMaxCount, m_pThis->m_iHardwareEncodeUsedCount );
			return NULL;
		}
		
		for(int i = 0; i < MAX_HARDWARECODEC_COUNT; i++)
		{
			if( m_pThis->m_CodecEntry[i].m_bUsed == FALSE )
			{
				unsigned int uiCodecType = uiOTI;

				if(uiOTI == eNEX_CODEC_V_H264_MC_S || uiOTI == 0x50010301)
				{
					uiCodecType = eNEX_CODEC_V_H264;
					uCalMethod = NEXCAL_METHOD_PLATFORM_SW;
				}
				else
				{
					uCalMethod = NEXCAL_METHOD_HW;
				}
            
				NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, eType, eMode, (NEX_CODEC_TYPE)uiCodecType, NULL, 0, 0, uCalMethod, NULL, NULL );
				if( hCALHandle == NULL )
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getCodec(getting codec handle failed)", __LINE__);
					return NULL;
				}
				m_pThis->m_CodecEntry[i].m_bUsed		= TRUE;
				m_pThis->m_CodecEntry[i].m_hCAL		= hCALHandle;
				m_pThis->m_CodecEntry[i].m_eMode		= eMode;
				m_pThis->m_CodecEntry[i].m_iWidth		= iWidth;
				m_pThis->m_CodecEntry[i].m_iHeight		= iHeight;
				m_pThis->m_CodecEntry[i].m_iPitch		= iPitch;

				m_pThis->m_iHardwareMemUsedSize +=  (iHeight * iPitch );

				eMode == NEXCAL_MODE_DECODER ? m_pThis->m_iHardwareDecodeUsedCount++ : m_pThis->m_iHardwareEncodeUsedCount++;
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] getCodec Hardware Codec successed oti(0x%x) (%p Mem Use %d UseCount:%d %d)", __LINE__, 
					uiOTI, hCALHandle, m_pThis->m_iHardwareMemUsedSize, m_pThis->m_iHardwareDecodeUsedCount, m_pThis->m_iHardwareEncodeUsedCount);
				return hCALHandle;
			}
		}
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getCodec(not available slot %d)", __LINE__);
		return NULL;
	}

	switch (uiOTI)
	{
		case eNEX_CODEC_A_AAC:
		case eNEX_CODEC_A_AACPLUS:
		case eNEX_CODEC_A_MP3:
		case eNEX_CODEC_A_AMR:
		case eNEX_CODEC_A_AMRWB:
			uCalMethod = NEXCAL_METHOD_PLATFORM_SW;
		break;

		case eNEX_CODEC_A_PCM_S16LE:
		case eNEX_CODEC_A_PCM_S16BE:
		case eNEX_CODEC_A_PCM_RAW:
		case eNEX_CODEC_A_PCM_FL32LE:
		case eNEX_CODEC_A_PCM_FL32BE:
		case eNEX_CODEC_A_PCM_FL64LE:
		case eNEX_CODEC_A_PCM_FL64BE:
		case eNEX_CODEC_A_PCM_IN24LE:
		case eNEX_CODEC_A_PCM_IN24BE:
		case eNEX_CODEC_A_PCM_IN32LE:
		case eNEX_CODEC_A_PCM_IN32BE:
		case eNEX_CODEC_A_PCM_LPCMLE:
		case eNEX_CODEC_A_PCM_LPCMBE:
		case eNEX_CODEC_A_FLAC:          
		case eNEX_CODEC_V_JPEG:
			uCalMethod = NEXCAL_METHOD_OWN_SW;
		break;
        
		case eNEX_CODEC_V_H264_S:          
			uiOTI = eNEX_CODEC_V_H264;
			uCalMethod = NEXCAL_METHOD_OWN_SW;
		break;

		case eNEX_CODEC_V_MPEG4V_S:          
			uiOTI = eNEX_CODEC_V_MPEG4V;
			uCalMethod = NEXCAL_METHOD_OWN_SW;
		break;

		case eNEX_CODEC_A_AAC_S:
			uiOTI = eNEX_CODEC_A_AAC;
			uCalMethod = NEXCAL_METHOD_OWN_SW;
		break;

		default:
			uCalMethod = NEXCAL_METHOD_OWN_SW;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getCodec(undefined Static property) %d", __LINE__, uiOTI);    		
		break;        
	}

	NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, eType, eMode, (NEX_CODEC_TYPE)uiOTI, NULL, 0, 0, uCalMethod, NULL, NULL );
	if( hCALHandle == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getCodec(getting codec handle failed)", __LINE__);
		return NULL;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] getCodec Software Successed oti(0x%x) (%p %d)", __LINE__, uiOTI, hCALHandle, m_pThis->m_iHardwareDecodeUsedCount);
	return hCALHandle;	
}

void CNexCodecManager::releaseCodec(NEXCALCodecHandle hCodecHandle)
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL || hCodecHandle == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] releaseCodec (%p)", __LINE__, hCodecHandle);

	for(int i = 0; i < MAX_HARDWARECODEC_COUNT; i++)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_FLOW, 0, "[CodecManager.cpp %d] CheckCodecType(%p == %p flag : %d)",
			__LINE__, m_pThis->m_CodecEntry[i].m_hCAL, hCodecHandle, m_pThis->m_CodecEntry[i].m_bUsed);
		
		if( m_pThis->m_CodecEntry[i].m_bUsed && m_pThis->m_CodecEntry[i].m_hCAL == hCodecHandle )
		{
			nexCAL_ReleaseCodec( m_pThis->m_CodecEntry[i].m_hCAL, NULL);
			m_pThis->m_iHardwareMemUsedSize -= (m_pThis->m_CodecEntry[i].m_iWidth * m_pThis->m_CodecEntry[i].m_iHeight );

			m_pThis->m_CodecEntry[i].m_hCAL		= NULL;
			m_pThis->m_CodecEntry[i].m_bUsed		= FALSE;
			m_pThis->m_CodecEntry[i].m_iWidth		= 0;
			m_pThis->m_CodecEntry[i].m_iHeight		= 0;
			m_pThis->m_CodecEntry[i].m_iPitch		= 0;
			
			m_pThis->m_CodecEntry[i].m_eMode == NEXCAL_MODE_DECODER ? m_pThis->m_iHardwareDecodeUsedCount-- : m_pThis->m_iHardwareEncodeUsedCount--;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] release HardWare Codec successed(%p Mem Use %d UseCount %d %d)", 
				__LINE__, hCodecHandle, m_pThis->m_iHardwareMemUsedSize, m_pThis->m_iHardwareDecodeUsedCount, m_pThis->m_iHardwareEncodeUsedCount);
			return;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] release SoftWare Codec successed(%p %d)", __LINE__, hCodecHandle, m_pThis->m_iHardwareDecodeUsedCount);
	nexCAL_ReleaseCodec( hCodecHandle, NULL );
}

NXBOOL CNexCodecManager::isHardwareCodec(NEXCALCodecHandle hCodecHandle)
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL && hCodecHandle == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}

	for(int i = 0; i < MAX_HARDWARECODEC_COUNT; i++)
	{
		if( m_pThis->m_CodecEntry[i].m_bUsed && m_pThis->m_CodecEntry[i].m_hCAL == hCodecHandle )
		{
			return TRUE;
		}
	}
	return FALSE;
}

NXBOOL CNexCodecManager::canUseHardwareDecode()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}
	
	if( m_pThis->m_iHardwareDecodeMaxCount > 0 )
		return TRUE;

	return FALSE;
}

NXBOOL CNexCodecManager::canUseHardwareDecode(int iWidth, int iHeight)
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}
	
	if( m_pThis->m_iHardwareMemSize <= iWidth * iHeight )
		return FALSE;
	
	if( m_pThis->m_iHardwareDecodeMaxCount > 0 )
		return TRUE;

	return FALSE;
}

NXBOOL CNexCodecManager::canUseHardwareEncode()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}
	
	if( m_pThis->m_iHardwareEncodeMaxCount > 0 )
		return TRUE;
	return FALSE;
}

NXBOOL CNexCodecManager::canUseHardwareEncode(int iWidth, int iHeight)
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}
	
	if( m_pThis->m_iHardwareMemSize <= iWidth * iHeight )
		return FALSE;
	
	if( m_pThis->m_iHardwareEncodeMaxCount > 0 )
		return TRUE;
	return FALSE;
}

NXBOOL CNexCodecManager::isHardwareDecodeAvailable()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}

	if( m_pThis->m_iHardwareDecodeMaxCount <= m_pThis->m_iHardwareDecodeUsedCount )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec count is full(Max:%d, Count:%d)", 
			__LINE__, 
			m_pThis->m_iHardwareDecodeMaxCount,
			m_pThis->m_iHardwareDecodeUsedCount);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec is available(Module %d %d, MemSize %d %d)", 
		__LINE__, 
		m_pThis->m_iHardwareDecodeMaxCount,
		m_pThis->m_iHardwareDecodeUsedCount,
		m_pThis->m_iHardwareMemSize,
		m_pThis->m_iHardwareMemUsedSize);
	return TRUE;
}

NXBOOL CNexCodecManager::isHardwareDecodeAvailable(int iWidth, int iHeight)
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}
	
	if( m_pThis->m_iHardwareDecodeMaxCount <= m_pThis->m_iHardwareDecodeUsedCount )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec count is full(Max:%d, Used:%d)", 
			__LINE__, 
			m_pThis->m_iHardwareDecodeMaxCount,
			m_pThis->m_iHardwareDecodeUsedCount);
		return FALSE;
	}

#ifdef FOR_CHECK_MEMSIZE_TEST		
	if( m_pThis->m_iHardwareMemSize < (m_pThis->m_iHardwareMemUsedSize +(iWidth * iHeight)) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec mem is full(Max:%d, Used:%d)", 
			__LINE__, 
			m_pThis->m_iHardwareMemSize,
			m_pThis->m_iHardwareMemUsedSize +(iWidth * iHeight));
		return FALSE;
	}
#endif	

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec is available(Module %d %d, MemSize %d %d)", 
		__LINE__, 
		m_pThis->m_iHardwareDecodeMaxCount,
		m_pThis->m_iHardwareDecodeUsedCount,
		m_pThis->m_iHardwareMemSize,
		m_pThis->m_iHardwareMemUsedSize +(iWidth * iHeight));
	return TRUE;
}

NXBOOL CNexCodecManager::isHardwareEncodeAvailable()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}
	
	if( m_pThis->m_iHardwareEncodeMaxCount <= m_pThis->m_iHardwareEncodeUsedCount )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec count is full(Max:%d, Count:%d)", 
			__LINE__, 
			m_pThis->m_iHardwareEncodeMaxCount,
			m_pThis->m_iHardwareEncodeUsedCount);
		return FALSE;
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec is available(Module %d %d, MemSize %d %d)", 
		__LINE__, 
		m_pThis->m_iHardwareEncodeMaxCount,
		m_pThis->m_iHardwareEncodeUsedCount,
		m_pThis->m_iHardwareMemSize,
		m_pThis->m_iHardwareMemUsedSize);
	return TRUE;
}

NXBOOL CNexCodecManager::isHardwareEncodeAvailable(int iWidth, int iHeight)
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return FALSE;
	}

	CAutoLock m(m_pThis->m_LockCodecEntry);
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return FALSE;
	}
	
	if( m_pThis->m_iHardwareEncodeMaxCount <= m_pThis->m_iHardwareEncodeUsedCount )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec count is full(Max:%d, Count:%d)", 
			__LINE__, 
			m_pThis->m_iHardwareEncodeMaxCount,
			m_pThis->m_iHardwareEncodeUsedCount);
		return FALSE;
	}

	if( m_pThis->m_iHardwareMemSize < (m_pThis->m_iHardwareMemUsedSize +(iWidth * iHeight)) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec mem is full(Max:%d, Count:%d)", 
			__LINE__, 
			m_pThis->m_iHardwareMemSize,
			m_pThis->m_iHardwareMemUsedSize +(iWidth * iHeight));
		return FALSE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] HW Codec is available(Module %d %d, MemSize %d %d)", 
		__LINE__, 
		m_pThis->m_iHardwareEncodeMaxCount,
		m_pThis->m_iHardwareEncodeUsedCount,
		m_pThis->m_iHardwareMemSize,
		m_pThis->m_iHardwareMemUsedSize +(iWidth * iHeight));
	return TRUE;
}

int CNexCodecManager::getHardwareDecodeMaxCount()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return 0;
	}
	
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return 0;
	}
	return m_pThis->m_iHardwareDecodeMaxCount;
}

int CNexCodecManager::getHardwareEncodeMaxCount()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return 0;
	}
	
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return 0;
	}

	return m_pThis->m_iHardwareEncodeMaxCount;
}

int CNexCodecManager::getHardwareMemRemainSize()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return 0;
	}
	
	return m_pThis->m_iHardwareMemSize - m_pThis->m_iHardwareMemUsedSize;
}

int CNexCodecManager::getHardwareDecoderUseCount()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return 0;
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getHardwareDecoderUseCount(%d)", __LINE__, m_pThis->m_iHardwareDecodeUsedCount);
	return m_pThis->m_iHardwareDecodeUsedCount;
}

int CNexCodecManager::getDeviceModel(char *pModelIdentifier)
{
	int iRet = -1;
	int iDefaultDeviceIndex = -1;
	char *pDefaultModelIdentifier = NULL;

	if (pModelIdentifier)
	{
		if (!strncmp(pModelIdentifier, "iPhone", 6))
			pDefaultModelIdentifier = DEFAULT_PHONE_MODEL_IDENTIFIER;
		else
			pDefaultModelIdentifier = DEFAULT_OTHER_MODEL_IDENTIFIER;

		for (int iIndex = 0 ; iIndex < sizeof(SupportedCodecMemoryTable) ; iIndex++)
		{
			if (iDefaultDeviceIndex == -1)
			{
				if (!strncmp(SupportedCodecMemoryTable[iIndex].sModelName, pDefaultModelIdentifier, sizeof(SupportedCodecMemoryTable[iIndex].sModelName) - 1))		
					iDefaultDeviceIndex= iIndex;;
			}

			if (!strncmp(SupportedCodecMemoryTable[iIndex].sModelName, pModelIdentifier, sizeof(SupportedCodecMemoryTable[iIndex].sModelName) - 1))
			{
				iRet = iIndex;
				break;
			}
		}
	}

	if (iRet == -1)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] getDeviceModel, can't find the registed device so set Iphone 7", __LINE__);
		iRet = iDefaultDeviceIndex;
	}

	return iRet;
}

int CNexCodecManager::isHardWareResourceAvailable(char *pModelIdentifier, int iCount, void* pInfo)
{
	int iRet = 0;
	int iSupportedCodecCount = 0, iSupportedMemorySize = 0, iSupportedWidth = 0, iSupportedHeight = 0;

	int iIndex = getDeviceModel(pModelIdentifier);

	iSupportedWidth			= SupportedCodecMemoryTable[iIndex].iWidth;
	iSupportedHeight		= SupportedCodecMemoryTable[iIndex].iHeight;	
	iSupportedCodecCount	= SupportedCodecMemoryTable[iIndex].iSupportedCodecCount;
	iSupportedMemorySize	= SupportedCodecMemoryTable[iIndex].iSupportedMemorySize;

	if ( iSupportedWidth		== 0 ||
		 iSupportedHeight		== 0 ||
		 iSupportedCodecCount	== 0 ||
		 iSupportedMemorySize	== 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] isHardWareResourceAvailable failed, iSupportedWidth(%d), iSupportedHeight(%d), iSupportedCodecCount(%d), iSupportedMemorySize(%d)", __LINE__, iSupportedWidth, iSupportedHeight, iSupportedCodecCount, iSupportedMemorySize);
		return iRet;
	}

	VideoMomoryInfo *pVideoInfo = (VideoMomoryInfo*)pInfo;
	if (pVideoInfo)
	{
		int iContentWidth = 0, iContentHeight = 0, iContentFps = 0;
		float fWeight = 1.0;
		int iAppliedCodecCount = iCount > iSupportedCodecCount ? iSupportedCodecCount : iCount;

		for (int i = 0 ; i < iAppliedCodecCount ; i++)
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] isHardWareResourceAvailable index:%d, width:%d, height:%d, fps:%d", __LINE__, i, pVideoInfo[i].iWidth, pVideoInfo[i].iHeight, pVideoInfo[i].ifps);

			iContentWidth	= pVideoInfo[i].iWidth;
			iContentHeight	= pVideoInfo[i].iHeight;
			iContentFps		= pVideoInfo[i].ifps;

			if (iContentWidth == 0 || iContentHeight == 0 || iContentFps == 0)
			{
				if (iContentWidth == 0 || iContentHeight == 0)
				{
					iContentWidth	= iSupportedWidth;
					iContentHeight	= iSupportedHeight;
				}
				if (iContentFps == 0)
				{
					iContentFps = 60;
				}
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] isHardWareResourceAvailable, some video info is zero (index:%d, w:%d, h:%d, fps:%d) so change to (w:%d, h:%d, fps:%d) ", __LINE__, i, pVideoInfo[i].iWidth, pVideoInfo[i].iHeight, pVideoInfo[i].ifps, iContentWidth, iContentHeight, iContentFps);
			}

			if (iSupportedWidth * iSupportedHeight < iContentWidth * iContentHeight)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] isHardWareResourceAvailable Fail, resolution is exceeded (index:%d) (%d, %d, %d, %d) ", __LINE__, i, iSupportedWidth, iSupportedHeight, iContentWidth, iContentHeight);
				iRet = i;
				break;
			}
#if 0
			if (iContentFps > 0 && iContentFps <= 33)
				fWeight = 0.5;
			else if (iContentFps > 33 && iContentFps <= 63)
				fWeight = 1;
			else if (iContentFps > 63 && iContentFps <= 93)
				fWeight = 1.5;
			else if (iContentFps >= 94)
				fWeight = 2;
#endif
			iSupportedMemorySize -= (int)(iContentWidth * iContentHeight * fWeight);
			if (iSupportedMemorySize < 0)
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] isHardWareResourceAvailable Fail, is not enough memory. (%d, %d, %d) ", __LINE__, i, iCount, iSupportedMemorySize);
				break;
			}
			iRet = i+1;
		}
	}

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] isHardWareResourceAvailable iRet:%d, pModelIdentifier:%s, iRemainedMemorySize:%d", __LINE__, iRet, pModelIdentifier, iSupportedMemorySize);
	return iRet;
}

int CNexCodecManager::getPrepareHardwareDecoderFlag()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return 0;
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getPrepareHardwareDecoderFlag(%d)", __LINE__, m_pThis->m_iPrepareHardwareDecoderFlag);
	return m_pThis->m_iPrepareHardwareDecoderFlag;
}

void CNexCodecManager::setPrepareHardwareDecoderFlag(int bSet)
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return;
	}
	// nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] setPrepareHardwareDecoderFlag(%d)", __LINE__, bSet);
	m_pThis->m_iPrepareHardwareDecoderFlag = bSet;
}

void CNexCodecManager::clearPrepareHardwareDecoderFlag()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] clearPrepareHardwareDecoderFlag", __LINE__);
	m_pThis->m_iPrepareHardwareDecoderFlag = 0;
}

NXBOOL CNexCodecManager::canUse_SW_H264()
{
	NXBOOL bCanUse = FALSE;
	NEXCALHandle hCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
	if( hCAL )
	{
		NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_H264/*(NEX_CODEC_TYPE)eNEX_CODEC_V_H264_S*/, NULL, 0, 0, NEXCAL_METHOD_OWN_SW, NULL, NULL ); 
		if( hCALHandle )
		{
			bCanUse = TRUE;
			nexCAL_ReleaseCodec( hCALHandle, NULL );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can use h264 decoder", __LINE__);
		}
		nexCAL_ReleaseHandle(hCAL);
		return bCanUse;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can not use h264 decoder", __LINE__);
	return bCanUse;
}

NXBOOL CNexCodecManager::canUse_SW_MP4()
{
	NXBOOL bCanUse = FALSE;
	NEXCALHandle hCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
	if( hCAL )
	{
		NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_MPEG4V/*(NEX_CODEC_TYPE)eNEX_CODEC_V_MPEG4V_S*/, NULL, 0, 0, NEXCAL_METHOD_OWN_SW, NULL, NULL ); 
		if( hCALHandle )
		{
			bCanUse = TRUE;
			nexCAL_ReleaseCodec( hCALHandle, NULL );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can use mp4v decoder", __LINE__);
		}
		nexCAL_ReleaseHandle(hCAL);
		return bCanUse;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can not use mp4v decoder", __LINE__);
	return bCanUse;
}

NXBOOL CNexCodecManager::canUse_SW_AAC()
{
	NXBOOL bCanUse = FALSE;
	NEXCALHandle hCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
	if( hCAL )
	{
		NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, eNEX_CODEC_A_AAC/*(NEX_CODEC_TYPE)eNEX_CODEC_A_AAC_S*/, NULL, 0, 0, NEXCAL_METHOD_OWN_SW, NULL, NULL ); 
		if( hCALHandle )
		{
			bCanUse = TRUE;
			nexCAL_ReleaseCodec( hCALHandle, NULL );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can use sw aac decoder", __LINE__);
		}
		nexCAL_ReleaseHandle(hCAL);
		return bCanUse;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can not use sw aac decoder", __LINE__);
	return bCanUse;
}

NXBOOL CNexCodecManager::canUse_MC_SW_H264()
{
	NXBOOL bCanUse = FALSE;
	NEXCALHandle hCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
	if( hCAL )
	{
		NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, NEXCAL_MEDIATYPE_VIDEO, NEXCAL_MODE_DECODER, eNEX_CODEC_V_H264/*(NEX_CODEC_TYPE)eNEX_CODEC_V_H264_MC_S*/, NULL, 0, 0, NEXCAL_METHOD_PLATFORM_SW, NULL, NULL ); 
		if( hCALHandle )
		{
			bCanUse = TRUE;
			nexCAL_ReleaseCodec( hCALHandle, NULL );
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can use mc sw H264 decoder", __LINE__);
		}
		nexCAL_ReleaseHandle(hCAL);
		return bCanUse;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] can not use mc sw H264 decoder", __LINE__);
	return bCanUse;
}

NXBOOL CNexCodecManager::canUseMonoInfoFromCodec()
{
	NXBOOL bCanUse = TRUE;
#ifdef _ANDROID
	NEXCALHandle hCAL = nexCAL_GetHandle(0, NEXCAL_COMPATIBILITY_NUM);
	if( hCAL )
	{
		NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, NEXCAL_MEDIATYPE_AUDIO, NEXCAL_MODE_DECODER, (NEX_CODEC_TYPE)eNEX_CODEC_A_MP3, NULL, 0, 0, NEXCAL_METHOD_PLATFORM_SW, NULL, NULL ); 
		if( hCALHandle )
		{
			unsigned char pDSI[] = {0xac, 0x44, 0x01, 0x04, 0x08};
			unsigned int uDSISize = sizeof(pDSI);

			unsigned char pFrame[] = {0xFF, 0xFB, 0x50, 0xC4, 0x00, 0x03, 0xC0, 0x00, 0x01, 0xA4, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00,  
							0x34, 0x80, 0x00, 0x00, 0x04, 0x4C, 0x41, 0x4D, 0x45, 0x33, 0x2E, 0x39, 0x39, 0x2E, 0x35, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x4C, 0x41, 0x4D, 0x45, 0x33, 0x2E, 0x39,  
							0x39, 0x2E, 0x35, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,   
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,  
							0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55 };
			unsigned int uFrameSize = sizeof(pFrame);

			int iRet = 0;
			unsigned int uSamplingRate = 44100;
			unsigned int uNumOfChannels = 1;
			unsigned int uNumOfSamplesPerChannel = 1152;
			unsigned int uBitsPerSample = 16;
			NXBOOL bFrameEnd = FALSE;			
			unsigned int uiDecoderErrRet = 0;
	
			unsigned char* pAudioDecodeBuf = NULL; 
			unsigned int uiPCMBufSize = 384*1024;

			unsigned int uiSamplingRate = 0;
			unsigned int uiChannels = 0;
			unsigned int uiBitPerSample = 0;
			unsigned int uiNumOfSamplesPerChannel = 0;

			pAudioDecodeBuf = (unsigned char*)nexSAL_MemAlloc(384*1024);

			if(pAudioDecodeBuf == NULL)
			{
				nexCAL_ReleaseCodec( hCALHandle, NULL );
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] pAudioDecodeBuf mem alloc failed", __LINE__);
				nexCAL_ReleaseHandle(hCAL);
				return bCanUse;
			}

			MPEG_AUDIO_INFO mpa_info;
			memset(&mpa_info, 0x00, sizeof(MPEG_AUDIO_INFO));
			NexCodecUtil_Get_MPEG_Audio_Info( pFrame, uFrameSize, &mpa_info );
			if (mpa_info.Version == NEX_MPEG_AUDIO_VER_1 && mpa_info.Layer == NEX_MPEG_AUDIO_LAYER_2)
				nexCAL_AudioDecoderSetProperty(hCALHandle, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 1);
			else
				nexCAL_AudioDecoderSetProperty(hCALHandle, NEXCAL_PROPERTY_AUDIO_MPEGAUDIO_LAYER2, 0);

			iRet = nexCAL_AudioDecoderInit(		hCALHandle,
												eNEX_CODEC_A_MP3, 
												pDSI, 
												uDSISize, 
												pFrame, 
												uFrameSize, 
												NULL,
												NULL,
												&uSamplingRate,
												&uNumOfChannels, 
												&uBitsPerSample, 
												&uNumOfSamplesPerChannel, 
												NEXCAL_ADEC_MODE_NONE,
												0, // USERDATATYPE
												(NXVOID *)hCALHandle );

			if( iRet != 0 ){
				nexCAL_ReleaseCodec( hCALHandle, NULL );
				nexCAL_ReleaseHandle(hCAL);
				return FALSE;
			}

			while(1)
			{
				uiPCMBufSize = 384*1024;
					
				nexCAL_AudioDecoderDecode(	hCALHandle,
											pFrame, 
											uFrameSize, 
											NULL, 
											pAudioDecodeBuf,
											(int *)&uiPCMBufSize,
											0,
											NULL,
											bFrameEnd == FALSE?NEXCAL_ADEC_FLAG_NONE:NEXCAL_ADEC_FLAG_END_OF_STREAM,
											&uiDecoderErrRet );

				bFrameEnd = TRUE;
				if( NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_DECODING_SUCCESS))
				{
					if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_CHANGED))
					{
						nexCAL_AudioDecoderGetInfo(hCALHandle , NEXCAL_AUDIO_GETINFO_SAMPLINGRATE, &uiSamplingRate);
						nexCAL_AudioDecoderGetInfo(hCALHandle , NEXCAL_AUDIO_GETINFO_NUMOFCHNNELS, &uiChannels);
						nexCAL_AudioDecoderGetInfo(hCALHandle , NEXCAL_AUDIO_GETINFO_BITSPERSAMPLE, &uiBitPerSample);
						nexCAL_AudioDecoderGetInfo(hCALHandle , NEXCAL_AUDIO_GETINFO_NUMOFSAMPLESPERCHANNEL, &uiNumOfSamplesPerChannel);

						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] Audio Output was changed!(S(%d) C(%d) BpS(%d) SC(%d)", 
							__LINE__, uiSamplingRate, uiChannels, uiBitPerSample, uiNumOfSamplesPerChannel);

						if(uiChannels == uNumOfChannels)
							break;
					}
					
					if(NEXCAL_CHECK_ADEC_RET(uiDecoderErrRet, NEXCAL_ADEC_OUTPUT_EXIST))			
					{
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] Audio Output pcm Size = %d", __LINE__, uiPCMBufSize);

						if(uiNumOfSamplesPerChannel == 0)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] Audio uiNumOfSamplesPerChannel = %d", __LINE__, uiNumOfSamplesPerChannel);
							break;
						}

						if(uiPCMBufSize/2/uiNumOfSamplesPerChannel != uiChannels)
						{
							nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] Can not trust Audio Mp3 Output Channel Info", __LINE__);
							bCanUse = FALSE;
						}
						break;
					}
				}
				else
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] decoding fail", __LINE__);
					break;
				}
			}

			nexSAL_MemFree(pAudioDecodeBuf);
			nexCAL_AudioDecoderDeinit( hCALHandle );	
			nexCAL_ReleaseCodec( hCALHandle, NULL );
		}
		nexCAL_ReleaseHandle(hCAL);
		return bCanUse;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] shuould trust audio info from codec", __LINE__);
#endif
	return bCanUse;
}

NXBOOL getLevelSupportsSize(int level, int* pSize)
{
	if( pSize == NULL ) return FALSE;
		
	int maxFrameSizeInMBs = 0;
	int maxMBsPerSecond = 0;

	switch (level)
	{
	case 100: /* AVCLevel1 */
	case 105: /* AVCLevel1b */
		maxFrameSizeInMBs = 99;
		maxMBsPerSecond = 1485;
		break;
	case 110: /* AVCLevel11 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 3000;
		break;
	case 120: /* AVCLevel12 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 6000;
		break;
	case 130: /* AVCLevel13 */
	case 200: /* AVCLevel2 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 11880;
		break;
	case 210: /* AVCLevel21 */
		maxFrameSizeInMBs = 792;
		maxMBsPerSecond = 19800;
		break;
	case 220: /* AVCLevel22 */
		maxFrameSizeInMBs = 1620;
		maxMBsPerSecond = 20250;
		break;
	case 300: /* AVCLevel3 */
		maxFrameSizeInMBs = 1620;
		maxMBsPerSecond = 40500;
		break;
	case 310: /* AVCLevel31 */
		maxFrameSizeInMBs = 3600;
		maxMBsPerSecond = 108000;
		break;
	case 320: /* AVCLevel32 */
		maxFrameSizeInMBs = 5120;
		maxMBsPerSecond = 216000;
		break;
	case 400: /* AVCLevel4 */
	case 410: /* AVCLevel41 */
		maxFrameSizeInMBs = 8192;
		maxMBsPerSecond = 245760;
		break;
	case 420: /* AVCLevel42 */
		maxFrameSizeInMBs = 8704;
		maxMBsPerSecond = 522240;
		break;
	case 500: /* AVCLevel5 */
		maxFrameSizeInMBs = 22080;
		maxMBsPerSecond = 589824;
		break;
	case 510: /* AVCLevel51 */
		maxFrameSizeInMBs = 36864;
		maxMBsPerSecond = 983040;
		break;
	case 520: /* AVCLevel52 */
		maxFrameSizeInMBs = 36864;
		maxMBsPerSecond = 2073600;
		break;
	default:
		*pSize = 0;
		return FALSE;
	}

	int iSize = maxFrameSizeInMBs * 16*16 ;
	*pSize = iSize;
	return TRUE;
}

NXBOOL CNexCodecManager::prepare_MC_AVC_Level(	NXBOOL bEncoder, 
														NXBOOL bHardware, 
														int* pBaselineLevel, 
														int* pBaselineSize, 
														int *pMainLevel, 
														int* pMainSize, 
														int* pHighLevel, 
														int* pHighSize)
{
	if( 	pBaselineLevel == NULL || pBaselineSize == NULL ||
		pMainLevel == NULL || pMainSize == NULL ||
		pHighLevel == NULL || pHighSize == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] get_MC_AVC_HW_Level failed(param error)", __LINE__);
		return FALSE;
	}
	
	NEXCALHandle hCAL = nexCAL_GetHandle( 0, NEXCAL_COMPATIBILITY_NUM);
	if( hCAL )
	{
		int codec = bHardware ? eNEX_CODEC_V_H264 : eNEX_CODEC_V_H264_MC_S;
		NEXCALMode eMode = bEncoder ? NEXCAL_MODE_ENCODER : NEXCAL_MODE_DECODER;
		
		NEXCALMethod uCalMethod;
		if(codec == eNEX_CODEC_V_H264)
			uCalMethod = NEXCAL_METHOD_HW;

		if(codec == eNEX_CODEC_V_H264_MC_S)
			uCalMethod = NEXCAL_METHOD_PLATFORM_SW;
		
		NEXCALCodecHandle hCALHandle = nexCAL_GetCodec( m_pThis->m_hCAL, NEXCAL_MEDIATYPE_VIDEO, eMode, eNEX_CODEC_V_H264, NULL, 0, 0, uCalMethod, NULL, NULL ); 
		if( hCALHandle )
		{
			NXINT64 iBaseLineLevel = 0;
			NXINT64 iMainLevel = 0;
			NXINT64 iHighLevel = 0;

			int iBaselineSize = 0;
			int iMainSize = 0;
			int iHighSize = 0;
			
			if( bEncoder )
			{
				nexCAL_VideoEncoderGetProperty( hCALHandle, NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL, &iBaseLineLevel);
				nexCAL_VideoEncoderGetProperty( hCALHandle, NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL, &iMainLevel);
				nexCAL_VideoEncoderGetProperty( hCALHandle, NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL, &iHighLevel);
			}
			else
			{
				nexCAL_VideoDecoderGetProperty( hCALHandle, NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL, &iBaseLineLevel);
				nexCAL_VideoDecoderGetProperty( hCALHandle, NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL, &iMainLevel);
				nexCAL_VideoDecoderGetProperty( hCALHandle, NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL, &iHighLevel);
			}


			getLevelSupportsSize(iBaseLineLevel * 10, &iBaselineSize);
			getLevelSupportsSize(iMainLevel* 10, &iMainSize);
			getLevelSupportsSize(iHighLevel* 10, &iHighSize);

			*pBaselineLevel = iBaseLineLevel;
			*pMainLevel = iMainLevel;
			*pHighLevel = iHighLevel;

			*pBaselineSize = iBaselineSize;
			*pMainSize = iMainSize;
			*pHighSize = iHighSize;

			nexCAL_ReleaseCodec( hCALHandle, NULL );
			nexCAL_ReleaseHandle(hCAL);
			if( bEncoder )
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] get avc enc level (b:%lld m:%lld h:%lld)", __LINE__, iBaseLineLevel, iMainLevel, iHighLevel);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] get avc enc level Size(b:%d m:%d h:%d)", __LINE__, iBaselineSize, iMainSize, iHighSize);
			}
			else
			{
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] get avc dec level (b:%lld m:%lld h:%lld)", __LINE__, iBaseLineLevel, iMainLevel, iHighLevel);
				nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] get avc dec level Size (b:%d m:%d h:%d)", __LINE__, iBaselineSize, iMainSize, iHighSize);
			}
			return true;
		}
		nexCAL_ReleaseHandle(hCAL);
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] get_MC_AVC_HW_Level failed(can't use avc mc codec)", __LINE__);
		return FALSE;
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[CodecManager.cpp %d] get_MC_AVC_HW_Level failed(can't use cal)", __LINE__);
	return FALSE;
}

#if 0
bool LevelSupportsContentAtFPS(int level, Common::spsInfo_t *info, float fps)
{
	int maxFrameSizeInMBs = 0;
	int maxMBsPerSecond = 0;

	int contentFrameSizeInMBs = (info->pic_width_in_mbs_minus1 + 1) * (info->pic_height_in_map_units_minus1 + 1);

	switch (level)
	{
	case 100: /* AVCLevel1 */
	case 105: /* AVCLevel1b */
		maxFrameSizeInMBs = 99;
		maxMBsPerSecond = 1485;
		break;
	case 110: /* AVCLevel11 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 3000;
		break;
	case 120: /* AVCLevel12 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 6000;
		break;
	case 130: /* AVCLevel13 */
	case 200: /* AVCLevel2 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 11880;
		break;
	case 210: /* AVCLevel21 */
		maxFrameSizeInMBs = 792;
		maxMBsPerSecond = 19800;
		break;
	case 220: /* AVCLevel22 */
		maxFrameSizeInMBs = 1620;
		maxMBsPerSecond = 20250;
		break;
	case 300: /* AVCLevel3 */
		maxFrameSizeInMBs = 1620;
		maxMBsPerSecond = 40500;
		break;
	case 310: /* AVCLevel31 */
		maxFrameSizeInMBs = 3600;
		maxMBsPerSecond = 108000;
		break;
	case 320: /* AVCLevel32 */
		maxFrameSizeInMBs = 5120;
		maxMBsPerSecond = 216000;
		break;
	case 400: /* AVCLevel4 */
	case 410: /* AVCLevel41 */
		maxFrameSizeInMBs = 8192;
		maxMBsPerSecond = 245760;
		break;
	case 420: /* AVCLevel42 */
		maxFrameSizeInMBs = 8704;
		maxMBsPerSecond = 522240;
		break;
	case 500: /* AVCLevel5 */
		maxFrameSizeInMBs = 22080;
		maxMBsPerSecond = 589824;
		break;
	case 510: /* AVCLevel51 */
		maxFrameSizeInMBs = 36864;
		maxMBsPerSecond = 983040;
		break;
	case 520: /* AVCLevel52 */
		maxFrameSizeInMBs = 36864;
		maxMBsPerSecond = 2073600;
		break;
	default:
		break;
	}

	if (contentFrameSizeInMBs > maxFrameSizeInMBs)
	{
		return false;
	}

	if (contentFrameSizeInMBs > (maxMBsPerSecond / fps))
	{
		return false;
	}

	return true;
}
#endif

CNexLock& CNexCodecManager::getVideoDecInitLock()
{
	/*Dead Code;
	if( m_pThis == NULL )
	{
		CNexLock lock;
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return lock;
	}
	*/
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] getVideoDecInitLock", __LINE__);
	return m_pThis->m_LockVideoDecInit;
}

unsigned int CNexCodecManager::getVideoPreparationTime()
{
	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return 1500;
	}
	
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return 1500;
	}
	if(m_pThis->m_iHardwareEncodeUsedCount > 0)
		return 990;
	return 1500;
}

unsigned int CNexCodecManager::image_preparation_time = 1000;

unsigned int CNexCodecManager::getImagePreparationTime(){

	return image_preparation_time;
}

void CNexCodecManager::setImagePreparationTime(int time){

	image_preparation_time = time;
}

int CNexCodecManager::isVideoLayerAvailable(){

	if( m_pThis == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Codec manager handle is null", __LINE__);
		return 0;
	}
	
	if( m_pThis->m_hCAL == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[CodecManager.cpp %d] Cal handle is null", __LINE__);
		return 0;
	}
	if(m_pThis->m_iHardwareDecodeMaxCount > 2)
		return 1;
	return 0;
}
