/*
 * File Name   : ExportController.m
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

#import "ExportController.h"
#import "Util.h"
#import "APIDemos-Swift.h"
@import NexEditorFramework;

@interface ExportController()
@property (nonatomic, retain) UIAlertController *exportProgressAlertController;
@property (nonatomic, retain) UIViewController *hostViewController;
@property (nonatomic, retain) NXEEngine *editor;
@property (nonatomic, strong) NSString *progressMessage;
@property (nonatomic, strong) NSString *exportPath;

- (void)setExportProgress:(int)percent;
- (void)didExportFinish:(BOOL) finish error:(NXEError *) error;
- (void)startExportWithSetting:(ExportSetting *) exportSetting;

@end

@implementation ExportController

- (void)showExportProgressViewWithSetting:(ExportSetting *) setting
{
    self.progressMessage = setting.displayText;
    self.exportProgressAlertController = [UIAlertController alertControllerWithTitle:@"Exporting"
                                                                             message:self.progressMessage
                                                                      preferredStyle:UIAlertControllerStyleAlert];
    
    __weak NXEEngine *editor = self.editor;
    __weak ExportController *me = self;
    UIAlertAction *cancel = [UIAlertAction actionWithTitle:@"Cancel"
                                                     style:UIAlertActionStyleDefault
                                                   handler:^(UIAlertAction * _Nonnull action) {
                                                       [editor stopAsync:^(ERRORCODE errorcode) {
                                                           [me.exportProgressAlertController dismissViewControllerAnimated:YES completion:nil];
                                                           me.exportProgressAlertController = nil;
                                                           [me.delegate exportController:me didFinish:NO error:nil];
                                                       }];
                                                   }];
    
    [self.exportProgressAlertController addAction:cancel];
    
    [self.hostViewController presentViewController:self.exportProgressAlertController animated:YES completion:nil];
}

- (void)setExportProgress:(int)percent
{
    self.exportProgressAlertController.message = [NSString stringWithFormat:@"%@\n%d%%", self.progressMessage, percent];
}

- (void)didExportFinish:(BOOL) finish error:(NXEError *) error
{
    __weak ExportController *me = self;
    [self.exportProgressAlertController dismissViewControllerAnimated:YES completion:^{
        [me.delegate exportController:me didFinish:finish error:error];
    }];
    self.exportProgressAlertController = nil;
    self.progressMessage = nil;
}

-  (void)startExportWithSetting:(ExportSetting *) exportSetting
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0]; // Get documents folder
    NSString *exportPath = [documentsDirectory stringByAppendingPathComponent:[NSString stringWithFormat:@"Export_%@.mp4", [Util getCurrentTime]]];
    
    NXEExportParams params = NXEExportParamsMake((int) exportSetting.width, (int) exportSetting.height);
    params.bitPerSec = (int) exportSetting.bitrate;
    params.framePerSec = (int) exportSetting.fps;
    
    __weak ExportController *me = self;
    [self.editor setCallbackWithEncodingProgress:^(int percent) {
        if(percent > 100) percent = 100;
        [me setExportProgress:percent];
        
    } encodingEnd:^(NXEError *nxeError){
        BOOL finish = (nxeError == nil);
        [me didExportFinish:finish error:nxeError];
    }];
    self.exportPath = exportPath;
    __weak NXEEngine *editor = self.editor;
    [self.delegate seekTo:0 complete: ^() {
        [editor exportToPath:exportPath withParams:params];
    }];
}

- (void)presentExportActionSheet
{
    NSArray <ExportSetting *>* settings = [ExportSettings settingsWith:self.editor.aspectType];
    NSMutableArray <NSString *> *titles = [NSMutableArray array];
    for(ExportSetting* setting in settings) {
        [titles addObject:setting.displayText];
    }
    
    __weak ExportController *me = self;
    [self.hostViewController alertSimpleActionSheet:@"Export"
                                            message:@"Select Quality"
                                       actionTitles:titles
                                       dismissTitle:@"Cancel"
                                           complete:^(NSInteger actionIndex) {
                                               if (actionIndex == NSNotFound) { return; }
                                               ExportSetting *setting = settings[actionIndex];
                                               [me showExportProgressViewWithSetting:setting];
                                               [me startExportWithSetting:setting];
                                           }];
}

- (instancetype) initWithHostViewController:(UIViewController *) host editor:(NXEEngine *) editor
{
    self = [super init];
    if ( self ) {
        self.hostViewController = host;
        self.editor = editor;
    }
    return self;
}

@end
