//
//  TemplateAlternativeItemsTest.swift
//  NexEditorFramework
//
//  Created by MJ.KONG-MAC on 07/06/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

extension TemplateSequenceParams {
    func appending(clipNames: [String]) -> TemplateSequenceParams {
        var added = self.clipNames
        added += clipNames
        return TemplateSequenceParams(path: self.path,
                                      clipNames: added,
                                      name: self.name,
                                      ext: self.ext,
                                      assetItemIds: self.assetItemIds)
    }
}

class ZTemplateAlternativeItemsTest: XCTestCase {
    
    let tripTemplateIds = ["com.nexstreaming.kmsdk.design.nexeditor.template.trip.v.2.0_1v1_Trip",
                           "com.nexstreaming.kmsdk.design.nexeditor.template.trip.v.2.0_9v16_Trip",
                           "com.nexstreaming.kmsdk.design.nexeditor.template.trip.v.2.0_16v9_Trip"]
    
    lazy var sequence3clips: TemplateSequenceParams = {
        return TemplateSequenceParams(path: "ClipSources/template/Trip",
                                      clipNames: ["01","02","03"],
                                      name: "Alternate Template",
                                      ext: "jpg",
                                      assetItemIds: self.tripTemplateIds)
    }()

    lazy var sequence4clips: TemplateSequenceParams = {
        return self.sequence3clips.appending(clipNames: ["04"])
    }()
    
    lazy var sequence5clips: TemplateSequenceParams = {
        return self.sequence3clips.appending(clipNames: ["04", "05"])
    }()

    lazy var sequence6clips: TemplateSequenceParams = {
        return self.sequence3clips.appending(clipNames: ["04", "05", "06"])
    }()
    
    override func setUp() {
        super.setUp()
        
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("DistinctTemplateAssets.bundle/packages"))
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func validateTitleEffect(clipCount: Int) {
        
        let sequence: TemplateSequenceParams
        let assetLib = NXEAssetLibrary.instance()
        
        var titleEffect: String = ""
        
        switch clipCount {
        case 3:
            sequence = self.sequence3clips
            titleEffect = "com.nexstreaming.kmsdk.design.nexeditor.effect.trip.16v9.outro_fx_2"
        case 4:
            sequence = self.sequence4clips
            titleEffect = "com.nexstreaming.kmsdk.design.nexeditor.effect.trip.16v9.outro_fx_3"
        case 5:
            sequence = self.sequence5clips
            titleEffect = "com.nexstreaming.kmsdk.design.nexeditor.effect.trip.16v9.outro_fx_4"
        case 6:
            sequence = self.sequence6clips
            titleEffect = "com.nexstreaming.kmsdk.design.nexeditor.effect.trip.16v9.outro_fx_1"
        default:
            XCTFail("clipCount \(clipCount) is not valid for Title Effect 'trip'")
            return
        }
        
        let templateAsset = assetLib.item(forId:"com.nexstreaming.kmsdk.design.nexeditor.template.trip.v.2.0_16v9_Trip")!
        
        let project: NXETemplateProject? = try? NXETemplateProject(templateAssetItem: templateAsset as! NXETemplateAssetItem, clips: sequence.clips())
        
        let clip = project?.visualClips.last!
        XCTAssert(clip!.clipEffectID == titleEffect, "titleEffectID: \(clip!.clipEffectID) is different from titleEffect: \(titleEffect)")
    }
    
    func testPreview3clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence3clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:assetItemId , preview: self.preview, label: self.label, commands: [.playEnd()])
            }
        }
        validateTitleEffect(clipCount: Int(3))
    }
    
    func testExport3clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence3clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTExport(sequence: sequence, templateId: assetItemId, preview: self.preview, label: self.label)
            }
        }
    }
    
    func testPreview4clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence4clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:assetItemId , preview: self.preview, label: self.label, commands: [.playEnd()])
            }
        }
        validateTitleEffect(clipCount: Int(4))
    }
    
    func testExport4clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence4clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTExport(sequence: sequence, templateId: assetItemId, preview: self.preview, label: self.label)
            }
        }
    }
    
    func testPreview5clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence5clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:assetItemId , preview: self.preview, label: self.label, commands: [.playEnd()])
            }
        }
        validateTitleEffect(clipCount: Int(5))
    }
    
    func testExport5clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence5clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTExport(sequence: sequence, templateId: assetItemId, preview: self.preview, label: self.label)
            }
        }
    }
    
    func testPreview6clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence6clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId:assetItemId , preview: self.preview, label: self.label, commands: [.playEnd()])
            }
        }
        validateTitleEffect(clipCount: Int(6))
    }
    
    func testExport6clips() {
        
        let editor = NXEEngine.instance()!
        let sequence = self.sequence6clips
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTExport(sequence: sequence, templateId: assetItemId, preview: self.preview, label: self.label)
            }
        }
        
    }
    
}
