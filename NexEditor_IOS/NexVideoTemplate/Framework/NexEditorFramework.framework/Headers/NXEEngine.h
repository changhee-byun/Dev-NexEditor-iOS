/**
 * \mainpage NexEditor&trade;SDK for iOS 
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
 * &copy;Copyright 2016 NexStreaming Corp. All rights reserved.
 *  
 *
 *
 *
 * \section abstract Abstract
 * 
 * The NexEditor&trade;&nbsp;SDK provides video editing services, including support for images and text,
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
 *
 *
 * \section support NexEditor&trade; SDK Capabilities and Limitations
 * 
 * <b>iOS versions:</b>
 * - The NexEditor SDK supports iOS 8 and above.
 *
 * <b>Input File Containers: </b>
 * - Video : MP4, 3GP and MOV
 * - Audio : MP3 and AAC
 * 
 * <b> Input Image Format: </b>
 * - PNG and JPEG
 * 
 * <b>Input Video Codec:</b>
 * - H.264
 *
 * <b> Input Audio Codec: </b>
 * - AAC and MP3
 *
 * <b>Output File Container:</b>
 * - MP4
 *
 * <b>Output Video Codec:</b>
 * - H.264
 *
 * <b>Output Audio Codec:</b>
 * - AAC
 *
 *
 *
 *
 *
 * \section concept NexEditor&trade; SDK Concept
 *
 * \subsection features NexEditor&trade; SDK Main Features
 *
 * <b>Multi-Video Editing</b>
 * - Photo : Make a video using a number of images with different formats.
 * - Video : Edit several videos and images in different formats at the same time.
 * 
 * <b>Basic Editing</b> 
 *  <p>Most video editing can be done with the basic editing features, including the following: adjusting the duration of images, moving, zooming, panning, rotating images, extracting clips of video, and merging several videos into a single video file.
 *  Just by using these basic editing features, detailed edits to the millisecond are possible with time-adjusting APIs.</p>
 *
 * <b>Instant Preview</b>
 * <p>Instant preview is possible because there is no need to convert the videos. </p>
 *
 * <b>Export</b>
 * <p>The videos can be converted into various resolutions (1080p, 720p, 640p, 480p).
 *
 * NOTE:  Depending on the chipset, supported resolutions may vary. </p>
 *
 * <b>Image Stickers on Video</b>    
 * <p>Images can be overlaid on top of the videos or other images. </p>
 *
 * <b>Various Themes</b> 
 * <p>Various themes are provided to allow videos to be edited with effects set automatically.</p> 
 *       \image latex theme_thumbnails.png "Theme Thumbnails" width = 15cm
 *       \image html theme_thumbnails.png 
 * 
 *
 * <b>Various Transition Effects</b>
 * <p>Various transition effects including PIP transition, 3D transition, and Mask transition are provided. </p>
 *       \image latex transitioneffect_icons.png "Transition Effect Icons" width = 15cm
 *       \image html transitioneffect_icons.png
 *
 * <b>Various Clip Effects</b>
 * <p>Various clip effects and text effects for video and image clips are supported. </p>
 *     \image latex clipeffect_icons.png "Clip Effect Icons" width = 15cm
 *       \image html clipeffect_icons.png   
 *
 * <b>Speed Control</b>
 * <p>Speed control for video is supported. The speed of an extracted video clip can be adjusted from 0.25
 * quarter speed) to 2 times the speed of the original video. The audio pitch will not change even when
 * the speed of a video is changed.</p>
 *
 * <b>Color Adjustment</b>
 * <p> The color of video clips can be adjusted, including allowing fine tuning of:</p>
 *  - Brightness:  Adjustment of video brightness,
 *  - Contrast:  Adjustment of video contrast,
 *  - Saturation:  Adjustment of video color saturation, and
 *  - Tint:  Adjustment of video tint (or use of color filters).
 *
 * <b>Multi-Audio Editing</b>
 * <p>Multiple audio files can be used when editing video. </p>
 * 
 * <b>Multi-Audio Track</b>
 * <p>Multiple audio files, to a maximum of three audio tracks, can be edited at the same time.</p>
 *
 * <b>Audio Fade-in & Fade-out</b>
 * <p>At the beginning and end of a video clip, fade-in and fade-out effects can be applied to prevent audio from starting or ending awkwardly.</p>
 *
 * <b>Audio Envelope </b>
 * <p>The volume of the audio can be adjusted to different levels in different sections of the content. </p>
 *
 * <b>Video Thumbnail & Audio PCM Graph</b>
 * - Video: Extracts thumbnail of the video clip.
 * - Audio: Extracts PCM Graph (Thumbnail) of the audio clip. 
 *
 *
 *
 *
 * \subsection terms NexEditor&trade; SDK Terms
 *
 * <b>Clip</b>
 * <p>A clip can be an image clip (photos and pictures), a video clip (MP4), or an audio clip (MP3, AAC).
 * Supported Formats: </p>
 * - Image:  File (media) formats (supported by the /  that can be decoded by) iOS framework.
 * - Video:  MP4 (with H.264 for video and AAC for audio).
 * - Audio:  MP3 and AAC.
 *
 * <b>Project</b>
 * <p>Collection of clips to be used and edited in the NexEditor&trade; SDK.
 * Image clips and video clips are handled in the primary track and audio files are edited in a secondary track. </p>
 *
 * <b>Clip Effect</b>
 * <p>Effect that can be added to a clip. Only one clip effect can be assigned to a clip at a time and the 
 * effect may or may not include text. </p>
 *
 * <b>Transition Effect </b>
 * <p>An effect used to transition between one clip and a second clip.  Depending on the transition effect added, 
 * clips may overlap during the transition. </p>
 *
 * <b>Theme</b>
 * <p>Automatic application of simple effects to clips based around a theme.
 * The clips of a project will be automatically edited to include the effects and transitions bundled in the theme.
 * A theme is a package of transition effects, clip effects with text, and clip effects without text (that are all used
 * to craft a story from the video clips to be edited).  
 * A basic theme is automatically applied when a project is created with the NexEditor&t SDK which later can be changed to other themes at any time.
 * Specific theme effects are dependent on the theme so if it is changed for a project, the included theme effects will
 * also change to the effects bundled with the newly selected theme.</p>
 *
 * <b>Template</b>
 * <p>Template is a predefined scenario, composed of trims, filters, transitions, stickers, and background music, to create amazing, professional-looking, and movie-like video, simply by choosing one of the templates such as Old Film, Sports, Love, Family, etc.</p>
 *
 * <b>Non-Theme Clip Effects</b>
 * <p>Clip effects that are not bundled in the themes. These independent clip effects can be used to replace the clip 
 * effects that are automatically set on a clip based around a theme. Therefore even the theme is changed, the independent
 * effects will remain on the specific clip.</p>
 *
 * <b>Non-Theme Transition Effects</b>
 * <p>Transition Effects that are not bundled in the themes. These independent transition effects can be used to replace 
 * the transition effects that are automatically set between the clips based around a theme. Even if the theme is changed,
 * the independent transition effect will remain between the clips.  </p>
 *
 * <b>Engine</b>
 * <p>The main module, developed based on singleton pattern design, that is needed to preview and export a project. </p>
 *
 * <b>Engine View</b>
 * <p>iOS layer needed for previewing the engine. In the the NexEditor&trade; SDK, this has to be generated as a view. </p>
 *
 * <b>Effect Library</b>
 * <p>Manages clip effects, transition effects, and theme effects. This module must be initialized when the program begins.
 * The effect library operates asynchronously so it must be used after loading has completed.  Developed based on singleton 
 * pattern design. </p>
 *
 * <b>Effect ID</b>
 * <p>String IDs to identify effects and themes.  NexEditor&trade;&nbsp; SDK IDs are stored in the Effect Library. </p>
 *
 * <b>Speed Control</b>
 * <p>A function to control the playback speed of a video clip. </p>
 *
 * <b>Trim</b>
 * <p>A function to select and cut a specific section of a video clip (instead of using the whole original clip). </p>
 *
 * <b>Image Duration</b>
 * <p>The duration of time to play an image clip in a project.</p>
 *
 * <b>Units</b><p></p>
 * - Time:  Unless otherwise indicated, all NexEditor&trade;&nbsp; SDK API time units are measured in \c milliseconds. 
 * - Playback speed:  The playback speed of a clip is controlled as a percentage.  Normal speed will be 100% and doubled 
 * speed will be 200%.  Input values lower than 99 slow down the playback speed compared to the speed of the original video.
 *
 *
 * \section setupguide NexEditor SDK Setup Guide
 * \subsection howto How to import the NexEditor&trade; SDK Framework
 * This provides the SDK framwork and samples about XCode. Find the NexEditorFramework files in the SDK folder.
 * For XCode
 * -# Create a new project.
 *    \image latex Figure1_4.png "Create Single View Application" width =15cm
 *    \image html Figure1_4.png
 *    -# Copy the NexEditorFramework.framework  files from the package to the newly created project folder. \c Warning: Deployment Target 8.0 and up supported.
 *      -# For instance, in a released SDK, you can see two kinds of file--NexEditorFramework.framework and NexEditorFramework.bundle and additionally template resources.
 *    \image latex Figure1_5a.png "_" width =15cm
 *    \image html Figure1_5a.png
 *      -# In the previously created project, copy and paste NexEditorFramework.framework and NexEditorFramework.bundle.
 *    \image latex Figure1_5b.png "_" width =15cm
 *    \image html Figure1_5b.png
 *      -#  Add template resources to the working project. The user can set template resource folders, which must be under the resource>asset directory in version 1.0.8. The following example is about copying 2 folders in  Resource/asset/ .
 *    \image latex Figure1_5c.png "_" width =15cm
 *    \image html Figure1_5c.png
 * The \c km folder has all the required effects for template operation.
 *    \image latex Figure1_5d.png "_" width =15cm
 *    \image html Figure1_5d.png
 *  The template folder has built-in mp3 files and text files required for template operation.
 *    \image latex Figure1_5e.png "Copy Framework to New Project" width =15cm
 *    \image html Figure1_5e.png
 *
 * -# Add NexEditor SDK Framework:
 *    -# Select File > Add files to "your project" from the menu.
 *    -# From the ADD Files window, select NexEditorFramework.framework, then click Add.
 *    \image latex Figure1_6.png "Add files" width =15cm
 *    \image html Figure1_6.png
 *
 * -# Set dependency to use NexEditor SDK Framework on the project.
 *    
 *    Select "ApiDemos" that you created in "TARGETS".
 *    -# Click Build Phases from the menu.
 *    -# Press the + button and choose New Copy Files Phase.
 *    \image latex Figure1_7.png "New Copy Files Phase" width =15cm
 *    \image html Figure1_7.png
 *    -# After finishing the above task, there will be a section called "Copy Files" which was changed to "Embedded Framework" Click the "Destination" menu and select "Frameworks"
 *    \image latex Figure1_8.png "Change Name of Files Phase" width =15cm
 *    \image html Figure1_8.png
 *    -# Press the + button to add the NexEditorFramework.framework file.
 *    \image latex Figure1_9.png "Add NexEditorFramework.framework to Embedded Framework"  width =15cm
 *    \image html Figure1_9.png
 *    -# Finally, check if the link has been added to the <tt>Link Binary With Libraries</tt> section. 
 *    \image latex Figure1_10.png "NexEditorFramework.framework in Link Binary" width =15cm
 *    \image html Figure1_10.png
 *
 * -# Load resource bundle
 *    -# Select File > Add files to "your project" from the menu. 
 *    -# From the Add Files window, select NexEditorFramework.bundle, then click Add.
 *    \image latex Figure1_11.png "Add NexEditorFramework.bundle in Project" width = 15cm
 *    \image html Figure1_11.png
 *    -# Select the created project in "TARGETS". Click "Build Phases" from the menu. 
 *    -# Check if there is NexEditorFramework.bundle in the "Copy Bundle Resources" section
 *    -# Check if the build is running properly.
 *    \image latex Figure1_12.png "Check NexEditorFramework.bundle" width = 15cm
 *    \image html Figure1_12.png

 * \section implementation NexEditor&trade; SDK Structure and Implementation
 *
 * \subsection struct Structure
 * <b>NexEditor SDK Structure Diagram</b>
 * The image below is a diagram of the structure of the NexEditor SDK.  The NexEditor SDK is primarily divided into EffectLibrary which manages the effects and nexEngine which manages the clips and actual functions.  
 *
 *    \image latex FlowChart1.png "NexEditor SDK Structure Diagram" width =15cm
 *    \image html FlowChart1.png
 *
 * <b>NexEditor SDK FlowChart</b>
 *
 * The image below is a flowchart of the NexEditor SDK.  NEClip generates clips which will be put into NEPorject to be edited. Edited NEProject projects will be passed into NEEngine to be previewed or exported. Care must be taken to add NEClip clips into a NEProject to edit them. 
 *
 *    \image latex FlowChart2.png "NexEditor SDK Flow Chart 1" width =15cm
 *    \image html FlowChart2.png
 * In the NexEditor SDK, all the effects are managed by NXEEffectLibrary. The user can get all the effect IDs via the APIs in NEEffectLibrary.
 *
 *    \image latex FlowChart3.png "NexEditor SDK Flow Chart 2" width =15cm
 *    \image html FlowChart3.png
 *
 * \section howtouse How to use NexEditor&trade; SDK
 * \subsection init NexEditor&trade; SDK initialization
 *
 * NexEditor&trade; SDK is implemented with a singleton pattern. Before initializing its engine, effectLibrary must be loaded. This action works asynchronously through the API <tt>NXEEffectLibrary sharedInstace:completeLoading.</tt>
 *
 * After loading, completeLoading callback will be called. 
 *
 * When EffectLibrary is loaded successfully, you can initialize the engine and do other actions.
 * -#  This is an example code about the callback that receives results after initializing \c NXEEffectLibrary. 
 *
 * <b>Example Codes</b>
 * 
 * AppDelegate.m :
 * 
 * \code
#import "NexEditorFramework/NexEditorFramework.h"

@implementation AppDelegate
{
    NXEffectLibrary *nxEffectLibrary;
}

- (void)handleInitProcess
{
    if(nxeEffectLibrary == nil) {
        nxeEffectLibrary = [NXEEffectLibrary sharedInstance:false
                                          completeLoading:^(int numError) {
                                              // ToDo : Write Your own Code
                                              //
                                          }];
    }
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [self handleInitProcess];
    return YES;
}
  \endcode
 *
 * The below listener will get the results when after the user creates a project, adds a clip, and executes any action such as play, encode, etc.
 * 
 *MainViewController.m:
 * \code
@property (nonatomic, retain) NXEEngineUIEventListener* eventListener;
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip;

- (void)viewDidLoad
{
    [super viewDidLoad];
    
// Create the engine and add a layer for preview.
// self.engineView is a UIView, which has an IBOutlet setting.
//
    engine = [NXEEngine sharedInstance];
    [engine setPreviewWithCALayer:self.engineView.layer
                      previewSize:CGSizeMake(self.engineView.bounds.size.width, self.engineView.bounds.size.height)];

// Create a project.
    project = [[NEProject alloc] init];

// Register the project in the engine.
    [engine setProject:project];

// Create a clip.
//
    int error;
    NSString *tmpClipPath = ...;
    NXEClip *clip = [NXEClip newSupportedClip:tmpClipPath error:&error];
    if(clip != nil) {
        [project addClip:clip];
    } else {
        NSLog(@"This clip is not supported.");
    }

    [engine setUiEventListener:[self initUICallbackBlock]];
}

- (NXEEngineUIEventListener *)initUICallbackBlock
{
    self.eventListener = [[NXEEngineUIEventListener alloc] init];
    
    [self.eventListener registerCallback4General_onStateChange:^(int oldState, int newState) {
        // A listener that gets the results when the engine state values have been changed.
        // ToDo : Write Your own Code
        //
           }];
        
    [self.eventListener registerCallback4Play_onPlayProgress
:^(int currentTime) {
        // A listener that gets the TimeStamp information when content is playing.
        // ToDo : Write Your own Code
        //
            } onPlayStart:^ {
        // When content begins playing, this listener is called.
        // ToDo : Write Your Wanted Code
        //
    } onPlayEnd:^ {
        // A listener that is called when content stops playing.
        // ToDo : Write Your own Code
        //
    }];
    
    [self.eventListener registerCallback4Encoding_onEncodingProgress
:^(int percent) {
        // A listener that gets the progress(out of 100%) while encoding.
        // ToDo : Write Your own Code
        //
    } onEncodingDone:^ {
        // A listener that is called when encoding is complete.
        // ToDo : Write Your own Code
        //
    }];
    
    return self.eventListener;
}

 \endcode
 * 
 *
 *
 * \subsection changingtheme Applying a Theme
 *
 * <p>This is an example on how to apply a theme to a project created in the step above.</p>
 * -# Set new theme IDs to the project.
 * -# Play the project from the engine. 
 *
 * <p>Example code:</p> 
 * \code

@property (nonatomic, retain) NXEEngineUIEventListener* eventListener;
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip;
//
@property (nonatomic, retain) NXETheme *theme;
- (void)viewDidLoad
{
    ...
// Create a project.
    project = [[NXEProject alloc] init];

// Register the project in the engine.
    [engine setProject:project];

// Create a clip and register to the project.
//
    int error;
    NSString *tmpClipPath = ...;
    NXEClip *clip = [NXEClip newSupportedClip:tmpClipPath error:&error];
    if(clip != nil) {
        [project addClip:clip];
    } else {
        NSLog(@"This clip is not supported.");
}

    // Get the predefined theme from NXEffectLibrary, and apply the first theme to the project.
    //
    NXEEffectLibrary *effectLibrary = [NXEEffectLibrary sharedInstance];
    NSArray *themeArray = [effectLibrary getThemeNames];
    theme = [[NETheme alloc] initWithTheme:themeArray[0]];
    [theme applyThemeToProject:project applyPredefinedThemeBGM:YES];
    
    [engine setUiEventListener:[self initUICallbackBlock]];
    }
    
-(IBAction)didPlayButton:(id)sender
{
    // Play content when the the play button is selected.
    [engine play];
}

   \endcode 
 *
 * <p><b>Pre-installed Themes List </b></p>
 * 
 * 
 *
 * | Theme name    |
 * | ------------- |
 * | Basic         |
 * | Travel        |
 * | On-Stage      |
 * | News          |
 * | 70s           |
 * | Serene        |
 *
 * \subsection applyingtemplete Applying a Template 
 * To apply a predefined Template, go through the instructions below.
 * -# Pick a template text file from the SDK template files, and deliver in the form of \c NSData*.
 * -# Apply the template to the project.
 * 
 * \code
@property (nonatomic, retain) NXEEngineUIEventListener* eventListener;
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip1;
@property (nonatomic, retain) NXEClip *clip2;
@property (nonatomic, retina) NXETemplateParser *templateParser;

- (void)viewDidLoad
{
    [super viewDidLoad];

// Create a project.
    project = [[NEProject alloc] init];

// Register the project in the engine.
    [engine setProject:project];

// Create 2 clips and register in the engine.
//
    int error;
    NSString *path1 = ...;
    NSString *path2 = ...;
    clip1 = [NXEClip newSupportedClip:path1 error:&error];
    clip2 = [NXEClip newSupportedClip:path2 error:&error];

    [project addClip:clip1];
    [project addClip:clip2];

// Create TemplateParser.
    templateParser = [[NXETemplateParser alloc] init];

//  Pick a template text file from the SDK template files, and deliver in the form of \c NSData*. Apply the template to the project.
//
    NSString *filePath = The path of the template text;
    NSData *data = [[NSFileManager defaultManager] contentsAtPath:filePath];
    TemplateInfo *templateInfo = [templateParser getTemplateInfo:data];
    [templateParser applyTemplateToProject:project templateData:data];

    [engine setUiEventListener:[self initUICallbackBlock]];
}

-(IBAction)didPlayButton:(id)sender
{
    // When the play button is selected, play content.
    [engine play];
} \endcode
 *
 * 
 *  <p><b>Pre-installed Transition Effect List</b></p>
 *
 * | Theme name    |
 * | ------------- |
 * | Family        |
 * | Grid          |
 * | Love          |
 * | OldFilm       |
 * | Simple        |
 * | Sports        |
 * | Symmetry      |
 *
 * \subsection changingtransitioneffect Applying a Transition Effect 
 * 
 * <p>To apply a transition effect:</p>
 * -# Get a transition effect from the clip of a project, then input a new transition effect ID.
 * -# Play the project from the engine. 
 * \c Warning: Transition effect cannot be applied to the last clip in a project.
 * <p>Example code:</p> 
 *
 * \code
@property (nonatomic, retain) NXEEngineUIEventListener* eventListener;
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip1;
@property (nonatomic, retain) NXEClip *clip2;

- (void)viewDidLoad
{
    [super viewDidLoad];

// Create a project.
    project = [[NEProject alloc] init];

// Register the project in the engine.
    [engine setProject:project];

// Create 2 clips and register in the engine.
//
    int error;
    NSString *path1 = ...;
    NSString *path2 = ...;
    clip1 = [NXEClip newSupportedClip:path1 error:&error];
    clip2 = [NXEClip newSupportedClip:path2 error:&error];

    [project addClip:clip1];
    [project addClip:clip2];

// Through NXEffectLibrary, get the transition effect that has been registered.
//
    NXEEffectLibrary *effectLibrary = [NXEEffectLibrary sharedInstance];
    NSArray *transtionArray = [effectLibrary getTransitionEffectIdWithFilterFlag:FILTER_THEMEONLY | FILTER_NONTHEME];

// Get the first clip from the project and apply the first transition effect to it.
// FYI, You can change the clip property before adding it to the project.
//
    NXEClip *getClip = [project.visualItem objectAtIndex:0];
    [getClip setTransitionEffect:transitionEffect[0]];

    [engine setUiEventListener:[self initUICallbackBlock]];
}

-(IBAction)didPlayButton:(id)sender
{
    // When the play button is selected, play content.
    [engine play];
}
   \endcode
 *
 * <p><b>Pre-installed Transition Effect List</b></p>
 *
 * Transition Effect Name | Transition Effect ID
 * -----------------------| ---------------------------------------------------------------
 * Split                  | com.nexstreaming.kinemaster.builtin4.split
 * Circle Wipe            | com.nexstreaming.kinemaster.builtin2.transition.circlewipe
 * Knock Aside            | com.nexstreaming.kinemaster.builtin3.knockaside
 * Color Tiles            | com.nexstreaming.kinemaster.builtin.transition.pip.colortiles
 * Spin                   | com.nexstreaming.kinemaster.builtin4.spin 
 * Cover                  | com.nexstreaming.kinemaster.builtin4.cover
 * Wipe                   | com.nexstreaming.kinemaster.builtin4.wipe
 * Slide                  | com.nexstreaming.kinemaster.builtin4.slide
 * Square Title           | com.nexstreaming.kinemaster.builtin3.boxtexttrans
 * Checker Flip           | com.nexstreaming.kinemaster.builtin.transition.checker
 * Strip Wipe             | com.nexstreaming.kinemaster.builtin4.stripwipe
 * Captioned Inset        | com.nexstreaming.kinemaster.builtin3.captioninsettrans
 * Zoom out, then in      | com.nexstreaming.kinemaster.builtin.transition.zoominout    
 * Block in               | com.nexstreaming.kinemaster.builtin4.blockin
 * Strips                 | com.nexstreaming.kinemaster.builtin.transition.strips
 * Many Circles           | com.nexstreaming.kinemaster.builtin2.transition.manycircles
 * Zoom out               | com.nexstreaming.kinemaster.builtin.transition.zoomout
 * Star Wipe              | com.nexstreaming.kinemaster.builtin4.starwipe
 * Heart Wipe             | com.nexstreaming.kinemaster.builtin4.heartwipe
 * Split-screen           | com.nexstreaming.kinemaster.builtin.transition.pip.splitscn
 * Clock Wipe             | com.nexstreaming.kinemaster.builtin4.clockwipe
 * 3D Zoom Flip           | com.nexstreaming.kinemaster.builtin.transition.zoomflip
 * Fade Through Color     | com.nexstreaming.kinemaster.builtin.transition.fadethroughcolor
 * Video Tiles            | com.nexstreaming.kinemaster.builtin.transition.pip.tiles
 * Corners                | com.nexstreaming.kinemaster.builtin.transition.pip.corners
 * Uncover                | com.nexstreaming.kinemaster.builtin4.uncover
 * Retro Pastel Title     | com.nexstreaming.kinemaster.builtin3.retropastel
 * Bouncy Box             | com.nexstreaming.kinemaster.builtin3.bouncebox
 * Inset Video            | com.nexstreaming.kinemaster.builtin.transition.pip.inset
 * Double Split           | com.nexstreaming.kinemaster.builtin4.doublesplit
 * Crossfade              | com.nexstreaming.kinemaster.builtin.transition.crossfade
 * 3D Flip                | com.nexstreaming.kinemaster.builtin.transition.flip
 *
 * \subsection clipeffect Applying a Clip Effect
 * To apply a clip effect:
 * -# Extract a clip from a project that you want to change the clip effect, then input a new clip effect ID. 
 * -# Play the project from the engine.
 *
 * \code
@property (nonatomic, retain) NXEEngineUIEventListener* eventListener;
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip1;
@property (nonatomic, retain) NXEClip *clip2;

- (void)viewDidLoad
{
    [super viewDidLoad];

// Create a project.
    project = [[NXEProject alloc] init];

// Register the project in the engine.
    [engine setProject:project];

// Create 2 clips and register in the engine.
//
    int error;
    NSString *path1 = ...;
    NSString *path2 = ...;
    clip1 = [NXEClip newSupportedClip:path1 error:&error];
    clip2 = [NXEClip newSupportedClip:path2 error:&error];

    [project addClip:clip1];
    [project addClip:clip2];

// Through NXEffectLibrary, get the transition effect that has been registered.
//
    NXEEffectLibrary *effectLibrary = [NXEEffectLibrary sharedInstance];
    NSArray *effectArray = [effectLibrary getClipEffectIdWithFilterFlag:FILTER_THEMEONLY | FILTER_NONTHEME];

// Get the first clip from the project and apply the first transition effect to it.
// You can change the clip property before creating a project.
//
    NXEClip *getClip = [project.visualItem objectAtIndex:0];
    [getClip setClipEffect:effectArray[0]];

    [engine setUiEventListener:[self initUICallbackBlock]];
}

-(IBAction)didPlayButton:(id)sender
{
    // You can change the clip property before creating a project.
    [engine play];
}
\endcode

 *
 * \subsection effectopt Adding EffectOption
 * <p>To change the text and color information in some effects with options, follow the instructions below.</p>
 * -# Choose a clip from a project, and check if there is any option information about the effect applied to the clip.
 * -# If there is an effect option, change or update the text and color information.
 * -# Play the project from the engine.
 * 
 * Example code:
 * \code
@property (nonatomic, retain) NXEEngineUIEventListener* eventListener;
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip1;
@property (nonatomic, retain) NXEClip *clip2;
@property (nonatomic, retain) NXEEffectOptions *effectOption;

- (void)viewDidLoad
{
    [super viewDidLoad];

// Create a project.
    project = [[NXEProject alloc] init];

// Register the project in the engine.
    [engine setProject:project];

// Create 2 clips and register in the engine.
//
    int error;
    NSString *path1 = ...;
    NSString *path2 = ...;
    clip1 = [NEClip newSupportedClip:path1 error:&error];
    clip2 = [NXEClip newSupportedClip:path2 error:&error];

    [project addClip:clip1];
    [project addClip:clip2];

// After creating NXEEffectOption, check if there is any option about the effect applied to the clip.
    effectOption = [[NEEffectOptions alloc] init];

// i. Clip Effect
    NSString *effectId = clip1.vinfo.titleEffectID;
    if([effectOption setupWithEffectId:effectid] == YES) {
        for(TextOption *textOption in effectOption.textOptionsInClipEffect) {
            NSLog(@"This EffectId(%@) TextOption[key, value]=(%@, %@)", effectId, textOption.key4textField, textOption.value4textField);
        }
        for(ColorOption *colorOption in effectOption.colorOptionsInClipEffect) {
            NSLog(@"This EffectId(%@) ColorOption[key, value]=(%@, %@)", effectId, colorOption.key4colorField, colorOption.value4colorField);
        }
    } else {
        NSLog(@"This Effect doens't have EffectOption.");
    }

// ii. Transition Effect
    NSString *transitionId = clip1.vinfo.clipEffectiD;
    if([effectOption setupWithTransitionId:transitionId] == YES) {
        for(TextOption *textOption in effectOption.textOptionsInTransitionEffect) {
            NSLog(@"This TransitioinId(%@) TextOption[key, value]=(%@, %@)", transitionId, textOption.key4textField, textOption.value4textField);
        }
        for(ColorOption *colorOption in effectOption.colorOptionsInTransitionEffect) {
            NSLog(@"This TransitioinId(%@) ColorOption[key, value]=(%@, %@)", transitionId, colorOption.key4colorField, colorOption.value4colorField);
    } else {
        NSLog(@"This Transition Effect doens't have EffectOption.");
    }

    [engine setUiEventListener:[self initUICallbackBlock]];
}

-(IBAction)didPlayButton:(id)sender
{
    // When the play button is selected, play content.
    [engine play];
}

 \endcode
 * <p><b>Pre-installed Text Effect List</b></p>
 * Text Effect Name    | Text Effect ID
 * --------------------| ---------------------------------------------------------------
 * Subtitle            | com.nexstreaming.kinemaster.builtin.title.simple
 * Thriller            | com.nexstreaming.kinemaster.builtin.title.thriller
 * Bam!                | com.nexstreaming.kinemaster.builtin.title.bam
 * Ghost Story         | com.nexstreaming.kinemaster.builtin.title.ghoststory
 * Modern              | com.nexstreaming.kinemaster.builtin.title.modern
 * Manuscript          | com.nexstreaming.kinemaster.builtin.title.manuscript
 * Poem                | com.nexstreaming.kinemaster.builtin6.title.poem
 * Flip Title          | com.nexstreaming.kinemaster.builtin.title.centerflip
 * Stylish Subtitle    | com.nexstreaming.kinemaster.builtin.title.stylishsub
 * Comic Book          | com.nexstreaming.kinemaster.builtin.title.comic
 * Floating Braces     | com.nexstreaming.kinemaster.builtin6.title.brackettitle
 * Center              | com.nexstreaming.kinemaster.builtin.title.center
 * Elegant             | com.nexstreaming.kinemaster.builtin.title.elegant
 * Center Low          | com.nexstreaming.kinemaster.builtin.title.centerlow
 * Bulletin            | com.nexstreaming.kinemaster.builtin.title.bulletin
 * Slide Across        | com.nexstreaming.kinemaster.builtin5.slideacrosstxt
 * Stickers            | com.nexstreaming.kinemaster.builtin3.overlay.sticker
 * Stylish Finale      | com.nexstreaming.kinemaster.builtin.title.stylishfinale  
 * Handwriting         | com.nexstreaming.kinemaster.builtin.overlay.handwriting
 * Credits             | com.nexstreaming.kinemaster.builtin.title.creditsmid
 * Stylish Main Title  | com.nexstreaming.kinemaster.builtin.title.stylishmain
 *
 *
 * \subsection speedcontrol Speed Control
 *
 * <p>To adjust the speed of a clip: </p>
 * -# Get getVideoClipEdit of a clip used in the project, then input the new speed value to use. The value range of setSpeedControl is 25 ~ 200. \c Warning: This feature does not apply to audio clips.
 *
 * <p>Example code:</p> 
 * \code
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip1;
@property (nonatomic, retain) NXEClip *clip2;

- (void)viewDidLoad
{
    [super viewDidLoad];

// Create a project.
    project = [[NEProject alloc] init];

// Register the project in the engine.
    [engine setProject:project];

// Create 2 clips and register in the engine.
//
    int error;
    NSString *path1 = ...;
    NSString *path2 = ...;
    clip1 = [NXEClip newSupportedClip:path1 error:&error];
    clip2 = [NXEClip newSupportedClip:path2 error:&error];

    [project addClip:clip1];
    [project addClip:clip2];

// Get the first clip from the project and in the change 100 in the first value.
//
    NXEClip *getClip = [project.visualItem objectAtIndex:0];
    [getClip setSpeed:100];

    [engine setUiEventListener:[self initUICallbackBlock]];
}

-(IBAction)didPlayButton:(id)sender
{
    // When the play button is selected, play content.
    [engine play];
}
  \endcode
 * 
 * \subsection export Exporting
 *
 *
 * <p>When a preview is successful, follow the instructions below to export the project:</p>
 * -# Before sending an encoding message, always deliver a stop message to the engine.
 * -# Deliver an encoding message to the engine.
 *
 * <p>Example code:</p> 
 * \code

@property (nonatomic, retain) NXEEngineUIEventListener* eventListener;
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEClip *clip1;
@property (nonatomic, retain) NXEClip *clip2;

- (void)viewDidLoad
{
    [super viewDidLoad];

    ...

    [engine setUiEventListener:[self initUICallbackBlock]];
}

-(IBAction)didPlayButton:(id)sender
{
    // When the play button is selected, play content.
    [engine play];
}

-(IBAction)didEncodingButton:(id)sender
{
    // Before the encoding message, always deliver a stop message to the engine.
    [engine stop];
    
    // Set the directory for encoding results file.
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0]; // Get documents folder
    NSString *encodingPath = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"Export_%@.mp4", [Util getCurrentTime]]];
    
    // Deliver an encoding message to the engine. 
    [engine exportProject:encodingPath
                    Width:1280
                   Height:720
                  Bitrate:6*1024*1024
              MaxFileSize:LONG_MAX
           ProjectDuration:0
        ForceDurationLimit:false
                      Flag:0x1];
}
   \endcode
 *
 *
 *
 *
 *
 *
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
 
 NexStreaming Confidential Proprietary
 Copyright (C) 2005~2016 NexStreaming Corporation
 All rights are reserved by NexStreaming Corporation
 ******************************************************************************/

#import <UIKit/UIKit.h>
#import "NXEEditorType.h"
#import "NXEEditorErrorCode.h"

@class NXEProject;
@class NXEThumbnailUICallbackBlock;

/**
 * \brief This class previews <b>NXEProject</b> with <b>NXEClip</b> and <b>NXEEffectLibrary</b> applied to it, or encodes into MP4 files.
 *        This acts as a media player and uses many resources, therefore working with a singleton pattern for the application is suggested.
 * \see <b>NXEProject</b>, <b>NXEClip</b>
 * \since version 1.0.5
 */
@interface NXEEngine : NSObject

/**
 * \brief This method gets a <b>NXEProject</b> instance, which is the result of setProject.
 * \since version 1.0.5
 */
@property (nonatomic, readonly) NXEProject* project;

/**
 * \brief This method gets an engine instance, which controls actions such as play, pause, encoding, etc in the preview. 
 * \since version 1.0.5
 */
+ (NXEEngine*)sharedInstance;

/**
 */
+ (void)setAspectMode:(NXEAspectType)aspect;

/**
 * \brief sets the screen aspect ratio. You must set one value from <b>NXEAspectType</b> Enum types.
 * \since version 1.0.5
 */
+ (NXEAspectType)getAspectMode;

/**
 * \brief NXEEngie instance를 release해주는 함수
 */
- (void)cleanup:(BOOL)includeLayer;

/**
 * \brief This method will play content from the beginning if the project is normally set.
 * \param playProgressCallbackBlock 재생 중 재생 될 때의 Time Stamp 정보를 Callback Block으로 전달한다.
 * \param playStartCallbackBlock 재생이 시작되었다는 정보를 Callback Block으로 전달한다.
 * \param playEndCallbackBlock 재생이 종료되었다는 정보를 Callback Block으로 전달한다.
 * \since version 1.0.5
 */
- (void)setCallbackWithPlayProgress:(void(^)(int currentTime))playProgressBlock
                         playStart:(void(^)())playStartBlock
                           playEnd:(void(^)())playEndBlock;
/**
 * \brief This method will play content from the beginning if the project is normally set.
 * \since version 1.0.5
 */
- (int)play;

/**
 * \brief This method will stop content if the project is normally set.
 *  주의사항. 해당 루틴 이후에 engine 내부 리소스들이 바로 종료되지 않습니다. 
 *          stopWithComplete 사용해 engine 관련 된 리소스들이 종료 된 후 해당 동작에 대한 결과값을 callback block으로 받는 것이 좋습니다.
 * \since version 1.0.5
 */
- (void)stopSync:(void(^)(ERRORCODE errorcode))stopCompleteBlock;

/**
 * \brief This method will stop content if the project is normally set.
 * \param stopCompleteBlock engine 관련 된 리소스들이 종료 된 후 해당 동작에 대한 결과값을 해당 callback block으로 받게 된다.
 *      만약 에러가 발생이 될 경우, 전달 된 값은 ERRORCODE 중, 하나의 값이고, 정상적으로 종료할 경우, ERROR_NONE 이다.
 * \since version 1.0.5
 */
- (void)stopASync:(void(^)(ERRORCODE errorcode))stopCompleteBlock;

/**
 * \brief This method will pause content if the project is normally set.
 * \since version 1.0.5
 */
- (int)pause;

/**
 * \brief This method will resume content if the project is normally set.
 * \since version 1.0.5
 */
- (int)resume;

/**
 * \brief
 * \param playProgressCallbackBlock 재생 중 재생 될 때의 Time Stamp 정보를 Callback Block으로 전달한다.
 */
- (void)setCallbackWithSetTime:(void(^)(int seekDoneTS))seekDoneBlock;

/**
 * \brief This method seeks for content if the project is normally set.
 *        For this method to work properly, methods like <tt>stop()</tt> or <tt>pause()</tt> should be called before.
 *        Otherwise, <tt>seek()</tt> method will fail.
 * \param time The seek time in <tt>msec</tt> (milliseconds)
 * \since version 1.0.5
 */
- (void)seek:(int)time;

/**
 * \brief This method seeks for the closest I-frame, before the input seek time value by the parameter <tt>time</tt>, of video content.
 *        Therefore, the actual seek result shown on the preview might be different from the input seek time.
 * \param time The seek time in <tt>msec</tt> (milliseconds)
 * \since version 1.0.5
 */
- (void)seekIDRorI:(int)time;

/**
 * \brief This method seeks for the closest IDR-frame, before the input seek time value by the parameter <tt>time</tt>, of video content.
 *        Therefore, the actual seek result shown on the preview might be different from the input seek time.
 * \param time The seek time in <tt>msec</tt> (milliseconds)
 * \since version 1.0.5
 */
- (int)seekIDROnly:(int)time;

/**
 * \brief This sets the view area which will show the preview screen.
 * \param previewSize width와 height를 설정한 CGSize
 * \param aspectType 16:9, 9:16, 1:1값을 가진다.
 * \code
 *  NXEEngine *engine = [[NXEEngine alloc] init];
 *  [engine setPreviewWithCALayer:view.layer previewSize:CGSizeMake(view.bounds.size.width, view.bounds.size.height)];
 *  ...
 * \endcode
 * \since version 1.0.5
 */
- (void)setPreviewWithCALayer:(CALayer*)layer previewSize:(CGSize)previewSize;


/**
 * \param encodingProgressBlock encoing 진행값, 0~100,을 Callback Block으로 전달한다.
 * \param encodingEndBlock encoing 종료되었음을 Callback Block으로 전달한다.
 */
- (void)setCallbackWithEncodingProgress:(void(^)(int percent))encodingProgressBlock
                            encodingEnd:(void(^)())encodingEndBlock;

/**
 * \brief This method encodes a project.
 * \param path This is a string indicating the path to where the exported file is saved.
 * \param width The width of the content to be encoded.
 * \param height The height of the content to be encoded.
 * \param bitrate The bitrate of the content to be encoded.
 * \param maxFileSize The maximum size of the content to be encoded. Input the size of available space from the current storage.
 * \param projectDuration forDurationLimit 값이 True인 경우, 해당 시간만큼만 Export 된다. forDurationLimit 값이 False 주게 될 경우, 0 입력하면 된다.
 * \param forDurationLimit 특정 시간만큼 저장하려면 True, 그 이외의 경우 False 입력한다.
 * \param flag Rotation, 0x0: 0 / 0x10: 90 / 0x20: 180 / 0x40: 270
 * \return 
 * \since version 1.0.5
 */
- (int)exportProject:(NSString*)path
               Width:(int)width
              Height:(int)height
             Bitrate:(int)bitrate
         MaxFileSize:(long)maxFileSize
     ProjectDuration:(int)projectDuration
  ForceDurationLimit:(bool)forDurationLimit
                Flag:(int)flag;

/**
 * \brief This method arranges the clip data in a project and delivers them to the engine.
 * \param flag Set to preview or export for when playing or exporting.
 */
- (void)resolveProject:(Boolean)flag;

/**
 * \brief This method captures the current frame when the preview is running.
 * \return -1 if there is an error; otherwise, 0. 
 * \since version 1.0.5
 */
- (int)captureCurrentFrame;

/**
 * \brief 화면 clearing
 */
- (int)clearScreen;

/**
 */
- (int)getClipVideoThumbs:(NSString*)clipPath
            ThumbnailPath:(NSString*)thumbnailPath
                    Width:(int)width Height:(int)height
                StartTime:(int)startTime
                  EndTime:(int)endTime
                    Count:(int)count
                     Flag:(int)flag
                 UserFlag:(int)userFlag
 videoThumbnailUICallback:(NXEThumbnailUICallbackBlock *)callback;

/**
 * UIListenerProtocol defined naming 같아서 on- 제거합니다.
 */
- (void)setTimeDone:(int) currentTime
            frameTime:(int) frameTime
            errorcode:(ERRORCODE)errorcode;

/**
 * UIListenerProtocol defined naming 같아서 on- 제거합니다.
 */
- (void)setTimeIgnored;

/**
 * \brief This method sets a project to be played or encoded.
 * \since version 1.0.5
 */
- (void)setProject:(NXEProject*)project;

/**
* \brief This method sets the screen size where layer rendering action will take place.
* \param screenSize Must be in \c CGSize unit.
 */
- (void)setLayerSize:(CGSize)layerSize;

/**
 * \brief 생성한 모든 Layer Resources들을 Release해준다.
 */
- (void)clearLayers;

@end