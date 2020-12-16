/*
 * File Name   : CollageEditView.swift
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

extension NXECollageProject {
    func editRegions(on preview: UIView) -> [SelectableRegion] {
        var regions: [SelectableRegion] = []
        for slot in self.info.slots {
            let path = self.info.convert(fromPositions: slot.position, to: preview) as! [CGPoint]
            let bounds = self.info.convert(from: slot.rect, to: preview)
            regions.append( SelectableRegion(path: path, bounds: bounds) )
        }
        for title in self.info.titles {
            let path = self.info.convert(fromPositions: title.position, to: preview) as! [CGPoint]
            let bounds = self.info.convert(from: title.rect, to: preview)
            regions.append( SelectableRegion(path: path, bounds: bounds) )
        }
        return regions
    }
}

class CollageEditView: RegionSelectorView {
    var preview: UIView?
    var project: NXECollageProject?
    public var onSlotEdit: ((_ slot: CollageSlotConfiguration?, _ index: Int?)->Void)?
    
    // Shows the slot edit view for the slot at slotIndex
    func showSlotEditView(project: NXECollageProject, slotIndex index: Int, focusRect: CGRect) {
        guard let superview = self.superview else { return }
        
        let slotEditView = SlotEditView(frame: self.bounds)
        superview.addSubview(slotEditView)
        slotEditView.center = self.center
        
        slotEditView.focusRect = slotEditView.convert(focusRect, from: self)
        
        let slotConfig = project.slots[index]
        let slotRect = project.info.convert(from: project.info.slots[index].rect, to: preview!)
        slotEditView.onScale = { (view, scale) in
            slotConfig.scale = CGFloat(scale)
        }
        
        slotEditView.onRotate = { (view, angle) in
            let degree = Int32(angle * 180 / Double.pi)
            slotConfig.angle = degree
        }
        
        var lastPosition = slotConfig.position
        slotEditView.onMove = { (view, began, move) in
            if began {
                lastPosition = slotConfig.position
            }
            let position = CGPoint(x: move.x / slotRect.size.width, y: move.y / slotRect.size.height)
            slotConfig.position = CGPoint(x: position.x + lastPosition.x, y: position.y + lastPosition.y )
        }
        
        slotEditView.onTap = { [weak self] (view) in
            self?.dismissSlotEditView(view)
        }
        
        self.isHidden = true
        onSlotEdit?(slotConfig, index)
    }
    
    func dismissSlotEditView(_ view: SlotEditView) {
        view.removeFromSuperview()
        self.isHidden = false
        self.selectedRegionIndex = NSNotFound
        self.onSlotEdit?(nil, nil)
    }
    
    /// override to hook up the moment nothing is selected
    func didDeselect() {
        
    }
    
    /// override to hook up the moment a slot selected. Default behavior is to focus the slot and allow user to rotate or scale
    func didSelectSlot(at index: Int) {
        guard let project = self.project else { return }
        
        let slot = project.info.slots[index]
        let rect = project.info.convert(from: slot.rect, to: preview)
        
        showSlotEditView(project: project, slotIndex: index, focusRect: rect)
    }

    /// override to hook up the moment a title selected. Default bahavior does nothing.
    func didSelectTitle(at index: Int) {
        print("Selected title: \(index)")
    }
    
    /// Prepares slot editing by visualizing slot positions and outlines
    public func setProject(_ project: NXECollageProject, preview: UIView) {
        
        self.project = project
        self.preview = preview
        self.regions = project.editRegions(on: preview)
        
        self.onSelect = { [weak self] (index) in
            guard let me = self else { return }
            if index == NSNotFound {
                me.didDeselect()
                return
            }
            
            if index < project.info.slots.count {
                me.didSelectSlot(at: index)
            } else {
                let titleIndex = index - project.info.slots.count
                me.didSelectTitle(at: titleIndex)
            }
        }
    }
    
    /// Dismisses the slot edit view currently selected slot. As a side effect selectedRegionIndex will be set to NSNotFound
    public func dismissSlotEditView() {
        if let slotEditView = self.superview!.subviews.filter({$0 is SlotEditView}).first as? SlotEditView {
            dismissSlotEditView(slotEditView)
        }
    }
    
    /// Editable regions are redrawn according to current bounds of self. Useful when self.bounds.size has been changed so the regions should be repositioned.
    public func updateRegionPositions() {
        guard let project = project else { return }
        guard let preview = preview else { return }
        self.regions = project.editRegions(on: preview)
    }
}
