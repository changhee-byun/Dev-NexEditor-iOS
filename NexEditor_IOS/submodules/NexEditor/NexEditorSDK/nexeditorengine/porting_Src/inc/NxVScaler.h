#ifndef _NXVSCALER_H_
#define _NXVSCALER_H_
typedef struct {
	unsigned int dWidth, dHeight, dPitch;
	unsigned int width, height, pitch;
	unsigned short *indexTableY;
	unsigned char *phaseTableY;
	unsigned short *indexTableC;
	unsigned char *phaseTableC;
} NxVScalerStruct;


extern "C" NxVScalerStruct *NxVScalerInitARMV5(unsigned int dWidth, unsigned int dHeight, unsigned int dPitch,
				unsigned int width, unsigned int height, unsigned int pitch);
extern "C" void NxVScalerResizeYUVARMV5(NxVScalerStruct *nv, unsigned char *dY, unsigned char *dU, unsigned char *dV,
				unsigned char *sY, unsigned char *sU, unsigned char *sV);
extern "C" void NxVScalerCloseARMV5(NxVScalerStruct *nv);


extern "C" NxVScalerStruct *NxVScalerInitARMV6(unsigned int dWidth, unsigned int dHeight, unsigned int dPitch,
				unsigned int width, unsigned int height, unsigned int pitch);
extern "C" void NxVScalerResizeYUVARMV6(NxVScalerStruct *nv, unsigned char *dY, unsigned char *dU, unsigned char *dV,
				unsigned char *sY, unsigned char *sU, unsigned char *sV);
extern "C" void NxVScalerCloseARMV6(NxVScalerStruct *nv);


extern "C" NxVScalerStruct *NxVScalerInitARMV7(unsigned int dWidth, unsigned int dHeight, unsigned int dPitch,
				unsigned int width, unsigned int height, unsigned int pitch);
extern "C" void NxVScalerResizeYUVARMV7(NxVScalerStruct *nv, unsigned char *dY, unsigned char *dU, unsigned char *dV,
				unsigned char *sY, unsigned char *sU, unsigned char *sV);
extern "C" void NxVScalerCloseARMV7(NxVScalerStruct *nv);


#endif
