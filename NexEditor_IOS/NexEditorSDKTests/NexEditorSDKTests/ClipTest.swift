//
//  ClipTest.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 7/27/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class ClipTest: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    /// NESI-454
    func testSolidColorRed() {
        let imageLoader = NexImage()
        var width: UInt = 0, height: UInt = 0
        var pixelData: UnsafeMutableRawPointer?
        var imageResource:UnsafeMutableRawPointer?
        imageLoader.openFile(NXEClip.newSolidClip("red")!.videoInfo.clipPath, width: &width, height: &height, pixelData: &pixelData, imageResource: &imageResource)
        let pixels = pixelData!.bindMemory(to: UInt8.self, capacity: 4)
        TTVerifyColorPixels(pixels: pixels, 255, 0, 0, 255)
        imageLoader.releaseImageResource(imageResource)
    }
    
    func testSolidColorBlue() {
        let imageLoader = NexImage()
        var width: UInt = 0, height: UInt = 0
        var pixelData: UnsafeMutableRawPointer?
        var imageResource:UnsafeMutableRawPointer?
        imageLoader.openFile(NXEClip.newSolidClip("blue")!.videoInfo.clipPath, width: &width, height: &height, pixelData: &pixelData, imageResource: &imageResource)
        let pixels = pixelData!.bindMemory(to: UInt8.self, capacity: 4)
        TTVerifyColorPixels(pixels: pixels, 0, 0, 255, 255)
        imageLoader.releaseImageResource(imageResource)
    }
    
    func testSolidColorGreen() {
        let imageLoader = NexImage()
        var width: UInt = 0, height: UInt = 0
        var pixelData: UnsafeMutableRawPointer?
        var imageResource:UnsafeMutableRawPointer?
        imageLoader.openFile(NXEClip.newSolidClip("green")!.videoInfo.clipPath, width: &width, height: &height, pixelData: &pixelData, imageResource: &imageResource)
        let pixels = pixelData!.bindMemory(to: UInt8.self, capacity: 4)
        TTVerifyColorPixels(pixels: pixels, 0, 255, 0, 255)
        imageLoader.releaseImageResource(imageResource)
    }
    
    func testSolidColorHashHex() {
        let imageLoader = NexImage()
        var width: UInt = 0, height: UInt = 0
        var pixelData: UnsafeMutableRawPointer?
        var imageResource:UnsafeMutableRawPointer?
        imageLoader.openFile(NXEClip.newSolidClip("#12345678")!.videoInfo.clipPath, width: &width, height: &height, pixelData: &pixelData, imageResource: &imageResource)
        let pixels = pixelData!.bindMemory(to: UInt8.self, capacity: 4)
        // Premultiplied RGB
        let red = premultiply(0x34, alpha: 0x12)
        let green = premultiply(0x56, alpha: 0x12)
        let blue = premultiply(0x78, alpha: 0x12)
        TTVerifyColorPixels(pixels: pixels, red, green, blue, 0xFF)
        imageLoader.releaseImageResource(imageResource)
    }
    
    func premultiply(_ colorCompo: UInt8, alpha: UInt8) -> UInt8 {
        let colorMax = 255.0
        return UInt8(round((Double(colorCompo) / colorMax) * (Double(alpha) / colorMax) * colorMax))
    }
    
    func TTVerifyColorPixels(pixels: UnsafePointer<UInt8>, _ red: UInt8, _ green: UInt8, _ blue: UInt8, _ alpha: UInt8, function: String = #function, line: Int = #line ) -> Void {
        
        // RGBA
        XCTAssert(pixels.advanced(by: 0).pointee == red,   "Red   \(pixels.advanced(by: 0).pointee) != \(red) \(function) \(line)")
        XCTAssert(pixels.advanced(by: 1).pointee == green, "Green \(pixels.advanced(by: 1).pointee) != \(green) \(function) \(line)")
        XCTAssert(pixels.advanced(by: 2).pointee == blue,  "Blue  \(pixels.advanced(by: 2).pointee) != \(blue) \(function) \(line)")
        XCTAssert(pixels.advanced(by: 3).pointee == alpha, "Alpha \(pixels.advanced(by: 3).pointee) != \(alpha) \(function) \(line)")
        
    }
    
}
