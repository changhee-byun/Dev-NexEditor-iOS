//
//  LUTMapTests.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 2/23/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

class LUTMapTests: XCTestCase {
    let lutStrings = ["none",
                      "afternoon",
                      "almond_blossom",
                      "autumn",
                      "boring",
                      "caramel_candy",
                      "caribbean",
                      "cinnamon",
                      "cloud",
                      "coral_candy",
                      "cranberry",
                      "daisy",
                      "dawn",
                      "disney",
                      "england",
                      "espresso",
                      "eyeshadow",
                      "gloomy",
                      "jazz",
                      "lavendar",
                      "moonlight",
                      "newspaper",
                      "paris",
                      "peach",
                      "rainy",
                      "raspberry",
                      "retro",
                      "sherbert",
                      "shiny",
                      "smoke",
                      "stoneedge",
                      "sunrising",
                      "symphony_blue",
                      "tangerine",
                      "tiffany",
                      "vintage_flower",
                      "vintage_romance",
                      "vivid",
                      "warm",
        // r1.0.19.05
        "blue",
        "blueonly",
        "dbright",
        "heat",
        "ludwig",
        "negative",
        "oldfilm",
        "rosy",
        "salmon_teal",
        "sunprint",
        "sunset",
        "sweet"]
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
        NXEAssetLibrary.addAssetSourceDirectoryURL(assetBundlesUrl.appendingPathComponent("SampleAssets.bundle/packages"))
        
    }
    
    override func tearDown() {
        // Put teardown code here. This method is called after the invocation of each test method in the class.
        super.tearDown()
    }
    
    func testLegacyStringToType() {
        XCTAssert(LUTMap.index(fromLutName:"none") == NXELutTypes.LUT_NONE.rawValue)
        XCTAssert(LUTMap.index(fromLutName:"") == NXELutTypes.LUT_NONE.rawValue)
        XCTAssert(LUTMap.index(fromLutName:"youcan'tfind this") == NXELutTypes.LUT_NONE.rawValue)
        for (index, string) in lutStrings.enumerated() {
            XCTAssert(LUTMap.index(fromLutName:string) == UInt(index))
        }
    }
    
    func testLegacyTypeToString() {
        XCTAssert(LUTMap.path(for:0) == nil)
        let maxPath = LUTMap.path(for:NXELutTypes.LUT_MAX.rawValue-1)! as NSString
        XCTAssert(maxPath.lastPathComponent == "lut_sweet.png", "maxPath.lastPathComponent '\(maxPath.lastPathComponent) != lut_sweet.png")
        
        for i in (NXELutTypes.LUT_NONE.rawValue + 1)...(NXELutTypes.LUT_MAX.rawValue-1) {
            let path = LUTMap.path(for:i)! as NSString
            let expected = String(format:"lut_\(lutStrings[Int(i)]).png")
            XCTAssert(path.lastPathComponent == expected, "path[\(i) '\(path.lastPathComponent) != \(expected)")
        }
    }
    
    func testLegacyTypeToStringValueOutOfRangeShouldReturnNil() {
        XCTAssert(LUTMap.path(for:333333) == nil)
    }

    func testAssetItemLUTId() {
        let index = LUTMap.index(fromLutName:"com.nexstreaming.kmsdk.design.nexeditor.template.lut.disney")
        let path = LUTMap.path(for:index)! as NSString
        XCTAssert(path.lastPathComponent == "disney.png", "path '\(path.lastPathComponent) != disney.png")
        
        // after adding asset item lut, builtin lut lookup should still work
        XCTAssert(LUTMap.index(fromLutName:"") == NXELutTypes.LUT_NONE.rawValue)
        XCTAssert(LUTMap.index(fromLutName:"warm") == NXELutTypes.LUT_WARM.rawValue)
    }
    
    func testDuplicateAssetItemLUTId() {
        let index = LUTMap.index(fromLutName:"com.nexstreaming.kmsdk.design.nexeditor.template.lut.disney")
        let path = LUTMap.path(for:index)! as NSString
        XCTAssert(path.lastPathComponent == "disney.png", "path '\(path.lastPathComponent) != disney.png")
        
        let indexAfterRetry = LUTMap.index(fromLutName:"com.nexstreaming.kmsdk.design.nexeditor.template.lut.disney")
        let pathAfterRetry = LUTMap.path(for:index)! as NSString
        
        XCTAssert(index == indexAfterRetry)
        XCTAssert(path == pathAfterRetry)
    }
    
    func testCubeLUTSource() {
        // retrive a cube lut and inspect properties
        let map = LUTMap()
        let cubeLutId = map.addEntry(NXECubeLutSource(path: "path/to/cube.cube"))
        var entry: NXEFileLutSource?
        
        // retrive a cube lut and inspect properties
        entry = map.entry(with: cubeLutId)
        XCTAssert(entry != nil)
        XCTAssert(entry! is NXECubeLutSource)
        XCTAssert(entry!.path == "path/to/cube.cube")
    }
    
    func testPNGLUTSource() {
        // retrive a png lut and inspect properties
        let map = LUTMap()
        let pngLutId = map.addEntry(NXEPNGLutSource(path: "path/to/png.png"))
        var entry: NXEFileLutSource?
        
        // retrive a cube lut and inspect properties
        entry = map.entry(with: pngLutId)
        XCTAssert(entry != nil)
        XCTAssert(entry! is NXEPNGLutSource)
        XCTAssert(entry!.path == "path/to/png.png")
    }
    
    func testWrongIDShouldReturnNil() {
        let map = LUTMap()
        var entry: NXEFileLutSource?
        entry = map.entry(with: 748)
        XCTAssert(entry == nil)
    }
    
    func testBuildLUT() {
        let map = LUTMap()
        let cubePath = Bundle.main.path(forResource: "T&O - RMN Z1", ofType: "cube")!
        let cubdesc = NXECubeLutSource(path: cubePath)
        let cubeLutId = map.addEntry(cubdesc!)
        
        // png lut
        let pngPath = Bundle(for: LUTMap.self).url(forResource: "lut_warm", withExtension: "png", subdirectory: "resource/asset/lut" )?.path
        let pngdesc = NXEPNGLutSource(path: pngPath)
        let pngLutId = map.addEntry(pngdesc!)
        
        // try cube
        let lutIds = [cubeLutId, pngLutId]
        for lutId in lutIds {
            let entry = map.entry(with: lutId)!
            
            if entry is NXEPNGLutSource {
                let image = UIImage(named:entry.path)
                XCTAssert(image?.size.width == 64)
                XCTAssert(image?.size.height == 4096)
                XCTAssert(entry.createTexture() >= 0)
                
            } else if entry is NXECubeLutSource {
                let source = entry as! NXECubeLutSource
                let cubeLutData: CubeLutData = source.loadData()
                XCTAssert(cubeLutData.bytes != nil)
                XCTAssert(cubeLutData.imageWidth == 64)
                XCTAssert(cubeLutData.imageHeight == 4096)
                XCTAssert(entry.createTexture() >= 0)
            }
        }
    }
}
