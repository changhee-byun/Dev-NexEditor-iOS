//
//  TextEffectTests.swift
//  NexEditorFramework
//
//  Created by MJ.KONG-MAC on 18/04/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class TextEffectTests: XCTestCase {
    var editor: NXEEngine!
    
    override func setUp() {
        super.setUp()
        editor = NXEEngine.instance()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("OverlayTitleAssets.bundle/packages"))
        
    }
    
    override func tearDown() {
        editor = nil
        super.tearDown()
    }
    
    func testItemNotFound() {
        let project = NXEProject()
        
        var paths: [String] = []
        var clips: [NXEClip] = []
        paths.append(clipsourceUrl.appendingPathComponent("picture/Desert.jpg").path)
        paths.append(clipsourceUrl.appendingPathComponent("video/canal.mp4").path)
        for path in paths {
            do {
                clips.append( try NXEClip(source: NXEClipSource(path: path)))
            } catch let error {
                XCTFail("Failed open clip a path \(path) error:\(error)")
            }
        }
        project.visualClips = clips
        editor.setProject(project)
        
        let itemId = "you.will.never.find"
        let textEffect = NXETextEffect(id: itemId, params: NXETextEffectParams())
        
        do {
            try project.setTextEffect(textEffect)
            XCTFail("Setting textEffect with invalid itemId(\(itemId) should throw an exception")
        } catch _ {
        }
    }
    
    func testDrive() {
        let project = NXEProject()
        
        var paths: [String] = []
        var clips: [NXEClip] = []
        paths.append(clipsourceUrl.appendingPathComponent("picture/Desert.jpg").path)
        paths.append(clipsourceUrl.appendingPathComponent("video/canal.mp4").path)
        for path in paths {
            do {
                clips.append( try NXEClip(source: NXEClipSource(path: path)))
            } catch let error {
                XCTFail("Failed open clip a path \(path) error:\(error)")
            }
        }
        project.visualClips = clips
        editor.setProject(project)
        
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.1v1.sub"
        
        let textEffectParams = NXETextEffectParams()
        textEffectParams.introTitle = "Intro"
        textEffectParams.introSubtitle = "Subtitle intro"
        textEffectParams.outroTitle = "Outro"
        textEffectParams.outroSubtitle = "Subtitle outro"
        
        let textEffect = NXETextEffect(id: itemId, params: textEffectParams)
        do {
            try project.setTextEffect(textEffect)
        } catch let e {
            XCTFail("Setting textEffect with valid itemId(\(itemId) should not throw an exception: \(e)")
        }
        XCTAssert(project.textEffect != nil)
        
        do {
            try project.setTextEffect(NXETextEffect(id: "you.will.never.find", params: NXETextEffectParams()))
        } catch _ {
        }
        
        XCTAssert(project.textEffect != nil, "textEffect should be unchanged after failing to set a new textEffect")
        XCTAssert(project.textEffect.itemId == itemId, "textEffect.itemId \(project.textEffect.itemId) should be unchanged \(itemId) after failing to set a new textEffect")
        
    }
    
}
