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

#include "nexSal_api.h"
#include "nexSal_internal.h"

#ifdef __cplusplus
extern "C" {
#endif
	
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

NEXSALTaskHandle nexSALBody_TaskGetCurrentHandle();    
int nexSALBody_TaskGetPriority(NEXSALTaskHandle hTask, NEXSALTaskPriority *piPriority);
int nexSALBody_TaskSetPriority(NEXSALTaskHandle hTask, NEXSALTaskPriority iPriority);

#ifdef __cplusplus
}
#endif
		
#endif

