/******************************************************************************
* File Name   :	nexSal.c
* Description :	source file
*******************************************************************************

NexStreaming Confidential Proprietary
Copyright (C) 2008 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#if defined(_WIN8) || defined(WIN32) || defined(WINCE) 
	#include "windows.h" //; Just for dll
#endif

#include <string.h>
#include <stdio.h>

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"
#include "NexSAL_Com.h"

#if (defined(_WIN8) || defined(WIN32) || defined(WINCE))

BOOL APIENTRY DllMain(HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}

#endif


static NXVOID _NotImplementedFunction();

typedef NXVOID (*NULLFN)();
#define _NIFS_ ((NULLFN)_NotImplementedFunction)

static NEXSALMemoryFunctionTable nexSALMemTable =
{	/* 5 */
	(NEXSALMemAlloc)_NIFS_,
	(NEXSALMemCalloc)_NIFS_,
	(NEXSALMemFree)_NIFS_,
	(NEXSALMemAlloc2)_NIFS_,
	(NEXSALMemFree2)_NIFS_
};

static NEXSALFileFunctionTable nexSALFileTable =
{	/* 24 */
	(NEXSALFileClose)_NIFS_,			//200h
	(NEXSALFileFreeDiskSpaceA)_NIFS_,	//201h
	(NEXSALFileOpenA)_NIFS_,			//202h
	(NEXSALFileRead)_NIFS_,				//203h
	(NEXSALFileRemoveA)_NIFS_,			//204h
	(NEXSALFileSeek)_NIFS_,				//205h
	(NEXSALFileSeek64)_NIFS_,			//206h
	(NEXSALFileSize)_NIFS_,				//207h
	(NEXSALFileWrite)_NIFS_,			//208h
	(NEXSALFileRenameA)_NIFS_,			//209h
	(NEXSALFileGetAttributeA)_NIFS_,	//20Ah
	(NEXSALDirMakeA)_NIFS_,				//20Bh
	(NEXSALDirRemoveA)_NIFS_,			//20Ch
	(NEXSALFindFirstA)_NIFS_,			//20Dh
	(NEXSALFindNext)_NIFS_,
	(NEXSALFindClose)_NIFS_, 
	(NEXSALFileOpenW)_NIFS_,
	(NEXSALFileRemoveW)_NIFS_,

	(NEXSALFileFreeDiskSpaceW)_NIFS_,
	(NEXSALFileRenameW)_NIFS_,
	(NEXSALFileGetAttributeW)_NIFS_,
	(NEXSALDirMakeW)_NIFS_,
	(NEXSALDirRemoveW)_NIFS_,
	(NEXSALFindFirstW)_NIFS_,
};


static NEXSALSyncObjectFunctionTable nexSALSyncObjectTable =
{	/* 13 */
	(NEXSALEventCreate)_NIFS_, 
	(NEXSALEventDelete)_NIFS_, 
	(NEXSALEventSet)_NIFS_, 
	(NEXSALEventWait)_NIFS_, 
	(NEXSALEventClear)_NIFS_,
	(NEXSALMutexCreate)_NIFS_, 
	(NEXSALMutexDelete)_NIFS_, 
	(NEXSALMutexLock)_NIFS_, 
	(NEXSALMutexUnlock)_NIFS_, 
	(NEXSALSemaphoreCreate)_NIFS_,

	(NEXSALSemaphoreDelete)_NIFS_,
	(NEXSALSemaphoreRelease)_NIFS_, 
	(NEXSALSemaphoreWait)_NIFS_,

	(NEXSALAtomicInc)_NIFS_,
	(NEXSALAtomicDec)_NIFS_
};


static NEXSALTaskFunctionTable nexSALTaskTable =
{	/* 8 */
	(NEXSALTaskCreate)_NIFS_,			//400h
	(NEXSALTaskCurrent)_NIFS_,			//401h
	(NEXSALTaskDelete)_NIFS_,			//402h
	(NEXSALTaskGetPriority)_NIFS_,		//403h
	(NEXSALTaskSetPriority)_NIFS_,		//404h
	(NEXSALTaskSleep)_NIFS_,			//405h
	(NEXSALTaskTerminate)_NIFS_,		//406h
	(NEXSALTaskWait)_NIFS_				//407h
};

static NEXSALSocketFunctionTable nexSALSocketTable =
{	/* 17 */
	(NEXSALNetSocket)_NIFS_, 
	(NEXSALNetClose)_NIFS_, 
	(NEXSALNetConnect)_NIFS_, 
	(NEXSALNetBind)_NIFS_,				//503h
	(NEXSALNetSelect)_NIFS_,			//504h
	(NEXSALNetSendTo)_NIFS_,			//505h
	(NEXSALNetSend)_NIFS_,				//506h
	(NEXSALNetRecvFrom)_NIFS_,			//507h
	(NEXSALNetRecv)_NIFS_,				//508h
	(NEXSALNetMultiGroup)_NIFS_,		//509h
	(NEXSALNetSSLTunneling)_NIFS_,		//50Ah
	(NEXSALNetListen)_NIFS_,			//50Bh
	(NEXSALNetAccept)_NIFS_,			//50Ch
	(NEXSALNetAsyncConnect)_NIFS_, 
	(NEXSALNetAsyncSSLTunneling)_NIFS_	//50Eh
};

static NEXSALTraceFunctionTable nexSALTraceTable =
{	/* 2 */
	(NEXSALDebugPrintf)_NIFS_,			//600h
	(NEXSALDebugOutputString)_NIFS_		//601h
};

static NEXSALEtcFunctionTable nexSALEtcTable =
{	/* 2 */
	(NEXSALGetTickCount)_NIFS_,			//1000h
	(NEXSALGetMSecFromEpoch)_NIFS_		//1001h
};

NEXSAL_API NEXSALMemoryFunctionTable*       g_nexSALMemoryTable = &nexSALMemTable;
NEXSAL_API NEXSALFileFunctionTable*         g_nexSALFileTable = &nexSALFileTable;
NEXSAL_API NEXSALSyncObjectFunctionTable*   g_nexSALSyncObjectTable = &nexSALSyncObjectTable;
NEXSAL_API NEXSALTaskFunctionTable*         g_nexSALTaskTable = &nexSALTaskTable;
NEXSAL_API NEXSALSocketFunctionTable*       g_nexSALSocketTable = &nexSALSocketTable;
NEXSAL_API NEXSALTraceFunctionTable*        g_nexSALTraceTable = &nexSALTraceTable;
NEXSAL_API NEXSALEtcFunctionTable*          g_nexSALEtcTable = &nexSALEtcTable;


#define VERSION_STRING_HELPER(a, b, c, d) #a "." #b "." #c "." d
#define VERSION_STRING_MAKER(a, b, c, d) VERSION_STRING_HELPER(a, b, c, d)
#define VERSION_STRING  VERSION_STRING_MAKER(NEXSAL_VERSION_MAJOR, NEXSAL_VERSION_MINOR, NEXSAL_VERSION_PATCH, NEXSAL_VERSION_BRANCH)
#define VERSION_INFO    "NexStreaming System Adaptation Layer Library Version Prx " VERSION_STRING " Build Date " __DATE__

NEXSAL_API const NXCHAR* nexSAL_GetVersionString()
{
	return VERSION_STRING;
}

NEXSAL_API NXINT32 nexSAL_GetMajorVersion()
{
	return NEXSAL_VERSION_MAJOR;
}

NEXSAL_API NXINT32 nexSAL_GetMinorVersion()
{
	return NEXSAL_VERSION_MINOR;
}

NEXSAL_API NXINT32 nexSAL_GetPatchVersion()
{
	return NEXSAL_VERSION_PATCH;
}

NEXSAL_API const NXCHAR* nexSAL_GetBranchVersion()
{
	return NEXSAL_VERSION_BRANCH;
}

NEXSAL_API const NXCHAR* nexSAL_GetVersionInfo()
{
	return VERSION_INFO;
}

NEXSAL_API NXBOOL nexSAL_CheckSameVersion(NXINT32 a_nMajor, NXINT32 a_nMinor, NXINT32 a_nPatch, NXCHAR *a_strBranch)
{
	if (a_nMajor != NEXSAL_VERSION_MAJOR)
	{
		return FALSE;
	}

	if (a_nMinor != NEXSAL_VERSION_MINOR)
	{
		return FALSE;
	}

	if (a_nPatch != NEXSAL_VERSION_PATCH)
	{
		return FALSE;
	}

	if ((NULL == a_strBranch) || (0 != strcmp(NEXSAL_VERSION_BRANCH, a_strBranch)))
	{
		return FALSE;
	}

	return TRUE;
}

NEXSAL_API NXBOOL nexSAL_CheckCompatibleVersion(NXINT32 a_nCompatibilityNum)
{
	if (a_nCompatibilityNum != NEXSAL_COMPATIBILITY_NUM)
	{
		return FALSE;
	}

	return TRUE;
}

static NXVOID _NotImplementedFunction()
{
	if (g_nexSALTraceTable->fnDegPrintf && (NULLFN)g_nexSALTraceTable->fnDegPrintf != _NotImplementedFunction)
	{
		g_nexSALTraceTable->fnDegPrintf("\r\n[nexSAL] Error(may be halted) : Unimplemented function is called.\r\n");
	}

	return;
}

static NEXSAL_RETURN _RegisterMemoryFunctions(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	switch (a_uFuncIdx)
	{
		/* Memory */
		case NEXSAL_REG_MEM_ALLOC:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALMemoryTable->fnMemAlloc) : 0;
			g_nexSALMemoryTable->fnMemAlloc = (NEXSALMemAlloc)a_pFunc;
			break;
		case NEXSAL_REG_MEM_CALLOC:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALMemoryTable->fnMemCalloc) : 0;
			g_nexSALMemoryTable->fnMemCalloc = (NEXSALMemCalloc)a_pFunc;            
			break;
		case NEXSAL_REG_MEM_FREE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALMemoryTable->fnMemFree) : 0;
			g_nexSALMemoryTable->fnMemFree = (NEXSALMemFree)a_pFunc;            
			break;
		case NEXSAL_REG_MEM_ALLOC2:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALMemoryTable->fnMemAlloc2) : 0;
			g_nexSALMemoryTable->fnMemAlloc2 = (NEXSALMemAlloc2)a_pFunc;
			break;
		case NEXSAL_REG_MEM_FREE2:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALMemoryTable->fnMemFree2) : 0;
			g_nexSALMemoryTable->fnMemFree2 = (NEXSALMemFree2)a_pFunc;
			break;
		default :
			return eNEXSAL_RET_GENERAL_ERROR;
	}

	return eNEXSAL_RET_NO_ERROR;
}

static NEXSAL_RETURN _RegisterFileFunctions(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	switch (a_uFuncIdx)
	{
		/* File / Directory */
		case NEXSAL_REG_FILE_CLOSE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileClose) : 0;
			g_nexSALFileTable->fnFileClose = (NEXSALFileClose)a_pFunc;            
			break;
		case NEXSAL_REG_FILE_FREEDISKSPACEA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileFreeDiskSpaceA) : 0;
			g_nexSALFileTable->fnFileFreeDiskSpaceA = (NEXSALFileFreeDiskSpaceA)a_pFunc;            
			break;
		case NEXSAL_REG_FILE_FREEDISKSPACEW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileFreeDiskSpaceW) : 0;
			g_nexSALFileTable->fnFileFreeDiskSpaceW = (NEXSALFileFreeDiskSpaceW)a_pFunc;            
			break;
		case NEXSAL_REG_FILE_OPENA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileOpenA) : 0;
			g_nexSALFileTable->fnFileOpenA = (NEXSALFileOpenA)a_pFunc;
			break;
		case NEXSAL_REG_FILE_OPENW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileOpenW) : 0;
			g_nexSALFileTable->fnFileOpenW = (NEXSALFileOpenW)a_pFunc;
			break;
		case NEXSAL_REG_FILE_READ:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileRead) : 0;
			g_nexSALFileTable->fnFileRead = (NEXSALFileRead)a_pFunc;            
			break;
		case NEXSAL_REG_FILE_REMOVEA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileRemoveA) : 0;
			g_nexSALFileTable->fnFileRemoveA = (NEXSALFileRemoveA)a_pFunc;
			break;
		case NEXSAL_REG_FILE_REMOVEW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileRemoveW) : 0;
			g_nexSALFileTable->fnFileRemoveW = (NEXSALFileRemoveW)a_pFunc;
			break;
		case NEXSAL_REG_FILE_SEEK:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileSeek) : 0;
			g_nexSALFileTable->fnFileSeek = (NEXSALFileSeek)a_pFunc;
			break;
		case NEXSAL_REG_FILE_SEEK64:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileSeek64) : 0;
			g_nexSALFileTable->fnFileSeek64 = (NEXSALFileSeek64)a_pFunc;
			break;
		case NEXSAL_REG_FILE_SIZE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileSize) : 0;
			g_nexSALFileTable->fnFileSize = (NEXSALFileSize)a_pFunc;
			break;
		case NEXSAL_REG_FILE_WRITE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileWrite) : 0;
			g_nexSALFileTable->fnFileWrite = (NEXSALFileWrite)a_pFunc;
			break;
		case NEXSAL_REG_FILE_RENAMEA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileRenameA) : 0;
			g_nexSALFileTable->fnFileRenameA = (NEXSALFileRenameA)a_pFunc;
			break;
		case NEXSAL_REG_FILE_RENAMEW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileRenameW) : 0;
			g_nexSALFileTable->fnFileRenameW = (NEXSALFileRenameW)a_pFunc;
			break;
		case NEXSAL_REG_FILE_GET_ATTRIBUTEA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileGetAttributeA) : 0;
			g_nexSALFileTable->fnFileGetAttributeA = (NEXSALFileGetAttributeA)a_pFunc;
			break;
		case NEXSAL_REG_FILE_GET_ATTRIBUTEW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFileGetAttributeW) : 0;
			g_nexSALFileTable->fnFileGetAttributeW = (NEXSALFileGetAttributeW)a_pFunc;
			break;
		case NEXSAL_REG_DIR_MAKEA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnDirMakeA) : 0;
			g_nexSALFileTable->fnDirMakeA = (NEXSALDirMakeA)a_pFunc;
			break;
		case NEXSAL_REG_DIR_MAKEW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnDirMakeW) : 0;
			g_nexSALFileTable->fnDirMakeW = (NEXSALDirMakeW)a_pFunc;
			break;
		case NEXSAL_REG_DIR_REMOVEA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnDirRemoveA) : 0;
			g_nexSALFileTable->fnDirRemoveA = (NEXSALDirRemoveA)a_pFunc;
			break;
		case NEXSAL_REG_DIR_REMOVEW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnDirRemoveW) : 0;
			g_nexSALFileTable->fnDirRemoveW = (NEXSALDirRemoveW)a_pFunc;
			break;
		case NEXSAL_REG_FIND_FIRSTA:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFindFirstA) : 0;
			g_nexSALFileTable->fnFindFirstA = (NEXSALFindFirstA)a_pFunc;
			break;
		case NEXSAL_REG_FIND_FIRSTW:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFindFirstW) : 0;
			g_nexSALFileTable->fnFindFirstW = (NEXSALFindFirstW)a_pFunc;
			break;
		case NEXSAL_REG_FIND_NEXT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFindNext) : 0;
			g_nexSALFileTable->fnFindNext = (NEXSALFindNext)a_pFunc;
			break;
		case NEXSAL_REG_FIND_CLOSE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALFileTable->fnFindClose) : 0;
			g_nexSALFileTable->fnFindClose = (NEXSALFindClose)a_pFunc;
			break;
		default :
			return eNEXSAL_RET_GENERAL_ERROR;
	}

	return eNEXSAL_RET_NO_ERROR;
}

static NEXSAL_RETURN _RegisterSyncObjFunctions(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	switch (a_uFuncIdx)
	{
		/* Synchronization Objects */
		/* Event */
		case NEXSAL_REG_EVENT_CREATE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnEventCreate) : 0;
			g_nexSALSyncObjectTable->fnEventCreate = (NEXSALEventCreate)a_pFunc;            
			break;
		case NEXSAL_REG_EVENT_DELETE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnEventDelete) : 0;
			g_nexSALSyncObjectTable->fnEventDelete = (NEXSALEventDelete)a_pFunc;            
			break;
		case NEXSAL_REG_EVENT_SET:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnEventSet) : 0;
			g_nexSALSyncObjectTable->fnEventSet = (NEXSALEventSet)a_pFunc;
			break;
		case NEXSAL_REG_EVENT_WAIT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnEventWait) : 0;
			g_nexSALSyncObjectTable->fnEventWait = (NEXSALEventWait)a_pFunc;
			break;
		case NEXSAL_REG_EVENT_CLEAR:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnEventClear) : 0;
			g_nexSALSyncObjectTable->fnEventClear = (NEXSALEventClear)a_pFunc;
			break;

			/* Mutex */
		case NEXSAL_REG_MUTEX_CREATE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnMutexCreate) : 0;
			g_nexSALSyncObjectTable->fnMutexCreate = (NEXSALMutexCreate)a_pFunc;            
			break;
		case NEXSAL_REG_MUTEX_DELETE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnMutexDelete) : 0;
			g_nexSALSyncObjectTable->fnMutexDelete = (NEXSALMutexDelete)a_pFunc;            
			break;
		case NEXSAL_REG_MUTEX_LOCK:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnMutexLock) : 0;
			g_nexSALSyncObjectTable->fnMutexLock = (NEXSALMutexLock)a_pFunc;
			break;
		case NEXSAL_REG_MUTEX_UNLOCK:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnMutexUnlock) : 0;
			g_nexSALSyncObjectTable->fnMutexUnlock = (NEXSALMutexUnlock)a_pFunc;
			break;

			/* Semaphore */
		case NEXSAL_REG_SEMAPHORE_CREATE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnSemaphoreCreate) : 0;
			g_nexSALSyncObjectTable->fnSemaphoreCreate = (NEXSALSemaphoreCreate)a_pFunc;            
			break;
		case NEXSAL_REG_SEMAPHORE_DELETE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnSemaphoreDelete) : 0;
			g_nexSALSyncObjectTable->fnSemaphoreDelete = (NEXSALSemaphoreDelete)a_pFunc;            
			break;
		case NEXSAL_REG_SEMAPHORE_RELEASE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnSemaphoreRelease) : 0;
			g_nexSALSyncObjectTable->fnSemaphoreRelease = (NEXSALSemaphoreRelease)a_pFunc;            
			break;
		case NEXSAL_REG_SEMAPHORE_WAIT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnSemaphoreWait) : 0;
			g_nexSALSyncObjectTable->fnSemaphoreWait = (NEXSALSemaphoreWait)a_pFunc;            
			break;

		case NEXSAL_REG_ATOMIC_INC:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnAtomicInc) : 0;
			g_nexSALSyncObjectTable->fnAtomicInc = (NEXSALAtomicInc)a_pFunc;
			break;
		case NEXSAL_REG_ATOMIC_DEC:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSyncObjectTable->fnAtomicDec) : 0;
			g_nexSALSyncObjectTable->fnAtomicDec = (NEXSALAtomicDec)a_pFunc;
			break;

		default :
			return eNEXSAL_RET_GENERAL_ERROR;
	}

	return eNEXSAL_RET_NO_ERROR;
}

static NEXSAL_RETURN _RegisterTaskFunctions(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	switch (a_uFuncIdx)
	{
		/* Task */
		case NEXSAL_REG_TASK_CREATE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskCreate) : 0;
			g_nexSALTaskTable->fnTaskCreate = (NEXSALTaskCreate)a_pFunc;
			break;
		case NEXSAL_REG_TASK_CURRENT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskCurrent) : 0;
			g_nexSALTaskTable->fnTaskCurrent = (NEXSALTaskCurrent)a_pFunc;            
			break;
		case NEXSAL_REG_TASK_DELETE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskDelete) : 0;
			g_nexSALTaskTable->fnTaskDelete = (NEXSALTaskDelete)a_pFunc;            
			break;
		case NEXSAL_REG_TASK_GETPRIORITY:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskGetPriority) : 0;
			g_nexSALTaskTable->fnTaskGetPriority = (NEXSALTaskGetPriority)a_pFunc;            
			break;
		case NEXSAL_REG_TASK_SETPRIORITY:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskSetPriority) : 0;
			g_nexSALTaskTable->fnTaskSetPriority = (NEXSALTaskSetPriority)a_pFunc;            
			break;
		case NEXSAL_REG_TASK_SLEEP:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskSleep) : 0;
			g_nexSALTaskTable->fnTaskSleep = (NEXSALTaskSleep)a_pFunc;            
			break;
		case NEXSAL_REG_TASK_TERMINATE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskTerminate) : 0;
			g_nexSALTaskTable->fnTaskTerminate = (NEXSALTaskTerminate)a_pFunc;            
			break;
		case NEXSAL_REG_TASK_WAIT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTaskTable->fnTaskWait) : 0;
			g_nexSALTaskTable->fnTaskWait = (NEXSALTaskWait)a_pFunc;            
			break;
		default :
			return eNEXSAL_RET_GENERAL_ERROR;
	}

	return eNEXSAL_RET_NO_ERROR;
}

static NEXSAL_RETURN _RegisterSocketFunctions(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	switch (a_uFuncIdx)
	{
		/* Socket */
		case NEXSAL_REG_NET_SOCKET:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetSocket) : 0;
			g_nexSALSocketTable->fnNetSocket = (NEXSALNetSocket)a_pFunc;            
			break;
		case NEXSAL_REG_NET_CLOSE:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetClose) : 0;
			g_nexSALSocketTable->fnNetClose = (NEXSALNetClose)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_CONNECT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetConnect) : 0;
			g_nexSALSocketTable->fnNetConnect = (NEXSALNetConnect)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_ASYNC_CONNECT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetAsyncConnect) : 0;
			g_nexSALSocketTable->fnNetAsyncConnect = (NEXSALNetAsyncConnect)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_BIND:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetBind) : 0;
			g_nexSALSocketTable->fnNetBind = (NEXSALNetBind)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_SELECT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetSelect) : 0;
			g_nexSALSocketTable->fnNetSelect = (NEXSALNetSelect)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_SENDTO:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetSendTo) : 0;
			g_nexSALSocketTable->fnNetSendTo = (NEXSALNetSendTo)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_SEND:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetSend) : 0;
			g_nexSALSocketTable->fnNetSend = (NEXSALNetSend)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_RECVFROM:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetRecvFrom) : 0;
			g_nexSALSocketTable->fnNetRecvFrom = (NEXSALNetRecvFrom)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_RECV:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetRecv) : 0;
			g_nexSALSocketTable->fnNetRecv = (NEXSALNetRecv)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_MULTIGROUP:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetMultiGroup) : 0;
			g_nexSALSocketTable->fnNetMultiGroup = (NEXSALNetMultiGroup)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_SSL_TUNNELING:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetSSLTunneling) : 0;
			g_nexSALSocketTable->fnNetSSLTunneling = (NEXSALNetSSLTunneling)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_ASYNC_SSL_TUNNELING:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetAsyncSSLTunneling) : 0;
			g_nexSALSocketTable->fnNetAsyncSSLTunneling = (NEXSALNetAsyncSSLTunneling)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_LISTEN:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetListen) : 0;
			g_nexSALSocketTable->fnNetListen = (NEXSALNetListen)a_pFunc;                        
			break;
		case NEXSAL_REG_NET_ACCEPT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALSocketTable->fnNetAccept) : 0;
			g_nexSALSocketTable->fnNetAccept = (NEXSALNetAccept)a_pFunc;                        
			break;
		default :
			return eNEXSAL_RET_GENERAL_ERROR;
	}

	return eNEXSAL_RET_NO_ERROR;
}

static NEXSAL_RETURN _RegisterTraceFunctions(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	switch (a_uFuncIdx)
	{
		/* Debugging */
		case NEXSAL_REG_DBG_PRINTF:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTraceTable->fnDegPrintf) : 0;
			g_nexSALTraceTable->fnDegPrintf = (NEXSALDebugPrintf)a_pFunc;
			break;
		case NEXSAL_REG_DBG_OUTPUTSTRING:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALTraceTable->fnOutputString) : 0;
			g_nexSALTraceTable->fnOutputString = (NEXSALDebugOutputString)a_pFunc;
		default :
			return eNEXSAL_RET_GENERAL_ERROR;
	}

	return eNEXSAL_RET_NO_ERROR;
}

static NEXSAL_RETURN _RegisterEtcFunctions(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	switch (a_uFuncIdx)
	{
		/* ETC : Time */
		case NEXSAL_REG_GETTICKCOUNT:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALEtcTable->fnGetTickCount) : 0;
			g_nexSALEtcTable->fnGetTickCount = (NEXSALGetTickCount)a_pFunc;            
			break;

		case NEXSAL_REG_GETMSECFROMEPOCH:
			a_ppPreFunc ? (*a_ppPreFunc = (NXVOID*)g_nexSALEtcTable->fnGetMSecFromEpoch) : 0;
			g_nexSALEtcTable->fnGetMSecFromEpoch = (NEXSALGetMSecFromEpoch)a_pFunc;            
			break;

		default :
			return eNEXSAL_RET_GENERAL_ERROR;
	}

	return eNEXSAL_RET_NO_ERROR;
}

NEXSAL_API NEXSAL_RETURN nexSAL_RegisterFunction(NXUINT32 a_uFuncIdx, NXVOID *a_pFunc, NXVOID **a_ppPreFunc)
{
	NEXSAL_RETURN eRet = eNEXSAL_RET_PADDING;
	
	NXUINT32 uiFunctionClass = a_uFuncIdx & 0x0000FF00;

	switch (uiFunctionClass)
	{
		case NEXSAL_MEM_FUNCTIONS_INDEX :
			eRet = _RegisterMemoryFunctions(a_uFuncIdx, a_pFunc, a_ppPreFunc);
			break;

		case NEXSAL_FILE_FUNCTIONS_INDEX :
			eRet = _RegisterFileFunctions(a_uFuncIdx, a_pFunc, a_ppPreFunc);
			break;

		case NEXSAL_SYNCOBJ_FUNCTIONS_INDEX :
			eRet = _RegisterSyncObjFunctions(a_uFuncIdx, a_pFunc, a_ppPreFunc);
			break;

		case NEXSAL_TASK_FUNCTIONS_INDEX :
			eRet = _RegisterTaskFunctions(a_uFuncIdx, a_pFunc, a_ppPreFunc);
			break;

		case NEXSAL_SOCKET_FUNCTIONS_INDEX :
			eRet = _RegisterSocketFunctions(a_uFuncIdx, a_pFunc, a_ppPreFunc);
			break;

		case NEXSAL_TRACE_FUNCTIONS_INDEX :
			eRet = _RegisterTraceFunctions(a_uFuncIdx, a_pFunc, a_ppPreFunc);
			break;

		case NEXSAL_ETC_FUNCTIONS_INDEX :
			eRet = _RegisterEtcFunctions(a_uFuncIdx, a_pFunc, a_ppPreFunc);
			break;

		default:
			eRet = eNEXSAL_RET_GENERAL_ERROR;
			break;
	}

	return eRet;
}

NEXSAL_API NXBOOL nexSAL_FDIsSet(NEXSALSockHandle a_hSock, NEXSALFDSet *a_pFDS)
{
	NXUINT32 ui;

	for (ui = 0 ; ui < a_pFDS->fd_count ; ui++)
	{
		if (a_pFDS->fd_array[ui] == a_hSock)
			return TRUE;
	}

	return FALSE;
}

/*
// this comment will be removed when next major version will be changed. by yoosh

NXBOOL nexSal_FDIsSet(NEXSALSockHandle hSock, NEXSALFDSet *pSet)
{
	return nexSAL_FDIsSet(hSock, pSet);
}
*/

struct NEXSALHandle_struct
{
	NXUSIZE uID;
	NXVOID* pUserData;
};

NEXSAL_API NEXSALHandle nexSAL_Create(NXUSIZE a_uID)
{
	NEXSALHandle pRet = (NEXSALHandle)nexSAL_MemAlloc(sizeof(struct NEXSALHandle_struct));
	memset(pRet, 0, sizeof(struct NEXSALHandle_struct));
	pRet->uID = a_uID;

	return pRet;
}

NEXSAL_API NXVOID nexSAL_Destroy(NEXSALHandle a_hSAL)
{
	nexSAL_MemFree(a_hSAL);
}

NEXSAL_API NXBOOL nexSAL_SetUserData(NEXSALHandle a_hSAL, NXVOID *a_pUserData)
{
	if (a_hSAL)
	{
		a_hSAL->pUserData = a_pUserData;
	}

	return TRUE;
}

NEXSAL_API NXBOOL nexSAL_GetUserData(NEXSALHandle a_hSAL, NXVOID **a_ppUserData)
{
	if (a_hSAL && a_ppUserData)
	{
		*a_ppUserData = a_hSAL->pUserData;
	}

	return TRUE;
}

NEXSAL_API NXUSIZE nexSAL_GetUID(NEXSALHandle a_hSAL)
{
	if (a_hSAL)
	{
		return a_hSAL->uID;
	}

	return 0;
}


/*-----------------------------------------------------------------------------
Revision History:
Author		Date			Version		Description of Changes
-------------------------------------------------------------------------------
pss			2005/08/11		1.0			Draft
ysh			2005/08/15		1.1			Revision
ysh			2006/03/12		1.2			NEXSAL_TRACE_MODULE added
hsc			2006/03/30		1.3			File related 7 functions is added
ysh			2006/04/11		1.4/0		Revision(classifying functions, unregistered function call, _VersionCheck())
ysh			2006/08/08		2.1			Apis related to Socket is revised.
ysh			2006/09/12		2.2			UNICODE is introduced. nexSAL_FileOpen(A/W) is adapted.
pss			2006/09/27		2.2/1	    nexSAL_FileRemove(A/W) is adapted.(temporarily)
ysh			2006/10/10		2.3			Version up. patch number is added.
ysh			2006/11/03		2.3.2		When initialize global variable, explicit typecasting is adapted.
ysh			2007/10/10		2.4.0		nexSAL_MemDump is added.
ysh			2008/06/13		2.5.0		Unicode functions(on File) was added.
ysh			2008/07/25		2.6.0		NEXSAL_TRACEMODULE revision.
...
robin		2014/07/29		3.2.0		NEXSAL_PATCH_NUM -> NEXSAL_VERSION_PATCH
-----------------------------------------------------------------------------*/

