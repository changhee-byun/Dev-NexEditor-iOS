/******************************************************************************
* File Name   :	NEXVIDEOEDITOR_SupportDevices.cpp
* Description :	
*******************************************************************************

Nextreaming Confidential Proprietary
Copyright (C) 2006 Nextreaming Corporation
All rights are reserved by Nextreaming Corporation

Revision History:
Author	Date		Description of Changes
-------------------------------------------------------------------------------
cskb		2013/05/07	Draft.
-----------------------------------------------------------------------------*/

#include "NEXVIDEOEDITOR_SupportDevices.h"
// #include <cutils/properties.h>
// #include <sys/system_properties.h>
// #include <stdio.h>
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PROP_VALUE_MAX	64
#else
#include <sys/system_properties.h>
#include <string.h>
#endif

#if 0

static SUPPORT_DEVICE_INFO g_SupportDeviceList[] = {
	{0x541147a3/*Model Code*/, 	  800/*Max Width*/,  480/*Max Height*/,   66/*H264Profile*/, 0/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// tcl_s380(unknown, msm8225)
	{0xbe2377c1/*Model Code*/,	  800/*Max Width*/,  480/*Max Height*/,   66/*H264Profile*/, 0/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// ALCATEL ONE TOUCH 8000A(unknown, msm8225)
	{0xbe2377c5/*Model Code*/,	  800/*Max Width*/,  480/*Max Height*/,   66/*H264Profile*/, 0/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// ALCATEL ONE TOUCH 8000E(unknown, msm8225)
	{0xbe2377d8/*Model Code*/, 	  800/*Max Width*/,  480/*Max Height*/,   66/*H264Profile*/, 0/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// ALCATEL ONE TOUCH 8000X(unknown, msm8225)
	{0xbe2377c4/*Model Code*/, 	  800/*Max Width*/,  480/*Max Height*/,   66/*H264Profile*/, 0/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// ALCATEL ONE TOUCH 8000D(unknown, msm8225)
	{0x509c351f/*Model Code*/, 	  800/*Max Width*/,  480/*Max Height*/,   66/*H264Profile*/, 0/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// TCL Y710(unknown, msm8225)
	{0xb24ad1fd/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// LG-F180S (Optimus G, APQ8064)
	{0xa1e1d0a3/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// msm8960 (unknown, APQ8064)
	{0xe2978a22/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// SCH-I535 (Galaxy S3 for verizon, APQ8064)
	{0xb7c95535/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// IM-A830S (Vega Racer2 for SKT, APQ8064)
	{0xed27871c/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// IM-A850 (Vega R3, APQ8064)
	{0xb7c95caf/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// IM-A850K (Vega R3, APQ8064)
	{0xa1e1d0c6/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// msm8974 (unknown, msm8974)
	{0xbe2378bc/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*HWDecWhileExporting*/, 1/*Use SMS*/},	// ALCATEL ONE TOUCH 8008D (unknown, MT6589)
	{0xbe2378cf/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*HWDecWhileExporting*/, 1/*Use SMS*/},	// ALCATEL ONE TOUCH 8008W (unknown, MT6589)
	{0xbe2378d0/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*HWDecWhileExporting*/, 1/*Use SMS*/},	// ALCATEL ONE TOUCH 8008X (unknown, MT6589)
	{0xbe29a6b5/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*HWDecWhileExporting*/, 1/*Use SMS*/},	// V370 (unknown, MT6589)
	{0xbe285060/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*HWDecWhileExporting*/, 1/*Use SMS*/},	// S510 (unknown, MT6589)
	{0xbe28507f/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// S520 (unknown, MSM8974)
	{0xd6652b74/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// SH931W (unknown, APQ8064)
	{0x06604873/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// IN810 (unknown, APQ8064)
	{0x4915497d/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// FIH-VKY (unknown, APQ8064)
};

static SUPPORT_DEVICE_INFO g_SupportDeviceList[] = {
	{0xbe29a6b5/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*HWDecWhileExporting*/, 1/*Use SMS*/},	// V370 (unknown, MT6589) ACER
	{0xbe285060/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*HWDecWhileExporting*/, 1/*Use SMS*/},	// S510 (unknown, MT6589) ACER
	{0xbe28507f/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// S520 (unknown, MSM8974) ACER
	{0xd6652b74/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// SH931W (unknown, APQ8064) FIH
	{0x06604873/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// IN810 (unknown, APQ8064) FIH
	{0x071862be/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*HWDecWhileExporting*/, 0/*Use SMS*/},	// VP810 (unknown, APQ8064) FIH
};

static SUPPORT_DEVICE_INFO g_SupportDeviceList[] = {
	{0xbe237f5a/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ALCATEL ONE TOUCH 8020X (unknown, MT6589)
	{0xbe237f46/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ALCATEL ONE TOUCH 8020D (unknown, MT6589)
	{0xbe237f43/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ALCATEL ONE TOUCH 8020A (unknown, MT6589)
	{0xbe237f47/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ALCATEL ONE TOUCH 8020E (unknown, MT6589)
	{0x506486d1/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ONE TOUCH 8020A (unknown, MT6589)
	{0x506486d4/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ONE TOUCH 8020D (unknown, MT6589)
	{0x506486d5/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ONE TOUCH 8020E (unknown, MT6589)
	{0x506486e8/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ONE TOUCH 8020X (unknown, MT6589)
	{0x506486e9/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// ONE TOUCH 8020Y (unknown, MT6589)
	{0x509c3ca1/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// TCL Y910 (unknown, MT6589)
	{0xc2eb57d3/*Model Code*/, 	1920/*Max Width*/, 1280/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 1/*Use SMS Mode*/},	// TCL Y910T (unknown, MT6589)
};
	
#endif

static SUPPORT_DEVICE_INFO g_SupportDeviceList[] = {
	{0xfe8642c/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// HUAWEI G750-T00 (unknown, MT6592)
	{0x68f094f1/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// G750-T00 (unknown, MT6592)
	
	{0xbe25a169/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M810 (unknown, MSM8974) FIH
	{0x68e8c2b/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M810t (unknown, MSM8974) FIH
	{0x92cc67d6/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M810 (unknown, MSM8974) FIH
	{0xc6c0935e/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M810t (unknown, MSM8974) FIH
	{0xc6c0935f/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M810u (unknown, MSM8974) FIH

	{0xbe259628/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M512 (unknown, MSM8926) FIH
	{0xbee64bae/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Smart503 (unknown, MSM8926) FIH
	{0xc6bf3641/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M510t (unknown, MSM8926) FIH
	
	{0x92cc5c93/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M510 (unknown, MSM8926) FIH
	{0xbe259626/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M510 (unknown, MSM8926) FIH

	{0xbe259627/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M511 (unknown, MSM8926) FIH
	{0x92cc5c94/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M511 (unknown, MSM8926) FIH
	{0xfb176371/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M2 (unknown, MSM8926) FIH
	{0x900aedd/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Amazing X3 (unknown, MSM8926) FIH
	{0xc087e5c4/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M2 (unknown, MSM8926) FIH
	{0x110bc88b/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M2PLUS (unknown, MSM8926) FIH
	
	{0xc6be5145/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M320u (unknown, MT6592) FIH
	{0xc6be513d/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M320m (unknown, MT6592) FIH
	{0x92cc5150/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// InFocus M210 (unknown, MT6582) FIH
	
	{0x68c4a12/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M320u (unknown, MT6592) FIH
	{0x68c4a0a/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M320m (unknown, MT6592) FIH
	{0xbe258ae3/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// M210 (unknown, MT6582) FIH

	{0x6604873/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// IN810 (unknown, APQ8064) FIH
	{0x6604878/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// IN815 (unknown, APQ8064) FIH
	
	{0x66035ec/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// IN330 (unknown, MSM8930) FIH
	{0x66035ed/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// IN331 (unknown, MSM8930) FIH
	{0x66035f1/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// IN335 (unknown, MSM8930) FIH


	{0xbe2aa529/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// X909 (unknown, APQ8064) OPPO
	{0x72a004b/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// X909T (unknown, APQ8064) OPPO
	{0x5074d6b2/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// N1T (unknown, APQ8064) OPPO
	{0x5074d6b5/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// N1W (unknown, APQ8064) OPPO
	{0x729ff17/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// X9007 (unknown, MSM8974) OPPO


	{0x826fbcbe/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 9970L (unknown, MSM8974) Coolpad
	{0x826c2148/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 9190L (unknown, MSM8926) Coolpad
	{0xef0f02f2/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad T1 (unknown, MSM8926) Coolpad
	{0x674d6dcb/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8680 (unknown, MT6595) Coolpad
	
	{0x674d6dcb/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8680 (unknown, MT6295) Coolpad
	{0x675afce6/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad V1-C (unknown, MT6295) Coolpad

	{0x674d6dea/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8690 (unknown, MT6595) Coolpad
	
	{0x83dca0db/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad S6-NT (unknown, MSM8939) Coolpad
	{0x2a2a367/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8675-C00 (unknown, MSM8939) Coolpad
	{0xc9a8cb25/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8675-A (unknown, MSM8939) Coolpad
	{0x2a2b1a6/*Model Code*/, 	1920/*Max Width*/, 1288/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8675-FHD (unknown, MSM8939) Coolpad

	{0x826fbd70/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 9976D (unknown, MT6592) Coolpad
	{0x2a2e339/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8675-T01 (unknown, MT6592) Coolpad

	{0x212acc88/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8297N-T00 (unknown, MT6592m) Coolpad
	{0x212ad7cb/*Model Code*/, 	1920/*Max Width*/, 1088/*Max Height*/, 100/*H264Profile*/, 2/*Dec Cnt*/, 1/*Enc Cnt*/, 0/*Use SMS Mode*/},	// Coolpad 8297N-W00 (unknown, MT6592m) Coolpad
	
};
	
#define  LOG_TAG    "NEXEDITOR"
#include <android/log.h>
#ifndef LOGE
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#endif

#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
#include <android/log.h>
#include <dlfcn.h>

typedef int (*PFN_SYSTEM_PROP_GET)(const char *, char *);
static int __nex_system_property_get(const char* name, char* value)
{
    static PFN_SYSTEM_PROP_GET __real_system_property_get = NULL;
    if (!__real_system_property_get) {
        // libc.so should already be open, get a handle to it.
        void *handle = dlopen("libc.so", RTLD_NOLOAD);
        if (!handle) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot dlopen libc.so: %s.\n", dlerror());
        } else {
            __real_system_property_get = (PFN_SYSTEM_PROP_GET)dlsym(handle, "__system_property_get");
        }
        if (!__real_system_property_get) {
            __android_log_print(ANDROID_LOG_ERROR, "foobar", "Cannot resolve __system_property_get(): %s.\n", dlerror());
        }
    }
    return (*__real_system_property_get)(name, value);
} 
#endif

int android_property_get(const char *key, char *value, const char *default_value)
{
#if defined(__LP64__) || defined(__aarch64__) || defined(__ia64__)
    int iReturn = __nex_system_property_get(key, value);
    if (!iReturn) strcpy(value, default_value);
    return iReturn;

#else
    int iReturn = __system_property_get(key, value);
    if (!iReturn) strcpy(value, default_value);
    return iReturn;
#endif
}

SUPPORT_DEVICE_INFO* getSupportDeviceInfo(char* pModelName)
{
	if( pModelName == NULL ) return NULL;

	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.product.model", strModel, "");

	if( strcmp(pModelName, strModel) != 0 ) return NULL;
	
	int ihKey = 16769023;
	char* pTmp = pModelName;
	while( *pTmp ) 
	{
		ihKey = 31 * ihKey + *pTmp;
		pTmp++;
	}
	
	// LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] CreateNexVideoEditor Model(%s) 0x%8x ", __LINE__, strModel, ihKey);
	
	for( int i = 0; i < sizeof(g_SupportDeviceList)/sizeof(*g_SupportDeviceList); i++)
	{
		if( ihKey == g_SupportDeviceList[i].m_uiModelCode )
			return &g_SupportDeviceList[i];
	}
	return NULL;
}

int getRGBADeviceProperty()
{
	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.hardware", strModel, "");
	if( strcmp(strModel, "mt6589") == 0 || strcmp(strModel, "mt6582") == 0 )
	{
		return 0;
	}
	return 1;
}

int getCheckChipsetType()
{
	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.board.platform", strModel, "");
	if( strcmp(strModel, "msm8960") == 0 || strcmp(strModel, "MSM8960") == 0 )
		return CHIPSET_MSM8960;
	else if( strcmp(strModel, "msm8064") == 0 || strcmp(strModel, "MSM8064") == 0)
	{
		return CHIPSET_MSM8064;
	}
	else if( strcmp(strModel, "msm8974") == 0 )
	{
		return CHIPSET_MSM8974;
	}
	else if( strcmp(strModel, "msm8226") == 0 || strcmp(strModel, "MSM8226") == 0)
	{
		return CHIPSET_MSM8226;
	}
	else if( strcmp(strModel, "msm8926") == 0 || strcmp(strModel, "MSM8926") == 0)
	{
		return CHIPSET_MSM8926;
	}
	else if( strcmp(strModel, "exynos4") == 0 )
	{
		return CHIPSET_EXYNOS4412;
	}
	else if( strcmp(strModel, "exynos5") == 0 )
	{
		return CHIPSET_EXYNOS5410;
	}
	return CHIPSET_UNKNOWN;
}

int getSupportSystemProperty(const char* pStrName, char* pValue)
{
	if( pStrName == NULL || pValue == NULL )
		return 0;
	android_property_get(pStrName, pValue, "");
	return 1;
}

int isLGEG2Device()
{
	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.hardware", strModel, "");
	if( strcmp(strModel, "g2") == 0 )
	{
		android_property_get("ro.board.platform", strModel, "");
		if( strcmp(strModel, "msm8974") == 0 )
		{
			return 1;
		}
	}
	return 0;
}

int isMT6592Device()
{
	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.hardware", strModel, "");
	if( strcmp(strModel, "mt6592") == 0 )
	{
		return 1;
	}
	return 0;
}

int isMSM8974Device()
{
	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.board.platform", strModel, "");
	if( strcmp(strModel, "msm8974") == 0 || strcmp(strModel, "MSM8974") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}

int isGalaxyNote3()
{
	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.product.model", strModel, "");

	if( strlen(strModel) < 8 )
		return FALSE;

	strModel[7] = '\0';

	// LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] isGalaxyNote3 Name(%s)", __LINE__, strModel);
	if( strcmp(strModel, "SM-N900") == 0 || strcmp(strModel, "sm-n900") == 0 )
		return TRUE;
	return FALSE;
}

int isXperiaM4Aqua()
{
	char strModel[PROP_VALUE_MAX];
	int iLength = 0;    
	strcpy(strModel, "");

	android_property_get("ro.product.model", strModel, "");

	if( strlen(strModel) < 5 )
		return FALSE;

	strModel[5] = '\0';

	//LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] isXperiaM4Aqua Name(%s)", __LINE__, strModel);
	if( strcmp(strModel, "E2363") == 0 || strcmp(strModel, "e2363") == 0)
		return TRUE;
	return FALSE;
}

int isGalaxyA7()
{
	char strModel[PROP_VALUE_MAX];
	int iLength = 0;    
	strcpy(strModel, "");

	android_property_get("ro.product.model", strModel, "");

	if( strlen(strModel) < 8 )
		return FALSE;

	strModel[8] = '\0';

	//LOGE("[NEXVIDEOEDITOR_VideoEditor.cpp %d] isGalaxyA7 Name(%s)", __LINE__, strModel);
	if( strcmp(strModel, "SM-A700") == 0 || strcmp(strModel, "sm-a700") == 0)
		return TRUE;
	return FALSE;
}

int isXperiaM4AquaOrGalaxyA7()
{
	return isXperiaM4Aqua() || isGalaxyA7();
}

int isSamsungMSM8974Device()
{
	char strModel[PROP_VALUE_MAX];
	strcpy(strModel, "");

	android_property_get("ro.product.brand", strModel, "");
	if( strcmp(strModel, "samsung") == 0  )
	{
		android_property_get("ro.chipname", strModel, "");
		if( strcmp(strModel, "MSM8974") == 0  || strcmp(strModel, "msm8974") == 0 )
		{
			return TRUE;
		}

		android_property_get("ro.board.platform", strModel, "");
		if( strcmp(strModel, "MSM8974") == 0  || strcmp(strModel, "msm8974") == 0 )
		{
			return TRUE;
		}
		
	}
	return FALSE;
}

int isQualcommChip()
{
	char strPlatform[PROP_VALUE_MAX];
	strcpy(strPlatform, "");

	android_property_get("ro.board.platform", strPlatform, "");
	strPlatform[3] = '\0';

	if( strcmp(strPlatform, "MSM") == 0  || strcmp(strPlatform, "msm") == 0  || strcmp(strPlatform, "SDM") == 0  || strcmp(strPlatform, "sdm") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}
		
int isMSM8x26Device()
{
	int iChipType = getCheckChipsetType();
	if( iChipType == CHIPSET_MSM8226 || iChipType == CHIPSET_MSM8926) 
		return TRUE;
	return FALSE;
}

int isNexus10Device()
{
	char strPlatform[PROP_VALUE_MAX];
	strcpy(strPlatform, "");

	android_property_get("ro.product.device", strPlatform, "");
	
	if( strcmp(strPlatform, "manta") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}

int isx86Device()
{
	char strPlatform[PROP_VALUE_MAX];
	strcpy(strPlatform, "");

	android_property_get("ro.product.cpu.abi", strPlatform, "");
	
	if( strcmp(strPlatform, "x86") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}

int isHuaweiHi3635Device()
{
	char strPlatform[PROP_VALUE_MAX];
	strcpy(strPlatform, "");

	android_property_get("ro.board.platform", strPlatform, "");
	
	if( strcmp(strPlatform, "hi3635") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}

int isHuaweiHi3630Device()
{
	char strPlatform[PROP_VALUE_MAX];
	strcpy(strPlatform, "");

	android_property_get("ro.board.platform", strPlatform, "");
	
	if( strcmp(strPlatform, "hi3630") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}

int isSDM660Device(){

	char strPlatform[PROP_VALUE_MAX];
	strcpy(strPlatform, "");

	android_property_get("ro.board.platform", strPlatform, "");
	
	if( strcasecmp(strPlatform, "sdm660") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}

int isXiaomiMI5C(){

	char strPlatform[PROP_VALUE_MAX];
	strcpy(strPlatform, "");

	android_property_get("ro.board.platform", strPlatform, "");
	
	if( strcasecmp(strPlatform, "song") == 0 )
	{
		return TRUE;
	}
	return FALSE;
}