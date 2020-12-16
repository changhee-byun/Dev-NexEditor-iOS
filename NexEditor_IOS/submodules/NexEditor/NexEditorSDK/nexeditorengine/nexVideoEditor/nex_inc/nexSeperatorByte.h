#ifndef _NEXSEPERATORBYTE_H_
#define _NEXSEPERATORBYTE_H_

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
int seperatorByte(int iWidth, int iHeight, unsigned char* pSrc, unsigned char* pY, unsigned char* pUV);

#ifdef __cplusplus
}
#endif

#endif // _NEXSEPERATORBYTE_H_

