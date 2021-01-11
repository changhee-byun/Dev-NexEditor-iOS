/******************************************************************************
 * File Name   :	NXEError.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Proprietary and Confidential
 Copyright (C) 2002~2017 NexStreaming Corp. All rights reserved.
 www.nexstreaming.com
 ******************************************************************************/

#import <Foundation/Foundation.h>
#import "NXEEditorErrorCode.h"

/** \class NXEError
 *  \brief NXEError class defines all error codes returned from NXEEngine.
 *
 *  This class is a NSError subclass. Hence, \c ERRORCODE value can be accessed through \c code property.
 *
 */
@interface NXEError : NSError

/*! Error code returned from the engine. */
@property (nonatomic, readonly) ERRORCODE errorCode;

/*! Error description. \c localizedDescription property returns the same description text. */
@property (nonatomic, readonly) NSString *errorDescription;

/**
 * Initialize the instance of this class.
 * \param errorCode The error code the instance of this class to represent
 * \return The instance of this class.
 */
- (instancetype)initWithErrorCode:(ERRORCODE)errorCode;

@end
