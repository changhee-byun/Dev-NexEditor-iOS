/******************************************************************************
 * File Name   : CustomLayerProtocol.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
#ifndef CustomLayerProtocol_h
#define CustomLayerProtocol_h

#import <Foundation/Foundation.h>

typedef NS_ENUM(NSUInteger, NexLayerRenderContextType) {
  NexLayerRenderContextPreview = 0,
  NexLayerRenderContextExport
};

@protocol CustomLayerProtocol <NSObject>

@required
- (BOOL)renderOverlay:(int)iParam1
               param2:(int)iParam2
               param3:(int)iParam3
               param4:(int)iParam4
               param5:(int)iParam5
               param6:(int)iParam6
               param7:(int)iParam7
               param8:(int)iParam8
               param9:(int)iParam9
              param10:(int)iParam10
              param11:(int)iParam11
              param12:(int)iParam12
              param13:(int)iParam13
              param14:(int)iParam14
              param15:(int)iParam15
              param16:(int)iParam16
              param17:(int)iParam17
              param18:(int)iParam18;

- (void)clearResourceInRenderContext:(NexLayerRenderContextType)renderContextType;

@end

#endif /* CustomLayerProtocol_h */
