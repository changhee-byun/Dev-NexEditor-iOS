/**
 * File Name   : AssetLibraryTextEffectTests.swift
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
/**
 * - items(inCategory:) -> [NXETextEffectAssetItem]
 * - fileInfoResourceData -> JSON
 * - image_res
 */
class AssetLibraryTextEffectTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("OverlayTitleAssets.bundle/packages"))
        
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testCategoryTextEffect() {
        let lib = NXEAssetLibrary.instance()
        var asyncDone = false
        lib.items(inCategory: NXEAssetItemCategory.textEffect) { (items) in
            asyncDone = true
            let expected = 50
            XCTAssert(items.count == expected, "Items count \(items.count) != \(expected)" )
        }
        
        while !asyncDone {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.1))
        }
    }
    
    func testTextEffectSingleItem() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.16v9.sub"
        let lib = NXEAssetLibrary.instance()

        let awardsCeremony = lib.item(forId: itemId)
        XCTAssert(awardsCeremony != nil, "Text effect not found for item Id \(itemId)")
        let sourceData = awardsCeremony!.fileInfoResourceData
        let expected = 7832
        XCTAssert(sourceData!.count == expected, "Text effect source data bytes \(sourceData!.count) != \(expected)")
    }
    
    func testTextEffectCategoryIsTextEffect() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.16v9.sub"
        let lib = NXEAssetLibrary.instance()
        
        let textEffect = lib.item(forId: itemId)!
        XCTAssert(textEffect.category == NXEAssetItemCategory.textEffect, "Text effect category \(textEffect.category) != \(NXEAssetItemCategory.textEffect)")
    }
    
    func testOverlayItemLine() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.cornerlines.res.line"
        
        let sdklib = AssetLibrary.library()!
        let item = sdklib.item(forId: itemId)
        
        let data = item!.fileInfoResourceData
        XCTAssert(data != nil)
        let expected = 120135
        XCTAssert(data!.count == expected, "file data bytes \(data!.count) != \(expected)")
    }
    
    func testOverlayItemHiddenNotFound() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.cornerlines.res.line"
        let apilib = NXEAssetLibrary.instance()
        
        let notfounditem = apilib.item(forId: itemId)
        XCTAssert(notfounditem == nil)
        
    }
    
    func testName() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.16v9.sub"
        let expected = "Awards Ceremony"
        let lib = NXEAssetLibrary.instance()
        
        let item = lib.item(forId: itemId)!
        XCTAssert(item.name == expected, "name \(item.name) != \(expected) for item \(itemId)")
    }
    
    func testDesc() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.16v9.sub"
        let expected = "Version : D005 - Text Effect Created by Design Team, NEXSTREAMING"
        let lib = NXEAssetLibrary.instance()
        
        let item = lib.item(forId: itemId)!
        XCTAssert(item.desc == expected, "desc \(item.desc) != \(expected) for item \(itemId)")
    }
    
    func testTextEffectsValidation() {
        
        let apilib = NXEAssetLibrary.instance()
        // for hidden item retrieval
        let sdklib = AssetLibrary.library()!
        
        let params = [
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.cornerlines",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.floatin",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.lightbulbs",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.parallellines",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.rhombus",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.underline",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomin",
            "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomout"
            
        ]

        var ids: [String] = []
        for param in params {
            ids.append(param.appending(".1v1.sub"))
            ids.append(param.appending(".1v2.sub"))
            ids.append(param.appending(".2v1.sub"))
            ids.append(param.appending(".9v16.sub"))
            ids.append(param.appending(".16v9.sub"))
        }
        
        for textEffectId in ids {
            let textEffect = apilib.item(forId: textEffectId)
            XCTAssert(textEffect != nil, "Not found text effect \(textEffectId)")
            
            let sourcePath = textEffect!.fileInfoResourcePath
            XCTAssert(sourcePath != nil, "Source path unknown for text effect \(textEffectId)")
            
            // FIXME: loadJSONObject(atPath:) shouldn't be used for encrypted files. Load through textEffect!.fileInfoResourceData
            if let source = AssetResourceLoader.loadJSONObject(atPath: sourcePath!) as? [String: Any] {
                let section = source["overlay"] as? [[String: Any]]
                XCTAssert(section != nil, "Overlay section not found for text effect \(textEffectId)")
                for element in section! {
                    if let overlayItemId = element["image_res"] as? String {
                        let overlay = sdklib.item(forId: overlayItemId)
                        XCTAssert(overlay != nil, "Overlay item \(overlayItemId) not found for text effect \(textEffectId)")
                        
                        
                        if overlay == nil {
                            continue
                        }
                        XCTAssert(overlay!.raw.packageId == textEffect!.raw.packageId,
                                  "Overlay item \(overlayItemId) has different packageId \(overlay!.raw.packageId) != \(textEffect!.raw.packageId) from text effect \(textEffectId)")
                    }
                }
            } else {
                let filename = (sourcePath! as NSString).lastPathComponent
                XCTFail("Source format \(filename) invalid for text effect \(textEffectId)")
            }
        }
    }
}
