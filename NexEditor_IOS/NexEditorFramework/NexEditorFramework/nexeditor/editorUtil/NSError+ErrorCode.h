/**
 * File Name   : NSError+ErrorCode.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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
#import "NXEEditorErrorCode.h"

@interface NSError (ErrorCode)

- (instancetype)initWithErrorCode:(ERRORCODE)errorCode;

+ (NSString *)getDescriptionWithErrorCode:(ERRORCODE)errorCode;

@end
