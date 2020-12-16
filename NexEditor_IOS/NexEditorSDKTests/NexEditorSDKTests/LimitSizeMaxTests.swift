//
//  LimitSizeMaxTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 1/26/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest

class LimitSizeMaxTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testLimitSizeMaxEqualWidth() {
        
        let cappedSize = ImageUtil.limitSize(CGSize(width: 4096, height: 172),
                                             maxSize: CGSize(width: 4096, height: 4096),
                                             diff: { (diff) in
                                                
        })
        XCTAssert( cappedSize.width == 4096, "cappedSize.width should be 4096 != \(cappedSize.width)")
        XCTAssert( cappedSize.height == 172, "height should be 172 != \(cappedSize.height)")
    }
    
    func testLimitSizeMaxLargerWidth() {
        var diffDetected = false
        let cappedSize = ImageUtil.limitSize(CGSize(width: 5000, height: 172),
                                             maxSize: CGSize(width: 4096, height: 4096),
                                             diff: { (diff) in
                                                XCTAssert(diff.width == (5000 - 4096))
                                                if (diff.width > 0) {
                                                    diffDetected = true
                                                }
                                                
        })
        XCTAssert(diffDetected == true)
        XCTAssert( cappedSize.width == 4096, "cappedSize.width should be 4096 != \(cappedSize.width)")
        XCTAssert( cappedSize.height == 172, "height should be 172 != \(cappedSize.height)")
    }
    
    func testLimitSizeMaxLargerHeight() {
        var diffDetected = false
        let cappedSize = ImageUtil.limitSize(CGSize(width: 128, height: 5000),
                                             maxSize: CGSize(width: 4096, height: 4096),
                                             diff: { (diff) in
                                                XCTAssert(diff.height == (5000 - 4096))
                                                if (diff.height > 0) {
                                                    diffDetected = true
                                                }
                                                
        })
        XCTAssert(diffDetected == true)
        XCTAssert( cappedSize.width == 128, "cappedSize.width should be 128 != \(cappedSize.width)")
        XCTAssert( cappedSize.height == 4096, "cappedSize.height should be 4096 != \(cappedSize.height)")
    }
    
    func testLimitSizeMaxSmaller() {
        var diffDetected = false
        let cappedSize = ImageUtil.limitSize(CGSize(width: 3000, height: 172),
                                             maxSize: CGSize(width: 4096, height: 4096),
                                             diff: { (diff) in
                                                diffDetected = true
                                                
        })
        XCTAssert(diffDetected == false)
        XCTAssert( cappedSize.width == 3000, "cappedSize.width should be 3000 != \(cappedSize.width)")
        XCTAssert( cappedSize.height == 172, "cappedSize.height should be 172 != \(cappedSize.height)")
    }
    
    func testLimitSizeMaxTextureSize() {
        
        let maxTextureSize: CGFloat = 4096
        XCTAssert( maxTextureSize != 0)
        
        let margin =  CGSize(width: 6.72, height: 6.72)
        var textSize = CGSize(width: maxTextureSize, height: 128)
        var marginedTextSize = CGSize(width: textSize.width + margin.width * 2, height: textSize.height + margin.height * 2)
        
        marginedTextSize = ImageUtil.limitSize(marginedTextSize,
                                               maxSize: CGSize(width: maxTextureSize, height: maxTextureSize),
                                               diff: { (diff) in
                                                textSize.width -= diff.width;
                                                textSize.height -= diff.height;
        })
        
        XCTAssert(marginedTextSize.width == maxTextureSize)
        XCTAssert(ceil(marginedTextSize.height) == ceil(128 + (margin.height * 2)))
        XCTAssert(ceil(textSize.width) == ceil(maxTextureSize - (margin.width * 2)))
        XCTAssert(textSize.height == 128)
    }
    
}
