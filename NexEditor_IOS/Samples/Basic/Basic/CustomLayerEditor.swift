/*
 * File Name   : CustomLayerEditor.swift
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

import UIKit
import NexEditorFramework

extension NXELayer {
    var angleRadian: Double {
        return Double(angle) * Double.pi / 180
    }
}

class CustomLayerEditor: NSObject, NXELayerEditor {
    
    var isRenderRegionSizeKnown = false
    var borderView: SelectionView!
    
    var tapRecognizer: UITapGestureRecognizer? = nil
    
    override init() {
        super.init()
    }
    
    // MARK: - NXELayerEditor
    var customization: NXELayerEditorCustomization!

    public func editorView(_ editorView: NXELayerEditorView!, didChangeStatus status: NXELayerEditorStatusKey, value: Bool) {
        if status == .registered {
            if value {
                registered(editorView: editorView)
            } else {
                unregistered(editorView: editorView)
            }
        } else if status == .editingEnabled {
            // TODO: Control editing enable status
        }
    }

    /// Changes currently selected NXELayer and updates selection visual
    var selectedLayer: NXELayer? = nil {
        didSet(old) {
            if let selectedLayer = selectedLayer {
                showSelectionBorder(for: selectedLayer)
            } else {
                borderView.isHidden = true
            }
        }
    }

    // MARK: -
    func registered(editorView: NXELayerEditorView) {
        
        tapRecognizer = UITapGestureRecognizer(target: self, action: #selector(CustomLayerEditor.didTap(sender:)))
        editorView.addGestureRecognizer(tapRecognizer!)
        
        borderView = SelectionView()
        editorView.addSubview(borderView)
        
        borderView.isHidden = true
        
        borderView.onScale = self.onScale(_:_:_:)
        borderView.onRotate = self.onRotate(_:_:_:)
        borderView.onMove = self.onMove(_:_:_:)
    }
    
    func unregistered(editorView: NXELayerEditorView) {
        if let tapRecognizer = tapRecognizer {
            editorView.removeGestureRecognizer(tapRecognizer)
        }
        borderView.removeFromSuperview()
        borderView = nil
    }
    
    var editorView: NXELayerEditorView? {
        return borderView.superview as? NXELayerEditorView
    }
    
    func showSelectionBorder(for layer: NXELayer) {
        guard let editorView = editorView else {
            print("Current layer editor not registered to layer editor view")
            return
        }
        guard let layerCoordinates = editorView.layerCoordinates else {
            print("Layer coordinates converter not available. Is the preview layer set?")
            return
        }
        
        var bounds = layerCoordinates.toUIViewRect(layer.scaledFrame)
        let center = CGPoint(x: bounds.midX, y: bounds.midY)
        // scale
        bounds.origin.x = 0
        bounds.origin.y = 0
        bounds.size.width *= layer.scale
        bounds.size.height *= layer.scale
        
        borderView.innerViewBounds = bounds
        borderView.angle = CGFloat(layer.angleRadian)
        
        borderView.center = center
        borderView.isHidden = false
        
        // Let the borderView.layer super impose previewLayer by adding again
        borderView.removeFromSuperview()
        editorView.addSubview(borderView)
    }
    
    func didTap(sender: UITapGestureRecognizer) {
        guard let editorView = editorView else { return }
        guard let layers = editorView.nxeeditor.project?.layers else { return }
        if layers.count == 0 { return }
        
        if sender.state == .ended {
            let location = sender.location(in: editorView)
            self.selectedLayer = editorView.layer(atUIViewPoint: location, layers: layers)
        }
    }
    
    var initialAngle: CGFloat = 0.0
    func onRotate(_ sender: SelectionView, _ state:  UIGestureRecognizerState, _ angle: Double) {
        guard let editorView = editorView else { return }
        guard let selectedLayer = self.selectedLayer else { return }
        if state == .began {
            self.initialAngle = CGFloat(selectedLayer.angle)
        } else if state == .changed {
            let degree = angle * 180.0 / Double.pi
            selectedLayer.angle = Float(self.initialAngle + CGFloat(degree))
            editorView.updatePreview()
        }
    }
    
    var initialScale: CGFloat = 1.0
    func onScale(_ sender: SelectionView, _ state: UIGestureRecognizerState, _ scale: Double) {
        guard let editorView = editorView else { return }
        guard let selectedLayer = self.selectedLayer else { return }
        if state == .began {
            self.initialScale = selectedLayer.scale
        } else if state == .changed {
            selectedLayer.scale = self.initialScale * CGFloat(scale)
            editorView.updatePreview()
        }
    }
    
    var initialPosition: CGPoint = CGPoint()
    func onMove(_ sender: SelectionView, _ state: UIGestureRecognizerState, _ move: CGSize) {
        guard let editorView = editorView else { return }
        guard let selectedLayer = self.selectedLayer else { return }
        if state == .began {
            self.initialPosition = selectedLayer.scaledFrame.origin
        } else if state == .changed {
            guard let layerCoordinates = editorView.layerCoordinates else {
                print("Layer coordinates not available. Is the preview layer set?")
                return
            }
            let layerMove = layerCoordinates.size(from: move)
            selectedLayer.scaledFrame.origin = CGPoint(x: self.initialPosition.x + layerMove.width, y: self.initialPosition.y + layerMove.height)
            editorView.updatePreview()
        }
    }
}
