/*
 * File Name   : AssetPackageManager.swift
 * Description :
 *
 *                NexStreaming Corp. Confidential & Proprietary
 *
 *            Copyright(c) 2002-2017 NexStreaming Corp. All rights reserved.
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

import Foundation

open class NexEditorAssetPackageManager {
    enum Location: Int {
        case mainBundle = 0
        case bundle = 1
        case documents = 2
        
        static let count = 3
    }
    
    private var installed: [URL: Bool] = [:]
    
    func scanPackageURLs() -> [Location: [URL]] {
        var result: [Location: [URL]] = [
            .mainBundle: [],
            .bundle: [],
            .documents: []
        ]
        
        if let url = Bundle.main.url(forResource: "AssetPackages", withExtension: nil) {
            result[.mainBundle] = url.urlsForSubdirectories()
        }
        
        let bundleNames = ["beattemplate", "SampleAssets", "TemplateAssets", "OverlayTitleAssets"]
        var bundlePackageURLs: [URL] = []
        for name in bundleNames {
            if let url = Bundle.main.url(forResource: name, withExtension: "bundle")?.appendingPathComponent("packages") {
                bundlePackageURLs += url.urlsForSubdirectories()
            }
        }
        result[.bundle] = bundlePackageURLs
        
        if let url = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask).first?.appendingPathComponent("nexeditor/packages") {
            result[.documents] = url.urlsForSubdirectories()
        }
        
        return result
    }
    
    open func isInstallled(packageAtURL url: URL) -> Bool {
        return installed[url] ?? false
    }
    
    open func installPackage(atURL url: URL) {
        NXEAssetLibrary.addAssetPackageDirectoryURLs([url])
        installed[url] = true
    }
    
    open func uninstallPackage(atURL url: URL) {
        // Not Implemented Yet
        NXEAssetLibrary.removeAssetPackageDirectoryURLs([url])
        installed[url] = false
    }
    
    open func installAllPackages() {
        let packageURLs = self.scanPackageURLs()
        for location in packageURLs.keys {
            for url in packageURLs[location]! {
                installPackage(atURL: url)
            }
        }
    }
    
    private init() { }
    
    public static var manager: NexEditorAssetPackageManager = {
        return NexEditorAssetPackageManager()
    }()
}


extension URL {
    func urlsForSubdirectories() -> [URL] {
        do {
            return try FileManager.default.contentsOfDirectory(at: self,
                                                               includingPropertiesForKeys: [.isDirectoryKey],
                                                               options: .skipsHiddenFiles)
        } catch _ {
            return []
        }
    }
}
