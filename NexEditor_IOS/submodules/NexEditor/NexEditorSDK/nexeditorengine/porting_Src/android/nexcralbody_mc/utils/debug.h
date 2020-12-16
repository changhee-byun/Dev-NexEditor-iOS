/******************************************************************************
* File Name        : debug.h
* Description      : Debug definitions for NexCRALBody_MC
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
#ifndef _NEXCRAL_MC_DEBUG_H_
#define _NEXCRAL_MC_DEBUG_H_

#include "./time.h"

#include <unistd.h>
#include <android/log.h>

#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#ifndef LOCATION_TAG
#define LOCATION_TAG __FILE__
#endif

#if 1 <= NEX_DEBUG_MODE
#define LOCATION_GENERAL __FUNCTION__
#define LOCATION_DELIMITER "::"
#else
#define LOCATION_GENERAL ""
#define LOCATION_DELIMITER ""
#endif

#if defined(NEX_TEST)
#define NUM_TYPES 4
#else
#define NUM_TYPES 3
#endif

#if -1 != NEX_DEBUG_MODE
#define MC_PRINT(printer, logType, logLevel, format, args...)                                                  \
    do {                                                                                                       \
        char timeBuf[32];                                                                                      \
        ::Nex_MC::Utils::Time::GetPrettyLocalTime(timeBuf, sizeof(timeBuf), "%M:%S", true);                    \
        printer("[%s][T%d] %3.3s%d:[%s%s%s L%d] " format "\n"                                                  \
            , timeBuf                                                                                          \
            , gettid()                                                                                         \
            , logType, logLevel                                                                                \
            , LOCATION_TAG, LOCATION_DELIMITER, LOCATION_GENERAL, __LINE__                                     \
            , ##args);                                                                                         \
    } while (0)

#define MC_LOG(mCondition, printer, logType, logLevel, format, args...)                                        \
    do {                                                                                                       \
        if ((mCondition))                                                                                      \
        {                                                                                                      \
            MC_PRINT(printer, logType, logLevel, format , ##args);                                             \
        }                                                                                                      \
    } while(0)

#define MC_IDENT_PRINT(printer, prependstr, codecname, ud, format, args...)                                    \
    do {                                                                                                       \
        char timeBuf[32];                                                                                      \
        ::Nex_MC::Utils::Time::GetPrettyLocalTime(timeBuf, sizeof(timeBuf), "%M:%S", true);                    \
        printer("[%s][T%d] %s%s(%p):[%s%s%s L%d] " format "\n"                                                 \
            , timeBuf                                                                                          \
            , gettid()                                                                                         \
            , prependstr, codecname, (void *)ud                                                                \
            , LOCATION_TAG, LOCATION_DELIMITER, LOCATION_GENERAL, __LINE__                                     \
            , ##args);                                                                                         \
    } while (0)

#define MC_IDENTITY(mCondition, printer, prependstr, codecname, ud, format, args...)                           \
    do {                                                                                                       \
        if ((mCondition))                                                                                      \
        {                                                                                                      \
            MC_IDENT_PRINT(printer, prependstr, codecname, ud, format , ##args);                               \
        }                                                                                                      \
    } while(0)

#define MC_ERR(format, args...) MC_PRINT(LOGE, "ERR", 0, format , ##args)
#define MC_WARN(format, args...) MC_PRINT(LOGW, "WAR", 0, format , ##args)
#define MC_INFO(format, args...) MC_PRINT(LOGI, "INF", 0, format , ##args)
#define MC_DEBUG(format, args...) MC_PRINT(LOGD, "DBG", 0, format , ##args)

#define MC_MEM_DUMPP( printer, logType, logLevel, pSrc, nSize, limit )                                         \
do {                                                                                                           \
	size_t i, j, nLineCnt;                                                                                     \
	size_t uAddress = (size_t)(pSrc);                                                                          \
	unsigned char uc;                                                                                          \
	unsigned char oneline[16*3+2];                                                                             \
	unsigned char *pBuffer = (unsigned char*)(pSrc);                                                           \
                                                                                                               \
	MC_PRINT(printer, logType, logLevel, "Memory Dump - Addr : %p, Size : %d/%d", pBuffer, (limit), (nSize) ); \
	MC_PRINT(printer, logType, logLevel, "---------------------------------------------------------" );        \
                                                                                                               \
	for ( i = 0, j = 0, nLineCnt = 0 ; i < (limit); i++ )                                                      \
	{                                                                                                          \
		if ( ( i % 16 ) == 0 )                                                                                 \
		{                                                                                                      \
			if ( nLineCnt != 0 )                                                                               \
			{                                                                                                  \
				oneline[j] = '\0';                                                                             \
				MC_PRINT(printer, logType, logLevel, "0x%08zX  %s", uAddress, oneline );                       \
			}                                                                                                  \
                                                                                                               \
			uAddress = (size_t)(pSrc) + i;                                                                     \
			j =0;                                                                                              \
                                                                                                               \
			nLineCnt++;                                                                                        \
		}                                                                                                      \
		uc = (*(pBuffer+i)) >> 4;                                                                              \
		if ( uc < 10 )                                                                                         \
			oneline[j] = uc+'0';                                                                               \
		else                                                                                                   \
			oneline[j] = uc-10+'A';                                                                            \
                                                                                                               \
		j++;                                                                                                   \
                                                                                                               \
		uc = (*(pBuffer+i)) & 0x0F;                                                                            \
		if ( uc < 10 )                                                                                         \
			oneline[j] = uc+'0';                                                                               \
		else                                                                                                   \
			oneline[j] = uc-10+'A';                                                                            \
                                                                                                               \
		j++; oneline[j++] = ' ';                                                                               \
	}                                                                                                          \
                                                                                                               \
	oneline[j] = '\0';                                                                                         \
	MC_PRINT(printer, logType, logLevel, "0x%08zX  %s", uAddress, oneline );                                   \
	MC_PRINT(printer, logType, logLevel, "---------------------------------------------------------" );        \
} while (0)

#define MC_MEM_DUMP(condition, printer, logType, logLevel, pSrc, nSize, limit)                                 \
do {                                                                                                           \
	if ((condition))                                                                                           \
		MC_MEM_DUMPP(printer, logType, logLevel, pSrc, nSize, (limit > nSize ? nSize : limit));                \
} while (0)

namespace Nex_MC {

namespace Log {
	typedef enum
	{
		FLOW=0,
	 	OUTPUT,
	 	INPUT,
	 	LOOP,
	 	EXTERNAL_CALL,
	 	MUTEX
	} NEXMCLogType;

	typedef enum
	{
		NO_LOG=0,
	 	ERROR,
	 	WARNING,
	 	INFO,
	 	DEBUG,
	 	VERBOSE
	} NEXMCLogLevel;

	void SetDebugLevel(int type, int avtype, int logTypes, int maxLevel);

	extern int gDebugLevel[NUM_TYPES][2][32];

	extern const char *LOGTYPE_NAME[];
}; //namespace Log

}; //namespace Nex_MC

#define STATUSP(MC_TYPE, MC_AVTYPE, type, level, printer, format, args...) MC_LOG((int)level <= ::Nex_MC::Log::gDebugLevel[MC_TYPE][MC_AVTYPE][type], printer, ::Nex_MC::Log::LOGTYPE_NAME[type], level, format , ##args)
#define STATUST(MC_TYPE, MC_AVTYPE, type, level, format, args...) STATUSP(MC_TYPE, MC_AVTYPE, type, level, LOGD, format , ##args)
#define STATUS(type, level, format, args...) STATUSP(NEX_MC_TYPE, NEX_MC_AVTYPE, type, level, LOGD, format , ##args)
#define IDENTITYP(MC_TYPE, MC_AVTYPE, type, level, printer, prependstr, codecname, ud, format, args...) MC_IDENTITY((int)level <= Nex_MC::Log::gDebugLevel[MC_TYPE][MC_AVTYPE][type], printer, prependstr, codecname, ud, format , ##args)
#define IDENTITY(prependstr, codecname, ud, format, args...) IDENTITYP(NEX_MC_TYPE, NEX_MC_AVTYPE, ::Nex_MC::Log::FLOW, ::Nex_MC::Log::WARNING, LOGD, prependstr, codecname, ud, format , ##args)
#define PRINTLNP(type, level, printer) STATUSP(NEX_MC_TYPE, NEX_MC_AVTYPE, type, level, printer, "")
#define PRINTLN(type, level) PRINTLNP(type, level, LOGD)

#define MEM_DUMPP(MC_TYPE, MC_AVTYPE, type, level, printer, pSrc, nSize, limit) MC_MEM_DUMP((int)level <= ::Nex_MC::Log::gDebugLevel[MC_TYPE][MC_AVTYPE][type], printer, ::Nex_MC::Log::LOGTYPE_NAME[type], level, pSrc, nSize, limit)
#define MEM_DUMP(type, level, pSrc, nSize, limit) MEM_DUMPP(NEX_MC_TYPE, NEX_MC_AVTYPE, type, level, LOGD, pSrc, nSize, limit)

/* for quick debugging: */
#define QSTATUSP(printer, format, args...) MC_PRINT(printer, "QCK", 0, format , ##args)
#define QSTATUS(format, args...) QSTATUSP(LOGE, format , ##args)
#define QPRINTLNP(printer) QSTATUSP(printer, "")
#define QPRINTLN() QPRINTLNP(LOGE)

#define QMEM_DUMPP(printer, pSrc, nSize, limit) MC_MEM_DUMPP(printer, "QCK", 0, pSrc, nSize, (limit > nSize ? nSize : limit))
#define QMEM_DUMP(pSrc, nSize, limit) QMEM_DUMPP(LOGE, pSrc, nSize, limit)
#else
#define MC_PRINT(printer, logType, logLevel, format, args...)
#define MC_LOG(mCondition, printer, logType, logLevel, format, args...)
#define MC_ERR(format, args...)
#define MC_WARN(format, args...)
#define MC_INFO(format, args...)
#define MC_DEBUG(format, args...)
#define MC_MEM_DUMPP( printer, logType, logLevel, pSrc, nSize, limit )
#define MC_MEM_DUMP(condition, printer, logType, logLevel, pSrc, nSize, limit)
namespace Nex_MC {

namespace Log {
	typedef enum
	{
		FLOW=0,
	 	OUTPUT,
	 	INPUT,
	 	LOOP,
	 	EXTERNAL_CALL,
	 	MUTEX
	} NEXMCLogType;

	typedef enum
	{
		NO_LOG=0,
	 	ERROR,
	 	WARNING,
	 	INFO,
	 	DEBUG,
	 	VERBOSE
	} NEXMCLogLevel;

	void SetDebugLevel(int type, int avtype, int logTypes, int maxLevel);

	extern int gDebugLevel[NUM_TYPES][2][32];

	extern const char *LOGTYPE_NAME[];
}; //namespace Log

}; //namespace Nex_MC

#define STATUSP(type, level, printer, format, args...)
#define STATUST(MC_TYPE, MC_AVTYPE, type, level, format, args...)
#define STATUS(type, level, format, args...)
#define IDENTITYP(MC_TYPE, MC_AVTYPE, type, level, printer, prependstr, codecname, ud, format, args...)
#define IDENTITY(prependstr, codecname, ud, format, args...)
#define PRINTLNP(type, level, printer)
#define PRINTLN(type, level)

#define MEM_DUMPP(type, level, printer, pSrc, nSize, limit)
#define MEM_DUMP(type, level, pSrc, nSize, limit)

/* for quick debugging: */
#define QSTATUSP(printer, format, args...)
#define QSTATUS(format, args...)
#define QPRINTLNP(printer)
#define QPRINTLN()

#define QMEM_DUMPP(printer, pSrc, nSize, limit)
#define QMEM_DUMP(pSrc, nSize, limit)
#endif

#endif //#ifndef _NEXCRAL_MC_DEBUG_H_

