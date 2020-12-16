/**
 * \mainpage NexEditor&trade; SDK for iOS
 *
 *
 * \section legal Legal Notices
 *
 * \par Disclaimer for Intellectual Property
 * <i>This product is designed for general purpose, and accordingly the customer is
 * responsible for all or any of intellectual property licenses required for
 * actual application. NexStreaming Corp. does not provide any
 * indemnification for any intellectual properties owned by third party.</i>
 *
 * \par Copyright
 * Copyright for all documents, drawings and programs related with this
 * specification are owned by NexStreaming Corp. All or any part of the
 * specification shall not be reproduced nor distributed without prior written
 * approval by NexStreaming Corp. Content and configuration of all or any
 * part of the specification shall not be modified nor distributed without prior
 * written approval by NexStreaming Corp.
 *
 * Copyright &copy; 2002-2017 NexStreaming Corp. All rights reserved.
 *
 *
 *
 *
 * \section abstract Abstract
 *
 * The NexEditor&trade; SDK provides video editing services, including support for images and text,
 * with various theme, text, and transition effects, that allow application developers to
 * build custom applications with video editing features efficiently. The NexEditor&trade;&nbsp;SDK
 * has been built to be reliable and robust without any sacrifice in performance, and has
 * proven compatibility with international standards.
 *
 * This documentation is a work in progress and details will continually be added to provide additional
 * information to developers.
 *
 * Note: NexEditor&trade; SDK APIs were developed based on Objective-C.
 *
 *
 * \section support NexEditor&trade; SDK Capabilities and Limitations
 *
 * Capabilities               | Supported Value
 * ---------------------------|-----------------
 * iOS Version                | iOS 9 or above
 * Input Video File Container | MP4, 3GP, MOV
 * Input Audio File Container | MP3, AAC
 * Input Image Format         | PNG, JPEG
 * Input Video Codec          | H.264
 * Input Audio Codec          | MP3, AAC
 * Output File Container      | MP4
 * Output Video Codec         | H.264
 * Output Audio Codec         | AAC
 *
 *
 * \section terms NexEditor&trade; SDK Features and Terms
 *
 * \subsection features NexEditor&trade; SDK Features
 *
 * NexEditor&trade; SDKis the full-fledged mobile video editing SDK.
 * By integrating this SDK, it is possible to make professional editors with specific targets as well as easy-to-use video editors targeting general public.
 * Various effects such as zooming, panning, filters, and transitions can be applied to images and photos to create attractive video.
 * Also it supports instant preview after applying or switching editing features to multiple images and videos, therefore improves user experience with quick response time.
 * The following are the features of SDK.
 *
 * <b>Image/Text Layers</b>
 *
 * Images and texts can be overlaid on top of the videos or images.
 *
 * <b>Speed Control</b>
 *
 * Speed control for video is supported. The speed of an extracted video clip can be adjusted from 0.25 quarter speed) to 2 times the speed of the original video.
 * The audio pitch will not change even when the speed of a video is changed.
 *
 * <b>Trim</b>
 *
 * Selecting and cutting a specific section of a video clip (instead of using the whole original clip).
 *
 * <b>LUT Filter</b>
 *
 * Color filter effect can be applied to a video clip.
 *
 * <b>Template</b>
 *
 * Template is a predefined scenario, composed of trims, filters, transitions, stickers, and background music, to create amazing, professional-looking, and movie-like video,
 * simply by choosing one of the templates such as Old Film, Sports, Love, Family, etc.
 *
 * <b>Text Effect</b>
 *
 * Text effect can be used to add title and ending credit to edited video (trimmed/merged/template/etc.) as overlay type.
 * Several types of text effects are provided, and each of them includes different animation or different font.
 *
 * <b>BGM</b>
 *
 * BGM can be mixed to original video's sound. Music that fits the story makes the video perfect.
 * If template's default BGM is replaced with another one, output video can give different atmosphere.
 *
 * <b>Color Adjustment</b>
 *
 * The color of video clips can be adjusted, including allowing fine tuning of:
 * - Brightness: Adjustment of video brightness
 * - Contrast: Adjustment of video contrast
 * - Saturation: Adjustment of video color saturation
 *
 * \subsection sdkterms NexEditor&trade; SDK Terms
 *
 * <b>Clip</b>
 *
 * A clip can be an image clip (photos and pictures), a video clip (MP4), or an audio clip (MP3, AAC).
 * Supported Formats:
 * - Image: File (media) formats (supported by the /  that can be decoded by) iOS framework.
 * - Video: MP4 (with H.264 for video and AAC for audio).
 * - Audio: MP3 and AAC.
 *
 * <b>Project</b>
 *
 * Collection of clips to be used and edited in the NexEditor&trade; SDK.
 * Image clips and video clips are handled in the primary track and audio files are edited in a secondary track.
 *
 * <b>Clip Effect</b>
 *
 * Effect that can be added to a clip. Only one clip effect can be assigned to a clip at a time and the
 * effect may or may not include text.
 *
 * <b>Transition Effect </b>
 *
 * An effect used to transition between one clip and a second clip.  Depending on the transition effect added,
 * clips may overlap during the transition.
 *
 * <b>Engine</b>
 *
 * The main module, developed based on singleton pattern design, that is needed to preview and export a project.
 *
 * <b>Engine View</b>
 *
 * iOS layer needed to preview the project currently set to the engine.
 *
 * <b>Speed Control</b>
 *
 * A function to control the playback speed of a video clip.
 *
 * <b>Trim</b>
 *
 * A function to select and cut a specific section of a video clip (instead of using the whole original clip).
 *
 * <b>Image Duration</b>
 *
 * The duration of time to play an image clip in a project.
 *
 * <b>Units</b><p></p>
 * - Time:  Unless otherwise indicated, all NexEditor&trade;&nbsp; SDK API time units are measured in \c milliseconds.
 * - Playback speed:  The playback speed of a clip is controlled as a percentage.  Normal speed will be 100% and doubled
 * speed will be 200%.  Input values lower than 99 slow down the playback speed compared to the speed of the original video.
 *
 * \section setupguide NexEditor&trade; SDK Setup Guide
 *
 * \subsection folderstructure NexEditor&trade; SDK Folder Structure
 *
 * NexEditor&trade; SDK is structured as follows:
 *    \image latex figure1_4_1.png "NexEditor SDK Folder Structure" width = 6cm
 *    \image html figure1_4_1.png
 *
 * <b> Framework </b>
 *
 * This folder contains framework(s) NexEditor&trade;&nbsp; SDK provides.
 *
 * NexEditorFramework.framework in the framework folder is avaiable for both the simulator and device target.
 *    \image latex figure1_4_2_a.png "NexEditor SDK Framework" width = 10cm
 *    \image html figure1_4_2_a.png
 *
 * NexEditorFramework.framework in the dist folder is avaiable for only the device target.
 *    \image latex figure1_4_2_b.png "NexEditor SDK Framework for Device Target" width = 10cm
 *    \image html figure1_4_2_b.png
 *
 * <b> Sample </b>
 *
 * This folder contains two sample application projects which you can try for yourself using iOS simulator or device.
 * Xcode 8.0 or higher is recommended to run this samples.
 *    \image latex figure1_4_3.png "Sample Codes" width = 10cm
 *    \image html figure1_4_3.png
 *
 * \subsection howto How to import the NexEditor&trade; SDK Framework
 *
 * -# Open Xcode->View->Project to create a new project and click Next to fill out the information about the application.
 *    \image latex figure1_5_1.png "Create New Project - 1" width = 10cm
 *    \image html figure1_5_1.png
 *
 * -# Set the Product Name, the device type and the language. Click Next to select the project location, and then click Create.
 *    \image latex figure1_5_2.png "Create New Project - 2" width = 10cm
 *    \image html figure1_5_2.png
 *
 * -# Drag & drop NexEditorFramework.framework file onto “Embedded Binaries” tab.
 *    \image latex figure1_5_3.png "Create New Project - 3" width = 10cm
 *    \image html figure1_5_3.png
 *
 * -# After checking "Copy items if needed", tap on Finish button.
 *    \image latex figure1_5_3_a.png "Create New Project - 4" width = 10cm
 *    \image html figure1_5_3_a.png
 *
 * -# Confirm whether the NexEditorFramework.framework file is listed in both "Copy items if needed" and “Linked Frameworks and Libraries” tab.
 *    \image latex figure1_5_4.png "Create New Project - 5" width = 10cm
 *    \image html figure1_5_4.png
 *
 * Once completed the above steps, basic project settings are done properly and build will go through without an issue.
 *
 * \subsection sample Sample Application
 *
 * APIdemos is a sample application included in the NexEditor&trade; SDK to help you understand NexEditor&trade; SDK APIs.
 *
 * To run the APIDemos application, open the APIDemos.xcworkspace file with Xcode.
 *    \image latex figure1_6_1.png "Sample Application - 1" width = 10cm
 *    \image html figure1_6_1.png
 *
 * To run the APIDemos on your iPhone device, change the "Bundle Identifier" to yours.
 *    \image latex figure1_6_2.png "Sample Application - 2" width = 10cm
 *    \image html figure1_6_2.png
 *
 * \section implementation NexEditor&trade; SDK Architecture and Implementation
 *
 * \subsection flowchart NexEditor&trade; SDK Flowchart
 *
 * The image below is a flowchart of the NexEditor&trade; SDK.
 * \c NXEClip generates clips which will be put into \c NXEProject to be edited. Edited \c NXEProject projects will be passed into \c NXEEngine to be previewed or exported.
 * Care must be taken to add \c NXEClip clips into an \c NXEProject to edit them.
 *    \image latex figure1_7_2.png "NexEditor SDK Flow Chart" width = 10cm
 *    \image html figure1_7_2.png
 *
 * \section howtouse How to Use NexEditor&trade; SDK
 *
 * In this section, you will find step-by-step example codes to create a basic video editing application with NexEditor&trade; SDK.
 * \note Example codes are provided for illustration purpose only. They may not work properly if simply copy & paste them.
 *
 * \subsection preview Creating a project for preview
 *
 * -# Initialize a video editing engine (NXEEngine). Once engine is created successfully, set an aspect ratio to work with. If not set explicitly, 16:9 aspect ratio will be used by default.
 *
 * -# Create a project (NXEProject)
 *
 * -# Create a clip (NXEClip) and add it to the project.
 *
 * -# Connect it to the engine
 *
 * -# Start preview by calling play
 *
 *
 \code
     self.engine = [NXEEngine instance];
     [self.engine setPreviewWithCALayer:self.view.layer previewSize:self.view.bounds.size];
     
     NXEProject *project = [[NXEProject alloc] init];
     
     PHAsset *asset = // fetch a PHAsset from Photos using PHAssetCollection;
     NXEClipSource *clipSource = [NXEClipSource sourceWithPHAsset:asset];
     NXEClip *clip = [NXEClip clipWithSource:clipSource error:nil];
     
     project.visualClips =  @[clip];
     [self.engine setProject:project];
 
     [self.engine preparedEditor {
        [self.engine play];
        }];
 \endcode
 *
 *
 * \subsection export Exporting project
 *
 * -# Seek to the beginning of the project
 *
 * -# Export to a file with resolution and bitrate parameters
 *
 \code
     NSString *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES)[0];
     path = [path stringByAppendingPathComponent:@"Export.mp4"];
     
     __weak NXEEngine *engine = self.engine;
     
     [engine setCallbackWithEncodingProgress:^(int percent) {
        // export progress
     }  encodingEnd:^(NXEError *error) {
        // export finish
     }];

     [engine setCallbackWithSetTime:^(int seekDoneTS) {
         int width = 1280;
         int height = 720;
         int bitrate = 512000;
         [engine exportProject:path
                         Width:width
                        Height:height
                       Bitrate:bitrate
                   MaxFileSize:LONG_MAX
               ProjectDuration:0
              ForDurationLimit:false
                  RotationFlag:NXE_ROTATION_0];
     }];
     [engine seek:0];
 \endcode
 *
 * \subsection previewlayer NXELayerEditorView and Layer Manipulation
 *
 * Instead of a UIView, using \c NXELayerEditorView allows users to resize, move, or rotate layers added to the current project set to NXEEngine.
 *    \image latex figure1_8_1.png "Resize / Move / Rotate layer" width = 10cm
 *    \image html figure1_8_1.png
 *
 * To use an \c NXELayerEditorView on a View Controller, follow the steps below.
 *
 * -# Drag & drop the UIView towards View Controller, then select the View.
 *    \image latex figure1_8_2.png "How to Use NXELayerEditorView - 1" width = 10cm
 *    \image html figure1_8_2.png
 *
 * -# Select NXELayerEditorView in the Custom Class section in the identity inspector.
 *    \image latex figure1_8_3.png "How to Use NXELayerEditorView - 2" width = 10cm
 *    \image html figure1_8_3.png
 *
 * -# Connect NXELayerEditorView as an IBOutlet property in the View Controller source file.
 *    \image latex figure1_8_5.png "How to Use NXELayerEditorView - 3" width = 10cm
 *    \image html figure1_8_5.png
 *
 *    \image latex figure1_8_6.png "How to Use NXELayerEditorView - 4" width = 10cm
 *    \image html figure1_8_6.png
 *
 *
 * \subsection layer Layers
 *
 * You can add unlimited numbers of image or text layers, but video layers can only be added upto three layers due to performance.
 *
 * Once successfully added a layer and no changes made to its layer properties, all layers will be aligned in the center of the NXELayerEditorView.
 * Image and text layers will be displayed for the first six seconds and video layer will show upto the length of the video clip.
 *
 \code
     UIImage *image = ...;
     NXEImageLayer *imageLayer = [[NXEImageLayer alloc] initWithImage:image
                                                                point:CGPointNXELayerCenter];
     
     NXETextLayer *textLayer = [[NXETextLayer alloc] initWithText:@"Hello"
                                                             font:nil
                                                             point:CGPointNXELayerCenter];
 
     NSString *path = ...;
     NXEVideoLayer *videoLayer = [[NXEVideoLayer alloc] initVideoLayerWithPath:path
                                                                         point:CGPointNXELayerCenter];
 
     self.engine.project.layers = @[imageLayer, textLayer, videoLayer];
 \endcode
 *
 * \subsection editClps Edit Clips
 *
 * It is recommended to use the pre-defined properties of \c NXEAudioClip and \c NXEVideoClip in read only.
 * To change \c NXEClip properties, use the APIs defined in the table below.
 *
 * <b> Effect APIs </b>
 *
 * Effect API                        | Description
 * --------------------------------- | -----------------------------------------------------
 * setTransitionEffect:              | set a transition effect to a clip.
 * setTransitionEffect:effectOption: | set a transition effect with effec option to a clip.
 * setTransitionEffectDuration:      | set the duration of a transition effect.
 * setClipEffect:                    | set a clip effect to a clip.
 * setClipEffect:effectOption:       | set a clip effect with option to a clip.
 * setEffectShowTime:endTime:        | set the start and end time of a clip effect.
 *
 *
 * <b> Clip APIs </b>
 *
 * Clip API                          | Description
 * --------------------------------- | -----------------------------------------------------
 * setDisplayStartTime:endTime:      | set the start and end time to display a clip.
 * setCropMode:                      | set a crop mode to a clip.
 * setImageClipDuration:             | set the duration of an image clip.
 * setTrim:EndTrimTime:              | set the start and end trim time to a video clip.
 * setSpeed:                         | set a playback speed to a video clip.
 * setLut:                           | set a color effect to a clip.
 * setVignette:                      | set vignette effect to a clip.
 * setCompressorValue:               | set a compressor value to an audio clip.
 * setPanLeftValue:                  | set a pan left value to an audio clip.
 * setPanRightValue:                 | set a pan right value to an audio clip.
 * setPitchValue:                    | set a pitch value to an audio clip.
 * setVoiceChangerValue:             | set a voice changer value to an audio clip.
 *
 * \subsection assetlibrary Asset Library
 *
 * NexEditor Assets such as Template and Text Effect can be provided separately.
 * These assets can be retrieved and applied to NexEditor projects using NXEAssetLibrary and other related APIs if properly bundled into your App project.
 *
 * Once you have the copy of the assets provided by NexStreaming Corp., the directory structure might look like below.
 *
 *    \image latex figure1_9_1.png "How to Add Asset Packages - 1" width = 10cm
 *    \image html figure1_9_1.png
 *
 * To bundle assets to your App project, follow the steps below.
 *
 * -# Drag and drop the asset packages onto the project navigator
 *
 *    \image latex figure1_9_2.png "How to Add Asset Packages - 2" width = 10cm
 *    \image html figure1_9_2.png
 *
 * -# Select options like below image to add resources as in bundle
 *
 *    \image latex figure1_9_3.png "How to Add Asset Packages - 3" width = 10cm
 *    \image html figure1_9_3.png
 *
 * -# Confirm whether the asset packages is listed in the "Copy Bundle Resources" tab
 *
 *    \image latex figure1_9_4.png "How to Add Asset Packages - 4" width = 10cm
 *    \image html figure1_9_4.png
 *
 * The following code snippets demonstrate how to retrieve and apply to NexEditor projects the assets bundled in your App.
 *
 * It is recommended to add asset source directories while initializing the App. For example, if asset packages are bundled in 'AssetPackages' directory as a folder reference,
 *
 \code
     - (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
         NSURL *bundlePackagesURL = [[NSBundle mainBundle] URLForResource:@"AssetPackages" withExtension:nil];
         if(bundlePackagesURL) {
             [NXEAssetLibrary addAssetSourceDirectoryURL:bundlePackagesURL];
         }
     }
 \endcode
 *
 * The following code is to configure the current project with the first Template asset item from the query.
 * Please note that a template asset item for 16:9 aspect type has been selected.
 *
 \code
     NXEAssetLibrary *assetLibrary = [NXEAssetLibrary instance];
     [assetLibrary itemsInCategory:NXEAssetItemCategory.Template completion:^(NSArray<NXEAssetItem *> * items) {
         NSPredicate *predicate = [NSPredicate predicateWithFormat:@"aspectType=%d", (NXEAspectType)NXEAspectTypeRatio16v9];
         NSArray *assets = [items filteredArrayUsingPredicate:predicate];
         NXETemplateProject *project = [[NXETemplateProject alloc] initWithTemplateAssetItem: (NXETemplateAssetItem *)assets[0] clips:clips error: nil];
     }];
 \endcode
 *
 * The following code is to configure the current project with the first Text Effect asset item from the query.
 * Please note that a Text Effect asset item for 16:9 aspect type has been selected.
 *
 \code
     NXEAssetLibrary *assetLibrary = [NXEAssetLibrary instance];
     [assetLibrary itemsInCategory:NXEAssetItemCategory.textEffect completion:^(NSArray<NXEAssetItem *> * items) {
         NSPredicate *predicate = [NSPredicate predicateWithFormat:@"aspectType=%d", (NXEAspectType)NXEAspectTypeRatio16v9];
         NSArray *assets = [items filteredArrayUsingPredicate:predicate];
         NXETextEffectParams *effectParams = [NXETextEffectParams alloc] init];
         effectParams.introTitle = @"intro title";
         effectParams.introSubtitle = @"intro subtitle";
         effectParams.outroTitle = @"outro title";
         effectParams.outroSubtitle = @"outro subtitle";
         NXETextEffect *textEffect = [[NXETextEffect alloc] initWithId:((NXEAssetItem *)assets[0]).itemId params:effectParams];
         [self.engine.project setTextEffect:textEffect error:nil];
     }];
 \endcode
 *
 */


/******************************************************************************
 * File Name   :	NXEEngine.h
 * Description :
 ******************************************************************************
 THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
 KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
 PURPOSE.
 
 NexStreaming Proprietary and Confidential
 Copyright (C) 2002~2017 NexStreaming Corp. All rights reserved.
 www.nexstreaming.com
 ******************************************************************************/

#import <UIKit/UIKit.h>
#import "NXEEditorType.h"
#import "NXEEditorErrorCode.h"

@class NXELayer;
@class NXEProject;
@class NXELayer;
@class NXEError;
@class NXEThumbnailUICallbackBlock;

/** \class NXEEngine
 *  \brief NXEEngine class is the main video editing core engine object and it defines all properties and methods to manage an engine.
 *
 *  This class handles communications between core engine and other objects and also manages layer elements operation. Here are some examples;<p>
 *  -# Delivers action requests, such as preview or export, to the engine.<p>
 *  -# Executes relayed request in core engine and returns the result of the operation. In some cases, the result is passed to UI object.<p>
 *  -# Handles layer item management.
 *  \since version 1.0.5
 */
@interface NXEEngine : NSObject

/** \brief This method gets a <b>NXEProject</b> instance, which is the result of setProject.
 *  \since version 1.0.5
 */
@property (nonatomic, readonly) NXEProject* project;

/** \brief Screen aspect ratio, 16:9 ratio by default.
 *  \note Setting NXEAspectTypeCustom is not accepted and discard. To set NXEAspectTypeCustom, use -setAspectType:withRatio: instead.
 *  \see NXEAspectType type.
 *  \since version 1.1.0
 */
@property (nonatomic) NXEAspectType aspectType;

/** \brief Screen aspect ratio, set by setAspectType:withRatio:.
 *  \see NXESizeInt type.
 *  \since version 1.1.0
 */
@property (nonatomic, readonly) NXESizeInt aspectRatio;

/**
 * \brief TBD
 * \see aspectRatio
 * \since version 1.5.1
 */
@property (nonatomic, readonly) CGSize logicalRenderSize;

/** \brief This method creates an NXEEngine instance, which controls actions such as play, pause, encoding, etc in the preview.
 *  \note Release the instance immediately if no longer used so the internal resource deallocation can start as soon as possible. All NXEEngine instances share the same internal resource. Hence, changing a property value of an instance affects all the other NXEEngine instances.
 *  \return NXEEngine instance
 *  \since version 1.1.0
 */
//+ (instancetype) instance;

#pragma mark - Set/Get properties related to engine

/** \brief Sets a screen aspect ratio, 16:9 ratio by default.
 *  \note Only 16:9 and 9:16 ratios are supported in current version. If unsupported ratio is set, effects may not work properly.
 *        This property is deprecated. Use \c aspectType property instead.
 *  \see NXEAspectType type.
 *  \since version 1.0.5
 *  \deprecated Use aspectType property
 */
+ (void)setAspectMode:(NXEAspectType)aspect __attribute__((deprecated));

/** \brief Returns the current screen aspect ratio.
 *  \note This property is deprecated. Use \c aspectType property instead.
 *  \return A screen aspect ratio.
 *  \see NXEAspectType type.
 *  \since version 1.0.5
 *  \deprecated Use aspectType property
 */
+ (NXEAspectType)getAspectMode __attribute__((deprecated));
/** \brief Sets a screen aspect ratio, NXEAspectTypeCustom with custom ratio, other than NXEAspectTypeRatio16v9 or NXEAspectTypeRatio9v16.
 *  \note This method accepts NXEAspectTypeCustom with ratio parmeter as well as other types defined in NXEAspectType.
 *  \param type NXEAspectTypeCustom with ratio param values. If this value is other than NXEAspectTypeCustom, ratio parmeter will be discarded.
 *  \param ratio A structure with ratio values. If width or height of the structure has value of 0, it will be discarded and aspect type will not be changed.
 *  \see NXEAspectType type.
 *  \since version 1.1.0
 */
- (void) setAspectType:(NXEAspectType) type withRatio:(NXESizeInt) ratio;

/** \brief Sets a log level to gather logs from core engine, no logs are gathered by default.
 *  \param err A Boolean value indiates whether to include Error type logs.
 *  \param warning A Boolean value indiates whether to include Warning type logs.
 *  \param flow A Boolean value indiates whether to include Flow type in the log.
 *  \since version 1.0.19
 *  \deprecated Use NexLogger.setLogCategory:withLevel instead.
 */
+ (void)setLogEnableErr:(BOOL)err warning:(BOOL)warning flow:(BOOL)flow __attribute__((deprecated));;

/** \brief Clears the current screen.
 *  \note Once called, current screent will be changed to black screen. Call this method to reset the screen to the original state.
 *  \since version 1.0.15
 */
- (int)clearScreen;

/** \brief Creates a new preview layer with the speficied preview size.
 *  \param layer A CALayer object.
 *  \param previewSize A preview size.
 *  \since version 1.0.5
 */
- (void)setPreviewWithCALayer:(CALayer*)layer previewSize:(CGSize)previewSize;

/** \brief Sets a project to be played or exported.
 *  \param project An NXEProject object
 *  \since version 1.0.5
 */
- (void)setProject:(NXEProject*)project;

/** \brief Returns the version string of the NexEditor SDK.
 *  \note Renamed from getVersionSDK.
 *  \return A NexEditor SDK version string
 * \since version 1.2
 */
+ (NSString *) sdkVersionString;


#pragma mark - AuthorizationData
/** \brief AuthrizationData which is for StoreKit use
 *  \since version 1.3.0
 */
- (NSDictionary*) authorizationData;

#pragma mark - Playback
/** \brief Checks whether the engine is ready for use.
 *  \note Before making any requests to the engine, call this method to check if the engine is ready to process any requests.
 *  \param onPrepared A callback block to be invoked when the engine initialization is completed.
 * \since version 1.0.19
 */
- (void)preparedEditor:(void(^)(void))onPrepared;

/** \brief Returns the current timestamp.
 *  \return A timestamp, in milliseconds.
 * \since version 1.0.19
 */
- (int)getCurrentPosition;

/** \brief Sets callback blocks for the play progress, play start and play stop events.
 *  \param onPlayProgress A callback block to be invoked when the play is in progress. The current timestamp is delivered to the callback block.
 *  \param onPlayStart A callback block to be invoked when the play is started.
 *  \param onPlayEnd A callback block to be invoked when the play is ended.
 *  \since version 1.0.19
 */
- (void)setCallbackWithPlayProgress:(void(^)(int currentTime))onPlayProgress
                          playStart:(void(^)(void))onPlayStart
                            playEnd:(void(^)(void))onPlayEnd;

/** \brief Plays a project from the beginning.
 *  \return A return value, ERROR_NONE(0) for success.
 *  \since version 1.0.5
 */
- (int)play;

/** \brief Stops the play operation synchronously and delivers the status to a callback block.
 *  \param onStopComplete A callback block to be invoked when the synchronous stop operation is completed.
 *  \since version 1.0.5
 *  \deprecated Use -stop instead.
 */
- (void)stopSync:(void(^)(ERRORCODE errorcode))onStopComplete  __attribute__((deprecated));

/** \brief Stops the play operation asynchronously and delivers the status to a callback block.
 *  \param onStopComplete A callback block to be invoked when the asynchronous stop operation is completed.
 *  \since version 1.0.5
 */
- (void)stopAsync:(void(^)(ERRORCODE errorcode))onStopComplete;
#define stopASync stopAsync /* fix typo silently */

/**
 * \brief Stops the play operation. This method blocks until the operation completes and return error code, if any.
 * \return ERROR_NONE if successful, an error code otherwise.
 * \since version 1.x
 */
- (ERRORCODE) stop;

/** \brief Pauses the play operation.
 *  \return A return value, ERROR_NONE(0) for success.
 *  \since version 1.0.5
 */
- (int)pause;

/** \brief Resumes the play operation.
 *  \return A return value, ERROR_NONE(0) for success.
 *  \since version 1.0.5
 */
- (int)resume;

/** \brief Sets a callback block for the seek complete event.
 *  \param onSeekDone A callback block to be invoked when the seek operation is completed. The timestamp of seek completion is delivered to the callback block.
 *  \since version 1.0.19
 */
- (void)setCallbackWithSetTime:(void(^)(int seekDoneTS))onSeekDone;

/** \brief Sets the current playback time to the speficied time.
 *  \note For this method to work properly, methods like <tt>stop()</tt> or <tt>pause()</tt> should be called before.
 *        Otherwise, <tt>seek()</tt> method will fail.
 *  \param time The time to seek, in milliseconds.
 *  \since version 1.0.5
 */
- (void)seek:(int)time;

/** \brief Sets the current playback time to the closest IDR-frame or I-frame to the speficied time.
 *  \param time The time to seek, in milliseconds.
 *  \since version 1.0.5
 */
- (void)seekIDRorI:(int)time;

/** \brief Sets the current playback time to the closest IDR-frame to the speficied time.
 *  \param time The time to seek, in milliseconds.
 *  \since version 1.0.5
 */
- (void)seekIDROnly:(int)time;

/** \brief Sets callback blocks for the export progress and export complete events.
 *  \param onExportProgress A callback block to be invoked when the export is in progress.
 *  \param onExportEnd A callback block to be invoked when the export is completed.
 *  \since version 1.0.5
 */
- (void)setCallbackWithEncodingProgress:(void(^)(int percent))onExportProgress
                            encodingEnd:(void(^)(NXEError *nxeError))onExportEnd;

/** \brief Exports a project with the specified parameters applied.
 *  \param path The path to save the exported file.
 *  \param width A width value of the content to be encoded.
 *  \param height A height of the content to be encoded.
 *  \param bitrate A bitrate of the content to be encoded.
 *  \param maxFileSize The maximum file size of the exported file.
 *  \param projectDuration A project duration to set forcefully. If forDurationLimit is TRUE, enter the desired duration value. If forDurationLimit is FALSE, enter 0.
 *  \param forDurationLimit A Boolean value whether to force project duration limit or not.
 *  \param rotationFlag A rotation angle value, NXE_ROTATION_0: 0 degree / NXE_ROTATION_90: 90 degree / NXE_ROTATION_180: 180 degree / NXE_ROTATION_270: 270 degree
 *  \return A return value, 1 for successful export, 0 for error.
 *  \deprecated Use exportToPath:withParams instead
 *  \since version 1.0.5
 */
- (int)exportProject:(NSString*)path
               Width:(int)width
              Height:(int)height
             Bitrate:(int)bitrate
         MaxFileSize:(long)maxFileSize
     ProjectDuration:(int)projectDuration
    ForDurationLimit:(BOOL)forDurationLimit
        RotationFlag:(NXERotationAngle)rotationFlag __attribute__((deprecated));

/** \brief Exports a project with the specified parameters applied.
 *  \param path The path to save the exported file.
 *  \param params sturcture that contains parameter for exporting
 *  \since version 1.3.6
 */
- (int)exportToPath:(NSString*)path withParams:(NXEExportParams)params;

/** \brief Previews a project on the screen with the specified options.
 *  \param option An option type.
 *  \param optionValue An option value.
 *        - NXE_NORMAL, enter -1.
 *        - NXE_COLOR_ADJUSTMENT, enter comma-separated values in brighness, contrast, saturation sequence, ranging from 0 through 255. For example, \@"255,255,255".
 *        - NXE_TINT_COLOR, enter comma-separated values in red, green, blue sequence, ranging from 0 through 255. For example, \@"255,255,255".
 *        - NXE_CROP_RECT, enter comma-separated values in left, top, right, bottom sequence, ranges in CGRectMake(0,0,1280,720). For example, \@"0,0,640,360".
 *        - NXE_NOFX, enter 0 or 1. For example, \@"1".
 *        - NXE_SWAP_VERTICAL, enter 0 or 1. For example, \@"1".
 *        - NXE_CTS, enter a timestamp in milliseconds. For example, \@"10".
 *        - NXE_360_VIDEO, enter comma-separated values in valid, horizontal, vertical sequence. valid in 0 or 1, horizon- tal in 0 to 360, vertical in 0 to 90. For example, \@"1,180,45".
 *  \param display A display option value, 1 for display on screen, 0 for no display.
 *  \return A return value, 1 for successful preview, 0 for error.
 *  \since version 1.0.19
 */
- (int)fastOptionPreview:(NXEFastOption)option optionValue:(NSString *)optionValue display:(BOOL)display;

/** \brief TBD
 *  \since version: 1.5.0
 */
- (void) updatePreview;

#pragma mark - Get Thumbnail/Frame

/** \brief The method captures the current frame when the preive is running
 *  \param captureBlock UIImage and ERROR_NONE if there isn't an error, otherwise, nil and error value.
 *  \since version 1.2
 */
- (void)captureFrameWithBlock:(void (^)(UIImage *image, ERRORCODE errorCode))captureBlock;

@end


NXEExportParams NXEExportParamsMake(int width, int height);
