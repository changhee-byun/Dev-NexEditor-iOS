/*
 * File Name   : NexEditorTypes.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2018 NexStreaming Corp. All rights reserved.
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

/// Values of NEXVIDEOEDITOR_ERROR defined in NEXVIDEOEDITOR_Error.h
typedef int NexEditorError;

typedef NS_OPTIONS(NSUInteger, NexEditorSetTimeOptions) {
    NexEditorSetTimeOptionsDisplay          = 1UL << 0,
    NexEditorSetTimeOptionsIDRFrameOnly     = 1UL << 1,
};
