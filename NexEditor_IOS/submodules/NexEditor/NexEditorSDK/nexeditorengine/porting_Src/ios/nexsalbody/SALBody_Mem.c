//
//  SALBody_Mem.c
//  ApiDemos
//
//  Created by rooney on 2016. 1. 27..
//  Copyright © 2016년 rooney. All rights reserved.
//
#include "SALBody_Mem.h"
#include "SALBody_Debug.h"
#include "NexTypeDef.h"

#include <stdio.h>
#include <stdlib.h>

void* nexSALBody_MemAlloc( NXUSIZE uSize )
{
    if ( uSize == 0 )
    {
        nexSALBody_DebugOutputString("Discarding MemAlloc(0)");
        return NULL;
    }
    
    return malloc( uSize );
}

void* nexSALBody_MemCalloc( NXUSIZE uNum, NXUSIZE uSize )
{
    if ( uNum == 0 || uSize == 0 )
    {
        nexSALBody_DebugOutputString("MemCalloc size is zero\n");
        return NULL;
    }
    
    return calloc( uNum, uSize );
}

void nexSALBody_MemFree( void* pMem )
{
    if ( pMem == NULL )
    {
        nexSALBody_DebugOutputString("MemFree target is NULL\n");
        return;
    }
    
    free( pMem );
}

NXVOID* nexSALBody_MemAlloc2( NXUSIZE uSize, NXUINT32 uType, const NXCHAR * pSrcFile, NXINT32 iSrcLine )
{
    void *mem = NULL;
    uint32_t align;
    int ret;
    
    if ( uSize > 0 )
    {
        align = uType & 0x0000FFFF;
        if ( align <= NEXSAL_MEM_ALIGN_128) {
            // NEXSAL_MEM_ALIGN_64(0x20) -> 64
            align = align * 2;
        } else {
            // fallback default: 16 bytes aligned
            align = 16;
        }
        
        ret = posix_memalign( &mem, align, uSize );
        if ( ret != 0 ) {
            mem = NULL;
        }
    } else {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_E_SYS, 0,"[SALBody_Mem.cpp %d]MemAlloc2 size is zero\n",__LINE__);
    }
    
    return mem;
}

NXVOID nexSALBody_MemFree2( NXVOID* pMem )
{
    if ( pMem != NULL )
    {
        free( pMem );
    }
}