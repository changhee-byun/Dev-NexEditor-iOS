/******************************************************************************
 * File Name   : NexAudioChunkParser.c
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

#include "NexChunkParser.h"
#include "NexCodecUtil.h"

#include <string.h>

#define ACP_LOG 3

#define AUDIOCHUNK_MATCHED_COUNT	6
#define _DEFAULT_AUDIO_FRAME_SIZE_	(5*1024)	// 5KB
#define AUDIOCHUNK_MEMORY_LIMIT 	2*1024*1024 // 2MB

#define _AC3_SAMPLES_PER_BLOCK_ 	256
#define _DTS_SAMPLES_PER_BLOCK_ 	32

// kinds of AMR FourCC
#define NORMAL_AMR_FOURCC 0x00007a22
#define NOSID_AMR_FOURCC 0x00007a21

#define NEX_AUDIO_PCM_DURATION	(5)		// Duration = Sampling Frequency / (1<<NEX_AUDIO_PCM_DURATION)

typedef enum
{
	eAUDIOCHUNK_CREATE          = 0,
	eAUDIOCHUNK_RESET           = 1,
	eAUDIOCHUNK_PUTFRAME        = 2,
	eAUDIOCHUNK_GETFRAME        = 3,
	eAUDIOCHUNK_DESTROY         = 4,
} AUDIOCHUNK_STATE;

typedef enum
{
	eAUDIOCHUNK_NOT_FOUND_LEFTOVERS = 0,
	eAUDIOCHUNK_FOUND_LEFTOVERS     = 1,
	eAUDIOCHUNK_PROCESSED_LEFTOVERS = 2,
} AUDIOCHUNK_LEFTOVERS_STATE;

typedef enum
{
	eAFF_RET_SUCCESS        = 0,
	eAFF_RET_NEED_MORE      = -1,
	eAFF_RET_FAIL           = -2,
} AFF_RETURN;

typedef struct _AUDIOCHUNK_INFO_
{
	NEX_CODEC_TYPE eCodecType;		// Codec Type
	NXUINT32 uFourCC;				// FourCC
	NEX_FILEFORMAT_TYPE eFFType;	// File Format Type

	NXUINT8 *pFrameData;			// Audio-Frame got by parsing
	NXUSIZE usFrameSize;			// Audio-Size got by parsing
	NXINT64 qFrameDTS;				// Audio-DTS got by parsing
	NXINT64 qFramePTS;				// Audio-PTS got by parsing

	NXUINT8 *pChunkPool;			// Chunk Memory Pool
	NXUSIZE usMaxChunkSize;			// Max Size of Chunk Memory Pool
	NXUSIZE usRemainSize;			// Left size that should be copied
	NXUSIZE usCurPos;				// Offset value of current position on pChunkPool
	NXBOOL bMatched;				// Check matched status or not
	AUDIOCHUNK_STATE eState;		// AudioChunk State
	NEX_WAVEFORMATEX *pWaveFormatEx; //WaveFormatEx //In case of LP mode, WaveFormatEx uses NxFFWAVEFORMATEX but NXPROTOCOLWaveFormatEx will be unified for convenience.
	NXUINT32 uSpareBlockAlign;		// To Prepare for wrong m_uBlockAlign
	AUDIOCHUNK_LEFTOVERS_STATE eLeftoversState;	// To regard leftovers of chunk as linear for previous frame
	NXINT64 qComplementTS;			// To get complementary TS from the beginning of the content filled by NULL 
	NXINT64 qDuration;				// Audio-Frame duration
	NXINT64 qTS4Null;				// To complement DTS for the beginning of the content filled by NULL
	NXUINT32 uNullCnt;				// To count Nulls from the beginning of the content filled by NULL	
	NXINT64 qChunkDTS;				// Audio Chunk DTS
	NXINT64 qChunkPTS;				// Audio Chunk PTS
	NXINT64 qPreChunkDTS;			// Audio Chunk Previous DTS
	NXINT64 qPreChunkPTS;			// Audio Chunk Previous PTS
	NXUINT32 uFrameCnt;				// Frame count in Chunk
	NXBOOL bVBR;					// VBR or not in AVI-MP3
	NXBOOL bVBRChecked;				// VBR checked or not in AVI-MP3
	NXBOOL bRenewalDTS;				// When meeting the chunk in which 0 is filled, DTS of chunk should renew.
	NXINT64 qOffsetDTS;				// Offset DTS for the beginning of NULL in AVI-MP3
	NXUINT32 uTimescale;			// Timescale value (if nTimescale is 0, it means user wants the calculated time with millisecond scale not using timescale.)
} AUDIOCHUNK_INFO;

static const NXINT32 AAC_SampleRate[16] = {
	96000, 88200, 64000,
	48000, 44100, 32000,
	24000, 22050, 16000,
	12000, 11025, 8000,
	7350 , 0, 0, 0
};

static const NXINT32 _ac3_FrameSizeTbl[38][3] = 
{
	/**/{64, 69, 96},		/**/{64, 70, 96},		/**/{80, 87, 120},		/**/{80, 88, 120},		/**/{96, 104, 144},
	/**/{96, 105, 144},		/**/{112, 121, 168},	/**/{112, 122, 168},	/**/{128, 139, 192},	/**/{128, 140, 192},
	/**/{160, 174, 240},	/**/{160, 175, 240},	/**/{192, 208, 288},	/**/{192, 209, 288},	/**/{224, 243, 336},
	/**/{224, 244, 336},	/**/{256, 278, 384},	/**/{256, 279, 384},	/**/{320, 348, 480},	/**/{320, 349, 480},
	/**/{384, 417, 576},	/**/{384, 418, 576},	/**/{448, 487, 672},	/**/{448, 488, 672},	/**/{512, 557, 768}, 
	/**/{512, 558, 768},	/**/{640, 696, 960},	/**/{640, 697, 960},	/**/{768, 835, 1152},	/**/{768, 836, 1152},
	/**/{896, 975, 1344},	/**/{896, 976, 1344},	/**/{1024, 1114, 1536},	/**/{1024, 1115, 1536},	/**/{1152, 1253, 1728},
	/**/{1152, 1254, 1728},	/**/{1280, 1393, 1920},	/**/{1280, 1394, 1920}
};

static const NXINT32 NXAMP3Bitrate[ 2 ][ 3 ][ 16 ] = 
{
	{{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,},
		{0,32,48,56, 64, 80, 96,112,128,160,192,224,256,320,384,},
		{0,32,40,48, 56, 64, 80, 96,112,128,160,192,224,256,320,}},

	{{0,32,48,56,64,80,96,112,128,144,160,176,192,224,256,},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,},
		{0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,}}
};

static const NXINT32 NXAMP3SampleRate[ 9 ] = 
{
	44100, 48000, 32000, 22050, 24000, 16000, 11025, 12000,  8000 
};

/* Header 1 byte를 포함한 Frame Size Table */
static const NXUINT8 FRAME_SIZE_AMRNB_IF2[16] = { 
	13,				/* AMR-NB 4.75 kbps */
	14,				/* AMR-NB 5.15 kbps */
	16,				/* AMR-NB 5.90 kbps */
	18,				/* AMR-NB 6.70 kbps (PDC-EFR) */
	19,				/* AMR-NB 7.40 kbps (TDMA-EFR) */
	21,				/* AMR-NB 7.95 kbps */
	26,				/* AMR-NB 10.2 kbps */
	31,				/* AMR-NB 12.2 kbps (GSM-EFR) */
	5,			   /* AMR-NB SID (Comfort Noise Frame) */
	1,			   /* AMR-NB GSM-EFR SID */
	1,			   /* AMR-NB TDMA-EFR SID */
	1,			   /* AMR-NB PDC-EFR SID */
	1, 1, 1,	   /* For future use */
	1			   /* No Data (No transmission / No reception) */
};
static const NXUINT8 FRAME_SIZE_AMRNB_IETF[16] = {
	13, 14, 16, 18, 20,  21, 27, 32, 6, 1,
	1 , 1 , 1 , 1 , 1 , 1
};
static const NXINT32 FRAME_SIZE_AMRWB_IETF[16] = {
	18,			   /* AMR-WB 6.60 kbps	 */
	24,			   /* AMR-WB 8.85 kbps	 */
	33,			   /* AMR-WB 12.65 kbps */
	37,			   /* AMR-WB 14.25 kbps */
	41,			   /* AMR-WB 15.85 kbps */
	47,			   /* AMR-WB 18.25 kbps */
	51,			   /* AMR-WB 19.85 kbps */
	59,			   /* AMR-WB 23.05 kbps */
	61,			   /* AMR-WB 23.85 kbps */
	6,			   /* AMR-WB SID (Comfort Noise Frame */
	1, 1, 1, 1,	   /* For future use	 */
	1,			   /* Speech Lost		 */
	1			   /* WBAMR Frame No Data	 */
};
static const NXINT32 BLK_SIZE_QCELP[6] = {
// blank(0), rate1/8(1), rate1/4(2), rate1/2(3), rate1(4), erasure(14)
	1,				/* Blank */
	4,				/* 1/8 rate */
	8,				/* 1/4 rate */
	17,				/* 1/2 rate */
	35,				/* full rate */
	1				/* Erasure */
};
static const NXINT32 BLK_SIZE_EVRC[6] = {
// blank, 1/8, (1/4) 1/2, 1, erasure
	1,				/* Blank */
	3,				/* 1/8 rate */
	6,				/* 1/4 rate */
	11,				/* 1/2 rate */
	23,				/* full rate */
	1				/* Erasure */
};

static AFF_RETURN _AudioFrameFinder(AUDIOCHUNK_INFO *a_pstInfo, NXUSIZE *a_pusFrameOffset)
{
	NXUSIZE i = 0;
	NXUSIZE usFrameSize=0;
	NXUINT32 uMatchCount = 0;
	NXUINT8 *pChunkFrame = a_pstInfo->pChunkPool + a_pstInfo->usCurPos;

	if ((a_pstInfo->pChunkPool == 0 || a_pstInfo->usRemainSize == 0)
		|| (a_pstInfo->eCodecType == eNEX_CODEC_A_AC3 && a_pstInfo->usRemainSize <= 5)
		|| (a_pstInfo->eCodecType == eNEX_CODEC_A_EAC3 && a_pstInfo->usRemainSize <= 5)
		|| (a_pstInfo->eCodecType == eNEX_CODEC_A_DTS && a_pstInfo->usRemainSize <= 7)
		|| (a_pstInfo->eCodecType == eNEX_CODEC_A_MP3 && a_pstInfo->usRemainSize <= 4)
		|| (a_pstInfo->eCodecType == eNEX_CODEC_A_MP2 && a_pstInfo->usRemainSize <= 4)
		|| ((a_pstInfo->eCodecType == eNEX_CODEC_A_AAC || a_pstInfo->eCodecType == eNEX_CODEC_A_AACPLUS || a_pstInfo->eCodecType == eNEX_CODEC_A_AACPLUSV2) && a_pstInfo->usRemainSize <= 5)
	   )
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] Chunk(%8"PRIuS") is small to find sync word\n", _FUNLINE_, a_pstInfo->usRemainSize);
		return eAFF_RET_NEED_MORE;
	}

	*a_pusFrameOffset = 0;
	a_pstInfo->usFrameSize = 0;

	switch (a_pstInfo->eCodecType)
	{
		case eNEX_CODEC_A_AMR:
			{
				NXUINT32 uCodecMode = 0;

				if (a_pstInfo->uFourCC == NOSID_AMR_FOURCC)
				{
					uCodecMode = pChunkFrame[0]&0x0F;

					if (uCodecMode > 15)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AMR Chunk is Abnormal[%u]\n", _FUNLINE_, a_pstInfo->pChunkPool[0]>>3);
						return eAFF_RET_FAIL;
					}

					*a_pusFrameOffset = 0;
					a_pstInfo->usFrameSize = FRAME_SIZE_AMRNB_IF2[uCodecMode];
					a_pstInfo->bMatched = TRUE;               
				}
				else
				{
					uCodecMode = (pChunkFrame[0]>>3) & 0x0F;

					if (uCodecMode > 15)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AMR Chunk is Abnormal[%u]\n", _FUNLINE_, a_pstInfo->pChunkPool[0]>>3);
						return eAFF_RET_FAIL;
					}
					*a_pusFrameOffset = 0;
					a_pstInfo->usFrameSize = FRAME_SIZE_AMRNB_IETF[uCodecMode];
					a_pstInfo->bMatched = TRUE;
				}
				return eAFF_RET_SUCCESS;
			}
			break;

		case eNEX_CODEC_A_AMRWB:
			{
				NXUINT32 uCodecMode = 0;

				uCodecMode = (pChunkFrame[0]>>3) & 0x0F;

				if (uCodecMode > 15)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AMR Chunk is Abnormal[%u]\n", _FUNLINE_, a_pstInfo->pChunkPool[0]>>3);
					return eAFF_RET_FAIL;
				}
				*a_pusFrameOffset = 0;
				a_pstInfo->usFrameSize = FRAME_SIZE_AMRWB_IETF[uCodecMode];
				a_pstInfo->bMatched = TRUE;

				return eAFF_RET_SUCCESS;
			}
			break;

		case eNEX_CODEC_A_QCELP:
			{
				NXUINT32 uCodecRate = 0;

				uCodecRate = pChunkFrame[0];

				if (uCodecRate > 4)
				{
					if (uCodecRate != 14)
					{
						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AMR Chunk is Abnormal[%u]\n", _FUNLINE_, a_pstInfo->pChunkPool[0]>>3);
						return eAFF_RET_FAIL;
					}
					uCodecRate = 5;
				}
				*a_pusFrameOffset = 0;
				a_pstInfo->usFrameSize = BLK_SIZE_QCELP[uCodecRate];
				a_pstInfo->bMatched = TRUE;

				return eAFF_RET_SUCCESS;
			}
			break;

		case eNEX_CODEC_A_EVRC:
			{
				NXUINT32 uCodecRate = 0;

				uCodecRate = pChunkFrame[0];

				if (uCodecRate > 4)
				{
					NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] AMR Chunk is Abnormal[%u]\n", _FUNLINE_, a_pstInfo->pChunkPool[0]>>3);
					return eAFF_RET_FAIL;
				}
				*a_pusFrameOffset = 0;
				a_pstInfo->usFrameSize = BLK_SIZE_EVRC[uCodecRate];
				a_pstInfo->bMatched = TRUE;

				return eAFF_RET_SUCCESS;
			}
			break;

		case eNEX_CODEC_A_AC3:
		case eNEX_CODEC_A_EAC3:
			{
				NXUINT32 uFsCod = 0;
				NXUINT32 uFrmSizeCod = 0;
				NXUINT32 frame_type = 0 ;

				for (i = 0; i < (a_pstInfo->usRemainSize-5); i++)
				{
					NXUINT32 uBSID = 0;

					if (pChunkFrame[i] != 0x0b || pChunkFrame[i+1] != 0x77)
					{
						uMatchCount = 0;
						continue;
					}

					uBSID = (pChunkFrame[i+5] & 0xF8) >> 3;

					if (uBSID <= 10) // AC3
					{
						//NEXSAL_TRACE("AC3 \n");
						uFsCod = (pChunkFrame[i+ 4] & 0xc0) >> 6;
						if (uFsCod == 3)
						{
							// nFsCod 가 3(11b) 이면 Reserved....
							uMatchCount = 0;
							continue;
						}

						uFrmSizeCod = pChunkFrame[i + 4] & 0x3f;
						if (uFrmSizeCod > 37)
						{
							// nFrmSizeCod 가 37 보다 크면 Error
							uMatchCount = 0;
							continue;
						}

						usFrameSize = _ac3_FrameSizeTbl[uFrmSizeCod][uFsCod] << 1;        

						if (a_pstInfo->bMatched == TRUE)
						{
							*a_pusFrameOffset = i;
							a_pstInfo->usFrameSize = usFrameSize;
							return eAFF_RET_SUCCESS;
						}
						else
						{
							if (uMatchCount == 0)
							{
								*a_pusFrameOffset = i;
								a_pstInfo->usFrameSize = usFrameSize;
							}

							if (++uMatchCount >= (NXUINT32)AUDIOCHUNK_MATCHED_COUNT)
							{
								a_pstInfo->bMatched = TRUE;
								return eAFF_RET_SUCCESS;
							}
						}

						i += usFrameSize - 1;
					}
					else if (uBSID >= 11 && uBSID <= 16) // E-AC3
					{
						usFrameSize = ((((pChunkFrame[i+2] & 0x07) << 8) | (pChunkFrame[i+3] & 0xFF)) + 1) << 1;

						if (usFrameSize+3  < a_pstInfo->usRemainSize)
						{
							if (pChunkFrame[usFrameSize] == 0x0b && pChunkFrame[usFrameSize+1] == 0x77)
							{
								frame_type  = (pChunkFrame[usFrameSize+2] & 0xC0) >> 6; 
								if (frame_type == 1) //Dependent frame type
								{
									usFrameSize += ((((pChunkFrame[usFrameSize+2] & 0x07) << 8) | (pChunkFrame[usFrameSize+3] & 0xFF)) + 1) << 1;;
								}
							}
						}

						if (usFrameSize < 7)
						{
							uMatchCount = 0;
							continue;
						}

						if (a_pstInfo->bMatched == TRUE)
						{
							*a_pusFrameOffset = i;
							a_pstInfo->usFrameSize = usFrameSize;
							return eAFF_RET_SUCCESS;
						}
						else
						{
							if (uMatchCount == 0)
							{
								*a_pusFrameOffset = i;
								a_pstInfo->usFrameSize = usFrameSize;
							}

							if (++uMatchCount >= (NXUINT32)AUDIOCHUNK_MATCHED_COUNT)
							{
								a_pstInfo->bMatched = TRUE;
								return eAFF_RET_SUCCESS;
							}
						}

						i += usFrameSize - 1;
					}
					else// (bsid == 9 || bsid == 10)	// error
					{
						uMatchCount = 0;
						continue;
					}

				}
			}
			break;

			//#define DTS_SYNCWORD_CORE				 0x7ffe8001
			//#define DTS_SYNCWORD_SUBSTREAM		 0x64582025
			//#deifne DTS_SYNCWORD_SUBSTREAM_CORE2   0x02b09261
		case eNEX_CODEC_A_DTS:
			{
				NXUSIZE j = 0;
				NXBOOL  bCoreHeaderFlag = FALSE;

				for (i = 0; i<(a_pstInfo->usRemainSize-7); i++)
				{
					if ((NXUINT8)pChunkFrame[i] != 0x7f || (NXUINT8)pChunkFrame[i+1] != 0xfe || (NXUINT8)pChunkFrame[i+2] != 0x80 || (NXUINT8)pChunkFrame[i+3] != 0x01)
					{
						if ((NXUINT8)pChunkFrame[i] == 0)
						{
							continue;
						}
						else if (((NXUINT8)pChunkFrame[i] == 0x64 && (NXUINT8)pChunkFrame[i+1] == 0x58 && (NXUINT8)pChunkFrame[i+2] == 0x20 && (NXUINT8)pChunkFrame[i+3] == 0x25) ||
								 ((NXUINT8)pChunkFrame[i] == 0x02 && (NXUINT8)pChunkFrame[i+1] == 0xb0 && (NXUINT8)pChunkFrame[i+2] == 0x92 && (NXUINT8)pChunkFrame[i+3] == 0x61))
						{
							// UserDefinedBits = ExtractBits(8), nExtSSIndex = ExtractBits(2)
							if ((pChunkFrame[i+5] >> 5) & 1) // bHeaderSizeType = ExtractBits(1)
							{
								// nuBits4Header = 12, nuBits4ExSSFsize = 20
								usFrameSize = (NXUINT8)pChunkFrame[i+6] & 0x01;
								usFrameSize <<= 8;
								usFrameSize += (NXUINT8)pChunkFrame[i+7];
								usFrameSize <<= 8;
								usFrameSize += (NXUINT8)pChunkFrame[i+8];
								usFrameSize <<= 3;
								usFrameSize += ((NXUINT8)pChunkFrame[i+9] >> 5) & 0x07;
								usFrameSize++;
							}
							else
							{ // nuBits4Header = 8, nuBits4ExSSFsize = 16
								usFrameSize = (NXUINT8)pChunkFrame[i+6] & 0x1F;
								usFrameSize <<= 8;
								usFrameSize += (NXUINT8)pChunkFrame[i+7];
								usFrameSize <<= 3;
								usFrameSize += ((NXUINT8)pChunkFrame[i+8] >> 5) & 0x07;
								usFrameSize++;
							}
						}
						else
						{
							continue;
						}

						if (bCoreHeaderFlag == TRUE)
						{
							if (usFrameSize)
							{
								*a_pusFrameOffset = 0;
								a_pstInfo->usFrameSize += usFrameSize;
								i += usFrameSize-1;
							}
							else
							{
								*a_pusFrameOffset = i;
							}

							if (i+1 >= a_pstInfo->usRemainSize-7)
							{
								return eAFF_RET_SUCCESS;
							}
							bCoreHeaderFlag = FALSE;
						}
					}
					else
					{
						if ((NXUINT8)pChunkFrame[i] == 0x7f && (NXUINT8)pChunkFrame[i+1] == 0xfe && (NXUINT8)pChunkFrame[i+2] == 0x80 && (NXUINT8)pChunkFrame[i+3] == 0x01)
						{
							usFrameSize = ((NXUINT8)pChunkFrame[i + 5] & 0x03);
							usFrameSize <<= 8;
							usFrameSize += ((NXUINT8)pChunkFrame[i + 6]);
							usFrameSize <<= 4;
							usFrameSize += (((NXUINT8)pChunkFrame[i + 7] >> 4) & 0x0f);
							usFrameSize++;

							bCoreHeaderFlag = TRUE;
						}
						else
						{
							continue;
						}
					}

					for (j = i + usFrameSize; j < a_pstInfo->usRemainSize; j++)
					{
						if ((NXUINT8)pChunkFrame[j] == 0x00)//|| (unsigned char)pSource->m_pChunkFrame[j] == 0xFF && (j >= pSource->m_uChunkRemainSize - 4 || *(NXUINT32 *)(void *)(&pSource->m_pChunkFrame[i]) != *(NXUINT32 *)(void *)(&pSource->m_pChunkFrame[j])))
						{
							usFrameSize++;
						}
						else
						{
							break;
						}
					}

					if (usFrameSize > a_pstInfo->usRemainSize)
					{
						return eAFF_RET_NEED_MORE;
					}

					// additional check for next sync
					if (i + usFrameSize < a_pstInfo->usRemainSize && pChunkFrame[i+usFrameSize] != pChunkFrame[i] && pChunkFrame[i+usFrameSize] != 0)
					{
						if ((pChunkFrame[i+usFrameSize] != 0x64/* && pChunkFrame[i+usFrameSize] != 0x58*/) &&
							(pChunkFrame[i+usFrameSize] != 0x02/* && pChunkFrame[i+usFrameSize] != 0xb0*/))
						{ // additional check for dts hd
							usFrameSize--;
						}
					}

					if (a_pstInfo->bMatched == TRUE)
					{
						*a_pusFrameOffset = i;
						a_pstInfo->usFrameSize = usFrameSize;
						return eAFF_RET_SUCCESS;
					}
					else
					{
						if (uMatchCount == 0)
						{
							*a_pusFrameOffset = i;
							a_pstInfo->usFrameSize = usFrameSize;
						}

						if (++uMatchCount >= (NXUINT32)AUDIOCHUNK_MATCHED_COUNT)
						{
							a_pstInfo->bMatched = TRUE;
							return eAFF_RET_SUCCESS;
						}
					}

					i += usFrameSize - 1;
				}
			}
			break;

		case eNEX_CODEC_A_MP3:
		case eNEX_CODEC_A_MP2:
			{
				NXUSIZE i = 0; 
				NXINT32 nLay = 0;
				NXUINT32 nID = 0; 
				NXUINT32 uLSF = 0;
				NXUINT32 uSamplingFreq = 0;
				NXUINT32 uPadding = 0;
				NXBOOL bMPEG25 = FALSE;
				NXUINT32 ch = 0;
				NXUINT32 uBitrateIndex = 0;
				NXUINT32 uBitRateIdx1st = 0;
				NXBOOL bNullCheck = TRUE;

				a_pstInfo->bRenewalDTS = FALSE;

				for (i = 0; i < a_pstInfo->usRemainSize - 4; i++)
				{
					//
					// Check MP3 header
					//
					if ((NXUINT8)pChunkFrame[i] != 0xFF)
					{
						if (a_pstInfo->qTS4Null == INVALID_VALUE && bNullCheck == TRUE)
						{
							if (pChunkFrame[i] == 0x00)
							{
								a_pstInfo->uNullCnt++;
								a_pstInfo->bRenewalDTS = TRUE; //[shoh][2011.08.22] If TS for Null is bigger than 0, DTS shuold be renewed regardless of state.
							}
							else //[shoh][2011.09.03] If the beginning of MP3 is not 0, don't calculate Null CTS.
							{
								a_pstInfo->uNullCnt = 0;                          
								bNullCheck = FALSE;
							}
						}

						uMatchCount = 0;
						continue;
					}
					else
					{
						a_pstInfo->bRenewalDTS = FALSE;
					}

					ch = pChunkFrame[ i + 1 ];
					if ((ch & 0x000000E0) != 0x000000E0)
					{
						uMatchCount = 0;
						continue;
					}

					nID = ch & 0x00000018;
					nLay = 4 - ((ch >> 1) & 3);
					if ((nLay != 1) && (nLay != 2) && (nLay != 3)) // Layer 1, Layer 2, Layer 3만 지원하록 수정
					{
						uMatchCount = 0;
						continue;
					}

					ch = pChunkFrame[ i + 2 ];
					uBitrateIndex = (ch >> 4) & 15;

					if (uBitrateIndex == 15 || uBitrateIndex == 0)
					{
						uMatchCount = 0;
						continue;
					}

					if (nID & 0x00000010)
					{
						uLSF = (nID & 0x00000008) ? 0 : 1;
						bMPEG25 = FALSE;
					}
					else
					{
						uLSF = 1;
						bMPEG25 = TRUE;
					}

					uSamplingFreq = (ch >> 2) & 3;

					if (uSamplingFreq == 3)
					{
						uMatchCount = 0;
						continue;
					}

					if (bMPEG25)
					{
						uSamplingFreq += 6;
					}
					else
					{
						uSamplingFreq += uLSF * 3;
					}

					uPadding = (ch >> 1) & 1;

					switch (nLay)
					{
						case 1:
							usFrameSize  = (NXUINT32)NXAMP3Bitrate[uLSF][0][uBitrateIndex] * 12000;
							usFrameSize /= NXAMP3SampleRate[uSamplingFreq];
							usFrameSize  = (usFrameSize + uPadding) << 2;
							break;
						case 2:
							usFrameSize = (NXUINT32)NXAMP3Bitrate[uLSF][1][uBitrateIndex] * 144000;
							usFrameSize /= NXAMP3SampleRate[uSamplingFreq];
							usFrameSize += uPadding;
							break;
						case 3:
							usFrameSize  = (NXUINT32)NXAMP3Bitrate[uLSF][2][uBitrateIndex] * 144000;
							usFrameSize /= NXAMP3SampleRate[uSamplingFreq] << uLSF;
							usFrameSize += uPadding;
					}

					if (a_pstInfo->bMatched == TRUE)
					{
						//Compare to first mp3 header and second mp3 header
						//Not match : find correct header 
						if ((i+usFrameSize+2) < (a_pstInfo->usRemainSize))
						{
							if ((pChunkFrame[i+usFrameSize] == 0xFF) && ((pChunkFrame[i+usFrameSize+1]&0xE0) == 0xE0))
							{
								if ((pChunkFrame[i+1] != pChunkFrame[i+usFrameSize+1]) 
									||  (((pChunkFrame[i + 2] >> 2) & 3) != ((pChunkFrame[i + usFrameSize + 2] >> 2) & 3)))
								{
									uMatchCount = 0;
									continue;
								}
							}
							else
							{
								uMatchCount = 0;
								continue;
							}
						}

						*a_pusFrameOffset = i;
						a_pstInfo->usFrameSize = usFrameSize;
						return eAFF_RET_SUCCESS;
					}
					else
					{
						if (uMatchCount == 0)
						{
							*a_pusFrameOffset = i;
							a_pstInfo->usFrameSize = usFrameSize;
						}

						if ((a_pstInfo->usRemainSize - 4) < (i + usFrameSize))
						{
							*a_pusFrameOffset = 0;
							a_pstInfo->usFrameSize = 0;
							return eAFF_RET_NEED_MORE;
						}

						//Compare to first mp3 header and second mp3 header
						//Not match : find correct header 
						if ((i+usFrameSize+2) < (a_pstInfo->usRemainSize))
						{
							if ((pChunkFrame[i+usFrameSize] == 0xFF) && ((pChunkFrame[i+usFrameSize+1]&0xE0) == 0xE0))
							{
								if ((pChunkFrame[i+1] != pChunkFrame[i+usFrameSize+1]) 
									||  (((pChunkFrame[i + 2] >> 2) & 3) != ((pChunkFrame[i + usFrameSize + 2] >> 2) & 3)))
								{
									uMatchCount = 0;
									continue;
								}
							}
							else
							{
								uMatchCount = 0;
								continue;
							}
						}

						if (!a_pstInfo->bVBRChecked)
						{
							if (!uMatchCount)
							{
								uBitRateIdx1st = uBitrateIndex;
							}

							if (uBitRateIdx1st != uBitrateIndex)
							{
								a_pstInfo->bVBR = TRUE;
								a_pstInfo->bVBRChecked = TRUE;
							}
						}

						if (++uMatchCount >= (NXUINT32)AUDIOCHUNK_MATCHED_COUNT)
						{
							a_pstInfo->bMatched = TRUE;
							return eAFF_RET_SUCCESS;
						}
					}

					i += usFrameSize - 1;
				}
			}
			break;

		case eNEX_CODEC_A_PCM_S16LE:
		case eNEX_CODEC_A_PCM_S16BE:
		case eNEX_CODEC_A_PCM_RAW:
		case eNEX_CODEC_A_PCM_FL32LE:
		case eNEX_CODEC_A_PCM_FL32BE:
		case eNEX_CODEC_A_PCM_FL64LE:
		case eNEX_CODEC_A_PCM_FL64BE:
		case eNEX_CODEC_A_PCM_IN24LE:
		case eNEX_CODEC_A_PCM_IN24BE:
		case eNEX_CODEC_A_PCM_IN32LE:
		case eNEX_CODEC_A_PCM_IN32BE:
		case eNEX_CODEC_A_PCM_LPCMLE:
		case eNEX_CODEC_A_PCM_LPCMBE:       
			{
				*a_pusFrameOffset = 0;

				if (a_pstInfo->pWaveFormatEx && (a_pstInfo->pWaveFormatEx->nSamplePerSecond > 0) && (a_pstInfo->pWaveFormatEx->nBitPerSample > 0) && (a_pstInfo->pWaveFormatEx->nChannel > 0))
				{
					a_pstInfo->usFrameSize = (a_pstInfo->pWaveFormatEx->nSamplePerSecond >> NEX_AUDIO_PCM_DURATION) * (a_pstInfo->pWaveFormatEx->nBitPerSample >> 3) * a_pstInfo->pWaveFormatEx->nChannel;
				}
				else
				{
					a_pstInfo->usFrameSize = 8064;	//for 8, 16, 24, 32 bit
				}           

				a_pstInfo->bMatched = TRUE;
				return eAFF_RET_SUCCESS;
			}
			break;

		case eNEX_CODEC_A_ADPCM_MS:
		case eNEX_CODEC_A_ADPCM_IMA_WAV:
		case eNEX_CODEC_A_ADPCM_SWF:
			{
				*a_pusFrameOffset = 0;

				if (a_pstInfo->pWaveFormatEx)
				{
					a_pstInfo->usFrameSize = a_pstInfo->pWaveFormatEx->nBlockAlignment;
				}
				else
				{
					a_pstInfo->usFrameSize = 0;
					return eAFF_RET_NEED_MORE;
				}
				a_pstInfo->bMatched = TRUE;
				return eAFF_RET_SUCCESS;
			}
			break;

		case eNEX_CODEC_A_AAC:
		case eNEX_CODEC_A_AACPLUS:
		case eNEX_CODEC_A_AACPLUSV2:
			{
				NXUINT32 uFsCod = 0;
				NXUINT32 uChannels = 0;
				NXUINT32 auBuffer[3];
				NXUINT8 *pTmp = NULL;

				for (i = 0; i<(a_pstInfo->usRemainSize-5); i++)
				{
					if (((NXUINT8)pChunkFrame[i] != 0xff) || (((NXUINT8)pChunkFrame[i+1] & 0xf6) != 0xf0))
					{
						uMatchCount = 0;
						continue;
					}
					pTmp = (NXUINT8*)pChunkFrame + i;
					//nFsCod = ((pTmp[2] >> 2) & 0x0F);

					uFsCod = ((pTmp[2] & 0x3C) >> 2);
					if (uFsCod > 13)
					{
						uMatchCount = 0;
						continue;
					}

					uChannels = (((pTmp[2] & 0x01)<<2) | ((pTmp[3] & 0xC0) >> 6));

					if (uChannels < 1)
					{
						NXINT32 nSynID;
						NXBOOL bCRC = (pTmp[1] & 0x01);

						if (bCRC)
						{
							nSynID = pTmp[7] & 0xe0;
						}
						else
						{
							nSynID = pTmp[9] & 0xe0;
						}

						if ((nSynID != 0) && (nSynID != 1))
						{
							uMatchCount = 0;
							continue;
						}
					}

					auBuffer[0] = (NXUINT32)pTmp[3];
					auBuffer[1] = (NXUINT32)pTmp[4];
					auBuffer[2] = (NXUINT32)pTmp[5];

					usFrameSize = ((auBuffer[0] & 0x03) << 11) + (auBuffer[1] << 3) +((auBuffer[2]>>5) & 0x07);

					if (usFrameSize < 11)
					{
						uMatchCount = 0;
						continue;
					}

					if (a_pstInfo->bMatched == TRUE)
					{
						*a_pusFrameOffset = i;
						a_pstInfo->usFrameSize = usFrameSize;

						if (a_pstInfo->pWaveFormatEx)
						{
							if ((NXINT32)a_pstInfo->pWaveFormatEx->nSamplePerSecond != AAC_SampleRate[uFsCod])
							{
								uMatchCount = 0;
								continue;
							}

							if (a_pstInfo->pWaveFormatEx->nChannel != uChannels)
							{
								uMatchCount = 0;
								continue;
							}
						}

						if ((i+usFrameSize+2) < (a_pstInfo->usRemainSize))
						{
							if ((pChunkFrame[i+usFrameSize] == 0xFF) && ((pChunkFrame[i+usFrameSize+1]&0xF6) == 0xF0))
							{
								if ((pChunkFrame[i+1] != pChunkFrame[i+usFrameSize+1]) 
									||  (((pChunkFrame[ i + 2 ] >> 2) & 3) != ((pChunkFrame[ i + usFrameSize + 2 ] >> 2) & 3)))
								{
									uMatchCount = 0;
									continue;
								}
							}
							else
							{
								uMatchCount = 0;
								continue;
							}
						}

						return eAFF_RET_SUCCESS;
					}
					else
					{
						if (uMatchCount == 0)
						{
							*a_pusFrameOffset = i;
							a_pstInfo->usFrameSize = usFrameSize;
						}

						if (a_pstInfo->pWaveFormatEx)
						{
							if ((NXINT32)a_pstInfo->pWaveFormatEx->nSamplePerSecond != AAC_SampleRate[uFsCod])
							{
								uMatchCount = 0;
								continue;
							}

							if (a_pstInfo->pWaveFormatEx->nChannel != uChannels)
							{
								uMatchCount = 0;
								continue;
							}
						}

						if ((i+usFrameSize+2) < (a_pstInfo->usRemainSize))
						{
							if ((pChunkFrame[i+usFrameSize] == 0xFF) && ((pChunkFrame[i+usFrameSize+1]&0xF6) == 0xF0))
							{
								if ((pChunkFrame[i+1] != pChunkFrame[i+usFrameSize+1]) 
									||  (((pChunkFrame[ i + 2 ] >> 2) & 3) != ((pChunkFrame[ i + usFrameSize + 2 ] >> 2) & 3)))
								{
									uMatchCount = 0;
									continue;
								}
							}
							else
							{
								uMatchCount = 0;
								continue;
							}
						}

						if (++uMatchCount >= (NXUINT32)AUDIOCHUNK_MATCHED_COUNT)
						{
							a_pstInfo->bMatched = TRUE;
							return eAFF_RET_SUCCESS;
						}
					}

					i += usFrameSize - 1;
				}
			}
			break;

		default:
			*a_pusFrameOffset = 0;
			a_pstInfo->usFrameSize = a_pstInfo->usRemainSize;

			if (a_pstInfo->bMatched == FALSE)
			{
				a_pstInfo->bMatched = TRUE;
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] Not Supported AudioChunk format, so this frame is regarded as normal frame.\n", _FUNLINE_);
			}
			return eAFF_RET_FAIL;
	}

	if ((*a_pusFrameOffset == 0) && (a_pstInfo->usFrameSize == 0))
	{
		a_pstInfo->bMatched = FALSE;
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] Not found Sync Word\n", _FUNLINE_);
		return eAFF_RET_NEED_MORE; //handle to file system error
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] found Sync Word, but not matched\n", _FUNLINE_);
	}

	return eAFF_RET_NEED_MORE;
}

static NXINT64 _GetMPEGAudioInterval(AUDIOCHUNK_INFO *a_pstInfo, NXUINT32 *a_uSampleRate)
{
	const NXUINT32 uSampleNumTbl[2][4]={/* Layer   x	I	  II		III */
		{0, 384, 1152, 1152},	/* MPEG-1	 */
		{0, 384, 1152, 576}		/* MPEG-2(.5) */
	};
	NXUINT32 uSampleTbl[3] = {44100, 48000, 32000};
	NXUINT32 uTmp = a_pstInfo->pFrameData[1] & 0x1e;  
	NXUINT32 uLayer = (4 - ((uTmp>>1) & 0x3));
	NXUINT32 uMPEGVer = (4 - ((uTmp>>3) & 0x3));
	NXUINT32 uSampleRate = *a_uSampleRate = (uSampleTbl[(a_pstInfo->pFrameData[2]&0xc)>>2]) >> (uMPEGVer == 4? 2 : (uMPEGVer-1));
	NXUINT32 uPCMSize;

	if (uMPEGVer == 1)
	{
		uMPEGVer = 0;
	}
	else
	{
		uMPEGVer = 1;
	}

	uPCMSize = uSampleNumTbl[uMPEGVer][uLayer];

	return(NXINT64)(uPCMSize * (a_pstInfo->uTimescale/(NXDOUBLE)uSampleRate));
}

static NXINT64 _AudioChunkParser_GetAudioDuration(AUDIOCHUNK_INFO *a_pstInfo)
{
	NXINT64 qDuration = 0;
	NXUINT32 uSampleRate = 0;

	switch (a_pstInfo->eCodecType)
	{
		case eNEX_CODEC_A_AMR:
		case eNEX_CODEC_A_AMRWB:
		case eNEX_CODEC_A_QCELP:
		case eNEX_CODEC_A_EVRC:
			qDuration = (NXINT64)(2e-2 * a_pstInfo->uTimescale + 0.5);
			break;

		case eNEX_CODEC_A_MP2:
		case eNEX_CODEC_A_MP3:
			qDuration = _GetMPEGAudioInterval(a_pstInfo, &uSampleRate);
			break;

		case eNEX_CODEC_A_AC3:
		case eNEX_CODEC_A_EAC3:
			{
				const NXUINT32 uSampleRateTBL[8] = {48000, 44100, 32000, 0};
				const NXUINT32 uNumBlocksTBL[4]  = {1, 2, 3, 6};
				NXUINT32 uNumBlocks = 0;
				NXUINT32 uBSID = (a_pstInfo->pFrameData[5] & 0xF8) >> 3;

				if (uBSID <= 8)	// AC3
				{
					// AC3 always have 6 audio blocks, each block has 256 PCM samples per channel
					uSampleRate = uSampleRateTBL[(a_pstInfo->pFrameData[4] & 0xC0) >> 6];
					uNumBlocks = 6;
				}
				else if (uBSID == 9)
				{
					uSampleRate = uSampleRateTBL[(a_pstInfo->pFrameData[4] & 0xC0) >> 6] >> 1;
					uNumBlocks = 6;
				}
				else if (uBSID == 10)
				{
					uSampleRate = uSampleRateTBL[(a_pstInfo->pFrameData[4] & 0xC0) >> 6] >> 2;
					uNumBlocks = 6;
				}
				else if (uBSID >= 11 && uBSID <= 16) // E-AC3
				{
					uSampleRate = (a_pstInfo->pFrameData[4] & 0xC0) >> 6;

					if (uSampleRate == 3)
					{
						uNumBlocks = 6;
						uSampleRate = uSampleRateTBL[(a_pstInfo->pFrameData[4] & 0x30) >> 4] >> 1;
					}
					else
					{
						uNumBlocks  = uNumBlocksTBL[(a_pstInfo->pFrameData[4] & 0x30) >> 4];
						uSampleRate = uSampleRateTBL[uSampleRate];
					}
				}

				if (uSampleRate)
				{
					qDuration = (NXINT64)(uNumBlocks * _AC3_SAMPLES_PER_BLOCK_ * (a_pstInfo->uTimescale/(NXDOUBLE)uSampleRate));
				}
				else
				{
					qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
				}
			}
			break;

		case eNEX_CODEC_A_DTS:
			{
				NXUINT32 bStaticFieldsPresent = 0;

				if ((NXUINT8)a_pstInfo->pFrameData[0] != 0x7f || (NXUINT8)a_pstInfo->pFrameData[1] != 0xfe || (NXUINT8)a_pstInfo->pFrameData[2] != 0x80 || (NXUINT8)a_pstInfo->pFrameData[3] != 0x01)
				{
					NXUINT32 bHeaderSizeType = 0, nuExSSFrameDurationCode = 0, nuRefClockCode = 0;
					const NXUINT32 uSampleRateTBL[4] = {32000, 44100, 48000, 0};

					bHeaderSizeType = (a_pstInfo->pFrameData[5] >> 5) & 1;
					if (bHeaderSizeType == 0)
					{
						//nuBits4Header = 8; //nuBits4ExSSFsize = 16;
						bStaticFieldsPresent = (a_pstInfo->pFrameData[8] >> 4) & 1;
						if (bStaticFieldsPresent == 1)
						{
							nuRefClockCode = (a_pstInfo->pFrameData[8] & 0x0a) >> 2;
							uSampleRate = uSampleRateTBL[nuRefClockCode];

							if (uSampleRate)
							{
								nuExSSFrameDurationCode = 512 * ((((a_pstInfo->pFrameData[8] & 0x03)<<1) | (a_pstInfo->pFrameData[9]>>7)) +1);
								qDuration = (NXINT64)(nuExSSFrameDurationCode * (a_pstInfo->uTimescale/(NXDOUBLE)uSampleRate));
							}
							else
							{
								qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
							}
						}
						else
						{
							qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
						}
					}
					else
					{
						//nuBits4Header = 12; //nuBits4ExSSFsize = 20;
						bStaticFieldsPresent = (a_pstInfo->pFrameData[9] >> 4) & 1;
						if (bStaticFieldsPresent == 1)
						{
							nuRefClockCode = (a_pstInfo->pFrameData[9] & 0x0a) >> 2;
							uSampleRate = uSampleRateTBL[nuRefClockCode];

							if (uSampleRate)
							{
								nuExSSFrameDurationCode = 512 * ((((a_pstInfo->pFrameData[9] & 0x03)<<1) | (a_pstInfo->pFrameData[10]>>7)) +1);
								qDuration = (NXINT64)(nuExSSFrameDurationCode * (a_pstInfo->uTimescale/(NXDOUBLE)uSampleRate));
							}
							else
							{
								qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
							}
						}
						else
						{
							qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
						}
					}
				}
				else
				{
					NXUINT32 uSampleRate, uNumBlocks;

					const NXUINT32 uSampleRateTBL[16] = {0, 8000, 16000, 32000, 0, 0, 
						11025, 22050, 44100, 0, 0,
						12000, 24000, 48000, 0, 0
					};

					uNumBlocks = (((a_pstInfo->pFrameData[4] & 0x1) << 7) | ((a_pstInfo->pFrameData[5] & 0xFC) >> 2)) + 1;
					if ((1 <= uNumBlocks) && (uNumBlocks <= 5))
					{
						qDuration = 0;
						break;
					}

					uSampleRate = uSampleRateTBL[ ((a_pstInfo->pFrameData[8] & 0x3C) >> 2) ];

					if (uSampleRate)
					{
						qDuration = (NXINT64)(uNumBlocks * _DTS_SAMPLES_PER_BLOCK_ * (a_pstInfo->uTimescale/(NXDOUBLE)uSampleRate));
					}
					else
					{
						qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
					}
				}
			}
			break;

		case eNEX_CODEC_A_PCM_S16LE:
		case eNEX_CODEC_A_PCM_S16BE:
		case eNEX_CODEC_A_PCM_RAW:
		case eNEX_CODEC_A_PCM_FL32LE:
		case eNEX_CODEC_A_PCM_FL32BE:
		case eNEX_CODEC_A_PCM_FL64LE:
		case eNEX_CODEC_A_PCM_FL64BE:
		case eNEX_CODEC_A_PCM_IN24LE:
		case eNEX_CODEC_A_PCM_IN24BE:
		case eNEX_CODEC_A_PCM_IN32LE:
		case eNEX_CODEC_A_PCM_IN32BE:
		case eNEX_CODEC_A_PCM_LPCMLE:
		case eNEX_CODEC_A_PCM_LPCMBE:   
			{
				// 6144 for 8, 16, 24, 32 bit
				NEX_WAVEFORMATEX *pWaveFormatEx = a_pstInfo->pWaveFormatEx;

				if (pWaveFormatEx && (0 != pWaveFormatEx->nBitPerSample) && (0 != pWaveFormatEx->nChannel) && (0 != pWaveFormatEx->nSamplePerSecond))
				{
					qDuration = (NXINT64)((a_pstInfo->usFrameSize / (pWaveFormatEx->nBitPerSample>>3) / pWaveFormatEx->nChannel)
										  * (a_pstInfo->uTimescale / (NXDOUBLE)pWaveFormatEx->nSamplePerSecond));
				}
				else
				{
					qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
				}
			}
			break;

		case eNEX_CODEC_A_ADPCM_MS:
			{
				NEX_WAVEFORMATEX *pWaveFormatEx = a_pstInfo->pWaveFormatEx;
				NXUINT32 uBitsPerSample;

				if (pWaveFormatEx && (0 != pWaveFormatEx->nSamplePerSecond))
				{
					if (pWaveFormatEx->nBitPerSample == 0)
					{
						uBitsPerSample = 4;
					}
					else
					{
						uBitsPerSample = pWaveFormatEx->nBitPerSample;
					}

					qDuration = (NXINT64)(((((pWaveFormatEx->nBlockAlignment - (7 * pWaveFormatEx->nChannel)) * 8) / (uBitsPerSample * pWaveFormatEx->nChannel)) + 2)
										  * (a_pstInfo->uTimescale / (NXDOUBLE)pWaveFormatEx->nSamplePerSecond));
				}
				else
				{
					qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
				}
			}
			break;

		case eNEX_CODEC_A_ADPCM_IMA_WAV:
			{
				NEX_WAVEFORMATEX *pWaveFormatEx = a_pstInfo->pWaveFormatEx;
				NXUINT32 uBitsPerSample;

				if (pWaveFormatEx && (0 != pWaveFormatEx->nChannel) && (0 != pWaveFormatEx->nSamplePerSecond))
				{
					if (pWaveFormatEx->nBitPerSample == 0)
					{
						uBitsPerSample = 4;
					}
					else
					{
						uBitsPerSample = pWaveFormatEx->nBitPerSample;
					}

					qDuration = (NXINT64)(((((pWaveFormatEx->nBlockAlignment - (4 * pWaveFormatEx->nChannel)) * 8) / (uBitsPerSample * pWaveFormatEx->nChannel)) + 1)
										  * (a_pstInfo->uTimescale / (NXDOUBLE)pWaveFormatEx->nSamplePerSecond));
				}
				else
				{
					qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
				}
			}
			break;

		case eNEX_CODEC_A_ADPCM_SWF:
			{
				NEX_WAVEFORMATEX *pWaveFormatEx = a_pstInfo->pWaveFormatEx;
				NXUINT32 uBitsPerSample;

				if (pWaveFormatEx && (0 != pWaveFormatEx->nChannel) && (0 != pWaveFormatEx->nSamplePerSecond))
				{
					if (pWaveFormatEx->nBitPerSample == 0)
					{
						uBitsPerSample = 4;
					}
					else
					{
						uBitsPerSample = pWaveFormatEx->nBitPerSample;
					}

					qDuration = (NXINT64)(((((a_pstInfo->usFrameSize - (NXUSIZE)(pWaveFormatEx->nBitPerSample * pWaveFormatEx->nChannel)) * 8) / (NXUSIZE)(uBitsPerSample * pWaveFormatEx->nChannel)) + 1)
										  * (a_pstInfo->uTimescale / (NXDOUBLE)pWaveFormatEx->nSamplePerSecond));
				}
				else
				{
					qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
				}
			}
			break;

		case eNEX_CODEC_A_AAC:
		case eNEX_CODEC_A_AACPLUS:
		case eNEX_CODEC_A_AACPLUSV2:
			{
				NEX_WAVEFORMATEX *pWaveFormatEx = a_pstInfo->pWaveFormatEx;

#if 1
				if (pWaveFormatEx && (0 != pWaveFormatEx->nSamplePerSecond))
				{
					qDuration = (NXINT64)(1024 * (a_pstInfo->uTimescale / (NXDOUBLE)pWaveFormatEx->nSamplePerSecond));
				}
				else
				{
					qDuration = (NXINT64)(23e-3 * a_pstInfo->uTimescale + 0.5);
				}
#else
				/* GetAACType()을 사용할 것인지, OTI 값을 사용할 것인지 고려. */
				uAACType = nexCALTools_GetAACType(a_pstInfo->eCodecType, pWaveFormatEx->nSamplePerSecond, pWaveFormatEx->nChannel, a_pstInfo->pFrameData, a_pstInfo->usFrameSize);

				if (pWaveFormatEx && (0 != pWaveFormatEx->nSamplePerSecond))
				{
					/* Sampling Frequency가 24kHz보다 클 경우 디코더는 DownSampling 또는 LC만 동작 */
					/* Max Rendering Frequency = 48kHz */
					if (pWaveFormatEx->nSamplePerSecond > 24000)
						uAACType = eNEX_CODEC_A_AAC;

					if (uAACType == eNEX_CODEC_A_AACPLUS)
						uDuration = 2048 * 1000 / (pWaveFormatEx->nSamplePerSecond << 1);
					else
						uDuration = 1024 * 1000 / pWaveFormatEx->nSamplePerSecond;
				}
				else
				{
					uDuration = 23;
				}
#endif
			}
			break;

		default:
			{
				qDuration = 0;
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] Not Supported AudioChunk format\n", _FUNLINE_);
			}
			break;
	}
	return qDuration;
}

//To convert 64bit variables to 32bit variables temporarily
//TEMP/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NXINT32 AudioChunkParser_PutFrame32(IN AUDIOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXUINT32 a_uDTS, IN NXUINT32 a_uPTS)
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

	return AudioChunkParser_PutFrame64(a_hChunkInfo, a_pChunk, a_uChunkLen, qDTS, qPTS);
}

NXINT32 AudioChunkParser_GetFrame32(IN AUDIOCHUNK_HANDLE a_hChunkInfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXUINT32 *a_puFrameDTS, OUT NXUINT32 *a_puFramePTS)
{
	NXINT64 qDTS = 0;
	NXINT64 qPTS = 0;
	NXINT32 nRet = 0;

	nRet = AudioChunkParser_GetFrame64(a_hChunkInfo, a_uGetOption, a_ppFrame, a_puFrameLen, &qDTS, &qPTS);

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

AUDIOCHUNK_HANDLE AudioChunkParser_CreateInstance(IN NEX_CODEC_TYPE a_eCodecType, IN NXUINT32 a_uFourCC, IN NEX_FILEFORMAT_TYPE a_eFFType, IN NXVOID *a_pWaveFormatEx, IN NXUINT32 a_uTimescale, IN NXVOID *a_pUserData)
{
	AUDIOCHUNK_INFO *pstInfo = (AUDIOCHUNK_INFO*)nexSAL_MemAlloc(sizeof(AUDIOCHUNK_INFO));

	a_pUserData = NULL;

	if (pstInfo)
	{
		memset(pstInfo, 0x00, sizeof(AUDIOCHUNK_INFO));

		pstInfo->eCodecType = a_eCodecType;
		pstInfo->uFourCC = a_uFourCC;
		pstInfo->eFFType = a_eFFType;
		pstInfo->eState = eAUDIOCHUNK_CREATE;
		pstInfo->usMaxChunkSize = _DEFAULT_AUDIO_FRAME_SIZE_;
		pstInfo->qTS4Null = INVALID_VALUE;
		pstInfo->qPreChunkDTS = INVALID_VALUE;
		pstInfo->qPreChunkPTS = INVALID_VALUE;
		pstInfo->qOffsetDTS = 0;
		pstInfo->bRenewalDTS = FALSE;
		pstInfo->eLeftoversState = eAUDIOCHUNK_NOT_FOUND_LEFTOVERS;

		if (0 == pstInfo->uTimescale) //[shoh][2017.01.05] use default value as millisecond scale if uTimescale is 0.
		{
			pstInfo->uTimescale = 1000;
		}
		else
		{
			pstInfo->uTimescale = a_uTimescale;
		}

		if ((pstInfo->eCodecType == eNEX_CODEC_A_ADPCM_MS) || (pstInfo->eCodecType == eNEX_CODEC_A_ADPCM_IMA_WAV) || (pstInfo->eCodecType == eNEX_CODEC_A_ADPCM_SWF)
			|| (pstInfo->eCodecType == eNEX_CODEC_A_AMR && pstInfo->uFourCC == NOSID_AMR_FOURCC) || (pstInfo->eCodecType == eNEX_CODEC_A_MP2)
			|| (pstInfo->eCodecType == eNEX_CODEC_A_MP3) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_S16LE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_S16BE)
			|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_RAW) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL32LE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL32BE)
			|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL64LE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL64BE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN24LE)
			|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN24BE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN32LE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN32BE)
			|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_LPCMLE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_LPCMBE) 
			|| (pstInfo->eCodecType == eNEX_CODEC_A_AAC) || (pstInfo->eCodecType == eNEX_CODEC_A_AACPLUS) || (pstInfo->eCodecType == eNEX_CODEC_A_AACPLUSV2))
		{
			pstInfo->pWaveFormatEx = nexSAL_MemAlloc(sizeof(NEX_WAVEFORMATEX));

			if (pstInfo->pWaveFormatEx && a_pWaveFormatEx)
			{
				NEX_WAVEFORMATEX *pWaveFormatEx = (NEX_WAVEFORMATEX*)a_pWaveFormatEx;

				pstInfo->pWaveFormatEx->nCodecID = pWaveFormatEx->nCodecID;
				pstInfo->pWaveFormatEx->nSamplePerSecond = pWaveFormatEx->nSamplePerSecond;
				pstInfo->pWaveFormatEx->nAvrByte = pWaveFormatEx->nAvrByte;
				pstInfo->pWaveFormatEx->nSampleNumber = pWaveFormatEx->nSampleNumber;
				pstInfo->pWaveFormatEx->nChannel = pWaveFormatEx->nChannel;
				pstInfo->pWaveFormatEx->nBlockAlignment = pWaveFormatEx->nBlockAlignment;
				pstInfo->pWaveFormatEx->nBitPerSample = pWaveFormatEx->nBitPerSample;
				pstInfo->pWaveFormatEx->nExtraDataLen = pWaveFormatEx->nExtraDataLen;
				pstInfo->pWaveFormatEx->pExtraData = pWaveFormatEx->pExtraData;

				if ((pstInfo->eCodecType == eNEX_CODEC_A_MP3 || pstInfo->eCodecType == eNEX_CODEC_A_MP2) && pstInfo->pWaveFormatEx->nExtraDataLen == 12)
				{
					NXUINT32 uUpperNum = (NXUINT32)(pstInfo->pWaveFormatEx->pExtraData[7] << 8);
					NXUINT32 uLowerNum = (NXUINT32)pstInfo->pWaveFormatEx->pExtraData[6];
					pstInfo->uSpareBlockAlign = uUpperNum | uLowerNum; // for little-endian ??
					pstInfo->bVBRChecked = FALSE;
				}
			}
			else
			{
				pstInfo->uSpareBlockAlign = 0;
			}
		}

		pstInfo->pChunkPool = (NXUINT8*)nexSAL_MemAlloc(pstInfo->usMaxChunkSize);
		if (pstInfo->pChunkPool == NULL)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MemAlloc failed\n", _FUNLINE_);
			SAFE_FREE(pstInfo);
			return NULL;
		}

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] NexChunkParser ver%s\n", _FUNLINE_, NexChunkParser_GetVersionString());
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] AudioChunkParser created instance.\n", _FUNLINE_);
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d] MemAlloc failed\n", _FUNLINE_);
		return NULL;
	}

	return(AUDIOCHUNK_HANDLE)pstInfo;
}

NXVOID AudioChunkParser_Destroy(IN AUDIOCHUNK_HANDLE a_hChunkInfo)
{
	AUDIOCHUNK_INFO *pstInfo = (AUDIOCHUNK_INFO*)a_hChunkInfo;

	if (pstInfo)
	{
		pstInfo->eState = eAUDIOCHUNK_DESTROY;
		SAFE_FREE(pstInfo->pWaveFormatEx);
		SAFE_FREE(pstInfo->pChunkPool);
		SAFE_FREE(pstInfo);

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] Destroy Done.\n", _FUNLINE_);
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] Destroy is not effected.\n", _FUNLINE_);        
	}
}

NXVOID AudioChunkParser_Reset(IN AUDIOCHUNK_HANDLE a_hChunkInfo)
{
	AUDIOCHUNK_INFO *pstInfo = (AUDIOCHUNK_INFO*)a_hChunkInfo;

	if (pstInfo)
	{
		pstInfo->eState = eAUDIOCHUNK_RESET;
		pstInfo->qFrameDTS = 0;
		pstInfo->qFramePTS = 0;
		pstInfo->qChunkDTS = 0;
		pstInfo->qChunkPTS = 0;
		pstInfo->usCurPos = 0;
		pstInfo->usRemainSize = 0;
		pstInfo->bMatched = FALSE;
		pstInfo->uFrameCnt = 0;
		pstInfo->qPreChunkDTS = INVALID_VALUE;
		pstInfo->qPreChunkPTS = INVALID_VALUE;
		pstInfo->eLeftoversState = eAUDIOCHUNK_NOT_FOUND_LEFTOVERS;

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] Reset Done.\n", _FUNLINE_);
	}
	else
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] Reset is not effected.\n", _FUNLINE_);  
	}
}

NXINT32 AudioChunkParser_PutFrame64(IN AUDIOCHUNK_HANDLE a_hChunkInfo, IN NXUINT8 *a_pChunk, IN NXUINT32 a_uChunkLen, IN NXINT64 a_qDTS, IN NXINT64 a_qPTS)
{
	AUDIOCHUNK_INFO *pstInfo = (AUDIOCHUNK_INFO*)a_hChunkInfo;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] pFrame(0x%X), len(%6d), remain(%6"PRIuS"), DTS(%10lld), PTS(%10lld)\n", _FUNLINE_, a_pChunk, a_uChunkLen, pstInfo->usRemainSize, a_qDTS, a_qPTS);

	if ((INVALID_VALUE == a_qDTS) && ((pstInfo->eState == eAUDIOCHUNK_CREATE) || (pstInfo->eState == eAUDIOCHUNK_RESET)))
	{
		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] The first incompleted frame was ignored!", _FUNLINE_);
		return 0;
	}

	//Copy remain data to the first of buffer
	if (pstInfo->usRemainSize > 0)
	{
		memmove(pstInfo->pChunkPool, pstInfo->pChunkPool + pstInfo->usCurPos, pstInfo->usRemainSize);
	}

	//Resizing ChunkPool because of insufficient memory
	if (((NXUSIZE)a_uChunkLen + pstInfo->usRemainSize) > pstInfo->usMaxChunkSize)
	{
		if (pstInfo->usMaxChunkSize >= AUDIOCHUNK_MEMORY_LIMIT)
		{
			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] ChunkPool reached memory limit(%10"PRIuS"), so previous chunk will be discarded!\n", _FUNLINE_, pstInfo->usMaxChunkSize);

			pstInfo->usRemainSize = 0;
		}
		else
		{
			NXUINT8 *pTmpFrame = 0;

			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] Resizing ChunkPoool (%10"PRIuS"->%10"PRIuS")\n", _FUNLINE_, pstInfo->usMaxChunkSize, ((NXUSIZE)a_uChunkLen + pstInfo->usRemainSize));

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
				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[%s %d]Malloc Failed\n", _FUNLINE_);
				return -1;
			}
		}
	}

	//Copy new Data
	memmove(pstInfo->pChunkPool + pstInfo->usRemainSize, a_pChunk, (NXUSIZE)a_uChunkLen);

	if (INVALID_VALUE != a_qDTS)
	{
		if ((pstInfo->eState == eAUDIOCHUNK_CREATE) || (pstInfo->eState == eAUDIOCHUNK_RESET) || (pstInfo->bRenewalDTS == TRUE))
		{
			pstInfo->qChunkDTS = a_qDTS;
			pstInfo->qChunkPTS = a_qPTS;
		}
		else if (pstInfo->eState == eAUDIOCHUNK_GETFRAME)
		{
			NXINT64 uNextFrameDTS = pstInfo->qFrameDTS + pstInfo->qDuration;

			//[shoh][2012.05.11] If there are leftovers and Chunk DTS is bigger than uNextFrameDTS, DTS of leftovers is regarded as linear DTS for previous fame.
			if (pstInfo->usRemainSize > 0 && a_qDTS > uNextFrameDTS && eAUDIOCHUNK_NOT_FOUND_LEFTOVERS == pstInfo->eLeftoversState)
			{
				pstInfo->eLeftoversState = eAUDIOCHUNK_FOUND_LEFTOVERS;
				pstInfo->qFrameDTS += pstInfo->qDuration;
				pstInfo->qFramePTS += pstInfo->qDuration;
			}
			else
			{
				pstInfo->eLeftoversState = eAUDIOCHUNK_NOT_FOUND_LEFTOVERS;
			}

			pstInfo->qChunkDTS = a_qDTS;
			pstInfo->qChunkPTS = a_qPTS;      
		}
	}

	pstInfo->usCurPos = 0;
	pstInfo->usRemainSize += a_uChunkLen;
	pstInfo->eState = eAUDIOCHUNK_PUTFRAME;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] PutFrame Done! m_uChunkDTS(%10lld), m_uChunkPTS(%10lld), a_uFrameLen(%6d), usRemainSize(%6"PRIuS"), bMatched(%d)\n", _FUNLINE_, pstInfo->qChunkDTS, pstInfo->qChunkPTS, a_uChunkLen, pstInfo->usRemainSize, pstInfo->bMatched);

	return 0;
}

NXINT32 AudioChunkParser_GetFrame64(IN AUDIOCHUNK_HANDLE a_hChunkinfo, IN NXUINT32 a_uGetOption, OUT NXUINT8 **a_ppFrame, OUT NXUINT32 *a_puFrameLen, OUT NXINT64 *a_pqFrameDTS, OUT NXINT64 *a_pqFramePTS)
{
	AFF_RETURN eRet = eAFF_RET_FAIL;
	NXUSIZE usFrameOffset;
	AUDIOCHUNK_INFO *pstInfo = (AUDIOCHUNK_INFO*)a_hChunkinfo;
	NXBOOL bEndChunk = (a_uGetOption!=AUDIOCHUNKPARSER_GETFRAME_OPTION_NORMAL ? TRUE : FALSE);

	if (pstInfo->usRemainSize > 0)
	{
		//[shoh][2011.06.13] If parser recognizes EOF, bMatched is TRUE. Then leftovers will be processed.
		if (TRUE == bEndChunk)
		{
			pstInfo->bMatched = TRUE;
		}

		eRet = _AudioFrameFinder(pstInfo, &usFrameOffset);

		NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] nRet(%d), uFrameOffset(%6"PRIuS"), usFrameSize(%6"PRIuS")\n", _FUNLINE_, eRet, usFrameOffset, pstInfo->usFrameSize);

		if (eAFF_RET_SUCCESS == eRet)
		{
			NXUSIZE usCurReadPos = usFrameOffset + pstInfo->usFrameSize;

			pstInfo->pFrameData = pstInfo->pChunkPool + pstInfo->usCurPos + usFrameOffset;

			pstInfo->qDuration = _AudioChunkParser_GetAudioDuration(pstInfo);

			//[shoh][2011.05.26] To complement TS of the beginning of MP3 filled by zero
			if (pstInfo->qTS4Null == INVALID_VALUE)
			{
				if ((pstInfo->eFFType == eNEX_FF_AVI) && (pstInfo->eCodecType == eNEX_CODEC_A_MP2 || pstInfo->eCodecType == eNEX_CODEC_A_MP3) && (pstInfo->uNullCnt > 0))
				{
					NXUINT32 uSampleRate = 0;
					NXUINT64 qIntervalTime = _GetMPEGAudioInterval(pstInfo, &uSampleRate);

					if (pstInfo->bVBR) // VBR case
					{
						if (pstInfo->pWaveFormatEx->nBlockAlignment > 1)
						{
							pstInfo->qTS4Null = (NXUINT32)(pstInfo->uNullCnt/pstInfo->pWaveFormatEx->nBlockAlignment+0.5) * qIntervalTime;
						}
						else if (pstInfo->uSpareBlockAlign > 7)
						{
							pstInfo->qTS4Null = (NXUINT32)(pstInfo->uNullCnt/pstInfo->uSpareBlockAlign+0.5) * qIntervalTime;
						}
						else
						{
							pstInfo->qTS4Null = 0;
						}
					}
					else // CBR case
					{
						pstInfo->qTS4Null = (pstInfo->uNullCnt*1000)/pstInfo->pWaveFormatEx->nAvrByte;
					}

					pstInfo->qOffsetDTS = pstInfo->qChunkDTS; //[shoh][2011.08.23] First frame CTS is offset CTS for the beginning of NULL.
				}
				else
				{
					pstInfo->qTS4Null = 0;
				}
			}

			if (pstInfo->usRemainSize < usCurReadPos)
			{
				if (TRUE == bEndChunk)
				{
					//[shoh][2011.04.14] If  bEndChunk is EOF(1), should return leftovers of frame in case of PCM or WAV.
					if ((pstInfo->eCodecType == eNEX_CODEC_A_PCM_S16LE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_S16BE)
						|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_RAW) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL32LE)
						|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL32BE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL64LE)
						|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_FL64BE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN24LE)
						|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN24BE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN32LE)
						|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_IN32BE) || (pstInfo->eCodecType == eNEX_CODEC_A_PCM_LPCMLE)
						|| (pstInfo->eCodecType == eNEX_CODEC_A_PCM_LPCMBE))
					{
						if (pstInfo->usRemainSize >= 4)
						{
							*a_ppFrame = pstInfo->pFrameData;
							pstInfo->usFrameSize = pstInfo->usRemainSize & (~0x03);	//[shoh][2011.06.16] Certain devices need PCM of 4 bytes' alignment.
							*a_puFrameLen = (NXUINT32)pstInfo->usFrameSize;
							*a_pqFrameDTS = pstInfo->qFrameDTS = pstInfo->qChunkDTS + pstInfo->qComplementTS;
							*a_pqFramePTS = pstInfo->qFramePTS = pstInfo->qChunkPTS + pstInfo->qComplementTS;
						}
						else if (0 < pstInfo->usRemainSize && pstInfo->usRemainSize < 4)
						{
							//[shoh][2012.06.05] If frame size is under 4bytes, 4bytes padding needs.
							memset(pstInfo->pFrameData+pstInfo->usRemainSize, 0x00, (4-pstInfo->usRemainSize));
							*a_ppFrame = pstInfo->pFrameData;
							pstInfo->usFrameSize = 4;
							*a_puFrameLen = (NXUINT32)pstInfo->usFrameSize;
							*a_pqFrameDTS = pstInfo->qFrameDTS = pstInfo->qChunkDTS + pstInfo->qComplementTS;
							*a_pqFramePTS = pstInfo->qFramePTS = pstInfo->qChunkPTS + pstInfo->qComplementTS;
						}
						else
						{
							*a_ppFrame = NULL;
							pstInfo->usFrameSize = 0;
							*a_puFrameLen = (NXUINT32)pstInfo->usFrameSize;
							*a_pqFrameDTS = pstInfo->qFrameDTS = 0;
							*a_pqFramePTS = pstInfo->qFramePTS = 0;
						}

						pstInfo->usRemainSize = 0;
						pstInfo->qChunkDTS = 0;
						pstInfo->qChunkPTS = 0;
						pstInfo->eState = eAUDIOCHUNK_GETFRAME;

						NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d]Returning leftovers of frame in case of PCM or WAV due to EOF.(size=%d, CTS=%d)\n", _FUNLINE_, *a_puFrameLen, *a_pqFrameDTS);
						return 0;                       
					}
				}

				pstInfo->uNullCnt = 0;

				*a_ppFrame = NULL;
				*a_puFrameLen = 0;
				*a_pqFrameDTS = 0;
				*a_pqFramePTS = 0;

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] Found but NEED MORE(Remained Chunk size=%"PRIuS", usCurReadPos=%"PRIuS")\n", _FUNLINE_, pstInfo->usRemainSize, usCurReadPos);              
				return 0; //Need AudioChunkParser_PutFrame
			}
			else
			{
				pstInfo->usCurPos += usCurReadPos;
				pstInfo->usRemainSize -= usCurReadPos;

				*a_ppFrame = pstInfo->pFrameData;
				*a_puFrameLen = (NXUINT32)pstInfo->usFrameSize;

				if (pstInfo->eLeftoversState == eAUDIOCHUNK_FOUND_LEFTOVERS)
				{
					pstInfo->eLeftoversState = eAUDIOCHUNK_PROCESSED_LEFTOVERS;

					*a_pqFrameDTS = pstInfo->qFrameDTS;
					*a_pqFramePTS = pstInfo->qFramePTS;
				}
				else
				{
					//[shoh][2011.08.01] Complement of CTS
					if (pstInfo->qChunkDTS == pstInfo->qPreChunkDTS)
					{
						pstInfo->uFrameCnt++;
					}
					else
					{
						pstInfo->uFrameCnt = 0;
						pstInfo->qPreChunkDTS = pstInfo->qChunkDTS;
						pstInfo->qPreChunkPTS = pstInfo->qChunkPTS;
					}
					pstInfo->qComplementTS = pstInfo->qTS4Null + (pstInfo->qDuration * pstInfo->uFrameCnt);

					//[shoh][2011.09.09] if seek to 0 after m_uTS4Null is calculated, m_uChunkDTS can be smaller than m_uOffsetDTS.
					if (pstInfo->qChunkDTS < pstInfo->qOffsetDTS)
					{
						pstInfo->qChunkDTS = pstInfo->qOffsetDTS;
						pstInfo->qChunkPTS = pstInfo->qOffsetDTS;
					}
					*a_pqFrameDTS = pstInfo->qFrameDTS = pstInfo->qChunkDTS + pstInfo->qComplementTS - pstInfo->qOffsetDTS;
					*a_pqFramePTS = pstInfo->qFramePTS = pstInfo->qChunkPTS + pstInfo->qComplementTS - pstInfo->qOffsetDTS;
				}

				pstInfo->eState = eAUDIOCHUNK_GETFRAME;

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] GetFrame Done! m_uFrameDTS(%10lld), usFrameSize(%6"PRIuS"), m_uReadPos(%6"PRIuS"), m_uRemainLen(%6"PRIuS") \n", _FUNLINE_, pstInfo->qFrameDTS, pstInfo->usFrameSize, pstInfo->usCurPos, pstInfo->usRemainSize);
				return 0;
			}
		}
		else if (eAFF_RET_NEED_MORE == eRet)
		{
			if (TRUE == bEndChunk)
			{
				*a_ppFrame = pstInfo->pFrameData = NULL;
				pstInfo->usFrameSize = 0;
				*a_puFrameLen = (NXUINT32)pstInfo->usFrameSize;

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] Audio Chunk End\n", _FUNLINE_);              
			}
			else
			{
				pstInfo->uNullCnt = 0;

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] Found but NEED MORE(Remained Chunk size: %"PRIuS", nRet = %d)\n", _FUNLINE_, pstInfo->usRemainSize, eRet);
			}

			return 0;
		}
		else
		{
			if (TRUE == bEndChunk)
			{
				*a_ppFrame = pstInfo->pFrameData = NULL;
				pstInfo->usFrameSize = 0;
				*a_puFrameLen = (NXUINT32)pstInfo->usFrameSize;

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] Audio Chunk End\n", _FUNLINE_);
				return 0;
			}
			else
			{
				pstInfo->uNullCnt = 0;

				NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] failed(%d)!)\n", _FUNLINE_, eRet);
				return -1;
			}
		}
	}
	else
	{
		if (TRUE == bEndChunk)
		{
			*a_ppFrame = pstInfo->pFrameData = NULL;
			pstInfo->usFrameSize = 0;
			*a_puFrameLen = (NXUINT32)pstInfo->usFrameSize;

			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, 0, "[%s %d] Audio Chunk End\n", _FUNLINE_);          
		}
		else
		{
			pstInfo->uNullCnt = 0;

			NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_AUDIO, ACP_LOG, "[%s %d] NEED MORE(Remained Chunk size: %"PRIuS")\n", _FUNLINE_, pstInfo->usRemainSize);
		}

		return 0;
	}
}

