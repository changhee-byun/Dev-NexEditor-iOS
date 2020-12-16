/******************************************************************************
 * File Name   : NXEAudioEnvelop.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEAudioEnvelop.h"
#import "NXEClip.h"

#define MAX_INDEX 65536

@interface NXEAudioEnvelop()

/* Audio Envelop */
@property (nonatomic, retain) NSMutableArray *cachedTimeList;
@property (nonatomic, retain) NSMutableArray *cachedLevelList;
@property (nonatomic, assign) Boolean isModified;
@property (nonatomic, assign) int totalTime;
@property (nonatomic, assign) int trimStartTime;
@property (nonatomic, assign) int trimEndTime;
///////////////////////////////////////////
@end

@implementation NXEAudioEnvelop

- (void)dealloc
{
    self.volumeEnvelopeTime = nil;
    self.volumeEnvelopeLevel = nil;
    self.cachedTimeList = nil;
    self.cachedLevelList = nil;
    //
    [super dealloc];
}

- (id)init
{
    self = [super init];
    if(self) {
        _isModified = NO;
        _totalTime = 0;
        _trimStartTime = 0;
        _trimEndTime = 0;
        //
        _volumeEnvelopeTime = nil;
        _volumeEnvelopeLevel = nil;
        //
        _cachedTimeList = nil;
        _cachedLevelList = nil;
    }
    return self;
}

- (id)initWithClip:(NXEClip*)clip
{
    self = [super init];
    if(self) {
        _isModified = YES;
        _trimStartTime = 0;
        _trimEndTime = clip.durationMs;
        _totalTime = clip.durationMs;
        //
        _volumeEnvelopeLevel = [[NSMutableArray alloc] init];
        
        [self.volumeEnvelopeLevel insertObject:[NSNumber numberWithInt:100] atIndex:0];
        [self.volumeEnvelopeLevel insertObject:[NSNumber numberWithInt:100] atIndex:1];
        
        _volumeEnvelopeTime = [[NSMutableArray alloc] init];
        
        [self.volumeEnvelopeTime insertObject:[NSNumber numberWithInt:0] atIndex:0];
        [self.volumeEnvelopeTime insertObject:[NSNumber numberWithInt:self.totalTime] atIndex:1];
        //
        _cachedTimeList = nil;
        _cachedLevelList = nil;
    }
    return self;
}

#pragma mark - Envelop

- (void)updateTrimTime:(int)start end:(int)end
{
    if(start == 0 && end == 0) {
        self.trimStartTime = self.trimEndTime = 0;
        self.isModified = YES;
        return;
    }
    
    if(start < 0) {
        NSException *e = [NSException exceptionWithName:@"InvalidArgumentException"
                                                 reason:@"InvalidArgumentException: start < 0"
                                               userInfo:nil];
        @throw e;
    }
    if(end <= start) {
        NSException *e = [NSException exceptionWithName:@"InvalidRangeException"
                                                 reason:@"InvalidRangeException: end <= start"
                                               userInfo:nil];
        @throw e;
    }
    
    self.trimStartTime = start;
    self.trimEndTime = end;
    self.isModified = YES;
}

- (int)addVolumeEnvelop:(int)time level:(int)level
{
    if(time < 0) {
        NSException *e = [NSException exceptionWithName:@"InvalidArgumentException"
                                                 reason:@"InvalidArgumentException:time<0"
                                               userInfo:nil];
        @throw e;
    }
    if(level > 200 || level < 0) {
        NSException *e = [NSException exceptionWithName:@"InvalidRangeException"
                                                 reason:@"An Available EnvelopLevel's value:0~200"
                                               userInfo:nil];
        @throw e;
    }
    //
    if(self.volumeEnvelopeLevel == nil) {
        _volumeEnvelopeLevel = [[NSMutableArray alloc] init];
        
        [self.volumeEnvelopeLevel insertObject:[NSNumber numberWithInt:100] atIndex:0];
        [self.volumeEnvelopeLevel insertObject:[NSNumber numberWithInt:100] atIndex:1];
    }
    if(self.volumeEnvelopeTime == nil) {
        _volumeEnvelopeTime = [[NSMutableArray alloc] init];
        
        [self.volumeEnvelopeTime insertObject:[NSNumber numberWithInt:0] atIndex:0];
        [self.volumeEnvelopeTime insertObject:[NSNumber numberWithInt:self.totalTime] atIndex:1];
    }
    //
    int index = [self findTime2Index:time level:level];
    
    if(index >= MAX_INDEX /* time값이 같을때 level변경이 될 수도 있기에 */) {
        int index_ = index - MAX_INDEX;
        int level_ = [[self.volumeEnvelopeLevel objectAtIndex:index_] intValue];
        if(level_ != level) {
            [self.volumeEnvelopeLevel replaceObjectAtIndex:index_ withObject:[NSNumber numberWithInt:level]];
        }
        return index_;
    }

    [self.volumeEnvelopeTime insertObject:[NSNumber numberWithInt:time] atIndex:index];
    [self.volumeEnvelopeLevel insertObject:[NSNumber numberWithInt:level] atIndex:index];
    
    _isModified = YES;
    
    return index;
}

- (int)findTime2Index:(int)time level:(int)level
{
    int index = 0;
    //
    for(int i = 0; i < [self.volumeEnvelopeTime count]; i++) {
        int basedTime = [[self.volumeEnvelopeTime objectAtIndex:i] intValue];

        // compare by the unit, second.
        if(basedTime/1000 == time/1000) {
            index = MAX_INDEX + i;
            [self.volumeEnvelopeLevel replaceObjectAtIndex:i withObject:[NSNumber numberWithInt:level]];
            break;
        } else if(basedTime/1000 > time/1000) {
            index = i;
            break;
        }
    }
    
    return index;
}

- (void)changeVolumeLevelValue:(int)index level:(int)level
{
    if(index > [self.volumeEnvelopeLevel count] - 1) {
        NSException *e = [NSException exceptionWithName:@"InvalidArgumentException"
                                                 reason:@"Index is over than the size of array, volumeEnvelopLevel"
                                               userInfo:nil];
        @throw e;
    }
    
    if(level > 200 || level < 0) {
        NSException *e = [NSException exceptionWithName:@"InvalidRangeException"
                                                 reason:@"An Available EnvelopLevel's range:0~200"
                                               userInfo:nil];
        @throw e;
    }
    
    [self.volumeEnvelopeLevel replaceObjectAtIndex:index withObject:[NSNumber numberWithInt:level]];
    self.isModified = YES;
}

- (void)removeVolumeEnvelop
{
    if(self.volumeEnvelopeTime != nil) {
        [self.volumeEnvelopeTime removeAllObjects];
        self.volumeEnvelopeTime = nil;
    }
    
    if(self.volumeEnvelopeLevel != nil) {
        [self.volumeEnvelopeLevel removeAllObjects];
        self.volumeEnvelopeLevel = nil;
    }
    
    _isModified = YES;
}

- (void)removeVolumeEnvelop:(int)index
{
    if(index > [self.volumeEnvelopeTime count] - 1 || index > [self.volumeEnvelopeLevel count] - 1) {
        NSException *e = [NSException exceptionWithName:@"InvalidArgumentException"
                                                 reason:@"Index is over than the size of array, envelopList"
                                               userInfo:nil];
        @throw e;
    }
    
    if(self.volumeEnvelopeTime != nil)
        [self.volumeEnvelopeTime removeObjectAtIndex:index];
    
    if(self.volumeEnvelopeLevel != nil)
        [self.volumeEnvelopeLevel removeObjectAtIndex:index];
    
    _isModified = YES;
}

- (int)getVolumeEnvelopeTime:(int)index
{
    if(index > [self.volumeEnvelopeTime count] - 1) {
        NSException *e = [NSException exceptionWithName:@"InvalidArgumentException"
                                                 reason:@"Index is over than the size of array, volumeEnvelopTime"
                                               userInfo:nil];
        @throw e;
    }
    
    if(self.volumeEnvelopeTime == nil)
        return -1;
    
    return (int)[[self.volumeEnvelopeTime objectAtIndex:index] integerValue];
}

- (int)getVolumeEnvelopeLevel:(int)index
{
    if(index > [self.volumeEnvelopeLevel count] - 1) {
        NSException *e = [NSException exceptionWithName:@"InvalidArgumentException"
                                                 reason:@"Index is over than the size of array, volumeEnvelopLevel"
                                               userInfo:nil];
        @throw e;
    }
    
    if(self.volumeEnvelopeLevel == nil)
        return -1;
    
    return (int)[[self.volumeEnvelopeLevel objectAtIndex:index] integerValue];

}

- (int)getVolumeEnvelopeTimeAdj:(int)index
{
    if(index > [self.volumeEnvelopeTime count] - 1) {
        NSException *e = [NSException exceptionWithName:@"InvalidArgumentException"
                                                 reason:@"Index is over than the size of array, volumeEnvelopTime"
                                               userInfo:nil];
        @throw e;
    }
    
    if(self.volumeEnvelopeTime == nil)
        return -1;
    
    return (int)[[self.volumeEnvelopeTime objectAtIndex:index] integerValue] - self.trimStartTime;
}

- (int)getVolumeEnvelopLength
{
    return (int)[self.volumeEnvelopeTime count];
}

- (int)getDuration
{
    if(self.trimEndTime == 0 && self.trimStartTime == 0) {
        return self.totalTime;
    } else {
        return self.trimEndTime - self.trimStartTime;
    }
}

- (NSArray*)getVolumeEnvelopeTimeList
{
    return [self getVolumeEnvelopeList:YES];
}

- (NSArray*)getVolumeEnvelopeLevelList
{
    return [self getVolumeEnvelopeList:NO];
}

- (NSArray*)getVolumeEnvelopeList:(Boolean)time
{
    if(self.isModified) {
        self.isModified =  NO;
        if (self.volumeEnvelopeLevel != nil) {
            NSMutableArray *envelopTimeArray = [[NSMutableArray alloc] init];
            NSMutableArray *envelopLevelArray = [[NSMutableArray alloc] init];
            
            int prevTime = 0;
            int prevLevel = 0;
            
            for(int i=0; i<[self.volumeEnvelopeLevel count]; i++) {
                int t = [self getVolumeEnvelopeTimeAdj:i];
                int l = [self getVolumeEnvelopeLevel:i];
                int duration = [self getDuration];
                
                if(t <= duration && t > 0) {
                    if([envelopTimeArray count] == 0) {
                        int level = (int)((self.trimStartTime-(prevTime+self.trimStartTime))/(float)((t+self.trimStartTime)-(prevTime+self.trimStartTime))*(float)(l-prevLevel)+(float)prevLevel);
                        [envelopTimeArray addObject:[NSNumber numberWithInt:0]];
                        [envelopLevelArray addObject:[NSNumber numberWithInt:level]];
                    }
                    [envelopTimeArray addObject:[NSNumber numberWithInt:t]];
                    [envelopLevelArray addObject:[NSNumber numberWithInt:l]];
                }
                else if(t <= duration && l > -1 && t == 0 ) {
                    [envelopTimeArray addObject:[NSNumber numberWithInt:t]];
                    [envelopLevelArray addObject:[NSNumber numberWithInt:l]];
                }
                else if( t > duration && self.trimEndTime > 0) {
                    if([envelopTimeArray count] == 0) {
                        int level = (int)((self.trimStartTime-(prevTime+self.trimStartTime))/(float)((t+self.trimStartTime)-(prevTime+self.trimStartTime))*(float)(l-prevLevel)+(float)prevLevel);
                        [envelopTimeArray addObject:[NSNumber numberWithInt:0]];
                        [envelopLevelArray addObject:[NSNumber numberWithInt:level]];
                    }
                    
                    int level = (int)((duration - prevTime) / (float)(t - prevTime)*(float)(l-prevLevel)+(float)prevLevel);
                    [envelopTimeArray addObject:[NSNumber numberWithInt:duration]];
                    [envelopLevelArray addObject:[NSNumber numberWithInt:level]];
                    break;
                }
                prevTime = t;
                prevLevel = l;
            }
            
            if(self.cachedTimeList == nil) {
                _cachedTimeList = [[NSMutableArray alloc] init];
                
                for(NSObject *time in envelopTimeArray) {
                    [self.cachedTimeList addObject:time];
                }
            } else {
                [self.cachedTimeList removeAllObjects];
                
                for(NSObject *time in envelopTimeArray) {
                    [self.cachedTimeList addObject:time];
                }
            }
            
            if(self.cachedLevelList == nil) {
                _cachedLevelList = [[NSMutableArray alloc] init];
                
                for(NSObject *level in envelopLevelArray) {
                    [self.cachedLevelList addObject:level];
                }
            } else {
                [self.cachedLevelList removeAllObjects];
                
                for(NSObject *level in envelopLevelArray) {
                    [self.cachedLevelList addObject:level];
                }
            }
            
            // 위에서 선언한 건 정리한다.
			[envelopTimeArray release];
			[envelopLevelArray release];
        } else {
            // 기존 정보는 삭제하고,
            [self.cachedTimeList removeAllObjects];
            [self.cachedLevelList removeAllObjects];
            
            return nil;
        }
    }
    
    if(time) {
        if([self.cachedTimeList count] == 0) {
            return nil;
        } else {
            return self.cachedTimeList;
        }
    } else {
        if([self.cachedLevelList count] == 0) {
            return nil;
        } else {
            return self.cachedLevelList;
        }
    }
}

- (instancetype) copyWithZone:(NSZone *)zone
{
    NXEAudioEnvelop *destination = [[self class] allocWithZone:zone];
    
    if(self.volumeEnvelopeTime != nil) {
        destination.volumeEnvelopeTime = [[[NSMutableArray alloc] initWithArray:self.volumeEnvelopeTime copyItems:YES] autorelease];
    }
    if(self.volumeEnvelopeLevel != nil) {
        destination.volumeEnvelopeLevel = [[[NSMutableArray alloc] initWithArray:self.volumeEnvelopeLevel copyItems:YES] autorelease];
    }
    destination.cachedTimeList = [[[NSMutableArray alloc] initWithArray:self.cachedTimeList copyItems:YES] autorelease];
    destination.cachedLevelList = [[[NSMutableArray alloc] initWithArray:self.cachedLevelList copyItems:YES] autorelease];
    destination.isModified = self.isModified;
    destination.totalTime = self.totalTime;
    destination.trimStartTime = self.trimStartTime;
    destination.trimEndTime = self.trimEndTime;
    
    return destination;
}

//////////////////////////////////////////
@end
