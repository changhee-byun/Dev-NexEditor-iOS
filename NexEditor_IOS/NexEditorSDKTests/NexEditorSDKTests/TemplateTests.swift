//
//  TemplateTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 8/1/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class TemplateTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    /// NESI-458
    func testExportVignetteRemoval() {
        
        let url = clipsourceUrl.appendingPathComponent("template/Thriller/01.jpg")
        XCTAssert(FileManager.default.fileExists(atPath: url.path), "NESI-365 Make sure clip sources available under ClipSources/template/ directory to proceed this test case")
        
        let sequence = TemplateSequenceParams(path: "ClipSources/template/Family",
                                              clipNames:["01", "02"],
                                              name: "Family",
                                              ext: "jpg",
                                              assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.family.template_16v9_family"])
        
        let editor = NXEEngine.instance()!
        let lib = NXEAssetLibrary.instance()
        let template = lib.item(forId: sequence.assetItemIds[0])
        do {
            var project: NXETemplateProject? = try NXETemplateProject(templateAssetItem: template as! NXETemplateAssetItem, clips: sequence.clips())
            editor.setProject(project)
            
            // remember the vignette layer and clean up the project
            let layerId = project!.vignetteLayers.imageLayers[0].layerId
            XCTAssert(LayerManager.sharedInstance().getLayer(layerId) != nil, "Vignette Layer should have been added")
            
            project = nil
            editor.setProject(nil)
            
            let layer = LayerManager.sharedInstance().getLayer(layerId)
            XCTAssert(layer == nil, "Vignette Layer should have been removed")
        } catch {
        }
    }
    
    /// NESI-458
    func testVignetteLayersRemoval() {
        autoreleasepool {
            let editor = NXEEngine.instance()!
            
            let path = BundleResource.path(for: .templateVignetteHorizontal)!
            var layer: KMImageLayer? = KMImageLayer(image: UIImage(named: path))!
            var project: NXEProject? = NXEProject()
            project!.vignetteLayers = VignetteLayers(imageLayers: [layer!])
            let layerId = layer!.layerId
            layer = nil
            
            editor.setProject(project!)
            editor.setProject(nil)
            project = nil
            
            let kmlayer = LayerManager.sharedInstance().getLayer(layerId)
            XCTAssert(kmlayer == nil)
        }
    }

    
}
