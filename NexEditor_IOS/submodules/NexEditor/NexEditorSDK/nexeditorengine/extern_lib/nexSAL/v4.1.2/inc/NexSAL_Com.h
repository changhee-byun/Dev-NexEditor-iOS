/******************************************************************************
* File Name   :	NexSAL_Com.h
* Description :	Common header file for nexSAL
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2005~2016 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

/**
* @mainpage NexSAL documentation
*
* Nexstreaming's NexSAL(System Abstraction Layer) is a set of function interface definitions.
* NexSAL decouples the application from the system OS. As a result, the application can be developed independently of the system.\n\n
* This document describes each function interface and operation for implementing NexSAL, and explains how to register that implementation.
* The NexSAL interface covers Task, File, Dynamic Memory Allocation, and Task Synchronization.
* Each application uses a different set of interfaces, so application developers should identify what interfaces of NexSAL is needed for their application
* before implementing NexSAL interfaces. That is, application developers need to support only the subset instead of the entire NexSAL interfaces.\n\n
* NexPlayer, NexCam, NexEditor, and more products use NexSAL.
* To port the Nexstreaming products, the functions described in this document must be implemented and registered.
* But all products do not need all functions in this document.
* Depending on the product, necessary functions are different.
* For the necessary functions by product, refer to the TRM document of the relevant product.
* If functions in this document are not described in the TRM of the relevant product, the functions are not necessary to be implemented.
*
* @note Important: NexSAL refers to NexCommonDef.h, NexMediaDef.h, NexTypeDef.h.
*/


/**
 * @defgroup sal_basic NexSAL Basic
 * @brief Basic Definition for NexSAL
 * @{
 * @}
 * @defgroup sal_memory Memory
 * @brief Memory APIs
 * @{
 * @}
 * @defgroup sal_file File
 * @brief File APIs
 * @{
 * @}
 * @defgroup sal_event Event
 * @brief Event APIs
 * @{
 * @}
 * @defgroup sal_mutex Mutex
 * @brief Mutex APIs
 * @{
 * @}
 * @defgroup sal_semaphore Semaphore
 * @brief Semaphore APIs
 * @{
 * @}
 * @defgroup sal_automic Automic
 * @brief Automic APIs
 * @{
 * @}
 * @defgroup sal_task Task
 * @brief Task APIs
 * @{
 * @}
 * @defgroup sal_socket Socket
 * @brief Socket APIs
 * @{
 * @}
 * @defgroup sal_fd File Descriptor
 * @brief File Descriptor
 * @{
 * @}
 * @defgroup sal_debug Debug
 * @brief Debug APIs
 * @{
 * @}
 * @defgroup sal_etc ETC
 * @brief ETC APIs
 * @{
 * @}
 */


#ifndef _NEXSAL_COMMON_HEADER_INCLUDED_
#define _NEXSAL_COMMON_HEADER_INCLUDED_

#include "NexTypeDef.h"

#define NEXSAL_VERSION_MAJOR	4
#define NEXSAL_VERSION_MINOR	1
#define NEXSAL_VERSION_PATCH	2
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

/**
 * @ingroup sal_basic 
 * NexSAL Invalid Handle
 */
#define	NEXSAL_INVALID_HANDLE	0

/**
 * @ingroup sal_basic 
 * NexSAL Handle
 */
	typedef struct NEXSALHandle_struct *NEXSALHandle;

/**
 * @ingroup sal_file
 * NexSAL File Handle
 */
	typedef NXVOID* NEXSALFileHandle;

/**
 * @ingroup sal_file
 * NexSAL Search Handle
 */	
	typedef NXVOID* NEXSALSearchHandle;

/**
 * @ingroup sal_task
 * NexSAL Task Handle
 */	
	typedef NXVOID* NEXSALTaskHandle;

/**
 * @ingroup sal_event
 * NexSAL Event Handle
 */
	typedef NXVOID* NEXSALEventHandle;

/**
 * @ingroup sal_mutex
 * NexSAL Mutex Handle
 */	
	typedef NXVOID* NEXSALMutexHandle;

/**
 * @ingroup sal_semaphore
 * NexSAL Semaphore Handle
 */	
	typedef NXVOID* NEXSALSemaphoreHandle;

/**
 * @ingroup sal_socket
 * NexSAL Socket Handle
 */	
	typedef NXVOID* NEXSALSockHandle;


// deprecated because socket is handle
//#define NEXSAL_INVALID_SOCK		(NEXSALSockHandle)(~0) 

/**
 * @ingroup sal_socket
 * @brief Time Value Struct
 */
	typedef struct NEXSALTimeValue
	{
		NXUINT32    tv_sec;		///< seconds
		NXUINT32    tv_usec;	///< microseconds
	} NEXSALTimeValue;

/**
 * @ingroup sal_fd
 * @brief File Descriptor Number
 */
#define NEXSAL_FD_SETSIZE 64

/**
 * @ingroup sal_fd
 * @brief File Descriptor Set Structure\n
 *		place sockets into NEXSALFDSet for various purposes
 */
	typedef struct NEXSALFDSet
	{
		NXUINT32 fd_count;									///< The number of sockets in the set
		NEXSALSockHandle fd_array[NEXSAL_FD_SETSIZE];		///< An array of sockets that are in the set
	} NEXSALFDSet;

/**
 * @ingroup sal_fd
 * NEXSAL_FD_CLR(NEXSALSockHandle fd, NEXSALFDSet set)\n
 * Remove the descriptor fd from set
 * @param fd file description is NEXSALSockHandle
 * @param set NEXSALFDSet including fd
 * @return None
 * @see NEXSALFDSet
 * @see NEXSALSockHandle
 */
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

/**
 * @ingroup sal_fd
 * NEXSAL_FD_SET(NEXSALSockHandle fd, NEXSALFDSet set)\n
 * Add descriptor fd to set
 * @param fd file description is NEXSALSockHandle
 * @param set NEXSALFDSet including fd
 * @return None
 * @NEXSALFDSet
 * @NEXSALSockHandle
 */
#define NEXSAL_FD_SET(fd, set) { \
    if (((NEXSALFDSet *)(set))->fd_count < NEXSAL_FD_SETSIZE) \
        ((NEXSALFDSet *)(set))->fd_array[((NEXSALFDSet *)(set))->fd_count++]=(fd);\
}

/**
 * @ingroup sal_fd
 * NEXSAL_FD_ZERO(NEXSALFDSet set)\n
 * Initialize the set to the null set
 * @param set NEXSALFDSet including fd
 * @see NEXSALFDSet
 */
#define NEXSAL_FD_ZERO(set) (((NEXSALFDSet *)(set))->fd_count=0)

/**
 * @ingroup sal_fd
 * nexSAL_FDIsSet(NEXSALSockHandle a_hSock, NEXSALFDSet *a_pFDS)\n
 * Return TRUE if a_hSock is a member of the set. Otherwise, FALSE.
 * @param a_hSock Socket handle
 * @param a_pFDS NEXSALFDSet including fd
 * @return TRUE: set, FALSE: not set.
 * @see NEXSALFDSet
 */
	NEXSAL_API NXBOOL nexSAL_FDIsSet(NEXSALSockHandle a_hSock, NEXSALFDSet *a_pFDS);


// this comment will be removed when next major version will be changed. by yoosh
//NXBOOL nexSal_FDIsSet(NEXSALSockHandle hSock, NEXSALFDSet *pSet); // will be deleted.

#define NEXSAL_FD_ISSET(fd, set) nexSAL_FDIsSet(fd, set)


/**
 * @ingroup sal_file
 * File open mode
 */
	typedef enum NEXSALFileMode
	{
		NEXSAL_FILE_READ         = 1,	///< Read
		NEXSAL_FILE_WRITE        = 2,	///< Write
		NEXSAL_FILE_READWRITE    = 3,	///< Read and Write
		NEXSAL_FILE_CREATE       = 4	///< Create
	} NEXSALFileMode;

/**
 * @ingroup sal_file
 * File seek mode
 */
	typedef enum NEXSALFileSeekOrigin
	{
		NEXSAL_SEEK_BEGIN        = 0,	///< Beginning of file
		NEXSAL_SEEK_CUR          = 1,	///< Current position
		NEXSAL_SEEK_END          = 2	///< End of file
	} NEXSALFileSeekOrigin;

/**
 * @ingroup sal_file
 * File Attribute
 */
#define	NEXSAL_FILE_ATTR_NORMAL		0x00	///< Normal file - No read/write restrictions
#define	NEXSAL_FILE_ATTR_RDONLY		0x01	///< Read only file
#define	NEXSAL_FILE_ATTR_HIDDEN		0x02	///< Hidden file
#define	NEXSAL_FILE_ATTR_SYSTEM		0x04	///< System file
#define	NEXSAL_FILE_ATTR_SUBDIR		0x10	///< Subdirectory
#define	NEXSAL_FILE_ATTR_ARCH		0x20	///< Archive file


#define NEXSAL_MAX_FILENAME		1024
/**
* @ingroup sal_file
* File search information structure
*/
	typedef struct NEXSALFindInfo
	{
		NXUINT32    m_uiAttribute;
		NXUINT32    m_dwFileSize;
		NXCHAR      m_szFilename[NEXSAL_MAX_FILENAME];
		NXUINT32    m_uiCreateDate;
	} NEXSALFindInfo;

/**
* @ingroup sal_basic
* Definition for the infinite
*/
#define NEXSAL_INFINITE          (~0)

/**
 * @ingroup sal_event
 * Event reset method setting
 */
	typedef enum NEXSALEventReset
	{
		NEXSAL_EVENT_MANUAL     = 0,	///< manually reset after set
		NEXSAL_EVENT_AUTO       = 1		///< automatically reset after set

	} NEXSALEventReset;

/**
 * @ingroup sal_event
 * Event initial state setting.
 */
	typedef enum NEXSALEventInitial
	{
		NEXSAL_EVENT_SET       = 0,		///< Initial state signaled
		NEXSAL_EVENT_UNSET     = 1		///< Initial state nonsignaled

	} NEXSALEventInitial;

/**
 * @ingroup sal_task
 * Task Priority Setting 
 */
	typedef enum NEXSALTaskPriority 
	{
		NEXSAL_PRIORITY_LOWEST        = 10000, ///< Lowest Priority
		NEXSAL_PRIORITY_LOW           = 10010, ///< Low Priority
		NEXSAL_PRIORITY_BELOW_NORMAL  = 10020, ///< Below normal Priority
		NEXSAL_PRIORITY_NORMAL        = 10030, ///< Normal Priority
		NEXSAL_PRIORITY_ABOVE_NORMAL  = 10040, ///< Above normal Priority
		NEXSAL_PRIORITY_HIGH          = 10050, ///< High Priority
		NEXSAL_PRIORITY_URGENT        = 10060, ///< Urgent Priority
		NEXSAL_PRIORITY_HIGHEST       = 10070  ///< Highest Priority

	} NEXSALTaskPriority;

/**
 * @ingroup sal_task
 * Task Creation Flag
 */
	typedef enum NEXSALTaskOption
	{
		NEXSAL_TASK_NO_OPTION   = 0,
		NEXSAL_TASK_USE_DSP     = 1,	///< Task using dsp
		NEXSAL_TASK_USE_FILE    = 2,	///< for Task using filesystem
	} NEXSALTaskOption;

/**
 * @ingroup sal_socket
 * Socket Type
 */
	typedef enum NEXSALSockType
	{
		NEXSAL_SOCK_STREAM      = 0,	///< Used for TCP address system that is reliable
		NEXSAL_SOCK_DGRAM       = 1,	///< Used for UDP address system that is not necessarily reliable, but fast
		NEXSAL_SOCK_SSL         = 2,	///< SSL
	} NEXSALSockType;

/*
 * @ingroup sal_socket
 * Socket Type
 */
	typedef enum NEXSALGroupManage
	{
		NEXSAL_MULTI_ADD        = 0,	///< add an IP group membership
		NEXSAL_MULTI_DROP       = 1,	///< drop an IP group membership
	} NEXSALGroupManage; 

/**
* @ingroup sal_task
* Task start functions type
*/
	typedef NXINT32 (*NEXSALTaskFunc)(NXVOID *);


/**
* @ingroup sal_basic
* SAL return code definition
*/
	typedef enum
	{
		eNEXSAL_RET_NO_ERROR            = 0,	///< No Error
		eNEXSAL_RET_GENERAL_ERROR       = -1,	///< General Error
		eNEXSAL_RET_TIMEOUT             = -2,	///< Timeout
		eNEXSAL_RET_NOT_AVAILABLE       = -3,	///< Not Available
		eNEXSAL_RET_EOF                 = -4,	///< End of File
		eNEXSAL_RET_SSL_CONNECT_FAIL    = -5,	///< SSL Connect Fail
		eNEXSAL_RET_NET_UNREACHABLE     = -6,	///< Unreachable
		eNEXSAL_RET_DNS_FAIL            = -7,	///< DNS Fail
		eNEXSAL_RET_SSL_CERT_FAIL       = -8,	///< SSL Certification Fail
		eNEXSAL_RET_SOCK_ERROR          = -9,	///< Socket Error
		eNEXSAL_RET_WOULDBLOCK          = -10,	///< Would Block
		eNEXSAL_RET_CONNECTION_CLOSE    = -11,	///< Connection Close
		//MUST add return codes to the last line

		eNEXSAL_RET_PADDING             = MAX_SIGNED32BIT
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
		NEXSALGetMSecFromEpoch      fnGetMSecFromEpoch;
	} NEXSALEtcFunctionTable;

	extern NEXSAL_API   NEXSALMemoryFunctionTable*      g_nexSALMemoryTable;
	extern NEXSAL_API   NEXSALFileFunctionTable*        g_nexSALFileTable;
	extern NEXSAL_API   NEXSALSyncObjectFunctionTable*  g_nexSALSyncObjectTable;
	extern NEXSAL_API   NEXSALTaskFunctionTable*        g_nexSALTaskTable;
	extern NEXSAL_API   NEXSALEtcFunctionTable*         g_nexSALEtcTable;
	extern NEXSAL_API   NEXSALSocketFunctionTable*      g_nexSALSocketTable;
	extern NEXSAL_API   NEXSALTraceFunctionTable*       g_nexSALTraceTable;

/**
* @ingroup sal_basic
* @brief return version info with string
* @return Pointer of string which consists of 3 digits with one word and it means major version number, 
*	minor version number, patch version number, and branch version name in sequence. (Ex. "1.0.0.OFFICIAL")
*/
	NEXSAL_API const NXCHAR* nexSAL_GetVersionString();

/**
* @ingroup sal_basic
* @brief return major version with NXINT32
* @return Major version number
*/
	NEXSAL_API NXINT32 nexSAL_GetMajorVersion();

/**
* @ingroup sal_basic
* @brief return minor version with NXINT32
* @return Minor version number
*/
	NEXSAL_API NXINT32 nexSAL_GetMinorVersion();

/**
* @ingroup sal_basic
* @brief return patch version with NXINT32
* @return Patch version number
*/
	NEXSAL_API NXINT32 nexSAL_GetPatchVersion();

/**
* @ingroup sal_basic
* @brief return branch version with NXINT32
* @return Branch version string
*/
	NEXSAL_API const NXCHAR* nexSAL_GetBranchVersion();

/**
* @ingroup sal_basic
* @brief return version info with string
* @return Version information
*/
	NEXSAL_API const NXCHAR* nexSAL_GetVersionInfo();

/**
* @ingroup sal_basic
* @brief check library version whether the same or not
* @param[in] a_nMajor major version
* @param[in] a_nMinor minor version
* @param[in] a_nPatch patch version
* @param[in] a_pBranch branch version
* @return TRUE: the same version, FALSE: another version
*/
	NEXSAL_API NXBOOL nexSAL_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch);

/**
* @ingroup sal_basic
* @brief check compatibility number
* @param[in] a_nCompatibilityNumber compatibility number
* @return TRUE: compatible, FALSE: not compatible
*/
	NEXSAL_API NXBOOL nexSAL_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum);

/**
* @ingroup sal_basic
* @brief print values of memory address with hex within a_nSize
* @param[in] a_pSrc source address to print
* @param[in] a_nSize size to print
* @return None
*/
	NEXSAL_API NXVOID nexSAL_MemDump(NXVOID *a_pSrc, NXINT32 a_nSize);

/**
* @ingroup sal_basic
* @brief Create NexSAL Instance
* @param[in] a_uID unique ID to distiguish from other NexSALs
* @return Handle of NexSAL
* @see NEXSALHandle
*/
	NEXSAL_API NEXSALHandle nexSAL_Create(NXUSIZE a_uID);

/**
* @ingroup sal_basic
* @brief Destroy NexSAL Instance
* @param[in] a_hSAL Handle of NexSAL
* @return None
* @see NEXSALHandle
*/
	NEXSAL_API NXVOID nexSAL_Destroy(NEXSALHandle a_hSAL);

/**
* @ingroup sal_basic
* @brief Set user data
* @param[in] a_hSAL Handle of NexSAL
* @param[in] a_pUserData Pointer to user data
* @return TRUE: success, FALSE: fail
* @see NEXSALHandle
*/
	NEXSAL_API NXBOOL nexSAL_SetUserData(NEXSALHandle a_hSAL, NXVOID *a_pUserData);

/**
* @ingroup sal_basic
* @brief Get user data
* @param[in] a_hSAL Handle of NexSAL
* @param[in] a_ppUserData Double pointer to user data
* @return TRUE: success, FALSE: fail
* @see NEXSALHandle
*/
	NEXSAL_API NXBOOL nexSAL_GetUserData(NEXSALHandle a_hSAL, NXVOID **a_ppUserData);

/**
* @ingroup sal_basic
* @brief Get unique ID which was set by nexSAL_Create
* @param[in] a_hSAL Handle of NexSAL
* @return unique ID to distiguish from other NexSALs
* @see NEXSALHandle
*/
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
#define NEX_TRACE_CATEGORY_RTMP						25

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

