/******************************************************************************
* File Name   :	NexSAL_API.h
* Description :	Functions registration header file for nexSAL
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2006~2014 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXSAL_API_HEADER_INCLUDED_
#define _NEXSAL_API_HEADER_INCLUDED_

#if !defined(_NEXSAL_COMMON_HEADER_INCLUDED_)
	#include "NexSAL_Com.h"
#endif

/*
 * Function Indexes
 */

#define NEXSAL_MEM_FUNCTIONS_INDEX			0x0100
#define NEXSAL_REG_MEM_ALLOC				(NEXSAL_MEM_FUNCTIONS_INDEX+0)
#define NEXSAL_REG_MEM_CALLOC				(NEXSAL_MEM_FUNCTIONS_INDEX+1)
#define NEXSAL_REG_MEM_FREE					(NEXSAL_MEM_FUNCTIONS_INDEX+2)
#define NEXSAL_REG_MEM_ALLOC2				(NEXSAL_MEM_FUNCTIONS_INDEX+3)
#define NEXSAL_REG_MEM_FREE2				(NEXSAL_MEM_FUNCTIONS_INDEX+4)

#define NEXSAL_FILE_FUNCTIONS_INDEX			0x0200
#define NEXSAL_REG_FILE_CLOSE				(NEXSAL_FILE_FUNCTIONS_INDEX+0)
#define NEXSAL_REG_FILE_FREEDISKSPACEA		(NEXSAL_FILE_FUNCTIONS_INDEX+1)
#define NEXSAL_REG_FILE_OPENA				(NEXSAL_FILE_FUNCTIONS_INDEX+2)
#define NEXSAL_REG_FILE_READ				(NEXSAL_FILE_FUNCTIONS_INDEX+3)
#define NEXSAL_REG_FILE_REMOVEA				(NEXSAL_FILE_FUNCTIONS_INDEX+4)
#define NEXSAL_REG_FILE_SEEK				(NEXSAL_FILE_FUNCTIONS_INDEX+5)
#define NEXSAL_REG_FILE_SEEK64				(NEXSAL_FILE_FUNCTIONS_INDEX+6)
#define NEXSAL_REG_FILE_SIZE				(NEXSAL_FILE_FUNCTIONS_INDEX+7)
#define NEXSAL_REG_FILE_WRITE				(NEXSAL_FILE_FUNCTIONS_INDEX+8)
#define NEXSAL_REG_FILE_RENAMEA				(NEXSAL_FILE_FUNCTIONS_INDEX+9)
#define NEXSAL_REG_FILE_GET_ATTRIBUTEA		(NEXSAL_FILE_FUNCTIONS_INDEX+10)
#define NEXSAL_REG_DIR_MAKEA				(NEXSAL_FILE_FUNCTIONS_INDEX+11)
#define NEXSAL_REG_DIR_REMOVEA				(NEXSAL_FILE_FUNCTIONS_INDEX+12)
#define NEXSAL_REG_FIND_FIRSTA				(NEXSAL_FILE_FUNCTIONS_INDEX+13)
#define NEXSAL_REG_FIND_NEXT				(NEXSAL_FILE_FUNCTIONS_INDEX+14)
#define NEXSAL_REG_FIND_CLOSE				(NEXSAL_FILE_FUNCTIONS_INDEX+15)
#define NEXSAL_REG_FILE_OPENW				(NEXSAL_FILE_FUNCTIONS_INDEX+16)
#define NEXSAL_REG_FILE_REMOVEW				(NEXSAL_FILE_FUNCTIONS_INDEX+17)
#define NEXSAL_REG_FILE_FREEDISKSPACEW		(NEXSAL_FILE_FUNCTIONS_INDEX+18)
#define NEXSAL_REG_FILE_RENAMEW				(NEXSAL_FILE_FUNCTIONS_INDEX+19)
#define NEXSAL_REG_FILE_GET_ATTRIBUTEW		(NEXSAL_FILE_FUNCTIONS_INDEX+20)
#define NEXSAL_REG_DIR_MAKEW				(NEXSAL_FILE_FUNCTIONS_INDEX+21)
#define NEXSAL_REG_DIR_REMOVEW				(NEXSAL_FILE_FUNCTIONS_INDEX+22)
#define NEXSAL_REG_FIND_FIRSTW				(NEXSAL_FILE_FUNCTIONS_INDEX+23)


#define NEXSAL_SYNCOBJ_FUNCTIONS_INDEX		0x0300
#define NEXSAL_REG_EVENT_CREATE				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+0)
#define NEXSAL_REG_EVENT_DELETE				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+1)
#define NEXSAL_REG_EVENT_SET				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+2)	
#define NEXSAL_REG_EVENT_WAIT				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+3)
#define NEXSAL_REG_EVENT_CLEAR				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+4)

#define NEXSAL_REG_MUTEX_CREATE				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+10)
#define NEXSAL_REG_MUTEX_DELETE				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+11)
#define NEXSAL_REG_MUTEX_LOCK				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+12)
#define NEXSAL_REG_MUTEX_UNLOCK				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+13)

#define NEXSAL_REG_SEMAPHORE_CREATE			(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+20)
#define NEXSAL_REG_SEMAPHORE_DELETE			(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+21)
#define NEXSAL_REG_SEMAPHORE_RELEASE		(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+22)
#define NEXSAL_REG_SEMAPHORE_WAIT			(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+23)

#define NEXSAL_REG_ATOMIC_INC				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+30)
#define NEXSAL_REG_ATOMIC_DEC				(NEXSAL_SYNCOBJ_FUNCTIONS_INDEX+31)

#define NEXSAL_TASK_FUNCTIONS_INDEX			0x0400
#define NEXSAL_REG_TASK_CREATE				(NEXSAL_TASK_FUNCTIONS_INDEX+0)
#define NEXSAL_REG_TASK_CURRENT				(NEXSAL_TASK_FUNCTIONS_INDEX+1)
#define NEXSAL_REG_TASK_DELETE				(NEXSAL_TASK_FUNCTIONS_INDEX+2)
#define NEXSAL_REG_TASK_GETPRIORITY			(NEXSAL_TASK_FUNCTIONS_INDEX+4)
#define NEXSAL_REG_TASK_SETPRIORITY			(NEXSAL_TASK_FUNCTIONS_INDEX+5)
#define NEXSAL_REG_TASK_SLEEP				(NEXSAL_TASK_FUNCTIONS_INDEX+6)
#define NEXSAL_REG_TASK_TERMINATE			(NEXSAL_TASK_FUNCTIONS_INDEX+7)	
#define NEXSAL_REG_TASK_WAIT				(NEXSAL_TASK_FUNCTIONS_INDEX+8)

#define NEXSAL_SOCKET_FUNCTIONS_INDEX		0x0500
#define NEXSAL_REG_NET_SOCKET				(NEXSAL_SOCKET_FUNCTIONS_INDEX+0)
#define NEXSAL_REG_NET_CLOSE				(NEXSAL_SOCKET_FUNCTIONS_INDEX+1)
#define NEXSAL_REG_NET_CONNECT				(NEXSAL_SOCKET_FUNCTIONS_INDEX+2)
#define NEXSAL_REG_NET_BIND					(NEXSAL_SOCKET_FUNCTIONS_INDEX+3)
#define NEXSAL_REG_NET_SELECT				(NEXSAL_SOCKET_FUNCTIONS_INDEX+4)
#define NEXSAL_REG_NET_SENDTO				(NEXSAL_SOCKET_FUNCTIONS_INDEX+5)
#define NEXSAL_REG_NET_SEND					(NEXSAL_SOCKET_FUNCTIONS_INDEX+6)	
#define NEXSAL_REG_NET_RECVFROM				(NEXSAL_SOCKET_FUNCTIONS_INDEX+7)
#define NEXSAL_REG_NET_RECV					(NEXSAL_SOCKET_FUNCTIONS_INDEX+9)
#define NEXSAL_REG_NET_MULTIGROUP			(NEXSAL_SOCKET_FUNCTIONS_INDEX+11)
#define NEXSAL_REG_NET_SSL_TUNNELING		(NEXSAL_SOCKET_FUNCTIONS_INDEX+12)
#define NEXSAL_REG_NET_LISTEN				(NEXSAL_SOCKET_FUNCTIONS_INDEX+13)
#define NEXSAL_REG_NET_ACCEPT				(NEXSAL_SOCKET_FUNCTIONS_INDEX+14)
#define NEXSAL_REG_NET_ASYNC_CONNECT		(NEXSAL_SOCKET_FUNCTIONS_INDEX+15)
#define NEXSAL_REG_NET_ASYNC_SSL_TUNNELING	(NEXSAL_SOCKET_FUNCTIONS_INDEX+16)


#define NEXSAL_TRACE_FUNCTIONS_INDEX		0x0600
#define NEXSAL_REG_DBG_PRINTF				(NEXSAL_TRACE_FUNCTIONS_INDEX+0)
//#define NEXSAL_REG_DBG_CONDITION			(NEXSAL_TRACE_FUNCTIONS_INDEX+1)
#define NEXSAL_REG_DBG_OUTPUTSTRING			(NEXSAL_TRACE_FUNCTIONS_INDEX+2)

#define NEXSAL_ETC_FUNCTIONS_INDEX			0x1000
#define NEXSAL_REG_GETTICKCOUNT				(NEXSAL_ETC_FUNCTIONS_INDEX+0)
#define NEXSAL_REG_GETMSECFROMEPOCH			(NEXSAL_ETC_FUNCTIONS_INDEX+1)


#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Registers SAL functions needed by specific application.
 * 
 * @param a_uFunctionIndex [in] Index of function to register.
 * @param a_pFunc [in] Pointer to function that corresponds with function index.
 */
	NEXSAL_API NEXSAL_RETURN nexSAL_RegisterFunction(NXUINT32 a_uFunctionIndex, NXVOID* a_pFunc, NXVOID** a_ppPreFunc);

#ifdef __cplusplus
}
#endif

#endif // _NEXSAL_API_HEADER_INCLUDED_

/*-----------------------------------------------------------------------------
Revision History 
Revision History:
Author		Date			Version		Description of Changes
-------------------------------------------------------------------------------
pss			2005/08/11		1.0			Draft
ysh			2005/08/15		1.1			Revision
ysh			2006/03/12		1.2			NEXSAL_TRACE_MODULE added
hsc			2006/03/30		1.3			File related 7 functions is added
ysh			2006/04/11		1.4/0		Revision(classifying functions)
pss         2006/06/08      2.0         FileSizeEx, FileFreeDiskSpaceEx Added
ysh			2006/08/08		2.1			Apis related to Socket is revised.
ysh			2006/08/24		2.1/1		Define name is changed. ex)NEXSAL_MEM_ALLOC -> NEXSAL_REG_MEM_ALLOC
ysh			2006/09/12		2.2			UNICODE is introduced. nexSAL_FileOpen(A/W) is adapted.
pss			2006/09/27		2.2/1	    nexSAL_FileRemove(A/W) is adapted.(temporarily)
ysh			2008/06/13		2.5.0		Unicode functions(on File) was added.
ysh			2008/07/28		2.6.0		NEXSAL_REG_DBG_OUTPUTSTRING was added.
ysh			2008/09/29		2.6.1		NEXSAL_REG_MEM_ALLOC2, NEXSAL_REG_MEM_FREE2 was added.
ysh			2009/12/21		2.6.2		NEXSAL_REG_FILE_SEEK64 was added.
robin		2010/06/01		2.6.3		Malloc, Calloc, Malloc2 Parameter Added for Memory Leak
ysh			2011/05/26		2.6.4		atomic inc/dec were added.
...
robin		2014/07/29		3.2.0		NEXSAL_REG_NET_ASYNC_CONNECT, NEXSAL_REG_NET_ASYNC_SSL_TUNNELING api added.
-----------------------------------------------------------------------------*/
