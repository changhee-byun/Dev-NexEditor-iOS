/******************************************************************************
* File Name        : jni.h
* Description      : JNI utilities for NexAudioRenderer
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

#ifndef _NEXAR_UTILS_JNI_H_
#define _NEXAR_UTILS_JNI_H_

#include <jni.h>

#include "NexSAL_Internal.h"

namespace Nex_AR {
namespace Utils {
namespace JNI {
	JavaVM * getJavaVM();

	void setJavaVMfromExternal(JavaVM* javaVM);	
	
	int jniThrowException(const char* className, const char* msg);

	class JNIEnvWrapper {
	public:
		JNIEnvWrapper();
		~JNIEnvWrapper();

		inline JNIEnv * operator-> () const { return env; }
		inline JNIEnv * get() const         { return env; }

		inline bool operator == (const JNIEnvWrapper& o) const
		{
			return env == o.env;
		}
		inline bool operator == (const JNIEnv *o) const
		{
			return env == o;
		}

		inline bool operator != (const JNIEnvWrapper& o) const
		{
			return env != o.env;
		}
		inline bool operator != (const JNIEnv *o) const
		{
			return env != o;
		}

	private:
		JNIEnv *env;
		bool needsDetach;
		JavaVM *vm;
	}; // class JNIEnvWrapper

	int jniThrowException(JNIEnv *env, const char *className, const char *msg);

#define JNI_OP(_op_,_ret_)                                                                                                      \
inline _ret_ _op_ (JNIEnvWrapper& env, jclass JCLASS, const char *name, const char *JNISignature, bool throwOnException = true) \
{                                                                                                                               \
    _ret_ ret = env->_op_(JCLASS, name, JNISignature);                                                                          \
    if (NULL == ret)                                                                                                            \
    {                                                                                                                           \
        if (throwOnException)                                                                                                   \
        {                                                                                                                       \
            nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "Can't find %s\n", name);                                                \
            jniThrowException(env.get(), "java/lang/Exception", NULL);                                                          \
        }                                                                                                                       \
        else                                                                                                                    \
        {                                                                                                                       \
            if (JNI_TRUE == env->ExceptionCheck())                                                                              \
                env->ExceptionClear();                                                                                          \
        }                                                                                                                       \
    }                                                                                                                           \
    return ret;                                                                                                                 \
}

	JNI_OP(GetMethodID,jmethodID)
	JNI_OP(GetStaticMethodID,jmethodID)
	JNI_OP(GetFieldID,jfieldID)
	JNI_OP(GetStaticFieldID,jfieldID)

	jobject NewDirectByteBuffer(jlong capacity, void *&address);
	void DeleteDirectByteBuffer(jobject directByteBuffer);
}; // namespace JNI
}; // namespace Utils
}; // namespace Nex_AR

#endif //#ifndef _NEXAR_UTILS_JNI_H_

