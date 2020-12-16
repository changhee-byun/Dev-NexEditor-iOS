#ifndef	NxFLACDecAPI
#define	NxFLACDecAPI

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//ERROR DEFINE
#define NX_FLAC_SUCCESS							0
#define NX_FLAC_ERROR							1
#define NX_FLAC_USE_STREAMINFO					2

typedef enum {
	NX_FIXED_BLOCKSIZE,
	NX_VARIABLE_BLOCKSIZE,
	NX_UNDEFINED_BLOCKSIZE
} NxBlockType;

typedef enum {
	NX_FRAME_NUMBER,
	NX_SAMPLE_NUMBER,
	NX_UNDEFINED_NUMBER
} NxIdxType;

typedef struct {

	unsigned char  *pBitstream;
	unsigned char *pPCMBuffer;

	unsigned int nInput;
	unsigned int nSamplesPerChannel;

	unsigned int nSampleRate;
	unsigned int nChannels;
	unsigned int nBitPerSample;

	unsigned int nConsumedBytes;

	int			bDownSampleFlag;  //1: 48kHz ÀÌ»ף sampling rate downSampling //pjb
	int			bOutBPS24;
	int			bPadding32bps;
	int			bDownMixingVolumeUp;	//1: multichannel downmix volume up, 0:default

	// Internal Handle
	void *CodecInternal;

} NXFLACDEC_HANDLE, *PNXFLACDEC_HANDLE;


//#define _USE_FLAC_EXTERNAL_MEMFUNC_

#ifdef _USE_FLAC_EXTERNAL_MEMFUNC_
typedef struct NXFLACDECMEMFUNC {
	
	void *(*pfnCallbackMalloc)(unsigned int);
	void *(*pfnCallbackCalloc)(unsigned int, unsigned int);
	void  (*pfnCallbackFree)(void*);

} NXFLACDECMEMFUNC, *PNXFLACDECMEMFUNC;

PNXFLACDEC_HANDLE NxFLACDecOpen(PNXFLACDECMEMFUNC hMemFunc);

#else
PNXFLACDEC_HANDLE NxFLACDecOpen(void);
#endif

int NxFLACDecInit(PNXFLACDEC_HANDLE hDecoder);
int NxFLACDecDecode(PNXFLACDEC_HANDLE hDecoder);
int NxFLACDecClose(PNXFLACDEC_HANDLE hDecoder);
int NxFLACDecReset(PNXFLACDEC_HANDLE hDecoder);

int NxFLACDecGetVersionNum( int mode );
const char* NxFLACDecGetVersionInfo( int mode );

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	// NxRAWDecAPI