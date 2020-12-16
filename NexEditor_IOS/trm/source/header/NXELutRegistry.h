/*
 * File Name   : NXELutRegistry.h
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
#import "NXEEditorType.h"
#import "NXELutSource.h"

static const NXELutID kLutIdNotFound = NSNotFound;

/** 
 * \class NXELutRegistry
 * \brief Provides a way to register custom LUT
 * \since version 1.3.0
 */
@interface NXELutRegistry : NSObject

/** 
 * \brief Register a new custom LUT.
 *        Currently, NXECubeLutSource is only supported source class. Instantiate a NXECubeLutSource class with a path to the Cube LUT file and call this method to register and get the LUT ID which can be passed to NXEClip's lutId property.
 * \param source Object describes the custom LUT source. See NXECubeLutSource for details.
 * \param error If kLutIdNotFound is returned, an NXEError object will be assigned.
 * \return New LUT ID if successfully registered, kLutIdNotFound otherwise.
 * \since version 1.3.0
 */
+ (NXELutID) registerLUTWithSource:(NXEFileLutSource*)source error:(NSError**)error;

/**
 * \brief Convert a legacy NXELutTypes value to NXELutID value which is suitable for NXEClip's lutId property.
 * \param lutType One of valid NXELutTypes values.
 * \return Converted LUT ID if lutTypes is valid, kLutIdNotFound otherwise.
 * \since version 1.3.0
 */
+ (NXELutID) lutIdFromType:(NXELutTypes)lutType;

@end
