//
//  ClipZoomPanningTests.swift
//  NexEditorSDKTests
//
//  Created by Simon Kim on 2/13/18.
//  Copyright © 2018 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework
@testable import NexEditorEngine

class ClipZoomPanningTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func test1920x1080() {
        let param = (width: 1920,
                     height: 1080,
                     rect: CGRect(x: 100, y: 100, width: 1000, height: 900),
                     abstract: CGRect(x: 5208.0, y: 9259.0, width: 52083.0, height: 83333.0))
        
        let vclip2 = NXEVisualClip()
        vclip2.width = Int32(param.width)
        vclip2.height = Int32(param.height)
        
        let rect = param.rect
        
        vclip2.cropStartRectInPixels = rect
        
        let startRect = vclip2.startRect
        let expected = param.abstract
        XCTAssert(startRect.origin.x == expected.origin.x, "\(startRect.origin.x)")
        XCTAssert(startRect.origin.y == expected.origin.y, "\(startRect.origin.y)")
        XCTAssert(startRect.size.width == expected.width, "\(startRect.size.width)")
        XCTAssert(startRect.size.height == expected.height, "\(startRect.size.height)")
    }

    func testVideo() {
        let url = clipsourceUrl.appendingPathComponent("video/canal.mp4")
        let clipSource = NXEClipSource(path: url.path)
        let fullRect = CGRect(x: 0.0, y: 0.0, width: 100000.0, height: 100000.0)
        
        _ = NXEEngine.instance()!
        NXEEngine.aspectRatio = NXESizeInt(width: 16, height: 9)
        do {
            let clip = try NXEClip(source: clipSource)
            
            print("\(clip.size.width) x \(clip.size.height)")
            clip.cropMode = .fill
            XCTAssert(clip.videoInfo.startRect == fullRect, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == fullRect, "\(clip.videoInfo.endRect)")
            clip.cropMode = .fit
            XCTAssert(clip.videoInfo.startRect == fullRect, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == fullRect, "\(clip.videoInfo.endRect)")
        } catch let e {
            XCTFail(e.localizedDescription)
        }
    }
    
    func testImage() {
        let url = clipsourceUrl.appendingPathComponent("picture/Jellyfish.jpg")
        let clipSource = NXEClipSource(path: url.path)
        let fillRect = CGRect(x: 0.0, y: 12500.0, width: 100000.0, height: 75000.0)
        let fitRect = CGRect(x: -16650.0, y: 0.0, width: 133300.0, height: 100000.0)
        
        _ = NXEEngine.instance()!
        NXEEngine.aspectRatio = NXESizeInt(width: 16, height: 9)
        do {
            let clip = try NXEClip(source: clipSource)
            print("\(clip.size.width) x \(clip.size.height)")
            clip.cropMode = .fill
            XCTAssert(clip.videoInfo.startRect == fillRect, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == fillRect, "\(clip.videoInfo.endRect)")
            clip.cropMode = .fit
            XCTAssert(clip.videoInfo.startRect == fitRect, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == fitRect, "\(clip.videoInfo.endRect)")
        } catch let e {
            XCTFail(e.localizedDescription)
        }
    }
    
    func testImageFitCropStartEndRect() {
        let url = clipsourceUrl.appendingPathComponent("picture/Jellyfish.jpg")
        let clipSource = NXEClipSource(path: url.path)
        let fitRect = CGRect(x: -16650.0, y: 0.0, width: 133300.0, height: 100000.0)
        let size = CGSize(width: 1024, height: 768)
        
        _ = NXEEngine.instance()!
        NXEEngine.aspectRatio = NXESizeInt(width: 16, height: 9)
        
        do {
            let clip = try NXEClip(source: clipSource)
            XCTAssert(clip.videoInfo.orientationRegardedSize.width == size.width, "\(clip.videoInfo.orientationRegardedSize.width)")
            XCTAssert(clip.videoInfo.orientationRegardedSize.height == size.height, "\(clip.videoInfo.orientationRegardedSize.height)")
            
            clip.cropMode = .fit
            XCTAssert(clip.videoInfo.startRect == fitRect, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == fitRect, "\(clip.videoInfo.endRect)")
        } catch let e {
            XCTFail(e.localizedDescription)
        }
    }
    
    func testX() {
        let url = clipsourceUrl.appendingPathComponent("picture/Jellyfish.jpg")
        let clipSource = NXEClipSource(path: url.path)
        let fillRect = CGRect(x: 0.0, y: 12500.0, width: 100000.0, height: 75000.0)
        let fitRect = CGRect(x: -16650.0, y: 0.0, width: 133300.0, height: 100000.0)
        let customFrom = CGRect(x: 9765.0 , y: 13020.0, width: 80468.0, height: 73958.0)
        let customTo = CGRect(x: 976.0, y: 1302.0, width: 98046.0, height: 97395.0)

        _ = NXEEngine.instance()!
        NXEEngine.aspectRatio = NXESizeInt(width: 16, height: 9)
        do {
            let clip = try NXEClip(source: clipSource)

            clip.cropMode = .fit
            XCTAssert(clip.videoInfo.startRect == fitRect, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == fitRect, "\(clip.videoInfo.endRect)")

            let width = Int(clip.videoInfo.width)
            let height = Int(clip.videoInfo.height)
            let from = CGRect(x: 100, y: 100, width: width - 200, height: height - 200)
            let to = CGRect(x: 10, y: 10, width: width - 20, height: height - 20)
            let transform = NXEClipTransform(sourceSize: clip.size, destinationSize: NXEEngine.instance().logicalRenderSize)!
            clip.cropMode = .custom
            clip.transformRamp.start = transform.zoom(to: from)
            clip.transformRamp.end = transform.zoom(to: to)
            XCTAssert(clip.cropMode == .custom, "\(clip.cropMode.rawValue)")
            XCTAssert(clip.videoInfo.startRect == customFrom, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == customTo, "\(clip.videoInfo.endRect)")

            clip.cropMode = .fill
            XCTAssert(clip.videoInfo.startRect == fillRect, "\(clip.videoInfo.startRect)")
            XCTAssert(clip.videoInfo.endRect == fillRect, "\(clip.videoInfo.endRect)")
        } catch let e {
            XCTFail(e.localizedDescription)
        }
    }
    
    func testCropAtAspectTypes() {
        let debugs: [CropMode: String] = [.fit: "AspectTypeFit", .fill: "AspectTypeFill", .panRandom: "AspectTypePanRandom", .custom:"AspectTypeCustom"]
        let types: [(type:NXEAspectType, h:Int32, v:Int32)] = [ (.ratio16v9, 16, 9), (.ratio9v16, 9, 16), (.ratio1v1, 1, 1), (.custom, 1, 1)]
        let sequence = ClipSequence(path: "ClipSources/video", clipNames: ["canal.mp4"], name: "Single image clip preview")
        let project = NXEProject()
        let clips = sequence.clips()
        let editor = NXEEngine.instance()!
        for type in types {
            editor.aspectType = type.type
            project.visualClips = clips
            project.update()
            let clip: NXEClip = project.visualClips.first!
            XCTAssert(clip.cropMode == .fit, "\(debugs[clip.cropMode]!)")
            editor.setProject(project)
        }
    }
}
