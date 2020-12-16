/******************************************************************************
* File Name        : debug.cpp
* Description      : debug utilities for NexCRALBody_MC
*******************************************************************************
*
*     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
*     PURPOSE.
*
*	Nexstreaming Confidential Proprietary
*	Copyright (C) 2006~2013 Nexstreaming Corporation
*	All rights are reserved by Nexstreaming Corporation
*
******************************************************************************/
#define LOCATION_TAG "Debug"

#define LOG_TAG "nexcral_mc"
#include "./debug.h"
#include "./utils.h"

namespace Nex_MC {
namespace Log {
	int gDebugLevel[NUM_TYPES][2][32] = { { {0, }, {0, } }
										, { {0, }, {0, } }
										, { {0, }, {0, } }
#if defined(NEX_TEST)
										, { {0, }, {0, } }
#endif
										};
	
	const char *LOGTYPE_NAME[] =
		{ "FLW"
		, "OUT"
		, "INP"
		, "LOP"
		, "EXT"
		, "MTX"
		};

	void SetDebugLevel( int type, int avtype, int logTypes, int maxLevel )
	{
		for ( unsigned int i = 0; i < sizeof(gDebugLevel[0][0])/sizeof(gDebugLevel[0][0][0]); ++i )
		{
			if (IS_BINARY_FLAG_SET(logTypes, 1<<i))
				gDebugLevel[type][avtype][i] = maxLevel;
		}
	}
}; // namespace Log
}; // namespace Nex_MC

#if defined(NEX_DEBUG_MODE)
namespace {
	class InitDebugLevels
	{
	public:
		InitDebugLevels()
		{
			using Nex_MC::Log::SetDebugLevel;
			SetDebugLevel(0 /*NEX_MC_TYPE_DECODER*/, 0, 0xFFFFFFFF, NEX_DEBUG_D_MODE);
			SetDebugLevel(0 /*NEX_MC_TYPE_DECODER*/, 1, 0xFFFFFFFF, NEX_DEBUG_D_MODE);
			SetDebugLevel(1 /*NEX_MC_TYPE_ENCODER*/, 0, 0xFFFFFFFF, NEX_DEBUG_E_MODE);
			SetDebugLevel(1 /*NEX_MC_TYPE_ENCODER*/, 1, 0xFFFFFFFF, NEX_DEBUG_E_MODE);
			SetDebugLevel(2 /*NEX_MC_TYPE_RENDERER*/, 0, 0xFFFFFFFF, NEX_DEBUG_R_MODE);
			SetDebugLevel(2 /*NEX_MC_TYPE_RENDERER*/, 1, 0xFFFFFFFF, NEX_DEBUG_R_MODE);
#if defined(NEX_TEST)
			SetDebugLevel(3 /*NEX_MC_TYPE_TEST*/, 0, 0xFFFFFFFF, NEX_DEBUG_T_MODE);
			SetDebugLevel(3 /*NEX_MC_TYPE_TEST*/, 1, 0xFFFFFFFF, NEX_DEBUG_T_MODE);
#endif
		}
		~InitDebugLevels() {}
	};

	InitDebugLevels initDebugLevels;
};
#endif //#if defined(NEX_DEBUG_MODE)

