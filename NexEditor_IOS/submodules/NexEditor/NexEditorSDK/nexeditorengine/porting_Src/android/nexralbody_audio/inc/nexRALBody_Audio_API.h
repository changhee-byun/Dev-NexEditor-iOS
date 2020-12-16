#ifndef __NEXRALBODY_AUDIO_API_H__
#define __NEXRALBODY_AUDIO_API_H__

#include "nexRALBody_Common_API.h"
#include "nexRALBody_Audio_Reg.h"

#define NEXRAL_AUDIO_PATH_EARPHONE	0
#define NEXRAL_AUDIO_PATH_SPEAKER	1

// Audio
#define 	NEXRALBODY_AUDIO_GETPROPERTY_FUNC 			"nexRALBody_Audio_getProperty"
#define		NEXRALBODY_AUDIO_SETPROPERTY_FUNC			"nexRALBody_Audio_setProperty"
#define 	NEXRALBODY_AUDIO_INIT_FUNC					"nexRALBody_Audio_init"
#define 	NEXRALBODY_AUDIO_DEINIT_FUNC				"nexRALBody_Audio_deinit"
#define 	NEXRALBODY_AUDIO_GETEMPTYBUFFER_FUNC 		"nexRALBody_Audio_getEmptyBuffer"
#define 	NEXRALBODY_AUDIO_CONSUMEBUFFER_FUNC 		"nexRALBody_Audio_consumeBuffer"
#define 	NEXRALBODY_AUDIO_SETBUFFERMUTE_FUNC		"nexRALBody_Audio_setBufferMute"
#define 	NEXRALBODY_AUDIO_GETCURRENTCTS_FUNC		"nexRALBody_Audio_getCurrentCTS"
#define 	NEXRALBODY_AUDIO_CLEARBUFFER_FUNC			"nexRALBody_Audio_clearBuffer"
#define 	NEXRALBODY_AUDIO_PAUSE_FUNC				"nexRALBody_Audio_pause"
#define		NEXRALBODY_AUDIO_RESUME_FUNC				"nexRALBody_Audio_resume"
#define 	NEXRALBODY_AUDIO_FLUSH_FUNC				"nexRALBody_Audio_flush"
#define		NEXRALBODY_AUDIO_SETTIME_FUNC				"nexRALBody_Audio_setTime"
#define 	NEXRALBODY_AUDIO_SETPLAYBACKRATE_FUNC		"nexRALBody_Audio_setPlaybackRate"
#define 	NEXRALBODY_AUDIO_SETSOUNDPATH				"nexRALBody_Audio_SetSoundPath"
#define 	NEXRALBODY_AUDIO_CREATE						"nexRALBody_Audio_create"
#define 	NEXRALBODY_AUDIO_DELETE						"nexRALBody_Audio_delete"
#define 	NEXRALBODY_AUDIO_PREPAREAUDIOTRACK_FUNC	"nexRALBody_Audio_prepareAudioTrack"
#define 	NEXRALBODY_AUDIO_SETVOLUME_FUNC			"nexRALBody_Audio_SetVolume"

#define 	NEXRALBODY_AUDIO_MAVENINIT_FUNC						"nexRALBody_Audio_MavenInit"
#define 	NEXRALBODY_AUDIO_MAVENSETVOLUME_FUNC					"nexRALBody_Audio_MavenSetVolume"
#define 	NEXRALBODY_AUDIO_MAVENSETOUTPUT_FUNC					"nexRALBody_Audio_MavenSetOutput"
#define 	NEXRALBODY_AUDIO_MAVENSETPARAM_FUNC					"nexRALBody_Audio_MavenSetParam"
#define 	NEXRALBODY_AUDIO_MAVENSEQSETPARAM_FUNC				"nexRALBody_Audio_MavenSEQSetParam"
#define 	NEXRALBODY_AUDIO_MAVENVMS2SETPARAM_FUNC				"nexRALBody_Audio_MavenVMS2SetParam"
#define 	NEXRALBODY_AUDIO_MAVENVMS2FILTERSETMODE_FUNC		"nexRALBody_Audio_MavenVMS2FilterSetMode"
#define 	NEXRALBODY_AUDIO_MAVENVMS2FILTERSETSETPARAM_FUNC	"nexRALBody_Audio_MavenVMS2FilterSetParam"

// Audio Function
typedef unsigned int (*NEXRALBody_Audio_getProperty)(unsigned int uiProperty, unsigned int *puValue, void* uUserData);
typedef unsigned int (*NEXRALBody_Audio_setProperty)(unsigned int uiProperty, unsigned int uValue, void* uUserData);

typedef unsigned int (*NEXRALBody_Audio_init)(unsigned int iCodecType, unsigned int uSamplingRate, unsigned int uNumOfChannels, unsigned int uBitsPerSample, unsigned int uNumOfSamplesPerChannel, void** puUserData);
typedef unsigned int (*NEXRALBody_Audio_deinit)(void* uUserData);
typedef unsigned int (*NEXRALBody_Audio_getEmptyBuffer)( void** ppEmptyBuffer, int* nMaxBufferSize, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_consumeBuffer)( void* pBuffer, int nBufferLen, unsigned int uCTS, int isDecodeSuccess, int bEndFrame,  void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_setBufferMute)( void* pBuffer, bool bSetPCMSize, int* piWrittenPCMSize, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_getCurrentCTS)( unsigned int* puCTS, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_clearBuffer)( void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_pause)(void* uUserData);
typedef unsigned int (*NEXRALBody_Audio_resume)(void* uUserData);
typedef unsigned int (*NEXRALBody_Audio_flush)(unsigned int uCTS, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_setTime)(unsigned int uCTS, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_setPlaybackRate)( int iRate, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_SetSoundPath)(int iPath, void* uUserData);
typedef unsigned int (*NEXRALBody_Audio_create)(int nLogLevel, unsigned int bUseAudioEffect, void * /*jobject*/ audioManager, FNRALCALLBACK fnCallback, void* pUserData);
typedef unsigned int (*NEXRALBody_Audio_delete)(void* uUserData);
typedef unsigned int (*NEXRALBody_Audio_getAudioSessionId)( unsigned int* puSessionId, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_prepareAudioTrack)(void* pAudioTrack, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_SetVolume)(float fGain, void* uUserData);

typedef unsigned int (*NEXRALBody_Audio_MavenInit)( void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenSetVolume)( float fVolume, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenSetOutput)( unsigned int uiOutputPath, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenSetParam)( unsigned int uiMavenMode, unsigned int uiMavenStength, unsigned int uiBassStrength, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenSetAutoVolumeParam)( unsigned int uiEnable, unsigned int uiStrength, unsigned int uiRelease, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenSEQSetParam)( unsigned int uiSEQMode, int* pSEQParam, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenVMS2SetParam)( short* pParam, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenVMS2FilterSetMode)( int* pVMSMode, void* uUserData );
typedef unsigned int (*NEXRALBody_Audio_MavenVMS2FilterSetParam)( int** ppParam, void* uUserData );

typedef struct _AUDIO_RALBODY_FUNCTION_
{

	NEXRALBody_Audio_getProperty				fnNexRALBody_Audio_getProperty;
	NEXRALBody_Audio_setProperty				fnNexRALBody_Audio_setProperty;
	NEXRALBody_Audio_init						fnNexRALBody_Audio_init;
	NEXRALBody_Audio_deinit						fnNexRALBody_Audio_deinit;
	NEXRALBody_Audio_getEmptyBuffer				fnNexRALBody_Audio_getEmptyBuffer;
	NEXRALBody_Audio_consumeBuffer				fnNexRALBody_Audio_consumeBuffer;
	NEXRALBody_Audio_setBufferMute				fnNexRALBody_Audio_setBufferMute;
	NEXRALBody_Audio_getCurrentCTS				fnNexRALBody_Audio_getCurrentCTS;
	NEXRALBody_Audio_clearBuffer				fnNexRALBody_Audio_clearBuffer;
	NEXRALBody_Audio_pause						fnNexRALBody_Audio_pause;
	NEXRALBody_Audio_resume						fnNexRALBody_Audio_resume;
	NEXRALBody_Audio_flush						fnNexRALBody_Audio_flush;
	NEXRALBody_Audio_setTime					fnNexRALBody_Audio_setTime;
	NEXRALBody_Audio_setPlaybackRate			fnNexRALBody_Audio_setPlaybackRate;
	NEXRALBody_Audio_SetSoundPath				fnNexRALBody_Audio_SetSoundPath;
	NEXRALBody_Audio_create						fnNexRALBody_Audio_create;
	NEXRALBody_Audio_delete						fnNexRALBody_Audio_delete;
	NEXRALBody_Audio_getAudioSessionId			fnNexRALBody_Audio_getAudioSessionId;
	NEXRALBody_Audio_prepareAudioTrack			fnNexRALBody_Audio_prepareAudioTrack;
	NEXRALBody_Audio_SetVolume					fnNexRALBody_Audio_SetVolume;

	NEXRALBody_Audio_MavenInit					fnNexRALBody_Audio_MavenInit;
	NEXRALBody_Audio_MavenSetVolume				fnNexRALBody_Audio_MavenSetVolume;
	NEXRALBody_Audio_MavenSetOutput				fnNexRALBody_Audio_MavenSetOutput;
	NEXRALBody_Audio_MavenSetParam				fnNexRALBody_Audio_MavenSetParam;
	NEXRALBody_Audio_MavenSetAutoVolumeParam	fnNexRALBody_Audio_MavenSetAutoVolumeParam;
	NEXRALBody_Audio_MavenSEQSetParam			fnNexRALBody_Audio_MavenSEQSetParam;
	NEXRALBody_Audio_MavenVMS2SetParam			fnNexRALBody_Audio_MavenVMS2SetParam;
	NEXRALBody_Audio_MavenVMS2FilterSetMode		fnNexRALBody_Audio_MavenVMS2FilterSetMode;
	NEXRALBody_Audio_MavenVMS2FilterSetParam	fnNexRALBody_Audio_MavenVMS2FilterSetParam;

}AUDIO_RALBODY_FUNCTION_ST;





typedef struct _AUDIO_RAL_FUNCTIONS_
{

	NEXRALBody_Audio_getProperty				fnNexRALBody_Audio_getProperty;
	NEXRALBody_Audio_init						fnNexRALBody_Audio_init;
	NEXRALBody_Audio_deinit					fnNexRALBody_Audio_deinit;
	NEXRALBody_Audio_getEmptyBuffer			fnNexRALBody_Audio_getEmptyBuffer;
	NEXRALBody_Audio_consumeBuffer			fnNexRALBody_Audio_consumeBuffer;
	NEXRALBody_Audio_setBufferMute			fnNexRALBody_Audio_setBufferMute;
	NEXRALBody_Audio_getCurrentCTS			fnNexRALBody_Audio_getCurrentCTS;
	NEXRALBody_Audio_clearBuffer				fnNexRALBody_Audio_clearBuffer;
	NEXRALBody_Audio_pause					fnNexRALBody_Audio_pause;
	NEXRALBody_Audio_resume					fnNexRALBody_Audio_resume;
	NEXRALBody_Audio_flush					fnNexRALBody_Audio_flush;
	NEXRALBody_Audio_setTime					fnNexRALBody_Audio_setTime;
	NEXRALBody_Audio_setPlaybackRate			fnNexRALBody_Audio_setPlaybackRate;
}RalAudioFunctions;

#ifdef __cplusplus
extern "C"
{
#endif

unsigned int nexRALBody_Audio_create(int nLogLevel,  unsigned int bUseAudioEffect, void * /*jobject*/ audioManager, FNRALCALLBACK fnCallback, void *pUserData);
unsigned int nexRALBody_Audio_delete(void* uUserData);

unsigned int nexRALBody_Audio_getProperty( unsigned int uProperty, unsigned int *puValue, void* uUserData);
unsigned int nexRALBody_Audio_setProperty(unsigned int uProperty, unsigned int uValue, void* uUserData);

unsigned int nexRALBody_Audio_init		( unsigned int uCodecType
									, unsigned int uSamplingRate
									, unsigned int uNumOfChannels
									, unsigned int uBitsPerSample
									, unsigned int uNumOfSamplesPerChannel
									, void** puUserData );

unsigned int nexRALBody_Audio_deinit( void* uUserData );
unsigned int nexRALBody_Audio_getEmptyBuffer( void** ppEmptyBuffer, int* nMaxBufferSize, void* uUserData );
unsigned int nexRALBody_Audio_consumeBuffer( void* pBuffer, int nBufferLen, unsigned int uCTS, int isDecodeSuccess, int bEndFrame, void* uUserData );
unsigned int nexRALBody_Audio_setBufferMute( void* pBuffer, bool bSetPCMSize, int* piWrittenPCMSize, void* uUserData );
unsigned int nexRALBody_Audio_clearBuffer( void* uUserData );
unsigned int nexRALBody_Audio_pause( void* uUserData );
unsigned int nexRALBody_Audio_resume( void* uUserData );
unsigned int nexRALBody_Audio_flush( unsigned int uCTS, void* uUserData );
unsigned int nexRALBody_Audio_setTime( unsigned int uCTS, void* uUserData );
unsigned int nexRALBody_Audio_setPlaybackRate(int iRate, void* uUserData);
unsigned int nexRALBody_Audio_getCurrentCTS( unsigned int* puCTS, void* uUserData );

unsigned int nexRALBody_Audio_SetSoundPath( int iPath, void* uUserData );
unsigned int nexRALBody_Audio_getAudioSessionId(unsigned int* puSessionId, void* uUserData);
unsigned int nexRALBody_Audio_prepareAudioTrack(void* pAudioTrack, void* uUserData);

unsigned int nexRALBody_Audio_SetVolume(float fGain, void* uUserData);


unsigned int nexRALBody_Audio_MavenInit( void* uUserData );
unsigned int nexRALBody_Audio_MavenSetVolume( float fVolume, void* uUserData );
unsigned int nexRALBody_Audio_MavenSetOutput( unsigned int uiOutputPath, void* uUserData );
unsigned int nexRALBody_Audio_MavenSetParam( unsigned int uiMavenMode, unsigned int uiMavenStength, unsigned int uiBassStrength, void* uUserData );
unsigned int nexRALBody_Audio_MavenSetAutoVolumeParam( unsigned int uiEnable, unsigned int uiStrength, unsigned int uiReleaseTime, void* uUserData);
unsigned int nexRALBody_Audio_MavenSEQSetParam( unsigned int uiSEQMode, int* pSEQParam, void* uUserData );
unsigned int nexRALBody_Audio_MavenVMS2SetParam( short* pParam, void* uUserData );
unsigned int nexRALBody_Audio_MavenVMS2FilterSetMode( int* pVMSMode, void* uUserData );
unsigned int nexRALBody_Audio_MavenVMS2FilterSetParam( int** ppParam, void* uUserData );

#ifdef __cplusplus
}
#endif

#endif //__NEXRALBODY_AUDIO_API_H__
