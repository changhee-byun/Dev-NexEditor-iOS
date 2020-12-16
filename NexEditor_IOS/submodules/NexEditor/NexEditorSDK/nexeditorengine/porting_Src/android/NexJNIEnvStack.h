/******************************************************************************
* File Name   :	NexJNIEnvStack.h
* Description : JNI Manager for jni call from Engine.
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2016 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
eric	2016/12/07	Draft.
-----------------------------------------------------------------------------*/

#ifndef _NEX_JNI_ENV_STACK_H_
#define _NEX_JNI_ENV_STACK_H_

#define DEFAULT_MAX_LIST_FOR_DELETEREF 4

class NexJNIEnvStack {
public:
	NexJNIEnvStack();
	~NexJNIEnvStack();
    static void setJavaVM(void* vm);
    inline JNIEnv * operator-> () const { return env; }
    inline JNIEnv * get() const         { return env; }
    
    void addAutoDeleteLocalRef(void* obj);
private:
	JNIEnv *env;
    void** list;
    long listCount;
    long listMax;
    void* default_list[DEFAULT_MAX_LIST_FOR_DELETEREF];
};

#endif // _NEX_JNI_ENV_STACK_H_
