/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_SupportDevices.h
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2012/07/6	Draft.
-----------------------------------------------------------------------------*/

//---------------------------------------------------------------------------
#ifndef _NEXVIDEOEDITOR_SUPPORTDEVICES__H_
#define _NEXVIDEOEDITOR_SUPPORTDEVICES__H_
//---------------------------------------------------------------------------

#include "NexSAL_Internal.h"

#define CHIPSET_UNKNOWN		0x00000000

#define CHIPSET_MSM8960		0x00000001
#define CHIPSET_MSM8064		0x00000002
#define CHIPSET_MSM8974		0x00000003
#define CHIPSET_MSM8226		0x00000004
#define CHIPSET_MSM8926		0x00000005

#define CHIPSET_EXYNOS4412		0x00000100
#define CHIPSET_EXYNOS5410		0x00000200

#define CHIPSET_MTK6589		0x00010000
#define CHIPSET_MTK6582		0x00020000
#define CHIPSET_MTK6592		0x00030000

typedef struct _SUPPORT_DEVICE_INFO
{
	unsigned int	m_uiModelCode;

	int			m_iSupportWidth;
	int			m_iSupportHeight;
	int			m_iSupportH264Profile;

	int			m_iSupportDecoderMaxCnt;
	int			m_iSupportEncoderMaxCnt;

	int			m_iUseSurfaceMediaSource;
}SUPPORT_DEVICE_INFO;

SUPPORT_DEVICE_INFO* getSupportDeviceInfo(char* pModelName);
int getRGBADeviceProperty();
int getCheckChipsetType();
int getSupportSystemProperty(const char* pStrName, char* pValue);

int isLGEG2Device();
int isMT6592Device();
int isMSM8974Device();
int isGalaxyNote3();
int isXperiaM4AquaOrGalaxyA7();
int isSamsungMSM8974Device();
int isQualcommChip();
int isMSM8x26Device();
int isNexus10Device();
int isx86Device();
int isHuaweiHi3635Device();
int isHuaweiHi3630Device();
int isSDM660Device();
int isXiaomiMI5C();
//---------------------------------------------------------------------------
#endif	// _NEXVIDEOEDITOR_SUPPORTDEVICES__H_
