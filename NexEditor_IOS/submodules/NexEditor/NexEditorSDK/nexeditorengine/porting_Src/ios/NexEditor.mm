/******************************************************************************
 * File Name   : NexEditor.mm
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
#import <AVFoundation/AVFoundation.h>
#import "NexEditor.h"
#import "NexEditorPrivate.h"
#import "NexEditor+AsyncAPI.h"
#import "NexSAL_Internal.h"
#import "NEXVIDEOEDITOR_Def.h"
#import "NXEVisualClip.h"
#import "NXEAudioClip.h"
#import "EditorEventListenerPrivate.h"
#import "EditorEventListener+EventDelegate.h"
#import "ObjCNexEditorEventHandler.h"
#import "NexClipInfoPrivate.h"
#import "NexProtection.h"
#import "NEXVIDEOEDITOR_Interface.h"
#import "NexLayer.h"
#import "porting_ios.h"
#import "NexEditorLog.h"
#import "NexDrawInfo.h"
#import "NexSceneInfo.h"
#import "NXEVisualClipInternal.h"
#include "CNexExportWriterStreamWriterAdapter.h"
#import "AVAssetStreamWriter.h"
#import "NexEditorUtil.h"

#import <stdlib.h>
#import <string.h>
#import <pthread.h>
#import <stdio.h>
#import <unistd.h>
#import <sys/time.h>
#import <string.h>
#import <sys/sysctl.h>

#define LOAD_THEME_PREVIEW 0x00000001
#define DEFAULT_EXPORT_AUDIO_BITRATE 128 * 1204

#define DEFAULT_EXPORT_PROFILE          0
#define DEFAULT_EXPORT_PROFILE_LEVEL    0

#define LOG_TAG @"NexEditor"

#define RETURN_ERROR_IF_EDITOR_NOT_VALID(p) if (p == NULL)    return NEXVIDEOEDITOR_ERROR_INVALID_STATE

static NexEditor *instance = nil;
static dispatch_once_t onceToken;

@interface NXEVisualClip(NexEditorEnginePrivate)
@property (nonatomic, readonly) CLIP_TYPE rawClipType;
@end

@interface NexEditor()
{
    INexVideoEditor* g_VideoEditorHandle;
    ObjCNexEditorEventHandler* g_VideoEditorEventHandler;

    BOOL isSeeking;
}
@property (nonatomic, retain) FastPreviewBuilder *fastPreviewBuilder;
@property (nonatomic, strong) EditorEventListener *editorEventListener;
@property (nonatomic) BOOL isAppActive;
@property (nonatomic) CNexExportWriterStreamWriterAdapter *exportWriter;
@property (nonatomic, strong) NSMutableArray *waitingEventBlocks;
@end

@implementation NexEditor

- (void)dealloc
{
    self.waitingEventBlocks = nil;
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidBecomeActiveNotification object:nil];
    self.editorEventListener = nil;
    delete g_VideoEditorEventHandler;
    
    //
    [self.fastPreviewBuilder release];
    
    [super dealloc];
}

#pragma mark - Class API
- (void)setDecoderCapacity
{
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char *machine = (char*)malloc(size);
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString *identifier = [NSString stringWithUTF8String:machine];
    free(machine);

    CapacityInfo info = [NexEditorUtil getAvailabilityOfHardwareResourceWithIdentifier:identifier];
    
    [self setProperty:self.kDecoderMaxCount Value:[NSString stringWithFormat:@"%u", info.uMaxDecoderCount]];
    [self setProperty:self.kDecoderMaxMemorySize Value:[NSString stringWithFormat:@"%u", info.uMaxMemorySize]];
}

+ (NexEditor*)sharedInstance
{
    dispatch_once(&onceToken, ^{
        if(instance == nil) {
            instance = [[NexEditor alloc] init];
        }
    });
    return instance;
}

+ (id<NexEditorAsyncAPI>) asyncAPI
{
    return self.sharedInstance.asyncAPI;
}

#pragma mark -

- (id)init
{    
    self = [super init];
    if(self) {
        NSString *appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:(id)kCFBundleIdentifierKey];
        setPackageName4Protection([appName UTF8String]);
        if(checkSDKProtection()) {
            NSException *e = [NSException exceptionWithName:@"Protection Exception"
                                                     reason:[NSString stringWithFormat:@"Time-out or Mismatch bundle identifier:%@", appName]
                                                   userInfo:nil];
            @throw e;
        }
        
        [self setUserData:0];
        [self createEditor];
        [self setLayer];
        
        [self setDecoderCapacity];
        //
        self.fastPreviewBuilder = [[[FastPreviewBuilder alloc] init] autorelease];
        
        self.isAppActive = YES;
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidChangeActiveState:) name:UIApplicationWillResignActiveNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidChangeActiveState:) name:UIApplicationDidBecomeActiveNotification object:nil];
        
        self.waitingEventBlocks = [NSMutableArray array];
    }
    return self;
}

#pragma mark - Properties
- (id<NexEditorAsyncAPI>) asyncAPI
{
    return self;
}

- (NSString *)kDecoderMaxCount
{
    return @"HardWareDecMaxCount";
}

- (NSString *)kDecoderMaxMemorySize
{
    return @"HardWareCodecMemSize";
}

#pragma mark -
- (void)applicationDidChangeActiveState:(NSNotification *)notif {
    BOOL active = NO;
    
    if ([notif.name isEqualToString:UIApplicationWillResignActiveNotification]) {
        active = NO;
    } else if ([notif.name isEqualToString:UIApplicationDidBecomeActiveNotification]) {
        active = YES;
    } else {
        return;
    }
    self.isAppActive = active;
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "NESI-533 [NexEditor.mm %d] Active state changed: %s", __LINE__, active ? "ACTIVE" : "Inactive");
}

- (int)setLayerWithRect:(int)x
                      y:(int)y
                  width:(int)width
                 height:(int)height
                  scale:(int)scale
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->setLayerWithRect(x, y, width, height,scale);
}

- (int)setAudioClip:(NXEAudioClip*)pAudioClip
           IClipItem:(IClipItem*)pClip
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    if( pClip == NULL ) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] Invalid Clip handle", __LINE__);
        return 1;
    }
    
    pClip->setClipType(CLIPTYPE_AUDIO);
    pClip->setTotalTime(pAudioClip.totalTime);
    pClip->setStartTime(pAudioClip.startTime);
    pClip->setEndTime(pAudioClip.endTime);
    pClip->setStartTrimTime(pAudioClip.startTrimTime);
    pClip->setEndTrimTime(pAudioClip.endTrimTime);
    
    NSString* clipPath = pAudioClip.clipPath;
    if(clipPath) {
        const char *str = [clipPath UTF8String];
        if( str == NULL ) {
            return 1;
        }
        pClip->setClipPath((char*)str);
    }
    
    pClip->setAudioOnOff(pAudioClip.audioOnOff);
    pClip->setAutoEnvelop(pAudioClip.autoEnvelop);
    pClip->setAudioVolume(pAudioClip.clipVolume);
    pClip->setVoiceChangerFactor(pAudioClip.voiceChanger);
    pClip->setPanLeftFactor(pAudioClip.panLeft);
    pClip->setPanRightFactor(pAudioClip.panRight);
    pClip->setCompressorFactor(pAudioClip.compressor);
    pClip->setPitchFactor(pAudioClip.pitch);
    
    int* pVolumeEnvelopLevel = nil;
    int* pVolumeEnvelopTime = nil;
    int nVolumeEnvelopeSize = 0;
    
    NSArray* arrVolumeEnvelopeLevel = pAudioClip.volumeEnvelopeLevel;
    if(arrVolumeEnvelopeLevel != nil) {
        int arrSize = (int)[arrVolumeEnvelopeLevel count];
        pVolumeEnvelopLevel = (int*)malloc(arrSize * sizeof(int));
        memset(pVolumeEnvelopLevel, 0x00, arrSize * sizeof(int));
        
        for(int i = 0; i < arrSize; i++) {
            pVolumeEnvelopLevel[i] = [[arrVolumeEnvelopeLevel objectAtIndex:i] intValue];
        }
    }
    
    NSArray* arrVolumeEnvelopeTime = pAudioClip.volumeEnvelopeTime;
    if( arrVolumeEnvelopeTime != nil) {
        int arrSize = (int)[arrVolumeEnvelopeTime count];
        pVolumeEnvelopTime = (int*)malloc(arrSize * sizeof(int));
        memset(pVolumeEnvelopTime, 0x00, arrSize * sizeof(int));
        for(int i = 0; i < arrSize; i++) {
            pVolumeEnvelopTime[i] = [[arrVolumeEnvelopeTime objectAtIndex:i] intValue];
        }
    }
    
    nVolumeEnvelopeSize = (int)[arrVolumeEnvelopeLevel count];
    
    if(nVolumeEnvelopeSize > 0 && pVolumeEnvelopTime != nil && pVolumeEnvelopLevel != nil)
        pClip->setAudioEnvelop(nVolumeEnvelopeSize, (unsigned int*)pVolumeEnvelopTime, (unsigned int*)pVolumeEnvelopLevel);
    
    free(pVolumeEnvelopTime);
    free(pVolumeEnvelopLevel);
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] setAudioClip Done(Clip ID : %d)", __LINE__, pClip->getClipID());
    return 0;
}

- (int)setVisualClip:(NXEVisualClip*)pVisualClip
            IClipItem:(IClipItem*)pClip
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    if(pClip == nil) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] Invalid Clip handle", __LINE__);
        return 1;
    }
    
    // clipType is member in NexClip
    pClip->setClipType((CLIP_TYPE)pVisualClip.rawClipType);
    pClip->setTotalTime(pVisualClip.totalTime);
    pClip->setStartTime(pVisualClip.startTime);
    pClip->setEndTime(pVisualClip.endTime);
    pClip->setStartTrimTime(pVisualClip.startTrimTime);
    pClip->setEndTrimTime(pVisualClip.endTrimTime);
    pClip->setWidth(pVisualClip.width);
    pClip->setHeight(pVisualClip.height);
    pClip->setVideoExist(pVisualClip.existVideo);
    pClip->setAudioExist(pVisualClip.existAudio);
    pClip->setMotionTracked(pVisualClip.isMotionTrackedVideo ? (int)1 : (int)0);
    
    NSString* clipPath = pVisualClip.clipPath;
    if(clipPath) {
        const char *str = [clipPath UTF8String];
        if(str == nil) {
            NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] Clip Path is NULL", __LINE__);
            return 1;
        }
        pClip->setClipPath((char*)str);
    }
    
    NSString* thumbnailPath = pVisualClip.thumbnailPath;
    if(thumbnailPath) {
        const char *str = [thumbnailPath UTF8String];
        if(str == nil) {
            return 1;
        }
        pClip->setThumbnailPath((const char*)str);
    }
    
    pClip->setTitleStyle(pVisualClip.titleStyle);
    pClip->setTitleStartTime(pVisualClip.titleStartTime);
    pClip->setTitleEndTime(pVisualClip.titleEndTime);
    
    NSString* title = pVisualClip.title;
    if(title) {
        const char *str = [title UTF8String];
        if(str == nil) {
            return 1;
        }
        pClip->setTitle((char*)str);
    }
    
    pClip->setAudioOnOff(pVisualClip.audioOnOff);
    pClip->setAudioVolume(pVisualClip.clipVolume);
    pClip->setBGMVolume(pVisualClip.bGMVolume);
    
    int* pVolumeEnvelopLevel = NULL;
    int nVolumeEnvelopeSize = 0;
    int* pVolumeEnvelopTime = NULL;
    
    NSArray* arrVolumeEnvelopeLevel = pVisualClip.volumeEnvelopeLevel;
    if(arrVolumeEnvelopeLevel != NULL) {
        int arrSize = (int)[arrVolumeEnvelopeLevel count];
        pVolumeEnvelopLevel = (int*)malloc(arrSize * sizeof(int));
        memset(pVolumeEnvelopLevel, 0x00, arrSize * sizeof(int));
        
        for(int i = 0; i < arrSize; i++) {
            pVolumeEnvelopLevel[i] = (int)[[arrVolumeEnvelopeLevel objectAtIndex:i] intValue];
        }
    }
    
    NSArray* arrVolumeEnvelopeTime = pVisualClip.volumeEnvelopeTime;
    if( arrVolumeEnvelopeTime != NULL) {
        int arrSize = (int)[arrVolumeEnvelopeTime count];
        pVolumeEnvelopTime = (int*)malloc(arrSize * sizeof(int));
        memset(pVolumeEnvelopTime, 0x00, arrSize * sizeof(int));
        
        for(int i = 0; i < arrSize; i++) {
            pVolumeEnvelopTime[i] = (int)[[arrVolumeEnvelopeTime objectAtIndex:i] intValue];
        }
    }
    
    nVolumeEnvelopeSize = (int)[arrVolumeEnvelopeLevel count];
    
    if(nVolumeEnvelopeSize > 0 && pVolumeEnvelopTime != NULL && pVolumeEnvelopLevel != NULL) {
        pClip->setAudioEnvelop(nVolumeEnvelopeSize, (unsigned int*)pVolumeEnvelopTime, (unsigned int*)pVolumeEnvelopLevel);
    }
    if(pVolumeEnvelopTime != NULL) free(pVolumeEnvelopTime);
    if(pVolumeEnvelopLevel != NULL) free(pVolumeEnvelopLevel);

    pClip->setClipEffectDuration(pVisualClip.effectDuration);
    pClip->setClipEffectOffset(pVisualClip.effectOffset);
    pClip->setClipEffectOverlap(pVisualClip.effectOverlap);
    pClip->setRotateState(pVisualClip.rotateState);
    // LOGI("[NexEditor.mm %d] Visual clip Rotate State(%d)", __LINE__, iRotate);
    
    pClip->setBrightness(pVisualClip.brightness);
    pClip->setContrast(pVisualClip.contrast);
    pClip->setSaturation(pVisualClip.saturation);
    pClip->setLUT(pVisualClip.lut);
    pClip->setVignette(pVisualClip.vignette);
    pClip->setTintcolor(pVisualClip.tintcolor);
    pClip->setSpeedCtlFactor(pVisualClip.speedControl);
    pClip->setVoiceChangerFactor(pVisualClip.voiceChanger);
    pClip->setPanLeftFactor(pVisualClip.panLeft);
    pClip->setPanRightFactor(pVisualClip.panRight);
    pClip->setCompressorFactor(pVisualClip.compressor);
    pClip->setPitchFactor(pVisualClip.pitch);
    pClip->setIframePlay(pVisualClip.useIFrameOnly?1:0);
    pClip->setKeepPitch(pVisualClip.keepPitch?1:0);
    
    NSString* clipEffectID = pVisualClip.clipEffectID;
    if(clipEffectID) {
        const char *str = [clipEffectID UTF8String];
        if(str == nil) {
            return 1;
        }
        
        pClip->setClipEffectID((char*)str);
        // LOGI("[NexEditor.mm %d] setVisualClip ClipEffectID(%s)", __LINE__, str);
    }
    
    NSString* titleEffectID = pVisualClip.titleEffectID;
    if(titleEffectID) {
        const char *str = [titleEffectID UTF8String];
        if(str == nil) {
            return 1;
        }
        
        pClip->setTitleEffectID((char*)str);
        // LOGI("[NexEditor.mm %d] setVisualClip TitleEffectID(%s)", __LINE__, str);
    }
    
    int iLeft   = 0;
    int iTop	= 0;
    int iRight	= 0;
    int iBottom	= 0;
    
    iLeft = pVisualClip.startRect.origin.x;
    iTop = pVisualClip.startRect.origin.y;
    iRight = iLeft + pVisualClip.startRect.size.width;
    iBottom = iTop + pVisualClip.startRect.size.height;
    
    IRectangle* pRect1 = pClip->getStartPosition();
    if(pRect1) {
        pRect1->setRect(iLeft, iTop, iRight, iBottom);
        pRect1->Release();
    }
    
    iLeft = pVisualClip.endRect.origin.x;
    iTop = pVisualClip.endRect.origin.y;
    iRight = iLeft + pVisualClip.endRect.size.width;
    iBottom = iTop + pVisualClip.endRect.size.height;
    
    IRectangle* pRect2 = pClip->getEndPosition();
    if(pRect2) {
        pRect2->setRect(iLeft, iTop, iRight, iBottom);
        pRect2->Release();
    }
    
    iLeft = pVisualClip.destRect.origin.x;
    iTop = pVisualClip.destRect.origin.y;
    iRight = pVisualClip.destRect.size.width;
    iBottom = pVisualClip.destRect.size.height;
    
    IRectangle* pRect3 = pClip->getDstPosition();
    if(pRect3) {
        pRect3->setRect(iLeft, iTop, iRight, iBottom);
        pRect3->Release();
    }
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] setVisualClip Done(Clip ID : %d)", __LINE__, pClip->getClipID());
    return 0;
}

- (int)getDuration
{
    if(g_VideoEditorHandle == NULL) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] Invalid VideoEditor handle", __LINE__);
        return 0;
    }
    return (int)(g_VideoEditorHandle->getDuration());
}

/**----------------------------------------------------------------------
 *                    porting layer functions                           *
 -----------------------------------------------------------------------*/
#pragma mark - porting layer functions

// function list in excel
- (NexEditorError)loadClipsAsync:(NSArray *)visualClips
           audioClips:(NSArray*)audioClips
{
    return [self _loadVisualClips:visualClips audioClips:audioClips setupEventWaiter:NULL];
}

#pragma mark - Private: Clip Loading
- (NexEditorError)_loadVisualClips:(NSArray*)visualClips
                        audioClips:(NSArray*)audioClips
                  setupEventWaiter:(void(^)(NexEditor *editor)) setupEventWaiter
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm  %d] ayncLoadList start", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    int ret = 0;
    IClipList* pClipList = g_VideoEditorHandle->createClipList();
    if(pClipList == nil) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm  %d] GetClipList failed", __LINE__);
        return NEXVIDEOEDITOR_ERROR_UNKNOWN;
    }
    
    if (setupEventWaiter) setupEventWaiter(self);

    pClipList->lockClipList();
    pClipList->clearClipList();
    
    if(visualClips == nil) {
        pClipList->unlockClipList();
        SAFE_RELEASE(pClipList);
        int iRet = g_VideoEditorHandle->updateClipList(pClipList, NO);
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] loadClipList(Cliplist clear because Visual Clip array is null)", __LINE__);
        return iRet;
    }
    
    int iVisualClipArrayCount = (int)[visualClips count];
    for(int i = 0; i < iVisualClipArrayCount; i++) {
        NXEVisualClip* visualClip = [visualClips objectAtIndex:i];
        
        if(visualClip == nil) {
            continue;
        }
        
        int iClipID = visualClip.clipId;
        
        IClipItem* pItem = pClipList->createEmptyClipUsingID(iClipID);
        if(pItem == nil) {
            continue;
        }
        
        [self setVisualClip:visualClip IClipItem:pItem];
        pClipList->addClipItem(pItem);
        pItem->printClipInfo();
        pItem->Release();
    }

    if(audioClips == nil || [audioClips count] == 0) {
        pClipList->unlockClipList();
        int iRet = g_VideoEditorHandle->updateClipList(pClipList, NO);
        SAFE_RELEASE(pClipList);
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] loadClipList End(%d)", __LINE__, iRet);
        return iRet;
    }
    
    NXEAudioClip* audioClip = [audioClips objectAtIndex:0];
    if(audioClip == nil) {
        pClipList->unlockClipList();
        int iRet = g_VideoEditorHandle->updateClipList(pClipList, NO);
        SAFE_RELEASE(pClipList);
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] loadClipList End(%d)", __LINE__, iRet);
        return iRet;
    }
    
    int iAudioClipArrayCount = (int)[audioClips count];
    for(int i = 0; i < iAudioClipArrayCount; i++) {
        NXEAudioClip* audioClip = (NXEAudioClip*)[audioClips objectAtIndex:i];
        if(audioClip == nil) {
            continue;
        }
        
        int iClipID = audioClip.clipId;
        
        IClipItem* pItem = (IClipItem*)pClipList->createEmptyClipUsingID(iClipID);
        if(pItem == nil) {
            continue;
        }
        
        [self setAudioClip:audioClip IClipItem:pItem];
        
        pClipList->addClipItem(pItem);
        pItem->printClipInfo();
        pItem->Release();
        
    }
    
    pClipList->unlockClipList();
    
    int iRet = g_VideoEditorHandle->updateClipList(pClipList, NO);
    SAFE_RELEASE(pClipList);
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] ayncLoadList End(%d)", __LINE__, iRet);
    return ret;
}

#pragma mark -
- (NSInteger) findCodecTypes:(int *)codecTypes withCodecType:(NEX_CODEC_TYPE)codecType loopCount:(int)loopCount
{
    for ( int loopid = 0; loopid < loopCount; loopid++ ) {
        NEX_CODEC_TYPE type = (NEX_CODEC_TYPE)(*(codecTypes + loopid));
        if( codecType == type ) {
            return loopid;
        }
    }
    return NSNotFound;
}

- (BOOL) isSupportedCodecType:(NEX_CODEC_TYPE)codecType isVideo:(BOOL)isVideo isDecoder:(BOOL)isDecoder
{
    int *codecTypes = nil;
    int codecTypesCount = 0;
    
    if ( isDecoder ) {
        codecTypes = getSupportedDecoderCodecTypes(isVideo);
        codecTypesCount = countSupportedDecoderCodecTypes(isVideo);
    } else {
        codecTypes = getSupportedEncoderCodecTypes(isVideo);
        codecTypesCount = countSupportedEncoderCodecTypes(isVideo);
    }
    
    BOOL result = ( [self findCodecTypes:codecTypes withCodecType:codecType loopCount:codecTypesCount] != NSNotFound );
    if (result) {
        // NESI-515 Check HEVC availability by OS version
        if (codecType == eNEX_CODEC_V_HEVC && isVideo && isDecoder) {
            BOOL isHEVCSupported = NO;
#if defined(__IPHONE_11_0) && (__IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0)
            if (@available(iOS 11.0, *)) {
                isHEVCSupported = YES;
            }
#endif
            result = isHEVCSupported;
        }
    }
    return result;
}

static NSString* IPodLibraryPrefix = @"ipod-library://";

- (int)getClipInfoSync:(NSString*)path
            NexClipInfo:(NexClipInfo*)info
       includeSeekTable:(bool)includeSeekTable
                userTag:(int)userTag
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] getClipInfo_Sync(iFlag:%d Tag:%d)", __LINE__, includeSeekTable, userTag);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);

    const char *strclip = [path UTF8String];
    if( strclip == NULL ) {
        return NEXVIDEOEDITOR_ERROR_NONE;
    }
    
    int iRet = 0;
    IClipInfo* pClipInfo = g_VideoEditorHandle->getClipInfo_Sync((char*)strclip, includeSeekTable, userTag, &iRet);
    
    if( pClipInfo == NULL ) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] getClipInfo_Sync error(Tag : %d iRet : %d)", __LINE__, includeSeekTable, iRet);
        return iRet;
    }
    
    if(pClipInfo->existVideo()) {
        info.existVideo = pClipInfo->existVideo();
        info.videoWidth = pClipInfo->getDisplayWidth();
        info.videoHeight = pClipInfo->getDisplayHeight();
        info.fps = pClipInfo->getVideoFPS();
        info.videoH264Profile = pClipInfo->getVideoH264Profile();
        info.videoH264Level = pClipInfo->getVideoH264Level();
        info.videoOrientation = pClipInfo->getVideoOrientation();
        info.videoBitRate = pClipInfo->getVideoBitRate();
        info.videoCodecType = (NEX_CODEC_TYPE)pClipInfo->getVideoCodecType();
    }
    
    if(pClipInfo->existAudio()) {
        info.existAudio = pClipInfo->existAudio();
        info.audioSampleRate = pClipInfo->getAudioSampleRate();
        info.audioChannels = pClipInfo->getAudioChannels();
        info.audioBitRate = pClipInfo->getAudioBitRate();
        info.audioCodecType = (NEX_CODEC_TYPE)pClipInfo->getAudioCodecType();
        if([path containsString:IPodLibraryPrefix]) {
            ExtAudioFileRef audioFile;
            ExtAudioFileOpenURL((CFURLRef)[NSURL URLWithString:path], &audioFile);
            AudioStreamBasicDescription audioFormat;
            UInt32 uiSize = sizeof(audioFormat);
            ExtAudioFileGetProperty(audioFile, kExtAudioFileProperty_FileDataFormat, &uiSize, &audioFormat);
            info.audioSampleRate = audioFormat.mSampleRate;
            info.audioChannels = audioFormat.mChannelsPerFrame;
        }
    }
    
    info.audioDuration = pClipInfo->getClipAudioDuration();
    info.videoDuration = pClipInfo->getClipVideoDuration();
    info.seekPointCount = pClipInfo->getSeekPointCount();
    
    if(strlen(pClipInfo->getThumbnailPath()) > 0) {
        NSString* strPath = [NSString stringWithUTF8String:pClipInfo->getThumbnailPath()];
        if(strPath == NULL) {
            SAFE_RELEASE(pClipInfo);
            return NEXVIDEOEDITOR_ERROR_UNKNOWN;
        }
        
        info.thumbnailPath = strPath;
    }
    
    if((includeSeekTable & GET_CLIPINFO_INCLUDE_SEEKTABLE) ==  GET_CLIPINFO_INCLUDE_SEEKTABLE)
    {
        int count = pClipInfo->getSeekTableCount();
        NSMutableArray* arr = [NSMutableArray array];
        
        for(int i = 0; i < count; i++) {
            [arr addObject:@(pClipInfo->getSeekTableValue(i))];
        }
        
        info.seekTable = [[arr copy] autorelease];
    }
    
    SAFE_RELEASE(pClipInfo);
    return NEXVIDEOEDITOR_ERROR_NONE;
}

- (NexEditorError)setTime:(int)time display:(int)display idrFrame:(int)idrFrame
{
    NexEditorSetTimeOptions options = 0;
    if (display) {
        options |= NexEditorSetTimeOptionsDisplay;
    }
    if (idrFrame) {
        options |= NexEditorSetTimeOptionsIDRFrameOnly;
    }
    
    return [self.asyncAPI setTime:CMTimeMake(time, 1000) options:options complete:NULL];
}

- (int)startPlay:(int)muteAudio
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] startPlay muteAudio(%d)", __LINE__, muteAudio);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->startPlay(muteAudio);
}

- (int)encodeProject:(NSString*)path
                Width:(int)width
               Height:(int)height
              Bitrate:(int)bitrate
          MaxFileSize:(long)maxFileSize
      ProjectDuration:(int)projectDuration
                  FPS:(int)frameRateBase100
         ProjectWidth:(int)projectWidth
       ProjectHeight:(int)projectHeight
        SamplingRate:(int)samplingRate
                Flag:(int)flag
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] encodeProject start", __LINE__);
    if(g_VideoEditorHandle == NULL || self.exportWriter == NULL) {
        return NEXVIDEOEDITOR_ERROR_INVALID_STATE;
    }
    
    NSURL *fileURL = [NSURL fileURLWithPath:path];
    StreamWriterSettings *settings = [StreamWriterSettings settingsWithFileURL:fileURL width:width height:height];
    /*
     * CNexVideoEditor provides StreamWriter with
     * - Degrees converted from 'flag'
     * - Audio bitrate (if '0' was given)
     */
    [settings updateVideoInfo:^(StreamWriterVideoInfo *videoInfo) {
        videoInfo->bitrate = bitrate;
        videoInfo->frameRate = (float) frameRateBase100 / 100.0;
    }];
    
    const int audioBitrate = 0;
    [settings updateAudioInfo:^(StreamWriterAudioInfo *audioInfo) {
        audioInfo->samplingRate = samplingRate;
        audioInfo->bitrate = audioBitrate;
    }];
    settings.durationMs = projectDuration;
    
    id<StreamWriter> writer = [AVAssetStreamWriter writerWithSettings:settings];
    self.exportWriter->setStreamWriterContext(writer.context);
    
    return g_VideoEditorHandle->encodeProject((char*)[path UTF8String], width, height, bitrate, maxFileSize, projectDuration, frameRateBase100, projectWidth, projectHeight, samplingRate, audioBitrate, DEFAULT_EXPORT_PROFILE, DEFAULT_EXPORT_PROFILE_LEVEL, EDITOR_DEFAULT_CODEC_TYPE, flag);
}
- (void) adjustProjectColorWith:(float)brightness contrast:(float)contrast saturation:(float)saturation;
{
    if(g_VideoEditorHandle != nil) {
        int brightness_ = (int)(brightness * 255);
        int contrast_ = (int)(contrast * 255);
        int saturation_ = (int)(saturation * 255);

        g_VideoEditorHandle->setBrightness(brightness_);
        g_VideoEditorHandle->setContrast(contrast_);
        g_VideoEditorHandle->setSaturation(saturation_);
    }
}

- (int)loadTheme:(NSString*)data flag:(Boolean)flag
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] setThemeData", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    if(data == nil) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] data is nil", __LINE__);
        return 1;
    }
    
    const char* statData = [data UTF8String];
    return g_VideoEditorHandle->loadTheme(statData, flag);
}

- (int)setProjectEffect:(NSString *)projectEffect
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] setProjectEffect", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    const char* pProjectEffect = [projectEffect UTF8String];
    
    int iRet = g_VideoEditorHandle->setProjectEffect((char*)pProjectEffect);
    return iRet;
}

- (int)createProject
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] createProject", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    if (self.exportWriter) {
        return NEXVIDEOEDITOR_ERROR_INVALID_STATE;
    }
    self.exportWriter = new CNexExportWriterStreamWriterAdapter();
    return g_VideoEditorHandle->createProject(self.exportWriter);
}

- (int)closeProject
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] closeProject", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    return g_VideoEditorHandle->closeProject();
}

- (int)destoryEditor
{
    instance = nil;
    onceToken = 0;
    
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] destroyEditor", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);

    // CNexVideoEditor and it's subsystem will delete the export writer.
    self.exportWriter = NULL;
    // NESI-104 ?�슈 ?�정
    delete g_VideoEditorHandle;
    g_VideoEditorHandle = NULL;
    //
    [self release];
    
    return 0;
}

- (int)commandMarker:(int)iTag
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] commandMarker", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    g_VideoEditorHandle->commandMarker(iTag);
    return 0;
}

- (int)clearTrackCache
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] clearTrackCache", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->clearTrackCache();
}

- (int)clearScreen:(int)tag
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    g_VideoEditorHandle->clearScreen(tag);
    return 0;
}

- (int)stopPlay
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] stopPlay", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->stopPlay(0);
}

- (int)setUserData:(int)userData
{
    // TODO. mj
    return NEXVIDEOEDITOR_ERROR_NONE;
}

- (int)setLayer
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    g_VideoEditorHandle->getThemeProperty(1, (void**)&_layer);
    if(self.layer == nil) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] g_View is nil", __LINE__);
        return NEXVIDEOEDITOR_ERROR_GENERAL;
    }
    return NEXVIDEOEDITOR_ERROR_NONE;
}

- (int)createEditor
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] createEditor", __LINE__);
    g_VideoEditorHandle = CreateNexVideoEditor(0, 0, (char*)"libPath", (char*)"modelName", 1, 1, NULL, NULL, NULL);
    
    if(g_VideoEditorHandle == NULL) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] createEditor error", __LINE__);
        return NEXVIDEOEDITOR_ERROR_GENERAL;
    }
    
    self.editorEventListener = [[[EditorEventListener alloc] init] autorelease];
    __block NexEditor *me = self;
    self.editorEventListener.onEvent = ^(NexEditorEvent event) {
        return [me didReceiveEditorEvent:event];
    };

    g_VideoEditorEventHandler = new ObjCNexEditorEventHandler(self.editorEventListener);
    g_VideoEditorHandle->setEventHandler(g_VideoEditorEventHandler);
    g_VideoEditorHandle->setThumbnailRoutine(2); // original 진행
    
    // Android?�서 ?�용?�고 ?�는 값들??기�??�로 ?�정?��??�니??
    // ....???
    //
    [self setProperty:@"canUseSoftwareCodec" Value:@"false"];
    [self setProperty:@"canUseMCSoftwareCodec" Value:@"false"];
    [self setProperty:@"AudioMultiChannelOut" Value:@"0"];
    [self setProperty:@"SupportFrameTimeChecker" Value:@"1"];
    [self setProperty:@"CreationTime" Value:@"0"];
    [self setProperty:@"MCHWAVCDecBaselineLevel" Value:@"40"];
    [self setProperty:@"MCHWAVCDecMainLevel" Value:@"40"];
    [self setProperty:@"MCHWAVCDecHighLevel" Value:@"40"];
    [self setProperty:@"MCHWAVCDecBaselineLevelSize" Value:@"2097152"];
    [self setProperty:@"MCHWAVCDecMainLevelSize" Value:@"2097152"];
    [self setProperty:@"MCHWAVCDecHighLevelSize" Value:@"2097152"];
    [self setProperty:@"MCHWAVCEncBaselineLevel" Value:@"0"];
    [self setProperty:@"MCHWAVCEncMainLevel" Value:@"0"];
    [self setProperty:@"MCHWAVCEncHighLevel" Value:@"0"];
    [self setProperty:@"MCHWAVCEncBaselineLevelSize" Value:@"0"];
    [self setProperty:@"MCHWAVCEncMainLevelSize" Value:@"0"];
    [self setProperty:@"MCHWAVCEncHighLevelSize" Value:@"0"];
    [self setProperty:@"HardWareCodecMemSize" Value:@"176947200"];
    [self setProperty:@"HardWareDecMaxCount" Value:@"20"];
    [self setProperty:@"HardWareEncMaxCount" Value:@"1"];
    [self setProperty:@"FeatureVersion" Value:@"3"];
    [self setProperty:@"useNexEditorSDK" Value:@"1"];
    [self setProperty:@"DeviceExtendMode" Value:@"1"];
    [self setProperty:@"InputMaxFPS" Value:@"120"];
    [self setProperty:@"JpegMaxWidthFactor" Value:@"2048"];
    [self setProperty:@"JpegMaxHeightFactor" Value:@"2048"];
    [self setProperty:@"JpegMaxSizeFactor" Value:@"4194304"];
    [self setProperty:@"SupportedMaxFPS" Value:@"240"];
    [self setProperty:@"supportContentDuration" Value:@"100"];
//    [self setProperty:@"UseAndroidJPEG" Value:@"1"];
    //

    return NEXVIDEOEDITOR_ERROR_NONE;
}

- (EditorEventListener *)getEditorEventHandler
{
    return self.editorEventListener;
}

- (void) releaseLUTTexture:(int)lut_resource_id
{
    if(g_VideoEditorHandle != nil) {
        g_VideoEditorHandle->releaseLUTTexture(lut_resource_id);
    }
}

- (NSString*)getProperty:(NSString*)name
{
    char strValue[93] = {0, };
    if( g_VideoEditorHandle != nil ) {
        g_VideoEditorHandle->getProperty((char*)[name UTF8String], strValue);
        return [NSString stringWithFormat:@"%s", strValue];
    }
    return nil;
}

- (int)setProperty:(NSString*)name Value:(NSString*)value
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    const char* strName = [name UTF8String];
    if(strName == NULL) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] setProperty error -> invalid parameter(name)", __LINE__);
        return 1;
    }
    
    const char* strValue = [value UTF8String];
    if(strValue == NULL) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] setProperty error -> invalid parameter(value)", __LINE__);
        return 1;
    }
    
    g_VideoEditorHandle->setProperty(strName, strValue);
    return 0;
}

- (int)getClipVideoThumbs:(NSString*)clipPath
             ThumbnailPath:(NSString*)thumbnailPath
                     Width:(int)width
                    Height:(int)height
                 StartTime:(int)startTime
                   EndTime:(int)endTime
                     Count:(int)count
                      Flag:(int)flag
                  UserFlag:(int)userFlag
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    if(clipPath == nil) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] clipPath is NULL", __LINE__);
        return 1;
    }
    
    if(thumbnailPath == nil) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] thumbnailPath is NULL", __LINE__);
        return 1;
    }
    
    return g_VideoEditorHandle->getClipVideoThumb((char*)[clipPath UTF8String], (char*)[thumbnailPath UTF8String], width, height, startTime, endTime, count, flag, userFlag);
}

- (int)captureCurrentFrame
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] captureCurrentFrame", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    g_VideoEditorHandle->captureCurrentFrame();
    return 0;
}

- (int)startVoiceRecorder:(NSString*) filePath SampleRate:(int)sampleRate Channels:(int)channels BitForSample:(int)bitForSample
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    const char* str = [filePath UTF8String];
    if(str == nil) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] filePath is NULL", __LINE__);
        return 1;
    }
    
    return g_VideoEditorHandle->startVoiceRecorder((char*)str, sampleRate, channels, bitForSample);
}

- (int)processVoiceRecoder:(Byte[])arrayPCM PCMLen:(int)pcmLen
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] processVoiceRecoder In PcmLen(%d)", __LINE__, pcmLen);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    if(arrayPCM == nil || pcmLen <= 0) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] processVoiceRecoder failed because arrayPCM is null(0x%x %d)", __LINE__, arrayPCM, pcmLen);
        return -1;
    }
    
    
    return g_VideoEditorHandle->processVoiceRecorder((int)pcmLen, (unsigned char*)arrayPCM);
}

- (int)endVoiceRecorder:(NexClipInfo*)clipInfo
{
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] endVoiceRecorder", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    IClipInfo* pClipInfo = g_VideoEditorHandle->endVoiceRecorder();
    if(pClipInfo == NULL) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_ERR, 0, "[NexEditor.mm %d] endVoiceRecorder failed because stopVoiceRecoder is failed", __LINE__);
        return -1;
    }
    
    clipInfo.existAudio = pClipInfo->existAudio();
    clipInfo.audioDuration = pClipInfo->getClipAudioDuration();
    pClipInfo->Release();
    return 0;
}

- (NexEditorError) fastOptionPreview:(NSString*)option display:(int)display
{
    return [self fastPreviewWithString:option display:display ? YES : NO];
}

- (NexEditorError) fastPreviewWithString:(NSString*)option display:(BOOL)display
{
    if (!self.isAppActive) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_WARNING, 0, "[NexEditor.mm %d] fastOptionPreview discarded due to applicate state: %s", __LINE__, self.isAppActive ? "Active" : "Inactive");
        return NEXVIDEOEDITOR_ERROR_INVALID_STATE;
    }
    NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] fastOptionPreview", __LINE__);
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    const char *str = [option UTF8String];
    int iRet = g_VideoEditorHandle->fastOptionPreview(str, display ? 1 : 0);
    return iRet;
}

- (NexEditorError) runFastPreviewCommand:(id<FastPreviewCommand>) command
{
    return [self.asyncAPI runFastPreviewCommand:command display:YES complete:NULL];
}

- (int)getTexNameForClipID:(int)exportFlag clipId:(int)clipId
{
    if(g_VideoEditorHandle == NULL) {
        return -1;
    }
    return g_VideoEditorHandle->getTexNameForClipID(exportFlag, clipId);
}

- (int)clearRenderItems:(int)flags
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->clearRenderItems(flags);
}

- (int)loadRenderItem:(NSString *)strEffectID effectData:(NSString *)strEffectData flags:(int)flag
{
    if(g_VideoEditorHandle == NULL) {
        return -1;
    }
    
    const char *pstrID = [strEffectID UTF8String];
    const char *pstrEffectData = [strEffectData UTF8String];
    
    return g_VideoEditorHandle->loadRenderItem(pstrID, pstrEffectData, flag);
}

- (int)createRenderItem:(NSString *)strEffectID exportFlag:(int)exportFlag
{
    if(g_VideoEditorHandle == NULL) {
        return -1;
    }
    
    const char *pstrEffectID = [strEffectID UTF8String];
    
    return g_VideoEditorHandle->createRenderItem(exportFlag, pstrEffectID);
}

- (int)releaseRenderItem:(int)effectID exportFlag:(int)exportFlag
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->releaseRenderItem(exportFlag, effectID);
}

- (int)drawRenderItemOverlay:(int)effectID
                effectOption:(NSString *)effectOption
                  exportFlag:(int)exportFlag
                     curTime:(int)curTime
                   startTime:(int)startTime
                     endTime:(int)endTime
                      matrix:(float *)matrix
                        left:(float)left
                         top:(float)top
                       right:(float)right
                      bottom:(float)bottom
                       alpha:(float)alpha
				 modeEnabled:(bool)modeEnabled
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    const char *peffectOption = [effectOption UTF8String];
	int textureNameForBlend = g_VideoEditorHandle->getTexNameForBlend(exportFlag);
	return g_VideoEditorHandle->drawRenderItemOverlay(effectID, textureNameForBlend, exportFlag, (char*)peffectOption, curTime, startTime, endTime, matrix, left, top, right, bottom, alpha, modeEnabled?1:0);
}

- (int)getWaterMarkInfo
{
    NEXSDKInformation stSDKInfo;
    
    if(getSDKInfo(&stSDKInfo))
        return 0;
    
    if(stSDKInfo.bCheckWaterMark) {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] enable WaterMark", __LINE__);
        return 1;
    } else {
        NEXSAL_TRACECAT(NEX_TRACE_CATEGORY_INFO, 0, "[NexEditor.mm %d] disable WaterMark", __LINE__);
        return 0;
    }
}

- (int)setProjectVolumeFadeInTime:(int)fadeInTime fadeOutTime:(int)fadeOutTime
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->setProjectVolumeFade(fadeInTime, fadeOutTime);
}

- (int)setProjectVolume:(int)projectVolume
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->setProjectVolume(projectVolume);
}

- (int)setProjectManualVolumeControl:(int)manualVolume
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->setProjectManualVolumeControl(manualVolume);
}

- (void) setIsGLOperationAllowed:(BOOL)isGLOperationAllowed
{
    _isGLOperationAllowed = isGLOperationAllowed;

    if (g_VideoEditorHandle) {
        g_VideoEditorHandle->setIsGLOperationAllowed(isGLOperationAllowed);
    }
}

#pragma end

#pragma mark - Collage

- (void) build:(IDrawInfo *)iDrawInfo withSceneInfo:(NexSceneInfo*)sceneInfo
{
    iDrawInfo->setDrawInfo(sceneInfo.sceneId, 0/*trackid*/, sceneInfo.subEffectId, sceneInfo.isTransition,
                           sceneInfo.startTimeMS, sceneInfo.endTimeMS,
                           0/*rotate*/, 0/*user_rotate*/, 0/*translate_x*/, 0/*translate_y*/, 0/*lut*/, 0/*custom_lut_a*/, 0/*custom_lut_b*/, 0/*custom_lut_power*/);
    
    if (sceneInfo.effectId) {
        const char *effectId = [sceneInfo.effectId UTF8String];
        iDrawInfo->setEffect(effectId);
    }
    if (sceneInfo.titleValue) {
        const char *title = [sceneInfo.titleValue UTF8String];
        iDrawInfo->setTitle(title);
    }
}

- (IDrawInfoList *) configureDrawInfoListWithSceneInfos:(NSArray <NexSceneInfo*>*)sceneInfos
{
    IDrawInfoList *drawInfoList = g_VideoEditorHandle->createDrawInfoList();

    for (NexSceneInfo *sceneInfo in sceneInfos) {
        IDrawInfo *iDrawInfo = drawInfoList->createDrawInfo();
        [self build:iDrawInfo withSceneInfo:sceneInfo];
        drawInfoList->addDrawInfo(iDrawInfo);
        SAFE_RELEASE(iDrawInfo);
    }
    return drawInfoList;
}

- (void) build:(IDrawInfo *)iDrawInfo withDrawInfo:(NexDrawInfo*)drawInfo
{
    iDrawInfo->setDrawInfo(drawInfo.drawId, drawInfo.clipId, drawInfo.subEffectId, 0/*isTransition*/,
                           drawInfo.startTimeMS, drawInfo.endTimeMS,
                           drawInfo.rotate, drawInfo.userRotate,
                           drawInfo.translate_x, drawInfo.translate_y,
                           drawInfo.lut, drawInfo.custom_lut_a, drawInfo.custom_lut_b, drawInfo.custom_lut_power);

    int iLeft, iTop, iRight, iBottom;
    
    iLeft   = drawInfo.startRect.origin.x;
    iTop    = drawInfo.startRect.origin.y;
    iRight  = iLeft + drawInfo.startRect.size.width;
    iBottom = iTop + drawInfo.startRect.size.height;
    
    iDrawInfo->setStartRect(iLeft, iTop, iRight, iBottom);
    
    iLeft   = drawInfo.endRect.origin.x;
    iTop    = drawInfo.endRect.origin.y;
    iRight  = iLeft + drawInfo.endRect.size.width;
    iBottom = iTop + drawInfo.endRect.size.height;
    
    iDrawInfo->setEndRect(iLeft, iTop, iRight, iBottom);}

- (IDrawInfoList *) configureDrawInfoListWithDrawInfos:(NSArray <NexDrawInfo*>*)drawInfos
{
    IDrawInfoList *drawInfoList = g_VideoEditorHandle->createDrawInfoList();
    
    for (NexDrawInfo *drawInfo in drawInfos) {
        IDrawInfo *iDrawInfo = drawInfoList->createDrawInfo();
        [self build:iDrawInfo withDrawInfo:drawInfo];
        drawInfoList->addDrawInfo(iDrawInfo);
        SAFE_RELEASE(iDrawInfo);
    }
    return drawInfoList;
}

- (int) configureDrawInfoListWithMasterList:(NSArray <NexSceneInfo*>*)sceneInfos slots:(NSArray <NexDrawInfo *>*)drawInfos
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    int iRet = -1;
    IDrawInfoList* drawInfoMasterlist = NULL; IDrawInfoList* drawInfoSubList = NULL;

    if (sceneInfos.count != 0 && drawInfos.count != 0) {
        drawInfoMasterlist = [self configureDrawInfoListWithSceneInfos:sceneInfos];
        drawInfoSubList = [self configureDrawInfoListWithDrawInfos:drawInfos];
    }

    if (drawInfoMasterlist != NULL && drawInfoSubList != NULL) {
        iRet = g_VideoEditorHandle->setDrawInfoList(drawInfoMasterlist, drawInfoSubList);
        SAFE_RELEASE(drawInfoMasterlist);
        SAFE_RELEASE(drawInfoSubList);
    }
    return iRet;
}

- (int) updateDrawInfoWith:(NexDrawInfo *)drawInfo
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    
    IDrawInfo *pDrawInfo = g_VideoEditorHandle->createDrawInfo();
    [self build:pDrawInfo withDrawInfo:drawInfo];
    
    int iRet = g_VideoEditorHandle->setDrawInfo(pDrawInfo);
    SAFE_RELEASE(pDrawInfo);
    return iRet;
}
#pragma mark - Private: Deep Internal

- (BOOL) isVideoEditorHandleValid
{
    return g_VideoEditorHandle != NULL;
}

- (NexEditorError) videoEditorHandleSetTime:(unsigned int) uiTime display:(int) iDisplay IDRFrame:(int) iIDRFrame
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->setTime(uiTime, iDisplay, iIDRFrame);
}

- (NexEditorError) videoEditorHandleStopPlayWithFlag:(int) iFlag
{
    RETURN_ERROR_IF_EDITOR_NOT_VALID(g_VideoEditorHandle);
    return g_VideoEditorHandle->stopPlay(iFlag);
}

#pragma mark - Private: Event Waiting Blocks
- (BOOL) didReceiveEditorEvent:(NexEditorEvent) event
{
    if (self.waitingEventBlocks.count == 0) {
        return NO;
    }
    
    @synchronized(self.waitingEventBlocks) {
        NSArray *blocks = [self.waitingEventBlocks copy];
        
        NSMutableArray *doneList = [[NSMutableArray alloc] init];
        for( NexEditorOnEventBlock block in blocks) {
            if (block(event)) {
                [doneList addObject:block];
            }
        }
        [self.waitingEventBlocks removeObjectsInArray:doneList];
        
        // Manual release in a custom thread
        [doneList release];
        [blocks release];
        return NO;
    }
}

- (void) addEventWaiterFor:(VIDEOEDITOR_EVENT) eventType onQueue:(dispatch_queue_t) queue block:(NexEditorOnEventBlock) block
{
    if (queue == NULL) {
        queue = dispatch_get_main_queue();
    }
    
    NexEditorOnEventBlock waiter = ^(NexEditorEvent event) {
        BOOL result = NO;
        if (event.type == eventType) {
            NexLogD(LOG_TAG, @"event:%08x {%08x, %08x, %08x, %08x}", event.type, event.params[0], event.params[1], event.params[2], event.params[3]);
            dispatch_async(queue, ^{
                block(event);
            });
            result = YES;
        }
        return result;
    };
    
    const void * entry = _Block_copy(waiter);
    @synchronized(self.waitingEventBlocks) {
        [self.waitingEventBlocks addObject:(id) entry];
    }
    _Block_release(entry);
}

- (void) addEventWaiterFor:(VIDEOEDITOR_EVENT) eventType block:(NexEditorOnEventBlock) block
{
    [self addEventWaiterFor:eventType onQueue:dispatch_get_main_queue() block:block];
}

@end
