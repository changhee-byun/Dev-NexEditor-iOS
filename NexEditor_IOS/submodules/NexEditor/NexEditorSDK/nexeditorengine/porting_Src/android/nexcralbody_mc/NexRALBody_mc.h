/******************************************************************************
* File Name        : NexRALBody_mc.h
* Description      : Generalized hardware video renderer for Android
* Revision History : Located at the bottom of this file
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

#ifndef _NEXRAL_MC_H_
#define _NEXRAL_MC_H_

#define NEXRAL_MC_API_VERSION 2

#include "NexRAL.h"

#define NRMC_USERDATA_TYPE 'NRMC'
#define NSMC_USERDATA_TYPE 'NSMC'

typedef struct _VIDEO_RALBODY_FUNCTION_ *VIDEO_RALBODY_FUNCTION_ST_PTR;

#ifdef __cplusplus
extern "C" {
#endif

/** getNexRAL_MC_Version
 * returns:  unsigned integer built by following formula:
 *           (((MAJOR)*100 + MINOR)*100 + PATCH)*100 + BUILD_NUMBER
 *           where each part has a valid range of [0,99].
 * see also: version.h.
 * note:     version.h's NEXCRALBODY_MC_VERSION_BUILD is an ascii character
 *            (i.e. 0 => '0' => 48; F => 'F' => 70)
 */
unsigned int getNexRAL_MC_Version();

struct _JavaVM;
typedef _JavaVM JavaVM;
void SetJavaVMforRender(JavaVM* javaVM);

/** getNexRAL_MC_VideoRenderer
 * input:    uBufferFormat -- the specified output buffer format
 * returns:  ptr to a VIDEO_RALBODY_FUNCTION_ST struct for the specified buffer
 *            format suitable for use with NexALFactory::registerVideoRendererHW.
 *           NULL if the specified buffer format is not supported.
 * see also: a list of possible buffer formats are declared in nexCAL.h.
 * note:     you _should_not_ call this function with more than one argument.
 */
VIDEO_RALBODY_FUNCTION_ST_PTR getNexRAL_MC_VideoRenderer(
	  unsigned int uBufferFormat
	, unsigned int API_Version = NEXRAL_MC_API_VERSION
	, unsigned int nexRAL_API_MAJOR = NEXRAL_VERSION_MAJOR
	, unsigned int nexRAL_API_MINOR = NEXRAL_VERSION_MINOR
	, unsigned int nexRAL_API_PATCH_NUM = NEXRAL_VERSION_PATCH
	);

/** getNexRAL_MC_SurfaceVideoRenderer
 * input:    uBufferFormat -- the specified output buffer format
 * returns:  ptr to a VIDEO_RALBODY_FUNCTION_ST struct for the specified buffer
 *            format suitable for use with NexALFactory::registerVideoRendererHW.
 *           NULL if the specified buffer format is not supported.
 * see also: a list of possible buffer formats are declared in nexCAL.h.
 * note:     you _should_not_ call this function with more than one argument.
 */
VIDEO_RALBODY_FUNCTION_ST_PTR getNexRAL_MC_SurfaceVideoRenderer(
	  unsigned int uBufferFormat
	, unsigned int API_Version = NEXRAL_MC_API_VERSION
	, unsigned int nexRAL_API_MAJOR = NEXRAL_VERSION_MAJOR
	, unsigned int nexRAL_API_MINOR = NEXRAL_VERSION_MINOR
	, unsigned int nexRAL_API_PATCH_NUM = NEXRAL_VERSION_PATCH
	);

#ifdef __cplusplus
}
#endif

#endif	//#ifndef _NEXRAL_MC_H_

/*=============================================================================
                                Revision History
===============================================================================
    Author         Date          API Version     nexRAL API
===============================================================================
    benson         2013/04/29    1               2.0.0
-------------------------------------------------------------------------------
 Initial version
===============================================================================
    benson         2013/08/05    2               2.0.0
-------------------------------------------------------------------------------
 Added getNexRAL_MC_Version
=============================================================================*/

