#ifdef NEX_DLL_EXPORT
#define DLLEXPORT __declspec(dllexport)
#elif NEX_DLL_IMPORT
#define DLLEXPORT __declspec(dllimport)
#else // For android library
#define DLLEXPORT
#endif

#ifndef __NEX_RESAMPLER_API_H__
#define __NEX_RESAMPLER_API_H__

/* Trace function */
//#include "NexSAL_Internal.h"
#define		NEX_TRACE		//nexSAL_DebugPrintf

/* Config setting */
//#define		_USE_RESAMPLER_EXTERNAL_MEM_FUNC_


/* Target Setting */
#if defined(__arm) && defined(__ARMCC_VERSION)
#	define RVDS_ARM

#	if (__TARGET_ARCH_ARM >= 5) && (__TARGET_ARCH_ARM < 7) && defined(__TARGET_FEATURE_DSPMUL)
#		define RVDS_ARMV5E
#	endif

#	if defined(__TARGET_ARCH_6) || ( (__TARGET_ARCH_ARM >= 6) && (__TARGET_ARCH_ARM < 7) )
#		define RVDS_ARMV6K
#	endif

#	if (__TARGET_ARCH_ARM >= 7) && defined(__TARGET_FEATURE_NEON)
#		define RVDS_ARMV7A
#	endif

#endif

#if defined(__arm) && defined(__APPLE__)

#	define IPHONE_ARM

#	if defined(__arm) && defined(__APPLE__)
#		ifdef __ARM_ARCH_7A__
#			define IPHONE_ARMV7A
#		endif

#		if defined(__ARM_ARCH_6K__) && !defined(__ARM_ARCH_7A__)
#			define IPHONE_ARMV6K
#		endif
#	endif

#endif


#if defined(RVDS_ARM) && !defined(__GNUC__)
#	define		PACK	__packed
#	define		ALIGN	__align(8)
#elif defined(RVDS_ARM) && defined(__GNUC__)
#	define		PACK	__attribute__((packed))
#	define		ALIGN	__attribute__((aligned(8)))
#else
#	define		PACK
#	define		ALIGN
#endif


/* Default values */
#define NEX_RESAMPLER_DEF_INRATE	44100
#define NEX_RESAMPLER_DEF_OUTRATE	48000
#define NEX_RESAMPLER_DEF_SIZE		4096
#define NEX_RESAMPLER_DEF_QUALITY	1


/* Error definitions */
#define NEX_RESAMPLER_STATUS_OK							 0
#define NEX_RESAMPLER_STATUS_INVALID_RATE				-1
#define NEX_RESAMPLER_STATUS_INVALID_CHANNEL			-2
#define NEX_RESAMPLER_STATUS_FILTER_ERROR				-3
#define NEX_RESAMPLER_STATUS_INTERNAL_HANDLE_ERROR		-4
#define NEX_RESAMPLER_STATUS_HIST_BUFF_ERROR			-5
#define NEX_RESAMPLER_STATUS_OUTPUT_ERROR				-6


//#define NX_BEEP_ENABLED
//#define NX_LIMITTIME_ENABLED

#ifdef NX_BEEP_ENABLED
#define NX_BEEP_INTERVAL_SEC   40  //20sec
#define NX_BEEP_DURATION_FRM   (nSamplesPerSec/dwFrameSize/2)  // 0.5 sec
#endif
#ifdef NX_LIMITTIME_ENABLED
#define NX_LIMITTIME_SEC       20*60   //10min
#endif

/* Memory function assign */
#if defined(_USE_RESAMPLER_EXTERNAL_MEM_FUNC_)

extern void *nex_malloc(unsigned int size);
extern void *nex_calloc(unsigned int count, unsigned int size);
extern void nex_free(void *pMem);

#define		NEX_RESAMPLER_MALLOC	nex_malloc
#define		NEX_RESAMPLER_CALLOC	nex_calloc
#define		NEX_RESAMPLER_FREE		nex_free

#else

#define		NEX_RESAMPLER_MALLOC	malloc
#define		NEX_RESAMPLER_CALLOC	calloc
#define		NEX_RESAMPLER_FREE		free

#endif	// _USE_RESAMPLER_EXTERNAL_MEM_FUNC_


typedef struct {
	unsigned char		*pInputBuffer;
	unsigned int		nInput;
	unsigned int		nInSampleRate;
	unsigned int		nInChannels;
	
	unsigned char		*pOutputBuffer;
	unsigned int		nOutput;
	unsigned int		nOutSampleRate;
	//unsigned int nOutChannels;	// same as input channel

	int			nBitPerSample;

	unsigned int Quality_Level;
	int nInitialized;

	// Internal Handle
	void *CodecInternal;

} NXRESAMPLERHANDLE, *PNXRESAMPLERHANDLE;



#ifdef __cplusplus
extern "C" {
#endif

DLLEXPORT PNXRESAMPLERHANDLE NxResamplerOpen(void);
DLLEXPORT int NxResamplerInit(PNXRESAMPLERHANDLE hDecoder);
DLLEXPORT int NxResamplerReset(PNXRESAMPLERHANDLE hDecoder);
DLLEXPORT int NxResamplerResample(PNXRESAMPLERHANDLE hDecoder);
DLLEXPORT void NxResamplerClose(PNXRESAMPLERHANDLE hDecoder);

DLLEXPORT int NxResamplerGetVersionNum( int mode );
DLLEXPORT const char* NxResamplerGetVersionInfo( int mode );

#ifdef __cplusplus
}
#endif

#endif	// __NEX_RESAMPLER_API_H__
