//=============================================================================
// Nextreaming Confidential Proprietary
// Copyright (C) 2005-2008 Nextreaming Corporation
// All rights are reserved by Nextreaming Corporation
//
// File       : NexSecureWare.h
// Description: 
//
// Revision History:
// Version   Date         Author			Description of Changes
//-----------------------------------------------------------------------------
// 1.0.0     2008/09/11   Kye-Beom Song		Draft
//=============================================================================

#ifndef _NEXSECUREWARE_H_
#define _NEXSECUREWARE_H_

#define NEXSECURE_MAJOR_VERSION	1
#define NEXSECURE_MINOR_VERSION	0

#define SECURE_KEY			"NEXTREAMING.COM"
#define SECURE_KEY_V2		"NEXTREAMING.COM_V2"
#define SECURE_KEY_V3		"NEXTREAMING.COM_V3"

#define ITEM_MAX_COUNT		48	
#define ITEM_NAME_MAX_LEN	12
#define ITEM_VALUE_MAX_LEN 24	

#define ITEM_MAX_COUNT_V2 100		// kyle.jung_130719_update max item count
#define ITEM_VALUE_MAX_LEN_V2 72		// kyle.jung_130719_update max item value length

#define ITEM_MAX_COUNT_V3 200		// kyle.jung_131025_update max item count
#define ITEM_VALUE_MAX_LEN_V3 72		// kyle.jung_131025_update max item value length

typedef enum 
{
	NEXSECURE_ERROR_NONE	= 0,
	NEXSECURE_ERROR,
	NEXSECURE_ERROR_VERSION_MISMATCH,
	NEXSECURE_ERROR_CREATE_FAIL,
    NEXSECURE_ERROR_NOT_CREATE
}NEXSECURE_RETURN;

typedef struct _NEXSECUREITEM
{
	char	m_strItemName[ITEM_NAME_MAX_LEN];
	char*	m_strItemValue;
	//char	m_strItemValue[ITEM_VALUE_MAX_LEN];
}NEXSECUREITEM;

typedef struct _NEXSECUREINFO
{
	char			m_strKey[24];	// kyle.jung_130719
	unsigned int	m_uiSecureItemCount;
	NEXSECUREITEM*	m_SecureItem; // 240
}NEXSECUREINFO;

NEXSECURE_RETURN NexSecure_Create(int nMajorVersion, int nMinorVersion);
NEXSECURE_RETURN NexSecure_Destroy();
NEXSECURE_RETURN NexSecure_GetInfo(NEXSECUREINFO *pSecureInfo);
NEXSECURE_RETURN NexSecure_SetInfo(NEXSECUREINFO *pSecureInfo);
NEXSECURE_RETURN NexSecure_GetBitsData(unsigned char* pBuff, int nBuffLen, int* pOutLen);
NEXSECURE_RETURN NexSecure_SetBitsData(unsigned char* pBuff, int nBuffLen);
long NexSecure_GetInfoSize();

#endif
