//=============================================================================
// Nextreaming Confidential Proprietary
// Copyright (C) 2005-2008 Nextreaming Corporation
// All rights are reserved by Nextreaming Corporation
//
// File       : NexSecureWare.h
// Description: 
//
// Revision History:
// Version   Date         Author			Description of Changes
//-----------------------------------------------------------------------------
// 1.0.0     2008/09/11   Kye-Beom Song		Draft
//=============================================================================
#include <ctype.h>
#include "NexBase64.h"
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char arr64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define MINUS1     -1

static const char arr64values[] = {
	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,
	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,
	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	62,		MINUS1,	MINUS1,	MINUS1,	 63,
	52,		53,		54,		55,		56,		57,		58,		59,		60,		61,		MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,
	MINUS1,	0,		1,		2,		3,		4,		5,		6,		7,		8,		9,		10,		11,		12,		13,		14,
	15,		16,		17,		18,		19,		20,		21,		22,		23,		24,		25,		MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1,
	MINUS1,	26,		27,		28,		29,		30,		31,		32,		33,		34,		35,		36,		37,		38,		39,		40,
	41,		42,		43,		44,		45,		46,		47,		48,		49,		50,		51,		MINUS1,	MINUS1,	MINUS1,	MINUS1,	MINUS1
};

#define DECODE64(c)  (isascii(c) ? arr64values[c] : MINUS1)

int BitsToBase64(unsigned char* pOut, int nOutLen, unsigned char* pIn, int nInLen)
{
	int nConvertLen = 0;
	if( pOut == 0x00 || pIn == 0x00 ) goto CONVERT_FAIL;

	for (; nInLen >= 3; nInLen -= 3)
	{
		if( nConvertLen + 4 > nOutLen ) goto CONVERT_FAIL;
		*pOut++ = arr64[pIn[0] >> 2];
		*pOut++ = arr64[((pIn[0] << 4) & 0x30) | (pIn[1] >> 4)];
		*pOut++ = arr64[((pIn[1] << 2) & 0x3c) | (pIn[2] >> 6)];
		*pOut++ = arr64[pIn[2] & 0x3f];
		pIn += 3;
		nConvertLen += 4;
    }
    if (nInLen > 0)
    {
		unsigned char fragment;
   
		if( nConvertLen + 4 > nOutLen ) goto CONVERT_FAIL;

		*pOut++ = arr64[pIn[0] >> 2];
		nConvertLen++;

		fragment = (pIn[0] << 4) & 0x30;
		if (nInLen > 1)
			fragment |= pIn[1] >> 4;
		*pOut++ = arr64[fragment];
		*pOut++ = (nInLen < 2) ? '=' : arr64[(pIn[1] << 2) & 0x3c];
		*pOut++ = '=';
		nConvertLen +=3;
	}
	*pOut = '\0';
	nConvertLen++;
	return nConvertLen;

CONVERT_FAIL:
	memset(pOut, 0x00, sizeof(unsigned char)*nOutLen);
	return 0;
}

int Base64ToBits(unsigned char* pOut, int nOutLen, unsigned char* pIn, int nInLen)
{
	int len = 0;
	register unsigned char digit1, digit2, digit3, digit4;

	if( pOut == 0x00 || pIn == 0x00 ) goto CONVERT_FAIL;

	if (pIn[0] == '+' && pIn[1] == ' ') pIn += 2;
	if (*pIn == '\r')  goto CONVERT_FAIL;

	do {
		digit1 = pIn[0];
		if (DECODE64(digit1) == MINUS1)
			return(len);
		digit2 = pIn[1];
		if (DECODE64(digit2) == MINUS1)
			return(len);
		digit3 = pIn[2];
		if (digit3 != '=' && DECODE64(digit3) == MINUS1)
			return(len); 
		digit4 = pIn[3];
		if (digit4 != '=' && DECODE64(digit4) == MINUS1)
			return(len);
		pIn += 4;
		*pOut++ = (DECODE64(digit1) << 2) | (DECODE64(digit2) >> 4);
		++len;
		if (digit3 != '=')
		{
			*pOut++ = ((DECODE64(digit2) << 4) & 0xf0) | (DECODE64(digit3) >> 2);
			++len;
			if (digit4 != '=')
			{
				*pOut++ = ((DECODE64(digit3) << 6) & 0xc0) | DECODE64(digit4);
				++len;
			}
		}
	} while (*pIn && *pIn != '\r' && digit4 != '=');

	return (len);

CONVERT_FAIL:
	// memset(pOut, 0x00, sizeof(unsigned char)*nOutLen);
	return 0;
}

