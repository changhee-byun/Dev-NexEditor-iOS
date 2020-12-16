
#ifndef _NEX_TEXTURES_LOGGER_H_
#define _NEX_TEXTURES_LOGGER_H_

//#define TEXTURE_LOGGING

#ifdef __cplusplus
extern "C" {
#endif

#ifdef TEXTURE_LOGGING
    void Logging_glGenTextures(int n, unsigned int* textures, const char* where, int line);
    void Logging_glDeleteTextures (int n, unsigned int* textures, const char* where, int line);
    #define GL_GenTextures(n,t) Logging_glGenTextures(n,t,__FILE__,__LINE__)
    #define GL_DeleteTextures(n,t) Logging_glDeleteTextures(n,t,__FILE__,__LINE__)
#else
    #define GL_GenTextures(n,t) glGenTextures(n,t)
    #define GL_DeleteTextures(n,t) glDeleteTextures(n,t)
#endif

#ifdef __cplusplus
}
#endif

#endif //_NEX_TEXTURES_LOGGER_H_
