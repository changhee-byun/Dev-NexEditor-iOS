/******************************************************************************
 * File Name   :	AppDelegate.m
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

// for nexeditor sdk framework
@import NexEditorFramework;

// for nexeditor sdk full soruce
//#import "NEEffectLibrary.h"

#import "AppDelegate.h"
#import "PreviewViewController.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (UIViewController *)topViewControllerWithRootViewController:(UIViewController *)rootViewController
{
    if(rootViewController == nil) {
        return nil;
    }
    if([rootViewController isKindOfClass:[UINavigationController class]]) {
        return [self topViewControllerWithRootViewController:((UINavigationController *)rootViewController).visibleViewController];
    } else if(rootViewController.presentedViewController != nil) {
        return [self topViewControllerWithRootViewController:rootViewController.presentedViewController];
    }
    return rootViewController;
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    [application setIdleTimerDisabled:YES];
    [NexLogger setLogLevel:0 forCategory:NexLogCategoryErr];
    [NexLogger setLogLevel:0 forCategory:NexLogCategoryWarning];
    [NexLogger setLogLevel:0 forCategory:NexLogCategoryInfo];

    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
    [application setIdleTimerDisabled:NO];
    [[NXEEngine instance] stop];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
//    if(neEffectLibrary == nil) {
//       [self handleInitProcess]; 
//    }
    [application setIdleTimerDisabled:YES];
    
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application {
}

@end
