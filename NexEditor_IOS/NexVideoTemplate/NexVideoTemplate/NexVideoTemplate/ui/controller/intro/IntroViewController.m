/******************************************************************************
 * File Name   :	IntroViewController.m
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
#import "IntroViewController.h"
#import "NexEditorFramework/NexEditorFramework.h"

@interface IntroViewController ()
@property (strong, nonatomic) IBOutlet UIButton *startButton;
@property (strong, nonatomic) IBOutlet UILabel *versionLabel;

@end

@implementation IntroViewController

- (void)handleInitProcess
{
    
    self.versionLabel.text = [NSString stringWithFormat:@"version %@", [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleShortVersionString"]];
    
}

- (void)viewWillAppear:(BOOL)animated
{
    [self.navigationController setNavigationBarHidden:YES animated:YES];
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self.navigationController setNavigationBarHidden:NO animated:YES];
    [super viewWillDisappear:animated];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view, typically from a nib.
    [self handleInitProcess];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)didInfoButtonClick:(id)sender
{
    NSString *urlString = @"http://www.nexeditorsdk.com/";
    NSURL* url = [[NSURL alloc] initWithString:urlString];
    [[UIApplication sharedApplication] openURL:url];
}

- (IBAction)didStartButtonClick:(id)sender
{
    NSLog(@"didStartButtonClick");
    
}
@end
