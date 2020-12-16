#ifndef _NEXYYUV2YUV420_H_
#define _NEXYYUV2YUV420_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
	iWidth : Src, Dst Width
	iWidth : Src, Dst Height
	pSrc : Src RGBA Buffer
	pY : Dst Y Buffer
	pUV : Dst UV Buffer
*/
int nexYYUVtoYUV420(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pUV);
int nexYYUVtoY2UV(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pUV);
int nexYYUVtoY2VU(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pUV);
int nexYYUVAtoYUV420(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pU, unsigned char* pV);
int nexUVYYAtoYUV420(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* p_U, unsigned char* p_V);

#ifdef __cplusplus
}
#endif

#endif // _NEXYYUV2YUV420_H_

