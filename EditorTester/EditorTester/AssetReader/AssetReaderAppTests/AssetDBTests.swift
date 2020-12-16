/**
 * File Name   : AssetDBTests.swift
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

class AssetDBTests: XCTestCase {
    
    var assetDB: AssetDBProtocol!
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
        assetDB = AssetDB.withSourceURLs([URL(string:AssetVolume(type: .documents, subpath: "SampleAssets.bundle/packages").assetVolumeURLString)!,
                                                  URL(string:AssetVolume(type: .documents, subpath: "TemplateAssets.bundle/packages").assetVolumeURLString)!])
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        assetDB = nil;
        super.tearDown()
    }
    
    func testInitialLoadingTime() {
        self.measure {
            _ = AssetDB.withSourceURLs([URL(string:AssetVolume(type: .documents, subpath: "SampleAssets.bundle/packages").assetVolumeURLString)!,
                                     URL(string:AssetVolume(type: .documents, subpath: "TemplateAssets.bundle/packages").assetVolumeURLString)!]).packages
        }
    }
    
    func testAudioItemInfoPathQuery() {
        let audioId = "com.nexstreaming.kmsdk.design.nexeditor.template.audio.happydays.bgm"
        let bgmItem = assetDB.item(forId: audioId)!
        let infoPath = (bgmItem.path as NSString).appendingPathComponent("_info.json")
        let data = NSData(contentsOfFile: infoPath)! as Data
        
        let expected = 219
        XCTAssert(data.count == expected,
                  "data length (\(data.count)) != expected(\(expected)) for audio asset item (\(audioId))")
        
    }
    
    func testItemNotFound() {
        XCTAssert(assetDB.item(forId: "you.can.never.find.this.item") == nil)
    }
    
    func testTransitionItemResourceQuery() {
        let itemId = "com.nexstreaming.kinemaster.builtin.transition.checker"
        let item = assetDB.item(forId: itemId)!
        
        XCTAssert(item.info.value(for: .type) as! String == "nex.kedl")
        XCTAssert(item.info.value(for: .hidden) as! Bool == false)
        XCTAssert(item.info.value(for: .sampleText) == nil)
        let labels = item.info.value(for: .label) as! [String: String]
        XCTAssert(labels.keys.count > 0)
    }
    
    func testTransitionItemResourceFilePathQuery() {
        let itemId = "com.nexstreaming.kinemaster.builtin.transition.checker"
        let item = assetDB.item(forId: itemId)!
        
        let filePath = item.path( for: .filename)!
        var data = NSData(contentsOfFile: filePath)!
        var expected = 0
        XCTAssert(data.length > expected, "size \(data.length) <= \(expected) for \(filePath) ")
        
        let iconPath = item.path( for: .icon)!
        data = NSData(contentsOfFile: iconPath)!
        expected = 0
        XCTAssert(data.length > expected, "size \(data.length) <= \(expected) for \(iconPath) ")
    }
    
    func testItemQueryImplicitId() {
        let itemId = "SampleAssets/packages.Alegreya SC.item.Alegreya SC Black"
        let item = assetDB.item(forId: itemId)
        
        XCTAssert(item != nil)
        XCTAssert(item!.type == .font)
        XCTAssert(item!.label == "Alegreya SC Black")
        XCTAssert(item!.labels["en"] == item!.label)
    }
    
    func testItemResourceQueryNotFound() {
        let itemId = "SampleAssets/packages.Alegreya SC.item.Alegreya SC Black"
        let item = assetDB.item(forId: itemId)!
        
        XCTAssert(item.info.value(for: .icon) == nil)
        XCTAssert(item.path(for: .icon) == nil)
    }
    
    func testItemTypes() {
        
        let params: [(id: String, expectedType: AssetItemType)] = [
            (id: "com.nexstreaming.kmsdk.design.nexeditor.template.lut.disney", expectedType: .lut),
            (id: "SampleAssets/packages.Alegreya SC.item.Alegreya SC Black", expectedType: .font),
            (id: "SampleAssets/packages.audio_Alright.item.Alright", expectedType: .audio),
            (id: "com.nexstreaming.csd.blurall", expectedType: .renderitem),
            (id: "SampleAssets/packages.Basic_Shapes.item.1", expectedType: .overlay),
            (id: "com.nexstreaming.kinemaster.builtin.title.bulletin", expectedType: .kedl),
            (id: "com.nexstreaming.kmsdk.design.nexeditor.template.v.2.0.16v9.happydays", expectedType: .template),
            ];
        
        for param in params {
            let item = assetDB.item(forId: param.id)!
            XCTAssert(item.type == param.expectedType, "expected(\(param.expectedType) != item.type\(item.type) for \(param.id)")
        }
    }
    
    func testPackageCount() {
        let expected = 50
        
        XCTAssert(assetDB.packages.count == expected, "packages.count \(assetDB.packages.count) != \(expected)")
    }
    
    func testCategoriesCount() {
        let expected = 10
        
        let count = assetDB.categories.count
        XCTAssert(count == expected, "categories.count(\(count)) != expected(\(expected))")
    }
    
    func testItemCountPerCategory() {
        let params: [(category: String, expected: Int)] = [
            (category: kAssetItemCategoryTransition,     expected: 369),
            (category: kAssetItemCategoryAudio,          expected: 28),
            (category: kAssetItemCategoryBackground,     expected: 0),
            (category: kAssetItemCategoryEffect,         expected: 747),
            (category: kAssetItemCategoryFilter,         expected: 47),
            (category: kAssetItemCategoryAudioFilter,    expected: 0),
            (category: kAssetItemCategoryFont,           expected: 3),
            (category: kAssetItemCategoryOverlay,        expected: 85),
            (category: kAssetItemCategoryTemplate,       expected: 104),
            ];
        
        for param in params {
            
            let items = assetDB.items(forCategory: param.category)!
            XCTAssert(items.count == param.expected,
                      "items.count(\(items.count)) != expected(\(param.expected)) for category(\(param.category))")
        }
    }
    
}
