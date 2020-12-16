/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |                            Nextreaming Corporation                       |
  |    	    Copyright¨Ï 2002-2008 All rights reserved to Nextreaming         |
  |                        http://www.nextreaming.com                        |
  |                                                                          |
  |__________________________________________________________________________|

*******************************************************************************
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
_______________________________________________________________________________

*     File Name        : NxMPEG4ASPVideoDecAPI.h
*     Author           : Kyoeng-joong Kim (kjkim@nextreaming.com) 
******************************************************************************/

// Header file for 1.6.0 or higher

#ifndef _NXMPEG4ASPVIDEODEC_API_H_
#define _NXMPEG4ASPVIDEODEC_API_H_
#ifdef __cplusplus
extern "C" 
{
#endif

//#define USE_DPB
#define DEC_MODE_RGB		1
#define DEC_MODE_DEBLOCK	2

// error code
#define ASPDEC_RESULT_OK						0
#define ASPDEC_RESULT_VLD_PARSE_ERROR			1
#define ASPDEC_RESULT_FEATURE_NOT_SUPPORTED		2
#define ASPDEC_RESULT_INTERLACE_NOT_SUPPORTED	3
#define	ASPDEC_RESULT_SESSION_END				4
#define ASPDEC_RESULT_INVALID_PARAMETER			5
#define ASPDEC_RESULT_BITSTREAM_TOO_SHORT		6
#define ASPDEC_RESULT_UNEXPECTED_BFRAME			7
#define ASPDEC_RESULT_BS_OVERRUN				8
#define ASPDEC_RESULT_FRAME_LIMIT				9
#define ASPDEC_RESULT_MEM_ALLOC_FAIL			10
#define ASPDEC_RESULT_NO_START_CODE				11
#define ASPDEC_RESULT_INVALID_VOP_SIZE			12
#define ASPDEC_RESULT_BVOP_OUT_OF_ORDER			13

// result code for NxMPEG4ASPVideoDecGetOutputFrame
#define FRAME_STATUS_OK			0
#define FRAME_STATUS_AFFECTED	1
#define FRAME_STATUS_ERROR		2
#define FRAME_STATUS_NO_IMAGE	3

#define FRAME_ATTR_BOTTOM_FIELD_FIRST	16

// CodecType argument of NxMPEG4ASPVideoDecConfig
#define CODEC_TYPE_MPEG4H263	0
#define	CODEC_TYPE_S263			1

//#define USE_EXTERNAL_RGBBUFFER

typedef struct {
	void *vo;
	void *bs;
	int	width;
	int height;
	int	pitch;
	int mode;
} NxMPEG4ASPVideoDecStruct;

#ifdef USE_DPB
NxMPEG4ASPVideoDecStruct *NxMPEG4ASPVideoDecInit(unsigned int numFrameBuffer);
int NxMPEG4ASPVideoGetNumBufferAvail(NxMPEG4ASPVideoDecStruct *mvi);
int NxMPEG4ASPVideoGetNumOutputFrames(NxMPEG4ASPVideoDecStruct *mvi, int last);
void NxMPEG4ASPVideoDecReleaseFrame(NxMPEG4ASPVideoDecStruct *mvi);
#else
NxMPEG4ASPVideoDecStruct *NxMPEG4ASPVideoDecInit(void);
#endif

int NxMPEG4ASPVideoDecConfig(NxMPEG4ASPVideoDecStruct *mvi, unsigned char *decoderSpecificInfo, int length, unsigned int CodecType);
int NxMPEG4ASPVideoDecPreConfig(NxMPEG4ASPVideoDecStruct *mvi, unsigned char *decoderSpecificInfo, int length, unsigned int CodecType);

int NxMPEG4ASPVideoDecDecode(NxMPEG4ASPVideoDecStruct *mvi,unsigned char *frameData, int length, int mode, unsigned int CTS);
int NxMPEG4ASPVideoDecDecodeStream(NxMPEG4ASPVideoDecStruct *mvi,unsigned char *frameData, int length, int mode, unsigned int CTS, int *consumed);

int NxMPEG4ASPVideoDecGetOutputFrame(NxMPEG4ASPVideoDecStruct *mvi, unsigned char **yp, unsigned char **up, unsigned char **vp, unsigned int *CTS, unsigned char last);

void NxMPEG4ASPVideoDecClose(NxMPEG4ASPVideoDecStruct *mvi);

int NxMPEG4ASPVideoDecGetVersionNum(int mode);

char *NxMPEG4ASPVideoDecGetVersionInfo(int mode);

int NxMPEG4ASPVideoDecGuessVOL(NxMPEG4ASPVideoDecStruct *mvi,unsigned char *frameData, int length,
							   unsigned int width, unsigned int height);


NxMPEG4ASPVideoDecStruct * NxMPEG4ASPVideoDecInitThumbnail(unsigned char *decoderSpecificInfo, int length, unsigned int CodecType);
int NxMPEG4ASPVideoDecDecodeThumbnail(NxMPEG4ASPVideoDecStruct *mvi,unsigned char *frameData, int length);
int NxMPEG4ASPVideoDecGetOutputFrameThumbnail(NxMPEG4ASPVideoDecStruct *mvi, unsigned char **yp, unsigned char **up, unsigned char **vp);
void NxMPEG4ASPVideoDecCloseThumbnail(NxMPEG4ASPVideoDecStruct *mvi);

#ifdef __cplusplus
}
#endif
#endif
