/*
 * File Name   : NexEditorLog.h
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

#define NexLogE(tag, format, ...) _NexLog(NexEditorLogCategoryErr, tag, format, ##__VA_ARGS__)
#define NexLogW(tag, format, ...) _NexLog(NexEditorLogCategoryWarning, tag, format, ##__VA_ARGS__)
#define NexLogI(tag, format, ...) _NexLog(NexEditorLogCategoryInfo, tag, format, ##__VA_ARGS__)
#define NexLogD(tag, format, ...) _NexLog(NexEditorLogCategoryDebug, tag, format, ##__VA_ARGS__)



/**
 * \ingroup types
 * \brief A list of log categories
 * \since version 1.3.6
 */
typedef NS_ENUM(NSInteger, NexEditorLogCategory)
{
    NexEditorLogCategoryAudio      = 0,
    NexEditorLogCategoryVideo      = 1,
    NexEditorLogCategoryErr        = 2,
    NexEditorLogCategoryWarning    = 3,
    NexEditorLogCategoryInfo       = 4,
    NexEditorLogCategoryDebug      = 5,
    NexEditorLogCategoryMisc       = 6,
} _DOC_ENUM(NexEditorLogCategory);

#ifdef __cplusplus
extern "C" {
#endif
    
    void _NexLog(NexEditorLogCategory cat, NSString *t, NSString *format, ...);
    
#ifdef __cplusplus
};
#endif

/**
 * \class NexEditorLog
 * \brief NexEditorLog defines methods for logging.
 * \since version 1.3.6
 */
@interface NexEditorLog : NSObject

/** \brief Sets a log category to gather logs from core engine, no logs are gathered by default.
 *  \param level A Integer value indiates level. Higher value will print more logs. value range -1 ~ 54
 *  \param category One of NexEditorLogCategory which will be enabled
 *  \since version 1.3.6
 */
+ (void) setLogLevel:(NSInteger)level forCategory:(NexEditorLogCategory)category;

+ (void) logMessage:(NSString*)tag message:(NSString*) message;

+ (void) logWithCategory:(NexEditorLogCategory)category tag:(NSString *)tag message:(NSString *)message;

@end
