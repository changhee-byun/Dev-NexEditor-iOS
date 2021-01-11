/******************************************************************************
 * File Name   :	NXEError.m
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import "NXEError.h"
#import "NXEErrorPrivate.h"
#import "NSError+ErrorCode.h"

static NSString * const kNexEditorErrorDomain = @"com.nexstreaming.nexeditor";

@interface NXEError()
@property (nonatomic, strong) NSString *extraDescription;
@end

@implementation NXEError

- (instancetype)initWithErrorCode:(ERRORCODE)errorCode extraDescription:(NSString *) extraDesc
{
    NSString *description = [NSError getDescriptionWithErrorCode:errorCode];
    if (extraDesc) {
        description = [NSString stringWithFormat:@"%@. %@", description, extraDesc];
    }
    
    NSDictionary *userInfo = @{NSLocalizedDescriptionKey: description};
    self = [super initWithDomain:kNexEditorErrorDomain code:errorCode userInfo:userInfo];
    if(self) {
        _errorCode = errorCode;
        self.extraDescription = extraDesc;
    }
    return self;
}

- (instancetype)initWithErrorCode:(ERRORCODE)errorCode
{
    return [self initWithErrorCode: errorCode extraDescription:nil];
}

/// NSLocalizedDescriptionKey value
- (NSString *) errorDescription
{
    return self.localizedDescription;
}
@end
