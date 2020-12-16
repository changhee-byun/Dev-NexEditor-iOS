/******************************************************************************
* File Name        : NexCALBody_mc.h
* Description      : Generalized hardware codec for Android
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

#ifndef _NEXCAL_MC_H_
#define _NEXCAL_MC_H_

#define NEXCAL_MC_API_VERSION 2

#include "NexCAL.h"

/** NCNW_USERDATA_TYPE
 * On Init, supplying this value as uUserDataType and supplying a native window
 *  pointer as puUserData will result in the decoder using the native window as
 *  the output destination.
 */
#define NCNW_USERDATA_TYPE 'NCNW'		//Nex Cal Native Window

#ifdef __cplusplus
extern "C" {
#endif

/** getNexCAL_MC_Version
 * returns:  unsigned integer built by following formula:
 *           (((MAJOR)*100 + MINOR)*100 + PATCH)*100 + BUILD_NUMBER
 *           where each part has a valid range of [0,99].
 * see also: version.h.
 * note:     version.h's NEXCRALBODY_MC_VERSION_BUILD is an ascii character
 *            (i.e. 0 => '0' => 48; F => 'F' => 70)
 */
unsigned int getNexCAL_MC_Version();

/** NEXCALCodec2
 * a scrambled set of functions meant to be descrambled before use
 */
typedef struct NEXCALCodec2
{
	void *func[13];
} NEXCALCodec2;

struct _JavaVM;
typedef _JavaVM JavaVM;
void SetJavaVMforCodec(JavaVM* javaVM);

/** getNexCAL_MC_Decoder
 * input:    uCodecObjectTypeIndication -- the specified codec
 * returns:  ptr to a NEXCALCodec2 struct for the specified codec type.
 *           NULL if the target hardware doesn't support the specified codec.
 * see also: a list of possible OTIs declared in nexOTIs.h.
 * note:     you _should_not_ call this function with more than one argument.
 */
NEXCALCodec2 *getNexCAL_MC_Decoder(
	  NEX_CODEC_TYPE eCodecType
	, unsigned int API_Version = NEXCAL_MC_API_VERSION
	, unsigned int nexCAL_API_MAJOR = NEXCAL_VERSION_MAJOR
	, unsigned int nexCAL_API_MINOR = NEXCAL_VERSION_MINOR
	, unsigned int nexCAL_API_PATCH_NUM = NEXCAL_VERSION_PATCH
	);

NEXCALCodec2 *GetHandle(
	);


/** getNexCAL_MC_Encoder
 * input:    uCodecObjectTypeIndication -- the specified codec
 * returns:  ptr to a NEXCALCodec2 struct for the specified codec type.
 *           NULL if the target hardware doesn't support the specified codec.
 * see also: a list of possible OTIs declared in nexOTIs.h.
 * note:     you _should_not_ call this function with more than one argument.
 */

#define eNEX_CODEC_V_H264_MC_S			 				0x50010300
#define eNEX_CODEC_V_H264_MC_ENCODER_USING_FRAMEDATA	0x50010301

NEXCALCodec2 *getNexCAL_MC_Encoder(
	  NEX_CODEC_TYPE eCodecType
	, unsigned int API_Version = NEXCAL_MC_API_VERSION
	, unsigned int nexCAL_API_MAJOR = NEXCAL_VERSION_MAJOR
	, unsigned int nexCAL_API_MINOR = NEXCAL_VERSION_MINOR
	, unsigned int nexCAL_API_PATCH_NUM = NEXCAL_VERSION_PATCH
	);

/** NexCAL_MC_De/Encoder_setDebugLevel
 * input:    types -- a bitmask representing the types of logs to change
 *                    0xFFFFFFFF to change all logs
 *             valid values: (any combination of)
 *               FLOW:          NTHFROMLSBMASK(0)
 *               OUTPUT:        NTHFROMLSBMASK(1)
 *               INPUT:         NTHFROMLSBMASK(2)
 *               LOOP:          NTHFROMLSBMASK(3)
 *               EXTERNAL_CALL: NTHFROMLSBMASK(4)
 *               MUTEX:         NTHFROMLSBMASK(5)
 *           max_level -- the maximum level of debug logs to be displayed
 *                        0 for no log
 * note:     default max level is 0.
 *           included in functions returned by getNexCAL_MC_De/Encoder.
 */
//void NexCAL_MC_De/Encoder_setDebugLevel( int types, int max_level );

#ifdef __cplusplus
}
#endif

#endif	//#ifndef _NEXCAL_MC_H_

/*=============================================================================
                                Revision History
===============================================================================
    Author         Date          API Version     nexCAL API
===============================================================================
    benson         2013/04/29    1               3.2.A
-------------------------------------------------------------------------------
 Initial version
===============================================================================
    benson         2013/08/05    1               3.3.5
-------------------------------------------------------------------------------
 Added getNexCAL_MC_Version
=============================================================================*/

