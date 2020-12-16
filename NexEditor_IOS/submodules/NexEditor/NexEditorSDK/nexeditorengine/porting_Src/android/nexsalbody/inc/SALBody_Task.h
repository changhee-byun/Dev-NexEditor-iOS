/*
 *  SALBody_task.h
 *  FirstVBA
 *
 *  Created by Sunghyun Yoo on 08. 12. 02.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef _SALBODY_TASK_HEADER_
#define _SALBODY_TASK_HEADER_

#include "NexSAL_API.h"
#include "NexSAL_Internal.h"
//#include "android_runtime/AndroidRuntime.h"

//namespace android {

#ifdef __cplusplus
extern "C" {
#endif

//void nexSALBODY_TaskSetJVM( JavaVM* pJVM );

NEXSALTaskHandle nexSALBody_TaskCreate
( char*					szTaskName
 , NEXSALTaskFunc		fnTask
 , void*					pParam
 , NEXSALTaskPriority	iPriority
 , unsigned int			uiStackSize
 , NEXSALTaskOption		fCreateOption        
 );

int nexSALBody_TaskDelete( NEXSALTaskHandle hTask );
int nexSALBody_TaskWait( NEXSALTaskHandle hTask );
void nexSALBody_TaskSleep( unsigned int milliseconds );

#ifdef __cplusplus
}
#endif
//};		
#endif

