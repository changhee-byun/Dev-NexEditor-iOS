//
//  NexTheme_Config.h
//  //
//  Created by YooSunghyun on 07/11/2016.
//  Copyright © 2016 Nexstreaming. All rights reserved.
//

#ifndef NexTheme_Config_h
#define NexTheme_Config_h

// GL_GLEXT_PROTOTYPES is needed for glEGLImageTargetTexture2DOES, ...
// Refer to glext.h or glext2.h
#define GL_GLEXT_PROTOTYPES

#ifndef __APPLE__
#define EGL_ERROR_CHECKING
#define NEX_ENABLE_GLES_IMAGE
// EGL_EGLEXT_PROTOTYPES is needed for eglCreateImageKHR, ...
// Refer to eglext.h
#define EGL_EGLEXT_PROTOTYPES
#endif

#ifdef ANDROID
#define FOR_TEST_MEDIACODEC_DEC
#endif
#ifdef __APPLE__
#define USE_PLATFORM_SUPPORT
#endif
//#define NEX_THEME_RENDERER_DEBUG_LOGGING
//#define NEX_THEME_RENDERER_PROFILE_READPIXELS

#define GL_ERROR_CHECKING



#ifdef __APPLE__
#define USE_FRAGMENT_SHADER_LUT_64x4096
#endif

// __check_nexthemerenderer_loglevel is implemented in NexThemeRenderer.c
unsigned int __check_nexthemerenderer_loglevel( int level );
    
#if defined(ANDROID) || defined(__APPLE__)

#include "NexSAL_Internal.h"

#undef LOGXV
#undef LOGV
#undef LOGD
#undef LOGI
#undef LOGW
#undef LOGE
#define  LOGXV(...) ({  if(__check_nexthemerenderer_loglevel(7)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })
#define  LOGV(...)  ({  if(__check_nexthemerenderer_loglevel(6)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })
#define  LOGD(...)  ({  if(__check_nexthemerenderer_loglevel(5)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })
#define  LOGI(...)  ({  if(__check_nexthemerenderer_loglevel(4)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })
#define  LOGW(...)  ({  if(__check_nexthemerenderer_loglevel(3)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })
#define  LOGE(...)  ({  if(__check_nexthemerenderer_loglevel(2)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })
#define  LOGF(...)  ({  if(__check_nexthemerenderer_loglevel(1)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })
#define  LOGN(...)  ({  if(__check_nexthemerenderer_loglevel(0)) nexSAL_TraceCat(NEX_TRACE_CATEGORY_INFO,0,__VA_ARGS__); })

//#elif defined(__APPLE__TEMP)
//#define  LOGXV(S,...) ({  if(0) printf(__VA_ARGS__); })
//#define  LOGV(S,...)  ({  if(0) printf(__VA_ARGS__); })
//#define  LOGD(S,...)  ({  if(0) printf(__VA_ARGS__); })
//#define  LOGI(S,...)  ({  if(0) printf(__VA_ARGS__); })
//#define  LOGW(S,...)  ({  if(0) printf(__VA_ARGS__); })
//#define  LOGE(S,...)  ({  if(0) printf(__VA_ARGS__); })
//#define  LOGF(S,...)  ({  if(0) printf(__VA_ARGS__); })
//#define  LOGN(S,...)  ({  if(0) printf(__VA_ARGS__); })

#else // defined(ANDROID) || defined(__APPLE__)

#include <stdio.h>

#define  LOGXV(S,...) ({  if(__check_nexthemerenderer_loglevel(7)) printf(S "\n", ##__VA_ARGS__); })
#define  LOGV(S,...)  ({  if(__check_nexthemerenderer_loglevel(6)) printf(S "\n", ##__VA_ARGS__); })
#define  LOGD(S,...)  ({  if(__check_nexthemerenderer_loglevel(5)) printf(S "\n", ##__VA_ARGS__); })
#define  LOGI(S,...)  ({  if(__check_nexthemerenderer_loglevel(4)) printf(S "\n", ##__VA_ARGS__); })
#define  LOGW(S,...)  ({  if(__check_nexthemerenderer_loglevel(3)) printf(S "\n", ##__VA_ARGS__); })
#define  LOGE(S,...)  ({  if(__check_nexthemerenderer_loglevel(2)) printf(S "\n", ##__VA_ARGS__); })
#define  LOGF(S,...)  ({  if(__check_nexthemerenderer_loglevel(1)) printf(S "\n", ##__VA_ARGS__); })
#define  LOGN(S,...)  ({  if(__check_nexthemerenderer_loglevel(0)) printf(S "\n", ##__VA_ARGS__); })

#endif // defined(ANDROID) || defined(__APPLE__)

#endif /* NexTheme_Config_h */
