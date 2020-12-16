//
//  SALBody_Mem.h
//  nexSalBody
//
//  Created by Simon Kim on 1/26/15.
//
//

#ifndef nexSalBody_SALBody_Mem_h
#define nexSalBody_SALBody_Mem_h
#include "NexSAL_API.h"
#include "NexSAL_Internal.h"
#ifdef __cplusplus
extern "C" {
#endif
	
    NXVOID* nexSALBody_MemAlloc( NXUSIZE uSize );
    NXVOID* nexSALBody_MemCalloc( NXUSIZE uNum, NXUSIZE uSize );
    NXVOID nexSALBody_MemFree( NXVOID* pMem );
	
	NXVOID* nexSALBody_MemAlloc2( NXUSIZE uSize, NXUINT32 uType, const NXCHAR * pSrcFile, NXINT32 iSrcLine );
	NXVOID nexSALBody_MemFree2( NXVOID* pMem );
	
#ifdef __cplusplus
}
#endif

#endif
