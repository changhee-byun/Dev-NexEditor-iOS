#ifndef __NEXRALBODY_AUDIO_REG_H__
#define __NEXRALBODY_AUDIO_REG_H__

#include "nexRALBody_Common_API.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \brief This function registers the audio renderer to be used by the NexPlayer&trade;&nbsp;engine.
 *
 * \param fnRegister	A function pointer to the audio renderer being registered.
 * \param nLogLevel		The level at which to produce logs for the Audio RAL (Rendering Adaptation Layer).
 *						This will be one of:
 *							  - <b>-1</b>: No logs are produced.
 *							  - <b>0</b> : Output basic log messages only (recommended).
 *							  - <b>1~5</b> : Output detailed log messages; higher numbers result in more verbose
 *                                      log entries, but may cause performance issues in some cases and are
 *                                      <b>not</b> recommended for general release code.
 *
 * \returns Always 0.
 */

unsigned int nexRALBody_Audio_Register(NXRALRegisterRenderer fnRegister, int nLogLevel);
unsigned int nexRALBody_Audio_Get_RALBody(void* pFunctions);
unsigned int nexRALBody_Audio_Close_RALBody(void* pFunctions);

#ifdef __cplusplus
}
#endif

#endif //__NEXRALBODY_AUDIO_REG_H__
