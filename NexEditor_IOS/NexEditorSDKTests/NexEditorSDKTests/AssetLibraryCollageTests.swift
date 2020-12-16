//
//  AssetLibraryAssetGroupTests.swift
//  NexEditorSDKTests
//
//  Created by Simon Kim on 12/15/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class AssetLibraryCollageTests: XCTestCase {
    
    var lib: NXEAssetLibrary!
    override func setUp() {
        InternalAssetLibrary().removeAllAssetPackageURLs()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("CollageAssets.bundle/packages"))
        lib = NXEAssetLibrary.instance()
        
        continueAfterFailure = false
        super.setUp()
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testGroupPropertyScan() {
        
        let groups = lib.groups(inCategory: NXEAssetItemCategory.collage)
        XCTAssert(groups.count > 0)
        let group = groups[0]
        XCTAssert(group.localizedName != nil)
        XCTAssert(group.localizedName.count > 0)
        let hasIcon = group.hasIcon
        group.loadIconImageData { (data, ext) in
            XCTAssert(hasIcon == (data != nil), "hasIcon \(hasIcon) != (image data != nil)")
            if data == nil {
                XCTAssert(ext == nil)
            } else {
                XCTAssert(ext != nil && ext!.count > 0)
            }
        }
        XCTAssert(group.category == NXEAssetItemCategory.collage)
        XCTAssert(group.items.count > 0)
        let item = group.items[0]
        XCTAssert(item.category == NXEAssetItemCategory.collage)
        XCTAssert(group.hasIcon == item.hasIcon)
    }
    
    func testCollageItemInfoPropertyScan() {
        let item = lib.item(forId: "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_16v9_2.08_doodlechalkboard") as! NXECollageAssetItem
        XCTAssert(item.info.slots.count == 8, "Slot count \(item.info.slots.count) != 8")
        XCTAssert(item.info.titles.count == 0, "Title count \(item.info.titles.count) != 0")
        XCTAssert(item.info.durationMs == 8000, "Duration \(item.info.durationMs) != 8000")
        XCTAssert(item.info.aspectRatio.width == 16, "aspectRatio.width \(item.info.aspectRatio.width) != 16")
        XCTAssert(item.info.aspectRatio.height == 9, "aspectRatio.width \(item.info.aspectRatio.height) != 9")
    }
}
