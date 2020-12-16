/******************************************************************************
 * File Name   : NexVideoChunkParser.c
 * Description : Common media definition.
 *******************************************************************************
 
	  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	  PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2010-2014 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#include <string.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

#include "NexChunkParser.h"
#include "NexCodecUtil.h"
#include "Queue.h"

#define VCP_LOG 3

#define _DEFAULT_VIDEO_FRAME_SIZE_	(10*1024*1024)	// 10MB
#define VIDEOCHUNK_MEMORY_LIMIT 	(100*1024*1024)	// 100MB
#define DUMMY_INTERVAL 33
#define MIN_REF_NUM 16
#define MAX_REF_NUM 64
#define _MAX_POC_LSB_	2

#define AVC_NONE_IDR_CODED_SLICE 0x01
#define AVC_IDR_CODED_SLICE 0x05
#define AVC_SEI 0x06
#define AVC_SPS 0x07
#define AVC_PPS 0x08
#define AVC_AUD 0x09

#define IS_AVC_PICTURE_FRAME(x) (((x) == AVC_NONE_IDR_CODED_SLICE || (x) == AVC_IDR_CODED_SLICE) ? TRUE : FALSE)

//related to status flag
#define SET_STATUS_ZERO(v) ((v) = 0x00)
#define SET_STATUS(v, x) ((v) = ((v) | (x)))
#define DEL_STATUS(v, x) ((v) = ((v) & (~(x)))
#define CHECK_STATUS(v, x) ((v)&(x) ? TRUE : FALSE)

#define STATUS_FIELD			(0x1<<0)
#define STATUS_DISORDER_POC		(0x1<<1)


typedef enum
{
	eFRAMEFINDER_OK             = 0,
	eFRAMEFINDER_NEED_MORE      = 1,
	eFRAMEFINDER_ERROR          = 2,
} FRAMEFINDER_RETURN;

typedef enum
{
	eVIDEOCHUNK_CREATE          = 0,
	eVIDEOCHUNK_RESET           = 1,
	eVIDEOCHUNK_PUTFRAME        = 2,
	eVIDEOCHUNK_DUMMY_PUTFRAME  = 3,
	eVIDEOCHUNK_GETFRAME        = 4,
	eVIDEOCHUNK_DESTROY         = 5,
} VIDEOCHUNK_STATE;

typedef struct _CHUNK_QUEUE
{
	NXUINT32 uLen;
	NXINT64 qDTS;
	NXINT64 qPTS;
	NXINT64 qCurIDRFramePTS;
	NXINT32 nTopFOC;    
} CHUNK_QUEUE;

typedef struct _VIDEOCHUNK_INFO
{
	NEX_CODEC_TYPE eCodecType;	// Codec Type
	NXUINT32 uFourCC;			// FourCC	
	NEX_FILEFORMAT_TYPE eFFType; // File Format Type
	NXUINT32 uNALLenSize;		// Size of NAL Header Length for H.26x
	NXUINT32 uH26xFormat;		// Raw or AnnexB for H.26x
	NEXCODECUTIL_SPS_INFO stSPSInfo;	// SPS info for H.26x
	NEXCODECUTIL_PPS_INFO stPPSInfo;	// PPS info for H.26x

	NXUINT8 *pFrameData;		// Video-Frame got by parsing
	NXUINT32 uFrameSize;		// Video-Size got by parsing
	NXINT64 qFrameDTS;			// Video-DTS got by parsing
	NXINT64 qFramePTS;			// Video-PTS got by parsing

	NXUINT8 *pChunkPool;		// Chunk Memory Pool
	NXUSIZE usMaxChunkSize;		// Max Size of Chunk Memory Pool
	NXINT64 qChunkDTS;			// DTS of chunks
	NXINT64 qChunkPTS;			// PTS of chunks
	NXINT64 qPreChunkDTS;		// Previous DTS
	NXUINT32 uChunkedCount;		// Chunked count withing one chunk
	NXUINT32 uTotalChunkedCount; // Total Chunked Count
	NXFLOAT fDuration;			// Interval TS because the chunk type should complement DTS
	NXFLOAT fCalculatedDuration; // num_units_in_tick * (1 +  nuit_field_based_flag))
	NXFLOAT fAveInterval4MP4;	// In the case of MP4, average duration should be calculated with NXFLOAT
	NXINT64 qTotalDuration;		// Total duration
	NXUSIZE usCurPos;			// Offset value of current position on m_pChunkFrame
	NXUSIZE usRemainSize;		// Left size that should be copied
	NXUSIZE usRemainSizeHEVC;	// Left size that should be copied
	VIDEOCHUNK_STATE eState;	// VideoChunk State
	QUEUE *pChunkQueue;			// Chunk Queue
	NXBOOL bReset;				// Reset flag for MPEG family
	NXBOOL bFirstFrame;			// First fram flag for MPEG family

	VIDEOCHUNK_EXTINFO stExtInfo;	// Extra Infomation

	NXINT64 qCurIDRFramePTS;	// Current IDR frame PTS
	NXINT32 nIFrameCnt;			// I-frame count;
	NXUINT32 uPrevPOCLsb;		// Previous picture order count Lsb
	NXINT32 nPrevPOCMsb;		// Previous picture order count Msb
	NXINT32 nOffsetFOC;			// Offset of top or bottom field order count preparing for spec out that TOP increases continuously regardless of  IDR
	NXUINT32 uMaxPOCLsb;			// Max picture order count Lsb
	NXUINT32 auPOCList[MAX_REF_NUM];	//POC list to gain POC unit
	NXUINT32 uPOCUnit;			// multiple value of picture order count (default value: 2)
	NXUINT32 uRefNum;			// Reference frame number to gain uPocUnit
	NXBOOL bNeedPTSCal;			// Whether PTS calcualtion needs or not
	NXUINT32 uStatusFlag;		// To check status
	NXBOOL bFirstPicDone;
} VIDEOCHUNK_INFO;


static NXINT32 _FindAnnexBStartCode(NXUINT8 *a_pFrame, NXUINT32 a_uFrameLen, NXUINT32 *puStartCodeLen)
{
	NXINT32 nPos = -1;
	NXUINT32 i;

	if (a_pFrame && a_uFrameLen > 4 && puStartCodeLen)
	{
		for (i=0; i<a_uFrameLen-4; i++)
		{
			if (a_pFrame[i] == 0 && a_pFrame[i+1] == 0 && a_pFrame[i+2] == 0 && a_pFrame[i+3] == 1)
			{
				*puStartCodeLen = 4;
				nPos = i;
				break;
			}
			else if (a_pFrame[i] == 0 && a_pFrame[i+1] == 0 && a_pFrame[i+2] == 1)
			{
				*puStartCodeLen = 3;
				nPos = i;
				break;
			}
		}
	}

	return nPos;
}

static NXINT32 _GetNALLength(NXUINT8 *a_pFrame, NXUINT32 a_uFrameLen, NXUINT32 a_uNALLenSize)
{
	NXINT32 nLen = 0;

	if (a_uFrameLen <= a_uNALLenSize)
	{
		return -1;
	}

	switch (a_uNALLenSize)
	{
		case 1:
			nLen = NexCodecUtil_ReadBigEndianValue8(a_pFrame);
			break;
		case 2:
			nLen = NexCodecUtil_ReadBigEndianValue16(a_pFrame);
			break;
		case 3:
			nLen = NexCodecUtil_ReadBigEndianValue24(a_pFrame);
			break;
		case 4:
			nLen = NexCodecUtil_ReadBigEndianValue32(a_pFrame);
			break;
		default:
			return -1;
	}

	if (nLen <= 0)
	{
		return -1;
	}

	return nLen;
}

static NXVOID _DestroyData(NXVOID *a_pData)
{
	SAFE_FREE(a_pData);
}

static FRAMEFINDER_RETURN _VideoFrameFinder(VIDEOCHUNK_INFO *a_pstInfo, NXUSIZE *a_pusFrameOffset, NXUSIZE *a_pusFrameSize)
{
	NXUSIZE i = 0;
	NXUSIZE usCurPos = 0;
	NXUSIZE usStartPos = 0;
	NXUINT32 uBS;
	NXBOOL bFirstSeqHeader = TRUE;
	//NXBOOL bPicFound = FALSE;
	NXBOOL bFoundStartCode = FALSE;
	NXUINT8 *pChunkFrame = a_pstInfo->pChunkPool + a_pstInfo->usCurPos;

	if ((a_pstInfo->pChunkPool == NULL || a_pstInfo->usRemainSize == 0)
		|| ((a_pstInfo->eCodecType == eNEX_CODEC_V_MPEG1V || a_pstInfo->eCodecType == eNEX_CODEC_V_MPEG2V) && a_pstInfo->usRemainSize <= 4))
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG,"[%s %d] Chunk is small to find sync word\n", _FUNLINE_);
		return eFRAMEFINDER_NEED_MORE;
	}

	if (a_pstInfo->eCodecType == eNEX_CODEC_V_MPEG1V || a_pstInfo->eCodecType == eNEX_CODEC_V_MPEG2V)
	{
		NXUINT32 uExtID;
		NXUINT32 uPicStruct = 0;
		NXUINT32 uField = 0;
		NXBOOL bPicExtFound = FALSE;

		uBS = (pChunkFrame[0] << 16) | (pChunkFrame[1] << 8) | pChunkFrame[2];

		for (i = 0; i < a_pstInfo->usRemainSize-4; i++)
		{
			uBS = (uBS << 8) | ((NXUINT32)pChunkFrame[i+3] & 0x000000FF);

			if ((uBS == 0x100) || (uBS == 0x1b3) || (uBS == 0x1B8))	//Pic Start or Sequence header
			{
				if ((uBS == 0x1b3) || (uBS == 0x1B8))
				{	//Sequence Header 
					if (a_pstInfo->bFirstFrame || a_pstInfo->bReset)
					{
						if (bFirstSeqHeader == TRUE)
						{
							bFirstSeqHeader = FALSE;
							usStartPos = i;	//find start position
						}
						continue;
					}
					else
					{
						usCurPos = i;
						//bPicFound  = FALSE;
						a_pstInfo->bReset = TRUE;
					}                   
				}
				else if (uBS == 0x100)
				{
					//bPicFound = TRUE;
					if (a_pstInfo->bFirstFrame || a_pstInfo->bReset)
					{
						a_pstInfo->bFirstFrame = FALSE;
						a_pstInfo->bReset = FALSE;
						bFoundStartCode = TRUE;
						continue;
					}

					if (bPicExtFound)
					{
						if (uPicStruct == 3) //frame
						{
							usCurPos = i;
							//bPicFound = FALSE;
						}
						else if (uPicStruct == 1 || uPicStruct == 2) // field
						{
							uField++;
							if (uField == 2)
							{
								usCurPos = i;
								//uField = 0;
								//bPicFound = FALSE;
							}
							else
							{
								continue;
							}
						}
						else
						{
							continue;
						}
					}
					else
					{
						if (bFoundStartCode == FALSE)
						{
							bFoundStartCode = TRUE;
							continue;
						}
						else
						{
							usCurPos = i;
							//bPicFound = FALSE;
						}
					}
				}

				*a_pusFrameSize = usCurPos - usStartPos;
				*a_pusFrameOffset = usStartPos;

				return eFRAMEFINDER_OK;
			}
			else if (uBS == 0x1b5)	//extension start code
			{
				uExtID = (pChunkFrame[i+4] & 0xf0) >> 4;
				if (uExtID == 8) //Pic Coding Ext Code 
				{
					bPicExtFound = TRUE;
					uPicStruct = pChunkFrame[i+6] & 0x3;
				}
			}
		}
	}
	else if ((a_pstInfo->eCodecType == eNEX_CODEC_V_DIVX) || (a_pstInfo->eCodecType == eNEX_CODEC_V_MPEG4V))
	{
		for (i = 0; i < a_pstInfo->usRemainSize-4; i++)
		{
			uBS = (pChunkFrame[i] << 24) | (pChunkFrame[i+1] << 16) | (pChunkFrame[i+2] << 8) | ((NXUINT32)pChunkFrame[i+3] & 0x000000FF);

			if ((uBS == 0x1B0) || (uBS == 0x1B3) || (uBS == 0x1B6))	//Pic Start or Sequence header
			{
				if ((uBS == 0x1B0) || (uBS == 0x1B3))
				{	//Sequence Header 
					if (a_pstInfo->bFirstFrame || a_pstInfo->bReset)
					{
						if (bFirstSeqHeader == TRUE)
						{
							bFirstSeqHeader = FALSE;
							usStartPos = i;	//find start position
						}
						continue;
					}
					else
					{
						usCurPos = i;
						a_pstInfo->bReset = TRUE;
					}                   
				}
				else if (uBS == 0x1B6)
				{
					if (a_pstInfo->bFirstFrame || a_pstInfo->bReset)
					{
						a_pstInfo->bFirstFrame = FALSE;
						a_pstInfo->bReset = FALSE;
						bFoundStartCode = TRUE;
						continue;
					}

					if (bFoundStartCode == FALSE)
					{
						bFoundStartCode = TRUE;
						continue;
					}
					else
					{
						usCurPos = i;
					}
				}

				*a_pusFrameSize = usCurPos - usStartPos;
				*a_pusFrameOffset = usStartPos;

				return eFRAMEFINDER_OK;
			}
		}
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0,"[%s %d] Not Supported Video format\n", _FUNLINE_);
	}

	return eFRAMEFINDER_NEED_MORE;
}

static NXBOOL _CheckDisorderPOC(NXUINT32 a_auList[], NXUINT32 a_uIdx)
{
	if (0 < a_uIdx && a_uIdx < MAX_REF_NUM)
	{
		NXINT32 nDiff = a_auList[a_uIdx] - a_auList[a_uIdx-1];

		if (nDiff < 0)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] Disorder POC\n", _FUNLINE_);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}

	return FALSE;
}

static NXINT32 _CompareUINT32(const void *a_pLeft, const void *a_pRight)
{
	return(*(NXUINT32*)a_pLeft - *(NXUINT32*)a_pRight);
}

static NXUINT32 _GetPOCUnit(NXUINT32 a_auList[], NXUINT32 a_uIdx)
{
	NXUINT32 i = 0;
	NXUINT32 uMinPOCUnit = _MAX_POC_LSB_;

	if (0 < a_uIdx && a_uIdx < MIN_REF_NUM)
	{
		qsort(a_auList, a_uIdx+1, sizeof(a_auList[0]), _CompareUINT32);

		for (i = 0; i<a_uIdx; i++)
		{
			NXUINT32 uDiff = a_auList[i+1] - a_auList[i];

			if (0 < uDiff)
			{
				uMinPOCUnit = MIN(uMinPOCUnit, uDiff);
			}
		}
	}

	return uMinPOCUnit;
}

static NXINT32 _GetPicOrderCntMsb(IN NXUINT32 a_uPrevPOCLsb, IN NXINT32 a_nPrevPOCMsb, IN NXUINT32 a_uMaxPOCLsb, IN NXUINT32 a_uPOCLsb)
{
	NXINT32 nPOCMsb = 0;

	if ((a_uPOCLsb < a_uPrevPOCLsb) && ((a_uPrevPOCLsb-a_uPOCLsb) >= (a_uMaxPOCLsb/2)))
	{
		nPOCMsb = a_nPrevPOCMsb + a_uMaxPOCLsb;
	}
	else if ((a_uPOCLsb > a_uPrevPOCLsb) && ((a_uPOCLsb-a_uPrevPOCLsb) > (a_uMaxPOCLsb/2)))
	{
		nPOCMsb = a_nPrevPOCMsb - a_uMaxPOCLsb;
	}
	else
	{
		nPOCMsb = a_nPrevPOCMsb;
	}

	return nPOCMsb;
}

typedef struct _UNRELIABLE_TB
{
	NXUINT32 num_units_in_tick;
	NXUINT32 time_scale;
	NXUINT32 fixed_frame_rate_flag;
} UNRELIABLE_TB;

//[shoh][2015.01.09] It is inevitalbe that we calculate PTS although time base of H264 is unreliable. so we need to add unreliable contents into our list.
static NXBOOL _CheckReliableTB(IN NXUINT32 a_num_units_in_tick, IN NXUINT32 a_time_scale, IN NXUINT32 a_fixed_frame_rate_flag)
{
	NXUINT32 uIdx = 0;
	//add unreliabe timebase
	UNRELIABLE_TB arrList[] = { {1000, 30000, 1},	//http://mobliestream.c3tv.com:1935/live/goodtv.sdp/playlist.m3u8
		{1000, 50000, 1},	//http://121.78.118.51:1935/live/tv/tv.m3u8
		{1001, 60000, 1},	//http://edaily.flive.skcdn.com/high/livestream_128k/playlist.m3u8
	};
	NXUINT32 uListCnt = sizeof(arrList)/sizeof(UNRELIABLE_TB);

	if (a_time_scale >= 101*a_num_units_in_tick || a_time_scale < 5*a_num_units_in_tick)
	{
		return FALSE;
	}

	if (0 == a_fixed_frame_rate_flag)
	{
		return FALSE;
	}

	for (uIdx=0; uIdx<uListCnt; uIdx++)
	{
		if ((a_num_units_in_tick == arrList[uIdx].num_units_in_tick)
			&& (a_time_scale == arrList[uIdx].time_scale))
		{
			return FALSE;
		}
	}

	return TRUE;
}

static NXINT64 _CalculateH264PTS(IN VIDEOCHUNK_INFO *a_pstInfo, IN NXBOOL a_bIDRFrame)
{
	NEXCODECUTIL_AVC_SLICE_HEADER_INFO stAVCSHInfo;
	NXUINT32 uPOCLsb = 0;
	NXINT32 nPOCMsb = 0;
	NXINT64 qCalculatedPTS = 0;
	NXINT32 nFOC = 0;
	NXINT32 nAdjustedFOC = 0;
	NXUINT32 TempPOC = 0;
	NXINT64 qDuration = 0;
	NXBOOL bReliableTB = FALSE;

	memset(&stAVCSHInfo, 0x00, sizeof(NEXCODECUTIL_AVC_SLICE_HEADER_INFO));
	if (0 != NexCodecUtil_AVC_GetSliceHeaderInfo((NXINT8*)a_pstInfo->pFrameData, a_pstInfo->uFrameSize, &a_pstInfo->stSPSInfo, a_pstInfo->uNALLenSize, a_pstInfo->uH26xFormat, &stAVCSHInfo))
	{
		return a_pstInfo->qChunkPTS;
	}

	if (TRUE == a_bIDRFrame || 0 == a_pstInfo->uRefNum)
	{
		a_pstInfo->qCurIDRFramePTS = a_pstInfo->qFrameDTS;
		a_pstInfo->uPrevPOCLsb = 0;
		a_pstInfo->nPrevPOCMsb = 0;
	}

	uPOCLsb = stAVCSHInfo.pic_order_cnt_lsb; //[shoh][2013.11.20] lsb is unsigned and msb is signed.
	nPOCMsb = _GetPicOrderCntMsb(a_pstInfo->uPrevPOCLsb, a_pstInfo->nPrevPOCMsb, a_pstInfo->uMaxPOCLsb, uPOCLsb);

	a_pstInfo->uPrevPOCLsb = uPOCLsb;
	a_pstInfo->nPrevPOCMsb = nPOCMsb;

	if (TRUE == stAVCSHInfo.field_pic_flag)
	{
		SET_STATUS(a_pstInfo->uStatusFlag, STATUS_FIELD);
	}

	if (FALSE == stAVCSHInfo.bottom_field_flag)	//TopFieldOrderCnt
	{
		nFOC = uPOCLsb + nPOCMsb;
	}
	else //BottomFieldOrderCnt
	{
		if (FALSE == stAVCSHInfo.field_pic_flag)
		{
			//[shoh][2015.02.09] Originally, we must calculate TopFieldOrderCnt + delta_pic_order_cnt_bottom.
			nFOC = uPOCLsb + nPOCMsb;
		}
		else
		{
			nFOC = uPOCLsb + nPOCMsb;
		}
	}

	if (TRUE == a_bIDRFrame || 0 == a_pstInfo->uRefNum)
	{
		a_pstInfo->nOffsetFOC = nFOC;
	}

	if (0 <= NexCodecUtil_AVC_SPSExists(a_pstInfo->pFrameData, a_pstInfo->uFrameSize, a_pstInfo->uH26xFormat, a_pstInfo->uNALLenSize))
	{
		NEXCODECUTIL_SPS_INFO stSPSInfo;
		const NXUINT32 nuit_field_based_flag = 1; //[shoh][2014.10.20] nuit_field_based_flag shall be always 1.

		memset(&stSPSInfo, 0x00, sizeof(NEXCODECUTIL_SPS_INFO));
		NexCodecUtil_AVC_GetSPSInfo((NXCHAR*)a_pstInfo->pFrameData, a_pstInfo->uFrameSize, &stSPSInfo, a_pstInfo->uH26xFormat);

		if (1 == stSPSInfo.timing_info_present_flag)
		{
			bReliableTB = _CheckReliableTB(stSPSInfo.num_units_in_tick, stSPSInfo.time_scale, stSPSInfo.fixed_frame_rate_flag);

			if (TRUE == bReliableTB)
			{
				// [shoh][2013.11.26] Calcualating TS : (num_units_in_tick * (1 +  nuit_field_based_flag))
				a_pstInfo->fCalculatedDuration = (stSPSInfo.num_units_in_tick*(1+nuit_field_based_flag))*(a_pstInfo->stExtInfo.m_uTimescale/(NXFLOAT)stSPSInfo.time_scale);
			}
			else
			{
				a_pstInfo->fCalculatedDuration = 0;
			}
		}
		else
		{
			a_pstInfo->fCalculatedDuration = 0;
		}
	}

	if (a_pstInfo->nOffsetFOC != 0)
	{
		nAdjustedFOC = nFOC - a_pstInfo->nOffsetFOC;
		TempPOC = uPOCLsb - a_pstInfo->nOffsetFOC;
	}
	else
	{
		nAdjustedFOC = nFOC;
		TempPOC = uPOCLsb;
	}

	if (a_pstInfo->uRefNum < MIN_REF_NUM) // Gaining POC unit value (Variable gaps can be led to in POC but we expect POC unit value is constant.)
	{
		if (TRUE == CHECK_STATUS(a_pstInfo->uStatusFlag, STATUS_FIELD))
		{
			a_pstInfo->uPOCUnit = 1; //[shoh][2015.02.09] If there are fields, I guess POC unit is 1.
		}
		else
		{
			a_pstInfo->auPOCList[a_pstInfo->uRefNum] = TempPOC;
			a_pstInfo->uPOCUnit = _GetPOCUnit(a_pstInfo->auPOCList, a_pstInfo->uRefNum);
		}

		a_pstInfo->uRefNum++;
	}

	if (fabs(a_pstInfo->fCalculatedDuration) >= FLT_EPSILON)
	{
		//[shoh][2015.02.09] If there are fields, I guess duration is divided by 2.
		a_pstInfo->fDuration = a_pstInfo->fCalculatedDuration/(CHECK_STATUS(a_pstInfo->uStatusFlag, STATUS_FIELD)==TRUE ? 2 : 1);
		qDuration = (NXINT64)(a_pstInfo->fDuration * nAdjustedFOC)/(NXINT64)a_pstInfo->uPOCUnit;
	}
	else
	{
		qDuration = (NXINT64)(a_pstInfo->fDuration * nAdjustedFOC)/(NXINT64)a_pstInfo->uPOCUnit;
	}

	if ((qDuration < 0) && ((NXINT64)a_pstInfo->qCurIDRFramePTS + qDuration < 0))
	{
		qCalculatedPTS = 0;
	}
	else
	{
		qCalculatedPTS = (NXINT64)((NXINT64)a_pstInfo->qCurIDRFramePTS + qDuration);
	}

	if (0 == VCP_LOG) // for debugging
	{
		if (FALSE == a_bIDRFrame)
		{
			if (TRUE == NexCodecUtil_IsSeekableFrame(a_pstInfo->eCodecType, a_pstInfo->pFrameData, a_pstInfo->uFrameSize, (NXVOID*)&a_pstInfo->uNALLenSize, a_pstInfo->uH26xFormat, FALSE))
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [TS] DTS(%10lld) PTS(%10lld) fDuration(%5d) nFOC(%5d) I - Frame\n",
								_FUNLINE_, a_pstInfo->qFrameDTS, qCalculatedPTS, (NXINT32)a_pstInfo->fDuration, nFOC);  
			}
			else if (TRUE == NexCodecUtil_IsBFrame(a_pstInfo->eCodecType, a_pstInfo->pFrameData, a_pstInfo->uFrameSize, (NXVOID*)&a_pstInfo->uNALLenSize, a_pstInfo->uH26xFormat, FALSE))
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [TS] DTS(%10lld) PTS(%10lld) fDuration(%5d) nFOC(%5d) B - Frame\n",
								_FUNLINE_, a_pstInfo->qFrameDTS, qCalculatedPTS, (NXINT32)a_pstInfo->fDuration, nFOC);
			}
			else
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [TS] DTS(%10lld) PTS(%10lld) fDuration(%5d) nFOC(%5d) P - Frame\n",
								_FUNLINE_, a_pstInfo->qFrameDTS, qCalculatedPTS, (NXINT32)a_pstInfo->fDuration, nFOC);
			}
		}
		else
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [TS] DTS(%10lld) PTS(%10lld) fDuration(%5d) nFOC(%5d) IDR - Frame\n",
							_FUNLINE_, a_pstInfo->qFrameDTS, qCalculatedPTS, (NXINT32)a_pstInfo->fDuration, nFOC);
		}

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [TS] field_pic_flag(%d) bottom_field_flag](%d)\n",
						_FUNLINE_, stAVCSHInfo.field_pic_flag, stAVCSHInfo.bottom_field_flag);
	}

	return qCalculatedPTS;
}

static NXINT32 _ParseH26x(IN NXUINT8 *a_pFrame, IN NXUINT32 a_uFrameLen, INOUT NXUINT32 *a_puH26xFormat, INOUT NXUINT32 *a_puNALLenSize)
{
	*a_puH26xFormat = (NXUINT32)NexCodecUtil_CheckByteFormat(a_pFrame, a_uFrameLen);

	if (NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_RAW == *a_puH26xFormat && (0 == *a_puNALLenSize || 4 < *a_puNALLenSize))
	{
		*a_puNALLenSize = NexCodecUtil_GuessNalHeaderLengthSize(a_pFrame, a_uFrameLen);

		if (0 == *a_puNALLenSize && 4 < *a_puNALLenSize)
		{
			return -1;
		}
	}

	return 0;
}

//To convert 64bit variables to 32bit variables temporarily
//TEMP/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NXINT32 VideoChunkParser_PutFrame32(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXUINT32 a_uDTS, IN NXUINT32 a_uPTS)
{
	NXINT64 qDTS = 0;
	NXINT64 qPTS = 0;

	if (INVALID_VALUE == a_uDTS)
	{
		qDTS = INVALID_VALUE;
	}
	else
	{
		qDTS = (NXINT64)a_uDTS;
	}

	if (INVALID_VALUE == a_uPTS)
	{
		qPTS = INVALID_VALUE;
	}
	else
	{
		qPTS = (NXINT64)a_uPTS;
	}

	return VideoChunkParser_PutFrame64(a_hChunkInfo, a_pChunk, a_uChunkLen, qDTS, qPTS);
}

NXINT32 VideoChunkParser_GetFrame32(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXUINT32 *a_puFrameDTS, OUT NXUINT32 *a_puFramePTS, OUT NXBOOL *a_pbIFrame)
{
	NXINT64 qDTS = 0;
	NXINT64 qPTS = 0;
	NXINT32 nRet = 0;

	nRet = VideoChunkParser_GetFrame64(a_hChunkInfo, a_uGetOption, a_ppFrame, a_puFrameLen, &qDTS, &qPTS, a_pbIFrame);

	if (INVALID_VALUE == qDTS)
	{
		*a_puFrameDTS = (NXUINT32)INVALID_VALUE;
	}
	else
	{
		*a_puFrameDTS = (NXUINT32)qDTS;
	}

	if (INVALID_VALUE == qPTS)
	{
		*a_puFramePTS = (NXUINT32)INVALID_VALUE;
	}
	else
	{
		*a_puFramePTS = (NXUINT32)qPTS;
	}

	return nRet;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VIDEOCHUNK_HANDLE VideoChunkParser_CreateInstance(IN VIDEOCHUNK_EXTINFO *a_pExtInfo, IN NXVOID *a_pUserData)
{
	VIDEOCHUNK_INFO *pstInfo = (VIDEOCHUNK_INFO *)nexSAL_MemAlloc(sizeof(VIDEOCHUNK_INFO));

	a_pUserData = NULL;

	if (NULL == pstInfo)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Malloc failed!\n", _FUNLINE_);
		return NULL;
	}

	memset(pstInfo, 0, sizeof(VIDEOCHUNK_INFO));

	if (a_pExtInfo == NULL)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] pExtInfo is NULL!\n", _FUNLINE_);
		return NULL;
	}
	else
	{
		memcpy(&pstInfo->stExtInfo, a_pExtInfo, sizeof(VIDEOCHUNK_EXTINFO));

		if (0 == pstInfo->stExtInfo.m_uTimescale) //[shoh][2017.01.05] use default value as millisecond scale if m_uTimescale is 0.
		{
			pstInfo->stExtInfo.m_uTimescale = 1000;
		}
	}

	NexUtil_ConvVideoCodecType(pstInfo->stExtInfo.m_eCodecType, pstInfo->stExtInfo.m_uFourCC, pstInfo->stExtInfo.m_eFFType, &pstInfo->eCodecType, &pstInfo->uFourCC);      

	pstInfo->eFFType = pstInfo->stExtInfo.m_eFFType;
	pstInfo->eState = eVIDEOCHUNK_CREATE;
	pstInfo->bReset = FALSE;
	pstInfo->bFirstFrame = TRUE;
	pstInfo->usMaxChunkSize = _DEFAULT_VIDEO_FRAME_SIZE_;
	pstInfo->fDuration = 0;
	pstInfo->fAveInterval4MP4 = 0;
	pstInfo->qTotalDuration = 0;
	pstInfo->uChunkedCount = 0;
	pstInfo->uTotalChunkedCount = 0;
	pstInfo->qFrameDTS = 0;
	pstInfo->qFramePTS = 0;
	pstInfo->qChunkDTS = 0;
	pstInfo->qChunkPTS = 0;
	pstInfo->qPreChunkDTS = INVALID_VALUE;
	pstInfo->qCurIDRFramePTS = 0;
	pstInfo->nIFrameCnt = 0;
	pstInfo->uPrevPOCLsb = 0;
	pstInfo->nPrevPOCMsb = 0;
	pstInfo->nOffsetFOC = 0;
	memset(pstInfo->auPOCList, 0x00, sizeof(pstInfo->auPOCList));
	pstInfo->uPOCUnit = _MAX_POC_LSB_;
	pstInfo->uRefNum = 0;
	pstInfo->bNeedPTSCal = FALSE;
	pstInfo->uH26xFormat = NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANY;
	SET_STATUS_ZERO(pstInfo->uStatusFlag);

	pstInfo->pChunkQueue = Queue_Init(_DestroyData);
	if (NULL == pstInfo->pChunkQueue)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Creating queue failed!\n", _FUNLINE_);
		SAFE_FREE(pstInfo);
		return NULL;
	}

	pstInfo->pChunkPool = (NXUINT8*)nexSAL_MemAlloc(pstInfo->usMaxChunkSize);
	if (pstInfo->pChunkPool == NULL)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Malloc failed!\n", _FUNLINE_);
		SAFE_FREE(pstInfo);
		return NULL;
	}

	if (eNEX_CODEC_V_H264 == pstInfo->eCodecType)
	{
		NXUINT32 uDSIFormat = NexCodecUtil_CheckByteFormat(pstInfo->stExtInfo.m_pDSI, pstInfo->stExtInfo.m_uDSILen);
		if (0 != NexCodecUtil_AVC_GetSPSInfo((NXCHAR *)pstInfo->stExtInfo.m_pDSI, pstInfo->stExtInfo.m_uDSILen, &pstInfo->stSPSInfo, uDSIFormat))
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] SPS does not exist!\n", _FUNLINE_);
			Queue_Destroy(pstInfo->pChunkQueue);
			SAFE_FREE(pstInfo->pChunkPool);
			SAFE_FREE(pstInfo);
			return NULL;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] ChunkParser Create for AVC\n", _FUNLINE_);

		if (eNEX_FF_MP4 == pstInfo->stExtInfo.m_eFFType || eNEX_FF_MP4MF == pstInfo->stExtInfo.m_eFFType)
		{
			if ((pstInfo->stExtInfo.m_pDSI) && (pstInfo->stExtInfo.m_uDSILen > 0))
			{
				pstInfo->uNALLenSize = NexCodecUtil_AVC_ParseH264NalHeaderLengthSize(pstInfo->stExtInfo.m_pDSI, pstInfo->stExtInfo.m_uDSILen);
			}
		}
		else if (eNEX_FF_MPEG_TS == pstInfo->stExtInfo.m_eFFType)
		{
			pstInfo->uH26xFormat = NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
			pstInfo->uNALLenSize = 4;
		}
		else
		{
			pstInfo->uNALLenSize = 0;
		}

		pstInfo->uMaxPOCLsb = 0x01 << pstInfo->stSPSInfo.log2_max_pic_order_cnt_lsb;

	}
	else if (eNEX_CODEC_V_HEVC == pstInfo->eCodecType)
	{
		NXUINT32 uDSIForamt = NexCodecUtil_CheckByteFormat(pstInfo->stExtInfo.m_pDSI, pstInfo->stExtInfo.m_uDSILen);
		if (0 != NexCodecUtil_HEVC_GetSPSInfo((NXCHAR *)pstInfo->stExtInfo.m_pDSI, pstInfo->stExtInfo.m_uDSILen, &pstInfo->stSPSInfo, uDSIForamt))
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] SPS does not exist!\n", _FUNLINE_);
			Queue_Destroy(pstInfo->pChunkQueue);
			SAFE_FREE(pstInfo->pChunkPool);
			SAFE_FREE(pstInfo);         
			return NULL;
		}

		nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] ChunkParser Create for HEVC\n", _FUNLINE_);

		if (eNEX_FF_MP4 == pstInfo->stExtInfo.m_eFFType || eNEX_FF_MP4MF == pstInfo->stExtInfo.m_eFFType)
		{
			if ((pstInfo->stExtInfo.m_pDSI) && (pstInfo->stExtInfo.m_uDSILen > 0))
			{
				pstInfo->uNALLenSize = NexCodecUtil_HEVC_ParseNalHeaderLengthSize(pstInfo->stExtInfo.m_pDSI, pstInfo->stExtInfo.m_uDSILen);
			}
		}
		else if (eNEX_FF_MPEG_TS == pstInfo->stExtInfo.m_eFFType)
		{
			pstInfo->uH26xFormat = NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANNEXB;
			pstInfo->uNALLenSize = 4;
		}
		else
		{
			pstInfo->uNALLenSize = 0;
		}

		pstInfo->uMaxPOCLsb = 0x01 << pstInfo->stSPSInfo.log2_max_pic_order_cnt_lsb;
	}

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] NexChunkParser ver%s\n", _FUNLINE_, NexChunkParser_GetVersionString());
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] VideoChunkParser created instance(pInfo = 0x%p).\n", _FUNLINE_, pstInfo);

	return(VIDEOCHUNK_HANDLE)pstInfo;
}

NXVOID VideoChunkParser_Destroy(IN VIDEOCHUNK_HANDLE a_hChunkInfo)
{
	VIDEOCHUNK_INFO *pstInfo = (VIDEOCHUNK_INFO*)a_hChunkInfo;

	if (pstInfo)
	{
		pstInfo->eState = eVIDEOCHUNK_DESTROY;

		Queue_Destroy(pstInfo->pChunkQueue);

		SAFE_FREE(pstInfo->pChunkPool);
		SAFE_FREE(pstInfo);

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Destroy done!\n", _FUNLINE_);
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Destroy isn't effected!\n", _FUNLINE_);
	}
}

NXVOID VideoChunkParser_Reset(IN VIDEOCHUNK_HANDLE a_hChunkInfo)
{
	VIDEOCHUNK_INFO *pstInfo = (VIDEOCHUNK_INFO*)a_hChunkInfo;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] pInfo = 0x%p\n", _FUNLINE_, pstInfo);

	if (pstInfo)
	{
		pstInfo->eState = eVIDEOCHUNK_RESET;
		pstInfo->usRemainSize = 0;
		pstInfo->usRemainSizeHEVC = 0;
		pstInfo->usCurPos = 0;
		pstInfo->fDuration = 0;
		pstInfo->fAveInterval4MP4 = 0;
		pstInfo->qTotalDuration = 0;
		pstInfo->uChunkedCount = 0;
		pstInfo->uTotalChunkedCount = 0;
		pstInfo->qFrameDTS = 0;
		pstInfo->qFramePTS = 0;
		pstInfo->qChunkDTS = 0;
		pstInfo->qChunkPTS = 0;
		pstInfo->qPreChunkDTS = INVALID_VALUE;
		pstInfo->qCurIDRFramePTS = 0;
		pstInfo->nIFrameCnt = 0;
		pstInfo->uPrevPOCLsb = 0;
		pstInfo->nPrevPOCMsb = 0;

		Queue_Destroy(pstInfo->pChunkQueue);
		pstInfo->pChunkQueue = Queue_Init(_DestroyData);

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Reset Done.\n", _FUNLINE_);
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Reset isn't effected.\n", _FUNLINE_);
	}
}

NXINT32 VideoChunkParser_PutFrame64(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXINT64 a_qDTS, IN NXINT64 a_qPTS)
{
	VIDEOCHUNK_INFO *pstInfo = (VIDEOCHUNK_INFO*)a_hChunkInfo;

	if ((INVALID_VALUE == a_qDTS) && ((pstInfo->eState == eVIDEOCHUNK_CREATE) || (pstInfo->eState == eVIDEOCHUNK_RESET)))
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] The first incompleted frame was ignored!", _FUNLINE_);
		return 0;
	}

	if (a_pChunk)
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] pChunk(0x%p), len(%8d), remain(%8"PRIuS"), DTS(%10lld), PTS(%10lld)\n", _FUNLINE_, a_pChunk, a_uChunkLen, pstInfo->usRemainSize, a_qDTS, a_qPTS);

		if (eNEX_FF_MP4 == pstInfo->stExtInfo.m_eFFType || eNEX_FF_MP4MF == pstInfo->stExtInfo.m_eFFType)
		{
			if (0 != _ParseH26x(a_pChunk, a_uChunkLen, &pstInfo->uH26xFormat, &pstInfo->uNALLenSize))
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] _ParseH26x failed!\n", _FUNLINE_);
				return -1;
			}
		}

		// Copy remain data to the first of buffer
		if (pstInfo->usRemainSize > 0)
		{
			memmove(pstInfo->pChunkPool, pstInfo->pChunkPool + pstInfo->usCurPos, pstInfo->usRemainSize);
		}

		// Resizing m_pChunkData because of insufficient memory
		if (((NXUSIZE)a_uChunkLen + pstInfo->usRemainSize) > pstInfo->usMaxChunkSize)
		{
			if (pstInfo->usMaxChunkSize >= VIDEOCHUNK_MEMORY_LIMIT)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] ChunkPool reached memory limit(%10"PRIuS"), so previous chunk will be discarded!\n", _FUNLINE_, pstInfo->usMaxChunkSize);

				pstInfo->usRemainSize = 0;
			}
			else
			{
				NXUINT8 *pTmpFrame = NULL;

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, 0, "[%s %d] Resizing m_pChunkData (%10"PRIuS"->%10"PRIuS")\n", _FUNLINE_, pstInfo->usMaxChunkSize, ((NXUSIZE)a_uChunkLen + pstInfo->usRemainSize));

				pTmpFrame = (NXUINT8*)nexSAL_MemAlloc((NXUSIZE)a_uChunkLen + pstInfo->usRemainSize);

				if (pTmpFrame)
				{
					if (pstInfo->usRemainSize > 0)
					{
						memmove(pTmpFrame, pstInfo->pChunkPool, pstInfo->usRemainSize);
					}

					SAFE_FREE(pstInfo->pChunkPool);
					pstInfo->pChunkPool = pTmpFrame;
					pstInfo->usMaxChunkSize = (NXUSIZE)a_uChunkLen + pstInfo->usRemainSize;
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Malloc Failed\n", _FUNLINE_);
					return -1;
				}
			}
		}

		// Copy new Data
		memmove(pstInfo->pChunkPool + pstInfo->usRemainSize, a_pChunk, (NXUSIZE)a_uChunkLen);

		//[shoh][2012.10.24] MP4 format needs to reorganize PTS.
		if ((eNEX_FF_MP4 == pstInfo->eFFType || eNEX_FF_MP4MF == pstInfo->eFFType) && pstInfo->eCodecType == eNEX_CODEC_V_H264)
		{
			NEXCODECUTIL_AVC_SLICE_HEADER_INFO stAVCSHInfo;
			NXUINT32 uPOCLsb = 0;
			NXINT32 nPOCMsb = 0; 
			NXBOOL bIDRFrame = FALSE;
			NXINT32 nTopFOC = 0;
			CHUNK_QUEUE *pstData = NULL;

			if (0 == pstInfo->stSPSInfo.pic_order_cnt_type)
			{
				bIDRFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, a_pChunk, a_uChunkLen, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, TRUE);

				if (TRUE == bIDRFrame)
				{
					pstInfo->qCurIDRFramePTS = a_qPTS;
					pstInfo->uPrevPOCLsb = 0;
					pstInfo->nPrevPOCMsb = 0;
					pstInfo->nIFrameCnt++;
				}
				else if ((pstInfo->eState == eVIDEOCHUNK_CREATE) || (pstInfo->eState == eVIDEOCHUNK_RESET))
				{
					NXBOOL bIFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, a_pChunk, a_uChunkLen, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, FALSE);

					if (TRUE == bIFrame)
					{
						bIDRFrame = TRUE; //[shoh][2013.11.20] I-frame is regarded as IDR because certain contents of none IDR or only one IDR cannot calculate PTS after start or seek.
						pstInfo->qCurIDRFramePTS = a_qPTS;
						pstInfo->uPrevPOCLsb = 0;
						pstInfo->nPrevPOCMsb = 0;
						pstInfo->nIFrameCnt++;
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] The first frame is I-frame immediately after start or seek, so it will be dealt with IDR frame.\n", _FUNLINE_);
					}
					else
					{
						pstInfo->nIFrameCnt++; //[shoh][2017.05.15] fake increase
					}
				}

				if (0 != NexCodecUtil_AVC_GetSliceHeaderInfo((NXINT8*)a_pChunk, a_uChunkLen, &pstInfo->stSPSInfo, pstInfo->uNALLenSize, pstInfo->uH26xFormat, &stAVCSHInfo))
				{
					pstInfo->uPrevPOCLsb = uPOCLsb = 0;
					pstInfo->nPrevPOCMsb = nPOCMsb = 0;
					nTopFOC = uPOCLsb + nPOCMsb;
				}
				else
				{
					uPOCLsb = stAVCSHInfo.pic_order_cnt_lsb; //[shoh][2013.11.20] lsb is unsigned and msb is signed.
					nPOCMsb = _GetPicOrderCntMsb(pstInfo->uPrevPOCLsb, pstInfo->nPrevPOCMsb, pstInfo->uMaxPOCLsb, uPOCLsb);

					pstInfo->uPrevPOCLsb = uPOCLsb;
					pstInfo->nPrevPOCMsb = nPOCMsb;

					nTopFOC = uPOCLsb + nPOCMsb;
				}

				if (TRUE == bIDRFrame)
				{
					pstInfo->nOffsetFOC = nTopFOC;
				}

				if (0 != pstInfo->nOffsetFOC)
				{
					nTopFOC = nTopFOC - pstInfo->nOffsetFOC;
					uPOCLsb = ((NXINT32)uPOCLsb >= pstInfo->nOffsetFOC) ? (uPOCLsb - pstInfo->nOffsetFOC) : uPOCLsb;
				}

				if (pstInfo->uRefNum < MAX_REF_NUM)
				{
					if (pstInfo->nIFrameCnt < 2)
					{
						pstInfo->auPOCList[pstInfo->uRefNum] = nTopFOC;

						if (FALSE == CHECK_STATUS(pstInfo->uStatusFlag, STATUS_DISORDER_POC) && TRUE == _CheckDisorderPOC(pstInfo->auPOCList, pstInfo->uRefNum))
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] Disorder POC is found.\n", _FUNLINE_);
							SET_STATUS(pstInfo->uStatusFlag, STATUS_DISORDER_POC);
						}

						if (pstInfo->uRefNum < MIN_REF_NUM)
						{
							pstInfo->uPOCUnit = _GetPOCUnit(pstInfo->auPOCList, pstInfo->uRefNum); // Gaining POC unit value (Variable gaps can be led to in POC but we expect POC unit value is constant.)
						}
					}
					else
					{
						if (TRUE == NexCodecUtil_IsBFrame(pstInfo->eCodecType, a_pChunk, a_uChunkLen, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, FALSE))
						{
							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] Guess disorder POC because B-Frame is found.\n", _FUNLINE_);
							SET_STATUS(pstInfo->uStatusFlag, STATUS_DISORDER_POC);
						}
					}

					pstInfo->uRefNum++;
				}

				if ((pstInfo->eState == eVIDEOCHUNK_CREATE) || (pstInfo->eState == eVIDEOCHUNK_RESET))
				{
					pstInfo->qTotalDuration = 0;
				}
				else
				{
					pstInfo->qTotalDuration += (a_qDTS > pstInfo->qChunkDTS) ? (a_qDTS - pstInfo->qChunkDTS) : 0;
				}

				if (INVALID_VALUE == a_qDTS)
				{
					pstInfo->fAveInterval4MP4 = DUMMY_INTERVAL;
				}
				else
				{
					pstInfo->fAveInterval4MP4 = ((pstInfo->uTotalChunkedCount > 0) ? (pstInfo->qTotalDuration / (NXFLOAT)pstInfo->uTotalChunkedCount) : 0);
				}
				pstInfo->uTotalChunkedCount++;
			}
			else
			{
				pstInfo->uRefNum++;
			}

			pstInfo->qChunkDTS = a_qDTS;
			pstInfo->qChunkPTS = a_qPTS;
			pstInfo->eState = eVIDEOCHUNK_PUTFRAME;

			pstData = nexSAL_MemAlloc(sizeof(CHUNK_QUEUE));
			if (NULL == pstData)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Malloc Failed\n", _FUNLINE_);
				return -1;
			}

			pstData->uLen = a_uChunkLen;
			pstData->qDTS = pstInfo->qChunkDTS;
			pstData->qPTS = pstInfo->qChunkPTS;
			pstData->qCurIDRFramePTS = pstInfo->qCurIDRFramePTS;
			pstData->nTopFOC = nTopFOC;
			Queue_Enqueue(pstInfo->pChunkQueue, (const NXVOID*)pstData);

		}
		else if ((eNEX_FF_MP4 == pstInfo->eFFType || eNEX_FF_MP4MF == pstInfo->eFFType || eNEX_FF_MPEG_TS == pstInfo->eFFType)
				 && pstInfo->eCodecType == eNEX_CODEC_V_HEVC)
		{
			NEXCODECUTILHEVCSliceHeaderInfo stHEVCSHInfo;
			NXUINT32 uPOCLsb = 0;
			NXINT32 nPOCMsb = 0; 
			NXBOOL bIDRFrame = FALSE;
			NXINT32 nCurTopFOC = 0;
			NXUINT32 uFrameLen = 0, uSumFrameLen = 0;

			NXUINT8 *pChunk;
			NXUINT32 uChunkLen;

			NXBOOL bPrefixDone = FALSE;
			NXUINT32 uNALLenSize = pstInfo->uNALLenSize;
			CHUNK_QUEUE *pstData = NULL;
			NXINT32 nErrNum = 0;

			if (pstInfo->usRemainSizeHEVC > 0)
			{
				pChunk = pstInfo->pChunkPool;
				uChunkLen = (NXUINT32)pstInfo->usRemainSizeHEVC + a_uChunkLen;
			}
			else
			{
				pChunk = a_pChunk;
				uChunkLen = a_uChunkLen;
			}

			HEVC_START :
			pChunk += (NXUSIZE)uFrameLen;
			uChunkLen -= uFrameLen;

			if (1 < pstInfo->uMaxPOCLsb)
			{
				//bOnlyIDR : TRUE	=> NALType 16~20 
				//bOnlyIDR : FALSE	=> NALType 16~21
				//NalType 16~21 can always seekable for HEVC
				memset(&stHEVCSHInfo, 0x00, sizeof(NEXCODECUTILHEVCSliceHeaderInfo));
				pstInfo->bFirstPicDone = TRUE;
				bIDRFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, pChunk, uChunkLen, (NXVOID*)&uNALLenSize, pstInfo->uH26xFormat, TRUE);
				if (TRUE == bIDRFrame)
				{
					pstInfo->qCurIDRFramePTS = a_qPTS;
					pstInfo->uPrevPOCLsb = 0;
					pstInfo->nPrevPOCMsb = 0;
					pstInfo->bFirstPicDone = FALSE;
				}
				else if ((pstInfo->eState == eVIDEOCHUNK_CREATE) || (pstInfo->eState == eVIDEOCHUNK_RESET))
				{
					NXUINT32 uNALLenSize = pstInfo->uNALLenSize;

					NXBOOL bIFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, pChunk, uChunkLen, (NXVOID*)&uNALLenSize, pstInfo->uH26xFormat, FALSE);
					if (TRUE == bIFrame)
					{
						bIDRFrame = TRUE; 
						pstInfo->qCurIDRFramePTS = a_qPTS;
						pstInfo->uPrevPOCLsb = 0;
						pstInfo->nPrevPOCMsb = 0;
						pstInfo->bFirstPicDone = FALSE;
						nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] The first frame is I-frame immediately after start or seek, so it will be dealt with IDR frame.\n", _FUNLINE_);
					}
				}

				nErrNum = NexCodecUtil_HEVC_GetSliceHeaderInfo((NXINT8*)pChunk, uChunkLen, &uPOCLsb, &nPOCMsb, 
															   &pstInfo->uPrevPOCLsb, &pstInfo->nPrevPOCMsb,
															   &pstInfo->stSPSInfo, &pstInfo->stPPSInfo, 
															   pstInfo->uNALLenSize, pstInfo->uH26xFormat, &stHEVCSHInfo, pstInfo->bFirstPicDone);
				if (nErrNum)
				{
					memmove(pstInfo->pChunkPool + pstInfo->usRemainSize, a_pChunk, (NXUSIZE)a_uChunkLen);
					pstInfo->usCurPos = 0;
					pstInfo->usRemainSize = (a_uChunkLen);
					pstInfo->usRemainSizeHEVC = (a_uChunkLen);
					return 0;
				}

				if (pstInfo->eState == eVIDEOCHUNK_RESET)
				{
					nPOCMsb = 0;
				}

				nCurTopFOC = uPOCLsb + nPOCMsb;

				if (TRUE == bIDRFrame)
				{
					pstInfo->nOffsetFOC = nCurTopFOC;
				}

				if (0 != pstInfo->nOffsetFOC)
				{
					nCurTopFOC = nCurTopFOC - pstInfo->nOffsetFOC;
					uPOCLsb = ((NXINT32)uPOCLsb >= pstInfo->nOffsetFOC) ? (uPOCLsb - pstInfo->nOffsetFOC) : uPOCLsb;
				}

				if (pstInfo->uRefNum < MIN_REF_NUM)
				{
					pstInfo->auPOCList[pstInfo->uRefNum] = uPOCLsb;
					pstInfo->uPOCUnit = _GetPOCUnit(pstInfo->auPOCList, pstInfo->uRefNum);
					pstInfo->uRefNum++;
				}

				if ((pstInfo->eState == eVIDEOCHUNK_CREATE) || (pstInfo->eState == eVIDEOCHUNK_RESET))
				{
					pstInfo->qTotalDuration = 0;
				}
				else
				{
					if (!bPrefixDone)
					{
						pstInfo->qTotalDuration += (a_qDTS > pstInfo->qChunkDTS) ? (a_qDTS - pstInfo->qChunkDTS) : 0;
					}
				}

				if (INVALID_VALUE == a_qDTS)
				{
					pstInfo->fAveInterval4MP4 = DUMMY_INTERVAL;
				}
				else
				{
					pstInfo->fAveInterval4MP4 = ((pstInfo->uTotalChunkedCount > 0) ? (pstInfo->qTotalDuration / (float)pstInfo->uTotalChunkedCount) : 0);
				}
				pstInfo->uTotalChunkedCount++;
			}

#if 0
			//Check Config Data (VPS, SPS, PPS, AUD, PrefixSEI & reserved)
			if (NexCodecUtil_HEVC_isPrefixConfigFrame(pChunk, uChunkLen, pstInfo->uNALLenSize, pstInfo->uH26xFormat))
			{
				NXUINT8 *pConfigStream = (NXUINT8)0;
				NXINT32 resultLength;

				if (pstInfo->uH26xFormat == NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANNEXB)
				{
					pConfigStream = NexCodecUtil_HEVC_ANNEXB_GetConfigStream(pChunk, uChunkLen, &resultLength);
				}
				else
				{
					pConfigStream = NexCodecUtil_HEVC_NAL_GetConfigStream(pChunk, uChunkLen, pstInfo->uNALLenSize, &resultLength);
				}

				if (pConfigStream != NULL && resultLength !=0)
				{
					uFrameLen = resultLength;
					uSumFrameLen += resultLength;
					if (1 < pstInfo->uMaxPOCLsb)
					{
						pstInfo->uTotalChunkedCount--;
						if (pstInfo->uTotalChunkedCount > 0)
						{
							bPrefixDone = TRUE;
						}
					}
					goto HEVC_START;
				}
			}
#endif

			pstInfo->qChunkDTS = a_qDTS;
			pstInfo->qChunkPTS = a_qPTS;
			pstInfo->eState = eVIDEOCHUNK_PUTFRAME;

			uFrameLen = NexCodecUtil_HEVC_GetAccessUnit(pChunk , uChunkLen, pstInfo->uNALLenSize, pstInfo->uH26xFormat);

			uSumFrameLen += uFrameLen;

			if (uSumFrameLen < uChunkLen)
			{
				if (NexCodecUtil_HEVC_isSuffixConfigFrame(pChunk + uFrameLen, uChunkLen - uSumFrameLen, pstInfo->uNALLenSize, pstInfo->uH26xFormat))
				{
					if (1 < pstInfo->uMaxPOCLsb)
					{
						pstInfo->uTotalChunkedCount--;

						if (pstInfo->uTotalChunkedCount > 0)
						{
							bPrefixDone = TRUE;
						}
					}
					goto HEVC_START;
				}
			}

			pstData = nexSAL_MemAlloc(sizeof(CHUNK_QUEUE));
			if (NULL == pstData)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Malloc Failed\n", _FUNLINE_);
				return -1;
			}

			pstData->uLen = uSumFrameLen;
			pstData->qDTS = pstInfo->qChunkDTS;
			pstData->qPTS = pstInfo->qChunkPTS;
			pstData->qCurIDRFramePTS = pstInfo->qCurIDRFramePTS;
			pstData->nTopFOC = nCurTopFOC;
			Queue_Enqueue(pstInfo->pChunkQueue, (const NXVOID*)pstData);

			uSumFrameLen = 0;

			if (uChunkLen > uFrameLen)
			{
				pstInfo->uTotalChunkedCount--;
				goto HEVC_START;
			}
		}
		else
		{
			// Getting chunk DTS & PTS & duration
			if (INVALID_VALUE != a_qDTS)
			{
				if ((pstInfo->eState == eVIDEOCHUNK_CREATE) || (pstInfo->eState == eVIDEOCHUNK_RESET))
				{
					pstInfo->qChunkDTS = a_qDTS;
					pstInfo->qChunkPTS = a_qPTS;
					pstInfo->fDuration = DUMMY_INTERVAL;
				}
				else
				{
					if (a_qDTS >= pstInfo->qChunkDTS)
					{
						pstInfo->uTotalChunkedCount++;
						pstInfo->qTotalDuration += a_qDTS - pstInfo->qChunkDTS;
						pstInfo->fDuration = (NXFLOAT)pstInfo->qTotalDuration / pstInfo->uTotalChunkedCount;
					}
					else
					{
						pstInfo->fDuration = DUMMY_INTERVAL;
					}

					pstInfo->qChunkDTS = a_qDTS;
					pstInfo->qChunkPTS = a_qPTS;
				}

				pstInfo->uChunkedCount = 0;
				pstInfo->eState = eVIDEOCHUNK_PUTFRAME;
			}
			else
			{
				pstInfo->eState = eVIDEOCHUNK_DUMMY_PUTFRAME;	// [shoh][2013.07.02] It is necessary due to dummy chunk(a_qDTS=0xFFFFFFFF)
			}
		}

		pstInfo->usCurPos = 0;
		pstInfo->usRemainSize += a_uChunkLen;
		pstInfo->usRemainSizeHEVC = 0;

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] PutFrame Done! qChunkDTS(%10lld), qChunkPTS(%10lld), usRemainSize(%8"PRIuS"), fDuration(%5d)\n", _FUNLINE_, pstInfo->qChunkDTS, pstInfo->qChunkPTS, pstInfo->usRemainSize, (NXINT32)pstInfo->fDuration);

		return 0;
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Invalid Param.\n", _FUNLINE_);
		return -1;
	}
}

NXINT32 VideoChunkParser_GetFrame64(IN VIDEOCHUNK_HANDLE a_hChunkInfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXINT64 *a_pqFrameDTS, OUT NXINT64 *a_pqFramePTS, OUT NXBOOL *a_pbIFrame)
{
	VIDEOCHUNK_INFO *pstInfo = (VIDEOCHUNK_INFO*)a_hChunkInfo;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] usCurPos(%8"PRIuS"), usRemainSize(%8"PRIuS"), a_uGetOption(%1d)\n", _FUNLINE_, pstInfo->usCurPos, pstInfo->usRemainSize, a_uGetOption);

	if (pstInfo->usRemainSize > 0)
	{
		if ((eNEX_FF_MP4 == pstInfo->eFFType || eNEX_FF_MP4MF == pstInfo->eFFType) && pstInfo->eCodecType == eNEX_CODEC_V_H264)
		{
			NXINT32 nTopFOC = 0;
			NXINT64 qCurIFramePTS = 0;
			NXINT64 qDuration = 0;
			CHUNK_QUEUE *pData = NULL;

			if ((pstInfo->uRefNum < MIN_REF_NUM) && (VIDEOCHUNKPARSER_GETFRAME_OPTION_NORMAL == a_uGetOption))
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] (pstInfo->uRefNum < %d) Need more frames!\n", _FUNLINE_, pstInfo->uRefNum);

				return 0;
			}

			if (Queue_Dequeue(pstInfo->pChunkQueue, (NXVOID**)&pData) != eDS_RET_SUCCESS)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Queue_Dequeue Failed\n", _FUNLINE_);
				return -1;
			}
			else
			{
				*a_puFrameLen = pData->uLen;
				*a_pqFrameDTS = pData->qDTS;
				*a_pqFramePTS = pData->qPTS;
				qCurIFramePTS = pData->qCurIDRFramePTS;
				nTopFOC = pData->nTopFOC;
				SAFE_FREE(pData);
			}

			if (0 == pstInfo->stSPSInfo.pic_order_cnt_type && TRUE == CHECK_STATUS(pstInfo->uStatusFlag, STATUS_DISORDER_POC))
			{
				qDuration = (NXINT64)(((pstInfo->fAveInterval4MP4 * nTopFOC) / (NXFLOAT)pstInfo->uPOCUnit) + 0.5) + 1;

				if (qCurIFramePTS + qDuration >= 0)
				{
					*a_pqFramePTS = qCurIFramePTS + qDuration;
				}
				else
				{
					*a_pqFramePTS = 0;
				}
			}

			pstInfo->pFrameData = *a_ppFrame = pstInfo->pChunkPool + pstInfo->usCurPos;
			pstInfo->uFrameSize = *a_puFrameLen;
			pstInfo->qFrameDTS = *a_pqFrameDTS;
			pstInfo->qFramePTS = *a_pqFramePTS;
			pstInfo->usRemainSize -= pstInfo->uFrameSize;
			pstInfo->usCurPos += pstInfo->uFrameSize;
			pstInfo->eState = eVIDEOCHUNK_GETFRAME;

			*a_pbIFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, pstInfo->pFrameData, pstInfo->uFrameSize, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, TRUE);

			if (0 == VCP_LOG) // for debugging
			{
				if (FALSE == *a_pbIFrame)
				{
					if (TRUE == NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, pstInfo->pFrameData, pstInfo->uFrameSize, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, FALSE))
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [MP4] nTopFOC(%5d) DTS(%10lld) PTS(%10lld) I - Frame\n", _FUNLINE_, nTopFOC, pstInfo->qFrameDTS, pstInfo->qFramePTS);
					}
					else if (TRUE == NexCodecUtil_IsBFrame(pstInfo->eCodecType, pstInfo->pFrameData, pstInfo->uFrameSize, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, FALSE))
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [MP4] nTopFOC(%5d) DTS(%10lld) PTS(%10lld) B - Frame\n", _FUNLINE_, nTopFOC, pstInfo->qFrameDTS, pstInfo->qFramePTS);
					}
					else
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [MP4] nTopFOC(%5d) DTS(%10lld) PTS(%10lld) P - Frame\n", _FUNLINE_, nTopFOC, pstInfo->qFrameDTS, pstInfo->qFramePTS);
					}
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [MP4] nTopFOC(%5d) DTS(%10lld) PTS(%10lld) IDR - Frame\n", _FUNLINE_, nTopFOC, pstInfo->qFrameDTS, pstInfo->qFramePTS);
				}
			}

			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] GetFrame Done! [MP4] Len(%8d), Remain(%8"PRIuS"), DTS(%10lld), PTS(%10lld), IFrame(%1d), fAveInterval4MP4(%5d)\n", _FUNLINE_, *a_puFrameLen, pstInfo->usRemainSize, *a_pqFrameDTS, *a_pqFramePTS, *a_pbIFrame, (NXINT32)pstInfo->fAveInterval4MP4);

			return 0;
		}
		else if (pstInfo->eCodecType == eNEX_CODEC_V_H264)
		{
			NXUINT32 uStartCodeLen = 0, uNextStartCodeLen = 0;
			NXINT32 nStartPos = 0;

			if (NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == pstInfo->uH26xFormat)
			{
				nStartPos = _FindAnnexBStartCode(pstInfo->pChunkPool + pstInfo->usCurPos, (NXUINT32)pstInfo->usRemainSize, &uStartCodeLen);
			}
			else
			{
				nStartPos = 0;
				uStartCodeLen = pstInfo->uNALLenSize;
			}

			if (nStartPos >= 0)
			{
				NXUINT8 *pFrame = pstInfo->pChunkPool + pstInfo->usCurPos + nStartPos;

				pstInfo->usRemainSize -= nStartPos;	//[shoh][2012.09.14] Junk frame skips. This value will be vaild the next time.
				pstInfo->usCurPos += nStartPos;

				if (a_uGetOption == VIDEOCHUNKPARSER_GETFRAME_OPTION_NORMAL || a_uGetOption == VIDEOCHUNKPARSER_GETFRAME_OPTION_GETREMAIN)
				{
					NXUINT32 uCurrNALType = pFrame[uStartCodeLen] & 0x1F;
					NXUINT32 uNextNALType = 0;
					NXUSIZE usReadLen = 0;
					NXUSIZE usRemainSize = pstInfo->usRemainSize;
					NXBOOL bIncludePicFrame = FALSE;
					NXBOOL bIncludeConfig = FALSE;
					NXBOOL bFoundNextStart = FALSE;
					NXINT32 nCurrNALLen = 0;

					while (TRUE)
					{
						if (IS_AVC_PICTURE_FRAME(uCurrNALType))
						{
							NXUINT32 first_mb_in_slice = (pFrame[uStartCodeLen + usReadLen + 1] >> 7) & 0x01;

							if (1 == first_mb_in_slice)
							{
								bIncludePicFrame = TRUE;
							}
						}
						else if (FALSE == bIncludeConfig && FALSE == bIncludePicFrame) //[shoh][2013.10.28] If config is found before picture frame, the config is the beginning of the frame.
						{
							if (AVC_SEI == uCurrNALType || AVC_SPS == uCurrNALType || AVC_PPS == uCurrNALType || AVC_AUD == uCurrNALType)
							{
								bIncludeConfig = TRUE;
								pFrame += usReadLen;
								pstInfo->usRemainSize -= usReadLen;
								pstInfo->usCurPos += usReadLen;         
								usReadLen = 0;
							}
						}

						if (NEXCODECUTIL_PROPERTY_BYTESTREAMFORMAT_ANNEXB == pstInfo->uH26xFormat)
						{
							nCurrNALLen = _FindAnnexBStartCode(pFrame + usReadLen + uStartCodeLen, (NXUINT32)(usRemainSize - uStartCodeLen), &uNextStartCodeLen);
						}
						else
						{
							nCurrNALLen = _GetNALLength(pFrame + usReadLen, (NXUINT32)usRemainSize, pstInfo->uNALLenSize);
							uNextStartCodeLen = pstInfo->uNALLenSize;
						}

						if (nCurrNALLen >= 0)
						{
							usReadLen += uStartCodeLen + (NXSSIZE)nCurrNALLen;

							if (usReadLen + uNextStartCodeLen + 1 < pstInfo->usRemainSize)
							{
								uNextNALType = pFrame[usReadLen + uNextStartCodeLen] & 0x1F;
								usRemainSize = pstInfo->usRemainSize - usReadLen;

								if (TRUE == bIncludePicFrame)
								{
									if (AVC_SEI == uNextNALType || AVC_SPS == uNextNALType || AVC_PPS == uNextNALType || AVC_AUD == uNextNALType)
									{
										bFoundNextStart = TRUE;
									}
									else if (IS_AVC_PICTURE_FRAME(uNextNALType))
									{
										NXUINT32 next_first_mb_in_slice = (pFrame[usReadLen + uNextStartCodeLen + 1] >> 7) & 0x01;

										if (1 == next_first_mb_in_slice)
										{
											bFoundNextStart = TRUE;
										}
									}

									if (TRUE == bFoundNextStart)
									{
										pstInfo->usCurPos += usReadLen;
										pstInfo->usRemainSize = usRemainSize;

										*a_ppFrame = pFrame;
										*a_puFrameLen = (NXUINT32)usReadLen;
										if (TRUE == (*a_pbIFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, *a_ppFrame, *a_puFrameLen, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, TRUE)))
										{
											pstInfo->qTotalDuration = 0;
											pstInfo->uTotalChunkedCount = 0;
										}

										if (eVIDEOCHUNK_GETFRAME == pstInfo->eState || eVIDEOCHUNK_DUMMY_PUTFRAME == pstInfo->eState)
										{
											pstInfo->uChunkedCount++;

											*a_pqFrameDTS = pstInfo->qChunkDTS + (NXINT64)(pstInfo->fDuration * pstInfo->uChunkedCount);
											*a_pqFramePTS = pstInfo->qChunkPTS + (NXINT64)(pstInfo->fDuration * pstInfo->uChunkedCount);
										}
										else
										{
											*a_pqFrameDTS = pstInfo->qChunkDTS;
											*a_pqFramePTS = pstInfo->qChunkPTS;
										}

										pstInfo->pFrameData = *a_ppFrame;
										pstInfo->uFrameSize = *a_puFrameLen;

										pstInfo->qFrameDTS = *a_pqFrameDTS;
										pstInfo->qFramePTS = *a_pqFramePTS;
										pstInfo->eState = eVIDEOCHUNK_GETFRAME;

										if ((TRUE == pstInfo->bNeedPTSCal) || (pstInfo->uRefNum < MIN_REF_NUM && 0 == pstInfo->stSPSInfo.pic_order_cnt_type))
										{
											NXINT64 qCalculatedPTS = _CalculateH264PTS(pstInfo, *a_pbIFrame);

											if (pstInfo->qFramePTS != qCalculatedPTS)	//[shoh][2013.04.16] If There is a different PTS, should artificially recalculate PTS.
											{
												pstInfo->bNeedPTSCal = TRUE;
												pstInfo->qFramePTS = *a_pqFramePTS = qCalculatedPTS;
											}
										}

										NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [TS] GetFrame Done! FrameLen(%8d) DTS(%10lld), PTS(%10lld), I(%d), fDuration(%5d)\n", _FUNLINE_, *a_puFrameLen, *a_pqFrameDTS, *a_pqFramePTS,*a_pbIFrame, (NXINT32)pstInfo->fDuration);

										return 0;
									}
								}

								uStartCodeLen = uNextStartCodeLen;
								uCurrNALType = uNextNALType;
							}
							else
							{
								*a_ppFrame = NULL;
								*a_puFrameLen = 0;

								NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] Need more frames\n", _FUNLINE_);

								return 0;                               
							}
						}
						else
						{
							*a_ppFrame = NULL;
							*a_puFrameLen = 0;

							NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] Need more frames\n", _FUNLINE_);

							return 0;
						}
					}
				}
				else
				{
					*a_puFrameLen = (NXUINT32)pstInfo->usRemainSize;
					*a_ppFrame = pFrame;
					*a_pbIFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, *a_ppFrame, *a_puFrameLen, (NXVOID*)&pstInfo->uNALLenSize, pstInfo->uH26xFormat, TRUE);

					if (eVIDEOCHUNK_GETFRAME == pstInfo->eState)
					{
						pstInfo->uChunkedCount++;

						*a_pqFrameDTS = pstInfo->qChunkDTS + (NXINT64)(pstInfo->fDuration * pstInfo->uChunkedCount);
						*a_pqFramePTS = pstInfo->qChunkPTS + (NXINT64)(pstInfo->fDuration * pstInfo->uChunkedCount);
					}
					else
					{
						*a_pqFrameDTS = pstInfo->qChunkDTS;
						*a_pqFramePTS = pstInfo->qChunkPTS;
					}

					pstInfo->pFrameData = *a_ppFrame;
					pstInfo->uFrameSize = *a_puFrameLen;

					if (*a_pqFrameDTS <= pstInfo->qFrameDTS)
					{
						*a_pqFrameDTS = ++pstInfo->qFrameDTS;
					}
					pstInfo->qFrameDTS = *a_pqFrameDTS;
					pstInfo->qFramePTS = *a_pqFramePTS;
					pstInfo->usRemainSize = 0;
					pstInfo->eState = eVIDEOCHUNK_GETFRAME;

					if ((TRUE == pstInfo->bNeedPTSCal) || (pstInfo->uRefNum < MIN_REF_NUM && 0 == pstInfo->stSPSInfo.pic_order_cnt_type))
					{
						NXINT64 qCalculatedPTS = _CalculateH264PTS(pstInfo, *a_pbIFrame);

						if (pstInfo->qFramePTS != qCalculatedPTS)	//[shoh][2013.04.16] If There is a different PTS, should artificially recalculate PTS.
						{
							pstInfo->bNeedPTSCal = TRUE;
							pstInfo->qFramePTS = *a_pqFramePTS = qCalculatedPTS;
						}
					}

					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [TS] GetFrame of last frame Done!(0x%p), Len(%8d), DTS(%10lld), PTS(%10lld), Cnt(%5d), fDuration(%5d)\n", _FUNLINE_, *a_ppFrame, *a_puFrameLen, *a_pqFrameDTS, *a_pqFramePTS, pstInfo->uChunkedCount, (NXINT32)pstInfo->fDuration);

					return 0;           
				}
			}
		}
		else if ((eNEX_FF_MP4 == pstInfo->eFFType || eNEX_FF_MP4MF == pstInfo->eFFType || eNEX_FF_MPEG_TS == pstInfo->eFFType)
				 && pstInfo->eCodecType == eNEX_CODEC_V_HEVC)
		{
			CHUNK_QUEUE *pData = NULL;
			NXINT32 nTopFOC = 0;
			NXINT64 qCurIFramePTS = 0;

			if (1 < pstInfo->uMaxPOCLsb)
			{
				if ((pstInfo->uRefNum < MIN_REF_NUM) && (VIDEOCHUNKPARSER_GETFRAME_OPTION_NORMAL == a_uGetOption))
				{
					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] (pstInfo->uRefNum < %d) Need more frames!\n", _FUNLINE_, pstInfo->uRefNum);

					return 0;
				}
				else
				{
					NXINT64 qDuration = 0;
					NXUINT32 uNALLenSize = pstInfo->uNALLenSize;

					if (Queue_Dequeue(pstInfo->pChunkQueue, (NXVOID**)&pData) != eDS_RET_SUCCESS)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Queue_Dequeue Failed\n", _FUNLINE_);
						return -1;
					}
					else
					{
						*a_puFrameLen = pData->uLen;
						*a_pqFrameDTS = pData->qDTS;
						*a_pqFramePTS = pData->qPTS;
						qCurIFramePTS = pData->qCurIDRFramePTS;
						nTopFOC = pData->nTopFOC;
						SAFE_FREE(pData);
					}

					qDuration = (NXINT64)(((pstInfo->fAveInterval4MP4 * nTopFOC) / (NXFLOAT)pstInfo->uPOCUnit) + 0.5) + 1;

					if ((qDuration < 0) && ((NXINT32)qCurIFramePTS + qDuration < 0))
					{
						*a_pqFramePTS = 0;
					}
					else
					{
						*a_pqFramePTS = qCurIFramePTS + qDuration;
					}

					pstInfo->pFrameData = *a_ppFrame = pstInfo->pChunkPool + pstInfo->usCurPos;
					pstInfo->uFrameSize = *a_puFrameLen;
					pstInfo->qFrameDTS = *a_pqFrameDTS;
					pstInfo->qFramePTS = *a_pqFramePTS;
					pstInfo->usRemainSize -= pstInfo->uFrameSize;
					pstInfo->usCurPos += pstInfo->uFrameSize;
					pstInfo->eState = eVIDEOCHUNK_GETFRAME;

					*a_pbIFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, pstInfo->pFrameData, pstInfo->uFrameSize, (NXVOID*)&uNALLenSize, pstInfo->uH26xFormat, FALSE);

					nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] GetFrame Done! [MP4 HP or MP] Len(%8d), Remain(%8"PRIuS"), DTS(%10lld), PTS(%10lld), IFrame(%d), fAveInterval4MP4(%d)\n", _FUNLINE_, *a_puFrameLen, pstInfo->usRemainSize, *a_pqFrameDTS, *a_pqFramePTS, *a_pbIFrame, (NXINT32)pstInfo->fAveInterval4MP4);

					return 0;
				} 
			}
			else
			{
				NXUINT32 uNALLenSize = pstInfo->uNALLenSize;

				if (Queue_Dequeue(pstInfo->pChunkQueue, (NXVOID**)&pData) != eDS_RET_SUCCESS)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Queue_Dequeue Failed\n", _FUNLINE_);
					return -1;                  
				}
				else
				{
					*a_puFrameLen = pData->uLen;
					*a_pqFrameDTS = pData->qDTS;
					*a_pqFramePTS = pData->qPTS;
					SAFE_FREE(pData);
				}

				pstInfo->pFrameData = *a_ppFrame = pstInfo->pChunkPool + pstInfo->usCurPos;
				pstInfo->uFrameSize = *a_puFrameLen;
				pstInfo->qFrameDTS = *a_pqFrameDTS;
				pstInfo->qFramePTS = *a_pqFramePTS;
				*a_pbIFrame = NexCodecUtil_IsSeekableFrame(pstInfo->eCodecType, pstInfo->pFrameData, pstInfo->uFrameSize, (NXVOID*)&uNALLenSize, pstInfo->uH26xFormat, FALSE);

				pstInfo->usRemainSize -= pstInfo->uFrameSize;
				pstInfo->usCurPos += pstInfo->uFrameSize;
				pstInfo->eState = eVIDEOCHUNK_GETFRAME;

				nexSAL_TraceCat(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] GetFrame Done! [MP4 BP] Len(%8d), DTS(%10lld), PTS(%10lld)\n", _FUNLINE_, *a_puFrameLen, *a_pqFrameDTS, *a_pqFramePTS);

				return 0;       
			}
		}
		else
		{
			FRAMEFINDER_RETURN eRet = eFRAMEFINDER_OK;
			NXUINT8 *pTmpFrame = NULL;
			NXUSIZE usFrameOffset = 0;
			NXUSIZE usFrameSize = 0;
			NXUSIZE usCurReadPos = 0;

			eRet = _VideoFrameFinder(pstInfo, &usFrameOffset, &usFrameSize);

			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] _VideoFrameFinder Ret(%5d), usFrameOffset=%8"PRIuS", usFrameSize=%8"PRIuS"\n", _FUNLINE_, eRet, usFrameOffset, usFrameSize);

			if (eRet != eFRAMEFINDER_OK)
			{
				if (eRet == eFRAMEFINDER_NEED_MORE)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] Need more frames\n", _FUNLINE_);
					return 0;
				}
				else
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Finding video-frames failed\n", _FUNLINE_);
					return -1;
				}
			}

			usCurReadPos = usFrameOffset + usFrameSize;

			pTmpFrame = pstInfo->pChunkPool + pstInfo->usCurPos + usFrameOffset;

			if (pstInfo->usRemainSize < usCurReadPos || 0 == usFrameSize)
			{
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] Need more frames\n", _FUNLINE_);
				return 0; //Need more frames
			}
			else
			{
				pstInfo->usCurPos += usCurReadPos;
				pstInfo->usRemainSize -= usCurReadPos;

				*a_ppFrame = pTmpFrame;
				*a_puFrameLen = (NXUINT32)usFrameSize;
				*a_pbIFrame = VideoChunkParser_IsIFrame(*a_ppFrame, *a_puFrameLen, &pstInfo->stExtInfo, NULL);

				// Complement TS
				if (pstInfo->qPreChunkDTS == pstInfo->qChunkDTS)
				{
					pstInfo->uChunkedCount++;

					if (pstInfo->fDuration > 0)
					{
						*a_pqFrameDTS = pstInfo->qChunkDTS + (NXINT64)(pstInfo->fDuration * pstInfo->uChunkedCount);
						*a_pqFramePTS = pstInfo->qChunkPTS + (NXINT64)(pstInfo->fDuration * pstInfo->uChunkedCount);
					}
					else
					{
						*a_pqFrameDTS = pstInfo->qChunkDTS + (DUMMY_INTERVAL * pstInfo->uChunkedCount);
						*a_pqFramePTS = pstInfo->qChunkPTS + (DUMMY_INTERVAL * pstInfo->uChunkedCount);
					}
				}
				else
				{
					*a_pqFrameDTS = pstInfo->qPreChunkDTS = pstInfo->qChunkDTS;
					*a_pqFramePTS = pstInfo->qChunkPTS;
				}

				pstInfo->pFrameData = *a_ppFrame;
				pstInfo->uFrameSize = *a_puFrameLen;
				pstInfo->qFrameDTS = *a_pqFrameDTS;
				//pstInfo->qFramePTS = *a_pqFramePTS;
				pstInfo->qFramePTS = *a_pqFramePTS = pstInfo->qFrameDTS;
				pstInfo->eState = eVIDEOCHUNK_GETFRAME;

				if (0 == VCP_LOG) // for debugging
				{
					if (TRUE == *a_pbIFrame)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [ETC] FrameType = I-Frame \n", _FUNLINE_);
					}
					else if (TRUE == NexCodecUtil_IsBFrame(pstInfo->eCodecType, *a_ppFrame, *a_puFrameLen, NULL, 0, FALSE))
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [ETC] FrameType = B-Frame \n", _FUNLINE_);
					}
					else
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [ETC] FrameType = P-Frame \n", _FUNLINE_);
					}
				}

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_VIDEO, VCP_LOG, "[%s %d] [ETC] GetFrame Done! usCurPos=%8"PRIuS", usRemainSize=%8"PRIuS", m_uDTS=%10lld, m_uPTS=%10lld, fDuration=%5d\n", _FUNLINE_, pstInfo->usCurPos, pstInfo->usRemainSize, *a_pqFrameDTS, *a_pqFramePTS, (NXINT32)pstInfo->fDuration);
				return 0;               
			}
		}
	}

	*a_ppFrame = NULL;
	return 0;
}

NXBOOL VideoChunkParser_IsIFrame(IN NXUINT8* a_pFrame, IN NXUINT32 a_uFrameSize, IN NXVOID *a_pExtInfo, IN NXVOID *a_pUserData)
{
	VIDEOCHUNK_EXTINFO *pExtInfo = (VIDEOCHUNK_EXTINFO*)a_pExtInfo;
	NEX_CODEC_TYPE eCodecType = 0;
	NXUINT32 uFourCC = 0;
	NXBOOL bResult = FALSE;

	a_pUserData = NULL;

	NexUtil_ConvVideoCodecType(pExtInfo->m_eCodecType, pExtInfo->m_uFourCC, pExtInfo->m_eFFType, &eCodecType, &uFourCC);

	if ((eCodecType == eNEX_CODEC_V_WMV) || (eCodecType == eNEX_CODEC_V_WMV1) || (eCodecType == eNEX_CODEC_V_WMV2) 
		|| (eCodecType == eNEX_CODEC_V_WMV3) || (eCodecType == eNEX_CODEC_V_WVC1) || (eCodecType == eNEX_CODEC_A_REALA) 
		|| (eCodecType == eNEX_CODEC_A_COOK) || (eCodecType == eNEX_CODEC_A_RAAC) || (eCodecType == eNEX_CODEC_A_RACP))
	{
		if (pExtInfo->m_pExtraData)
		{
			bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, pExtInfo->m_pExtraData, 0, TRUE);
		}
		else if (pExtInfo->m_pDSI && pExtInfo->m_uDSILen > 0)
		{
			bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, pExtInfo->m_pDSI, 0,  TRUE);
		}
		else
		{
			bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, NULL, 0,  TRUE);
		}
	}
	else if (eCodecType == eNEX_CODEC_V_DIVX)
	{
		//[shoh][2011.09.06] If data is encrypted by DRM, don't try to check coded-frame.
		if (pExtInfo->m_bEncrypted)
		{
			NXUINT32 uVOPTimeIncBits = NEXCODECUTIL_TIMEINCBITS_ENCRYPTED;
			bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, (NXVOID*)&uVOPTimeIncBits, 0, TRUE);
		}
		else
		{
			NXINT32 nRet = 0;
			NEXCODECUTIL_MPEG4V_DSI_INFO DSI_Info;

			memset(&DSI_Info, 0x00, sizeof(NEXCODECUTIL_MPEG4V_DSI_INFO));

			nRet = NexCodecUtil_MPEG4V_GetDSIInfo(eCodecType, (char *)a_pFrame, a_uFrameSize, &DSI_Info);

			if (0 != nRet)
			{
				NXUINT8 *pDSI;
				NXUINT32 uDSISize = 0;

				if (pExtInfo->m_uDSILen)
				{
					pDSI = pExtInfo->m_pDSI;
					uDSISize = pExtInfo->m_uDSILen;
				}
				else if (pExtInfo->m_uExtraDSILen)
				{
					pDSI = pExtInfo->m_pExtraDSI;
					uDSISize = pExtInfo->m_uExtraDSILen;
				}
				else
				{
					pDSI = NULL;
					uDSISize = 0;
				}

				nRet = NexCodecUtil_MPEG4V_GetDSIInfo(eCodecType, (char *)pDSI, uDSISize, &DSI_Info);
			}

			if (nRet == 0)
			{
				bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, (NXVOID*)&DSI_Info.uVOPTimeIncBits, 0, TRUE);
			}
			else
			{
				NXUINT32 uVOPTimeIncBits = NEXCODECUTIL_TIMEINCBITS_INVALID;
				bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, (NXVOID*)&uVOPTimeIncBits, 0, TRUE);
			}
		}
	}
	else if (eCodecType == eNEX_CODEC_V_MJPEG)
	{
		bResult = TRUE;
	}
	else if (eCodecType == eNEX_CODEC_V_HEVC) //[shoh][2016.07.18] Not only IDR but CRA can be seekable in HEVC.
	{
		NXUINT32 uH26xFormat4Frame = NexCodecUtil_CheckByteFormat(a_pFrame, a_uFrameSize);

		bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, (NXVOID*)&pExtInfo->m_uNalHeaderLen, uH26xFormat4Frame, FALSE);
	}
	else
	{
		NXUINT32 uH26xFormat4Frame = NexCodecUtil_CheckByteFormat(a_pFrame, a_uFrameSize);

		bResult = NexCodecUtil_IsSeekableFrame(eCodecType, a_pFrame, a_uFrameSize, (NXVOID*)&pExtInfo->m_uNalHeaderLen, uH26xFormat4Frame, TRUE);
	}

	return bResult;
}

