#ifndef _NEX_RENDERER_ENTRY_DOT_H_INCLUDED_
#define _NEX_RENDERER_ENTRY_DOT_H_INCLUDED_

#include "NexRAL.h"

typedef union
{
	NEXRALVideoRenderer vr;
	NEXRALAudioRenderer ar;
	NEXRALTextRenderer  tr;

} NEXRALRendererFuncs;

typedef struct _tagNEXRALRendererEntry
{
	struct _tagNEXRALRendererEntry *m_pNext;

	NEXRALMediaType m_eType; // now, only A/V/T/I support. 0:Video, 1:Audio, 2:Text, 3:Still Image
	NXUINT32 m_uPrivate;
	NXUINT32 m_uBufferFormat;
	NEXRALGetRendererNotification m_pNotificationCB;
	NXVOID *m_pUserData;
	NEXRALRendererFuncs m_ri;

} NEXRALRendererEntry;

#define _IS_SAME_ENTRY_(a, b)  (((a)->m_eType == (b)->m_eType) && \
								((a)->m_uBufferFormat == (b)->m_uBufferFormat))

#define _IS_SAME_TYPE_(a, t, bf)  (((a)->m_eType == (t)) && \
									((a)->m_uBufferFormat == (bf)))

#ifdef __cplusplus
extern "C" {
#endif

	NEXRALRendererEntry* _RE_New();
	NXINT32 _RE_Add(NEXRALRendererEntry **a_ppStart, NEXRALRendererEntry *a_pEntry);
	NEXRALRendererEntry* _RE_Find(NEXRALRendererEntry *a_pStart, NEXRALMediaType a_eType, NXUINT32 a_uBufferFormat);
	NXVOID _RE_FreeListAll(NEXRALRendererEntry *a_pStart);

#ifdef __cplusplus
}
#endif

#endif //_NEX_RENDERER_ENTRY_DOT_H_INCLUDED_
