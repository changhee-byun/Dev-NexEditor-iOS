/******************************************************************************
 * File Name   :	NXEVisualEdit.h
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

#import <Foundation/Foundation.h>

@class NXEVisualClip;

/* Allow Doxygen to detect NS_ENUM as enum */
#ifndef _DOC_ENUM
#define _DOC_ENUM(name)
#endif

typedef NS_ENUM(NSUInteger, CropMode) {
	/*! The largest dimension of the original clip will be fit to the project display, 
		without any distortion of the original dimensions (and the rest of the display will display black). */
    FIT = 0,
    /*! The clip will be cropped to fill the project display, and the original may be distorted. */
    FILL,
    /*! The start and end positions of the crop will be randomly selected. */
    PAN_RAND
} _DOC_ENUM(CropMode);

/**
 * \brief A class that encompasses features such as Crop and Speed, that are complicated and changes values from <b>NexVisualClip</b>.
 */
@interface NXEVisualEdit : NSObject

/**
 * \brief This method randomizes the start and end crops of a clip, based on the chosen crop mode
 * \param cropMode A value from <b>cropMode</b>.
 * \param vinfo <b>endRect</b> value changes from <b>NexVisualClip</b>, which is passed as a parameter by cropMode. 
 */
- (void)randomizeStartEndPositionWithCropMode:(CropMode)cropMode
                        visualClip:(NXEVisualClip *)vinfo;

@end
