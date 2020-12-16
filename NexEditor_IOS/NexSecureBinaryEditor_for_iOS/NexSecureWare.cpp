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

#include "NexSecureWare.h"
#include "NexBase64.h"
#include <memory.h>
#include <stdio.h>
#include <string.h>
#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif

#define MAJOR_VERSION	1
#define MINOR_VERSION	0

#define TRACE printf

static NEXSECUREINFO*	g_pSecureInfo = 0x00;

NEXSECURE_RETURN NexSecure_Create(int nMajorVersion, int nMinorVersion)
{
	if(	nMajorVersion != MAJOR_VERSION || nMinorVersion != MINOR_VERSION )
	{
		return NEXSECURE_ERROR_VERSION_MISMATCH;
	}
	g_pSecureInfo = new NEXSECUREINFO;
	if( g_pSecureInfo == 0x00 ) return NEXSECURE_ERROR_CREATE_FAIL;
	memset((void*)g_pSecureInfo, 0x00, sizeof(NEXSECUREINFO));
	return NEXSECURE_ERROR_NONE;
}

NEXSECURE_RETURN NexSecure_Destroy()
{
	if( g_pSecureInfo )
	{
		delete g_pSecureInfo;
		g_pSecureInfo = 0x00;
	}
	return NEXSECURE_ERROR_NONE;
}

NEXSECURE_RETURN NexSecure_GetInfo(NEXSECUREINFO *pSecureInfo)
{
	if( g_pSecureInfo == 0x00 ) return NEXSECURE_ERROR_NOT_CREATE;
	if( pSecureInfo ==  0x00 ) return NEXSECURE_ERROR;

	memcpy(pSecureInfo, g_pSecureInfo, sizeof(NEXSECUREINFO));
	return NEXSECURE_ERROR_NONE;
}

NEXSECURE_RETURN NexSecure_SetInfo(NEXSECUREINFO *pSecureInfo)
{
	if( g_pSecureInfo == 0x00 ) return NEXSECURE_ERROR_NOT_CREATE;
	if( pSecureInfo ==  0x00 ) return NEXSECURE_ERROR;

	memcpy(g_pSecureInfo, pSecureInfo, sizeof(NEXSECUREINFO));
	return NEXSECURE_ERROR_NONE;
}

NEXSECURE_RETURN NexSecure_GetBitsData(unsigned char* pBuff, int nBuffLen, int* pOutLen)
{
	if( g_pSecureInfo == 0x00 ) return NEXSECURE_ERROR_NOT_CREATE;
	if( pOutLen == 0x00 ) return NEXSECURE_ERROR;

	int nLen = 0;
	long infoSize = NexSecure_GetInfoSize();
	int valueSize = 0;
    int i = 0;

	unsigned char* pIn = new unsigned char[infoSize];

	if( pIn == 0x00 ) return NEXSECURE_ERROR;
	memset(pIn, 0x00, infoSize);
	
	memcpy(pIn, g_pSecureInfo->m_strKey, 24);
	pIn += 24;

	if(strcmp(g_pSecureInfo->m_strKey, "NEXTREAMING.COM_V2") == 0)
	{
		valueSize = ITEM_VALUE_MAX_LEN_V2;
	}
	else if(strcmp(g_pSecureInfo->m_strKey, "NEXTREAMING.COM_V3") == 0)
	{
		valueSize = ITEM_VALUE_MAX_LEN_V3;
	}
	else
	{
		valueSize = ITEM_VALUE_MAX_LEN;
	}

	memcpy(pIn, &(g_pSecureInfo->m_uiSecureItemCount), 4);
	pIn += 4;

	for(i = 0; i < g_pSecureInfo->m_uiSecureItemCount ; i++)
	{
		if((g_pSecureInfo->m_SecureItem)[i].m_strItemName != NULL)
			memcpy(pIn, (g_pSecureInfo->m_SecureItem)[i].m_strItemName, ITEM_NAME_MAX_LEN);

        pIn += ITEM_NAME_MAX_LEN;

		if((g_pSecureInfo->m_SecureItem)[i].m_strItemValue != NULL)
			memcpy(pIn, (g_pSecureInfo->m_SecureItem)[i].m_strItemValue, valueSize);

        pIn += valueSize;
	}
	//set cursor position of 'pIn' to the front of buffer to free data
	pIn -= i*(ITEM_NAME_MAX_LEN+valueSize)+28;
	//memcpy(pIn, g_pSecureInfo, infoSize);

	nLen = BitsToBase64(pBuff, nBuffLen, pIn, infoSize);
	if( nLen == 0 )
	{
		delete [] pIn;
		*pOutLen = nLen;
		return NEXSECURE_ERROR;
	}
	delete [] pIn;
	*pOutLen = nLen;
	return NEXSECURE_ERROR_NONE;
}

NEXSECURE_RETURN NexSecure_SetBitsData(unsigned char* pBuff, int nBuffLen)
{
	if( g_pSecureInfo == 0x00 ) return NEXSECURE_ERROR_NOT_CREATE;

	unsigned char* pOut = new unsigned char[nBuffLen];
	
	if( pOut == 0x00 ) return NEXSECURE_ERROR;

	int nByte = Base64ToBits(pOut, nBuffLen, pBuff, nBuffLen);

    TRACE("NexSecure_SetBitsData nByte : %d\n", nByte);
	
	// kyle.jung_130722-start
	// set item name and value to dynamic memory area to read/write recent version of engine library file
#if 0
	if( nByte != sizeof(NEXSECUREINFO) )
	{
		delete [] pOut;
		return NEXSECURE_ERROR;
	}

	memcpy(g_pSecureInfo, pOut, infoSize);
#endif

	//value to check cursor position of pOut.
	long pOutPointer = 0;

	int valueSize = 0;
	int maxItemNum = 0;

	//get key value
	memcpy(g_pSecureInfo->m_strKey, pOut, 24);
	pOut += 24;

	//get item num
	memcpy(&(g_pSecureInfo->m_uiSecureItemCount), pOut, 4);
	pOut += 4;

	int i = 0;
	long infoSize = NexSecure_GetInfoSize();	
	int nameSize = ITEM_NAME_MAX_LEN;

	//add new case when version changes
	if(strcmp(g_pSecureInfo->m_strKey, "NEXTREAMING.COM_V2") == 0)
	{
		valueSize = ITEM_VALUE_MAX_LEN_V2;
	}
	else if(strcmp(g_pSecureInfo->m_strKey, "NEXTREAMING.COM_V3") == 0)
	{
		valueSize = ITEM_VALUE_MAX_LEN_V3;
	}
	else
	{
		valueSize = ITEM_VALUE_MAX_LEN;
	}

	g_pSecureInfo->m_SecureItem = (NEXSECUREITEM*)malloc(infoSize);
	memset(g_pSecureInfo->m_SecureItem, 0x00, infoSize);

	for(i = 0; i < g_pSecureInfo->m_uiSecureItemCount; i++)
	{
		memcpy((g_pSecureInfo->m_SecureItem)[i].m_strItemName, pOut, nameSize);
		pOut += nameSize;

		(g_pSecureInfo->m_SecureItem)[i].m_strItemValue = new char[valueSize];
		memcpy((g_pSecureInfo->m_SecureItem)[i].m_strItemValue, (char*) pOut, valueSize);
		pOut += valueSize;
	}
	//set cursor position of 'pOut' to the front of buffer to free data
	pOut -= i*(nameSize+valueSize)+28;
	// kyle.jung_130722-end
	
	delete [] pOut;
	return NEXSECURE_ERROR_NONE;
}

long NexSecure_GetInfoSize()
{
	long infoSize = 0;

	//add new case when version changes
	if(strcmp(g_pSecureInfo->m_strKey, "NEXTREAMING.COM_V2") == 0)
	{
		infoSize = (ITEM_NAME_MAX_LEN + ITEM_VALUE_MAX_LEN_V2) * ITEM_MAX_COUNT_V2 + 28;
	}
	else if(strcmp(g_pSecureInfo->m_strKey, "NEXTREAMING.COM_V3") == 0)
	{
		infoSize = (ITEM_NAME_MAX_LEN + ITEM_VALUE_MAX_LEN_V3) * ITEM_MAX_COUNT_V3 + 28;
	}
	else
	{
		infoSize = (ITEM_NAME_MAX_LEN + ITEM_VALUE_MAX_LEN) * ITEM_MAX_COUNT + 28;
	}

	return infoSize;
}

