//
//  APIDemosUITests.m
//  APIDemosUITests
//
//  Created by Simon Kim on 1/19/17.
//  Copyright © 2017 NexStreaming. All rights reserved.
//

#import <XCTest/XCTest.h>

@interface APIDemosUITests : XCTestCase

@end

@implementation APIDemosUITests

- (void)setUp {
    [super setUp];
    
    // Put setup code here. This method is called before the invocation of each test method in the class.
    
    // In UI tests it is usually best to stop immediately when a failure occurs.
    self.continueAfterFailure = NO;
    // UI tests must launch the application that they test. Doing this in setup will make sure it happens for each test method.
    [[[XCUIApplication alloc] init] launch];
    
    // In UI tests it’s important to set the initial state - such as interface orientation - required for your tests before they run. The setUp method is a good place to do this.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

/// From Theme List, select a theme, preview for 6sec. and return
- (void) runTheme:(NSString *) themeName app:(XCUIApplication *) app
{
    XCUIElementQuery *tablesQuery = app.tables;

    [tablesQuery.staticTexts[themeName] tap];
    [app.navigationBars[@"Theme Title"].buttons[@"Done"] tap];

    [app.buttons[@"play default"] tap];
    [[[[app.otherElements childrenMatchingType:XCUIElementTypeOther] childrenMatchingType:XCUIElementTypeOther] elementBoundByIndex:1] pressForDuration:2];
    [app.buttons[@"pause default"] tap];

    [[app.navigationBars elementBoundByIndex:0].buttons[@"ThemeList Scene"] tap];
    
}

/**
 * Tests ThemeParams class by using 'Serene' theme with canal.mp4 bundle video clip
 */

- (void)testThemesEachBundleVideoClip {
    
    XCUIApplication *app = [[XCUIApplication alloc] init];
    XCUIElementQuery *tablesQuery = app.tables;
    
    NSArray *clipNames = @[@"bears.mp4", @"canal.mp4", @"highway.mp4", @"roller.mp4"];
    
    for (NSString *clipName in clipNames) {
        [tablesQuery.staticTexts[@"Theme"] tap];
        XCUIElement *selectMediaNavigationBar = app.navigationBars[@"Select Media"];
        [app.collectionViews.staticTexts[clipName] tap];
        
        [selectMediaNavigationBar.buttons[@"Done"] tap];
        
        NSArray *themeNames = @[@"Serene", @"Basic", @"Travel", @"On-Stage"];
        
        for (NSString *themeName in themeNames) {
            [self runTheme:themeName app:app];
        }
        
        [app.navigationBars[@"ThemeList Scene"].buttons[@"Select Media"] tap];
        [selectMediaNavigationBar.buttons[@"NexEditor SDK Demo"] tap];
    }
}

- (void) playForDuration:(NSTimeInterval) duration app:(XCUIApplication *) app
{
    NSString *const labelPlay = @"play default";
    NSString *const labelPause = @"pause default";
    
    // Tap play
    [app.buttons[labelPlay] tap];
    
    // Wait until pause button is visible
    XCUIElement *pauseButton = app.buttons[labelPause];
    NSPredicate *notExistsPredicate = [NSPredicate predicateWithFormat:@"exists == true"];
    [self expectationForPredicate:notExistsPredicate evaluatedWithObject:pauseButton handler: nil];
    [self waitForExpectationsWithTimeout:10.0 handler: nil]; // Adjust timeout as needed
    
    // wait for playback duration
    [[[[app.otherElements childrenMatchingType:XCUIElementTypeOther] childrenMatchingType:XCUIElementTypeOther] elementBoundByIndex:1] pressForDuration:duration];
    
    // Tap pause
    [app.buttons[labelPause] tap];
    
}
/**
 * Tests TemplateParams class with 'Family' template and VideDemo_1st.mp4
 */
- (void) testTemplateClip1 {
    
    XCUIApplication *app = [[XCUIApplication alloc] init];
    [app.tables.staticTexts[@"Template"] tap];
    
    XCUIElementQuery *collectionViewsQuery = app.collectionViews;
    XCUIElementQuery *cellsQuery = collectionViewsQuery.cells;
    [[cellsQuery.otherElements containingType:XCUIElementTypeStaticText identifier:@"bears.mp4"].element tap];
    
    XCUIElement *selectMediaNavigationBar = app.navigationBars[@"Select Media"];
    [selectMediaNavigationBar.buttons[@"Done"] tap];
    [[cellsQuery.otherElements containingType:XCUIElementTypeStaticText identifier:@"Family"].element swipeUp];
    [[cellsQuery.otherElements containingType:XCUIElementTypeStaticText identifier:@"Family"].element swipeUp];

    [collectionViewsQuery.staticTexts[@"Grid"] tap];

    [self playForDuration:2 app:app];
    
    [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Template"] tap];
    [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Media"] tap];
    [selectMediaNavigationBar.buttons[@"NexEditor SDK Demo"] tap];
    
}

- (void) testPreviewEachBundleVideoClip {
    
    XCUIApplication *app = [[XCUIApplication alloc] init];
    XCUIElementQuery *tablesQuery = app.tables;
    NSArray *clipNames = @[@"bears.mp4", @"canal.mp4", @"highway.mp4", @"roller.mp4"];
    NSArray *ratios = @[@"16v9", @"9v16"];
    
    for (NSString *clipName in clipNames) {
        for (NSString *ratio in ratios) {
            [tablesQuery.staticTexts[@"Preview"] tap];
            XCUIElementQuery *collectionViewsQuery = app.collectionViews;
            
            [collectionViewsQuery.staticTexts[clipName] tap];
            [app.navigationBars[@"Select Media"].buttons[@"Done"] tap];
            [app.alerts[@"Confirm"].buttons[ratio] tap];
            [app.buttons[@"play default"] tap];
            [[[[app.otherElements childrenMatchingType:XCUIElementTypeOther] childrenMatchingType:XCUIElementTypeOther] elementBoundByIndex:1] pressForDuration:2];
            [app.buttons[@"pause default"] tap];
            
            [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Media"] tap];
            [[app.navigationBars elementBoundByIndex:0].buttons[@"NexEditor SDK Demo"] tap];
        }
    }
}

/**
 * Tests ProjectParams class with bears.mp4 bundle video clip and Flame 60 background music
 */
- (void) testPreviewAudioTrack {
    XCUIApplication *app = [[XCUIApplication alloc] init];
    XCUIElementQuery *tablesQuery = app.tables;
    NSArray *audioNames = @[
                                @"Cute Step Ver1",
                                @"Flame 60",
//                                @"Magical Dream Ver1",
                                @"Saturday Ver1",
//                                @"Sweet Sping"
                            ];
    NSArray *clipNames = @[ @"roller.mp4"];
    NSArray *ratios = @[@"16v9"];
    
    for (NSString *audioName in audioNames) {
        for (NSString *clipName in clipNames) {
            for (NSString *ratio in ratios) {
                [tablesQuery.staticTexts[@"Preview"] tap];
                XCUIElementQuery *collectionViewsQuery = app.collectionViews;
                
                [collectionViewsQuery.staticTexts[clipName] tap];
                [app.navigationBars[@"Select Media"].buttons[@"Done"] tap];
                [app.alerts[@"Confirm"].buttons[ratio] tap];
                
                [app.buttons[@"audio files"] tap];
                
                XCUIElement *audioTrackNavigationBar = app.navigationBars[@"Audio Track"];
                [audioTrackNavigationBar.buttons[@"Add clip"] tap];
                [tablesQuery.staticTexts[audioName] tap];
                [app.navigationBars[@"AudioFiles List"].buttons[@"Next"] tap];
                [audioTrackNavigationBar.buttons[@"Apply in project"] tap];
                
                
                [app.buttons[@"play default"] tap];
                [[[[app.otherElements childrenMatchingType:XCUIElementTypeOther] childrenMatchingType:XCUIElementTypeOther] elementBoundByIndex:1] pressForDuration:2];
                [app.buttons[@"pause default"] tap];
                
                [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Media"] tap];
                [[app.navigationBars elementBoundByIndex:0].buttons[@"NexEditor SDK Demo"] tap];
            }
        }
    }
}

/**
 * Tests TemplateParams class with Family template, canal.mp4 bundle video clip and removing audio track.
 */
- (void) testTemplateRemoveAudio {
    XCUIApplication *app = [[XCUIApplication alloc] init];
    [app.tables.staticTexts[@"Template"] tap];
    
    XCUIElementQuery *collectionViewsQuery = app.collectionViews;
    XCUIElementQuery *cellsQuery = collectionViewsQuery.cells;
    [[cellsQuery.otherElements containingType:XCUIElementTypeStaticText identifier:@"bears.mp4"].element tap];
    
    XCUIElement *selectMediaNavigationBar = app.navigationBars[@"Select Media"];
    [selectMediaNavigationBar.buttons[@"Done"] tap];
    
    [collectionViewsQuery.staticTexts[@"Family"] tap];
    
    // remove audio
    [app.buttons[@"audio files"] tap];
    [app.tables.staticTexts[@"family_bgm"] pressForDuration:1.4];
    [app.alerts[@"Information"].buttons[@"OK"] tap];
    [[app.navigationBars elementBoundByIndex:0].buttons[@"Apply in project"] tap];
    
    [app.buttons[@"play default"] tap];
    [[[[app.otherElements childrenMatchingType:XCUIElementTypeOther] childrenMatchingType:XCUIElementTypeOther] elementBoundByIndex:1] pressForDuration:2];
    [app.buttons[@"pause default"] tap];
    
    [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Template"] tap];
    [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Media"] tap];
    [selectMediaNavigationBar.buttons[@"NexEditor SDK Demo"] tap];
}

/**
 * Tests TemplateParams class with various templates, various bundle video clips and replaced audio track per each video clip.
 */
- (void) testAllTemplatesWithAudioChange {
    XCUIApplication *app = [[XCUIApplication alloc] init];
    
    NSArray *templates = @[
//                           @"Bokeh",
//                           @"Camera",
//                           @"Cartoon",
//                           @"CheckerFlip",
//                           @"Cool",
                           @"CrossFade",
//                           @"Disco",
//                           @"Dynamic",
//                           @"Dynamic2",
//                           @"Epic Opener",
//                           @"Family",
//                           @"Fashion",
//                           @"FilmSlide",
                           @"Funny"
                           ];

    NSArray *audioNames = @[
//                            @"Cute Step Ver1",
//                            @"Flame 60",
                            @"Magical Dream Ver1",
//                            @"Saturday Ver1",
//                            @"Sweet Sping"
                            ];
    
    NSString *clipName = @"roller.mp4";
    for (NSString *audioName in audioNames) {
        [app.tables.staticTexts[@"Template"] tap];
        
        XCUIElementQuery *collectionViewsQuery = app.collectionViews;
        XCUIElementQuery *cellsQuery = collectionViewsQuery.cells;
        [[cellsQuery.otherElements containingType:XCUIElementTypeStaticText identifier:clipName].element tap];
        XCUIElement *selectMediaNavigationBar = app.navigationBars[@"Select Media"];
        [selectMediaNavigationBar.buttons[@"Done"] tap];
        
        for (NSString *template in templates) {
            [collectionViewsQuery.staticTexts[template] tap];
            
            // Audio track settings
            [app.buttons[@"audio files"] tap];
            
            // remove audio
            [[app.tables.cells elementBoundByIndex:0] pressForDuration:1.4];
            [app.alerts[@"Information"].buttons[@"OK"] tap];
            
            // Change audio
            XCUIElement *audioTrackNavigationBar = app.navigationBars[@"Audio Track"];
            [audioTrackNavigationBar.buttons[@"Add clip"] tap];
            [app.tables.staticTexts[audioName] tap];
            [app.navigationBars[@"AudioFiles List"].buttons[@"Next"] tap];
            [audioTrackNavigationBar.buttons[@"Apply in project"] tap];
            
            // play for 2 sec.
            [app.buttons[@"play default"] tap];
            [[[[app.otherElements childrenMatchingType:XCUIElementTypeOther] childrenMatchingType:XCUIElementTypeOther] elementBoundByIndex:1] pressForDuration:2];
            [app.buttons[@"pause default"] tap];
            
            [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Template"] tap];
        }
        [[app.navigationBars elementBoundByIndex:0].buttons[@"Select Media"] tap];
        [[app.navigationBars elementBoundByIndex:0].buttons[@"NexEditor SDK Demo"] tap];
    }
}
@end
