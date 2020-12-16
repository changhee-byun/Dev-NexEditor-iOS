
#ifndef NXANIMATEDGIF_H
#define NXANIMATEDGIF_H

#ifdef __cplusplus
extern "C" {
#endif
	

typedef struct
{
	// Input parameters
	unsigned char	*pInputBitStream;
	unsigned char	*pOutputGIF;
	int				nOutputGifSize;
	unsigned short	width;
	unsigned short	height;
	int				delayMs;
	int				mode; //0: speed mode, 1: quality mode

	//mode 1 setting parameter
	int quantization_method;	//default 1
	float quantization_gamma;	//default 1.8
	int sample_factor;			//default 1
	//mode 1 setting parameter end
	

	int				nNumCores;
	int				nInputCnt;
	int				bLastFrm;
	int				bOutputRepeat; //default 1(repeat)
	unsigned int	nUseChacheSize; //width*height 값보다 큰 경우에면 chache 사용(ex:960x720=691200, 이사이즈보다 큰 경우에만 chache 사용)
	
	// Internal use: Do not edit below
	void			*AniGIFInternal[8];

} NXANIGIFHANDLE, *PNXANIGIFHANDLE;

///////////////////////////////////////////////////////////////////////////////
// APIs
///////////////////////////////////////////////////////////////////////////////

PNXANIGIFHANDLE NxAniGIFOpen( void );
void NxAniGIFClose( PNXANIGIFHANDLE hDecoder );
int NxAniGIFInit( PNXANIGIFHANDLE hDecoder);
int NxAniGIFStart( PNXANIGIFHANDLE hDecoder );


int NxAniGIFGetVersionNum( int mode );
const char* NxAniGIFGetVersionInfo( int mode );

#ifdef __cplusplus
}
#endif

#endif	// NXANIMATEDGIF_H
