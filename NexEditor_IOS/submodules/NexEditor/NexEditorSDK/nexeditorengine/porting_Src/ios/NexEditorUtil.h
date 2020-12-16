/******************************************************************************
 * File Name   : NexEditorUtil.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
#ifndef NexEditorUtil_h
#define NexEditorUtil_h

typedef struct {
    CGRect start;
    CGRect end;
} KenburnsRects;

typedef struct {
    unsigned int uMaxMemorySize;
    unsigned int uMaxDecoderCount;
} CapacityInfo;

@interface NexEditorUtil : NSObject

/**
 * \brief get kenburns rects
 * \param resolution : the width and height of the original source
 * \param ratio : the ratio width and height
 * \param faceRects : the face detected rects
 * \return KenburnsRects : the result start and edn rects
 */
+ (KenburnsRects) getKenBurnsRectsInSize:(CGSize)resolution withTargetRatio:(CGSize)ratio duration:(int)duration faceRects:(NSArray <NSValue*>*)faceRects;

+ (int) checkAvailabilityOfHardwareResourceWithIdentifier:(NSString*)identifier videoInfos:(NSArray*)videoInfo;
+ (CapacityInfo) getAvailabilityOfHardwareResourceWithIdentifier:(NSString*)identifier;

@end

#endif /* NexEditorUtil_h */
