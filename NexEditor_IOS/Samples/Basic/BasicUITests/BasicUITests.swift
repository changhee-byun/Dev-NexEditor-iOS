/**
 * File Name   : BasicUITests.swift
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

import XCTest

class BasicUITests: XCTestCase {
        
    override func setUp() {
        super.setUp()
        
        // Put setup code here. This method is called before the invocation of each test method in the class.
        
        // In UI tests it is usually best to stop immediately when a failure occurs.
        continueAfterFailure = false
        // UI tests must launch the application that they test. Doing this in setup will make sure it happens for each test method.
        
        let app = XCUIApplication()
        setupSnapshot(app)
        app.launch()

        // In UI tests it’s important to set the initial state - such as interface orientation - required for your tests before they run. The setUp method is a good place to do this.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testAspectRatioChanges() {
        XCUIDevice.shared().orientation = .portrait
        
        let app = XCUIApplication()
        let toolbarsQuery = app.toolbars
        toolbarsQuery.buttons["Add"].tap()
        app.children(matching: .window).element(boundBy: 0).children(matching: .other).element.tap()
        app.navigationBars["Camera Roll"].buttons["Select(1)"].tap()
        
        snapshot("01_16x9")
        
        let iconSettingsButton = toolbarsQuery.buttons["icon settings"]
        iconSettingsButton.tap()
        
        let tablesQuery2 = app.tables
        let aspectRatioStaticText = tablesQuery2.staticTexts["Aspect Ratio"]
        aspectRatioStaticText.tap()
        
        let tablesQuery = tablesQuery2
        tablesQuery.staticTexts["9 x 16"].tap()
        
        let backButton = app.navigationBars["Basic.SettingsView"].children(matching: .button).matching(identifier: "Back").element(boundBy: 0)
        backButton.tap()
        snapshot("02_9x16")
        
        iconSettingsButton.tap()
        aspectRatioStaticText.tap()
        tablesQuery.staticTexts["1 x 1"].tap()
        backButton.tap()
        snapshot("03_1x1")
        
        iconSettingsButton.tap()
        aspectRatioStaticText.tap()
        tablesQuery.staticTexts["4 x 3"].tap()
        backButton.tap()
        snapshot("04_4x3")

        iconSettingsButton.tap()
        aspectRatioStaticText.tap()
        tablesQuery.staticTexts["3 x 4"].tap()
        backButton.tap()
        snapshot("05_3x4")
        
        iconSettingsButton.tap()
        aspectRatioStaticText.tap()
        tablesQuery.staticTexts["3 x 1"].tap()
        backButton.tap()
        snapshot("06_3x1")

        iconSettingsButton.tap()
        aspectRatioStaticText.tap()
        tablesQuery.staticTexts["1 x 3"].tap()
        backButton.tap()
        snapshot("07_1x3")
        
        iconSettingsButton.tap()
        aspectRatioStaticText.tap()
        tablesQuery.staticTexts["640 x 480"].tap()
        backButton.tap()
        snapshot("08_640x480")
    }
    
}
