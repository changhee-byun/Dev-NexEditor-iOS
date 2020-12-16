/******************************************************************************
* File Name   :	NexFFErrorDef.h
* Description :	Error type definition header for file parser
*******************************************************************************

	 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
	 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
	 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
	 PURPOSE.

NexStreaming Confidential Proprietary
Copyright (C) 2014 NexStreaming Corporation
All rights are reserved by NexStreaming Corporation
******************************************************************************/

#ifndef _NXFFERRORDEF_H_INCLUDED_
#define _NXFFERRORDEF_H_INCLUDED_

#include "NexCommonDef.h"

#define NEXERROR_VERSION_MAJOR	1
#define NEXERROR_VERSION_MINOR	0
#define NEXERROR_VERSION_PATCH	0
#define NEXERROR_VERSION_BRANCH "OFFICIAL"

typedef enum {
	/*
	// eNEX_FF_RET_(return type)
	// eNEX_FF_RET_(function dependence)_(return type)
	*/
	eNEX_FF_RET_SUCCESS = 0,
	eNEX_FF_RET_FAIL,
	eNEX_FF_RET_UNSUPPORT,							// Do not support for the function.
	eNEX_FF_RET_UNSUPPORT_FORMAT,					// Do not support for File format
	eNEX_FF_RET_FILE_API_ERROR,						// Fil API error
	eNEX_FF_RET_FILE_IO_ERROR,
	eNEX_FF_RET_FILE_OPEN_ERROR,
	eNEX_FF_RET_FILE_CLOSE_ERROR,
	eNEX_FF_RET_FILE_WRITE_ERROR,
	eNEX_FF_RET_FILE_READ_ERROR,
	eNEX_FF_RET_FILE_SEEK_ERROR,
	eNEX_FF_RET_NOT_YET,
	eNEX_FF_RET_EOS,								// End of Stream
	eNEX_FF_RET_EOF,								// End of File
	eNEX_FF_RET_MEMORY_ERROR,
	eNEX_FF_RET_MEMORY_ALLOC_ERROR,
	eNEX_FF_RET_INVALID_SYNTAX,						// A syntax problem of content, Not File I/O issue.
	eNEX_FF_RET_INVALID_PARAM,						// Input parameter problem for the function.
	eNEX_FF_RET_HAS_NO_EFFECT,						// 
	eNEX_FF_RET_POLICY_DECISION,					// 

	// The return value is in accordance with the function characteristic
	eNEX_FF_RET_READFRAME_TRYAGAIN					= 0x10000000,
	eNEX_FF_RET_READFRAME_SIZE_TOO_BIG,
	eNEX_FF_RET_READFRAME_INVALID_TS,
	eNEX_FF_RET_READFRAME_HAS_MORE_SAME_TIME,
	eNEX_FF_RET_READFRAME_SUBSAMPLE_SIZE_TOO_BIG,

	// NxFFSubtitle return type
	eNEX_FF_RET_UNSUPPORT_ENCODING					= 0x20000000,                // Do not support for the function.
	eNEX_FF_RET_PARSING_CONTINUE,							// Do not critical error. parsing continue.

	eNEX_FF_RET_PADDING 			= MAX_SIGNED32BIT

} NEX_FF_RETURN;

#endif // _NXFFERRORDEF_H_INCLUDED_