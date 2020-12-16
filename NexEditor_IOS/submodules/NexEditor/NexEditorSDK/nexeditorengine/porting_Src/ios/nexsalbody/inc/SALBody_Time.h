//
//  SALBody_Time.h
//  nexSalBody
//
//  Created by Simon Kim on 1/26/15.
//
//

#ifndef nexSalBody_SALBody_Time_h
#define nexSalBody_SALBody_Time_h
#include "nexTypeDef.h"

#ifdef __cplusplus
extern "C" {
#endif
	
	NXUINT32 nexSALBody_GetTickCount();
    NXVOID nexSALBody_GetMSecFromEpoch(NXUINT64 *a_puqMSec);
	
#ifdef __cplusplus
}
#endif
#endif
