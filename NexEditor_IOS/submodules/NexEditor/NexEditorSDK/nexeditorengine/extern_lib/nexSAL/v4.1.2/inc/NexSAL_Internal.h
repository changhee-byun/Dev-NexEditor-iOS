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


/**
 * @ingroup sal_memory
 *
 * NXVOID* nexSAL_MemAlloc(NXUSIZE a_uSize)\n
 * This function allocates a block of memory.
 *
 * @param[in] a_uSize Size in bytes of memory block to be allocated
 * @return Pointer to the allocated memory block. If the available memory is insufficient, NULL is returned.
 */
#define nexSAL_MemAlloc(a)			g_nexSALMemoryTable->fnMemAlloc(a, __FILE__, __LINE__)

/**
 * @ingroup sal_memory
 *
 * NXVOID* nexSAL_MemCalloc(NXUSIZE a_uNum, NXUSIZE a_uSize)\n
 * This function allocates an array in memory with elements initialized to 0.
 * @param[in] a_uNum Number of elements
 * @param[in] a_uSize Size in bytes of each element
 * @return Pointer to the allocated memory. If the available memory is insufficient, NULL is returned.
 */
#define nexSAL_MemCalloc(a, b)		g_nexSALMemoryTable->fnMemCalloc(a, b, __FILE__, __LINE__)

/**
 * @ingroup sal_memory
 *
 * NXVOID* nexSAL_MemFree(NXVOID *a_pMem)\n
 * This function frees a memory block.
 *
 * @param[in] a_pMem Pointer to the memory block to be freed
 * @return None
 */
#define nexSAL_MemFree(a)			g_nexSALMemoryTable->fnMemFree(a, __FILE__, __LINE__)

/**
 * @ingroup sal_memory
 *
 * NXVOID* nexSAL_MemAlloc2(NXUSIZE a_uSize, NXUINT32 a_uType)\n
 * This function allocates an aligned block of memory.
 *
 * @param[in] a_uSize Size in bytes of memory block to be allocated
 * @param[in] a_uType align bytes type\n
 *		- NEXSAL_MEM_ALIGN_NORMAL: no align\n
 *		- NEXSAL_MEM_ALIGN_2: 2bytes align\n
 *		- NEXSAL_MEM_ALIGN_4: 4bytes align\n
 *		- NEXSAL_MEM_ALIGN_8: 8bytes align\n
 *		- NEXSAL_MEM_ALIGN_16: 16bytes align\n
 *		- NEXSAL_MEM_ALIGN_32: 32bytes align\n
 *		- NEXSAL_MEM_ALIGN_64: 64bytes align\n
 *		- NEXSAL_MEM_ALIGN_128: 128bytes align
 * @return Pointer to the allocated memory. If the available memory is insufficient, NULL is returned.
 */
#define nexSAL_MemAlloc2(a,b)		g_nexSALMemoryTable->fnMemAlloc2(a, b, __FILE__, __LINE__)

/**
 * @ingroup sal_memory
 *
 * NXVOID* nexSAL_MemFree2(NXVOID *a_pMem)\n
 * This function frees a memory block for nexSAL_MemAlloc2.
 *
 * @param[in] a_pMem Pointer to the memory block to be freed
 * @return None
 */
#define nexSAL_MemFree2				g_nexSALMemoryTable->fnMemFree2


/* File */
/**
 * @ingroup sal_file
 *
 * NEXSALFileHandle nexSAL_FileOpenA(const NXCHAR *a_strFilePath, NEXSALFileMode a_eMode)\n
 * This function opens a file in not UNICODE.
 *
 * @param[in] a_strFilePath Path name of file to open
 * @param[in] a_eMode Open flag. Some compositions are not supported by some platforms.
 * @param[in] a_strFilePath Path name of file to open\n
 *		Available compositions:\n
 *	 	- NEXSAL_FILE_READ: Read existing file.\n
 *	 	- NEXSAL_FILE_WRITE: Append to existing file. (If it does not exist, operation not defined.)\n
 *		- NEXSAL_FILE_READ | NEXSAL_FILE_WRITE (=NEXSAL_FILE_READWRITE): Read & write file (If not exist, operation not defined.)\n
 *	 	- NEXSAL_FILE_WRITE | NEXSAL_FILE_CREATE: Create new file to write. If file exists, overwrite.\n
 *		- NEXSAL_FILE_READ | NEXSAL_FILE_WRITE | NEXSAL_FILE_CREATE: Create new file to read, write. If file exists, overwrite.
 * @return Handle of the file object. NEXSAL_INVALID_HANDLE value indicates an error.
 * @see NEXSALFileHandle
 * @see NEXSALFileMode
 */
#define nexSAL_FileOpenA			g_nexSALFileTable->fnFileOpenA

/**
* @ingroup sal_file
*
* NEXSALFileHandle nexSAL_FileOpenW(const NXWCHAR *a_strFilePath, NEXSALFileMode a_eMode)\n
* This function opens a file in UNICODE. For Parameters meanings and return value, see '3.3.4 nexSAL_FileOpenA'
*
* @param[in] a_strFilePath Path name of file to open
* @param[in] a_eMode Open flag. Some compositions are not supported by some platforms.
* @param[in] a_strFilePath Path name of file to open\n
*		Available compositions\n
*	 	- NEXSAL_FILE_READ: Read existing file.\n
*	 	- NEXSAL_FILE_WRITE: Append to existing file. (If it does not exist, operation not defined.)\n
*		- NEXSAL_FILE_READ | NEXSAL_FILE_WRITE (=NEXSAL_FILE_READWRITE): Read & write file (If not exist, operation not defined.)\n
*	 	- NEXSAL_FILE_WRITE | NEXSAL_FILE_CREATE: Create new file to write. If file exists, overwrite.\n
*		- NEXSAL_FILE_READ | NEXSAL_FILE_WRITE | NEXSAL_FILE_CREATE: Create new file to read, write. If file exists, overwrite.
* @return Handle of the file object. NEXSAL_INVALID_HANDLE value indicates an error.
* @see NEXSALFileHandle
* @see NEXSALFileMode
*/
#define nexSAL_FileOpenW			g_nexSALFileTable->fnFileOpenW

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FileClose(NEXSALFileHandle a_hFile)\n
 * This function closes an open file handle.
 *
 * @param[in] a_hFile Handle to an open file 
 * @return	eNEXSAL_RET_NO_ERROR: success
 *			Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALFileHandle 
 */
#define nexSAL_FileClose			g_nexSALFileTable->fnFileClose

/**
 * @ingroup sal_file
 *
 * NXSSIZE nexSAL_FileRead(NEXSALFileHandle a_hFile, NXVOID *a_pBuf, NXUSIZE a_uReadLen)\n
 * This function reads bytes from a file.
 *
 * @param[in] a_hFile Handle referring to open file
 * @param[out] a_pBuf Pointer to the buffer that receives the data read from the file 
 * @param[in] a_uReadLen Number of bytes to be read from the file
 * @return	> 0: If the work is successful, returns byte count of actual data\n
 *			0: It reached to the end of the tile - no more data to read.\n
 *			< 0: An error occurred during reading process.
 * @see NEXSALFileHandle
 */
#define nexSAL_FileRead				g_nexSALFileTable->fnFileRead

/**
 * @ingroup sal_file
 *
 * NXSSIZE nexSAL_FileWrite(NEXSALFileHandle a_hFile, NXVOID *a_pBuf, NXUSIZE a_uWriteLen)\n
 * This function writes bytes to the file.
 *
 * @param[in] a_hFile Handle of file into which data are written
 * @param[in] a_pBuf Pointer to the buffer that contains the data to write to file 
 * @param[in] a_uWriteLen Number of bytes to write to the file
 * @return	>= 0: the number of bytes written, if the work is successful.\n
 *			< 0: Error occurred during writing process.
 * @remark The nexSAL_FileWrite operation begins at the current position of the file pointer (if any) associated with the given file.\n
 *			After this operation, the file pointer is increased by the number of bytes actually written.
 * @see NEXSALFileHandle
 */
#define nexSAL_FileWrite			g_nexSALFileTable->fnFileWrite

/**
 * @ingroup sal_file
 *
 * NXINT32 nexSAL_FileSeek (NEXSALFileHandle a_hFile, NXINT32 a_nOffset, NEXSALFileSeekOrigin a_eOrigin)\n
 * This function moves a file pointer to the specified location.
 *
 * @param[in] a_hFile Handle referring to open file
 * @param[in] a_nOffset Number of bytes from origin by 32 bits range
 * @param[in] a_eOrigin Starting point from which the file pointer moves\n
 * 			The iOrigin must be one of the following constants, which are defined in nexSal_com.h.\n
 *				- NEXSAL_SEEK_BEGIN\n
 * 				- NEXSAL_SEEK_END\n
 * 				- NEXSAL_SEEK_CUR\n
 * @return	>= 0: If the work was successful the byte distance from the beginning of the file to the resultant position of the file. \n
 *			< 0: Error occurred during the process.
 * @see NEXSALFileHandle
 * @see NEXSALFileSeekOrigin
 */
#define nexSAL_FileSeek				g_nexSALFileTable->fnFileSeek

/**
 * @ingroup sal_file
 *
 * NXINT64 nexSAL_FileSeek64 (NEXSALFileHandle a_hFile, NXINT64 a_nOffset, NEXSALFileSeekOrigin a_eOrigin)\n
 * This function moves a file pointer to the specified location.
 *
 * @param[in] a_hFile Handle referring to open file
 * @param[in] a_nOffset Number of bytes from origin by 64 bits range
 * @param[in] a_eOrigin Starting point from which the file pointer moves\n
 * 			The iOrigin must be one of the following constants, which are defined in nexSal_com.h.\n
 *				- NEXSAL_SEEK_BEGIN\n
 * 				- NEXSAL_SEEK_END\n
 * 				- NEXSAL_SEEK_CUR\n
 * @return	>= 0: If the work was successful the byte distance from the beginning of the file to the resultant position of the file. \n
 *			< 0: Error occurred during the process.
 * @see NEXSALFileHandle
 * @see NEXSALFileSeekOrigin 
 */
#define nexSAL_FileSeek64			g_nexSALFileTable->fnFileSeek64

/**
 * @ingroup sal_file
 *
 * NXINT64 nexSAL_FileSize(NEXSALFileHandle a_hFile)\n
 * This function retunes the size of the file.
 *
 * @param[in] a_hFile Handle referring to open file.
 * @return	>= 0: If the work is successful, the byte size of the content data.\n 
 *			< 0: Error occurred during reading process
 * @see NEXSALFileHandle 
 */
#define nexSAL_FileSize				g_nexSALFileTable->fnFileSize

/**
 * @ingroup sal_file
 *
 * NXINT64 nexSAL_FileFreeDiskSpaceA(NXCHAR *a_strPath)\n
 * This function gets the free disk space.
 *
 * @param[in] a_strPath Root path from which to get free disk size in not UNICODE. If this parameter is NULL, return free space of the default disk.
 * @return	>= 0: If the work is successful, the byte size of the free disk space.\n
 *			< 0: Error occurred.
 */
#define nexSAL_FileFreeDiskSpaceA	g_nexSALFileTable->fnFileFreeDiskSpaceA

/**
 * @ingroup sal_file
 *
 * NXINT64 nexSAL_FileFreeDiskSpaceW(NXWCHAR *a_wstrPath)\n
 * This function gets the free disk space.
 *
 * @param[in] a_wstrPath Root path from which to get free disk size in UNICODE. If this parameter is NULL, return free space of the default disk.
 * @return	>= 0: If the work is successful, the byte size of the free disk space.\n
 *			< 0: Error occurred.
 */
#define nexSAL_FileFreeDiskSpaceW	g_nexSALFileTable->fnFileFreeDiskSpaceW

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FileRemoveA(const NXCHAR *a_strPath)\n
 * This function removes a file in not UNICODE.
 *
 * @param[in] a_strPath Path name of the file to remove from file system
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_FileRemoveA			g_nexSALFileTable->fnFileRemoveA

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FileRemoveW(const NXWCHAR *a_wstrFilePath)\n
 * This function removes a file in UNICODE.
 *
 * @param[in] a_wstrFilePath Path name of the file to remove from file system
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_FileRemoveW			g_nexSALFileTable->fnFileRemoveW

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FileRenameA(const NXCHAR *a_strOldName, const NXCHAR *a_strNewName)\n
 * This function renames a file or directory.
 *
 * @param[in] a_strOldName Pointer to old name
 * @param[in] a_strNewName Pointer to new name
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_FileRenameA			g_nexSALFileTable->fnFileRenameA

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FileRenameW(const NXWCHAR *a_wstrOldName, const NXWCHAR *a_wstrNewName)\n
 * This function renames a file or directory in UNICODE.
 *
 * @param[in] a_wstrOldName Pointer to old name
 * @param[in] a_wstrNewName Pointer to new name
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_FileRenameW			g_nexSALFileTable->fnFileRenameW

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FileGetAttributeA(const NXCHAR *a_strFilePath, NXUINT32 *a_puAttribute)\n
 * This function retrieves attribute of a file or directory.
 *
 * @param[in] a_strFilePath Path name of file to get attribute
 * @param[out] a_puAttribute Attribute retrieved(See "NexSALFindInfo" for more information)
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_FileGetAttributeA	g_nexSALFileTable->fnFileGetAttributeA

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FileGetAttributeW(const NXWCHAR *a_wstrFilePath, NXUINT32 *a_puAttribute)\n
 * This function retrieves attribute of a file or directory in UNICODE.
 *
 * @param[in] a_wstrFilePath Path name of file to get attribute
 * @param[out] a_puAttribute Attribute retrieved. (See "NexSALFindInfo" for more information)
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_FileGetAttributeW	g_nexSALFileTable->fnFileGetAttributeW

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_DirMakeA(const NXCHAR *a_strDirName)\n
 * This function creates a new directory.
 *
 * @param[in] a_strDirName Path name of directory to create
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_DirMakeA				g_nexSALFileTable->fnDirMakeA

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_DirMakeW(const NXWCHAR *a_wstrDirName)\n
 * This function creates a new directory in UNICODE.
 *
 * @param[in] a_wstrDirName Path name of directory to create
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_DirMakeW				g_nexSALFileTable->fnDirMakeW

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_DirRemoveA(const NXCHAR *a_strDirName)\n
 * This function remove a new directory.
 *
 * @param[in] a_strDirName Path name of directory to remove.
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_DirRemoveA			g_nexSALFileTable->fnDirRemoveA

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_DirRemoveW(const NXWCHAR *a_wstrDirName)\n
 * This function remove a new directory in UNICODE.
 *
 * @param[in] a_wstrDirName Path name of directory to remove
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail
 * @see NEXSAL_RETURN 
 */
#define nexSAL_DirRemoveW			g_nexSALFileTable->fnDirRemoveW

/**
 * @ingroup sal_file
 *
 * NEXSALSearchHandle nexSAL_FindFirstA(NXCHAR *a_strFileSpec, NEXSALFindInfo *a_pstFindInfo)\n
 * This function provides information about the first instance of a filename that matches the file specified in the a_strFileSpec argument.
 *
 * @param[in] a_strFileSpec Target file specification(may include wild characters)
 * @param[out] a_pstFindInfo File information structure
 * @return Handle to the new search object. NEXSAL_INVALID_HANDLE value indicates that "a_strFileSpec" is invalid or could not be matched.
 * @see NEXSALSearchHandle 
 * @see NEXSALFindInfo
 */
#define nexSAL_FindFirstA			g_nexSALFileTable->fnFindFirstA

/**
 * @ingroup sal_file
 *
 * NEXSALSearchHandle nexSAL_FindFirstW(NXWCHAR *a_strFileSpec, NEXSALFindInfo *a_pstFindInfo)\n
 * This function provides information about the first instance of a filename that matches the file specified in the a_strFileSpec argument.
 *
 * @param[in] a_strFileSpec Target file specification(may include wild characters)
 * @param[out] a_pstFindInfo File information structure
 * @return	Handle to the new search object.\n
 *			NEXSAL_INVALID_HANDLE value indicates that "a_strFileSpec" is invalid or could not be matched.
 * @see NEXSALSearchHandle 
 * @see NEXSALFindInfo 
 */
#define nexSAL_FindFirstW			g_nexSALFileTable->fnFindFirstW

/**
 * @ingroup sal_file
 *
 * NEXSAL_RETURN nexSAL_FindNext (NEXSALSearchHandle a_hSearch, NEXSALFindInfo *a_pstFindInfo)\n
 * This function provides information about the next instance of a filename that matches the pFileSpec argument in a previous call to nexSAL_FindFirst.
 *
 * @param[in] a_hSearch Handle of search object
 * @param[out] a_pstFindInfo File information structure
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 * 			Others: no more matching files could be found.
 * @see NEXSALSearchHandle 
 * @see NEXSALFindInfo 
 */
#define nexSAL_FindNext				g_nexSALFileTable->fnFindNext

/**
 * @ingroup sal_file
 *
 * @brief NEXSAL_RETURN nexSAL_FindClose (NEXSALSearchHandle a_hSearch)\n
 * 			This function closes the specified search handle.
 *
 * @param[in] a_hSearch Handle of search object
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail 
 * @see NEXSALSearchHandle  
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
 * @ingroup sal_event
 *
 * NEXSALEventHandle nexSAL_EventCreate(NEXSALEventReset a_eManualReset, NEXSALEventInitial a_eInitialSet)\n
 * This function creates new event object.
 *
 * @param[in] a_eManualReset Reset Method This parameter must be one of the following constants, which are defined in\n
 * @param[in] a_eInitialSet Initial state This parameter must be one of the following constants, which are defined in\n
 * @return	Handle to the new event object.\n
 *			NEXSAL_INVALID_HANDLE value indicates an error. 
 * @note When the state of a manual-reset event object is signaled, it remains signaled until it is explicitly reset to not-signaled by the nexSAL_EventClear function.\n
 *		Any number of waiting tasks, or tasks that subsequently begin wait operations for the specified event object, can be released while the event state is signaled.\n
 * 		When the state of an auto-reset event object is signaled, it remains signaled until a single waiting task is released;\n
 * 		the system then automatically resets the state to not-signaled. If no tasks are waiting, the event state remains signaled.
 * @see NEXSALEventReset
 * @see NEXSALEventInitial
 */
#define nexSAL_EventCreate         g_nexSALSyncObjectTable->fnEventCreate

/**
 * @ingroup sal_event
 *
 * NEXSAL_RETURN nexSAL_EventDelete(NEXSALEventHandle a_hEvent)\n
 * This function deletes event object.
 *
 * @param[in] a_hEvent Handle to the event object
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail 
 * @see NEXSAL_RETURN
 * @see NEXSALEventHandle
 */
#define nexSAL_EventDelete         g_nexSALSyncObjectTable->fnEventDelete

/**
 * @ingroup sal_event
 *
 * NEXSAL_RETURN nexSAL_EventSet(NEXSALEventHandle a_hEvent)\n
 * This function sets event state to signal. If the event is signaled, all the tasks waiting for the event are released and execution continues.
 *
 * @param[in] a_hEvent Handle to the event object
 * @return	eNEXSAL_RET_NO_ERROR: success\n
 *			Others: fail 
 * @see NEXSAL_RETURN
 * @see NEXSALEventHandle
 */
#define nexSAL_EventSet            g_nexSALSyncObjectTable->fnEventSet

/**
 * @ingroup sal_event
 *
 * NEXSAL_RETURN nexSAL_EventWait(NEXSALEventHandle a_hEvent, NXUINT32 a_uTimeoutMSec)\n
 * This function waits until event object signaled.
 *
 * @param a_hEvent [in] Handle to the event object 
 * @param a_uTimeoutMSec [in] Timeout interval in milliseconds. If a_uTimeoutMSec is NEXSAL_INFINITE, it never returns unless signaled.
 * @return
 *		- eNEXSAL_RET_NO_ERROR: if signaled\n
 *		- eNEXSAL_RET_TIMEOUT: if timeout occurred\n
 *		- eNEXSAL_RET_GENERAL_ERROR: if other error 
 * @note
 * 		In case that a task deletes the event in use of nexSAL_EventDelete while other task is waiting for the event,\n
 * 		the relevant operations can be different depending on the platform to be ported. Thus, the relevant operations are not defined here.\n
 * 		For example, nexSAL_EventWait can be returned because of the waiting task is cancelled. Also, the task cannot be waked up.\n
 * 		Therefore, the products created based on NexSAL must avoid the above case.
 * @see NEXSAL_RETURN
 * @see NEXSALEventHandle 
 */
#define nexSAL_EventWait           g_nexSALSyncObjectTable->fnEventWait

/**
 * @ingroup sal_event
 *
 * NEXSAL_RETURN nexSAL_EventClear(NEXSALEventHandle a_hEvent)\n
 * This function sets the state of the specified event object to not-signaled.
 *
 * @param[in] a_hEvent Handle to the event object
 * @return
 *		- eNEXSAL_RET_NO_ERROR: success\n
 *		- Others: fail
 * @note The nexSAL_EventClear function is used primarily for manual-reset event objects, which must be set explicitly to the not-signaled state.
 * @see NEXSAL_RETURN
 * @see NEXSALEventHandle 
 */
#define nexSAL_EventClear          g_nexSALSyncObjectTable->fnEventClear


/* Mutex */
/**
 * @ingroup sal_mutex
 *
 * brief NEXSALMutexHandle nexSAL_MutexCreate( )\n
 * This function creates new mutex object.
 *
 * @param None
 * @return Handle to the new mutex object. NEXSAL_INVALID_HANDLE value indicates an error.
 * @remark When mutex object is created, its state is unlocked.
 * @see NEXSALMutexHandle
 */
#define nexSAL_MutexCreate         g_nexSALSyncObjectTable->fnMutexCreate

/**
 * @ingroup sal_mutex
 *
 * NEXSAL_RETURN nexSAL_MutexDelete(NEXSALMutexHandle a_hMutex)\n
 * This function deletes mutex object.
 *
 * @param[in] a_hMutex Handle to mutex object
 * @return
 *		eNEXSAL_RET_NO_ERROR: success
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALMutexHandle 
 */
#define nexSAL_MutexDelete         g_nexSALSyncObjectTable->fnMutexDelete

/**
 * @ingroup sal_mutex
 *
 * NEXSAL_RETURN nexSAL_MutexLock(NEXSALMutexHandle a_hMutex, NXUINT32 a_uTimeoutMSec)\n
 * This function locks the mutex object.
 *
 * @param[in] a_hMutex Handle to mutex object
 * @param[in] a_uTimeoutMSec Interval in milliseconds. If a_uTimeoutMSec is NEXSAL_INFINITE, never returns with timeout.
 * @return
 *		- eNEXSAL_RET_NO_ERROR: if signaled\n
 *		- eNEXSAL_RET_TIMEOUT: if timeout occurred\n
 *		- eNEXSAL_RET_GENERAL_ERROR: if other error
 * @note When mutex object is created, its state is unlocked.\n
 *		If the waiting task which called nexSAL_MutexLock exists then nexSAL_MutexUnlock must be called \n
 *		before nexSAL_MutexDelete is called. (see. nexSAL_EventWait)
 * @see NEXSAL_RETURN
 * @see NEXSALMutexHandle  
 */
#define nexSAL_MutexLock           g_nexSALSyncObjectTable->fnMutexLock

/**
 * @ingroup sal_mutex
 *
 * NEXSAL_RETURN nexSAL_MutexUnlock (NEXSALMutexHandle a_hMutex)\n
 * This function unlocks the mutex object.
 *
 * @param[in] a_hMutex Handle to mutex object
 * @param[in] a_uTimeoutMSec Interval in milliseconds. If a_uTimeoutMSec is NEXSAL_INFINITE, never returns with timeout.
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALMutexHandle  
 */
#define nexSAL_MutexUnlock         g_nexSALSyncObjectTable->fnMutexUnlock


/* Semaphore */
/**
 * @ingroup sal_semaphore
 *
 * NEXSALSemaphoreHandle nexSAL_SemaphoreCreate(NXINT32 a_nInitCount, NXINT32 a_nMaxCount)\n
 * This function creates a new semaphore object.
 *
 * @param[in] a_nInitCount initial semaphore count\n
 *				This value must be greater than or equal to zero and less than or equal to a_nMaxCount. \n 
 *				The state of a semaphore is signaled when its count is greater than zero and nonsignaled when it is zero. 
 * @param[in] a_nMaxCount Max semaphore count\n 
 * 			This value must be greater than zero.
 * @return 
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSALSemaphoreHandle
 */
#define nexSAL_SemaphoreCreate     g_nexSALSyncObjectTable->fnSemaphoreCreate

/**
 * @ingroup sal_semaphore
 *
 * NEXSAL_RETURN nexSAL_SemaphoreDelete(NEXSALSemaphoreHandle a_hSema)\n
 * This function deletes a semaphore object.
 *
 * @param[in] a_hSema Handle to semaphore object
 * @return 
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALSemaphoreHandle 
 */
#define nexSAL_SemaphoreDelete     g_nexSALSyncObjectTable->fnSemaphoreDelete

/**
 * @ingroup sal_semaphore
 *
 * NEXSAL_RETURN nexSAL_SemaphoreRelease (NEXSALSemaphoreHandle a_hSema)\n
 * This function increases the count of the specified semaphore object by 1.
 *
 * @param[in] a_hSema semaphore to release
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALSemaphoreHandle  
 */
#define nexSAL_SemaphoreRelease    g_nexSALSyncObjectTable->fnSemaphoreRelease

/**
 * @ingroup sal_semaphore
 *
 * NEXSAL_RETURN nexSAL_SemaphoreWait(NEXSALSemaphoreHandle a_hSema, NXUINT32 a_uTimeoutMSec)\n
 * This function decreases semaphore count or wait.\n
 * - Count>=1, decrease count by 1\n
 * - Count=0, wait until other tasks release the semaphore
 *
 * @param[in] a_hSema Handle to semaphore object
 * @param[in] a_uTimeoutMSec Timeout interval in milliseconds. If a_uTimeoutmsec is NEXSAL_INFINITE, it never returns until signaled.
 * @return 
 *		eNEXSAL_RET_NO_ERROR: if signaled\n
 *		eNEXSAL_RET_TIMEOUT: if timeout occurred\n
 *		eNEXSAL_RET_GENERAL_ERROR: if other error 
 * @see NEXSAL_RETURN
 * @note
 *		When mutex object is created, its state is unlocked.\n
 *		If the waiting task which called nexSAL_SemaphoreWait exists, then nexSAL_SemaphoreRelease must be called\n
 *		before nexSAL_SemaphoreDelete is called. (See "nexSAL_EventWait")
 * @see NEXSAL_RETURN
 * @see NEXSALSemaphoreHandle  
 */
#define nexSAL_SemaphoreWait       g_nexSALSyncObjectTable->fnSemaphoreWait


/* atomic inc/dec */
/**
 * @ingroup sal_automic
 *
 * NXINT32 nexSAL_AtomicInc(NXINT32 *a_pnValue)\n
 * Read the 32-bit value (referred to as old) stored at location pointed by a_pnValue.\n
 * Compute atomically (old + 1) and store result at location pointed by a_pnValue. The function returns old.
 *
 * @param[in] a_pnValue pointer of atomic integer	
 * @return Old value of *a_pnValue
 */
#define nexSAL_AtomicInc			g_nexSALSyncObjectTable->fnAtomicInc

/**
 * @ingroup sal_automic
 *
 * NXINT32 nexSAL_AtomicDec(NXINT32 *a_pnValue)\n
 * Read the 32-bit value (referred to as old) stored at location pointed by a_pnValue.\n
 * Compute atomically (old - 1) and store result at location pointed by a_pnValue. The function returns old.
 *
 * @param[in] a_pnValue pointer of atomic integer
 * @returns Old value of *a_pnValue
 */
#define nexSAL_AtomicDec			g_nexSALSyncObjectTable->fnAtomicDec


/* Task */
/**
 * @ingroup sal_task
 *
 * NEXSALTaskHandle nexSAL_TaskCreate (const NXCHAR *a_strTaskName,
 *										NEXSALTaskFunc a_fnTask, 
 *										NXVOID *a_pParam, 
 *										NEXSALTaskPriority a_ePriority, 
 *										NXUINT32 a_uStackSize, 
 *										NEXSALTaskOption a_eCreateOption)\n
 * This function creates new task and executes it. 
 *
 * @param[in] a_strTaskName  Pointer to a null-terminated string that specifies the name of task. This value can be helpful to find bugs.
 * @param[in] a_fnTask Pointer to the function to be executed
 * @param[in] a_pParam Pointer to a variable to be passed to the task
 * @param[in] a_ePriority Priority Value
 * @param[in] a_uStackSize Size of Stack 
 * @param[in] a_eCreateOption Flags that controls the creation of the task. This flag includes platform specific options.
 * @return Handle to the new task. NEXSAL_INVALID_HANDLE value indicates an error.
 *
 * @see NEXSALTaskHandle
 * @see NEXSALTaskFunc
 * @see NEXSALTaskPriority
 * @see NEXSALTaskOption
 */
#define nexSAL_TaskCreate          g_nexSALTaskTable->fnTaskCreate

/**
 * @ingroup sal_task
 *
 * NEXSAL_RETURN nexSAL_TaskDelete(NXSYSTaskHandle a_hTask)\n
 * This function deletes a task object. If the task is running, it will be terminated and closed.
 *
 * @param[in] a_hTask Handle to Task 
 * @return 
 * 		eNEXSAL_RET_NO_ERROR: success\n
 * 		Others: fail
 * @note 
 *		When mutex object is created, its state is unlocked.\n
 *		The tasks which SAL defines are not deleted by TaskTerminate, and the task operations are just stopped. \n
 *		This function must be called to recall resources related to the task.
 * @see NEXSAL_RETURN
 * @see NEXSALTaskHandle 
 */
#define nexSAL_TaskDelete          g_nexSALTaskTable->fnTaskDelete

/**
 * @ingroup sal_task
 *
 * NEXSALTaskHandle nexSAL_TaskCurrent()\n
 * This function returns handle to current executing task
 *
 * @param None
 * @return Handle to current task
 * @see NEXSALTaskHandle  
 */
#define nexSAL_TaskCurrent         g_nexSALTaskTable->fnTaskCurrent

/**
 * @ingroup sal_task
 *
 * NEXSAL_RETURN nexSAL_TaskGetPriority(NEXSALTaskHandle a_hTask, NEXSALTaskPriority *a_pePriority)\n
 * This function gets the priority of target task.
 *
 * @param[in] a_hTask Handle to Task
 * @param[out] a_pePriority Pointer to get priority 
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail 
 * @see NEXSAL_RETURN
 * @see NEXSALTaskHandle 
 * @see NEXSALTaskPriority
 */
#define nexSAL_TaskGetPriority     g_nexSALTaskTable->fnTaskGetPriority

/**
 * @ingroup sal_task
 *
 * NEXSAL_RETURN nexSAL_TaskSetPriority(NEXSALTaskHandle a_hTask, NEXSALTaskPriority a_ePriority)\n
 * This function sets priority to target task.
 *
 * @param[in] a_hTask Handle to Task
 * @param[out] a_pePriority Priority to set
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail 
 * @see NEXSAL_RETURN
 * @see NEXSALTaskHandle 
 * @see NEXSALTaskPriority
 */
#define nexSAL_TaskSetPriority     g_nexSALTaskTable->fnTaskSetPriority

/**
 * @ingroup sal_task
 *
 * NXVOID nexSAL_TaskSleep (NXUINT32 a_uMSec)\n
 * This function suspends the calling task for the specified interval.
 *
 * @param[in] a_uMSec Time interval in milliseconds 
 * @return None
 */
#define nexSAL_TaskSleep           g_nexSALTaskTable->fnTaskSleep

/**
 * @ingroup sal_task
 *
 * NEXSAL_RETURN nexSAL_TaskTerminate(NEXSALTaskHandle a_hTask)\n
 * This function terminates a task.\n
 * Do not use this in normal situations. It's dangerous because the target task has no chance to clear resources. 
 *
 * @param[in] a_hTask Handle to Task
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN 
 * @see NEXSALTaskHandle
 */
#define nexSAL_TaskTerminate       g_nexSALTaskTable->fnTaskTerminate

/**
 * @ingroup sal_task
 *
 * NEXSAL_RETURN nexSAL_TaskWait(NEXSALTaskHandle a_hTask)\n
 * This function blocks the calling task until a target task terminates.
 *
 * @param[in] a_hTask Target task
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN 
 * @see NEXSALTaskHandle 
 */
#define nexSAL_TaskWait            g_nexSALTaskTable->fnTaskWait


/* Socket */
/**
 * @ingroup sal_socket
 *
 * NEXSALSockHandle nexSAL_NetSocket(NEXSALHandle a_hSAL, NEXSALSockType a_eType)\n 
 * This function creates a socket that is bound to a specific service provider.
 *
 * @param[in] a_hSAL  SAL Handle
 * @param[in] a_eType selects socket connection type
 * @return
 *		NEXSAL_INVALID_HANDLE: fail\n
 *		Others: socket handle
 * @note
 *		It required only when using Streaming Player.
 * @see NEXSALSockHandle
 * @see NEXSALHandle
 * @see NEXSALSockType
 */
#define nexSAL_NetSocket           g_nexSALSocketTable->fnNetSocket

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetClose(NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock)\n 
 * This function closes an existing socket.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Socket to close 
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN 
 * @see NEXSALHandle
 * @see NEXSALSockHandle
 */
#define nexSAL_NetClose            g_nexSALSocketTable->fnNetClose

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetConnect(NEXSALHandle a_hSAL, 
 * 										NEXSALSockHandle a_hSock, 
 * 										const NXCHAR *a_strAddr, 
 * 										NXUINT16 a_uwPort, 
 * 										NXUINT32 a_uTimeout)\n
 * This function establishes a connection to a specified socket.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket
 * @param[in] a_strAddr A null-terminated character string to connect. this string is expressed in the internet standard ".'' (dotted) notation.
 * @param[in] a_uwPort Port value of destination to connect to.
 * @param[in] a_uTimeout Timeout period
 * @return
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN 
 * @see NEXSALHandle
 * @see NEXSALSockHandle 
 */
#define nexSAL_NetConnect          g_nexSALSocketTable->fnNetConnect

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetBind(NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock, NXUINT16 a_uwPort)\n
 * This function associates a local address with a socket.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket 
 * @param[in] a_uwPort Port value to bind to
 * @return 
 *		eNEXSAL_RET_NO_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN 
 * @see NEXSALHandle
 * @see NEXSALSockHandle 
 */
#define nexSAL_NetBind             g_nexSALSocketTable->fnNetBind

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetSelect(NEXSALHandle a_hSAL,
 * 									NEXSALFDSet *a_pReadFDS,
 * 									NEXSALFDSet *a_pWriteFDS,
 * 									NEXSALFDSet *a_ExceptFDS,
 * 									NEXSALTimeValue *a_pTimeValue)\n
 * This function determines the status of one or more sockets, waiting if necessary, to perform synchronous I/O.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in,out] a_pReadFDS  An optional pointer to a set of sockets to be checked for readability 
 * @param[in,out] a_pWriteFDS An optional pointer to a set of sockets to be checked for writability 
 * @param[in,out] a_ExceptFDS An optional pointer to a set of sockets to be checked for errors
 * @param[in] a_pTimeValue The maximum time for select to wait, provided in the form of a NEXSALTimeValue structure.\n
 * 						Set the timeout parameter to null for blocking operations\n
 * @see NEXSALTimeValue
 * @return
 *		eNEXSAL_RET_WOULDBLOCK: would block\n
 *		eNEXSAL_RET_TIMEOUT: Timeout\n
 *		eNEXSAL_RET_SOCK_ERROR: Fail\n
 *		>0: Total number of socket handles that are included in NEXSALFDSet structure and are ready at designated I/O.
 * @see NEXSAL_RETURN 
 * @see NEXSALHandle
 * @see NEXSALFDSet
 * @see NEXSALTimeValue
 */
#define nexSAL_NetSelect           g_nexSALSocketTable->fnNetSelect

/**
 * @ingroup sal_socket
 *
 * NXSSIZE nexSAL_NetSendTo(NEXSALHandle a_hSAL, 
 * 								NEXSALSockHandle a_hSock, 
 * 								NXCHAR *a_pBuf, 
 * 								NXUSIZE a_uLen, 
 * 								NXCHAR *a_strAddr, 
 * 								NXUINT16 a_uwPort)\n
 * This function transfers data to assigned address.
 *
 * @param[in] a_hSAL  SAL Handle
 * @param[in] a_hSock Handle to socket.
 * @param[in] a_pBuf Buffer that holds data to be transferred
 * @param[in] a_uLen Length of the data in pBuf, in bytes
 * @param[in] a_strAddr Remote host address that data will be transferred to. Expressed in character row
 * @param[in] a_uwPort port of the remote host that data will be transferred to
 * @return
 *		>= 0: size of total transferred data\n
 *		Others: fail
 * @note 
 *		Applies to socket configured as NEXSAL_SOCK_DGRAM.
 * @see NEXSALHandle
 * @see NEXSALSockHandle
 */
#define nexSAL_NetSendTo           g_nexSALSocketTable->fnNetSendTo

/**
 * @ingroup sal_socket
 *
 * NXSSIZE nexSAL_NetSend(NEXSALHandle a_hSAL, 
 * 							NEXSALSockHandle a_hSock, 
 * 							NXCHAR *a_pBuf, 
 * 							NXUSIZE a_uLen)\n
 * This function sends data to connected socket.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket. 
 * @param[in] a_pBuf Buffer that holds data to be transferred
 * @param[in] a_uLen Length of the data in pBuf, in bytes
 * @returns 
 *		>= 0: size of total transferred data\n
 *		Others: fail
 * @note Applies to socket configured as NEXSAL_SOCK_STREAM.
 * @see NEXSALHandle
 * @see NEXSALSockHandle
 */
#define nexSAL_NetSend             g_nexSALSocketTable->fnNetSend

/**
 * @ingroup sal_socket
 *
 * NXSSIZE nexSAL_NetRecvFrom(NEXSALHandle a_hSAL, 
 * 								NEXSALSockHandle a_hSock, 
 * 								NXCHAR *a_pBuf, 
 * 								NXUSIZE a_uLen, 
 * 								NXUINT32 *a_puAddr, 
 * 								NXUINT16 *a_uwPort, 
 * 								NXUINT32 a_uTimeout)\n
 * This function receives a datagram from socket and returns the corresponding address and port number.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket.
 * @param[in] a_pBuf Buffer that holds data to be transferred 
 * @param[in] a_uLen Length of the data in pBuf, in bytes
 * @param[out] a_puAddr Address of the terminal that sent the received data
 * @param[out] a_uwPort Port of the terminal that sent the receives the data
 * @param[in] a_uTimeout  Timeout period
 * @return
 *		<=0: failure or no data to receive\n
 *		>0: size of the transferred data
 * @see NEXSALHandle
 * @see NEXSALSockHandle 
 */
#define nexSAL_NetRecvFrom         g_nexSALSocketTable->fnNetRecvFrom

/**
 * @ingroup sal_socket
 *
 * NXSSIZE nexSAL_NetRecv (NEXSALHandle a_hSAL,
 * 							NEXSALSockHandle a_hSock,
 * 							NXCHAR *a_pBuf,
 * 							NXUSIZE a_uLen,
 * 							NXUINT32 a_uTimeout)\n
 * This function receives a data from the connected socket.\n 
 * It will just return if there is no data to receive during pre-assigned timeout.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket. 
 * @param[in] a_pBuf Buffer that holds data to be transferred	 
 * @param[in] a_uLen Length of the data in pBuf, in bytes 
 * @param[in] a_uTimeout Timeout period
 * @return
 *		<=0: failure or no data to receive\n
 *		>0: size of the transferred data
 
 */
#define nexSAL_NetRecv             g_nexSALSocketTable->fnNetRecv

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetMultiGroup(NEXSALHandle a_hSAL,
 * 										NEXSALSockHandle a_hSock,
 * 										NXCHAR *a_strMultiGroupAddr,
 * 										NEXSALGroupManage a_eProp)\n 
 * This function set multicast socket option behavior.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket.
 * @param[in] a_strMultiGroupAddr MultiGroup Address
 * @param[in] a_eProp NEXSALGroupManage
 * @return
 *		eNEXSAL_RET_SOCK_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALHandle
 * @see NEXSALSockHandle
 * @see NEXSALGroupManage
 */
#define nexSAL_NetMultiGroup	   g_nexSALSocketTable->fnNetMultiGroup

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetSSLTunneling(NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock)\n
 * This function initiates the TLS/SSL handshake with a server.
 *
 * @param[in] a_hSAL  SAL Handle
 * @param[in] a_hSock  Handle to socket.
 * @return 
 *		eNEXSAL_RET_SOCK_ERROR: success\n 
 *		Others: fail
 */
#define nexSAL_NetSSLTunneling	   g_nexSALSocketTable->fnNetSSLTunneling

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetListen(NEXSALHandle a_hSAL,
 * 									NEXSALSockHandle a_hSock,
 * 									NXINT32 a_nBackLog)\n 
 * This function places a socket in a state in which it is listening for an incoming connection.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket.
 * @param[in] a_nBackLog The maximum length of the queue of pending connections.\n 
 * 						If set to SOMAXCONN, the underlying service provider responsible for socket s will set the backlog to a maximum reasonable value.\n 
 * 						There is no standard provision to obtain the actual backlog value.
 * @return 
 *		eNEXSAL_RET_SOCK_ERROR: success\n 
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALHandle
 * @see NEXSALSockHandle 
 */
#define nexSAL_NetListen		   g_nexSALSocketTable->fnNetListen

/**
 * @ingroup sal_socket
 *
 * NEXSALSockHandle nexSAL_NetAccept(NEXSALHandle a_hSAL,
 * 										NEXSALSockHandle a_hSock,
 * 										NXUINT32 *a_puAddr,
 * 										NXUINT16 *a_puwPort,
 * 										NXUINT32 a_uTimeOutInMSec,
 * 										NXINT32 *a_pnResult)\n 
 * This function permits an incoming connection attempt on a socket.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket
 * @param[out] a_puAddr IP address in network byte order 
 * @param[out] a_puwPort Port number
 * @param[in] a_uTimeOutInMSec Timeout period
 * @param[out] a_pnResult result value (0: success, -1: fail, -2: timeout)
 * @return 
 *		NEXSAL_INVALID_HANDLE: fail\n
 *		<0: file descriptor of the accepted socket
 * @see NEXSALSockHandle
 * @see NEXSALHandle
 */
#define nexSAL_NetAccept		   g_nexSALSocketTable->fnNetAccept

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetAsyncConnect(NEXSALHandle a_hSAL,
 *											NEXSALSockHandle a_hSock,
 *											NXCHAR *a_strAddr,
 *											NXUINT16 a_uwPort,
 *											NXUINT32 a_uTimeout)\n
 * This function establishes an asynchronous connection to a specified socket.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket
 * @param[in] a_strAddr A null-terminated character string to connect. This string is expressed in the internet standard ".'' (dotted) notation.
 * @param[in] a_uwPort Port value of destination to connect to
 * @param[in] a_uTimeout Timeout period
 * @return
 *		eNEXSAL_RET_SOCK_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALHandle
 * @see NEXSALSockHandle  
 */
#define nexSAL_NetAsyncConnect     g_nexSALSocketTable->fnNetAsyncConnect

/**
 * @ingroup sal_socket
 *
 * NEXSAL_RETURN nexSAL_NetAsyncSSLTunneling(NEXSALHandle a_hSAL, NEXSALSockHandle a_hSock)\n
 * This function initiates the TLS/SSL handshake with a server asynchronously.
 *
 * @param[in] a_hSAL SAL Handle
 * @param[in] a_hSock Handle to socket
 * @return
 *		eNEXSAL_RET_SOCK_ERROR: success\n
 *		Others: fail
 * @see NEXSAL_RETURN
 * @see NEXSALHandle
 * @see NEXSALSockHandle  
 */
#define nexSAL_NetAsyncSSLTunneling	g_nexSALSocketTable->fnNetSSLTunneling


/* Debugging */
/**
 * @ingroup sal_debug
 *
 * NXVOID nexSAL_DebugPrintf(const NXCHAR *a_strFormat, [argument]...) 
 * This function print formatted output. Where to display is diverse according to target system.
 *
 * @param[in] a_strFormat Format control. Refer to ANSI C run-time library manuals.
 * @param[in] argument Optional arguments
 * @return None 
 */
#define nexSAL_DebugPrintf         g_nexSALTraceTable->fnDegPrintf

/**
 * @ingroup sal_debug
 *
 * NXVOID nexSAL_DebugOutputString(NXCHAR *a_strOutput)
 * This function print formatted output. Where to display is diverse according to target system.
 *
 * @param a_strOutput [in] string format to print
 * @return None
 */
#define nexSAL_DebugOutputString   g_nexSALTraceTable->fnOutputString


/* ETC */
/**
 * @ingroup sal_etc
 *
 * NXUINT32 nexSAL_GetTickCount()\n
 * This function retrieves the current system time in milliseconds.
 *
 * @param None
 * @return None
 */
#define nexSAL_GetTickCount        g_nexSALEtcTable->fnGetTickCount

/**
 * @ingroup sal_etc
 * 
 * NXVOID nexSAL_GetEpochMS(NXUINT64 *a_puqEpochMS)\n
 * This function can get the time. The argument gives the number of milliseconds since the Epoch.\n
 * 00:00:00 Coordinated Universal Time (UTC), Thursday, 1 January 1970
 * 
 * @param a_puqEpchMS [out] the pointer of current milliseconds since the Epoch	 
 * @return None
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
pss        	2006/05/03      	1.5/2       	Remove nexSAL_TaskExit
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
