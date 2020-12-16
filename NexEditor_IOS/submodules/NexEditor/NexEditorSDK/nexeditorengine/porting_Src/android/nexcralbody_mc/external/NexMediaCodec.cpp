/******************************************************************************
* File Name        : NexMediaCodec.cpp
* Description      : NexMediaCodec implementation for NexCRALBody_MC
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
#define LOCATION_TAG "NexMediaCodec"

#include "./NexMediaCodec.h"
#include "./NexSurfaceTexture.h"

// for differentiating between encoder and decoder in logs (U == undefined)
//#define NEX_MC_TYPE ('U' - 'D')
#define NEX_MC_TYPE 0

namespace Nex_MC {

NexMediaCodec::NexMediaCodec()
{
}

NexMediaCodec::~NexMediaCodec()
{
}

/* static */ int NexMediaCodec::getSurfaceObjectFromSurfaceTexture(NexSurfaceTexture *surface, jobject &result)
{
	if (NULL == surface)
	{
		result = NULL;
	}
	else
	{
		return surface->getSurfaceObject(result);
	}
	return 0;
}

}; // namespace Nex_MC
