//
//  NexRALBody.m
//  nexRalBody
//
//  Created by Simon Kim on 7/22/14.
//
//
#import "NexRALBody.h"
#import "NexEditorLog.h"

#define LOG_TAG @"NexRALBody"

@interface NexRALBody()
@property (nonatomic) void *hPlayer;
@property (nonatomic, retain) NSMutableDictionary* map;
@end

@implementation NexRALBody
@synthesize userData = _userData;

- (id) initWithHPlayer:(void *) hPlayer
{
    self = [super init];
    if ( self ) {
        self.hPlayer = hPlayer;
        [[self class] addRalBody:self];
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
}

#pragma mark - API
- (void) disconnectFromHPlayer
{
    [[self class] removeRalBody:self];
}


#pragma mark - Internal methods

- (id) userData
{
	if ( _userData == nil ) {
		_userData = @(self.hash);
	}
    return _userData;
}

#pragma mark - Class methods
+ (NSMutableDictionary *) map
{
    NSAssert(NO, @"[%@] class must implement +%@ method to have it's own player-ralbody mapping table", NSStringFromClass([self class]), NSStringFromSelector(_cmd));
    return nil;
}

+ (void) addRalBody:(NexRALBody *) ralBody
{
    if ( ralBody ) {
        id key = ralBody.userData;
        NSMutableDictionary *map = [[self class] map];
        NexLogD(LOG_TAG, @"[%@ %d] %@:hPlayer=%p key=0x%p", NSStringFromClass([self class]), __LINE__, NSStringFromSelector(_cmd), ralBody.hPlayer, ralBody.userData);
        
        @synchronized([self class]) {
            map[key] = ralBody;
        }
    }
}

+ (void) removeRalBody:(NexRALBody *) ralBody
{
    if ( ralBody ) {
        id key = ralBody.userData;
        NSMutableDictionary *map = [[self class] map];
        
        @synchronized([self class]) {
            [map removeObjectForKey:key];
            if ( map.count == 0 ) {
                NexLogD(LOG_TAG, @"[%@ %d] No remaining objects", NSStringFromClass([self class]), __LINE__);
            }
        }
    }
}

+ (NexRALBody *) ralBodyWithUserData:(id) userData
{
    NSMutableDictionary *map = [[self class] map];
    
    @synchronized([self class]) {
        return (NexRALBody *) map[userData];
    }
}

+ (NexRALBody *) ralBodyForPlayer:(void *) hPlayer
{
    /* SLOW */
    NSMutableDictionary *map = [[self class] map];
    
    NexRALBody *ralBody = nil;
    NexLogD(LOG_TAG, @"[%@ %d] %@:hPlayer=%p", NSStringFromClass([self class]), __LINE__, NSStringFromSelector(_cmd), hPlayer);
    
    @synchronized([self class]) {
        for( id key in [map allKeys]) {
            NexRALBody *obj = (NexRALBody *) map[key];
            if ( obj.hPlayer == hPlayer ) {
                ralBody = obj;
                NexLogD(@"nexRalBody_Audio", @"[%@ %i] Found ralBody(userData:%p) hPlayer:%p ", NSStringFromClass([self class]), __LINE__, obj.userData, hPlayer);
                break;
            }
        }
    }
    if ( ralBody == nil ) {
        NexLogE(LOG_TAG, @"[%@ %i] ralBodyForPlayer:%p not found", NSStringFromClass([self class]), __LINE__, hPlayer);
    }
    return ralBody;
}

+ (NexRALBody *) ralBodyForDefaultPlayer
{
    /* SLOW */
    NSMutableDictionary *map = [[self class] map];
    NexRALBody *ralBody = nil;
    
    @synchronized([self class]) {
        id key = [[map allKeys] firstObject];
        ralBody = (NexRALBody *) map[key];
    }
    
    if ( ralBody == nil ) {
        NexLogE(LOG_TAG, @"[%@ %i] ralBodyForDefaultPlayer not found", NSStringFromClass([self class]), __LINE__);
    }
    
    return ralBody;
}

+ (NSUInteger) numberOfInstances
{
    return [[[self class] map] count];
}
@end
