#ifndef __NEXRALBODY_COMMON_API_H__
#define __NEXRALBODY_COMMON_API_H__

typedef void (*FNRALCALLBACK)(int msg, int ext1, int ext2, int ext3, void* ext4, void* userData);


/**
 * \brief This functions registers the renderers to be used by the NexPlayer&trade;&nbsp;engine.
 *
 */
typedef int	(*NXRALRegisterRenderer)
(
//	unsigned int		uiType,
	void *			pFunctions,
	unsigned int		uiKey
);

typedef unsigned int (*NEXRALBody_Register)( NXRALRegisterRenderer ftRegister, int nLogLevel);



typedef void* (*GetNexRAL_HW_VideoRenderer) ( \
																  unsigned int uBufferFormat \
																, unsigned int API_Version \
																, unsigned int nexRAL_API_MAJOR \
																, unsigned int nexRAL_API_MINOR \
																, unsigned int nexRAL_API_PATCH_NUM \
																);
										
//struct ANativeWindow;
//typedef struct ANativeWindow ANativeWindow;
																
typedef void *(*InitNexRAL_HW_VideoRenderer) ( \
																  unsigned int hPlayer \
																, void *pSurface \
																, unsigned int uBufferFormat \
																);			
typedef int (*RegisterCallBackNexRAL_HW_VideoRenderer) (	void *callbackFunction, void *callbackUserData, void *userData );

typedef int (*ReleaseNexRAL_HW_VideoRenderer) ( void **userData );		

typedef int (*DisplayLogoNexRAL_NW_VideoRenderer)( void *userData );

#endif //__NEXRALBODY_COMMON_API_H__
