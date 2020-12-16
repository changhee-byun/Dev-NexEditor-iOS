#include <pthread.h>
#include <dlfcn.h>

#include "SALBody_Debug.h"
#include "SALBody_File.h"
#include "SALBody_Mem.h"
#include "SALBody_Sock.h"
#include "SALBody_SyncObj.h"
#include "SALBody_Task.h"
#include "SALBody_Time.h"

#include "utils/utils.h"
#include "utils/jni.h"

namespace {

class RegisterSAL : Nex_AR::Utils::Initializer {
public:
	virtual void initialize()
	{
		JavaVM *vm = Nex_AR::Utils::JNI::getJavaVM();
		if (NULL == vm)
		{
			nexSALBody_DebugPrintf((char *)"can't get vm!\n");
		}

//		nexSALBODY_TaskSetJVM(vm);
	}

	virtual void deinitialize()
	{

	}

	RegisterSAL()
	{
		Nex_AR::Utils::registerInitializer(this);
		//nexSAL_MemCheckStart();

		nexSALBODY_SyncObjectsInit();
		//nexSALBody_InitialFD();

		// Heap
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_ALLOC, (void*)nexSALBody_MemAlloc, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_CALLOC, (void*)nexSALBody_MemCalloc, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MEM_FREE, (void*)nexSALBody_MemFree, NULL );

		// File
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_OPENA, (void*)nexSALBody_FileOpen, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_CLOSE, (void*)nexSALBody_FileClose, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_READ, (void*)nexSALBody_FileRead, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_WRITE, (void*)nexSALBody_FileWrite, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK, (void*)nexSALBody_FileSeek, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SEEK64, (void*)nexSALBody_FileSeek64, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_SIZE, (void*)nexSALBody_FileSize, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_FILE_REMOVEA, (void*)nexSALBody_FileRemove, NULL );

		// Sync Objects
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_CREATE, (void*)nexSALBody_EventCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_DELETE, (void*)nexSALBody_EventDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_SET, (void*)nexSALBody_EventSet, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_WAIT, (void*)nexSALBody_EventWait, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_EVENT_CLEAR, (void*)nexSALBody_EventClear, NULL );

		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_CREATE, (void*)nexSALBody_MutexCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_DELETE, (void*)nexSALBody_MutexDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_LOCK, (void*)nexSALBody_MutexLock, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_MUTEX_UNLOCK, (void*)nexSALBody_MutexUnlock, NULL );

		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_CREATE, (void*)nexSALBody_SemaphoreCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_DELETE, (void*)nexSALBody_SemaphoreDelete, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_RELEASE, (void*)nexSALBody_SemaphoreRelease, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_SEMAPHORE_WAIT, (void*)nexSALBody_SemaphoreWait, NULL );

		nexSAL_RegisterFunction( NEXSAL_REG_ATOMIC_INC, (void*)nexSALBody_AtomicInc, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_ATOMIC_DEC, (void*)nexSALBody_AtomicDec, NULL );

		// Task
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_CREATE, (void*)nexSALBody_TaskCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_DELETE, (void*)nexSALBody_TaskDelete, NULL );
		//nexSAL_RegisterFunction( NEXSAL_REG_TASK_CURRENT, (void*)nexSALBody_TaskCurrent, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_SLEEP, (void*)nexSALBody_TaskSleep, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_TASK_WAIT, (void*)nexSALBody_TaskWait, NULL );

		nexSAL_RegisterFunction( NEXSAL_REG_DBG_PRINTF, (void*)nexSALBody_DebugPrintf, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_DBG_OUTPUTSTRING, (void*)nexSALBody_DebugOutputString, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_GETTICKCOUNT, (void*)nexSALBody_GetTickCount, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_GETMSECFROMEPOCH, (void*)nexSALBody_GetMSecFromEpoch, NULL );		
#if 0
		//Socket
		nexSAL_RegisterFunction( NEXSAL_REG_NET_SOCKET, (void*)nexSALBody_SockCreate, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_CLOSE, (void*)nexSALBody_SockClose, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_CONNECT, (void*)nexSALBody_SockConnect, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_BIND, (void*)nexSALBody_SockBind, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_SELECT, (void*)nexSALBody_SockSelect, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_SENDTO, (void*)nexSALBody_SockSendTo, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_SEND, (void*)nexSALBody_SockSend, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_RECVFROM, (void*)nexSALBody_SockRecvFrom, NULL );
		nexSAL_RegisterFunction( NEXSAL_REG_NET_RECV, (void*)nexSALBody_SockRecv, NULL );
#endif
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_FLOW, "FLW" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_E_AUDIO, "AUD" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_E_VIDEO, "VID" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_P_AUDIO, "AUD" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_P_VIDEO, "VID" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_E_SYS, "SYS" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_P_SYS, "SYS" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_INFO, "INF" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_WARNING, "WRN" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_ERR, "ERR" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_F_READER, "F_R" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_F_WRITER, "F_W" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_CRAL, "CRA" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_SOURCE, "SRC" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_TARGET, "TAR" );
		nexSAL_TraceSetPrefix( NEX_TRACE_CATEGORY_NONE, "NON" );

		nexSAL_TraceSetCondition( NEX_TRACE_CATEGORY_FLOW, 0 );
		nexSAL_TraceSetCondition( NEX_TRACE_CATEGORY_ERR, 0 );
		nexSAL_TraceSetCondition( NEX_TRACE_CATEGORY_WARNING, 0 );
		nexSAL_TraceSetCondition( NEX_TRACE_CATEGORY_INFO, -1 );
		nexSAL_TraceSetCondition( NEX_TRACE_CATEGORY_SOURCE, 0 );
		nexSAL_TraceSetCondition( NEX_TRACE_CATEGORY_P_AUDIO, 0 );
	}

	virtual ~RegisterSAL()
	{
		nexSALBODY_SyncObjectsDeinit();

//		nexSAL_MemCheckEnd();
	}
};

RegisterSAL auto_registerSAL;

}


