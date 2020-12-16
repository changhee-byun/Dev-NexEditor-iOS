/******************************************************************************
* File Name   : NexUtil.c
* Description : Common media definition.
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2013-2014 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#include "NexUtil.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define VERSION_STRING_HELPER(a, b, c, d) #a "." #b "." #c "." d
#define VERSION_STRING_MAKER(a, b, c, d) VERSION_STRING_HELPER(a, b, c, d)
#define VERSION_STRING  VERSION_STRING_MAKER(NEXUTIL_VERSION_MAJOR, NEXUTIL_VERSION_MINOR, NEXUTIL_VERSION_PATCH, NEXUTIL_VERSION_BRANCH)
#define VERSION_INFO    "NexStreaming Util Version Prx " VERSION_STRING " Build Date " __DATE__

const NXCHAR* NexUtil_GetVersionString()
{
	return VERSION_STRING;
}

const NXINT32 NexUtil_GetMajorVersion()
{
	return NEXUTIL_VERSION_MAJOR;
}

const NXINT32 NexUtil_GetMinorVersion()
{
	return NEXUTIL_VERSION_MINOR;
}

const NXINT32 NexUtil_GetPatchVersion()
{
	return NEXUTIL_VERSION_PATCH;
}

const NXCHAR* NexUtil_GetBranchVersion()
{
	return NEXUTIL_VERSION_BRANCH;
}

const NXCHAR* NexUtil_GetVersionInfo()
{
	return VERSION_INFO;
}

NXBOOL NexUtil_CheckSameVersion(IN NXINT32 a_nMajor, IN NXINT32 a_nMinor, IN NXINT32 a_nPatch, IN NXCHAR *a_strBranch)
{
	NXINT32 nRet = 0;

	if (a_nMajor != NEXUTIL_VERSION_MAJOR)
	{
		return FALSE;
	}

	if (a_nMinor != NEXUTIL_VERSION_MINOR)
	{
		return FALSE;
	}

	if (a_nPatch != NEXUTIL_VERSION_PATCH)
	{
		return FALSE;
	}

	if ((NULL == a_strBranch) || (0 != strcmp(NEXUTIL_VERSION_BRANCH, a_strBranch)))
	{
		return FALSE;
	}

	return TRUE;
}

NXBOOL NexUtil_CheckCompatibleVersion(IN NXINT32 a_nCompatibilityNum)
{
	if (a_nCompatibilityNum != NEXUTIL_COMPATIBILITY_NUM)
	{
		return FALSE;
	}

	return TRUE;
}

const NXCHAR* NexUtil_GetStrFileFormat(IN NEX_FILEFORMAT_TYPE a_eFFType)
{
	switch (a_eFFType)
	{
		case eNEX_FF_MP4:
			return "MP4";
		case eNEX_FF_MP4MF:
			return "MP4MF";
		case eNEX_FF_AVI:
			return "AVI";
		case eNEX_FF_WAVE:
			return "WAVE";
		case eNEX_FF_MATROSKA:
			return "MKV";
		case eNEX_FF_ASF:
			return "ASF";
		case eNEX_FF_OGG:
			return "OGG";
		case eNEX_FF_MPEG_PS:
			return "MPEG_PS";
		case eNEX_FF_MPEG_TS:
			return "MPEG_TS";
		case eNEX_FF_FLV:
			return "FLV";
		case eNEX_FF_RMFF:
			return "RMFF";
		case eNEX_FF_FLAC:
			return "FLAC";
		case eNEX_FF_AMR:
			return "AMR";
		case eNEX_FF_AMRWB:
			return "AMRWB";
		case eNEX_FF_EVRC:
			return "EVRC";
		case eNEX_FF_QCELP:
			return "QCELP";
		case eNEX_FF_APE:
			return "APE";
		case eNEX_FF_MP3:
			return "MP3";
		case eNEX_FF_ADIFAAC:
			return "ADIFAAC";
		case eNEX_FF_ADTSAAC:
			return "ADTSAAC";
		case eNEX_FF_AC3:
			return "AC3";
		case eNEX_FF_EAC3:
			return "EAC3";
		case eNEX_FF_SUBTITLE:
			return "SUBTITLE";
		case eNEX_FF_MP4_3GPP:
			return "MP4_3GPP";
		case eNEX_FF_MP4_3GP2:
			return "MP4_3GP2";
		case eNEX_FF_MP4_K3G:
			return "MP4_K3G";
		case eNEX_FF_MP4_PIFF:
			return "MP4_PIFF";
		case eNEX_FF_MP4_OMA_BCAST_APDCF:
			return "MP4_OMA_BCAST_APDCF";
		case eNEX_FF_MP4_OMA_PDCFV2:
			return "MP4_OMA_PDCFV2";
		case eNEX_FF_MP4_MOV:
			return "MP4_MOV";
		case eNEX_FF_MP4_WMF:
			return "MP4_WMF";
		case eNEX_FF_MP4_SKM2:
			return "MP4_SKM2";
		case eNEX_FF_MP4_AMC:
			return "MP4_AMC";
		case eNEX_FF_MP4_KDDI:
			return "MP4_KDDI";
		case eNEX_FF_MP4_DECE_CFF:
			return "MP4_DECE_CFF";
		case eNEX_FF_MKV_WEBM:
			return "MKV_WEBM";
		case eNEX_FF_MPEG_TS_HLS:
			return "MPEG_TS_HLS";
		default:
			break;
	}

	return "UNKNOWN";
}

const NXCHAR* NexUtil_GetStrVCodec(IN NEX_CODEC_TYPE a_eCodec)
{
	switch (a_eCodec)
	{
		case eNEX_CODEC_V_H261:
			return "H261";
		case eNEX_CODEC_V_H263:
			return "H263";
		case eNEX_CODEC_V_H264:
			return "H264";
		case eNEX_CODEC_V_HEVC:
			return "HEVC";
		case eNEX_CODEC_V_MPEG4V:
			return "MPEG4V";
		case eNEX_CODEC_V_MPEG1V:
			return "MPEG1V";
		case eNEX_CODEC_V_MPEG2V:
			return "MPEG2V";
		case eNEX_CODEC_V_MSMPEG4V1:
			return "MSMPEG4V1";
		case eNEX_CODEC_V_MSMPEG4V2:
			return "MSMPEG4V2";
		case eNEX_CODEC_V_MSMPEG4V3:
			return "MSMPEG4V3";
		case eNEX_CODEC_V_DIVX:
			return "DIVX";
		case eNEX_CODEC_V_XVID:
			return "XVID";
		case eNEX_CODEC_V_WMV:
			return "WMV";
		case eNEX_CODEC_V_WMV1:
			return "WMV1";
		case eNEX_CODEC_V_WMV2:
			return "WMV2";
		case eNEX_CODEC_V_WMV3:
			return "WMV3";
		case eNEX_CODEC_V_WVC1:
			return "WVC1";
		case eNEX_CODEC_V_MSRLE:
			return "MSRLE";
		case eNEX_CODEC_V_MSVIDEO1:
			return "MSVIDEO1";
		case eNEX_CODEC_V_DVVIDEO:
			return "DVVIDEO";
		case eNEX_CODEC_V_INDEO:
			return "INDEO";
		case eNEX_CODEC_V_INDEO2:
			return "INDEO2";
		case eNEX_CODEC_V_INDEO3:
			return "INDEO3";
		case eNEX_CODEC_V_INDEO4:
			return "INDEO4";
		case eNEX_CODEC_V_INDEO5:
			return "INDEO5";
		case eNEX_CODEC_V_VP:
			return "VP";
		case eNEX_CODEC_V_VP3:
			return "VP3";
		case eNEX_CODEC_V_VP5:
			return "VP5";
		case eNEX_CODEC_V_VP6:
			return "VP6";
		case eNEX_CODEC_V_VP6A:
			return "VP6A";
		case eNEX_CODEC_V_VP6F:
			return "VP6F";
		case eNEX_CODEC_V_VP7:
			return "VP7";
		case eNEX_CODEC_V_VP8:
			return "VP8";
		case eNEX_CODEC_V_VP9:
			return "VP9";
		case eNEX_CODEC_V_REALV:
			return "REALV";
		case eNEX_CODEC_V_REALV01:
			return "REALV01";
		case eNEX_CODEC_V_REALV10:
			return "REALV10";
		case eNEX_CODEC_V_REALV13:
			return "REALV13";
		case eNEX_CODEC_V_REALV20:
			return "REALV20";
		case eNEX_CODEC_V_REALV30:
			return "REALV30";
		case eNEX_CODEC_V_REALV40:
			return "REALV40";
		case eNEX_CODEC_V_FLV:
			return "FLV";
		case eNEX_CODEC_V_FLV1:
			return "FLV1";
		case eNEX_CODEC_V_FLV4:
			return "FLV4";
		case eNEX_CODEC_V_FLASHSV:
			return "FLASHSV";
		case eNEX_CODEC_V_RAWV:
			return "RAWV";
		case eNEX_CODEC_V_ASV1:
			return "ASV1";
		case eNEX_CODEC_V_ASV2:
			return "ASV2";
		case eNEX_CODEC_V_QPEG:
			return "QPEG";
		case eNEX_CODEC_V_TRUEMOTION1:
			return "TRUEMOTION1";
		case eNEX_CODEC_V_TRUEMOTION2:
			return "TRUEMOTION2";
		case eNEX_CODEC_V_VCR1:
			return "VCR1";
		case eNEX_CODEC_V_FFV1:
			return "FFV1";
		case eNEX_CODEC_V_XXAN:
			return "XXAN";
		case eNEX_CODEC_V_CINEPAK:
			return "CINEPAK";
		case eNEX_CODEC_V_MSZH:
			return "MSZH";
		case eNEX_CODEC_V_ZLIB:
			return "ZLIB";
		case eNEX_CODEC_V_SNOW:
			return "SNOW";
		case eNEX_CODEC_V_4XM:
			return "4XM";
		case eNEX_CODEC_V_SVQ1:
			return "SVQ1";
		case eNEX_CODEC_V_TSCC:
			return "TSCC";
		case eNEX_CODEC_V_ULTI:
			return "ULTI";
		case eNEX_CODEC_V_LOCO:
			return "LOCO";
		case eNEX_CODEC_V_WNV1: 
			return "WNV1";
		case eNEX_CODEC_V_AASC:
			return "AASC";
		case eNEX_CODEC_V_FRAPS:
			return "FRAPS";
		case eNEX_CODEC_V_THEORA:
			return "THEORA";
		case eNEX_CODEC_V_CSCD:
			return "CSCD";
		case eNEX_CODEC_V_ZMBV:
			return "ZMBV";
		case eNEX_CODEC_V_KMVC:
			return "KMVC";
		case eNEX_CODEC_V_CAVS: 
			return "CAVS";
		case eNEX_CODEC_V_VMNC:
			return "VMNC";
		case eNEX_CODEC_V_SORENSONH263:
			return "SORENSONH263";
		case eNEX_CODEC_V_SCRREENVIDEO:
			return "SCRREENVIDEO";
		case eNEX_CODEC_V_SCREEMVODEPV2:
			return "SCREEMVODEPV2";
		case eNEX_CODEC_V_QT:
			return "QT";
		case eNEX_CODEC_V_JPEG:
			return "JPEG";
		case eNEX_CODEC_V_JPEG2000:
			return "JPEG2000";
		case eNEX_CODEC_V_MJPEG:
			return "MJPEG";
		case eNEX_CODEC_V_STILL_IMG:
			return "STILL_IMG";
		case eNEX_CODEC_V_RGB24:
			return "RGB24";
		default:
			break;
	}

	return "UNKNOWN";
}

const NXCHAR* NexUtil_GetStrACodec(IN NEX_CODEC_TYPE a_eCodec)
{
	switch (a_eCodec)
	{
		case eNEX_CODEC_A_MPEGAUDIO:
			return "MPEGAUDIO";
		case eNEX_CODEC_A_MP2:
			return "MP2";
		case eNEX_CODEC_A_MP3:
			return "MP3";
		case eNEX_CODEC_A_MP4A:
			return "MP4A";
		case eNEX_CODEC_A_AAC:
			return "AAC";
		case eNEX_CODEC_A_AACPLUS:
			return "AAC_PLUS";
		case eNEX_CODEC_A_AACPLUSV2:
			return "AAC_PLUS_V2";	
		case eNEX_CODEC_A_ELDAAC:
			return "ELDAAC";
		case eNEX_CODEC_A_AC3:
			return "AC3";
		case eNEX_CODEC_A_EAC3:
			return "EAC3";
		case eNEX_CODEC_A_DTS:
			return "DTS";
		case eNEX_CODEC_A_DTS_DTSC:
			return "DTS_DTSC";
		case eNEX_CODEC_A_DTS_DTSH:
			return "DTS_DTSH";
		case eNEX_CODEC_A_DTS_DTSE:
			return "DTS_DTSE";
		case eNEX_CODEC_A_DTS_DTSL:
			return "DTS_DTSL";
		case eNEX_CODEC_A_PCM_S16LE:
			return "PCM_S16LE";
		case eNEX_CODEC_A_PCM_S16BE:
			return "PCM_S16BE";
		case eNEX_CODEC_A_PCM_RAW:
			return "PCM_RAW";
		case eNEX_CODEC_A_PCM_FL32LE:
			return "PCM_FL32LE";
		case eNEX_CODEC_A_PCM_FL32BE:
			return "PCM_FL32BE";
		case eNEX_CODEC_A_PCM_FL64LE:
			return "PCM_FL64LE";
		case eNEX_CODEC_A_PCM_FL64BE:
			return "PCM_FL64BE";
		case eNEX_CODEC_A_PCM_IN24LE:
			return "PCM_IN24LE";
		case eNEX_CODEC_A_PCM_IN24BE:
			return "PCM_IN24BE";
		case eNEX_CODEC_A_PCM_IN32LE:
			return "PCM_IN32LE";
		case eNEX_CODEC_A_PCM_IN32BE:
			return "PCM_IN32BE";
		case eNEX_CODEC_A_PCM_LPCMLE:
			return "PCM_LPCMLE";
		case eNEX_CODEC_A_PCM_LPCMBE:
			return "PCM_LPCMBE";
		case eNEX_CODEC_A_ADPCM_MS:
			return "ADPCM_MS";
		case eNEX_CODEC_A_ADPCM_IMA_WAV:
			return "ADPCM_IMA_WAV";
		case eNEX_CODEC_A_ADPCM_IMA_DK4:
			return "ADPCM_IMA_DK4";
		case eNEX_CODEC_A_ADPCM_IMA_DK3:
			return "ADPCM_IMA_DK3";
		case eNEX_CODEC_A_ADPCM_YAMAHA:
			return "ADPCM_YAMAHA";
		case eNEX_CODEC_A_ADPCM_G726:
			return "ADPCM_G726";
		case eNEX_CODEC_A_ADPCM_CT:
			return "ADPCM_CT";
		case eNEX_CODEC_A_ADPCM_SWF:
			return "ADPCM_SWF";
		case eNEX_CODEC_A_WMA:
			return "WMA";
		case eNEX_CODEC_A_WMA1:
			return "WMA1";
		case eNEX_CODEC_A_WMA2:
			return "WMA2";
		case eNEX_CODEC_A_WMA3:
			return "WMA3";
		case eNEX_CODEC_A_WMA_LOSSLESS:
			return "WMA_LOSSLESS";
		case eNEX_CODEC_A_WMA_SPEECH:
			return "WMA_SPEECH";
		case eNEX_CODEC_A_WMASPDIF:
			return "WMASPDIF";
		case eNEX_CODEC_A_VORBIS:
			return "VORBIS";
		case eNEX_CODEC_A_SONIC:
			return "SONIC";
		case eNEX_CODEC_A_TRUESPEECH:
			return "TRUESPEECH";
		case eNEX_CODEC_A_FLAC:
			return "FLAC";
		case eNEX_CODEC_A_APE:
			return "APE";
		case eNEX_CODEC_A_IMC:
			return "IMC";
		case eNEX_CODEC_A_REALA:
			return "REALA";
		case eNEX_CODEC_A_REAL14_4:
			return "REAL14_4";
		case eNEX_CODEC_A_REAL28_8:
			return "REAL28_8";
		case eNEX_CODEC_A_ATRC:
			return "ATRC";
		case eNEX_CODEC_A_COOK:
			return "COOK";
		case eNEX_CODEC_A_DNET:
			return "DNET";
		case eNEX_CODEC_A_SIPR:
			return "SIPR";
		case eNEX_CODEC_A_RAAC:
			return "RAAC";
		case eNEX_CODEC_A_RACP: 
			return "RACP";
		case eNEX_CODEC_A_ADUU:
			return "ADUU";
		case eNEX_CODEC_A_QDMC:
			return "QDMC";
		case eNEX_CODEC_A_QDMC2:
			return "QDMC2";
		case eNEX_CODEC_A_FLV_ID_3:
			return "FLV_ID_3";
		case eNEX_CODEC_A_FLV_ID_4:
			return "FLV_ID_4";
		case eNEX_CODEC_A_FLV_ID_5:
			return "FLV_ID_5";
		case eNEX_CODEC_A_FLV_ID_6:
			return "FLV_ID_6";
		case eNEX_CODEC_A_FLV_ID_7:
			return "FLV_ID_7";
		case eNEX_CODEC_A_FLV_ID_8:
			return "FLV_ID_8";
		case eNEX_CODEC_A_FLV_ID_11:
			return "FLV_ID_11";
		case eNEX_CODEC_A_FLV_ID_14:
			return "FLV_ID_14";
		case eNEX_CODEC_A_AMR:
			return "AMR";
		case eNEX_CODEC_A_AMRWB:
			return "AMRWB";
		case eNEX_CODEC_A_EAMRWB:
			return "EAMRWB";
		case eNEX_CODEC_A_SMV:
			return "SMV";
		case eNEX_CODEC_A_QCELP:
			return "QCELP";
		case eNEX_CODEC_A_QCELP_ALT:
			return "QCELP_ALT";
		case eNEX_CODEC_A_EVRC:
			return "EVRC";
		case eNEX_CODEC_A_G711:
			return "G711";
		case eNEX_CODEC_A_G711_ALAW:
			return "G711_ALAW";
		case eNEX_CODEC_A_G711_MULAW:
			return "G711_MULAW";
		case eNEX_CODEC_A_G723:
			return "G723";
		case eNEX_CODEC_A_BSAC:
			return "BSAC";
		case eNEX_CODEC_A_ALAC:
			return "ALAC";
		case eNEX_CODEC_A_GSM610:
			return "GSM610";
		case eNEX_CODEC_A_BLURAYPCM:
			return "BLURAYPCM";
		case eNEX_CODEC_A_DRA:
			return "DRA";
		default:
			break;
	}

	return "UNKNOWN";
}

const NXCHAR* NexUtil_GetStrTCodec(IN NEX_CODEC_TYPE a_eCodec)
{
	switch (a_eCodec)
	{
		case eNEX_CODEC_T_3GPP:
			return "3GPP";
		case eNEX_CODEC_T_SKT:
			return "SKT";
		case eNEX_CODEC_T_SMI:
			return "SMI";
		case eNEX_CODEC_T_SRT:
			return "SRT";
		case eNEX_CODEC_T_ID3_UNSYNC_LYRIC:
			return "ID3_UNSYNC_LYRIC";
		case eNEX_CODEC_T_ID3_SYNC_LYRIC:
			return "ID3_SYNC_LYRIC";
		case eNEX_CODEC_T_VOB_SUB:
			return "VOB_SUB";
		case eNEX_CODEC_T_MICRODVD_SUB:
			return "MICRODVD_SUB";
		case eNEX_CODEC_T_LRC:
			return "LRC";
		case eNEX_CODEC_T_SSA:
			return "SSA";
		case eNEX_CODEC_T_SYLT:
			return "SYLT";
		case eNEX_CODEC_T_TTML:
			return "TTML";
		case eNEX_CODEC_T_TT:
			return "TT";
		case eNEX_CODEC_T_WEB_VTT:
			return "WEB_VTT";
		case eNEX_CODEC_T_CC_CEA:
			return "CC_CEA";
		case eNEX_CODEC_T_CC_CEA_608:
			return "CC_CEA_608";
		case eNEX_CODEC_T_CC_CEA_708:
			return "CC_CEA_708";
		case eNEX_CODEC_T_DIVX_XSUB:
			return "DIVX_XSUB";
		case eNEX_CODEC_T_DIVX_XSUBPLUS:
			return "DIVX_XSUBPLUS";
		case eNEX_CODEC_T_UTF:
			return "UTF";
		case eNEX_CODEC_T_UTF8:
			return "UTF8";
		case eNEX_CODEC_T_UTF16:
			return "UTF16";
		case eNEX_CODEC_T_UTF32:
			return "UTF32";
		case eNEX_CODEC_T_TIMEDMETA:
			return "TimedMeta";
		default:
			break;
	}

	return "UNKNOWN";
}

NXVOID NexUtil_WStrCpy(IN const NXWCHAR* a_wstrSrc, OUT NXWCHAR* a_wstrDest)
{
	NXUINT8 *dest_byte_ptr, *src_byte_ptr;

	if (a_wstrSrc == NULL)
	{
		*a_wstrDest = 0x0000;
		return;
	}

	if (W_ALIGNED(a_wstrSrc) && W_ALIGNED(a_wstrDest))
	{
		while (*a_wstrSrc)
		{
			*a_wstrDest++ = *a_wstrSrc++;
		}
		*a_wstrDest = 0x0000;
	}
	else
	{
		dest_byte_ptr = (NXUINT8*)a_wstrDest;
		src_byte_ptr = (NXUINT8*)a_wstrSrc;
		while (!(src_byte_ptr[0] == 0x00 && src_byte_ptr[1] == 0x00))
		{
			*dest_byte_ptr++ = *src_byte_ptr++;
			*dest_byte_ptr++ = *src_byte_ptr++;
		}
		dest_byte_ptr[0] = 0x00;
		dest_byte_ptr[1] = 0x00;
	}

	return;
}

NXUINT32 NexUtil_WStrLen(IN const NXWCHAR* a_wstrSrc)
{
	NXUINT32 return_val = 0;
	NXUINT8 *src_byte_ptr;

	if (W_ALIGNED(a_wstrSrc))
	{
		while (*a_wstrSrc++ != 0x00)
		{
			return_val++;
		}
	}
	else
	{
		src_byte_ptr = (NXUINT8*)a_wstrSrc;

		while (TRUE)
		{
			if ((src_byte_ptr[0] == 0x00) && (src_byte_ptr[1] == 0x00))
			{
				break;
			}
			return_val++;
			src_byte_ptr += 2;
		}
	}          
	return return_val;
}

//This character map does not have the difference between upper case and lower case.
static const NXCHAR g_arrCharMap[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
	0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
	0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
	0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
	0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
	0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
	0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,
};

NXINT32 NexUtil_StrCaseCmp(IN const NXCHAR* a_pStr1, IN const NXCHAR* a_pStr2)
{
	register const NXCHAR *cm = &g_arrCharMap[0];
	const NXCHAR *us1 = (const NXCHAR*)a_pStr1;
	const NXCHAR *us2 = (const NXCHAR*)a_pStr2;

	while (cm[*us1] == cm[*us2++])
	{
		if (*us1++ == '\0')
		{
			return(0);
		}
	}

	return(cm[*us1] - cm[*--us2]);
}

NXINT32 NexUtil_StrNCaseCmp(IN const NXCHAR* a_pStr1, IN const NXCHAR* a_pStr2, IN NXINT32 a_nCnt)
{
	if (a_nCnt != 0)
	{
		register const NXCHAR *cm = &g_arrCharMap[0];
		const NXCHAR *us1 = (const NXCHAR*)a_pStr1;
		const NXCHAR *us2 = (const NXCHAR*)a_pStr2;

		do
		{
			if (cm[*us1] != cm[*us2++])
			{
				return(cm[*us1] - cm[*--us2]);
			}

			if (*us1++ == '\0')
			{
				break;
			}
		} while (--a_nCnt != 0);
	}

	return 0;
}

NXBOOL NexUtil_IsIP(IN NXCHAR *a_str, IN NXUINT32 a_uLen)
{
	NXUINT32 uIdx = 0;

	if (NULL == a_str)
	{
		return FALSE;
	}

	for (uIdx=0; uIdx<a_uLen; uIdx++)
	{
		if ((a_str[uIdx] < '0') || (a_str[uIdx] > '9'))
		{
			if ('.' == a_str[uIdx] || ':' == a_str[uIdx])
			{
				if (0 == uIdx)
				{
					return FALSE;
				}
			}
		}
		else
		{
			return FALSE;
		}
	}

	return TRUE;
}

NXBOOL NexUtil_IsNumber(IN NXCHAR *a_str, IN NXUINT32 a_uLen)
{
	NXUINT32 uIdx = 0;

	if (NULL == a_str)
	{
		return FALSE;
	}

	for (uIdx=0; uIdx<a_uLen; uIdx++)
	{
		if ((a_str[uIdx] < '0') || (a_str[uIdx] > '9'))
		{
			return FALSE;
		}
	}

	return TRUE;
}

NXUINT16 NexUtil_ChangeEndian16(IN NXUINT8 *a_pData)
{
	NXUINT16 uwRet = 0;

	uwRet |= ((NXUINT16)a_pData[0]<< 8) & 0xFF00;
	uwRet |= ((NXUINT16)a_pData[1])     & 0x00FF;

	return uwRet;
}

NXUINT32 NexUtil_ChangeEndian32(IN NXUINT8 *a_pData)
{
	NXUINT32 uRet = 0;

	uRet |= ((NXUINT32)a_pData[0]<<24)  & 0xFF000000;
	uRet |= ((NXUINT32)a_pData[1]<<16)  & 0x00FF0000;
	uRet |= ((NXUINT32)a_pData[2]<<8)   & 0x0000FF00;
	uRet |= ((NXUINT32)a_pData[3])      & 0x000000FF;

	return uRet;
}

NXINT32 NexUtil_ConvVideoCodecType(IN NEX_CODEC_TYPE a_uInCodecType, IN NXUINT32 a_uInFourCC, IN NEX_FILEFORMAT_TYPE a_eInFFType, OUT NEX_CODEC_TYPE *a_puOutCodecType, OUT NXUINT32 *a_puOutFourCC)
{
	*a_puOutFourCC = a_uInFourCC;

	switch (a_uInCodecType)
	{
		case eNEX_CODEC_V_MPEG4V:
			if ((a_eInFFType == eNEX_FF_AVI) || (a_eInFFType == eNEX_FF_MATROSKA) || (a_eInFFType == eNEX_FF_RMFF) 
				|| (a_eInFFType == eNEX_FF_ASF) || (a_eInFFType == eNEX_FF_MPEG_TS) || (a_eInFFType == eNEX_FF_OGG))
			{
				if (NexUtil_IsMPEG4AP(*a_puOutFourCC))
				{
					*a_puOutCodecType = eNEX_CODEC_V_DIVX;
				}
				else
				{
					*a_puOutCodecType = eNEX_CODEC_V_MPEG4V;
				}
			}
			else
			{
				*a_puOutCodecType = eNEX_CODEC_V_MPEG4V;
			}
			break;
		case eNEX_CODEC_V_WMV:
			if ('WVC1' == *a_puOutFourCC)
			{
				*a_puOutCodecType = eNEX_CODEC_V_WVC1;
			}
			else
			{
				*a_puOutCodecType = eNEX_CODEC_V_WMV;
			}
			break;
		default:
			*a_puOutCodecType = a_uInCodecType;
			break;
	}

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 9, "[%s %d] Video codec type is 0x%x and FourCC is 0x%x.\n", _FUNLINE_, *a_puOutCodecType, *a_puOutFourCC);

	return 0;
}

//This function compares FourCC based on firstly BIG endian and sencondarily LITTLE endian.
NXBOOL NexUtil_FourCC_CaseCmp(IN NXUINT32 a_uFourCC, IN const NXCHAR *a_szFourCC)
{
	NXUINT32 uIdx = 0;
	NXCHAR szTemp[4] = {0, };

	szTemp[0] = (NXCHAR)((a_uFourCC >> 24) & 0xFF);
	szTemp[1] = (NXCHAR)((a_uFourCC >> 16) & 0xFF);
	szTemp[2] = (NXCHAR)((a_uFourCC >> 8) & 0xFF);
	szTemp[3] = (NXCHAR)(a_uFourCC & 0xFF);

	if (0 == NexUtil_StrNCaseCmp(szTemp, a_szFourCC, 4))
	{
		return TRUE;
	}

	szTemp[0] = (NXCHAR)(a_uFourCC & 0xFF);
	szTemp[1] = (NXCHAR)((a_uFourCC >> 8) & 0xFF);
	szTemp[2] = (NXCHAR)((a_uFourCC >> 16) & 0xFF);
	szTemp[3] = (NXCHAR)((a_uFourCC >> 24) & 0xFF);

	if (0 == NexUtil_StrNCaseCmp(szTemp, a_szFourCC, 4))
	{
		return TRUE;
	}

	return FALSE;
}

NXBOOL NexUtil_IsMPEG4AP(IN NXUINT32 a_uFourCC)	// FourCC is supposed to arrange to big-endian regardless of system.
{
	NXCHAR szTemp[5] = {0, };

	szTemp[0] = (NXUINT8)((a_uFourCC >> 24) & 0xFF);
	szTemp[1] = (NXUINT8)((a_uFourCC >> 16) & 0xFF);
	szTemp[2] = (NXUINT8)((a_uFourCC >> 8) & 0xFF);
	szTemp[3] = (NXUINT8)(a_uFourCC & 0xFF);
	szTemp[4] = 0;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_SOURCE, 4, "[%s %d] FourCC=%s \n", _FUNLINE_, szTemp);

	if ((NexUtil_StrNCaseCmp(szTemp, "DIVX",4) == 0)
		|| (NexUtil_StrNCaseCmp(szTemp, "XVID",4) == 0)
		|| (NexUtil_StrNCaseCmp(szTemp, "DX",2) == 0)
		|| (NexUtil_StrNCaseCmp(szTemp, "DIV",3) == 0)
	   )
	{
		return TRUE;
	}

	return 	FALSE;
}

#define IsNeedLength(x) (x==eNEX_CODEC_V_MSMPEG4V3) || (x==eNEX_CODEC_V_MPEG1V) \
						|| (x==eNEX_CODEC_V_MPEG2V) || (x==eNEX_CODEC_V_THEORA) \
						|| (x==eNEX_CODEC_V_WMV1) || (x==eNEX_CODEC_V_WMV2) \
						|| (x==eNEX_CODEC_V_WMV3) || (x==eNEX_CODEC_V_WVC1) || (x==eNEX_CODEC_V_VP8)

NXINT32 NexUtil_FrameDump(IN const NXUINT8 *a_pFilePath, IN NEX_CODEC_TYPE a_eCodecType, IN NXUINT8 *a_pData, IN NXUINT32 a_uLen)
{
	NEXSALFileHandle hFile = NEXSAL_INVALID_HANDLE;

	if (NULL == a_pData || 0 == a_uLen || eNEX_CODEC_UNKNOWN == a_eCodecType)
	{
		return -1;
	}

	hFile = nexSAL_FileOpenA((const NXINT8*)a_pFilePath, (NEXSALFileMode)(NEXSAL_FILE_WRITE));

	if (NEXSAL_INVALID_HANDLE == hFile)
	{
		hFile = nexSAL_FileOpenA((const NXINT8*)a_pFilePath, (NEXSALFileMode)(NEXSAL_FILE_CREATE | NEXSAL_FILE_WRITE));
	}

	if (hFile == NEXSAL_INVALID_HANDLE)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_FLOW, 0, "[%s %d] FileOpen Failed!!\n", _FUNLINE_);
		return -1;
	}
	else
	{
		nexSAL_FileSeek(hFile, 0, NEXSAL_SEEK_END);

		if (IsNeedLength(a_eCodecType))
		{
			nexSAL_FileWrite(hFile, (NXVOID*)&a_uLen, sizeof(NXUINT32));
		}

		nexSAL_FileWrite(hFile, a_pData, (NXUSIZE)a_uLen);
		nexSAL_FileClose(hFile);
	}

	return 0;
}

NXVOID NexUtil_ConvHTTPEscape(INOUT NXCHAR *a_pStr)
{
	NXCHAR arrHex[3] = {'\0', '\0', '\0'};
	NXCHAR *pcTemp;
	NXUSIZE usSrcPos = 0;
	NXUSIZE usDestPos = 0;
	NXUSIZE usStrLen = strlen(a_pStr);

	while (usSrcPos < usStrLen)
	{
		if (a_pStr[usSrcPos] == '%' && usSrcPos+2 < usStrLen && isdigit(a_pStr[usSrcPos+1]) && isdigit(a_pStr[usSrcPos+2]))
		{
			arrHex[0] = a_pStr[usSrcPos+1];
			arrHex[1] = a_pStr[usSrcPos+2];
			a_pStr[usDestPos] = (NXCHAR)strtol(arrHex, &pcTemp, 16);
			usDestPos++;
			usSrcPos += 3;
		}
		else if (usSrcPos != usDestPos)
		{
			a_pStr[usDestPos] = a_pStr[usSrcPos];
			usSrcPos++;
			usDestPos++;
		}
		else
		{
			usSrcPos++;
			usDestPos++;
		}
	}
	
	a_pStr[usDestPos] = '\0';
}

