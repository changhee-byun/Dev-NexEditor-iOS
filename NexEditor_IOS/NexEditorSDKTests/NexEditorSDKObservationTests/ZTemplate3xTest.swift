//
//  ZTemplate3xTest.swift
//  NexEditorSDKObservationTests
//
//  Created by MJ.KONG-MAC on 14/02/2018.
//  Copyright © 2018 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class ZTemplate3xTest: XCTestCase {
    let template30id = "com.nexstreaming.kmsdk.design.nexeditor.template.tm30mtv.template_1v1_tm30"

    override func setUp() {
        super.setUp()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
        NexLogger.setLogLevel(0, for: NexLogCategory.info)
        self.continueAfterFailure = false
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func project(clipCount:Int) -> NXETemplateProject {
        let names: [String] = ["picture/Tower.jpg", "picture/Desert.jpg", "picture/Church.jpg", "picture/Koala.jpg", "picture/Cupcakes.jpg", "picture/Tulips.jpg", "picture/Schoolbus.jpg", "picture/Lighthouse.jpg", "picture/Penguins.jpg", "picture/Chrysanthemum.jpg", "picture/Hydrangeas.jpg", "picture/Lighthouse.jpg"]
        var clipList: [NXEClip] = []
        for index in 0...clipCount-1 {
            let url = clipsourceUrl.appendingPathComponent(names[index])
            let clip = try? NXEClip(source: NXEClipSource(path: url.path)!)
            XCTAssert(clip != nil, "Clip Sources not installed? \(names[index]) not found")
            clipList.append(clip!)
        }
        let templateAssetItem = NXEAssetLibrary.instance().item(forId: template30id)
        XCTAssert(templateAssetItem != nil, "templateAssetItem item: \(String(describing:template30id)) not found. Assets not installed?")
        return try! NXETemplateProject(templateAssetItem: templateAssetItem as! NXETemplateAssetItem, clips: clipList)
    }
    
    func setPreview(editor: NXEEngine, project: NXETemplateProject) {
        let templateAssetItem = NXEAssetLibrary.instance().item(forId: template30id)
        let aspectType = templateAssetItem!.aspectType
        
        editor.setProject(project)
        editor.aspectType = aspectType
        editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
    }
    
    func testPreview() {
        self.label.text = "Template 3.x - preview"
        
        let project = self.project(clipCount: 12)
        let editor = NXEEngine.instance()!
        self.setPreview(editor: editor, project: project)
        editor.TTPlay(for: TimeInterval(editor.project.getTotalTime()/1000))
    }
    
}
