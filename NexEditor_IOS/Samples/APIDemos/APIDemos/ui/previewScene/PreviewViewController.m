/*
 * File Name   : PreviewViewController.m
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

#import "PreviewViewController.h"
#import "LayerOptionTableViewController.h"
#import "AudioTrackTableViewController.h"
#import "ProjectClipListEditorViewController.h"
#import "ProjectSource.h"
#import "NXEProject+Convenience.h"
#import "APIDemos-Swift.h"
#import "NXEClip+Extensions.h"
#import "NSArray+NXELayer.h"
#import "LayerEditorCustomizer.h"
#import "ExportController.h"

#define NEXEDITOR_MAX_AUDIOTRACK_COUNT  4
#define NEXEDITOR_MAX_TEMPLATE_BGMTRACK_COUNT   1

@import NexEditorFramework;

@import MediaPlayer;

#define DEFAULT_SPEEDCONTROL 100

typedef enum STATE
{
    NO_STATE = -1,
    INIT = 0,
    PLAY,
    RECORDING,
    PAUSE,
    STOP,
    SEEKING,
    TRIMMING,
    ERROR,
}PLAY_STATE;

// Values are assigned as the subview index of each button within menuView.
// See PreviewViewController in storyboard for the index of each button in the Menu View
typedef enum {
    MenuButtonExport = 0,
    MenuButtonEditProject,              // segueProjectList
    MenuButtonLayers,                   // segueLayerOption
    MenuButtonAudioTracks,              // segueAudioList
    MenuButtonTextEffect,               // segueTextEffect
    MenuButtonTemplate,
    MenuButtonAspectRatio,
    MenuButtonColorAdjustment,
} MenuButton;

@interface PreviewViewController (Export) <ExportControllerDelegate>
@end

@interface PreviewViewController () <TextEffectPickerDelegate, ColorAdjustmentDelegate>
@property (strong, nonatomic) IBOutlet UIView *menuView;

@property (nonatomic, assign) int seekCurrentTS;

@property (nonatomic, retain) NXEEngine *engine;

@property (nonatomic, assign) enum STATE state;
@property (nonatomic, assign) BOOL repeatPlayback;

@property (nonatomic, strong) NSArray <NXEClipSource *> *clipSources;

/// Available only after successful viewDidLoad. Nil otherwise.
@property (nonatomic, strong) NXEProject *project;

@property (nonatomic) CGSize previewSize;
@property (nonatomic) ColorAdjustmentView* colorAdjustmentView;
@property (nonatomic, strong) TemplatePickerPresenter *templatePickerPresenter;

@property (nonatomic, strong) ProjectSource *projectSource;
@property (nonatomic, strong) void (^onSeekComplete)(void);

@property (nonatomic, strong) ExportController *exportController;
@end

@implementation PreviewViewController

- (void)dealloc
{
}

#pragma mark - Properties

#pragma mark - TextEffectPickerDelegate

- (void)itemPicker:(TextEffectPickerViewController * _Nonnull)picker
         didSelect:(NSIndexPath * _Nonnull)indexPath
            itemId:(NSString *) itemId
            params:(NXETextEffectParams *) textEffectParams
{
    [self.navigationController popViewControllerAnimated:YES];
    
    self.projectSource.textEffect = [[NXETextEffect alloc] initWithId:itemId
                                                               params:textEffectParams];
    [self rebuildAndSetProjectWith:self.projectSource];
    [self actionResume];
}

- (void) didClearTextEffectAssetItem:(UIBarButtonItem *) sender
{
    [self.navigationController popViewControllerAnimated:YES];
    if (self.projectSource.textEffect) {
        self.projectSource.textEffect = nil;
        [self rebuildAndSetProjectWith:self.projectSource];
    }
}

#pragma mark - ItemPickerObjcDelegate
- (void)itemPicker:(ItemPickerCollectionViewController * _Nonnull)picker didSelect:(NSIndexPath * _Nonnull)indexPath
{
}

- (void)itemPickerDidCancel:(ItemPickerCollectionViewController * _Nonnull)picker
{
    if ([picker isKindOfClass:TextEffectPickerViewController.class]) {
        [self actionResume];
    }
}

#pragma mark - API

- (void) setClipSources:(NSArray <NXEClipSource *> *) clipSources
{
    _clipSources = clipSources;
}

#pragma mark - audio

- (void)setAudioTrack:(NSMutableArray *)audioTrackList
{
    if (self.projectSource.clips.count == 0) {
        // setProject4XXX were not called with proper arguments before presenting this view controller.
        [self unrecoverableErrorWithMessage:@"Project not configured properly"];
        return;
    }

    self.projectSource.audioTracks = audioTrackList;
    [self rebuildAndSetProjectWith:self.projectSource];
}

#pragma mark - layer

// startTime, endTime 두개의 값을 기준으로 videoLayer List 항목들 중, 몇 개의 Layer가 중복이 되었는지를 알려주는 함수
//
- (int)getDuplicateCount:(int)index endTime:(int)endTime inLayers:(NSArray<NXEVideoLayer *> *) layers
{
    int overlayCount = 0;
    

    do {
        NXEVideoLayer *videoLayer = layers[index];
        if(videoLayer.startTime < endTime) {
            overlayCount++;
        }
        index++;
    } while (index < (int)[layers count]);
    
    return overlayCount;
}

- (NSInteger) numberOfOverlappingVideoLayersIn:(NSArray<NXEVideoLayer *> *) layers
{
    NSInteger overlaps = 0;
    
    int videolayerCount = (int)layers.count;
    if(videolayerCount > 2) {
        int loopid = 0;
        for(NXEVideoLayer *videoLayer in layers) {
            overlaps = [self getDuplicateCount:loopid endTime:videoLayer.endTime inLayers:layers];
            loopid++;
            if(overlaps > 2) {
                break;
            }
        }
    }
    return overlaps;
}


- (void) replaceLayers:(NSArray <NXELayer *> *) layers
{
    NSArray *videoLayers = [layers layersWithType:NXE_LAYER_VIDEO];
    if(videoLayers.count > 2 && [self numberOfOverlappingVideoLayersIn:videoLayers] > 2) {
        videoLayers = [videoLayers subarrayWithRange:NSMakeRange(0, 2)];
        layers = [layers mergingLayers:videoLayers type:NXE_LAYER_VIDEO];
        [self performSelector:@selector(alertDiscardingVideoLayersExceedLimits) withObject:nil afterDelay:0];
    }
    self.projectSource.layers = layers;
    
    [self rebuildAndSetProjectWith:self.projectSource seekDone:^(NXETimeMillis targetTIme) { }];
}

#pragma mark - preview changing property

/**
 * Replaces visual clips
 */
- (void)setClips:(NSArray<NXEClip *> *)clips
{
    self.projectSource.clips = clips;
    [self rebuildAndSetProjectWith:self.projectSource];
}

#pragma mark - layer editor

- (NXESimpleLayerEditorCustomization *)layerEditorCustomization
{
    __weak NXEEngine *weakEngine = self.engine;
    __weak PreviewViewController *weakSelf = self;
    return [LayerEditorCustomizer customizationWithButtonTap:^(NXESimpleLayerEditorButtonType buttonType, NXELayer *layer) {
        if ( buttonType == NXESimpleLayerEditorButtonTypeDelete ) {
            NSMutableArray *layers = [weakEngine.project.layers mutableCopy];
            [layers removeObject:layer];
            weakEngine.project.layers = layers;
            weakSelf.projectSource.layers = layers;
            
            // refresh
            [weakEngine fastOptionPreview:NXE_NORMAL optionValue:nil display:YES];
        } else if (buttonType == NXESimpleLayerEditorButtonTypeCustom ) {
            [weakSelf alert:@"Custom button tapped" title:@"Layer Editor" onOK:NULL];
        }
    }];
}

#pragma mark - Project
- (void) updateProjectDuration:(NSInteger) duration current:(int) currentTime
{
    self.trimSlider.minimumValue = 0;
    self.trimSlider.maximumValue = duration;
    self.trimSlider.lowerValue = self.trimSlider.minimumValue;
    self.trimSlider.upperValue = self.trimSlider.maximumValue;
    self.trimSlider.centerValue = currentTime;
    
    self.totalLabel.text = [StringTools formatTimeMs: (int) duration];
}

- (ProjectSource *) projectSource
{
    if (_projectSource == nil) {
        _projectSource = [[ProjectSource alloc] init];
    }
    return _projectSource;
}

- (void) setProject:(NXEProject *)project
{
    _project = project;
    int duration = [project getTotalTime];
    int currentTime = [self.engine getCurrentPosition];
    [self.engine setProject:project];
    
    [self updateProjectDuration:duration current:currentTime];
    self.title = [self updatedTitleString];
    
    BOOL hiddenForTemplate = self.projectSource.isTemplate;
    self.layerButton.hidden = hiddenForTemplate;
    self.projectEditButton.hidden = hiddenForTemplate;
}

- (void) rebuildAndSetProjectWith:(ProjectSource *) projectSource seekDone:(void (^)(NXETimeMillis targetTIme)) done
{
    NSError *error = nil;
    NXEProject *project = [projectSource buildProjectWithError:&error];
    if (project) {
        int currentTime = [self.engine getCurrentPosition];
        self.project = project;
        int duration = [project getTotalTime];
        if (currentTime > duration) {
            currentTime = duration;
        }
        [self seekTo:currentTime complete:^{
            if (done) done(currentTime);
        }];
    } else if (error) {
        // To alert or make the error visible to the user, pass the error to the caller
        // where may already alert for some other reason.
        NSLog(@"Error building a project:%@", error.localizedDescription);
    }
}

- (void) rebuildAndSetProjectWith:(ProjectSource *) projectSource
{
    [self rebuildAndSetProjectWith:projectSource seekDone:NULL];
}

/// Guess title of bgm audio clip set for Template Project if any. Nil otherwise.
- (NSString *) bgmClipTitleFromProject:(NXEProject *) project
{
    NSString *result = nil;
    NXEClip *clip = project.bgmClip;
    if (clip) {
        result = clip.clipSource.shortDisplayText;
    }
    return result;
}
#pragma mark view


- (NSString *)updatedTitleString
{
    NSString *title = @"Movie";
    if (self.projectSource.isTemplate) {
        NSString *templateName = self.projectSource.templateAssetItem.name.capitalizedString;
        title = [NSString stringWithFormat:@"%@ - %@", title, templateName];
    }
    return title;
}

- (void)setupEventHandler
{
    NXEEngine *engine = self.engine;
    __weak PreviewViewController *blockself = self;
    __weak NXEEngine *weakEngine = engine;
    
    // register callback related to play
    //
    [engine setCallbackWithPlayProgress:^(int currentTime) {
        
        [blockself playProgress:currentTime];
        
    } playStart:^{
        
        [blockself playStart];
        
    } playEnd:^{
        
        if(blockself.repeatPlayback) {
            [NSThread sleepForTimeInterval:1.5];
            
            [weakEngine play];
            
            blockself.state = PLAY;
            [blockself.playButton setImage:[UIImage imageNamed:@"pause_default.png"]
                                  forState:UIControlStateNormal];
        } else {
            [blockself playEnd];
        }
    }];
    
    // register callback related to seek(or setTime)
    //
    [engine setCallbackWithSetTime:^(int seekDoneTS) {
        if (blockself.onSeekComplete) {
            blockself.onSeekComplete();
            blockself.onSeekComplete = nil;
        }
    }];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    self.menuView.hidden = YES;
    self.menuView.layer.cornerRadius = 4;
    
    _seekCurrentTS = 0;
    _state = NO_STATE;
    
    self.engine = [NXEEngine instance];
    self.engine.aspectType = NXEAspectTypeRatio16v9;
    [self.engine setPreviewWithCALayer:self.engineView.layer
                           previewSize:self.engineView.bounds.size];
    self.previewSize = self.engineView.bounds.size;

    [_trimSlider.upperKnobLayer setHidden:YES];
    [_trimSlider.lowerKnobLayer setHidden:YES];
    _trimSlider.trimKnobTouchEnable = FALSE;
    
    self.title = [self updatedTitleString];
    self.progressLabel.text = [StringTools formatTimeMs:0];

    BOOL projectConfigured = NO;
    ///
    NSError *error = nil;
    do {
        NSArray <NXEClip *> *clips = [NXEClip clipsWithSources:self.clipSources error:&error];
        if (clips.count == 0 || clips.count != self.clipSources.count) {
            // Bad clip sources or none
            break;
        }

        self.projectSource.clips = clips;
        NXEProject *project = [self.projectSource buildProjectWithError:&error];
        if ( project == nil ) {
            break;
        }
        
        self.project = project;
        [self.engine preparedEditor:^{
            [self seekTo:0];
            self.state = INIT;
            [self setupEventHandler];
        }];
        
        projectConfigured = YES;

    } while(NO);
    
    if ( projectConfigured ) {
        [self.trimSlider setCallbackBlock:[self trimSliderCallbackBlock]];
        self.engineView.layerEditor.customization = [self layerEditorCustomization];
    } else if (error) {
        self.state = ERROR;
        [self unrecoverableErrorWithMessage:error.localizedDescription];
    }
}

- (void) alert:(NSString *) message title:(NSString *) title onOK:( void (^)(void)) completion
{
    if (completion == nil) {
        completion = ^{};
    }
    [self alert:message title:@"OK" onOK:completion];
}

- (void) unrecoverableErrorWithMessage:(NSString *) message
{
    if (self.presentedViewController != nil && [self.presentedViewController isKindOfClass:[UIAlertController class]]) {
        NSLog(@"Discarding redundant unrecoverable error: %@", message);
        return;
    }
    
    [self alert:message title:@"Error" onOK:^{
        [self.navigationController popViewControllerAnimated:YES];
    }];
}

- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];
    if(self.engine != nil) {
        if (!CGSizeEqualToSize(self.previewSize, self.engineView.bounds.size)) {
            [self realignPreview];
        }
    }
}

- (void)didMoveToParentViewController:(UIViewController *)parent
{
    if (![parent isEqual:self.parentViewController]) {
        // NESI-64 이슈 수정
        if(self.state == STOP || self.state == INIT) {
            self.state = NO_STATE;
            //
            self.engineView = nil;
            self.engine = nil;
        } else if(self.state == ERROR) {
            self.state = NO_STATE;
            //
        } else {
            // NESI-104 이슈 수정
            self.state = NO_STATE;
            
            [self.engine stop];
            self.engineView = nil;
            self.engine = nil;
            //
        }
    }
}

-(void) viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    //
    [self.engine clearScreen];
    
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    if(self.state == PAUSE) {
        // Layer 버튼 누른 시점으로 Seek 동작 수행.
        [self seekTo:[self.engine getCurrentPosition]];
    }
    
}

#pragma mark -

- (void) realignPreview
{
    [self.engine setPreviewWithCALayer:self.engineView.layer
                           previewSize:self.engineView.bounds.size];
    self.previewSize = self.engineView.bounds.size;
}
#pragma mark - seek,play,trim...

- (void) playProgress:(int) currentTime
{
    // Set Progress Label
    //
    self.progressLabel.text = [StringTools formatTimeMs:currentTime];
    
    // Set CenterBall
    //
    if(([self.engine.project getTotalTime] - currentTime) % 3 == 0) {
        self.trimSlider.centerValue = (float)currentTime;
    }
}

- (void) playStart
{
}

- (void) playEnd
{
    int totalTime = [self.engine.project getTotalTime];
    if(self.state != PAUSE) {
        self.trimSlider.centerValue = totalTime;
        self.state = STOP;
    }
    [self.playButton setImage:[UIImage imageNamed:@"play_default.png"] forState:UIControlStateNormal];
    self.progressLabel.text = [StringTools formatTimeMs:totalTime];
}

- (IBAction)didPlayButton:(id)sender
{
    if(self.state == NO_STATE) return;
    
    self.repeatPlayback = NO;
    //
    if(self.state == PLAY) {
        [self actionPause];
    } else {
        if(self.state == PAUSE
           || self.state == SEEKING) {
            [self actionResume];
        } else {
            self.trimSlider.centerValue = 0;
            //
            [self actionPlay];
        }
    }
}

- (void)actionPlay
{
    self.state = PLAY;
    self.engineView.isEditingEnabled = NO;
    [self.engine play];
    [self.playButton setImage:[UIImage imageNamed:@"pause_default.png"] forState:UIControlStateNormal];
}

- (void)actionPause
{
    self.state = PAUSE;
    self.engineView.isEditingEnabled = YES;
    [self.engine pause];
    [self.playButton setImage:[UIImage imageNamed:@"play_default.png"] forState:UIControlStateNormal];
}

- (void)actionResume
{
    self.state = PLAY;
    self.engineView.isEditingEnabled = NO;
    [self.engine resume];
    [self.playButton setImage:[UIImage imageNamed:@"pause_default.png"] forState:UIControlStateNormal];
}

- (void)showColorAdjustmentView:(BOOL) show {
    // Show or hide color adjustment
    void (^completionBlock)(BOOL finished) = nil;
    CGRect moveTo;
    
    if (show) {
        if (self.colorAdjustmentView) { return; }
#define PROEJCT_VIEW_WIDTH 270
        
        CGRect frame =
        CGRectMake(self.view.frame.origin.x + self.view.frame.size.width, self.engineView.frame.origin.y - self.engineView.layoutMargins.top,
                   PROEJCT_VIEW_WIDTH, self.view.frame.size.height);
        self.colorAdjustmentView = [[ColorAdjustmentView alloc] initWithFrame:frame];
        self.colorAdjustmentView.backgroundColor = [[UIColor blackColor] colorWithAlphaComponent:0.3];
        self.colorAdjustmentView.delegate = self;
        self.colorAdjustmentView.colorAdjustments = self.project.colorAdjustments;
        [self.view addSubview:self.colorAdjustmentView];
        
        moveTo = self.colorAdjustmentView.frame;
        moveTo.origin.x = self.colorAdjustmentView.frame.origin.x - self.colorAdjustmentView.frame.size.width;
    } else {
        if (self.colorAdjustmentView == nil) { return; }
        moveTo = self.colorAdjustmentView.frame;
        moveTo.origin.x = self.colorAdjustmentView.frame.origin.x + self.colorAdjustmentView.frame.size.width;
        
        completionBlock = ^(BOOL finished){
            if( finished ) {
                [self.colorAdjustmentView removeFromSuperview];
                self.colorAdjustmentView = nil;
            }
        };
    }
    
    [UIView animateWithDuration:0.3
                     animations:^{
                         self.colorAdjustmentView.frame = moveTo;}
                     completion:completionBlock];
}

- (void)colorAdjustmentView:(ColorAdjustmentView * _Nonnull)view didChangeValue:(NXEColorAdjustments)value
{
    NSString* optionValue = [NSString stringWithFormat:@"%f, %f, %f", value.brightness, value.contrast, value.saturation];
    [self.engine fastOptionPreview:NXE_PROJECT_COLOR_ADJUSTMENT optionValue:optionValue display:YES];
}

- (void)colorAdjustmentView:(ColorAdjustmentView * _Nonnull)view willDisappearWithValue:(NXEColorAdjustments)value
{
    self.project.colorAdjustments = value;
    [self seekTo:[self.engine getCurrentPosition]];
}

- (IBAction)doHiddenPlay:(UILongPressGestureRecognizer *)sender
{
    if(sender.state == UIGestureRecognizerStateBegan) {
        self.repeatPlayback = TRUE;
        if(self.state != PLAY) {
            [self actionPlay];
        }
    }
}

- (void) seekTo:(NXETimeMillis) target idrOnly:(BOOL) idrOnly complete:(void (^)(void)) complete
{
    self.seekCurrentTS = target;

    self.onSeekComplete = complete;
    if (idrOnly) {
        [self.engine seekIDRorI:target];
    } else {
        [self.engine seek:target];
    }
}

- (void) seekTo:(NXETimeMillis) target
{
    [self seekTo:target idrOnly:NO complete:NULL];
}

#pragma mark - Export
- (ExportController *) exportController
{
    if ( _exportController == nil ) {
        _exportController = [[ExportController alloc] initWithHostViewController:self editor:self.engine];
        _exportController.delegate = self;
    }
    return _exportController;
}

- (void)showExportActionsIfApplicable
{
    if(self.state < INIT) {
        [self alert:@"Not Complete Engine Initialization." title:@"Waring" onOK:^{
        }];
        
        return;
    }
    
    if(self.state == RECORDING) {
        return;
    }
    
    // engine state 변경이 되지 않아서 아래와 같이 조건을 2개로 나눔
    //
    if(self.state == PLAY) {
        [self.playButton setImage:[UIImage imageNamed:@"play_default.png"] forState:UIControlStateNormal];
        self.state = STOP;
        ERRORCODE errorCode = [self.engine stop];
        if(errorCode == ERROR_NONE) {
            [self.exportController presentExportActionSheet];
        }
    } else {
        [self.exportController presentExportActionSheet];
    }

}

#pragma mark - Internal

/// @param selector Argument to UIBarButtonItem's action parameter of -initWithTitle:style:target:action:
- (UIBarButtonItem *) barButtonItemClearAssetItemWithAction:(SEL) selector
{
    UIBarButtonItem *barButtonNone = [[UIBarButtonItem alloc] initWithTitle:@"None" style:UIBarButtonItemStylePlain target:self action:selector];
    return barButtonNone;
}

#pragma mark - Navigation
// In a storyboard-based application, you will often want to do a little preparation before navigation
-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    //
    [self actionPause];
    
    [self.playButton setImage:[UIImage imageNamed:@"play_default.png"] forState:UIControlStateNormal];
    
    if ([segue.identifier isEqualToString:@"segueLayerOption"]) {
        //
        LayerOptionTableViewController *vc = [segue destinationViewController];
        vc.layers = self.project.layers;
        
    } else if([segue.identifier isEqualToString:@"segueAudioList"]) {
        //
        NSArray *audioItems = self.engine.project.audioClips;
        int totalTime = [self.engine.project getTotalTime];
        NSArray <NSString *> *trackTitles = self.projectSource.audioTrackTitles;
        NSInteger maxTracks = NEXEDITOR_MAX_AUDIOTRACK_COUNT;
        
        if (self.projectSource.isTemplate) {
            maxTracks = NEXEDITOR_MAX_TEMPLATE_BGMTRACK_COUNT;

            audioItems = @[];
            NXEClip *bgmClip = self.engine.project.bgmClip;
            if ( bgmClip ) {
                audioItems = @[bgmClip];
            }
            
            if (trackTitles.count == 0) {
                NSString *title = [self bgmClipTitleFromProject:self.project];
                if (title) {
                    trackTitles = @[title];
                }
            }
        }
        
        [(AudioTrackTableViewController *)[segue destinationViewController]setupAudioList:audioItems
                                                                              projectTime:totalTime
                                                                               titleArray:trackTitles
                                                                                maxTracks:maxTracks];
    } else if([segue.identifier isEqualToString:@"segueProjectList"]) {
        //
        ProjectClipListEditorViewController *destination = (ProjectClipListEditorViewController *) segue.destinationViewController;
        destination.clips = self.projectSource.clips;
        __weak PreviewViewController *me = self;
        destination.onDone = ^(UIViewController *viewController, BOOL done) {
            [me.navigationController popViewControllerAnimated:YES];
            if (done) {
                ProjectClipListEditorViewController *projectEditor = (ProjectClipListEditorViewController *) viewController;
                me.clips = projectEditor.clips;
            }
        };
    } else if([segue.identifier isEqualToString:@"segueTextEffect"]) {
        
        TextEffectPickerViewController *destination = (TextEffectPickerViewController *) segue.destinationViewController;
        destination.objcDelegate = self;
        NXETextEffectParams *params = self.projectSource.textEffect.params;
        if (params == nil) {
            params = [[NXETextEffectParams alloc] init];
        }
        destination.textEffectParams = params;
        [destination setDefaultDataSourceWith:self.engine.aspectType];
        UIBarButtonItem *barButtonNone = [self barButtonItemClearAssetItemWithAction:@selector(didClearTextEffectAssetItem:)];
        destination.navigationItem.rightBarButtonItems = @[barButtonNone];
    }
}

#pragma mark - CERangeSliderCallback

- (CERangeSliderCallbackBlock*)trimSliderCallbackBlock
{
    __weak PreviewViewController *weakSelf = self;
    __weak NXELayerEditorView *weakEngineView = self.engineView;
    CERangeSliderCallbackBlock *callbackBlock = [[CERangeSliderCallbackBlock alloc] initWithCallbackBlock:^(bool isTouchedCenterKnob) {
        if ( weakSelf == nil ) return;
        CERangeSlider *slider = weakSelf.trimSlider;
        NXEEngine *engine = weakSelf.engine;
        
        if(isTouchedCenterKnob == true) {
            int seekTime = slider.centerValue;

            [weakSelf seekTo:seekTime idrOnly:(abs(weakSelf.seekCurrentTS - seekTime) > 250) complete:^{
                weakEngineView.isEditingEnabled = NO;
                
                weakSelf.state = PLAY;
                [engine resume];
                [weakSelf.playButton setImage:[UIImage imageNamed:@"pause_default.png"] forState:UIControlStateNormal];
            }];
        }
    } ContinueTrackingWithTouchCallbackBlock:^(bool isTouchedCenterKnob) {
        if ( weakSelf == nil ) return;
        CERangeSlider *slider = weakSelf.trimSlider;
        NXEEngine *engine = weakSelf.engine;
        
        if(isTouchedCenterKnob == true) {
            if(weakSelf.state != SEEKING) {
                [engine pause];
            }
            
            int seekTime = slider.centerValue;
            [weakSelf seekTo:seekTime idrOnly:(abs(weakSelf.seekCurrentTS - seekTime) > 250) complete:NULL];
            
            weakSelf.progressLabel.text = [StringTools formatTimeMs:seekTime];
            
            weakSelf.state = SEEKING;
        }
    }];
    
    return callbackBlock;
}

#pragma mark - Template Picker
- (void) presentTemplatePicker
{
    [self actionPause];
    
    TemplatePickerPresenter *presenter = [[TemplatePickerPresenter alloc] initWithAspectType:self.engine.aspectType];

    __weak PreviewViewController *me = self;
    presenter.didEnd = ^(ItemPickerCollectionViewController * picker, NXEAssetItem *assetItem) {
        [me.navigationController popViewControllerAnimated:YES];
        me.templatePickerPresenter = nil;
        
        if (assetItem != nil) {
            BOOL alert = me.projectSource.audioTracks.count > 0;
            me.projectSource.templateAssetItem = (NXETemplateAssetItem *)assetItem;
            me.projectSource.audioTracks = @[];
            [me rebuildAndSetProjectWith:me.projectSource];
            
            if (alert) {
                [me alert:@"Audio tracks were cleared due to Template change" title:@"Information" onOK:^{}];
            }
        }
    };
    
    UIBarButtonItem *barButtonNone = [self barButtonItemClearAssetItemWithAction:@selector(didClearTemplateAssetItem:)];
    [presenter presentFrom:self
          beforePresenting:^(ItemPickerCollectionViewController * viewController) {
              viewController.title = @"Select Template";
              viewController.navigationItem.rightBarButtonItems = @[barButtonNone];
          }
     ];
    
    self.templatePickerPresenter = presenter;
}

- (void) didClearTemplateAssetItem:(id) sender
{
    [self.navigationController popViewControllerAnimated:YES];
    if (self.projectSource.templateAssetItem) {
        self.projectSource.templateAssetItem = nil;
        [self rebuildAndSetProjectWith:self.projectSource];
    }
}

#pragma mark - Aspect Ratio

- (void) presentAspectRatioSelector
{
    NSArray *titles = [NXEAspectTypeExtension displayTexts];
    NXEAspectType currentType = self.engine.aspectType;
    __weak PreviewViewController *me = self;
    
    NSString *message = @"";
    if (self.projectSource.templateAssetItem != nil || self.projectSource.textEffect != nil) {
        message = @"Template of Text Effect will be cleared";
    }
    
    [self alertSimpleActionSheet:@"Change Aspect Ratio"
                         message:message
                    actionTitles:titles
                    dismissTitle:@"Cancel" complete:^(NSInteger index) {
                        if (index == NSNotFound || index == currentType) return;
                        [me changeAspectType:(NXEAspectType) index];
                    }];
}

- (void) changeAspectType:(NXEAspectType) type
{
    [self actionPause];

    BOOL alert = (self.projectSource.templateAssetItem != nil || self.projectSource.textEffect != nil);
    
    self.engine.aspectType = type;
    self.projectSource.templateAssetItem = nil;
    self.projectSource.textEffect = nil;
    [self rebuildAndSetProjectWith:self.projectSource];
    [self realignPreview];
    
    if (alert) {
        [self alert:@"Due to aspect ratio change, Template and/or Text Effect set previously have been cleared" title:@"Information" onOK:^{}];
    }
}

#pragma mark - Menu View
#define BACKGROUNDVIEW_TAG  (0x892D)

- (void) showMenuView:(BOOL) show
{
    self.menuView.hidden = !show;
    UIView *backgroundView = [self.view viewWithTag:BACKGROUNDVIEW_TAG];
    if (show) {
        if (backgroundView == nil) {
            backgroundView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 0, 0)];
            [self.view insertSubview:backgroundView aboveSubview:self.engineView];
            backgroundView.frame = self.view.bounds;
            backgroundView.backgroundColor = UIColor.clearColor;
            backgroundView.tag = BACKGROUNDVIEW_TAG;
            UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapBackground:)];
            [backgroundView addGestureRecognizer:tap];
        }
    } else {
        [backgroundView removeFromSuperview];
        
        [self showColorAdjustmentView:NO];
    }
}

- (void) didTapBackground:(UITapGestureRecognizer *) sender
{
    [self showMenuView:NO];
}

// Show Menu View: Action Bar Button
// Hide Menu View: Action Bar Button or Background
- (IBAction)didTapActions:(id)sender {
    BOOL show = self.menuView.hidden;
    [self showMenuView:show];
}

- (IBAction)didTapMenuButton:(UIButton *)sender {
    MenuButton button = (MenuButton) [sender.superview.subviews indexOfObject:sender];
    
    switch(button) {
        case MenuButtonExport:
            [self showExportActionsIfApplicable];
            break;
        case MenuButtonColorAdjustment:
            [self showColorAdjustmentView:YES];
            break;
            
        case MenuButtonTemplate:
            [self presentTemplatePicker];
            break;
            
        case MenuButtonAspectRatio:
            [self presentAspectRatioSelector];
            break;
        default:
            // Performing segue.
            break;
    }
}

#pragma mark - Unwind Segue
- (void) alertDiscardingVideoLayersExceedLimits
{
    [self alert:@"There are too many video layers at the same time.\nAnd delete other layers except for two." title:@"Video Layer Maximum Exceeded" onOK:^{}];
}

- (IBAction)prepareForUnwindToPreview:(UIStoryboardSegue *)segue {
    UIViewController *vc = segue.sourceViewController;
    
    if ( [vc conformsToProtocol:@protocol(LayerListEditor)] ) {
        id<LayerListEditor> editor = (id<LayerListEditor>) vc;
        [self replaceLayers: editor.layers];
    }
}

@end

#pragma mark - PreviewViewController - ExportControllerDelegate
@implementation PreviewViewController (Export)

- (void)seekTo:(int)time complete:(void (^)(void)) complete
{
    [self seekTo:time idrOnly:NO complete:complete];
}

- (void) exportController:(ExportController *) controller didFinish:(BOOL) finish error:(NXEError *) error
{
    self.state = STOP;
    self.exportController = nil;

    if (error) {
        [self alert:error.localizedDescription title:@"Export Failed" onOK:^{}];
        return;
    }
    
    if (finish) {
        NSURL *url = [NSURL fileURLWithPath:controller.exportPath];

        [[PHPhotoLibrary sharedPhotoLibrary] performChanges:^{
            [PHAssetChangeRequest creationRequestForAssetFromVideoAtFileURL:url];
        } completionHandler:^(BOOL success, NSError *error) {
            if (error) {
                AVURLAsset *asset = [AVURLAsset assetWithURL:url];
                NSString *message = [NSString stringWithFormat:@"Failed saving video to Photos name:%@ playable:%@ error:%@",
                                     url.lastPathComponent,
                                     asset.isPlayable ? @"YES" : @"NO",
                                     error.localizedDescription];
                
                NSLog(@"ERROR: %@", message );
                [self alert:message title:@"Error" onOK:^{}];
                return;
            }
            [self alert:@"Export video saved to Photos" title:@"Information" onOK:^{}];
            
            [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
        }];
    } // else: User cancelled
}
@end
