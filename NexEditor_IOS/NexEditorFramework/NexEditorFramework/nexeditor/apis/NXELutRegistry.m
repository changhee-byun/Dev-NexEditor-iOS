/*
 * File Name   : NXELutRegistry.m
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
#import <UIKit/UIKit.h>
#import "NXELutRegistry.h"
#import "NXELutSource.h"
#import "LUTMap.h"
#import "NSError+ErrorCode.h"

NXELutID const kLutIdNotFound = NSNotFound;

@implementation NXELutRegistry

+ (NXELutID) registerLUTWithSource:(NXEFileLutSource*)source error:(NSError**)error
{
    NXELutID result = kLutIdNotFound;
    ERRORCODE errorCode = [source checkValidity];
    
    if( errorCode == ERROR_NONE ) {
        result = [LUTMap.instance indexOfEntry:source];
        if(result == NSNotFound) {
            result = [LUTMap.instance addEntry:source];
        }
    }
    else {
        if (error) {
            *error = [[NSError alloc] initWithErrorCode:errorCode];
        }
    }
    return result;
}

+ (NXELutID) lutIdFromType:(NXELutTypes)lutType
{
    NXELutID lutId = kLutIdNotFound;
    
    if( (lutType > NXE_LUT_NONE) && (lutType < NXE_LUT_MAX) ) {
        lutId = lutType;
    }
    return lutId;
}

+ (NSString *) lutStringFromType:(NXELutTypes)lutType
{
    NSString *lutId = nil;
    
    if( (lutType > NXE_LUT_NONE) && (lutType < NXE_LUT_MAX) ) {
        lutId = [[LUTMap builtinLUTNames] objectAtIndex:lutType];
    }
    return lutId;
}

@end


