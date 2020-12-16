//
//  LayerTests.swift
//  NexEditorFramework
//
//  Created by MJ.KONG-MAC on 24/07/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class LayerTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testLayerAnimation() {
        let engine = NXEEngine.instance()!
        engine.TTPrepare()
        
        let clips: [NXEClip] = [NXEClip.newSolidClip("red")]
        let project = NXEProject()
        
        project.visualClips = clips;
        engine.setPreviewWith(preview.layer, previewSize:preview.bounds.size)
        engine.setProject(project)

        let image = UIImage(contentsOfFile: clipsourceUrl.appendingPathComponent("picture/Desert.jpg").path)
        let imagelayer:NXEImageLayer = NXEImageLayer.init(image: image!, point: CGPoint(x: 0, y: 0))
        
        // 1. without KMLayer, setXXX() vs. property values
        imagelayer.setInAnimationType(.ANIMATION_IN_POP, duration: 4000)
        
        XCTAssert(imagelayer.inAnimation == .ANIMATION_IN_POP)
        XCTAssert(imagelayer.inAnimationDuration == 4000)
        
        // 2. with KMLayer (Set NXELayer properties -> Read KMLayer properties)
        project.layers = [imagelayer]
        imagelayer.inAnimation = .ANIMATION_IN_FADE
        imagelayer.inAnimationDuration = 5000
        
        var kmlayer = LayerManager.sharedInstance()!.getLayer(imagelayer.layerId)
        XCTAssert(kmlayer != nil)
        XCTAssert(kmlayer!.layerId == imagelayer.layerId)
        XCTAssert(imagelayer.inAnimation == .ANIMATION_IN_FADE)
        XCTAssert(imagelayer.inAnimationDuration == 5000)
        
        // 3. Remove KMLayer, Read NXELayer properties == previously set property values
        project.layers = []
        
        kmlayer = LayerManager.sharedInstance()!.getLayer(imagelayer.layerId)
        XCTAssert(kmlayer == nil)
        
        XCTAssert(imagelayer.inAnimation == .ANIMATION_IN_FADE)
        XCTAssert(imagelayer.inAnimationDuration == 5000)
    }
    
}
