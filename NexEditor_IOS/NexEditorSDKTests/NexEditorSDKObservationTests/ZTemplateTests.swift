//
//  TemplateTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 5/23/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class ZTemplateTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testPreview() {
        
        let url = clipsourceUrl.appendingPathComponent("template/Family/01.jpg")
        XCTAssert(FileManager.default.fileExists(atPath: url.path), "NESI-365 Make sure clip sources available under ClipSources/template/ directory to proceed this test case")
        
        let editor = NXEEngine.instance()!
        let sequence =
            TemplateSequenceParams(path: "ClipSources/template/Family",
                                   clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16"],
                                   name: "Family",
                                   ext: "jpg",
                                   assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.family.template_1v1_family",
                                                  "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_2v1_family",
                                                  "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_9v16_family",
                                                  "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_16v9_family"])
        
        for assetItemId in sequence.assetItemIds {
            autoreleasepool {
                editor.TTPreview(sequence: sequence, templateId: assetItemId, preview: self.preview, label: self.label, commands: [.play(for: 5.0)])
            }
        }
    }
    
    func testChangeClipsAfterTemplate() {

        let url = clipsourceUrl.appendingPathComponent("template/Family/01.jpg")
        XCTAssert(FileManager.default.fileExists(atPath: url.path), "NESI-365 Make sure clip sources available under ClipSources/template/ directory to proceed this test case")
        
        // NESI-370
        let editor = NXEEngine.instance()!
        let sequence = TemplateSequenceParams(path: "ClipSources/template/Family",
                                              clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16"],
                                              name: "Family",
                                              ext: "jpg",
                                              assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.family.template_1v1_family"])
        let sequence2 =  TemplateSequenceParams(path: "ClipSources/template/Cartoon",
                                                clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10"],
                                                name: "Cartoon",
                                                ext: "jpg")
        
        if sequence.clips().count == 0 {
            XCTFail("No clips loaded")
            return
        }
        let assetLib = NXEAssetLibrary.instance()
        let templateAsset = assetLib.item(forId:sequence.assetItemIds[0])!
        let aspectType = templateAsset.aspectType
        label.text = "Exporting Template \(templateAsset.name!) \(aspectType.displayText) \(sequence.clipNames.count) photos"
        editor.aspectType = aspectType
        
        let project: NXETemplateProject? = try? NXETemplateProject(templateAssetItem: templateAsset as! NXETemplateAssetItem, clips: sequence.clips())
        editor.setProject(project)
        
        // Preview the first frame
        editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
        editor.TTPrepare()
        editor.TTPlay(for:5)
        editor.stopSync { errorcode in }
        
        // Was crashing before NESI-370 fix:
        // - Disposing visualClipEffectTimeGroups and adjustedAudioStartTimeScales at the end of NXEProject.updateProject()
        project!.visualClips = sequence2.clips()
        editor.TTPlay(for:5)
        editor.stopSync { errorcode in }
    }
    
    func testTemplatePreviewPresetSequences() {
        // VVII
        let sequences = [
            TemplateSequenceParams(path: "ClipSources",
                                   clipNames:["picture/Tulips.jpg", "video/tvc30sec.mp4", "picture/Koala.jpg", "video/tvc3sec.mp4"],
                                   name: "IVIV" ),
            TemplateSequenceParams(path: "ClipSources",
                                   clipNames:["video/tvc30sec.mp4", "video/tvc3sec.mp4", "picture/Koala.jpg", "picture/Tulips.jpg"],
                                   name: "VVII" ),
            ]
        
        let lib = NXEAssetLibrary.instance()
        let templates = lib.items(inCategory: NXEAssetItemCategory.template)
        let commands: [PlaybackCommand] = [
            .playEnd()
        ]
        
        let editor = NXEEngine.instance()!
        editor.TTPrepare()
        
        var index = 0
        let filterTemplates = templates.filter({ (assetItem: NXEAssetItem) -> Bool in
            return !assetItem.name.contains("3d") && assetItem.aspectType == .ratio16v9
        })
        let countTemplates = filterTemplates.count
        for template in filterTemplates {
            for sequence in sequences {
                label.text = "Previewing Sequence \(sequence.name) \(template.name!) \(index + 1)/\(countTemplates) \(template.aspectType.displayText) \(sequence.clipNames.count) clips"
                autoreleasepool{
                    editor.TTPreview(sequence: sequence, templateId: template.itemId, preview: self.preview, commands: commands)
                }
            }
            index = index + 1
        }
    }
    
    func testTemplateExportToCompareWithPreview() {
        
        let assetLib = NXEAssetLibrary.instance()
        let items = assetLib.items(inCategory: NXEAssetItemCategory.template).filter({ $0.aspectType == .ratio16v9 })
        
        for item in items {
            let sequences = [
                TemplateSequenceParams(path: "ClipSources",
                                       clipNames:["picture/Tulips.jpg", "video/tvc30sec.mp4", "picture/Koala.jpg", "video/tvc3sec.mp4"],
                                       name: item.name + "(IVIV)" ),
                TemplateSequenceParams(path: "ClipSources",
                                       clipNames:["video/tvc30sec.mp4", "video/tvc3sec.mp4", "picture/Koala.jpg", "picture/Tulips.jpg"],
                                       name: item.name + "(VVII)" ),
                ]
            
            for sequence in sequences {
                autoreleasepool {
                    let editor = NXEEngine.instance()!
                    editor.TTExport(sequence: sequence, preview: self.preview, label: self.label, configure: { (engine) -> NXEProject? in
                        do {
                            let project: NXETemplateProject = try NXETemplateProject(templateAssetItem: item as! NXETemplateAssetItem, clips: sequence.clips())
                            return project
                        } catch {
                            return nil
                        }
                    })
                }
            }
        }
    }
    
    func testTemplatePreviewSetTimedClip() {
        let sequences = [
            TemplateSequenceParams(path: "ClipSources",
                                   clipNames:["video/tvc30sec.mp4", "video/tvc3sec.mp4"],
                                   name: "SetTimedClips" ),
        ]
        let lib = NXEAssetLibrary.instance()
        let templates = lib.items(inCategory: NXEAssetItemCategory.template)
        
        let countTemplates = templates.count
        
        var index = 0
        for template in templates {
            autoreleasepool {
                let editor = NXEEngine.instance()!
                for sequence in sequences {
                    label.text = "Previewing Sequence \(sequence.name) \(template.name!) \(index + 1)/\(countTemplates) \(template.aspectType.displayText) \(sequence.clipNames.count) clips"
                    autoreleasepool{
                        let assetLib = NXEAssetLibrary.instance()
                        let templateAsset = assetLib.item(forId:template.itemId)!
                        let aspectType = templateAsset.aspectType
                        editor.aspectType = aspectType

                        do {
                            let project: NXETemplateProject = try NXETemplateProject(templateAssetItem: templateAsset as! NXETemplateAssetItem, clips: sequence.clips())
                            
                            project.visualClips[0].setTrim(5000, endTrimTime: 15000)
                            project.visualClips[1].setTrim(1000, endTrimTime: 2500)
                            project.update()
                            
                            editor.setProject(project)
                            
                            var finished = false
                            
                            // Preview the first frame
                            editor.setPreviewWith(preview.layer, previewSize: preview.bounds.size)
                            editor.TTPrepare()
                            editor.setCallbackWithPlayProgress({ curTime in
                            }, playStart: {
                            }, playEnd: {
                                finished = true
                            })
                            editor.play()
                            while !finished {
                                RunLoop.current.run(until: Date(timeIntervalSinceNow: 0.5))
                            }
                        } catch {
                        }
                    }
                }
                index = index + 1
            }
        }
    }
}
