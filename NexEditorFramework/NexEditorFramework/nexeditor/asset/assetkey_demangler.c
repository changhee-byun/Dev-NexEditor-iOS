/**
 * File Name   : assetkey_demangler.c
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

#include "assetkey_demangler.h"

void akd_demangle_digits(const int *digits, size_t length, char *output)
{
    for(int i = 0; i < length; i++ ) {
        output[i] = (char)(digits[i] ^ 0x5A);
    }
}

void akd_scan_hex( const char *s, size_t length, uint8_t *output)
{
    unsigned int digit;
    for( int i = 0; i < length/2; i ++) {
        sscanf( &s[i * 2], "%02x", &digit);
        output[i] = (uint8_t) digit;
    }
}

void akd_demangle_keybytes(uint8_t *bufinout, size_t length)
{
    uint8_t mask = bufinout[0] ^ 32;
    uint8_t *buf = &bufinout[1];
    for(int i = 0; i < (length -1); i++) {
        uint8_t b = buf[i];
        buf[i-1] = (uint8_t)  ((((b >>4) & 0x0F) | ((b << 4) & 0xF0)) ^ mask);
    }
}
