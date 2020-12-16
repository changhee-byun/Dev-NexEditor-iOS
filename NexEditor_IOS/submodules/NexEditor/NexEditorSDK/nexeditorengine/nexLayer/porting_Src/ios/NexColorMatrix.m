/******************************************************************************
 * File Name   : NexColorMatrix.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2016 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NexColorMatrix.h"

#define MAXMATRIXSIZE 20

@interface NexColorMatrix()


@end


@implementation NexColorMatrix

- (void)dealloc
{
    [super dealloc];
    free(_matrix);
}

- (id)init {
    if(self = [super init]) {
       
    }
    return self;
}

- (id)initWithColorMatrixArray:(float*)colorMatrixArray {
    if(self = [super init]) {
        if(colorMatrixArray != nil) {
            if(_matrix == nil) {
                _matrix = (float*)malloc(MAXMATRIXSIZE * sizeof(float));
                memset(self.matrix, 0x00, MAXMATRIXSIZE * sizeof(float));
            }
            
            memcpy(self.matrix, colorMatrixArray, MAXMATRIXSIZE * sizeof(float));
        }
    }
    return self;
}

- (void)set:(NexColorMatrix*)colorMatrix {
    memcpy(self.matrix, colorMatrix.matrix, MAXMATRIXSIZE * sizeof(float));
}

- (void)setWithFloatArray:(float*)matrix {
    if(self.matrix)
        memcpy(self.matrix, matrix, MAXMATRIXSIZE * sizeof(float));
}

- (void)reset {
    memset(self.matrix, 0x00, MAXMATRIXSIZE * sizeof(float));
    self.matrix[0] = self.matrix[6] = self.matrix[12] = self.matrix[18] = 1;
}

- (void)setConcat:(NexColorMatrix*)matA colorMatrixB:(NexColorMatrix*)matB {
    float tmpMatrix[20];
    float *tmp;
    
    if (matA == self || matB == self) {
        
        tmp = tmpMatrix;
    }
    else {
        
        tmp = self.matrix;
    }
    
    const float* a = matA.matrix;
    const float* b = matB.matrix;
    int index = 0;
    for(int j = 0; j < 20; j += 5) {
        for (int i = 0; i < 4; i++) {
            tmp[index++] = a[j + 0] * b[i + 0] + a[j + 1] * b[i + 5] +
            a[j + 2] * b[i + 10] + a[j + 3] * b[i + 15];
        }
        tmp[index++] = a[j + 0] * b[4] + a[j + 1] * b[9] +
        a[j + 2] * b[14] + a[j + 3] * b[19] +
        a[j + 4];
    }
    
    if (tmp != self.matrix) {
        
        memcpy(self.matrix, tmp, MAXMATRIXSIZE * sizeof(float));
    }
}

- (void)preConcat:(NexColorMatrix*)preMatrix {
    [self setConcat:self colorMatrixB:preMatrix];
}

- (void)applyTo44:(float*)pmatrix {
    pmatrix[0] = self.matrix[0];
    pmatrix[1] = self.matrix[1];
    pmatrix[2] = self.matrix[2];
    pmatrix[3] = self.matrix[4];
    
    pmatrix[4] = self.matrix[5];
    pmatrix[5] = self.matrix[6];
    pmatrix[6] = self.matrix[7];
    pmatrix[7] = self.matrix[9];
    
    pmatrix[8] = self.matrix[10];
    pmatrix[9] = self.matrix[11];
    pmatrix[10] = self.matrix[12];
    pmatrix[11] = self.matrix[14];
    
    pmatrix[12] = self.matrix[15];
    pmatrix[13] = self.matrix[16];
    pmatrix[14] = self.matrix[17];
    pmatrix[15] = 1.0f;
}

@end
