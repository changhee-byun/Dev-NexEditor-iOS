/*-----------------------------------------------------------------------------
	File Name   :	NexCAL_Wrapper.h
	Description :	none.
 ------------------------------------------------------------------------------

 NexStreaming Confidential Proprietary
 Copyright (C) 2007 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

#ifndef _NEXCAL_WRAPPER_HEADER_INCLUDED_
#define _NEXCAL_WRAPPER_HEADER_INCLUDED_

#include "NexCAL.h"
#include "NexMediaDef.h"
#include "NexSAL_Internal.h"

#define NEX_MEDIA_VALUE	4
typedef struct _tagNEXCALRegistrationInfo
{
	NXUINT32            m_uRefCnt;
	NXUINT32            m_uCnt;
	NEXSALFileHandle    m_ahDump[NEX_MEDIA_VALUE];
	NXUINT32            m_aReserved4Dump[NEX_MEDIA_VALUE];

	NEXCALCodecEntry    *m_pStart;
	NXUINT32            m_uVideoCodecLastErrorInfo;
	NEXCALPolicy        m_ePolicy;
} NEXCALRegistrationInfo;


struct NEXCALCodecHandle_struct
{
	NEXCALCodecFuncs    m_cfs; // this is just for compatibility.
	NXVOID*             m_pUserData;
	NXUINT32            m_uType;
	NEX_CODEC_TYPE      m_eCodecType;
	NXVOID              *m_pEntry;

	NXVOID              *m_pDSI;
	NXUINT32            m_uDSILen;

	NXUINT8*            m_pUserFrame; // used to indicate the pointer of silence frame in Audio.
	NXUINT32            m_uUserFrameLen; // used to indicate the size of silence frame in Audio.
	NXVOID              *m_pIntermediateFrame;
	NXUINT32            m_uIntermediateFrameSize;
	NXUINT32            m_uNALLenSize;
	NXUINT32            m_uByteFormat4Frame;
	NXUINT32            m_uByteFormat4DSI;
	NXBOOL              m_bByteFormatConverted4Frame;

	NXUINT32            m_uWidth;
	NXUINT32            m_uHeight;
	NXUINT32            m_uWidthPitch;
	NXUINT32            m_uHeightPitch;

	NXUINT32            m_uVOPTimeIncBits;
	NXUINT32            m_uMask4Bframe;	//[shoh][2011.05.23] B-frame should be decoded by the condition decoding both I-frame and P-frame after seek.

	NEXCALQueryHandler  m_fnQueryHandler;
	NXVOID*             m_pUserDataForQueryHandler;
	NXUINT32            m_uProperties;
	NXUINT32            m_uOutBufferFormat;
	NXUINT32            m_uAVCMaxProfile;

	NXUINT32            m_uVDecCnt;	// Total video decoding count
	NXUINT32            m_uTotalVDecTime; // Total video decoding time

	NXUINT32            uSamplingRate;
	NXUINT32            uNumOfChannels;
	NXUINT32            uBitsPerSample;
	NXUINT32            uNumOfSamplesPerChannel;

	NEXCALHandle        m_hCAL;

	//Property
	NXINT64             m_qIncludeConfig;
	NXINT64             m_qBFrameSkip;
	NXINT64             m_qSupportADTSHeader;
};

#endif //_NEXCAL_WRAPPER_HEADER_INCLUDED_

/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date		Description of Changes
 ------------------------------------------------------------------------------
 ysh		07-06-14	Draft.
 ----------------------------------------------------------------------------*/

