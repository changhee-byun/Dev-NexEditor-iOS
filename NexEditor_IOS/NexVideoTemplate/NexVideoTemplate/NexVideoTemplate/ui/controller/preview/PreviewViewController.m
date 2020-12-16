/******************************************************************************
 * File Name   :	PreviewViewController.m
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
#import "PreviewViewController.h"
#import "TemplateComposer.h"
#import "TemplateCollectionViewCell.h"
#import "CustomIOSAlertView.h"
#import "Util.h"
#import "NexEditorFramework/NexEditorFramework.h"

typedef enum STATE
{
    INIT = 0,
    WILLPLAY,
    PLAY,
    PAUSE,
    SEEK,
    PAUSE_INSEEK,
    PAUSE_INSEEK_END,
    STOP,
    EXPORT,
    EXPORT_INBACKGROUND
}PLAY_STATE;

typedef enum SEEKMODE
{
    SEEKMODE = 1,
    SEEKMODE_IDR = 2
} SEEK_MODE;


@interface PreviewViewController() <UICollectionViewDataSource, UICollectionViewDelegate, UICollectionViewDelegateFlowLayout>

/* ui component */
@property (strong, nonatomic) IBOutlet UICollectionView *templateCollectionView;
@property (strong, nonatomic) IBOutlet UISlider *progressSlider;
@property (strong, nonatomic) IBOutlet UILabel *totalLabel;
@property (strong, nonatomic) IBOutlet UILabel *startLabel;
@property (strong, nonatomic) IBOutlet UIView *engineView;
@property (strong, nonatomic) IBOutlet UIButton *playButton;
@property (strong, nonatomic) IBOutlet UIView *saveVideoView;
@property (strong, nonatomic) IBOutlet UIButton *aspectButton;
@property (nonatomic, retain) UIProgressView *exportProgressView;
@property (nonatomic, retain) UILabel *exportLabel;

/* nexeditor */
@property (nonatomic, retain) NXEEngine *engine;
@property (nonatomic, retain) NXEProject *project;
@property (nonatomic, retain) NXEProject *cloneProject;
@property (nonatomic, retain) NSString *exportPath;
@property (nonatomic, assign) enum STATE state;
@property (nonatomic, retain) CustomIOSAlertView *exportAlert;
@property (nonatomic, retain) CustomIOSAlertView *exportRetryAlert;

@property (nonatomic, retain) TemplateComposer *templatecomposer;
@property (nonatomic, assign) NXEAspectType aspectType;

/* about seek */
@property (nonatomic, assign) enum SEEKMODE seekMode;
@property (nonatomic, assign) int seekCurrentTS;

/* selected content information */
@property (nonatomic, retain) NSMutableArray *selectedFileList;
@property (nonatomic, retain) NSMutableArray *selectedAVURLAssetList;
@end

@implementation PreviewViewController


- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    
    // initialize config
    [self initconfig];
    
    _templatecomposer = [[TemplateComposer alloc] init];
    
    // engine setup: assign template index 0 for default
    [self initPreview];
    
    // effect loading
    [self effectLoading];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)viewDidLayoutSubviews
{
    [super viewDidLayoutSubviews];
    
    if(self.state == INIT) {
        [self.engine setPreviewWithCALayer:self.engineView.layer
                               previewSize:CGSizeMake(self.engineView.bounds.size.width, self.engineView.bounds.size.height)];
        
        [self setupUI:0];
        self.state = STOP;

    }
}

- (void)viewDidDisappear:(BOOL)animated
{
    if(self.state != PLAY) {
        [self cleanup];
    } else {
        [self.engine stopSync:^(ERRORCODE errorcode) {
            [self cleanup];
        }];
    }
    
    
    self.project = nil;
    self.cloneProject = nil;
    
    self.exportAlert = nil;
    self.exportRetryAlert = nil;
    self.templatecomposer = nil;
}

-(void)viewWillDisappear:(BOOL)animated
{
    [super viewWillDisappear:animated];
}

#pragma mark - NexEditor
- (void)setupProject
{
    self.project = [[NXEProject alloc] init];
    if(self.selectedFileList != nil) {
        for(int i = 0; i < [self.selectedFileList count]; i++) {
            int error;
            NXEClip *clip = [NXEClip newSupportedClip:[self.selectedFileList objectAtIndex:i] error:&error];
            
            if(clip != nil) {
                [self.project addClip:clip];
            } else {
                NSException *e = [NSException exceptionWithName:@"GeneralException"
                                                         reason:[NSString stringWithFormat:@"%@%d", @"newSupportedClip Error:", error]
                                                       userInfo:nil];
                @throw e;
            }
            clip = nil;
        }
    } else {
        NSException *e = [NSException exceptionWithName:@"GeneralException"
                                                 reason:@"selectedFileList is nil"
                                               userInfo:nil];
        @throw e;
    }
    
    [self.engine setProject:self.project];
    [self.project updateProject];
}

- (void)setupUICallback
{
    __block PreviewViewController *blockself = self;
    [self.engine setCallbackWithPlayProgress:^(int currentTime) {
        blockself.startLabel.text = [Util getStringTime:currentTime];
        blockself.progressSlider.value = currentTime;
    } playStart:^{
        blockself.state = PLAY;
        
        blockself.templateCollectionView.userInteractionEnabled = YES;
        blockself.aspectButton.enabled = YES;
    } playEnd:^{
        [blockself playStop];
    }];
    
    [self.engine setCallbackWithEncodingProgress:^(int percent) {
        blockself.exportLabel.text = [NSString stringWithFormat:@"Progress %d%%", (int)percent];
        blockself.exportProgressView.progress = (float)percent / (float)100;
    } encodingEnd:^{
        [blockself.exportAlert close];
        blockself.saveVideoView.hidden = NO; // change save photo album view
        blockself.state = STOP;
    }];
    
    [self.engine setCallbackWithSetTime:^(int seekDoneTS) {
        if(blockself.state == EXPORT) {
            [blockself.engine exportProject:blockself.exportPath
                                 Width:1280
                                Height:720
                               Bitrate:6*1024*1024
                           MaxFileSize:LONG_MAX
                       ProjectDuration:0
                    ForceDurationLimit:FALSE
                                  Flag:0x1];
        } else if(blockself.state == PAUSE_INSEEK_END) {
            [blockself resume];
        }
    }];
}

- (void)setupTemplate:(NSArray*)templateInfo
{
    self.cloneProject = [self.project copy];
    NSString *templatePath = [templateInfo objectAtIndex:3];
    NSString *templateFileName = [templatePath lastPathComponent];
    
    [self.templatecomposer applyTemplateToProject:self.cloneProject templateFileName:templateFileName inDirectory:@"Resource/asset/template"];
    [self.engine setProject:self.cloneProject];
    [self.cloneProject updateProject];
}

- (void)changeAspect:(NXEAspectType)aspect
{
    if(self.engine == nil) {
        NSLog(@"error -> engine is nil");
        return ;
    }
        
    if(aspect == NXE_ASPECT_16v9) {
        self.aspectType = NXE_ASPECT_16v9;
        [self.aspectButton setImage:[UIImage imageNamed:@"aspectRatio-16_9_white.png"] forState:UIControlStateNormal];
    } else if(aspect == NXE_ASPECT_9v16) {
        self.aspectType = NXE_ASPECT_9v16;
        [self.aspectButton setImage:[UIImage imageNamed:@"aspectRatio-9_16_white.png"] forState:UIControlStateNormal];
    }
    
    CGRect changeframe = self.aspectButton.frame;
    int oldWidth = self.aspectButton.frame.size.width;
    int oldHeight = self.aspectButton.frame.size.height;
    int newWitdh = self.aspectButton.frame.size.height;
    int newHeight = self.aspectButton.frame.size.width;
    
    changeframe.size = CGSizeMake(newWitdh, newHeight);
    changeframe.origin = CGPointMake(self.aspectButton.frame.origin.x + oldWidth - newWitdh,
                                     self.aspectButton.frame.origin.y + oldHeight - newHeight);
    self.aspectButton.frame = changeframe;
    
    NSInteger index = [[self.templateCollectionView indexPathsForSelectedItems] objectAtIndex:0].row;
    
    if(self.state == PLAY) {
        self.aspectButton.enabled = NO;
        self.templateCollectionView.userInteractionEnabled = NO;
        [self playStopWithComplete:^{
            [self cleanup];
            
            // engine setup: assign template index
            [self initPreview:(int)index];
            [self play];
        }];
    } else {
        [self cleanup];
        [self initPreview:(int)index];
    }
}

- (void)cleanup
{
    [self.engine clearScreen];
    [self.engine cleanup:NO];
    
    self.engine = nil;
}

- (void)play
{
//    for(int i = 0; i < [self.engine.project.visualItems count]; i++) {
//        NXEClip* clip = (NXEClip*)self.engine.project.visualItems[i];
//        NSLog(@"clip %d rotate: %d", (int)i, (int)clip.vinfo.rotateState);
//    }
    
    [self.engine play];
    [self.playButton setImage:[UIImage imageNamed:@"pause_btn.png"] forState:UIControlStateNormal];
    self.state = WILLPLAY;
}

- (void)pause
{
    [self.engine pause];
    [self.playButton setImage:[UIImage imageNamed:@"play_btn.png"] forState:UIControlStateNormal];
    self.state = PAUSE;
}

- (void)resume
{
    [self.engine resume];
    [self.playButton setImage:[UIImage imageNamed:@"pause_btn.png"] forState:UIControlStateNormal];
    self.state = PLAY;
}

- (void)playStop
{
    [self.engine stopSync:nil];
    [self.playButton setImage:[UIImage imageNamed:@"play_btn.png"] forState:UIControlStateNormal];
    self.progressSlider.value = 0;
    self.startLabel.text = @"00:00";
    self.state = STOP;
}

- (void)playStopWithComplete:(void(^)())stopCompleteBlock;
{
    self.playButton.enabled = NO;
    [self.engine stopASync:^(ERRORCODE errorcode) {
        self.playButton.enabled = YES;
        [self.playButton setImage:[UIImage imageNamed:@"play_btn.png"] forState:UIControlStateNormal];
        self.state = STOP;
        stopCompleteBlock();
    }];

}

- (void)export
{
    self.state = EXPORT;
    [self seek:0];
}

- (void)exportStop:(PLAY_STATE)changeState
{
    [self.engine stopASync:^(ERRORCODE errorcode) {
        [Util removeFileInDocument:self.exportPath];
        self.state = changeState;
    }];
}

- (void)seek:(int)seekTime
{
    self.seekCurrentTS = seekTime;
    if(self.seekMode == SEEKMODE) {
        [self.engine seek:seekTime];
    } else {
        // TODO : change seekIDRorI
        [self.engine seek:seekTime];
    }
}
#pragma end

#pragma mark - Private
- (void)effectLoading
{
    UIAlertView *alertView = [[UIAlertView alloc] initWithTitle:@"NexVideoTemplate"
                                                        message:@"Prepareing..."
                                                       delegate:self
                                              cancelButtonTitle:nil
                                              otherButtonTitles:nil];
    
    [alertView show];
    [NXEEffectLibrary sharedInstance:FALSE
                     completeLoading:^(int numError) {
                         dispatch_async(dispatch_get_main_queue(), ^{
                             [alertView dismissWithClickedButtonIndex:-1 animated:TRUE];
                             
                             // assign default template 0
                             [self setupTemplate:[self.templatecomposer getTemplateItemAtIndex:0 aspectRatio:self.aspectType]];

                         });
                     }];
}

- (void)initPreview
{
    [NXEEngine setAspectMode:self.aspectType];
    _engine = [NXEEngine sharedInstance];
    [self setupProject];
    [self setupUICallback];
}

- (void)initPreview:(int)cellIndex
{
    [NXEEngine setAspectMode:self.aspectType];
    self.engine = [NXEEngine sharedInstance];
    [self setupProject];
    [self setupUICallback];

    [self.engine setPreviewWithCALayer:self.engineView.layer
                           previewSize:CGSizeMake(self.engineView.bounds.size.width, self.engineView.bounds.size.height)];
    
    [self setupTemplate:[self.templatecomposer getTemplateItemAtIndex:cellIndex aspectRatio:self.aspectType]];
    [self setupUI:cellIndex];
    
}

- (void)setupUI:(int)cellIndex
{
    if(cellIndex >= 0) {
        [self.templateCollectionView selectItemAtIndexPath:[NSIndexPath indexPathForItem:cellIndex inSection:0]
                                                  animated:YES
                                            scrollPosition:UICollectionViewScrollPositionCenteredVertically];
    }
    
    self.totalLabel.text = [Util getStringTime:[self.engine.project getTotalTime]];
    self.progressSlider.maximumValue = [self.engine.project getTotalTime];
}

- (void)initconfig
{
    _aspectType = NXE_ASPECT_16v9;
    _seekMode = 1;
    _seekCurrentTS = 0;
    _state = INIT;
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didEnterForeground) name:UIApplicationDidBecomeActiveNotification object:nil];
    self.navigationController.navigationBar.backItem.title = @"";
    
    UICollectionViewFlowLayout *collectionViewLayout = (UICollectionViewFlowLayout*)self.templateCollectionView.collectionViewLayout;
    collectionViewLayout.sectionInset = UIEdgeInsetsMake(20, 0, 20, 0);
    
    self.templateCollectionView.delegate = self;
    self.templateCollectionView.dataSource = self;
    self.saveVideoView.hidden = YES;

}

- (void)showExportAlert
{
    __block PreviewViewController *blockself = self;
    self.exportAlert = [[CustomIOSAlertView alloc] init];
    [self.exportAlert setContainerView:[self createExportAlertView]];
    [self.exportAlert setButtonTitles:[NSMutableArray arrayWithObjects:@"Cancel", nil]];
    
    [self.exportAlert setOnButtonTouchUpInside:^(CustomIOSAlertView *alertView, int buttonIndex) {
        NSLog(@"Block: Button at position %d is clicked on alertView %d.", buttonIndex, (int)[alertView tag]);
        [blockself exportStop:STOP];
        [alertView close];
    }];
    
    [self.exportAlert setUseMotionEffects:TRUE];
    [self.exportAlert show];
}

- (UIView*)createExportAlertView
{
    int containerWidth = 300;
    int containerHeight = 100;
    int centerMargin = 20;
    
    UIView *container = [[UIView alloc]initWithFrame:CGRectMake(0, 0, containerWidth, containerHeight)];
    
    self.exportLabel = [[UILabel alloc]initWithFrame:CGRectMake(0, 0, containerWidth / 2, 50)];
    self.exportLabel.text = @"Progress 0%";
    self.exportLabel.textAlignment = NSTextAlignmentCenter;
    self.exportLabel.center = CGPointMake(container.center.x, container.center.y - centerMargin);
    
    self.exportProgressView = [[UIProgressView alloc]initWithFrame:CGRectMake(0, 0, containerWidth * 2 / 3, 2)];
    self.exportProgressView.progress = 0;
    self.exportProgressView.backgroundColor = [UIColor redColor];
    self.exportProgressView.center = CGPointMake(container.center.x, container.center.y + centerMargin);
    
    [container addSubview:self.exportProgressView];
    [container addSubview:self.exportLabel];
    
    return container;
}

- (void)showExportRetryAlert
{
    __block PreviewViewController *blockself = self;
    self.exportRetryAlert = [[CustomIOSAlertView alloc] init];
    [self.exportRetryAlert setContainerView:[self createExportRetryAlertView]];
    [self.exportRetryAlert setButtonTitles:[NSMutableArray arrayWithObjects:@"Retry", @"Cancel", nil]];
    [self.exportRetryAlert setOnButtonTouchUpInside:^(CustomIOSAlertView *alertView, int buttonIndex) {
        if(buttonIndex == 0) {
            // retry
            [alertView close:0];
            [blockself showExportAlert];
            [blockself export];
        } else if(buttonIndex == 1) {
            // cancel
            [blockself.exportRetryAlert close];
            [blockself exportStop:STOP];
        }
    }];
    [self.exportRetryAlert setUseMotionEffects:TRUE];
    [self.exportRetryAlert show];
}

- (UIView*)createExportRetryAlertView
{
    int containerWidth = 300;
    int containerHeight = 100;
    
    UIView *container = [[UIView alloc]initWithFrame:CGRectMake(0, 0, containerWidth, containerHeight)];
    
    UILabel *label = [[UILabel alloc]initWithFrame:CGRectMake(0, 0, containerWidth, containerHeight)];
    label.text = @"The Application stopped exporting your video due to another active application. Retry exporting";
    label.textAlignment = NSTextAlignmentCenter;
    label.center = container.center;
    label.adjustsFontSizeToFitWidth = YES;
    label.lineBreakMode = UILineBreakModeWordWrap;
    label.numberOfLines = 4;
    
    [container addSubview:label];
    
    return container;
}
#pragma end

#pragma mark - Collection View Delegate
- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    /* templatecomposer listcount is total count with 16v9an 9v16. so listcount must be divided into two */
    return [self.templatecomposer listCount] / 2;
}

- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *identifier = @"TemplateCollectionViewCell";
    TemplateCollectionViewCell *cell = (TemplateCollectionViewCell *)[collectionView dequeueReusableCellWithReuseIdentifier:identifier forIndexPath:indexPath];
    
    if(cell.selected) {
        cell.isChecked.hidden = NO;
    } else {
        cell.isChecked.hidden = YES;
    }
    
    NSString *name = @"";
    UIImage *thumbnail = nil;
    
    // assign filename and thumbnail
    name = [self.templatecomposer getTemplateNameAtIndex:indexPath.row aspectRatio:self.aspectType];
    thumbnail = [self.templatecomposer getTemplateImageAtIndex:indexPath.row aspectRatio:self.aspectType];
    
    if(thumbnail != nil) {
        cell.thumbnail.image = thumbnail;
    } else {
        cell.thumbnail.image = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:@"default_black" ofType:@"png"]];
    }
    
    return cell;
}


- (void)collectionView:(UICollectionView *)collectionView didHighlightItemAtIndexPath:(NSIndexPath *)indexPath
{
}

- (void)collectionView:(UICollectionView *)collectionView didUnhighlightItemAtIndexPath:(NSIndexPath *)indexPath
{
}

- (void)collectionView:(UICollectionView *)collectionView didSelectItemAtIndexPath:(NSIndexPath *)indexPath
{
   
    TemplateCollectionViewCell* cell = (TemplateCollectionViewCell*)[collectionView cellForItemAtIndexPath:indexPath];
    
    cell.selected = YES;
    cell.isChecked.hidden = NO;
    
    self.templateCollectionView.userInteractionEnabled = NO;
    self.aspectButton.enabled = NO;

    // pause
    if(self.state == PLAY) {
        [self playStop];
    } else {
        self.progressSlider.value = 0;
        self.startLabel.text = @"00:00";
    }
    
    
    // apply template
    NSArray *templateInfo = [self.templatecomposer getTemplateItemAtIndex:indexPath.row aspectRatio:self.aspectType];
    
    [self setupTemplate:templateInfo];
    [self setupUI:(int)indexPath.row];
    
    // play
    [self play];
    
}


- (void)collectionView:(UICollectionView *)collectionView didDeselectItemAtIndexPath:(NSIndexPath *)indexPath
{
    TemplateCollectionViewCell* cell = (TemplateCollectionViewCell*)[collectionView cellForItemAtIndexPath:indexPath];
    
    cell.selected = NO;
    cell.isChecked.hidden = YES;
    
}

- (CGSize)collectionView:(UICollectionView *)collectionView layout:(UICollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath
{
    float margin = 1.0f;
    float cellWidth = self.templateCollectionView.frame.size.width / 3.0f - margin;
    float cellHeight = cellWidth;
    return CGSizeMake(cellWidth, cellHeight);
}
#pragma end

#pragma mark - UI Component Delegate
- (void)didEnterBackground
{
    if(self.state == EXPORT) {
        [self.exportAlert close:0];
        [self exportStop:EXPORT_INBACKGROUND];
    }
}

- (void)didEnterForeground
{
    if(self.state == EXPORT_INBACKGROUND) {
        [self showExportRetryAlert];
    }
}

- (IBAction)didAspectTypeButtonClick:(id)sender
{
    if(self.aspectType == NXE_ASPECT_16v9) {
        [self changeAspect:NXE_ASPECT_9v16];
    } else if(self.aspectType == NXE_ASPECT_9v16){
        [self changeAspect:NXE_ASPECT_16v9];
    }
}

- (IBAction)didExportButtonClick:(id)sender
{
    if(self.state == EXPORT) {
        NSLog(@"retry !!");
        return ;
    }
    
    if(self.state == PLAY) {
        [self playStop];
    }
    
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0]; // Get documents folder
    self.exportPath = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"Export_%@.mp4", [Util getCurrentTime]]];
    [self showExportAlert];
    [self export];
}

- (IBAction)didPlayButtonClick:(id)sender
{
    if(self.state == SEEK || self.state == PAUSE_INSEEK) {
        return ;
    }
    
    if(self.state == PLAY) {
        [self pause];
    } else {
        if(self.state == PAUSE) {
            [self resume];
        } else {
            [self play];
        }
    }
}

- (IBAction)didSaveVideoToPhotoAlbumButtonClick:(id)sender
{
    UISaveVideoAtPathToSavedPhotosAlbum(self.exportPath, self, @selector(video:didFinishSavingWithError:contextInfo:), nil);
    self.saveVideoView.hidden = YES;
    
    [Util toastMessage:self title:nil message:@"save success!" duration:1];
}

- (IBAction)progressSliderValueChanged:(id)sender
{
    UISlider *progressSlider = sender;
    if(fabs(self.seekCurrentTS - progressSlider.value) > 100) {
        self.seekMode = SEEKMODE;
    } else {
        self.seekMode = SEEKMODE_IDR;
    }
    
    if(self.state == PLAY) {
        [self pause];
        self.state = PAUSE_INSEEK;
    } else if(self.state == PAUSE_INSEEK) {
        
    } else {
        self.state = SEEK;
    }
    
    [self seek:progressSlider.value];
    self.startLabel.text = [Util getStringTime:progressSlider.value];
}

- (IBAction)progressSliderTouchUpInside:(id)sender
{
    if(self.state == PAUSE_INSEEK) {
        self.state = PAUSE_INSEEK_END;
    }
}

- (IBAction)progressSliderTouchOutside:(id)sender
{
    if(self.state == PAUSE_INSEEK) {
        self.state = PAUSE_INSEEK_END;
    }
}

- (void)video:(NSString *)videoPath didFinishSavingWithError:(NSError *)error contextInfo:(void *)contextInfo
{
    NSLog(@"save finish so remove export file in document");
    [Util removeFileInDocument:self.exportPath];
    
    if(error) {
        NSLog(@"didFinishSavingWithError: %@", error);
    }
}
#pragma end

#pragma mark - setPreview from segue
- (void)setProject4Preview:(NSMutableArray*)selectedFileList avURLAssetList:(NSMutableArray*)selectedAVURLAssetList
{
    self.selectedFileList = selectedFileList;
    self.selectedAVURLAssetList = selectedAVURLAssetList;
    
}
#pragma mark end
@end
