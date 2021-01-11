//
//  NexEditorView.swift
//  NexEditorFramework
//
//  Created by CHANGHEE2 on 2021/01/07.
//  Copyright © 2021 NexStreaming Corp. All rights reserved.
//

import SwiftUI

public struct NexEditorView: UIViewRepresentable {
    public var engineExt : NexEditorExt
    public var viewFrame: CGRect
    
    public init(engine: NexEditorExt, viewFrame: CGRect) {
        self.engineExt = engine
        self.viewFrame = viewFrame
    }
    
    public init(engine: NexEditorExt) {
        self.init(engine: engine, viewFrame: CGRect(x: 0, y: 0, width: 900, height: 1600))
    }
    
    public init(engine: NexEditorExt, viewWidth: CGFloat) {
        self.init(engine: engine, viewFrame: CGRect(x: 0, y: 0, width: viewWidth, height: viewWidth*16/9))
    }
    
    public init(engine: NexEditorExt, viewWidth: CGFloat, viewHeight: CGFloat) {
        self.init(engine: engine, viewFrame: CGRect(x: 0, y: 0, width: viewWidth, height: viewHeight))
    }
    
    public init(engine: NexEditorExt, viewSize: CGSize) {
        self.init(engine: engine, viewFrame: CGRect(x: 0, y: 0, width: viewSize.width, height: viewSize.height))
    }
    
    public func makeUIView(context: Context) -> UIView {
        engineExt.preview = UIView(frame: viewFrame)
        return engineExt.preview
    }
    
    public func updateUIView(_ view: UIView, context: Context) {
        
    }
}
