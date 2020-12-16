//
//  TextEffectDemoTest.swift
//  NexEditorFramework
//
//  Created by MJ.KONG-MAC on 19/05/2017.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class TextEffectParams {
    let assetIds: [String]
    
    init(assetIds:[String] = []) {
        self.assetIds = assetIds
    }
}

class ZTextEffectDemoTest: XCTestCase {
    var view: UIView!
    
    let textEfectParams: [TextEffectParams] = [
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.1v1.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.1v2.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.2v1.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.9v16.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.awardsceremony.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.cornerlines.1v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.cornerlines.1v2.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.cornerlines.2v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.cornerlines.9v16.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.cornerlines.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.1v1.sub",
                                      "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.1v2.sub",
                                      "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.2v1.sub",
                                      "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.9v16.sub",
                                      "com.nexstreaming.kmsdk.design.nexeditor.tfx.fade.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.floatin.1v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.floatin.1v2.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.floatin.2v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.floatin.9v16.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.floatin.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.lightbulbs.1v1.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.lightbulbs.1v2.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.lightbulbs.2v1.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.lightbulbs.9v16.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.lightbulbs.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.parallellines.1v1.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.parallellines.1v2.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.parallellines.2v1.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.parallellines.9v16.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.parallellines.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.rhombus.1v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.rhombus.1v2.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.rhombus.2v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.rhombus.9v16.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.rhombus.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.underline.1v1.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.underline.1v2.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.underline.2v1.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.underline.9v16.sub",
                                       "com.nexstreaming.kmsdk.design.nexeditor.tfx.underline.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomin.1v1.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomin.1v2.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomin.2v1.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomin.9v16.sub",
                                    "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomin.16v9.sub"]),
        TextEffectParams(assetIds: ["com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomout.1v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomout.1v2.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomout.2v1.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomout.9v16.sub",
                                     "com.nexstreaming.kmsdk.design.nexeditor.tfx.zoomout.16v9.sub"]),
    ]
    
    func getAspectType(aspectType: NXEAspectType) -> String {
        switch(aspectType) {
        case .ratio1v1:
            return "1v1"
        case .ratio1v2:
            return "1v2"
        case .ratio2v1:
            return "2v1"
        case .ratio9v16:
            return "9v16"
        default:
            return "16v9"
        }
    }
    
    func TTExport(textEffectId: String, paths: [String]) {
        
        let engine = NXEEngine.instance()!
        let project = NXEProject()
        
        var clips: [NXEClip] = []
        
        for path: String in paths {
            do {
                clips.append(try NXEClip(source: NXEClipSource(path: path)))
            } catch let error {
                XCTFail("Failed open clip a pat \(path) error:\(error)")
            }
        }
        
        project.visualClips = clips
        project.update()
        
        engine.setProject(project)
        
        let assetLib = NXEAssetLibrary.instance()
        let textEffectAsset = assetLib.item(forId: textEffectId)!
        let textEffectParams = NXETextEffectParams()
        let aspectType = textEffectAsset.aspectType
        
        label.text = "Exporting TextEffect \(textEffectAsset.name!) \(aspectType.displayText)"
        
        textEffectParams.introTitle = "\(textEffectAsset.name!)"
        textEffectParams.introSubtitle = getAspectType(aspectType: aspectType)
        textEffectParams.outroTitle = "Thank you"
        
        let textEffect = NXETextEffect(id: textEffectId, params: textEffectParams)
        do {
            try project.setTextEffect(textEffect)
        } catch let error {
            XCTFail("Failed setTextEffect name \(textEffectAsset.name!) error:\(error)")
        }
        
        engine.aspectType = aspectType
        
        engine.setPreviewWith(view.layer, previewSize: view.bounds.size)
        engine.TTPrepare()
        engine.play()
        engine.pause()
        engine.TTSeek(100)
        
        // seek to beginning
        var finished = false
        engine.setCallbackWithSetTime { (seekDoneTs: NXETimeMillis) in
            finished = true
        }
        engine.seek(0)
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
        
        print("mode: \(aspectType.displayText), start text effect name: \(textEffectAsset.name!)")
        
        // export
        finished = false
        
        let resolution = textEffectAsset.aspectType.exportResolution
        let bitrate = Int32(Double(resolution.width * resolution.height * 30) * 0.17)
        var path = NSSearchPathForDirectoriesInDomains(.documentDirectory, .userDomainMask, true)[0]
        path = (path as NSString).appendingPathComponent(textEffectAsset.name!+"_" + aspectType.displayText + ".mp4")
        
        engine.setCallbackWithEncodingProgress({ (percent) in
        }, encodingEnd:{ (error) in
            XCTAssert(error == nil, "Export finished with error \(error!.localizedDescription)")
            finished = true
        })
        
        let ret = engine.exportProject(path,
                                       width: Int32(resolution.width),
                                       height: Int32(resolution.height),
                                       bitrate: bitrate,
                                       maxFileSize: LONG_MAX,
                                       projectDuration: 0,
                                       forDurationLimit: false,
                                       rotationFlag: ._ROTATION_0)
        XCTAssert(ret == Int32(ERRORCODE.ERROR_NONE.rawValue))
        
        while !finished {
            RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
        }
        
        print("mode: \(aspectType.displayText), end text effect name: \(textEffectAsset.name!)")
    }
    
    override func setUp() {
        super.setUp()
        view = self.preview
        
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("OverlayTitleAssets.bundle/packages"))
    }
    
    override func tearDown() {
        super.tearDown()
    }
    
    func testExportTextEffect_2files() {
        let paths = [clipsourceUrl.appendingPathComponent("picture/Lighthouse.jpg").path,
                     clipsourceUrl.appendingPathComponent("picture/Chrysanthemum.jpg").path]
        
        for param in textEfectParams.prefix(2) {
            for assetItemId in param.assetIds {
                autoreleasepool {
                    TTExport(textEffectId: assetItemId, paths: paths)
                }
            }
        }
    }
    
    func testExportTextEffect_1file() {
        let paths = [clipsourceUrl.appendingPathComponent("picture/Lighthouse.jpg").path]
        
        for param in textEfectParams.prefix(2) {
            for assetItemId in param.assetIds {
                autoreleasepool {
                    TTExport(textEffectId: assetItemId, paths: paths)
                }
            }
        }
    }
    
}
