//
//  CollageDataTests.swift
//  NexEditorSDKTests
//
//  Created by Simon Kim on 12/13/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class CollageDataTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testCollageDataWithInvalidPathShouldNotCrash() {
        do {
            _ = try CollageData(path:"you-will-never-found-this-file")
            XCTFail("Error should have been occured with a wrong path")
        } catch _ {
            XCTAssert(true)
        }
    }
}
