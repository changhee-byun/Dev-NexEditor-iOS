/**
 * File Name   : LUTMap.h
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
#import "NXELutSource.h"
#import "NXELutSourcePrivate.h"

@interface LUTMap : NSObject

- (NSUInteger) addEntry:(NXEFileLutSource* _Nonnull)entry;
- (NXEFileLutSource* _Nullable) entryWith:(NXELutID)lutId;
- (NSUInteger) indexOfEntry:(NXEFileLutSource* _Nonnull)entry;

/**
 *
 * \param lutId string representation of LUT
 * \return nemeric representation of LUT for use with NXEClip
 */
+ (NSUInteger) indexFromLutName:(NSString * _Nonnull) lutName;

/**
 *
 * \param index numeric representation of LUT returned from -indexFromLutName:
 * \return file system path to LUT
 */
+ (NSString * _Nullable)pathForIndex:(NSUInteger)index;

+ (instancetype _Nonnull) instance;

+ (NSArray<NSString *> * _Nullable) builtinLUTNames;

@end
