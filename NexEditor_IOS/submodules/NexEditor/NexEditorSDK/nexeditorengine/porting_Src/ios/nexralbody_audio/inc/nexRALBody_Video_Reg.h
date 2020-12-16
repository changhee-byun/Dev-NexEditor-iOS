#ifndef __NEXRALBODY_VIDEO_REG_H__
#define __NEXRALBODY_VIDEO_REG_H__

#include "nexRALBody_Common_API.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief This function registers the video renderer to be used by the NexPlayer&trade;&nbsp;engine.
 *
 * \param pFunctions	A function pointer to the video renderer being registered.
 *
 * \returns Always 0.
 */
unsigned int nexRALBody_Video_Get_RALBody(void* pFunctions);

#ifdef __cplusplus
}
#endif

#endif //__NEXRALBODY_VIDEO_REG_H__
