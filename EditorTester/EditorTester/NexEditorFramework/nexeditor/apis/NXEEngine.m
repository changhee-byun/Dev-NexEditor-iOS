/******************************************************************************
 * File Name   : NXEEngine.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import <Foundation/Foundation.h>

#import "NXEEngine.h"
#import "NXEEnginePrivate.h"
#import "NXEEditorType.h"
#import "NXEClipInternal.h"
#import "NXEProject.h"
#import "NXEAudioEnvelop.h"
#import "NXEColorEffect.h"
#import "NXELayer.h"
#import "NXEEngineEventListener.h"
#import "NXEEngineUiEventListener.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "NXEVideoLayer.h"
#import "NXEImageLayer.h"
#import "NXETextLayer.h"
#import "NXETextLayerPrivate.h"
#import "EditorUtil.h"
#import "KMVideoLayer.h"
#import "KMImageLayer.h"
#import "KMTextLayer.h"
#import "KMWaterMark.h"
#import "LayerManager.h"
#import "NXELayersManager.h"
#import "NexEditorVersion.h"
#import "AssetLibraryProxy.h"
#import "AspectRatioTool.h"
#import <NexEditorEngine/NexEditorConfiguration.h>
#import "NXEProjectInternal.h"
#import <NexEditorEngine/NXEVisualClipInternal.h>
#import "NexLogger.h"
#import "NXETemplateProject.h"
#import "NXETemplateProject+Internal.h"

#define LOG_TAG @"NXEEngine"


static NXEAspectType __aspectType = NXEAspectTypeRatio16v9;
static NXESizeInt __aspectRatio = {16, 9};

@class EditorResourceHolder;

@protocol InternalSeekDelegate <NSObject>
// mapped to NXEEngineEventListener's callback
- (int) resourceHolder:(EditorResourceHolder *) holder didSeekTo:(int) time display: (BOOL) display flag:(int) flag;
@end

@interface NXELayer(KMLayer)
- (void) syncPropertiesTo:(KMLayer *)kmLayer;
@end

@interface NXEClip (Utils)
- (NSString *)getEncodedEffectoptions;
- (NSString *)encodeTransitionEffectOptions;
- (NSString *)encodeEffectOptions;
@end

@interface NXEVisualClip (Utils)
- (void)setPropertyWithClip:(NXEClip *)clip;
@end

@interface NXEAudioClip (Utils)
- (void)setPropertyWithClip:(NXEClip *)clip projectTime:(int)projectTime;
@end

@interface EditorResourceHolder: NSObject
@property (nonatomic, assign) NexEditor* videoEditor;
@property (nonatomic, assign) NXELayersManager *nxeLayerManager;
@property (nonatomic, assign) LayerManager *layerManager;
@property (nonatomic, retain) NXEEngineEventListener *engineEventListener;
@property (nonatomic, retain) NXEEngineUIEventListener *eventUiListener;

@property (nonatomic, assign) BOOL isProjectResolved;
@property (nonatomic, strong) NXEProject* project;
@property (nonatomic) long listId4watermark;
@property (nonatomic, assign) id<InternalSeekDelegate> seekDelegate;
@end

@implementation EditorResourceHolder
- (instancetype) init
{
    self = [super init];
    if (self) {
        NSLog(@"%@ SDK Version: %@", [NXEEngine productName], [NXEEngine sdkVersionString]);
        
        self.videoEditor = [NexEditor sharedInstance];
        self.nxeLayerManager = [NXELayersManager sharedInstance];
        self.layerManager = [LayerManager sharedInstance];
        [self.layerManager setAspectRatio:NXEEngine.aspectRatio];
        
        self.eventUiListener = [[[NXEEngineUIEventListener alloc] init] autorelease];
        
        __block EditorResourceHolder *weakSelf = self;
        self.engineEventListener = [[[NXEEngineEventListener alloc] initWithCB:^int(int time, BOOL display, int flag) {
            int result = 0;
            if (weakSelf.seekDelegate) {
                result = [weakSelf.seekDelegate resourceHolder:weakSelf didSeekTo:time display:display flag:flag];
            }
            return result;
        }] autorelease];
        [self.engineEventListener setUIListener:self.eventUiListener];
        
        // A GL context is available due to NexEditor initialization so we can access the GL context.
        [NexEditorConfiguration captureGLContextParams];
        //
        [self.videoEditor setProperty:@"supportPeakMeter" Value:@"0"];
    }
    return self;
}

- (void) dealloc
{
    self.project = nil;
    [self.videoEditor destoryEditor];
    
    [self.nxeLayerManager cleanup];
    [self.layerManager cleanup];
    
    // Messed up everywhere else. No better place to clean up the NexLayer shared instance.
    [[NexLayer sharedInstance] cleanup];
    
    [self.engineEventListener setUIListener:nil];
    self.engineEventListener = nil;
    self.eventUiListener = nil;
    [super dealloc];
}

@end

@interface EditorResourceHolder(KMLayer) <KMLayerDelegate>
@end

@interface NXEEngine() <InternalSeekDelegate>

@property (nonatomic, strong) EditorResourceHolder *resourceHolder;
@property (nonatomic, readonly) SeekInfo *seekInfo;
@property (nonatomic, assign) BOOL isProjectResolved;
@property (nonatomic) long listId4watermark;

@end

@implementation NXEEngine
/*
+ (instancetype) instance
{
    return [[[NXEEngine alloc] init] autorelease];
}
*/
// deprecated, remove from 1.2
+ (void)setAspectMode:(NXEAspectType)type
{
    self.aspectType = type;
}
// deprecated, remove from 1.2
+ (NXEAspectType)getAspectMode
{
    return self.aspectType;
}

+ (NXESizeInt) aspectRatio
{
    return __aspectRatio;
}

+ (void) setAspectRatio:(NXESizeInt)ratio
{
    __aspectRatio = ratio;
}

+ (NXEAspectType) aspectType
{
    return __aspectType;
}

+ (void) setAspectType:(NXEAspectType)type
{
    [self setAspectType:type withRatio:NXESizeIntMake(0, 0)];
}

+ (void) setAspectType:(NXEAspectType) type withRatio:(NXESizeInt) ratio
{
    BOOL chanceAccepted = YES;
    if (type != NXEAspectTypeCustom ) {
        ratio = [AspectRatioTool ratioWithType:type];
    } else if (ratio.width <= 0 || ratio.height <= 0) {
        chanceAccepted = NO;
    }
    
    if (chanceAccepted) {
        __aspectType = type;
        __aspectRatio = ratio;
    }
}

+ (NSString *) productName
{
    NSBundle *bundle = [NSBundle bundleForClass:self];
    return bundle.infoDictionary[@"CFBundleName"];
}

// New API for version 1.2
+ (NSString *) sdkVersionString
{
    NSBundle *bundle = [NSBundle bundleForClass:self];
    NSString *result =  bundle.infoDictionary[@"CFBundleShortVersionString"];
    NSArray *compos = [result componentsSeparatedByString:@"."];
    if (compos.count > 3) {
        // Prefix 'RC' to indicate this is not an official version
        result = [NSString stringWithFormat:@"RC %@ Unofficial", result];
        NSLog(@"This version of SDK is not suitable for App Store submit. Please request for an official version of the SDK.");
    }
    return result;
}

+ (void)setLogEnableErr:(BOOL)err warning:(BOOL)warning flow:(BOOL)flow
{
    [NexLogger setLogLevel:(err ? 0 : LogLevelDisabled) forCategory:NexLogCategoryErr];
    [NexLogger setLogLevel:(warning ? 0 : LogLevelDisabled) forCategory:NexLogCategoryWarning];
    [NexLogger setLogLevel:(flow ? 0 : LogLevelDisabled) forCategory:NexLogCategoryDebug];
}

#pragma mark -

- (void)dealloc
{
    @synchronized (self) {
        NSMutableDictionary *holderMap = [self.class resourceHolderMap];
        [holderMap removeObjectForKey:self.instanceKey];
        
        // NESI-375 avoid dangling seekDelegate
        if (self.resourceHolder.seekDelegate == self) {
            NSValue *instanceKey = holderMap.allKeys.firstObject;
            if ( instanceKey ) {
                NXEEngine *promoted = (NXEEngine *) instanceKey.pointerValue;
                self.resourceHolder.seekDelegate = promoted;
            }
        }
    }
    self.resourceHolder = nil;
    
    [super dealloc];
}

- (id)init
{
    self = [super init];
    if(self) {
        BOOL firstInstance = NO;
        
        EditorResourceHolder *holder = nil;
        @synchronized (self) {
            
            NSMutableDictionary *holderMap = [self.class resourceHolderMap];
            
            if (holderMap.allValues.count > 0) {
                holder = holderMap.allValues[0];
            } else {
                holder = [[[EditorResourceHolder alloc] init] autorelease];
                holder.seekDelegate = self;
                firstInstance = YES;
            }
            holderMap[self.instanceKey] = holder;
        }
        self.resourceHolder = holder;
        
        if ( firstInstance ) {
            [self.videoEditor createProject];
            
            if([self.videoEditor getWaterMarkInfo]) {
                [self setWaterMarkEffect:YES];
            }
        }
        return self;
    }
    return nil;
}

- (NSValue *) instanceKey
{
    return [NSValue valueWithPointer:self];
}

+ (NSMutableDictionary *) resourceHolderMap {
    static dispatch_once_t onceToken;
    static NSMutableDictionary *map = nil;
    dispatch_once(&onceToken, ^{
        map = [[NSMutableDictionary alloc] init];
    });
    return map;
}

#pragma mark - InternalSeekDelegate
- (int) resourceHolder:(EditorResourceHolder *)holder didSeekTo:(int)time display:(BOOL)display flag:(int)flag
{
    return [self seekInternal:time Display:display Flag:flag];
}

#pragma mark -
- (NexEditor *) videoEditor
{
    return self.resourceHolder.videoEditor;
}

- (NXELayersManager *) nxeLayerManager
{
    return self.resourceHolder.nxeLayerManager;
}

- (LayerManager *) layerManager
{
    return self.resourceHolder.layerManager;
}

- (SeekInfo *) seekInfo
{
    return self.resourceHolder.engineEventListener.seekInfo;
}

- (NXEEngineEventListener *) engineEventListener
{
    return self.resourceHolder.engineEventListener;
}
- (BOOL) isProjectResolved
{
    return self.resourceHolder.isProjectResolved;
}

- (void) setIsProjectResolved:(BOOL)resolved
{
    self.resourceHolder.isProjectResolved = resolved;
}

- (long) listId4watermark
{
    return self.resourceHolder.listId4watermark;
}

- (void) setListId4watermark:(long)listId
{
    self.resourceHolder.listId4watermark = listId;
}

#pragma mark - AuthorizationData
- (NSDictionary*) authorizationData
{
    // Authorization Data Key/Value
#define kNXESDKLevel 2
    static NSString *const kNXEAuthDataKeySDKLevel = @"SDKLevel";
    
    return @{ kNXEAuthDataKeySDKLevel: @(kNXESDKLevel)};
}

#pragma mark -
- (NXESizeInt) aspectRatio
{
    return self.class.aspectRatio;
}

- (void) setAspectRatio:(NXESizeInt)ratio
{
    [self.class setAspectRatio:ratio];
}

- (NXEAspectType) aspectType
{
    return self.class.aspectType;
}

- (void) setAspectType:(NXEAspectType)type
{
    [self setAspectType:type withRatio:NXESizeIntMake(0, 0)];
}

- (void) setAspectType:(NXEAspectType) type withRatio:(NXESizeInt) ratio
{
    [self.class setAspectType:type withRatio:ratio];
 
    self.layerManager.aspectRatio = self.class.aspectRatio;
    KMLayer *watermark = [self.layerManager getLayer:self.listId4watermark];
    if(watermark != nil) {
        [watermark setNeedsGeometryUpdate];
    }
}

- (CGSize) logicalRenderSize
{
    return self.layerManager.renderRegionSize;
}

#pragma mark -

- (int)clearScreen
{
    return [self.videoEditor clearScreen:0x1];
}

- (void)setPreviewWithCALayer:(CALayer*)layer previewSize:(CGSize)previewSize
{
    if(previewSize.width <= 0 || previewSize.height <= 0) {
        NexLogE(LOG_TAG, @"error -> input value is invalid");
        return ;
    }
    
    if(layer == nil) {
        NexLogE(LOG_TAG, @"error -> viewController is nil");
        return ;
    }

    CGRect rect = [AspectRatioTool aspectFit:previewSize
                                         ratio:self.aspectRatio];
    
    [self.videoEditor setLayerWithRect:rect.origin.x
                                     y:rect.origin.y
                                 width:rect.size.width
                                height:rect.size.height
                                 scale:[UIScreen mainScreen].scale];
    
    CALayer *previewLayer = self.videoEditor.layer;
    NSMutableArray *layersToKeep = [NSMutableArray array];
    for (CALayer *sublayer in layer.sublayers ) {
        if ( ![sublayer isKindOfClass:previewLayer.class]) {
            [layersToKeep addObject:sublayer];
        }
    }
    layer.sublayers = [[layersToKeep copy] autorelease];
    [layer addSublayer:previewLayer];
    
    [self.layerManager setVideoRect:rect];
}

- (void)setWaterMarkEffect:(BOOL)isEnabled
{
    if(isEnabled) {
        KMWaterMark *watermark = [[[KMWaterMark alloc] init] autorelease];
        watermark.delegate = self.resourceHolder;
        [watermark setNeedsGeometryUpdate];
        self.listId4watermark = [self.layerManager addLayer:watermark];
    } else {
        self.listId4watermark = -1;
    }
}

- (void)setCallbackWithPlayProgress:(void(^)(int currentTime))onPlayProgress
                          playStart:(void(^)(void))onPlayStart
                            playEnd:(void(^)(void))onPlayEnd
{
    NXEEngineUIEventListener *uiEventListener = [self.engineEventListener getUiListener];
    [uiEventListener registerOnPlayProgressCb:onPlayProgress
                                              onPlayStartCb:onPlayStart
                                                onPlayEndCb:onPlayEnd];
}

- (void)preparedEditor:(void(^)(void))onPrepared
{
    if(self.engineEventListener.playStateEarlyUpdated == NXE_PLAYSTATE_NONE) {
        NXEEngineUIEventListener *uiEventListener = [self.engineEventListener getUiListener];
        [uiEventListener registerOnStageChangeCb:^(NXEPLAYSTATE oldState, NXEPLAYSTATE newState) {
            if(newState == NXE_PLAYSTATE_IDLE) {
                onPrepared();
                [uiEventListener registerOnStageChangeCb:nil];
            }
        }];
    } else {
        onPrepared();
    }
}

- (int)getCurrentPosition
{
    return [self.engineEventListener getCurrentPosition];
}

- (int)play
{
    if(self.project == nil) {
        return ERROR_GENERAL;
    }
    [self resolveProject:true];
    return [self.videoEditor startPlay:0];
}

- (int)resume
{
    if(self.project == nil) {
        return ERROR_GENERAL;
    }
    
    if(self.isProjectResolved == false) {
        [self resolveProject:true];
    }
    return [self.videoEditor startPlay:0];
}

/// deprecated
- (void)stopSync:(void(^)(ERRORCODE errorcode))onStopComplete
{
    [self stopSyncWithBlock:onStopComplete];
}

- (void)stopSyncWithBlock:(void(^)(ERRORCODE errorcode))onStopComplete
{
    __block ERRORCODE errorcode = ERROR_NONE;

#define STOP_COMPLETE_TIMEOUT_NSEC  (NSEC_PER_SEC * 3)

    dispatch_group_t group = dispatch_group_create();
    
    dispatch_group_enter(group);
    
    dispatch_queue_t queue = dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0);
    [self.videoEditor.asyncAPI stopOnQueue:queue complete:^(NexEditorError result) {
        errorcode = result;
        dispatch_group_leave(group);
    }];
    
    long timeout = dispatch_group_wait(group, dispatch_time(DISPATCH_TIME_NOW, STOP_COMPLETE_TIMEOUT_NSEC));
    dispatch_release(group);
    
    if (timeout != 0) {
        errorcode = ERROR_UNKNOWN;
        NexLogE(LOG_TAG, @"stopSync Timeout");
    }
    
    if (onStopComplete) onStopComplete(errorcode);
}

- (void)stopAsync:(void(^)(ERRORCODE errorcode))onStopComplete
{
    [self.videoEditor.asyncAPI stopWithComplete:^(NexEditorError result) {
        if (onStopComplete) onStopComplete((ERRORCODE) result);
    }];
}

- (ERRORCODE) stop
{
    __block ERRORCODE result = ERROR_NONE;
    [self stopSyncWithBlock:^(ERRORCODE errorcode) {
        result = errorcode;
    }];
    return result;
}

- (int)pause
{
    if(self.project == nil) {
        return ERROR_GENERAL;
    }
    [self stop];
    return ERROR_NONE;
}

- (int)setTime:(int)time display:(int)display idrFrame:(int)idrFrame
{
    if(self.isProjectResolved == false) {
        [self resolveProject:true];
    }
    return [self.videoEditor setTime:time display:display idrFrame:idrFrame];
}

- (void)setCallbackWithSetTime:(void(^)(int seekDoneTS))onSeekDone
{
    NXEEngineUIEventListener *uiEventListener = [self.engineEventListener getUiListener];
    [uiEventListener registerOnSetTimeDoneCb:^(int doneTime) {
        onSeekDone(doneTime);
    }];
}

- (void)seek:(int)time
{
    if([self seekInternal:time Display:true Flag:0] != ERROR_NONE)
    {
        NexLogE(LOG_TAG, @"seek is fail");
        return ;
    }
}

- (void)seekIDRorI:(int)time
{
    [self seekIOnly:time];
}

- (void)seekIOnly:(int)time
{
    [self seekInternal:time Display:true Flag:1];
}

- (void)seekIDROnly:(int)time
{
    [self seekInternal:time Display:true Flag:2];
}

- (int)seekInternal:(int)time Display:(BOOL)display Flag:(int)flag
{
    int result = ERROR_NONE;
    NexLogD(LOG_TAG, @"[NXEEngine.m %d] setTime requested ts:%d / display:%d / flag:%d", __LINE__, time, display, flag);
    //
    if(self.seekInfo.isSeeking) {
        if(display) {
            self.seekInfo.isPendingSeek = true;
            self.seekInfo.pendingSeekLocation = time;
            self.seekInfo.pendingSeekIDR = flag;
            //
            NexLogD(LOG_TAG, @"[NXEEngine.m %d] setTime pendingSeek ts:%d", __LINE__, self.seekInfo.pendingSeekLocation);
        } else {
            self.seekInfo.isPendingNonDisplaySeek = true;
            self.seekInfo.pendingNonDisplaySeekLocation = time;
            //
            NexLogD(LOG_TAG, @"[NXEEngine.m %d] setTime pendingNonSeek ts:%d", __LINE__, self.seekInfo.pendingNonDisplaySeekLocation);
        }
    } else {
        self.seekInfo.didSetTimeSinceLastPlay = true;
        
        NexLogD(LOG_TAG, @"NXEEngine -> SetTime : %d", time);
        [self setIsSeekingFlag:YES];
        
        if(display) {
            self.seekInfo.isPendingSeek = NO;
        } else {
            self.seekInfo.isPendingNonDisplaySeek = NO;
        }
        
        result = [self setTime:time display:display?1:0 idrFrame:flag]; // IDRFrame param( Only display idrfreme if idrframe is 1)
        //
        if(result != ERROR_NONE) {
            NexLogE(LOG_TAG, @"[NXEEngine.m %d] setTime fail!!", __LINE__);
            return ERROR_GENERAL;
        } else {
            NexLogD(LOG_TAG, @"[NXEEngine.m %d] setTime start ts:%d", __LINE__, time);
        }
        
    }
    
    return ERROR_NONE;
}

- (void)setIsSeekingFlag:(Boolean)isSeeking
{
    if(isSeeking == self.seekInfo.isSeeking) {
        return ;
    }
    
    self.seekInfo.isSeeking = isSeeking;
}

- (void)loadEffectsInEditor:(Boolean)flag
{
    id<AssetLibraryProxy> lib = [AssetLibrary proxy];
    if(lib == nil) {
        return;
    }
    
    NSMutableSet *effectIds = [[NSMutableSet alloc] init];
    NSMutableSet *effects = [[NSMutableSet alloc] init];
    
    NSMutableArray *themes = [[NSMutableArray alloc] init];
    NSMutableArray *clipEffects = [[NSMutableArray alloc] init];
    NSMutableArray *transitionEffects = [[NSMutableArray alloc] init];
    
    NSString *clipEffectId; NSString *transitionEffectId;
    
    for(NXEClip *clip in self.project.visualClips) {
        clipEffectId = clip.videoInfo.getClipEffectId;
        if(![clipEffectId isEqualToString:kEffectIDNone]) {
            [effectIds addObject:clipEffectId];
        }
        transitionEffectId = clip.videoInfo.getTransitionEffectId;
        if(![transitionEffectId isEqualToString:kEffectIDNone]) {
            [effectIds addObject:transitionEffectId];
        }
    }
    
    NSArray<NexSceneInfo *>*sceneList = self.project.sceneList;
    if (sceneList != nil) {
        for(NexSceneInfo *sceneInfo in sceneList) {
            [effectIds addObject:sceneInfo.effectId];
        }
        sceneList = nil;
    }

    for(NSString *effectId in effectIds) {
        id<AssetItem> effect = [lib itemForId:effectId];
        [effects addObject:effect];
    }
    
    if([effects count] > 0) {
        NSString *themeData = [lib buildThemeDataWithItems:[effects allObjects]];
        [self.videoEditor loadTheme:themeData flag:flag];
    }
    
    NSString *renderItemData = [NSString string];

    [self.videoEditor clearRenderItems:flag];

    for(AssetItem *effect in effects) {
        renderItemData = [lib buildRenderItemDataWithItem:effect];
        if( renderItemData ) {
            [self.videoEditor loadRenderItem:effect.itemId effectData:renderItemData flags:flag];
        }
    }
    
    [effectIds release];
    [effects release];
    
    [themes release];
    [clipEffects release];
    [transitionEffects release];
}

- (void)resolveProject:(BOOL)flag
{
    if(self.project == nil) {
        NexLogE(LOG_TAG, @"project is nil");
        return ;
    }
    
    [self.project updateProject];
    
    if(self.listId4watermark != -1) {
        KMLayer *watermark = [self.layerManager getLayer:self.listId4watermark];
        if(watermark != nil) {
            watermark.startTime = 0;
            watermark.endTime = [self.project getTotalTime];
        }
    }
    
    [self loadEffectsInEditor:flag];

    int clipId = 1/* based index incase of visual */;
    
    // create visual and audio clip array
    NSMutableArray* vclipList = [[NSMutableArray alloc] init];
    NSMutableArray* aclipList = [[NSMutableArray alloc] init];
    
    NXEVisualClip* vclip = nil;
    NXEAudioClip* aclip = nil;
    
    NXEVideoLayer *videoLayer = nil;
    
    // set visual clip
    for (NXEClip * clip in self.project.visualClips) {
        clip.videoInfo.clipId = clipId++;// set visual clip id
        
        vclip = [[NXEVisualClip alloc] init];
        [vclip setPropertyWithClip:clip];
        [vclipList addObject:vclip];
        [vclip release];
    }
    
    // set videolayer
    NSArray *layerList = [self getLayerList:NXE_LAYER_VIDEO];
    
    for(NXELayer *layer in layerList) {
        videoLayer = (NXEVideoLayer *)layer;
        videoLayer.layerClip.clipType = NXE_CLIPTYPE_VIDEO_LAYER;
        vclip = [[NXEVisualClip alloc] init];
        [vclip setPropertyWithClip:videoLayer.layerClip];
        vclip.startTime = videoLayer.startTime;
        vclip.endTime = videoLayer.endTime;
        [vclipList addObject:vclip];
        [vclip release];
    }
    
    clipId = 10001/* based index incase of audio */;
    
    NXEClip *bgmClip = self.project.bgmClip;
    
    if(bgmClip != nil) {
        
        bgmClip.audioInfo.clipId = clipId++;// set audio clip id
        
        aclip = [[NXEAudioClip alloc] init];
        [aclip setPropertyWithClip:bgmClip projectTime:[self.project getTotalTime]];
        [aclipList addObject:aclip];
        [aclip release];
    }
    
    for (NXEClip *clip in self.project.audioClips) {
        clip.audioInfo.clipId = clipId++;// set audio clip id
        
        aclip = [[NXEAudioClip alloc] init];
        [aclip setPropertyWithClip:clip projectTime:[self.project getTotalTime]];
        [aclipList addObject:aclip];
        [aclip release];
    }
    
    [self.videoEditor loadClipsAsync:vclipList audioClips:aclipList];
    
    NSArray<NexSceneInfo *>*sceneList = self.project.sceneList;
    NSArray<NexDrawInfo *>*slotList = self.project.slotList;
    
    if (sceneList != nil && slotList != nil) {
        [self.videoEditor configureDrawInfoListWithMasterList:sceneList slots:slotList];
        sceneList = nil; slotList = nil;
    }
    
    [self adjustProjectColorWith:self.project.colorAdjustments];
    
    self.isProjectResolved = true;
    
    [vclipList release];
    [aclipList release];
    
    int audioVolume = self.project.options.audioVolume * 200;
    [self.videoEditor setProjectVolume:audioVolume];
    
    int audioFadeInDuration = self.project.options.audioFadeInDuration;
    int aduioFadeOutDuration = self.project.options.audioFadeOutDuration;
    int projectDuration = [self.project getTotalTime];
    
    if(audioFadeInDuration + aduioFadeOutDuration > projectDuration) {
        
        audioFadeInDuration = audioFadeInDuration * projectDuration / (audioFadeInDuration + aduioFadeOutDuration);
        aduioFadeOutDuration = projectDuration - audioFadeInDuration;
    }
    [self.videoEditor setProjectVolumeFadeInTime:audioFadeInDuration fadeOutTime:aduioFadeOutDuration];
}

- (void)adjustProjectColorWith:(NXEColorAdjustments)colorAdjustments
{
    [self.videoEditor adjustProjectColorWith:colorAdjustments.brightness contrast:colorAdjustments.contrast saturation:colorAdjustments.saturation];
}

- (void)setCallbackWithEncodingProgress:(void(^)(int percent))onExportProgress
                            encodingEnd:(void(^)(NXEError *nxeError))onExportEnd
{
    NXEEngineUIEventListener *uiEventListener = [self.engineEventListener getUiListener];
    [uiEventListener registerOnEncodingProgressCb:onExportProgress onEncodingDoneCb:onExportEnd];
}

- (int)exportProject:(NSString*)path
               Width:(int)width
              Height:(int)height
             Bitrate:(int)bitrate
         MaxFileSize:(long)maxFileSize
     ProjectDuration:(int)projectDuration
    ForDurationLimit:(BOOL)forDurationLimit
        RotationFlag:(NXERotationAngle)rotationFlag
{
    NXEExportParams params = NXEExportParamsMake(width, height);
    params.bitPerSec = bitrate;
    params.durationMs = forDurationLimit?projectDuration:0;
    params.maxFileSizeByte = maxFileSize;
    params.rotationAngle = rotationFlag;
    
    return [self exportToPath:path withParams:params];
}

- (int)exportToPath:(NSString*)path withParams:(NXEExportParams)params
{
    // \param flag Rotation 0x0: 0 / 0x10: 90 / 0x20: 180 / 0x40: 270
    int flag =  0x0;
    if(params.rotationAngle == NXE_ROTATION_90) {
        flag =  0x10;
    } else if(params.rotationAngle == NXE_ROTATION_180) {
        flag =  0x20;
    } else if(params.rotationAngle == NXE_ROTATION_270) {
        flag =  0x40;
    }
    [self resolveProject:false];

    int fps = [self getValidFpsWith:(float)params.framePerSec];
    
    int result = [self.videoEditor encodeProject:path
                              Width:params.width
                             Height:params.height
                            Bitrate:params.bitPerSec
                        MaxFileSize:params.maxFileSizeByte
                    ProjectDuration:params.durationMs
                                FPS:fps
                       ProjectWidth:0
                      ProjectHeight:0
                       SamplingRate:44100
                               Flag:flag];
    // NESI-188
    self.isProjectResolved = false;
    //
    return result;
}

- (int) getValidFpsWith:(float)fps {

#define DEFAULT_FPS 30
#define MIN_FPS 1
#define MAX_FPS 60
    
    int result = DEFAULT_FPS;
    if( fps >= MIN_FPS && fps <= MAX_FPS) {
        result = fps;
    }
    return result * 100;
}

- (void)captureFrameWithBlock:(void (^)(UIImage *image, ERRORCODE errorCode))captureBlock
{
    if(self.videoEditor == nil) {
        NexLogE(LOG_TAG, @"videoeditor is nil");
        return;
    }
    
    [self.engineEventListener registerCaptureCallback:^(CGImageRef imageRef) {
        UIImage *image = [UIImage imageWithCGImage:imageRef];
        captureBlock(image, ERROR_NONE);
    } captureFailCallback:^(ERRORCODE errorCode) {
        captureBlock(nil, errorCode);
    }];
    
    [self.videoEditor captureCurrentFrame];
    return;
}

- (int)fastOptionPreview:(NXEFastOption)option optionValue:(NSString *)optionValue display:(BOOL)display
{
    if(option == NXE_NORMAL) {
        [self.videoEditor.asyncAPI runFastPreviewCommand:kFastPreviewCommandNormal display:display complete:NULL];
        return 1;
    }
    
    FastPreviewBuilder *builder = [FastPreviewBuilder builder];
    
    switch(option) {
        case NXE_COLOR_ADJUSTMENT:
        {
            NSArray *values = [optionValue componentsSeparatedByString:@","];
            
            int brightenss = [values[0] intValue];
            int contrast = [values[1] intValue];
            int saturation = [values[2] intValue];
            
            [builder applyColorAdjustWithBrightness:brightenss contrast:contrast saturation:saturation];
        }
            break;
            
            
        case NXE_PROJECT_COLOR_ADJUSTMENT:
        {
            NSArray *values = [optionValue componentsSeparatedByString:@","];
            
            float brightenss = [values[0] floatValue];
            float contrast = [values[1] floatValue];
            float saturation = [values[2] floatValue];
            
            [builder applyProjectColorAdjustments:brightenss contrast:contrast saturation:saturation];
        }
            break;
            
        case NXE_TINT_COLOR:
        {
            NSArray *values = [optionValue componentsSeparatedByString:@","];
            
            int red = [values[0] intValue];
            int green = [values[1] intValue];
            int blue = [values[2] intValue];
            
            int color = (red << 16) | (green << 8) | blue;
            
            [builder applyTintColor:color];
        }
            break;
            
        case NXE_CROP_RECT:
        {
            NSArray *values = [optionValue componentsSeparatedByString:@","];
            
            int left = [values[0] intValue];
            int top = [values[1] intValue];
            int right = [values[2] intValue];
            int bottom = [values[3] intValue];
            
            [builder applyCropRect:CGRectMake(left, top, right, bottom)];
        }
            break;
            
        case NXE_NOFX:
        {
            int isNofx = [optionValue intValue];
            
            [builder applyNofx:isNofx];
        }
            break;
            
        case NXE_SWAP_VERTICAL:
        {
            int isSwapV = [optionValue intValue];
            
            [builder applySwapv:isSwapV];
        }
            break;
            
        case NXE_CTS:
        {
            int cts = [optionValue intValue];
            
            [builder applyCTS:cts];
        }
            break;
            
        case NXE_360_VIDEO:
        {
            NSArray *values = [optionValue componentsSeparatedByString:@","];
            
            int is360Video = [values[0] intValue];
            int horizontalBased = [values[1] intValue];
            int verticalBased = [values[2] intValue];
            
            [builder applyVideo360:is360Video horizontalBasedX:horizontalBased verticalBasedY:verticalBased];
        }
            break;
            
        default:
            break;
    }
    
    id<FastPreviewCommand> command = [builder buildCommand];
    [self.videoEditor.asyncAPI runFastPreviewCommand:command display:display complete:NULL];
    
    return 1;
}

- (NXEProject *) project
{
    return self.resourceHolder.project;
}

- (void)setProject:(NXEProject*)project
{
    if (self.resourceHolder.project == project) {
        return;
    }
    if ([project isKindOfClass:NXETemplateProject.class]) {
        if (self.aspectType != ((NXETemplateProject *)project).aspectType) {
            self.aspectType = ((NXETemplateProject *)project).aspectType;
        }
    }
    
    [self setLayers:@[]];
    
    self.resourceHolder.project = project;
    if ( project ) {
        [self setLayers:project.layers];
    }
    
    // NESI-174
    self.isProjectResolved = false;
    
    // NESI-189
    [self.videoEditor clearTrackCache];
}

- (void) updatePreview
{
    if (self.project.visualClips.count == 0) {
        return;
    }
    
    NXETimeMillis curTimeMS = self.getCurrentPosition;
    NXETimeMillis projectTimeMS = self.project.getTotalTime;
    if (curTimeMS > projectTimeMS) {
        curTimeMS = 0;
    }

    [self resolveProject:true];
    [self.videoEditor setTime:curTimeMS display:1 idrFrame:1];
}

- (void) updateDrawInfo:(NexDrawInfo *)drawInfo
{
    [self.videoEditor updateDrawInfoWith:drawInfo];
}

#pragma mark - Layer

- (void) setLayers:(NSArray<NXELayer *> *)layers
{
    // remove layers
    NSMutableArray<NXELayer *> *toRemove = [[[self.nxeLayerManager getLayerList] mutableCopy] autorelease];
    [toRemove removeObjectsInArray:layers];
    for( NXELayer *layer in toRemove) {
        [self removeLayerForIndex:layer.layerId];
    }
    
    // add new layers
    // layers - nxeLayermanager = new layers
    NSMutableArray<NXELayer *> *toAdd = [[layers mutableCopy] autorelease];
    [toAdd removeObjectsInArray:[self.nxeLayerManager getLayerList]];
    
    for( NXELayer *layer in toAdd) {
        [self addLayer:layer];
    }
}

- (NSArray *)getLayerList:(NXELayerType)layerType
{
    NSMutableArray *layerLists = [NSMutableArray array];
    for(NXELayer *layer in [self.nxeLayerManager getLayerList]) {
        if(layerType == NXE_LAYER_VIDEO) {
            if([layer isKindOfClass:(NXEVideoLayer.class)]) {
                [layerLists addObject:layer];
            }
        } else if(layerType == NXE_LAYER_IMAGE) {
            if([layer isKindOfClass:(NXEImageLayer.class)]) {
                [layerLists addObject:layer];
            }
        } else if(layerType == NXE_LAYER_STICKER) {
            // TODO
        } else {
            if([layer isKindOfClass:(NXETextLayer.class)]) {
                [layerLists addObject:layer];
            }
        }
    }
    return layerLists;
}

- (void)removeLayerForIndex:(long)layerIndex
{
    [self.layerManager removeLayer:layerIndex];
    [self.nxeLayerManager removeEntryForIndex:layerIndex];
}

- (void)removeAllLayers
{
    [self.layerManager clear:NO];
    [self.nxeLayerManager removeAllEntry];
}

- (void)setPropertyOfKMLayer:(KMLayer *)kmLayer withNXELayer:(NXELayer *)nxeLayer
{
    kmLayer.x = nxeLayer.x;
    kmLayer.y = nxeLayer.y;
    kmLayer.width = nxeLayer.width;
    kmLayer.height = nxeLayer.height;
    kmLayer.startTime = nxeLayer.startTime;
    kmLayer.endTime = nxeLayer.endTime;
    kmLayer.angle = (CGFloat)nxeLayer.angle;
    kmLayer.scale = (CGFloat)nxeLayer.scale;
    kmLayer.hFlip = nxeLayer.hFlip;
    kmLayer.vFlip = nxeLayer.vFlip;
    kmLayer.brightness = nxeLayer.brightness;
    kmLayer.contrast = nxeLayer.contrast;
    kmLayer.saturation = nxeLayer.saturation;
    kmLayer.alpha = nxeLayer.alpha;
    kmLayer.scale = nxeLayer.scale;
    
    [kmLayer setLut:(KMLutType)nxeLayer.colorFilter];
    
    KMAnimationType inAnimatinType = nxeLayer.inAnimation == 0 ? KMAnimationNone : KMAnimationNone + nxeLayer.inAnimation;
    KMAnimationType outAnimatinType = nxeLayer.outAnimation == 0 ? KMAnimationNone : KMAnimationOverallMax + nxeLayer.outAnimation;
    KMAnimationType overallAnimationType = nxeLayer.expression == 0 ? KMAnimationNone : KMAnimationInMax + nxeLayer.expression;
    
    [kmLayer setInAnimationType:inAnimatinType WithDuration:nxeLayer.inAnimationDuration];
    [kmLayer setOutAnimationType:outAnimatinType WithDuration:nxeLayer.outAnimationDuration];
    [kmLayer setOverallAnimationType:overallAnimationType];
}

- (void)addLayer:(NXELayer *)layer
{
    if(layer.layerType == NXE_LAYER_VIDEO) {
        NXEVideoLayer *videoLayer = (NXEVideoLayer *)layer;
        KMVideoLayer *kmVideoLayer = [[[KMVideoLayer alloc] initWithWidth:videoLayer.width height:videoLayer.height] autorelease];
        
        [videoLayer syncPropertiesTo:kmVideoLayer];
        
        videoLayer.layerId = [self.layerManager addLayer:kmVideoLayer];
        videoLayer.layerClip.videoInfo.clipId = kmVideoLayer.layerIndex = (int)(2000 + layer.layerId);
        
        [self.nxeLayerManager addLayer:videoLayer];
    }
    else if(layer.layerType == NXE_LAYER_IMAGE) {
        NXEImageLayer *imageLayer = (NXEImageLayer *)layer;
        KMImageLayer *kmImageLayer = [[[KMImageLayer alloc] initWithImage:imageLayer.uiImage] autorelease];
        
        [imageLayer syncPropertiesTo:kmImageLayer];
        
        imageLayer.layerId = [self.layerManager addLayer:kmImageLayer];
        
        [self.nxeLayerManager addLayer:imageLayer];
    }
    else if(layer.layerType == NXE_LAYER_TEXT) {
        NXETextLayer *textLayer = (NXETextLayer *)layer;
        KMTextLayer *kmTextLayer = [[[KMTextLayer alloc] init] autorelease];

        // these values should be set before calling setText:Font:.
        [kmTextLayer setKerningRatio:textLayer.textLayerProperty.kerningRatio];
        [kmTextLayer setSapceBetweenLines:textLayer.textLayerProperty.spaceBetweenLines];
        [kmTextLayer setHorizontalAlign:textLayer.textLayerProperty.horizontalAlign];
        [kmTextLayer setVerticalAlign:textLayer.textLayerProperty.verticalAlign];
        [kmTextLayer setUnderlineEnable:textLayer.textLayerProperty.useUnderline];
        [kmTextLayer setShadowAngle:textLayer.textLayerProperty.shadowAngle];
        [kmTextLayer setShadowDistance:textLayer.textLayerProperty.shadowDistance];
        [kmTextLayer setShadowSpread:textLayer.textLayerProperty.shadowSpread];
        [kmTextLayer setShadowSize:textLayer.textLayerProperty.shadowSize];
        [kmTextLayer setOutlineThickness:textLayer.textLayerProperty.outlineThickness];
        [kmTextLayer setGlowSpread:textLayer.textLayerProperty.glowSpread];
        [kmTextLayer setGlowSize:textLayer.textLayerProperty.glowSize];

        // TextLayer 경우, setText:Font api이후에 layer width, height를 받을 수 있다.
        [kmTextLayer setText:textLayer.getText Font:textLayer.getFont];
        
        textLayer.width = kmTextLayer.width;
        textLayer.height = kmTextLayer.height;
        
        CGSize size = self.layerManager.renderRegionSize;
        
        if(textLayer.layerPoint.x == CGPointNXELayerCenter.x) {
            textLayer.x = (size.width - textLayer.width) / 2;
        }
        if (textLayer.layerPoint.y ==  CGPointNXELayerCenter.y) {
            textLayer.y = (size.height - textLayer.height) / 2;
        }
        textLayer.layerPoint = CGPointMake(textLayer.x, textLayer.y);
        
        [textLayer syncPropertiesTo:kmTextLayer];
        
        //[kmTextLayer setKerningRatio:textLayer.textLayerProperty.kerningRatio];
        [kmTextLayer setTextColor:textLayer.textLayerProperty.textColor];
        [kmTextLayer setShadowEnable:textLayer.textLayerProperty.useShadow];
        [kmTextLayer setShadowColor:textLayer.textLayerProperty.shadowColor];
        [kmTextLayer setGlowEnable:textLayer.textLayerProperty.useGlow];
        [kmTextLayer setGlowColor:textLayer.textLayerProperty.glowColor];
        [kmTextLayer setOutlineEnable:textLayer.textLayerProperty.useOutline];
        [kmTextLayer setOutlineColor:textLayer.textLayerProperty.outlineColor];
        [kmTextLayer setBackgroundEnable:textLayer.textLayerProperty.useBackground];
        [kmTextLayer setBackgroundColor:textLayer.textLayerProperty.bgColor];
        
        textLayer.layerId = [self.layerManager addLayer:kmTextLayer];
        
        [self.nxeLayerManager addLayer:textLayer];
    }
    /* TODO
     else if(layer.layerType == NXE_LAYER_STICKER) {
    }*/
}

- (void)setLayerScaleImagePath:(NSString *)path
{//TODO
}

- (void)setLayerRotateImagePath:(NSString *)path
{//TODO
}

- (void)setLayerOutlineColor:(CGColorRef)color
{//TODO
}

@end

@implementation EditorResourceHolder(KMLayer)
#define WATERMARK_MARGIN_RIGHT  10
#define WATERMARK_MARGIN_TOP    10

- (KMLayerGeometry) layer:(KMLayer *) layer shouldUpdateGeometry:(BOOL *) should;
{
    KMLayerGeometry result = { 0, };
    if (self.listId4watermark != -1 && [layer isKindOfClass:KMWaterMark.class] ) {
        result.width = layer.width;
        result.height = layer.height;
        result.startTime = layer.startTime;
        result.endTime = layer.endTime;
        
        CGSize size = self.layerManager.renderRegionSize;
        result.y = WATERMARK_MARGIN_TOP;
        result.x = size.width - (layer.width + WATERMARK_MARGIN_RIGHT);
        *should = YES;
    }
    return result;
}

@end

@implementation NXELayer(KMLayer)
- (void) syncPropertiesTo:(KMLayer *)kmLayer
{
    kmLayer.x = self.x;
    kmLayer.y = self.y;
    kmLayer.width = self.width;
    kmLayer.height = self.height;
    kmLayer.startTime = self.startTime;
    kmLayer.endTime = self.endTime;
    kmLayer.angle = (CGFloat)self.angle;
    kmLayer.scale = (CGFloat)self.scale;
    kmLayer.hFlip = self.hFlip;
    kmLayer.vFlip = self.vFlip;
    kmLayer.brightness = self.brightness;
    kmLayer.contrast = self.contrast;
    kmLayer.saturation = self.saturation;
    kmLayer.alpha = self.alpha;
    kmLayer.scale = self.scale;
    
    [kmLayer setLut:(KMLutType)self.colorFilter];
    
    KMAnimationType inAnimatinType = self.inAnimation == NXE_ANIMATION_IN_NONE ? KMAnimationNone : KMAnimationNone + self.inAnimation;
    KMAnimationType outAnimatinType = self.outAnimation == NXE_ANIMATION_IN_NONE ? KMAnimationNone : KMAnimationOverallMax + self.outAnimation;
    KMAnimationType overallAnimationType = self.expression == NXE_EXPRESSION_NONE ? KMAnimationNone : KMAnimationInMax + self.expression;
    
    [kmLayer setInAnimationType:inAnimatinType WithDuration:self.inAnimationDuration];
    [kmLayer setOutAnimationType:outAnimatinType WithDuration:self.outAnimationDuration];
    [kmLayer setOverallAnimationType:overallAnimationType];
}

@end


NXEExportParams NXEExportParamsMake(int width, int height) {
    
#define DefaultFrameRate 30
    
    NXEExportParams params;
    
    params.width = width;
    params.height = height;
    params.framePerSec = DefaultFrameRate;
    params.bitPerSec = ((float)(width * height * params.framePerSec * 0.17) / 1024) * 1000;
    params.durationMs = 0;
    params.maxFileSizeByte = LONG_MAX;
    params.rotationAngle = NXE_ROTATION_0;

    return params;
}

@implementation NXEClip (Utils)
- (NSString *)getEncodedEffectoptions
{
    int showStartTime=0; int showEndTime=0;
    [self getEffectShowTime:&showStartTime endTime:&showEndTime];
    
    NSString *transitionOptions = [self encodeTransitionEffectOptions];
    NSString *clipOptions = [self encodeEffectOptions];
    
    NSString *ret = [NSString stringWithFormat:@"%d,%d?%@?%@", showStartTime, showEndTime, transitionOptions, clipOptions];
    
    return ret;
}

- (NSString *)encodeTransitionEffectOptions
{
    if([self.transitionEffectOptions count] != 0) {
        NSMutableString *result = [NSMutableString stringWithCapacity:1];
        BOOL first = true;
        
        for(NSString *key in [self.transitionEffectOptions allKeys]) {
            if(first) {
                first = false;
            } else {
                [result appendString:@"&"];
            }
            NSString *value = self.transitionEffectOptions[key];
            
            [result appendString:[key stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet letterCharacterSet]]];
            [result appendString:@"="];
            [result appendString:[value stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet letterCharacterSet]]];
        }
        return result;
    }
    return @"";
}

- (NSString *)encodeEffectOptions
{
    if([self.effectOptions count] != 0) {
        NSMutableString *result = [NSMutableString stringWithCapacity:1];
        BOOL first = true;
        
        for(NSString *key in [self.effectOptions allKeys]) {
            if(first) {
                first = false;
            } else {
                [result appendString:@"&"];
            }
            NSString *value = self.effectOptions[key];
            
            [result appendString:[key stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet letterCharacterSet]]];
            [result appendString:@"="];
            [result appendString:[value stringByAddingPercentEncodingWithAllowedCharacters:[NSCharacterSet letterCharacterSet]]];
        }
        return result;
    }
    return @"";
}
@end

@implementation NXEVisualClip (Utils)
- (void)setPropertyWithClip:(NXEClip *)clip
{
    self.clipType = clip.clipType;
    self.clipId = clip.videoInfo.clipId;
    self.totalAudioTime = clip.videoInfo.totalAudioTime;
    self.totalVideoTime = clip.videoInfo.totalVideoTime;
    self.totalTime = clip.videoInfo.totalTime;
    self.startTime = clip.vinfo.startTime;
    self.endTime = clip.videoInfo.endTime;
    self.startTrimTime = clip.videoInfo.startTrimTime;
    self.endTrimTime = clip.videoInfo.endTrimTime;
    self.width = clip.videoInfo.width;
    self.height = clip.videoInfo.height;
    self.existVideo = clip.videoInfo.existVideo;
    self.existAudio = clip.videoInfo.existAudio;
    self.title = [clip getEncodedEffectoptions];
    self.titleStyle = clip.videoInfo.titleStyle;
    self.titleStartTime = clip.videoInfo.titleStartTime;
    self.titleEndTime = clip.videoInfo.titleEndTime;
    
    self.audioOnOff = clip.videoInfo.audioOnOff;
    self.bGMVolume = clip.videoInfo.bGMVolume;
    self.clipVolume = clip.videoInfo.clipVolume;
    self.rotateState = clip.videoInfo.rotateState;
    
    self.brightness = [clip getCombinedBrightness];
    self.contrast = [clip getCombinedContrast];
    self.saturation = [clip getCombinedSaturation];
    self.tintcolor = clip.colorEffect.tintColor;
    self.speedControl = clip.videoInfo.speedControl;
    self.voiceChanger = clip.videoInfo.voiceChanger;
    
    self.effectType = clip.videoInfo.effectType;
    self.effectOffset = clip.videoInfo.effectOffset;
    self.effectOverlap = clip.videoInfo.effectOverlap;
    
    self.clipEffectID = clip.videoInfo.clipEffectID;
    if([self.clipEffectID isEqualToString:kEffectIDNone]) {
        self.effectDuration = 0;
    } else {
        self.effectDuration = clip.videoInfo.effectDuration;
    }
    self.titleEffectID = clip.videoInfo.titleEffectID;
    if([self.titleEffectID isEqualToString:kEffectIDNone]) {
        self.titleEffectID = nil;
    }
    
    self.startRect = clip.videoInfo.startRect;
    self.endRect = clip.videoInfo.endRect;
    self.destRect = clip.videoInfo.destRect;
    
    self.clipPath = clip.videoInfo.clipPath;
    self.thumbnailPath = clip.videoInfo.thumbnailPath;
    
    self.lut = clip.videoInfo.lut;
    self.vignette = clip.videoInfo.vignette;
    
    if([[clip.audioEnvelop getVolumeEnvelopeTimeList] count] != 0) {
        self.volumeEnvelopeTime = [clip.audioEnvelop getVolumeEnvelopeTimeList];
    } else {
        self.volumeEnvelopeTime = @[[NSNumber numberWithInt:0], [NSNumber numberWithInt:self.endTime - self.startTime]];
    }
    
    if([[clip.audioEnvelop getVolumeEnvelopeLevelList] count] != 0) {
        self.volumeEnvelopeLevel = [clip.audioEnvelop getVolumeEnvelopeLevelList];
    } else {
        self.volumeEnvelopeLevel = @[[NSNumber numberWithInt:100], [NSNumber numberWithInt:100]];
    }
    
    self.voiceChanger = clip.videoInfo.voiceChanger;
    self.pitch = clip.videoInfo.pitch;
    self.compressor = clip.videoInfo.compressor;
    self.panLeft = clip.videoInfo.panLeft;
    self.panRight = clip.videoInfo.panRight;
    self.musicEffect = clip.videoInfo.musicEffect;
    self.processorStrength = clip.videoInfo.processorStrength;
    self.bassStrength = clip.videoInfo.bassStrength;
    self.isMotionTrackedVideo = clip.videoInfo.isMotionTrackedVideo;
}
@end

@implementation NXEAudioClip (Utils)
- (void)setPropertyWithClip:(NXEClip *)clip projectTime:(int)projectTime
{
    NXEAudioClip *audioInfo = clip.audioInfo;
    self.clipId = audioInfo.clipId;
    self.clipPath = audioInfo.clipPath;
    self.totalTime = audioInfo.totalTime;
    self.startTime = audioInfo.startTime;
    
    if(clip.bLoop) {
        self.endTime = audioInfo.startTime + projectTime;
    } else {
        if(audioInfo.startTrimTime > 0 || audioInfo.endTrimTime > 0) {
            self.endTime = audioInfo.startTime + clip.trimEndTime - clip.headTrimDuration;
        } else {
            self.endTime = audioInfo.endTime;
        }
    }
    
    self.startTrimTime = audioInfo.startTrimTime;
    self.endTrimTime = audioInfo.endTrimTime;
    
    self.audioOnOff = audioInfo.audioOnOff;
    self.autoEnvelop = audioInfo.autoEnvelop;
    self.clipVolume = audioInfo.clipVolume;
    
    if([[clip.audioEnvelop getVolumeEnvelopeTimeList] count] != 0) {
        self.volumeEnvelopeTime = [clip.audioEnvelop getVolumeEnvelopeTimeList];
    } else {
        self.volumeEnvelopeTime = @[[NSNumber numberWithInt:0], [NSNumber numberWithInt:self.endTime - self.startTime]];
    }
    
    if([[clip.audioEnvelop getVolumeEnvelopeLevelList] count] != 0) {
        self.volumeEnvelopeLevel = [clip.audioEnvelop getVolumeEnvelopeLevelList];
    } else {
        self.volumeEnvelopeLevel = @[[NSNumber numberWithInt:100], [NSNumber numberWithInt:100]];
    }
    
    self.voiceChanger = audioInfo.voiceChanger;
    self.pitch = audioInfo.pitch;
    self.compressor = audioInfo.compressor;
    self.panLeft = audioInfo.panLeft;
    self.panRight = audioInfo.panRight;
    self.musicEffect = audioInfo.musicEffect;
    self.processorStrength = audioInfo.processorStrength;
    self.bassStrength = audioInfo.bassStrength;
}
@end
