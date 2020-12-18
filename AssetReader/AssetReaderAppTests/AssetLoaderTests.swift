/**
 * File Name   : AssetLoaderTests.swift
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
@testable import AssetReaderApp

class AssetLoaderTests: XCTestCase {
    var loader: AssetLoader!
    let volumes = [
        AssetVolume(type: .bundle, subpath:"SampleAssets/packages")!,
//        AssetVolume(type: .library, subpath:"Application Support/_nexeditor/packages")!,
//        AssetVolume(type: .documents, subpath:"_nexeditor/packages")!,
//        AssetVolume(type: .documents, subpath:"SampleAssets.bundle/packages")!,
        ]
    
    override func setUp() {
        super.setUp()
        loader = AssetLoader(volumes: volumes, indexer: nil)!
    }
    
    override func tearDown() {
        loader = nil
        self.continueAfterFailure = true

        super.tearDown()
    }
    
    
    func testScanPackages() {
        self.continueAfterFailure = false
        let volumes = [
            AssetVolume(type: .bundle, subpath:"SampleAssets/packages")!,
            ]
        
        for volume in volumes {
            let loader = AssetLoader()
            let packageURLs = try! FileManager.default.contentsOfDirectory(at: volume.resolvedURL,
                                                    includingPropertiesForKeys: [.isDirectoryKey],
                                                    options: FileManager.DirectoryEnumerationOptions.skipsHiddenFiles)
            
            for packageURL in packageURLs {
                loader.addPackage(at: packageURL)
            }
        }
        XCTAssert(loader.packageIds.count == 17, "\(loader.packageIds.count) != 17")
        let transitionItemCount = loader.itemIds(inCategory: "transition").count
        XCTAssert( transitionItemCount == 45, "\(transitionItemCount)")
        let categoryCount = loader.categories.count
        XCTAssert( categoryCount == 11, "\(categoryCount)")
        let pathInfo = loader.itemPathInfo(forId: "com.nexstreaming.kinemaster.builtin.transition.checker")
        XCTAssert( pathInfo != nil)
        let lastPathComponent = pathInfo!.infoPath.split(separator: "/").last
        XCTAssert( lastPathComponent! == "_info.json", "\(lastPathComponent!)")
    }
    
    func testAssetLoaderPackageIds() {
        XCTAssert(loader.packageIds.count == 17, "packageIds.count = \(loader.packageIds.count)")
    }
    
    func testAssetLoaderPackagePath() {
        self.continueAfterFailure = false
        let packageId = "asset-volume://mainBundle/SampleAssets.bundle/packages/basic_effects"
        let path = loader.packagePath(forId: packageId)
        XCTAssert(path != nil, "packagePath for \(packageId) is nil")
        var isDir = ObjCBool(false)
        XCTAssert(FileManager.default.fileExists(atPath: path!, isDirectory:&isDir) && isDir.boolValue)
        
    }
    
    func testAssetLoaderOverall() {
        
        // packageIds                   -> all package id list
        // categories                   -> all category names
        // packagePath(forId:)          -> packageId -> packagePath
        // categories(forPackageId:)    -> packageId -> category names
        // itemIds(inCategory:)         -> category name -> itemIds
        // itemPathInfo(itemId:)        -> itemId    -> ItemPathInfo(packageId, category, path, infoPath)

        XCTAssert(loader.packageIds.count == 17, "packageIds.count = \(loader.packageIds.count)")
        XCTAssert(loader.categories.count == 11, "categories.count = \(loader.categories.count)")

        for packageId in loader.packageIds {
            XCTAssert(loader.categories(forPackageId: packageId).count > 0, "categories(forPackageId:\(packageId).count(\(loader.categories(forPackageId: packageId).count)) <= 0")
            let path = loader.packagePath(forId: packageId)
            XCTAssert(path != nil, "packagePath for \(packageId) is nil")
            var isDir = ObjCBool(false)
            XCTAssert(FileManager.default.fileExists(atPath: path!, isDirectory:&isDir) && isDir.boolValue)
            
            let infoPath = loader.packagePath(forId: packageId)
            XCTAssert(infoPath != nil, "package infoPath for \(packageId) is nil")
            XCTAssert(FileManager.default.fileExists(atPath: infoPath!))
        }
        
        var itemIdsCount = 0
        for category in loader.categories {
            let itemIds = loader.itemIds(inCategory:category)!
            for itemId in itemIds {
                let pathInfo = loader.itemPathInfo(forId: itemId)!
                XCTAssert(pathInfo.packageId.count > 0)
                XCTAssert(pathInfo.category.count > 0)
                
                var directory = ObjCBool(false)
                XCTAssert(FileManager.default.fileExists(atPath: pathInfo.path, isDirectory: &directory) && directory.boolValue == true)
                XCTAssert(FileManager.default.fileExists(atPath: pathInfo.infoPath))
            }
            itemIdsCount += itemIds.count
        }
        XCTAssert(itemIdsCount == 191, "itemIdsCount for all categories(\(itemIdsCount)) != 266)")
    }
    
    func testIndexingTime() {
        self.measure {
            let volumes = [
                AssetVolume(type: .bundle, subpath:"SampleAssets/packages")!,
                AssetVolume(type: .library, subpath:"Application Support/_nexeditor/packages")!,
                AssetVolume(type: .documents, subpath:"_nexeditor/packages")!,
                AssetVolume(type: .documents, subpath:"SampleAssets.bundle/packages")!,
                ]
            
            let loader = AssetLoader(volumes: volumes, indexer: nil)!
            XCTAssert(loader.packageIds.count == 17, "packageIds.count = \(loader.packageIds.count)")
        }
    }

}
