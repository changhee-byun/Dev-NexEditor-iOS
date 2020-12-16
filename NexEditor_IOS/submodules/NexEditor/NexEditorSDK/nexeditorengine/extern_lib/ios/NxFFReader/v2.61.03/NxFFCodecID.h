/******************************************************************************
   __________________________________________________________________________
  |                                                                          |
  |                                                                          |
  |    	  Copyrightⓒ 2002-2018 NexStreaming Corp. All rights reserved.      |
  |                        http://www.nexstreaming.com                       |
  |                                                                          |
  |__________________________________________________________________________|

*******************************************************************************
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
_______________________________________________________________________________
*
*		File Name			: NxFFCodecID.h
*		Author				: NexStreaming MiddleWare Team (mw@nexstreaming.com)
*		Description			:
*		Revision History	: 
*		Comment				: 
*
******************************************************************************/

#ifndef _NXFFCODECID_H_
#define _NXFFCODECID_H_


// H264(AVC) codec support bitstream type
#define NXFFREADER_AVC_SAMPLE_STREAM	0x00000000
#define NXFFREADER_AVC_BYTE_STREAM		0x00000001

// for Meta data
enum {
	NXFF_METADATA_UNSUPPORT = 0x00000000,
	NXFF_METADATA_ID3		= 0x00001000,
	NXFF_METADATA_PADDING	= 0xFFFFFFFF
};

#define	FLAG_NXFF_READER_SUPPORT_ALL			0xFFFFFFFF
#define FLAG_NXFF_READER_SUPPORT_MP4			0x00000001		// 
#define	FLAG_NXFF_READER_SUPPORT_SPEECH			0x00000002
#define	FLAG_NXFF_READER_SUPPORT_ADIFAAC		0x00000008
#define	FLAG_NXFF_READER_SUPPORT_ADTSAAC		0x00000020		// ADTS는 지원 하지 않는다.
#define	FLAG_NXFF_READER_SUPPORT_MP3			0x00000010
#define	FLAG_NXFF_READER_SUPPORT_AVI			0x00000040
#define	FLAG_NXFF_READER_SUPPORT_ASF			0x00000080
#define FLAG_NXFF_READER_SUPPORT_RMFF			0x00000100
#define FLAG_NXFF_READER_SUPPORT_MATROSKA		0x00000200
#define FLAG_NXFF_READER_SUPPORT_OGG			0x00001000
#define FLAG_NXFF_READER_SUPPORT_FLAC			0x00002000
#define FLAG_NXFF_READER_SUPPORT_APE			0x00004000
#define FLAG_NXFF_READER_SUPPORT_MPEGPS			0x00008000
#define FLAG_NXFF_READER_SUPPORT_MPEGTS			0x00010000
#define FLAG_NXFF_READER_SUPPORT_EXTFF			0x00020000		// External Lib를 이용한 File Parser	
#define FLAG_NXFF_READER_SUPPORT_WAVE			0x00040000	
#define FLAG_NXFF_READER_SUPPORT_FLV			0x00080000
#define FLAG_NXFF_READER_SUPPORT_QCELP			0x00100000
#define FLAG_NXFF_READER_SUPPORT_MP4MF			0x00200000		// MP4 (Smooth Streaming) Movie Fragment
#define FLAG_NXFF_READER_SUPPORT_AC3			0x00400000		// AC3
#define FLAG_NXFF_READER_SUPPORT_EAC3			0x00800000		// Enhanced AC3
#define FLAG_NXFF_READER_SUPPORT_MPEGTSDEMUX	0x01000000
#define FLAG_NXFF_READER_SUPPORT_SUBTITLE		0x10000000		// Subtitle

/*
// FourCC define
const NXFF_CODEC_4CC_TYPE g_NxFFVideoCodecID[] = {
	// MPEG4
	{NXFF_CODEC_V_MPEG4, 0x464D5034},			// FMP4: FFMpeg MPEG-4
	{NXFF_CODEC_V_MPEG4, 0x44495658},			// DIVX: OpenDivX (DivX 4.0 and later)
	{NXFF_CODEC_V_MPEG4, 0x44583530},			// DX50: DivX 5.0 codec
	{NXFF_CODEC_V_MPEG4, 0x58564944},			// XVID: XviD MPEG-4 codec
	{NXFF_CODEC_V_MPEG4, 0x78766964},			// xvid: XviD MPEG-4 codec
	{NXFF_CODEC_V_MPEG4, 0x4D503453},			// MP4S: Microsoft MPEG-4 (Windows Media 7.0)
	{NXFF_CODEC_V_MPEG4, 0x4D345332},			// M4S2: Microsoft MPEG-4 (hacked MS MPEG-4)
	{NXFF_CODEC_V_MPEG4, 0x44495631},			// DIV1: FFmpeg-4 V1 (hacked MS MPEG-4 V1)
	{NXFF_CODEC_V_MPEG4, 0x424C5A30},			// BLZ0: DivX for Blizzard Decoder Filter
	{NXFF_CODEC_V_MPEG4, 0x4D503456},			// MP4V
	{NXFF_CODEC_V_MPEG4, 0x6D703476},			// mp4v: Apple QuickTime MPEG-4 native
	{NXFF_CODEC_V_MPEG4, 0x554D5034},			// UMP4: UB Video MPEG 4
	{NXFF_CODEC_V_MPEG4, 0x57563146},			// WV1F: WV1F codec (supported by ffdshow)
	{NXFF_CODEC_V_MPEG4, 0x53454447},			// SEDG: Samsung MPEG-4 codec
	{NXFF_CODEC_V_MPEG4, 0x524D5034},			// RMP4: REALmagic MPEG-4 Video Codec (Sigma Design, built on XviD)
	{NXFF_CODEC_V_MPEG4, 0x46564657},			// FVFW: ff MPEG-4 based on XviD codec
	{NXFF_CODEC_V_MPEG4, 0x33495632},			// 3IV2: MPEG4-based codec 3ivx

	// H261
	{NXFF_CODEC_V_H261,  0x48323631},			// H261: Intel ITU H.261
	// H263
	{NXFF_CODEC_V_H263, 0x48323633},			// H263: Intel ITU H.263
	{NXFF_CODEC_V_H263, 0x68323633},			// h263: Intel ITU H.263
	{NXFF_CODEC_V_H263, 0x49323633},			// I263: Intel ITU H.263
	{NXFF_CODEC_V_H263, 0x75323633},			// u263: UB Video StreamForce H.263
	{NXFF_CODEC_V_H263, 0x76697631},			// viv1: Vivo H.263
	// H264
	{NXFF_CODEC_V_H264, 0x48323634},			// H264: Intel ITU H.264
	{NXFF_CODEC_V_H264, 0x68323634},			// h264: Intel ITU H.264
	{NXFF_CODEC_V_H264, 0x78323634},			// x264: XiWave GNU GPL x264 MPEG-4 Codec
	{NXFF_CODEC_V_H264, 0x61766331},			// avc1: AVC1 codec (supported by ffdshow)
	{NXFF_CODEC_V_H264, 0x56535348},			// VSSH: Vanguard VSS H.264
	// MS MPEG4 V1
	{NXFF_CODEC_V_MSMPEG4V1, 0x4D504734},		// MPG4: Microsoft MPEG-4 V1
	// MS MPEG4 V2
	{NXFF_CODEC_V_MSMPEG4V2, 0x4D503432},		// MP42: Microsoft MPEG-4 V2
	// MS MPEG4 V3
	{NXFF_CODEC_V_MSMPEG4V3, 0x4D504733},		// MPG3: Same as Low motion DivX MPEG-4 
	{NXFF_CODEC_V_MSMPEG4V3, 0x44495633},		// DIV3: Same as Low motion DivX MPEG-4 
	{NXFF_CODEC_V_MSMPEG4V3, 0x4D503433},		// MP43: Same as Low motion DivX MPEG-4 
	{NXFF_CODEC_V_MSMPEG4V3, 0x44495635},		// DIV5: DivX MPEG-4
	{NXFF_CODEC_V_MSMPEG4V3, 0x44495636},		// DIV6: DivX MPEG-4
	{NXFF_CODEC_V_MSMPEG4V3, 0x44495634},		// DIV4: Fast motion DivX MPEG-4 
	{NXFF_CODEC_V_MSMPEG4V3, 0x41503431},		// AP41: AngelPotion Definitive (hack MS MP43)
	{NXFF_CODEC_V_MSMPEG4V3, 0x434F4C31},		// COL1: FFmpeg DivX ;-) (MS MPEG-4 v3)
	{NXFF_CODEC_V_MSMPEG4V3, 0x434F4C30},		// COL0: FFmpeg DivX ;-) (MS MPEG-4 v3)
	// WMV
	{NXFF_CODEC_V_WMV1, 0x574D5631},			// WMV1: Windows Media Video V7 
	{NXFF_CODEC_V_WMV2, 0x574D5632},			// WMV2: Windows Media Video V8
	{NXFF_CODEC_V_WMV3, 0x574D5633},			// WMV3: Windows Media Video V9
	{NXFF_CODEC_V_WVC1, 0x57564331},			// WVC1: the SMPTE 421M video codec standard initially developed by Microsoft

	// MS
	{NXFF_CODEC_V_MSRLE, 0x6D726C65},			// mrle: Microsoft Run Length Encoding
	{NXFF_CODEC_V_MSVIDEO1, 0x4D535643},		// MSVC: Microsoft Video 1
	{NXFF_CODEC_V_MSVIDEO1, 0x6D737663},		// msvc: Microsoft Video 1
	{NXFF_CODEC_V_MSVIDEO1, 0x4352414D},		// CRAM: Microsoft Video 1
	{NXFF_CODEC_V_MSVIDEO1, 0x6372616D},		// cram: Microsoft Video 1
	{NXFF_CODEC_V_MSVIDEO1, 0x5748414D},		// WHAM: Microsoft Video 1
	{NXFF_CODEC_V_MSVIDEO1, 0x7768616D},		// wham: Microsoft Video 1

	// DVVIDEO
	{NXFF_CODEC_V_DVVIDEO, 0x64767364},		// dvsd: Sony Digital Video (DV) 525 lines at 29.97 Hz or 625 lines at 25.00 Hz
	{NXFF_CODEC_V_DVVIDEO, 0x64766864},		// dvhd: DV 1125 lines at 30.00 Hz or 1250 lines at 25.00 Hz
	{NXFF_CODEC_V_DVVIDEO, 0x64767331},		// dvs1
	{NXFF_CODEC_V_DVVIDEO, 0x64763235},		// dv25: Matrox DVCPRO codec 
	// MPEG1,2
	{NXFF_CODEC_V_MPEG1V, 0x6D706731},		// mpg1: FFmpeg-1
	{NXFF_CODEC_V_MPEG1V, 0x50494D31},		// PIM1: Pinnacle DC1000 hardware codec (MPEG compression) 
	{NXFF_CODEC_V_MPEG1V, 0x56435232},		// VCR2: ATI VCR 2.0 (MPEG YV12)
	{NXFF_CODEC_V_MPEG1V, 0x10000001},		// 
	{NXFF_CODEC_V_MPEG1V, 0x4D504547},		// MPEG: Chromatic MPEG 1 Video I Frame 
	{NXFF_CODEC_V_MPEG2V, 0x6D706732},		// mpg2: FFmpeg-1 
	{NXFF_CODEC_V_MPEG2V, 0x10000002},		// 
	{NXFF_CODEC_V_MPEG2V, 0x44565200},		// DVR
	//RAWV
	//{NXFF_CODEC_V_RAWV, 0x00000000},			//
	{NXFF_CODEC_V_RAWV, 0x49343230},			// I420: Intel Indeo 4 H.263 
	{NXFF_CODEC_V_RAWV, 0x59555632},			// YUV2: Apple Component Video (YUV 4:2:2)
	{NXFF_CODEC_V_RAWV, 0x59343232},			// Y422: Direct copy of UYVY as used by ADS Technologies Pyro WebCam firewire camera  
	{NXFF_CODEC_V_RAWV, 0x59563132},			// YV12: ATI YVU12 4:2:0 Planar 
	{NXFF_CODEC_V_RAWV, 0x49595556},			// IYUV: Intel Indeo iYUV 4:2:0    
	{NXFF_CODEC_V_RAWV, 0x59383030},			// Y800: Simple grayscale video  
	// INDEO3
	{NXFF_CODEC_V_INDEO3, 0x49563331},		// IV31: Intel Indeo Video 3.1 
	{NXFF_CODEC_V_INDEO3, 0x49563332},		// IV32: Intel Indeo Video 3.2
	{NXFF_CODEC_V_INDEO5, 0x49563530},		// IV50: Intel Indeo Video 5.0 Wavelet
	// VP
	{NXFF_CODEC_V_VP3, 0x76703330},			// VP30: On2 (ex Duck TrueMotion) VP3  
	{NXFF_CODEC_V_VP3, 0x76703331},			// VP31: On2 (ex Duck TrueMotion) VP3  
	{NXFF_CODEC_V_VP5, 0x76703530},			// VP50: On2 TrueCast VP5 
	{NXFF_CODEC_V_VP6, 0x76703630},			// VP60: On2 TrueCast VP6
	{NXFF_CODEC_V_VP6, 0x76703631},			// VP61: On2 TrueCast VP6.1
	{NXFF_CODEC_V_VP6, 0x76703632},			// VP62: On2 TrueCast VP6.2

	// ASV
	{NXFF_CODEC_V_ASV1, 0x41535631},			// ASV1: Asus Video V1
	{NXFF_CODEC_V_ASV2, 0x41535632},			// ASV2: Asus Video V2
	//QPEG
	{NXFF_CODEC_V_QPEG, 0x51504547},			// QPEG: Q-Team QPEG 1.1
	{NXFF_CODEC_V_QPEG, 0x51312E30},			// Q1.0: Q-Team QPEG 1.0 (www.q-team.de)
	{NXFF_CODEC_V_QPEG, 0x51312E31},			// Q1.1: Q-Team QPEG 1.1 (www.q-team.de)
	// TRUE Motion
	{NXFF_CODEC_V_TRUEMOTION1, 0x4455434B},		// DUCK: True Motion 1.0 
	{NXFF_CODEC_V_TRUEMOTION2, 0x544D3230},		// TM20: Duck TrueMotion 2.0
	// FLASH
	{NXFF_CODEC_V_FLV1, 0x464C5631},			// FLV1: FLV1 codec (supported by ffdshow)
	{NXFF_CODEC_V_FLV4, 0x464C5634},			// FLV4: FLV4 codec (supported by ffdshow)
	{NXFF_CODEC_V_FLASHSV, 0x46535631},		// FSV1: Flash Screen Video

	// ETC
	{NXFF_CODEC_V_VCR1, 0x56435231},			// VCR1: ATI VCR 1.0  
	{NXFF_CODEC_V_FFV1, 0x46465631},			// FFV1: FFDShow supported
	{NXFF_CODEC_V_XXAN, 0x5878616E},			// Xxan: Origin Video Codec (used in Wing Commander 3 and 4) 
	{NXFF_CODEC_V_CINEPAK,0x63766964 },		// cvid: Cinepak by CTi (ex. Radius) Vector Quantization
	{NXFF_CODEC_V_MSZH, 0x4D535A48},			// MSZH: Lossless codec (ZIP compression)
	{NXFF_CODEC_V_ZLIB, 0x5A4C4942},			// ZLIB: Lossless codec (ZIP compression)
	{NXFF_CODEC_V_SNOW, 0x534E4F57},			// SNOW: SNOW codec (supported by ffdshow) 
	{NXFF_CODEC_V_4XM, 0x34584D56},			// 4XMV: 4xm Video codec
	{NXFF_CODEC_V_VP6F, 0x56503646},			// VP6F: Flash Video
	{NXFF_CODEC_V_SVQ1, 0x73767131},			// svq1: Sorenson Video (Apple Quicktime 3) 
	{NXFF_CODEC_V_TSCC, 0x74736363},			// tscc: TechSmith Screen Capture Codec  
	{NXFF_CODEC_V_ULTI, 0x554C5449},			// ULTI: IBM Corp. Ultimotion 
	{NXFF_CODEC_V_LOCO, 0x4C4F434F},			// LOCO: LOCO Lossless Codec
	{NXFF_CODEC_V_WNV1, 0x574E5631},			// WNV1: WinNow Videum Hardware Compression
	{NXFF_CODEC_V_AASC, 0x41415343},			// AASC: Autodesk Animator codec (RLE)
	{NXFF_CODEC_V_INDEO2, 0x52543231},		// RT21: Intel Real Time Video 2.1
	{NXFF_CODEC_V_FRAPS, 0x46505331},			// FPS1: Fraps
	{NXFF_CODEC_V_THEORA, 0x7468656F},		// theo: FFVFW Supported Codec
	{NXFF_CODEC_V_CSCD, 0x43534344},			// CSCD: RenderSoft CamStudio lossless Codec (LZO & GZIP compression)
	{NXFF_CODEC_V_ZMBV, 0x5A4D4256},			// ZMBV: DosBox Capture Codec
	{NXFF_CODEC_V_KMVC, 0x4B4D5643},			// KMVC: Karl Morton Video Codec
	{NXFF_CODEC_V_CAVS, 0x43415653},			// CAVS:
	{NXFF_CODEC_V_MJPEG, 0x4D4A5047},			// MJPG: Motion JPEG including Huffman Tables
	{NXFF_CODEC_V_MJPEG, 0x6D6A7067},			// mjpg: Motion JPEG including Huffman Tables
	{NXFF_CODEC_V_JPEG2000, 0x4D4A3243},		// MJ2C: Morgan Multimedia JPEG2000 Compression
	{NXFF_CODEC_V_JPEG2000, 0x6a706567},		// jpeg: Morgan Multimedia JPEG2000 Compression
	{NXFF_CODEC_V_VMNC, 0x564D6E63},			// VMnc: VMware Video

	// Real
	{NXFF_CODEC_V_REALV01, FCC_CODEC_RV01},
	{NXFF_CODEC_V_REALV13, FCC_CODEC_RV13},
	{NXFF_CODEC_V_REALV10, FCC_CODEC_RV10},
	{NXFF_CODEC_V_REALV20, FCC_CODEC_RV20},
	{NXFF_CODEC_V_REALV30, FCC_CODEC_RV30},
	{NXFF_CODEC_V_REALV40, FCC_CODEC_RV40},

	// 여기서부터의 4CC값은 그냥 임의로 넣은 것 이다.
	{NXFF_CODEC_V_SORENSONH263, 0x73697631},	// s263 
	{NXFF_CODEC_V_SCRREENVIDEO, 0x70000002},
	{NXFF_CODEC_V_VP6A,			0x70000003},
	{NXFF_CODEC_V_SCREEMVODEPV2, 0x70000004},
	{0x00, 0x00}
};

const NXFF_CODEC_4CC_TYPE g_NxFFAudioCodecID[] = {
	{NXFF_CODEC_A_MP2, 0x50},
	{NXFF_CODEC_A_MP3, 0x55},
	{NXFF_CODEC_A_MP3, 0x4c414d45},			// LAME
	{NXFF_CODEC_A_MP3, 0x4d503300},			// MP3
	{NXFF_CODEC_A_AC3, 0x2000},
	{NXFF_CODEC_A_DTS, 0x2001},
	{NXFF_CODEC_A_PCM_S16LE, 0x01},	// PCM_S16LE, PCMU8, PCMS24LE, PCM32LE, RAWA
	{NXFF_CODEC_A_ADPCM_MS, 0x02},
	{NXFF_CODEC_A_ASPCM_IMA_WAV, 0x11},
	{NXFF_CODEC_A_ASPCM_IMA_DK4, 0x61},
	{NXFF_CODEC_A_ASPCM_IMA_DK3, 0x62},
	{NXFF_CODEC_A_ADPCM_YAMAHA, 0x20},
	{NXFF_CODEC_A_ADPCM_G726, 0x45},
	{NXFF_CODEC_A_ADPCM_CT, 0x200},
	{NXFF_CODEC_A_ADPCM_SWF, 0x5346},
	{NXFF_CODEC_A_WMAV1, 0x160},
	{NXFF_CODEC_A_WMAV2, 0x161},
	{NXFF_CODEC_A_WMAV3, 0x162},
	{NXFF_CODEC_A_AAC, 0x706d},
	{NXFF_CODEC_A_AAC, 0xff},
	{NXFF_CODEC_A_VORBIS, 0x566f},
	{NXFF_CODEC_A_SONIC, 0x2048},		// SONIC_LS
	{NXFF_CODEC_A_TRUESPEECH, 0x22},
	{NXFF_CODEC_A_FLAC, 0xF1AC},
	{NXFF_CODEC_A_IMC, 0x401},

	// REAL
	{NXFF_CODEC_A_REAL14_4, FCC_CODEC_144},
	{NXFF_CODEC_A_REAL28_8, FCC_CODEC_288},
	{NXFF_CODEC_A_ATRC, FCC_CODEC_atrc},
	{NXFF_CODEC_A_COOK, FCC_CODEC_cook},
	{NXFF_CODEC_A_DNET, FCC_CODEC_dnet},
	{NXFF_CODEC_A_SIPR, FCC_CODEC_sipr},
	{NXFF_CODEC_A_RACC, FCC_CODEC_raac},
	{NXFF_CODEC_A_RACP, FCC_CODEC_racp},
	{NXFF_CODEC_A_MP4A, FCC_CODEC_MP4A},
	{NAFF_CODEC_A_aduU, FCC_CODEC_aduU},

	// QDesign
	{NXFF_CODEC_A_QDMC, 0x51444d43},
	{NXFF_CODEC_A_QDMC2, 0x51444d32},


	// 여기서부터의 4CC값은 그냥 임의로 넣은 것 이다.
	{NXFF_CODEC_A_APE, 0x70000001},
	{NXFF_CODEC_A_FLV_ID_3, 0x70000002},
	{NXFF_CODEC_A_FLV_ID_4, 0x70000003},		// Nellymoser 16K mono
	{NXFF_CODEC_A_FLV_ID_5, 0x70000004},		// Nellymoser 8K mono
	{NXFF_CODEC_A_FLV_ID_6, 0x70000005},		// Nellymoser 
	{NXFF_CODEC_A_FLV_ID_7, 0x70000006},		// G.711 A-law logarithmic PCM
	{NXFF_CODEC_A_FLV_ID_8, 0x70000007},		// G.711 mu-law logarithmic PCM
	{NXFF_CODEC_A_FLV_ID_11, 0x70000008},		// Sppex
	{NXFF_CODEC_A_FLV_ID_14, 0x70000009},		// device-specific sound

		//Voice codec
	{NXFF_CODEC_A_AMR,		NXFF_DEFINE_FOURCC('A','M','R','_')},
	{NXFF_CODEC_A_AMRWB,	NXFF_DEFINE_FOURCC('A','M','R','W')},
	{NXFF_CODEC_A_SMV,		NXFF_DEFINE_FOURCC('S','M','V','_')},
	{NXFF_CODEC_A_QCELP,	NXFF_DEFINE_FOURCC('Q','C','E','L')},
	{NXFF_CODEC_A_QCELP_ALT,NXFF_DEFINE_FOURCC('Q','C','E','A')},
	{NXFF_CODEC_A_EVRC,		NXFF_DEFINE_FOURCC('E','V','R','C')},
	{NXFF_CODEC_A_EAMRWB,	NXFF_DEFINE_FOURCC('E','V','R','W')},
	{NXFF_CODEC_A_G711,		NXFF_DEFINE_FOURCC('G','7','1','1')},
	{NXFF_CODEC_A_G723,		NXFF_DEFINE_FOURCC('G','7','2','3')},

	{0x00, 0x00},
};
*/

#endif //_NXFFCODECID_H_
