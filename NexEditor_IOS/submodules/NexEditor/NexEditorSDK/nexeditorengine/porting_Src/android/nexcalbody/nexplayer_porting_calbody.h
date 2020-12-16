#ifndef	__NEXPLAYER_CALBODY_PORTING_H__
#define 	__NEXPLAYER_CALBODY_PORTING_H__


typedef void* DLCALBodyHandle;
#define DEFAULT_NEXCALBODY_LIB "/data/data/com.nextreaming.nexplayersample/lib/libnexcalbody.so"


#define NEX_CPUINFO_ARMV4			0x4
#define NEX_CPUINFO_ARMV5			0x5
#define NEX_CPUINFO_ARMV6			0x6
#define NEX_CPUINFO_ARMV7			0x7

#define NEX_SDKINFO_CUPCAKE 		0x15
#define NEX_SDKINFO_DONUT			0x16
#define NEX_SDKINFO_ECLAIR 		0x21
#define NEX_SDKINFO_FROYO 			0x22
#define NEX_SDKINFO_GINGERBREAD 	0x30

//COMMON

#define NEXCALBODY_INIT								"nexCALBody_Init"
#define NEXCALBODY_DEINIT								"nexCALBody_Deinit"

// AUDIO

#define NEXCALBODY_AUDIO_GETPROPERTY_FUNC			"nexCALBody_Audio_GetProperty"
#define NEXCALBODY_AUDIO_SETPROPERTY_FUNC			"nexCALBody_Audio_SetProperty"
// AAC

#define NEXCALBODY_AUDIO_AAC_INIT_FUNC				"nexCALBody_Audio_AAC_Init"
#define NEXCALBODY_AUDIO_AAC_DEINIT_FUNC			"nexCALBody_Audio_AAC_Deinit"
#define NEXCALBODY_AUDIO_AAC_DEC_FUNC				"nexCALBody_Audio_AAC_Dec"
#define NEXCALBODY_AUDIO_AAC_RESET_FUNC				"nexCALBody_Audio_AAC_Reset"

// WMA
#define NEXCALBODY_AUDIO_WMA_INIT_FUNC				"nexCALBody_Audio_WMA_Init"
#define NEXCALBODY_AUDIO_WMA_DEINIT_FUNC			"nexCALBody_Audio_WMA_Deinit"
#define NEXCALBODY_AUDIO_WMA_DEC_FUNC				"nexCALBody_Audio_WMA_Dec"
#define NEXCALBODY_AUDIO_WMA_RESET_FUNC			"nexCALBody_Audio_WMA_Reset"

// MP3
#define NEXCALBODY_AUDIO_MP3_INIT_FUNC				"nexCALBody_Audio_MP3_Init"
#define NEXCALBODY_AUDIO_MP3_DEINIT_FUNC			"nexCALBody_Audio_MP3_Deinit"
#define NEXCALBODY_AUDIO_MP3_DEC_FUNC				"nexCALBody_Audio_MP3_Dec"
#define NEXCALBODY_AUDIO_MP3_RESET_FUNC				"nexCALBody_Audio_MP3_Reset"

// JDKIM 2010/11/10
// AMR-NB
#define NEXCALBODY_AUDIO_AMRNB_INIT_FUNC			"nexCALBody_Audio_AMR_Init"
#define NEXCALBODY_AUDIO_AMRNB_DEINIT_FUNC			"nexCALBody_Audio_AMR_Deinit"
#define NEXCALBODY_AUDIO_AMRNB_DEC_FUNC			"nexCALBody_Audio_AMR_Dec"
#define NEXCALBODY_AUDIO_AMRNB_RESET_FUNC			"nexCALBody_Audio_AMR_Reset"

// AMR-WB
#define NEXCALBODY_AUDIO_AMRWB_INIT_FUNC			"nexCALBody_Audio_AMRWB_Init"
#define NEXCALBODY_AUDIO_AMRWB_DEINIT_FUNC			"nexCALBody_Audio_AMRWB_Deinit"
#define NEXCALBODY_AUDIO_AMRWB_DEC_FUNC			"nexCALBody_Audio_AMRWB_Dec"
#define NEXCALBODY_AUDIO_AMRWB_RESET_FUNC			"nexCALBody_Audio_AMRWB_Reset"
// JDKIM : end

// END AUDIO

// VIDEO
#define NEXCALBODY_VIDEO_GETPROPERTY_FUNC			"nexCALBody_Video_GetProperty"
#define NEXCALBODY_VIDEO_SETPROPERTY_FUNC			"nexCALBody_Video_SetProperty"
// AVC
#define NEXCALBODY_VIDEO_AVC_INIT_FUNC				"nexCALBody_Video_AVC_Init"
#define NEXCALBODY_VIDEO_AVC_CLOSE_FUNC				"nexCALBody_Video_AVC_Close"
#define NEXCALBODY_VIDEO_AVC_DEC_FUNC				"nexCALBody_Video_AVC_Dec"
#define NEXCALBODY_VIDEO_AVC_GETOUTPUT_FUNC		"nexCALBody_Video_AVC_GetOutput"
#define NEXCALBODY_VIDEO_AVC_RESET_FUNC				"nexCALBody_Video_AVC_Reset"


// DIVX
#define NEXCALBODY_VIDEO_DIVX_INIT_FUNC				"nexCALBody_Video_DIVX_Init"
#define NEXCALBODY_VIDEO_DIVX_CLOSE_FUNC			"nexCALBody_Video_DIVX_Close"
#define NEXCALBODY_VIDEO_DIVX_DEC_FUNC				"nexCALBody_Video_DIVX_Dec"
#define NEXCALBODY_VIDEO_DIVX_GETOUTPUT_FUNC		"nexCALBody_Video_DIVX_GetOutput"
#define NEXCALBODY_VIDEO_DIVX_RESET_FUNC			"nexCALBody_Video_DIVX_Reset"

// WMV
#define NEXCALBODY_VIDEO_WMV_INIT_FUNC				"nexCALBody_Video_WMV_Init"
#define NEXCALBODY_VIDEO_WMV_CLOSE_FUNC				"nexCALBody_Video_WMV_Close"
#define NEXCALBODY_VIDEO_WMV_DEC_FUNC				"nexCALBody_Video_WMV_Dec"
#define NEXCALBODY_VIDEO_WMV_GETOUTPUT_FUNC		"nexCALBody_Video_WMV_GetOutput"
#define NEXCALBODY_VIDEO_WMV_RESET_FUNC				"nexCALBody_Video_WMV_Reset"

// WVC1
#define NEXCALBODY_VIDEO_WVC1_INIT_FUNC				"nexCALBody_Video_WVC1_Init"
#define NEXCALBODY_VIDEO_WVC1_CLOSE_FUNC				"nexCALBody_Video_WVC1_Close"
#define NEXCALBODY_VIDEO_WVC1_DEC_FUNC				"nexCALBody_Video_WVC1_Dec"
#define NEXCALBODY_VIDEO_WVC1_GETOUTPUT_FUNC		"nexCALBody_Video_WVC1_GetOutput"
#define NEXCALBODY_VIDEO_WVC1_RESET_FUNC				"nexCALBody_Video_WVC1_Reset"

// END VIDEO


// TEXT
#define NEXCALBODY_TEXT_INIT_FUNC						"nexCALBody_Text_Init"
#define NEXCALBODY_TEXT_CLOSE_FUNC					"nexCALBody_Text_Close"
#define NEXCALBODY_TEXT_DEC_FUNC						"nexCALBody_Text_Dec"
#define NEXCALBODY_TEXT_GETOUTPUT_FUNC				"nexCALBody_Text_GetOutput"

// END TEXT

typedef int	(*NXCALRegisterCodec)
(
	unsigned int		uiCodec,
	void *			pFunctions,
	unsigned int		uiKey						// JDKIM 2011/04/05
);

// Common
//typedef unsigned int (*NEXCALBody_Init)(int nLogLevel);			// JDKIM 2010/11/11
//typedef unsigned int (*NEXCALBody_Deinit)();

typedef unsigned int (*NEXCALBody_GetProperty)( unsigned int uProperty, unsigned int *puValue, unsigned int uUserData );

typedef unsigned int (*NEXCALBody_SetProperty)(unsigned int uProperty, unsigned int uValue, unsigned int uUserData);

typedef unsigned int (*NEXCALBody_Register)( NXCALRegisterCodec ftRegister, int nLogLevel );


// Audio
typedef unsigned int (*NEXCALBody_Audio_Init)(	unsigned int uCodecObjectTypeIndication,
												unsigned char* pConfig,
												int iConfigLen,
												unsigned char* pFrame,
												int iFrameLen,
												unsigned int* piSamplingRate,
												unsigned int* piNumOfChannels,
												unsigned int* puBitsPerSample,
												unsigned int* piNumOfSamplesPerChannel,
												unsigned int* puUserData );

typedef unsigned int (*NEXCALBody_Audio_Deinit)(unsigned int uUserData);

typedef unsigned int (*NEXCALBody_Audio_Dec)(unsigned char* pSource,
												int iLen,
												void* pDest,
												int* piWrittenPCMSize,
												unsigned int uDTS,
												unsigned int uPTS,
												unsigned int* puDecodeResult,
												unsigned int uUserData );

typedef unsigned int (*NEXCALBody_Audio_Reset)( unsigned int uUserData );


// Video

typedef unsigned int (*NEXCALBody_Video_Init)(	unsigned int uCodecObjectTypeIndication, 
												unsigned char* pConfig, 
												int iLen, 
											  	unsigned char* pConfigEnhance, 
											  	int iEnhLen, 
											  	int iNALHeaderLengthSize, 
											  	int* piWidth, 
											  	int* piHeight, 
											  	int* piPitch, 
											  	unsigned int* puUserData );

typedef unsigned int (*NEXCALBody_Video_Close)(unsigned int uUserData );

typedef unsigned int (*NEXCALBody_Video_Dec)(	unsigned char* pData, 
												int iLen, 
												unsigned int uDTS, 
												unsigned int uPTS,												
												int iEnhancement, 
									  			unsigned int* puDecodeResult, 
									  			unsigned int uUserData );

typedef unsigned int (*NEXCALBody_Video_GetOutput)(	unsigned char** ppBits1, 
														unsigned char** ppBits2, 
														unsigned char** ppBits3, 
														unsigned int *pDTS, 
														unsigned int uUserData );

typedef unsigned int (*NEXCALBody_Video_Reset)( unsigned int uUserData );

typedef unsigned int (*NEXCALBody_Video_GetInfo)(unsigned int uIndex, unsigned int* puResult, unsigned int uUserData);

// Text

typedef unsigned int (*NEXCALBody_Text_Init)(	unsigned int uCodecObjectTypeIndication, 
												unsigned char* pConfig, 
												int iLen, 
												unsigned int* puUserData );

typedef unsigned int (*NEXCALBody_Text_Close)(unsigned int uUserData );
typedef unsigned int (*NEXCALBody_Text_Dec)(	unsigned char* pData, 
												int iLen, 
												unsigned int uDTS, 
												unsigned int* puDecodeResult, 
												unsigned int uUserData );

typedef unsigned int (*NEXCALBody_Text_GetOutput)(unsigned char** ppData, unsigned int uUserData );

typedef unsigned int ( *NEXCALBody_Video_EncoderInit )
	( unsigned int uCodecObjectTypeIndication
	, unsigned char** ppConfig
	, int* piConfigLen
	, int iQuality
	, int iWidth
	, int iHeight
	, int iFPS
	, unsigned int bCBR
	, int iBitRate
	, unsigned int* puUserData );

typedef unsigned int ( *NEXCALBody_Video_EncoderDeinit ) ( unsigned int uUserData );

typedef unsigned int ( *NEXCALBody_Video_EncoderEncode )
	( unsigned char* pData1
	, unsigned char* pData2
	, unsigned char* pData3
	, unsigned char** ppOutData
	, unsigned int* piOutLen
	, unsigned int* puEncodeResult
	, unsigned int uUserData );

typedef unsigned int ( *NEXCALBody_Video_EncoderSkip )
	( unsigned char** ppOutData
	, unsigned int* piOutLen
	, unsigned int uUserData );

typedef	unsigned int ( *NEXCALBody_Audio_EncoderInit ) 
	( unsigned int uCodecObjectTypeIndication
	, unsigned char* pConfig
	, int iConfigLen
	, unsigned int uSamplingRate
	, unsigned int uNumOfChannels
	, unsigned int* puUserData );

typedef unsigned int ( *NEXCALBody_Audio_EncoderDeinit ) ( unsigned int uUserData );



typedef	unsigned int ( *NEXCALBody_Audio_EncoderEncode ) 
	( unsigned char* pData
	, int iLen
	, unsigned char** ppOutData
	, unsigned int* piOutLen
	, unsigned int* puEncodeResult
	, unsigned int uUserData );

typedef unsigned int ( *NEXCALBody_Audio_EncoderReset ) ( unsigned int uUserData );


// JDKIM 2011/07/01
/*
typedef struct _CALBODY_FUNCTION_
{ 


//	NEXCALBody_Init								fnNexCALBody_Init;
//	NEXCALBody_Deinit							fnNexCALBody_Deinit;
	
	// AAC
	NEXCALBody_Audio_Init						fnNexCALBody_Audio_AAC_Init;
	NEXCALBody_Audio_Deinit						fnNexCALBody_Audio_AAC_Deinit;
	NEXCALBody_Audio_Dec						fnNexCALBody_Audio_AAC_Dec;
	NEXCALBody_Audio_Reset						fnNexCALBody_Audio_AAC_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Audio_AAC_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Audio_AAC_SetProperty;

	// WMA
	NEXCALBody_Audio_Init						fnNexCALBody_Audio_WMA_Init;
	NEXCALBody_Audio_Deinit						fnNexCALBody_Audio_WMA_Deinit;
	NEXCALBody_Audio_Dec						fnNexCALBody_Audio_WMA_Dec;
	NEXCALBody_Audio_Reset						fnNexCALBody_Audio_WMA_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Audio_WMA_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Audio_WMA_SetProperty;
	

	// MP3
	NEXCALBody_Audio_Init						fnNexCALBody_Audio_MP3_Init;
	NEXCALBody_Audio_Deinit						fnNexCALBody_Audio_MP3_Deinit;
	NEXCALBody_Audio_Dec						fnNexCALBody_Audio_MP3_Dec;
	NEXCALBody_Audio_Reset						fnNexCALBody_Audio_MP3_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Audio_MP3_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Audio_MP3_SetProperty;

	// JDKIM 2010/11/10
	// AMR-NB
	NEXCALBody_Audio_Init						fnNexCALBody_Audio_AMRNB_Init;
	NEXCALBody_Audio_Deinit						fnNexCALBody_Audio_AMRNB_Deinit;
	NEXCALBody_Audio_Dec						fnNexCALBody_Audio_AMRNB_Dec;
	NEXCALBody_Audio_Reset						fnNexCALBody_Audio_AMRNB_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Audio_AMRNB_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Audio_AMRNB_SetProperty;
	
	// AMR-WB
	NEXCALBody_Audio_Init						fnNexCALBody_Audio_AMRWB_Init;
	NEXCALBody_Audio_Deinit						fnNexCALBody_Audio_AMRWB_Deinit;
	NEXCALBody_Audio_Dec						fnNexCALBody_Audio_AMRWB_Dec;
	NEXCALBody_Audio_Reset						fnNexCALBody_Audio_AMRWB_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Audio_AMRWB_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Audio_AMRWB_SetProperty;
	
	// JDKIM : end
	// Audio End
	
	// Video
	// AVC
	NEXCALBody_Video_Init						fnNexCALBody_Video_AVC_Init;
	NEXCALBody_Video_Close						fnNexCALBody_Video_AVC_Close;
	NEXCALBody_Video_Dec						fnNexCALBody_Video_AVC_Dec;
	NEXCALBody_Video_GetOutput					fnNexCALBody_Video_AVC_GetOutput;
	NEXCALBody_Video_Reset						fnNexCALBody_Video_AVC_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Video_AVC_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Video_AVC_SetProperty;
	NEXCALBody_Video_GetInfo						fnNexCALBody_Video_AVC_GetInfo;
	
	// DIVX
	NEXCALBody_Video_Init						fnNexCALBody_Video_DIVX_Init;
	NEXCALBody_Video_Close						fnNexCALBody_Video_DIVX_Close;
	NEXCALBody_Video_Dec						fnNexCALBody_Video_DIVX_Dec;
	NEXCALBody_Video_GetOutput					fnNexCALBody_Video_DIVX_GetOutput;
	NEXCALBody_Video_Reset						fnNexCALBody_Video_DIVX_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Video_DIVX_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Video_DIVX_SetProperty;
	NEXCALBody_Video_GetInfo						fnNexCALBody_Video_DIVX_GetInfo;
	
	// WMV
	NEXCALBody_Video_Init						fnNexCALBody_Video_WMV_Init;
	NEXCALBody_Video_Close						fnNexCALBody_Video_WMV_Close;
	NEXCALBody_Video_Dec						fnNexCALBody_Video_WMV_Dec;
	NEXCALBody_Video_GetOutput					fnNexCALBody_Video_WMV_GetOutput;
	NEXCALBody_Video_Reset						fnNexCALBody_Video_WMV_Reset;
	NEXCALBody_GetProperty						fnNexCALBody_Video_WMV_GetProperty;
	NEXCALBody_SetProperty						fnNexCALBody_Video_WMV_SetProperty;
	NEXCALBody_Video_GetInfo						fnNexCALBody_Video_WMV_GetInfo;	


	// Text
	NEXCALBody_GetProperty						fnNexCALBody_Text_GetProperty;
	NEXCALBody_Text_Init							fnNexCALBody_Text_Init;
	NEXCALBody_Text_Close						fnNexCALBody_Text_Close;
	NEXCALBody_Text_Dec							fnNexCALBody_Text_Dec;
	NEXCALBody_Text_GetOutput					fnNexCALBody_Text_GetOutput;

}CALBODY_FUNCTION_ST;
*/

#define	MAX_AUDIO_CODEC_NUM			10
#define	MAX_VIDEO_CODEC_NUM			10

typedef struct _CALBODY_FUNCTION_
{ 
	unsigned int			uiAudioCodecNum;
	unsigned int			uiVideoCodecNum;
	struct
	{
		unsigned int					uiCodecType;
		NEXCALBody_Audio_Init		ftAudioInit;
		NEXCALBody_Audio_Deinit		ftAudioClose;
		NEXCALBody_Audio_Dec		ftAudioDec;
		NEXCALBody_Audio_Reset		ftAudioReset;
		NEXCALBody_GetProperty		ftGetProperty;
		NEXCALBody_SetProperty		ftSetProperty;	
	} stAudio[MAX_AUDIO_CODEC_NUM];

	struct
	{
		unsigned int					uiCodecType;
		NEXCALBody_Video_Init		ftVideoInit;
		NEXCALBody_Video_Close		ftVideoClose;
		NEXCALBody_Video_Dec		ftVideoDec;
		NEXCALBody_Video_GetOutput	ftVideoGetOutput;
		NEXCALBody_Video_Reset		ftVideoReset;
		NEXCALBody_GetProperty		ftGetProperty;
		NEXCALBody_SetProperty		ftSetProperty;
		NEXCALBody_Video_GetInfo		ftGetInfo;
	} stVideo[MAX_VIDEO_CODEC_NUM];

}CALBODY_FUNCTION_ST;
// JDKIM : end



typedef struct CALVideoFunctions
{
	NEXCALBody_Video_Init		ftVideoInit;
	NEXCALBody_Video_Close		ftVideoClose;
	NEXCALBody_Video_Dec		ftVideoDec;
	NEXCALBody_Video_GetOutput	ftVideoGetOutput;
	NEXCALBody_Video_Reset		ftVideoReset;
	NEXCALBody_GetProperty		ftGetProperty;
	NEXCALBody_SetProperty		ftSetProperty;
	NEXCALBody_Video_GetInfo		ftGetInfo;

	NEXCALBody_Video_EncoderInit		ftVideoEncInit;
	NEXCALBody_Video_EncoderDeinit	ftVideoEncDeinit;
	NEXCALBody_Video_EncoderEncode	ftVideoEncEnc;
	NEXCALBody_Video_EncoderSkip		ftVideoEncSkip;
} CALVideoFunctions;

typedef struct CALAudioFunctions
{
	NEXCALBody_Audio_Init		ftAudioInit;
	NEXCALBody_Audio_Deinit		ftAudioClose;
	NEXCALBody_Audio_Dec		ftAudioDec;
	NEXCALBody_Audio_Reset		ftAudioReset;
	NEXCALBody_GetProperty		ftGetProperty;	
	NEXCALBody_SetProperty		ftSetProperty;

	NEXCALBody_Audio_EncoderInit		ftAudioEncInit;
	NEXCALBody_Audio_EncoderDeinit	ftAudioEncDeinit;
	NEXCALBody_Audio_EncoderEncode	ftAudioEncEnc;
	NEXCALBody_Audio_EncoderReset	ftAudioEncReset;
} CALAudioFunctions;

#endif //__NEXPLAYER_PORTING_H__
