//
//  NexEditorFrameworkWrapper.swift
//  NexEditorFramework
//
//  Created by CHANGHEE2 on 2021/01/07.
//  Copyright © 2021 NexStreaming Corp. All rights reserved.
//

import Foundation


public class NexEditor {
    public var nxeEngine : NXEEngine = NXEEngine()
    public var preview : UIView = UIView()
    
    public init(engine: NXEEngine) {
        self.nxeEngine = engine
        setDefault()
        
    }
    
    public convenience init() {
        self.init(engine: NXEEngine())
    }
    
    public func setDefault() {
        enableEditorLog(true)
        self.nxeEngine.aspectType = .ratio9v16
        self.nxeEngine.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
    }
    
    public func refreshPreview() {
        DispatchQueue.main.async {
            self.nxeEngine.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
        }
    }
    
    public func refreshEngine() {
        nxeEngine = NXEEngine()
        setDefault()
    }
    
    public func enableEditorLog(_ enabled:Bool) {
        let level = enabled ? 0 : 4
//        NexEditorLog.setLogLevel(level, for: .err)
//        NexEditorLog.setLogLevel(level, for: .warning)
//        NexEditorLog.setLogLevel(level, for: .info)
//        NexEditorLog.setLogLevel(level, for: .debug)
    }
}
