/**
 * File Name   : TemplateExportTests.swift
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

/// Prefix 'Z' to run the test later than others
class ZTemplateExportTests: XCTestCase {
    var view: UIView!
    
    override func setUp() {
        super.setUp()
        view = self.preview
        
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("TemplateAssets.bundle/packages"))
    }
    
    override func tearDown() {
        super.tearDown()
    }
    
    let templateParams: [TemplateSequenceParams] = [
        TemplateSequenceParams(path: "ClipSources/template/Bokeh",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08"],
                       name: "Bokeh",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_1v1_bokeh",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_2v1_bokeh",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_9v16_bokeh",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.bokeh.template_16v9_bokeh"]),
        TemplateSequenceParams(path: "ClipSources/template/Camera",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09"],
                       name: "Camera",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_1v1_camera",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_2v1_camera",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_9v16_camera",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.camera.template_16v9_camera"]),
        TemplateSequenceParams(path: "ClipSources/template/Cartoon",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10"],
                       name: "Cartoon",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_1v1_cartoon",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_2v1_cartoon",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_9v16_cartoon",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.cartoon.template_16v9_cartoon"]),
        TemplateSequenceParams(path: "ClipSources/template/Family",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16"],
                       name: "Family",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.family.template_1v1_family",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_2v1_family",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_9v16_family",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.family.template_16v9_family"]),
        TemplateSequenceParams(path: "ClipSources/template/Fashion",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15"],
                       name: "Fashion",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_1v1_fashion",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_2v1_fashion",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_9v16_fashion",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.fashion.template_16v9_fashion"]),
        TemplateSequenceParams(path: "ClipSources/template/Funny",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21"],
                       name: "Funny",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.funny.template_1v1_funny",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.funny.template_2v1_funny",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.funny.template_9v16_funny",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.funny.template_16v9_funny"]),
        TemplateSequenceParams(path: "ClipSources/template/Grid",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15"],
                       name: "Grid",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.grid.template_1v1_grid",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.grid.template_2v1_grid",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.grid.template_9v16_grid",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.grid.template_16v9_grid"]),
        TemplateSequenceParams(path: "ClipSources/template/HappyBirthday",
                       clipNames:["01", "02", "03", "04", "05", "06", "07"],
                       name: "HappyBirthday",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.happybirthday.v.2.0_1v1_happybirthday",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.happybirthday.v.2.0_2v1_happybirthday",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.happybirthday.v.2.0_9v16_happybirthday",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.happybirthday.v.2.0_16v9_happybirthday"]),
        TemplateSequenceParams(path: "ClipSources/template/HappyDays",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10"],
                       name: "HappyDays",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.happydays.v.2.0_1v1_happydays",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.happydays.v.2.0_2v1_happydays",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.happydays.v.2.0_9v16_happydays",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.happydays.v.2.0_16v9_happydays"]),
        TemplateSequenceParams(path: "ClipSources/template/ModernWhite",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08"],
                       name: "ModernWhite",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.modernwhite.v.2.0_1v1_modernwhite",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.modernwhite.v.2.0_2v1_modernwhite",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.modernwhite.v.2.0_9v16_modernwhite",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.modernwhite.v.2.0_16v9_modernwhite"]),
        TemplateSequenceParams(path: "ClipSources/template/NeonLight",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14"],
                       name: "NeonLight",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_1v1_neonlight",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_2v1_neonlight",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_9v16_neonlight",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.neonlight.template_16v9_neonlight"]),
        TemplateSequenceParams(path: "ClipSources/template/OldFilm",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11"],
                       name: "OldFilm",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_1v1_oldfilm",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_2v1_oldfilm",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_9v16_oldfilm",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.oldfilm.template_16v9_oldfilm"]),
        TemplateSequenceParams(path: "ClipSources/template/PhotoFrame",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11"],
                       name: "PhotoFrame",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_1v1_photoframe",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_2v1_photoframe",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_9v16_photoframe",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.photoframe.template_16v9_photoframe"]),
        TemplateSequenceParams(path: "ClipSources/template/Polaroid",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09"],
                       name: "Polaroid",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_1v1_polaroid",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_2v1_polaroid",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_9v16_polaroid",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_16v9_polaroid"]),
        TemplateSequenceParams(path: "ClipSources/template/Romantic",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08"],
                       name: "Romantic",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_1v1_romantic",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_2v1_romantic",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_9v16_romantic",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.romantic.template_16v9_romantic"]),
        TemplateSequenceParams(path: "ClipSources/template/Snow",
                       clipNames:["01", "02", "03", "04", "05"],
                       name: "Snow",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_1v1_snow",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_2v1_snow",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_9v16_snow",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.snow.template_16v9_snow"]),
        TemplateSequenceParams(path: "ClipSources/template/Snow2",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18"],
                       name: "Snow2",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.snow2.template_2.0_1v1_snow2",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.snow2.template_2.0_2v1_snow2",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.snow2.template_2.0_9v16_snow2",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.snow2.template_2.0_16v9_snow2"]),
        TemplateSequenceParams(path: "ClipSources/template/Sports",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18",
                                  "19", "20", "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31", "32", "33", "34", "35", "36",
                                  "37", "38", "39", "40", "41", "42", "43", "44", "45", "46", "47", "48"],
                       name: "Sports",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.sports.template_1v1_sports",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.sports.template_2v1_sports",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.sports.template_9v16_sports",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.sports.template_16v9_sports"]),
        TemplateSequenceParams(path: "ClipSources/template/Symmetry",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15"],
                       name: "Symmetry",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_1v1_symmetry",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_2v1_symmetry",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_9v16_symmetry",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.symmetry.template_16v9_symmetry"]),
        TemplateSequenceParams(path: "ClipSources/template/Table",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11"],
                       name: "Table",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.table.template_1v1_table",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.table.template_2v1_table",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.table.template_9v16_table",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.table.template_16v9_table"]),
        TemplateSequenceParams(path: "ClipSources/template/Thriller",
                       clipNames:["01", "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13"],
                       name: "Thriller",
                       ext: "jpg",
                       assetItemIds: ["com.nexstreaming.kmsdk.design.nexeditor.template.thriller.template_1v1_thriller",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.thriller.template_2v1_thriller",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.thriller.template_9v16_thriller",
                                      "com.nexstreaming.kmsdk.design.nexeditor.template.thriller.template_16v9_thriller"]),
        ]
    
    func testExportTemplates() {
        let url = clipsourceUrl.appendingPathComponent("template/Thriller/01.jpg")
        XCTAssert(FileManager.default.fileExists(atPath: url.path), "NESI-365 Make sure clip sources available under ClipSources/template/ directory to proceed this test case")
        
        for param in templateParams {
            for assetItemId in param.assetItemIds {
                autoreleasepool {
                    let editor = NXEEngine.instance()!
                    editor.TTExport(sequence: param, templateId: assetItemId, preview: self.preview, label: self.label)
                }
            }
        }
    }
}
