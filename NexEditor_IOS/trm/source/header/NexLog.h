/*
 * File Name   : NexLog.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#import <Foundation/Foundation.h>

#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif

#define LogLevelDisabled -1

/**
 * \ingroup types
 * \brief A list of log categories
 * \since version 1.3.6
 */
typedef NS_ENUM(NSInteger, NexLogCategory)
{
    NexLogCategoryAudio      = 0,
    NexLogCategoryVideo      = 1,
    NexLogCategoryErr        = 2,
    NexLogCategoryWarning    = 3,
    NexLogCategoryInfo       = 4,
    NexLogCategoryDebug      = 5,
    NexLogCategoryMisc       = 6,
} _DOC_ENUM(NexLogCategory);


/**
 * \class NexLogger
 * \brief NexLogger defines methods for logging.
 * \since version 1.3.6
 */
@interface NexLogger : NSObject

/** \brief Sets a log category to gather logs from core engine, no logs are gathered by default.
 *  \param level A Integer value indiates level. Higher value will print more logs. value range -1 ~ 54
 *  \param category One of NexLogCategory which will be enabled
 *  \since version 1.3.6
 */
+ (void) setLogLevel:(NSInteger)level forCategory:(NexLogCategory)category;

@end
