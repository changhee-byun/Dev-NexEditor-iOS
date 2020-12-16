//
//  NERSampleTests.swift
//  NERSampleTests
//
//  Created by Simon Kim on 9/29/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NERSample
import NexRenderer

class NERSampleTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testExample() {
        let plat = NXT_ThemeRenderer_Platform_CreateiOS();
        let renderer = NXT_ThemeRenderer_CreateWithPlatform(NXT_RendererType_InternalExportContext, (1024*1024*16), nil, plat)
        NXT_ThemeRenderer_Destroy(renderer, 0)
    }

}
