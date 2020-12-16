/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_PerformanceChecker.h
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/


#ifndef __NEXVIDEOEDITOR_PERFORMANCECHECKER_H__
#define __NEXVIDEOEDITOR_PERFORMANCECHECKER_H__

enum PC_ERROR_TYPE
{
    PC_ERROR_FRAME_DROP = 0,
    PC_ERROR_FRAME_WAIT_TIMEOUT,
    PC_ERROR_MAX
};

class CPerformanceChecker
{
public:
    CPerformanceChecker();
    virtual ~CPerformanceChecker();

    void Catch(int errorType);
    const long GetErrorCount(int errorType);

private:
    volatile long m_lCount[PC_ERROR_MAX];
};

extern void PerformanceChecker_Catch(int);

#endif // __NEXVIDEOEDITOR_PERFORMANCECHECKER_H__