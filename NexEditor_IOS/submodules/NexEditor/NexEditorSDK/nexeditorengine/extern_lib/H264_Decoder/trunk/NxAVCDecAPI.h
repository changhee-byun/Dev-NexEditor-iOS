/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |                            Nextreaming Corporation                       |
  |    	    Copyright¨Ï 2002-2009 All rights reserved to Nextreaming         |
  |                        http://www.nextreaming.com                        |
  |                                                                          |
  |__________________________________________________________________________|

*******************************************************************************
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
_______________________________________________________________________________

*     File Name        : NxAVCDecAPI.h
*     Version          : 1.1.8
*     Author           : Kyeong-joong Kim
******************************************************************************/

// header file for version 1.1.8 or later

#ifndef _NXAVCDEC_API_H_
#define _NXAVCDEC_API_H_

#define EXTERNAL_FRAMEBUFFER
#undef  PRE_ALLOC_FRAMEBUFFER

#define FRAME_TYPE_ERROR			0
#define FRAME_TYPE_OTHER			1
#define FRAME_TYPE_NON_IDR_SLICE	2
#define FRAME_TYPE_IDR_SLICE		3

#define AVC_ERROR_SYNTAX			1
#define AVC_ERROR_INVALID_PROFILE	2
#define	AVC_ERROR_INVALID_LEVEL		3
#define	AVC_ERROR_OUT_OF_RANGE		4
#define	AVC_ERROR_INVALID_TOOL		5
#define	AVC_ERROR_NOSPSPPS			6
#define	AVC_ERROR_INVALID_SPS		7
#define	AVC_ERROR_INVALID_PPS		8
#define	AVC_ERROR_NO_REF_FRAME		9
#define	AVC_ERROR_OUT_OF_MEMORY		10
#define AVC_ERROR_MISSING_SLICE		11
#define AVC_ERROR_NO_STARTCODE		12
#define AVC_ERROR_ASSIGN_FRAME		13
#define AVC_ERROR_NOT_IMPLEMENTED	14
#define AVC_ERROR_THUMBNAIL_NOT_ISLICE			15

#define AVC_FRAME_STATUS_NONE				0
#define AVC_FRAME_STATUS_ERROR_OCCURED		1
#define AVC_FRAME_STATUS_ERROR_AFFECTED		2
#define AVC_FRAME_STATUS_NORMAL				3

typedef struct {
	void *vo;
	int	width;
	int height;
	int	pitch;
} NxAVCDecStruct;

#ifdef __cplusplus
extern "C" 
{
#endif

#ifdef PRE_ALLOC_FRAMEBUFFER
NxAVCDecStruct *NxAVCDecInit(unsigned int ignoreInBandPS, unsigned int numFrameBuffer, void *(callbackAlloc)(int), void (*callBackFree)(void*));
#else
NxAVCDecStruct *NxAVCDecInit(unsigned int ignoreInBandPS, unsigned int numAddFrameBuffer,
							 void *(callbackAlloc)(int), void (*callbackFree)(void*),
							 void *(callbackAllocSmall)(int), void (*callbackFreeSmall)(void*));
#endif

int NxAVCDecConfig(NxAVCDecStruct *mvi, unsigned char *decoderSpecificInfo, int nalLengthBytes);
int NxAVCDecConfigFromRecord(NxAVCDecStruct *mvi, unsigned char *decoderSpecificInfo);

void NxAVCDecGetOutputFrame(NxAVCDecStruct *mvi, unsigned char **, unsigned char **, unsigned char **);
int NxAVCDecGetOutputFrameFromDPB(NxAVCDecStruct *mvi, unsigned int flush, unsigned int *CTS, unsigned char **y, unsigned char **u, unsigned char **v);
int NxAVCDecGetNumBufferAvailable(NxAVCDecStruct *mvi);

void NxAVCDecClose(NxAVCDecStruct *mvi);

// decoding function for NAL frame
int NxAVCDecDecodeFrame(NxAVCDecStruct *mvi,unsigned char *frameData, int length, unsigned int CTS, int *dispFlag);

// decoding function for AnnexB stream
int NxAVCDecDecodeAnnexBFrame(NxAVCDecStruct *mvi,unsigned char *frameData, unsigned int length, unsigned int CTS, unsigned int *consumed, int *dispFlag);

int NxAVCDecGetFrameType(NxAVCDecStruct *mvi,unsigned char *frameData, int length);
int NxAVCDecGetAnnexBFrameType(NxAVCDecStruct *mvi,unsigned char *frameData, unsigned int length);

// single NAL decoding functions
int NxAVCDecDecodeNAL(NxAVCDecStruct *mvi,unsigned char *frameData, int length, unsigned int CTS, int *dispFlag);
int NxAVCDecDecodeAnnexBNAL(NxAVCDecStruct *mvi,unsigned char *frameData, unsigned int length, unsigned int CTS, unsigned int *consumed, int *dispFlag);

// DXVA
NxAVCDecStruct *NxAVCDecInitDXVA(unsigned int ignoreInBandPS, unsigned int numAddFrameBuffer,
							 void *(callbackAlloc)(int), void (*callbackFree)(void*),
							 void *(callbackAllocSmall)(int), void (*callbackFreeSmall)(void*), void *pDev);
void NxAVCDecGetOutputDXVAFrame(NxAVCDecStruct *mvi, void **pSample);
int NxAVCDecGetOutputDXVAFrameFromDPB(NxAVCDecStruct *mvi, unsigned flush, unsigned int *CTS, void **pSample);
void NxAVCDecGetOutputThumbnailDXVA(NxAVCDecStruct *mvi, void **pSample);

// Thumbnail : optional
NxAVCDecStruct *NxAVCDecInitThumbnail(void *(callbackAllocSmall)(int), void (*callbackFreeSmall)(void*));
void NxAVCDecGetOutputThumbnail(NxAVCDecStruct *mvi, unsigned char **y, unsigned char **u, unsigned char **v);

void NxAVCDecResetDPB(NxAVCDecStruct *mvi);

int NxAVCDecGetVersionNum(int mode);
char *NxAVCDecGetVersionInfo(int mode);
#ifdef __cplusplus
}
#endif

#endif
