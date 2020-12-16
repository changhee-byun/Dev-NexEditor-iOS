//
//  EncryptedAssetTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 4/14/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class EncryptedAssetTests: XCTestCase {
    var lib: NXEAssetLibraryProtocol!
    
    override func setUp() {
        super.setUp()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
        lib = AssetLibrary.library()
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testSmoke() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_16v9_polaroid"
        _ = lib.item(forId: itemId)
        // Pass if it didn't crash
    }

    func testLoadEncryptedItem() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_16v9_polaroid"
        let item = lib.item(forId: itemId)
        XCTAssert(item != nil)
    }
}
