/******************************************************************************
* File Name   :	NexSAL_Internal.h
* Description :	Internal use of SAL header file
*******************************************************************************
	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2005~7 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXSAL_INTERNAL_HEADER_INCLUDED_
#define _NEXSAL_INTERNAL_HEADER_INCLUDED_

#if !defined(_NEXSAL_COMMON_HEADER_INCLUDED_)
	#include "NexSAL_Com.h"
#endif

/* _NEXSAL_TRACECATEGORY_OPERATED_ definition is for NEXSAL_TRACECATEGORY
   The size of release version binary can be bigger than the others.
*/
#define _NEXSAL_TRACECATEGORY_OPERATED_


/* Memory */
/**
 * \ingroup SALAPI
 * \brief NXVOID* nexSAL_MemAlloc(NXUSIZE a_uSize)
 * 
 * This function allocates a block of memory.
 *
 * \param a_uSize [in] Size in bytes of memory block to be allocated.
 *
 * \returns Pointer to the allocated memory block. If the available memory is insufficient, NULL is returned.
 *
 */
#define nexSAL_MemAlloc(a)			g_nexSALMemoryTable->fnMemAlloc(a, __FILE__, __LINE__)

/**
 * \ingroup SALAPI
 * \brief NXVOID* nexSAL_MemCalloc(NXUSIZE a_uNum, NXUSIZE a_uSize) 
 * 
 * This function allocates an array in memory with elements initialized to 0.
 *
 * \param a_uNum [in] Number of elements
 *
 * \param a_uSize [in] Size in bytes of each element
 *
 * \returns Pointer to the allocated memory. If the available memory is insufficient, NULL is returned.
 *
 */
#define nexSAL_MemCalloc(a, b)		g_nexSALMemoryTable->fnMemCalloc(a, b, __FILE__, __LINE__)

/**
 * \ingroup SALAPI
 * \brief NXVOID* nexSAL_MemFree(NXVOID *a_pMem)
 * 
 * This function frees a memory block.
 *
 * \param a_pMem [in] Pointer to the memory block to be freed
 *
 * \returns None
 *
 */
#define nexSAL_MemFree(a)			g_nexSALMemoryTable->fnMemFree(a, __FILE__, __LINE__)

/**
 * \ingroup SALAPI
 * \brief NXVOID* nexSAL_MemAlloc2(NXUSIZE a_uSize, NXUINT32 a_uType)
 * 
 * This function allocates an aligned block of memory.
 *
 * \param a_uSize [in] Size in bytes of memory block to be allocated
 *
 * \param a_uType [in] align bytes type<BR>
 *		NEXSAL_MEM_ALIGN_NORMAL	no align<BR>
 *		NEXSAL_MEM_ALIGN_2			2bytes align<BR>
 *		NEXSAL_MEM_ALIGN_4			4bytes align<BR>
 *		NEXSAL_MEM_ALIGN_8			8bytes align<BR>
 *		NEXSAL_MEM_ALIGN_16			16bytes align<BR>
 *		NEXSAL_MEM_ALIGN_32			32bytes align<BR>
 *		NEXSAL_MEM_ALIGN_64			64bytes align<BR>
 *		NEXSAL_MEM_ALIGN_128		128bytes align
 *
 * \returns Pointer to the allocated memory. If the available memory is insufficient, NULL is returned.
 *
 */
#define nexSAL_MemAlloc2(a,b)		g_nexSALMemoryTable->fnMemAlloc2(a, b, __FILE__, __LINE__)

/**
 * \ingroup SALAPI
 * \brief NXVOID* nexSAL_MemFree2(NXVOID *a_pMem)
 * 
 * This function frees a memory block for nexSAL_MemAlloc2.
 *
 * \param a_pMem [in] Pointer to the memory block to be freed
 *
 * \returns None
 *
 */
#define nexSAL_MemFree2				g_nexSALMemoryTable->fnMemFree2


/* File */
/**
 * \ingroup SALAPI
 * \brief NEXSALFileHandle nexSAL_FileOpenA(const NXCHAR *a_strFilePath, NEXSALFileMode a_eMode)
 * 
 * This function opens a file in not UNICODE.
 *
 * \param a_strFilePath [in] Path name of file to open
 *
 * \param a_eMode [in] Open flag. Some compositions are not supported by some platforms.
 *
 * \param a_strFilePath [in] Path name of file to open
 *		Available compositions:
 *	 	NEXSAL_FILE_READ: Read existing file.
 *	 	NEXSAL_FILE_WRITE: Append to existing file. (If it does not exist, operation not defined.)
 *		NEXSAL_FILE_READ | NEXSAL_FILE_WRITE (=NEXSAL_FILE_READWRITE): Read & write file (If not exist, operation not defined.)
 *	 	NEXSAL_FILE_WRITE | NEXSAL_FILE_CREATE: Create new file to write. If file exists, overwrite.
 *		NEXSAL_FILE_READ | NEXSAL_FILE_WRITE | NEXSAL_FILE_CREATE (=	NEXSAL_FILE_READWRITE | NEXSAL_FILE_CREATE)
 *	 		: Create new file to read, write. If file exists, overwrite.
 *
 * \returns Handle of the file object. NEXSAL_INVALID_HANDLE value indicates an error.
 *
 */
#define nexSAL_FileOpenA			g_nexSALFileTable->fnFileOpenA

/**
 * \ingroup SALAPI
 * \brief  NEXSALFileHandle nexSAL_FileOpenW(const NXWCHAR *a_strFilePath, NEXSALFileMode a_eMode)
 * 
 * This function opens a file in UNICODE. For Parameters meanings and return value, see '3.3.4 nexSAL_FileOpenA'
 *
 */
#define nexSAL_FileOpenW			g_nexSALFileTable->fnFileOpenW

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_FileClose(NEXSALFileHandle a_hFile)
 * 
 * This function closes an open file handle.
 *
 * \param a_hFile [in] Handle to an open file 
 *
 * \returns  
 *		eNEXSAL_RET_NO_ERROR: success<BR>
 *		Others: fail
 *
 */
#define nexSAL_FileClose			g_nexSALFileTable->fnFileClose

/**
 * \ingroup SALAPI
 * \brief NXSSIZE nexSAL_FileRead(NEXSALFileHandle a_hFile, NXVOID *a_pBuf, NXUSIZE a_uReadLen)
 * 
 * This function reads bytes from a file.
 *
 * \param a_hFile [in] Handle referring to open file
 *
 * \param a_pBuf [out] Pointer to the buffer that receives the data read from the file 
 *
 * \param a_uReadLen [in] Number of bytes to be read from the file
 *
 * \returns 
 *		> 0: If the work is successful, returns byte count of actual data<BR>
 *		0: It reached to the end of the tile - no more data to read.<BR>
 *		< 0: An error occurred during reading process.
 *
 */
#define nexSAL_FileRead				g_nexSALFileTable->fnFileRead

/**
 * \ingroup SALAPI
 * \brief NXSSIZE nexSAL_FileWrite(NEXSALFileHandle a_hFile, NXVOID *a_pBuf, NXUSIZE a_uWriteLen)
 * 
 * This function writes bytes to the file.
 *
 * \param a_hFile [in] Handle of file into which data are written
 *
 * \param a_pBuf [in] Pointer to the buffer that contains the data to write to file 
 *
 * \param a_uWriteLen [in] Number of bytes to write to the file
 *
 * \returns 
 *		>= 0: the number of bytes written, if the work is successful.<BR>
 *		< 0: Error occurred during writing process.
 *
 * \Remark/Tips/Suggestions 
 *		The nexSAL_FileWrite operation begins at the current position of the file pointer (if any) associated with the given file.<BR>
 *		After this operation, the file pointer is increased by the number of bytes actually written.
 */
#define nexSAL_FileWrite			g_nexSALFileTable->fnFileWrite

/**
 * \ingroup SALAPI
 * \brief NXINT32 nexSAL_FileSeek (NEXSALFileHandle a_hFile, NXINT32 a_nOffset, NEXSALFileSeekOrigin a_eOrigin)
 * 
 * This function moves a file pointer to the specified location.
 *
 * \param a_hFile [in] Handle referring to open file
 *
 * \param a_nOffset [in] Number of bytes from origin
 *
 * \param a_eOrigin [in] Starting point from which the file pointer moves
 * 			The iOrigin must be one of the following constants, which are defined in nexSal_com.h.
 *				NEXSAL_SEEK_BEGIN
 * 				NEXSAL_SEEK_END
 * 				NEXSAL_SEEK_CUR
 *
 * \returns 
 *		>= 0: If the work was successful the byte distance from the beginning of the file to the resultant position of the file. 
 *		< 0: Error occurred during the process.
 *
 */
#define nexSAL_FileSeek				g_nexSALFileTable->fnFileSeek
#define nexSAL_FileSeek64			g_nexSALFileTable->fnFileSeek64

/**
 * \ingroup SALAPI
 * \brief NXINT64 nexSAL_FileSize(NEXSALFileHandle a_hFile)
 * 
 * This function retunes the size of the file.
 *
 * \param a_hFile [in] Handle referring to open file.
 *
 * \returns 
 *		>= 0: If the work is successful, the byte size of the content data. 
 *		< 0: Error occurred during reading process
 *
 */
#define nexSAL_FileSize				g_nexSALFileTable->fnFileSize

/**
 * \ingroup SALAPI
 * \brief NXINT64 nexSAL_FileFreeDiskSpaceA(NXCHAR *a_strPath)
 * 
 * This function gets the free disk space.
 *
 * \param a_strPath [in] Root path from which to get free disk size in not UNICODE. If this parameter is NULL, return free space of the default disk.
 *
 * \returns 
 *		>= 0: If the work is successful, the byte size of the free disk space. 
 *		< 0: Error occurred.
 *
 */
#define nexSAL_FileFreeDiskSpaceA	g_nexSALFileTable->fnFileFreeDiskSpaceA
#define nexSAL_FileFreeDiskSpaceW	g_nexSALFileTable->fnFileFreeDiskSpaceW

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_FileRemoveA(const NXCHAR *a_strPath)
 * 
 * This function removes a file in not UNICODE.
 *
 * \param a_strPath [in] Path name of the file to remove from file system.
 *
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_FileRemoveA			g_nexSALFileTable->fnFileRemoveA
#define nexSAL_FileRemoveW			g_nexSALFileTable->fnFileRemoveW

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_FileRename(const NXCHAR *a_strOldName, const NXCHAR *a_strNewName)
 * 
 * This function renames a file or directory.
 *
 * \param a_strOldName [in] Pointer to old name
 *
 * \param a_strNewName  [in] Pointer to new name
 *
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_FileRenameA			g_nexSALFileTable->fnFileRenameA
#define nexSAL_FileRenameW			g_nexSALFileTable->fnFileRenameW

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_FileGetAttribute(const NXCHAR *a_strFilePath, NXUINT32 *a_puAttribute)
 * 
 * This function retrieves attribute of a file or directory.
 *
 * \param a_strFilePath [in] Path name of file to get attribute.
 *
 * \param a_puAttribute [out] Attribute retrieved. (See "NexSALFindInfo" for more information)
 *
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_FileGetAttributeA	g_nexSALFileTable->fnFileGetAttributeA
#define nexSAL_FileGetAttributeW	g_nexSALFileTable->fnFileGetAttributeW

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_DirMake (const NXCHAR *a_strDirName)
 * 
 * This function creates a new directory.
 *
 * \param a_strDirName [in] Path name of directory to create.
 *
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_DirMakeA				g_nexSALFileTable->fnDirMakeA
#define nexSAL_DirMakeW				g_nexSALFileTable->fnDirMakeW

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_DirRemove(const NXCHAR *a_strDirName)
 * 
 * This function remove a new directory.
 *
 * \param a_strDirName [in] Path name of directory to remove.
 *
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_DirRemoveA			g_nexSALFileTable->fnDirRemoveA
#define nexSAL_DirRemoveW			g_nexSALFileTable->fnDirRemoveW

/**
 * \ingroup SALAPI
 * \brief NEXSALSearchHandle nexSAL_FindFirst (NXCHAR *a_strFileSpec, NEXSALFindInfo *a_pstFindInfo)
 * 
 * This function provides information about the first instance of a filename that matches the file specified in the a_strFileSpec argument.
 *
 * \param a_strFileSpec [in] Target file specification(may include wild characters)
 *
 * \param a_pstFindInfo [out] File information structure
 *
 * \returns Handle to the new search object. NEXSAL_INVALID_HANDLE value indicates that "a_strFileSpec" is invalid or could not be matched.
 *
 */
#define nexSAL_FindFirstA			g_nexSALFileTable->fnFindFirstA
#define nexSAL_FindFirstW			g_nexSALFileTable->fnFindFirstW

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_FindNext (NEXSALSearchHandle a_hSearch, NEXSALFindInfo *a_pstFindInfo)
 * 
 * This function provides information about the next instance of a filename that matches the pFileSpec argument in a previous call to nexSAL_FindFirst.
 *
 * \param a_hSearch [in] Handle of search object
 *
 * \param a_pstFindInfo [out] File information structure
 *
 * \returns
 * 		eNEXSAL_RET_NO_ERROR: success
 * 		Others: if no more matching files could be found.
 *
 */
#define nexSAL_FindNext				g_nexSALFileTable->fnFindNext

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_FindClose (NEXSALSearchHandle a_hSearch) 
 * 
 * This function closes the specified search handle.
 *
 * \param a_hSearch [in] Handle of search object
 *
 * \returns
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail 
 *
 */
#define nexSAL_FindClose			g_nexSALFileTable->fnFindClose

#ifdef NEX_UNICODE
	#define nexSAL_FileOpen				nexSAL_FileOpenW
	#define nexSAL_FileFreeDiskSpace	nexSAL_FileFreeDiskSpaceW
	#define nexSAL_FileRemove			nexSAL_FileRemoveW
	#define nexSAL_FileRename			nexSAL_FileRenameW
	#define nexSAL_FileGetAttribute		nexSAL_FileGetAttributeW
	#define nexSAL_DirMake				nexSAL_DirMakeW
	#define nexSAL_DirRemove			nexSAL_DirRemoveW
	#define nexSAL_FindFirst			nexSAL_FindFirstW
#else
	#define nexSAL_FileOpen				nexSAL_FileOpenA
	#define nexSAL_FileFreeDiskSpace	nexSAL_FileFreeDiskSpaceA
	#define nexSAL_FileRemove			nexSAL_FileRemoveA
	#define nexSAL_FileRename			nexSAL_FileRenameA
	#define nexSAL_FileGetAttribute		nexSAL_FileGetAttributeA
	#define nexSAL_DirMake				nexSAL_DirMakeA
	#define nexSAL_DirRemove			nexSAL_DirRemoveA
	#define nexSAL_FindFirst			nexSAL_FindFirstA
#endif


/* Synchronization Objects*/
/* Event */
/**
 * \ingroup SALAPI
 * \brief NEXSALEventHandle nexSAL_EventCreate(NEXSALEventReset a_eManualReset, NEXSALEventInitial a_eInitialSet) 
 * 
 * This function creates new event object.
 *
 * \param a_eManualReset [in] Reset Method
 *						This parameter must be one of the following constants, which are defined in   
 *						nexSal_com.h :
 *						NEXSAL_EVENT_MANUAL: manually reset after set.
 *						NEXSAL_EVENT_AUTO: automatically reset after set.  
 *
 * \param a_eInitialSet [in] Initial state
 *						This parameter must be one of the following constants, which are defined in 
 *						nexSal_com.h :
 *						NEXSAL_EVENT_SET: initially signaled.
 *						NEXSAL_EVENT_UNSET: initially not-signaled.
 * 
 * \returns Handle to the new event object. NEXSAL_INVALID_HANDLE value indicates an error. 
 * 
 * \Remark/Tips/Suggestions
 *		When the state of a manual-reset event object is signaled, it remains signaled until it is explicitly reset to not-signaled by the nexSAL_EventClear function. 
 *		Any number of waiting tasks, or tasks that subsequently begin wait operations for the specified event object, can be released while the event state is signaled. 
 * 		When the state of an auto-reset event object is signaled, it remains signaled until a single waiting task is released;
 * 		the system then automatically resets the state to not-signaled. If no tasks are waiting, the event state remains signaled.
 */
#define nexSAL_EventCreate         g_nexSALSyncObjectTable->fnEventCreate

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_EventDelete(NEXSALEventHandle a_hEvent)
 * 
 * This function deletes event object.
 *
 * \param a_hEvent [in] Handle to the event object
 *
 * \returns
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail 
 *
 */
#define nexSAL_EventDelete         g_nexSALSyncObjectTable->fnEventDelete

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_EventSet(NEXSALEventHandle a_hEvent) 
 * 
 * This function sets event state to signal. If the event is signaled, all the tasks waiting for the event are released and execution continues.
 *
 * \param a_hEvent [in] Handle to the event object
 *
 * \returns
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail 
 *
 */
#define nexSAL_EventSet            g_nexSALSyncObjectTable->fnEventSet

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_EventWait(NEXSALEventHandle a_hEvent, NXUINT32 a_uTimeoutMSec) 
 * 
 * This function waits until event object signaled.
 *
 * \param a_hEvent [in] Handle to the event object 
 *
 * \param a_uTimeoutMSec [in] Timeout interval in milliseconds. If a_uTimeoutMSec is NEXSAL_INFINITE, it never returns unless signaled.
 * 
 * \returns
 *		eNEXSAL_RET_NO_ERROR: if signaled
 *		eNEXSAL_RET_TIMEOUT: if timeout occurred
 *		eNEXSAL_RET_GENERAL_ERROR: if other error 
 *
 * \Remark/Tips/Suggestions 
 * 		In case that a task deletes the event in use of nexSAL_EventDelete while other task is waiting for the event,
 * 		the relevant operations can be different depending on the platform to be ported. Thus, the relevant operations are not defined here. 
 * 		For example, nexSAL_EventWait can be returned because of the waiting task is cancelled. Also, the task cannot be waked up.
 * 		Therefore, the products created based on NexSAL¢â must avoid the above case.
 *
 */
#define nexSAL_EventWait           g_nexSALSyncObjectTable->fnEventWait

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_EventClear(NEXSALEventHandle a_hEvent)
 * 
 * This function sets the state of the specified event object to not-signaled.
 *
 * \param a_hEvent [in] Handle to the event object
 * 
 * \returns
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 * \Remark/Tips/Suggestions 
 *		The nexSAL_EventClear function is used primarily for manual-reset event objects, which must be set explicitly to the not-signaled state.
 *
 */
#define nexSAL_EventClear          g_nexSALSyncObjectTable->fnEventClear


/* Mutex */
/**
 * \ingroup SALAPI
 * \brief NEXSALMutexHandle nexSAL_MutexCreate( ) 
 * 
 * This function creates new mutex object.
 *
 * \param None
 * 
 * \returns Handle to the new mutex object. NEXSAL_INVALID_HANDLE value indicates an error.
 *
 * \Remark/Tips/Suggestions When mutex object is created, its state is unlocked.
 *
 */
#define nexSAL_MutexCreate         g_nexSALSyncObjectTable->fnMutexCreate

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_MutexDelete(NEXSALMutexHandle a_hMutex)  
 * 
 * This function deletes mutex object.
 *
 * \param a_hMutex [in] Handle to mutex object
 * 
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_MutexDelete         g_nexSALSyncObjectTable->fnMutexDelete

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_MutexLock(NEXSALMutexHandle a_hMutex, NXUINT32 a_uTimeoutMSec)
 * 
 * This function locks the mutex object.
 *
 * \param a_hMutex [in] Handle to mutex object
 * 
 * \param a_uTimeoutMSec [in] Interval in milliseconds. If a_uTimeoutMSec is NEXSAL_INFINITE, never returns with timeout.
 *  
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: if signaled
 *		eNEXSAL_RET_TIMEOUT: if timeout occurred
 *		eNEXSAL_RET_GENERAL_ERROR: if other error
 *
 * \Remark/Tips/Suggestions When mutex object is created, its state is unlocked.
 *		If the waiting task which called nexSAL_MutexLock exists then nexSAL_MutexUnlock must be called 
 *		before nexSAL_MutexDelete is called. (see. nexSAL_EventWait)
 *
 */
#define nexSAL_MutexLock           g_nexSALSyncObjectTable->fnMutexLock

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_MutexUnlock (NEXSALMutexHandle a_hMutex)
 * 
 * This function unlocks the mutex object.
 *
 * \param a_hMutex [in] Handle to mutex object
 * 
 * \param a_uTimeoutMSec [in] Interval in milliseconds. If a_uTimeoutMSec is NEXSAL_INFINITE, never returns with timeout.
 *	
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_MutexUnlock         g_nexSALSyncObjectTable->fnMutexUnlock


/* Semaphore */
/**
 * \ingroup SALAPI
 * \brief NEXSALSemaphoreHandle nexSAL_SemaphoreCreate(NXINT32 a_nInitCount, NXINT32 a_nMaxCount)
 * 
 * This function creates a new semaphore object.
 *
 * \param a_nInitCount [in] initial semaphore count
 *				This value must be greater than or equal to zero and less than or equal to a_nMaxCount.  
 *				The state of a semaphore is signaled when its count is greater than zero and nonsignaled when it is zero. 
 * 
 * \param a_nMaxCount [in] Max semaphore count 
 * 				This value must be greater than zero.
 *	
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_SemaphoreCreate     g_nexSALSyncObjectTable->fnSemaphoreCreate

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_SemaphoreDelete(NEXSALSemaphoreHandle a_hSema)
 * 
 * This function deletes a semaphore object.
 *
 * \param a_hSema [in] Handle to semaphore object
 *	
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_SemaphoreDelete     g_nexSALSyncObjectTable->fnSemaphoreDelete

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_SemaphoreRelease (NEXSALSemaphoreHandle a_hSema)
 * 
 * This function increases the count of the specified semaphore object by 1.
 *
 * \param a_hSema [in] semaphore to release
 *	
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_SemaphoreRelease    g_nexSALSyncObjectTable->fnSemaphoreRelease

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_SemaphoreWait(NEXSALSemaphoreHandle a_hSema, NXUINT32 a_uTimeoutMSec)
 * 
 * This function decreases semaphore count or wait.
 *		Count>=1, decrease count by 1
 *		Count=0, wait until other tasks release the semaphore
 *
 * \param a_hSema [in] Handle to semaphore object
 *	
 * \param a_uTimeoutMSec [in] Timeout interval in milliseconds. If a_uTimeoutmsec is NEXSAL_INFINITE, it never returns until signaled.
 *	
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: if signaled
 *		eNEXSAL_RET_TIMEOUT: if timeout occurred
 *		eNEXSAL_RET_GENERAL_ERROR: if other error 
 *
 * \Remark/Tips/Suggestions When mutex object is created, its state is unlocked.
 *		If the waiting task which called nexSAL_SemaphoreWait exists, then nexSAL_SemaphoreRelease must be called 
 *		before nexSAL_SemaphoreDelete is called. (See "nexSAL_EventWait")
 */
#define nexSAL_SemaphoreWait       g_nexSALSyncObjectTable->fnSemaphoreWait


/* atomic inc/dec */
/**
 * \ingroup SALAPI
 * \brief NXINT32 nexSAL_AtomicInc(NXINT32 *a_pnValue)
 * 
 * Read the 32-bit value (referred to as old) stored at location pointed by a_pnValue. 
 * Compute atomically (old + 1) and store result at location pointed by a_pnValue. The function returns old.
 *
 * \param a_pnValue [in] pointer of atomic integer
 *	
 * \returns Old value of *a_pnValue
 *
 */
#define nexSAL_AtomicInc			g_nexSALSyncObjectTable->fnAtomicInc

/* atomic inc/dec */
/**
 * \ingroup SALAPI
 * \brief NXINT32 nexSAL_AtomicDec(NXINT32 *a_pnValue)
 * 
 * Read the 32-bit value (referred to as old) stored at location pointed by a_pnValue.
 * Compute atomically (old - 1) and store result at location pointed by a_pnValue. The function returns old.
 *
 * \param a_pnValue [in] pointer of atomic integer
 *	
 * \returns Old value of *a_pnValue
 *
 */
#define nexSAL_AtomicDec			g_nexSALSyncObjectTable->fnAtomicDec


/* Task */
/**
 * \ingroup SALAPI
 * \brief NEXSALTaskHandle nexSAL_TaskCreate (NXCHAR*	 a_strTaskName, 
 *												NXSYSTaskFunc a_fnTask,
 *												NXVOID* a_pParam, 
 *												NXSYSTaskPriority a_ePriority, 
 *												NXUINT32 a_uStackSize, 
 *												NXSYSTaskOption a_eCreateOption) 
 * 
 * This function creates new task and executes it. 
 *
 * \param a_strTaskName [in] Pointer to a null-terminated string that specifies the name of task. This value can be helpful to find bugs.
 *	
 * \param a_fnTask [in] Pointer to the function to be executed
 *	
 * \param a_pParam [in] Pointer to a variable to be passed to the task
 *	
 * \param a_ePriority [in] Priority Value
 *	
 * \param a_uStackSize [in] Size of Stack 
 *	
 * \param a_eCreateOption [in] Flags that controls the creation of the task. This flag includes platform specific options.
 *	 
 * \returns Handle to the new task. NEXSAL_INVALID_HANDLE value indicates an error.
 *
 */
#define nexSAL_TaskCreate          g_nexSALTaskTable->fnTaskCreate

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_TaskDelete(NXSYSTaskHandle a_hTask) 
 * 
 * This function deletes a task object. If the task is running, it will be terminated and closed.
 *
 * \param a_hTask [in] Handle to Task
 *	 
 * \returns 
 * 		eNEXSAL_RET_NO_ERROR: success
 * 		Others: fail
 *
 * \Remark/Tips/Suggestions When mutex object is created, its state is unlocked.
 *		The tasks which SAL defines are not deleted by TaskTerminate, and the task operations are just stopped. 
 *		This function must be called to recall resources related to the task.
 */
#define nexSAL_TaskDelete          g_nexSALTaskTable->fnTaskDelete

/**
 * \ingroup SALAPI
 * \brief NEXSALTaskHandle nexSAL_TaskCurrent( ) 
 * 
 * This function returns handle to current executing task
 *  
 * \param None
 *	 
 * \returns Handle to current task
 *
 */
#define nexSAL_TaskCurrent         g_nexSALTaskTable->fnTaskCurrent

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_TaskGetPriority(NXSYSTaskHandle a_hTask, NXSYSTaskPriority *a_pePriority)
 * 
 * This function gets the priority of target task.
 *	
 * \param a_hTask [in] Handle to Task
 * 
 * \param a_pePriority [out] Pointer to get priority
 *	 
 * \returns
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail 
 *
 */
#define nexSAL_TaskGetPriority     g_nexSALTaskTable->fnTaskGetPriority

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_TaskSetPriority(NXSYSTaskHandle a_hTask, NXSYSTaskPriority a_ePriority) 
 * 
 * This function sets priority to target task.
 *	
 * \param a_hTask [in] Handle to Task
 * 
 * \param a_pePriority [out] Priority to set
 *	 
 * \returns
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail 
 *
 */
#define nexSAL_TaskSetPriority     g_nexSALTaskTable->fnTaskSetPriority

/**
 * \ingroup SALAPI
 * \brief NXVOID nexSAL_TaskSleep (NXUINT32 a_uMSec)
 * 
 * This function suspends the calling task for the specified interval.
 *	
 * \param a_uMSec [in] Time interval in milliseconds
 *	 
 * \returns None
 *
 */
#define nexSAL_TaskSleep           g_nexSALTaskTable->fnTaskSleep

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_TaskTerminate(NXSYSTaskHandle a_hTask) 
 * 
 * This function terminates a task.
 * Do not use this in normal situations. It's dangerous because the target task has no chance to clear resources. 
 *
 * \param a_hTask [in] Handle to Task
 *	 
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_TaskTerminate       g_nexSALTaskTable->fnTaskTerminate

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_TaskWait (NXSYSTaskHandle a_hTask) 
 * 
 * This function blocks the calling task until a target task terminates.
 *
 * \param a_hTask [in] Target task
 *	 
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_TaskWait            g_nexSALTaskTable->fnTaskWait


/* Socket */
/**
 * \ingroup SALAPI
 * \brief NEXSALSockHandle nexSAL_NetSocket(NEXSALHandle a_hSAL, NEXSALSockType a_eType) 
 * 
 * This function creates a socket that is bound to a specific service provider.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_eType [in] selects socket connection type
 *		NEXSAL_SOCK_STREAM: Used for TCP address system that is reliable
 *		NEXSAL_SOCK_DGRAM: Used for UDP address system that is not necessarily reliable, but fast.
 *		NEXSAL_SOCK_SSL: SSL
 *	 
 * \returns 
 *		NEXSAL_INVALID_HANDLE: fail
 *		Others: socket handle
 *
 * \Remark/Tips/Suggestions It required only when using Streaming Player.
 *
 */
#define nexSAL_NetSocket           g_nexSALSocketTable->fnNetSocket

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetClose(NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock) 
 * 
 * This function closes an existing socket.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Socket to close
 *	 
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetClose            g_nexSALSocketTable->fnNetClose

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetConnect(NEXSALHandle a_hSAL, 
 * 											NEXSALSockHandle a_hSock, 
 * 											const NXCHAR *a_strAddr, 
 * 											NXUINT16 a_uwPort, 
 * 											NXUINT32 a_uTimeout) 
 * 
 * This function establishes a connection to a specified socket.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket
 *	 
 * \param a_strAddr [in] A null-terminated character string to connect. this string is expressed in the internet standard ".'' (dotted) notation.
 *  
 * \param a_uwPort [in] Port value of destination to connect to.
 *
 * \param a_uTimeout [in] Timeout period
 *
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetConnect          g_nexSALSocketTable->fnNetConnect

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetBind(NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXUINT16 a_uwPort)
 * 
 * This function associates a local address with a socket.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket
 *	 
 * \param a_uwPort [in] Port value to bind to
 *
 * \returns 
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetBind             g_nexSALSocketTable->fnNetBind

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetSelect(NEXSALHandle a_hSAL,
 * 										NEXSALFDSet *a_pReadFDS,
 * 										NEXSALFDSet *a_pWriteFDS,
 * 										NEXSALFDSet *a_ExceptFDS,
 * 										NEXSALTimeValue *a_pTimeValue)
 * 
 * This function determines the status of one or more sockets, waiting if necessary, to perform synchronous I/O.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_pReadFDS [in/out] An optional pointer to a set of sockets to be checked for readability
 *	 
 * \param a_pWriteFDS [in/out] An optional pointer to a set of sockets to be checked for writability
 *	 
 * \param a_ExceptFDS [in/out] An optional pointer to a set of sockets to be checked for errors
 *	 
 * \param a_pTimeValue [in] The maximum time for select to wait, provided in the form of a NEXSALTimeValue structure. 
 * 						Set the timeout parameter to null for blocking operations
 *
 *			typedef struct NEXSALTimeValue
 *			{
 *				NXUINT32	 tv_sec;	
 *				NXUINT32	 tv_usec;
 *			} NEXSALTimeValue;
 * \returns 
 *		eNEXSAL_RET_WOULDBLOCK: would block
 *		eNEXSAL_RET_TIMEOUT: Timeout
 *		eNEXSAL_RET_SOCK_ERROR: Fail
 *		>0: Total number of socket handles that are included in NEXSALFDSet structure and are ready at designated I/O.
 *
 */
#define nexSAL_NetSelect           g_nexSALSocketTable->fnNetSelect

/**
 * \ingroup SALAPI
 * \brief NXSSIZE nexSAL_NetSendTo(NEXSALHandle a_hSAL, 
 * 									NEXSALSockHandle a_hSock, 
 * 									NXCHAR *a_pBuf, 
 * 									NXUSIZE a_uLen, 
 * 									NXCHAR *a_strAddr, 
 * 									NXUINT16 a_uwPort) 
 * 
 * This function transfers data to assigned address.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 *	 
 * \param a_pBuf [in] Buffer that holds data to be transferred
 *	 
 * \param a_uLen [in] Length of the data in pBuf, in bytes
 *	 
 * \param a_strAddr [in] Remote host address that data will be transferred to. Expressed in character row
 *
 * \param a_uwPort [in] port of the remote host that data will be transferred to
 *
 * \returns 
 *		>= 0: size of total transferred data
 *		Others: fail
 *
 * \Remark/Tips/Suggestions Applies to socket configured as NEXSAL_SOCK_DGRAM.
 *
 */
#define nexSAL_NetSendTo           g_nexSALSocketTable->fnNetSendTo

/**
 * \ingroup SALAPI
 * \brief NXSSIZE nexSAL_NetSend(NEXSALHandle a_hSAL, 
 * 								NEXSALSockHandle a_hSock, 
 * 								NXCHAR *a_pBuf, 
 * 								NXUSIZE a_uLen) 
 * 
 * This function sends data to connected socket.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 *	 
 * \param a_pBuf [in] Buffer that holds data to be transferred
 *	 
 * \param a_uLen [in] Length of the data in pBuf, in bytes
 *
 * \returns 
 *		>= 0: size of total transferred data
 *		Others: fail
 *
 * \Remark/Tips/Suggestions Applies to socket configured as NEXSAL_SOCK_STREAM.
 *
 */
#define nexSAL_NetSend             g_nexSALSocketTable->fnNetSend

/**
 * \ingroup SALAPI
 * \brief NXSSIZE nexSAL_NetRecvFrom(NEXSALHandle a_hSAL, 
 * 									NEXSALSockHandle a_hSock, 
 * 									NXCHAR *a_pBuf, 
 * 									NXUSIZE a_uLen, 
 * 									NXUINT32 *a_puAddr, 
 * 									NXUINT16 *a_uwPort, 
 * 									NXUINT32 a_uTimeout) 
 * 
 * This function receives a datagram from socket and returns the corresponding address and port number.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 *	 
 * \param a_pBuf [in] Buffer that holds data to be transferred
 *	 
 * \param a_uLen [in] Length of the data in pBuf, in bytes
 *
 * \param a_puAddr [out] Address of the terminal that sent the received data
 *
 * \param a_uwPort [out] Port of the terminal that sent the receives the data
 *
 * \param a_uTimeout [in] Timeout period
 * 
 * \returns 
 *		<=0: failure or no data to receive
 *		>0: size of the transferred data
 *
 */
#define nexSAL_NetRecvFrom         g_nexSALSocketTable->fnNetRecvFrom

/**
 * \ingroup SALAPI
 * \brief NXSSIZE nexSAL_NetRecv (NEXSALHandle a_hSAL,
 * 								NEXSALSockHandle a_hSock,
 * 								NXCHAR *a_pBuf,
 * 								NXUSIZE a_uLen,
 * 								NXUINT32 a_uTimeout)
 * 
 * This function receives a data from the connected socket. 
 * It will just return if there is no data to receive during pre-assigned timeout.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 *	 
 * \param a_pBuf [in] Buffer that holds data to be transferred
 *	 
 * \param a_uLen [in] Length of the data in pBuf, in bytes 
 *
 * \param a_uTimeout [in] Timeout period
 * 
 * \returns 
 *		<=0: failure or no data to receive
 *		>0: size of the transferred data
 *
 */
#define nexSAL_NetRecv             g_nexSALSocketTable->fnNetRecv

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetMultiGroup(NEXSALHandle a_hSAL,
 * 											NEXSALSockHandle a_hSock,
 * 											NXCHAR *a_strMultiGroupAddr,
 * 											NEXSALGroupManage a_eProp) 
 * 
 * This function set multicast socket option behavior.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 *	 
 * \param a_strMultiGroupAddr [in] MultiGroup Address
 *	 
 * \param a_eProp [in]
 * 		typedef enum NEXSALGroupManage
 * 		{
 * 			NEXSAL_MULTI_ADD		= 0, //add an IP group membership
 * 			NEXSAL_MULTI_DROP	= 1, //drop an IP group membership
 * 		} NEXSALGroupManage;
 * 
 * \returns 
 *		eNEXSAL_RET_SOCK_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetMultiGroup	   g_nexSALSocketTable->fnNetMultiGroup

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetSSLTunneling(NEXSALHandle a_hSAL,
 * 												NEXSALSockHandle a_hSock) 
 * 
 * This function initiates the TLS/SSL handshake with a server.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 * 
 * \returns 
 *		eNEXSAL_RET_SOCK_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetSSLTunneling	   g_nexSALSocketTable->fnNetSSLTunneling

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetListen(NEXSALHandle a_hSAL, 
 * 										NEXSALSockHandle a_hSock,
 * 										NXINT32 a_nBackLog) 
 * 
 * This function places a socket in a state in which it is listening for an incoming connection.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 * 
 * \param a_nBackLog [in] The maximum length of the queue of pending connections. 
 * 						If set to SOMAXCONN, the underlying service provider responsible for socket s will set the backlog to a maximum reasonable value. 
 * 						There is no standard provision to obtain the actual backlog value.
 * \returns 
 *		eNEXSAL_RET_SOCK_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetListen		   g_nexSALSocketTable->fnNetListen

/**
 * \ingroup SALAPI
 * \brief NEXSALSockHandle nexSAL_NetAccept(NEXSALHandle a_hSAL,
 * 											NEXSALSockHandle a_hSock,
 * 											NXUINT32 *a_puAddr,
 * 											NXUINT16 *a_puwPort,
 * 											NXUINT32 a_uTimeOutInMSec,
 * 											NXINT32 *a_pnResult) 
 * 
 * This function permits an incoming connection attempt on a socket.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 * 
 * \param a_puAddr [out] IP address in network byte order
 *  
 * \param a_puwPort [out] Port number
 *  
 * \param a_uTimeOutInMSec [in] Timeout period
 *  
 * \param a_pnResult [out] result value
 *   0: success
 *   -1: fail
 *   -2: timeout
 *  
 * \returns 
 *		NEXSAL_INVALID_HANDLE: fail
 *		<0: file descriptor of the accepted socket
 *
 */
#define nexSAL_NetAccept		   g_nexSALSocketTable->fnNetAccept

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetAsyncConnect(NEXSALHandle a_hSAL,
 *												NEXSALSockHandle a_hSock,
 *												NXCHAR *a_strAddr,
 *												NXUINT16 a_uwPort,
 *												NXUINT32 a_uTimeout) 
 * 
 * This function establishes an asynchronous connection to a specified socket.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 * 
 * \param a_strAddr [in] A null-terminated character string to connect. This string is expressed in the internet standard ".'' (dotted) notation.
 *	
 * \param a_uwPort [in] Port value of destination to connect to.
 *	
 * \param a_uTimeout [in] Timeout period
 *	
 * \returns 
 *		eNEXSAL_RET_SOCK_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetAsyncConnect     g_nexSALSocketTable->fnNetAsyncConnect

/**
 * \ingroup SALAPI
 * \brief NEXSAL_RETURN nexSAL_NetAsyncSSLTunneling(NEXSALHandle a_hSAL,
 * 													NEXSALSockHandle a_hSock) 
 * 
 * This function initiates the TLS/SSL handshake with a server asynchronously.
 *
 * \param a_hSAL [in] SAL Handle
 *
 * \param a_hSock [in] Handle to socket.
 *	
 * \returns 
 *		eNEXSAL_RET_SOCK_ERROR: success
 *		Others: fail
 *
 */
#define nexSAL_NetAsyncSSLTunneling	g_nexSALSocketTable->fnNetSSLTunneling


/* Debugging */
/**
 * \ingroup SALAPI
 * \brief NXVOID nexSAL_DebugPrintf(const NXCHAR *a_strFormat, [argument]...) 
 * 
 * This function print formatted output. Where to display is diverse according to target system.
 *
 * \param a_strFormat [in] Format control. Refer to ANSI C run-time library manuals.
 *
 * \param argument [in] Optional arguments
 *	
 * \returns None
 *
 */
#define nexSAL_DebugPrintf         g_nexSALTraceTable->fnDegPrintf

/* Debugging */
/**
 * \ingroup SALAPI
 * \brief NXVOID nexSAL_DebugOutputString(NXCHAR *a_strOutput)
 * 
 * This function print formatted output. Where to display is diverse according to target system.
 *
 * \param a_strOutput [in] string format to print
 *
 * \returns None
 *
 */
#define nexSAL_DebugOutputString   g_nexSALTraceTable->fnOutputString


/* ETC */
/* Time */
/**
 * \ingroup SALAPI
 * \brief NXUINT32 nexSAL_GetTickCount( )
 * 
 * This function retrieves the current system time in milliseconds.
 *
 * \param None
 *	 
 * \returns None
 *
 */
#define nexSAL_GetTickCount        g_nexSALEtcTable->fnGetTickCount

/* ETC */
/* Time */
/**
 * \ingroup SALAPI
 * \brief NXVOID nexSAL_GetEpochMS(NXUINT64 *a_puqEpochMS);
 * 
 * This function can get the time. The argument gives the number of milliseconds since the Epoch.
 *
 * \param a_puqEpchMS [out] the pointer of current milliseconds since the Epoch
 *	 
 * \returns None
 *
 */
#define nexSAL_GetMSecFromEpoch			g_nexSALEtcTable->fnGetMSecFromEpoch


#ifndef NOLOG
	#define NEXSAL_TRACE		nexSAL_DebugPrintf 
	#define NEXSAL_TRACECAT		nexSAL_TraceCat
#else
	#define NEXSAL_TRACE		1 ? (NXVOID)0 : nexSAL_DebugPrintf
	#define NEXSAL_TRACECAT		1 ? (NXVOID)0 : nexSAL_TraceCat
#endif

#ifdef _NEXSAL_TRACECATEGORY_OPERATED_
	#define NEXSAL_TRACECATEGORY(c, l, f)	if(nexSAL_TraceCondition(c, l)) {nexSAL_DebugPrintf f;};
#else
	#define NEXSAL_TRACECATEGORY(c, l, f)	((NXVOID)0)
#endif

#endif // _NEXSAL_INTERNAL_HEADER_INCLUDED_

/*-----------------------------------------------------------------------------
Revision History:
Author		Date			Version		Description of Changes
-------------------------------------------------------------------------------
pss			2005/08/11		1.0			Draft
ysh			2005/08/15		1.1			Revision
ysh			2006/03/12		1.2			NEXSAL_TRACE_MODULE added
hsc			2006/03/30		1.3			File related 7 functions is added
ysh			2006/04/11		1.4/0		Revision(classifying functions)
pss         	2006/04/27      	1.5/0       	Add Memory Debug Define _DEBUG_MEM
pss         	2006/04/27      	1.5/1       	Add Primitive type and define       
pss        	 	2006/05/03      	1.5/2       	Remove nexSAL_TaskExit
pss         	2006/05/04      	1.5/3       	ASSERT changed.
pss         	2006/06/08      	2.0         	FileSizeEx, FileFreeDiskSpaceEx added
ysh			2006/08/08		2.1			Apis related to Socket is revised.
ysh			2006/09/12		2.2			UNICODE is introduced. nexSAL_FileOpen(A/W) is adapted.
pss			2006/09/27		2.2/1	    	nexSAL_FileRemove(A/W) is adapted.(temporarily)
ysh			2006/10/09		2.4.0		Obsolete functions was removed.
ysh			2008/06/13		2.5.0		Unicode functions(on File) was added.
ysh			2008/07/25		2.6.0		NEXSAL_TRACEMODULE revision.
ysh			2009/09/29		2.6.1		nexSAL_MemAlloc2, nexSAL_MemFree2 was added.
ysh			2009/12/21		2.6.2		nexSAL_FileSeek64 was added.
robin		2010/06/01		2.6.3		Malloc, Calloc, Malloc2 Parameter Added for Memory Leak
ysh			2011/05/26		2.6.4		atomic inc/dec were added.
-----------------------------------------------------------------------------*/
