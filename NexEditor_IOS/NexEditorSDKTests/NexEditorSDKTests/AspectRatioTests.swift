//
//  AspectRatioTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 3/22/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class AspectRatioTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testAspectRatioTool() {
        let params: [(h: Int32, v: Int32, base: Int32, width: Int32, height: Int32)] = [
            (  16,    9, 720, 1280,  720),
            (   9,   16, 720,  720, 1280),
            (   1,    1, 720,  720,  720),
            (   4,    3, 720,  960,  720),
            (   3,    4, 720,  720,  960),
            (   2,    1, 720, 1440,  720),
            (   1,    2, 720,  720, 1440),
            ( 640,  480, 720,  960,  720),
            (1920, 1080, 720, 1280,  720),
            ( 720, 1280, 720,  720, 1280),
            (   0, 1280, 720,    0,    0),
            ( 720,    0, 720,    0,    0),
            ( 720, 1280,   0,    0,    0),
            ]
        
        for param in params {
            let ratio = NXESizeInt(width: param.h, height: param.v)
            let frameSize = AspectRatioTool.frameSize(withRatio: ratio, minSize: param.base)
            
            XCTAssert(frameSize.width == param.width,   "width  \(frameSize.width) != \(param.width), ratio: \(param.h)x\(param.v)")
            XCTAssert(frameSize.height == param.height, "height \(frameSize.height) != \(param.height), ratio: \(param.h)x\(param.v)")
        }
    }
    
    func testAspectFit() {
        let params: [(ratio: NXESizeInt, in: CGSize, out: CGRect)] = [
            (
                NXESizeIntMake(16, 9),
                CGSize(width: 545, height: 320),
                CGRect(x: 0, y: 7, width: 545, height: 306)),
            
            (   NXESizeIntMake(16, 9),
                CGSize(width: 320, height: 545),
                CGRect(x: 0, y: 183, width: 320, height: 179)),
            
            (   NXESizeIntMake(9, 16),
                CGSize(width: 545, height: 320),
                CGRect(x: 182, y: 0, width: 180, height: 320)),
            
            (   NXESizeIntMake(9, 16),
                CGSize(width: 320, height: 545),
                CGRect(x: 7, y: 0, width: 306, height: 545)),

            (   NXESizeIntMake(1, 1),
                CGSize(width: 545, height: 320),
                CGRect(x: 112, y: 0, width: 320, height: 320)),
            
            (   NXESizeIntMake(1, 1),
                CGSize(width: 320, height: 545),
                CGRect(x: 0, y: 112, width: 320, height: 320)),

            // 1:1 414x554 -> 414x414
            (
                NXESizeIntMake(1, 1),
                CGSize(width: 414, height: 554),
                CGRect(x: 0, y: 70, width: 414, height: 414)),
            
            (
                NXESizeIntMake(16, 9),
                CGSize(width: 736, height: 252),
                CGRect(x: 144, y: 0, width: 448, height: 252)),
            (
                NXESizeIntMake(9, 16),
                CGSize(width: 736, height: 252),
                CGRect(x: 297, y: 0, width: 141, height: 252)),
            ]
        for param in params {
            let shrunk = AspectRatioTool.aspectFit(param.in, ratio: param.ratio)
            XCTAssert(shrunk.origin.x == param.out.origin.x,       "\(param.ratio.width)x\(param.ratio.height) \(shrunk.origin.x) != \(param.out.origin.x)")
            XCTAssert(shrunk.origin.y == param.out.origin.y,       "\(param.ratio.width)x\(param.ratio.height) \(shrunk.origin.y) != \(param.out.origin.y)")
            XCTAssert(shrunk.size.width == param.out.size.width,   "\(param.ratio.width)x\(param.ratio.height) \(shrunk.size.width) != \(param.out.size.width)")
            XCTAssert(shrunk.size.height == param.out.size.height, "\(param.ratio.width)x\(param.ratio.height) \(shrunk.size.height) != \(param.out.size.height)")
        }
        
    }
    
    func testAspecTypeToRatio() {
        let types: [(type:NXEAspectType, h:Int32, v:Int32)] = [ (.ratio16v9, 16, 9), (.ratio9v16, 9, 16), (.ratio1v1, 1, 1), (.custom, 1, 1)]
        
        let editor = NXEEngine.instance()!
        for type in types {
            NXEEngine.instance().aspectType = type.type;
            
            XCTAssert(editor.aspectRatio.width == type.h, "\(editor.aspectRatio.width)")
            XCTAssert(editor.aspectRatio.height == type.v, "\(editor.aspectRatio.height)")
        }
    }
    
    func testCustomAspectRatio() {
        let editor = NXEEngine.instance()!
        
        editor.setAspectType(.custom, withRatio: NXESizeInt(width: 2, height: 1))
        let size = LayerManager.sharedInstance().renderRegionSize
        XCTAssert(size.width == 720 * 2, "\(size.width) != 1440")
        XCTAssert(size.height == 720, "\(size.height) != 720")
    }
    
    func testInvalidCustomAspectRatio() {
        let editor = NXEEngine.instance()!
        
        let typeCopy = editor.aspectType
        let ratioCopy = editor.aspectRatio
        
        editor.setAspectType(.custom, withRatio: NXESizeInt(width: -1, height: 0))
        XCTAssert(editor.aspectType == typeCopy, "\(editor.aspectType)")
        XCTAssert(editor.aspectRatio.width == ratioCopy.width, "\(editor.aspectRatio.width) != \(ratioCopy.width)")
        XCTAssert(editor.aspectRatio.height == ratioCopy.height, "\(editor.aspectRatio.height) != \(ratioCopy.height)")
        
    }
}
