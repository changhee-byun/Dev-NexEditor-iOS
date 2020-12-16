#include "../nexCalBody/nexCalBody.h"
#include "NexCAL.h"
#include "NexMediaDef.h"
#include "nexCalBody_JPEG.h"
#include <android/log.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "NxJPEGDecAPI.h"

#ifdef __cplusplus
}
#endif

#define JPEG_MAX_WIDTH		1440
#define JPEG_MAX_HEIGHT	810
#define JPEG_MAX_SIZE		1500000

// #define JPEG_DEC_RESULT_DUMP

unsigned int nexCALBody_Image_JPEG_Rotate(				unsigned char* pBits1, unsigned char* pBits2, unsigned char* pBits3, 
														unsigned char* pSrc1, unsigned char* pSrc2, unsigned char* pSrc3,
														int* piWidth, int* piHeight, int* piPitch, int rotate)
{

	int iWidth = *piWidth;
	int iHeight = *piHeight;
	int iPitch = *piPitch;
	int iHalfWidth = iWidth>>1;
	int iHalfHeight = iHeight>>1;
	int iHalfPitch = iPitch>>1;
	unsigned char* pSrc;
	unsigned char* pDest;
	int iHalfIndex = 0;
	
	if (rotate == 90)
	{
		for( int i = 0; i < iHeight ; i++)
		{
			pSrc = pSrc1 + i*iPitch;
			pDest = pBits1 +  (iHeight-i)-1;
			for( int j = 0; j < iWidth ; j++)
			{
				*(pDest + iHeight*j) = *(pSrc++);
			}
			if(!( i & 1))
			{
				iHalfIndex = i>>1;
				pSrc = pSrc2 +( iHalfIndex*iHalfPitch);
				pDest = pBits2 +(iHalfHeight-iHalfIndex)-1;
				for( int j = 0; j < iHalfWidth ; j++)
				{
					*(pDest + iHalfHeight*j) = *(pSrc++);
				}

				pSrc = pSrc3 +( iHalfIndex*iHalfPitch);
				pDest = pBits3 +(iHalfHeight-iHalfIndex)-1;
				for( int j = 0; j < iHalfWidth ; j++)
				{
					*(pDest + iHalfHeight*j) = *(pSrc++);
				}				
			}
		}
		*piWidth = iHeight;
		*piPitch = iHeight;
		*piHeight = iWidth;
		
	}
	else if( rotate == 180)
	{
		for( int i = 0; i < iHeight; i++)
		{
			pDest = pBits1 + (iPitch*i);
			pSrc = pSrc1 + (iPitch*(iHeight-i));
			memcpy(pDest, pSrc, iPitch);

			if(!( i & 1))
			{
				iHalfIndex = i>>1;
				pDest = pBits2 + (iHalfPitch*iHalfIndex);
				pSrc = pSrc2 + (iHalfPitch*(iHalfHeight-iHalfIndex));
				memcpy(pDest, pSrc, iHalfPitch);
				pDest = pBits3 + (iHalfPitch*iHalfIndex);
				pSrc = pSrc3 + (iHalfPitch*(iHalfHeight-iHalfIndex));
				memcpy(pDest, pSrc, iHalfPitch);
			}
		}

	}
	else if( rotate == 270)
	{
		for( int i = 0; i < iHeight ; i++)
		{
			pSrc = pSrc1 + i*iPitch;
			pDest = pBits1 + (iWidth-1)*iHeight + i;
			for( int j = 0; j < iWidth ; j++)
			{
				*(pDest - iHeight*j) = *(pSrc++);
			}
			if(!( i & 1))
			{
				iHalfIndex = i>>1;				
				pSrc = pSrc2 +( iHalfIndex*iHalfPitch);
				pDest = pBits2 + (iHalfWidth-1)*iHalfHeight + iHalfIndex;
				for( int j = 0; j < iHalfWidth ; j++)
				{
					*(pDest - iHalfHeight*j) = *(pSrc++);
				}

				pSrc = pSrc3 +(iHalfIndex*iHalfPitch);
				pDest = pBits3 + (iHalfWidth-1)*iHalfHeight + iHalfIndex;
				for( int j = 0; j < iHalfWidth ; j++)
				{
					*(pDest -iHalfHeight*j) = *(pSrc++);
				}				
			}
			
		}
		*piWidth = iHeight;
		*piPitch = iHeight;
		*piHeight = iWidth;
		
	}
	else
	{
		memcpy(pBits1, pSrc1, iPitch*iHeight);
		memcpy(pBits1, pSrc1, (iPitch*iHeight)>>2);
		memcpy(pBits1, pSrc1, (iPitch*iHeight)>>2);
	}
}
NXINT32 nexCALBody_Image_JPEG_GetProperty( NXUINT32 a_uProperty, NXINT64 *a_pqValue, NXVOID *a_pUserData )
{
	return 0;
}
 
NXINT32 nexCALBody_Image_JPEG_SetProperty( NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData )
{
	return 0;
}

NXINT32 nexCALBody_Image_JPEG_getHeader(	NEX_CODEC_TYPE a_eCodecType, 
														NXCHAR *a_strFileName, 
														NXUINT8 *a_pSrc, 
														NXUINT32 a_uSrcLen, 
														NXINT32 *a_pnWidth, 
														NXINT32 *a_pnHeight, 
														NXINT32 *a_pnPitch, 
														NXUINT32 a_uUserDataType, 
														NXVOID **a_ppUserData)
{
	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder version %s\n", __LINE__, NxJPEGDecGetVersionInfo(1));
	nexCAL_DebugLogPrintf("JPEG getHeader : pFile(%s) pSrc(0x%x) uSrcLen(%d)", a_strFileName == NULL ? "" : a_strFileName, a_pSrc, a_uSrcLen);

	if( a_eCodecType != eNEX_CODEC_V_JPEG )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder get header param failed %s\n", __LINE__);
		return 1;
	}

	if( a_strFileName == NULL && (a_pSrc == NULL || a_uSrcLen <= 0 ) )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder get header param failed %s\n", __LINE__);
		return 1;
	}

	if( a_pnWidth == NULL || a_pnHeight == NULL || a_pnPitch == NULL || a_ppUserData == NULL )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder get header param failed %s\n", __LINE__);
		return 1;
	}

	int iScaleWidth	= JPEG_MAX_WIDTH;
	int iScaleHeight	= JPEG_MAX_HEIGHT;
	int iScaleSize	= JPEG_MAX_SIZE;
	
	unsigned int		uiRet		= 0;
	unsigned char*	pTempBuffer	= NULL;

	NxJPEGDecParam	nx_jpeg[1];
	NxImgInputStream  nx_in[1];
	NxJPEGDecExif Ext;

	memset(&Ext, 0, sizeof(NxJPEGDecExif));
	memset(nx_in, 0x00, sizeof(NxImgInputStream));

	if( *a_pnWidth != 0 && *a_pnHeight != 0 && *a_pnPitch != 0 )
	{
		iScaleWidth	= *a_pnWidth;
		iScaleHeight	= *a_pnHeight;
		iScaleSize	= *a_pnPitch;

		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader (%d %d %d)", __LINE__, iScaleWidth, iScaleHeight, iScaleSize);
	}

	if( a_strFileName )
	{
		FILE* pFile = fopen(a_strFileName, "rb");
		if( pFile )
		{
			unsigned int uiTail = fseek(pFile, 0, SEEK_END);
			uiTail = ftell(pFile);
			pTempBuffer = (unsigned char*)nexCAL_MemAlloc(uiTail);
			if( pTempBuffer == NULL )
			{
				nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader failed because tmp buffer can not alloc(size:%d)", __LINE__, uiTail);
				fclose(pFile);
				return 1;
			}
			fseek(pFile, 0, SEEK_SET);

			unsigned int uiRead = fread(pTempBuffer, sizeof(unsigned char), uiTail, pFile);
			if( uiRead != uiTail )
			{
				nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader failed because file can not read ", __LINE__);
				nexCAL_MemFree(pTempBuffer);
				fclose(pFile);
				return 1;
			}
			nx_in->file_ptr = pTempBuffer;
			nx_in->file_size = uiRead;
			fclose(pFile);
		}
	}
	else if( a_pSrc && a_uSrcLen > 0 )
	{
		nx_in->file_ptr		= a_pSrc;
		nx_in->file_size		= a_uSrcLen;
	}

	if( nx_in->file_ptr == NULL || nx_in->file_size <= 0 )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader failed", __LINE__);
		return 1;
	}

	NxJPEGDec_initParam(nx_jpeg);
	uiRet = NxJPEGDec_getHeader(nx_in, nx_jpeg, 0, 0);

	if( uiRet )
	{
		if( pTempBuffer )
		{
			nexCAL_MemFree(pTempBuffer);
		}
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader failed", __LINE__);
		return 1;
	}

	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader(%d %d %d)", __LINE__, nx_jpeg->output_width, nx_jpeg->output_height, nx_jpeg->output_y_pitch);	

	int sampleSize = 1;
	while( 	sampleSize < 8 && 
			(( nx_jpeg->output_width / sampleSize > iScaleWidth && nx_jpeg->output_height / sampleSize > iScaleHeight) || 
			(( nx_jpeg->output_width / sampleSize * nx_jpeg->output_height / sampleSize > iScaleSize) )))
	{
		sampleSize *= 2;
	}	
	/*
	if( sampleSize > nx_jpeg->uMaxSamplingFactor )
	{
		if( pTempBuffer )
		{
			nexCAL_MemFree(pTempBuffer);
		}

		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader check scale failed(%d %d %d)", __LINE__,
			nx_jpeg->uSamplingFactor[0], nx_jpeg->uSamplingFactor[1], nx_jpeg->uSamplingFactor[2]);
		
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader scale factor can't use(%d %d)", __LINE__,
			sampleSize, nx_jpeg->uMaxSamplingFactor);
		return 1;
	}

	bool bCheckScale = false;
	for(int i = 0; i < 3; i++ )
	{
		if(  sampleSize == nx_jpeg->uSamplingFactor[i] )
		{
			bCheckScale = true;
			break;
		}
	}
	
	if( bCheckScale == FALSE )
	{
		if( pTempBuffer )
		{
			nexCAL_MemFree(pTempBuffer);
		}
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader check scale failed(%d %d %d)", __LINE__,
			nx_jpeg->uSamplingFactor[0], nx_jpeg->uSamplingFactor[1], nx_jpeg->uSamplingFactor[2]);
		return 1;
	}
	*/

	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader downsize(%d)", __LINE__, sampleSize);
	NxJPEGDec_initParam(nx_jpeg);
	nx_jpeg->downsize_coeff = sampleSize;

	Ext.str_buf = (char*)nexCAL_MemAlloc(4096);
	Ext.str_buf_length = 4096;
	nx_jpeg->bRotate = 1; // adjust orientation to output image.
	if( Ext.str_buf != NULL)
	{
		uiRet = NxJPEGDec_getHeader(nx_in, nx_jpeg, 0, &Ext);
		nexCAL_MemFree(Ext.str_buf);
	}
	else
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == Memory Alloc Fail!", __LINE__);
		uiRet = NxJPEGDec_getHeader(nx_in, nx_jpeg, 0, 0);
	}

	if( uiRet )
	{
		if( pTempBuffer )
		{
			nexCAL_MemFree(pTempBuffer);
		}
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader failed(%d)", __LINE__, uiRet);
		return 1;
	}

	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader(%d %d %d)", __LINE__, nx_jpeg->output_width, nx_jpeg->output_height, nx_jpeg->output_y_pitch);	
	
	*a_pnWidth	= nx_jpeg->output_width;
	*a_pnHeight	= nx_jpeg->output_height;
	*a_pnPitch		= nx_jpeg->output_y_pitch;

	if( pTempBuffer )
	{
		nexCAL_MemFree(pTempBuffer);
	}
	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader Sucessed(%d %d %d)", __LINE__, *a_pnWidth, *a_pnHeight, *a_pnPitch);
	return 0;
}

NXINT32 nexCALBody_Image_JPEG_Decoder(		NXCHAR *a_strFileName, 
														NXUINT8 *a_pSrc, 
														NXUINT32 a_uSrcLen, 
														NXINT32 *a_pnWidth, 
														NXINT32 *a_pnHeight, 
														NXINT32 *a_pnPitch, 
														NXUINT8 *a_pBits1, 
														NXUINT8 *a_pBits2, 
														NXUINT8 *a_pBits3, 
														NXUINT32 *a_puDecodeResult, 
														NXVOID *a_pUserData)
{
	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder version %s\n", __LINE__, NxJPEGDecGetVersionInfo(1));
	nexCAL_DebugLogPrintf("JPEG getHeader : pFile(%s) pSrc(0x%x) uSrcLen(%d)", a_strFileName == NULL ? "" : a_strFileName, a_pSrc, a_uSrcLen);

	if( a_strFileName == NULL && (a_pSrc == NULL || a_uSrcLen <= 0 ) )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder get decode param failed %s\n", __LINE__);
		return 1;
	}

	if( a_pnWidth == NULL || a_pnHeight == NULL || a_pnPitch == NULL )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder get decode param failed %s\n", __LINE__);
		return 1;
	}

	if( a_pBits1 == NULL || a_pBits2 == NULL || a_pBits3 == NULL )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] JPEG Decoder get decode param failed %s\n", __LINE__);
		return 1;
	}

	int iScaleWidth	= JPEG_MAX_WIDTH;
	int iScaleHeight	= JPEG_MAX_HEIGHT;
	int iScaleSize	= JPEG_MAX_SIZE;

	unsigned int		uiRet			= 0;
	unsigned char*	pTempBuffer	= NULL;

	NxJPEGDecParam	nx_jpeg[1];
	NxImgInputStream  nx_in[1];
	NxJPEGDecExif Ext;
	int iInitParamRet = 0;

	memset(&Ext, 0, sizeof(NxJPEGDecExif));
	memset(nx_in, 0x00, sizeof(NxImgInputStream));

	if( *a_pnWidth != 0 && *a_pnHeight != 0 && *a_pnPitch != 0 )
	{
		iScaleWidth	= *a_pnWidth;
		iScaleHeight	= *a_pnHeight;
		iScaleSize	= *a_pnPitch;
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader (%d %d %d)", __LINE__, iScaleWidth, iScaleHeight, iScaleSize);
	}

	if( a_strFileName )
	{
		FILE* pFile = fopen(a_strFileName, "rb");
		if( pFile )
		{
			unsigned int uiTail = fseek(pFile, 0, SEEK_END);
			uiTail = ftell(pFile);
			pTempBuffer = (unsigned char*)nexCAL_MemAlloc(uiTail);
			if( pTempBuffer == NULL )
			{
				nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_decode failed because tmp buffer can not alloc ", __LINE__);
				fclose(pFile);
				return 1;
			}
			fseek(pFile, 0, SEEK_SET);

			unsigned int uiRead = fread(pTempBuffer, sizeof(unsigned char), uiTail, pFile);
			if( uiRead != uiTail )
			{
				nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_decode failed because file can not read ", __LINE__);
				nexCAL_MemFree(pTempBuffer);
				fclose(pFile);
				return 1;
			}
			nx_in->file_ptr = pTempBuffer;
			nx_in->file_size = uiRead;
			fclose(pFile);
		}
	}
	else if( a_pSrc && a_uSrcLen > 0 )
	{
		nx_in->file_ptr	= a_pSrc;
		nx_in->file_size	= a_uSrcLen;
	}

	if( nx_in->file_ptr == NULL || nx_in->file_size <= 0 )
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_decode failed", __LINE__);
		return 1;
	}

	iInitParamRet = NxJPEGDec_initParam(nx_jpeg);
	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == NxJPEGDec_initParam Ret (%d)", __LINE__, iInitParamRet);
	
	uiRet = NxJPEGDec_getHeader(nx_in, nx_jpeg, 0, 0);

	if( uiRet )
	{
		if( pTempBuffer )
		{
			nexCAL_MemFree(pTempBuffer);
		}
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_decode failed", __LINE__);
		return 1;
	}

	int sampleSize = 1;
	while( 	sampleSize < 8 && 
			(( nx_jpeg->output_width / sampleSize > iScaleWidth && nx_jpeg->output_height / sampleSize > iScaleHeight) || 
			(( nx_jpeg->output_width / sampleSize * nx_jpeg->output_height / sampleSize > iScaleSize) )))
	{
		sampleSize *= 2;
	}	

	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader downsize(%d)", __LINE__, sampleSize);
	NxJPEGDec_initParam(nx_jpeg);
	nx_jpeg->downsize_coeff = sampleSize;

	Ext.str_buf = (char*)nexCAL_MemAlloc(4096);
	Ext.str_buf_length = 4096;
	nx_jpeg->bRotate = 1; // adjust orientation to output image.
	if( Ext.str_buf != NULL)
	{
		uiRet = NxJPEGDec_getHeader(nx_in, nx_jpeg, 0, &Ext);
		nexCAL_MemFree(Ext.str_buf);
	}
	else
	{
		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == Memory Alloc Fail!", __LINE__);
		uiRet = NxJPEGDec_getHeader(nx_in, nx_jpeg, 0, 0);
	}
	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader(%d %d %d)Oritentation:%d", __LINE__, nx_jpeg->output_width, nx_jpeg->output_height, nx_jpeg->output_y_pitch, Ext.orientation);

	nx_jpeg->length_buffer_y = nx_jpeg->output_y_pitch * nx_jpeg->output_height * 3 /2;
	nx_jpeg->length_buffer_uv = nx_jpeg->length_buffer_y >> 1;				

	unsigned int uiMemBufferSize = nx_jpeg->output_y_pitch * nx_jpeg->output_height * 3 /2;//((nx_jpeg->output_y_pitch + 7)>>3)*64*3;
	unsigned short* pMemBuffer = (unsigned short*)nexCAL_MemAlloc(sizeof(unsigned short)*uiMemBufferSize);

	nx_jpeg->buffer_y = a_pBits1;
	nx_jpeg->buffer_u = a_pBits2;
	nx_jpeg->buffer_v = a_pBits3;
	
	nx_jpeg->buffer_temp = pMemBuffer;
	nx_jpeg->length_buffer_temp = uiMemBufferSize;
	nx_jpeg->output_image_format = FORMAT_YUV_420;
	nx_jpeg->downsize_coeff = sampleSize;

	uiRet = NxJPEGDec_decode(nx_in, nx_jpeg, 0, 0);

	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_getHeader(%d %d %d)", __LINE__, nx_jpeg->output_width, nx_jpeg->output_height, nx_jpeg->output_y_pitch);	

	if( uiRet == 0 ) 
	{
		*a_puDecodeResult = 0;

		*a_pnWidth	= nx_jpeg->output_width;
		*a_pnHeight	= nx_jpeg->output_height;
		*a_pnPitch	= nx_jpeg->output_y_pitch;

		if( pTempBuffer )
			nexCAL_MemFree(pTempBuffer);
		if( pMemBuffer )
			nexCAL_MemFree(pMemBuffer);

#ifdef JPEG_DEC_RESULT_DUMP
		FILE* pFile = fopen("/sdcard/nexjpeg_dec.yuv", "wb");
		if( pFile )
		{
			fwrite(pBits1,sizeof(unsigned char),(*a_pnPitch)*(*a_pnHeight),pFile);
			fwrite(pBits2,sizeof(unsigned char),(*a_pnPitch)*(*a_pnHeight)/4,pFile);
			fwrite(pBits3,sizeof(unsigned char),(*a_pnPitch)*(*a_pnHeight)/4,pFile);
			
			fclose(pFile);
		}
#endif

		nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_decode Sucessed(%d %d %d)", __LINE__, *a_pnWidth, *a_pnHeight, *a_pnPitch);
		return 0;
	}

	NxJPEGDecClose(nx_jpeg);

	if( pTempBuffer )
		nexCAL_MemFree(pTempBuffer);
	if( pMemBuffer )
		nexCAL_MemFree(pMemBuffer);

	*a_puDecodeResult = 1;
	nexCAL_DebugLogPrintf("[CALBody.cpp, %d] == nexCALBody_Image_JPEG_decode Failed(%d)", __LINE__, uiRet);
	return 1;
}
