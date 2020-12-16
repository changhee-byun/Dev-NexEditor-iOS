/*
 * File Name   : SelectionView.swift
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
import UIKit

extension CGSize {
    var diagonalLength: Double {
        return sqrt((Double) (self.width * self.width + self.height * self.height))
    }
}
/*
 ```
 let scaler = ScalerByPanGesture(with: self.view)
 scaler.onScale = { (scaler, state, scale) in
 if state == .began {
 self.initialSize = self.view.bounds.size
 } else if state == .changed {
 self.view.bounds.size = CGSize(width: initialSize.width * scale, height: initialSize.height * scale )
 }
 }
 scaler.didPan(sender: panGestureRecognizer)
 ```
 */

public class ScalerByPanGesture {
    private let view: UIView
    private var initialDiagonal: Double = 0
    
    public var onChange: ((_ sender: ScalerByPanGesture, _ state: UIGestureRecognizerState, _ scale: Double ) -> Void) = { ( sender, state, scale) in }
    public var customState: Any?
    
    public init( with view: UIView) {
        self.view = view
    }
    
    /// Finds the closest corner of view from location and returns the distance
    func offsetToClosestCorner(in view: UIView, from location: CGPoint) -> CGSize {
        // 4 cornors of the target view
        let center = view.center
        let size = view.bounds.size
        let spots = [CGPoint(x: center.x - size.width / 2, y: center.y - size.height / 2),
                     CGPoint(x: center.x - size.width / 2, y: center.y + size.height / 2),
                     CGPoint(x: center.x + size.width / 2, y: center.y + size.height / 2),
                     CGPoint(x: center.x + size.width / 2, y: center.y - size.height / 2),
                     ]
        
        var shortest: Double?
        var result: CGSize?
        for spot in spots {
            let offset = CGSize(width: spot.x - location.x, height: spot.y - location.y)
            let distance = offset.diagonalLength
            if shortest == nil || shortest! > distance {
                shortest = distance
                result = offset
            }
        }
        return result!
    }
    
    var offsetToDragSpot: CGSize = CGSize()
    public func didPan(sender: UIPanGestureRecognizer) {
        // scale
        let location = sender.location(in: self.view.superview)
        let scale: Double
        
        let center = self.view.center
        if sender.state == .began {
            offsetToDragSpot = offsetToClosestCorner(in: self.view, from: location)
            let dragSpot = CGPoint(x: location.x + offsetToDragSpot.width, y: location.y + offsetToDragSpot.height)
            initialDiagonal = CGSize(width: dragSpot.x - center.x, height: dragSpot.y - center.y).diagonalLength
            scale = 1.0
        } else if sender.state == .changed || sender.state == .ended {
            let dragSpot = CGPoint(x: location.x + offsetToDragSpot.width, y: location.y + offsetToDragSpot.height)
            let changedDiagonal = CGSize(width: dragSpot.x - center.x, height: dragSpot.y - center.y).diagonalLength
            scale = changedDiagonal / initialDiagonal
        } else {
            scale = 1.0
        }
        
        onChange(self, sender.state, scale)
    }
}

public class RotatorByPanGesture {
    private let view: UIView
    private var initialAngle: Double = 0
    public var onChange: ((_ sender: RotatorByPanGesture, _ state: UIGestureRecognizerState, _ angle: Double ) -> Void) = { ( sender, state, angle) in }
    public var customState: Any?
    
    public init( with view: UIView) {
        self.view = view
    }
    
    public func didPan(sender: UIPanGestureRecognizer) {
        let location = sender.location(in: self.view.superview)
        let center = self.view.center
        
        let angle = Double(atan2(location.x - center.x, location.y - center.y))
        
        var update = false
        if sender.state == .began {
            initialAngle = angle
            update = true
        } else if sender.state == .changed || sender.state == .ended {
            update = true
        }
        
        if update {
            onChange(self, sender.state, initialAngle - angle)
        }
    }
}

class SelectionView: UIView {
    
    enum ControlButton: UInt {
        case scale = 0
        case rotate = 1
        case delete = 2
    }
    var onScale: ((_ sender: SelectionView, _ state: UIGestureRecognizerState, _ scale: Double ) -> Void) = { ( sender, state, scale) in }
    var onRotate: ((_ sender: SelectionView, _ state: UIGestureRecognizerState, _ angle: Double ) -> Void) = { ( sender, state, angle) in }
    var onMove: ((_ sender: SelectionView, _ state: UIGestureRecognizerState, _ move: CGSize ) -> Void) = { ( sender, state, move) in }
    
    var buttonImageViews: [UIImageView]!
    var innerView: UIView!
    
    var innerViewInset: CGSize = CGSize(width: 32, height: 32) {
        didSet(old) {
            self.setNeedsLayout()
        }
    }
    
    var innerViewBounds: CGRect {
        get {
            return innerView.bounds
        }
        set(bounds) {
            innerView.bounds = bounds
            self.bounds.size = CGSize(width: bounds.size.width + innerViewInset.width * 2, height: bounds.size.height + innerViewInset.height * 2)
        }
    }
    
    /// Radian
    var angle: CGFloat = 0 {
        didSet(old) {
            self.transform = CGAffineTransform.identity.rotated(by: angle)
        }
    }
    
    private var scaler: ScalerByPanGesture!
    private var rotator: RotatorByPanGesture!
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        commonInit()
    }
    
    required init?(coder aDecoder: NSCoder) {
        super.init(coder: aDecoder)
    }
    
    override func awakeFromNib() {
        super.awakeFromNib()
        commonInit()
    }
    
    func commonInit() {
        
        let buttonImageNames = ["scale", "rotate", "delete"]
        
        buttonImageViews = buttonImageNames.map { UIImageView(image: UIImage(named: $0)) }
        for imageView in buttonImageViews {
            self.addSubview(imageView)
            let pan = UIPanGestureRecognizer(target: self, action: #selector(SelectionView.didPan(sender:)))
            imageView.addGestureRecognizer(pan)
            imageView.isUserInteractionEnabled = true
        }
        
        innerView = UIView()
        innerView.backgroundColor = UIColor.green.withAlphaComponent(0.1)
        innerView.layer.shadowColor = UIColor.red.cgColor
        innerView.layer.shadowOffset = CGSize(width: 1, height: 1)
        innerView.layer.shadowOpacity = 1.0
        let pan = UIPanGestureRecognizer(target: self, action: #selector(SelectionView.didPan(sender:)))
        innerView.addGestureRecognizer(pan)
        self.addSubview(innerView)
        
        scaler = ScalerByPanGesture(with: self.innerView)
        scaler.onChange = { [weak self] (sender, state, scale) in
            guard let me = self else { return }
            
            if state == .began {
                sender.customState = me.innerView.bounds.size
            } else if state == .changed {
                if let initialSize = sender.customState as? CGSize {
                    me.bounds.size = CGSize(width: initialSize.width * CGFloat(scale) + me.innerViewInset.width * 2,
                                            height: initialSize.height * CGFloat(scale) + me.innerViewInset.height * 2 )
                }
            }
            
            me.onScale(me, state, scale)
        }
        
        rotator = RotatorByPanGesture(with: self)
        rotator.onChange = { [weak self] (sender, state, angle) in
            guard let me = self else { return }
            
            if state == .began {
                sender.customState = me.transform
            } else if state == .changed {
                if let initialTransform = sender.customState as? CGAffineTransform {
                    me.transform = initialTransform.rotated(by: CGFloat(angle))
                }
            }
            
            me.onRotate(me, state, angle)
        }
    }
    
    override func layoutSubviews() {
        super.layoutSubviews()
        let imageSize = buttonImageViews[0].image!.size
        buttonImageViews[0].frame = CGRect(x: 0, y: 0, width: imageSize.width, height: imageSize.height)
        buttonImageViews[1].frame = CGRect(x: 0, y: self.bounds.size.height - imageSize.height, width: imageSize.width, height: imageSize.height)
        buttonImageViews[2].frame = CGRect(x: self.bounds.size.width - imageSize.width,
                                           y: 0, width: imageSize.width, height: imageSize.height)
        
        innerView.bounds = CGRect(x: 0, y: 0, width: self.bounds.size.width - innerViewInset.width * 2, height: self.bounds.size.height - innerViewInset.height * 2)
        innerView.center = self.convert(self.center, from: self.superview)
    }
    
    private var initialLocation: CGPoint = CGPoint()
    private var initialCenter: CGPoint = CGPoint()
    func didPan(sender: UIPanGestureRecognizer) {
        if sender.view == buttonImageViews[0] {
            self.scaler.didPan(sender: sender)
        } else if sender.view == buttonImageViews[1] {
            self.rotator.didPan(sender: sender)
        } else if sender.view == innerView {
            let location = sender.location(in: self.superview)
            let move: CGSize
            if sender.state == .began {
                self.initialLocation = location
                self.initialCenter = self.center
                move = CGSize(width: 0, height: 0)
            } else if sender.state == .changed || sender.state == .ended {
                move = CGSize(width: location.x - initialLocation.x, height: location.y - initialLocation.y )
            } else {
                move = CGSize(width: 0, height: 0)
            }
            
            self.center = CGPoint(x: self.initialCenter.x + move.width, y: self.initialCenter.y + move.height)
            onMove(self, sender.state, move)
        }
    }
}
