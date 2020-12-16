/*
 * File Name   : ZoomCollageEditView.swift
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

extension CGRect {
    /// 0.0 < scale <= 1.0
    func insetSquareBy(scale: CGFloat) -> CGRect {
        var width = size.width > size.height ? size.height : size.width
        width *= scale
        return insetBy(dx: (size.width - width) / 2, dy: (size.height - width) / 2)
    }
    
    func affineTransformForZoom(in bounds: CGRect, fill: CGFloat) -> CGAffineTransform {
        let width = self.size.width > self.size.height ? self.size.width : self.size.height
        
        let tx = bounds.midX - self.midX
        let ty = bounds.midY - self.midY
        let narrowSide = bounds.size.width > bounds.size.height ? bounds.size.height : bounds.size.width
        let scalexy = (narrowSide * fill) / width
        return CGAffineTransform(scaleX: scalexy, y: scalexy).translatedBy(x: tx, y: ty)
    }
}

class ZoomCollageEditView: CollageEditView {
    
    func zoomForFocusRect(_ bounds: CGRect, fill: CGFloat) {
        guard let preview = self.preview else { return }
        let transform = bounds.affineTransformForZoom(in: preview.bounds, fill: fill)
        
        if let previewLayer = preview.layer.sublayers!.filter( {$0 is CAEAGLLayer}).first as? CAEAGLLayer {
            previewLayer.setAffineTransform(CGAffineTransform.identity)
            previewLayer.setAffineTransform(transform)
        }
    }
    
    func resetZoom() {
        guard let preview = self.preview else { return }
        if let previewLayer = preview.layer.sublayers!.filter( {$0 is CAEAGLLayer}).first as? CAEAGLLayer {
            previewLayer.setAffineTransform(CGAffineTransform.identity)
        }
    }
    
    override func dismissSlotEditView(_ view: SlotEditView) {
        super.dismissSlotEditView(view)
        resetZoom()
    }

    override func didSelectSlot(at index: Int) {
        guard let project = self.project else { return }
        
        let fill = CGFloat(0.7)
        let slot = project.info.slots[index]
        let rect = project.info.convert(from: slot.rect, to: preview)
        self.zoomForFocusRect(rect, fill: fill)
        
        let focusRect = self.bounds.insetSquareBy(scale: fill)
        self.showSlotEditView(project: project, slotIndex: index, focusRect: focusRect)
    }

}
