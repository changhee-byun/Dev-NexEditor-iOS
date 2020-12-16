/******************************************************************************
 * File Name   : NexColorMatrix.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

@interface NexColorMatrix : NSObject

@property(nonatomic, assign) float* matrix;

- (id)init;
- (id)initWithColorMatrixArray:(float*)colorMatrixArray;
- (void)set:(NexColorMatrix*)colorMatrix;
- (void)setWithFloatArray:(float*)matrix;
- (void)reset;
- (void)setConcat:(NexColorMatrix*)matA colorMatrixB:(NexColorMatrix*)matB;
- (void)preConcat:(NexColorMatrix*)preMatrix;
- (void)applyTo44:(float*)pmatrix;

@end
