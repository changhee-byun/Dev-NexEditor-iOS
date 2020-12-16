/******************************************************************************
* File Name        : h264.cpp
* Description      : h264 codec for Android
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#define LOCATION_TAG "h264"

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

namespace H264 {

const char *MIMETYPE = "video/avc";

namespace { // (anon1)

typedef enum
{
	NEXAVCProfile_UNKNOWN		    = 0,
	//NEXAVCProfile_CAVLC			= 44,	//CAVLC 4:4:4 Intra profile
	NEXAVCProfile_BASELINE		    = 66,
	NEXAVCProfile_MAIN			    = 77,
	NEXAVCProfile_EXTENDED		    = 88,		//Extended Profile
	NEXAVCProfile_HIGH				= 100,		//High Profile
	NEXAVCProfile_HIGH10			= 110,		//High 10 Profile
	NEXAVCProfile_HIGH422			= 122,		//High 4:2:2 Profile
	NEXAVCProfile_HIGH444			= 244,		//High 4:4:4 Profile

	NEXAVCProfile_FOR4BYTESALIGN	= 0x7FFFFFFF

} NEXAVCProfile;

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
		char c;
		unsigned short usSize;
		unsigned char *p = (unsigned char*)&usSize;

		int iNewLen = iTargetLen;

		if (   ( 0x00 != pSource[1] ) // size if greater than 256
			&& ( 7 != ( pSource[3] & 0x1F ) ) // nal type is not sps
			&& ( pSource[1] == pSource[9] )
			&& 15 < iSourceLen ) //
		{
			c = ( pSource[5] & 0x1F );
			pSource += 6;
			iSourceLen -= 6;
		}
		else
		{
			c = (char)(*pSource & 0x1F);

			if(c != 0)
			{
				pSource++; iSourceLen--;
			}
			else
			{
				c = 1; // [shoh][2012.04.20] This is exception case for abscence of SPS count . ex) pot encoder
			}
		}

		for ( int i = 0; i < c && 4 < iTargetLen; i++ )
		{
			memcpy( pTarget, NAL_START_CODE, sizeof(NAL_START_CODE) );
			pTarget += sizeof(NAL_START_CODE);
			iTargetLen -= sizeof(NAL_START_CODE);

			memcpy( p, pSource, 2 );
			pSource += 2;
			iSourceLen -= 2;

			usSize = Utils::readBigEndianValue16( p );

			memcpy( pTarget, pSource, usSize );
			pTarget += usSize;
			iTargetLen -= usSize;
			pSource += usSize;
			iSourceLen -= usSize;
		}

		c = (char)*pSource;

		if(c != 0)
		{
			pSource++;
			iSourceLen--;
		}
		else
		{
			c = 1; // [shoh][2012.04.20] This is exception case for abscence of PPS count . ex) pot encoder
		}

		for ( int i = 0; i < c && 4 < iTargetLen; i++ )
		{
			memcpy( pTarget, NAL_START_CODE, sizeof(NAL_START_CODE) );
			pTarget += sizeof(NAL_START_CODE);
			iTargetLen -= sizeof(NAL_START_CODE);

			memcpy( p, pSource, 2 );
			pSource += 2; iSourceLen -= 2;

			usSize = Utils::readBigEndianValue16( p );

			memcpy( pTarget, pSource, usSize );
			pTarget += usSize;
			iTargetLen -= usSize;
			pSource += usSize;
			iSourceLen -= usSize;
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
} // namespace (anon321)

namespace { // anon320

using Nex_MC::Utils::ue_t;
using Nex_MC::Utils::se_t;
using Nex_MC::Utils::BitStreamReader;
using Nex_MC::Utils::BitStreamWriter;

class NALReader : public BitStreamReader
{
public:
	NALReader(const uint8_t *_pIn, const int _inLen)
	 : BitStreamReader(_pIn, _inLen)
	 , numZeroBytes(0)
	{
	}

	virtual ~NALReader() {}

	virtual bool ReadBit()
	{
		if (0 == bitOffset)
		{
			if (2 == numZeroBytes && 3 == (*pIn))
			{
				++pIn;
				numZeroBytes = 0;
			}

			if (0 == (*pIn))
			{
				++numZeroBytes;
			}
			else
			{
				numZeroBytes = 0;
			}
		}

		return BitStreamReader::ReadBit();
	}
private:
	uint8_t numZeroBytes;
};

class NALWriter : public BitStreamWriter
{
public:
	NALWriter(uint8_t *_pOut)
	 : BitStreamWriter(_pOut)
	 , numZeroBytes(0)
	{
	}

	virtual ~NALWriter() {}

	virtual void WriteBit(bool setBit)
	{
		BitStreamWriter::WriteBit(setBit);

		if (0 == bitOffset)
		{
			if (2 == numZeroBytes)
			{
				uint8_t temp = *(pOut-1);
				switch (temp)
				{
				case 0:
				case 1:
				case 2:
					{
						*(pOut-1) = 3;
						*pOut++ = temp;
						numZeroBytes = 0 == temp ? 1 : 0;
					}
					break;
				default:
					numZeroBytes = 0;
					break;
				}
			}
			else if (0 == *(pOut-1))
			{
				++numZeroBytes;
			}
		}
	}

	virtual void WriteBits(bool setBit, int numBits)
	{
		while (0 < numBits)
		{
			WriteBit(setBit);
			--numBits;
		}
	}
private:
	uint8_t numZeroBytes;
};

typedef struct spsInfo_t {
	struct nal_unit {
		bool forbidden_zero_bit : 1;
		unsigned nal_ref_idc : 2;
		unsigned nal_unit_type : 5;
	} __attribute__ ((packed)) nal_unit_info;
	unsigned profile_idc:8;
	struct constraint_set {
		bool set0_flag : 1;
		bool set1_flag : 1;
		bool set2_flag : 1;
		bool set3_flag : 1;
		bool set4_flag : 1;
		bool set5_flag : 1;
		unsigned reserved : 2;
	} __attribute__ ((packed)) constraints;
	unsigned level_idc:8;

	ue_t seq_parameter_set_id;

	// profile == 100 || 110 || 122 || 244 || 44 || 83 || 86 || 118 || 128
	ue_t chroma_format_idc;
	// chroma_format_idc==3
	bool separate_colour_plane_flag:1;

	ue_t bit_depth_luma_minus8;
	ue_t bit_depth_chroma_minus8;
	bool qpprime_y_zero_transform_bypass_flag:1;

	bool seq_scaling_matrix_present_flag:1;
	// seq_scaling_matrix_present_flag
	struct scaling_list_struct {
		bool seq_scaling_list_present_flag:1;
		size_t numDeltaScales;
		se_t delta_scale[64];
	} __attribute__ ((packed)) scaling_list[12];
	//

	ue_t log2_max_frame_num_minus4;

	ue_t pic_order_cnt_type;

	// type==0
	ue_t log2_max_pic_order_cnt_lsb_minus4;
	// type ==1
	bool delta_pic_order_always_zero_flag:1;
	se_t offset_for_non_ref_pic;
	se_t offset_for_top_to_bottom_field;
	ue_t num_ref_frames_in_pic_order_cnt_cycle;
	se_t offset_for_ref_frame[128];

	ue_t max_num_ref_frames;
	bool gaps_in_frame_num_value_allowed_flag:1;
	ue_t pic_width_in_mbs_minus1;
	ue_t pic_height_in_map_units_minus1;
	bool frame_mbs_only_flag:1;

	// !frame_mbs_only_flag
	bool mb_adaptive_frame_field_flag:1;

	bool direct_8x8_inference_flag:1;
	bool frame_cropping_flag:1;

	// frame_cropping_flag
	ue_t frame_crop_left_offset;
	ue_t frame_crop_right_offset;
	ue_t frame_crop_top_offset;
	ue_t frame_crop_bottom_offset;

	bool vui_parameters_present_flag:1;

	struct vui_parameters {
		bool aspect_ratio_info_present_flag:1;
		unsigned aspect_ratio_idc:8;
		unsigned sar_width:16;
		unsigned sar_height:16;

		bool overscan_info_present_flag:1;
		bool overscan_appropriate_flag:1;

		bool video_signal_type_present_flag:1;
		unsigned video_format:3;
		bool video_full_range_flag:1;
		bool colour_description_present_flag:1;
		unsigned colour_primaries:8;
		unsigned transfer_characteristics:8;
		unsigned matrix_coefficients:8;

		bool chroma_loc_info_present_flag:1;
		ue_t chroma_sample_loc_type_top_field;
		ue_t chroma_sample_loc_type_bottom_field;

		bool timing_info_present_flag:1;
		unsigned num_units_in_tick:32;
		unsigned time_scale:32;
		bool fixed_frame_rate_flag:1;

		bool nal_hrd_parameters_present_flag:1;
		bool vcl_hrd_parameters_present_flag:1;

		struct hrd_parameters {
			ue_t cpb_cnt_minus1;
			unsigned bit_rate_scale:4;
			unsigned cpb_size_scale:4;

			ue_t bit_rate_value_minus1[32];
			ue_t cpb_size_value_minus1[32];
			bool cbr_flag[32];

			unsigned initial_cpb_removal_delay_length_minus1:5;
			unsigned cpb_removal_delay_length_minus1:5;
			unsigned dpb_output_delay_length_minus1:5;
			unsigned time_offset_length:5;
		} __attribute__ ((packed)) hrd;

		bool low_delay_hrd_flag:1;

		bool pic_struct_present_flag:1;
		bool bitstream_restriction_flag:1;
		bool motion_vectors_over_pic_boundaries_flag:1;
		ue_t max_bytes_per_pic_denom;
		ue_t max_bits_per_mb_denom;
		ue_t log2_max_mv_length_horizontal;
		ue_t log2_max_mv_length_vertical;
		ue_t num_reorder_frames;
		ue_t max_dec_frame_buffering;
	} __attribute__ ((packed)) vui;
} __attribute__ ((packed)) spsInfo_t;

void parseScalingList( NALReader *nr, size_t sizeOfScalingList, struct spsInfo_t::scaling_list_struct *pScalingList, bool &useDefaultScalingMatrixFlag )
{
	se_t scalingList[64];

	se_t lastScale = 8;
	se_t nextScale = 8;

	pScalingList->numDeltaScales = 0;

	for (size_t j = 0; j < sizeOfScalingList; ++j)
	{
		if (0 != nextScale)
		{
			pScalingList->delta_scale[pScalingList->numDeltaScales] = nr->se();
			nextScale = (lastScale + pScalingList->delta_scale[pScalingList->numDeltaScales] + 256) % 256;
			++pScalingList->numDeltaScales;
			useDefaultScalingMatrixFlag = (j == 0 && nextScale == 0);
		}
		scalingList[j] = (nextScale == 0) ? lastScale : nextScale;
		lastScale = scalingList[j];
	}
}

void parseHRDParams( NALReader *nr, spsInfo_t *pInfo )
{
	pInfo->vui.hrd.cpb_cnt_minus1 = nr->ue();
	pInfo->vui.hrd.bit_rate_scale = nr->ReadBits(4);
	pInfo->vui.hrd.cpb_size_scale = nr->ReadBits(4);
	for (size_t SchedSelIdx = 0; SchedSelIdx <= pInfo->vui.hrd.cpb_cnt_minus1; ++SchedSelIdx)
	{
		pInfo->vui.hrd.bit_rate_value_minus1[SchedSelIdx] = nr->ue();
		pInfo->vui.hrd.cpb_size_value_minus1[SchedSelIdx] = nr->ue();
		pInfo->vui.hrd.cbr_flag[SchedSelIdx] = nr->ReadBit();
	}
	pInfo->vui.hrd.initial_cpb_removal_delay_length_minus1 = nr->ReadBits(5);
	pInfo->vui.hrd.cpb_removal_delay_length_minus1 = nr->ReadBits(5);
	pInfo->vui.hrd.dpb_output_delay_length_minus1 = nr->ReadBits(5);
	pInfo->vui.hrd.time_offset_length = nr->ReadBits(5);
}

void parseVUI( NALReader *nr, spsInfo_t *pInfo )
{
	pInfo->vui.aspect_ratio_info_present_flag = nr->ReadBit();
	if (pInfo->vui.aspect_ratio_info_present_flag)
	{
		pInfo->vui.aspect_ratio_idc = nr->ReadBits(8);
		if (255 /*Extended_SAR*/ == pInfo->vui.aspect_ratio_idc)
		{
			pInfo->vui.sar_width = nr->ReadBits(16);
			pInfo->vui.sar_height = nr->ReadBits(16);
		}
	}

	pInfo->vui.overscan_info_present_flag = nr->ReadBit();
	if (pInfo->vui.overscan_info_present_flag)
	{
		pInfo->vui.overscan_appropriate_flag = nr->ReadBit();
	}

	pInfo->vui.video_signal_type_present_flag = nr->ReadBit();
	if (pInfo->vui.video_signal_type_present_flag)
	{
		pInfo->vui.video_format = nr->ReadBits(3);
		pInfo->vui.video_full_range_flag = nr->ReadBit();
		pInfo->vui.colour_description_present_flag = nr->ReadBit();
		if (pInfo->vui.colour_description_present_flag)
		{
			pInfo->vui.colour_primaries = nr->ReadBits(8);
			pInfo->vui.transfer_characteristics = nr->ReadBits(8);
			pInfo->vui.matrix_coefficients = nr->ReadBits(8);
		}
	}

	pInfo->vui.chroma_loc_info_present_flag = nr->ReadBit();
	if (pInfo->vui.chroma_loc_info_present_flag)
	{
		pInfo->vui.chroma_sample_loc_type_top_field = nr->ue();
		pInfo->vui.chroma_sample_loc_type_bottom_field = nr->ue();
	}

	pInfo->vui.timing_info_present_flag = nr->ReadBit();
	if (pInfo->vui.timing_info_present_flag)
	{
		pInfo->vui.num_units_in_tick = nr->ReadBits(32);
		pInfo->vui.time_scale = nr->ReadBits(32);
		pInfo->vui.fixed_frame_rate_flag = nr->ReadBit();
	}

	pInfo->vui.nal_hrd_parameters_present_flag = nr->ReadBit();
	if (pInfo->vui.nal_hrd_parameters_present_flag)
	{
		parseHRDParams(nr, pInfo);
	}

	pInfo->vui.vcl_hrd_parameters_present_flag = nr->ReadBit();
	if (pInfo->vui.vcl_hrd_parameters_present_flag)
	{
		parseHRDParams(nr, pInfo);
	}

	if (pInfo->vui.nal_hrd_parameters_present_flag || pInfo->vui.vcl_hrd_parameters_present_flag)
	{
		pInfo->vui.low_delay_hrd_flag = nr->ReadBit();
	}

	pInfo->vui.pic_struct_present_flag = nr->ReadBit();
	pInfo->vui.bitstream_restriction_flag = nr->ReadBit();
	if (pInfo->vui.bitstream_restriction_flag)
	{
		pInfo->vui.motion_vectors_over_pic_boundaries_flag = nr->ReadBit();
		pInfo->vui.max_bytes_per_pic_denom = nr->ue();
		pInfo->vui.max_bits_per_mb_denom = nr->ue();
		pInfo->vui.log2_max_mv_length_horizontal = nr->ue();
		pInfo->vui.log2_max_mv_length_vertical = nr->ue();
		pInfo->vui.num_reorder_frames = nr->ue();
		pInfo->vui.max_dec_frame_buffering = nr->ue();
	}
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
bool parseSPS(const uint8_t * const pInOrig, const size_t inLen, /* out */ spsInfo_t *pInfo)
{
	const uint8_t *pIn = pInOrig;

	int nalHeaderLength = 0;
	int spsLen = 0;

	while (0 == spsLen || (0x7 != (0x1F & pIn[0])))
	{
		spsLen = getNalSize(pIn, inLen - (pIn-pInOrig), nalHeaderLength);

		if (0 > spsLen)
		{
			MC_ERR("error while parsing SPS (not annex-b format or couldn't find SPS)");
			return false;
		}

		pIn += nalHeaderLength;
	}

	//++pIn;
	memcpy(pInfo, pIn, 4);
	pIn += 4;

	NALReader _nr(pIn, inLen - (pIn-pInOrig));
	NALReader *nr = &_nr;

	pInfo->seq_parameter_set_id = nr->ue();

	switch (pInfo->profile_idc)
	{
	case NEXAVCProfile_HIGH:
	case NEXAVCProfile_HIGH10:
	case NEXAVCProfile_HIGH422:
	case NEXAVCProfile_HIGH444:
	case 44:
	case 83:
	case 86:
	case 118:
	case 128:
		{
			pInfo->chroma_format_idc = nr->ue();
			if (3 == pInfo->chroma_format_idc)
			{
				pInfo->separate_colour_plane_flag = nr->ReadBit();
			}

			pInfo->bit_depth_luma_minus8 = nr->ue();
			pInfo->bit_depth_chroma_minus8 = nr->ue();
			pInfo->qpprime_y_zero_transform_bypass_flag = nr->ReadBit();

			pInfo->seq_scaling_matrix_present_flag = nr->ReadBit();
			if (pInfo->seq_scaling_matrix_present_flag)
			{
				for (int i = 0; i < ((3 != pInfo->chroma_format_idc) ? 8 : 12); i++)
				{
					pInfo->scaling_list[i].seq_scaling_list_present_flag = nr->ReadBit();
					if (pInfo->scaling_list[i].seq_scaling_list_present_flag)
					{
						if (6 > i)
						{
							bool throwaway; // UseDefaultScalingMatrix4x4Flag[i]
							parseScalingList(nr, 16, &pInfo->scaling_list[i], throwaway);
						}
						else
						{
							bool throwaway; // UseDefaultScalingMatrix8x8Flag[i-6]
							parseScalingList(nr, 64, &pInfo->scaling_list[i], throwaway);
						}
					}
				}
			}
		}
		break;
	default:
		pInfo->chroma_format_idc = 1;
		break;
	}

	pInfo->log2_max_frame_num_minus4 = nr->ue();
	pInfo->pic_order_cnt_type = nr->ue();

	if (0 == pInfo->pic_order_cnt_type)
	{
		//		LOGE("%d herer\n", __LINE__);
		pInfo->log2_max_pic_order_cnt_lsb_minus4 = nr->ue();
	}
	else if (1 == pInfo->pic_order_cnt_type)
	{
		//		LOGE("%d herer\n", __LINE__);
		pInfo->delta_pic_order_always_zero_flag = nr->ReadBit();
		pInfo->offset_for_non_ref_pic = nr->se();
		pInfo->offset_for_top_to_bottom_field = nr->se();
		pInfo->num_ref_frames_in_pic_order_cnt_cycle = nr->ue();
		//info->offset_for_ref_frame = (int*)malloc(sizeof(unsigned) * info->num_ref_frames_in_pic_order_cnt_cycle);
		for (unsigned int i = 0; i < pInfo->num_ref_frames_in_pic_order_cnt_cycle; ++i)
		{
			pInfo->offset_for_ref_frame[i] = nr->se();
		}
	}
	pInfo->max_num_ref_frames = nr->ue();
	pInfo->gaps_in_frame_num_value_allowed_flag = nr->ReadBit();
	pInfo->pic_width_in_mbs_minus1 = nr->ue();
	pInfo->pic_height_in_map_units_minus1 = nr->ue();

	pInfo->frame_mbs_only_flag = nr->ReadBit();
	if (!pInfo->frame_mbs_only_flag)
	{
		//		LOGE("%d herer\n", __LINE__);
		pInfo->mb_adaptive_frame_field_flag = nr->ReadBit();
	}

	pInfo->direct_8x8_inference_flag = nr->ReadBit();
	pInfo->frame_cropping_flag = nr->ReadBit();

	if (pInfo->frame_cropping_flag)
	{
		//		LOGE("%d herer\n", __LINE__);
		pInfo->frame_crop_left_offset = nr->ue();
		pInfo->frame_crop_right_offset = nr->ue();
		pInfo->frame_crop_top_offset = nr->ue();
		pInfo->frame_crop_bottom_offset = nr->ue();
	}

	pInfo->vui_parameters_present_flag = nr->ReadBit();

	if (pInfo->vui_parameters_present_flag)
	{
		parseVUI(nr, pInfo);
	}

	return true;
}

void writeScalingList(NALWriter *nw, spsInfo_t::scaling_list_struct *pScalingList)
{
	for (size_t j = 0; j < pScalingList->numDeltaScales; ++j)
	{
		nw->Write_se(pScalingList->delta_scale[j]);
	}
}

void writeHRDParams(NALWriter *nw, spsInfo_t *pInfo)
{
	nw->Write_ue(pInfo->vui.hrd.cpb_cnt_minus1);
	nw->WriteValue(pInfo->vui.hrd.bit_rate_scale, 4);
	nw->WriteValue(pInfo->vui.hrd.cpb_size_scale, 4);

	for (size_t SchedSelIdx = 0; SchedSelIdx <= pInfo->vui.hrd.cpb_cnt_minus1; ++SchedSelIdx)
	{
		nw->Write_ue(pInfo->vui.hrd.bit_rate_value_minus1[SchedSelIdx]);
		nw->Write_ue(pInfo->vui.hrd.cpb_size_value_minus1[SchedSelIdx]);
		nw->WriteBit(pInfo->vui.hrd.cbr_flag[SchedSelIdx]);
	}
	nw->WriteValue(pInfo->vui.hrd.initial_cpb_removal_delay_length_minus1, 5);
	nw->WriteValue(pInfo->vui.hrd.cpb_removal_delay_length_minus1, 5);
	nw->WriteValue(pInfo->vui.hrd.dpb_output_delay_length_minus1, 5);
	nw->WriteValue(pInfo->vui.hrd.time_offset_length, 5);
}

void writeVUI(NALWriter *nw, spsInfo_t *pInfo)
{
	nw->WriteBit(pInfo->vui.aspect_ratio_info_present_flag);
	if (pInfo->vui.aspect_ratio_info_present_flag)
	{
		nw->WriteValue(pInfo->vui.aspect_ratio_idc, 8);
		if (255 /*Extended_SAR*/ == pInfo->vui.aspect_ratio_idc)
		{
			nw->WriteValue(pInfo->vui.sar_width, 16);
			nw->WriteValue(pInfo->vui.sar_height, 16);
		}
	}
	nw->WriteBit(pInfo->vui.overscan_info_present_flag);
	if (pInfo->vui.overscan_info_present_flag)
	{
		nw->WriteBit(pInfo->vui.overscan_appropriate_flag);
	}

	nw->WriteBit(pInfo->vui.video_signal_type_present_flag);
	if (pInfo->vui.video_signal_type_present_flag)
	{
		nw->WriteValue(pInfo->vui.video_format, 3);
		nw->WriteBit(pInfo->vui.video_full_range_flag);
		nw->WriteBit(pInfo->vui.colour_description_present_flag);
		if (pInfo->vui.colour_description_present_flag)
		{
			nw->WriteValue(pInfo->vui.colour_primaries, 8);
			nw->WriteValue(pInfo->vui.transfer_characteristics, 8);
			nw->WriteValue(pInfo->vui.matrix_coefficients, 8);
		}
	}

	nw->WriteBit(pInfo->vui.chroma_loc_info_present_flag);
	if (pInfo->vui.chroma_loc_info_present_flag)
	{
		nw->Write_ue(pInfo->vui.chroma_sample_loc_type_top_field);
		nw->Write_ue(pInfo->vui.chroma_sample_loc_type_bottom_field);
	}

	nw->WriteBit(pInfo->vui.timing_info_present_flag);
	if (pInfo->vui.timing_info_present_flag)
	{
		nw->WriteValue(pInfo->vui.num_units_in_tick, 32);
		nw->WriteValue(pInfo->vui.time_scale, 32);
		nw->WriteBit(pInfo->vui.fixed_frame_rate_flag);
	}

	nw->WriteBit(pInfo->vui.nal_hrd_parameters_present_flag);
	if (pInfo->vui.nal_hrd_parameters_present_flag)
	{
		writeHRDParams(nw, pInfo);
	}

	nw->WriteBit(pInfo->vui.vcl_hrd_parameters_present_flag);
	if (pInfo->vui.vcl_hrd_parameters_present_flag)
	{
		writeHRDParams(nw, pInfo);
	}

	if (pInfo->vui.nal_hrd_parameters_present_flag || pInfo->vui.vcl_hrd_parameters_present_flag)
	{
		nw->WriteBit(pInfo->vui.low_delay_hrd_flag);
	}

	nw->WriteBit(pInfo->vui.pic_struct_present_flag);
	nw->WriteBit(pInfo->vui.bitstream_restriction_flag);
	if (pInfo->vui.bitstream_restriction_flag)
	{
		nw->WriteBit(pInfo->vui.motion_vectors_over_pic_boundaries_flag);
		nw->Write_ue(pInfo->vui.max_bytes_per_pic_denom);
		nw->Write_ue(pInfo->vui.max_bits_per_mb_denom);
		nw->Write_ue(pInfo->vui.log2_max_mv_length_horizontal);
		nw->Write_ue(pInfo->vui.log2_max_mv_length_vertical);
		nw->Write_ue(pInfo->vui.num_reorder_frames);
		nw->Write_ue(pInfo->vui.max_dec_frame_buffering);
	}
}

void write_rbsp_trailing_bits(NALWriter *nw)
{
	nw->WriteBit(true);
	if (0 != nw->bitOffset)
	{
		nw->WriteBits(false, 8 - nw->bitOffset);
	}
}

size_t writeSPS(uint8_t *target, spsInfo_t *pInfo, bool raw)
{
	NALWriter _nw(target);
	NALWriter *nw = &_nw;

	if (raw)
	{
		nw->pOut += 2;
	}
	else
	{
		*(uint32_t *)nw->pOut = 0x1; // nal start code
		nw->pOut += 4;
	}

	memcpy(nw->pOut, pInfo, 4); // nal unit info, profile, constraint set, level
	nw->pOut += 4;

	nw->Write_ue(pInfo->seq_parameter_set_id);

	switch (pInfo->profile_idc)
	{
	case NEXAVCProfile_HIGH:
	case NEXAVCProfile_HIGH10:
	case NEXAVCProfile_HIGH422:
	case NEXAVCProfile_HIGH444:
	case 44:
	case 83:
	case 86:
	case 118:
	case 128:
		{
			nw->Write_ue(pInfo->chroma_format_idc);
			if (3 == pInfo->chroma_format_idc)
			{
				nw->WriteBit(pInfo->separate_colour_plane_flag);
			}
			nw->Write_ue(pInfo->bit_depth_luma_minus8);
			nw->Write_ue(pInfo->bit_depth_chroma_minus8);

			nw->WriteBit(pInfo->qpprime_y_zero_transform_bypass_flag);

			nw->WriteBit(pInfo->seq_scaling_matrix_present_flag);
			if (pInfo->seq_scaling_matrix_present_flag)
			{
				for (size_t i = 0; i < ((3 != pInfo->chroma_format_idc) ? 8 : 12); ++i)
				{
					nw->WriteBit(pInfo->scaling_list[i].seq_scaling_list_present_flag);
					if (pInfo->scaling_list[i].seq_scaling_list_present_flag)
					{
						if (6 > i)
						{
							writeScalingList(nw, &pInfo->scaling_list[i]);
						}
						else
						{
							writeScalingList(nw, &pInfo->scaling_list[i]);
						}
					}

				}

			}
		}
		break;
	default:
		break;
	}

	nw->Write_ue(pInfo->log2_max_frame_num_minus4);
	nw->Write_ue(pInfo->pic_order_cnt_type);
	if (0 == pInfo->pic_order_cnt_type)
	{
		nw->Write_ue(pInfo->log2_max_pic_order_cnt_lsb_minus4);
	}
	else if (1 == pInfo->pic_order_cnt_type)
	{
		nw->WriteBit(pInfo->delta_pic_order_always_zero_flag);
		nw->Write_se(pInfo->offset_for_non_ref_pic);
		nw->Write_se(pInfo->offset_for_top_to_bottom_field);
		nw->Write_ue(pInfo->num_ref_frames_in_pic_order_cnt_cycle);
		for (size_t i = 0; i < pInfo->num_ref_frames_in_pic_order_cnt_cycle; ++i)
		{
			nw->Write_se(pInfo->offset_for_ref_frame[i]);
		}
	}

	nw->Write_ue(pInfo->max_num_ref_frames);
	nw->WriteBit(pInfo->gaps_in_frame_num_value_allowed_flag);

	nw->Write_ue(pInfo->pic_width_in_mbs_minus1);
	nw->Write_ue(pInfo->pic_height_in_map_units_minus1);

	nw->WriteBit(pInfo->frame_mbs_only_flag);
	if (!pInfo->frame_mbs_only_flag)
	{
		nw->WriteBit(pInfo->mb_adaptive_frame_field_flag);
	}

	nw->WriteBit(pInfo->direct_8x8_inference_flag);

	nw->WriteBit(pInfo->frame_cropping_flag);
	if (pInfo->frame_cropping_flag)
	{
		nw->Write_ue(pInfo->frame_crop_left_offset);
		nw->Write_ue(pInfo->frame_crop_right_offset);
		nw->Write_ue(pInfo->frame_crop_top_offset);
		nw->Write_ue(pInfo->frame_crop_bottom_offset);
	}

	nw->WriteBit(pInfo->vui_parameters_present_flag);
	if (pInfo->vui_parameters_present_flag)
	{
		writeVUI(nw, pInfo);
	}

	write_rbsp_trailing_bits(nw);

	size_t bytesWritten = nw->pOut - target;
	if (raw)
	{
		uint16_t nalLength = bytesWritten - 2;
		target[0] = nalLength >> 8;
		target[1] = 0xFF & nalLength;
	}

	return bytesWritten;
}

int TranslatedOMXProfile(int OMXProfile)
{
	switch (OMXProfile)
	{
	case 0x1: /* AVCProfileBaseline */
		return NEXAVCProfile_BASELINE;
	case 0x2: /* AVCProfileMain */
		return NEXAVCProfile_MAIN;
	case 0x4: /* AVCProfileExtended */
		return NEXAVCProfile_EXTENDED;
	case 0x8: /* AVCProfileHigh */
		return NEXAVCProfile_HIGH;
	case 0x10: /* AVCProfileHigh10 */
		return NEXAVCProfile_HIGH10;
	case 0x20: /* AVCProfileHigh422 */
		return NEXAVCProfile_HIGH422;
	case 0x40: /* AVCProfileHigh444 */
		return NEXAVCProfile_HIGH444;
	default:
		return 0;
	}
}

int TranslatedOMXLevel(int OMXLevel)
{
	switch (OMXLevel)
	{
	case 0x1: /* AVCLevel1 */
		return 100;
	case 0x2: /* AVCLevel1b */
		return 105;
	case 0x4: /* AVCLevel11 */
		return 110;
	case 0x8: /* AVCLevel12 */
		return 120;
	case 0x10: /* AVCLevel13 */
		return 130;
	case 0x20: /* AVCLevel2 */
		return 200;
	case 0x40: /* AVCLevel21 */
		return 210;
	case 0x80: /* AVCLevel22 */
		return 220;
	case 0x100: /* AVCLevel3 */
		return 300;
	case 0x200: /* AVCLevel31 */
		return 310;
	case 0x400: /* AVCLevel32 */
		return 320;
	case 0x800: /* AVCLevel4 */
		return 400;
	case 0x1000: /* AVCLevel41 */
		return 410;
	case 0x2000: /* AVCLevel42 */
		return 420;
	case 0x4000: /* AVCLevel5 */
		return 500;
	case 0x8000: /* AVCLevel51 */
		return 510;
	case 0x10000: /* AVCLevel52 */
		return 520;
	default:
		return 0;
	}
}

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
#define CODEC_NAME "H264D"

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

int TranslatedSPSInfoLevel(Common::spsInfo_t *info)
{
	if ((NEXAVCProfile_BASELINE == info->profile_idc || NEXAVCProfile_MAIN == info->profile_idc || NEXAVCProfile_EXTENDED == info->profile_idc)
		&& 11 == info->level_idc && info->constraints.set3_flag)
	{
		return 105;
	}

	return info->level_idc * 10;
}

bool LevelSupportsContentAtFPS(int level, Common::spsInfo_t *info, float fps)
{
	int maxFrameSizeInMBs = 0;
	int maxMBsPerSecond = 0;

	int contentFrameSizeInMBs = (info->pic_width_in_mbs_minus1 + 1) * (info->pic_height_in_map_units_minus1 + 1);

	switch (level)
	{
	case 100: /* AVCLevel1 */
	case 105: /* AVCLevel1b */
		maxFrameSizeInMBs = 99;
		maxMBsPerSecond = 1485;
		break;
	case 110: /* AVCLevel11 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 3000;
		break;
	case 120: /* AVCLevel12 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 6000;
		break;
	case 130: /* AVCLevel13 */
	case 200: /* AVCLevel2 */
		maxFrameSizeInMBs = 396;
		maxMBsPerSecond = 11880;
		break;
	case 210: /* AVCLevel21 */
		maxFrameSizeInMBs = 792;
		maxMBsPerSecond = 19800;
		break;
	case 220: /* AVCLevel22 */
		maxFrameSizeInMBs = 1620;
		maxMBsPerSecond = 20250;
		break;
	case 300: /* AVCLevel3 */
		maxFrameSizeInMBs = 1620;
		maxMBsPerSecond = 40500;
		break;
	case 310: /* AVCLevel31 */
		maxFrameSizeInMBs = 3600;
		maxMBsPerSecond = 108000;
		break;
	case 320: /* AVCLevel32 */
		maxFrameSizeInMBs = 5120;
		maxMBsPerSecond = 216000;
		break;
	case 400: /* AVCLevel4 */
	case 410: /* AVCLevel41 */
		maxFrameSizeInMBs = 8192;
		maxMBsPerSecond = 245760;
		break;
	case 420: /* AVCLevel42 */
		maxFrameSizeInMBs = 8704;
		maxMBsPerSecond = 522240;
		break;
	case 500: /* AVCLevel5 */
		maxFrameSizeInMBs = 22080;
		maxMBsPerSecond = 589824;
		break;
	case 510: /* AVCLevel51 */
		maxFrameSizeInMBs = 36864;
		maxMBsPerSecond = 983040;
		break;
	case 520: /* AVCLevel52 */
		maxFrameSizeInMBs = 36864;
		maxMBsPerSecond = 2073600;
		break;
	default:
		break;
	}

	if (contentFrameSizeInMBs > maxFrameSizeInMBs)
	{
		return false;
	}

	if (contentFrameSizeInMBs > (maxMBsPerSecond / fps))
	{
		return false;
	}

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

namespace { // (anotheranon1)
	using Common::TranslatedOMXProfile;
	using Common::TranslatedOMXLevel;
}

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

	case NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM:
		*puValue = NEXCAL_PROPERTY_AVC_SUPPORT_TO_INCLUDE_CONFIGSTREAM_OK;
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
	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
	case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL:
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
				MC_ERR("couldn't find hw H264 decoder");
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

	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
		{
			int highestProfile = 0;

			for (int i = 0; i < g_profileLevels[0]; ++i)
			{
				highestProfile = MAX(highestProfile, TranslatedOMXProfile(g_profileLevels[i*2 + 1]));
			}

			if (0 == highestProfile)
			{
				highestProfile = NEXAVCProfile_BASELINE;
			}

			*puValue = (NXINT64)highestProfile;
		}
		break;

	case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_BASELINE;
		}
		break;
	case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_MAIN;
		}
		break;
	case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_EXTENDED;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH10;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH422;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH444;
		}
		break;
	}

	if (isLevelCheck)
	{
		int highestLevel = 51;

		for (int i = 0; i < g_profileLevels[0]; ++i)
		{
			if (profileForLevelCheck == TranslatedOMXProfile(g_profileLevels[i*2 + 1]))
			{
				highestLevel = MAX(highestLevel, (TranslatedOMXLevel(g_profileLevels[i*2 + 2])/10));
			}
		}

		*puValue = (NXINT64)highestLevel;
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 GetPropertySW( NXUINT32 uProperty, NXINT64 *puValue, NXVOID *pUserData )
{
	IDENTITY("+", CODEC_NAME, pUserData, "prop(0x%X)", uProperty);

	// call common GetProperty
	NXINT32 retValue = GetPropertyCommon(uProperty, puValue, pUserData);

	//process separate part
	NXINT64 input = *puValue;

	switch ( uProperty )
	{
	case NEXCAL_PROPERTY_COMPONENT_NAME:
	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
	case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL:
		{
			NexMediaCodec::CodecClassRequest req = NexMediaCodec::ANY_SW;
			if (Utils::Manufacturer::SAMSUNG == Utils::GetManufacturer())
			{
				req = NexMediaCodec::GOOGLE_SW;
			}
			
			pthread_mutex_lock(&g_preferredDecoderMutex);
			if (!g_checkedForPreferredSWDecoder)
			{
				int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, req, g_preferredSWDecoderName, &g_swProfileLevels, &g_swSupportedFeature);
				g_checkedForPreferredSWDecoder = true;
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
			if (NULL == g_preferredSWDecoderName)
			{
				MC_ERR("couldn't find sw H264 decoder");
				*puValue = (NXINT64)"not found";
			}
			else
			{
				*puValue = (NXINT64)g_preferredSWDecoderName;
			}
		}
		break;

	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
		{
			int highestProfile = 0;

			for (int i = 0; i < g_swProfileLevels[0]; ++i)
			{
				highestProfile = MAX(highestProfile, TranslatedOMXProfile(g_swProfileLevels[i*2 + 1]));
			}

			if (0 == highestProfile)
			{
				highestProfile = NEXAVCProfile_BASELINE;
			}

			*puValue = (NXINT64)highestProfile;
		}
		break;

	case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_BASELINE;
		}
		break;
	case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_MAIN;
		}
		break;
	case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_EXTENDED;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH10;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH422;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH444;
		}
		break;
	}

	if (isLevelCheck)
	{
		int highestLevel = 0;

		for (int i = 0; i < g_swProfileLevels[0]; ++i)
		{
			if (profileForLevelCheck == TranslatedOMXProfile(g_swProfileLevels[i*2 + 1]))
			{
				highestLevel = MAX(highestLevel, (TranslatedOMXLevel(g_swProfileLevels[i*2 + 2])/10));
			}
		}

		*puValue = (NXINT64)highestLevel;
	}

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertySWF = GetPropertySW;

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

		if (parseSPS(pConfig, iLen, &csd->spsInfo))
		{
			STATUS(Log::FLOW, Log::DEBUG, "interlace (%d)", 0 == csd->spsInfo.frame_mbs_only_flag ? 1 : 0);
			mc->inputFormat->setInteger("interlace", 0 == csd->spsInfo.frame_mbs_only_flag ? 1 : 0);
		}

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
		mc->vd.bInitFailed = true;
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

// TODO: refactor
NXINT32 InitSW( 
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
		, NXVOID **ppUserData  )
{
	IDENTITY("+", CODEC_NAME, *ppUserData, "eCodecType(0x%X) udtype(0x%X)", eCodecType, uUserDataType);

	unsigned int retValue = 0;

	NexCAL_mc *mc = Video::Decoder::CreateNexCAL_mc(CODEC_NAME, MIMETYPE, *piWidth, *piHeight, uUserDataType, ppUserData);

	if (NULL == mc)
	{
		MC_ERR("failed to create userData");
		return -1;
	}

	NexMediaCodec::CodecClassRequest req = NexMediaCodec::ANY_SW;
	if (Utils::Manufacturer::SAMSUNG == Utils::GetManufacturer())
	{
		req = NexMediaCodec::GOOGLE_SW;
	}
			
	pthread_mutex_lock(&g_preferredDecoderMutex);
	if (!g_checkedForPreferredSWDecoder)
	{
		int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, req, g_preferredSWDecoderName, &g_swProfileLevels, &g_swSupportedFeature);
		g_checkedForPreferredSWDecoder = true;
	}
	pthread_mutex_unlock(&g_preferredDecoderMutex);

	if (NULL == g_preferredSWDecoderName)
	{
		MC_ERR("couldn't find sw H264 decoder");
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -2);
		return -2;
	}

	mc->preferredCodecName = (const char *)g_preferredSWDecoderName;

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
				Deinit((NXVOID*)mc);
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

		if (parseSPS(pConfig, iLen, &csd->spsInfo))
		{
			mc->inputFormat->setInteger("interlace", 0 == csd->spsInfo.frame_mbs_only_flag ? 1 : 0);
		}

		void *csd0;
		csd->jobj_csd0 = Utils::JNI::NewDirectByteBuffer(iLen, csd0);
		memcpy(csd0, pConfig, iLen);

		if (Utils::ChipType::EDEN == Utils::GetChipType())
		{
			free(pConfig);
			pConfig = NULL;
		}

		mc->vd.codecSpecific = (NXVOID*)csd;
	}

	if (NULL != csd && NULL != csd->jobj_csd0)
	{
		mc->inputFormat->setByteBuffer("csd-0", csd->jobj_csd0);
	}

	mc->vd.maxInputSize = ROUNDUP(mc->vd.width, 16) * ROUNDUP(mc->vd.height, 16) * 3 / 4 + 30*1024; // similar to nvidia's formula

	*ppUserData = (NXVOID*)mc;

	retValue = Video::Decoder::Init(mc);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	*piWidth = mc->vd.width;
	*piHeight = mc->vd.height;
	*piPitch = mc->vd.pitch;

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoInit InitSWF = InitSW;

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

NXINT32 InitPR( 
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
	IDENTITY("+", CODEC_NAME, *ppUserData, "oti(0x%X) udtype(0x%X)", eCodecType, uUserDataType);

	NXINT32 retValue = 0;

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
				Deinit((NXVOID*)mc);
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

	Nex_MC::Common::MediaCryptoInfo *mci = (Nex_MC::Common::MediaCryptoInfo *)pConfigEnhance;

	int ret = 0;
	ret = NexMediaCodec_using_jni::createMediaCrypto(mci->mostSignificantBits, mci->leastSignificantBits, mci->initData, mci->initDataLength, mc->vd.crypto);
	if (0 != ret)
	{
		MC_ERR("exception occurred while creating media crypto (%d)", ret);
		return -2;
	}
	ret = NexMediaCodec_using_jni::createCryptoInfo(mc->vd.cryptoInfo);
	if (0 != ret)
	{
		MC_ERR("exception occurred while creating crypto info (%d)", ret);
		return -3;
	}

	*ppUserData = (NXVOID*)mc;

	retValue = Video::Decoder::Init(mc);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID*)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
	}

	*piWidth = mc->vd.width;
	*piHeight = mc->vd.height;
	*piPitch = mc->vd.pitch;

	IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
	return retValue;
}
NEXCALVideoInit InitPRF = InitPR;

NXINT32 DecodePR( 
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
					NEXCAL_INIT_VENC_RET(*puDecodeResult);
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

	retValue = Video::Decoder::DecodePR(pData, iLen, uDTS, uPTS, nFlag, puDecodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) decRet(0x%X)", retValue, *puDecodeResult);
	return retValue;
}
NEXCALVideoDecode DecodePRF = DecodePR;

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

					if (!parseSPS(pIn, nalLen + nalHeaderLength, &info))
					{
						MC_ERR("parseSPS failed!");
						ret = NEXCAL_ERROR_INVALID_PARAMETER;
						break;
					}

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

// TODO: refactor
int QueryForSupportSW(uint8_t *dsi, size_t dsiLen)
{
	IDENTITY("+", CODEC_NAME, 0, "dsi(%p) dsiLen(%zu)", dsi, dsiLen);
	int ret = NEXCAL_ERROR_NOT_SUPPORT_DEVICE;

	NexMediaCodec::CodecClassRequest req = NexMediaCodec::ANY_SW;
	if (Utils::Manufacturer::SAMSUNG == Utils::GetManufacturer())
	{
		req = NexMediaCodec::GOOGLE_SW;
	}

	pthread_mutex_lock(&g_preferredDecoderMutex);
	if (!g_checkedForPreferredSWDecoder)
	{
		int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, false, req, g_preferredSWDecoderName, &g_swProfileLevels, &g_swSupportedFeature);
		g_checkedForPreferredSWDecoder = true;
	}
	pthread_mutex_unlock(&g_preferredDecoderMutex);

	if (NULL == g_swProfileLevels)
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

					if (!parseSPS(pIn, nalLen + nalHeaderLength, &info))
					{
						MC_ERR("parseSPS failed!");
						ret = NEXCAL_ERROR_INVALID_PARAMETER;
						break;
					}

					for (int i = 0; i < g_swProfileLevels[0]; ++i)
					{
						int deviceLevel = TranslatedOMXLevel(g_swProfileLevels[i*2 + 2]);
						if (deviceLevel >= TranslatedSPSInfoLevel(&info) || LevelSupportsContentAtFPS(deviceLevel, &info, 14.9)) // 14.9 for ~15fps
						{
							int deviceProfile = TranslatedOMXProfile(g_swProfileLevels[i*2 + 1]);

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
#define CODEC_NAME "H264E"

using Nex_MC::Video::Encoder::NexCAL_mcve;


// returns length of altered dsi
size_t setCropToAchieveResolution(uint8_t *dsi_in_out, uint32_t maxLength, size_t reqWidth, size_t reqHeight)
{
	IDENTITY("+", CODEC_NAME, 0, "dsi(%p) maxLen(%u) req_WxH(%zux%zu)", dsi_in_out, maxLength, reqWidth, reqHeight);
	MEM_DUMP(Log::INPUT, Log::VERBOSE, dsi_in_out, maxLength, maxLength);

	uint8_t dsi_in_annexb_4byte[256] = { 0, };
	int iNewLen = Common::convertRawToAnnexB_4byte_config(dsi_in_annexb_4byte, 256, dsi_in_out, 150);


	uint8_t *pIn = dsi_in_annexb_4byte;
	int nalHeaderLength = 0;

	short numSPS = 0;
	short numPPS = 0;


	int nalLen = 0;

	size_t bytesWritten = 1; // skip first byte for numSPS
	size_t PPS_startPos = 0;

	while (pIn - dsi_in_annexb_4byte < iNewLen)
	{
		nalLen = Common::getNalSize(pIn, iNewLen - (pIn - dsi_in_annexb_4byte), nalHeaderLength);

		if (0 > nalLen && 0 == numSPS)
		{
			MC_ERR("error while parsing SPS (not annex-b format or couldn't find SPS)");
			IDENTITY("-", CODEC_NAME, 0, "ret(%u)", 0);
			return 0;
		}


		switch ((0x1F & pIn[nalHeaderLength]))
		{
		case 0x7:
			{
				if (0 != numPPS)
				{
					MC_WARN("found sps after pps!");
				}

				Common::spsInfo_t info;
				memset(&info, 0x00, sizeof(Common::spsInfo_t));

				if (!parseSPS(pIn, nalLen + nalHeaderLength, &info))
				{
					MC_ERR("parseSPS failed!");
					IDENTITY("-", CODEC_NAME, 0, "ret(%u)", 0);
					return 0;
				}

				size_t codedWidth = (info.pic_width_in_mbs_minus1 + 1) * 16;
				size_t codedHeight = (info.pic_height_in_map_units_minus1 + 1) * 16;
				if (!info.frame_mbs_only_flag)
				{
					codedHeight <<= 1;
				}

				if (codedWidth < reqWidth || codedHeight < reqHeight)
				{
					MC_ERR("original WxH (%zux%zu) < requested WxH (%zux%zu)", codedWidth, codedHeight, reqWidth, reqHeight);
					IDENTITY("-", CODEC_NAME, 0, "ret(%u)", 0);
					return 0;
				}

				size_t necessaryCropWidth = codedWidth - reqWidth;
				size_t necessaryCropHeight = codedHeight - reqHeight;

				Nex_MC::Utils::ue_t chromaArrayType = 0;
				if (0 == info.separate_colour_plane_flag)
				{
					chromaArrayType = info.chroma_format_idc;
				}

				int cropUnitX = 0;
				int cropUnitY = 0;

				if (0 == chromaArrayType)
				{
					cropUnitX = 1;
					cropUnitY = 2 - info.frame_mbs_only_flag;
				}
				else
				{
					int subWidthC = 0;
					int subHeightC = 0;
					if (0 == info.separate_colour_plane_flag)
					{
						switch (info.chroma_format_idc)
						{
						case 1:
							subWidthC = 2;
							subHeightC = 2;
							break;
						case 2:
							subWidthC = 2;
							subHeightC = 1;
							break;
						case 3:
							subWidthC = 1;
							subHeightC = 1;
							break;
						default:
							break;
						}
					}
					cropUnitX = subWidthC;
					cropUnitY = subHeightC * (2 - info.frame_mbs_only_flag);
				}

				info.frame_crop_left_offset = 0;
				info.frame_crop_right_offset = necessaryCropWidth / cropUnitX;
				info.frame_crop_top_offset = 0;
				info.frame_crop_bottom_offset = necessaryCropHeight / cropUnitY;

				if (0 < info.frame_crop_left_offset || 0 < info.frame_crop_right_offset || 0 < info.frame_crop_top_offset || 0 < info.frame_crop_bottom_offset)
				{
					info.frame_cropping_flag = true;
				}

				bytesWritten += writeSPS(dsi_in_out + bytesWritten, &info, true);

				++numSPS;
			}
			break;
		case 0x8:
			{
				if (0 == numSPS)
				{
					MC_WARN("found pps before sps!");
				}

				if (0 == numPPS)
				{
					PPS_startPos = bytesWritten++; // ++ for writing numPPS
				}

				uint16_t ppsLen = nalLen;
				dsi_in_out[bytesWritten++] = ppsLen >> 8;
				dsi_in_out[bytesWritten++] = 0xFF & ppsLen;

				memcpy(dsi_in_out + bytesWritten, pIn + nalHeaderLength, nalLen);
				bytesWritten += nalLen;

				++numPPS;
			}
			break;
		default:
			break;
		}

		pIn += nalHeaderLength + nalLen;
	}

	dsi_in_out[0] = numSPS;
	dsi_in_out[PPS_startPos] = numPPS;

	MEM_DUMP(Log::OUTPUT, Log::VERBOSE, dsi_in_out, bytesWritten, bytesWritten);

	IDENTITY("-", CODEC_NAME, 0, "ret(%zu)", bytesWritten);
	return bytesWritten;
}


namespace { // (anon101)
	using Nex_MC::Utils::WrapSetProperty;
	
	pthread_mutex_t g_preferredEncoderMutex;
	char *g_preferredEncoderName = NULL;
	bool g_checkedForPreferredEncoder = false;
	int *g_supportedColorFormats = NULL;

	int *g_encoderProfileLevels = NULL;

	int g_widthAlignmentForEncode = 0;
	int g_heightAlignmentForEncode = 0;

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
			g_widthAlignmentForEncode = 0;
			g_heightAlignmentForEncode = 0;
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

namespace { // (anotheranon2)
	using Common::TranslatedOMXProfile;
	using Common::TranslatedOMXLevel;
}

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
	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
	case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL:
	case NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL:
		{
			pthread_mutex_lock(&g_preferredEncoderMutex);
			if (!g_checkedForPreferredEncoder)
			{
				int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName, &g_encoderProfileLevels, NULL, &g_widthAlignmentForEncode, &g_heightAlignmentForEncode);
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

	case NEXCAL_PROPERTY_AVC_SUPPORT_PROFILE:
		{
			int highestProfile = 0;

			for (int i = 0; i < g_encoderProfileLevels[0]; ++i)
			{
				highestProfile = MAX(highestProfile, TranslatedOMXProfile(g_encoderProfileLevels[i*2 + 1]));
			}

			if (0 == highestProfile)
			{
				highestProfile = NEXAVCProfile_BASELINE;
			}

			*puValue = highestProfile;
		}
		break;

	case NEXCAL_PROPERTY_AVC_BASELINE_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_BASELINE;
		}
		break;
	case NEXCAL_PROPERTY_AVC_MAIN_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_MAIN;
		}
		break;
	case NEXCAL_PROPERTY_AVC_EXTENDED_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_EXTENDED;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH10_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH10;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH422_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH422;
		}
		break;
	case NEXCAL_PROPERTY_AVC_HIGH444_SUPPORT_LEVEL:
		{
			isLevelCheck = true;
			profileForLevelCheck = NEXAVCProfile_HIGH444;
		}
		break;
	}

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

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) value(%lld)", retValue, *puValue);
	return retValue;
}
NEXCALGetProperty GetPropertyF = GetProperty;

NXINT32 GetTranslatedOMXProfileValue(NXINT32 iValue)
{
	NXINT32 iRet = 0x01;
	switch (iValue)
	{
		case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_BASELINE:
			iRet = 0x01/*AVCProfileBaseline*/;
			break;
		case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_MAIN:
			iRet = 0x02/*AVCProfileMain*/;
			break;
		case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_EXTENDED:
			iRet = 0x04/*AVCProfileExtended*/;
			break;
		case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH:
			iRet = 0x08/*AVCProfileHigh*/;
			break;
		case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH10:
			iRet = 0x10/*AVCProfileHigh10*/;
			break;
		case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH422:
			iRet = 0x20/*AVCProfileHigh422*/;
			break;
		case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH444:
			iRet = 0x40/*AVCProfileHigh444*/;
			break;
	}

	return iRet;
}

static NXINT32 _SetPropertyReal(NXUINT32 a_uProperty, NXINT64 a_qValue, NXVOID *a_pUserData)
{
	NexCAL_mc *mc = (NexCAL_mc *)a_pUserData;

	switch ( a_uProperty )
	{
#ifdef FOR_PROJECT_LGE
	case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE:
		if ( a_qValue == NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_BASELINE )
			mc->ve.profile = 0;
		else if ( a_qValue == NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE_VAULE_IS_HIGH )
			mc->ve.profile = 1; //AVCProfileHigh
		break;
#else
	case NEXCAL_PROPERTY_AVC_ENCODER_SET_PROFILE:
			mc->ve.profile = GetTranslatedOMXProfileValue((NXINT32)a_qValue);
		break;

	case NEXCAL_PROPERTY_AVC_ENCODER_SET_LEVEL:
			mc->ve.level = (NXINT32)a_qValue;
		break;
#endif
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
		, NXINT32 a_nPitch
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
			int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName, &g_encoderProfileLevels, NULL, &g_widthAlignmentForEncode, &g_heightAlignmentForEncode);
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

	if (23 <= Nex_MC::Utils::GetBuildVersionSDK() && 0 < g_widthAlignmentForEncode && 0 < g_heightAlignmentForEncode)
	{
		mc->ve.width = (( iWidth + (g_widthAlignmentForEncode-1)) & ~(g_widthAlignmentForEncode-1));
		mc->ve.height = (( iHeight + (g_heightAlignmentForEncode-1)) & ~(g_heightAlignmentForEncode-1));
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

#ifdef FOR_PROJECT_LGE
		MC_WARN("FOR_PROJECT_LGE mc->ve.profile = 0x%x, mc->ve.level=0x%x\n",mc->ve.profile, mc->ve.level );
#else
	if ( backup_profile == NEXSAL_INFINITE && backup_level == NEXSAL_INFINITE )
	{
		backup_profile = mc->ve.profile;
		backup_level = mc->ve.level;

		if ( g_encoderProfileLevels && (mc->ve.profile==0 || mc->ve.level==0) )
		{
			//in case that profile&level is not set
			if (mc->ve.profile == 0)
			{
				mc->ve.profile = 0x1 /*AVCProfileBaseline*/;
				mc->ve.level = 0;
			}

			for (int i = 0; i < g_encoderProfileLevels[0]; ++i)
			{
				if (g_encoderProfileLevels[i*2+1] == mc->ve.profile)
				{
					mc->ve.level = g_encoderProfileLevels[i*2+2];
				}
			}
			MC_WARN("change to mc->ve.profile = 0x%x->0x%x, mc->ve.level = 0x%x->0x%x\n", backup_profile, mc->ve.profile, backup_level, mc->ve.level );
		}
	}
	else
	{
		mc->ve.profile = backup_profile;
		mc->ve.level = backup_level;
	}
#endif
	
	g_wrapSetProperty.RegisterUserData( *ppUserData, (NXVOID *)mc  );
	
	*ppUserData = (NXVOID *)mc;

	retValue = Video::Encoder::Init(mc);

	if (0 != retValue)
	{
		if ( backup_profile == 0 ||  backup_level == 0 )
		{
			MC_WARN("video encoder init fail. retry 0x01/0x200. - ret(0x%X)", retValue);
			Video::Encoder::Deinit(mc);
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

NXINT32 InitUsingFrameData(
		NEX_CODEC_TYPE eCodecType
		, NXUINT8** ppConfig
		, NXINT32 * piConfigLen
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

	NexCAL_mc *mc = (NexCAL_mc *)malloc(sizeof(NexCAL_mc));
	if (NULL == mc)
	{
		MC_ERR("malloc failed!");
		*ppUserData = 0;
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", -1);
		return -1;
	}
	STATUS(Log::FLOW, Log::INFO, "%p = malloc(%d)", mc, sizeof(NexCAL_mc));

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

	{
		pthread_mutex_lock(&g_preferredEncoderMutex);
		if (!g_checkedForPreferredEncoder)
		{
			int ret = NexMediaCodec_using_jni::findPreferredCodec(MIMETYPE, true, NexMediaCodec::ANY, g_preferredEncoderName, &g_encoderProfileLevels, NULL, &g_widthAlignmentForEncode, &g_heightAlignmentForEncode);
			g_checkedForPreferredEncoder = true;
		}
		pthread_mutex_unlock(&g_preferredEncoderMutex);

		mc->preferredCodecName = (const char *)g_preferredEncoderName;

		if (23 <= Nex_MC::Utils::GetBuildVersionSDK() && 0 < g_widthAlignmentForEncode && 0 < g_heightAlignmentForEncode)
		{
			mc->ve.width = (( iWidth + (g_widthAlignmentForEncode-1)) & ~(g_widthAlignmentForEncode-1));
			mc->ve.height = (( iHeight + (g_heightAlignmentForEncode-1)) & ~(g_heightAlignmentForEncode-1));
		}

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
				MC_WARN("Exynos : decidng colorformat");

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

	// MC_INFO("color selected: %d", mc->ve.colorFormat);

	//mc->ve.writeOutputFrame = WriteOutputFrame;

	g_wrapSetProperty.RegisterUserData( *ppUserData, (NXVOID *)mc  );

	*ppUserData = (NXVOID *)mc;

	retValue = Video::Encoder::Init(mc);

	if (0 != retValue)
	{
		*ppUserData = 0;
		Deinit((NXVOID *)mc);
		IDENTITY("-", CODEC_NAME, *ppUserData, "ret(0x%X)", retValue);
		return retValue;
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
NEXCALVideoEncoderInit InitUsingFrameDataF = InitUsingFrameData;

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

NXINT32 EncodeUsingFrameData( 
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

	retValue = Video::Encoder::EncodeUsingFrameData(pData1, pData2, pData3, uPTS, puEncodeResult, pUserData);

	IDENTITY("-", CODEC_NAME, pUserData, "ret(0x%X) encRet(0x%X)", retValue, *puEncodeResult);
	return retValue;
}
NEXCALVideoEncoderEncode EncodeUsingFrameDataF = EncodeUsingFrameData;

} // namespace Encoder

} // namespace H264

} // namespace Nex_MC

