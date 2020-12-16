//
//  NexTheme_Internal.h
//  NexTheme
//
//  Created by Matthew Feinberg on 8/31/11.
//  Copyright (c) 2011 NexStreaming. All rights reserved.
//

#ifndef NexTheme_NexTheme_Internal_h
#define NexTheme_NexTheme_Internal_h
#define NXT_ALLOW_DIRECT_THEME_ACCESS
#include "NexTheme.h"
#undef NXT_ALLOW_DIRECT_THEME_ACCESS

#include <pthread.h>

#include "NexTheme_Parser.h"
#include "NexTheme_Nodes.h"


struct NXT_ThemeSet_ {
    NXT_HThemeParser themeParser;
    int retainCount;
    pthread_mutex_t retainCountLock;
    NXT_NodeHeader *rootNode;
};


#endif
