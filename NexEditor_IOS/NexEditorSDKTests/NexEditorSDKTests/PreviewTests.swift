//
//  PreviewTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 5/25/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class PreviewTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testKeepPreviewLayerOne() {
        // NESI-394
        
        autoreleasepool {
            let editor = NXEEngine.instance()!
            editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        }
        autoreleasepool {
            let editor = NXEEngine.instance()!
            editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        }
        
        let count = self.preview.layer.sublayers!.count
        XCTAssert(count == 1, "preview layers \(count) != 1")
    }
    
    func testReplaceNexEditorPreviewLayerOnly() {
        // NESI-394
        
        let sublayer = CAEAGLLayer()
        self.preview.layer.addSublayer( sublayer )
        
        autoreleasepool {
            let editor = NXEEngine.instance()!
            editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        }
        autoreleasepool {
            let editor = NXEEngine.instance()!
            editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        }
        
        let count = self.preview.layer.sublayers!.count
        XCTAssert(count == 2, "preview layers \(count) != 2 (existing one + new)")
    }
    
    func testSingleImageClip() {
        let editor = NXEEngine.instance()!
        
        editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        self.label.text = "Single image clip preview"
        
        let sequence = ClipSequence(path: "ClipSources/picture", clipNames: ["Desert.jpg"], name: "Single image clip preview")
        let project = NXEProject()
        project.visualClips = sequence.clips()
        editor.setProject(project)
        editor.TTPlayTillEnd()
    }
    
    func testSingleVideoClip() {
        let editor = NXEEngine.instance()!
        
        editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        self.label.text = "Single image clip preview"
        
        let sequence = ClipSequence(path: "ClipSources/video", clipNames: ["canal.mp4"], name: "Single image clip preview")
        let project = NXEProject()
        project.visualClips = sequence.clips()
        editor.setProject(project)
        editor.play(commands: [.play(for: 1.0), .pause(for: 0), .seekTo(scale: 0.9), .resume(for: 1.0)])
    }
}
