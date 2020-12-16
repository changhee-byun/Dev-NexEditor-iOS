/*
 * File Name   : RegionSelectorView.swift
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

class SelectableRegion {
    let path: [CGPoint]
    let bounds: CGRect
    
    init(path: [CGPoint], bounds: CGRect)
    {
        self.path = path
        self.bounds = bounds
    }
    
    func cgPath() -> CGPath {
        let bezierPath = UIBezierPath()
        if path.count > 0 {
            bezierPath.move(to: path[0])
            for i in 1..<path.count {
                bezierPath.addLine(to: path[i])
            }
        }
        return bezierPath.cgPath
    }
}

private let colorUnselected = UIColor.red // UIColor.clear
private let colorSelected = UIColor.green // UIColor.red

// select region
// deselect region
// rotate region
// zoom region
class RegionSelectorView: UIView {
    
    private var shapes: [CAShapeLayer] = []
    public var regions: [SelectableRegion] = [] {
        didSet(old) {
            if selectedRegionIndex != NSNotFound && selectedRegionIndex >= regions.count {
                selectedRegionIndex = NSNotFound
            }
            self.updateShapesWithRegions(regions)
        }
    }
    
    public var selectedRegionIndex: Int = NSNotFound {
        didSet(old) {
            if selectedRegionIndex == old { return }
            
            if old != NSNotFound {
                let shape = shapes[old]
                shape.strokeColor = colorUnselected.cgColor
            }
            onSelect?(selectedRegionIndex)
            if selectedRegionIndex != NSNotFound {
                let shape = shapes[selectedRegionIndex]
                shape.strokeColor = colorSelected.cgColor
            }
        }
    }
    
    public var onSelect: ((_ index: Int)->Void)?
    
    func commonInit() {
        let tap = UITapGestureRecognizer(target: self, action: #selector(didTap(_:)))
        self.addGestureRecognizer(tap)
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
        commonInit()
    }
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }
    
    func updateShapesWithRegions(_ regions: [SelectableRegion]) {
        while(shapes.count > regions.count) {
            let shape = shapes.removeLast()
            shape.removeFromSuperlayer()
        }
        // existing shapes
        for (index, shape) in shapes.enumerated() {
            let region = regions[index]
            shape.path = region.cgPath()
        }
        // new regions
        for i in shapes.count..<regions.count {
            let region = regions[i]
            let shape = CAShapeLayer()
            shape.strokeColor = colorUnselected.cgColor
            shape.lineWidth = 2
            shape.lineJoin = CAShapeLayerLineJoin.round
            shape.fillColor = UIColor.clear.cgColor
            shape.path = region.cgPath()
            self.layer.addSublayer(shape)
            self.shapes.append(shape)
        }
    }
    
    // MARK: - Gesture recognizers
    @objc func didTap(_ sender: UITapGestureRecognizer) {
        let location = sender.location(in: self)
        
        var selected: Int? = nil
        for (index, shape) in shapes.enumerated() {
            if shape.path!.boundingBox.contains(location) {
                selected = index
                break
            }
        }
        selectedRegionIndex = selected ?? NSNotFound
    }
}
