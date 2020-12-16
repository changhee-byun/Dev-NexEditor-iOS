/******************************************************************************
 * File Name   : EditorFrameworkTestTests.swift
 * Description :
 *******************************************************************************
 
 NexStreaming Corp. Confidential & Proprietary
 Copyright (C) 2017 NexStreaming Corp. All rights are reserved.
 
 Revision History:
 Author	Date		Description of Changes
 -------------------------------------------------------------------------------
 
 -----------------------------------------------------------------------------*/

import XCTest
@testable import NexEditorFramework
@testable import NexEditorEngine

class EditorFrameworkTestTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testVersion() {
        let sdkVersion = NXEEngine.sdkVersionString()!
        let bundle = Bundle(for: NXEEngine.self)
        let version = bundle.infoDictionary!["CFBundleShortVersionString"] as! String
        let compos = version.characters.split{ $0 == "." }
        
        let formatted: String
        if compos.count == 3 {
            formatted = version
        } else {
            formatted = String(format:"RC \(version) Unofficial")
        }
        XCTAssert(sdkVersion == formatted, "sdkVersion \(sdkVersion) != \(formatted)")
        
    }
	
    func testLevel() {
        let editor = NXEEngine.instance()
        let authData = editor!.authorizationData()
        let sdkLevel:NSNumber = authData!["SDKLevel"] as! NSNumber
        XCTAssert(sdkLevel == NSNumber(value:2), "sdkLevel \(sdkLevel) !=2")
    }
    
    func testBundleFontBevan() {
        let fontName = FontTools.fontName(withBundleFontName: "bevan")
        XCTAssert(fontName == "Bevan", "bevan.ttf font name not found from bundle or not equals to 'Bevan'")
    }
    
    func testAccessFrameworkBundleForClasses() {
        let bundleIdentifier = "com.nexstreaming.NexEditorFramework"
        XCTAssert(Bundle(for:NXEEngine.self).bundleIdentifier == bundleIdentifier)
//        XCTAssert(Bundle(for:EditorUtil.self).bundleIdentifier == bundleIdentifier)
        XCTAssert(Bundle(for:KMLayer.self).bundleIdentifier == bundleIdentifier)
        XCTAssert(Bundle(for:KMWaterMark.self).bundleIdentifier == bundleIdentifier)
        XCTAssert(Bundle(for:LUTMap.self).bundleIdentifier == bundleIdentifier)
        XCTAssert(Bundle(for:NexImage.self).bundleIdentifier == bundleIdentifier)
        XCTAssert(Bundle(for:NXEProject.self).bundleIdentifier == bundleIdentifier)
    }
    
    func testHEVCDecoderAvailability() {
        let editor = NexEditor()
        let supported = editor.isSupportedCodecType(eNEX_CODEC_V_HEVC, isVideo: true, isDecoder: true)
        let version = ProcessInfo.processInfo.operatingSystemVersion
        
        if version.majorVersion >= 11 {
            XCTAssert(supported, "HEVC decoder should be supported on iOS \(version.majorVersion) or later")
        } else {
            XCTAssert(!supported, "HEVC decoder should not be supported on iOS \(version.majorVersion) or earlier")
        }
    }
}
