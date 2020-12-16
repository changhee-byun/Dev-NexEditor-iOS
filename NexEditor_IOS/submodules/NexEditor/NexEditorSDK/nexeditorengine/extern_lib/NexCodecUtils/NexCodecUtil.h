/*-----------------------------------------------------------------------------
	File Name   :	nexCodecUtils.h
	Description :	the header file for the client of nexCodecUtils.
 ------------------------------------------------------------------------------

 NexStreaming Confidential Proprietary
 Copyright (C) 2007 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------
Revision History:
Author              Date			Version		Description of Changes
------------------------------------------------------------------------------------------
Jongbae, Park		2013/12/01		1.3.9		MW(오석훈) to Codec team
Jongbae, Park		2013/12/04		1.3.10		NexCodecUtil_AVC_GetSPSInfo() -NEXCODECUTIL_SPS_INFO 구조체 변수 추가
Jongbae, Park		2013/12/04		1.3.11		NexCodecUtil_AVC_GetSPSInfo() - _ReadNShiftBits() 32bit처리 변경
Jongbae, Park		2013/12/05		1.3.12		NexCodecUtil_AVC_GetSPSInfo() - 32bit  _ReadNShiftBits() 오류 수정
Jongbae, Park		2013/12/23		1.3.13		NexEngine CalTools에 aac 관련 *.c 코드 -> NexCalTools Library 안으로(독립사용시 심볼없음 오류)
												iNALHeaderLengthSize 0 or 4초과 부분 예외처리 (by 오석훈)
Jongbae, Park		2014/01/06		1.3.14		NexCodecUtil_MP3_MuteFrame() 추가
												NexCodecUtil_GuessNalHeaderLengthSize() iNalLen == 0 에러리턴 처리
KT,Hwang			2014/01/23		1.3.15		NexCodecUtil_AVC_ANNEXB_GetConfigStream() NALType 에 따른 SPSStartIndex처리 수정
SJ,Lee				2014/02/03		1.3.16		NexCodecUtil_VC1_GetExtraDataInfo()함수 및  NEXCODECUTIL_PPS_INFO 구조체 추가
KT,Hwang			2014/03/05		1.3.17		NexCodecUtil_IsBFrame() - eNEX_CODEC_V_WVC1 경우 IsBFrame 추가
KT,Hwang			2014/03/05		1.3.18		NexCodecUtil_IsBFrame() - eNEX_CODEC_V_WMV3 경우 IsBFrame 추가
KT,Hwang			2014/03/10		1.3.19		_DecodeShortVideoHeader() - Check H.263 UMV
Jongbae, Park		2014/03/18		1.3.20		NexCodecUtil_GetSizeOfADIFHeader() 추가
Jongbae, Park		2014/04/04		1.4.0		CodecID(OTI) define 변경 (NexOTIs.h:삭제 -> NexMediaDef.h:추가)
Jongbae, Park		2014/04/14		2.0.0		NexCalTools->NexCodecUtil
Jongbae, Park		2014/05/19		2.0.1		remove warning message(casting 관련)
Liam, Park			2014/10/27		2.0.2		NexCodecUtil_FindNAL() - do while 조건문에서 input frame buffer 이상의 메모리를 참조할 수 있는 문제 수정
Liam, Park			2014/11/17		2.0.3		HEVC seekable support
												'NexCodecUtil_IsSeekableFrame()', 'NexCodecUtil_IsBFrame()', 'NexCodecUtil_IsPFrame()' - 입력 관련 수정
												'NexCodecUtil_ConvertFormat()' - 메모리 참조 문제 수정
Jongbae, Park		2014/11/17		2.0.4		NexCodecUtil_ConvertFormat : size compare 조건 수정
												NexCodecUtil_HEVC_GetSliceHeaderInfo() add
Jongbae, Park		2014/12/11		2.0.5		_IsRAPFrameHEVC(), _IsCRAFrameHEVC() :size compare 조건 수정, NexCodecUtil_HEVC_NAL_GetConfigStream() :offset수정
Jongbae, Park		2014/12/18		2.0.6		NexCodecUtil_HEVC_GetAccessUnit() 추가
												NexCodecUtil_HEVC_GetPPSInfo(), NexCodecUtil_HEVC_GetSPSInfo() : raw type 적용
Jongbae, Park		2014/12/24		2.0.7		NexCodecUtil_HEVC_GetSliceHeaderInfo()-annexB type bug fix
Jongbae, Park		2015/01/05		2.0.8		NexCodecUtil_AVC_DecodeSEI() add
Liam, Park			2015/02/24		2.0.9		'nexSAL_TraceCat' log 정리
Jongbae, Park		2015/03/06		2.0.10		NexCodecUtil_AVC_ParseH264NalHeaderLengthSize() check bug fix
Jongbae, Park		2015/03/12		2.0.11		NexCodecUtil_HEVC_ConvertFormatPSs() bug fix
Jongbae, Park		2015/03/13		2.0.12		NexCodecUtil_HEVC_FindNAL() : nal type 수정
												NexCodecUtil_HEVC_GetPPSInfo() : annexB bug fix
												NexCodecUtil_AVC_IsDirectMixable() add
												NexCodecUtil_HEVC_IsDirectMixable() add
												NexCodecUtil_MPEG4V_IsDirectMixable() add
												NexCodecUtil_GetAACType() aacV2 parsing add
												NexCodecUtil_AAC_MuteFrame() 내부변수 초기화
GJ, Kim				2015/04/20		2.0.13		`NexCodecUtil_HEVC_ANNEXB_GetConfigStream` `NexCodecUtil_HEVC_NAL_GetConfigStream`
												NALType case 관련 변경. (JIRA issue number : NPDS-3395)
GJ, Kim				2015/05/22		2.0.14		Add 'NexCodecUtil_HEVC_IsPictureFrame_Raw()'
												Add 'NexCodecUtil_HEVC_IsPictureFrame_withType()'
												Add 'NexCodecUtil_HEVC_GetAccessUnit_Raw()'
												Add 'NexCodecUtil_HEVC_isPrefixConfigFrame()'
												Add 'NexCodecUtil_HEVC_isSuffixConfigFrame()'
												Fix issues about JIRA issue 'NPDS-3710', 'NPDS-3719'
GJ, Kim				2015/06/18		2.0.15		Fix issues about JIRA issue	'NPDS-3806', 'NPDS-3815', 'NPDS-3819'
												Add 'nal_unit_type' in 'NEXCODECUTILHEVCSliceHeaderInfo'
												Add 'bFirstPicDone' in 'NexCodecUtil_HEVC_GetSliceHeaderInfo()'
												Add 'NalLengthBytes', 'uHEVCFormat' in 'NexCodecUtil_HEVC_GetAccessUnit()'
												Add 'NexCodecUtil_HEVC_GetAccessUnit_AnnexB()'

Jongbae, Park		2015/07/21		2.0.16		1.Config1,Config2-AnnexB,Raw type cross Support
													-NexCodecUtil_AVC_IsDirectMixable(), NexCodecUtil_HEVC_IsDirectMixable()
												2.NEX_AVC_PROFILE -> NEX_H26x_PROFILE
													-add HEVC profile
GJ, Kim											3.Add NexCodecUtil_AVC_ResetPOC()
Jongbae, Park		2015/07/27		2.0.17		NexCodecUtil_HEVC_GetSPSInfo() : parsing Tier and Level Value
Liam, Park			2015/09/08		2.0.18		Change parameter type in 'NexCodecUtil_VC1_GetExtraDataInfo()'
												Add 'NexCodecUtil_WVC1_GetSeqHeaderStartPoint()'
Liam, Park			2015/09/14		2.0.19		Change parameter type in 'NexCodecUtil_WVC1_GetSeqHeaderStartPoint()'
Liam, Park			2015/10/01		2.0.20		Add 'NexCodecUtil_AVC_FindAnnexBStartCode()'
GJ, Kim				2016/02/16		2.0.21		Check reserved bit without configuration version in HEVC decoder configuration record. (JIRA : NPDS-4517)
GJ, Kim				2016/09/21		2.0.22		Add NexCodecUtil_ConvertFormat2(), NexCodecUtil_AVC_ConvertFormatPSs2(), NexCodecUtil_HEVC_ConvertFormatPSs2()
												Bug fix NexCodecUtil_AVC_SPSExists()
GJ, Kim				2016/10/06		2.0.23		Fix JIRA issue DSTS-1593, NexCodecUtil_AVC_IsConfigFromRecordType()
GJ, Kim				2016/11/01		2.0.24		Fix seekable frame check for VP9, NexCodecUtil_IsSeekableFrame().
GJ, Kim				2016/11/02		2.0.25		Fix AAC dsi parsing function, NexCodecUtil_AAC_ParseDSI().
GJ, Kim				2016/12/21		2.0.26		Fix bit read function for AVC, HEVC emulation prevention byte.
												Separate the bit read function into AVC, HEVC and others.
GJ, Kim				2017/01/20		2.0.27		Fix NexCodecUtil_AVC_IsDirectMixable_SPSCheck() for pre-loaded byte.
												Add return value for only decodable mixing.
GJ, Kim				2017/02/17		2.0.28		Add exception handling code in _LoadBSAVC().
GJ, Kim				2017/03/13		2.0.29		Add exception handling code for buffer overflow (DecoderConfigurationRecord type).
												NexCodecUtil_HEVC_GetPPSInfo(), NexCodecUtil_HEVC_GetSPSInfo(), NexCodecUtil_HEVC_SPSExists()
GJ, Kim				2017/04/13		2.1.0		Add variable in SPSInfo for set the HEVCDecoderConfigurationRecord.
												Fix NexCodecUtil_HEVC_GetSPSInfo() for saving the HEVCDecoderConfigurationRecord variable.
GJ, Kim				2017/04/18		2.1.1		Add NexCodecUtil_AVC_IsAVCReferenceFrame().
												Fix resolution update routine  via cropping infomation in NexCodecUtil_HEVC_GetSPSInfo()
												Fix level_idc update routine in NexCodecUtil_HEVC_IsDirectMixable_SPSCheck()
GJ, Kim				2017/04/26		2.1.2		Fix size reading routine in NexCodecUtil_AVC_IsAVCReferenceFrame().
GJ, Kim				2017/05/17		2.1.3		Fix AVC annexB startcode posion update routine in NexCodecUtil_AVC_IsAVCReferenceFrame().
GJ, Kim				2017/06/02		2.1.4		Add field_pic_flag initialize code when frame_mbs_only_flag is 0.
GJ, Kim				2017/08/28		2.1.5		Search SPS start position when NexCodecUtil_HEVC_GetSPSInfo() with annexB type.
GJ, Kim				2017/11/21		2.1.6		Add HEVC SEI message(HDR Info.) parsing.
												mastering_display_colour_volume, colour_remapping_info, content_light_level_info
GJ, Kim				2018/05/18		2.1.7		Modify NexCodecUtil_GuessNalHeaderLengthSize() [raw frame start with '00 00 00 01']
GJ, Kim				2018/06/11		2.1.8		Modify NexCodecUtil_CheckByteFormat() [raw frame start with '00 00 00 01']
												replace  ver2.1.7
GJ, Kim				2018/06/28		2.1.9		Fix NexCodecUtil_CheckByteFormat() for nal_size_overflow(update ver2.1.8)
												Modify NexCodecUtil_HEVC_SEI_GetHDRInfo() for raw_frame+annexB_frame and multi-sei.
GJ, Kim				2018/08/06		2.1.10		Add NexCodecUtil_Get_MPEG_Audio_Info().
GJ, Kim				2018/08/23		2.1.11		Add video signal information(full_range_flag, colour_primaries, etc.) in SPS info. for AVC, HEVC.
GJ, Kim				2018/09/03		2.1.12		Add SVC extension information parsing in prefix nal.
GJ, Kim				2018/11/14		2.1.13		Add NexCodecUtil_HEVC_GetNalCount() for config stream(AnnexB).
GJ, Kim				2018/11/28		2.1.14		Fix NexCodecUtil_HEVC_SEI_GetHDRInfo() for multi-nal(buffer overflow).
												Add MPEG4 video profile definition.
GJ, Kim				2018/12/10		2.1.15		Remove some reserved bit checking in HEVC decoder configuration record.
GJ, Kim				2018/12/11		2.1.16		Add NexCodecUtil_AVC_LevelLimits().
------------------------------------------------------------------------------------------*/

#if !defined(_NEXCODECUTIL_DOT_H_INCLUDED_)
#define _NEXCODECUTIL_DOT_H_INCLUDED_

#define NEXCODECUTIL_MAJOR_VERSION		2
#define NEXCODECUTIL_MINOR_VERSION		1
#define NEXCODECUTIL_PATCH_NUM			16 

#include "NexSAL_Internal.h"
#include "NexMediaDef.h"


#if (defined(WIN32) || defined(WINCE))
#	if defined(NEXCODEC_UTIL_EXPORTS)
#		define NEXCODECUTIL_API __declspec(dllexport)
#	else
#		define NEXCODECUTIL_API __declspec(dllimport)
#	endif
#
#elif (defined(SYMBIAN60))
#	if (defined(NEXCODEC_UTIL_EXPORTS)
#		define NEXCODECUTIL_API	EXPORT_C
#       else
#		define NEXCODECUTIL_API	IMPORT_C
#	endif
#else
#	define NEXCODECUTIL_API
#endif


#define NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANY				0
#define NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_RAW				1
#define NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANNEXB			2

// Error type list
#define NEXCODECUTIL_ERROR_NONE									0x00000000
#define NEXCODECUTIL_ERROR_INVALID_PARAMETER					0x00000001
#define NEXCODECUTIL_ERROR_NOT_SUPPORT_AUDIO_CODEC				0x00000010
#define NEXCODECUTIL_ERROR_NOT_SUPPORT_VIDEO_CODEC				0x00000011
#define NEXCODECUTIL_ERROR_INVALID_CODEC						0x00000012
#define NEXCODECUTIL_ERROR_CORRUPTED_FRAME						0x00000013
#define NEXCODECUTIL_ERROR_SYSTEM_FAIL							0x00000014
#define NEXCODECUTIL_ERROR_MEMORY_FAIL							0x00000015
#define NEXCODECUTIL_ERROR_RESOURCE_FAIL						0x00000016
#define NEXCODECUTIL_ERROR_NOT_SUPPORT_DEVICE					0x00000017
#define NEXCODECUTIL_ERROR_OVER_SPEC 							0x00000018
#define NEXCODECUTIL_ERROR_OUTPUT_NOT_EXIST						0x00000019
#define NEXCODECUTIL_ERROR_FAIL									0xFFFFFFFF

#define NEXCODECUTIL_NOT_DIRECT_MIXABLE							0x00000000
#define NEXCODECUTIL_DIRECT_MIXABLE								0x00000001
#define NEXCODECUTIL_PPS_NOT_DIRECT_MIXABLE						0x00000002
#define NEXCODECUTIL_DIRECT_MIXABLE_ONLY_DECODING				0x00000003

typedef enum
{
	eNEX_H26x_PROFILE_UNKNOWN			= 0,
	eNEX_HEVC_PROFILE_MAIN				= 1,		//Main Profile
	eNEX_HEVC_PROFILE_MAIN10			= 2,		//Main 10 Profile
	eNEX_HEVC_PROFILE_MAINSTILLPICTURE	= 3,		//Main Still Picture Profile
	//eNEX_AVC_PROFILE_CAVLC			= 44,		//CAVLC 4:4:4 Intra profile
	eNEX_AVC_PROFILE_BASELINE			= 66,
	eNEX_AVC_PROFILE_MAIN				= 77,
	eNEX_AVC_PROFILE_EXTENDED			= 88,		//Extended Profile
	eNEX_AVC_PROFILE_HIGH				= 100,		//High Profile
	eNEX_AVC_PROFILE_HIGH10				= 110,		//High 10 Profile
	eNEX_AVC_PROFILE_HIGH422			= 122,		//High 4:2:2 Profile
	eNEX_AVC_PROFILE_HIGH444			= 244,		//High 4:4:4 Profile
	eNEX_H26x_PROFILE_FOR4BYTESALIGN	= 0x7FFFFFFF

} NEX_H26x_PROFILE;

typedef enum
{
	eNEX_MPEG4V_PROFILE_SIMPLE				= 0,		// Simple Profile
	eNEX_MPEG4V_PROFILE_SIMPLE_SCALABLE		= 1,		// Simple Scalable Profile
	eNEX_MPEG4V_PROFILE_ADVANCED_SIMPLE		= 2,		// Asvanced Simple Profile (ASP)

	eNEX_MPEG4V_PROFILE_UNKNOWN				= 0xFF

} NEX_MPEG4V_PROFILE;

typedef enum
{
	eNEX_AAC_PROFILE_AAC_LC           = 2,		// AAC-LC
	eNEX_AAC_PROFILE_SBR              = 5,		// AAC+
	eNEX_AAC_PROFILE_PS               = 29,		// AAC+v2
	eNEX_AAC_PROFILE_FOR4BYTESALIGN		= 0x7FFFFFFF

} NEX_AAC_PROFILE;

#define NEX_AAC_MAX_CHANNELS	8
typedef struct
{
    NXUINT8 ele_inst_tag;						//element_instance_tag;
    NXUINT8 obj_type;
    NXUINT8 sampling_freq_index;				//sampling_freq_index;
    NXUINT8 nofront_ch_ele;						//num_front_channel_elements;
    NXUINT8 noside_ch_ele;						//num_side_channel_elements;
    NXUINT8 noback_ch_ele;						//num_back_channel_elements;
    NXUINT8 nolfe_ch_ele;						//num_lfe_channel_elements;
    NXUINT8 noassoc_data_elements;				//num_assoc_data_elements
    NXUINT8 novalid_cc_elements;				//num_valid_cc_elements
    NXUINT8 present_of_mono_mixdown;			//mono_mixdown_present
    NXUINT8 num_mono_mixdown_element;			//mono_mixdown_element_number
    NXUINT8 present_of_stereo_mixdown;			//stereo_mixdown_present
    NXUINT8 num_stereo_mixdown_element;			//stereo_mixdown_element_number
    NXUINT8 present_of_matrix_mixdown_idx;		//matrix_mixdown_idx_present
    NXUINT8 pseudo_surround_enable;
    NXUINT8 index_of_matrix_mixdown;			//matrix_mixdown_idx
    NXUINT8 front_ele_is_cpe[16];				//front_element_is_cpe
    NXUINT8 front_ele_tag_select[16];			//front_element_tag_select
    NXUINT8 side_ele_is_cpe[16];				//side_element_is_cpe
    NXUINT8 side_ele_tag_select[16];			//side_element_tag_select
    NXUINT8 back_ele_is_cpe[16];				//back_element_is_cpe
    NXUINT8 back_ele_tag_select[16];			//back_element_tag_select
    NXUINT8 lfe_ele_tag_select[16];				//lfe_element_tag_select
    NXUINT8 assoc_data_ele_tag_select[16];		//assoc_data_element_tag_select
    NXUINT8 cc_ele_is_ind_sw[16];				//cc_element_is_ind_sw
    NXUINT8 valid_cc_ele_tag_select[16];		//valid_cc_element_tag_select

    NXUINT8 ch;

    NXUINT8 bytes_of_comment_field;				//comment_field_bytes
    NXUINT8 data_of_comment_field[257];			//comment_field_data

    /* extra added values */
    NXUINT8 front_channels_num;					//front_channels_num
    NXUINT8 side_channels_num;					//num_side_channels
    NXUINT8 back_channels_num;					//num_back_channels
    NXUINT8 lfe_channels_num;					//num_lfe_channels
    NXUINT8 single_ch_ele_channel[16];			//sce_channel
    NXUINT8 ch_pair_ele_channel[16];			//cpe_channel
} NEX_AAC_PROGRAM_CONFIG;

typedef enum
{
	eNEX_NAL_SLICE=1,
	eNEX_NAL_DPA,
	eNEX_NAL_DPB,
	eNEX_NAL_DPC,
	eNEX_NAL_IDR_SLICE,
	eNEX_NAL_SEI,
	eNEX_NAL_SPS,
	eNEX_NAL_PPS,
	eNEX_NAL_AUD,
	eNEX_NAL_END_SEQUENCE,
	eNEX_NAL_END_STREAM,
	eNEX_NAL_FILLER_DATA,
	eNEX_NAL_SPS_EXT,
	eNEX_NAL_PREFIX,
	eNEX_NAL_AUXILIARY_SLICE = 19,
	eNEX_NAL_SLICE_EXT = 20

} NEX_NAL_TYPE;

typedef enum
{
	eNEX_HEVC_NALU_TYPE_TRAIL_N = 0,
	eNEX_HEVC_NALU_TYPE_TRAIL_R,
	eNEX_HEVC_NALU_TYPE_TSA_N,
	eNEX_HEVC_NALU_TYPE_TSA_R,
	eNEX_HEVC_NALU_TYPE_STSA_N,
	eNEX_HEVC_NALU_TYPE_STSA_R,
	eNEX_HEVC_NALU_TYPE_RADL_N,
	eNEX_HEVC_NALU_TYPE_RADL_R,
	eNEX_HEVC_NALU_TYPE_RASL_N,
	eNEX_HEVC_NALU_TYPE_RASL_R,
	eNEX_HEVC_NALU_TYPE_BLA_W_LP = 16,
	eNEX_HEVC_NALU_TYPE_BLA_W_RADL,
	eNEX_HEVC_NALU_TYPE_BLA_N_LP,
	eNEX_HEVC_NALU_TYPE_IDR_W_RADL,
	eNEX_HEVC_NALU_TYPE_IDR_N_LP,
	eNEX_HEVC_NALU_TYPE_CRA,
	eNEX_HEVC_NALU_TYPE_RSV_IRAP_VCL23 = 23,
	eNEX_HEVC_NALU_TYPE_VPS =32,
	eNEX_HEVC_NALU_TYPE_SPS,
	eNEX_HEVC_NALU_TYPE_PPS,
	eNEX_HEVC_NALU_TYPE_AUD,
	eNEX_HEVC_NALU_TYPE_EOS,
	eNEX_HEVC_NALU_TYPE_EOB,
	eNEX_HEVC_NALU_TYPE_FILLER,
	eNEX_HEVC_NALU_TYPE_PREFIX_SEI,
	eNEX_HEVC_NALU_TYPE_SUFFIX_SEI = 40

} NEX_HEVC_NAL_TYPE;

typedef enum
{
	eNEX_HEVC_MAIN_TIER = 0,
	eNEX_HEVC_HIGH_TIER = 1
} NEX_HEVC_TIER_TYPE;

typedef struct
{
	NEX_H26x_PROFILE	eProfile;
	NEX_HEVC_TIER_TYPE	eTier;
	NXUINT32			uLevel;
	NXUINT32		usWidth;
	NXUINT32		usHeight;
	NXUINT32		uWidthMBS;//pic_width_in_mbs
	NXUINT32		uHeightMBS;//pic_height_in_mbs
	NXUINT32		uNumRefFrames;
	NXBOOL			bConstrainedSet3;
	//NXBOOL		bInterlaced; // Interlaced or not
	NXBOOL			frame_mbs_only_flag;
	NXBOOL			mb_adaptive_frame_field_flag;
	NXBOOL			field_pic_flag;
	NXBOOL			bottom_field_flag;

	NXUINT32		uSARWidth;
	NXUINT32		uSARHeight;

	NXBOOL			separate_colour_plane_flag;
	NXUINT32		log2_max_frame_num;
	NXUINT32		log2_max_pic_order_cnt_lsb;
	NXUINT32		pic_order_cnt_type;
	NXUINT32		delta_pic_order_always_zero_flag;

	NXUINT32		timing_info_present_flag;
	NXUINT32		num_units_in_tick;
	NXUINT32		time_scale;
	NXUINT32		fixed_frame_rate_flag;

	NXBOOL			CpbDpbDelaysPresentFlag;
	NXUINT32		cpb_removal_delay_length;
	NXUINT32		dpb_output_delay_length;
	NXUINT32		time_offset_length;

	NXBOOL			pic_struct_present_flag;
	NXUINT32		slice_segment_address_length;

	// variable for HEVCDecoderConfigurationRecord
	NXINT8			general_profile_space;
	NXINT8			general_tier_flag;
	NXINT8			general_profile_idc;
	NXINT8			general_profile_compatibility_flag[32];

	NXUINT32		general_level_idc;
	
	NXINT8			vui_parameters_present_flag;
	NXINT8			bitstream_restriction_flag;
	
	NXUINT32		min_spatial_segmentation_idc;
	
	NXINT8			chroma_format_idc;
	NXINT8			bit_depth_luma_minus8;
	NXINT8			bit_depth_chroma_minus8;
	NXINT8			sps_max_sub_layers_minus1;
	
	NXINT8			sps_temporal_id_nesting_flag;

	NXUINT32		video_signal_type_present_flag;
	NXUINT32		video_format;
	NXUINT32		video_full_range_flag;
	
	NXUINT32		colour_description_present_flag;
	NXUINT32		colour_primaries;				// 1 : BT.709
													// 5 : unspecified
													// 9 : BT2020, BT2100

	NXUINT32		transfer_characteristics;		// 1  : BT.709
													// 5  : unspecified
													// 14 : BT.2020 (same as BT.709)
													// 15 : BT.2020 (same as BT.709)
													// 16 : BT.2100 perceptual quantization (PQ) system,
													//		ST2084(for 10, 12, 14 and 16-bit systems)

	NXUINT32		matrix_coefficients;			// 1  : BT.709
													// 5  : unspecified
													// 9  : BT.2020
													// 10 : BT.2020
													// 14 : BT.2100-0

	NXINT32			level_idc_offset;
} NEXCODECUTIL_SPS_INFO;

typedef struct
{
	NXCHAR		*pStart;
	NXUINT32	uLen;


	NXUINT32 pps_seq_parameter_set_id;
	NXUINT32 num_extra_slice_header_bits;
	NXUINT32 num_ref_idx_l0_default_active_minus1;
	NXUINT32 num_ref_idx_l1_default_active_minus1;
	NXUINT32 init_qp;
	NXUINT32 diff_cu_qp_delta_depth;
	NXINT32	 pps_cb_qp_offset;
	NXINT32  pps_cr_qp_offset;
	NXUINT32 num_tile_columns;
	NXUINT32 num_tile_rows;
	NXUINT32 column_width_minus1;
	NXUINT32 row_height_minus1;
	NXINT32  pps_beta_offset;
	NXINT32  pps_tc_offset;
	NXUINT32 log2_parallel_merge_level;

	NXUINT8 available;
	NXUINT8 dependent_slice_segments_enabled_flag;
	NXUINT8 output_flag_present_flag;
	NXUINT8 sign_data_hiding_flag;
	NXUINT8 cabac_init_present_flag;
	NXUINT8 constrained_intra_pred_flag;
	NXUINT8 transform_skip_enabled_flag;
	NXUINT8 cu_qp_delta_enabled_flag;
	NXUINT8 pps_slice_chroma_qp_offsets_present_flag;
	NXUINT8 weighted_pred_flag;
	NXUINT8 weighted_bipred_flag;
	NXUINT8 transquant_bypass_enabled_flag;
	NXUINT8 tiles_enabled_flag;
	NXUINT8 entropy_coding_sync_enabled_flag;
	NXUINT8 uniform_spacing_flag;
	NXUINT8 loop_filter_across_tiles_enabled_flag;
	NXUINT8 loop_filter_across_slices_enabled_flag;
	NXUINT8 deblocking_filter_control_present_flag;
	NXUINT8 deblocking_filter_override_enabled_flag;
	NXUINT8 pps_deblocking_filter_disabled_flag;
	NXUINT8 pps_scaling_list_data_present_flag;
	NXUINT8 lists_modification_present_flag;
	NXUINT8 slice_segment_header_extension_present_flag;
	NXUINT8 pps_extension_flag;

} NEXCODECUTIL_PPS_INFO;

typedef struct _NEXCODECUTIL_H264SliceHeaderInfo
{
	NXUINT32	first_mb_in_slice;
	NXUINT32	slice_type;
	NXUINT32	pic_parameter_set_id;
	NXUINT32	colour_plane_id;
	NXUINT32	frame_num;
	NXUINT32	field_pic_flag;
	NXUINT32	bottom_field_flag;
	NXUINT32	idr_pic_id;
	NXUINT32	pic_order_cnt_lsb;

} NEXCODECUTIL_AVC_SLICE_HEADER_INFO;

typedef struct
{
	NXBOOL		clock_timestamp_flag;
	NXUINT32	ct_type;
	NXBOOL		nuit_field_based_flag;
	NXUINT32	counting_type;
	NXBOOL		full_timestamp_flag;
	NXBOOL		discontinuity_flag;
	NXBOOL		cnt_dropped_flag;
	NXUINT32	n_frames;
	NXUINT32	seconds_value;
	NXUINT32	minutes_value;
	NXUINT32	hours_value;
	NXINT32		time_offset;

} NEXCODECUTIL_PIC_TIMING_CLOCK_TS_INFO;

typedef struct
{
	NXUINT32	cpb_removal_delay;
	NXUINT32	dpb_output_delay;
	NXUINT32	pic_struct;
	NXUINT32	NumClockTS;
	NEXCODECUTIL_PIC_TIMING_CLOCK_TS_INFO TSInfo[3];

} NEXCODECUTIL_PIC_TIMING;

typedef enum
{
	eNEXCODECUTIL_SEI_BUFFERING_PERIOD	= 0,
	eNEXCODECUTIL_SEI_PIC_TIMING		= 1,
	eNEXCODECUTIL_SEI_USER_DATA			= 5,
	eNEXCODECUTIL_SEI_APS				= 129,	// active_parameter_sets
	eNEXCODECUTIL_SEI_MDCV				= 137,	// mastering_display_colour_volume
	eNEXCODECUTIL_SEI_CRI				= 142,	// colour_remapping_info
	eNEXCODECUTIL_SEI_CLL				= 144,	// content_light_level_info

	eNEXCODECUTIL_SEI_TYPE_MAX			= 0x7FFFFFFF

} NEXCODECUTIL_SEI_TYPE;

// D.2.28 Mastering display colour volume SEI message syntax
typedef struct
{
	NXFLOAT		display_primaries[3][2];	// [GBR][xy]
	NXFLOAT		white_point[2];				// [xy]

	NXFLOAT		max_display_mastering_luminance;
	NXFLOAT		min_display_mastering_luminance;

	//NXFLOAT		prim_xyz_coord[3][3];
	//NXFLOAT		prim_xyz_coord_invt[3][3];
	//NXFLOAT		white_xyz_coord[3];

	//NXFLOAT		prim_XYZ_coord[3][3];
	//NXFLOAT		white_XYZ_coord[3];
} NEXCODECUTIL_SEI_MDCV_INFO;

// D.2.33 Colour remapping information SEI message
typedef struct
{
	NXUINT32	colour_remap_id;
	NXBOOL		colour_remap_cancel_flag;

	NXBOOL		colour_remap_persistence_flag;
	NXBOOL		colour_remap_video_signal_info_present_flag;

	NXBOOL		colour_remap_full_range_flag;
	NXUINT32	colour_remap_primaries;
	NXUINT32	colour_remap_transfer_function;
	NXUINT32	colour_remap_matrix_coefficients;

	NXUINT32	colour_remap_input_bit_depth;
	NXUINT32	colour_remap_output_bit_depth;

	// pre_lut_num_val_minus1[c] is equal to 0,
	// the default end points of the input/output values are 0 and (2^colour_remap_input/output_bit_depth) - 1
	// pre_lut_num_val_minus1[ c ] shall be in the range of 0 to 32
	NXUINT32	pre_lut_num_val_minus1[3];		// 0:luma or G, 1:Cb or B, 2:Cr or R
	NXUINT32	pre_lut_coded_value[3][33];
	NXUINT32	pre_lut_target_value[3][33];

	NXBOOL		colour_remap_matrix_present_flag;
	NXUINT32	log2_matrix_denom;
	NXINT32		colour_remap_coeffs[3][3];

	// post_lut_num_val_minus1[c] is equal to 0,
	// the default end points of the input/output values are 0 and (2^colour_remap_input/output_bit_depth) - 1
	// post_lut_num_val_minus1[ c ] shall be in the range of 0 to 32
	NXUINT32	post_lut_num_val_minus1[3];
	NXUINT32	post_lut_coded_value[3][33];
	NXUINT32	post_lut_target_value[3][33];

} NEXCODECUTIL_SEI_CRI_INFO;

// D.2.35 Content light level information SEI message syntax
typedef struct
{
	NXINT32		max_content_light_level;
	NXINT32		max_pic_average_light_level;
} NEXCODECUTIL_SEI_CLL_INFO;

typedef struct
{
	NXBOOL						MDCV_prsent_flag;
	NXBOOL						CRI_present_flag;
	NXBOOL						CLL_present_flag;

	NEXCODECUTIL_SEI_MDCV_INFO	stMDCVinfo;
	NEXCODECUTIL_SEI_CRI_INFO	stCRIinfo;
	NEXCODECUTIL_SEI_CLL_INFO	stCLLinfo;
} NEXCODECUTIL_SEI_HDR_INFO;

// AVC Annex G.7.3.1
typedef struct
{
	NXBOOL		idr_flag;
	NXINT32		priority_id;
	NXBOOL		no_inter_layer_pred_flag;
	NXINT32		dependency_id;
	NXINT32		quality_id;
	NXINT32		temporal_id;
	NXBOOL		use_ref_base_pic_flag;
	NXBOOL		discardable_flag;
	NXBOOL		output_flag;
	NXINT32		reserved_three_2bits;
} NEXCODECUTIL_SVC_EXT_INFO;


typedef enum
{
	eNEXCODECUTIL_3D_NONE,
	eNEXCODECUTIL_3D_LEFT_RIHGT,
	eNEXCODECUTIL_3D_TOP_BOTTOM

} NEXCODECUTIL_3D_TYPE;

typedef struct
{
	NXUINT32	uProfile;
	NXUINT32	uLevel;
	NXUINT32	uSourceFormat;	// H.263 인 경우, 7 이면 extended PTYPE
	NXUINT32	usWidth;
	NXUINT32	usHeight;
	NXUINT32	uGMC; //GMC option on/off
	NXUINT32	uVOPTimeIncBits; //To check the coded VOP type
	NXBOOL		bInterlaced; // Interlaced or not
	NXBOOL		bQPEL; //To check QPEL option
	NXUINT32	no_of_sprite_warping_points; //no_of_sprite_warping_points
	NXUINT32	data_partitioned;
	NXBOOL		AnnexD; // Check H.263 UMV

} NEXCODECUTIL_MPEG4V_DSI_INFO;

typedef struct
{
	NXUINT32	uProfile; /* 0 - Simple, 1 - Main, 2 - Complex, 3 - Advanced */
	NXBOOL		bNotYUV420;
} NEXCODECUTIL_VC1_EXTRADATA_INFO;

#define MAX_PackedBitstream 4
typedef struct
{
	NXUINT32	nFrameCount;
	NXUINT32	nOffset[MAX_PackedBitstream];
} NEXCODECUTIL_PACKED_BITSTREAM_INFO;

typedef struct
{
	NXUINT32 first_slice_segment_in_pic_flag;
	NXUINT32 dependent_slice_segment_flag;
	NXUINT32 slice_type;
	NXUINT32 nal_unit_type;
	NXUINT32 pic_output_flag;
	NXUINT32 slice_pic_order_cnt_lsb;
	NXUINT32 pic_parameter_set_id;
	NXUINT32 colour_plane_id;
	NXUINT32 frame_num;
	NXUINT32 field_pic_flag;
	NXUINT32 bottom_field_flag;
	NXUINT32 idr_pic_id;
	NXUINT32 pic_order_cnt_lsb;
} NEXCODECUTILHEVCSliceHeaderInfo;

typedef struct
{
	NXUINT32	BS;
	NXUINT32	BitCount;
	NXUINT8		*endbuf;
	NXUINT8		*buffer;
	NXUINT32	rear;
	NXUINT32	length;
	NXUINT32	remain_bits;
} BITSTREAM;

typedef struct {
	NXUINT32	BS;
	NXUINT32	BitCount;
	NXUINT32	front, rear;
	NXUINT32	length;
	NXUINT8		*buffer;
} WBITSTREAM;


typedef enum NEX_MPEG_AUDIO_VER
{
	NEX_MPEG_AUDIO_VER_REV = 0,			// resereved
	NEX_MPEG_AUDIO_VER_1   = 1,			// MPEG 1
	NEX_MPEG_AUDIO_VER_2   = 2,			// MPEG 2
	NEX_MPEG_AUDIO_VER_25  = 3,			// MPEG 2.5
	NEX_MPEG_AUDIO_VER_MAX = 0x7fffffff
}NEX_MPEG_AUDIO_VER;

typedef enum NEX_MPEG_AUDIO_LAYER
{
	NEX_MPEG_AUDIO_LAYER_REV = 0,		// reserved
	NEX_MPEG_AUDIO_LAYER_1   = 1,		// Layer I
	NEX_MPEG_AUDIO_LAYER_2   = 2,		// Layer II
	NEX_MPEG_AUDIO_LAYER_3   = 3,		// Layer III
	NEX_MPEG_AUDIO_LAYER_MAX = 0x7fffffff
}NEX_MPEG_AUDIO_LAYER;

typedef enum NEX_MPEG_AUDIO_CHANNEL
{
	NEX_MPEG_AUDIO_STEREO         = 0,		// stereo
	NEX_MPEG_AUDIO_JOINT_STEREO   = 1,		// stereo
	NEX_MPEG_AUDIO_DUAL_CHANNEL   = 2,		// 2 mono channels
	NEX_MPEG_AUDIO_SINGLE_CHANNEL = 3,		// mono
	NEX_MPEG_AUDIO_CHANNEL_MAX    = 0x7fffffff
}NEX_MPEG_AUDIO_CHANNEL;

static const NXUINT32 NEX_MPEG_AUDIO_SAMPLE_NUM[4][4]={
/* Layer   x	I		II		III */
		  {0, 0, 0, 0},
		  {0,	384,	1152,	1152},	/* MPEG-1   */
		  {0,	384,	1152,	576},	/* MPEG-2   */
		  {0,	384,	1152,	576}	/* MPEG-2.5 */
};

static NXUINT32 NEX_MPEG_AUDIO_SAMPLING_FREQUENCY[4][3] = {
	0, 0, 0,
	44100, 48000, 32000,	/* MPEG 1   */
	22050, 24000, 16000,	/* MPEG 2   */
	11025, 12000, 8000		/* MPEG 2.5 */
};

typedef struct MPEG_AUDIO_INFO
{
	NEX_MPEG_AUDIO_VER     Version;
	NEX_MPEG_AUDIO_LAYER   Layer;
	NEX_MPEG_AUDIO_CHANNEL Channel;
	NXINT32                SamplingFrequency;
}MPEG_AUDIO_INFO, *PMPEG_AUDIO_INFO;


#define NEXCODECUTIL_TIMEINCBITS_INVALID	0xFFFFFFFF
#define NEXCODECUTIL_TIMEINCBITS_ENCRYPTED	0xFFFFFFFE // To avoid checking coded frame

#ifdef __cplusplus
extern "C" {
#endif

NEXCODECUTIL_API const NXCHAR* NexCodecUtil_GetVersion();
NEXCODECUTIL_API NXUINT32	NexCodecUtil_ReadBigEndianValue8( NXUINT8* pData );
NEXCODECUTIL_API NXUINT32	NexCodecUtil_ReadBigEndianValue16( NXUINT8* pData );
NEXCODECUTIL_API NXUINT32	NexCodecUtil_ReadBigEndianValue24( NXUINT8* pData );
NEXCODECUTIL_API NXUINT32	NexCodecUtil_ReadBigEndianValue32( NXUINT8* pData );

NEXCODECUTIL_API NXINT32 _AlignBits(BITSTREAM *bs);
NEXCODECUTIL_API NXINT32 _AlignBitsAVC(BITSTREAM *bs);
NEXCODECUTIL_API NXINT32 _LoadBS(BITSTREAM *bs);
NEXCODECUTIL_API NXINT32 _LoadBSAVC(BITSTREAM *bs);
NEXCODECUTIL_API NXUINT32 _GetUE_V(BITSTREAM *bs);
NEXCODECUTIL_API NXUINT32 _GetUE_V_AVC(BITSTREAM *bs);
NEXCODECUTIL_API NXUINT32 _ReadNShiftBits(BITSTREAM *bs, NXINT32 n);
NEXCODECUTIL_API NXUINT32 _ReadNShiftBitsAVC(BITSTREAM *bs, NXINT32 n);

// MPEG Audio Info.
NEXCODECUTIL_API NXUINT32 NexCodecUtil_Get_MPEG_Audio_Info( NXUINT8* pData, NXUINT32 iDataLen, MPEG_AUDIO_INFO *mpa_info );

// the following 4 functions is for H.264
NEXCODECUTIL_API NXUINT8*	NexCodecUtil_FindNAL( NXUINT8* pStart, NXINT32 iInLen, NXUINT32 uByteFormat, NXUINT32 iNALSize, NEX_NAL_TYPE eFindNAL, NXINT32 *iOutLen );
NEXCODECUTIL_API NXUINT32	NexCodecUtil_CheckByteFormat( NXUINT8* pFrame, NXINT32 iLen );
NEXCODECUTIL_API NXINT32	NexCodecUtil_FindStartCode( NXUINT8* pSource, NXINT32 iLen );
NEXCODECUTIL_API NXINT32	NexCodecUtil_FindAnnexBStartCode(NXUINT8 *frame, NXINT32 start, NXINT32 frameLength, NXINT32 *codeLength);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_AVC_IsAVCReferenceFrame(NXUINT8 *pSource, NXUINT32 uSize, NXUINT32 uByteFormat, NXUINT32 uNalLengthBytes);
NEXCODECUTIL_API NXINT32	NexCodecUtil_AVC_ParseH264NalHeaderLengthSize( NXUINT8 *pDSI, NXUINT32 iDSILen);
NEXCODECUTIL_API NXINT32	NexCodecUtil_GuessNalHeaderLengthSize( NXUINT8 *pSource, NXUINT32 iSourceLen);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_AVC_IsStartAUDelimiterNAL( NXUINT8 *pSource, NXUINT32 iSourceLen, NXINT32 iNALHeaderLengthSize, NXUINT32 uByteFormat);

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_AddAUDelimiter_ConvertFormat
	( NXUINT8 *pTarget
	, NXUINT32 iTargetLen
	, NXUINT8 *pSource
	, NXUINT32 iSourceLen
	, NXUINT32 iTargetFormat
	, NXINT32 iNALHeaderLengthSize );

NEXCODECUTIL_API NXINT32 NexCodecUtil_ConvertFormat
	( NXUINT8 *pTarget
	, NXUINT32 iTargetLen
	, NXUINT8 *pSource
	, NXUINT32 iSourceLen
	, NXUINT32 uByteFormat
	, NXINT32 iNALHeaderLengthSize );

NEXCODECUTIL_API NXINT32 NexCodecUtil_ConvertFormat2
	( NXUINT8 *pTarget
	, NXUINT32 iTargetLen
	, NXUINT8 *pSource
	, NXUINT32 iSourceLen
	, NXUINT32 uSrcFormat
	, NXUINT32 uDstFormat
	, NXINT32 iSrcNALHeaderLengthSize
	, NXINT32 iDstNALHeaderLengthSize);

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_ConvertFormatPSs
	( NXUINT8 *pTarget
	, NXINT32 iTargetLen
	, NXUINT8 *pSource
	, NXINT32 iSourceLen
	, NXUINT32 uByteFormat );

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_ConvertFormatPSs2
	( NXUINT8 *pTarget
	, NXINT32 iTargetLen
	, NXUINT8 *pSource
	, NXINT32 iSourceLen
	, NXUINT32 uSrcByteFormat
	, NXUINT32 uDstByteFormat
	, NXUINT32 uDstNALHeaderLengthSize
	, NXBOOL bFrameMode);

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_GetSPSFormatAbnormalType
	( NXUINT8 *pSource
	, NXINT32 iSourceLen
	, NXINT32 iNALHeaderLengthSize );

NEXCODECUTIL_API NXBOOL NexCodecUtil_AVC_ConvertAbnormalSPSFormat
	( NXUINT8 *pNewConfig
	, NXUINT8 *pSource
	, NXINT32* iSourceLen
	, NXINT32 iNALHeaderLengthSize
	, NXINT32 iType );

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_SPSExists
	( NXUINT8 *pSource
	 , NXINT32 iSourceLen
	 , NXUINT32 uByteFormat
	 , NXINT32 iNALHeaderLengthSize );

NEXCODECUTIL_API NXUINT8* NexCodecUtil_AVC_ANNEXB_GetConfigStream
	( NXUINT8 *frame
		, NXINT32 frameLength
		, NXINT32 *resultLength );

NEXCODECUTIL_API NXUINT8* NexCodecUtil_AVC_NAL_GetConfigStream
	( NXUINT8 *frame
		, NXINT32 frameLength
		, NXINT32 sizeOfNAL
		, NXINT32 *resultLength );

NEXCODECUTIL_API NXBOOL NexCodecUtil_AVC_IsConfigFromRecordType(NXUINT8* a_pConfig, NXINT32 a_nLen);

NEXCODECUTIL_API NXBOOL NexCodecUtil_AVC_IsPictureFrame(NXUINT8 *a_pFrame, NXINT32 a_nLen, NXUINT32 a_uByteFormat, NXINT32 a_nNALHeaderLen, NXUINT32 *a_puOffset);

NEXCODECUTIL_API NXBOOL NexCodecUtil_IsValidVideo
					( NEX_CODEC_TYPE iVideoObjType
					, NXUINT8* pFrame
					, NXINT32 iFrameSize );

/*
NEX_CODEC_TYPE							pETC input argument
-------------------------------------------
eNEX_CODEC_V_H264						NALHeader Length
eNEX_CODEC_V_WMV3,eNEX_CODEC_V_WVC1		ExtraData(DSI)
else									NULL
-------------------------------------------
*/
NEXCODECUTIL_API NXBOOL NexCodecUtil_IsBFrame
					( NEX_CODEC_TYPE iVideoObjType
					, NXUINT8* pFrame
					, NXINT32 iFrameSize
					, NXVOID *pETC
					, NXUINT32 uByteFormat
					, NXBOOL bOnlyNotRefIDC );
/*
NEX_CODEC_TYPE							pETC input argument
-------------------------------------------
eNEX_CODEC_V_DIVX,eNEX_CODEC_V_MPEG4V	VOPTimeIncBits
eNEX_CODEC_V_H264,eNEX_CODEC_V_HEVC		NALHeader Length
eNEX_CODEC_V_WMV3,eNEX_CODEC_V_WVC1		ExtraData(DSI)
else									NULL
-------------------------------------------
*/
NEXCODECUTIL_API NXBOOL NexCodecUtil_IsSeekableFrame
					( NEX_CODEC_TYPE iVideoObjType
					, NXUINT8* pFrame
					, NXINT32 iFrameSize
					, NXVOID *pETC
					, NXUINT32 uByteFormat
					, NXBOOL bOnlyIDR);

NEXCODECUTIL_API NXINT32 NexCodecUtil_AAC_MakeDSI
	( NXUINT32 uiInputSamplingRate /* in */
	, NXUINT32 uiChannel /* in */
	, NEX_AAC_PROFILE eAACProfile /* in */
	, NXUINT8** ppAACDSI /* out */
	);

// the following NexCodecUtil_AAC_ParseDSI function is for AAC
NEXCODECUTIL_API NXINT32 NexCodecUtil_AAC_ParseDSI
	( NXUINT8 *pConfig
	, NXUINT32 uConfigLen
	, NXUINT32 *samplerate
	, NXUINT32 *channels
	, NXINT32 *bSBR);

NEXCODECUTIL_API NXINT32 NexCodecUtil_GetAACType
	( NEX_CODEC_TYPE uOTI
	, NXUINT32 uSamplingRate
	, NXUINT32 uNumOfChannels
	, NXUINT8 *pBuffer
	, NXUINT32 uBufferLength);

NEXCODECUTIL_API NXINT32 NexCodecUtil_GetSizeOfADIFHeader
	(NXUINT8 *pBuffer
	, NXUINT32 uBufferLength);

NEXCODECUTIL_API NXINT32 NexCodecUtil_GetSizeOfADTSHeader ( NXUINT8 *pSource );

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_SEI_GetPicTiming( NXUINT8 *a_pData, NXINT32 a_iLen, NXUINT32 a_uByteFrame, NXINT32 a_iNALSize, NEXCODECUTIL_SPS_INFO *a_pSPSInfo, NEXCODECUTIL_PIC_TIMING *a_pPicTiming );

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_CheckStereoscopic3D( NXUINT8 *pData, NXINT32 iLen, NXUINT32 uByteFrame, NXINT32 iNALSize, NEXCODECUTIL_3D_TYPE *pt3D );

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_FindAnnexBStartCode(NXUINT8 *pBuffer, NXINT32 nLength, NXINT32 *pLengthBetweenStartCodes, NXINT32 *pFirstStartCodeOffset, NXINT32 *pFirstStartCodeLength);

/*
---- Comment about 'NexCodecUtil_AVC_FindAnnexBStartCode()' ----

1. Input parameter
	- pLengthBetweenStartCodes : Length from 'First start code point' to 'Second start code point'.
	- pFirstStartCodeOffset : Offset value from 'pBuffer start point' to 'First start code point'. 'pBuffer start point' is 0.
	- pFirstStartCodeLength : First start code length. The value is 3 or 4.

2. function return value
	-  0 : First and Second start code was found.
	-  1 : Only First start code was found. In this case, 'pLengthBetweenStartCodes' value is length from 'First start code point' to 'pBuffer end point'.
	- -1 : Both First and Second start code was not found.

---- End of Comment ----
*/

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_GetSliceHeaderInfo(NXINT8 *a_pFrame, NXUINT32 a_uFrameLen, NEXCODECUTIL_SPS_INFO *a_pstSPSInfo, NXUINT32 a_uNalHeaderLen, NXUINT32 a_uFormat, NEXCODECUTIL_AVC_SLICE_HEADER_INFO *a_pstSliceHeaderInfo);

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_ResetPOC(NXINT8 *a_pFrame, NXUINT32 a_uFrameLen,
												   NEXCODECUTIL_SPS_INFO *a_pstSPSInfo,
												   NXUINT32 a_uNalHeaderLen, NXUINT32 a_uFormat);

NEXCODECUTIL_API NXBOOL NexCodecUtil_AVC_DecodeSEI(NXUINT8 *ptr, NXINT32 length);

NEXCODECUTIL_API NXINT32 NexCodecUtil_HEVC_GetPPSInfo( NXINT8* pConfig, NXINT32 nLen, NEXCODECUTIL_PPS_INFO* pPPS, NXUINT32 uByteFormat);

NEXCODECUTIL_API NXINT32 NexCodecUtil_HEVC_GetSliceHeaderInfo(NXINT8 *a_pFrame, NXUINT32 a_uFrameLen,
								  NXUINT32 *uPicOrderCntLsb, NXINT32 *iPicOrderCntMsb ,NXUINT32 *uPrevPOCLsb , NXINT32 *nPrevPOCMsb , /*chunk info */
								  NEXCODECUTIL_SPS_INFO *pSPS , /*sps info */NEXCODECUTIL_PPS_INFO *pPPS, /*pps info*/
								  NXUINT32 a_uNalHeaderLen, NXUINT32 a_uFormat, NEXCODECUTILHEVCSliceHeaderInfo *a_pstSHInfo, NXBOOL bFirstPicDone);

NEXCODECUTIL_API NXBOOL NexCodecUtil_AVC_IsDirectMixable(NXUINT8* pConfig1, NXINT32 nLen1, NXUINT8* pConfig2, NXINT32 nLen2);

NEXCODECUTIL_API NXBOOL NexCodecUtil_AVC_GetSVCExtInfo
					( NXUINT8 *a_pFrame
					, NXINT32 a_nLen
					, NXUINT32 a_uByteFormat
					, NXINT32 a_nNALHeaderLen
					, NEXCODECUTIL_SVC_EXT_INFO *pSVCExtInfo );

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_GetSPSInfo
					( NXCHAR* pConfig
					, NXINT32 nLen
					, NEXCODECUTIL_SPS_INFO* pRet
					, NXUINT32 uByteFormat );

NEXCODECUTIL_API NXINT32 NexCodecUtil_AVC_GetPPSInfo
					( NXCHAR* pConfig
					, NXINT32 nLen
					, NEXCODECUTIL_PPS_INFO* pRet
					, NXUINT32 uByteFormat );

NEXCODECUTIL_API NXINT32 NexCodecUtil_MPEG4V_GetDSIInfo
					( NEX_CODEC_TYPE iVideoObjType
					, NXCHAR* pDSI
					, NXINT32 nLen
					, NEXCODECUTIL_MPEG4V_DSI_INFO* pRet );

NEXCODECUTIL_API NXBOOL	NexCodecUtil_MPEG4V_IsDirectMixable(NXUINT8* pConfig1, NXINT32 nLen1, NXUINT8* pConfig2, NXINT32 nLen2);

NEXCODECUTIL_API NXBOOL NexCodecUtil_H263_IsEmulatedStartCode
					( NXUINT8 *a_pFrame
					, NXUINT32 a_uFrameLength );

NEXCODECUTIL_API NXINT32 NexCodecUtil_VC1_GetExtraDataInfo
                    ( NXCHAR* pExtraData
                    , NXINT32 nLen
					, NEX_CODEC_TYPE uiCodecType
                    , NEXCODECUTIL_VC1_EXTRADATA_INFO *pRet );

NEXCODECUTIL_API NXINT32 NexCodecUtil_WVC1_GetSeqHeaderStartPoint( NXUINT8 *pFrame, NXINT32 nFrameLen, NXUINT8 **ppSeqHeaderStartPoint, NXINT32 *pSeqLen );

NEXCODECUTIL_API NXINT32	NexCodecUtil_AVC_GetOneFrame( NXUINT8* pFrame, NXINT32 iFrameSize, NXINT32 etc );
NEXCODECUTIL_API NXINT32	NexCodecUtil_Make_ADTS_Header( NXCHAR *pDSI, NXINT32 nSample, NXCHAR* pTarget );
NEXCODECUTIL_API NXBOOL		NexCodecUtil_IsPackedBitstream( NXUINT8* pFrame, NXINT32 iFrameSize, NEXCODECUTIL_PACKED_BITSTREAM_INFO* pPBInfo );
NEXCODECUTIL_API NXUINT8*	NexCodecUtil_FindVOP(NXUINT8* start,NXINT32 iInLen, NXINT32 *iOutLen);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_CheckCodedVOP(NXUINT8 *a_pData, NXUINT32 a_uSize, NXUINT32 a_uTimeIncBits);
NEXCODECUTIL_API NXINT32	NexCodecUtil_ASP_VideoDecGuessVOL(NXUINT8 *frameData, NXINT32 length, NXUINT32 width, NXUINT32 height, NXUINT8 *outBuffer, NXUINT8 *outSize);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_ASP_CheckValidVOL(NXUINT8 *a_pVOLHeader);

#if 0
NEXCODECUTIL_API NXINT32	NexCodecUtil_ASP_FindStartCode(NXUINT8 *a_pSource, NXINT32 a_nSourceLen);
NEXCODECUTIL_API NXINT32	NexCodecUtil_ASP_RemoveReservedStartCode(NXUINT8 *a_pSource, NXINT32 a_nSourceLen, NXINT32 *a_pnDestLen);
#endif
NEXCODECUTIL_API NXUINT32	NexCodecUtil_AVC_LevelLimits( NXINT32 nWidth, NXINT32 nHeight );
NEXCODECUTIL_API NXBOOL		NexCodecUtil_AVC_IsErrorFrame(NXUINT8 *pSource, NXUINT32 iSourceLen, NXUINT32 uByteFormat, NXINT32 iNALHeaderLengthSize);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_DoesAudioDecoderReinit( NXUINT32 uOTI, NXUINT8 * pNew, NXINT32 iNewLen, NXUINT8 * pOld, NXINT32 iOldLen );
NEXCODECUTIL_API NXINT32	NexCodecUtil_AVC_EntropyMode( NXUINT8 *pDSI, NXINT32 iLen, NXUINT32 uByteFormat );
NEXCODECUTIL_API NXUINT32	NexCodecUtil_AVC_TotalMacroBlocks_To_Level_min30fps( NXINT32 nMBTotal );
NEXCODECUTIL_API NXINT32	NexCodecUtil_AVC_MakeDSI(NXUINT8 *a_pSource, NXUINT32 a_uSourceLen, NXUINT32 a_uByteFormat, NXUINT32 a_uNALHeaderLen, NXUINT8 *a_pOutDSI, NXUINT32 *a_uOutDSILen);
NEXCODECUTIL_API NXINT32	NexCodecUtil_ASP_CheckVOPHeader(NXUINT8 *frameData, NXINT32 length, NXINT32 vop_time_inc_bits);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_GetSPSInfo( NXCHAR* pConfig, NXINT32 nLen, NEXCODECUTIL_SPS_INFO* pRet, NXUINT32 uByteFormat);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_ConvertFormatPSs(NXUINT8 *pTarget, NXINT32 iTargetLen, NXUINT8 *pSource, NXINT32 iSourceLen, NXUINT32 uByteFormat);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_ConvertFormatPSs2(NXUINT8 *pTarget, NXINT32 iTargetLen, NXUINT8 *pSource, NXINT32 iSourceLen, NXUINT32 uSrcByteFormat, NXUINT32 uDstByteFormat, NXUINT32 uDstNALHeaderLengthSize, NXBOOL bFrameMode);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_SPSExists( NXUINT8 *pSource , NXINT32 iSourceLen, NXUINT32 uByteFormat, NXINT32 iNALHeaderLengthSize );
NEXCODECUTIL_API NXBOOL		NexCodecUtil_HEVC_IsPictureFrame(NXUINT8 *a_pFrame, NXINT32 a_nLen, NXUINT32 *a_puOffset);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_HEVC_IsPictureFrame_Raw(NXUINT8 *a_pFrame, NXINT32 a_nLen, NXUINT32 *a_puOffset, NXUINT32 a_uNalLengthBytes);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_HEVC_IsPictureFrame_withType(NXUINT8 *a_pFrame, NXINT32 a_nLen, NXUINT32 *a_puOffset, NXUINT32 a_uNalLengthBytes, NXUINT32 uFormat);
NEXCODECUTIL_API NXUINT8*	NexCodecUtil_HEVC_ANNEXB_GetConfigStream( NXUINT8 *frame, NXINT32 frameLength, NXINT32 *resultLength );
NEXCODECUTIL_API NXUINT8*	NexCodecUtil_HEVC_NAL_GetConfigStream(NXUINT8 *frame, NXINT32 frameLength, NXINT32 sizeOfNAL, NXINT32 *resultLength);
NEXCODECUTIL_API NXUINT8*	NexCodecUtil_HEVC_FindNAL( NXUINT8* pStart, NXINT32 iInLen, NXUINT32 iNALSize, NEX_HEVC_NAL_TYPE eFindNAL, NXINT32 *iOutLen );
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_GetNalCount( NXUINT8* pStart, NXINT32 iInLen, NEX_HEVC_NAL_TYPE eFindNAL );
NEXCODECUTIL_API NXBOOL		NexCodecUtil_HEVC_IsStartAUDelimiterNAL( NXUINT8 *pSource, NXUINT32 iSourceLen, NXINT32 iNALHeaderLengthSize);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_SEI_GetPicTiming( NXUINT8 *a_pData, NXINT32 a_iLen, NXINT32 a_iNALSize, NEXCODECUTIL_SPS_INFO *a_pSPSInfo, NEXCODECUTIL_PIC_TIMING *a_pPicTiming );
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_SEI_GetHDRInfo( NXUINT8 *a_pData, NXINT32 a_iLen, NXINT32 a_iNALSize, NEXCODECUTIL_SPS_INFO *a_pSPSInfo, NEXCODECUTIL_SEI_HDR_INFO *a_pHDRInfo );
NEXCODECUTIL_API NXBOOL		NexCodecUtil_HEVC_IsConfigFromRecordType(NXUINT8* a_pConfig, NXINT32 a_nLen, NXINT32 *a_pnPSStartPos);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_ParseNalHeaderLengthSize(NXUINT8 *a_pConfig, NXUINT32 a_pConfigLen);
NEXCODECUTIL_API NXUINT32	NexCodecUtil_HEVC_GetAccessUnit(NXUINT8 *buffer, NXUINT32 length, NXUINT32 NalLengthBytes, NXUINT32 uHEVCFormat);
NEXCODECUTIL_API NXUINT32	NexCodecUtil_HEVC_GetAccessUnit_AnnexB(NXUINT8 *buffer, NXUINT32 length);
NEXCODECUTIL_API NXUINT32	NexCodecUtil_HEVC_GetAccessUnit_Raw(NXUINT8 *buffer, NXUINT32 length, NXUINT32 NalLengthBytes);
NEXCODECUTIL_API NXBOOL		NexCodecUtil_HEVC_IsDirectMixable(NXUINT8* pConfig1, NXINT32 nLen1, NXUINT8* pConfig2, NXINT32 nLen2);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_isPrefixConfigFrame(NXUINT8 *a_pFrame, NXINT32 a_nLen, NXUINT32 a_uNalLengthBytes, NXUINT32 uFormat);
NEXCODECUTIL_API NXINT32	NexCodecUtil_HEVC_isSuffixConfigFrame(NXUINT8 *a_pFrame, NXINT32 a_nLen, NXUINT32 a_uNalLengthBytes, NXUINT32 uFormat);

/*	Audio Mute Frame Maker

	-주의1 MakeADTS함수에서 buffer는 ADTS가 저장될 메모리 주소며, 7바이트의 크기를 갖고 있어야 함.
	-주의2 MPEG4인 AAC의 경우 ADTS 없이 사용해야 함.
	-주의3 MuteFrame함수에서의 buffer는 디코더에 입력되는 오디오 스트림 버퍼의 주소며, MuteFrame함수 실행전과 실행후에 NxAACDecReset을 해주어야 함.

	-Make ADTS 함수의 사용법은 MuteFrmae 함수에서의 참조

	-사용예
		iRet = MuteFrame(pBufferPos, MPEGTYPE2, dwSampleRate, dwChannelNum, ADTS_PRESENT, AACLCMONO);
		if(iRet == MuteERROR)
			printf("MuteFrame ERROR!!!\n");
*/
NEXCODECUTIL_API NXINT32 NexCodecUtil_AAC_MakeADTS(NXUINT8 *buffer, NXINT32 samplingrate, NXINT32 channel, NXINT32 aac_mode, NXUINT32 framesize);
NEXCODECUTIL_API NXINT32 NexCodecUtil_AAC_MuteFrame(NXUINT8 **pFrame, NXUINT32 *uFrameLen, NXINT32 mpegtype, NXINT32 samplingrate, NXINT32 channel, NXINT32 adtsflag, NXINT32 aac_mode);
NEXCODECUTIL_API NXINT32 NexCodecUtil_MP3_MuteFrame(NXUINT8 **pFrame, NXUINT32 *uFrameLen, NXINT32 samplingrate, NXINT32 channel);

NEXCODECUTIL_API NXINT32 NexCodecUtil_AMRNB_MuteFrame (NXUINT8 **pFrame, NXUINT32 *uFrameLen, NXINT32 FrameType);
NEXCODECUTIL_API NXINT32 NexCodecUtil_AMRWB_MuteFrame (NXUINT8 **pFrame, NXUINT32 *uFrameLen, NXINT32 FrameType);
NEXCODECUTIL_API NXINT32 NexCodecUtil_DRA_MuteFrame(NXUINT8 **pFrame, NXUINT32 *uFrameLen, NXINT32 samplingrate, NXINT32 channel);

#ifdef __cplusplus
}
#endif

/*-----------------------------------------------------------------------------
*/
//Return value of Make ADTS Header function
#define NEXCODECUTIL_MakeADTSOK			0
#define NEXCODECUTIL_MakeADTSERROR		-1
//Make ADTS Header AAC Profile
#define NEXCODECUTIL_AAC_LC				2
//ADTS Header Length
#define NEXCODECUTIL_ADTS_LENGTH		7
//aac_mode in AAC Mute function
#define NEXCODECUTIL_AACLCMONO			0
#define NEXCODECUTIL_AACLCSTEREO		1
#define NEXCODECUTIL_AACPLUSMONO		2
#define NEXCODECUTIL_AACPLUSSTEREO		3
#define NEXCODECUTIL_EAACPLUSSTEREO		4

//주의 MPEG4인 AAC의 경우 ADTS 없이 사용해야 함.
//일반적인 경우 MPEGTYPE2를 쓴다
#define NEXCODECUTIL_MUTE_MPEGTYPE2		0
#define NEXCODECUTIL_MUTE_MPEGTYPE4		1

//AAC Mute 함수에서의 변수 adtsflag 정리
#define NEXCODECUTIL_MUTE_ADTS_ABSENT	0
#define NEXCODECUTIL_MUTE_ADTS_PRESENT	1

//AAC Mute 함수의 리턴값
#define NEXCODECUTIL_MUTE_SUCCESS		0
#define NEXCODECUTIL_MUTE_ERROR			-1

#endif //_NEXCODECUTIL_DOT_H_INCLUDED_

/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date		Description of Changes
 ------------------------------------------------------------------------------
 ysh		07-06-14	Draft.
 ----------------------------------------------------------------------------*/
