/******************************************************************************
* File Name        : utils.cpp
* Description      : Utilities for NexAudioRenderer
* Revision History : Located at the bottom of this file
*******************************************************************************
*
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
*
*	Nexstreaming Confidential Proprietary
*	Copyright (C) 2006~2014 Nexstreaming Corporation
*	All rights are reserved by Nexstreaming Corporation
*
******************************************************************************/
#define LOCATION_TAG "utils"

#define LOG_TAG "nex_ar"
#include "./jni.h"

#include "./utils.h"

#include <pthread.h>
#include <vector>

#include <sys/system_properties.h>

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#include <android/log.h>
#include <dlfcn.h>

typedef int (*PFN_SYSTEM_PROP_GET)(const char *, char *);
static int __nex_system_property_get(const char* name, char* value)
{
    static PFN_SYSTEM_PROP_GET __real_system_property_get = NULL;
    if (!__real_system_property_get) {
        // libc.so should already be open, get a handle to it.
        void *handle = dlopen("libc.so", RTLD_NOLOAD);
        if (!handle) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot dlopen libc.so: %s.\n", dlerror());
        } else {
            __real_system_property_get = (PFN_SYSTEM_PROP_GET)dlsym(handle, "__system_property_get");
        }
        if (!__real_system_property_get) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot resolve __system_property_get(): %s.\n", dlerror());
        }
    }
    return (*__real_system_property_get)(name, value);
} 
#endif

namespace { // (anon0)
	class Initializers {
	public:
		Initializers()
		{
			pthread_mutex_init(&initializersLock, NULL);
		}
		~Initializers()
		{
			pthread_mutex_destroy(&initializersLock);
		}
		void registerInitializer(Nex_AR::Utils::Initializer *initializer)
		{
			pthread_mutex_lock(&initializersLock);
			initializers.push_back(initializer);
			pthread_mutex_unlock(&initializersLock);
		}
		void initializeAll()
		{
			pthread_mutex_lock(&initializersLock);
			std::vector<Nex_AR::Utils::Initializer *>::iterator it;

			for (it = initializers.begin(); initializers.end() != it; ++it)
			{
				(*it)->initialize();
			}
			pthread_mutex_unlock(&initializersLock);
		}
		void deinitializeAll()
		{
			pthread_mutex_lock(&initializersLock);
			std::vector<Nex_AR::Utils::Initializer *>::reverse_iterator rit;

			for (rit = initializers.rbegin(); initializers.rend() != rit; ++rit)
			{
				(*rit)->deinitialize();
			}

			pthread_mutex_unlock(&initializersLock);
		}
	private:
		pthread_mutex_t initializersLock;
		std::vector<Nex_AR::Utils::Initializer *> initializers;
	};
	Initializers initializersInstance;
} // namespace (anon0)

namespace Nex_AR {
namespace Utils {
	void registerInitializer(Initializer *initializer)
	{
		initializersInstance.registerInitializer(initializer);
	}

	void initializeAll()
	{
		initializersInstance.initializeAll();
	}

	void deinitializeAll()
	{
		initializersInstance.deinitializeAll();
	}

	int GetSystemProperty(const char* p,char* v)
	{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
        return __nex_system_property_get(p,v);
#else
		return __system_property_get(p,v);
#endif
	}
		

} // namespace Utils
} // namespace Nex_AR
