/******************************************************************************
* File Name        : NexMediaFormat.h
* Description      : MediaFormat interface declarations for NexCRALBody_MC
*******************************************************************************
*
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
*
*	Nexstreaming Confidential Proprietary
*	Copyright (C) 2006~2013 Nexstreaming Corporation
*	All rights are reserved by Nexstreaming Corporation
*
******************************************************************************/

#ifndef _NEXCRAL_NEXMEDIAFORMAT_H
#define _NEXCRAL_NEXMEDIAFORMAT_H

#include <stdlib.h>
#include <jni.h>

namespace Nex_MC {
class NexMediaFormat {
protected:
	NexMediaFormat() {};
	~NexMediaFormat() {};
public:
	virtual int setInteger(const char *key, int value) = 0;
	virtual int getInteger(const char *key, int &result) = 0;
	virtual int setByteBuffer(const char *key, jobject byteBuffer) = 0;
	virtual int toString(char *buffer, size_t bufferSize, size_t &result) = 0;
};
};

#endif //#ifndef _NEXCRAL_NEXMEDIAFORMAT_H

