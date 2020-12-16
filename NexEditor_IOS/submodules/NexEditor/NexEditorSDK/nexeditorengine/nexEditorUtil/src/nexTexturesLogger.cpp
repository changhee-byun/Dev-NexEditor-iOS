#include <stdio.h>
#include <stdlib.h>
#include "nexTexturesLogger.h"
#include "NexSAL_Internal.h"

#ifdef TEXTURE_LOGGING

#if defined(ANDROID)
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
#elif defined(__APPLE__)
    #include "TargetConditionals.h"
    #if TARGET_OS_IOS > 0
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #endif
#endif

static int count_texures_not_threadsafe = 0;

void Logging_glGenTextures(int n, unsigned int* textures, const char* where, int line)
{
    glGenTextures(n,textures);
    
    count_texures_not_threadsafe += n;
    char * temps = (char*)malloc(n*8); temps[0] = '\0';

    for ( int i = 0 ; i < n ; i++ ) {
        char buff [16]; 
        int ret = snprintf(buff, sizeof(buff), ",%d", textures[i]);
        if(ret < sizeof(buff))
            strcat( temps, buff );
    }

	nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TEXL][%s,%d] Gen %s (%d)", strrchr(where, '/') ? strrchr(where, '/') + 1 : where, line, temps, count_texures_not_threadsafe);

    free(temps);
}

void Logging_glDeleteTextures (int n, unsigned int* textures, const char* where, int line)
{
    char * temps = (char*)malloc(n*8); temps[0] = '\0';

    for ( int i = 0 ; i < n ; i++ ) {
        char buff [16]; 
        int ret = snprintf(buff, sizeof(buff), ",%d", textures[i]);
        if(ret < sizeof(buff))
            strcat( temps, buff );
    }

    count_texures_not_threadsafe -= n;
    nexSAL_TraceCat(NEX_TRACE_CATEGORY_ERR, 0, "[TEXL][%s,%d] Del %s (%d)", strrchr(where, '/') ? strrchr(where, '/') + 1 : where, line, temps, count_texures_not_threadsafe);

    free(temps);

    glDeleteTextures(n,textures);
}

#endif
