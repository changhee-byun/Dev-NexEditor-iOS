/******************************************************************************
 * File Name   : NexImage.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <NexEditorEngine/ImageLoaderProtocol.h>
#import <NexEditorEngine/LUTLoaderProtocol.h>
#import <NexEditorEngine/VignetteLoaderProtocol.h>


@interface NexImage : NSObject <ImageLoaderProtocol, LUTLoaderProtocol, VignetteLoaderProtocol>

@end
