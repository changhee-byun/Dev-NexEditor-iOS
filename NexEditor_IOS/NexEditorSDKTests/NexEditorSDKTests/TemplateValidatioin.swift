//
//  TemplateValidatioin.swift
//  NexEditorFramework
//
//  Created by eric on 27/02/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class TemplateValidatioin: XCTestCase {
    var lib: NXEAssetLibraryProtocol!
    override func setUp() {
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("SampleAssets.bundle/packages"))
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
        
        lib = AssetLibrary.library()
        super.setUp()
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func TTAssertSamePackageId(_ assetItem: NXEAssetItem, _ subItemId: String, _ label: String) {
        let packageId = assetItem.raw.packageId
        let item = lib.item(forId: subItemId)
        XCTAssert(item != nil, "subitem \(subItemId) not found, asset item id:\(assetItem.itemId), \(label)")
        if item != nil {
            XCTAssert(packageId == item!.raw.packageId, "asset item package id:\(packageId!) != subitem package id:\(item!.raw.packageId!), subitem id: \(subItemId), asset item id:\(assetItem.itemId), \(label)")
        }
    }
    
    func testTemplateSubItems()
    {
        /**
         * - intro, loop, outro
         *   - effects, transition, lut
         */
        //        let templateId = "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid_v2.template_16v9_polaroid_v2"
        let params = [
            // polaroid
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_16v9_polaroid", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_9v16_polaroid", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_2v1_polaroid", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_1v1_polaroid", version: "2.0.0", mode: "1v1"),
            // polaroid_v2
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid_v2.template_16v9_polaroid_v2", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid_v2.template_9v16_polaroid_v2", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid_v2.template_2v1_polaroid_v2", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid_v2.template_1v1_polaroid_v2", version: "2.0.0", mode: "1v1"),
            // simple
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.simple.template_16v9_simple", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.simple.template_9v16_simple", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.simple.template_2v1_simple", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.simple.template_1v1_simple", version: "2.0.0", mode: "1v1"),
            // symmetry
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_16v9_symmetry", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_9v16_symmetry", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_2v1_symmetry", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_1v1_symmetry", version: "2.0.0", mode: "1v1"),
            // table
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.table.template_16v9_table", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.table.template_9v16_table", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.table.template_2v1_table", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.table.template_1v1_table", version: "2.0.0", mode: "1v1"),
            // boken
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_16v9_bokeh", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_9v16_bokeh", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_2v1_bokeh", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_1v1_bokeh", version: "2.0.0", mode: "1v1"),
            // camera
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_16v9_camera", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_9v16_camera", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_2v1_camera", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_1v1_camera", version: "2.0.0", mode: "1v1"),
            // cartoon
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_16v9_cartoon", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_9v16_cartoon", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_2v1_cartoon", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_1v1_cartoon", version: "2.0.0", mode: "1v1"),
            // family
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_16v9_family", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_9v16_family", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_2v1_family", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_1v1_family", version: "2.0.0", mode: "1v1"),
            // fashion
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_16v9_fashion", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_9v16_fashion", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_2v1_fashion", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_1v1_fashion", version: "2.0.0", mode: "1v1"),
            // love
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.love.template_16v9_love", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.love.template_9v16_love", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.love.template_2v1_love", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.love.template_1v1_love", version: "2.0.0", mode: "1v1"),
            // neonlight
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_16v9_neonlight", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_9v16_neonlight", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_2v1_neonlight", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_1v1_neonlight", version: "2.0.0", mode: "1v1"),
            // old film
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_16v9_oldfilm", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_9v16_oldfilm", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_2v1_oldfilm", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_1v1_oldfilm", version: "2.0.0", mode: "1v1"),
            // old film bebop
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_bebop.2.0_16v9_oldfilm_bebop", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_bebop.2.0_9v16_oldfilm_bebop", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_bebop.2.0_2v1_oldfilm_bebop", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_bebop.2.0_1v1_oldfilm_bebop", version: "2.0.0", mode: "1v1"),
            // old film busterkeyton
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_busterkeyton.template_2.0_16v9_oldfilm_busterKeyton", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_busterkeyton.template_2.0_9v16_oldfilm_busterKeyton", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_busterkeyton.template_2.0_2v1_oldfilm_busterKeyton", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_busterkeyton.template_2.0_1v1_oldfilm_busterKeyton", version: "2.0.0", mode: "1v1"),
            // old film cameraman
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_cameraman.template_2.0_16v9_oldfilm_cameraman", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_cameraman.template_2.0_9v16_oldfilm_cameraman", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_cameraman.template_2.0_2v1_oldfilm_cameraman", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_cameraman.template_2.0_1v1_oldfilm_cameraman", version: "2.0.0", mode: "1v1"),
            // old film charliechaplin
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_charliechaplin.template_2.0_16v9_oldfilm_charlieChaplin", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_charliechaplin.template_2.0_9v16_oldfilm_charlieChaplin", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_charliechaplin.template_2.0_2v1_oldfilm_charlieChaplin", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_charliechaplin.template_2.0_1v1_oldfilm_charlieChaplin", version: "2.0.0", mode: "1v1"),
            // old film moderntimes
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_moderntimes.template_2.0_16v9_oldfilm_moderntimes", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_moderntimes.template_2.0_9v16_oldfilm_moderntimes", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_moderntimes.template_2.0_2v1_oldfilm_moderntimes", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_moderntimes.template_2.0_1v1_oldfilm_moderntimes", version: "2.0.0", mode: "1v1"),
            // old film swing
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_swing.template_2.0_16v9_oldfilm_swing", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_swing.template_2.0_9v16_oldfilm_swing", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_swing.template_2.0_2v1_oldfilm_swing", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm_swing.template_2.0_1v1_oldfilm_swing", version: "2.0.0", mode: "1v1"),
            // photoframe
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_16v9_photoframe", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_9v16_photoframe", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_2v1_photoframe", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_1v1_photoframe", version: "2.0.0", mode: "1v1"),
            // photoframe_v2
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v2.template_16v9_photoframe_v2", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v2.template_9v16_photoframe_v2", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v2.template_2v1_photoframe_v2", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v2.template_1v1_photoframe_v2", version: "2.0.0", mode: "1v1"),
            // photoframe_v3
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v3.template_16v9_photoframe_v3", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v3.template_9v16_photoframe_v3", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v3.template_2v1_photoframe_v3", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v3.template_1v1_photoframe_v3", version: "2.0.0", mode: "1v1"),
            // photoframe_v4
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v4.template_2.0_16v9_photoframe_v4", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v4.template_2.0_9v16_photoframe_v4", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v4.template_2.0_2v1_photoframe_v4", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe_v4.template_2.0_1v1_photoframe_v4", version: "2.0.0", mode: "1v1"),
            // romantic
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_16v9_romantic", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_9v16_romantic", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_2v1_romantic", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_1v1_romantic", version: "2.0.0", mode: "1v1"),
            // snow
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_16v9_snow", version: "2.0.0", mode: "16v9"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_9v16_snow", version: "2.0.0", mode: "9v16"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_2v1_snow", version: "2.0.0", mode: "2v1"),
            (templateId: "com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_1v1_snow", version: "2.0.0", mode: "1v1"),
            ]
        for param in params {
            let template = lib.item(forId: param.templateId)!
            let data = template.fileInfoResourceData!
            var error: NXEError?
            let helper = TemplateHelper(jsonData: data, deepScan: true, nxeError:&error)!
            XCTAssert(error == nil, "error: \(error!.errorDescription)")
            XCTAssert(helper.version == param.version, "version \(helper.version) not \(param.version)")
            XCTAssert(helper.mode == param.mode, "mode \(helper.mode!) not \(param.mode), templateId:\(param.templateId)")
            if helper.bgm != nil {
                let bgmItem = lib.item(forId: helper.bgm)
                XCTAssert(bgmItem != nil, "bgm \(helper.bgm!) not found, templateId:\(param.templateId)")
                if bgmItem != nil {
                    XCTAssert(template.raw.packageId == bgmItem!.raw.packageId, "package id templae:\(template.raw.packageId!) != bgm:\(bgmItem!.raw.packageId!), bgm \(helper.bgm!), templateId:\(param.templateId)")
                }
            }
            
            let groups: [(group: [TemplateComponent], name: String)] = [
                (group:helper.intro!, name:"intro"),
                (group:helper.loop!, name:"loop"),
                (group:helper.outro!, name:"outro")
            ]
            for group in groups {
                for (index, component) in group.group.enumerated(){
                    if let subItemId = component.effects {
                        TTAssertSamePackageId(template, subItemId as String, "effect at group[\(index)]:\(group.name)")
                    }
                    if let subItemId = component.transition {
                        TTAssertSamePackageId(template, subItemId as String, "transition at group[\(index)]:\(group.name)")
                    }
                    if let subItemId = component.lut {
                        TTAssertSamePackageId(template, subItemId as String, "lut at group[\(index)]:\(group.name)")
                    }
                }
            }
        }
    }
    
    func testSingleTemplateSubItems() {
        var finished = false
        
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("DistinctTemplateAssets.bundle/packages"))
        NXEAssetLibrary.instance().items(inCategory: NXEAssetItemCategory.template) { (items:[NXEAssetItem]) in
            
            if items.count == 0 {
                return
            }
            let assetItems = items.filter( { $0.raw.packageId.contains("DistinctTemplateAssets.bundle") } )
            
            let library = AssetLibrary.library()!
            
            for assetItem in assetItems {
                let template = library.item(forId: assetItem.itemId)!
                let data = template.fileInfoResourceData!
                var error: NXEError?
                let helper = TemplateHelper(jsonData: data, deepScan: true, nxeError:&error)!
                XCTAssert(error == nil, "error: \(error!.errorDescription)")
                XCTAssert(helper.version == "2.0.0", "version \(helper.version) not 2.0.0")
                if helper.bgm_single != nil {
                    let bgmItem = library.item(forId: helper.bgm_single)
                    XCTAssert(bgmItem != nil, "bgm \(helper.bgm_single!) not found, templateId:\(template.itemId)")
                    if bgmItem != nil {
                        XCTAssert(template.raw.packageId == bgmItem!.raw.packageId, "package id templae:\(template.raw.packageId!) != bgm:\(bgmItem!.raw.packageId!), bgm \(helper.bgm_single!), templateId:\(template.itemId)")
                    }
                }
                let groups: [(group: [TemplateComponent], name: String)] = [
                    (group:helper.single!, name:"single"),
                ]
                for group in groups {
                    for (index, component) in group.group.enumerated(){
                        if let subItemId = component.effects {
                            self.TTAssertSamePackageId(template, subItemId as String, "effect at group[\(index)]:\(group.name)")
                        }
                        if let subItemId = component.transition {
                            self.TTAssertSamePackageId(template, subItemId as String, "transition at group[\(index)]:\(group.name)")
                        }
                        if let subItemId = component.lut {
                            self.TTAssertSamePackageId(template, subItemId as String, "lut at group[\(index)]:\(group.name)")
                        }
                    }
                }
            }
            finished = true
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testAlternativeItemsTemplateSubItems() {
        var finished = false
        
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("AltTemplates.bundle/packages"))
        NXEAssetLibrary.instance().items(inCategory: NXEAssetItemCategory.template) { (items:[NXEAssetItem]) in
            
            if items.count == 0 {
                return
            }
            let assetItems = items.filter( { $0.raw.packageId.contains("AltTemplates.bundle") } )
            
            let library = AssetLibrary.library()!
            
            for assetItem in assetItems {
                let template = library.item(forId: assetItem.itemId)!
                let data = template.fileInfoResourceData!
                var error: NXEError?
                let helper = TemplateHelper(jsonData: data, deepScan: true, nxeError:&error)!
                XCTAssert(error == nil, "error: \(error!.errorDescription)")
                XCTAssert(helper.version == "2.0.0", "version \(helper.version) not 2.0.0")
                if helper.bgm_single != nil {
                    let bgmItem = library.item(forId: helper.bgm)
                    XCTAssert(bgmItem != nil, "bgm \(helper.bgm!) not found, templateId:\(template.itemId)")
                    if bgmItem != nil {
                        XCTAssert(template.raw.packageId == bgmItem!.raw.packageId, "package id templae:\(template.raw.packageId!) != bgm:\(bgmItem!.raw.packageId!), bgm \(helper.bgm!), templateId:\(template.itemId)")
                    }
                }
                let groups: [(group: [TemplateComponent], name: String)] = [
                    (group:helper.outro!, name:"outro")
                    ]
                for group in groups {
                    for (index, component) in group.group.enumerated(){
                        if let subItemId = component.effects {
                            self.TTAssertSamePackageId(template, subItemId as String, "effect at group[\(index)]:\(group.name)")
                        }
                        if let subItemId = component.transition {
                            self.TTAssertSamePackageId(template, subItemId as String, "transition at group[\(index)]:\(group.name)")
                        }
                        if let subItemId = component.lut {
                            self.TTAssertSamePackageId(template, subItemId as String, "lut at group[\(index)]:\(group.name)")
                        }
                        if component.alternative_effect != nil {
                            XCTAssert(component.alternative_effect.count == helper.loop.count, "alternative_effect count:\(component.alternative_effect.count) is different from loop count:\(helper.loop.count)")
                            for (efidx, value) in component.alternative_effect {
                                self.TTAssertSamePackageId(template, value, "alternative_effect[\(efidx)] at group[\(index)]:\(group.name)")
                            }
                        }
                        if component.alternative_transition != nil {
                            XCTAssert(component.alternative_audio_pos.count == helper.loop.count, "alternative_transition count:\(component.alternative_transition.count) is different from loop count:\(helper.loop.count)")
                            for (tridx, value) in component.alternative_transition {
                                self.TTAssertSamePackageId(template, value, "alternative_transition[\(tridx)] at group[\(index)]:\(group.name)")
                            }
                        }
                        if component.alternative_lut != nil {
                            XCTAssert(component.alternative_audio_pos.count == helper.loop.count, "alternative_lut count:\(component.alternative_lut.count) is different from loop count:\(helper.loop.count)")
                            for (lutidx, value) in component.alternative_lut {
                                self.TTAssertSamePackageId(template, value, "alternative_lut[\(lutidx)] at group[\(index)]:\(group.name)")
                            }
                        }
                        if component.alternative_audio != nil {
                            XCTAssert(component.alternative_audio_pos.count == helper.loop.count, "alternative_audio count:\(component.alternative_audio.count) is different from loop count:\(helper.loop.count)")
                            for (auidx, value) in component.alternative_audio {
                                self.TTAssertSamePackageId(template, value, "alternative_audio[\(auidx)] at group[\(index)]:\(group.name)")
                            }
                        }
                        if component.alternative_audio_pos != nil {
                            XCTAssert(component.alternative_audio_pos.count == helper.loop.count, "alternative_audio_pos count:\(component.alternative_audio_pos.count) is different from loop count:\(helper.loop.count)")
                        }
                    }
                }
            }
            finished = true
        }
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
}
