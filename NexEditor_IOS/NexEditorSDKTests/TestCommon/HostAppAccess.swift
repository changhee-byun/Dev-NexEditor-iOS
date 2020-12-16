//
//  HostAppAccess.swift
//  NexEditorFramework
//
//  Created by Simon Kim on 6/29/17.
//  Copyright © 2017 NexStreaming Corp. All rights reserved.
//

import Foundation
import UIKit
import XCTest
@testable import NexEditorSDKTest

extension XCTestCase {
    var viewController: ViewController {
        let nvc = UIApplication.shared.keyWindow?.rootViewController as! UINavigationController
        return nvc.topViewController as! ViewController
    }
    var preview: UIView! {
        return viewController.preview!
    }
    
    var label: UILabel! {
        return viewController.labelTestCase!
    }
    
    var documentsUrl: URL {
        return FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    }
    
    var clipsourceUrl: URL {
        return documentsUrl.appendingPathComponent("ClipSources")
    }
    
    var assetBundlesUrl: URL {
        return documentsUrl.appendingPathComponent("AssetBundles")
    }
}
