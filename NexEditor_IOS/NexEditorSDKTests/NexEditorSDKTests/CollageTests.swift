//
//  CollageTests.swift
//  NexEditorFramework
//
//  Created by MJ.KONG-MAC on 22/11/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

extension UIView {
    func drawLine(startPoint: CGPoint, endPoint: CGPoint) {
        
        let line = CAShapeLayer()
        let linePath = UIBezierPath()
        linePath.move(to: startPoint)
        linePath.addLine(to: endPoint)
        line.path = linePath.cgPath
        line.strokeColor = UIColor.red.cgColor
        line.lineWidth = 1
        line.lineJoin = kCALineJoinRound
        self.layer.addSublayer(line)
    }
}

class CollageTests: XCTestCase {
        
    override func setUp() {
        super.setUp()
        self.continueAfterFailure = false

        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("CollageAssets.bundle/packages"))
//        NexLogger.setLogLevel(0, for: NexLogCategory.info)
//        NexLogger.setLogLevel(0, for: NexLogCategory.err)
    }
    
    override func tearDown() {
        super.tearDown()
    }
    
    func configureSlots(project: NXECollageProject) {
        let names: [String] = ["picture/Tower.jpg", "picture/Desert.jpg", "picture/Church.jpg", "picture/Koala.jpg",
                               "picture/Cupcakes.jpg", "picture/Tulips.jpg", "picture/Schoolbus.jpg", "picture/Lighthouse.jpg", "picture/Penguins.jpg"]
        let slotcount = project.slots.count
        for index in 0...slotcount-1 {
            let url = clipsourceUrl.appendingPathComponent(names[index])
            let clip = try? NXEClip(source: NXEClipSource(path: url.path)!)
            project.slots[index].clip = clip
        }
        project.update()
    }
    
    func setPreview(editor: NXEEngine, project: NXECollageProject) {
        editor.setProject(project)
        let width = project.info.aspectRatio.width
        let height = project.info.aspectRatio.height
        editor.setAspectType(NXEAspectType.custom, withRatio: NXESizeIntMake(width, height))
        editor.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
    }
    
    func createCollageProject(dynamic: Bool, index: Int) throws -> NXECollageProject {
        let dynamicCollages = ["com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_1_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_2_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_3_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_4_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_5_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_6_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_7_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_8_baby album",
                               "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_9_baby album"]
        let staticCollages = ["com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_4v3_2.01_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_4v3_2.02_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_4v3_2.03_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_4v3_2.04_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_4v3_2.05_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_4v3_2.06_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_4v3_2.07_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_16v9_2.08_doodlechalkboard",
                              "com.nexstreaming.kmsdk.thenew.nexeditor.collage.doodle chalkboard.v1.0_2v1_2.09_doodlechalkboard"]
        var collageID: String! = staticCollages[index]
        if dynamic {
            collageID = dynamicCollages[index]
        }
        
        let assetItem = NXEAssetLibrary.instance().item(forId: collageID)!
        let project = try NXECollageProject(collageAssetItem: assetItem)
        XCTAssert(project.slots.count > 0, "No slots are found. Collage loading failed?")
        if dynamic {
            XCTAssert(project.info.type == .dynamic, "Type is not dynamic collage.")
        } else {
            XCTAssert(project.info.type == .static, "Type is not static collage.")
        }

        return project
    }
    
    func testCollageSlotVideo() {
        self.label.text = "Slot - Video Clip Play"

        let project = try? self.createCollageProject(dynamic: true, index: 0)
        XCTAssert(project != nil, "Failed creating project")
        
        let slotConfig: NXECollageSlotConfiguration = project!.slots[0]
        let path = clipsourceUrl.appendingPathComponent("video/canal.mp4").path
        let clip = try? NXEClip(source: NXEClipSource(path: path))
        slotConfig.clip = clip
        project!.update()
        
        let editor = NXEEngine.instance()!
        self.setPreview(editor: editor, project: project!)
        
        editor.TTPlay(for: TimeInterval(editor.project.getTotalTime()/1000))
    }
    
    func configureCollage(editor: NXEEngine, project: NXECollageProject) {
        self.configureSlots(project: project)
        self.setPreview(editor: editor, project: project)
        //
        editor.TTSeek(project.info.editTimeMs)
    }

    func testConfigureCollage() {
        self.label.text = "Slot - Image Clip Play"

        let editor = NXEEngine.instance()!
        let project = try? self.createCollageProject(dynamic: false, index: 0)
        XCTAssert(project != nil, "Failed creating project")

        self.configureCollage(editor: editor, project: project!)
        editor.TTPlay(for: TimeInterval(editor.project.getTotalTime()/1000))
    }

    func testDrawBorderOfSlot() {
        self.label.text = "Slot Border Drawing"

        let project = try? self.createCollageProject(dynamic: true, index: 0)
        XCTAssert(project != nil, "Failed creating project")

        let editor = NXEEngine.instance()!
        self.configureCollage(editor: editor, project: project!)
        
        for slot: CollageSlotInfo in project!.info.slots {
            let rect = project!.info.convert(from: slot.rect, to: self.preview)
            let slotBorder = UIView(frame: rect)
            slotBorder.layer.borderColor = UIColor.yellow.cgColor
            slotBorder.layer.borderWidth = 3
            self.preview.addSubview(slotBorder)
            
            let points = project!.info.convert(fromPositions: slot.position, to: self.preview)
            for var index in 0...points!.count-1 {
                let startpoint: CGPoint = points![index].cgPointValue
                index += 1
                if index > points!.count-1 {
                    index = 0
                }
                let endpoint: CGPoint = points![index].cgPointValue
                self.preview.drawLine(startPoint: startpoint, endPoint: endpoint)
            }
        }
        for slot: CollageTitleInfo in project!.info.titles {
            let rect = project!.info.convert(from: slot.rect, to: self.preview)
            let slotBorder = UIView(frame: rect)
            slotBorder.layer.borderColor = UIColor.yellow.cgColor
            slotBorder.layer.borderWidth = 3
            self.preview.addSubview(slotBorder)
            
            let points = project!.info.convert(fromPositions: slot.position, to: self.preview)
            for var index in 0...points!.count-1 {
                let startpoint: CGPoint = points![index].cgPointValue
                index += 1
                if index > points!.count-1 {
                    index = 0
                }
                let endpoint: CGPoint = points![index].cgPointValue
                self.preview.drawLine(startPoint: startpoint, endPoint: endpoint)
            }
        }
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 2.0))
        for subUIView in self.preview.subviews as [UIView] {
            subUIView.removeFromSuperview()
        }
        if let subLayers = self.preview.layer.sublayers {
            for subLayer in subLayers {
                subLayer.removeFromSuperlayer()
            }
        }
    }
    
    func testUpdateRotate() {
        self.label.text = "Slot Rotate"
        
        let project = try? self.createCollageProject(dynamic: false, index: 0)
        XCTAssert(project != nil, "Failed creating project")
        
        let editor = NXEEngine.instance()!
        self.configureCollage(editor: editor, project: project!)
        editor.updatePreview()
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.0))
        
        project!.slots[0].flip = .horizontal
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.0))
        project!.slots[0].flip = .vertical
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.0))
        project!.slots[0].flip = .horizontal
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.0))
        project!.slots[0].flip = .vertical
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.0))
    }
    
    func testChangeClip() {
        self.label.text = "Slot Clip Change"
        
        let project = try? self.createCollageProject(dynamic: false, index: 0)
        XCTAssert(project != nil, "Failed creating project")
        
        let editor = NXEEngine.instance()!
        self.configureCollage(editor: editor, project: project!)
        editor.updatePreview()
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.5))
        
        project!.beginUpdates()
        let url = clipsourceUrl.appendingPathComponent("picture/Penguins.jpg")
        let clip = try? NXEClip(source: NXEClipSource(path: url.path)!)
        project!.slots[0].clip = clip
        project!.slots[0].lutID = "paris"
        project!.endUpdates()
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1.5))
    }
    
    func testScaleAngle() {
        self.label.text = "Slot Scale/Angle simulate"
        
        let project = try? self.createCollageProject(dynamic: false, index: 0)
        let editor = NXEEngine.instance()!
        self.configureCollage(editor: editor, project: project!)
        editor.updatePreview()
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        // angle
        project!.slots[0].scale = 1.4
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.1))
        var angle: Int32 = 0
        repeat {
            project!.slots[0].angle = angle
            angle = angle + 1
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.1))
        } while(angle < 360);
        // scale
        project!.slots[0].angle = 210
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.1))
        var scale: CGFloat = 1.0
        repeat {
            project!.slots[0].scale = scale
            scale = scale + 0.05
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.1))
        } while (scale < 2.0)
        repeat {
            project!.slots[0].scale = scale
            scale = scale - 0.05
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.1))
        } while (scale > 1.0)
    }
    
    func moveToLeft(project:NXECollageProject?, slotIndex:Int, limit:CGFloat, startPosition:CGPoint) {
        var x = startPosition.x, y = startPosition.y
        repeat {
            project!.slots[slotIndex].position = CGPoint(x: x, y: y)
            NSLog("moveToLeft: input = (%2.3f, %2.3f), output =  (%2.3f, %2.3f)",
                  x, y, project!.slots[slotIndex].position.x, project!.slots[slotIndex].position.y)
            x = x - 0.01
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05))
        } while (x >= limit)
    }
    
    func moveToRight(project:NXECollageProject?, slotIndex:Int, limit:CGFloat, startPosition:CGPoint) {
        var x = startPosition.x, y = startPosition.y
        repeat {
            project!.slots[slotIndex].position = CGPoint(x: x, y: y)
            NSLog("moveToRight: input = (%2.3f, %2.3f), output =  (%2.3f, %2.3f)",
                  x, y, project!.slots[slotIndex].position.x, project!.slots[slotIndex].position.y)
            x = x + 0.01
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05))
        } while (x <= limit)
    }
    
    func moveToUp(project:NXECollageProject?, slotIndex:Int, limit:CGFloat, startPosition:CGPoint) {
        var x = startPosition.x, y = startPosition.y
        repeat {
            project!.slots[slotIndex].position = CGPoint(x: x, y: y)
            NSLog("moveToUp: input = (%2.3f, %2.3f), output = (%2.3f, %2.3f)",
                  x, y, project!.slots[slotIndex].position.x, project!.slots[slotIndex].position.y)
            y = y - 0.01
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05))
        } while (y >= limit)
    }
    
    func moveToDown(project:NXECollageProject?, slotIndex:Int, limit:CGFloat, startPosition:CGPoint) {
        var x = startPosition.x, y = startPosition.y
        repeat {
            project!.slots[slotIndex].position = CGPoint(x: x, y: y)
            NSLog("moveToDown: input = (%2.3f, %2.3f), output =  (%2.3f, %2.3f)",
                  x, y, project!.slots[slotIndex].position.x, project!.slots[slotIndex].position.y)
            y = y + 0.01
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05))
        } while (y <= limit)
    }
    
    func moveToZero(project:NXECollageProject?, slotIndex:Int) {
        project!.slots[slotIndex].position = CGPoint(x: 0, y: 0)
        NSLog("moveToZero: input = (0, 0), output =  (%2.3f, %2.3f)",
              project!.slots[slotIndex].position.x, project!.slots[slotIndex].position.y)
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.05))
    }
    
    /*  (0,0) -> (0,1) -> (1,1) -> (1,-1) -> (-1,-1) -> (-1,1) -> (0,1) -> (0,1)
     */
    func testSlotMove() {
        self.label.text = "Slot Move simulate"
        
        let project = try? self.createCollageProject(dynamic: false, index: 0)
        let editor = NXEEngine.instance()!
        self.configureCollage(editor: editor, project: project!)
        project!.beginUpdates()
        project!.slots[0].angle = 0
        project!.slots[0].scale = 1
        project!.endUpdates()
        editor.updatePreview()
        RunLoop.current.run(until: Date(timeIntervalSinceNow: 1))
        
        moveToZero(project: project!, slotIndex: 0)
        moveToDown(project: project!, slotIndex: 0, limit: 1.0, startPosition: project!.slots[0].position)
        moveToRight(project: project!, slotIndex: 0, limit: 1.0, startPosition: project!.slots[0].position)
        moveToUp(project: project!, slotIndex: 0, limit: -1.0, startPosition: project!.slots[0].position)
        moveToLeft(project: project!, slotIndex: 0, limit: -1.0, startPosition: project!.slots[0].position)
        moveToDown(project: project!, slotIndex: 0, limit: 1.0, startPosition: project!.slots[0].position)
        moveToRight(project: project!, slotIndex: 0, limit: 0.0, startPosition: project!.slots[0].position)
        moveToUp(project: project!, slotIndex: 0, limit: 0.0, startPosition: project!.slots[0].position)
        moveToZero(project: project!, slotIndex: 0)
    }

    func testSlotClipStrongReference() {
        let assetItem = NXEAssetLibrary.instance().item(forId: "com.nexstreaming.kmsdk.dori.nexeditor.collage.baby album.v3.0_3v4_1_baby album")!
        let project = try? NXECollageProject(collageAssetItem: assetItem)
        project!.slots[0].clip = NXEClip.newSolidClip("#00000000")
        
        // This line wouldn't crash if succeed: slot configuration retains the clip property
        XCTAssert(project!.slots[0].clip.debugDescription!.count > 0)
    }
}
