//
//  DecryptionDelegateTests.swift
//  AssetReader
//
//  Created by Simon Kim on 4/13/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import XCTest
@testable import AssetReaderApp

class DecryptionDelegateTests: XCTestCase, AssetResourceDecryptionDelegate {
    var assetDB: AssetDBProtocol!
    
    override func setUp() {
        super.setUp()
        assetDB = AssetDB.withSourceURLs([
            URL(string:AssetVolume(type: .documents, subpath: "TemplateAssets.bundle/packages").assetVolumeURLString)!])
    }
    
    override func tearDown() {
        assetDB = nil
        super.tearDown()
    }
    
    func testExample() {
        
        assetDB.decryptionDelegate = self
        
        _ = assetDB.item(forId: "com.nexstreaming.kmsdk.design.nexeditor.template.polaroid.template_16v9_polaroid")
    }

    
    // MARK: -
    var manifest: [String] = []
    
    var packagePath: String? = nil
    
    var subpathToEncryptionManifest: String? = nil {
        willSet(subpath) {
            if subpath == nil {
                manifest = []
            }
            if subpath == subpathToEncryptionManifest {
                return
            }
            
            manifest = []
            do {
                let url = URL(fileURLWithPath: packagePath!).appendingPathComponent(subpath!)
                let data = try Data(contentsOf: url)
                let info = try JSONSerialization.jsonObject(with: data) as? [String: Any]
                if let psdString = info!["psd"] as? String,
                    let pstData = psdString.data(using: .utf8),
                    let psd = try JSONSerialization.jsonObject(with: pstData) as? [String: Any],
                    let filelist = psd["f"] as? [String]
                {
                    manifest = filelist
                }
            } catch _ {
            }
        }
    }
    
    func decryptData(_ encrypted: Data!, atSubpath subpathFromPackage: String!) -> Data! {
        
        if manifest.count == 0 {
            return encrypted
        }
        
        let url = URL(fileURLWithPath: packagePath!).appendingPathComponent(subpathFromPackage)
        XCTAssert(FileManager.default.isReadableFile(atPath: url.path), "file not found \(subpathFromPackage)")
        
        if let index = manifest.index(of:subpathFromPackage) {
            // perform decryption
            print("Decrypt: \(subpathFromPackage!)[\(index)]")
        }
        return encrypted
    }
    
    func releaseResource() {
        manifest = []
    }

}
