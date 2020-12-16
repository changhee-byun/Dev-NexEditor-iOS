/******************************************************************************
* File Name   : NexCommonDef.h
* Description : Common definition.
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2013-2014 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NEXCOMMONDEF_H_INCLUDED_
#define _NEXCOMMONDEF_H_INCLUDED_

#include "NexTypeDef.h"

#define NEXCOMDEF_VERSION_MAJOR		1
#define NEXCOMDEF_VERSION_MINOR		0
#define NEXCOMDEF_VERSION_PATCH		3
#define NEXCOMDEF_VERSION_BRANCH	"OFFICIAL"

#if defined(__GNUC__)
#define PACKED(struct_to_pack) struct_to_pack __attribute__((__packed__))
#elif defined(__arm)
#define PACKED(struct_to_pack) struct_to_pack __packed
#elif defined(_MSC_VER)
#define PACKED(struct_to_pack) __pragma(pack(push, 1)) struct_to_pack __pragma(pack(pop))
#else
#define PACKED(struct_to_pack) struct_to_pack
#endif


#define IN
#define OUT
#define INOUT

#define _FUNLINE_ __FUNCTION__, __LINE__

#define SAFE_FREE(pMem) {if(pMem) {nexSAL_MemFree(pMem);} pMem=0;}

#ifndef MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#endif

#define W_ALIGNED(ptr) 		(!(((NXPTRDIFF)(ptr)) & 0x00000001))
#define ALIGN_ADDR(a, w)	(((NXPTRDIFF)(a)+(w)-1)&~((w)-1))

#define INVALID_VALUE (~0)

#endif //_NEXCOMMONDEF_H_INCLUDED_

