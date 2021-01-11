//
//  Header.h
//  NexEditorFramework
//
//  Created by ERIC0 on 2020/11/26.
//

#ifndef NXEEffectEntry_h
#define NXEEffectEntry_h

@interface NXEEffectEntry: NSObject

- (instancetype) initWithSource:(NSDictionary *)source;

@property (nonatomic, readonly) int startTime;
@property (nonatomic, readonly) int beatIndex;
@property (nonatomic, readonly) BOOL sourceChange;
@property (nonatomic, readonly) NSString *clipEffectId;
@property (nonatomic, readonly) float clipEffectStartTime;
@property (nonatomic, readonly) float clipEffectEndTime;

@end


#endif /* NXEEffectEntry_h */
