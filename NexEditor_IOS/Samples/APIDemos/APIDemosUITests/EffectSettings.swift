//
//  EffectSettings.swift
//  APIDemos
//
//  Created by Simon Kim on 2/2/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest

class EffectSettings: XCTestCase {
        
    override func setUp() {
        super.setUp()
        
        // Put setup code here. This method is called before the invocation of each test method in the class.
        
        // In UI tests it is usually best to stop immediately when a failure occurs.
        continueAfterFailure = false
        // UI tests must launch the application that they test. Doing this in setup will make sure it happens for each test method.
        XCUIApplication().launch()

        // In UI tests it’s important to set the initial state - such as interface orientation - required for your tests before they run. The setUp method is a good place to do this.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testSelectClipEffect() {
        
        let app = XCUIApplication()
        let tablesQuery = app.tables
        tablesQuery.staticTexts["Preview"].tap()

        app.collectionViews.staticTexts["Chrysanthemum.jpg"].tap()
        app.collectionViews.staticTexts["Desert.jpg"].tap()
        
        app.navigationBars.element(boundBy: 0).buttons["Done"].tap()
        app.alerts["Confirm"].buttons["16v9"].tap()
        app.buttons["project edit"].tap()
        tablesQuery.cells.element(boundBy: 0).tap()
        tablesQuery.cells.containing(.staticText, identifier:"Clip Effect").staticTexts["none"].tap()

        tablesQuery.staticTexts["kinemaster.builtin.title.bam"].tap()
        tablesQuery.staticTexts["#FFFFFF"].tap()
        tablesQuery.staticTexts["Aqua"].tap()
        tablesQuery.staticTexts["#FFAA00"].tap()
        tablesQuery.staticTexts["Lime"].tap()
        tablesQuery.staticTexts["#FFFF00"].tap()
        tablesQuery.staticTexts["Blue"].tap()
        app.navigationBars.element(boundBy: 0).buttons["Select"].tap()
        app.navigationBars.element(boundBy: 0).buttons["Next"].tap()
        app.navigationBars.element(boundBy: 0).buttons["Apply in project"].tap()
        app.buttons["play default"].tap()
        app.otherElements.element.children(matching: .other).element(boundBy: 1).press(forDuration: 3);
        
        let pauseDefaultButton = app.buttons["pause default"]
        pauseDefaultButton.tap()
        app.navigationBars.element(boundBy: 0).buttons["Select Media"].tap()
        app.navigationBars.element(boundBy: 0).buttons["NexEditor SDK Demo"].tap()
        
        
    }
    
    func testSelectTransitionEffect() {
        
        let app = XCUIApplication()
        let tablesQuery = app.tables
        tablesQuery.staticTexts["Preview"].tap()
        
        app.collectionViews.staticTexts["Chrysanthemum.jpg"].tap()
        app.collectionViews.staticTexts["Desert.jpg"].tap()
        
        app.navigationBars.element(boundBy: 0).buttons["Done"].tap()
        app.alerts["Confirm"].buttons["16v9"].tap()
        
        app.buttons["project edit"].tap()
        tablesQuery.cells.element(boundBy: 0).tap()
        tablesQuery.cells.containing(.staticText, identifier:"Transition Effect").staticTexts["none"].tap()
        tablesQuery.staticTexts["kinemaster.builtin.transition.crossfade"].tap()
        app.navigationBars["Image Clip Properties"].buttons["Next"].tap()
        app.navigationBars["Clips"].buttons["Apply in project"].tap()
        app.buttons["play default"].tap()
        app.otherElements.element.children(matching: .other).element(boundBy: 1).press(forDuration: 3);
        
        let pauseDefaultButton = app.buttons["pause default"]
        pauseDefaultButton.tap()
        app.navigationBars.element(boundBy: 0).buttons["Select Media"].tap()
        app.navigationBars.element(boundBy: 0).buttons["NexEditor SDK Demo"].tap()
        
    }
}
