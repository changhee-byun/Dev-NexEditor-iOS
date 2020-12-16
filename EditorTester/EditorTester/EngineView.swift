//
//  EngineView.swift
//  EditorTester
//
//  Created by ERIC0 on 2020/10/26.
//

import SwiftUI
import NexEditorEngine

struct EngineView: UIViewRepresentable {
    var engineMan : EngineManager? = nil
    
    func makeUIView(context: Context) -> UIView {
        engineMan?.view = UIView(frame: CGRect(x: 0, y: 0, width: 600, height: 400))
        // NXELayerEditorView(frame: CGRect(x: 0, y: 0, width: 600, height: 400))
        return engineMan!.view!
    }
    
    func updateUIView(_ view: UIView, context: Context) {
//        if ( engineMan?.editor?.layer != nil ) {
//            view.layer.addSublayer((engineMan?.editor?.layer)!)
//        }
    }
}

struct EngineView_Previews: PreviewProvider {
    static var previews: some View {
        EngineView(engineMan:EngineManager())
    }
}

