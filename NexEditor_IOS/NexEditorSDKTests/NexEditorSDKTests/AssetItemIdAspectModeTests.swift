//
//  AssetItemIdAspectModeTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 4/19/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class AssetItemIdAspectModeTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("OverlayTitleAssets.bundle/packages"))
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        NXEAssetLibrary.removeAllAssetPackageDirectoryURLs()
        super.tearDown()
    }
    
    func testDetectAspectTypes() {
        let params: [(expected: NXEAspectType, text: String?)] = [
            (NXEAspectType.unknown, "com.nexstreaming."),
            (NXEAspectType.unknown, nil),
            (NXEAspectType.unknown, ""),
            (NXEAspectType.unknown, "com.nexstreaming.kinemaster.builtin2.accent.pink"),
            (NXEAspectType.unknown, ".3v3"),
            (NXEAspectType.ratio1v1,"com.nexstreaming.kmsdk.design.nexeditor.template.v.2.0.1v1.happydays"),
            (NXEAspectType.ratio1v1,"com.nexstreaming.kmsdk.design.nexeditor.template.v.2.0.1v1"),
            (NXEAspectType.ratio2v1,"com.nexstreaming.kmsdk.design.nexeditor.template.v.2.0.1v1.happydays.2v1"),
            (NXEAspectType.ratio16v9,"com.nexstreaming.kmsdk.design.nexeditor.template.v.2.0.16v9."),
        ]
        
        for param in params {
            let detected: NXEAspectType
            if let text = param.text {
                detected = text.detectAspectType()
            } else {
                detected = NSString.detectAspectType(from: nil)
            }
            XCTAssert(detected == param.expected, "detected: \(detected) != expected: \(param.expected) from '\(param.text ?? "nil")'")
        }
    }
    
    func testDetectAspectTypesFromAssets() {
        let params: [(expected: NXEAspectType, itemId: String)] = [
            // text effects
            (.ratio1v1, "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.1v1.sub"),
            (.ratio1v2, "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.1v2.sub"),
            (.ratio2v1, "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.2v1.sub"),
            (.ratio9v16, "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.9v16.sub"),
            (.ratio16v9, "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.16v9.sub"),
            // templates
            (.ratio1v1, "com.nexstreaming.kmsdk.design.nexeditor.template.cool.2.0_1v1_cool"),
            (.ratio9v16, "com.nexstreaming.kmsdk.design.nexeditor.template.cool.2.0_9v16_cool"),
            (.ratio16v9, "com.nexstreaming.kmsdk.design.nexeditor.template.cool.2.0_16v9_cool"),
            ]
        
        let lib = AssetLibrary.library()!
        for param in params {
            let item = lib.item(forId: param.itemId)!
            XCTAssert(item.aspectType == param.expected, "aspectType \(item.aspectType) != \(param.expected) for itemId: \(param.itemId)")
        }
    }

}
