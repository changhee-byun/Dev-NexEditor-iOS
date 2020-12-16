/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_Util.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#include "stdlib.h"
#include "string.h"
#include "NexSAL_Internal.h"
#include "NEXVIDEOEDITOR_Util.h"

int getAACProfile(unsigned char* pBuf, int iSize, int* piAACProfile)
{
	*piAACProfile = -1;

	unsigned char* pBuffer = pBuf;
	unsigned int uiSize = iSize;

	if( pBuffer == NULL || uiSize == 0)
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_Util.cpp %d]getAACProfile invalid parameter!!",__LINE__);
		return -1;
	}

	if( (pBuffer[0] & 0xFF) == 0xFF && (pBuffer[1] & 0xF6) == 0xF0 )
	{
		if( iSize < 3)
		{
			return -1;
		}
		else
		{
			*piAACProfile = (pBuffer[2]>>6)&0x03;
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_Util.cpp %d]getAACProfile ADTS Profile(%d)",__LINE__, *piAACProfile);
		}
	}
	else
	{
		if(((pBuffer[0] & 0xF8) >> 3) != 5 && ((pBuffer[0] & 0xF8) >> 3) != 29) 
		{
			*piAACProfile = (((pBuffer[0]  & 0xF8) >> 3) - 1);
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[NEXVIDEOEDITOR_Util.cpp %d]getAACProfile DSI Profile(%d)",__LINE__, *piAACProfile);
		}
	}
	return 0;
}

IMAGE_TYPE getImageType(char* pFile)
{
	if( pFile == NULL )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] Wrong file path", __LINE__);
		return NOT_IMAGE;
	}

	if( pFile[0] == '@'	 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] Iw wsa RGB file", __LINE__);
		return IMAGE_RGB;
	}

#if defined(__APPLE__)
    if(strstr(pFile, "phasset-image://"))
    {
        return IMAGE_JPEG_IOS;
    }
#endif
    
	// unsigned char JPEG_HEADER[] = {0xFF, 0xD8, 0xFF, 0xE0};
	unsigned char JPEG_HEADER[] = {0xFF, 0xD8, 0xFF};
	unsigned char PNG_HEADER[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

	unsigned char SVG1_HEADER[] = {0x3C, 0x73, 0x76, 0x67};
	unsigned char SVG2_HEADER[] = {0x3C, 0x53, 0x56, 0x47};

	unsigned char WEBP_HEADER[] = {'R', 'I', 'F', 'F', 'W', 'E', 'B', 'P'};
	
	unsigned char GIF_HEADER1[] = {0x47, 0x49, 0x46, 0x38, 0x37, 0x61};
	unsigned char GIF_HEADER2[] = {0x47, 0x49, 0x46, 0x38, 0x39, 0x61};

	unsigned char BMP_HEADER[] = {0x42, 0x4D};

	unsigned char HEIF_HEADER1[] = {0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63};
	unsigned char HEIF_HEADER2[] = {0x66, 0x74, 0x79, 0x70, 0x6D, 0x69, 0x66, 0x31};

	unsigned char pTmpBuffer[16];
	NEXSALFileHandle	hFile = nexSAL_FileOpen(pFile, (NEXSALFileMode)NEXSAL_FILE_READ);
	if( hFile == NEXSAL_INVALID_HANDLE )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] open file failed", __LINE__);
		return NOT_IMAGE;
	}

	int nRead = nexSAL_FileRead(hFile, pTmpBuffer, sizeof(pTmpBuffer));
	if( nRead != sizeof(pTmpBuffer) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] read file failed filename:%s", __LINE__, pFile);
		nexSAL_FileClose(hFile);
		return NOT_IMAGE;
	}
	nexSAL_FileClose(hFile);
    if (nexSAL_TraceCondition(NEX_TRACE_CATEGORY_FLOW, 1)) {
	nexSAL_MemDump(pTmpBuffer, sizeof(pTmpBuffer));
    }

	if( memcmp(pTmpBuffer, JPEG_HEADER, sizeof(JPEG_HEADER)) == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was JEPG File", __LINE__);
		return IMAGE_JPEG;
	}
	else if( memcmp(pTmpBuffer, PNG_HEADER, sizeof(PNG_HEADER)) == 0 )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was PNG File", __LINE__);
		return IMAGE_PNG;
	}
	else if( (memcmp(pTmpBuffer, SVG1_HEADER, sizeof(SVG1_HEADER))== 0) || (memcmp(pTmpBuffer, SVG2_HEADER, sizeof(SVG2_HEADER)) == 0 ) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was SVG File", __LINE__);
		return IMAGE_SVG;
	}
	else if( (memcmp(pTmpBuffer, WEBP_HEADER, 4) == 0) && (memcmp(pTmpBuffer+8, WEBP_HEADER + 4, 4) == 0 ) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was WEBP File", __LINE__);
		return IMAGE_WEBP;
	}
	else if( (memcmp(pTmpBuffer, GIF_HEADER1, 6) == 0) || (memcmp(pTmpBuffer, GIF_HEADER2, 6) == 0 ) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was GIF File", __LINE__);
		return IMAGE_GIF;
	}
	else if( (memcmp(pTmpBuffer, BMP_HEADER, 2) == 0) )
	{
		nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was BMP File", __LINE__);
		return IMAGE_BMP;
	}
	else if( memcmp(pTmpBuffer+4, HEIF_HEADER1, 4) == 0 )
	{
		if ( memcmp(pTmpBuffer+8, HEIF_HEADER1 + 4, 4) == 0 || memcmp(pTmpBuffer+8, HEIF_HEADER2 + 4, 4) == 0 )
		{
			nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was HEIF File", __LINE__);
			return IMAGE_HEIF;
		}
	}
	nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO, 0, "[NEXVIDEOEDITOR_Util.cpp %d] It was not image File", __LINE__);
	return NOT_IMAGE;
}

int findString(unsigned char *src_str, int src_size ,unsigned char *find_str , int find_size )
{
    unsigned char *pstr =  src_str;

    int i = 0 ;
    int hit_count = 0;
    
    if(src_size < find_size)
    {
        return -1;
    }    
    
    int case_diff =  'a' - 'A';
    
    for( i = 0 ; i < src_size ; i++ ){
        unsigned char c = *pstr;
        unsigned char f = find_str[hit_count];
        
        if( c >= 'A' && c <= 'Z' ){
            c += case_diff;
        }
        

        if( f >= 'A' && f <= 'Z' ){
            f += case_diff;
        }

        if( c == f ){
            hit_count++;
//            printf("[%d]hitCount=%d\n",i,hit_count);
            if( hit_count == find_size ){
                return (i - hit_count)+1;
            }
        }
        else
        {
            hit_count = 0;
        }
        pstr++;
    }
    return -1;
}

