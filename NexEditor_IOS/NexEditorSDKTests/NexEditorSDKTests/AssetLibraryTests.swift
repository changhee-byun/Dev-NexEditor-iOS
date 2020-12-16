/**
 * File Name   : AssetLibraryTests.swift
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
@testable import NexEditorFramework
@testable import NexEditorSDKTest

class AssetLibraryTests: XCTestCase {
    var lib: NXEAssetLibrary!
    override func setUp() {
        InternalAssetLibrary().removeAllAssetPackageURLs()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("SampleAssets.bundle/packages"))
        lib = NXEAssetLibrary.instance()
        super.setUp()
    }
    
    override func tearDown() {
        super.tearDown()
    }
    
    func testTemplatesCountNonZero() {
        
        let templates = lib.items(inCategory: NXEAssetItemCategory.template)
        XCTAssert(templates.count == 4)
    }
    
    func testTemplatesMetaData() {
        let templates = lib.items(inCategory: NXEAssetItemCategory.template) as! [NXETemplateAssetItem]
        
        var jobs = 0
        for template in templates {
            XCTAssert(template.label.characters.count != 0)
            XCTAssert(template.name.characters.count != 0)
            XCTAssert(template.desc.characters.count != 0)
            
            XCTAssert(template.aspectType != .unknown)
            if template.hasIcon {
                jobs += 1
                template.loadIconImageData { data, ext in
                    XCTAssert(data != nil, "Has icon but image not loaded for \(template.itemId!)")
                    jobs -= 1
                }
            }
        }
        while jobs > 0 {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testTransitionIcons() {
        let items = lib.items(inCategory: NXEAssetItemCategory.transition)

        let iconExts = [ "png", "jpg", "svg"]
        
        var jobs = 0
        for item in items {
            if item.hasIcon {
                jobs += 1
                print("loading \(jobs) \(item.name!)")
                item.loadIconImageData { data, ext in
                    XCTAssert(data!.count > 0, "data.count (\(data!.count)) <= 0 item:\(item.name!)")
                    XCTAssert(iconExts.index(of: ext!) != nil)
                    jobs -= 1
                    print("loaded  \(jobs) \(item.name!)")
                }
            }
        }
        while jobs > 0 {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
        print("done")
    }
    
    func testTransitionInfoCrossfade() {
        let itemId = "com.nexstreaming.kinemaster.builtin.transition.crossfade"
        let item = lib.item(forId: itemId) as! NXETransitionAssetItem
        XCTAssert(item.effectDuration == 2000)
        XCTAssert(item.effectOffset == 100)
        XCTAssert(item.effectOverlap == 100)
        XCTAssert(item.userFields.count == 0)
    }
	
    func testAssetItemProtocolTransitionInfoCrossfade() {
        let itemId = "com.nexstreaming.kinemaster.builtin.transition.crossfade"
        let item = lib.item(forId: itemId) as! NXEAssetItemProtocol
        var jobs = 0

        XCTAssert( item.itemId == itemId, "itemId \(item.itemId) != expected \(itemId)")
        XCTAssert( item.category == NXEAssetItemCategory.transition, "itemId \(item.category) != expected \(NXEAssetItemCategory.transition)")
        XCTAssert( item.aspectType == .unknown, "itemId \(item.aspectType.rawValue) != expected \(NXEAspectType.unknown.rawValue)")
        XCTAssert( item.hasIcon == true, "itemId \(item.hasIcon) != expected true")
		
        jobs += 1
        item.loadIconImageData { data, extention in
            jobs -= 1
            XCTAssert(data!.count > 0, "data.count (\(data!.count)) <= 0 item:\(item.itemId!)")
        }
		
        while jobs > 0 {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
	
    func testEffectAssetInfoBulletin() {
        let itemId = "com.nexstreaming.kinemaster.builtin.title.bulletin"
        let item = lib.item(forId: itemId) as! NXEEffectAssetItem
        XCTAssert(item.userFields.count == 3)
        XCTAssert(item.itemId == itemId)
        XCTAssert(item.category == NXEAssetItemCategory.effect)
    }
    
    func testTemplateForId()
    {
        let templateId = "com.nexstreaming.kmsdk.design.nexeditor.template.v.2.0.1v1.happydays"
        
        let template = lib.item(forId: templateId)!
        XCTAssert(template.label == "v.2.0_1v1_happydays")
    }
    
    func testItemForId() {
        let params = [
            (id:"com.nexstreaming.kmsdk.design.nexeditor.template.audio.happydays.bgm",
             label:"happydays_bgm"),
            (id:"com.nexstreaming.kmsdk.design.nexeditor.effect.happydays.1v1.accent",
             label: "com.nexstreaming.kmsdk.design.nexeditor.effect.happydays.1v1.accent"),
            (id:"com.nexstreaming.kmsdk.design.nexeditor.template.lut.disney",
             label: "disney"),
            (id:"com.nexstreaming.kmsdk.design.nexeditor.transition.happydays.1v1.transition",
             label: "com.nexstreaming.kmsdk.design.nexeditor.transition.happydays.1v1.transition"),
        ]
        
        for param in params {
            let item = lib.item(forId: param.id)!
            XCTAssert(item.label == param.label)
            XCTAssert((item.raw.info.value(for: .hidden) as! NSNumber) .boolValue == false)
        }
    }
    
    func testFileInfoResource() {
        let id = "com.nexstreaming.kinemaster.builtin.transition.zoomflip"
        let item = lib.item(forId: id)!
        let data = AssetResourceLoader.loadFileContents(atPath: item.fileInfoResourcePath)!
        XCTAssert(data.count == 860)
    }
    
    func testFileResource() {
        let id = "com.nexstreaming.kmsdk.design.nexeditor.effect.happydays.1v1.accent"
        let item = lib.item(forId: id)!
        
        let mintPath = item.raw.path(forResourceFile: "stick_mint.png")
        let data = AssetResourceLoader.loadFileContents(atPath: mintPath)!
        XCTAssert(data.count == 70309)
    }
    
    func testHiddenAssetItem() {
        let id = "com.nexstreaming.kinemaster.builtin2.accent.pink"
        let lib = NXEAssetLibrary()
        let item1 = lib.item(forId: id)
        
        XCTAssert(item1 == nil)
        
        let libinternal = AssetLibrary.library()!
        let item2 = libinternal.item(forId: id)
        XCTAssert(item2 != nil)
        XCTAssert(item2!.raw.hidden == true)
    }
    
    func testFontItem() {
        let itemId = "AssetBundles/SampleAssets.bundle/packages.Alegreya SC.item.Alegreya SC Black"
        let libinternal = AssetLibrary.library()!
        let item = libinternal.item(forId: itemId)
        
        XCTAssert(item != nil)
        XCTAssert(item!.raw.type == .font)
        XCTAssert(item!.label == "Alegreya SC Black")
        XCTAssert(item!.raw.labels["en"] == item!.label)
        
        // path to the font resource file
        let path = item!.fileInfoResourcePath!
        XCTAssert(FileManager.default.fileExists(atPath: path), "Font '\(itemId)' resource file not found at path: \(path)")
        // Contents of the font resource file
        let data = item!.fileInfoResourceData
        XCTAssert(data != nil)
        XCTAssert(data!.count == 71140, "\(data!.count) != 71140")
    }
    
    func testEffectFromCategoryHiddenOrAll() {
        let lib = NXEAssetLibrary()
        let hiddenItems = lib.items(inCategory: NXEAssetItemCategory.effect)
        
        XCTAssert(hiddenItems.count == 59, "hiddemItems.count \(hiddenItems.count)")
        
        let libinternal = AssetLibrary.library()!
        
        let allItems = libinternal.items(inCategory: NXEAssetItemCategory.effect)
        
        XCTAssert(allItems.count == 66, "allItems \(allItems.count)")
    }
    
    func testPerformanceItemForId() {

        self.measure {
            let templateId = "com.nexstreaming.kmsdk.design.nexeditor.template.v.2.0.1v1.happydays"
            
            let lib = NXEAssetLibrary.instance()
            _ = lib.item(forId: templateId)!
        }
    }
    
    func testPerformanceItemInCategory() {
        
        self.measure {
            let lib = NXEAssetLibrary.instance()
            _ = lib.items(inCategory: NXEAssetItemCategory.template)
        }
    }
}

