//
//  NXELutRegistryTests.swift
//  NexEditorFramework
//
//  Created by jake.you on 8/14/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import NexEditorFramework

/*
 Possible errors on registration
 File not found
 Invalid format
 
 Possible errors on creation (internal)
 
 */
class NXELutRegistryTests: XCTestCase {
    
    override func setUp() {
        super.setUp()
        // Put setup code here. This method is called before the invocation of each test method in the class.
    }
    
    override func tearDown() {
        self.continueAfterFailure = true
        super.tearDown()
    }
    
    func testRegisterLUT() {
        let bundlePath = Bundle.main.path(forResource: "T&O - RMN Z1", ofType: "cube")
        let desc = NXECubeLutSource(path: bundlePath!)!
        
        let lutId: NXELutID = NXELutRegistry.registerLUT(with:desc, error:nil)
        XCTAssert( lutId != kLutIdNotFound )
    }
    
    func testLutIdFromLutType() {
        for idx in (NXELutTypes.LUT_NONE.rawValue + 1)..<NXELutTypes.LUT_MAX.rawValue {
            let type = NXELutTypes(rawValue: idx)!
            let lutId = NXELutRegistry.lutId(fromType:type)
            XCTAssert(type.rawValue == lutId)
        }
    }
    
    func testLutIdFromInvalidLutType() {
        
        // IN -> OUT
        let params: [(lutType: NXELutTypes, lutId: NXELutID)] = [
            (lutType: .LUT_NONE, lutId: kLutIdNotFound),
            (lutType: .LUT_MAX, lutId: kLutIdNotFound),
            ]
        for param in params {
            let lutId = NXELutRegistry.lutId(fromType: param.lutType)
            XCTAssert(lutId == param.lutId, "lutId \(lutId) != expected \(param.lutId) from lutType \(param.lutType.rawValue)")
        }
    }
    
    func testRegisterCustomCubeLUT() {
        // Public API: NXELutRegistry
        let bundlePath = Bundle.main.path(forResource: "T&O - RMN Z1", ofType: "cube")
        let source = NXECubeLutSource(path: bundlePath!)!
        
        let lutId = NXELutRegistry.registerLUT(with:source, error:nil)
        
        // Internal API access for TEST ONLY
        let entry = LUTMap.instance().entry(with: lutId)
        XCTAssert(entry != nil)
        XCTAssert(entry!.path == bundlePath!)
    }
    
    func testErrorNoFile() {
        self.continueAfterFailure = false
        
        // Public API: NXELutRegistry
        let source = NXECubeLutSource(path: "fails/to/find.me")!
        
        var error: NSError? = nil
        let lutId = NXELutRegistry.registerLUT(with:source, error:&error)
        
        XCTAssert(error != nil, "Registering file not available should fail with an error")
        XCTAssert(error! is NXEError)
        XCTAssert(error!.code == Int(ERRORCODE.ERROR_FILE_IO_FAILED.rawValue))
        XCTAssert(lutId == kLutIdNotFound)
    }
    
    func testErrorInvalidFormat() {
        self.continueAfterFailure = false
        // Public API: NXELutRegistry
        let pngPath = Bundle(for: LUTMap.self).url(forResource: "lut_warm", withExtension: "png", subdirectory: "resource/asset/lut" )?.path
        let source = NXECubeLutSource(path: pngPath!)!
        
        var error: NSError? = nil
        let lutId = NXELutRegistry.registerLUT(with:source, error:&error)
        
        XCTAssert(error != nil, "Registering wrong cube format should fail with an error")
        XCTAssert(error! is NXEError)
        XCTAssert(error!.code == Int(ERRORCODE.ERROR_UNSUPPORT_FORMAT.rawValue))
        XCTAssert(lutId == kLutIdNotFound)
    }
    
    func testClipSetLutTypeGetLutId() {
        let clip = NXEClip.newSolidClip("red")!
        
        for idx in NXELutTypes.LUT_NONE.rawValue..<NXELutTypes.LUT_MAX.rawValue {
            let type = NXELutTypes(rawValue: idx)!
            clip.setLut(type)
            XCTAssert(clip.lutId == NXELutRegistry.lutId(fromType:type), "lutId \(clip.lutId) fromType\(NXELutRegistry.lutId(fromType:type))")
        }
    }
    
    func testClipDefaultLutId() {
        let clip = NXEClip.newSolidClip("red")!
        XCTAssert(clip.lutId == kLutIdNotFound)
        XCTAssert(clip.vinfo.lut == Int32(NXELutTypes.LUT_NONE.rawValue))
        
    }
    
    func testClipSetLutIdGetLutType() {
        let clip = NXEClip.newSolidClip("red")!
        clip.lutId = NXELutID(1)
        XCTAssert(clip.vinfo.lut == 1)
    }
    
    func testClipSetLutIdWithLargeNumberGetLutType() {
        let clip = NXEClip.newSolidClip("red")!
        clip.lutId = NXELutID(711911157)
        XCTAssert(clip.vinfo.lut == 711911157)
    }
    
    func testLayerSetColorFilterGetLutId() {
        let layer = NXELayer()
        
        layer.colorFilter = .LUT_NONE
        XCTAssert(layer.lutId == kLutIdNotFound, "lutId \(layer.lutId) != expected \(kLutIdNotFound)")
        
        for idx in (NXELutTypes.LUT_NONE.rawValue + 1)..<NXELutTypes.LUT_MAX.rawValue {
            layer.colorFilter = NXELutTypes(rawValue: idx)!
            
            XCTAssert(layer.lutId == idx, "lutId \(layer.lutId) != expected \(idx)")
        }
    }
    
    func testLayerLutIdDefault() {
        let editor = NXEEngine.instance()!
        let project = NXEProject()
        
        editor.setProject(project)
        
        let image = UIImage(named: clipsourceUrl.appendingPathComponent("picture/Desert.jpg").path)!
        let layer = NXEImageLayer(image: image, point: CGPointNXELayerCenter)!
        
        XCTAssert(layer.colorFilter == NXELutTypes.LUT_NONE)
        
        project.layers = [layer]
        
        XCTAssert(layer.lutId == kLutIdNotFound)
        XCTAssert(layer.colorFilter == NXELutTypes.LUT_NONE)
    }
    
    func testLayerLutIdResetFromValidLut() {
        let editor = NXEEngine.instance()!
        let project = NXEProject()
        
        editor.setProject(project)
        
        let image = UIImage(named: clipsourceUrl.appendingPathComponent("picture/Desert.jpg").path)!
        let layer = NXEImageLayer(image: image, point: CGPointNXELayerCenter)!
        
        project.layers = [layer]
        
        layer.lutId = NXELutRegistry.lutId(fromType: .LUT_BLUE)
        XCTAssert(layer.lutId == NXELutTypes.LUT_BLUE.rawValue)
        XCTAssert(layer.colorFilter == NXELutTypes.LUT_BLUE)
        
        layer.lutId = kLutIdNotFound
        XCTAssert(layer.lutId == kLutIdNotFound)
        XCTAssert(layer.colorFilter == NXELutTypes.LUT_NONE)
        
    }
    
    func testLayerSetLutIdGetColorFilter() {
        let layer = NXELayer()
        layer.lutId = NXELutID(1)
        XCTAssert(layer.colorFilter.rawValue == 1)
    }
    
    func testRegstierDuplicateLUT() {
        let bundlePath = Bundle.main.path(forResource: "T&O - RMN Z1", ofType: "cube")
        let desc = NXECubeLutSource(path: bundlePath!)!
        
        let lutId: NXELutID = NXELutRegistry.registerLUT(with:desc, error:nil)
        XCTAssert(lutId != kLutIdNotFound)
        
        var lutIdToCompare: NXELutID = kLutIdNotFound
        for _ in 0..<5 {
            lutIdToCompare = NXELutRegistry.registerLUT(with:desc, error:nil)
            XCTAssert(lutId == lutIdToCompare)
        }
    }
    
    func testRegstierLUTWithNilPath() {
        let desc = NXECubeLutSource(path: nil)!
        var error: NSError? = nil
        
        let lutId: NXELutID = NXELutRegistry.registerLUT(with:desc, error:&error)
        XCTAssert(lutId == kLutIdNotFound)
        XCTAssert(error!.code == Int(ERRORCODE.ERROR_FILE_IO_FAILED.rawValue))
    }
}
