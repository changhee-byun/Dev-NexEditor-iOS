//
//  NexEditorEngineWrapper.swift
//  EditorDemo
//
//  Created by CHANGHEE2 on 2020/12/23.
//

import Foundation
//import NexEditorEngine

//class NexEditorEngineWrapper {
//    var nxeEngine : NXEEngine = NXEEngine()
//    var preview : UIView = UIView()
//    
//    init(engine: NXEEngine) {
//        self.nxeEngine = engine
//        setDefault()
//        
//    }
//    
//    convenience init() {
//        self.init(engine: NXEEngine())
//    }
//    
//    func setDefault() {
//        enableEditorLog(true)
//        self.nxeEngine.aspectType = .ratio9v16
//        self.nxeEngine.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
//    }
//    
//    func refreshPreview() {
//        DispatchQueue.main.async {
//            self.nxeEngine.setPreviewWith(self.preview.layer, previewSize: self.preview.bounds.size)
//        }
//    }
//    
//    func refreshEngine() {
//        nxeEngine = NXEEngine()
//        setDefault()
//    }
//    
//    func enableEditorLog(_ enabled:Bool) {
//        let level = enabled ? 0 : 4
//        NexEditorLog.setLogLevel(level, for: .err)
//        NexEditorLog.setLogLevel(level, for: .warning)
//        NexEditorLog.setLogLevel(level, for: .info)
//        NexEditorLog.setLogLevel(level, for: .debug)
//    }
//}
