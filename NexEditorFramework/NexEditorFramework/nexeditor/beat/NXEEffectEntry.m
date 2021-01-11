//
//  NXEEffectEntry.m
//  NexEditorFramework
//
//  Created by ERIC0 on 2020/11/26.
//

#import <Foundation/Foundation.h>
#import "NXEEffectEntry.h"

static NSString *const kStartTime = @"startTime";
static NSString *const kBeatIndex = @"beatIndex";
static NSString *const kSourceChange = @"sourceChange";
static NSString *const kClipEffectId = @"clipEffectId";
static NSString *const kClipEffectStartTime = @"clipEffectStartTime";
static NSString *const kClipEffectEndTime = @"clipEffectEndTime";

@interface NXEEffectEntry()

@property (nonatomic) int startTime;
@property (nonatomic) int beatIndex;
@property (nonatomic) BOOL sourceChange;
@property (nonatomic, copy) NSString *clipEffectId;
@property (nonatomic) float clipEffectStartTime;
@property (nonatomic) float clipEffectEndTime;

@end

@implementation NXEEffectEntry: NSObject

- (instancetype) initWithSource:(NSDictionary *)source
{
    self = [super init];
    if (self) {
        self.startTime = [source[kStartTime] intValue];
        self.beatIndex = [source[kBeatIndex] intValue];
        self.sourceChange = [source[kSourceChange] boolValue];
//        NSString* test = source[kClipEffectId];
        self.clipEffectId = source[kClipEffectId];

        self.clipEffectStartTime = (source[kClipEffectStartTime] != nil) ? [source[kClipEffectStartTime] floatValue] : 0;
        self.clipEffectEndTime = (source[kClipEffectEndTime] != nil) ? [source[kClipEffectEndTime] floatValue] : 0;
    }
    return self;

}

- (void) dealloc
{
    //self.clipEffectId = nil;
    [super dealloc];
}

@end
