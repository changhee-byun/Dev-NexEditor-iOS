/**
 * File Name   : TemplateHelper.h
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *    	    Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
 *                         http://www.nexstreaming.com
 *
 *******************************************************************************
 *     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 *     KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 *     IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 *     PURPOSE.
 *******************************************************************************
 *
 */

#import <Foundation/Foundation.h>

@class NXEError;
@class NXEProject;

@interface TemplateComponent : NSObject

/*! \brief templateid, for debugging
 *  \discussion 관련 키(:id)
 *  \version 1.x
 */
@property (nonatomic) int templateid;

/*! \brief clip에서 볼륨 속성값을 변경 해 준다.
 *  \discussion 관련 키(:volume)
 *  \discussion 0 ~ 200, default: 100
 *  \version 1.x
 */
@property (nonatomic) int volume;

/*! \brief clip에서 speed value 속성값을 변경 해 준다.
 *  \discussion 관련 키(:speed_control)
 *  \discussion 0 ~ 400, default : 100
 *  \version 1.x
 */
@property (nonatomic) int speed_control;

/*! \brief clip에서 saturation 속성값을 변경 해 준다.
 *  \discussion 관련 키(:saturation)
 *  \discussion -255 ~ 255, default: 0
 *  \version 1.x
 */
@property (nonatomic) int saturation;

/*! \brief clip에서 brightness 속성값을 변경 해 준다.
 *  \discussion 관련 키(:brightness)
 *  \discussion -255 ~ 255, default: 0
 *  \version 1.x
 */
@property (nonatomic) int brightness;

/*! \brief clip에서 contrast 속성값을 변경 해 준다.
 *  \discussion 관련 키(:contrast)
 *  \discussion -255 ~ 255, default: 0
 *  \version 1.x
 */
@property (nonatomic) int contrast;

/** \brief clip에서 color effect정보를 변경 해 준다.
 *  \discussion 관련 키(:color_filter)
 *  \version 1.x
 */
@property (nonatomic) int color_filter;

/*! \brief effect 대한 duration 속성값을 변경 해 준다.
 *  \discussion 관련 키(:)
 *  \discussion
 *  \version 1.x
 */
@property (nonatomic) int duration;

/*! \brief effect 대한 maximum duration 속성값을 변경 해 준다.
 *  \discussion 관련 키(:duration_max)
 *  \discussion
 *  \version 2.0.0
 */
@property (nonatomic) int duration_max;

/*! \brief effect 대한 minimum duration 속성값을 변경 해 준다.
 *  \discussion 관련 키(:duration_min)
 *  \discussion
 *  \version 2.0.0
 */
@property (nonatomic) int duration_min;

/*! \brief clip effect 관련 된 정보인지 transition effect 관련 된 정보인지를 알려줌
 *  \discussion 관련 키(:type)
 *  \discussion v1.x에에서는 "scene"은 clip effect 그리고 "transition"은 transition effect 내용으로 분류되었지만, v2.0.0에서는 transition effect 정보 내용이 "scene"쪽에 포함되었음.
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *type;

/*! \brief clip에서 crop 속성값을 변경 해 준다.
 *  \discussion 관련 키(:crop_mode)
 *  \discussion fill -> 화면을 preview display에 늘려준다.
 *  \discussion fit -> 화면을 preview display에 맞게 맞춰준다.
 *  \discussion pan_rand -> crop 동작 시 기준이 되는 start, end position값을 임의의 값으로 설정해준다.
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *crop_mode;

/*! \brief video clip에 대한 crop 속성값을 변경 해 준다. 해당 키가 없을 경우 "crop_mode"를 따른다.
 *  \discussion 관련 키(:video_crop_mode)
 *  \discussion fill -> 화면을 preview display에 늘려준다.
 *  \discussion fit -> 화면을 preview display에 맞게 맞춰준다.
 *  \discussion pan_rand -> crop 동작 시 기준이 되는 start, end position값을 임의의 값으로 설정해준다.
 *  \version 2.0.0
 */
@property (nonatomic, assign) NSString *video_crop_mode;

/*! \brief image clip에 대한 crop 속성값을 변경 해 준다. 해당 키가 없을 경우 "crop_mode"를 따른다.
 *  \discussion 관련 키(:image_crop_mode)
 *  \discussion fill -> 화면을 preview display에 늘려준다.
 *  \discussion fit -> 화면을 preview display에 맞게 맞춰준다.
 *  \discussion pan_rand -> crop 동작 시 기준이 되는 start, end position값을 임의의 값으로 설정해준다.
 *  \version 2.0.0
 */
@property (nonatomic, assign) NSString *image_crop_mode;

/*! \brief clip에서 effect 속성값을 변경 해 준다.
 *  \discussion 관련 키(:effects)
 *  \discussion 만약 effect 반영을 원하지 않을 경우, "none"입력하거나 해당 pair를 생략합니다.
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *effects;

/*! \brief clip에 vignette속성값을 변경 해 주거나 image overlay동작할 수 있는 정보를 제공해준다.
 *  \discussion 관련 키(:vignette)
 *  \discussion clip에 vignette속성값 변경은
 *  \discussion clip,yes / clip,no
 *  \discussion image overlay동작할 수 있는 정보는
 *  \discussion overlay,+displaytime,±variation,true(false),+fadein_starttime,+fadein_duration,+fadeout_starttime,+fadeout_duration
 *  \discussion overlay overlay 동작.
 *  \discussion +displaytime overlay 동작시간
 *  \discussion ±variation clip start time 기준으로 overlay 시작시간을 변경
 *  \discussion true(false) fade animation 반영에 대한 가부
 *  \discussion +fadein_starttime fade in 시작시간
 *  \discussion +fadein_duration fade in 동작시간
 *  \discussion +fadeout_starttime fade out 시작시간
 *  \discussion +fadeout_duration fade out 동작시간
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *vignette;

/*! \brief "source_type"값이 SOLID경우, 클립의 색(by RGB, ARGB Format)을 설정 해 준다
 *  \discussion 관련 키(:solid_color)
 *  \discussion pre-defined color string값이 있는데, 아래와 같다.
 *  \discussion 'red', 'blue', 'green', 'black', 'white', 'gray', 'cyan', 'magenta', 'yellow', 'lightgray', 'darkgray', 'grey', 'lightgrey', 'darkgrey', 'aqua', 'fuschia', 'lime', 'maroon', 'navy', 'olive', 'purple', 'silver', 'teal'
 *  \discussion version 2.0.0부터 deprecated
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *solid_color;

/*! \brief object내용이 반영 되야 할 clip type
 *  \discussion 관련 키(:source_type)
 *  \discussion v1.x
 *  \discussion ALL -> clip type이 video, image구분없이 반영된다.
 *  \discussion VIDEO -> clip type이 video인 경우 반영한다.
 *  \discussion IMAGE -> clip type이 image인 경우 반영한다.
 *  \discussion EXTERNAL_VIDEO -> "external_video_path" 기록 된 값으로 video clip 만든 후 반영한다.
 *  \discussion EXTERNAL_IMAGE -> "external_image_path" 기록 된 값으로 image clip 만든 후 반영한다.
 *  \discussion SOLID -> solid clip 만든 후 반영한다.
 *  \discussion v2.0.0
 *  \discussion res_default -> clip type이 video, image 구분없이 반영된다.
 *  \discussion res_solid -> "res_path"에 지정된 값을 참고하여 내용을 반영한다.
 *  \discussion res_video -> "res_path"에 지정된 값을 참고하여 내용을 반영한다.
 *  \discussion res_image -> "res_path"에 지정된 값을 참고하여 내용을 반영한다.
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *source_type;

/*! \brief "source_type"이 res_solid, res_video 그리고 res_image로 설정이 되었을 경우, "res_path"가 설정되어야 하며 추가될 클립의 경로 혹은 색(by RGB, ARGB Format)이 설정되어야 함.
 *  \discussion 관련 키(:res_path)
 *  \discussion pre-defined color string값이 있는데, 아래와 같다.
 *  \discussion 'red', 'blue', 'green', 'black', 'white', 'gray', 'cyan', 'magenta', 'yellow', 'lightgray', 'darkgray', 'grey', 'lightgrey', 'darkgrey', 'aqua', 'fuschia', 'lime', 'maroon', 'navy', 'olive', 'purple', 'silver', 'teal'
 *  \version 2.0.0
 */
@property (nonatomic, assign) NSString *res_path;

/** \brief clip에서 color effect정보를 변경해준다.
 *  \discussion 관련 키(:lut)
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *lut;

/*! \brief "source_type"값이 EXTERNAL_VIDEO경우, 기록된 정보를 기반으로 클립을 생성해준다.
 *  \discussion 관련 키(:external_image_path)
 *  \discussion version 2.0.0부터 deprecated
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *external_image_path;

/*! \brief "source_type"값이 EXTERNAL_IMAGE경우, 기록된 정보를 기반으로 클립을 생성해준다.
 *  \discussion 관련 키(:external_video_path)
 *  \discussion version 2.0.0부터 deprecated
 *  \version 1.x
 */
@property (nonatomic, assign) NSString *external_video_path;

/*! \brief effect에 여러 개의 clip을 사용할 수 있는지 여부를 설정 해 준다.
 *  \discussion 관련 키(:apply_effect_on_res)
 *  \discussion 1 -> 여러 개의 clip에 하나의 effect를 적용한다.
 *  \discussion 0 -> 1개의 clip에 하나의 effect를 적용한다.
 *  \version 2.0.0
 */
@property (nonatomic, assign) int apply_effect_on_res;

/*! \brief effect의 audio resource를 설정하는데, 추가되는 audio resource는 effect duration보다 짧아야 함
 *  \discussion 관련 키(:audio_res)
 *  \discussion 기술 된 template파일을 기준으로 sound 폴더와 파일이름이 기록됨, 없을 경우 "none"
 *  \version 2.0.0
 */
@property (nonatomic, assign) NSString *audio_res;

/*! \brief "audio_res" 설정 된 audio effect의 시작 시간을 설정 해 준다. effect duration의 percentage로 적용되어 시작된다.
 *  \discussion res_video, res_image, res_solid 만든 클립을 기준으로 audio_res 시작 시간을 정해주고 있고, 앞뒤로 싱크 조정이 가능하다고 함.
 *  \discussion 관련 키(:audio_res_pos)
 *  \discussion 0 ~ 100
 *  \version 2.0.0
 */
@property (nonatomic, assign) int audio_res_pos;

/*! \brief transition effect 속성값을 변경 해 준다.
 *  \discussion 관련 키(:transition_name)
 *  \discussion 만약 effect 반영을 원하지 않을 경우, "none"입력하거나 해당 pair를 생략합니다.
 *  \version 2.0.0
 */
@property (nonatomic, assign) NSString *transition;

/*! \brief transition duration, millisecond 단위
 *  \discussion 관련 키(:transition_duration)
 *  \version 2.0.0
 */
@property (nonatomic, assign) int transition_duration;

/** \brief identifier
 *  \discussion 관련 키(:identifier)
 *  \version 2.0.0
 *  \since 1.x
 */
@property (nonatomic, assign) NSString *identifier;

/** \brief outro scene 기술 된 transition에 대해서 effect set을 구성하고, template 적용 시 identifier를 참고해서 업데이트 한다.
 *  \discussion 관련 키(:alternative_effect)
 *  \version 2.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSDictionary <NSString *, NSString*> *alternative_effect;

/** \brief outro scene 기술 된 transition에 대해서 transition set을 구성하고, template 적용 시 identifier를 참고해서 업데이트 한다.
 *  \discussion 관련 키(:alternative_transition)
 *  \version 2.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSDictionary <NSString *, NSString*> *alternative_transition;

/** \brief outro scene outro scene lut 대하여 부가적인 lut 셋을 구성하고 Template 적용 시 identifier을 참고 하여 lut 업데이트 한다
 *  \discussion 관련 키(:alternative_lut)
 *  \version 2.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSDictionary <NSString *, NSString*> *alternative_lut;

/** \brief outro scene Outro Scene의 audio effect 대하여 부가적인 audio effect 셋을 구성하고 Template 적용 시 identifier을 참고 하여 audio effect 업데이트 한다.
 *  \discussion 관련 키(:alternative_audio)
 *  \version 2.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSDictionary <NSString *, NSString*> *alternative_audio;

/** \brief Outro Scene의 audio effect 대하여 부가적인 audio effect pos 셋을 구성하고 Template 적용 시 identifier을 참고 하여 audio effect position을 업데이트 한다
 *  \discussion 관련 키(:alternative_audio_pos)
 *  \version 2.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSDictionary <NSString *, NSString*> *alternative_audio_pos;

/** \brief collage에 적용 될 source에 대한 정보를 기술한다.
 *  \version 3.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSArray <NSDictionary *> *drawInfos;

/** \brief Scene에 대한 alternative id를 설정하고, Outro 적용시 설정된 id를 참고 하여 적용한다.
 *  \version 3.0.0
 *  \since 1.x
 */
@property (nonatomic, assign) NSString *alternative_id;

/** \brief Loop에서 설정한 alternative id에 대응하는 outro  effect list를 배열 형태로 기술 한다.
 *  \version 3.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSArray <NSDictionary *> *alternative_outro;


- (instancetype)initWithRawDictionary:(NSDictionary *)dictionary templateId:(int)templateId;

@end

///

@interface TemplateHelper : NSObject

/*! \brief Template naming
 *  \discussion 관련 키(:template_name)
 *  \version 1.x
 */
@property (nonatomic, retain) NSString *name;

/*! \brief Template description
 *  \discussion 관련 키(:template_name)
 *  \version 1.x
 */
@property (nonatomic, retain) NSString *desc;

/*! \brief Template version
 *  \discussion 관련 키(:template_desc)
 *  \discussion v1.0(Template 1.0), v1.x(Template 1.x) 2.0.0(Template 2.0)
 *  \version 1.x
 */
@property (nonatomic, retain) NSString *version;

/*! \brief Template JSON 파일에 대한 mode
 *  \discussion 관련 키(:template_mode)
 *  \discussion 16v9, 9v16, 1v1
 *  \version 1.x
 */
@property (nonatomic, retain) NSString *mode;

/*! \brief Project에 설정되는 배경음원에 대한 경로
 *  \discussion 관련 키(:template_bgm)
 *  \version 1.x
 */
@property (nonatomic, retain) NSString *bgm;

/*! \brief Project에 설정되는 배경 음원에 대한 볼륨값
 *  \discussion 관련 키(:template_bgm_volume)
 *  \discussion 0.0 ~ 1.0 (default: 0.5)
 *  \version 1.x
 */
@property (nonatomic) float bgmvolume;

/*! \brief Project에 추가되는 이미지의 기본 길이
 *  \discussion 관련 키(:template_default_image_duration)
 *  \version 2.0.0
 */
@property (nonatomic) int image_duration;

/*! \brief 입력 된 project duration이 min_duration보다 작은경우, default_effect에 기록된 effect를 template으로 반영시키게 된다.
 *  \discussion 관련 키(:template_min_duration)
 *  \version 2.0.0
 */
@property (nonatomic) int min_duration;

/*! \brief 입력 된 project duration이 min_duration보다 작은경우, 반영하게 되는 template effect
 *  \discussion 관련 키(:template_default_effect)
 *  \version 2.0.0
 */
@property (nonatomic, retain) NSString *default_effect;

/*! \brief default_effect 반영할 때, effect duration을 조절할지에 대한 정보를 기술
 *  \discussion 관련 키(:template_default_effect_scale)
 *  \discussion 0: 시간이 조절되지 않음. 1: source duration에 맞게 설정이 됨
 *  \version 2.0.0
 */
@property (nonatomic) int effect_scale;

/*! \brief project volume fade in time설정, 단위는 milliseconds, default value: 200
 *  \discussion 관련 키(:template_project_vol_fade_in_time) 
 *  \version 2.0.0
 */
@property (nonatomic) int project_volume_fadein_time;

/*! \brief project volume fade out time설정, 단위는 milliseconds, default value: 5000
 *  \discussion 관련 키(:template_project_vol_fade_out_time)
 *  \version 2.0.0
 */
@property (nonatomic) int project_volume_fadeout_time;

/** \brief Project 에 설정되는 single template배경 음원에 대한 경로를 기록합니다.
 *  \discussion 관련 키(:template_single_bgm)
 *  \version 2.0.0
 *  \since 1.x
 */
@property (nonatomic, retain) NSString *bgm_single;

/*! \brief project volume fade in time설정, 단위는 milliseconds, default value: 200
 *  \discussion 관련 키(:template_single_project_vol_fade_in_time)
 *  \version 2.0.0
 */
@property (nonatomic) int project_volume_fadein_time_single;

/*! \brief project volume fade out time설정, 단위는 milliseconds, default value: 5000
 *  \discussion 관련 키(:template_single_project_vol_fade_out_time)
 *  \version 2.0.0
 */
@property (nonatomic) int project_volume_fadeout_time_single;

/*! \brief intro list
 *  \version 1.x
 */
@property (nonatomic, retain) NSArray <TemplateComponent *> *intro;

/*! \brief loop list
 *  \version 1.x
 */
@property (nonatomic, retain) NSArray <TemplateComponent *> *loop;

/*! \brief outro list
 *  \version 1.x
 */
@property (nonatomic, retain) NSArray <TemplateComponent *> *outro;

/** \brief single list
 *  \since version 1.x
 */
@property (nonatomic, retain) NSArray <TemplateComponent *> *single;

/*! \brief json형식의 file data를 가지고 TemplateHelper instance를 만든다.
 *  \param jsonData json형식의 file data
 *  \param deepScan intro,loop,outro list를 구성할 필요가 있으면 YES, 그 이외의 경우 NO 입력
 *  \param nxeError 구성 시 에러가 발생할 경우, NXEError instance를 통해 내용을 공유한다.
 */
- (instancetype)initWithJSONData:(NSData *)jsonData deepScan:(BOOL)deepScan nxeError:(NXEError **)nxeError;

@end
