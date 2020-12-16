#ifndef __NEXRALBODY_VIDEO_API_H__
#define __NEXRALBODY_VIDEO_API_H__

#include "nexRALBody_Common_API.h"
#include "nexRALBody_Video_Reg.h"

// Video 
#define	NEXRALBODY_VIDEO_GETPROPERTY_FUNC 			"nexRALBody_Video_getProperty"
#define	NEXRALBODY_VIDEO_SETPROPERTY_FUNC			"nexRALBody_Video_setProperty"
#define 	NEXRALBODY_VIDEO_INIT_FUNC					"nexRALBody_Video_init"
#define 	NEXRALBODY_VIDEO_DEINIT_FUNC 				"nexRALBody_Video_deinit"
#define 	NEXRALBODY_VIDEO_PAUSE_FUNC 				"nexRALBody_Video_pause"
#define 	NEXRALBODY_VIDEO_RESUME_FUNC 				"nexRALBody_Video_resume"
#define 	NEXRALBODY_VIDEO_FLUSH_FUNC 				"nexRALBody_Video_flush"
#define 	NEXRALBODY_VIDEO_DISPLAY_FUNC				"nexRALBody_Video_display"
#define	NEXRALBODY_VIDEO_CREATE_FUNC				"nexRALBody_Video_create"
#define 	NEXRALBODY_VIDEO_DELETE_FUNC				"nexRALBody_Video_delete"
#define 	NEXRALBODY_VIDEO_RESIZE_FUNC				"nexRALBody_Video_resize"
#define 	NEXRALBODY_VIDEO_RENDER_FUNC				"nexRALBody_Video_render"
#define 	NEXRALBODY_VIDEO_GETFRAMEBUFFER_FUNC		"nexRALBody_Video_GetFrameBuffer"
#define 	NEXRALBODY_VIDEO_PREPARESURFACE_FUNC		"nexRALBody_Video_prepareSurface"
#define 	NEXRALBODY_VIDEO_SETRENDEROPTION_FUNC		"nexRALBody_Video_setRenderOption"
#define 	NEXRALBODY_VIDEO_SETOUTPUTPOS_FUNC		"nexRALBody_Video_setOutputPos"
#define 	NEXRALBODY_VIDEO_ONOFF_FUNC				"nexRALBody_Video_OnOff"
#define 	NEXRALBODY_VIDEO_CAPTURE_FUNC				"nexRALBody_Video_Capture"
#define	NEXRALBODY_VIDEO_SETBITMAP_FUNC			"nexRALBody_Video_SetBitmap"
#define	NEXRALBODY_VIDEO_FILLBITMAP_FUNC			"nexRALBody_Video_FillBitmap"
#define	NEXRALBODY_VIDEO_GLINIT_FUNC				"nexRALBody_Video_GLInit"
#define	NEXRALBODY_VIDEO_GLDRAW_FUNC				"nexRALBody_Video_GLDraw"
#define	NEXRALBODY_VIDEO_SETCONTRASTBRIGHTNESS_FUNC	"nexRALBody_Video_SetContrastBrightness"

// Video Function.
typedef	unsigned int (*NEXRALBody_Video_getProperty)(unsigned int uiProperty, unsigned int* puValue, unsigned int uUserData);
typedef unsigned int (*NEXRALBody_Video_setProperty)(unsigned int uProperty, unsigned int uValue, unsigned int uUserData);

typedef unsigned int (*NEXRALBody_Video_init)(unsigned int iCodecType, unsigned int uWidth, unsigned int uHeight, unsigned int uPitch, unsigned int * puUserData);
typedef unsigned int (*NEXRALBody_Video_deinit)(unsigned int uUserData);
typedef unsigned int (*NEXRALBody_Video_display)(int bDisplay, unsigned int uCTS, unsigned char* pBits1, unsigned char* pBits2, unsigned char* pBits3,  unsigned int *pResult, unsigned int uUserData);

typedef unsigned int ( *NEXRALBody_Video_pause ) ( unsigned int uUserData );
typedef unsigned int ( *NEXRALBody_Video_resume ) ( unsigned int uUserData );
typedef unsigned int ( *NEXRALBody_Video_flush ) ( unsigned int uCTS, unsigned int uUserData );

typedef unsigned int (*NEXRALBody_Video_create)(int nLogLevel, FNRALCALLBACK fnCallback, void *pUserData,
												unsigned int uiLogo, int nLogoPos, int nLogoTime, int nLogoPeriod, unsigned int uiCpuInfo,
												unsigned int uiPlatformInfo, unsigned int uiRenderInfo, unsigned int uiDeviceColorFormat, unsigned int uiIOMXColorFormat);
typedef unsigned int (*NEXRALBody_Video_delete)(void *pUserData);

typedef void* (*NEXRALBody_Video_GetFrameBuffer)(unsigned int uWidth, unsigned int uHeight, unsigned int uPixelFormat);
typedef unsigned int (*NEXRALBody_Video_prepareSurface)(void* pSurface, unsigned int uUserData);
typedef unsigned int (*NEXRALBody_Video_setRenderOption)(unsigned int uiFlag);
typedef unsigned int (*NEXRALBody_Video_setOutputPos)(int iX, int iY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uUserData);
typedef unsigned int (*NEXRALBody_Video_OnOff)(unsigned int bOn, unsigned int bErase);
typedef unsigned int (*NEXRALBody_Video_Capture)(unsigned int uiCount, unsigned int uiInterval);

typedef unsigned int (*NEXRALBody_Video_SetBitmap)(void* jniEnv, void* Bitmap);
typedef unsigned int (*NEXRALBody_Video_FillBitmap)(void* jniEnv, void* Bitmap);
typedef unsigned int (*NEXRALBody_Video_GLInit)(int width, int height);
typedef unsigned int (*NEXRALBody_Video_GLDraw)(int nMode);
typedef unsigned int (*NEXRALBody_Video_SetContrastBrightness)(int contrast, int brightness, int uUserData);
typedef unsigned int (*NEXRALBody_Video_SetGLDisplayStop)(unsigned int isStopped);

typedef int (*NEXRALBody_Video_DisplayBlank)( void *userData );

typedef struct _VIDEO_RALBODY_FUNCTION_
{		

	unsigned int								uiCPUInfo;
	unsigned int								uiPlatformInfo;
	unsigned int								uiRenderInfo;
	unsigned int								uiDevColorFormat;
	unsigned int								uiIOMXColorFormat;

	NEXRALBody_Video_getProperty 				fnNexRALBody_Video_getProperty;
	NEXRALBody_Video_setProperty				fnNexRALBody_Video_setProperty;
	NEXRALBody_Video_init						fnNexRALBody_Video_init;
	NEXRALBody_Video_deinit					fnNexRALBody_Video_deinit;
	NEXRALBody_Video_pause					fnNexRALBody_Video_pause;
	NEXRALBody_Video_resume					fnNexRALBody_Video_resume;
	NEXRALBody_Video_flush					fnNexRALBody_Video_flush;
	NEXRALBody_Video_display					fnNexRALBody_Video_display;
	NEXRALBody_Video_create					fnNexRALBody_Video_create;
	NEXRALBody_Video_delete					fnNexRALBody_Video_delete;
	NEXRALBody_Video_prepareSurface			fnNexRALBody_Video_prepareSurface;
	NEXRALBody_Video_setRenderOption			fnNexRALBody_Video_setRenderOption;
	NEXRALBody_Video_setOutputPos				fnNexRALBody_Video_setOutputPos;
	NEXRALBody_Video_OnOff					fnNexRALBody_Video_OnOff;
	NEXRALBody_Video_Capture					fnNexRALBody_Video_Capture;
	NEXRALBody_Video_SetBitmap				fnNexRALBody_Video_SetBitmap;
	NEXRALBody_Video_FillBitmap				fnNexRALBody_Video_FillBitmap;	
	NEXRALBody_Video_GLInit					fnNexRALBody_Video_GLInit;
	NEXRALBody_Video_GLDraw					fnNexRALBody_Video_GLDraw;
	NEXRALBody_Video_SetContrastBrightness		fnNexRALBody_Video_SetContrastBrightness;
	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayBlank; //for ics native window.
	NEXRALBody_Video_DisplayBlank			fnNexRALBody_Video_DisplayLogo; //for ics native window.
	NEXRALBody_Video_SetGLDisplayStop		fnNexRALBody_Video_SetGLDisplayStop;
}VIDEO_RALBODY_FUNCTION_ST;


typedef struct _VIDEO_RAL_FUNCTIONS_
{

	NEXRALBody_Video_getProperty 				fnNexRALBody_Video_getProperty;
	NEXRALBody_Video_setProperty				fnNexRALBody_Video_setProperty;
	NEXRALBody_Video_init						fnNexRALBody_Video_init;
	NEXRALBody_Video_deinit					fnNexRALBody_Video_deinit;
	NEXRALBody_Video_display					fnNexRALBody_Video_display;
	NEXRALBody_Video_pause					fnNexRALBody_Video_pause;
	NEXRALBody_Video_resume					fnNexRALBody_Video_resume;
	NEXRALBody_Video_flush					fnNexRALBody_Video_flush;
	
}RalVideoFunctions;



#ifdef __cplusplus
extern "C"
{
#endif

unsigned int nexRALBody_Video_create(	int nLogLevel,
											FNRALCALLBACK fnCallback,
											void *pUserData,
											unsigned int uiLogo,
											int	nLogoPos,
											int uiLogoTime,
											int uiLogoPeriod,
											unsigned int uiCPUInfo,
											unsigned int uiPlatformInfo,
											unsigned int uiRenderInfo,
											unsigned int uiDeviceColorFormat,
											unsigned int uiIOMXColorFormat);						// JDKIM 2011/06/30
unsigned int nexRALBody_Video_delete(void *pUserData);

unsigned int nexRALBody_Video_getProperty( unsigned int uProperty, unsigned int *puValue, unsigned int uUserData );
unsigned int nexRALBody_Video_setProperty(unsigned int uProperty, unsigned int uValue, unsigned int uUserData);

unsigned int nexRALBody_Video_init		( unsigned int iCodecType
											, unsigned int uWidth
											, unsigned int uHeight
											, unsigned int uPitch
											, unsigned int *puUserData );
			
unsigned int nexRALBody_Video_deinit	( unsigned int uUserData );

unsigned int nexRALBody_Video_display	( int bDisplay
											, unsigned int uCTS
											, unsigned char *pBits1
											, unsigned char *pBits2
											, unsigned char *pBits3
											, unsigned int* puResult
											, unsigned int uUserData );

unsigned int nexRALBody_Video_pause( unsigned int uUserData );
unsigned int nexRALBody_Video_resume( unsigned int uUserData );
unsigned int nexRALBody_Video_flush( unsigned int uCTS, unsigned int uUserData );

unsigned int nexRALBody_Video_prepareSurface(void* pSurface, unsigned int uUserData);
unsigned int nexRALBody_Video_setRenderOption(unsigned int uiFlag);

unsigned int nexRALBody_Video_setOutputPos( int iX, int iY, unsigned int uiWidth, unsigned int uiHeight, unsigned int uUserData);
unsigned int nexRALBody_Video_capture(unsigned int uiCount, unsigned int uiInterval);
unsigned int nexRALBody_Video_OnOff(unsigned int bOn, unsigned int bErase );
unsigned int nexRALBody_Video_Capture( unsigned int uiCount, unsigned int uiInterval);
unsigned int nexRALBody_Video_SetBitmap( void* jniEnv, void* Bitmap);
unsigned int nexRALBody_Video_GLInit(int width, int height);
unsigned int nexRALBody_Video_GLDraw(int nMode);
unsigned int nexRALBody_Video_FillBitmap( void* jniEnv, void* Bitmap);
unsigned int nexRALBody_Video_SetContrastBrightness(int contrast, int brightness, int uUserData);
int nexRALBody_Video_DisplayLogo(void *userdata);
int nexRALBody_Video_DisplayLogo( void *userData );

unsigned int nexRALBody_setGLDisplayStop(unsigned int isStopped);

#ifdef __cplusplus
}
#endif

#endif //__NEXRALBODY_VIDEO_API_H__
