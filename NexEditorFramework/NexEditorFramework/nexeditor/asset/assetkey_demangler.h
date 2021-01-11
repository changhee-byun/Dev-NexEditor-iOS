/**
 * File Name   : assetkey_demangler.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#ifndef assetkey_demangler_h
#define assetkey_demangler_h

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif
    void akd_demangle_digits(const int *digits, size_t length, char *output);
    void akd_scan_hex( const char *s, size_t length, uint8_t *output);
    void akd_demangle_keybytes(uint8_t *bufinout, size_t length);
    
#ifdef __cplusplus
};
#endif

#endif /* assetkey_demangler_h */
