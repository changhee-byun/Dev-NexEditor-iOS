/*-----------------------------------------------------------------------------
	File Name   :	NexCodecEntry.h
	Description :	none.
 ------------------------------------------------------------------------------

 NexStreaming Confidential Proprietary
 Copyright (C) 2007 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation

 Revision History was described at the bottom of this file.
-----------------------------------------------------------------------------*/

#ifndef _NEX_CODEC_ENTRY_DOT_H_INCLUDED_
#define _NEX_CODEC_ENTRY_DOT_H_INCLUDED_

#include "NexCAL.h"

typedef union
{
	NEXCALVideoDecoder vd;
	NEXCALAudioDecoder ad;
	NEXCALTextDecoder  td;
	NEXCALImageDecoder id;
	NEXCALVideoEncoder ve;
	NEXCALAudioEncoder ae;

} NEXCALCodecFuncs;

typedef struct _tagNEXCALCodecEntry
{
	struct _tagNEXCALCodecEntry *m_pNext;

	NEXCALMediaType     m_eType; // now, only A/V/T/I support. 0:Video, 1:Audio, 2:Text, 3:Still Image
	NEXCALMode          m_eMode; // 0: decoder, 1:encoder
	NEX_CODEC_TYPE      m_eCodecType;

	NEXCALOnLoad        m_pOnLoad;
	NXVOID*             m_pUserDataForOnLoad;
	NXINT32             m_nNumOfSameType; // the first entry has the meaning. others has no meaning.
	NXUINT32            m_uStaticProperties;

	//NEXCALCodecFuncs m_ci;

} NEXCALCodecEntry;

#define _IS_SAME_TYPE_(a,t,m,o)  ((a->m_eType == t) && \
									(a->m_eMode == m) && \
									(a->m_eCodecType == o))

#ifdef __cplusplus
extern "C" {
#endif
	NEXCALCodecEntry * _CE_New();
	NXINT32 _CE_Add(NEXCALCodecEntry **a_pStart, NEXCALCodecEntry *a_pEntry);
	NXINT32 _CE_Remove(NEXCALCodecEntry **a_pStart, NEXCALCodecEntry *a_pEntry);
	NEXCALCodecEntry * _CE_Find(NEXCALCodecEntry *a_pStart, NEXCALMediaType a_eType, NEXCALMode a_eMode, NEX_CODEC_TYPE a_eCodec);
	NXVOID _CE_FreeListAll(NEXCALCodecEntry *a_pStart);

#ifdef __cplusplus
}
#endif

#endif //_NEX_CODEC_ENTRY_DOT_H_INCLUDED_

/*-----------------------------------------------------------------------------
 Revision History:
 Author		Date		Description of Changes
 ------------------------------------------------------------------------------
 ysh		07-04-14	Draft.
 ----------------------------------------------------------------------------*/
