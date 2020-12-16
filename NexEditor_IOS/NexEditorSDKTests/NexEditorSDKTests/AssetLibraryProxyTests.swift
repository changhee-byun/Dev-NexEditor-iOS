/**
 * File Name   : AssetLibraryProxyTests.swift
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

class AssetLibraryProxyTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("SampleAssets.bundle/packages"))
        self.continueAfterFailure = false
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testEffectConfigurable() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.effect.happydays.1v1.accent"
        let item = AssetLibrary.proxy().item(forId: itemId)
        XCTAssert(item != nil, "Item \(itemId) not found. Assets not installed?")
        XCTAssert(item!.conforms(to:ConfigurableAssetItem.self), "\(type(of:item!)) does not conform to \(ConfigurableAssetItem.self)")
    }
    
    func testTransitionConfigurable() {
        let item = AssetLibrary.proxy().item(forId:"com.nexstreaming.kinemaster.builtin.transition.checker")!
        
        XCTAssert(item.conforms(to:ConfigurableAssetItem.self), "\(type(of:item)) does not conform to \(ConfigurableAssetItem.self)")
    }

    func testTransitionAttributes() {
        let item = AssetLibrary.proxy().item(forId:"com.nexstreaming.kinemaster.builtin.transition.checker")!
        
        XCTAssert(item.conforms(to:TransitionAssetItemAttributes.self), "\(type(of:item)) does not conform to \(TransitionAssetItemAttributes.self)")
    }
    
    func testResourceData() {
        let itemId = "com.nexstreaming.kmsdk.design.nexeditor.effect.cool.1v1.3screen"
        let data = AssetLibrary.proxy().loadResourceData(atSubpath: "line.png", forItemId: itemId)
        XCTAssert(data != nil, "Item \(itemId) not found. Assets not installed?")
        XCTAssert(data!.count == 43862, "data.count \(data!.count) != 43862")
    }
}
