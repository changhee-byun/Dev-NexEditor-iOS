/*
 * File Name   : NXETextEffect.h
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
#import "NXEProject.h"

/**
 * \class NXETextEffectParams
 * \brief Represents intro/outro titles for text effect asset item represented by a NXETextEffect object
 * \since version 1.2
 */
@interface NXETextEffectParams : NSObject

/** \brief title information of in-animation in overlaytitle
 *  \since version 1.2
 */
@property (nonatomic, retain) NSString *introTitle;

/** \brief sub-title information of in-animation in overlaytitle
 *  \since version 1.2
 */
@property (nonatomic, retain) NSString *introSubtitle;

/** \brief title information of out-animation in overlaytitle
 *  \since version 1.2
 */
@property (nonatomic, retain) NSString *outroTitle;

/** \brief sub-title information of out-animation in overlaytitle
 *  \since version 1.2
 */
@property (nonatomic, retain) NSString *outroSubtitle;

@end

/**
 * \class NXETextEffect
 * \brief Represents text effect asset item id and intro/outro titles.
 *
 *        An object of this class can be set to a NXEProject to apply a text effect.
 *
 * \since version 1.2
 */
@interface NXETextEffect: NSObject
/*! Item id of a Text Effect asset item. Use the value of a NXETextEffectAssetItem's itemId property. */
@property (nonatomic, readonly) NSString *itemId;
/*! Intro/outro title and subtitle parameters to be used for a project with the Text Effect retrievable using \c itemId property. */
@property (nonatomic, readonly) NXETextEffectParams *params;
/**
 * \brief Designated initializer
 * \param itemId Text effect asset item id
 * \param params NXETextEffectParams object
 * \return Instance of NXETextEffect class.
 */
- (instancetype) initWithId:(NSString *) itemId params:(NXETextEffectParams *) params;
@end

/**
 * Extends NXEProject with NXETextEffect related properties and methods.
 */
@interface NXEProject(TextEffect)

/** 
 * \brief NXETextEffect object currently set to this project.
 * \since version 1.2
 */
@property (nonatomic, readonly) NXETextEffect *textEffect;

/**
 * \brief Apply the text effect to the project.
 * \param textEffect NXETextEffect object or nil to unset previously set NXETextEffect.
 * \param error Non-nil if return value was NO.
 * \return YES if succesfully set. NO, otherwise.
 * \since version 1.2
 */
- (BOOL) setTextEffect:(NXETextEffect *)textEffect error:(NSError **) error;

@end
