/******************************************************************************
* File Name   : NexMediaDef.h
* Description : Common media definition.
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

Copyright (C) 2013-2015 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXMEDIADEF_H_INCLUDED_
	#define _NEXMEDIADEF_H_INCLUDED_

#include "NexCommonDef.h"

#define NEXMEDIA_VERSION_MAJOR	1
#define NEXMEDIA_VERSION_MINOR	1
#define NEXMEDIA_VERSION_PATCH	2
#define NEXMEDIA_VERSION_BRANCH "OFFICIAL"

typedef enum 
{
	/*----------------------------------------------------------*/
	//	0xAABBBBBB
	//	AA : 대분류(Mainset, Subset)
	//	BBBBBB : 소분류
	/*----------------------------------------------------------*/

	/* Common					           						*/
	eNEX_FF_UNKNOWN				= 0x00000000,

	/*----------------------------------------------------------*/
	/* Main-Set Type											*/
	eNEX_FF_MP4					= 0x01000100,		// MPEG-4
	eNEX_FF_MP4MF				= 0x01000200,		// MP4 Movie Fragment FF.(기존 Smooth Streaming 이였음.)
	eNEX_FF_AVI					= 0x01000300,
	eNEX_FF_WAVE				= 0x01000400,		// WAVE
	eNEX_FF_MATROSKA 			= 0x01000500,		// Matroska(MKV)
	eNEX_FF_ASF					= 0x01000600,
	eNEX_FF_OGG					= 0x01000700,		// OGG
	eNEX_FF_MPEG_PS				= 0x01000800,		// MPEG-PS
	eNEX_FF_MPEG_TS				= 0x01000900,		// MPEG-TS
	eNEX_FF_FLV					= 0x01000A00,		// FLV
	eNEX_FF_RMFF 				= 0x01000B00,		// RealMedia
	eNEX_FF_FLAC 				= 0x01000C00,		// FLAC
	eNEX_FF_AMR					= 0x01000D00,		// AMR - NarrowBand
	eNEX_FF_AMRWB				= 0x01000E00,		// AMR - WideBand
	eNEX_FF_EVRC 				= 0x01000F00,
	eNEX_FF_QCELP				= 0x01001000,		// QCELP
	eNEX_FF_APE					= 0x01001100,		// Monkey's Audio (file format for lossless audio data compression)
	eNEX_FF_MP3					= 0x01001200,		// MPEG Audio layer 2/3
	eNEX_FF_ADIFAAC				= 0x01001300,		// AAC - Audio Data Interchange Format (ADIF)
	eNEX_FF_ADTSAAC				= 0x01001400,		// AAC - Audio Data Transport Stream (ADTS)
	eNEX_FF_AC3					= 0x01001500,
	eNEX_FF_EAC3 				= 0x01001600,		// Enhanced-AC3
	eNEX_FF_SUBTITLE			= 0x01001700,

	/*----------------------------------------------------------*/
	/* Sub-Set Type 											*/
	// ISOBMFF Family
	eNEX_FF_MP4_3GPP 			= 0x03000001,
	eNEX_FF_MP4_3GP2 			= 0X03000002,
	eNEX_FF_MP4_K3G				= 0x03000003,
	eNEX_FF_MP4_PIFF 			= 0x03000004,		// Protected Interoperable File Format (PIFF)
	eNEX_FF_MP4_OMA_BCAST_APDCF	= 0x03000005,
	eNEX_FF_MP4_OMA_PDCFV2		= 0x03000006,		//PDCF v2.0 JHKIM
	eNEX_FF_MP4_MOV				= 0x03000007,		// MOV (Apple Corp.)
	eNEX_FF_MP4_WMF				= 0x03000008,		// WMF
	eNEX_FF_MP4_SKM2 			= 0x03000009,
	eNEX_FF_MP4_AMC				= 0x0300000A,
	eNEX_FF_MP4_KDDI 			= 0x0300000B,
	eNEX_FF_MP4_DECE_CFF 		= 0x0300000C,		// Digital Entertainment Content Ecosystem Common File Format (DECE CFF)

	eNEX_FF_MKV_WEBM 			= 0x03000100,

	eNEX_FF_MPEG_TS_HLS			= 0x03000200,

	eNEX_FF_UNSUPPORT			= MAX_SIGNED32BIT
} NEX_FILEFORMAT_TYPE;

typedef enum 
{
	/*----------------------------------------------------------*/
	//	0xABBBCCDD
	//	A    : Video / Audio / Text Classification
	//	BBB  : 대분류
	//	CC   : 중분류
	//	DD   : 소분류
	/*----------------------------------------------------------*/
	eNEX_CODEC_UNKNOWN 				= 0x00000000,

	// Video Codec
	/*----------------------------------------------------------*/
	eNEX_CODEC_V_BEGINE				= 0x10000000,
	
	//H.xxx series
	eNEX_CODEC_V_H261				= 0x10010100,
	eNEX_CODEC_V_H263				= 0x10010200,
	eNEX_CODEC_V_H264			 	= 0x10010300,
	eNEX_CODEC_V_HEVC				= 0x10010400,
	
	// MPEG video series
	eNEX_CODEC_V_MPEG4V				= 0x10020100,
	eNEX_CODEC_V_MPEG1V				= 0x10020200,
	eNEX_CODEC_V_MPEG2V				= 0x10020300,
	
	// Microsoft
	eNEX_CODEC_V_MSMPEG4V1			= 0x10030100,
	eNEX_CODEC_V_MSMPEG4V2			= 0x10030200,
	eNEX_CODEC_V_MSMPEG4V3			= 0x10030300,
	
	//Divx
	eNEX_CODEC_V_DIVX				= 0x10040000,
	
	//Xvid
	eNEX_CODEC_V_XVID				= 0x10050000,	
	
	eNEX_CODEC_V_WMV 				= 0x10060000,	// Representation value
	eNEX_CODEC_V_WMV1				= 0x10060100,
	eNEX_CODEC_V_WMV2				= 0x10060200,
	eNEX_CODEC_V_WMV3				= 0x10060300,
	eNEX_CODEC_V_WVC1				= 0x10060400,
	
	eNEX_CODEC_V_MSRLE				= 0x10070000,
	
	eNEX_CODEC_V_MSVIDEO1			= 0x10080000,
	
	eNEX_CODEC_V_DVVIDEO	 		= 0x10090000,

	eNEX_CODEC_V_INDEO				= 0x100A0000,	// Representation value
	eNEX_CODEC_V_INDEO2				= 0x100A0100,
	eNEX_CODEC_V_INDEO3				= 0x100A0200,
	eNEX_CODEC_V_INDEO4				= 0x100A0300,
	eNEX_CODEC_V_INDEO5				= 0x100A0400,

	eNEX_CODEC_V_VP					= 0x100B0000,	// Representation value
	eNEX_CODEC_V_VP3 				= 0x100B0100,
	eNEX_CODEC_V_VP5 				= 0x100B0200,
	eNEX_CODEC_V_VP6 				= 0x100B0301,
	eNEX_CODEC_V_VP6A				= 0x100B0302,
	eNEX_CODEC_V_VP6F				= 0x100B0303,
	eNEX_CODEC_V_VP7 				= 0x100B0400,
	eNEX_CODEC_V_VP8 				= 0x100B0500,
	eNEX_CODEC_V_VP9 				= 0x100B0600,

	// Real Video
	eNEX_CODEC_V_REALV				= 0x100C0000,	// Representation value
	eNEX_CODEC_V_REALV01 			= 0x100C0100,
	eNEX_CODEC_V_REALV10 			= 0x100C0200,
	eNEX_CODEC_V_REALV13 			= 0x100C0300,
	eNEX_CODEC_V_REALV20 			= 0x100C0400,
	eNEX_CODEC_V_REALV30 			= 0x100C0500,
	eNEX_CODEC_V_REALV40 			= 0x100C0600,

	eNEX_CODEC_V_FLV 				= 0x100D0000,	// Representation value
	eNEX_CODEC_V_FLV1				= 0x100D0100,
	eNEX_CODEC_V_FLV4				= 0x100D0200,
	
	eNEX_CODEC_V_FLASHSV 			= 0x100E0100,

	eNEX_CODEC_V_RAWV				= 0x100F0100,
	
	eNEX_CODEC_V_ASV1				= 0x10100100,
	eNEX_CODEC_V_ASV2				= 0x10100200,

	eNEX_CODEC_V_QPEG				= 0x10110100,

	eNEX_CODEC_V_TRUEMOTION1		= 0x10120100,
	eNEX_CODEC_V_TRUEMOTION2		= 0x10120200,
	
	eNEX_CODEC_V_VCR1				= 0x10130100,
	
	eNEX_CODEC_V_FFV1				= 0x10140100,
	
	eNEX_CODEC_V_XXAN				= 0x10150100,
	
	eNEX_CODEC_V_CINEPAK			= 0x10160100,
	
	eNEX_CODEC_V_MSZH				= 0x10170100,
	
	eNEX_CODEC_V_ZLIB				= 0x10180100,
	
	eNEX_CODEC_V_SNOW				= 0x10190100,
	
	eNEX_CODEC_V_4XM 				= 0x101A0100,

	eNEX_CODEC_V_SVQ1				= 0X101B0100,
	
	eNEX_CODEC_V_TSCC				= 0X101C0100,
	
	eNEX_CODEC_V_ULTI				= 0X101D0100,	
	
	eNEX_CODEC_V_LOCO				= 0X101E0100,	
	
	eNEX_CODEC_V_WNV1				= 0X101F0100,	
	
	eNEX_CODEC_V_AASC				= 0X10200100,

	eNEX_CODEC_V_FRAPS				= 0X10210100,
	eNEX_CODEC_V_THEORA				= 0X10220100,
	eNEX_CODEC_V_CSCD				= 0X10230100,
	eNEX_CODEC_V_ZMBV				= 0X10240100,
	eNEX_CODEC_V_KMVC				= 0X10250100,
	eNEX_CODEC_V_CAVS				= 0X10260100,

	eNEX_CODEC_V_VMNC				= 0X10270100,

	eNEX_CODEC_V_SORENSONH263		= 0X10280100,
	eNEX_CODEC_V_SCRREENVIDEO		= 0X10290100,
	eNEX_CODEC_V_SCREEMVODEPV2		= 0X102A0100,
	
	// Apple Quick time
	eNEX_CODEC_V_QT					= 0X102B0100,

	// IntraPicture
	//NX_CODEC_V_INTRAPICTURE		= 0x102A0000,	// Representation value
	eNEX_CODEC_V_JPEG				= 0X102C0100,
	eNEX_CODEC_V_JPEG2000			= 0X102C0200,
	eNEX_CODEC_V_MJPEG				= 0X102C0300,
	eNEX_CODEC_V_STILL_IMG			= 0X102C0400,

	eNEX_CODEC_V_RGB24				= 0X102D0100,

	eNEX_CODEC_V_END	 			= 0x1FFFFFFF,
	/*----------------------------------------------------------*/

	// Audio Codec
	/*----------------------------------------------------------*/
	eNEX_CODEC_A_BEGINE				= 0x20000000,
	
	eNEX_CODEC_A_MPEGAUDIO			= 0x20010000,	// Representation value
	eNEX_CODEC_A_MP2 				= 0x20010100,
	eNEX_CODEC_A_MP3 				= 0x20010200,
	eNEX_CODEC_A_MP4A				= 0x20010300,

	eNEX_CODEC_A_AAC				= 0x20020000,	// Representation value
	eNEX_CODEC_A_AACPLUS			= 0x20020100,	// High-Efficiency AAC (AKA -> AAC+, enahnced AAC)
	eNEX_CODEC_A_AACPLUSV2			= 0x20020101,	// High-Efficiency AAC version 2(AKA -> AAC++, enhanced AAC+)
	eNEX_CODEC_A_ELDAAC				= 0x20020200,	// Enhanced Low Delay - AAC

	eNEX_CODEC_A_AC3 				= 0x20030000,	// Representation value
	eNEX_CODEC_A_EAC3				= 0x20030100,	// Enhanced AC3

	eNEX_CODEC_A_DTS 				= 0x20040000,	// Representation value
	eNEX_CODEC_A_DTS_DTSC			= 0x20040100,	// DTS formats prior to DTS-HD
	eNEX_CODEC_A_DTS_DTSH			= 0x20040200,	// DTS-HD audio formats
	eNEX_CODEC_A_DTS_DTSE			= 0x20040300,	// DTS-HD low bit rate foramts
	eNEX_CODEC_A_DTS_DTSL			= 0x20040400,	// DTS-HD Lossless formats

	eNEX_CODEC_A_PCM 				= 0x20050000,	// Representation value
	eNEX_CODEC_A_PCM_S16LE			= 0x20050100,
	eNEX_CODEC_A_PCM_S16BE			= 0x20050200,
	eNEX_CODEC_A_PCM_RAW			= 0x20050300,	// raw,  k8BitOffsetBinary-Format
	eNEX_CODEC_A_PCM_FL32LE			= 0x20050400,	// fl32, kFloat32Format, Little endian
	eNEX_CODEC_A_PCM_FL32BE			= 0x20050401,	// fl32, kFloat32Format, Big endian
	eNEX_CODEC_A_PCM_FL64LE			= 0x20050500,	// fl64, kFloat64Format, Little endian
	eNEX_CODEC_A_PCM_FL64BE			= 0x20050501,	// fl64, kFloat64Format, Big endian
	eNEX_CODEC_A_PCM_IN24LE			= 0x20050600,	// in24, k24BitFormat, Little endian
	eNEX_CODEC_A_PCM_IN24BE			= 0x20050601,	// in24, k24BitFormat, Big endian
	eNEX_CODEC_A_PCM_IN32LE			= 0x20050700,	// in32, k32BitFormat, Little endian
	eNEX_CODEC_A_PCM_IN32BE			= 0x20050701,	// in32, k32BitFormat, Big endian
	
	eNEX_CODEC_A_PCM_LPCMLE			= 0x20050800,	// lpcm, kAudioFormatLinearPCM, Little endian
	eNEX_CODEC_A_PCM_LPCMBE			= 0x20050801,	// lpcm, kAudioFormatLinearPCM, Big endian

	//eNEX_CODEC_A_ADPCM 			= 0x20060000,	// Representation value
	eNEX_CODEC_A_ADPCM_MS			= 0x20060100,
	//eNEX_CODEC_A_ADPCM_IMA 		= 0x20060200,	// Sub Representation value
	eNEX_CODEC_A_ADPCM_IMA_WAV		= 0x20060201,
	eNEX_CODEC_A_ADPCM_IMA_DK4		= 0x20060202,
	eNEX_CODEC_A_ADPCM_IMA_DK3		= 0x20060203,
	eNEX_CODEC_A_ADPCM_IMA4			= 0x20060204, //IMA-ADPCM, Quick Time File Format
	eNEX_CODEC_A_ADPCM_YAMAHA		= 0x20060300,
	eNEX_CODEC_A_ADPCM_G726			= 0x20060400,
	eNEX_CODEC_A_ADPCM_CT			= 0x20060500,
	eNEX_CODEC_A_ADPCM_SWF			= 0x20060600,

	eNEX_CODEC_A_WMA 				= 0x20070000,	// Representation value
	eNEX_CODEC_A_WMA1				= 0x20070100,
	eNEX_CODEC_A_WMA2				= 0x20070200,
	eNEX_CODEC_A_WMA3				= 0x20070300,
	eNEX_CODEC_A_WMA_LOSSLESS		= 0x20070400,
	eNEX_CODEC_A_WMA_SPEECH			= 0x20070500,
	eNEX_CODEC_A_WMASPDIF			= 0x20070600,
	
	eNEX_CODEC_A_VORBIS				= 0x20080100,
	eNEX_CODEC_A_SONIC				= 0x20090100,
	eNEX_CODEC_A_TRUESPEECH			= 0x200A0100,
	eNEX_CODEC_A_FLAC				= 0x200B0100,
	eNEX_CODEC_A_APE 				= 0x200C0100,
	eNEX_CODEC_A_IMC 				= 0x200D0100,
	
	eNEX_CODEC_A_REALA				= 0x200E0000,
	eNEX_CODEC_A_REAL14_4			= 0x200E0100,
	eNEX_CODEC_A_REAL28_8			= 0x200E0200,
	eNEX_CODEC_A_ATRC				= 0x200F0100,
	eNEX_CODEC_A_COOK				= 0x20100100,
	eNEX_CODEC_A_DNET				= 0x20110100,
	eNEX_CODEC_A_SIPR				= 0x20120100,
	eNEX_CODEC_A_RAAC				= 0x20130100,
	eNEX_CODEC_A_RACP				= 0x20140100,
	
	eNEX_CODEC_A_ADUU 				= 0x20150100,

	// QDesign
	eNEX_CODEC_A_QDMC				= 0x20160100,
	eNEX_CODEC_A_QDMC2				= 0x20160200,
	
	//FLV 관련 Audio Codec
	eNEX_CODEC_A_FLV_ID_3			= 0x20170100,	// Linear PCM, little endian
	eNEX_CODEC_A_FLV_ID_4			= 0x20170200,	// Nellymoser 16K mono
	eNEX_CODEC_A_FLV_ID_5			= 0x20170300,	// Nellymoser 8K mono
	eNEX_CODEC_A_FLV_ID_6			= 0x20170400,	// Nellymoser 
	eNEX_CODEC_A_FLV_ID_7			= 0x20170500,	// G.711 A-law logarithmic PCM
	eNEX_CODEC_A_FLV_ID_8			= 0x20170600,	// G.711 mu-law logarithmic PCM
	eNEX_CODEC_A_FLV_ID_11			= 0x20170700,	// Sppex
	eNEX_CODEC_A_FLV_ID_14			= 0x20170800,	// device-specific sound

	//Voice codec
	eNEX_CODEC_A_AMR 				= 0x20180000,	// Representation value and -NB
	eNEX_CODEC_A_AMRWB				= 0x20180100,
	eNEX_CODEC_A_EAMRWB				= 0x20180200,

	eNEX_CODEC_A_SMV 				= 0x20190100,
	eNEX_CODEC_A_QCELP				= 0x201A0000,	// Representation value
	eNEX_CODEC_A_QCELP_ALT			= 0x201A0100,
	eNEX_CODEC_A_EVRC				= 0x201B0100,
	eNEX_CODEC_A_G711				= 0x201C0000,	// Representation value, G.711 사용 안함. 아래 A-LAW 및 MU-LAW 사용.
	eNEX_CODEC_A_G711_ALAW			= 0x201C0100,	
	eNEX_CODEC_A_G711_MULAW			= 0x201C0200,
	eNEX_CODEC_A_G723				= 0x201D0100,

	eNEX_CODEC_A_BSAC				= 0x201E0100,
	eNEX_CODEC_A_ALAC				= 0x201F0100,	// Apple lossless (ALAC)
	eNEX_CODEC_A_GSM610				= 0x20200100,	//_WAVE_CODEC_GSM_610_	= 0x0031,	// 49 (0x0031) GSM 6.10
	eNEX_CODEC_A_BLURAYPCM			= 0x20210100,

	eNEX_CODEC_A_DRA				= 0x20220100,

	eNEX_CODEC_A_END 				= 0x2FFFFFFF,
	/*----------------------------------------------------------*/
		
	//Text Codec
	/*----------------------------------------------------------*/
	eNEX_CODEC_T_BEGINE				= 0x30000000,
	
	eNEX_CODEC_T_3GPP				= 0x30010100,
	eNEX_CODEC_T_SKT 				= 0x30020100,
	eNEX_CODEC_T_SMI 				= 0x30030100,
	eNEX_CODEC_T_SRT 				= 0x30040100,
	
	//eNEX_CODEC_T_ID3_LYRIC 		= 0x30050000,	// Representation value
	eNEX_CODEC_T_ID3_UNSYNC_LYRIC	= 0x30050100,
	eNEX_CODEC_T_ID3_SYNC_LYRIC		= 0x30050200,
	eNEX_CODEC_T_VOB_SUB			= 0x30060100,
	eNEX_CODEC_T_MICRODVD_SUB		= 0x30070100,
	eNEX_CODEC_T_LRC 				= 0x30080100,
	eNEX_CODEC_T_SSA 				= 0x30090100,
	eNEX_CODEC_T_SYLT				= 0x300A0100,
	eNEX_CODEC_T_TTML				= 0x300B0100,
	eNEX_CODEC_T_TT					= 0x300B0200,	// Timed-Text
	eNEX_CODEC_T_WEB_VTT 			= 0x300C0100,
	
	eNEX_CODEC_T_CC_CEA				= 0x300D0100,
	eNEX_CODEC_T_CC_CEA_608			= 0x300D0200,
	eNEX_CODEC_T_CC_CEA_708			= 0x300D0300,
	
	eNEX_CODEC_T_DIVX_XSUB			= 0x300E0000,	// Representation value
	eNEX_CODEC_T_DIVX_XSUBPLUS		= 0x300E0100,

	eNEX_CODEC_T_UTF				= 0x300F0000, 	// Representation value
	eNEX_CODEC_T_UTF8				= 0x300F0100,
	eNEX_CODEC_T_UTF16				= 0x300F0200,
	eNEX_CODEC_T_UTF32				= 0x300F0300,	
	
	eNEX_CODEC_T_TIMEDMETA			= 0x30100000, 	// Representation value
	
	eNEX_CODEC_T_END				= 0x3FFFFFFF,
	/*----------------------------------------------------------*/
	
	eNEX_CODEC_PADDING 				= MAX_SIGNED32BIT
} NEX_CODEC_TYPE;

// Text Encoding type define
typedef enum {
	eNEX_TEXT_ENCODING_UNKNOWN		= 0x00000000,

	eNEX_TEXT_ENCODING_UNICODE		= 0x10000000,
	eNEX_TEXT_ENCODING_UTF8			= 0x10000010,
	eNEX_TEXT_ENCODING_UTF16		= 0x10000020,	// 16bits Little endian
	eNEX_TEXT_ENCODING_UTF16_BE		= 0x10000030,	// 16bits Big endian
	eNEX_TEXT_ENCODING_UTF32		= 0x10000040,	// 32bits Little endian
	eNEX_TEXT_ENCODING_UTF32_BE		= 0x10000050,	// 32bits Big endian

	eNEX_TEXT_ENCODING_ASCII		= 0x20000000,

	eNEX_TEXT_ENCODING_ISO_8859		= 0x30000000,
	eNEX_TEXT_ENCODING_ISO_8859_1	= 0x30000010,

	eNEX_TEXT_ENCODING_PADDING		= MAX_SIGNED32BIT
	
}NEX_TEXT_ENCODING_TYPE;

PACKED(
struct _NEX_WAVEFORMATEX
{
	NXUINT32	nCodecID;		// 원래는 16bits 이나, FOURCC를 저장하기 위해 32bits 로 사용.
	NXUINT32	nSamplePerSecond;
	NXUINT32	nAvrByte;		// Average byte.
	NXUINT32	nSampleNumber;	// 원래는 WaveFormat EX에 없는 값.
	NXUINT16	nChannel;		// Byte align 때문에 위치변경
	NXUINT16	nBlockAlignment;
	NXUINT16	nBitPerSample;
	NXUINT16	nExtraDataLen;
	NXUINT8		*pExtraData;
} 
);

typedef struct _NEX_WAVEFORMATEX NEX_WAVEFORMATEX;

PACKED(
struct _NEX_BITMAPINFOHEADER
{
	NXUINT32	nFormatDataSize;
	NXUINT32	nImgWidth;
	NXUINT32	nImgHeight;
	NXUINT16	nPlanes;
	NXUINT16	nBitPerPixel;
	NXUINT32	nCompressionID; 			// Compression ID
	NXUINT32	nImageSize;
	NXUINT32	nHorizontalPixelPerMeter;
	NXUINT32	nVerticalPixelPerMeter;
	NXUINT32	nColorCnt;
	NXUINT32	nImportantColorCnt;
	NXUINT32	nExtraDataLen;
	NXUINT8		*pExtraData;
}
);

typedef struct _NEX_BITMAPINFOHEADER NEX_BITMAPINFOHEADER;



PACKED(
struct	_NEX_ID3TEXTINFO
{
	NXCHAR*						pData;					// Text Data.
	NXUINT32					uSize;					// Size of text data. (in bytes)
	NEX_TEXT_ENCODING_TYPE			eEncodingType;			// Encoding Type.
}
);
typedef struct _NEX_ID3TEXTINFO NEX_ID3TEXTINFO;


PACKED(
struct	_NEX_ID3PICTUREINFO
{
	NXINT8*						pData;					// Text Data.
	NXUINT32					uSize;					// Size of text data. (in bytes)
	NEX_ID3TEXTINFO				stMIMEType;
}
);
typedef struct _NEX_ID3PICTUREINFO NEX_ID3PICTUREINFO;

//Extra ID3 Tag Element
PACKED(
struct _NEX_ID3TAGDATA
{
	NXCHAR							aTagID[4];		//ID3 Tag ID
	NXBOOL							bPicture;		// 1: NEX_ID3PICTUREINFO	
	union
	{
		NEX_ID3TEXTINFO					stText;
		NEX_ID3PICTUREINFO				stPicture;		
	}UID3Data;
}
);
typedef struct _NEX_ID3TAGDATA NEX_ID3TAGDATA;

PACKED(
struct _NEX_TOTALID3
{
	NEX_ID3TEXTINFO					stTitle;
	NEX_ID3TEXTINFO					stArtist;
	NEX_ID3TEXTINFO					stAlbum;
	NEX_ID3TEXTINFO					stAlbumArtist;
	NEX_ID3TEXTINFO					stGenre;
	NEX_ID3TEXTINFO					stLyric;
	NEX_ID3TEXTINFO					stYear;
	NEX_ID3TEXTINFO					stTrackNum;		
	NEX_ID3TEXTINFO					stComment;
	NEX_ID3TEXTINFO					stText;
	NEX_ID3TEXTINFO					stPrivateFrame;
	NEX_ID3PICTUREINFO				stPicture;
	NXUINT32						uExtraID3TagDataCount;
	NEX_ID3TAGDATA*					pExtraID3TagData;		//Extra ID3 Tag
}
);
typedef struct _NEX_TOTALID3 NEX_TOTALID3;


#endif //_NEXMEDIADEF_H_INCLUDED_

