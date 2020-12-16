/******************************************************************************
 * File Name   : NexClip.m
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

#import "NXEClipInternal.h"
#import "NXEEngine.h"
#import "NXEProject.h"
#import "NXEEngineEventListener.h"
#import "NXEEffectOptions.h"
#import "EditorUtil.h"
#import <NexEditorEngine/NexEditorEngine.h>
#import "AssetLibraryProxy.h"
#import "NXEEffectAssetItemPrivate.h"
#import "UserField.h"
#import "NXETransitionAssetItemPrivate.h"
#import "MediaInfo.h"
#import "NXEClipSourcePrivate.h"
#import "NXEClipTransformPrivate.h"
#import <NexEditorEngine/NXEVisualClipInternal.h>

#define LOG_TAG @"NXEClip"

#define kDefaultClipEffectShowTime     0
#define kDefaultClipEffectEndTime  10000

#define CLAMP(x, min, max)                  (MAX(min, MIN(max, x)))
#define COLORADJ_COMPO_FLOAT2INT255(c)       (CLAMP(c, -1.0, 1.0) * 255)
#define COLORADJ_COMPO_INT2552FLOAT(c)       (COLORADJ_COMPO_CLAMP_INT255(c) / 255.0)
#define COLORADJ_COMPO_MAX_INT255   (255)
#define COLORADJ_COMPO_MIN_INT255   (-255)
#define COLORADJ_COMPO_CLAMP_INT255(c)          CLAMP(c, COLORADJ_COMPO_MIN_INT255, COLORADJ_COMPO_MAX_INT255)

/**
 * \ingroup types
 * \brief A list of Effect Types
 * \since version 1.0.5
 */
typedef NS_ENUM(NSUInteger, EffectTypes) {
    /*! This indicates that no effect is set to a clip. */
    EFFECT_NONE = 0,
    /*! This indicates that the clip effect is automatically set matching the theme currently set. */
    EFFECT_CLIP_AUDIO,
    /*! This indicates that the clip effect is not part of any theme but can be set by the user. */
    EFFECT_CLIP_USER,
    /*! This indicates that the transition effect is automatically set matching the theme currently set. */
    EFFECT_TRANSITION_AUTO,
    /*! This indicates the transition effect is set by the user. */
    EFFECT_TRANSITION_USER
} _DOC_ENUM(EffectTypes);

@interface NXEVisualClip(ColorAdjustment)
- (void) setColorAdjustment:(NXEColorAdjustments) colorAdjustment;
@end

@implementation NXEVisualClip(ColorAdjustment)
- (void) setColorAdjustment:(NXEColorAdjustments) colorAdjustment
{
    self.brightness = COLORADJ_COMPO_FLOAT2INT255(colorAdjustment.brightness);
    self.contrast = COLORADJ_COMPO_FLOAT2INT255(colorAdjustment.contrast);
    self.saturation = COLORADJ_COMPO_FLOAT2INT255(colorAdjustment.saturation);
}
@end

@interface NXEAudioClip(ClipInfo) <NXEClipInfo>
@end

@interface NXEVisualClip(ClipInfo) <NXEVisualClipInfo>
@end

@implementation NXEAudioClip(ClipInfo)
@end

@implementation NXEVisualClip(ClipInfo)
@end

@interface NXEClip ()

/// storage
@property (nonatomic, retain) id<NXEClipInfo> clipInfo;
/// Type casted getters
@property (nonatomic, retain, readonly) NXEVisualClip *videoInfo;
@property (nonatomic, retain, readonly) NXEAudioClip *audioInfo;


@property (nonatomic) int templateTagid;

@property (nonatomic, readonly) int speedControlledTrimDuration;
@property (nonatomic, readonly) int speedControlledStartTrimDuration;
@property (nonatomic, readonly) int speedControlledEndTrimDuration;
@property (nonatomic) NXEClipType clipType;
@property (nonatomic) int effectShowTime;
@property (nonatomic) int effectEndTime;
@property (nonatomic, retain) NSMutableDictionary *effectOptions;
@property (nonatomic, retain) NSMutableDictionary *transitionEffectOptions;

/** \brief The properties of a color effect.
 *  \see NXEColorEffect, NXEColorEffect class.
 *  \since version 1.0.5
 */
@property (nonatomic, retain) NXEColorEffect *colorEffect;

@property (nonatomic, strong) NXEClipSource *clipSource;

/** \brief Initializes the new clip object with the speficied media file path.
 *  \note Dependant upon the size and format of the media, this initialization may take a while.
 *  \param path The path of media files.
 *  \param error An error code.
 *  \return An NXEClip object.
 *  \since version 1.0.5
 */
- (instancetype)initWithPath:(NSString *)path error:(NXEError **)error;

@end

@implementation NXEClip

- (void)dealloc
{
    self.clipSource = nil;
    self.clipInfo = nil;
    [_audioEnvelop release];
    [_effectOptions release];
    [_transitionEffectOptions release];
    [_colorEffect release];
    [super dealloc];
}

- (instancetype)init
{
    self = [super init];
    if(self) {
        self.clipInfo = [[[NXEVisualClip alloc] init] autorelease];
    }
    return self;
}

- (instancetype)initWithPath:(NSString *)path error:(NXEError **)error
{
    self = [super init];
    if(self) {
        if (path == nil) {
            *error = [[NXEError alloc] initWithErrorCode:ERROR_UNSUPPORT_FORMAT];
            [self release];
            return nil;
        }
        MediaInfo *minfo = [[MediaInfo alloc] initWithPath:path error:error];
        if(minfo == nil) {
            [minfo release];
            [self release];
            return nil;
        }
        NXEClipTransformRamp ramp;
        ramp.start = CGAffineTransformIdentity;
        ramp.end = CGAffineTransformIdentity;
        _transformRamp = ramp;
        
        
        _effectOptions = [[NSMutableDictionary alloc] init];
        _transitionEffectOptions = [[NSMutableDictionary alloc] init];
        
        if(minfo.hasImage) {
            _clipType = NXE_CLIPTYPE_IMAGE;
            
            NXEVisualClip *clipInfo = [[[NXEVisualClip alloc] init] autorelease];
            clipInfo.clipPath = path;
            clipInfo.clipType = _clipType;
            clipInfo.width = minfo.videoWidth;
            clipInfo.height = minfo.videoHeight;
            clipInfo.existVideo = 1;
            clipInfo.totalTime = 6000;
            clipInfo.clipVolume = 100;
            clipInfo.bGMVolume = 100;
            clipInfo.rotateState = minfo.rotateDegree;
            self.clipInfo = clipInfo;
            self.cropMode = CropModeFit;
            
            [self setClipEffect:kEffectIDNone];
            [self setTransitionEffect:kEffectIDNone];
        } else if(minfo.hasVideo) {
            _clipType = NXE_CLIPTYPE_VIDEO;
            
            NXEVisualClip *clipInfo = [[[NXEVisualClip alloc] init] autorelease];
            clipInfo.clipPath = path;
            clipInfo.clipType = _clipType;
            clipInfo.width = minfo.videoWidth;
            clipInfo.height = minfo.videoHeight;
            clipInfo.totalVideoTime = minfo.videoDuration;
            clipInfo.totalAudioTime = minfo.audioDuration;
            clipInfo.existAudio = [minfo hasAudio];
            clipInfo.existVideo = [minfo hasVideo];
            clipInfo.audioOnOff = clipInfo.existAudio ? 1 : 0;
            clipInfo.rotateState = minfo.videoOrientation;
            clipInfo.totalTime = clipInfo.existAudio ? clipInfo.totalAudioTime : clipInfo.totalVideoTime;
            clipInfo.bGMVolume = 100;
            clipInfo.clipVolume = 100;
            clipInfo.pitch = 0;
            clipInfo.panLeft = -100;
            clipInfo.panRight = 100;
            clipInfo.compressor = 0;
            clipInfo.voiceChanger = 0;
            self.clipInfo = clipInfo;
            self.cropMode = CropModeFit;
            
            // the base value of total time is audio time,
            // but if it's satisfied with below conditions, the total time is video time.
            /// FIXME: totalTime adjustment should be done inside of NXEVisualClip
            if(clipInfo.totalVideoTime != clipInfo.totalAudioTime
               && (clipInfo.totalAudioTime - clipInfo.totalVideoTime) > 0) {
                clipInfo.totalTime = clipInfo.totalVideoTime;
            }
            [self setClipEffect:kEffectIDNone];
            [self setTransitionEffect:kEffectIDNone];
        } else {
            _bLoop = true;
            //
            _clipType = NXE_CLIPTYPE_AUDIO;
            
            NXEAudioClip *clipInfo = [[[NXEAudioClip alloc] init] autorelease];
            clipInfo.clipPath = path;
            clipInfo.audioOnOff = [minfo hasAudio] ? 1 : 0;
            clipInfo.totalTime = minfo.audioDuration;
            clipInfo.startTime = 0;
            clipInfo.endTime = clipInfo.totalTime;
            clipInfo.clipVolume = 100;
            clipInfo.autoEnvelop = 1;
            clipInfo.pitch = 0;
            clipInfo.panLeft = -100;
            clipInfo.panRight = 100;
            clipInfo.compressor = 0;
            clipInfo.voiceChanger = 0;
            self.clipInfo = clipInfo;
        }
        
        _audioEnvelop = [[NXEAudioEnvelop alloc] initWithClip:self];
        //
        _effectShowTime = -1;
        _effectEndTime = -1;
        //
        [minfo release];
    }
    return self;
}

+ (instancetype)newSupportedClip:(NSString *)path
{
    NXEError *error;
    NXEClip *clip = [[NXEClip alloc] initWithPath:path error:&error];
    if(clip == nil) {
        NSString *errorDescription = [NSString stringWithString:error.errorDescription];
        NSException *e = [NSException exceptionWithName:@"GeneralException"
                                                 reason:[NSString stringWithFormat:@"The Error of newSupportedClip(fileName:%@) is \"%@.\"", [path lastPathComponent],errorDescription]
                                               userInfo:nil];
        [error release];
        @throw e;
    } else {
        return clip;
    }
}

+ (instancetype)newSolidClip:(NSString *)color
{
    NXEClip *clip = [[NXEClip alloc] init];
    
    clip.clipType = NXE_CLIPTYPE_IMAGE;
    clip.videoInfo.clipPath = [NSString stringWithFormat:@"%@%@%@", @"@solid:", color, @".jpg"];
    clip.videoInfo.clipType = clip.clipType;
    clip.videoInfo.existVideo = 1;
    clip.videoInfo.width = 480;
    clip.videoInfo.height = 270;
    clip.videoInfo.totalTime = 6000;
    clip.videoInfo.clipVolume = 100;
    clip.videoInfo.bGMVolume = 100;
    clip.cropMode = CropModeFit;
    return clip;
}

+ (instancetype)clipWithSource:(NXEClipSource *) source error:(NSError **)error
{
    NXEError *perror = nil;
    NXEClip *clip = [[[NXEClip alloc] initWithPath:source.path error:&perror] autorelease];
    if (perror == nil) {
        clip.clipSource = source;
    } else if (error) {
        *error = perror;
    }
    return clip;
}

#pragma mark - NXEClipInfo properties

+ (BOOL) isVisualClipType:(NXEClipType) clipType
{
    BOOL result = NO;
    switch(clipType) {
        case NXE_CLIPTYPE_IMAGE:
        case NXE_CLIPTYPE_VIDEO:
        case NXE_CLIPTYPE_VIDEO_LAYER:
            result = YES;
            break;
        default:
            break;
    }
    return result;
}

- (id<NXEVisualClipInfo>) vinfo
{
    id<NXEVisualClipInfo> result = nil;
    if ([self.class isVisualClipType:self.clipType] && [self.clipInfo conformsToProtocol:@protocol(NXEVisualClipInfo)]) {
        result = (id<NXEVisualClipInfo>) self.clipInfo;
    }
    return result;
}

- (id<NXEClipInfo>) ainfo
{
    id<NXEClipInfo> result = nil;
    if (self.clipType == NXE_CLIPTYPE_AUDIO) {
        result = (id<NXEClipInfo>) self.clipInfo;
    }
    return result;
}
#pragma mark -

- (NXEVisualClip *) videoInfo
{
    NXEVisualClip *result = nil;
    if ([self.clipInfo isKindOfClass:NXEVisualClip.class]) {
        result = (NXEVisualClip *) self.clipInfo;
    }
    return result;
}

- (NXEAudioClip *) audioInfo
{
    NXEAudioClip *result = nil;
    if ([self.clipInfo isKindOfClass:NXEAudioClip.class]) {
        result = (NXEAudioClip *) self.clipInfo;
    }
    return result;
}

#pragma mark -
- (void)setImageClipDuration:(int)time
{
    if(self.clipType == NXE_CLIPTYPE_IMAGE) {
        self.videoInfo.totalTime = time;
    }
}

- (void)setClipVolume:(int)volume
{
    if (volume < 0) volume = 0;
    else if (volume > 200) volume = 200;

    if (self.clipType == NXE_CLIPTYPE_AUDIO) {
        self.audioInfo.clipVolume = volume;
    } else if ([self.class isVisualClipType:self.clipType]) {
        self.videoInfo.clipVolume = volume;
    }
}

- (NSString *)getClipEffectID
{
    return self.videoInfo.titleEffectID;
}

/*  if you don't want to apply the clipEffect, you must apply kEffectIDNone at the first parameter.
 */
- (void)setClipEffect:(NSString*)effectId
{
    if (effectId == nil) {
        effectId = kEffectIDNone;
    }
    if([effectId isEqualToString:kEffectIDNone] != true) {
        id<AssetItem> effect = [[AssetLibrary proxy] itemForId:effectId];
        if(effect != nil && [effect conformsToProtocol:@protocol(ConfigurableAssetItem)]) {
            self.effectShowTime = kDefaultClipEffectShowTime;
            self.effectEndTime = kDefaultClipEffectEndTime;
            
            self.videoInfo.effectType = EFFECT_CLIP_USER;
            self.videoInfo.titleEffectID = effectId;
            //
            [self.effectOptions removeAllObjects];
            for(UserField *userField in ((id<ConfigurableAssetItem>)effect).userFields) {
                [self.effectOptions setObject:userField.userfieldDefault==nil?@"":[userField.userfieldDefault copy]
                                       forKey:[userField.userfieldID copy]];
            }
        } else {
            self.effectShowTime = 0;
            self.effectEndTime = 0;
            
            self.videoInfo.titleEffectID = kEffectIDNone;
        }
    } else {
        self.effectShowTime = 0;
        self.effectEndTime = 0;
        
        self.videoInfo.titleEffectID = kEffectIDNone;
    }
}

- (void)setClipEffect:(NSString *)effectId effectOption:(NXEEffectOptions *)effectOption
{
    if (effectId == nil) {
        effectId = kEffectIDNone;
    }
    if([effectId isEqualToString:kEffectIDNone] != YES) {
        id<ConfigurableAssetItem> effect = (NXEEffectAssetItem *)[[AssetLibrary proxy] itemForId:effectId];
        if(effect != nil && [effect conformsToProtocol:@protocol(ConfigurableAssetItem)]) {
            self.effectShowTime = kDefaultClipEffectShowTime;
            self.effectEndTime = kDefaultClipEffectEndTime;
            
            self.videoInfo.effectType = EFFECT_CLIP_USER;
            self.videoInfo.titleEffectID = effectId;
            //
            [self.effectOptions removeAllObjects];
            for(UserField *userField in effect.userFields) {
                [self.effectOptions setObject:userField.userfieldDefault==nil?@"":[userField.userfieldDefault copy]
                                       forKey:[userField.userfieldID copy]];
            }
            if([effectOption.textOptions count] > 0) {
                for(NXETextOption *textOption in effectOption.textOptions) {
                    [self.effectOptions setObject:textOption.value4textField
                                           forKey:textOption.key4textField];
                }
            }
            if([effectOption.colorOptions count] > 0) {
                for(NXEColorOption *colorOption in effectOption.colorOptions) {
                    [self.effectOptions setObject:colorOption.value4colorField
                                           forKey:colorOption.key4colorField];
                }
            }
            // delete
            /*
            if(effectOption.selectionOptionInClipEffect.key4selectionField != nil) {
                [self.effectOptions setObject:(effectOption.selectionOptionInClipEffect.value4selectionField == nil ? @"" : effectOption.selectionOptionInClipEffect.value4selectionField)
                                       forKey:effectOption.selectionOptionInClipEffect.key4selectionField];
            }
            */
        } else {
            self.effectShowTime = 0;
            self.effectEndTime = 0;
            
            self.videoInfo.titleEffectID = kEffectIDNone;
        }
    } else {
        self.effectShowTime = 0;
        self.effectEndTime = 0;
        
        self.videoInfo.titleEffectID = kEffectIDNone;
    }
}

- (NSString *)getTransitionEffectID
{
    return self.videoInfo.clipEffectID;
}

/*  if you don't want to apply the transitionEffect, you must apply kEffectIDNone at the first parameter.
 */
- (void)setTransitionEffect:(NSString*)effectId
{
    if (effectId == nil) {
        effectId = kEffectIDNone;
    }
    if([effectId isEqualToString:kEffectIDNone] != YES) {
        id<AssetItem> transitionEffect = (id<AssetItem>)[[AssetLibrary proxy] itemForId:effectId];
        if(transitionEffect != nil && [transitionEffect conformsToProtocol:@protocol(TransitionAssetItemAttributes)]) {
            self.videoInfo.clipEffectID = effectId;
            self.videoInfo.effectDuration = (int) ((id<TransitionAssetItemAttributes>)transitionEffect).effectDuration;
            self.videoInfo.effectOffset = (int) ((id<TransitionAssetItemAttributes>)transitionEffect).effectOffset;
            self.videoInfo.effectOverlap = (int) ((id<TransitionAssetItemAttributes>)transitionEffect).effectOverlap;
            //
            [self.transitionEffectOptions removeAllObjects];
            if ( [transitionEffect conformsToProtocol:@protocol(ConfigurableAssetItem)] ) {
                id<ConfigurableAssetItem> configurable = (id<ConfigurableAssetItem>) transitionEffect;
                for(UserField *userField in configurable.userFields) {
                    [self.transitionEffectOptions setObject:userField.userfieldDefault==nil?@"":[userField.userfieldDefault copy]
                                                     forKey:[userField.userfieldID copy]];
                }
            }
        } else {
            self.videoInfo.clipEffectID = kEffectIDNone;
            self.videoInfo.effectDuration = 0;
            self.videoInfo.effectOffset = 0;
            self.videoInfo.effectOverlap = 0;
        }
    } else {
        self.videoInfo.clipEffectID = kEffectIDNone;
        self.videoInfo.effectDuration = 0;
        self.videoInfo.effectOffset = 0;
        self.videoInfo.effectOverlap = 0;
    }
}

- (void)setTransitionEffect:(NSString *)effectId effectOption:(NXEEffectOptions *)effectOption
{
    if (effectId == nil) {
        effectId = kEffectIDNone;
    }
    if([effectId isEqualToString:kEffectIDNone] != YES) {
        id<AssetItem> transitionEffect = (id<AssetItem>)[[AssetLibrary proxy] itemForId:effectId];
        if(transitionEffect != nil && [transitionEffect conformsToProtocol:@protocol(TransitionAssetItemAttributes)]) {
            self.videoInfo.clipEffectID = effectId;
            self.videoInfo.effectDuration = (int) ((id<TransitionAssetItemAttributes>)transitionEffect).effectDuration;
            self.videoInfo.effectOffset = (int) ((id<TransitionAssetItemAttributes>)transitionEffect).effectOffset;
            self.videoInfo.effectOverlap = (int) ((id<TransitionAssetItemAttributes>)transitionEffect).effectOverlap;
            //
            [self.transitionEffectOptions removeAllObjects];
            if ( [transitionEffect conformsToProtocol:@protocol(ConfigurableAssetItem)] ) {
                id<ConfigurableAssetItem> configurable = (id<ConfigurableAssetItem>) transitionEffect;
                for(UserField *userField in configurable.userFields) {
                    [self.transitionEffectOptions setObject:userField.userfieldDefault==nil?@"":[userField.userfieldDefault copy]
                                                     forKey:[userField.userfieldID copy]];
                }
            }
            for(NXETextOption *textOption in effectOption.textOptions) {
                [self.transitionEffectOptions setObject:textOption.value4textField
                                                 forKey:textOption.key4textField];
            }
            for(NXEColorOption *colorOption in effectOption.colorOptions) {
                [self.transitionEffectOptions setObject:colorOption.value4colorField
                                                 forKey:colorOption.key4colorField];
            }
            // delete
            /*
            if(effectOption.selectionOptionInTransitionEffect.key4selectionField != nil) {
                [self.transitionEffectOptions setObject:(effectOption.selectionOptionInTransitionEffect.value4selectionField == nil ? @"" : effectOption.selectionOptionInTransitionEffect.value4selectionField)
                                       forKey:effectOption.selectionOptionInTransitionEffect.key4selectionField];
            }
            */
        } else {
            self.videoInfo.clipEffectID = kEffectIDNone;
            self.videoInfo.effectDuration = 0;
            self.videoInfo.effectOffset = 0;
            self.videoInfo.effectOverlap = 0;
        }
    } else {
        self.videoInfo.clipEffectID = kEffectIDNone;
        self.videoInfo.effectDuration = 0;
        self.videoInfo.effectOffset = 0;
        self.videoInfo.effectOverlap = 0;
    }
}

- (void)ensureTransitionFit:(NXEProject *)project wantedClipIndex:(int)index
{
    NXEClip *clip = [project.visualClips objectAtIndex:index];
    
    int durationLimit = 30000/* max duration */;
    
    if(index >= 1 && index < project.visualClips.count - 1) {
        NXEClip *prevItem = [project.visualClips objectAtIndex:index-1];
        int prevItemSpace = ((prevItem.videoInfo.totalTime - prevItem.videoInfo.startTrimTime - prevItem.videoInfo.endTrimTime) * 100 / prevItem.videoInfo.speedControl) - 750/* guarantee duration */;
        if(prevItemSpace < 0) {
            prevItemSpace = 0;
        }
        if(durationLimit > prevItemSpace) {
            durationLimit = prevItemSpace;
        }
        NXEClip *nextItem = [project.visualClips objectAtIndex:index+1];
        int nextItemSpace = ((nextItem.videoInfo.totalTime - nextItem.videoInfo.startTrimTime - nextItem.videoInfo.endTrimTime) * 100 / nextItem.videoInfo.speedControl) - 750/* guarantee duration */;
        if(nextItemSpace < 0) {
            nextItemSpace = 0;
        }
        if(durationLimit > nextItemSpace) {
            durationLimit = nextItemSpace;
        }
    } else {
        int itemSpace = ((clip.videoInfo.totalTime - clip.videoInfo.startTrimTime - clip.videoInfo.endTrimTime) * 100 / clip.videoInfo.speedControl) - 750/* guarantee duration */;
        if(itemSpace < 0) {
            itemSpace = 0;
        }
        if(durationLimit > itemSpace) {
            durationLimit = itemSpace;
        }
    }
    
    durationLimit = durationLimit + (durationLimit * (100 - clip.videoInfo.effectOverlap) / 100);
    
    int effectDuration = clip.vinfo.effectDuration;

    if(effectDuration < 100/* min duration */) {
        effectDuration = 100;
    }
    if(effectDuration > 30000/* max duration */) {
        effectDuration = 30000;
    }
    if(effectDuration > durationLimit) {
        effectDuration = durationLimit;
    }

    clip.videoInfo.effectDuration = effectDuration;
}

- (void)setTransitionEffectDuration:(int)duration
{
    self.videoInfo.effectDuration = duration;
}

- (void)setAudioClipStartTime:(int)startTime EndTime:(int)endTime
{
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        self.audioInfo.startTime = startTime;
        if(endTime == 0) {
            self.audioInfo.endTime = self.audioInfo.totalTime;
        } else {
            self.audioInfo.endTime = endTime;
        }
    }
}

- (int)headTrimDuration
{
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        return self.ainfo.startTrimTime;
    } else if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        return self.vinfo.startTrimTime;
    } else {
        return 0;
    }
}

- (int)trimEndTime
{
    return self.clipInfo.totalTime - self.clipInfo.endTrimTime;
}

- (NXETimeMillis) durationMs
{
    return self.clipInfo.totalTime;
}

- (int)speedControlledTrimDuration
{
    int duration = self.durationMs;
    
    if(self.clipInfo.startTrimTime != 0 || self.clipInfo.endTrimTime != 0) {
        duration -= (self.clipInfo.startTrimTime + self.clipInfo.endTrimTime);
    }
    if(self.clipType == NXE_CLIPTYPE_VIDEO) {
        duration = (int)round((float)(duration * 100)/(float)self.videoInfo.speedControl);
    }
    
    return duration;
}

- (int)speedControlledStartTrimDuration
{
    int startTrimTime = self.clipInfo ? 0 : self.clipInfo.startTrimTime;
    if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        int speed = self.videoInfo.speedControl;
        startTrimTime = (int)round((float)(self.videoInfo.startTrimTime * 100)/(float)speed);
    }
    return startTrimTime;
}

- (int)speedControlledEndTrimDuration
{
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        return self.ainfo.endTrimTime;
    } else if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        int speed = self.vinfo.speedControl;
        return (int)round((float)(self.vinfo.endTrimTime * 100)/(float)speed);
    } else {
        return 0;
    }
}

- (void)setTrim:(int)startTime EndTrimTime:(int)endTime
{
    if(startTime == 0 && endTime == 0) {
        
        if(self.clipType == NXE_CLIPTYPE_AUDIO) {
            
            self.audioInfo.startTrimTime = 0; self.audioInfo.endTrimTime = 0;
            
        } else {
            
            self.videoInfo.startTrimTime = 0; self.videoInfo.endTrimTime = 0;
        }
        return;
    }
    
    if(endTime <= startTime) {
        NSException *e = [NSException exceptionWithName:@"InvalidRangeException"
                                                 reason:[NSString stringWithFormat:@"eTrimTime is low than sTrimTime (%d >= %d)", startTime, endTime]
                                               userInfo:nil];
        @throw e;
    }
    
    [self.audioEnvelop updateTrimTime:startTime end:endTime];
    
    int headTrimTime = startTime;
    int tailTrimTime = self.clipInfo.totalTime - endTime;
    
    if(headTrimTime < 0 || tailTrimTime < 0) {
        NSException *e = [NSException exceptionWithName:@"InvalidValueException"
                                                 reason:[NSString stringWithFormat:@"startTrimTime is negative/ endTrimTime is larger than totlaTime(%d >= %d)", tailTrimTime, self.videoInfo.totalTime]
                                               userInfo:nil];
        @throw e;
    }
    
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        
        self.audioInfo.startTrimTime = headTrimTime; self.audioInfo.endTrimTime = tailTrimTime;
        
    } else {
        
        self.videoInfo.startTrimTime = headTrimTime; self.videoInfo.endTrimTime = tailTrimTime;
    }
}

- (void)setSpeed:(int)clipSpeed
{
    if (clipSpeed < 13) clipSpeed = 13;
    else if (clipSpeed > 400) clipSpeed = 400;
    
    if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        self.videoInfo.speedControl = clipSpeed;
    }
}

- (void)setEffectShowTime:(int)startTime endTime:(int)endTime
{
    self.effectShowTime = startTime;
    self.effectEndTime = endTime;
}

- (void)getEffectShowTime:(int *)startTime endTime:(int *)endTime
{
    // NESI-186
    //
    *startTime = self.effectShowTime;
    *endTime = self.effectEndTime;
}

- (void)setDisplayStartTime:(int)startTime endTime:(int)endTime
{
    if(/*self.clipType == NXE_CLIPTYPE_VIDEO ||*/ self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        self.videoInfo.startTime = startTime;
        self.videoInfo.endTime = endTime;
    }
}

- (void)setColorEffectValues:(NXEPresetColorEffect)presetName
{
    if (presetName < NXE_CE_NONE) presetName = NXE_CE_NONE;
    else if (presetName > NXE_CE_MAX) presetName = NXE_CE_MAX;

    if(self.colorEffect == nil) {
        self.colorEffect = [[[NXEColorEffect alloc] initWithPresetColorEffect:presetName] autorelease];
    } else {
        [self.colorEffect setValues:presetName];
    }
}

- (void)setCropMode:(CropMode)cropMode
{
    _cropMode = cropMode;
    [self setTransformRampWithMode:cropMode];
}

- (void) setTransformRamp:(NXEClipTransformRamp)ramp
{
    if (self.cropMode == CropModeCustom) {
        [self updateTransformRampWith:ramp];
    }
}

- (void) updateTransformRampWith:(NXEClipTransformRamp)ramp
{
    _transformRamp = ramp;
    
    CGSize renderSize = CGSizeMake(1280,720);// = NXEEngine.instance.logicalRenderSize;
    CGRect renderRect = CGRectMake(0, 0, renderSize.width, renderSize.height);
    
    CGRect r1 = CGRectApplyAffineTransform(renderRect, CGAffineTransformInvert(ramp.start));
    [self setCropStartRect:r1];

    CGRect r2 = CGRectApplyAffineTransform(renderRect, CGAffineTransformInvert(ramp.end));
    [self setCropEndRect:r2];

}

- (CGSize) size
{
    CGSize result = CGSizeZero;
    if (self.vinfo) {
        result = self.videoInfo.orientationRegardedSize;
    }
    return result;
}

- (BOOL)setBrightness:(int)brightnessValue
{
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        return false;
    }
    if (brightnessValue < -255) brightnessValue = -255;
    else if (brightnessValue > 255) brightnessValue = 255;
    
    self.videoInfo.brightness = brightnessValue;
    return true;
}

- (int)getCombinedBrightness
{
    if(self.colorEffect == nil) {
        return self.vinfo.brightness;
    } else {
        return self.vinfo.brightness + (int)(255 * self.colorEffect.brightness);
    }
}

- (BOOL)setContrast:(int)contrastValue
{
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        return false;
    }
    if (contrastValue < -255) contrastValue = -255;
    else if (contrastValue > 255) contrastValue = 255;

    self.videoInfo.contrast = contrastValue;
    return true;
}

- (int)getCombinedContrast
{
    if(self.colorEffect == nil) {
        return self.vinfo.contrast;
    } else {
        return self.vinfo.contrast + (int)(255 * self.colorEffect.contrast);
    }
}

- (BOOL)setSaturation:(int)saturationValue
{
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        return false;
    }
    if (saturationValue < -255) saturationValue = -255;
    else if (saturationValue > 255) saturationValue = 255;

    self.videoInfo.saturation = saturationValue;
    return true;
}

- (int)getCombinedSaturation
{
    if(self.colorEffect == nil) {
        return self.vinfo.saturation;
    } else {
        return self.vinfo.saturation + (int)(255 * self.colorEffect.saturation);
    }
}

- (NXEColorAdjustments) colorAdjustment
{
    // NESI-600 Avoid access to vinfo.brightness, contrast, and saturation
    NXEColorAdjustments result = NXEColorAdjustmentsMake(0, 0, 0);
    if (self.vinfo) {
        result.brightness = COLORADJ_COMPO_INT2552FLOAT(self.vinfo.brightness);
        result.contrast = COLORADJ_COMPO_INT2552FLOAT(self.vinfo.contrast);
        result.saturation = COLORADJ_COMPO_INT2552FLOAT(self.vinfo.saturation);
    }
    return result;
}

- (void) setColorAdjustment:(NXEColorAdjustments)colorAdjustment
{
    [self.videoInfo setColorAdjustment:colorAdjustment];
}

- (void)setLut:(NXELutTypes)lutTypes
{
    self.videoInfo.lut = lutTypes;
}

- (void)setLutId:(NXELutID)lutId
{
    self.videoInfo.lut = lutId;
}

- (NXELutID) lutId
{
    if( self.vinfo.lut == NXE_LUT_NONE ) {
        return kLutIdNotFound;
    }
    return self.vinfo.lut;
}

- (void)setVignette:(BOOL)isApply
{
    self.videoInfo.vignette = isApply ? 1: 0;
}

- (int)getSeekPointCount
{
    NXEError *error;
    int seekPointCount;
    MediaInfo *minfo = [[MediaInfo alloc] initWithPath:self.videoInfo.clipPath error:&error];
    if(minfo == nil && error.errorCode != ERROR_NONE) {
        return -1;
    }
    seekPointCount = minfo.seekPointCount;
    [minfo release];
    return seekPointCount;
}

- (int)getSeekPointInterval
{
    NXEError *error;
    int seekPointInterval;
    MediaInfo *minfo = [[MediaInfo alloc] initWithPath:self.videoInfo.clipPath error:&error];
    if(minfo == nil && error.errorCode != ERROR_NONE) {
        return -1;
    }
    seekPointInterval = minfo.videoDuration / minfo.seekPointCount;
    [minfo release];
    return seekPointInterval;
}

- (void)setCompressorValue:(int)compressorValue
{
    if (compressorValue < 0) compressorValue = 0;
    else if (compressorValue > 7) compressorValue = 7;
    
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        self.audioInfo.compressor = compressorValue;
    } else if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        self.videoInfo.compressor = compressorValue;
    }
}

- (void)setPanLeftValue:(int)panLeftValue
{
    if (panLeftValue < -100) panLeftValue = -100;
    else if (panLeftValue > 100) panLeftValue = 100;
    
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        self.audioInfo.panLeft = panLeftValue;
    } else if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        self.videoInfo.panLeft = panLeftValue;
    }
}

- (void)setPanRightValue:(int)panRightValue
{
    if (panRightValue < -100) panRightValue = -100;
    else if (panRightValue > 100) panRightValue = 100;

    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        self.audioInfo.panRight = panRightValue;
    } else if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        self.videoInfo.panRight = panRightValue;
    }
}

- (void)setPitchValue:(int)pitchValue
{
    if (pitchValue < -6) pitchValue = -6;
    else if (pitchValue > 6) pitchValue = 6;
    
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        self.audioInfo.pitch = pitchValue;
    } else if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        self.videoInfo.pitch = pitchValue;
    }
}

- (void)setVoiceChangerValue:(int)voiceChangerValue
{
    if(self.clipType == NXE_CLIPTYPE_AUDIO) {
        self.audioInfo.voiceChanger = voiceChangerValue;
    } else if(self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_VIDEO_LAYER) {
        self.videoInfo.voiceChanger = voiceChangerValue;
    }
}

#pragma mark - NSCopying

- (instancetype)copyWithZone:(NSZone *)zone
{
    NXEClip *destination = [[self class] allocWithZone:zone];

    destination.clipSource = self.clipSource;
    destination.clipType = self.clipType;
    destination.clipInfo = [[self.clipInfo copy] autorelease];
    destination.transformRamp = self.transformRamp;
    destination.cropMode = self.cropMode;
    destination.audioEnvelop = [self.audioEnvelop copy];
    destination.colorEffect = [[self.colorEffect copy] autorelease];
    destination.effectOptions = [[NSMutableDictionary alloc] initWithDictionary:self.effectOptions copyItems:YES];
    destination.transitionEffectOptions = [[NSMutableDictionary alloc] initWithDictionary:self.transitionEffectOptions copyItems:YES];
    destination.effectShowTime = self.effectShowTime;
    destination.effectEndTime = self.effectEndTime;
    destination.bLoop = self.bLoop;
    
    // vinfo, ainfo, crop, audioEnvelop, colorEffect, effectOptions, transitionEffectOptions 배열에 들어가는 Element의 retainCount값을 1로 유지하기 위해서.
    //
    [destination.audioEnvelop release];
    [destination.effectOptions release];
    [destination.transitionEffectOptions release];
    
    return destination;
}

#pragma mark -
- (NSString *) description
{
    NSString *path = @"Unknown";
    NSString *fields = nil;
    
    if (self.clipSource != nil) {
        path = self.clipSource.shortDisplayText;
    } else {
        if (self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_IMAGE) {
            path = self.videoInfo.clipPath.lastPathComponent;
        } else if ( self.clipType == NXE_CLIPTYPE_AUDIO ) {
            path = self.audioInfo.clipPath.lastPathComponent;
        }
    }
    
    if (self.clipType == NXE_CLIPTYPE_VIDEO || self.clipType == NXE_CLIPTYPE_IMAGE) {
        NXEVisualClip *vinfo = self.videoInfo;
        fields = [NSString stringWithFormat:@"t:%d s:%d e:%d title.s:%d title.e:%d effect.d:%d effect.offset:%d effect.overlap:%d",
                  vinfo.totalTime, vinfo.startTime, vinfo.endTime,
                  vinfo.titleStartTime, vinfo.titleEndTime,
                  vinfo.effectDuration, vinfo.effectOffset, vinfo.effectOverlap
                  ];
    } else if ( self.clipType == NXE_CLIPTYPE_AUDIO ) {
        fields = [NSString stringWithFormat:@"t:%d s:%d e:%d", self.audioInfo.totalTime, self.audioInfo.startTime, self.audioInfo.endTime];
    }
    
    if (fields == nil) {
        fields = @"";
    }
    NSString *result = [NSString stringWithFormat:@"%@\n%@", path, fields];
    return result;
}

#pragma mark - Internal
- (void) setCropStartRect:(CGRect)rect
{
    if (!CGRectEqualToRect(rect, self.videoInfo.cropStartRectInPixels)) {
        self.videoInfo.cropStartRectInPixels = rect;
    }
}

- (void) setCropEndRect:(CGRect) rect
{
    if (!CGRectEqualToRect(rect, self.videoInfo.cropEndRectInPixels)) {
        self.videoInfo.cropEndRectInPixels = rect;
    }
}

- (void) setTransformRampWithMode:(CropMode) mode
{
    if (mode == CropModeCustom) {
        return;
    }
    
    if (self.videoInfo) {
        CGSize size = self.videoInfo.orientationRegardedSize;
        @autoreleasepool {
            // NESI-610: Avoid too many NXEEngine/NXEClipTransform instances left created
            CGSize renderSize = CGSizeMake(1280,720);//NXEEngine.instance.logicalRenderSize;
            [self updateTransformRampWith:[NXEClipTransform transformRampForMode:mode imageSize:size renderSize:renderSize]];
        }
    }
}
@end
