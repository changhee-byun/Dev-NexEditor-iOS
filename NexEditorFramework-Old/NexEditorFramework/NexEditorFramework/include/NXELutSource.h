/*
 * File Name   : NXELutSource.h
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


/**
 * \class NXEFileLutSource
 * \brief Base class of file based custom LUT source classes.
 * \note Do not use this class directly. To register a custom LUT file, use NXECubeLutSource, a subclass of NXEFileLutSource.
 * \since version 1.3.0
 */
@interface NXEFileLutSource : NSObject

/** 
 * \brief Filesystem path to the custom LUT file
 * \since version 1.3.0
 */
@property (nonatomic, readonly) NSString *path;

/**
 * \brief Designated initializer
 * \param path Filesystem path to the custom LUT file
 * \since version 1.3.0
 */
- (instancetype) initWithPath:(NSString*)path;

@end

/**
 * \class NXECubeLutSource
 * \brief Describes Cube format custom LUT source. 
 *        Instiantiate this class with a filesystem path to a .cube file to be used as LUT and register with NXELutRegistry's registerLUTWithSource:error: method.
 * \see NXELutRegistry class
 * \since version 1.3.0
 */
@interface NXECubeLutSource : NXEFileLutSource

@end
