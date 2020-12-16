/**
 * File Name   : ClipSourceTests.swift
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
import Photos
@testable import NexEditorFramework

class ClipSourceTests: XCTestCase {
    
    override func setUp() {
        super.setUp()

        // MARK: Without this, CNexVideoEditor::getClipInfo_Sync fails with NEXVIDEOEDITOR_ERROR_ARGUMENT_FAILED
        
        _ = NXEEngine.instance()!
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testWithPath() {
        let url = clipsourceUrl.appendingPathComponent("video/canal.mp4")
        let clipSource = NXEClipSource(path: url.path)
        XCTAssert(clipSource != nil)
    }
    
    func testPathNotFound() {
        let path = clipsourceUrl.appendingPathComponent("video/bears.mp4").path
        let wrongPath = "\(path).jpg"
        let clipSource = NXEClipSource(path: wrongPath)!
        
        do {
            _ = try NXEClip(source: clipSource)
            XCTFail("NXEClip() with a file not present should raise exception. path:\(wrongPath)")
        } catch let error as NSError {
            XCTAssert(error.code == Int(ERRORCODE.ERROR_FILE_IO_FAILED.rawValue))
        }
    }
    
    func testWithAVURLAsset() {
        let url = clipsourceUrl.appendingPathComponent("video/canal.mp4")
        let clipSource = NXEClipSource(avurlAsset: AVURLAsset(url: url))
        XCTAssert(clipSource != nil)
    }
    
    func testClip() {
        let url = clipsourceUrl.appendingPathComponent("video/bears.mp4")
        let clipSource = NXEClipSource(path: url.path)!
        
        do {
            let clip = try NXEClip(source: clipSource)
            XCTAssert(clip.durationMs > 0, "clip duration \(clip.durationMs) <= 0")
        } catch let error as NSError {
            XCTFail("Cannot create clip with source \(clipSource.shortDisplayText) error:\(error.localizedDescription)")
        }
        
    }
    
    func testNotSupportClip() {
        let url = clipsourceUrl.appendingPathComponent("notSupportedContents/H263video.mp4")
        let clipSource = NXEClipSource(path: url.path)!
        do {
            _ = try NXEClip(source: clipSource)
            XCTFail("Why does NXEClip throw an exception..?");
        } catch let error as NSError {
            XCTAssert(error.code == Int(ERRORCODE.ERROR_UNSUPPORT_FORMAT.rawValue), "Cannot create clip. error.code = \(error.code)")
        }
    }
}
