//
//  TemplateSingleTests.swift
//  NexEditorFramework
//
//  Created by MJ.KONG-MAC on 05/06/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class TemplateSingleTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("DistinctTemplateAssets.bundle/packages"))
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testTemplateSingleOfCamera() {
        var finished = false
        NXEAssetLibrary.instance().items(inCategory: NXEAssetItemCategory.template) { (assetItems:[NXEAssetItem]) in
            let editor = NXEEngine.instance()!
            let sequence = TemplateSequenceParams(path: "ClipSources/template/Camera", clipNames: ["01"], name: "CameraSingle", ext: "jpg", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.camera_s.v.2.0_16v9_Camera_s"])
            
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:sequence.assetItemIds[0] , preview: self.preview, commands: [.play(for: 4.0)])
                finished = true
            }
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testTemplateSingleVideoOfCamera() {
        var finished = false
        NXEAssetLibrary.instance().items(inCategory: NXEAssetItemCategory.template) { (assetItems:[NXEAssetItem]) in
            let editor = NXEEngine.instance()!
            let sequence = TemplateSequenceParams(path: "ClipSources", clipNames: ["video/tvc3sec.mp4"], name: "CameraVidSingle", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.camera_s.v.2.0_16v9_Camera_s"])
            
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:sequence.assetItemIds[0] , preview: self.preview, commands: [.play(for: 4.0)])
                finished = true
            }
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testExportTemplateSingleOfCamera() {
        var finished = false
        
        let editor = NXEEngine.instance()!
        let sequence = TemplateSequenceParams(path: "ClipSources/template/Camera", clipNames: ["01"], name: "CameraSingle", ext: "jpg", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.camera_s.v.2.0_16v9_Camera_s"])
        
        autoreleasepool {
            editor.TTExport(sequence: sequence, templateId: sequence.assetItemIds[0], preview: self.preview, label: self.label)
            finished = true
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testExportTemplateSingleVideoOfCamera() {
        var finished = false
        
        let editor = NXEEngine.instance()!
        let sequence = TemplateSequenceParams(path: "ClipSources", clipNames: ["video/tvc3sec.mp4"], name: "CameraVidSingle", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.camera_s.v.2.0_16v9_Camera_s"])
        
        autoreleasepool {
            editor.TTExport(sequence: sequence, templateId: sequence.assetItemIds[0], preview: self.preview, label: self.label)
            finished = true
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testTemplateSingleOfNeonight() {
        var finished = false
        NXEAssetLibrary.instance().items(inCategory: NXEAssetItemCategory.template) { (assetItems:[NXEAssetItem]) in
            let editor = NXEEngine.instance()!
            let sequence = TemplateSequenceParams(path: "ClipSources/template/NeonLight", clipNames: ["01"], name: "NeonlightSingle", ext: "jpg", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.neonlight_s.v.2.0_16v9_neonlight_s"])
            
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:sequence.assetItemIds[0] , preview: self.preview, commands: [.play(for: 4.0)])
                finished = true
            }
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testTemplateSingleVideoOfNeonight() {
        var finished = false
        NXEAssetLibrary.instance().items(inCategory: NXEAssetItemCategory.template) { (assetItems:[NXEAssetItem]) in
            let editor = NXEEngine.instance()!
            let sequence = TemplateSequenceParams(path: "ClipSources", clipNames: ["video/tvc3sec.mp4"], name: "NeonlightVidSingle", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.neonlight_s.v.2.0_16v9_neonlight_s"])
            
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:sequence.assetItemIds[0] , preview: self.preview, commands: [.play(for: 4.0)])
                finished = true
            }
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testExportTemplateSingleOfNeonight() {
        var finished = false
        
        let editor = NXEEngine.instance()!
        let sequence = TemplateSequenceParams(path: "ClipSources/template/NeonLight", clipNames: ["01"], name: "NeonlightSingle", ext: "jpg", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.neonlight_s.v.2.0_16v9_neonlight_s"])
        
        autoreleasepool {
            editor.TTExport(sequence: sequence, templateId: sequence.assetItemIds[0], preview: self.preview, label: self.label)
            finished = true
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testExportTemplateSingleVideoOfNeonight() {
        var finished = false
        
        let editor = NXEEngine.instance()!
        let sequence = TemplateSequenceParams(path: "ClipSources", clipNames: ["video/tvc3sec.mp4"], name: "NeonlightVidSingle", assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.neonlight_s.v.2.0_16v9_neonlight_s"])
        
        autoreleasepool {
            editor.TTExport(sequence: sequence, templateId: sequence.assetItemIds[0], preview: self.preview, label: self.label)
            finished = true
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
}
