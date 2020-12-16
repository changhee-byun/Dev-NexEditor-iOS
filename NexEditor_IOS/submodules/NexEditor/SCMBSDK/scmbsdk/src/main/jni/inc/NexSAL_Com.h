/******************************************************************************
* File Name   :	NexSAL_Com.h
* Description :	Common header file for nexSAL
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2005~2007 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXSAL_COMMON_HEADER_INCLUDED_
#define _NEXSAL_COMMON_HEADER_INCLUDED_

#include "NexTypeDef.h"

#define NEXSAL_VERSION_MAJOR	4
#define NEXSAL_VERSION_MINOR	1
#define NEXSAL_VERSION_PATCH	0
#define NEXSAL_VERSION_BRANCH	"OFFICIAL"

#define NEXSAL_COMPATIBILITY_NUM	2


#ifdef __cplusplus
extern "C" {
#endif


#if (defined(_WIN8) || defined(WIN32) || defined(WINCE))
	#if defined(NEXSAL_EXPORTS)
		#define NEXSAL_API __declspec(dllexport)
	#else
		#define NEXSAL_API __declspec(dllimport)
	#endif
#else
	#define NEXSAL_API
#endif

/*
 *	Handles 
 */
#define	NEXSAL_INVALID_HANDLE	0

	typedef struct NEXSALHandle_struct *NEXSALHandle;

	typedef NXVOID* NEXSALFileHandle;
	typedef NXVOID* NEXSALSearchHandle;
	typedef NXVOID* NEXSALTaskHandle;
	typedef NXVOID* NEXSALEventHandle;
	typedef NXVOID* NEXSALMutexHandle;
	typedef NXVOID* NEXSALSemaphoreHandle;
	typedef NXVOID* NEXSALSockHandle;

// deprecated because socket is handle
//#define NEXSAL_INVALID_SOCK		(NEXSALSockHandle)(~0) 

/*
 *	Struct Types
 */

	typedef struct NEXSALTimeValue
	{
		NXUINT32    tv_sec;			/* seconds */
		NXUINT32    tv_usec;		/* and microseconds */
	} NEXSALTimeValue;

#define NEXSAL_FD_SETSIZE 64

	typedef struct NEXSALFDSet
	{
		NXUINT32 fd_count;
		NEXSALSockHandle fd_array[NEXSAL_FD_SETSIZE];
	} NEXSALFDSet;

#define NEXSAL_FD_CLR(fd, set) { \
    u_int __i; \
    for (__i = 0; __i < ((NEXSALFDSet *)(set))->fd_count ; __i++) { \
        if (((NEXSALFDSet *)(set))->fd_array[__i] == fd) { \
            while (__i < ((NEXSALFDSet *)(set))->fd_count-1) { \
                ((NEXSALFDSet *)(set))->fd_array[__i] = \
                    ((NEXSALFDSet *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((NEXSALFDSet *)(set))->fd_count--; \
            break; \
        } \
    } \
}

#define NEXSAL_FD_SET(fd, set) { \
    if (((NEXSALFDSet *)(set))->fd_count < NEXSAL_FD_SETSIZE) \
        ((NEXSALFDSet *)(set))->fd_array[((NEXSALFDSet *)(set))->fd_count++]=(fd);\
}


#define NEXSAL_FD_ZERO(set) (((NEXSALFDSet *)(set))->fd_count=0)

	NEXSAL_API NXBOOL nexSAL_FDIsSet(NEXSALSockHandle a_hSock, NEXSALFDSet *a_pFDS);

// this comment will be removed when next major version will be changed. by yoosh
//NXBOOL nexSal_FDIsSet(NEXSALSockHandle hSock, NEXSALFDSet *pSet); // will be deleted.

#define NEXSAL_FD_ISSET(fd, set) nexSAL_FDIsSet(fd, set)


/*
 * File open mode
 */
	typedef enum NEXSALFileMode
	{
		NEXSAL_FILE_READ         = 1, /**< Read                      */
		NEXSAL_FILE_WRITE        = 2, /**< Write                     */
		NEXSAL_FILE_READWRITE    = 3, /**< Read and Write            */
		NEXSAL_FILE_CREATE       = 4  /**< Create                    */

	} NEXSALFileMode;

/*
 * File seek mode
 */
	typedef enum NEXSALFileSeekOrigin
	{
		NEXSAL_SEEK_BEGIN        = 0,  /**< Beginning of file         */
		NEXSAL_SEEK_CUR          = 1,  /**< Current position          */
		NEXSAL_SEEK_END          = 2   /**< End of file               */ 

	} NEXSALFileSeekOrigin;

/*
 * File Attribute
 */
#define	NEXSAL_FILE_ATTR_NORMAL		0x00	/* Normal file - No read/write restrictions */
#define	NEXSAL_FILE_ATTR_RDONLY		0x01	/* Read only file */
#define	NEXSAL_FILE_ATTR_HIDDEN		0x02	/* Hidden file */
#define	NEXSAL_FILE_ATTR_SYSTEM		0x04	/* System file */
#define	NEXSAL_FILE_ATTR_SUBDIR		0x10	/* Subdirectory */
#define	NEXSAL_FILE_ATTR_ARCH		0x20	/* Archive file */

/*
 * File search information
 */
#define NEXSAL_MAX_FILENAME		1024
	typedef struct NEXSALFindInfo
	{
		NXUINT32    m_uiAttribute;
		NXUINT32    m_dwFileSize;
		NXCHAR      m_szFilename[NEXSAL_MAX_FILENAME];
		NXUINT32    m_uiCreateDate;

	} NEXSALFindInfo;


#define NEXSAL_INFINITE          (~0)

/*
 * Event reset method setting
 */
	typedef enum NEXSALEventReset
	{
		NEXSAL_EVENT_MANUAL     = 0,		 /**< Manual reset       */
		NEXSAL_EVENT_AUTO       = 1			 /**< Auto reset         */

	} NEXSALEventReset;

/*
 * Event initial state setting.
 */
	typedef enum NEXSALEventInitial
	{
		NEXSAL_EVENT_SET       = 0,			 /**< Initial state signaled     */ 
		NEXSAL_EVENT_UNSET     = 1			 /**< Initial state nonsignaled  */

	} NEXSALEventInitial;

/*
 * Task Priority Setting 
 */
	typedef enum NEXSALTaskPriority 
	{
		NEXSAL_PRIORITY_LOWEST        = 10000, /**< Lowest Priority           */
		NEXSAL_PRIORITY_LOW           = 10010, /**< Low    Priority           */
		NEXSAL_PRIORITY_BELOW_NORMAL  = 10020, /**< Below normal Priority     */
		NEXSAL_PRIORITY_NORMAL        = 10030, /**< Normal Priority           */
		NEXSAL_PRIORITY_ABOVE_NORMAL  = 10040, /**< Above normal Priority     */
		NEXSAL_PRIORITY_HIGH          = 10050, /**< High Priority             */
		NEXSAL_PRIORITY_URGENT        = 10060, /**< Urgent Priority           */
		NEXSAL_PRIORITY_HIGHEST       = 10070  /**< Highest Priority          */

	} NEXSALTaskPriority;

/*
 * Task Creation Flag
 */
	typedef enum NEXSALTaskOption
	{
		NEXSAL_TASK_NO_OPTION   = 0,
		NEXSAL_TASK_USE_DSP     = 1,	 /**< Task using dsp. */
		NEXSAL_TASK_USE_FILE    = 2,		 /* for Task using filesystem */
	} NEXSALTaskOption;

	typedef enum NEXSALSockType
	{
		NEXSAL_SOCK_STREAM      = 0,
		NEXSAL_SOCK_DGRAM       = 1,
		NEXSAL_SOCK_SSL         = 2,
	} NEXSALSockType;

	typedef enum NEXSALGroupManage
	{
		NEXSAL_MULTI_ADD        = 0,	/* add an IP group membership */
		NEXSAL_MULTI_DROP       = 1,	/* drop an IP group membership */ 
	} NEXSALGroupManage;

/*
 * Task start functions type
 */
	typedef NXINT32 (*NEXSALTaskFunc)(NXVOID *);
	

/******************************************
 *	SAL return code definition
 ******************************************/

typedef enum {
	eNEXSAL_RET_NO_ERROR			= 0,
	eNEXSAL_RET_GENERAL_ERROR		= -1,
	eNEXSAL_RET_TIMEOUT				= -2,
	eNEXSAL_RET_NOT_AVAILABLE		= -3,
	eNEXSAL_RET_EOF 				= -4,
	eNEXSAL_RET_SSL_CONNECT_FAIL	= -5,
	eNEXSAL_RET_NET_UNREACHABLE		= -6,
	eNEXSAL_RET_DNS_FAIL			= -7,
	eNEXSAL_RET_SSL_CERT_FAIL		= -8,
	eNEXSAL_RET_SOCK_ERROR			= -9,
	eNEXSAL_RET_WOULDBLOCK		 	= -10,
	eNEXSAL_RET_CONNECTION_CLOSE	= -11,
	//MUST add return codes to the last line
	
	eNEXSAL_RET_PADDING				= MAX_SIGNED32BIT
} NEXSAL_RETURN;

/*
#define eNEXSAL_RET_NO_ERROR                      0
// by yoosh. following value should be reviewd.
#define NEXSAL_WAIT_TIMEOUT              -100
#define NEXSAL_WAIT_FAILED               -101

#define NEXSAL_FILE_RET_NOT_AVAILABLE		-2
//by YK
#define NEXSAL_FILE_RET_EOF		0

// following 3 constants can be return value of NEXSALNetConnect, NEXSALNetRecvFrom, NEXSALNetRecv
#define NEXSAL_SOCK_RET_SSL_CONNECT			-11
#define NEXSAL_SOCK_RET_WOULDBLOCK			-10
#define NEXSAL_SOCK_RET_DNSFAIL				-9
#define NEXSAL_SOCK_RET_SSL_CERT_FAIL		-3
#define NEXSAL_SOCK_RET_TIMEOUT				-2
#define NEXSAL_SOCK_RET_ERROR				-1
#define NEXSAL_SOCK_RET_CONNECTION_CLOSE	0
*/

/******************************************
 *	SAL functions type definition part
 ******************************************/

/* NEXSALRemovedFunction type is needed for the entry of table for removed functions */
	typedef NXVOID (*NEXSALRemovedFunction) ();

/* Memory */
	typedef NXVOID* (*NEXSALMemAlloc) (NXUSIZE a_uSize, const NXCHAR *a_strFileName, NXINT32 a_nLine);
	typedef NXVOID* (*NEXSALMemCalloc) (NXUSIZE a_uNum, NXUSIZE a_uSize, const NXCHAR *a_strFileName, NXINT32 a_nLine);
	typedef NXVOID (*NEXSALMemFree) (NXVOID *a_pMem, const NXCHAR *a_strFileName, NXINT32 a_nLine);
	typedef NXVOID* (*NEXSALMemAlloc2) (NXUSIZE a_uSize, NXUINT32 a_uType, const NXCHAR *a_strFileName, NXINT32 a_nLine);
	typedef NXVOID (*NEXSALMemFree2) (NXVOID *a_pMem);

/* File / Directory */
	typedef NEXSALFileHandle (*NEXSALFileOpenA) (const NXCHAR *a_strFilePath, NEXSALFileMode a_eMode);
	typedef NEXSALFileHandle (*NEXSALFileOpenW) (const NXWCHAR *a_strFilePath, NEXSALFileMode a_eMode);
	typedef NEXSAL_RETURN (*NEXSALFileClose) (NEXSALFileHandle a_hFile);
	typedef NXSSIZE (*NEXSALFileRead) (NEXSALFileHandle a_hFile, NXVOID *a_pBuf, NXUSIZE a_uReadLen);
	typedef NXSSIZE (*NEXSALFileWrite) (NEXSALFileHandle a_hFile, NXVOID *a_pBuf, NXUSIZE a_uWriteLen);
	typedef NXINT32 (*NEXSALFileSeek) (NEXSALFileHandle a_hFile, NXINT32 a_nOffset, NEXSALFileSeekOrigin a_eOrigin);
	typedef NXINT64 (*NEXSALFileSize) (NEXSALFileHandle a_hFile);
	typedef NXINT64 (*NEXSALFileFreeDiskSpaceA) (NXCHAR *a_strPath);
	typedef NXINT64 (*NEXSALFileFreeDiskSpaceW) (NXWCHAR *a_wstrPath);

	typedef NEXSAL_RETURN (*NEXSALFileRemoveA) (const NXCHAR *a_strFilePath);
	typedef NEXSAL_RETURN (*NEXSALFileRemoveW) (const NXWCHAR *a_wstrFilePath);
	typedef NEXSAL_RETURN (*NEXSALFileRenameA) (const NXCHAR *a_strOldName, const NXCHAR *a_strNewName);
	typedef NEXSAL_RETURN (*NEXSALFileRenameW) (const NXWCHAR *a_wstrOldName, const NXWCHAR *a_wstrNewName);
	typedef NEXSAL_RETURN (*NEXSALFileGetAttributeA) (const NXCHAR *a_strFilePath, NXUINT32 *a_puAttribute);
	typedef NEXSAL_RETURN (*NEXSALFileGetAttributeW) (const NXWCHAR *a_wstrFilePath, NXUINT32 *a_puAttribute);
	typedef NEXSAL_RETURN (*NEXSALDirMakeA) (const NXCHAR *a_strDirName);
	typedef NEXSAL_RETURN (*NEXSALDirMakeW) (const NXWCHAR *a_wstrDirName);
	typedef NEXSAL_RETURN (*NEXSALDirRemoveA) (const NXCHAR *a_strDirName);
	typedef NEXSAL_RETURN (*NEXSALDirRemoveW) (const NXWCHAR *a_wstrDirName);
	typedef NEXSALSearchHandle (*NEXSALFindFirstA) (NXCHAR *a_strFileSpec, NEXSALFindInfo *a_pstFindInfo);
	typedef NEXSALSearchHandle (*NEXSALFindFirstW) (NXWCHAR *a_strFileSpec, NEXSALFindInfo *a_pstFindInfo);
	typedef NEXSAL_RETURN (*NEXSALFindNext) (NEXSALSearchHandle a_hSearch, NEXSALFindInfo *a_pstFindInfo);
	typedef NEXSAL_RETURN (*NEXSALFindClose) (NEXSALSearchHandle a_hSearch);
	typedef NXINT64 (*NEXSALFileSeek64) (NEXSALFileHandle a_hFile, NXINT64 a_nOffset, NEXSALFileSeekOrigin a_eOrigin);

/* Event */
	typedef NEXSALEventHandle (*NEXSALEventCreate) (NEXSALEventReset a_eManualReset, NEXSALEventInitial a_eInitialSet);
	typedef NEXSAL_RETURN (*NEXSALEventDelete) (NEXSALEventHandle a_hEvent);
	typedef NEXSAL_RETURN (*NEXSALEventSet) (NEXSALEventHandle a_hEvent);
	typedef NEXSAL_RETURN (*NEXSALEventWait) (NEXSALEventHandle a_hEvent, NXUINT32 a_uTimeoutMSec);
	typedef NEXSAL_RETURN (*NEXSALEventClear) (NEXSALEventHandle a_hEvent);

/* Mutex */
	typedef NEXSALMutexHandle (*NEXSALMutexCreate) ();
	typedef NEXSAL_RETURN (*NEXSALMutexDelete) (NEXSALMutexHandle a_hMutex);
	typedef NEXSAL_RETURN (*NEXSALMutexLock) (NEXSALMutexHandle a_hMutex, NXUINT32 a_uTimeoutMSec);
	typedef NEXSAL_RETURN (*NEXSALMutexUnlock) (NEXSALMutexHandle a_hMutex);

/* Semaphore */
	typedef NEXSALSemaphoreHandle (*NEXSALSemaphoreCreate) (NXINT32 a_nInitCount, NXINT32 a_nMaxCount);
	typedef NEXSAL_RETURN (*NEXSALSemaphoreDelete) (NEXSALSemaphoreHandle a_hSema);
	typedef NEXSAL_RETURN (*NEXSALSemaphoreRelease) (NEXSALSemaphoreHandle a_hSema);
	typedef NEXSAL_RETURN (*NEXSALSemaphoreWait) (NEXSALSemaphoreHandle a_hSema, NXUINT32 a_uTimeoutMSec);

/* atomic inc/dec */
	typedef NXINT32 (*NEXSALAtomicInc) (NXINT32 *a_pnValue);
	typedef NXINT32 (*NEXSALAtomicDec) (NXINT32 *a_pnValue);

/* Task */    
	typedef NEXSALTaskHandle (*NEXSALTaskCreate) (
												 const NXCHAR*           a_strTaskName,
												 NEXSALTaskFunc          a_fnTask, 
												 NXVOID *                a_pParam, 
												 NEXSALTaskPriority      a_ePriority, 
												 NXUINT32                a_uStackSize, 
												 NEXSALTaskOption        a_eCreateOption);

	typedef NEXSAL_RETURN (*NEXSALTaskDelete) (NEXSALTaskHandle a_hTask);
	typedef NEXSALTaskHandle (*NEXSALTaskCurrent) ();
	typedef NEXSAL_RETURN (*NEXSALTaskGetPriority) (NEXSALTaskHandle a_hTask, NEXSALTaskPriority *a_pePriority);
	typedef NEXSAL_RETURN (*NEXSALTaskSetPriority) (NEXSALTaskHandle a_hTask, NEXSALTaskPriority a_ePriority);

	typedef NEXSAL_RETURN (*NEXSALTaskSleep) (NXUINT32 a_uMSec);
	typedef NEXSAL_RETURN (*NEXSALTaskTerminate) (NEXSALTaskHandle a_hTask);
	typedef NEXSAL_RETURN (*NEXSALTaskWait) (NEXSALTaskHandle a_hTask);

/* Time */
	typedef NXUINT32 (*NEXSALGetTickCount) ();
	typedef NXVOID (*NEXSALGetMSecFromEpoch) (NXUINT64 *a_puqEpochMS);

/* Socket */
	typedef NEXSALSockHandle (*NEXSALNetSocket) (NEXSALHandle a_hSAL, NEXSALSockType a_eType);
	typedef NEXSAL_RETURN (*NEXSALNetClose) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock);
	typedef NEXSAL_RETURN (*NEXSALNetConnect) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, const NXCHAR *a_strAddr, NXUINT16 a_uwPort, NXUINT32 a_uTimeout);
	typedef NEXSAL_RETURN (*NEXSALNetBind) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXUINT16 a_uwPort);
	typedef NXINT32 (*NEXSALNetSelect) (NEXSALHandle a_hSAL, NEXSALFDSet *a_pReadFDS, NEXSALFDSet *a_pWriteFDS, NEXSALFDSet *a_ExceptFDS, NEXSALTimeValue *a_pTimeValue);
	typedef NXSSIZE (*NEXSALNetSendTo) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXCHAR *a_pBuf, NXUSIZE a_uLen, NXCHAR *a_strAddr, NXUINT16 a_uwPort);
	typedef NXSSIZE (*NEXSALNetSend) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXCHAR *a_pBuf, NXUSIZE a_uLen);
	typedef NXSSIZE (*NEXSALNetRecvFrom) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXCHAR *a_pBuf, NXUSIZE a_uLen, 
										   NXUINT32 *a_puAddr, NXUINT16 *a_uwPort, NXUINT32 a_uTimeout);
	typedef NXSSIZE (*NEXSALNetRecv) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXCHAR *a_pBuf, NXUSIZE a_uLen, NXUINT32 a_uTimeout);
	typedef NEXSAL_RETURN (*NEXSALNetMultiGroup) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXCHAR *a_strMultiGroupAddr, NEXSALGroupManage a_eProp);
	typedef NEXSAL_RETURN (*NEXSALNetSSLTunneling) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock);
	typedef NEXSAL_RETURN (*NEXSALNetListen) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXINT32 a_nBackLog);
	typedef NEXSALSockHandle (*NEXSALNetAccept) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXUINT32 *a_puAddr, NXUINT16 *a_puwPort, NXUINT32 a_uTimeOutInMSec, NXINT32 *a_pnResult);
	typedef NEXSAL_RETURN (*NEXSALNetAsyncConnect) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXCHAR *a_strAddr, NXUINT16 a_uwPort, NXUINT32 a_uTimeout);
	typedef NEXSAL_RETURN (*NEXSALNetAsyncSSLTunneling) (NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock);

/* Debugging */
	typedef NXVOID (*NEXSALDebugPrintf) (const NXCHAR *a_strFormat, ...);
	typedef NXVOID (*NEXSALDebugOutputString) (NXCHAR *a_strOutput);

/******************************************
 *	Table of SAL functions
 ******************************************/

/* Memory */
	typedef struct _NEXSALMemTable_
	{
		NEXSALMemAlloc              fnMemAlloc;
		NEXSALMemCalloc             fnMemCalloc;
		NEXSALMemFree               fnMemFree;
		NEXSALMemAlloc2             fnMemAlloc2;
		NEXSALMemFree2              fnMemFree2;

	} NEXSALMemoryFunctionTable;

/* File / Directory */
	typedef struct _NEXSALFileTable_
	{
		NEXSALFileClose             fnFileClose;
		NEXSALFileFreeDiskSpaceA    fnFileFreeDiskSpaceA;
		NEXSALFileOpenA             fnFileOpenA;
		NEXSALFileRead              fnFileRead;
		NEXSALFileRemoveA           fnFileRemoveA;
		NEXSALFileSeek              fnFileSeek;
		NEXSALFileSeek64            fnFileSeek64;
		NEXSALFileSize              fnFileSize;
		NEXSALFileWrite             fnFileWrite;
		NEXSALFileRenameA           fnFileRenameA;
		NEXSALFileGetAttributeA     fnFileGetAttributeA;
		NEXSALDirMakeA              fnDirMakeA;
		NEXSALDirRemoveA            fnDirRemoveA;
		NEXSALFindFirstA            fnFindFirstA;
		NEXSALFindNext              fnFindNext;
		NEXSALFindClose             fnFindClose;
		NEXSALFileOpenW             fnFileOpenW;			//18
		NEXSALFileRemoveW           fnFileRemoveW;
		NEXSALFileFreeDiskSpaceW    fnFileFreeDiskSpaceW;
		NEXSALFileRenameW           fnFileRenameW;
		NEXSALFileGetAttributeW     fnFileGetAttributeW;
		NEXSALDirMakeW              fnDirMakeW;
		NEXSALDirRemoveW            fnDirRemoveW;
		NEXSALFindFirstW            fnFindFirstW;

	} NEXSALFileFunctionTable;

/* Synchronization Object */
	typedef struct _NEXSALSyncObjectTable_
	{
		/* Event */
		NEXSALEventCreate           fnEventCreate;
		NEXSALEventDelete           fnEventDelete;
		NEXSALEventSet              fnEventSet;
		NEXSALEventWait             fnEventWait;
		NEXSALEventClear            fnEventClear;

		/* Mutex */
		NEXSALMutexCreate           fnMutexCreate;
		NEXSALMutexDelete           fnMutexDelete;
		NEXSALMutexLock             fnMutexLock;
		NEXSALMutexUnlock           fnMutexUnlock;

		/* Semaphore */
		NEXSALSemaphoreCreate       fnSemaphoreCreate;
		NEXSALSemaphoreDelete       fnSemaphoreDelete;
		NEXSALSemaphoreRelease      fnSemaphoreRelease;
		NEXSALSemaphoreWait         fnSemaphoreWait;

		NEXSALAtomicInc             fnAtomicInc;
		NEXSALAtomicDec             fnAtomicDec;

	} NEXSALSyncObjectFunctionTable;

/* Task */
	typedef struct _NEXSALTaskTable_
	{
		NEXSALTaskCreate            fnTaskCreate;
		NEXSALTaskCurrent           fnTaskCurrent;
		NEXSALTaskDelete            fnTaskDelete;
		NEXSALTaskGetPriority       fnTaskGetPriority;
		NEXSALTaskSetPriority       fnTaskSetPriority;
		NEXSALTaskSleep             fnTaskSleep;
		NEXSALTaskTerminate         fnTaskTerminate;
		NEXSALTaskWait              fnTaskWait;

	} NEXSALTaskFunctionTable;

/* Socket */
	typedef struct _NEXSALSocketTable_
	{
		NEXSALNetSocket             fnNetSocket;
		NEXSALNetClose              fnNetClose;
		NEXSALNetConnect            fnNetConnect;
		NEXSALNetBind               fnNetBind;
		NEXSALNetSelect             fnNetSelect;
		NEXSALNetSendTo             fnNetSendTo;
		NEXSALNetSend               fnNetSend;
		NEXSALNetRecvFrom           fnNetRecvFrom;
		NEXSALNetRecv               fnNetRecv;
		NEXSALNetMultiGroup         fnNetMultiGroup;
		NEXSALNetSSLTunneling       fnNetSSLTunneling;
		NEXSALNetListen             fnNetListen;
		NEXSALNetAccept             fnNetAccept;
		NEXSALNetAsyncConnect       fnNetAsyncConnect;
		NEXSALNetAsyncSSLTunneling  fnNetAsyncSSLTunneling;

	} NEXSALSocketFunctionTable;

	typedef struct _NEXSALTraceTable_
	{
		NEXSALDebugPrintf           fnDegPrintf;
		NEXSALDebugOutputString     fnOutputString;

	} NEXSALTraceFunctionTable;

/* Etc */
	typedef struct _NEXSALEtcTable_
	{
		NEXSALGetTickCount          fnGetTickCount;
		NEXSALGetMSecFromEpoch		fnGetMSecFromEpoch;
	} NEXSALEtcFunctionTable;

	extern NEXSAL_API   NEXSALMemoryFunctionTable*      g_nexSALMemoryTable;
	extern NEXSAL_API   NEXSALFileFunctionTable*        g_nexSALFileTable;
	extern NEXSAL_API   NEXSALSyncObjectFunctionTable*  g_nexSALSyncObjectTable;
	extern NEXSAL_API   NEXSALTaskFunctionTable*        g_nexSALTaskTable;
	extern NEXSAL_API   NEXSALEtcFunctionTable*         g_nexSALEtcTable;
	extern NEXSAL_API   NEXSALSocketFunctionTable*      g_nexSALSocketTable;
	extern NEXSAL_API   NEXSALTraceFunctionTable*       g_nexSALTraceTable;

	NEXSAL_API const NXCHAR* nexSAL_GetVersionString();
	NEXSAL_API NXINT32 nexSAL_GetMajorVersion();
	NEXSAL_API NXINT32 nexSAL_GetMinorVersion();
	NEXSAL_API NXINT32 nexSAL_GetPatchVersion();
	NEXSAL_API const NXCHAR* nexSAL_GetBranchVersion();
	NEXSAL_API const NXCHAR* nexSAL_GetVersionInfo();
	NEXSAL_API NXBOOL nexSAL_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch);
	NEXSAL_API NXBOOL nexSAL_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum);
	NEXSAL_API NXVOID nexSAL_MemDump(NXVOID *a_pSrc, NXINT32 a_nSize);
	NEXSAL_API NEXSALHandle nexSAL_Create(NXUSIZE a_uID);
	NEXSAL_API NXVOID nexSAL_Destroy(NEXSALHandle a_hSAL);
	NEXSAL_API NXBOOL nexSAL_SetUserData(NEXSALHandle a_hSAL, NXVOID *a_pUserData);
	NEXSAL_API NXBOOL nexSAL_GetUserData(NEXSALHandle a_hSAL, NXVOID **a_ppUserData);
	NEXSAL_API NXUSIZE nexSAL_GetUID(NEXSALHandle a_hSAL);

// This part is for MEM_ALLOC2's type values
// HIWORD
#define NEXSAL_MEM_TYPE_NORMAL		0x00000000
#define NEXSAL_MEM_TYPE_BITSTREAM	0x00010000
// LOWORD
#define NEXSAL_MEM_ALIGN_NORMAL		0x00000000
#define NEXSAL_MEM_ALIGN_2			0x00000001
#define NEXSAL_MEM_ALIGN_4			0x00000002
#define NEXSAL_MEM_ALIGN_8			0x00000004
#define NEXSAL_MEM_ALIGN_16			0x00000008
#define NEXSAL_MEM_ALIGN_32			0x00000010
#define NEXSAL_MEM_ALIGN_64			0x00000020
#define NEXSAL_MEM_ALIGN_128		0x00000040


/* This part is for TraceCategory */
#define NEXSAL_MAX_TRACE_CATEGORY	30
#define NEXSAL_MAX_TRACE_LEVEL		10
#define NEXSAL_MAX_TRACE_PREFIX		3

/* The following NEX_TRACE_CATEGORY_... can be ignored. Use as your plan */
#define NEX_TRACE_CATEGORY_FLOW						0
#define NEX_TRACE_CATEGORY_E_AUDIO					1
#define NEX_TRACE_CATEGORY_E_VIDEO					2
#define NEX_TRACE_CATEGORY_P_AUDIO					3
#define NEX_TRACE_CATEGORY_P_VIDEO					4
#define NEX_TRACE_CATEGORY_E_SYS					5
#define NEX_TRACE_CATEGORY_P_SYS					6
#define NEX_TRACE_CATEGORY_TEXT						7
#define NEX_TRACE_CATEGORY_DLOAD					8
#define NEX_TRACE_CATEGORY_INFO						9
#define NEX_TRACE_CATEGORY_WARNING					10
#define NEX_TRACE_CATEGORY_ERR						11
#define NEX_TRACE_CATEGORY_F_READER					12
#define NEX_TRACE_CATEGORY_F_WRITER					13
#define NEX_TRACE_CATEGORY_PVPD						14
#define NEX_TRACE_CATEGORY_PROTOCOL					15
#define NEX_TRACE_CATEGORY_CRAL						16
#define NEX_TRACE_CATEGORY_SOURCE					17
#define NEX_TRACE_CATEGORY_TARGET					18
#define NEX_TRACE_CATEGORY_DIVXDRM					19
#define NEX_TRACE_CATEGORY_RFC						20
#define NEX_TRACE_CATEGORY_NONE						21
#define NEX_TRACE_CATEGORY_FASTPLAY 				22
#define NEX_TRACE_CATEGORY_HD						23
#define NEX_TRACE_CATEGORY_TESTCASE					24

#define NEX_TRACE_CATEGORY_USER_START				1000
// 20 ~ : 각 응용에서 알아서 사용

#define NEXSAL_MAX_DEBUG_STRING_LENGTH 512

	NEXSAL_API NXBOOL nexSAL_TraceCondition(NXINT32 a_nCategory, NXINT32 a_nLevel);
	NEXSAL_API NXVOID nexSAL_TraceGetCondition(NXINT32 a_nCategory, NXINT32 *a_pnLevel);
	NEXSAL_API NXVOID nexSAL_TraceSetCondition(NXINT32 a_nCategory, NXINT32 a_nLevel);
	NEXSAL_API NXVOID nexSAL_TraceSetPrefix(NXINT32 a_nCategory, const NXCHAR *a_szPrefix);
	NEXSAL_API NXVOID nexSAL_TraceCat(NXINT32 a_nCategory, NXINT32 a_nLevel, const NXCHAR *a_strFormat, ...);
	NEXSAL_API NXVOID nexSAL_TraceCat2(NEXSALHandle a_hSAL, NXINT32 a_nCategory, NXINT32 a_nLevel, const NXCHAR *a_strFormat, ...);

#ifdef __cplusplus
}
#endif

#endif // _NEXSAL_COMMON_HEADER_INCLUDED_

/*
Revision History:
Author		Date			Version		Description of Changes
-------------------------------------------------------------------------------
pss			2005/08/11		1.0			Draft
ysh			2005/08/15		1.1			Revision
ysh			2006/03/12		1.2			NEXSAL_TRACE_MODULE added
hsc			2006/03/30		1.3			File related 7 functions is added
ysh			2006/04/11		1.4/0		Revision(classifying functions, _VersionCheck)
pss         	2006/04/27      	1.5/0       	Remove warning in enumeration def.
pss        		2006/04/27      	1.5/1      	 Add Primitive type and define       
pss         	2006/05/03      	1.5/2       	Add Sync Return Define
pss         	2006/06/08     	 2.0         	TaskCreate Prototype Changed
										FileSizeEx, FileFreeDiskSpaceEx Added
ysh			2006/08/08		2.1			Apis related to Socket is revised.
ysh			2006/09/12		2.2			UNICODE is introduced. nexSAL_FileOpen(A/W) is adapted.
pss			2006/09/27		2.2/1	    	nexSAL_FileRemove(A/W) is adapted.(temporarily)
ysh			2006/10/10		2.3			Version up. patch number is added.
ysh			2007/04/12		2.3.3		converted to DLL version in WIN32 or WINCE
ysh			2007/06/27		2.3.4		_USRDLL is removed. nexSAL_IsFDSet is modified.
ysh			2007/09/16		2.4.0		nexSAL_MemDump is added.
										nexSal_FDIsSet/nexSAL_NetRecvFromMulti/nexSAL_NetRecvFrom2 is removed.
ysh			2008/06/13		2.5.0		Unicode functions(on File) was added.
ysh			2008/07/28		2.6.0		nexSAL_Trace????? functions were added.
ysh			2008/09/29		2.6.1		nexSAL_MemAlloc2,nexSAL_MemFree2 function was added.
ysh			2009/12/21		2.6.2		nexSAL_FileSeek64 was added. NX(U)INT64
robin		2010/06/01		2.6.3		Malloc, Calloc, Malloc2 Parameter Added for Memory Leak
ysh			2011/05/26		2.6.4		atomic inc/dec were added.
ysh			2011/09/27		2.6.5		NEXSALNetSSLTunneling/fnNetListen/fnNetAccept were added.
...
robin		2014/07/29		3.2.0		move type define to "NexTypeDef.h",
										change socket handle "NXVOID*" to "NXINT32"
										add socket api return type
										NEXSALNetAsyncConnect, NEXSALNetAsyncSSLTunneling api added
										NEXSAL_PATCH_NUM -> NEXSAL_VERSION_PATCH
robin		2014/08/04		3.3.0		File/Task/Event/Mutex/Semaphore/Socket handle change to unsigned int
shoh		2014/09/02		4.0.0		changed the rule of version
										followed NexTypeDef.h to support OS regardless of bits
										recapped deprecated APIs
										changed FD size (16 -> 64)
shoh		2015/03/05		4.0.1		changed NexSAL APIs paramter about "const"
shoh		2015/08/31		4.0.2		1. fixed wrong memset for structure
										2. changed the format of trace log [thread/task id: time tick] in windows simulator
shoh		2015/12/17		4.1.0		1. changed return type
										2. added nexSAL_GetMSecFromEpoch()
-----------------------------------------------------------------------------*/

