/******************************************************************************
* File Name        : h265.cpp
* Description      : h265 codec for Android
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#define LOCATION_TAG "hevc"

#undef LOG_TAG
#define LOG_TAG "nexcral_mc"

#include "NexCAL.h"

#include "NexCRALBody_mc_impl.h"
#include "external/NexMediaCodec.h"
#include "external/jni/jni_MediaCodec.h"
#include "external/jni/jni_MediaFormat.h"

#include "utils/utils.h"
#include "utils/jni.h"

#ifdef NEX_MC_AVTYPE
#undef NEX_MC_AVTYPE
#endif
#define NEX_MC_AVTYPE NEX_MC_AVTYPE_VIDEO

namespace Nex_MC {

using Common::NexCAL_mc;

namespace HEVC {

const char *MIMETYPE = "video/hevc";

namespace { // (anon1)

typedef enum HEVC_PROFILE
{
	HEVC_PROFILE_NONE = 0,
	HEVC_PROFILE_MAIN = 1,
	HEVC_PROFILE_MAIN10 = 2,
	HEVC_PROFILE_MAINSTILLPICTURE = 3,
	HEVC_PROFILE_MAINREXT = 4,
	HEVC_PROFILE_HIGHTHROUGHPUTREXT = 5
}HEVC_PROFILE;

typedef enum HEVC_LEVEL
{
    HEVC_LEVEL_10 = 10,
    HEVC_LEVEL_20 = 20,
    HEVC_LEVEL_21 = 21,
    HEVC_LEVEL_30 = 30,
    HEVC_LEVEL_31 = 31,
    HEVC_LEVEL_40 = 40,
    HEVC_LEVEL_41 = 41,
    HEVC_LEVEL_50 = 50,
    HEVC_LEVEL_51 = 51,
    HEVC_LEVEL_52 = 52,
    HEVC_LEVEL_60 = 60,
    HEVC_LEVEL_61 = 61,
    HEVC_LEVEL_62 = 62,
}HEVC_LEVEL;

static const unsigned int hevc_max_support_luma_pic_size[] =
{
    36864,		/* Level 1   */
    122880,		/* Level 2   */
    245760,		/* Level 2.1 */
    552960,		/* Level 3   */
    983040,		/* Level 3.1 */
    2228224,	/* Level 4   */
    2228224,	/* Level 4.1 */
    8912896,	/* Level 5   */
    8912896,	/* Level 5.1 */
    8912896,	/* Level 5.2 */
    33423360,	/* Level 6   */
    33423360,	/* Level 6.1 */
    33423360	/* Level 6.2 */
};

static const unsigned int hevc_max_support_sample_rate_size[] =
{
    552960,		/* Level 1   */
    3686400,	/* Level 2   */
    7372800,	/* Level 2.1 */
    16588800,	/* Level 3   */
    33177600,	/* Level 3.1 */
    66846720,	/* Level 4   */
    133693440,	/* Level 4.1 */
    267386880,	/* Level 5   */
    534773760,	/* Level 5.1 */
    1069547520,	/* Level 5.2 */
    1069547520,	/* Level 6   */
    2139095040,	/* Level 6.1 */
    4278190080U	/* Level 6.2 */
};

int hevc_get_level_index(int level)
{
    int idx = 0;

    if(level < HEVC_LEVEL_20)
        idx = 0;
    else if(level >= HEVC_LEVEL_20 && level < HEVC_LEVEL_21)
        idx = 1;
    else if(level >= HEVC_LEVEL_21 && level < HEVC_LEVEL_30)
        idx = 2;
    else if(level >= HEVC_LEVEL_30 && level < HEVC_LEVEL_31)
        idx = 3;
    else if(level >= HEVC_LEVEL_31 && level < HEVC_LEVEL_40)
        idx = 4;
    else if(level >= HEVC_LEVEL_40 && level < HEVC_LEVEL_41)
        idx = 5;
    else if(level >= HEVC_LEVEL_41 && level < HEVC_LEVEL_50)
        idx = 6;
    else if(level >= HEVC_LEVEL_50 && level < HEVC_LEVEL_51)
        idx = 7;
    else if(level >= HEVC_LEVEL_51 && level < HEVC_LEVEL_52)
        idx = 8;
    else if(level >= HEVC_LEVEL_52 && level < HEVC_LEVEL_60)
        idx = 9;
    else if(level >= HEVC_LEVEL_60 && level < HEVC_LEVEL_61)
        idx = 10;
    else if(level >= HEVC_LEVEL_61 && level < HEVC_LEVEL_62)
        idx = 11;
    else if(level >= HEVC_LEVEL_62)
        idx = 12;

    return (idx);
}
}; // namespace (anon1)

namespace Common {

namespace { // anon321
	typedef unsigned int (*bigEndianReader)(unsigned char *pData);

	unsigned char NAL_START_CODE[] = { 0x00, 0x00, 0x00, 0x01 };

	// pTarget must be different from pSource
	int convertRawToAnnexB_4byte_config
			( unsigned char *pTarget
			, int iTargetLen
			, unsigned char *pSource
			, int iSourceLen )
	{
		unsigned int i, j;
		unsigned int numOfArrays;
		unsigned short usSize;
		unsigned char *p = (unsigned char*)&usSize;

		int iNewLen = iTargetLen;

		unsigned int bRawFrameType = 0;

		// check HEVCDecoderConfigurationRecord reserved bit to detect start position.
		if ((pSource[13] & 0xF0) == 0xF0 &&
			(pSource[15] & 0xFC) == 0xFC &&
			(pSource[16] & 0xFC) == 0xFC &&
			(pSource[17] & 0xF8) == 0xF8 &&
			(pSource[18] & 0xF8) == 0xF8
			)
		{
			numOfArrays = (unsigned int)pSource[22];
			pSource += 23;
			iSourceLen -= 23;
		}
		else
		{
			numOfArrays = (unsigned int)*pSource;
			if(numOfArrays != 0)
			{
				pSource++; iSourceLen--;
			}
			else
			{
				numOfArrays = 1;		// error?
			}
		}

		for (i = 0; i < numOfArrays && 4 < iTargetLen; i++ )
		{
			unsigned int nal_unit_type = (unsigned int)(pSource[0] & 0x3F);
			unsigned int numNalus = (unsigned int)((pSource[1] << 8) | (pSource[2]));
			pSource += 3;

			for(j = 0; j < numNalus; j++)
			{
				unsigned int nalUnitLength = (unsigned int)((pSource[0] << 8) | (pSource[1]));
				pSource += 2;	

				memcpy( pTarget, NAL_START_CODE, sizeof(NAL_START_CODE) );
				pTarget += sizeof(NAL_START_CODE);
				iTargetLen -= sizeof(NAL_START_CODE);

				memcpy( pTarget, pSource, nalUnitLength );
				pTarget += nalUnitLength;
				iTargetLen -= nalUnitLength;
				pSource += nalUnitLength;
				iSourceLen -= nalUnitLength;
			}
		}

		return iNewLen - iTargetLen;
	}

	// unless iNALHeaderLengthSize == 4, pTarget must be different from pSource
	int convertRawToAnnexB_4byte
			( unsigned char *pTarget
			, int iTargetLen
			, unsigned char *pSource
			, int iSourceLen
			, int iNALHeaderLengthSize )
	{
		int iNewLen = iTargetLen;
		bool failed = false;

		unsigned char pSizeAsBigEndian[4] = {0, 0, 0, 0};
		unsigned int uSize = 0;

		if ((iSourceLen < iNALHeaderLengthSize) || !pTarget || (4 != iNALHeaderLengthSize && pSource == pTarget))
		{
			MC_ERR("(%d,%d,%p,%p)", iSourceLen, iNALHeaderLengthSize, pTarget, pSource);
			return -1;
		}

		bigEndianReader readBigEndianValue;

		switch (iNALHeaderLengthSize)
		{
		case 4:
			readBigEndianValue = Utils::readBigEndianValue32;
			break;
		case 3:
			readBigEndianValue = Utils::readBigEndianValue24;
			break;
		case 2:
			readBigEndianValue = Utils::readBigEndianValue16;
			break;
		case 1:
			readBigEndianValue = Utils::readBigEndianValue8;
			break;
		default:
			MC_ERR("iNALHeaderLengthSize(%d) is incorrect!", iNALHeaderLengthSize);
			return -1;
		}

		while ( iSourceLen > iNALHeaderLengthSize )
		{
			memcpy( pSizeAsBigEndian, pSource, iNALHeaderLengthSize );
			pSource += iNALHeaderLengthSize;
			iSourceLen -= iNALHeaderLengthSize;

			uSize = readBigEndianValue(pSizeAsBigEndian);

			if ( 0 < (int)uSize && (iSourceLen >= (int)uSize) )
			{
				memcpy( pTarget, NAL_START_CODE, sizeof(NAL_START_CODE));
				pTarget += sizeof(NAL_START_CODE);
				iTargetLen -= sizeof(NAL_START_CODE);

				if (pTarget != pSource || sizeof(NAL_START_CODE) != iNALHeaderLengthSize)
				{
					memcpy( pTarget, pSource, uSize );
				}
				pTarget += uSize;
				iTargetLen -= uSize;
				pSource += uSize;
				iSourceLen -= uSize;
			}
			else if ( (uSize == 0) && (iNewLen != iTargetLen) )
			{
				MC_ERR("(%d,%d,%d)", iSourceLen, uSize, iNewLen-iTargetLen);
				break;
			}
			else
			{
				failed = true;
				MC_ERR("(%d,%d)", iSourceLen, uSize);
				break;
			}
		}

		return failed ? -1 : iNewLen - iTargetLen;
	}
}// namespace (anon321)

namespace { // anon320

/*****************************************************************************/
/* SPS restrictions                                                          */
/*****************************************************************************/

/* Number of SPS allowed in Main Profile*/
/* An extra buffer is allocated to write the parsed data
 * It is copied to the appropriate location later */
#define MAX_SPS_CNT         (16 + 1)

/* Max sub layers in PPS */
#define SPS_MAX_SUB_LAYERS  7

/* Maximum long term reference pics */
#define MAX_LTREF_PICS_SPS 16

#define MAX_STREF_PICS_SPS 64

/*****************************************************************************/
/* Profile tier level defs                                                   */
/*****************************************************************************/
#define MAX_PROFILE_COMPATBLTY 32

/*****************************************************************************/
/* VPS restrictions                                                          */
/*****************************************************************************/
/* Number of VPS allowed  in Main Profile */
#define MAX_VPS_CNT         16

/* Max sub layers in VPS */
#define VPS_MAX_SUB_LAYERS  7

/* Max number of HRD parameters */
#define VPS_MAX_HRD_PARAMS  2

/* Maximum number of operation point layers */
#define VPS_MAX_OP_LAYERS 2

#define SCALING_LIST_SIZE_NUM	4
#define SCALING_LIST_NUM		6

#define MAX_DPB_SIZE	16

typedef struct
{
	char forbidden_zero_bit;
    char nal_unit_type;
	char nuh_layer_id;
    char nuh_temporal_id_plus1;
}__attribute__ ((packed)) nal_header_t;

typedef struct
{
    char profile_space;
    char tier_flag;
    char profile_idc;
    char profile_compatibility_flag[MAX_PROFILE_COMPATBLTY];
    char progressive_source_flag;
    char interlaced_source_flag;
	char non_packed_constraint_flag;
	char frame_only_constraint_flag;

	char max_12bit_constraint_flag;
	char max_10bit_constraint_flag;
	char max_8bit_constraint_flag;
	char max_422chroma_constraint_flag;
	char max_420chroma_constraint_flag;
	char max_monochrome_constraint_flag;
	char intra_constraint_flag;
	char one_picture_only_constraint_flag;
	char lower_bit_rate_constraint_flag;

	char max_14bit_constraint_flag;

	char inbld_flag;

	unsigned int level_idc;

}__attribute__ ((packed)) profile_tier_lvl_t;

/**
 * Structure to hold Profile tier level info for all layers
 */
typedef struct
{
    profile_tier_lvl_t s_ptl_gen;

    char sub_layer_profile_present_flag[VPS_MAX_SUB_LAYERS - 1];
    char sub_layer_level_present_flag[VPS_MAX_SUB_LAYERS - 1];

    profile_tier_lvl_t s_ptl_sub[VPS_MAX_SUB_LAYERS - 1];

}__attribute__ ((packed))profile_tier_lvl_info_t;


/**
 * Sub-layer HRD parameters Info
 */
typedef struct
{
    unsigned int bit_rate_value_minus1[32];
    unsigned int cpb_size_value_minus1[32];
    unsigned int cpb_size_du_value_minus1[32];
    unsigned int bit_rate_du_value_minus1[32];
    unsigned char  cbr_flag[32];

}__attribute__ ((packed))hrd_params_sub_t;

/**
 * HRD parameters Info
 */
typedef struct
{
    unsigned char timing_info_present_flag;
    unsigned int num_units_in_tick;
    unsigned int time_scale;
    unsigned char nal_hrd_parameters_present_flag;
    unsigned char vcl_hrd_parameters_present_flag;
    unsigned char cpbdpb_delays_present_flag;
    unsigned char sub_pic_cpb_params_present_flag;
    unsigned char tick_divisor_minus2;
    unsigned char du_cpb_removal_delay_increment_length_minus1;
    unsigned char sub_pic_cpb_params_in_pic_timing_sei_flag;
    unsigned char dpb_output_delay_du_length_minus1;
    unsigned int bit_rate_scale;
    unsigned int cpb_size_scale;
    unsigned int cpb_size_du_scale;
    unsigned char  initial_cpb_removal_delay_length_minus1;
    unsigned char  au_cpb_removal_delay_length_minus1;
    unsigned char  dpb_output_delay_length_minus1;

	unsigned char fixed_pic_rate_general_flag[6];
    unsigned char fixed_pic_rate_within_cvs_flag[6];

    unsigned char elemental_duration_in_tc_minus1[6];
    unsigned char low_delay_hrd_flag[6];
    unsigned char cpb_cnt_minus1[6];
    hrd_params_sub_t hrd_sub[6];

}__attribute__ ((packed))hrd_params_t;


/**
 * Structure to hold VUI parameters Info
 */
typedef struct
{
    unsigned char aspect_ratio_info_present_flag;
    unsigned char aspect_ratio_idc;
    unsigned short sar_width;
    unsigned short sar_height;

    unsigned char overscan_info_present_flag;
    unsigned char overscan_appropriate_flag;
    
	unsigned char video_signal_type_present_flag;
    unsigned char video_format;
    unsigned char video_full_range_flag;
    unsigned char colour_description_present_flag;
    unsigned char colour_primaries;
    unsigned char transfer_characteristics;
    unsigned char matrix_coeffs;
    
	unsigned char chroma_loc_info_present_flag;
	unsigned char chroma_sample_loc_type_top_field;
    unsigned char chroma_sample_loc_type_bottom_field;

	unsigned char neutral_chroma_indication_flag;
    unsigned char field_seq_flag;
    unsigned char frame_field_info_present_flag;

	unsigned char default_display_window_flag;
	unsigned int def_disp_win_left_offset;
    unsigned int def_disp_win_right_offset;
    unsigned int def_disp_win_top_offset;
    unsigned int def_disp_win_bottom_offset;

	unsigned char vui_timing_info_present_flag;
    unsigned int vui_num_units_in_tick;
    unsigned int vui_time_scale;

	unsigned char vui_poc_proportional_to_timing_flag;
    unsigned char vui_num_ticks_poc_diff_one_minus1;

    unsigned char vui_hrd_parameters_present_flag;
    hrd_params_t vui_hrd_parameters;

    unsigned char bitstream_restriction_flag;
    unsigned char tiles_fixed_structure_flag;
    unsigned char motion_vectors_over_pic_boundaries_flag;
    unsigned char restricted_ref_pic_lists_flag;

	unsigned int min_spatial_segmentation_idc;

	unsigned char max_bytes_per_pic_denom;
    unsigned char max_bits_per_mincu_denom;
    unsigned char log2_max_mv_length_horizontal;
    unsigned char log2_max_mv_length_vertical;


}__attribute__ ((packed))vui_t;


typedef struct
{
    char inter_ref_pic_set_prediction_flag;
	char delta_idx_minus1;
	char delta_rps_sign;
	char abs_delta_rps_minus1;

	char used_by_curr_pic_flag[MAX_DPB_SIZE];
	char use_delta_flag[MAX_DPB_SIZE];

	char num_negative_pics;
    char num_positive_pics;

	short delta_poc[MAX_DPB_SIZE];
	short delta_poc_minus1[MAX_DPB_SIZE];
	char num_delta_pocs;
    char num_ref_idc;
    
}__attribute__ ((packed))ref_picset_t;

typedef struct
{
	nal_header_t nal_header;
    
	char sps_video_parameter_set_id;
	char sps_max_sub_layers_minus1;
	char sps_temporal_id_nesting_flag;

	profile_tier_lvl_info_t s_ptl;

	char sps_seq_parameter_set_id;
	char chroma_format_idc;
	char separate_colour_plane_flag;

	short pic_width_in_luma_samples;
	short pic_height_in_luma_samples;

	char conformance_window_flag;
	short conf_win_left_offset;
	short conf_win_right_offset;
	short conf_win_top_offset;
	short conf_win_bottom_offset;

	char bit_depth_luma_minus8;
	char bit_depth_chroma_minus8;
	char log2_max_pic_order_cnt_lsb_minus4;
	char sps_sub_layer_ordering_info_present_flag;

	char sps_max_dec_pic_buffering_minus1[SPS_MAX_SUB_LAYERS];
	char sps_max_num_reorder_pics[SPS_MAX_SUB_LAYERS];
	char sps_max_latency_increase_plus1[SPS_MAX_SUB_LAYERS];

	char log2_min_luma_coding_block_size_minus3;
	char log2_diff_max_min_luma_coding_block_size;
	char log2_min_luma_transform_block_size_minus2;
	char log2_diff_max_min_luma_transform_block_size;
	char max_transform_hierarchy_depth_inter;
	char max_transform_hierarchy_depth_intra;
	
	char scaling_list_enabled_flag;
	char sps_scaling_list_data_present_flag;

	int scaling_list_pred_mode_flag[SCALING_LIST_SIZE_NUM][SCALING_LIST_NUM];
	int scaling_list_pred_matrix_id_delta[SCALING_LIST_SIZE_NUM][SCALING_LIST_NUM];
	int scaling_list_dc_coef_minus8[SCALING_LIST_SIZE_NUM][SCALING_LIST_NUM];
	int scaling_list_delta_coef[SCALING_LIST_SIZE_NUM][SCALING_LIST_NUM][64];

	char amp_enabled_flag;
	char sample_adaptive_offset_enabled_flag;

	char pcm_enabled_flag;
	char pcm_sample_bit_depth_luma_minus1;
	char pcm_sample_bit_depth_chroma_minus1;
	char log2_min_pcm_luma_coding_block_size_minus3;
	char log2_diff_max_min_pcm_luma_coding_block_size;
	char pcm_loop_filter_disabled_flag;

	char num_short_term_ref_pic_sets;
	ref_picset_t short_term_ref_picset[MAX_STREF_PICS_SPS];

	char long_term_ref_pics_present_flag;
	char num_long_term_ref_pics_sps;
	char lt_ref_pic_poc_lsb_sps[MAX_LTREF_PICS_SPS];
	char used_by_curr_pic_lt_sps_flag[MAX_LTREF_PICS_SPS];

	char sps_temporal_mvp_enabled_flag;
	char strong_intra_smoothing_enabled_flag;
	
	char vui_parameters_present_flag;
	vui_t vui_param;

	char sps_extension_present_flag;
	char sps_range_extension_flag;
	char sps_multilayer_extension_flag;
	char sps_3d_extension_flag;
	char sps_scc_extension_flag;
	char sps_extension_4bits;

}__attribute__ ((packed))spsInfo_t;

typedef struct
{
	unsigned int	BS;
	unsigned int	BitCount;
	unsigned char		*endbuf;
	unsigned char		*buffer;
	unsigned int	rear;
	unsigned int	length;
} __attribute__ ((packed))BITSTREAM;

int _LoadBSHEVC(BITSTREAM *bs)
{
	unsigned int data, bsd, rear;

	if ( bs->rear >= bs->length )
		return 1;

	rear = bs->rear;
	data = bs->buffer[rear];
	bsd = bs->BS;

	if (data == 3 && !( bsd << 16 ) )
	{
		if ( (!bsd) && (rear > 1) )
		{
			if (!(bs->buffer[rear-1] | bs->buffer[rear-2]))
				data = bs->buffer[++rear];
		} else
			if( ( bsd << 8 ) )
				data = bs->buffer[++rear];
	}
	bs->BS = (bs->BS << 8) | data;
	bs->rear = rear + 1;

	return (bs->rear >= bs->length);
}

int _ShiftBitsHEVC(BITSTREAM *bs, int n)
{
	int i;
	int tempBitCount;
	int BytesToLoad = (bs->BitCount + n) >> 3;

	if(bs->rear < bs->length)
	{
		bs->BitCount = (bs->BitCount + n) & 7;

		for (i = 0 ; i < BytesToLoad; i++)
		{
			if (_LoadBSHEVC(bs))
			{
				return 1;
			}
		}
	}
	else
	{
		if((tempBitCount = (bs->BitCount + n)) > 32)
			return 0;

		bs->BitCount = tempBitCount;
	}

	return 0;
}

int _ShiftBytesHEVC(BITSTREAM *bs, int n)
{
	int	i;

	for (i = 0 ; i < n; i++)
	{
		if (_LoadBSHEVC(bs))
		{
			return 1;
		}
	}

	return 0;
}


int _AlignBitsHEVC(BITSTREAM *bs)
{
	if (bs->BitCount)
		return _ShiftBitsHEVC(bs, 8 - (bs->BitCount & 7));
	return 0;
}


unsigned int _ReadNShiftBitsHEVC(BITSTREAM *bs, int n)
{
	unsigned int BSL;
	int i;
	int BytesToLoad;
	int tempBitCount=0;

	if(n == 32)
	{
		unsigned int j,tempBSL = 0;

		for(j =0 ; j<2 ; j++)
		{
			if(bs->rear < bs->length)
			{
				BytesToLoad = (bs->BitCount + 16) >> 3;
				BSL = bs->BS << bs->BitCount;
				bs->BitCount = (bs->BitCount + 16) & 7;

				for (i = 0 ; i < BytesToLoad; i++)
				{
					if (_LoadBSHEVC(bs))
						break;
				}
			}
			else
			{
				BSL = bs->BS << (((4 > bs->rear) ? (4 - bs->rear) << 3 : 0) + bs->BitCount);

				if((tempBitCount = (bs->BitCount + 16)) >= ((4 > bs->rear) ? bs->rear << 3 : 32))
					return 0;

				bs->BitCount += tempBitCount;
			}
			tempBSL += (BSL >> 16) | (tempBSL <<16);
		}
		return tempBSL;
	}
	else
	{
		if(bs->rear < bs->length)
		{
			BytesToLoad = (bs->BitCount + n) >> 3;
			BSL = bs->BS << bs->BitCount;
			bs->BitCount = (bs->BitCount + n) & 7;

			for (i = 0 ; i < BytesToLoad; i++)
			{
				if (_LoadBSHEVC(bs))
				{
					break;
				}
			}
		}
		else
		{
			BSL = bs->BS << (((4 > bs->rear) ? (4 - bs->rear) << 3 : 0) + bs->BitCount);

			if((tempBitCount = (bs->BitCount + n)) >= ((4 > bs->rear) ? bs->rear << 3 : 32))
				return 0;

			bs->BitCount = tempBitCount;
		}
		return BSL >> (32 - n);
	}
}

int _ReadBitsMSB(BITSTREAM *bs)
{
	return ( bs->BS << bs->BitCount );
}

int _CountLeadingZeros32( unsigned int n )
{
	int ret=0, i;

	for ( i = 0 ; (!(n & 0x80000000) && i < 32) ; i++ )
	{
		n <<= 1;
		ret++;
	}

	return ret;
}

unsigned int _GetUE_V_HEVC(BITSTREAM *bs)
{
	int count;
	unsigned int n;

	n = _ReadBitsMSB(bs) << ((4 > bs->rear) ? ((4 - bs->rear) << 3) : 0);

	count = _CountLeadingZeros32( n );
	if ( count == 32 )
		return 0xFFFFFFFF;

	n = _ReadNShiftBitsHEVC(bs, (count<<1)+1);

	return n - 1;
}

int _GetSE_V_HEVC(BITSTREAM *bs)
{
	unsigned int n;
	int a;

	n = _GetUE_V_HEVC(bs);
	a = (n + 1) >> 1;
	return (n & 1) ? a : -a;
}


///////////////////////////////////////////////////////////////////////////
typedef struct {
	unsigned int		Reg;
	unsigned int		BitCount;
	unsigned int		BitsCountFrame;
	unsigned int		numZeroBytes;

	unsigned int		idx;
	unsigned int		buffer_length;
	unsigned char		*buffer;
}WBITSTREAM;

void InitWBitstream(WBITSTREAM *bs, unsigned char *buffer, unsigned int length)
{
	bs->Reg = 0;
	bs->BitCount = 0;
	bs->BitsCountFrame = 0;
	bs->numZeroBytes = 0;

	bs->idx = 0;
	bs->buffer_length = length;
	bs->buffer = buffer;
}

void WriteBitstream32(WBITSTREAM *bs)
{
	unsigned char *sptr = (unsigned char *)(&(bs->Reg));
	unsigned char *dptr;
	int i;

	dptr = &bs->buffer[bs->idx];

	for(i = 3; i >= 0; i--)
	{
		if( (bs->numZeroBytes == 2) && (sptr[i] == 0) )
		{
			*dptr++ = 3;
			bs->idx++;
			bs->BitsCountFrame += 8;
			*dptr++ = sptr[i];
			bs->idx++;
			bs->numZeroBytes = 1;
		}
		else
		{
			*dptr++ = sptr[i];
			bs->idx++;
			if(sptr[i] == 0)
				bs->numZeroBytes++;
			else
				bs->numZeroBytes = 0;
		}
	}
	bs->Reg = 0;
	bs->BitCount = 0;
}

void Write_NBits(WBITSTREAM *bs, unsigned long value, int length)
{
	int	BitLocation;

	BitLocation = 32 - bs->BitCount - length;

	if (BitLocation > 0) {
		bs->Reg |= (value << BitLocation);
		bs->BitCount += length;
	} else if (BitLocation < 0) { // Carry
		BitLocation = -BitLocation;
		bs->Reg |= (value >> BitLocation);
		WriteBitstream32(bs);
		bs->Reg = (value & ((1 << BitLocation) - 1)) << (32 - BitLocation);
		bs->BitCount = BitLocation;
	} else {
		bs->Reg |= value;
		WriteBitstream32(bs);
		//bs->Reg = 0;
		//bs->BitCount = 0;
	}
	bs->BitsCountFrame += length;
}

void Write_AlignBits(WBITSTREAM *bs)
{
	int	BitsToStuff, count;

	count = bs->BitCount & 0x07;
	if (count) {
		BitsToStuff = 8 - count;
		Write_NBits(bs, 0x00, BitsToStuff);
	}
}


int Write_UE(WBITSTREAM *bs, unsigned int val)
{
	unsigned int temp;
	int len;
	temp = val + 1;

	for (len = 0; temp; ) {
		temp = temp >> 1;
		len++;
	};
	len = (len * 2 - 1);
	Write_NBits(bs, val + 1, len);
	return len;
}

int Write_TE(WBITSTREAM *bs, unsigned int val, int max_val)
{
	if (max_val > 1)
		return Write_UE(bs, val);
	else
		Write_NBits(bs, !val, 1);
	return 1;
}

int Write_SE(WBITSTREAM *bs, int val)
{
	unsigned int code, temp;
	int len, sign;

	if (val == 0) {
		Write_NBits(bs, 1, 1);
		return 1;
	} else if (val < 0) {
		code = (unsigned int) (-val);
		sign = 1;
	} else {
		code = (unsigned int) val;
		sign = 0;
	}
	temp = code * 2;
	for (len = 0; temp; ) {
		temp = temp >> 1;
		len++;
	}
	len = len * 2 - 1;
	Write_NBits(bs, code * 2 + sign, len);
	return len;
}

void Write_FlushBits(WBITSTREAM *bs)
{
	int	BytesToFlush = (bs->BitCount + 7) >> 3;
	unsigned char *ptr;
	int	i;

	if(bs->BitCount==0)	return;

	//if(bs->VLCBufferIndex > (unsigned int)(VLC_PACKET_SIZE-BytesToFlush))
	//	WriteToEncBufferH264(bs);

	ptr = (unsigned char *)(&bs->Reg);
	for (i = 0; i < BytesToFlush; i++){
#if 1//def LITTLE_ENDIAN
		bs->buffer[bs->idx++] = ptr[3-i];
#else
		bs->buffer[bs->idx++] = ptr[i];
#endif
	}
	bs->Reg = 0;
	bs->BitCount = 0;
}

///////////////////////////////////////////////////////////////////////////

void hevc_parse_profile_tier_level_layer(BITSTREAM *bs, profile_tier_lvl_t *s_ptl)
{
	unsigned int i;

	s_ptl->profile_space = _ReadNShiftBitsHEVC(bs, 2);
	s_ptl->tier_flag     = _ReadNShiftBitsHEVC(bs, 1);
	s_ptl->profile_idc   = _ReadNShiftBitsHEVC(bs, 5);

	for (i = 0; i < 32; i++)
		s_ptl->profile_compatibility_flag[i] = _ReadNShiftBitsHEVC(bs, 1);

	s_ptl->progressive_source_flag    = _ReadNShiftBitsHEVC(bs, 1);
	s_ptl->interlaced_source_flag     = _ReadNShiftBitsHEVC(bs, 1);
	s_ptl->non_packed_constraint_flag = _ReadNShiftBitsHEVC(bs, 1);
	s_ptl->frame_only_constraint_flag = _ReadNShiftBitsHEVC(bs, 1);

	//_ShiftBytesHEVC(bs, 5);_ShiftBitsHEVC(bs, 4);	// general_reserved_zero_44bits	u(44)
	// update spec. ITU-T-REC-H.265-201612 7.3.3
	if( s_ptl->profile_idc == 4  || s_ptl->profile_compatibility_flag[4] ||
		s_ptl->profile_idc == 5  || s_ptl->profile_compatibility_flag[5] ||
		s_ptl->profile_idc == 6  || s_ptl->profile_compatibility_flag[6] ||
		s_ptl->profile_idc == 7  || s_ptl->profile_compatibility_flag[7] ||
		s_ptl->profile_idc == 8  || s_ptl->profile_compatibility_flag[8] ||
		s_ptl->profile_idc == 9  || s_ptl->profile_compatibility_flag[9] ||
		s_ptl->profile_idc == 10 || s_ptl->profile_compatibility_flag[10]
	)
	{
		s_ptl->max_12bit_constraint_flag        = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->max_10bit_constraint_flag        = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->max_8bit_constraint_flag         = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->max_422chroma_constraint_flag    = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->max_420chroma_constraint_flag    = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->max_monochrome_constraint_flag   = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->intra_constraint_flag            = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->one_picture_only_constraint_flag = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->lower_bit_rate_constraint_flag   = _ReadNShiftBitsHEVC(bs, 1);

		if( s_ptl->profile_idc == 5  || s_ptl->profile_compatibility_flag[5] ||
			s_ptl->profile_idc == 9  || s_ptl->profile_compatibility_flag[9] ||
			s_ptl->profile_idc == 10 || s_ptl->profile_compatibility_flag[10]
		)
		{
			s_ptl->max_14bit_constraint_flag = _ReadNShiftBitsHEVC(bs, 1);
			_ShiftBytesHEVC(bs, 4);_ShiftBitsHEVC(bs, 1);	// general_reserved_zero_33bits
		}
		else
			_ShiftBytesHEVC(bs, 4);_ShiftBitsHEVC(bs, 2);	// general_reserved_zero_34bits
	}
	else
		_ShiftBytesHEVC(bs, 5);_ShiftBitsHEVC(bs, 3);	// general_reserved_zero_43bits

	if((s_ptl->profile_idc >= 1 && s_ptl->profile_idc <= 5 ) ||
		s_ptl->profile_idc == 9 ||
		s_ptl->profile_compatibility_flag[1] || s_ptl->profile_compatibility_flag[2] ||
		s_ptl->profile_compatibility_flag[3] || s_ptl->profile_compatibility_flag[4] ||
		s_ptl->profile_compatibility_flag[5] || s_ptl->profile_compatibility_flag[9]
	)
		s_ptl->inbld_flag = _ReadNShiftBitsHEVC(bs, 1);
	else
		_ShiftBitsHEVC(bs, 1);	// general_reserved_zero_1bits

}

void hevc_profile_tier_level(BITSTREAM *bs, unsigned int maxNumSubLayersMinus1, profile_tier_lvl_info_t *s_ptl)
{
	unsigned int i;

	hevc_parse_profile_tier_level_layer(bs, &s_ptl->s_ptl_gen);

	s_ptl->s_ptl_gen.level_idc =_ReadNShiftBitsHEVC(bs, 8);

	for (i = 0; i < maxNumSubLayersMinus1; i++)
	{
		s_ptl->sub_layer_profile_present_flag[i] = _ReadNShiftBitsHEVC(bs, 1);
		s_ptl->sub_layer_level_present_flag[i]   = _ReadNShiftBitsHEVC(bs, 1);
	}

	if (maxNumSubLayersMinus1 > 0)
		for (i = maxNumSubLayersMinus1; i < 8; i++)
			_ReadNShiftBitsHEVC(bs, 2);	// reserved_zero_2bits

	for (i = 0; i < maxNumSubLayersMinus1; i++)
	{
		if (s_ptl->sub_layer_profile_present_flag[i])
			hevc_parse_profile_tier_level_layer(bs, &s_ptl->s_ptl_sub[i]);

		if (s_ptl->sub_layer_level_present_flag[i])
			s_ptl->s_ptl_sub[i].level_idc = _ReadNShiftBitsHEVC(bs, 8);
	}
}


void write_hevc_parse_profile_tier_level_layer(WBITSTREAM *bs, profile_tier_lvl_t *s_ptl)
{
	unsigned int i;

	Write_NBits(bs, s_ptl->profile_space, 2);
	Write_NBits(bs, s_ptl->tier_flag, 1);
	Write_NBits(bs, s_ptl->profile_idc, 5);

	for (i = 0; i < 32; i++)
		Write_NBits(bs, s_ptl->profile_compatibility_flag[i], 1);

	Write_NBits(bs, s_ptl->progressive_source_flag, 1);
	Write_NBits(bs, s_ptl->interlaced_source_flag, 1);
	Write_NBits(bs, s_ptl->non_packed_constraint_flag, 1);
	Write_NBits(bs, s_ptl->frame_only_constraint_flag, 1);

	//_ShiftBytesHEVC(bs, 5);_ShiftBitsHEVC(bs, 4);	// general_reserved_zero_44bits	u(44)
	// update spec. ITU-T-REC-H.265-201612 7.3.3
	if( s_ptl->profile_idc == 4  || s_ptl->profile_compatibility_flag[4] ||
		s_ptl->profile_idc == 5  || s_ptl->profile_compatibility_flag[5] ||
		s_ptl->profile_idc == 6  || s_ptl->profile_compatibility_flag[6] ||
		s_ptl->profile_idc == 7  || s_ptl->profile_compatibility_flag[7] ||
		s_ptl->profile_idc == 8  || s_ptl->profile_compatibility_flag[8] ||
		s_ptl->profile_idc == 9  || s_ptl->profile_compatibility_flag[9] ||
		s_ptl->profile_idc == 10 || s_ptl->profile_compatibility_flag[10]
	)
	{
		Write_NBits(bs, s_ptl->max_12bit_constraint_flag, 1);
		Write_NBits(bs, s_ptl->max_10bit_constraint_flag, 1);
		Write_NBits(bs, s_ptl->max_8bit_constraint_flag, 1);
		Write_NBits(bs, s_ptl->max_422chroma_constraint_flag, 1);
		Write_NBits(bs, s_ptl->max_420chroma_constraint_flag, 1);
		Write_NBits(bs, s_ptl->max_monochrome_constraint_flag, 1);
		Write_NBits(bs, s_ptl->intra_constraint_flag, 1);
		Write_NBits(bs, s_ptl->one_picture_only_constraint_flag, 1);
		Write_NBits(bs, s_ptl->lower_bit_rate_constraint_flag, 1);

		if( s_ptl->profile_idc == 5  || s_ptl->profile_compatibility_flag[5] ||
			s_ptl->profile_idc == 9  || s_ptl->profile_compatibility_flag[9] ||
			s_ptl->profile_idc == 10 || s_ptl->profile_compatibility_flag[10]
		)
		{
			int general_reserved_zero_33bits = 0;

			Write_NBits(bs, s_ptl->max_14bit_constraint_flag, 1);

			Write_NBits(bs, general_reserved_zero_33bits, 33);
		}
		else
		{
			int general_reserved_zero_34bits = 0;
			Write_NBits(bs, general_reserved_zero_34bits, 34);
		}
	}
	else
	{
		int general_reserved_zero_43bits = 0;
		Write_NBits(bs, general_reserved_zero_43bits, 43);
	}

	if((s_ptl->profile_idc >= 1 && s_ptl->profile_idc <= 5 ) ||
		s_ptl->profile_idc == 9 ||
		s_ptl->profile_compatibility_flag[1] || s_ptl->profile_compatibility_flag[2] ||
		s_ptl->profile_compatibility_flag[3] || s_ptl->profile_compatibility_flag[4] ||
		s_ptl->profile_compatibility_flag[5] || s_ptl->profile_compatibility_flag[9]
	)
		//s_ptl->inbld_flag = _ReadNShiftBitsHEVC(bs, 1);
		Write_NBits(bs, s_ptl->inbld_flag, 1);
	else
	{
		int general_reserved_zero_1bits = 0;
		Write_NBits(bs, general_reserved_zero_1bits, 1);
	}

}

void write_hevc_profile_tier_level(WBITSTREAM *bs, unsigned int maxNumSubLayersMinus1, profile_tier_lvl_info_t *s_ptl)
{
	unsigned int i;

	write_hevc_parse_profile_tier_level_layer(bs, &s_ptl->s_ptl_gen);

	Write_NBits(bs, s_ptl->s_ptl_gen.level_idc, 8);

	for (i = 0; i < maxNumSubLayersMinus1; i++)
	{
		Write_NBits(bs, s_ptl->sub_layer_profile_present_flag[i], 1);
		Write_NBits(bs, s_ptl->sub_layer_level_present_flag[i], 1);
	}

	if (maxNumSubLayersMinus1 > 0)
	{
		int reserved_zero_2bits = 0;
		for (i = maxNumSubLayersMinus1; i < 8; i++)
			Write_NBits(bs, reserved_zero_2bits, 2);
	}

	for (i = 0; i < maxNumSubLayersMinus1; i++)
	{
		if (s_ptl->sub_layer_profile_present_flag[i])
			write_hevc_parse_profile_tier_level_layer(bs, &s_ptl->s_ptl_sub[i]);

		if (s_ptl->sub_layer_level_present_flag[i])
			Write_NBits(bs, s_ptl->s_ptl_sub[i].level_idc, 8);
	}
}



#define CLIP3(x, min, max) (((x) > max) ? max :(((x) < min)? min:(x)))
//#define MAX(x,y)    (((x) > (y)) ? (x) :(y))
//#define MIN(x,y)    (((x) < (y)) ? (x) :(y))

int  hevc_scaling_list_data(BITSTREAM *bs, spsInfo_t *sps)
{
    int ret = 0;
    int size_id, matrix_id;
    int coef_num;
    int i;

	for(size_id = 0; size_id < SCALING_LIST_SIZE_NUM; size_id++)
	{
		for(matrix_id = 0; matrix_id < SCALING_LIST_NUM; matrix_id += (size_id == 3) ? 3 : 1)
		{
			sps->scaling_list_pred_mode_flag[size_id][matrix_id] = _ReadNShiftBitsHEVC(bs, 1);

			if(!sps->scaling_list_pred_mode_flag[size_id][matrix_id])
				sps->scaling_list_pred_matrix_id_delta[size_id][matrix_id] = _GetUE_V_HEVC(bs);
			else
			{
				coef_num = MIN(64, (1 << (4 + (size_id << 1))));

				if(size_id > 1)
					sps->scaling_list_dc_coef_minus8[size_id][matrix_id] = _GetSE_V_HEVC(bs);

				for(i = 0; i < coef_num; i++)
					sps->scaling_list_delta_coef[size_id][matrix_id][i] = _GetSE_V_HEVC(bs);
			}
		}
    }

    return ret;
}


int  write_hevc_scaling_list_data(WBITSTREAM *bs, spsInfo_t *sps)
{
    int ret = 0;
    int size_id, matrix_id;
    int coef_num;
    int i;

	for(size_id = 0; size_id < SCALING_LIST_SIZE_NUM; size_id++)
	{
		for(matrix_id = 0; matrix_id < SCALING_LIST_NUM; matrix_id += (size_id == 3) ? 3 : 1)
		{
			Write_NBits(bs, sps->scaling_list_pred_mode_flag[size_id][matrix_id], 1);

			if(!sps->scaling_list_pred_mode_flag[size_id][matrix_id])
				Write_UE(bs, sps->scaling_list_pred_matrix_id_delta[size_id][matrix_id]);
			else
			{
				coef_num = MIN(64, (1 << (4 + (size_id << 1))));

				if(size_id > 1)
					Write_SE(bs, sps->scaling_list_dc_coef_minus8[size_id][matrix_id]);

				for(i = 0; i < coef_num; i++)
					Write_SE(bs, sps->scaling_list_delta_coef[size_id][matrix_id][i]);
			}
		}
    }

    return ret;
}


int hevc_parse_sub_layer_hrd_parameters(BITSTREAM *bs, hrd_params_sub_t *hrd_sub, int cpb_cnt, int sub_pic_cpb_params_present_flag)
{
    int ret = 0;
    int i;

    for(i = 0; i <= cpb_cnt; i++)
    {
		hrd_sub->bit_rate_value_minus1[i] = _GetUE_V_HEVC(bs);
		hrd_sub->cpb_size_value_minus1[i] = _GetUE_V_HEVC(bs);

        if(sub_pic_cpb_params_present_flag)
        {
			hrd_sub->cpb_size_du_value_minus1[i] = _GetUE_V_HEVC(bs);
			hrd_sub->bit_rate_du_value_minus1[i] = _GetUE_V_HEVC(bs);
        }
		hrd_sub->cbr_flag[i] = _ReadNShiftBitsHEVC(bs, 1);
    }

    return ret;
}


int write_hevc_sub_layer_hrd_parameters(WBITSTREAM *bs, hrd_params_sub_t *hrd_sub, int cpb_cnt, int sub_pic_cpb_params_present_flag)
{
    int ret = 0;
    int i;

    for(i = 0; i <= cpb_cnt; i++)
    {
		Write_UE(bs, hrd_sub->bit_rate_value_minus1[i]);
		Write_UE(bs, hrd_sub->cpb_size_value_minus1[i]);

        if(sub_pic_cpb_params_present_flag)
        {
			Write_UE(bs, hrd_sub->cpb_size_du_value_minus1[i]);
			Write_UE(bs, hrd_sub->bit_rate_du_value_minus1[i]);
        }
		Write_NBits(bs, hrd_sub->cbr_flag[i], 1);
    }

    return ret;
}


int hevc_parse_hrd_parameters(BITSTREAM *bs, hrd_params_t *hrd, int common_info_present_flag, int max_num_sub_layers_minus1)
{
    int ret = 0;
    int i;

    hrd->nal_hrd_parameters_present_flag = 0;
    hrd->vcl_hrd_parameters_present_flag = 0;

    hrd->sub_pic_cpb_params_present_flag = 0;

    hrd->tick_divisor_minus2 = 0;
    hrd->du_cpb_removal_delay_increment_length_minus1 = 0;
    hrd->sub_pic_cpb_params_in_pic_timing_sei_flag = 0;
    hrd->dpb_output_delay_du_length_minus1 = 0;

    hrd->bit_rate_scale = 0;
    hrd->cpb_size_scale = 0;
    hrd->cpb_size_du_scale = 0;

    hrd->initial_cpb_removal_delay_length_minus1 = 23;
    hrd->au_cpb_removal_delay_length_minus1 = 23;
    hrd->dpb_output_delay_length_minus1 = 23;

    if(common_info_present_flag)
    {
		hrd->nal_hrd_parameters_present_flag = _ReadNShiftBitsHEVC(bs, 1);
		hrd->vcl_hrd_parameters_present_flag = _ReadNShiftBitsHEVC(bs, 1);

        if(hrd->nal_hrd_parameters_present_flag || hrd->vcl_hrd_parameters_present_flag)
        {
			hrd->sub_pic_cpb_params_in_pic_timing_sei_flag = _ReadNShiftBitsHEVC(bs, 1);
            if(hrd->sub_pic_cpb_params_present_flag)
            {
				hrd->tick_divisor_minus2                          = _ReadNShiftBitsHEVC(bs, 8);
				hrd->du_cpb_removal_delay_increment_length_minus1 = _ReadNShiftBitsHEVC(bs, 5);
				hrd->sub_pic_cpb_params_in_pic_timing_sei_flag    = _ReadNShiftBitsHEVC(bs, 1);
				hrd->dpb_output_delay_du_length_minus1            = _ReadNShiftBitsHEVC(bs, 5);
            }

			hrd->bit_rate_scale = _ReadNShiftBitsHEVC(bs, 4);
			hrd->cpb_size_scale = _ReadNShiftBitsHEVC(bs, 4);
            if(hrd->sub_pic_cpb_params_present_flag)
				hrd->cpb_size_du_scale = _ReadNShiftBitsHEVC(bs, 4);

			hrd->initial_cpb_removal_delay_length_minus1 = _ReadNShiftBitsHEVC(bs, 5);
			hrd->au_cpb_removal_delay_length_minus1      = _ReadNShiftBitsHEVC(bs, 5);
			hrd->dpb_output_delay_length_minus1          = _ReadNShiftBitsHEVC(bs, 5);
        }
    }


    for(i = 0; i <= max_num_sub_layers_minus1; i++)
    {
		hrd->fixed_pic_rate_general_flag[i] = _ReadNShiftBitsHEVC(bs, 1);

        hrd->fixed_pic_rate_within_cvs_flag[i] = 1;
        hrd->elemental_duration_in_tc_minus1[i] = 0;
        hrd->low_delay_hrd_flag[i] = 0;
        hrd->cpb_cnt_minus1[i] = 0;

        if(!hrd->fixed_pic_rate_general_flag[i])
			hrd->fixed_pic_rate_within_cvs_flag[i] = _ReadNShiftBitsHEVC(bs, 1);

        if(hrd->fixed_pic_rate_within_cvs_flag[i])
			hrd->elemental_duration_in_tc_minus1[i] = _GetUE_V_HEVC(bs);
        else
			hrd->low_delay_hrd_flag[i] = _ReadNShiftBitsHEVC(bs, 1);

        if(!hrd->low_delay_hrd_flag[i])
			hrd->cpb_cnt_minus1[i] = _GetUE_V_HEVC(bs);

        if(hrd->nal_hrd_parameters_present_flag)
            hevc_parse_sub_layer_hrd_parameters(bs, &hrd->hrd_sub[i], hrd->cpb_cnt_minus1[i], hrd->sub_pic_cpb_params_present_flag);

        if(hrd->vcl_hrd_parameters_present_flag)
            hevc_parse_sub_layer_hrd_parameters(bs, &hrd->hrd_sub[i], hrd->cpb_cnt_minus1[i], hrd->sub_pic_cpb_params_present_flag);
    }

    return ret;
}


int write_hevc_hrd_parameters(WBITSTREAM *bs, hrd_params_t *hrd, int common_info_present_flag, int max_num_sub_layers_minus1)
{
    int ret = 0;
    int i;

    hrd->nal_hrd_parameters_present_flag = 0;
    hrd->vcl_hrd_parameters_present_flag = 0;

    hrd->sub_pic_cpb_params_present_flag = 0;

    hrd->tick_divisor_minus2 = 0;
    hrd->du_cpb_removal_delay_increment_length_minus1 = 0;
    hrd->sub_pic_cpb_params_in_pic_timing_sei_flag = 0;
    hrd->dpb_output_delay_du_length_minus1 = 0;

    hrd->bit_rate_scale = 0;
    hrd->cpb_size_scale = 0;
    hrd->cpb_size_du_scale = 0;

    hrd->initial_cpb_removal_delay_length_minus1 = 23;
    hrd->au_cpb_removal_delay_length_minus1 = 23;
    hrd->dpb_output_delay_length_minus1 = 23;

    if(common_info_present_flag)
    {
		Write_NBits(bs, hrd->nal_hrd_parameters_present_flag, 1);
		Write_NBits(bs, hrd->vcl_hrd_parameters_present_flag, 1);

        if(hrd->nal_hrd_parameters_present_flag || hrd->vcl_hrd_parameters_present_flag)
        {
			Write_NBits(bs, hrd->sub_pic_cpb_params_in_pic_timing_sei_flag, 1);
            if(hrd->sub_pic_cpb_params_present_flag)
            {
				Write_NBits(bs, hrd->tick_divisor_minus2, 8);
				Write_NBits(bs, hrd->du_cpb_removal_delay_increment_length_minus1, 5);
				Write_NBits(bs, hrd->sub_pic_cpb_params_in_pic_timing_sei_flag, 1);
				Write_NBits(bs, hrd->dpb_output_delay_du_length_minus1, 5);
            }

			Write_NBits(bs, hrd->bit_rate_scale, 4);
			Write_NBits(bs, hrd->cpb_size_scale, 4);
            if(hrd->sub_pic_cpb_params_present_flag)
				Write_NBits(bs, hrd->cpb_size_du_scale, 4);

			Write_NBits(bs, hrd->initial_cpb_removal_delay_length_minus1, 5);
			Write_NBits(bs, hrd->au_cpb_removal_delay_length_minus1, 5);
			Write_NBits(bs, hrd->dpb_output_delay_length_minus1, 5);
        }
    }


    for(i = 0; i <= max_num_sub_layers_minus1; i++)
    {
		Write_NBits(bs, hrd->fixed_pic_rate_general_flag[i], 1);

        hrd->fixed_pic_rate_within_cvs_flag[i] = 1;
        hrd->elemental_duration_in_tc_minus1[i] = 0;
        hrd->low_delay_hrd_flag[i] = 0;
        hrd->cpb_cnt_minus1[i] = 0;

        if(!hrd->fixed_pic_rate_general_flag[i])
			Write_NBits(bs, hrd->fixed_pic_rate_within_cvs_flag[i], 1);

        if(hrd->fixed_pic_rate_within_cvs_flag[i])
			Write_UE(bs, hrd->elemental_duration_in_tc_minus1[i]);
        else
			Write_NBits(bs, hrd->low_delay_hrd_flag[i], 1);

        if(!hrd->low_delay_hrd_flag[i])
			Write_UE(bs, hrd->cpb_cnt_minus1[i]);

        if(hrd->nal_hrd_parameters_present_flag)
            write_hevc_sub_layer_hrd_parameters(bs, &hrd->hrd_sub[i], hrd->cpb_cnt_minus1[i], hrd->sub_pic_cpb_params_present_flag);

        if(hrd->vcl_hrd_parameters_present_flag)
            write_hevc_sub_layer_hrd_parameters(bs, &hrd->hrd_sub[i], hrd->cpb_cnt_minus1[i], hrd->sub_pic_cpb_params_present_flag);
    }

    return ret;
}


int hevc_parse_vui(BITSTREAM *bs, vui_t *vui, int sps_max_sub_layers_minus1)
{
    int ret = 0;

	vui->aspect_ratio_info_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	vui->aspect_ratio_idc = 0;
	vui->sar_width        = 0;
	vui->sar_height       = 0;
	if(vui->aspect_ratio_info_present_flag)
	{
		vui->aspect_ratio_idc = _ReadNShiftBitsHEVC(bs, 8);
		if(vui->aspect_ratio_idc  ==  255)
		{
			vui->sar_width  = _ReadNShiftBitsHEVC(bs, 16);
			vui->sar_height = _ReadNShiftBitsHEVC(bs, 16);
		}
	}

	vui->overscan_info_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(vui->overscan_info_present_flag)
	{
		vui->overscan_appropriate_flag = _ReadNShiftBitsHEVC(bs, 1);
	}

	vui->video_signal_type_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(vui->video_signal_type_present_flag)
	{
		vui->video_format          = _ReadNShiftBitsHEVC(bs, 3);
		vui->video_full_range_flag = _ReadNShiftBitsHEVC(bs, 1);
		
		vui->colour_description_present_flag = _ReadNShiftBitsHEVC(bs, 1);
		if(vui->colour_description_present_flag)
		{
			vui->colour_primaries         = _ReadNShiftBitsHEVC(bs, 8);
			vui->transfer_characteristics = _ReadNShiftBitsHEVC(bs, 8);
			vui->matrix_coeffs            = _ReadNShiftBitsHEVC(bs, 8);
		}
	}

	vui->chroma_loc_info_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(vui->chroma_loc_info_present_flag)
	{
		vui->chroma_sample_loc_type_top_field    = _GetUE_V_HEVC(bs);
		vui->chroma_sample_loc_type_bottom_field = _GetUE_V_HEVC(bs);
	}

	vui->neutral_chroma_indication_flag = _ReadNShiftBitsHEVC(bs, 1);
	vui->field_seq_flag                 = _ReadNShiftBitsHEVC(bs, 1);
	vui->frame_field_info_present_flag  = _ReadNShiftBitsHEVC(bs, 1);
	
	vui->default_display_window_flag    = _ReadNShiftBitsHEVC(bs, 1);
	if(vui->default_display_window_flag)
	{
		vui->def_disp_win_left_offset   = _GetUE_V_HEVC(bs);
		vui->def_disp_win_right_offset  = _GetUE_V_HEVC(bs);
		vui->def_disp_win_top_offset    = _GetUE_V_HEVC(bs);
		vui->def_disp_win_bottom_offset = _GetUE_V_HEVC(bs);
	}
	else
	{
		vui->def_disp_win_left_offset = 0;
		vui->def_disp_win_right_offset = 0;
		vui->def_disp_win_top_offset = 0;
		vui->def_disp_win_bottom_offset = 0;
	}
    
	vui->vui_timing_info_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(vui->vui_timing_info_present_flag)
	{
		vui->vui_num_units_in_tick = _ReadNShiftBitsHEVC(bs, 32);
		vui->vui_time_scale = _ReadNShiftBitsHEVC(bs, 32);
		
		vui->vui_poc_proportional_to_timing_flag = _ReadNShiftBitsHEVC(bs, 1);
		if(vui->vui_poc_proportional_to_timing_flag)
			vui->vui_num_ticks_poc_diff_one_minus1 = _GetUE_V_HEVC(bs);

		vui->vui_hrd_parameters_present_flag = _ReadNShiftBitsHEVC(bs, 1);
		if(vui->vui_hrd_parameters_present_flag)
		{
			hevc_parse_hrd_parameters(bs, &vui->vui_hrd_parameters, 1, sps_max_sub_layers_minus1);
		}
	}

	vui->bitstream_restriction_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(vui->bitstream_restriction_flag)
	{
		vui->tiles_fixed_structure_flag              = _ReadNShiftBitsHEVC(bs, 1);
		vui->motion_vectors_over_pic_boundaries_flag = _ReadNShiftBitsHEVC(bs, 1);
		vui->restricted_ref_pic_lists_flag           = _ReadNShiftBitsHEVC(bs, 1);

		vui->min_spatial_segmentation_idc  = _GetUE_V_HEVC(bs);
		vui->max_bytes_per_pic_denom       = _GetUE_V_HEVC(bs);
		vui->max_bits_per_mincu_denom      = _GetUE_V_HEVC(bs);
		vui->log2_max_mv_length_horizontal = _GetUE_V_HEVC(bs);
		vui->log2_max_mv_length_vertical   = _GetUE_V_HEVC(bs);
	}

	return ret;
}


int write_hevc_vui(WBITSTREAM *bs, vui_t *vui, int sps_max_sub_layers_minus1)
{
    int ret = 0;

	Write_NBits(bs, vui->aspect_ratio_info_present_flag, 1);
	vui->aspect_ratio_idc = 0;
	vui->sar_width        = 0;
	vui->sar_height       = 0;
	if(vui->aspect_ratio_info_present_flag)
	{
		Write_NBits(bs, vui->aspect_ratio_idc, 8);
		if(vui->aspect_ratio_idc  ==  255)
		{
			Write_NBits(bs, vui->sar_width, 16);
			Write_NBits(bs, vui->sar_height, 16);
		}
	}

	Write_NBits(bs, vui->overscan_info_present_flag, 1);
	if(vui->overscan_info_present_flag)
	{
		Write_NBits(bs, vui->overscan_appropriate_flag, 1);
	}

	Write_NBits(bs, vui->video_signal_type_present_flag, 1);
	if(vui->video_signal_type_present_flag)
	{
		Write_NBits(bs, vui->video_format, 3);
		Write_NBits(bs, vui->video_full_range_flag, 1);

		Write_NBits(bs, vui->colour_description_present_flag, 1);
		if(vui->colour_description_present_flag)
		{
			Write_NBits(bs, vui->colour_primaries, 8);
			Write_NBits(bs, vui->transfer_characteristics, 8);
			Write_NBits(bs, vui->matrix_coeffs, 8);
		}
	}

	Write_NBits(bs, vui->chroma_loc_info_present_flag, 1);
	if(vui->chroma_loc_info_present_flag)
	{
		Write_UE(bs, vui->chroma_sample_loc_type_top_field);
		Write_UE(bs, vui->chroma_sample_loc_type_bottom_field);
	}

	Write_NBits(bs, vui->neutral_chroma_indication_flag, 1);
	Write_NBits(bs, vui->field_seq_flag, 1);
	Write_NBits(bs, vui->frame_field_info_present_flag, 1);

	Write_NBits(bs, vui->default_display_window_flag, 1);
	if(vui->default_display_window_flag)
	{
		Write_UE(bs, vui->def_disp_win_left_offset);
		Write_UE(bs, vui->def_disp_win_right_offset);
		Write_UE(bs, vui->def_disp_win_top_offset);
		Write_UE(bs, vui->def_disp_win_bottom_offset);
	}

	Write_NBits(bs, vui->vui_timing_info_present_flag, 1);
	if(vui->vui_timing_info_present_flag)
	{
		Write_NBits(bs, vui->vui_num_units_in_tick, 32);
		Write_NBits(bs, vui->vui_time_scale, 32);

		Write_NBits(bs, vui->vui_poc_proportional_to_timing_flag, 1);
		if(vui->vui_poc_proportional_to_timing_flag)
			Write_UE(bs, vui->vui_num_ticks_poc_diff_one_minus1);

		Write_NBits(bs, vui->vui_hrd_parameters_present_flag, 1);
		if(vui->vui_hrd_parameters_present_flag)
		{
			write_hevc_hrd_parameters(bs, &vui->vui_hrd_parameters, 1, sps_max_sub_layers_minus1);
		}
	}

	Write_NBits(bs, vui->bitstream_restriction_flag, 1);
	if(vui->bitstream_restriction_flag)
	{
		Write_NBits(bs, vui->tiles_fixed_structure_flag, 1);
		Write_NBits(bs, vui->motion_vectors_over_pic_boundaries_flag, 1);
		Write_NBits(bs, vui->restricted_ref_pic_lists_flag, 1);

		Write_UE(bs, vui->min_spatial_segmentation_idc);
		Write_UE(bs, vui->max_bytes_per_pic_denom);
		Write_UE(bs, vui->max_bits_per_mincu_denom);
		Write_UE(bs, vui->log2_max_mv_length_horizontal);
		Write_UE(bs, vui->log2_max_mv_length_vertical);
	}

	return ret;
}


int hevc_short_term_ref_pic_set(BITSTREAM *bs, ref_picset_t * ref_pic_base, int num_short_term_ref_pic_sets, int idx)
{
	ref_picset_t * ref_pic = ref_pic_base + idx;
	ref_picset_t * ref_pic_ref;
	int delta_rps;
	int r_idx;
	unsigned int i;

	if(idx != 0)
		ref_pic->inter_ref_pic_set_prediction_flag = _ReadNShiftBitsHEVC(bs, 1);
	else
		ref_pic->inter_ref_pic_set_prediction_flag = 0;

	if(ref_pic->inter_ref_pic_set_prediction_flag)
	{
		int num_neg_pics = 0;
        int num_pos_pics = 0;
        int num_pics = 0;

		if(idx == num_short_term_ref_pic_sets)
			ref_pic->delta_idx_minus1 = _GetUE_V_HEVC(bs);
		else
			ref_pic->delta_idx_minus1 = 1;

		r_idx = (idx - ref_pic->delta_idx_minus1 + 1);
		r_idx = CLIP3(r_idx, 0, idx - 1);
		ref_pic_ref = ref_pic_base + r_idx;

		ref_pic->delta_rps_sign       = _ReadNShiftBitsHEVC(bs, 1);
		ref_pic->abs_delta_rps_minus1 = _GetUE_V_HEVC(bs);

		delta_rps = (1 - 2 * ref_pic->delta_rps_sign) * (ref_pic->abs_delta_rps_minus1 + 1);

		for(i = 0; i <= ref_pic_ref->num_delta_pocs; i++)
		{
			ref_pic->used_by_curr_pic_flag[i] = _ReadNShiftBitsHEVC(bs, 1);
			if(ref_pic->used_by_curr_pic_flag[i] == 0)
				ref_pic->use_delta_flag[i] = _ReadNShiftBitsHEVC(bs, 1);

			if( (ref_pic->used_by_curr_pic_flag[i] == 1) ||
				(ref_pic->used_by_curr_pic_flag[i] == 2)
			)
			{
				int delta_poc = delta_rps;
                delta_poc += ((i < ref_pic_ref->num_delta_pocs) ? ref_pic_ref->delta_poc[i] : 0);
				
				ref_pic_ref->delta_poc[num_pics] = delta_poc;

				if(delta_poc < 0)
                    num_neg_pics++;
                else
                    num_pos_pics++;
                num_pics++;
			}
		}
		num_neg_pics = CLIP3(num_neg_pics, 0, MAX_DPB_SIZE - 1);
        num_pos_pics = CLIP3(num_pos_pics, 0, (MAX_DPB_SIZE - 1 - num_neg_pics));
        num_pics = num_neg_pics + num_pos_pics;

		ref_pic->num_ref_idc       = ref_pic_ref->num_delta_pocs + 1;
		ref_pic->num_positive_pics = num_pos_pics;
		ref_pic->num_negative_pics = num_neg_pics;
	}
	else
	{
		int prev_poc = 0;

		ref_pic->num_negative_pics = _GetUE_V_HEVC(bs);
		ref_pic->num_positive_pics = _GetUE_V_HEVC(bs);

		ref_pic->num_negative_pics = CLIP3(ref_pic->num_negative_pics, 0, MAX_DPB_SIZE - 1);
		ref_pic->num_positive_pics = CLIP3(ref_pic->num_positive_pics, 0, MAX_DPB_SIZE - 1);

		ref_pic->num_delta_pocs = ref_pic->num_negative_pics + ref_pic->num_positive_pics;

		for(i = 0; i < ref_pic->num_negative_pics; i++)
		{
			ref_pic->delta_poc_minus1[i]      = _GetUE_V_HEVC(bs);
			ref_pic->used_by_curr_pic_flag[i] = _ReadNShiftBitsHEVC(bs, 1);

			ref_pic->delta_poc[i] = prev_poc - (ref_pic->delta_poc_minus1[i] + 1);
			prev_poc = ref_pic->delta_poc[i];
		}

		for(i = ref_pic->num_negative_pics; i < ref_pic->num_delta_pocs; i++)
		{
			ref_pic->delta_poc_minus1[i]      = _GetUE_V_HEVC(bs);
			ref_pic->used_by_curr_pic_flag[i] = _ReadNShiftBitsHEVC(bs, 1);

			ref_pic->delta_poc[i] = prev_poc - (ref_pic->delta_poc_minus1[i] + 1);
			prev_poc = ref_pic->delta_poc[i];
		}
	}

	return 0;
}

int write_hevc_short_term_ref_pic_set(WBITSTREAM *bs, ref_picset_t * ref_pic_base, int num_short_term_ref_pic_sets, int idx)
{
	ref_picset_t * ref_pic = ref_pic_base + idx;
	ref_picset_t * ref_pic_ref;
	int delta_rps;
	int r_idx;
	unsigned int i;

	if(idx != 0)
		Write_NBits(bs, ref_pic->inter_ref_pic_set_prediction_flag, 1);
	else
		ref_pic->inter_ref_pic_set_prediction_flag = 0;

	if(ref_pic->inter_ref_pic_set_prediction_flag)
	{
		int num_neg_pics = 0;
        int num_pos_pics = 0;
        int num_pics = 0;

		if(idx == num_short_term_ref_pic_sets)
			Write_UE(bs, ref_pic->delta_idx_minus1);
		else
			ref_pic->delta_idx_minus1 = 1;

		r_idx = (idx - ref_pic->delta_idx_minus1 + 1);
		r_idx = CLIP3(r_idx, 0, idx - 1);
		ref_pic_ref = ref_pic_base + r_idx;

		Write_NBits(bs, ref_pic->delta_rps_sign, 1);
		Write_UE(bs, ref_pic->abs_delta_rps_minus1);

		delta_rps = (1 - 2 * ref_pic->delta_rps_sign) * (ref_pic->abs_delta_rps_minus1 + 1);

		for(i = 0; i <= ref_pic_ref->num_delta_pocs; i++)
		{
			Write_NBits(bs, ref_pic->used_by_curr_pic_flag[i], 1);
			if(ref_pic->used_by_curr_pic_flag[i] == 0)
				Write_NBits(bs, ref_pic->use_delta_flag[i], 1);

			if( (ref_pic->used_by_curr_pic_flag[i] == 1) ||
				(ref_pic->used_by_curr_pic_flag[i] == 2)
			)
			{
				int delta_poc = delta_rps;
                delta_poc += ((i < ref_pic_ref->num_delta_pocs) ? ref_pic_ref->delta_poc[i] : 0);
				
				ref_pic_ref->delta_poc[num_pics] = delta_poc;

				if(delta_poc < 0)
                    num_neg_pics++;
                else
                    num_pos_pics++;
                num_pics++;
			}
		}
		num_neg_pics = CLIP3(num_neg_pics, 0, MAX_DPB_SIZE - 1);
        num_pos_pics = CLIP3(num_pos_pics, 0, (MAX_DPB_SIZE - 1 - num_neg_pics));
        num_pics = num_neg_pics + num_pos_pics;

		ref_pic->num_ref_idc       = ref_pic_ref->num_delta_pocs + 1;
		ref_pic->num_positive_pics = num_pos_pics;
		ref_pic->num_negative_pics = num_neg_pics;
	}
	else
	{
		int prev_poc = 0;

		Write_UE(bs, ref_pic->num_negative_pics);
		Write_UE(bs, ref_pic->num_positive_pics);

		for(i = 0; i < ref_pic->num_negative_pics; i++)
		{
			Write_UE(bs, ref_pic->delta_poc_minus1[i]);
			Write_NBits(bs, ref_pic->used_by_curr_pic_flag[i], 1);
		}

		for(i = ref_pic->num_negative_pics; i < ref_pic->num_delta_pocs; i++)
		{
			Write_UE(bs, ref_pic->delta_poc_minus1[i]);
			Write_NBits(bs, ref_pic->used_by_curr_pic_flag[i], 1);
		}
	}

	return 0;
}

int hevc_parse_sps(BITSTREAM *bs, spsInfo_t *sps)
{
	int ret = 0;
	unsigned int i;

	sps->nal_header.forbidden_zero_bit    = _ReadNShiftBitsHEVC(bs, 1);
	sps->nal_header.nal_unit_type         = _ReadNShiftBitsHEVC(bs, 6);
	sps->nal_header.nuh_layer_id          = _ReadNShiftBitsHEVC(bs, 6);
	sps->nal_header.nuh_temporal_id_plus1 = _ReadNShiftBitsHEVC(bs, 3);
    
	sps->sps_video_parameter_set_id   = _ReadNShiftBitsHEVC(bs, 4);
	sps->sps_max_sub_layers_minus1    = _ReadNShiftBitsHEVC(bs, 3);
	sps->sps_temporal_id_nesting_flag = _ReadNShiftBitsHEVC(bs, 1);

	hevc_profile_tier_level(bs, sps->sps_max_sub_layers_minus1, &sps->s_ptl);

	sps->sps_seq_parameter_set_id = _GetUE_V_HEVC(bs);
	sps->chroma_format_idc        = _GetUE_V_HEVC(bs);
	if(sps->chroma_format_idc == 3)
		sps->separate_colour_plane_flag = _ReadNShiftBitsHEVC(bs, 1);
	else
		sps->separate_colour_plane_flag = 0;

	sps->pic_width_in_luma_samples  = _GetUE_V_HEVC(bs);
	sps->pic_height_in_luma_samples = _GetUE_V_HEVC(bs);
	
	// pic cropping flag
	sps->conformance_window_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(sps->conformance_window_flag)
	{
		sps->conf_win_left_offset   = _GetUE_V_HEVC(bs);
		sps->conf_win_right_offset  = _GetUE_V_HEVC(bs);
		sps->conf_win_top_offset    = _GetUE_V_HEVC(bs);
		sps->conf_win_bottom_offset = _GetUE_V_HEVC(bs);
	}
	else
	{
		sps->conf_win_left_offset   = 0;
		sps->conf_win_right_offset  = 0;
		sps->conf_win_top_offset    = 0;
		sps->conf_win_bottom_offset = 0;
	}

	sps->bit_depth_luma_minus8   = _GetUE_V_HEVC(bs);
	sps->bit_depth_chroma_minus8 = _GetUE_V_HEVC(bs);

	sps->log2_max_pic_order_cnt_lsb_minus4 = _GetUE_V_HEVC(bs);

	sps->sps_sub_layer_ordering_info_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	i = (sps->sps_sub_layer_ordering_info_present_flag ? 0 : (sps->sps_max_sub_layers_minus1));
    for(; i <= sps->sps_max_sub_layers_minus1; i++)
	{
		sps->sps_max_dec_pic_buffering_minus1[i] = _GetUE_V_HEVC(bs);
		sps->sps_max_num_reorder_pics[i]         = _GetUE_V_HEVC(bs);
		sps->sps_max_latency_increase_plus1[i]   = _GetUE_V_HEVC(bs);
	}

	sps->log2_min_luma_coding_block_size_minus3      = _GetUE_V_HEVC(bs);
	sps->log2_diff_max_min_luma_coding_block_size    = _GetUE_V_HEVC(bs);
	sps->log2_min_luma_transform_block_size_minus2   = _GetUE_V_HEVC(bs);
	sps->log2_diff_max_min_luma_transform_block_size = _GetUE_V_HEVC(bs);
	sps->max_transform_hierarchy_depth_inter         = _GetUE_V_HEVC(bs);
	sps->max_transform_hierarchy_depth_intra         = _GetUE_V_HEVC(bs);

	sps->scaling_list_enabled_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(sps->scaling_list_enabled_flag)
	{
		sps->sps_scaling_list_data_present_flag = _ReadNShiftBitsHEVC(bs, 1);
		hevc_scaling_list_data(bs, sps);
	}

	sps->amp_enabled_flag                    = _ReadNShiftBitsHEVC(bs, 1);
	sps->sample_adaptive_offset_enabled_flag = _ReadNShiftBitsHEVC(bs, 1);

	sps->pcm_enabled_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(sps->pcm_enabled_flag)
	{
		sps->pcm_sample_bit_depth_luma_minus1             = _ReadNShiftBitsHEVC(bs, 4);
		sps->pcm_sample_bit_depth_chroma_minus1           = _ReadNShiftBitsHEVC(bs, 4);
		sps->log2_min_pcm_luma_coding_block_size_minus3   = _GetUE_V_HEVC(bs);
		sps->log2_diff_max_min_pcm_luma_coding_block_size = _GetUE_V_HEVC(bs);
		sps->pcm_loop_filter_disabled_flag                = _ReadNShiftBitsHEVC(bs, 1);
	}

	sps->num_short_term_ref_pic_sets     = _GetUE_V_HEVC(bs);
	for(i = 0; i < sps->num_short_term_ref_pic_sets; i++)
	{
		hevc_short_term_ref_pic_set(bs, &sps->short_term_ref_picset[0], sps->num_short_term_ref_pic_sets, i);
	}
	
	sps->long_term_ref_pics_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(sps->long_term_ref_pics_present_flag)
	{
		sps->num_long_term_ref_pics_sps = _GetUE_V_HEVC(bs);
		for(i = 0; i < sps->num_long_term_ref_pics_sps; i++)
		{
			sps->lt_ref_pic_poc_lsb_sps[i]       = _ReadNShiftBitsHEVC(bs, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
			sps->used_by_curr_pic_lt_sps_flag[i] = _ReadNShiftBitsHEVC(bs, 1);
		}
	}

	sps->sps_temporal_mvp_enabled_flag       = _ReadNShiftBitsHEVC(bs, 1);
	sps->strong_intra_smoothing_enabled_flag = _ReadNShiftBitsHEVC(bs, 1);
	
	sps->vui_parameters_present_flag = _ReadNShiftBitsHEVC(bs, 1);
	if(sps->vui_parameters_present_flag)
	{
		hevc_parse_vui(bs, &sps->vui_param, sps->sps_max_sub_layers_minus1);
	}

	sps->sps_extension_present_flag = _ReadNShiftBitsHEVC(bs, 1);

	// sps_extension & rbsp_trailing_bits( )

	return 0;
}


int write_hevc_sps(WBITSTREAM *bs, spsInfo_t *sps)
{
	int ret = 0;
	unsigned int i;

	Write_NBits(bs, sps->nal_header.forbidden_zero_bit, 1);
	Write_NBits(bs, sps->nal_header.nal_unit_type, 6);
	Write_NBits(bs, sps->nal_header.nuh_layer_id, 6);
	Write_NBits(bs, sps->nal_header.nuh_temporal_id_plus1, 3);
    
	Write_NBits(bs, sps->sps_video_parameter_set_id, 4);
	Write_NBits(bs, sps->sps_max_sub_layers_minus1, 3);
	Write_NBits(bs, sps->sps_temporal_id_nesting_flag, 1);

	write_hevc_profile_tier_level(bs, sps->sps_max_sub_layers_minus1, &sps->s_ptl);

	Write_UE(bs, sps->sps_seq_parameter_set_id);
	Write_UE(bs, sps->chroma_format_idc);
	if(sps->chroma_format_idc == 3)
		Write_NBits(bs, sps->separate_colour_plane_flag, 1);

	Write_UE(bs, sps->pic_width_in_luma_samples);
	Write_UE(bs, sps->pic_height_in_luma_samples);

	// pic cropping flag
	Write_NBits(bs, sps->conformance_window_flag, 1);
	if(sps->conformance_window_flag)
	{
		Write_UE(bs, sps->conf_win_left_offset);
		Write_UE(bs, sps->conf_win_right_offset);
		Write_UE(bs, sps->conf_win_top_offset);
		Write_UE(bs, sps->conf_win_bottom_offset);
	}

	Write_UE(bs, sps->bit_depth_luma_minus8);
	Write_UE(bs, sps->bit_depth_chroma_minus8);

	Write_UE(bs, sps->log2_max_pic_order_cnt_lsb_minus4);

	Write_NBits(bs, sps->sps_sub_layer_ordering_info_present_flag, 1);
	i = (sps->sps_sub_layer_ordering_info_present_flag ? 0 : (sps->sps_max_sub_layers_minus1));
    for(; i <= sps->sps_max_sub_layers_minus1; i++)
	{
		Write_UE(bs, sps->sps_max_dec_pic_buffering_minus1[i]);
		Write_UE(bs, sps->sps_max_num_reorder_pics[i]);
		Write_UE(bs, sps->sps_max_latency_increase_plus1[i]);
	}

	Write_UE(bs, sps->log2_min_luma_coding_block_size_minus3);
	Write_UE(bs, sps->log2_diff_max_min_luma_coding_block_size);
	Write_UE(bs, sps->log2_min_luma_transform_block_size_minus2);
	Write_UE(bs, sps->log2_diff_max_min_luma_transform_block_size);
	Write_UE(bs, sps->max_transform_hierarchy_depth_inter);
	Write_UE(bs, sps->max_transform_hierarchy_depth_intra);

	Write_NBits(bs, sps->scaling_list_enabled_flag, 1);
	if(sps->scaling_list_enabled_flag)
	{
		Write_NBits(bs, sps->sps_scaling_list_data_present_flag, 1);
		if(sps->sps_scaling_list_data_present_flag)
			write_hevc_scaling_list_data(bs, sps);
	}

	Write_NBits(bs, sps->amp_enabled_flag, 1);
	Write_NBits(bs, sps->sample_adaptive_offset_enabled_flag, 1);

	Write_NBits(bs, sps->pcm_enabled_flag, 1);
	if(sps->pcm_enabled_flag)
	{
		Write_NBits(bs, sps->pcm_sample_bit_depth_luma_minus1, 4);
		Write_NBits(bs, sps->pcm_sample_bit_depth_chroma_minus1, 4);
		Write_UE(bs, sps->log2_min_pcm_luma_coding_block_size_minus3);
		Write_UE(bs, sps->log2_diff_max_min_pcm_luma_coding_block_size);
		Write_NBits(bs, sps->pcm_loop_filter_disabled_flag, 1);
	}

	Write_UE(bs, sps->num_short_term_ref_pic_sets);
	for(i = 0; i < sps->num_short_term_ref_pic_sets; i++)
	{
		write_hevc_short_term_ref_pic_set(bs, &sps->short_term_ref_picset[0], sps->num_short_term_ref_pic_sets, i);
	}

	Write_NBits(bs, sps->long_term_ref_pics_present_flag, 1);
	if(sps->long_term_ref_pics_present_flag)
	{
		Write_UE(bs, sps->num_long_term_ref_pics_sps);
		for(i = 0; i < sps->num_long_term_ref_pics_sps; i++)
		{
			Write_NBits(bs, sps->lt_ref_pic_poc_lsb_sps[i], sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
			Write_NBits(bs, sps->used_by_curr_pic_lt_sps_flag[i], 1);
		}
	}

	Write_NBits(bs, sps->sps_temporal_mvp_enabled_flag, 1);
	Write_NBits(bs, sps->strong_intra_smoothing_enabled_flag, 1);

	Write_NBits(bs, sps->vui_parameters_present_flag, 1);
	if(sps->vui_parameters_present_flag)
	{
		write_hevc_vui(bs, &sps->vui_param, sps->sps_max_sub_layers_minus1);
	}

	// sps_extension parsing & writting is not implemented.
	if(sps->sps_extension_present_flag)
		sps->sps_extension_present_flag = 0;

	Write_NBits(bs, sps->sps_extension_present_flag, 1);

	Write_NBits(bs, 1, 1);
	Write_AlignBits(bs);

	Write_FlushBits(bs);

	return 0;
}

//==============================================================================
// returns: index where start code begins
//          maxLength if not found
//          < 0 on invalid input
int findAnnexBStartCode(const unsigned char *pInput, unsigned int maxLength, int &nalHeaderLength)
{
	if (3 > maxLength)
	{
		nalHeaderLength = 0;
		return -1;
	}

	unsigned int currentWord = ((unsigned int)pInput[0] << 16) | ((unsigned int)pInput[1] << 8) | pInput[2];

	if (1 == currentWord)
	{
		nalHeaderLength = 3;
		return 0;
	}

	for (unsigned int i = 3; i < maxLength; ++i)
	{
		currentWord = currentWord << 8 | pInput[i];
		if (1 == (0xffffff & currentWord))
		{
			if (1 == currentWord)
			{
				nalHeaderLength = 4;
				return i - 3;
			}
			nalHeaderLength = 3;
			return i - 2;
		}
	}

	nalHeaderLength = 0;
	return maxLength;
}
//==============================================================================
// returns: length of nal without startcode filling nalHeaderLength with length of start code.
//          < 0 if no start code found at the start
int getNalSize(const unsigned char *pInput, unsigned int maxLength, int &nalHeaderLength)
{
	int annexBStartCodeIndex = findAnnexBStartCode(pInput, 4 > maxLength ? maxLength : 4, nalHeaderLength);
	if (0 != annexBStartCodeIndex)
	{
		// error occurred
		nalHeaderLength = 0;
		return -1;
	}

	pInput += nalHeaderLength;
	maxLength -= nalHeaderLength;

	int tmpNalHeaderLength = 0;

	annexBStartCodeIndex = findAnnexBStartCode(pInput, maxLength, tmpNalHeaderLength);

	if (0 > annexBStartCodeIndex)
	{
		return maxLength;
	}
	else
	{
		return annexBStartCodeIndex;
	}
}
//==============================================================================

} // namespace (anon320)

} // namespace Common

namespace Decoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_DECODER

#ifdef CODEC_NAME
#undef CODEC_NAME
#endif
#define CODEC_NAME "HEVCD"

using Nex_MC::Video::Decoder::NexCAL_mcvd;

namespace { // (anon201)
	using Nex_MC::Utils::WrapSetProperty;
    
	pthread_mutex_t g_preferredDecoderMutex;
	char *g_preferredDecoderName = NULL;
	bool g_checkedForPreferredDecoder = false;
	int *g_profileLevels = NULL;
	int  g_supportedFeature = 0;

	char *g_preferredSWDecoderName = NULL;
	bool g_checkedForPreferredSWDecoder = false;
	int *g_swProfileLevels = NULL;
	int  g_swSupportedFeature = 0;

	WrapSetProperty g_wrapSetProperty;

	class CleanUpCrewDecoder {
	public:
		CleanUpCrewDecoder()
		{
			pthread_mutex_init(&g_preferredDecoderMutex, NULL);
		}

		~CleanUpCrewDecoder()
		{
			pthread_mutex_lock(&g_preferredDecoderMutex);
			if (NULL != g_preferredDecoderName)
			{
				free(g_preferredDecoderName);
				g_preferredDecoderName = NULL;
			}
			if (NULL != g_profileLevels)
			{
				free(g_profileLevels);
				g_profileLevels = NULL;
			}
			g_checkedForPreferredDecoder = false;

			if (NULL != g_preferredSWDecoderName)
			{
				free(g_preferredSWDecoderName);
				g_preferredSWDecoderName = NULL;
			}
			if (NULL != g_swProfileLevels)
			{
				free(g_swProfileLevels);
				g_swProfileLevels = NULL;
			}
			g_checkedForPreferredSWDecoder = false;
			pthread_mutex_unlock(&g_preferredDecoderMutex);

			pthread_mutex_destroy(&g_preferredDecoderMutex);
		}
	};

	CleanUpCrewDecoder cleanUp;
} // namespace (anon201)

int TranslatedSPSInfoLevel(Common::spsInfo_t *sps)
{
	Common::profile_tier_lvl_t *info = &sps->s_ptl.s_ptl_gen;

	return info->level_idc / 3;
}

bool LevelSupportsContentAtFPS(int level, Common::spsInfo_t *sps, float fps)
{
	int level_idx = hevc_get_level_index(level);
	unsigned int maxFrameSizeInSamples = hevc_max_support_luma_pic_size[level_idx];
	unsigned int maxSamplesPerSecond = hevc_max_support_sample_rate_size[level_idx];
	unsigned int contentFrameSizeInSamples = (sps->pic_width_in_luma_samples) * (sps->pic_height_in_luma_samples);

	// support profile check
	//if(sps->s_ptl.s_ptl_gen.profile_idc != HEVC_PROFILE_MAIN)
	//	return false;

	if (contentFrameSizeInSamples > maxFrameSizeInSamples)
		return false;

	if (contentFrameSizeInSamples > (maxSamplesPerSecond / fps))
		return false;

	return true;
}

namespace { // (anon11)
	using Common::spsInfo_t;
typedef struct CodecSpecificData {
	jobject jobj_csd0;
	int iNALHeaderLengthSize;
	unsigned char *tempBuffer;
	int tempBufferLen;
	spsInfo_t spsInfo;
} CodecSpecificData;
} // namespace (anon11)

//namespace { // (anotheranon1)
//	using Common::TranslatedOMXProfile;
//	using Common::TranslatedOMXLevel;
//}

NXINT32 GetPropertyCommon( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	int retValue = Video::Decoder::GetProperty(uProperty, puValue, pUserData);

	//process separate part
	unsigned int input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_INITPS_FORMAT :
		*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;

		if (Utils::ChipType::EDEN == Utils::GetChipType())
		{
			*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;
		}
		break;

	case NEXCAL_PROPERTY_BYTESTREAM_FORMAT :
		*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;

		if (Utils::ChipType::EDEN == Utils::GetChipType())
		{
			*puValue = NEXCAL_PROPERTY_BYTESTREAMFORMAT_RAW;
		}
		break;

	case NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM:
		*puValue= NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_OK;
		break;
	case NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER:
		*puValue= NEXCAL_PROPERTY_HEVC_SUPPORT_TO_INCLUDE_CONFIGRECORDER_OK;
		break;
        
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}

NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = GetPropertyCommon(uProperty, puValue, pUserData);

	//process separate part
	NXINT64  input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
	case NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT:
	case NEXCAL_PROPERTY_HEVC_SUPPORT_PROFILE:
	case NEXCAL_PROPERTY_HEVC_MAIN_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_HEVC_MAIN10_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_HEVC_MAINSP_SUPPORT_LEVEL:
		{
			pthread_mutex_lock(&g_preferredDecoderMutex);
			if (!g_checkedForPreferredDecoder)
			{
				int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, NexMediaCodec::HW, g_preferredDecoderName, &g_profileLevels, &g_supportedFeature);
				g_checkedForPreferredDecoder = true;
			}
			pthread_mutex_unlock(&g_preferredDecoderMutex);
		}
		break;
	}

	bool isLevelCheck = false;
	int profileForLevelCheck = 0;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
		{
			if (NULL == g_preferredDecoderName)
			{
				MC_ERR("couldn't find hw HEVC decoder");
				*puValue = (NXINT64)"not found";
			}
			else
			{
				*puValue = (NXINT64)g_preferredDecoderName;
			}
		}
		break;

	case NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT:
		{
			int supportSeamless = g_supportedFeature & Nex_MC::JNI::MC_Feat_AdaptivePlayback;
			if(supportSeamless)
				*puValue = (NXINT64)NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT_YES;
			else
				*puValue = (NXINT64)NEXCAL_PROPERTY_DECODER_SEAMLESS_SUPPORT_NO;
		}
		break;

	case NEXCAL_PROPERTY_HEVC_SUPPORT_PROFILE:
		{
			int highestProfile = 0;

			for (int i = 0; i < g_profileLevels[0]; ++i)
			{
				highestProfile = MAX(highestProfile, g_profileLevels[i*2 + 1]/*TranslatedOMXProfile(g_profileLevels[i*2 + 1])*/);
			}

			if (0 == highestProfile)
			{
				highestProfile = HEVC_PROFILE_MAIN;
			}

			*puValue = (NXINT64)highestProfile;
		}
		break;

#if 0
	case NEXCAL_PROPERTY_HEVC_MAIN_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = HEVC_PROFILE_MAIN;
		}
		break;
	case NEXCAL_PROPERTY_HEVC_MAIN10_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = HEVC_PROFILE_MAIN10;
		}
		break;
	case NEXCAL_PROPERTY_HEVC_MAINSP_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = HEVC_PROFILE_MAIN10;
		}
		break;
#endif        
	}

#if 0
	if (isLevelCheck)
	{
		int highestLevel = 0;

		for (int i = 0; i < g_profileLevels[0]; ++i)
		{
			if (profileForLevelCheck == TranslatedOMXProfile(g_profileLevels[i*2 + 1]))
			{
				highestLevel = MAX(highestLevel, (TranslatedOMXLevel(g_profileLevels[i*2 + 2])/10));
			}
		}

		*puValue = (NXINT64)highestLevel;
	}
#endif

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

static NXINT32 _SetPropertyReal(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	NexCAL_mc *mc = (NexCAL_mc *)a_pUserData;

	switch ( a_uProperty )
	{
	case NEXCAL_PROPERTY_VIDEO_FPS :
			mc->vd.fps = (NXINT32)a_qValue;
		break;
	}
	return 0;
}

NXINT32 SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	WrapSetProperty::CheckRet ret = g_wrapSetProperty.CheckClientIdExist( a_pUserData );
	if ( WrapSetProperty::NONE_EXIST == ret )
	{
		g_wrapSetProperty.RegisterClientId( a_pUserData );
		ret = WrapSetProperty::ID_ONLY_EXIST;
	}

	if ( WrapSetProperty::ID_ONLY_EXIST == ret )
	{
		g_wrapSetProperty.PushPropertyAndValuePair( a_pUserData, a_uProperty, a_qValue );
	}
	else // ret == USERDATA_EXIST
	{
		_SetPropertyReal( a_uProperty, a_qValue, a_pUserData );
	}
	
	//bool bMC = CheckRegistedMC( a_pUserData );
	return 0;
}

NEXCALSetProperty SetPropertyF = SetProperty;

NXINT32 Deinit(NXVOID *pUserData);

NXINT32 Init( 
		NEX_CODEC_TYPE eCodecType
		, NXUINT8* pConfig
		, NXINT32 iLen
		, NXUINT8* pConfigEnhance
		, NXINT32 iEnhLen
		, NXVOID *pInitInfo
		, NXVOID *pExtraInfo
		, NXINT32 iNALHeaderLengthSize
		, NXINT32* piWidth
		, NXINT32* piHeight
		, NXINT32* piPitch
		, NXUINT32 uMode
		, NXUINT32 uUserDataType
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "eCodecType(0x%X) udtype(0x%X)", eCodecType, uUserDataType);

	int retValue = 0;

	NexCAL_mc *mc = Video::Decoder::CreateNexCAL_mc(CODEC_NAME, MIMETYPE, *piWidth, *piHeight, uUserDataType, ppUserData);

	if (NULL == mc)
	{
		MC_ERR("failed to create userData");
		return -1;
	}

	pthread_mutex_lock(&g_preferredDecoderMutex);
	if (!g_checkedForPreferredDecoder)
	{
		int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, NexMediaCodec::HW, g_preferredDecoderName, &g_profileLevels, &g_supportedFeature);
		g_checkedForPreferredDecoder = true;
	}
	pthread_mutex_unlock(&g_preferredDecoderMutex);

	if (NULL == g_preferredDecoderName)
	{
		MC_ERR("couldn't find hw H264 decoder");
		*ppUserData = 0;
		Deinit((NXVOID *)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -2);
		return -2;
	}

	mc->preferredCodecName = (const char *)g_preferredDecoderName;

	CodecSpecificData *csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
	if (NULL == csd)
	{
		MC_ERR("malloc failed!");
		*ppUserData = 0;
		free(mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -21);
		return -21;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
	memset(csd, 0x00, sizeof(CodecSpecificData));

	csd->iNALHeaderLengthSize = iNALHeaderLengthSize;

	if (0 < iLen && NULL != pConfig)
	{
		if (Utils::ChipType::EDEN == Utils::GetChipType())
		{
			unsigned char *pNewConfig = (unsigned char *)malloc(iLen + 64);
			if (NULL == pNewConfig)
			{
				MC_ERR("malloc failed!");
				*ppUserData = 0;
				Deinit((NXVOID *)mc);
				IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -22);
				return -22;
			}
			STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", pNewConfig, iLen + 64);

			int iNewLen = Common::convertRawToAnnexB_4byte_config(pNewConfig, iLen + 64, pConfig, iLen);

			if (0 < iNewLen)
			{
				pConfig = pNewConfig;
				iLen = iNewLen;
			}
		}

#if 0
		if (parseSPS(pConfig, iLen, &csd->spsInfo))
		{
			STATUS(Log::FLOW, Log::DEBUG, "interlace (%d)", 0 == csd->spsInfo.frame_mbs_only_flag ? 1 : 0);
			mc->inputFormat->setInteger("interlace", 0 == csd->spsInfo.frame_mbs_only_flag ? 1 : 0);
		}
#endif

		void *csd0;
		csd->jobj_csd0 = Utils::JNI::NewDirectByteBuffer(iLen, csd0);
		memcpy(csd0, pConfig, iLen);

		if (Utils::ChipType::EDEN == Utils::GetChipType())
		{
			free(pConfig);
			pConfig = NULL;
		}

		mc->vd.codecSpecific = (void *)csd;
	}

	if (NULL != csd && NULL != csd->jobj_csd0)
	{
		mc->inputFormat->setByteBuffer("csd-0", csd->jobj_csd0);
	}

	NXUINT32 uProperty;
	NXINT64 qValue;
	WrapSetProperty::PopRet pret;
	while ( WrapSetProperty::VALID_PAIR == ( pret = g_wrapSetProperty.PopPropertyAndValuePair( *ppUserData, &uProperty, &qValue ) ) )
	{
		_SetPropertyReal( uProperty, qValue, (NXVOID *)mc );
	}
	g_wrapSetProperty.RegisterUserData( *ppUserData, (NXVOID *)mc  );
	*ppUserData = (NXVOID *)mc;

	retValue = Video::Decoder::Init(mc);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID *)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	*piWidth = mc->vd.width;
	*piHeight = mc->vd.height;
	*piPitch = mc->vd.pitch;

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoInit InitF = Init;

NXINT32 Deinit( NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "");

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;

	g_wrapSetProperty.UnregisterUserData( pUserData );

	// call common Deinit
	NXINT32 retValue = Video::Decoder::Deinit(mc);

	CodecSpecificData *csd = (CodecSpecificData *)mc->vd.codecSpecific;

	if (NULL != csd)
	{
		if (NULL != csd->jobj_csd0)
		{
			Utils::JNI::DeleteDirectByteBuffer(csd->jobj_csd0);
		}

		if (NULL != csd->tempBuffer)
		{
			free(csd->tempBuffer);
			csd->tempBuffer = NULL;
			csd->tempBufferLen = 0;
		}

		free(csd);
		csd = NULL;
		mc->vd.codecSpecific = NULL;
	}

	if (NULL != mc->vd.crypto)
	{
		NexMediaCodec_using_jni::releaseMediaCrypto(mc->vd.crypto);
		mc->vd.crypto = NULL;
	}

	if (NULL != mc->vd.cryptoInfo)
	{
		NexMediaCodec_using_jni::releaseCryptoInfo(mc->vd.cryptoInfo);
		mc->vd.cryptoInfo = NULL;
	}

	free(mc);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoDeinit DeinitF = Deinit;

NXINT32 Decode( 
		NXUINT8* pData
		, NXINT32 iLen
		, NXVOID *pExtraInfo
		, NXUINT32 uDTS
		, NXUINT32 uPTS
		, NXINT32 nFlag
		, NXUINT32* puDecodeResult
		, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "len(%d) ts(%u/%u) flag(0x%X)", iLen, uDTS, uPTS, nFlag);

	NXINT32 retValue = 0;

	if (Utils::ChipType::EDEN == Utils::GetChipType() && NULL != pData && 0 < iLen)
	{
		NexCAL_mc *mc = (NexCAL_mc *)pUserData;

		CodecSpecificData *csd = (CodecSpecificData *)mc->vd.codecSpecific;

		unsigned char *pNewData = pData;
		int iTargetLen = iLen;

		if (4 != csd->iNALHeaderLengthSize)
		{
			if (csd->tempBufferLen < iLen + 64)
			{
				if (NULL != csd->tempBuffer)
				{
					free(csd->tempBuffer);
				}

				csd->tempBuffer = (unsigned char *)malloc(iLen + 64);
				if (NULL == csd->tempBuffer)
				{
					MC_ERR("malloc failed!");
					NEXCAL_INIT_VDEC_RET(*puDecodeResult);
					IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) decRet(0x%X)", -1, *puDecodeResult);
					return -1;
				}
				STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd->tempBuffer, iLen + 64);
				csd->tempBufferLen = iLen + 64;
			}
			iTargetLen = csd->tempBufferLen;
			pNewData = csd->tempBuffer;
		}
		int iNewLen = Common::convertRawToAnnexB_4byte(pNewData, iTargetLen, pData, iLen, csd->iNALHeaderLengthSize);

		if (0 < iNewLen)
		{
			pData = pNewData;
			iLen = iNewLen;
		}
	}

	retValue = Video::Decoder::Decode(pData, iLen, uDTS, uPTS, nFlag, puDecodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
	return retValue;
}
NEXCALVideoDecode DecodeF = Decode;

int QueryForSupport(uint8_t *dsi, size_t dsiLen)
{
	IDENTITY("+", CODEC_NAME, 0, "dsi(%p) dsiLen(%zu)", dsi, dsiLen);
	int ret = NEXCAL_ERROR_NOT_SUPPORT_DEVICE;

	pthread_mutex_lock(&g_preferredDecoderMutex);
	if (!g_checkedForPreferredDecoder)
	{
		int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, NexMediaCodec::HW, g_preferredDecoderName, &g_profileLevels, &g_supportedFeature);
		g_checkedForPreferredDecoder = true;
	}
	pthread_mutex_unlock(&g_preferredDecoderMutex);

	return NEXCAL_ERROR_NONE;

	if (NULL == g_profileLevels)
	{
		MC_ERR("couldn't find hw H264 decoder");
		ret = NEXCAL_ERROR_NOT_SUPPORT_DEVICE;
	}
	else
	{
		uint8_t *new_dsi = NULL;
		int new_dsiLen = 0;
		if (2 <= dsiLen && (0x00 != dsi[0] || 0x00 != dsi[1])) // not annexb 4-byte format
		{
			new_dsi = (uint8_t *)malloc(dsiLen + 64);
			if (NULL == new_dsi)
			{
				MC_ERR("malloc failed!");
				return NEXCAL_ERROR_NOT_SUPPORT_DEVICE;
			}

			STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", new_dsi, dsiLen + 64);

			new_dsiLen = Common::convertRawToAnnexB_4byte_config(new_dsi, dsiLen + 64, dsi, dsiLen);

			if (0 < new_dsiLen)
			{
				dsi = new_dsi;
				dsiLen = new_dsiLen;
			}
		}

		{
			uint8_t *pIn = dsi;
			int nalHeaderLength = 0;

			int nalLen = 0;

			while ((size_t)(pIn - dsi) < dsiLen)
			{
				nalLen = Common::getNalSize(pIn, dsiLen - (pIn - dsi), nalHeaderLength);

				if (0 > nalLen)
				{
					MC_ERR("error while parsing SPS (not annex-b format or couldn't find SPS)");
					ret = NEXCAL_ERROR_INVALID_PARAMETER;
					break;
				}

				if (0x7 == (0x1F & pIn[nalHeaderLength]))
				{
					Common::spsInfo_t info;
					memset(&info, 0x00, sizeof(Common::spsInfo_t));

					//if (!parseSPS(pIn, nalLen + nalHeaderLength, &info))
					{
						MC_ERR("parseSPS failed!");
						ret = NEXCAL_ERROR_INVALID_PARAMETER;
						break;
					}

#if 0
					for (int i = 0; i < g_profileLevels[0]; ++i)
					{
						int deviceLevel = TranslatedOMXLevel(g_profileLevels[i*2 + 2]);
						if (deviceLevel >= TranslatedSPSInfoLevel(&info) || LevelSupportsContentAtFPS(deviceLevel, &info, 14.9)) // 14.9 for ~15fps
						{
							int deviceProfile = TranslatedOMXProfile(g_profileLevels[i*2 + 1]);

							switch (deviceProfile)
							{
							case NEXAVCProfile_BASELINE:
								if (NEXAVCProfile_BASELINE == info.profile_idc || info.constraints.set0_flag)
								{
									ret = NEXCAL_ERROR_NONE;
								}
								break;
							case NEXAVCProfile_MAIN:
								if (NEXAVCProfile_MAIN == info.profile_idc || info.constraints.set1_flag)
								{
									ret = NEXCAL_ERROR_NONE;
								}
								break;
							case NEXAVCProfile_EXTENDED:
								if ( (NEXAVCProfile_EXTENDED == info.profile_idc || info.constraints.set2_flag)
									|| (NEXAVCProfile_BASELINE == info.profile_idc || info.constraints.set0_flag) )
								{
									ret = NEXCAL_ERROR_NONE;
								}
								break;
							case NEXAVCProfile_HIGH:
								if ( (NEXAVCProfile_MAIN == info.profile_idc || info.constraints.set1_flag)
									|| (NEXAVCProfile_HIGH == info.profile_idc))
								{
									ret = NEXCAL_ERROR_NONE;
								}
								break;
							case NEXAVCProfile_HIGH10:
								if ( (NEXAVCProfile_MAIN == info.profile_idc || info.constraints.set1_flag)
									|| (NEXAVCProfile_HIGH == info.profile_idc || NEXAVCProfile_HIGH10 == info.profile_idc))
								{
									ret = NEXCAL_ERROR_NONE;
								}
								break;
							case NEXAVCProfile_HIGH422:
								if ( (NEXAVCProfile_MAIN == info.profile_idc || info.constraints.set1_flag)
									|| (NEXAVCProfile_HIGH == info.profile_idc || NEXAVCProfile_HIGH10 == info.profile_idc || NEXAVCProfile_HIGH422 == info.profile_idc))
								{
									ret = NEXCAL_ERROR_NONE;
								}
								break;
							case NEXAVCProfile_HIGH444:
								if ( (NEXAVCProfile_MAIN == info.profile_idc || info.constraints.set1_flag)
									|| (NEXAVCProfile_HIGH == info.profile_idc || NEXAVCProfile_HIGH10 == info.profile_idc || NEXAVCProfile_HIGH422 == info.profile_idc || NEXAVCProfile_HIGH444 == info.profile_idc))
								{
									ret = NEXCAL_ERROR_NONE;
								}
								break;
							default:
								break;
							} // switch

							if (NEXCAL_ERROR_NONE == ret)
							{
								STATUS(Log::FLOW, Log::INFO, "found acceptable profile(%d) level(%d) content(%d, %d)"
									, deviceProfile, deviceLevel
									, info.profile_idc, TranslatedSPSInfoLevel(&info));
								break; // out of for-loop
							}
						}
					}
#endif
					break;
				}

				pIn += nalHeaderLength + nalLen;
			}

		}

		if (NULL != new_dsi)
		{
			free(new_dsi);
		}
	}

	IDENTITY("-", CODEC_NAME, 0, "ret(0x%X)", ret);
	return ret;
}

} // namespace Decoder

namespace Encoder {
#ifdef NEX_MC_TYPE
#undef NEX_MC_TYPE
#endif
#define NEX_MC_TYPE NEX_MC_TYPE_ENCODER

#ifdef CODEC_NAME
#undef CODEC_NAME
#endif
#define CODEC_NAME "HEVCE"

using Nex_MC::Video::Encoder::NexCAL_mcve;


// returns length of altered dsi
size_t setCropToAchieveResolution(uint8_t *dsi_in_out, uint32_t maxLength, size_t reqWidth, size_t reqHeight)
{
	unsigned int i, j, idx, idx_dst;
	unsigned char dsi_in[256] = {0,};

	short numOfArrays = 0;
	short numVPS = 0;
	short numSPS = 0;
	short numPPS = 0;

	//int iNewLen = HEVCconvertRawToAnnexB_4byte_config(dsi_in_annexb_4byte, 256, dsi_in_out, 150);

	memcpy(dsi_in, dsi_in_out, maxLength);
	memset(dsi_in_out, 0, maxLength);

	idx = 0;	idx_dst = 0;
	// check HEVCDecoderConfigurationRecord reserved bit to detect start position.
	if ((dsi_in[13] & 0xF0) == 0xF0 &&
		(dsi_in[15] & 0xFC) == 0xFC &&
		(dsi_in[16] & 0xFC) == 0xFC &&
		(dsi_in[17] & 0xF8) == 0xF8 &&
		(dsi_in[18] & 0xF8) == 0xF8
		)
	{
		memcpy(dsi_in_out, dsi_in, 22);
		numOfArrays = (unsigned int)dsi_in[22];
		idx     += 23;
		idx_dst += 22;
	}
	else
	{
		numOfArrays = (unsigned int)dsi_in[0];
		if(numOfArrays != 0)
			idx++;
	}

	if(numOfArrays == 0)
		return 0;

	dsi_in_out[idx_dst++] = (unsigned char)(numOfArrays & 0xFF);
	for(i = 0; i < numOfArrays; i++)
	{
		unsigned int numNalus, nal_unit_type, nal_unit_length;

		nal_unit_type = (unsigned int)(dsi_in[idx++] & 0x3F);
		numNalus      = (dsi_in[idx++] << 8);
		numNalus     |= (dsi_in[idx++] & 0xFF);

		dsi_in_out[idx_dst++] = (unsigned char)(nal_unit_type & 0xFF);
		dsi_in_out[idx_dst++] = (unsigned char)(numNalus >> 8);
		dsi_in_out[idx_dst++] = (unsigned char)(numNalus & 0xFF);

		if(nal_unit_type == 33)	// SPS
		{
			Common::BITSTREAM bs;
			Common::WBITSTREAM wbs;
			Common::spsInfo_t sps;
			unsigned int picWidth, picHeight;

			Common::InitWBitstream(&wbs, &dsi_in_out[idx_dst], 256 - idx_dst);

			for(j = 0; j < numNalus; j++)
			{
				nal_unit_length  = (dsi_in[idx++] << 8);
				nal_unit_length |= (dsi_in[idx++] & 0xFF);

				Common::Write_NBits(&wbs, nal_unit_length, 16);

				/////////////
				bs.BS = 0;
				bs.BitCount = 0;
				bs.endbuf = NULL;
				bs.rear = 0;
				bs.length = nal_unit_length;
				bs.buffer = (unsigned char*)&dsi_in[idx];

				_LoadBSHEVC(&bs);_LoadBSHEVC(&bs);_LoadBSHEVC(&bs);_LoadBSHEVC(&bs);
				_AlignBitsHEVC(&bs);

				// Parse SPS
				hevc_parse_sps(&bs, &sps);
				
				/////////////
				// update cropping info.
				{
					
					int cropWidth, cropHeight;
					int cropUnitX, cropUnitY;

					if(sps.chroma_format_idc == 1)	// CHROMA_FMT_IDC_YUV420
					{
						cropUnitX = 2;
						cropUnitY = 2;
					}
					else
					{
						cropUnitX = 1;
						cropUnitY = 1;
					}

					picWidth  = sps.pic_width_in_luma_samples;
					picHeight = sps.pic_height_in_luma_samples;
					if(sps.conformance_window_flag)	// cropping flag
					{
						picWidth  -= (sps.conf_win_left_offset   * cropUnitX);
						picWidth  -= (sps.conf_win_right_offset  * cropUnitX);
						picHeight -= (sps.conf_win_top_offset    * cropUnitY);
						picHeight -= (sps.conf_win_bottom_offset * cropUnitY);
					}

					if( (picWidth < reqWidth) || (picHeight < reqHeight) )
					{
						// error
						return 0;
					}

					cropWidth  = picWidth - reqWidth;
					cropHeight = picHeight - reqHeight;

					if(cropWidth || cropHeight)
					{
						sps.conformance_window_flag = 1;
						sps.conf_win_left_offset    = 0;
						sps.conf_win_right_offset   = cropWidth / cropUnitX;
						sps.conf_win_top_offset     = 0;
						sps.conf_win_bottom_offset  = cropHeight / cropUnitY;
					}
				}

				// Write SPS
				write_hevc_sps(&wbs, &sps);
				/////////////

				idx     += nal_unit_length;
				if( (wbs.idx - 2) != nal_unit_length)
				{
					nal_unit_length = (wbs.idx - 2);
					dsi_in_out[idx_dst + 0] = (nal_unit_length >> 8) & 0xff;
					dsi_in_out[idx_dst + 1] = (nal_unit_length) & 0xff;
				}
				idx_dst += wbs.idx;
			}
		}
		else
		{	// 32 : VPS, 34 : PPS
			for(j = 0; j < numNalus; j++)
			{
				nal_unit_length  = (dsi_in[idx++] << 8);
				nal_unit_length |= (dsi_in[idx++] & 0xFF);
				dsi_in_out[idx_dst++] = (nal_unit_length >> 8);
				dsi_in_out[idx_dst++] = (nal_unit_length & 0xFF);

				memcpy(&dsi_in_out[idx_dst], &dsi_in[idx], nal_unit_length);

				idx     += nal_unit_length;
				idx_dst += nal_unit_length;
			}
		}
	}

	return idx_dst;
}


namespace { // (anon101)
	using Nex_MC::Utils::WrapSetProperty;
	
	pthread_mutex_t g_preferredEncoderMutex;
	char *g_preferredEncoderName = NULL;
	bool g_checkedForPreferredEncoder = false;
	int *g_supportedColorFormats = NULL;

	int *g_encoderProfileLevels = NULL;

	WrapSetProperty g_wrapSetProperty;
	
	class CleanUpCrewEncoder {
	public:
		CleanUpCrewEncoder()
		{
			pthread_mutex_init(&g_preferredEncoderMutex, NULL);
		}

		~CleanUpCrewEncoder()
		{
			if (NULL != g_supportedColorFormats)
			{
				free(g_supportedColorFormats);
				g_supportedColorFormats = NULL;
			}

			pthread_mutex_lock(&g_preferredEncoderMutex);
			if (NULL != g_preferredEncoderName)
			{
				free(g_preferredEncoderName);
				g_preferredEncoderName = NULL;
			}
			if (NULL != g_encoderProfileLevels)
			{
				free(g_encoderProfileLevels);
				g_encoderProfileLevels = NULL;
			}
			g_checkedForPreferredEncoder = false;
			pthread_mutex_unlock(&g_preferredEncoderMutex);

			pthread_mutex_destroy(&g_preferredEncoderMutex);
		}
	};

	CleanUpCrewEncoder cleanUp;
} // namespace (anon101)

namespace { // (anon22)
typedef struct CodecSpecificData {
	void * csd0;
} CodecSpecificData;
}; // namespace (anon22)

//namespace { // (anotheranon2)
//	using Common::TranslatedOMXProfile;
//	using Common::TranslatedOMXLevel;
//}

NXINT32 GetProperty( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = Video::Encoder::GetProperty(uProperty, puValue, pUserData);

	//process separate part
	NXINT64 input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
	case NEXCAL_PROPERTY_HEVC_SUPPORT_PROFILE:
	case NEXCAL_PROPERTY_HEVC_MAIN_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_HEVC_MAIN10_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_HEVC_MAINSP_SUPPORT_LEVEL:     
		{
			pthread_mutex_lock(&g_preferredEncoderMutex);
			if (!g_checkedForPreferredEncoder)
			{
				int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName, &g_encoderProfileLevels);
				g_checkedForPreferredEncoder = true;
			}
			pthread_mutex_unlock(&g_preferredEncoderMutex);
		}
		break;
	}

	bool isLevelCheck = false;
	int profileForLevelCheck = 0;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
		{
			if (NULL == g_preferredEncoderName)
			{
				MC_ERR("couldn't find hw H264 encoder");
				*puValue = (NXINT64)"not found";
			}
			else
			{
				*puValue = (NXINT64)g_preferredEncoderName;
			}
		}
		break;

	case NEXCAL_PROPERTY_HEVC_SUPPORT_PROFILE:
		{
			int highestProfile = 0;

			for (int i = 0; i < g_encoderProfileLevels[0]; ++i)
			{
				highestProfile = MAX(highestProfile, g_encoderProfileLevels[i*2 + 1]/*TranslatedOMXProfile(g_encoderProfileLevels[i*2 + 1])*/);
			}

			if (0 == highestProfile)
			{
				highestProfile = 0x01;
			}

			*puValue = highestProfile;
		}
		break;

#if 0
	case NEXCAL_PROPERTY_HEVC_MAIN_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_BASELINE;
		}
		break;
	case NEXCAL_PROPERTY_HEVC_MAIN10_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_MAIN;
		}
		break;
	case NEXCAL_PROPERTY_HEVC_MAINSP_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_EXTENDED;
		}
		break;
#endif        
	}

#if 0
	if (isLevelCheck)
	{
		int highestLevel = 0;

		for (int i = 0; i < g_encoderProfileLevels[0]; ++i)
		{
			if (profileForLevelCheck == TranslatedOMXProfile(g_encoderProfileLevels[i*2 + 1]))
			{
				highestLevel = MAX(highestLevel, (TranslatedOMXLevel(g_encoderProfileLevels[i*2 + 2])/10));
			}
		}

		*puValue = (NXINT64)highestLevel;
	}
#endif

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 GetTranslatedOMXProfileValue(NXINT32 iValue)
{
	NXINT32 iRet = 0x01;
	switch (iValue)
	{
		case NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN:
			iRet = 0x01/*HEVCProfileMain*/;
			break;
		case NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN10:
			iRet = 0x02/*HEVCProfileMain10*/;
			break;
		case NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE_VAULE_IS_MAINSTILLPICTURE:
			iRet = 0x1000/*HEVCProfileMain10HDR10*/;
			break;
	}

	return iRet;
}

static NXINT32 _SetPropertyReal(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	NexCAL_mc *mc = (NexCAL_mc *)a_pUserData;

	switch ( a_uProperty )
	{
	case NEXCAL_PROPERTY_HEVC_ENCODER_SET_PROFILE:
			mc->ve.profile = GetTranslatedOMXProfileValue((NXINT32)a_qValue);
			//If not set level, encoder set profile to baseline.
			mc->ve.level = 0x400/*HEVCMainTierLevel41*/;
		break;

	case NEXCAL_PROPERTY_HEVC_ENCODER_SET_LEVEL:
			//mc->ve.level = (NXINT32)a_qValue;
			mc->ve.level = 0x400/*HEVCMainTierLevel41*/;
		break;
	}
	return 0;	
}

NXINT32 SetProperty(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	WrapSetProperty::CheckRet ret = g_wrapSetProperty.CheckClientIdExist( a_pUserData );
	if ( WrapSetProperty::NONE_EXIST == ret )
	{
		g_wrapSetProperty.RegisterClientId( a_pUserData );
		ret = WrapSetProperty::ID_ONLY_EXIST;
	}

	if ( WrapSetProperty::ID_ONLY_EXIST == ret )
	{
		g_wrapSetProperty.PushPropertyAndValuePair( a_pUserData, a_uProperty, a_qValue );
	}
	else // ret == USERDATA_EXIST
	{
		_SetPropertyReal( a_uProperty, a_qValue, a_pUserData );
	}
	
	//bool bMC = CheckRegistedMC( a_pUserData );
	return 0;
}

NEXCALSetProperty SetPropertyF = SetProperty;

NXINT32 Deinit(NXVOID *pUserData);

NXINT32 Init( 
		NEX_CODEC_TYPE eCodecType
		, NXUINT8** ppConfig
		, NXINT32* piConfigLen
		, NXINT32 iQuality
		, NXINT32 iWidth
		, NXINT32 iHeight
		, NXINT32 iPitch
		, NXINT32 iFPS
		, NXINT32 bCBR
		, NXINT32 iBitRate
		, NXVOID **ppUserData )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "oti(0x%X)", eCodecType);

	NXINT32 retValue = 0;
	int	backup_profile=NEXSAL_INFINITE, backup_level=NEXSAL_INFINITE;


	NexCAL_mc *mc = (NexCAL_mc *)malloc(sizeof(NexCAL_mc));
	if (NULL == mc)
	{
		MC_ERR("malloc failed!");
		*ppUserData = 0;
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -1);
		return -1;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mc, sizeof(NexCAL_mc));

RETRY_INIT:
	memset(mc, 0x00, sizeof(NexCAL_mc));

	mc->type = NEX_MC_TYPE;
	mc->avtype = NEX_MC_AVTYPE;
	mc->codecName = CODEC_NAME;

	mc->mimeType = MIMETYPE;

	mc->ve.quality = iQuality;
	mc->ve.width = iWidth;
	mc->ve.height = iHeight;
	mc->ve.fps = iFPS;
	mc->ve.cbr = 0 != bCBR;
	mc->ve.bitrate = iBitRate;
	mc->ve.profile = 0;
	mc->ve.colorFormat = 0x13 /* COLOR_FormatYUV420Planar */;

	if (NULL == JNI::JMETHODS::MediaCodec.createInputSurface)
	{
		pthread_mutex_lock(&g_preferredEncoderMutex);
		if (!g_checkedForPreferredEncoder)
		{
			int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName, &g_encoderProfileLevels);
			g_checkedForPreferredEncoder = true;
		}
		pthread_mutex_unlock(&g_preferredEncoderMutex);

		mc->preferredCodecName = (const char *)g_preferredEncoderName;

		if (NULL == g_supportedColorFormats)
		{
			NexMediaCodec_using_jni::getSupportedInputColorFormats(MIMETYPE, g_supportedColorFormats);
		}
		if (NULL != g_supportedColorFormats)
		{
			int i = 0;

			if (Nex_MC::Utils::ChipType::EXYNOS4 == Nex_MC::Utils::GetChipType()
				|| Nex_MC::Utils::ChipType::EXYNOS == Nex_MC::Utils::GetChipType())
			{
				for (i = 0; -1 != g_supportedColorFormats[i]; ++i)
				{
					if (0x15 == g_supportedColorFormats[i])
					{
						break;
					}
				}

				if (-1 == g_supportedColorFormats[i])
				{
					MC_WARN("couldn't find valid(0x15) input color format (%d)", i);
					i = 0;
				}
			}

			for ( ; -1 != g_supportedColorFormats[i]; ++i)
			{
				if (0x7f000789 /* COLOR_FormatSurface */ == g_supportedColorFormats[i]
					|| 0x7f000200 == g_supportedColorFormats[i])
				{
					continue;
				}
				else
				{
					mc->ve.colorFormat = g_supportedColorFormats[i];
					break;
				}
			}

			if (-1 == g_supportedColorFormats[i])
			{
				MC_WARN("couldn't find valid input color format (%d)", i);
			}
		}
		else
		{
			MC_WARN("couldn't find valid input color format");
		}
	}
	else
	{
		mc->ve.colorFormat = 0x7f000789 /* COLOR_FormatSurface */;
	}

	// MC_INFO("color selected: %d", mc->ve.colorFormat);

	//mc->ve.writeOutputFrame = WriteOutputFrame;


	// drain property_value pair queue
	NXUINT32 uProperty;
	NXINT64 qValue;
	WrapSetProperty::PopRet pret;
	while ( WrapSetProperty::VALID_PAIR == ( pret = g_wrapSetProperty.PopPropertyAndValuePair( *ppUserData, &uProperty, &qValue ) ) )
	{
		_SetPropertyReal( uProperty, qValue, (NXVOID *)mc );
	}

	if ( backup_profile == NEXSAL_INFINITE && backup_level == NEXSAL_INFINITE )
	{
		backup_profile = mc->ve.profile;
		backup_level = mc->ve.level;

		if ( g_encoderProfileLevels && (mc->ve.profile==0 || mc->ve.level==0) )
		{
			//in case that profile&level is not set
			if (mc->ve.profile == 0)
				mc->ve.profile = g_encoderProfileLevels[1];
			if (mc->ve.level == 0)
				mc->ve.level = g_encoderProfileLevels[2];

			for (int i = 0; i < g_encoderProfileLevels[0]; ++i)
			{
				if (g_encoderProfileLevels[i*2+1] > mc->ve.profile)
				{
					break;
				}
				else if (g_encoderProfileLevels[i*2+1] == mc->ve.profile)
				{
					mc->ve.level = g_encoderProfileLevels[i*2+2];
				}
			}
			MC_WARN("change to mc->ve.profile == 0x%x, mc->ve.level=0x%x\n",mc->ve.profile, mc->ve.level );
		}
	}
	else
	{
		mc->ve.profile = backup_profile;
		mc->ve.level = backup_level;
	}

	g_wrapSetProperty.RegisterUserData( *ppUserData, (NXVOID *)mc  );
	
	*ppUserData = (NXVOID *)mc;

	retValue = Video::Encoder::Init(mc);

	if (0 != retValue)
	{
		if ( backup_profile == 0 ||  backup_level == 0 )
		{
			MC_WARN("video encoder init fail. retry 0x01/0x200. - ret(0x%X)", retValue);
			Video::Encoder::Deinit(mc);
			//Nx_robin__170608 To-do : We must choose default value for HEVC.
			backup_profile = 0x01;
			backup_level = 0x200;
			goto RETRY_INIT;
		}
		else
		{
			*ppUserData = 0;
			Deinit((NXVOID *)mc);
			IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
			return retValue;
		}
	}

	if (false)
	{
		STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
		pthread_mutex_lock(&mc->engLock);
		STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

		mc->outTimeout = 1000; // 1ms

		int breakAt0 = 3;

		while (0 > mc->outIndex)
		{
			mc->engWaiting = true;

			if (0 >= breakAt0)
			{
				break;
			}

			if (-2 == mc->outIndex)
			{
				mc->outIndex = -1;
			}

			if (mc->outWaiting)
			{
				STATUS(Log::MUTEX, Log::VERBOSE, "++(%s)", "outLock");
				pthread_mutex_lock(&mc->outLock);
				STATUS(Log::MUTEX, Log::VERBOSE, "(%s)++", "outLock");

				mc->outWaiting = false;

				STATUS(Log::MUTEX, Log::VERBOSE, "( ( (%sCond) ) )", "out");
				pthread_cond_broadcast(&mc->outCond);

				STATUS(Log::MUTEX, Log::VERBOSE, "--(%s)", "outLock");
				pthread_mutex_unlock(&mc->outLock);
			}

			STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
			pthread_mutex_unlock(&mc->engLock);
			sched_yield();
			STATUS(Log::MUTEX, Log::DEBUG, "++(%s)", "engLock");
			pthread_mutex_lock(&mc->engLock);
			STATUS(Log::MUTEX, Log::DEBUG, "(%s)++", "engLock");

			mc->engWaiting = false;

			--breakAt0;
		}

		STATUS(Log::FLOW, Log::INFO, "output info: %ld %d %d %lld 0x%X", mc->outIndex, mc->lastOutputOffset, mc->lastOutputSize, mc->lastOutputPTS, mc->lastOutputFlags);

		*ppConfig = NULL;
		*piConfigLen = 0;

		if (0 > mc->outIndex)
		{
			MC_WARN("timed out waiting for DSI!");
		}
		else
		{
			if (2 != (mc->lastOutputFlags & 2))
			{
				MC_WARN("first output from encoder was not CODEC_CONFIG! No DSI found!");
			}
			else
			{
				CodecSpecificData *csd = NULL;

				csd = (CodecSpecificData *)malloc(sizeof(CodecSpecificData));
				if (NULL == csd)
				{
					MC_ERR("malloc failed!");
					*ppUserData = 0;
					Deinit((NXVOID *)mc);
					IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -21);
					return -21;
				}
				STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd, sizeof(CodecSpecificData));
				memset(csd, 0x00, sizeof(CodecSpecificData));
				csd->csd0 = malloc(mc->lastOutputSize);
				if (NULL == csd->csd0)
				{
					MC_ERR("malloc failed!");
					*ppUserData = 0;
					Deinit((NXVOID *)mc);
					IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -22);
					return -22;
				}
				STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", csd->csd0, mc->lastOutputSize);

				memcpy(csd->csd0, ((unsigned char *)mc->outputBuffers[mc->outIndex]) + mc->lastOutputOffset, mc->lastOutputSize);

				mc->ve.codecSpecific = (void *)csd;

				*ppConfig = (unsigned char *)csd->csd0;
				*piConfigLen = mc->lastOutputSize;
			}

			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "->releaseOutputBuffer");
			mc->mc->releaseOutputBuffer(mc->outIndex, false);
			STATUS(Log::EXTERNAL_CALL, Log::DEBUG, "releaseOutputBuffer->");
			mc->outIndex = -1;
		}

		STATUS(Log::MUTEX, Log::DEBUG, "--(%s)", "engLock");
		pthread_mutex_unlock(&mc->engLock);

		//Nex_MC::Video::Encoder::Reset((NXVOID *)mc);

		mc->outTimeout = mc->defaultOutTimeout;
	}

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoEncoderInit InitF = Init;

NXINT32 Deinit( NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "");

	NexCAL_mc *mc = (NexCAL_mc *)pUserData;


	g_wrapSetProperty.UnregisterUserData( pUserData );


	// call common Deinit
	NXINT32 retValue = Video::Encoder::Deinit(mc);

	CodecSpecificData *csd = (CodecSpecificData *)mc->ve.codecSpecific;

	if (NULL != csd)
	{
		if (NULL != csd->csd0)
		{
			free(csd->csd0);
		}

		free(csd);
		csd = NULL;
		mc->ve.codecSpecific = NULL;
	}

	free(mc);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoEncoderDeinit DeinitF = Deinit;

NXINT32 Encode( 
		NXUINT8* pData1
		, NXUINT8* pData2
		, NXUINT8* pData3
	//	, NXUINT8** ppOutData
	//	, NXUINT32* piOutLen
		, NXUINT32 uPTS
		, NXUINT32* puEncodeResult
		, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "pData(%p,%p,%p) uPTS(%u)", pData1, pData2, pData3, uPTS);

	NXINT32 retValue = 0;

	retValue = Video::Encoder::Encode(pData1, pData2, pData3, uPTS, puEncodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
	return retValue;
}
NEXCALVideoEncoderEncode EncodeF = Encode;

} // namespace Encoder
} // namespace HEVC

} // namespace Nex_MC
