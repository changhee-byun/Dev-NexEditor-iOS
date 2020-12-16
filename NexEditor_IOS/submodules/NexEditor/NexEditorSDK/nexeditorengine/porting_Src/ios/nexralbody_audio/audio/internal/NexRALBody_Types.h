//
//  NexRALBody_Types.h
//  nexRalBody
//
//  Created by Simon Kim on 7/22/14.
//
//

#import <Foundation/Foundation.h>

typedef enum {
    NexRALBODYCallbackTypeInit,
    NexRALBODYCallbackTypeDeinit
} NexRALBODYCallbackType;

@class NexRALBody;

@protocol NexRALBodyDelegate <NSObject>
@optional
- (void) ralBody:(NexRALBody *) ralBodyText didReceiveCallbackType:(NexRALBODYCallbackType) callbackType;
@end

@interface NexRALBody : NSObject
- (id) initWithHPlayer:(void *) hPlayer;
- (void) disconnectFromHPlayer;
@property (nonatomic, assign) id<NexRALBodyDelegate> delegate;
@property (nonatomic, readonly) id userData;
+ (NexRALBody *) ralBodyWithUserData:(id) userData;
+ (NexRALBody *) ralBodyForPlayer:(void *) hPlayer;
+ (NexRALBody *) ralBodyForDefaultPlayer;
+ (NSUInteger) numberOfInstances;
@end