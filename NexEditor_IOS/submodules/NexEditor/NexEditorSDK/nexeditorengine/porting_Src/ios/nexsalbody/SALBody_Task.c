/*
 *  SALBody_task.c
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 02.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "SALBody_Task.h"
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <pthread.h>
#include "SALBodyTaskObjc.h"

#define OS_PRIORITY_LOWEST                     10
#define OS_PRIORITY_LOW                        20
#define OS_PRIORITY_BELOW_NORMAL               30
#define OS_PRIORITY_NORMAL                     40 
#define OS_PRIORITY_ABOVE_NORMAL               50
#define OS_PRIORITY_HIGH                       60
#define OS_PRIORITY_URGENT                     70
#define OS_PRIORITY_HIGHEST                    80

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

static NEXSALTaskPriority convert_priority_nex( int lPriority )
{
	int lRet;
    
	switch ( lPriority )
	{
		case OS_PRIORITY_LOWEST:		lRet = NEXSAL_PRIORITY_LOWEST;			break;
		case OS_PRIORITY_LOW:			lRet = NEXSAL_PRIORITY_LOW;				break;
		case OS_PRIORITY_BELOW_NORMAL:	lRet = NEXSAL_PRIORITY_BELOW_NORMAL;	break;
		case OS_PRIORITY_NORMAL:		lRet = NEXSAL_PRIORITY_NORMAL;			break;
		case OS_PRIORITY_ABOVE_NORMAL:	lRet = NEXSAL_PRIORITY_ABOVE_NORMAL;	break;
		case OS_PRIORITY_HIGH:			lRet = NEXSAL_PRIORITY_HIGH;			break;
		case OS_PRIORITY_URGENT:		lRet = NEXSAL_PRIORITY_URGENT;			break;
		case OS_PRIORITY_HIGHEST:		lRet = NEXSAL_PRIORITY_HIGHEST;			break;
		default:						lRet = lPriority; // for deprecated method
	}
	
	return lRet;
}

typedef struct _tag_TaskMasterStruct {

	NEXSALTaskFunc fn;
	void* pParam;
	const char *szTaskName;
} TaskMasterStruct;

int _task_master( void* p )
{
	int ret;
	TaskMasterStruct* pms = (TaskMasterStruct*)p;

	/* Setting the thread name helps your debugging easy */
	pthread_setname_np(pms->szTaskName);
	ret = SALBodyTaskMasterAutoreleasePool(pms->fn, pms->pParam);
	
	free( pms );
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
	TaskMasterStruct* tms;
		 
	if ( pthread_attr_init( &tattr ) != 0 )
	{
		return (NEXSALTaskHandle)0;
	}
		 
	pthread_attr_getschedparam( &tattr, &param );
	//printf( "default priority %d\n", param.sched_priority ); //Default 100
	param.sched_priority = convert_priority_os( iPriority );

	if ( pthread_attr_setschedparam( &tattr, &param ) != 0 )
	{
		 return (NEXSALTaskHandle)0;
	}

	stacksize = uiStackSize;

	if( pthread_attr_setstacksize( &tattr, stacksize ) != 0 )
	{
		return (NEXSALTaskHandle)0;	
	}

	tms = (TaskMasterStruct*)malloc(sizeof(TaskMasterStruct));
	tms->fn = fnTask;
	tms->pParam = pParam;
	tms->szTaskName = szTaskName;
	
	if ( pthread_create( &handle, &tattr, (void *(*)(void *))_task_master, tms ) != 0 )
	{
		return (NEXSALTaskHandle)0;
	}
		 
	if ( pthread_attr_destroy( &tattr ) != 0 )
	{
		return (NEXSALTaskHandle)0;
	}
	
	//printf("nexSALBody_TaskCreate : %s, %x\n", szTaskName, handle );
	return (NEXSALTaskHandle)handle;
}


int nexSALBody_TaskDelete( NEXSALTaskHandle hTask )
{
	return 0;
}

int nexSALBody_TaskWait( NEXSALTaskHandle hTask )
{
	void* pRet;

	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_FLOW, 0,"nexSALBody_TaskWait - Start (0x%x)\n", hTask);
	int ret = pthread_join( (pthread_t)hTask, &pRet ); 
	NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_FLOW, 0,"nexSALBody_TaskWait - End (0x%x), ret = %d\n", hTask, ret);

	return ret;
}

void nexSALBody_TaskSleep( unsigned int milliseconds )
{
	usleep( milliseconds * 1000 );
}

NEXSALTaskHandle nexSALBody_TaskGetCurrentHandle()
{
    return (NEXSALTaskHandle)pthread_self();
}

int nexSALBody_TaskGetPriority(NEXSALTaskHandle hTask, NEXSALTaskPriority *piPriority)
{
    struct sched_param param;
    int policy = SCHED_OTHER;
    int nRet = 0;
    
    if( hTask == 0 )
        return 1;
    
    nRet = pthread_getschedparam((pthread_t)hTask, &policy, &param);
    if( nRet != 0 )
    {
        return 1;
    }
    
    *piPriority = convert_priority_nex( param.sched_priority );
    return 0;
}

int nexSALBody_TaskSetPriority(NEXSALTaskHandle hTask, NEXSALTaskPriority iPriority)
{
    struct sched_param param;
    int policy = SCHED_OTHER;
    int nRet = 0;
    
    if( hTask == 0 )
        return 1;
    
    param.sched_priority = convert_priority_os(iPriority);
    
    
    nRet = pthread_setschedparam((pthread_t)hTask, policy, &param);
    if( nRet != 0 )
    {
        return 1;
    }
    return 0;
}



