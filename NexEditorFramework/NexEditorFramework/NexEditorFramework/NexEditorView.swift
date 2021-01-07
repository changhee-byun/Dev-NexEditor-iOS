//
//  NexEditorView.swift
//  NexEditorFramework
//
//  Created by CHANGHEE2 on 2021/01/07.
//  Copyright © 2021 NexStreaming Corp. All rights reserved.
//

import SwiftUI

public struct NexEditorView: UIViewRepresentable {
    public var engineWrapper : NexEditor
    
    public init(engineWrapper: NexEditor) {
        self.engineWrapper = engineWrapper
    }
    
    public func makeUIView(context: Context) -> UIView {
        engineWrapper.preview = UIView(frame: CGRect(x: 0, y: 0, width: 1000, height: 1500))
        return engineWrapper.preview
    }
    
    public func updateUIView(_ view: UIView, context: Context) {
        
    }
}
