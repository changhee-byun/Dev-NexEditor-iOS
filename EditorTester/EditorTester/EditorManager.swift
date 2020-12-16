//
//  EditorManager.swift
//  EditorTester
//
//  Created by ERIC0 on 2020/10/22.
//

import Foundation
import NexEditorEngine

class EngineManager {
    //var editor : NexEditor?
    var engine : NXEEngine?
    var view : UIView? //NXELayerEditorView?
    
    // view has to be ready
    func makeReady() {
        setLogEngabled(true)
        engine = NXEEngine()
        engine?.aspectType = .ratio9v16
        engine?.setPreviewWith(view?.layer, previewSize: (view?.bounds.size)!)
        
    }
    
    func setLogEngabled(_ enabled:Bool) {
        let level = enabled ? 0 : 4
        NexEditorLog.setLogLevel(level, for: .err)
        NexEditorLog.setLogLevel(level, for: .warning)
        NexEditorLog.setLogLevel(level, for: .info)
        NexEditorLog.setLogLevel(level, for: .debug)
    }
}
