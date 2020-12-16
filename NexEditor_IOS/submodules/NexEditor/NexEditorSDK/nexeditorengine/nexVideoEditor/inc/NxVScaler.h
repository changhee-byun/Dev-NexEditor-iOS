#ifndef _NXVSCALER_H_
#define _NXVSCALER_H_


#define XMERGE2(a, b)	a##b
#define MERGE2(a, b)	XMERGE2(a, b)
#define XMERGE3(a, b, c)	a##b##c
#define MERGE3(a, b, c)	XMERGE3(a, b, c)

#ifdef __ARMCC_VERSION
#define ARCH_POSTFIX	MERGE2(ARMV, __TARGET_ARCH_ARM)
#else
#define ARCH_POSTFIX	PC
//#define ARCH_POSTFIX
#endif

#define FUNC_ARM(a) MERGE2(a, ARCH_POSTFIX)

typedef struct {
	unsigned int dWidth, dHeight, dPitch;
	unsigned int width, height, pitch;
	unsigned short *indexTableY;
	//unsigned char *phaseTableY;
	unsigned char *posTableY;
	unsigned short *indexTableC;
	//unsigned char *phaseTableC;
	unsigned char *posTableC;
	unsigned char *avgBuffer;
	unsigned int nW, nH;
	unsigned int cW, cH;
	unsigned int m, n;
	unsigned int mode;
} NxVScalerStruct;

#ifdef __cplusplus
extern "C" {
#endif

NxVScalerStruct *FUNC_ARM(NxVScalerInit)(unsigned int dWidth, unsigned int dHeight, unsigned int dPitch,
				unsigned int width, unsigned int height, unsigned int pitch);

//void NxVScalerEnlargeYUV(NxVScalerStruct *nv, unsigned char *dY, unsigned char *dU, unsigned char *dV,
//				unsigned char *sY, unsigned char *sU, unsigned char *sV);
void FUNC_ARM(NxVScalerResizeYUV)(NxVScalerStruct *nv, unsigned char *dY, unsigned char *dU, unsigned char *dV,
				unsigned char *sY, unsigned char *sU, unsigned char *sV);
void FUNC_ARM(NxVScalerClose)(NxVScalerStruct *nv);

#ifdef __cplusplus
}
#endif

#endif
