/*****************************************************************************************
*
*       File Name   :	NxAACDecAPI.h
*		Module		:	Nextreaming AAC+v2 decoder library
*       Description :	Library interface header
*
******************************************************************************************
                         Nextreaming Confidential Proprietary
                     Copyright (C) 2002 Nextreaming Corporation
                 All rights are reserved by Nextreaming Corporation
----------------------------------------------------------------------------------------*/

#ifndef NXAACDECAPI_H
#define NXAACDECAPI_H

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////////////////////////////////

#define NXAACDECERR_OK									0
#define NXAACDECERR_NO_GAIN_CTRL						1
#define NXAACDECERR_NOT_ALLOWED_PULSE_CODING			2
#define NXAACDECERR_SCALEFACTOR_OVER_RANGE				4
#define NXAACDECERR_NOT_ADTS_SYNCWORD					5
#define NXAACDECERR_CHANNEL_ERROR_CONF_ERRRES			7
#define NXAACDECERR_HUFF_DEC_ERROR_CODEWORD				10
#define NXAACDECERR_NONEXIST_HUFF_CODEBOOK				11
#define NXAACDECERR_INVALID_CHANNEL						12
#define NXAACDECERR_EXCEEDED_BITSTREAM_ELE				13
#define NXAACDECERR_ERROR_PULSE_DECODE					14
#define NXAACDECERR_ERROR_SECTION_DATA					15
#define NXAACDECERR_EXCEEDED_SCALEFACTOR_BAND			16
#define NXAACDECERR_LTP_LAG_RANGE_OVER					18
#define NXAACDECERR_INVALID_SBR_PARAM					19
#define NXAACDECERR_SBR_WITHOUT_INITIALIZATION			20
#define NXAACDECERR_ERROR_SYNTAX_ELE					21
#define NXAACDECERR_ERROR_PRG_CONF_ELE					22
#define NXAACDECERR_ERROR_SBR_RECONST					23
#define NXAACDECERR_ERROR_FILL_ELE_SBR					24
#define NXAACDECERR_NOT_ENOUGH_SBR_DATA					25
#define NXAACDECERR_NO_LTP_DATA							26
#define NXAACDECERR_ERROR_PCE_FRAME						31
#define NXAACDECERR_WRONG_BITSTREAM_VAL					32
#define NXAACDECERR_INVALID_SAMPLERATE					33
#define NXAACDECERR_ERROR_PRED							34
#define NXAACDECERR_INVALID_OBJECTTYPE					36
#define NXAACDECERR_INVALID_OUTPUTFORMAT				37
#define NXAACDECERR_INVALID_HANDLE						38
#define NXAACDECERR_CHANNEL_CHANGED						39
#define NXAACDECERR_NEED_MORE_DATA						100
#define NXAACDECERR_INIT_FAIL							101

// NXAACDECHANDLE.AACType
#define NXAACDEC_AACTYPE_UNKNOWN		0
#define NXAACDEC_AACTYPE_MAIN			1
#define NXAACDEC_AACTYPE_LC				2
#define NXAACDEC_AACTYPE_SBR			5
#define NXAACDEC_AACTYPE_SBR_PS			29
#define NXAACDEC_AACTYPE_ELD			39
#define NXAACDEC_AACTYPE_DUAL			100
#define NXAACDEC_AACTYPE_DUAL_SBR		101


///////////////////////////////////////////////////////////////////////////////
// Types
///////////////////////////////////////////////////////////////////////////////

typedef struct
{
	// Input parameters
    unsigned char	*pInputBitStream;
	int				InputBitstreamLength;
	short			*pOutputPCM;

	short			*pOutputALLPCM; //Outbuffer pointer 0f 6channel (5.1)
	
	// Output info after decoding first frame
	int             TotalSamples;
	int				SamplePerChannel;
	int				SamplingRate;
	int				InputChannels;
	int				OutputChannels;
	int				AACType;
	int				InputBitstreamUsedBytes;
	int				is_adif;
	int				bDecodeLCOnly;
	int				bReadFrame;
	int				bHeaderPresent;
	int				bMultiChan_Output_flag;
	int				bDownMixed_Output_flag;
	
	//0 : Non zero padding 
	//1 : zero padding (3,4,5 채널->6 채널 output) 
	int				bMultiOut_ZeroPaddig;

	int				bMonoToStereo;		//1: mono to stereo , 0:mono to mono
	int				bDownSamplingFlag;	//0:no downSampling , 1: 48kHz 이하로 downSampling <-default

										
	
	// Internal use: Do not edit below
	void *CodecInternal;

} NXAACDECHANDLE, *PNXAACDECHANDLE;


///////////////////////////////////////////////////////////////////////////////
// APIs
///////////////////////////////////////////////////////////////////////////////

PNXAACDECHANDLE NxAACDecOpen( void );


void NxAACDecClose( PNXAACDECHANDLE hDecoder );
int NxAACDecInit( PNXAACDECHANDLE hDecoder, unsigned int *samplerate, unsigned int *channels );
int NxAACDecDecode( PNXAACDECHANDLE hDecoder );
void NxAACDecReset( PNXAACDECHANDLE hDecoder );
int NxAACDecParseConfig( PNXAACDECHANDLE hDecoder, unsigned int *samplerate, unsigned int *channels );
int NxAACErrCheck( PNXAACDECHANDLE hDecoder);

unsigned char *NxAACDecFindheader( PNXAACDECHANDLE hDecoder ); //adts,,,,

int NxAACDecGetVersionNum( int mode );
const char* NxAACDecGetVersionInfo( int mode );

#ifdef __cplusplus
}
#endif

#endif	// NXAACDECAPI_H
