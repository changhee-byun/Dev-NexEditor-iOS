/*
 *  SALBody_task.c
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 02.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "SALBody_Task.h"
#include "SALBody_Mem.h"
#include "stdio.h"
#include <unistd.h>
//#include <pthread.h>
//#include <utils/Log.h>
//#include <utils/threads.h>
#include <jni.h>
#include <pthread.h>

//#define _NOT_SERVICE_


#define	LOGE	nexSAL_DebugPrintf

#define OS_PRIORITY_LOWEST                     19
#define OS_PRIORITY_LOW                        10
#define OS_PRIORITY_BELOW_NORMAL               5
#define OS_PRIORITY_NORMAL                     0
#define OS_PRIORITY_ABOVE_NORMAL               -2
#define OS_PRIORITY_HIGH                       -4
#define OS_PRIORITY_URGENT                     -6
#define OS_PRIORITY_HIGHEST                    -8

/*
#define OS_PRIORITY_LOWEST                     10
#define OS_PRIORITY_LOW                        20
#define OS_PRIORITY_BELOW_NORMAL               30
#define OS_PRIORITY_NORMAL                     40 
#define OS_PRIORITY_ABOVE_NORMAL               50
#define OS_PRIORITY_HIGH                       60
#define OS_PRIORITY_URGENT                     70
#define OS_PRIORITY_HIGHEST                    80
*/
//namespace android {

static int convert_priority_os( NEXSALTaskPriority lPriority )
{
	int lRet;

	switch ( lPriority )
	{
		case NEXSAL_PRIORITY_LOWEST:		lRet = OS_PRIORITY_LOWEST;			break;
		case NEXSAL_PRIORITY_LOW:			lRet = OS_PRIORITY_LOW;				break;
		case NEXSAL_PRIORITY_BELOW_NORMAL:	lRet = OS_PRIORITY_BELOW_NORMAL;	break;
		case NEXSAL_PRIORITY_NORMAL:		lRet = OS_PRIORITY_NORMAL;			break;
		case NEXSAL_PRIORITY_ABOVE_NORMAL:	lRet = OS_PRIORITY_ABOVE_NORMAL;	break;
		case NEXSAL_PRIORITY_HIGH:			lRet = OS_PRIORITY_HIGH;			break;
		case NEXSAL_PRIORITY_URGENT:		lRet = OS_PRIORITY_URGENT;			break;
		case NEXSAL_PRIORITY_HIGHEST:		lRet = OS_PRIORITY_HIGHEST;			break;
		default:							lRet = lPriority; // for deprecated method
	}
	
	return lRet;
}

static JavaVM* jvm;
void nexSALBODY_TaskSetJVM( JavaVM* pJVM )
{
	jvm = pJVM;
}

typedef struct _tag_TaskMasterStruct {

	NEXSALTaskFunc fn;
	void* pParam;
	
} TaskMasterStruct;

int _task_master( void *p )
{
    	JNIEnv* env;
	int ret;
	TaskMasterStruct* pms = (TaskMasterStruct*)p;

#if defined(_NOT_SERVICE_)
    	LOGE("%s: AttachCurrentThread() before", __FUNCTION__);
	if (jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
	    LOGE("AttachCurrentThread() failed");
	    return -2;
	}
    	LOGE("%s: AttachCurrentThread() after", __FUNCTION__);
#endif
	
	ret = (pms->fn)(pms->pParam);

	nexSALBody_MemFree( pms );

#if defined(_NOT_SERVICE_)
    LOGE("%s: DetachCurrentThread() before", __FUNCTION__);
	if (jvm->DetachCurrentThread() != JNI_OK)
	{
	    LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
	}
    LOGE("%s: DetachCurrentThread() after", __FUNCTION__);
#endif
	
	return ret;
}

NEXSALTaskHandle nexSALBody_TaskCreate
		( char*					szTaskName
		, NEXSALTaskFunc		fnTask
		, void*					pParam
		, NEXSALTaskPriority	iPriority
		, unsigned int			uiStackSize
		, NEXSALTaskOption		fCreateOption        
		)
{
	pthread_t handle; //it just unsigned int
	pthread_attr_t tattr;
	struct sched_param param;
	size_t stacksize;
		 
	if ( pthread_attr_init( &tattr ) != 0 )
	{
		return (NEXSALTaskHandle)0;
	}
		 
	pthread_attr_getschedparam( &tattr, &param );
	printf( "default priority %d\n", param.sched_priority ); //Default 100
	param.sched_priority = convert_priority_os( iPriority );

	if ( pthread_attr_setschedparam( &tattr, &param ) != 0 )
	{
		 return (NEXSALTaskHandle)0;
	}
	/*
	if ( pthread_attr_setdetachstate( &tattr, PTHREAD_CREATE_DETACHED ) != 0 )
	{
		return (NEXSALTaskHandle)0;
	}
*/
#if 0	 
	if ( uiStackSize > PTHREAD_STACK_MIN )
	{
		stacksize = uiStackSize;
	}
	else
	{
		stacksize = PTHREAD_STACK_MIN;
	}
#else
	stacksize = uiStackSize;
#endif
	stacksize = stacksize*2;
	if( pthread_attr_setstacksize( &tattr, stacksize ) != 0 )
	{
		return (NEXSALTaskHandle)0;	
	}
	
	TaskMasterStruct* tms = (TaskMasterStruct*)nexSALBody_MemAlloc( sizeof(TaskMasterStruct) );
	tms->fn = fnTask;
	tms->pParam = pParam;
	
		 
	if ( pthread_create( &handle, &tattr, (void *(*)(void *))_task_master, tms ) != 0 )
	//if(androidCreateThreadEtc((void* )fnTask,(void *(*)(void *))_task_master, szTaskName, param.sched_priority, uiStackSize, &handle) != 0)
	{
		return (NEXSALTaskHandle)0;
	}
		 
	if ( pthread_attr_destroy( &tattr ) != 0 )
	{
		return (NEXSALTaskHandle)0;
	}
	
	//LOGE("=============================== (%s) task create  (%d)\n", szTaskName, param.sched_priority);
	//nexSAL_DebugPrintf("=============================== (%s) task create  (%d)\n", szTaskName, param.sched_priority);
	return (NEXSALTaskHandle)handle;
}


int nexSALBody_TaskDelete( NEXSALTaskHandle hTask )
{
	//int ret = pthread_detach( (pthread_t)hTask); 
	//nexSAL_DebugPrintf("========== pthread_detech %d\n", ret);
	return 0;
}

int nexSALBody_TaskWait( NEXSALTaskHandle hTask )
{
	void* pRet;
	int ret = pthread_join( (pthread_t)hTask, &pRet ); 

	return ret;
}

void nexSALBody_TaskSleep( unsigned int milliseconds )
{
	usleep( milliseconds * 1000 );
}

//};


