/**
 * File Name   : AssetVolumeTests.swift
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
@testable import AssetReaderApp

class AssetVolumeTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testVolumePathHasSpace() {
        let sourcePath = AssetVolume(volumePath: "\(kURLSchemeAssetVolume)://library/Application Support/_nexeditor/packages")!
        
        XCTAssert(sourcePath.volumeType == .library)
    }
    
    func testVolumePathToVolumeTypePath() {
        var fromType = AssetVolume(type: .bundle, subpath: "SampleAssets/packages")!
        var fromPath = AssetVolume(volumePath: "\(kURLSchemeAssetVolume)://bundle/SampleAssets/packages")!
        XCTAssert(fromType.resolvedPath == fromPath.resolvedPath, "\(fromType.resolvedPath!) != \(fromPath.resolvedPath!)")
        
        fromType = AssetVolume(type: .documents, subpath: "/")!
        fromPath = AssetVolume(volumePath: "\(kURLSchemeAssetVolume)://documents")!
        XCTAssert(fromType.resolvedPath == fromPath.resolvedPath, "\(fromType.resolvedPath!) != \(fromPath.resolvedPath!)")
    }
    
    func testResolvedURLForVolumeType() {
        let subpath = "Application Support/_nexeditor/packages";
        let url = AssetVolume.resolvedFileURL(for: .library, subpath: subpath)!
        do {
            let fmurl = try FileManager.default.url(for: .libraryDirectory, in: .userDomainMask, appropriateFor: nil, create: false)
            let fmpath = (fmurl.path as NSString).appendingPathComponent(subpath)
            XCTAssert(url.path == fmpath)
        } catch(_) {
            XCTFail()
        }
    }

    
}
