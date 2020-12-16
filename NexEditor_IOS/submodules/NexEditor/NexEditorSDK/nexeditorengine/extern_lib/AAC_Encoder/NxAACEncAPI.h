
#ifndef _NXAACENCAPI_H_
#define _NXAACENCAPI_H_

#ifdef MONO_ONLY
#define MAX_CHANNELS        1
#else
#define MAX_CHANNELS        2
#endif

#define		AACENC_BLOCKSIZE	1024

#define NXAAC_ENC_OK	0

typedef struct
{
    short			*pInputPCM;				/* Buffer size must not over AACENC_BLOCKSIZE*MAX_CHANNELS */
	int				nInputPCMLength;
	unsigned char	*pOutputBitStream;
	int             nEncodedLength;

	unsigned char	*pConfig;
	int				nConfigLen;

	/* Config for Encode */
	int				nSampleRate;			/* audio file sample rate */
	int				nChannels;				/* number of channels on input (1,2) */
	int				nBitRate;               /* encoder bit rate in bits/sec */
	int				nBandWidth;             /* targeted audio bandwidth in Hz */
	int				bEncodeMono;            /* number of channels on output */
	
	// Internal use: Do not edit below
	void *CodecInternal;

} NXAACENCHANDLE, *PNXAACENCHANDLE;

#ifdef __cplusplus
extern "C" {
#endif

PNXAACENCHANDLE NxAACEncOpen( void );
void NxAACEncClose( PNXAACENCHANDLE hEncoder );
int NxAACEncInit( PNXAACENCHANDLE hEncoder );
int NxAACEncEncode( PNXAACENCHANDLE hEncoder );

int NxAACEncGetVersionNum( int mode );
const char* NxAACEncGetVersionInfo( int mode );

#ifdef __cplusplus
}
#endif

#endif	// _NXAACENCAPI_H_
