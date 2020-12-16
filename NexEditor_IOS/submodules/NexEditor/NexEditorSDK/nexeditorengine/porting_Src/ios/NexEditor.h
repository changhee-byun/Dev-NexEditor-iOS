/******************************************************************************
 * File Name   : NexEditor.h
 * Description :
 *******************************************************************************
 
 Nextreaming Confidential Proprietary
 Copyright (C) 2006 Nextreaming Corporation
 All rights are reserved by Nextreaming Corporation
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/
#ifndef NexEditor_h
#define NexEditor_h

#import <UIKit/UIKit.h>
#import "NexMediaDef.h"
#import "NexEditorTypes.h"
#import "NexEditorAsyncAPI.h"
#import "FastPreview.h"

@class NexClipInfo;
@class NexSceneInfo;
@class NexDrawInfo;
@class EditorEventListener;

typedef enum _GETNexClipInfo_TYPE
{
    GET_NEXCLIPINFO_INCLUDE_SEEKTABLE = 0x00000001,
    GET_NEXCLIPINFO_CHECK_AUDIO_DEC = 0x00000010,
    GET_NEXCLIPINFO_CHECK_VIDEO_DEC = 0x00000100
} GETNexClipInfo_TYPE;

typedef enum _PLAYSTATE
{
    PLAYSTATE_NONE = 0,
    PLAYSTATE_IDLE = 1,
    PLAYSTATE_RUN = 2,
    PLAYSTATE_RECORD = 3,
    PLAYSTATE_PAUSE = 4,
    PLAYSTATE_RESUME = 5
} PLAYSTATE;

typedef struct VideoInfo
{
    int width;
    int height;
    int fps;
} VideoInfo;

@interface NexEditor : NSObject
{

}

@property (nonatomic, retain, readonly) CALayer* layer;

@property (nonatomic, readonly) NSString* kDecoderMaxCount;
@property (nonatomic, readonly) NSString* kDecoderMaxMemorySize;

/**
 * \deprecated Create object using [FastPreviewBuilder builder] instead of accessing this property
 */
@property (nonatomic, readonly) FastPreviewBuilder *fastPreviewBuilder  __attribute__((deprecated));
@property (nonatomic) BOOL isGLOperationAllowed;

/**
 * \brief The object with API methods that can handle asynchronous requests.
 * \code
 * [NexEditor.asyncAPI stopWithComplete:^(NexEditorError result) {
 *    ...
 * }];
 * \endcode
 */
@property (nonatomic, readonly) id<NexEditorAsyncAPI> asyncAPI;

@property (class, nonatomic, readonly) id<NexEditorAsyncAPI> asyncAPI;

#pragma mark - pre-defined functions

/**
 * \brief Loads visual and audio clips into the editor engine asynchronously.
 *        VIDEOEDITOR_EVENT_ASYNC_LOADLIST event will be sent when the operation complete.
 * \param visualClips TBD
 * \param audioClips TBD
 * \return NEXVIDEOEDITOR_ERROR_NONE if successfuly requested, one of NEXVIDEOEDITOR_ERROR values otherwise. Returning NEXVIDEOEDITOR_ERROR_NONE does not mean the operation was successful.
 */
- (NexEditorError)loadClipsAsync:(NSArray*)visualClips audioClips:(NSArray*)audioClips;

/**
 * \brief TBD
 * \param path TBD
 * \param info TBD
 * \param includeSeekTable TBD
 * \param userTag TBD
 * \return TBD
 */
- (int)getClipInfoSync:(NSString*)path
           NexClipInfo:(NexClipInfo*)info
      includeSeekTable:(bool)includeSeekTable
               userTag:(int)userTag;
/**
 * \brief TBD
 * \param time TBD
 * \param display TBD
 * \param idrFrame TBD
 * \return TBD
 */
- (NexEditorError)setTime:(int)time display:(int)display idrFrame:(int)idrFrame;

/**
 * \brief TBD
 * \return TBD
 */
- (int)captureCurrentFrame;
/**
 * \brief TBD
 * \param muteAudio TBD
 * \return TBD
 */
- (int)startPlay:(int)muteAudio;
/**
 * \brief Project Encoding 해 주는 함수
 * \param path Encoding 결과 파일에 대한 경로
 * \param width Encoding 결과 파일에 대한 width
 * \param height Encoding 결과 파일에 대한 height
 * \param bitrate Encoding 결과 파일에 대한 bitrate
 * \param maxFileSize Encoding 결과 파일에 대한 maximum file size
 * \param projectDuration Encoding 진행 할 TS값, CAUTION!! 처음부터 끝까지 진행하시려면 0값을 입력
 * \param fps Encoding 결과 파일에 대한 FPS, CAUTION!! 0을 입력하면 엔진에 정의 된 기본값(30FPS) 반영. 만약 임의의 값을 입력할 경우, inputedValue*100 형식으로 입력
 * \param projectWidth IOS 에서는 무시해도 되는 값
 * \param projectHeight IOS 에서는 무시해도 되는 값
 * \param samplingRate Encoding 결과 파일에 대한 SamplingRate
 * \param flag Rotation 0x0: 0 / 0x10: 90 / 0x20: 180 / 0x40: 270
 * \return Project Encoding 진행에 대한 결과 값
 * \since version 1.0.5
 */
- (int)encodeProject:(NSString*)path
               Width:(int)width
              Height:(int)height
             Bitrate:(int)bitrate
         MaxFileSize:(long)maxFileSize
     ProjectDuration:(int)projectDuration
                 FPS:(int)fps
        ProjectWidth:(int)projectWidth
       ProjectHeight:(int)projectHeight
        SamplingRate:(int)samplingRate
                Flag:(int)flag;
/**
 * \brief TBD
 * \param data TBD
 * \param flag TBD
 * \return TBD
 */
- (int)loadTheme:(NSString*)data flag:(Boolean)flag;
/**
 * \brief TBD
 * \param strData TBD
 * \return TBD
 */
- (int)setProjectEffect:(NSString*)strData;
/**
 * \brief TBD
 * \return TBD
 */
- (int)createProject;
/**
 * \brief TBD
 * \return TBD
 */
- (int)closeProject;
/**
 * \brief TBD
 * \return TBD
 */
- (int)destoryEditor;
/**
 * \brief TBD
 * \param iTag TBD
 * \return TBD
 */
- (int)commandMarker:(int)iTag;
/**
 * \brief TBD
 * \return TBD
 */
- (int)clearTrackCache;
/**
 * \brief TBD
 * \param tag TBD
 * \return TBD
 */
- (int)clearScreen:(int)tag;
/**
 * \brief TBD
 * \param name TBD
 * \return TBD
 */
- (NSString*)getProperty:(NSString*)name;
/**
 * \brief TBD
 * \param name TBD
 * \param value TBD
 * \return TBD
 */
- (int)setProperty:(NSString*)name Value:(NSString*)value;
/**
 * \brief TBD
 * \param clipPath TBD
 * \param thumbnailPath TBD
 * \param width TBD
 * \param height TBD
 * \param startTime TBD
 * \param endTime TBD
 * \param count TBD
 * \param flag TBD
 * \param userFlag TBD
 * \return TBD
 */
- (int)getClipVideoThumbs:(NSString*)clipPath
             ThumbnailPath:(NSString*)thumbnailPath
                     Width:(int)width Height:(int)height
                 StartTime:(int)startTime
                   EndTime:(int)endTime
                     Count:(int)count
                      Flag:(int)flag
                  UserFlag:(int)userFlag;
/**
 * \brief TBD
 * \param filePath TBD
 * \param sampleRate TBD
 * \param channels TBD
 * \param bitForSample TBD
 * \return TBD
 */
- (int)startVoiceRecorder:(NSString*)filePath SampleRate:(int)sampleRate Channels:(int)channels BitForSample:(int)bitForSample;
/**
 * \brief TBD
 * \param arrayPCM TBD
 * \param pcmLen TBD
 * \return TBD
 */
- (int)processVoiceRecoder:(Byte[])arrayPCM PCMLen:(int)pcmLen;
/**
 * \brief TBD
 * \param clipInfo TBD
 * \return TBD
 */
- (int)endVoiceRecorder:(NexClipInfo*)clipInfo;
/**
 * \brief TBD
 * \param option TBD
 * \param display TBD
 * \return TBD
 * \deprecated Use NexEditor.asyncAPI's -runFastPreviewCommand:display:complete instead
 */
- (NexEditorError) fastOptionPreview:(NSString*)option display:(int)display __attribute__((deprecated));

/// \brief Equivalent to [NexEditor.asyncAPI runFastPreviewCommand:command display:YES complete:NULL]
/// \param command Can be built by [FastPreviewBuilder buildCommand] or kFastPreviewCommandNormal
- (NexEditorError) runFastPreviewCommand:(id<FastPreviewCommand>) command;
/**
 * \brief TBD
 * \param exportFlag TBD
 * \param clipId TBD
 * \return TBD
 */
- (int)getTexNameForClipID:(int)exportFlag clipId:(int)clipId;
/**
 * \brief TBD
 * \return TBD
 */
+ (NexEditor*)sharedInstance;
/**
 * \brief TBD
 * \return TBD
 */
- (int)stopPlay;

/**
 * \brief TBD
 * \param userData TBD
 * \return TBD
 */
- (int)setUserData:(int)userData;
/**
 * \brief TBD
 * \return TBD
 */
- (int)setLayer;
/**
 * \brief TBD
 * \param x TBD
 * \param y TBD
 * \param width TBD
 * \param height TBD
 * \param scale TBD
 * \return TBD
 */
- (int)setLayerWithRect:(int)x
                      y:(int)y
                  width:(int)width
                 height:(int)height
                  scale:(int)scale;
/**
 * \brief TBD
 * \return TBD
 */
- (int)getDuration;

/**
 * \brief TBD
 * \return TBD
 */
- (EditorEventListener *)getEditorEventHandler;
/**
 * \brief theme renderer에서 관리하는 lut_texture_group_ 배열에서 lut_resource_id 해당하는 항목을 삭제 해 준다.
 */
- (void) releaseLUTTexture:(int)lut_resource_id;
//

#pragma mark - RenderItem
/**
 * \brief renderitem 정리를 위해서는 해당함수가 아닌 releaseRenderItem 호출해야 함. 해당함수는 미구현(?)상태입니다.
 * \param flags 1: prevew, 0: export
 * \return TBD
 */
- (int)clearRenderItems:(int)flags;


/**
 * \brief adjust project color with brightness, contrast and saturation.
 * \param brightness brightness value to be set
 * \param contrast contrast value to be set
 * \param saturation saturation value to be set
 * \note parameter value range:-1.0 ~ 1.0, default:0
 */
- (void) adjustProjectColorWith:(float)brightness contrast:(float)contrast saturation:(float)saturation;

/**
 * \brief TBD
 * \param strEffectID TBD
 * \param strEffectData TBD
 * \param flag 1: preview, 0: export
 * \return 정상적으로 load된 경우 0 그이외 경우 에러값을 보내준다.
 */
- (int)loadRenderItem:(NSString *)strEffectID effectData:(NSString *)strEffectData flags:(int)flag;
/**
 * \brief strEffectID 해당하는 renderItem 등록 한 index를 반환 해 주는 함수이고, 
 *          loadRenderItem:effectData:flags: 호출하지 않아 등록되지 않는 RenderItem인 경우, 
 *          내부적으로 동일한 동작을 수행하고 있습니다.
 * \param strEffectID TBD
 * \param exportFlag YES: export, NO: preview
 * \return renderItem 등록 한 id값
 */
- (int)createRenderItem:(NSString *)strEffectID exportFlag:(int)exportFlag;
/**
 * \brief effectID에 해당하는 RenderItem 리소스를 정리 해 준다.
 * \param effectID -1을 입력할 경우, RenderItem 모든 리소스를 정리 해 준다. 원하지 않을 경우, 정리하고자 하는 effectID값을 반영 해 준다.
 * \param exportFlag YES: export, NO: preview
 * \return TBD
 */
- (int)releaseRenderItem:(int)effectID exportFlag:(int)exportFlag;
/**
 * \brief drawing render item
 * \param effectID createRenderItem 통해서 받은 index
 * \param effectOption TBD
 * \param exportFlag YES: export, NO: preview
 * \param curTime rendering 될 current time stamp
 * \param startTime rendering start time stamp
 * \param endTime rendering end time stamp
 * \param matrix matrix 값
 * \param left left 값
 * \param top top 값
 * \param right right 값
 * \param bottom bottom 값
 * \param alpha alpha 값
 * \return 정상적 처리된 경우 0, 그 이외의 경우 에러값
 */
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
				 modeEnabled:(bool)modeEnabled;

#pragma mark - Watermark
/**
 * \brief TBD
 * \return TBD
 */
- (int)getWaterMarkInfo;

#pragma mark - Project Volume
/**
 * \brief TBD
 * \param fadeInTime TBD
 * \param fadeOutTime TBD
 * \return TBD
 */
- (int)setProjectVolumeFadeInTime:(int)fadeInTime fadeOutTime:(int)fadeOutTime;
/**
 * \brief TBD
 * \param projectVolume TBD
 * \return TBD
 */
- (int)setProjectVolume:(int)projectVolume;
/**
 * \brief TBD
 * \param manualVolume TBD
 * \return TBD
 */
- (int)setProjectManualVolumeControl:(int)manualVolume;
/**
 * \brief TBD
 * \param codecType TBD
 * \param isVideo TBD
 * \param isDecoder TBD
 * \return 인자로 제공 된 codecType에 대한 지원여부를 리턴해준다. 지원할 경우 YES, 그 이외에 NO
 */
- (BOOL) isSupportedCodecType:(NEX_CODEC_TYPE)codecType isVideo:(BOOL)isVideo isDecoder:(BOOL)isDecoder;

#pragma mark - Collage

- (int) configureDrawInfoListWithMasterList:(NSArray <NexSceneInfo*>*)sceneInfos slots:(NSArray <NexDrawInfo *>*)drawInfos;
- (int) updateDrawInfoWith:(NexDrawInfo *)drawInfo;
@end

#endif /* NexEditor_h */
