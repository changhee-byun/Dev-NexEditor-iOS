//
//  NexThemeRenderer_Internal_Log.h
//  NexEffectRenderer
//
//  Created by Simon Kim on 8/18/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

#ifndef NexThemeRenderer_Internal_Log_h
#define NexThemeRenderer_Internal_Log_h

#if defined(ANDROID)
#include <android/log.h>
#define LOGFORCE(...)    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#elif defined(__APPLE__)
#define LOGFORCE(...)
#else
#define LOGFORCE(...)
#endif

#endif /* NexThemeRenderer_Internal_Log_h */
