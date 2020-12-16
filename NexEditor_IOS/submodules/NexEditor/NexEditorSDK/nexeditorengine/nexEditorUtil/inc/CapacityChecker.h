/******************************************************************************
 * File Name   :    CapacityChecker.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author    Date        Description of Changes
 -------------------------------------------------------------------------------
 cskb        2012/08/29    Draft.
 -----------------------------------------------------------------------------*/

#ifndef __CapacityChecker_H__
#define __CapacityChecker_H__

typedef struct VideoInfo
{
    int width;
    int height;
    int fps;
} VideoInfo;

class CapacityChecker
{
public:
    CapacityChecker( void );
    virtual ~CapacityChecker( void );
    
    static int isAvailableMemoryCapacity(char *pModelIdentifier, int iCount, void* pInfo);
	static int getAvailableMemoryCapacity(char *pModelIdentifier, unsigned int *pMaxMemorySize, unsigned int *pMaxDecoderCount);
    
private:
    static int getDeviceModel(char *pModelIdentifier);
    
};

#endif // __CapacityChecker_H__
