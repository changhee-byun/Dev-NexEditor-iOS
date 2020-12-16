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

*     File Name        : NxYUVtoRGBAPI.h
*     Version          : 0.0.1

******************************************************************************/

// header file for version 0.0.1 or later

#ifndef _NXYUVTORGB_API_H_
#define _NXYUVTORGB_API_H_

#ifdef __cplusplus
extern "C" {
#endif


int NxYYUVtoYUV420(int iWidth, int iHeight, unsigned char *pSrc, unsigned char *pY, unsigned char *pUV);
int NxYYUVtoY2UV(int iWidth, int iHeight, unsigned char *pSrc, unsigned char *pY, unsigned char *pUV);
int NxYYUVtoY2VU(int iWidth, int iHeight, unsigned char *pSrc, unsigned char *pY, unsigned char *pUV);
int NxYYUVAtoYUV420(int iWidth, int iHeight, unsigned char *pSrc, unsigned char *pY, unsigned char *pU, unsigned char *pV);
int NxUVYYAtoYUV420(int iWidth, int iHeight, unsigned char *pSrc, unsigned char *pY, unsigned char *pU, unsigned char *pV);

int NxRGBtoYUV420(unsigned char *pDst, unsigned char *pSrc, int iWidth, int iHeight, int iByteCount);		// RGB to YUV420p  (YV12)
int NxRGBtoYUV420semi(unsigned char *pDst, unsigned char *pSrc, int iWidth, int iHeight, int iByteCount);	// RGB to YUV420sp (NV21)

void NxYUV420toRGB565(unsigned short *pDst, unsigned char *pY, unsigned char *pU, unsigned char *pV, unsigned int iWidth, unsigned int iHeight, unsigned int SrcPitch, unsigned int DstPitch);
void NxYUV420toRGB888(unsigned int *pDst, unsigned char *pY, unsigned char *pU, unsigned char *pV, unsigned int iWidth, unsigned int iHeight, unsigned int SrcPitch, unsigned int DstPitch);

void NxYUV420toRGB565_BC( unsigned short *pDst, unsigned char *pY, unsigned char *pU, unsigned char *pV, unsigned int iWidth, unsigned int iHeight, unsigned int SrcPitch, unsigned int DstPitch, int Brightness, int Contrast);
void NxYUV420toRGB888_BC( unsigned int *pDst, unsigned char *pY, unsigned char *pU, unsigned char *pV, unsigned int iWidth, unsigned int iHeight, unsigned int SrcPitch, unsigned int DstPitch, int Brightness, int Contrast);

int NxYUVtoRGBGetVersionNum(int mode);
char *NxYUVtoRGBGetVersionInfo(int mode);

void NxYUV420ptoYUV420sp(unsigned int iWidth, unsigned int iHeight, unsigned char* pSrc, unsigned char* pDst);	// YUV420p(YV12) to YUV420sp(NV21)
void NxYUV420sptoYUV420p(unsigned int iWidth, unsigned int iHeight, unsigned char* pSrc, unsigned char* pDst);	// YUV420sp(NV21) to YUV420p(YV12)


#ifdef __cplusplus
}
#endif

#endif	//_NXYUVTORGB_API_H_