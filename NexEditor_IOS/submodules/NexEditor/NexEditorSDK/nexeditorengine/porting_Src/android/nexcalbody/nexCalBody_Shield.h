#ifndef _CALBODY_SHIELD_
#define _CALBODY_SHIELD_


#include "nexplayer_porting_calbody.h"

#ifdef __cplusplus
extern "C" {
#endif


#if defined(_TARGET_CORETEXA8_)
#define NEXCALBODY_CPU_INFORMATION "CORETEX A8"
#elif defined(_TARGET_ARMV6)
#define NEXCALBODY_CPU_INFORMATION "ARMV6"
#elif defined(_TARGET_ARMV5)
#define NEXCALBODY_CPU_INFORMATION "ARMV5"
#else
#define NEXCALBODY_CPU_INFORMATION "ARMV5"
#endif


//namespace android {

unsigned int nexCALBody_Register(NXCALRegisterCodec ftRegister, int nLogLevel);

//};

#ifdef __cplusplus
}
#endif
#endif	// _CALBODY_SHIELD_

