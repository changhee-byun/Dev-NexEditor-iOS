/*
 * File Name   : LayerGLContext.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#ifndef LayerGLContext_h
#define LayerGLContext_h

typedef enum {
    LayerGLContextIndexPreview = 0,
    LayerGLContextIndexExport = 1,
    LayerGLContextIndexFirst = LayerGLContextIndexPreview,
    LayerGLContextIndexLast = LayerGLContextIndexExport
} LayerGLContextIndex;

#define NUM_TEXTURE_CONTEXTS    (LayerGLContextIndexLast + 1)

#define LAYER_GL_INVALID_TEXTURE_ID  (-1)

#endif /* LayerGLContext_h */
