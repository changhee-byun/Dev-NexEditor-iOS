//
//  TemplateAssetsTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 4/19/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class TemplateAssetsTests: XCTestCase {
    
    var lib: NXEAssetLibraryProtocol!
    override func setUp() {
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("SampleAssets.bundle/packages"))
        
        lib = AssetLibrary.library()
        super.setUp()
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testTemplatesMetaData() {
        let templates = lib.items(inCategory: NXEAssetItemCategory.template) as! [NXETemplateAssetItem]
        
        var jobs = 0
        for template in templates {
            XCTAssert(template.label.characters.count != 0)
            XCTAssert(template.name.characters.count != 0)
            XCTAssert(template.desc.characters.count != 0)
            
            XCTAssert(template.aspectType != .unknown, "aspectType of template \(template.itemId) should be known")
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
    
    func testTemplateIcon() {
        let template = lib.item(forId: "com.nexstreaming.kmsdk.design.nexeditor.template.softwipelut.2.0_16v9_softwipelut")!
        
        var jobs = 0
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
        while jobs > 0 {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
    }
    
    func testResourceData() {
        let item = lib.item(forId: "com.nexstreaming.kmsdk.design.nexeditor.effect.cool.1v1.3screen")!
        let data = item.data(forResourceFile: "line.png")!
        XCTAssert(data.count == 43862, "data.count \(data.count) != 43862")
    }
    
}
