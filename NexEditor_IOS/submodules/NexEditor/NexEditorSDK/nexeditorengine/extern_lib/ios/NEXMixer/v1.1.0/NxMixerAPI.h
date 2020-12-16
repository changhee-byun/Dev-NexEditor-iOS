#ifdef NEX_DLL_EXPORT
#define DLLEXPORT __declspec(dllexport)
#elif NEX_DLL_IMPORT
#define DLLEXPORT __declspec(dllimport)
#else // For android library
#define DLLEXPORT
#endif

#ifndef NXMIXERAPI_H
#define NXMIXERAPI_H

#ifdef _WINDOWS
typedef __int64 Long64_t; 
typedef unsigned __int64 ULong64_t; 
#else	// For Android library
typedef long long Long64_t; 
typedef unsigned long long ULong64_t;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _INTEGRATED_LIBRARY_
#include "DataDefineForMixer.h"
#else
#define		MAX_PCM_BUFFER_SIZE		8192  //pjb - Max size °í·Á!!

#ifdef _WINDOWS
//#define		_USE_EXTERNAL_MEMFUNC_
#endif

#define		MIXER_OK						0
#define		MIXER_TRACK_NUM_0				-1
#define		MIXER_ERROR_NOT_ENOUGH_SIZE		-2

typedef struct
{
	unsigned int			nSamplePerFrame;
	unsigned int			nInChannel;
	//unsigned int			nInSampleRate;
	int						nVolume;  //range (0~200 - default :100)	
	int						nGain;
	int						bNULL_Input_Flag;

	// 0 : normal volume control (using "nVolume"), 
	// 1 : separate channel volume control (using nChannelVolume[])
	short					bSeparateChannelVolumeControl;

	// separate channel volume value.
	// [0]:L, [1]:R, [2]:C, [3]:LFE, [4]RL, [5]:RR
	int						nChannelVolume[6];

	// range ( -100 ~ 100, -100:leftmost, 100:rightmost)
	// [0] : Left pan(or Mono pan) , [1]: Right pan
	int						nPan[2];

} NXMIXER_TrackInfo, *PNXMIXER_TrackInfo;

typedef struct
{
	//Mix  Info
	unsigned int				nNum_Track2Mix;  //Number of track to mix
	unsigned int				nOutChannel;
	unsigned int				nOutSampleRate;
	int							nOutFrameSize;		//smallest nSamplePerFrame of each Track

	//Input , Output
    short						**pInputPCM;
	short						*pOutputPCM;

	int							nMixgain;
	//Track Info
	PNXMIXER_TrackInfo			*TrackInfo;
	short						bManualVolumeControl;	// 0 : Auto mix gain, 1 : Manual mix gain	
} NXMIXERHANDLE, *PNXMIXERHANDLE;


#ifdef _USE_EXTERNAL_MEMFUNC_
extern void *nex_malloc(unsigned int size);
extern void nex_free(void *pMem);
#define NEX_MIXER_MALLOC	nex_malloc
#define NEX_MIXER_FREE		nex_free
#else
#define NEX_MIXER_MALLOC	malloc
#define NEX_MIXER_FREE		free
#endif

#endif // ifdef _INTERGRATED_LIBRARY_


///////////////////////////////////////////////////////////////////////////////
// APIs
///////////////////////////////////////////////////////////////////////////////

DLLEXPORT PNXMIXERHANDLE NxMixerOpen(int numberOfTracks);

DLLEXPORT void NxMixerClose( PNXMIXERHANDLE hHandle );
DLLEXPORT int NxMixerInit(PNXMIXERHANDLE hHandle);
DLLEXPORT int NxMixerProcess( PNXMIXERHANDLE hHandle );
DLLEXPORT void NxMixerReset( PNXMIXERHANDLE hHandle );

DLLEXPORT int NxMixerGetVersionNum( int mode );
DLLEXPORT const char* NxMixerGetVersionInfo( int mode );


#ifdef __cplusplus
}
#endif

#endif	// NXMIXERAPI_H
