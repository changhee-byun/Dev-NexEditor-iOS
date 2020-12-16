/******************************************************************************
* File Name   :	NexTypeDef.h
* Description :	Data type definition header
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2014 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXTYPE_DEFINITION_INCLUDED_
#define _NEXTYPE_DEFINITION_INCLUDED_

#include <stddef.h>

#define NEXDEF_VERSION_MAJOR	1
#define NEXDEF_VERSION_MINOR	0
#define NEXDEF_VERSION_PATCH	5
#define NEXDEF_VERSION_BRANCH	"OFFICIAL"


#ifndef NULL
	#ifdef __cplusplus
		#define NULL		0
	#else
		#define NULL		((NXVOID*)0)
	#endif
#endif
	
#ifndef FALSE
	#define FALSE	0
#endif
	
#ifndef TRUE                    
	#define TRUE	1
#endif

// printf macros for NXSSIZE, in the style of inttypes.h
#ifdef _MSC_VER
#define __PRIS_PREFIX "I"
#else
#define __PRIS_PREFIX "z"
#endif

#define PRIdS __PRIS_PREFIX "d"
#define PRIxS __PRIS_PREFIX "x"
#define PRIuS __PRIS_PREFIX "u"
#define PRIXS __PRIS_PREFIX "X"
#define PRIoS __PRIS_PREFIX "o"

#define MAX_SIGNED8BIT		0x7F
#define MAX_UNSIGNED8BIT	0xFF
#define MAX_SIGNED16BIT		0x7FFF
#define MAX_UNSIGNED16BIT	0xFFFF
#define MAX_SIGNED32BIT		0x7FFFFFFF
#define MAX_UNSIGNED32BIT	0xFFFFFFFF
#define MAX_SIGNED64BIT		0x7FFFFFFFFFFFFFFF
#define MAX_UNSIGNED64BIT	0xFFFFFFFFFFFFFFFF

#define NXUSIZE_MAX			SIZE_MAX
#define NXSSIZE_MAX			PTRDIFF_MAX

	typedef char            NXINT8;
	typedef char            NXCHAR;
	typedef short           NXINT16;
	typedef int             NXINT32;
	typedef unsigned char   NXUINT8;
	typedef unsigned short  NXUINT16;
	typedef unsigned int    NXUINT32;
	typedef int             NXBOOL;
	typedef unsigned short  NXWCHAR;
	typedef float           NXFLOAT;
	typedef double          NXDOUBLE;
	typedef long double		NXLDOUBLE;
	typedef void            NXVOID;

	typedef size_t			NXUSIZE; // bit unsigned size depends on platform
	typedef ptrdiff_t		NXSSIZE; // bit signed size depends on platform
	typedef ptrdiff_t		NXPTRDIFF; // bit size depends on platform	

	typedef long long       NXINT64;
	typedef unsigned long long  NXUINT64;

#ifdef NEX_UNICODE
	typedef NXWCHAR     NXTCHAR;
#else
	typedef char        NXTCHAR;
#endif

#endif //_NEXTYPE_DEFINITION_INCLUDED_

